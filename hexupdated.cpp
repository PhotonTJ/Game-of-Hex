#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <queue>
#include <string>
#include <utility>
#include <chrono>
#include <random>

using namespace std;

enum class Player {
    Red,
    Blue,
    None
};

class Board {
private:
    static const char red;
    static const char blue;
    static const char empty;
    static int node[6][2];
    int size;
    vector<vector<char>> board;

    bool inBoard(int x, int y) const;
    void bfsSearch(vector<pair<int, int>>& start, vector<bool>& condition, char side);

public:
    Board(int size);
    bool place(int x, int y, Player p);
    bool remove(int x, int y);
    vector<pair<int, int>> getEmpty() const;
    bool isWinningMove(int x, int y, Player player);
    Player getWinner();
    void printBoard() const;
    Player getCell(int x, int y) const;
};

class AI {
public:
    pair<int, int> nextMove(Board& board, Player player, int maxTrials = 1000, int maxTimeSeconds = 120);
private:
    double evaluateMove(Board& board, Player player, int x, int y, int trials);
    double simulate(Board& board, Player player, int trials);
};

class Game {
private:
    Player human;
    Player ai;
    AI bot;
    Board board;
    void setup();
    void chooseSide();
    bool playerTurn();
    bool aiTurn();
    void announceWinner(Player winner);
public:
    Game(AI& bot);
    void play();
};

const char Board::red = 'R';
const char Board::blue = 'B';
const char Board::empty = '+';

int Board::node[6][2] = {
    {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}
};

Board::Board(int size) : size(size), board(size, vector<char>(size, empty)) {}

bool Board::inBoard(int x, int y) const {
    return x >= 0 && x < size && y >= 0 && y < size;
}

bool Board::place(int x, int y, Player p) {
    if (inBoard(x, y) && board[x][y] == empty) {
        board[x][y] = (p == Player::Blue) ? blue : red;
        return true;
    }
    return false;
}

bool Board::remove(int x, int y) {
    if (inBoard(x, y) && board[x][y] != empty) {
        board[x][y] = empty;
        return true;
    }
    return false;
}

vector<pair<int, int>> Board::getEmpty() const {
    vector<pair<int, int>> emptyCells;
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            if (board[i][j] == empty) {
                emptyCells.emplace_back(i, j);
            }
        }
    }
    return emptyCells;
}

bool Board::isWinningMove(int x, int y, Player player) {
    char side = (player == Player::Blue) ? blue : red;
    vector<pair<int, int>> start{ {x, y} };
    vector<bool> condition(2, false);
    bfsSearch(start, condition, side);
    return condition[0] && condition[1];
}

Player Board::getWinner() {
    vector<pair<int, int>> startBlue, startRed;
    for (int i = 0; i < size; ++i) {
        if (board[i][0] == blue) {
            startBlue.emplace_back(i, 0);
        }
        if (board[0][i] == red) {
            startRed.emplace_back(0, i);
        }
    }
    vector<bool> blueCondition(2, false);
    bfsSearch(startBlue, blueCondition, blue);
    if (blueCondition[0] && blueCondition[1]) {
        return Player::Blue;
    }
    vector<bool> redCondition(2, false);
    bfsSearch(startRed, redCondition, red);
    if (redCondition[0] && redCondition[1]) {
        return Player::Red;
    }
    return Player::None;
}

void Board::bfsSearch(vector<pair<int, int>>& start, vector<bool>& condition, char side) {
    if (start.empty()) return;
    queue<pair<int, int>> q;
    vector<vector<bool>> visited(size, vector<bool>(size, false));

    for (const auto& s : start) {
        q.push(s);
        visited[s.first][s.second] = true;
    }

    while (!q.empty()) {
        auto [x, y] = q.front();
        q.pop();
        if (side == blue) {
            if (y == 0) condition[0] = true;
            if (y == size - 1) condition[1] = true;
        } else {
            if (x == 0) condition[0] = true;
            if (x == size - 1) condition[1] = true;
        }
        for (const auto& [dx, dy] : node) {
            int nx = x + dx, ny = y + dy;
            if (inBoard(nx, ny) && board[nx][ny] == side && !visited[nx][ny]) {
                visited[nx][ny] = true;
                q.emplace(nx, ny);
            }
        }
    }
}

