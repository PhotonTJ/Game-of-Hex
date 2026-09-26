#include "hex/board.hpp"

#include <algorithm>
#include <queue>

namespace hex {

namespace {
// The six axial-hex neighbor offsets, unchanged from the original
// hexupdated.cpp's `node` table.
constexpr int kNeighborOffsets[6][2] = {{-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}};
} // namespace

Player opponent(Player p) {
    if (p == Player::Red) return Player::Blue;
    if (p == Player::Blue) return Player::Red;
    return Player::None;
}

Board::Board(int size) : size_(size), cells_(size, std::vector<Player>(size, Player::None)) {}

bool Board::inBounds(int row, int col) const noexcept {
    return row >= 0 && row < size_ && col >= 0 && col < size_;
}

Player Board::at(int row, int col) const { return cells_[row][col]; }

bool Board::place(int row, int col, Player player) {
    if (!inBounds(row, col) || cells_[row][col] != Player::None) return false;
    cells_[row][col] = player;
    return true;
}

bool Board::remove(int row, int col) {
    if (!inBounds(row, col) || cells_[row][col] == Player::None) return false;
    cells_[row][col] = Player::None;
    return true;
}

std::vector<std::pair<int, int>> Board::emptyCells() const {
    std::vector<std::pair<int, int>> cells;
    for (int r = 0; r < size_; ++r) {
        for (int c = 0; c < size_; ++c) {
            if (cells_[r][c] == Player::None) cells.emplace_back(r, c);
        }
    }
    return cells;
}

bool Board::isFull() const {
    for (const auto& row : cells_) {
        for (Player p : row) {
            if (p == Player::None) return false;
        }
    }
    return true;
}

Player Board::winner() const {
    // Start the flood fill only from stones already on each player's start
    // edge -- Blue's column 0, Red's row 0 -- so reaching the far edge
    // (column size-1 for Blue, row size-1 for Red) proves an unbroken chain
    // all the way across, not just two disconnected clusters that each
    // happen to touch an edge.
    std::vector<std::pair<int, int>> blueStart, redStart;
    for (int i = 0; i < size_; ++i) {
        if (cells_[i][0] == Player::Blue) blueStart.emplace_back(i, 0);
        if (cells_[0][i] == Player::Red) redStart.emplace_back(0, i);
    }

    auto reachesFarEdge = [&](std::vector<std::pair<int, int>> start, Player side) {
        if (start.empty()) return false;
        std::vector<std::vector<bool>> visited(size_, std::vector<bool>(size_, false));
        std::queue<std::pair<int, int>> q;
        for (auto& s : start) {
            q.push(s);
            visited[s.first][s.second] = true;
        }
        while (!q.empty()) {
            auto [r, c] = q.front();
            q.pop();
            if (side == Player::Blue && c == size_ - 1) return true;
            if (side == Player::Red && r == size_ - 1) return true;
            for (auto& off : kNeighborOffsets) {
                int nr = r + off[0], nc = c + off[1];
                if (inBounds(nr, nc) && !visited[nr][nc] && cells_[nr][nc] == side) {
                    visited[nr][nc] = true;
                    q.emplace(nr, nc);
                }
            }
        }
        return false;
    };

    if (reachesFarEdge(blueStart, Player::Blue)) return Player::Blue;
    if (reachesFarEdge(redStart, Player::Red)) return Player::Red;
    return Player::None;
}

std::vector<std::pair<int, int>> Board::winningPath(Player player) const {
    std::vector<std::pair<int, int>> path;
    if (player != Player::Red && player != Player::Blue) return path;

    std::vector<std::pair<int, int>> start;
    for (int i = 0; i < size_; ++i) {
        if (player == Player::Blue && cells_[i][0] == Player::Blue) start.emplace_back(i, 0);
        if (player == Player::Red && cells_[0][i] == Player::Red) start.emplace_back(0, i);
    }
    if (start.empty()) return path;

    std::vector<std::vector<bool>> visited(size_, std::vector<bool>(size_, false));
    std::vector<std::vector<std::pair<int, int>>> parent(
        size_, std::vector<std::pair<int, int>>(size_, {-1, -1}));
    std::queue<std::pair<int, int>> q;
    for (auto& s : start) {
        q.push(s);
        visited[s.first][s.second] = true;
    }

    std::pair<int, int> goal{-1, -1};
    while (!q.empty()) {
        auto [r, c] = q.front();
        q.pop();
        bool atFarEdge = (player == Player::Blue) ? (c == size_ - 1) : (r == size_ - 1);
        if (atFarEdge) {
            goal = {r, c};
            break;
        }
        for (auto& off : kNeighborOffsets) {
            int nr = r + off[0], nc = c + off[1];
            if (inBounds(nr, nc) && !visited[nr][nc] && cells_[nr][nc] == player) {
                visited[nr][nc] = true;
                parent[nr][nc] = {r, c};
                q.emplace(nr, nc);
            }
        }
    }

    if (goal.first == -1) return path; // not actually connected yet

    for (auto cur = goal; cur.first != -1; cur = parent[cur.first][cur.second]) {
        path.push_back(cur);
    }
    std::reverse(path.begin(), path.end());
    return path;
}

std::vector<std::vector<int>> Board::grid() const {
    std::vector<std::vector<int>> g(size_, std::vector<int>(size_, 0));
    for (int r = 0; r < size_; ++r) {
        for (int c = 0; c < size_; ++c) {
            g[r][c] = cells_[r][c] == Player::Red ? 1 : cells_[r][c] == Player::Blue ? 2 : 0;
        }
    }
    return g;
}

} // namespace hex
