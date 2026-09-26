#pragma once

#include "hex/board.hpp"

namespace hex {

enum class GameStatus { InProgress, Finished };

// A thin state machine around Board. The original hexupdated.cpp's Game
// class read from `cin` and wrote to `cout` directly inside playerTurn()
// and aiTurn() -- console I/O and game logic were the same functions, which
// is exactly what stood between it and a web frontend. This version knows
// nothing about consoles, HTTP, or AI strategies: it only tracks whose turn
// it is and whether the game is over, so a CLI, a test, and the FastAPI
// backend can all drive the same logic identically.
class Game {
public:
    Game(int boardSize, Player firstToMove);

    Board& board() { return board_; }
    const Board& board() const { return board_; }
    Player toMove() const { return toMove_; }
    GameStatus status() const { return status_; }
    Player winner() const { return winner_; }

    // Places `player`'s stone if it is their turn and the cell is free,
    // updates status/winner, and advances the turn. Returns false (no state
    // change) on an illegal move.
    bool applyMove(int row, int col, Player player);

private:
    Board board_;
    Player toMove_;
    GameStatus status_ = GameStatus::InProgress;
    Player winner_ = Player::None;
};

} // namespace hex
