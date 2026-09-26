#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <utility>
#include <vector>

#include "hex/ai.hpp"
#include "hex/board.hpp"
#include "hex/game.hpp"

using namespace hex;

TEST_CASE("opponent() flips Red/Blue and leaves None alone") {
    CHECK(opponent(Player::Red) == Player::Blue);
    CHECK(opponent(Player::Blue) == Player::Red);
    CHECK(opponent(Player::None) == Player::None);
}

TEST_CASE("Board: empty board has no winner") {
    Board b(5);
    CHECK(b.winner() == Player::None);
}

TEST_CASE("Board: Blue wins by connecting column 0 to column size-1") {
    Board b(5);
    for (int c = 0; c < 5; ++c) {
        CHECK(b.place(2, c, Player::Blue));
    }
    CHECK(b.winner() == Player::Blue);

    auto path = b.winningPath(Player::Blue);
    REQUIRE(path.size() >= 5);
    CHECK(path.front().second == 0);
    CHECK(path.back().second == 4);
}

TEST_CASE("Board: Red wins by connecting row 0 to row size-1") {
    Board b(5);
    for (int r = 0; r < 5; ++r) {
        CHECK(b.place(r, 2, Player::Red));
    }
    CHECK(b.winner() == Player::Red);

    auto path = b.winningPath(Player::Red);
    REQUIRE(path.size() >= 5);
    CHECK(path.front().first == 0);
    CHECK(path.back().first == 4);
}

TEST_CASE("Board: a staggered zig-zag chain still counts as connected") {
    // Hex adjacency includes (r,c)-(r+1,c-1) and (r,c)-(r-1,c+1), so a
    // staircase path connects two edges without needing a straight line.
    Board b(4);
    std::vector<std::pair<int, int>> path = {{0, 0}, {1, 0}, {1, 1}, {2, 1}, {2, 2}, {3, 2}, {3, 3}};
    for (auto& cell : path) {
        CHECK(b.place(cell.first, cell.second, Player::Blue));
    }
    CHECK(b.winner() == Player::Blue);
}

TEST_CASE("Board: a broken chain does not count as a win") {
    Board b(4);
    b.place(0, 0, Player::Blue);
    b.place(2, 2, Player::Blue); // disconnected from column 0
    b.place(0, 3, Player::Blue); // touches the far edge but isolated
    CHECK(b.winner() == Player::None);
}

TEST_CASE("Game enforces turn order and rejects illegal moves") {
    Game game(5, Player::Blue);
    CHECK(game.toMove() == Player::Blue);
    CHECK_FALSE(game.applyMove(0, 0, Player::Red)); // wrong player's turn
    CHECK(game.applyMove(0, 0, Player::Blue));
    CHECK(game.toMove() == Player::Red);
    CHECK_FALSE(game.applyMove(0, 0, Player::Red)); // occupied
    CHECK(game.applyMove(1, 1, Player::Red));
    CHECK(game.toMove() == Player::Blue);
    CHECK(game.status() == GameStatus::InProgress);
}

TEST_CASE("Game reports the winner once a side connects") {
    Game game(3, Player::Blue);
    REQUIRE(game.applyMove(1, 0, Player::Blue));
    REQUIRE(game.applyMove(0, 0, Player::Red));
    REQUIRE(game.applyMove(1, 1, Player::Blue));
    REQUIRE(game.applyMove(0, 1, Player::Red));
    REQUIRE(game.applyMove(1, 2, Player::Blue));
    CHECK(game.status() == GameStatus::Finished);
    CHECK(game.winner() == Player::Blue);
}

TEST_CASE("Every AI strategy always returns a currently-empty cell") {
    Board b(4);
    b.place(0, 0, Player::Blue);
    b.place(1, 1, Player::Red);

    RandomAI randomAi(/*seed=*/1);
    auto move = randomAi.selectMove(b, Player::Blue);
    CHECK(b.at(move.first, move.second) == Player::None);

    FlatMonteCarloAI flatAi(/*trials=*/20, /*seed=*/1);
    move = flatAi.selectMove(b, Player::Blue);
    CHECK(b.at(move.first, move.second) == Player::None);

    MctsAI mctsAi(/*iterations=*/100, 1.4142135623730951, /*seed=*/1);
    move = mctsAi.selectMove(b, Player::Blue);
    CHECK(b.at(move.first, move.second) == Player::None);
}

namespace {

// Plays one full game between two strategies on an empty board, alternating
// turns starting with `firstSide` played by `first`. Returns the winner.
Player playGame(AIStrategy& first, AIStrategy& second, Player firstSide, int boardSize) {
    Game game(boardSize, firstSide);
    AIStrategy* toMoveAi = &first;
    while (game.status() == GameStatus::InProgress) {
        Player side = game.toMove();
        auto move = toMoveAi->selectMove(game.board(), side);
        REQUIRE(game.applyMove(move.first, move.second, side));
        toMoveAi = (toMoveAi == &first) ? &second : &first;
    }
    return game.winner();
}

} // namespace

TEST_CASE("MCTS beats a random opponent by a wide, non-flaky margin") {
    // Small board and modest iteration/game counts keep this fast in CI;
    // the standalone benchmark (engine/tools/benchmark.cpp) uses a larger
    // board and bigger budgets for the number quoted in the README.
    constexpr int kBoardSize = 5;
    constexpr int kGames = 24;
    MctsAI mcts(/*iterations=*/300, 1.4142135623730951, /*seed=*/2026);
    RandomAI random(/*seed=*/2027);

    int mctsWins = 0;
    for (int i = 0; i < kGames; ++i) {
        // Alternate which side MCTS plays and who moves first, so neither
        // strategy benefits from a first-move advantage over the whole run.
        Player mctsSide = (i % 2 == 0) ? Player::Blue : Player::Red;
        Player winner = (mctsSide == Player::Blue) ? playGame(mcts, random, Player::Blue, kBoardSize)
                                                    : playGame(random, mcts, Player::Blue, kBoardSize);
        if (winner == mctsSide) mctsWins++;
    }

    INFO("mctsWins = ", mctsWins, " / ", kGames);
    CHECK(mctsWins >= kGames * 3 / 4); // expect at least ~75% -- MCTS should crush random
}
