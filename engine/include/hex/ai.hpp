#pragma once

#include <random>
#include <utility>

#include "hex/board.hpp"

namespace hex {

// Strategy pattern: every opponent, human-facing or not, is something that
// can look at a board and a side and return a move. The backend and the
// benchmark tool both just hold an AIStrategy*.
class AIStrategy {
public:
    virtual ~AIStrategy() = default;
    virtual std::pair<int, int> selectMove(const Board& board, Player player) = 0;
    virtual const char* name() const = 0;
};

class RandomAI : public AIStrategy {
public:
    explicit RandomAI(std::mt19937::result_type seed = std::random_device{}());
    std::pair<int, int> selectMove(const Board& board, Player player) override;
    const char* name() const override { return "Random"; }

private:
    std::mt19937 rng_;
};

// The original hexupdated.cpp's AI: for every empty cell, tentatively play
// it, then run `trials` fully-random rollouts to a forced decision (Hex has
// no draws once the board fills, which is what makes "just fill the rest of
// the board randomly and see who ended up connected" a valid, if noisy,
// move evaluator), and keep the move with the best win rate.
//
// One behavioral fix versus the original: its rollout always started the
// random fill with a color fixed by which player was being evaluated,
// regardless of whose turn it actually was next. This version starts each
// rollout with the correct next player (the opponent of whoever just
// tentatively moved), so playouts respect real turn order.
class FlatMonteCarloAI : public AIStrategy {
public:
    explicit FlatMonteCarloAI(int trials = 200, std::mt19937::result_type seed = std::random_device{}());
    std::pair<int, int> selectMove(const Board& board, Player player) override;
    const char* name() const override { return "Flat Monte Carlo"; }

private:
    int trials_;
    std::mt19937 rng_;
    double rollout(Board board, Player nextToMove, Player evaluatedFor);
};

// Monte Carlo Tree Search with UCT selection. Unlike FlatMonteCarloAI, which
// spends its simulation budget evenly across every candidate move,
// MctsAI builds a search tree and concentrates simulations on the lines
// that are actually looking good, using the standard
// win_rate + c * sqrt(ln(parent_visits) / child_visits) selection rule.
// Simulation (rollout) exploits the same no-draw property as
// FlatMonteCarloAI. Final move choice is the most-visited root child
// ("robust child"), which is less noisy than picking by raw win rate.
class MctsAI : public AIStrategy {
public:
    explicit MctsAI(int iterations = 1500, double explorationConstant = 1.4142135623730951,
                     std::mt19937::result_type seed = std::random_device{}());
    std::pair<int, int> selectMove(const Board& board, Player player) override;
    const char* name() const override { return "MCTS (UCT)"; }

private:
    int iterations_;
    double explorationConstant_;
    std::mt19937 rng_;
};

} // namespace hex