void Board::printBoard() const {
    cout << "  ";
    for (int i = 0; i < size; ++i) {
        cout << i << ' ';
    }
    cout << '\n';
    for (int i = 0; i < size; ++i) {
        cout << i << ' ';
        for (int j = 0; j < size; ++j) {
            cout << board[i][j] << ' ';
        }
        cout << '\n';
    }
}

Player Board::getCell(int x, int y) const {
    if (board[x][y] == blue) return Player::Blue;
    if (board[x][y] == red) return Player::Red;
    return Player::None;
}

pair<int, int> AI::nextMove(Board& board, Player player, int maxTrials, int maxTimeSeconds) {
    auto emptyCells = board.getEmpty();
    if (emptyCells.empty()) return { -1, -1 };

    random_device rd;
    mt19937 rng(rd());
    uniform_int_distribution<int> dist(0, emptyCells.size() - 1);

    double bestWinRate = 0.0;
    pair<int, int> bestMove = emptyCells[0];

    auto start = chrono::steady_clock::now();
    for (const auto& [x, y] : emptyCells) {
        double winRate = evaluateMove(board, player, x, y, maxTrials);
        if (winRate > bestWinRate) {
            bestWinRate = winRate;
            bestMove = { x, y };
        }

        auto now = chrono::steady_clock::now();
        auto duration = chrono::duration_cast<chrono::seconds>(now - start).count();
        if (duration > maxTimeSeconds) {
            break;
        }
    }

    return bestMove;
}

double AI::evaluateMove(Board& board, Player player, int x, int y, int trials) {
    board.place(x, y, player);
    double winRate = simulate(board, player, trials);
    board.remove(x, y);
    return winRate;
}

double AI::simulate(Board& board, Player player, int trials) {
    auto emptyCells = board.getEmpty();
    int winCount = 0;

    random_device rd;
    mt19937 rng(rd());
    uniform_int_distribution<int> dist(0, emptyCells.size() - 1);

    for (int i = 0; i < trials; ++i) {
        shuffle(emptyCells.begin(), emptyCells.end(), rng);
        int turn = (player == Player::Blue) ? 0 : 1;

        for (const auto& [x, y] : emptyCells) {
            board.place(x, y, turn == 0 ? Player::Blue : Player::Red);
            turn = 1 - turn;
        }

        if (board.getWinner() == player) {
            winCount++;
        }

        for (const auto& [x, y] : emptyCells) {
            board.remove(x, y);
        }
    }

    return static_cast<double>(winCount) / trials;
}

Game::Game(AI& bot) : board(11), bot(bot), human(Player::None), ai(Player::None) {}

void Game::setup() {
    cout << "Enter board size (default is 11): ";
    int size;
    cin >> size;
    if (size <= 0) size = 11;
    board = Board(size);
}

void Game::chooseSide() {
    cout << "Choose your side (B for Blue, R for Red): ";
    char side;
    cin >> side;
    if (side == 'B' || side == 'b') {
        human = Player::Blue;
        ai = Player::Red;
    } else {
        human = Player::Red;
        ai = Player::Blue;
    }
}

bool Game::playerTurn() {
    int x, y;
    cout << "Enter your move (x y): ";
    cin >> x >> y;

    while (!board.place(x, y, human)) {
        cout << "Invalid move. Enter again (x y): ";
        cin >> x >> y;
    }

    board.printBoard();
    return !board.isWinningMove(x, y, human);
}

bool Game::aiTurn() {
    auto [x, y] = bot.nextMove(board, ai);
    if (x == -1 && y == -1) {
        cout << "AI has no moves left.\n";
        return false;
    }

    board.place(x, y, ai);
    board.printBoard();
    return !board.isWinningMove(x, y, ai);
}

void Game::announceWinner(Player winner) {
    if (winner == Player::Blue) {
        cout << "Blue wins!\n";
    } else if (winner == Player::Red) {
        cout << "Red wins!\n";
    } else {
        cout << "It's a draw!\n";
    }
}

void Game::play() {
    setup();
    chooseSide();
    board.printBoard();

    while (true) {
        if (!playerTurn()) {
            announceWinner(human);
            break;
        }

        if (!aiTurn()) {
            announceWinner(ai);
            break;
        }
    }

    cout << "Game over!\n";
}

int main() {
    AI aiBot;
    Game game(aiBot);
    game.play();
    return 0;
}
