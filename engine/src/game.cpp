#include "hex/game.hpp"

namespace hex {

Game::Game(int boardSize, Player firstToMove) : board_(boardSize), toMove_(firstToMove) {}

bool Game::applyMove(int row, int col, Player player) {
    if (status_ != GameStatus::InProgress) return false;
    if (player != toMove_) return false;
    if (!board_.place(row, col, player)) return false;

    Player w = board_.winner();
    if (w != Player::None) {
        status_ = GameStatus::Finished;
        winner_ = w;
    } else if (board_.isFull()) {
        // Unreachable in real Hex (the board can't fill without a winner),
        // but guarded rather than assumed.
        status_ = GameStatus::Finished;
    } else {
        toMove_ = opponent(toMove_);
    }
    return true;
}

} // namespace hex
