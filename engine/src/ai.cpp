#include "hex/ai.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <stdexcept>

namespace hex {

// ---------------------------------------------------------------- RandomAI

RandomAI::RandomAI(std::mt19937::result_type seed) : rng_(seed) {}

std::pair<int, int> RandomAI::selectMove(const Board& board, Player /*player*/) {
    auto empties = board.emptyCells();
    if (empties.empty()) throw std::runtime_error("RandomAI::selectMove: board is full");
    std::uniform_int_distribution<std::size_t> dist(0, empties.size() - 1);
    return empties[dist(rng_)];
}

// -------------------------------------------------------- FlatMonteCarloAI

FlatMonteCarloAI::FlatMonteCarloAI(int trials, std::mt19937::result_type seed)
    : trials_(trials), rng_(seed) {}

double FlatMonteCarloAI::rollout(Board board, Player nextToMove, Player evaluatedFor) {
    auto empties = board.emptyCells();
    std::shuffle(empties.begin(), empties.end(), rng_);
    Player turn = nextToMove;
    for (auto& cell : empties) {
        board.place(cell.first, cell.second, turn);
        turn = opponent(turn);
    }
    return board.winner() == evaluatedFor ? 1.0 : 0.0;
}

std::pair<int, int> FlatMonteCarloAI::selectMove(const Board& board, Player player) {
    auto empties = board.emptyCells();
    if (empties.empty()) throw std::runtime_error("FlatMonteCarloAI::selectMove: board is full");

    double bestRate = -1.0;
    std::pair<int, int> bestMove = empties.front();
    for (auto& cell : empties) {
        Board trial = board;
        trial.place(cell.first, cell.second, player);
        int wins = 0;
        for (int t = 0; t < trials_; ++t) {
            wins += rollout(trial, opponent(player), player) > 0.5 ? 1 : 0;
        }
        double rate = static_cast<double>(wins) / trials_;
        if (rate > bestRate) {
            bestRate = rate;
            bestMove = cell;
        }
    }
    return bestMove;
}

// ------------------------------------------------------------------ MctsAI

namespace {

struct MctsNode {
    Player toMove;                                    // player to move AT this node
    std::pair<int, int> move{-1, -1};                 // move that led here (invalid at root)
    MctsNode* parent = nullptr;
    std::vector<std::unique_ptr<MctsNode>> children;
    std::vector<std::pair<int, int>> untried;
    int visits = 0;
    double wins = 0.0; // wins credited to parent->toMove (the player who made `move`)

    MctsNode(Player toMove_, std::vector<std::pair<int, int>> untried_, MctsNode* parent_,
             std::pair<int, int> move_)
        : toMove(toMove_), move(move_), parent(parent_), untried(std::move(untried_)) {}
};

} // namespace

MctsAI::MctsAI(int iterations, double explorationConstant, std::mt19937::result_type seed)
    : iterations_(iterations), explorationConstant_(explorationConstant), rng_(seed) {}

std::pair<int, int> MctsAI::selectMove(const Board& rootBoard, Player rootPlayer) {
    auto rootEmpty = rootBoard.emptyCells();
    if (rootEmpty.empty()) throw std::runtime_error("MctsAI::selectMove: board is full");

    MctsNode root(rootPlayer, rootEmpty, nullptr, {-1, -1});

    for (int iter = 0; iter < iterations_; ++iter) {
        Board board = rootBoard;
        MctsNode* node = &root;

        // ---- Selection: descend via UCT while every child has been tried ----
        while (node->untried.empty() && !node->children.empty()) {
            double bestScore = -std::numeric_limits<double>::infinity();
            MctsNode* best = nullptr;
            for (auto& childPtr : node->children) {
                MctsNode* child = childPtr.get();
                double winRate = child->wins / std::max(1, child->visits);
                double exploration = explorationConstant_ *
                    std::sqrt(std::log(static_cast<double>(node->visits + 1)) / std::max(1, child->visits));
                double score = winRate + exploration;
                if (score > bestScore) {
                    bestScore = score;
                    best = child;
                }
            }
            node = best;
            board.place(node->move.first, node->move.second, opponent(node->toMove));
        }

        // ---- Expansion: try one untried move, if any and not terminal ----
        Player decided = board.winner();
        if (decided == Player::None && !node->untried.empty()) {
            std::uniform_int_distribution<std::size_t> dist(0, node->untried.size() - 1);
            std::size_t idx = dist(rng_);
            std::pair<int, int> move = node->untried[idx];
            node->untried.erase(node->untried.begin() + static_cast<long>(idx));

            board.place(move.first, move.second, node->toMove);
            auto child =
                std::make_unique<MctsNode>(opponent(node->toMove), board.emptyCells(), node, move);
            MctsNode* childPtr = child.get();
            node->children.push_back(std::move(child));
            node = childPtr;
            decided = board.winner();
        }

        // ---- Simulation: random rollout to a forced decision ----
        Player simWinner = decided;
        if (simWinner == Player::None) {
            auto empties = board.emptyCells();
            std::shuffle(empties.begin(), empties.end(), rng_);
            Player turn = node->toMove;
            for (auto& cell : empties) {
                board.place(cell.first, cell.second, turn);
                turn = opponent(turn);
            }
            simWinner = board.winner();
        }

        // ---- Backpropagation ----
        for (MctsNode* n = node; n != nullptr; n = n->parent) {
            n->visits++;
            if (n->parent != nullptr && simWinner == n->parent->toMove) {
                n->wins += 1.0;
            }
        }
    }

    MctsNode* best = nullptr;
    int bestVisits = -1;
    for (auto& childPtr : root.children) {
        if (childPtr->visits > bestVisits) {
            bestVisits = childPtr->visits;
            best = childPtr.get();
        }
    }
    return best != nullptr ? best->move : rootEmpty.front();
}

} // namespace hex
