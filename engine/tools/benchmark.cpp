// Head-to-head benchmark harness between AI strategies, used to produce the
// win-rate numbers quoted in the README. The doctest suite has a smaller,
// fast version of this same idea for CI; this tool uses bigger budgets to
// get a number worth quoting.
//
// Usage: hex_benchmark [board_size] [games] [mcts_iterations] [flat_trials]
#include <cstdlib>
#include <iostream>
#include <string>

#include "hex/ai.hpp"
#include "hex/board.hpp"
#include "hex/game.hpp"

using namespace hex;

namespace {

Player playGame(AIStrategy& first, AIStrategy& second, Player firstSide, int boardSize) {
    Game game(boardSize, firstSide);
    AIStrategy* toMoveAi = &first;
    while (game.status() == GameStatus::InProgress) {
        Player side = game.toMove();
        auto move = toMoveAi->selectMove(game.board(), side);
        game.applyMove(move.first, move.second, side);
        toMoveAi = (toMoveAi == &first) ? &second : &first;
    }
    return game.winner();
}

struct MatchResult {
    int aWins = 0;
    int bWins = 0;
};

MatchResult runMatch(const char* labelA, AIStrategy& a, const char* labelB, AIStrategy& b, int games,
                      int boardSize) {
    MatchResult result;
    for (int i = 0; i < games; ++i) {
        Player aSide = (i % 2 == 0) ? Player::Blue : Player::Red;
        Player winner =
            (aSide == Player::Blue) ? playGame(a, b, Player::Blue, boardSize) : playGame(b, a, Player::Blue, boardSize);
        if (winner == aSide) {
            result.aWins++;
        } else {
            result.bWins++;
        }
    }
    std::cout << labelA << " vs " << labelB << " on a " << boardSize << "x" << boardSize << " board, "
              << games << " games (alternating who moves first and which side each plays):\n"
              << "  " << labelA << ": " << result.aWins << " (" << (100.0 * result.aWins / games) << "%)\n"
              << "  " << labelB << ": " << result.bWins << " (" << (100.0 * result.bWins / games) << "%)\n";
    return result;
}

} // namespace

int main(int argc, char** argv) {
    int boardSize = argc > 1 ? std::atoi(argv[1]) : 7;
    int games = argc > 2 ? std::atoi(argv[2]) : 100;
    int mctsIterations = argc > 3 ? std::atoi(argv[3]) : 1500;
    int flatTrials = argc > 4 ? std::atoi(argv[4]) : 100;

    RandomAI random(/*seed=*/1);
    FlatMonteCarloAI flat(flatTrials, /*seed=*/2);
    MctsAI mcts(mctsIterations, 1.4142135623730951, /*seed=*/3);

    runMatch("MCTS", mcts, "Random", random, games, boardSize);
    std::cout << "\n";
    runMatch("MCTS", mcts, "FlatMonteCarlo", flat, games, boardSize);
    std::cout << "\n";
    runMatch("FlatMonteCarlo", flat, "Random", random, games, boardSize);
    return 0;
}
