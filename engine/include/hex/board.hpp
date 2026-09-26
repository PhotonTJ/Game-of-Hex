#pragma once

#include <cstdint>
#include <utility>
#include <vector>

namespace hex {

enum class Player : std::uint8_t { Red, Blue, None };

Player opponent(Player p);

// An n x n Hex board. Blue connects left-to-right (column 0 to column
// size-1); Red connects top-to-bottom (row 0 to row size-1) -- the same
// convention the original hexupdated.cpp used. Win detection is an
// edge-to-edge BFS flood fill over the six hex-grid neighbor offsets,
// exactly as in the original.
//
// This class adds what a console-only program never needed: cloning (cheap
// value semantics, so an AI can copy the board for a rollout), a plain
// integer grid for JSON, and a reconstructed winning path for the frontend
// to highlight. The original's `Game` mixed console I/O directly into move
// handling; none of that lives here -- Board only knows about the grid.
class Board {
public:
    explicit Board(int size);

    int size() const noexcept { return size_; }
    Player at(int row, int col) const;
    bool inBounds(int row, int col) const noexcept;

    bool place(int row, int col, Player player);
    bool remove(int row, int col);

    std::vector<std::pair<int, int>> emptyCells() const;
    bool isFull() const;

    // None if neither side has connected their two edges yet.
    Player winner() const;

    // The connected chain of `player`'s stones from their start edge to
    // their end edge. Empty if `player` has not won.
    std::vector<std::pair<int, int>> winningPath(Player player) const;

    // 0 = empty, 1 = red, 2 = blue -- a plain grid for serializing to JSON.
    std::vector<std::vector<int>> grid() const;

private:
    int size_;
    std::vector<std::vector<Player>> cells_;
};

} // namespace hex
