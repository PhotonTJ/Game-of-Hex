#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <queue>
#include <string>
#include <utility>

using namespace std;

enum class Player {
    Red,
    Blue
};

class Board {
private:
    static char red;
    static char blue;
    static char null;
    static int node[6][2];
    int size;
    string line;
    vector<vector<char>> b;

    bool inBoard(int x, int y);
    void borders(int x, int y, vector<bool>& condition, char side);
    void bfsSearch(vector<pair<int, int>>& start, vector<bool>& condition);

public:
   Board(){}
    Board(int size) : size(size), b(size, vector<char>(size, '+')) {
        line = "\\";
        for (int i = 1; i < size; i++) {
            line += "/ \\";
        }
    }

    bool place(int x, int y, Player p);
    bool badMove(int x, int y);
    vector<pair<int, int>> getEmpty();
    bool win(int x, int y);
    Player winner();
    void printBoard();
};

class AI {
public:
    static double wins(Board& b, Player p);
    pair<int, int> next(Board& b, Player p);
};

class Game {
private:
    Player ai;
    Player player;
    AI bot;
    Board b;

public:
    Game(AI& bot){this->bot=bot;}

    void play();

private:
    void setup();
    void choose();
    bool playerTurn();
    bool aiTurn();
};

int Board::node[6][2] = {
    {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}
};

char Board::red = 'R';
char Board::blue = 'B';
char Board::null = '+';

bool Board::inBoard(int x, int y) {
    return (x < size && y < size && x >= 0 && y >= 0);
}

void Board::borders(int x, int y, vector<bool>& condition, char side) {
    if (side == blue) {
        if (y == 0)
            condition[0] = true;
        if (y == size - 1)
            condition[1] = true;
    } else {
        if (x == 0)
            condition[0] = true;
        if (x == size - 1)
            condition[1] = true;
    }
}

bool Board::place(int x, int y, Player p) {
    if (inBoard(x, y) && b[x][y] == null) {
        if (p == Player::Blue)
            b[x][y] = blue;
        else
            b[x][y] = red;
        return true;
    }
    return false;
}

bool Board::badMove(int x, int y) {
    if (inBoard(x, y)) {
        b[x][y] = null;
        return true;
    }
    return false;
}

vector<pair<int, int>> Board::getEmpty() {
    vector<pair<int, int>> blankSpots;
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++)
            if (b[i][j] == null)
                blankSpots.push_back(make_pair(i, j));
    }
    return blankSpots;
}

void Board::bfsSearch(vector<pair<int, int>>& start, vector<bool>& condition) {
    if (start.size() != 0) {
        int x = start[0].first;
        int y = start[0].second;
        char side = b[x][y];

        vector<vector<bool>> visited(size, vector<bool>(size));
        queue<pair<int, int>> trace;

        for (auto itr = start.cbegin(); itr != start.cend(); ++itr) {
            trace.push(*itr);
            visited[itr->first][itr->second] = true;
        }
        while (!(trace.empty())) {
            auto top = trace.front();
            borders(top.first, top.second, condition, side);
            trace.pop();

            for (int i = 0; i < 6; i++) {
                int xCursor = top.first + node[i][0];
                int yCursor = top.second + node[i][1];
                if (inBoard(xCursor, yCursor) && b[xCursor][yCursor] == side && visited[xCursor][yCursor] == false) {
                    visited[xCursor][yCursor] = true;
                    trace.push(make_pair(xCursor, yCursor));
                }
            }
        }
    }
}

bool Board::win(int x, int y) {
    if (inBoard(x, y) && b[x][y] != null) {
        vector<bool> condition(2, false);
        vector<pair<int, int>> start(1, make_pair(x, y));

        bfsSearch(start, condition);
        return condition[0] && condition[1];
    }
    return false;
}

Player Board::winner() {
    vector<bool> condition(2, false);
    vector<pair<int, int>> start;
    for (int i = 0; i < size; i++)
        if (b[i][0] == blue)
            start.push_back(make_pair(i, 0));

    bfsSearch(start, condition);
    return (condition[0] && condition[1]) ? Player::Blue : Player::Red;
}

void Board::printBoard() {
    if (size <= 0)
        return;

    cout << "  0";
    for (int i = 1; i < size; i++)
        cout << " w " << i;
    cout << endl;

    cout << "0 " << b[0][0];
    for (int i = 1; i < size; i++)
        cout << "---" << b[0][i];
    cout << endl;

    string indent = "";
    for (int i = 1; i < size; i++) {
        indent += ' ';
        cout << indent << "b " << line << endl;
        if (i < 10) {
            indent += ' ';
            cout << indent << i << ' ' << b[i][0];
        } else {
            cout << indent << i << ' ' << b[i][0];
            indent += ' ';
        }

        for (int j = 1; j < size; j++)
            cout << "---" << b[i][j];
        cout << endl;
    }
    cout << "_________________________________________________________" << endl;
}

double AI::wins(Board& b, Player player) {
    auto blank = b.getEmpty();
    int winCount = 0;
    vector<int> perm(blank.size());
    for (int i = 0; i < perm.size(); i++)
        perm[i] = i;
    for (int n = 0; n < 1000; n++) {
        int turn = (player == Player::Blue ? 0 : 1);
        for (int i = perm.size(); i > 1; i--) {
            int swap = rand() % i;
            int temp = perm[i - 1];
            perm[i - 1] = perm[swap];
            perm[swap] = temp;
        }
        for (int i = 0; i < perm.size(); i++) {
            turn = !turn;
            int x = blank[perm[i]].first;
            int y = blank[perm[i]].second;
            if (turn) {
                b.place(x, y, Player::Red);
            } else {
                b.place(x, y, Player::Blue);
            }
        }
        if (b.winner() == player)
            winCount++;

        for (auto itr = blank.begin(); itr != blank.end(); ++itr)
            b.badMove(itr->first, itr->second);
    }
    return static_cast<double>(winCount) / 1000;
}

pair<int, int> AI::next(Board& board, Player p) {
    auto blank = board.getEmpty();
    double bestMove = 0;
    pair<int, int> move = blank[0];

    for (int i = 0; i < blank.size(); i++) {
        int x = blank[i].first;
        int y = blank[i].second;
        board.place(x, y, p);

        double moveValue = wins(board, p);
        if (moveValue > bestMove) {
            move = blank[i];
            bestMove = moveValue;
        }

        board.badMove(x, y);
    }
    return move;
}

void Game::play() {
    cout << "Welcome to Hex!" << endl;
    setup();
    choose();

    while (true) {
        if (!playerTurn())
            break;

        if (b.win(0, 0)) {
            cout << "Congratulations! You win!" << endl;
            break;
        }

        if (!aiTurn())
            break;

        if (b.win(0, 0)) {
            cout << "Sorry, you lost. AI wins!" << endl;
            break;
        }
    }
    cout << "Game over!" << endl;
}

void Game::setup() {
    int dimensions;
    cout << "Enter board size (recommended 5 - 11): ";
    cin >> dimensions;

    if (dimensions > 0) {
        cin.clear();
        b = Board(dimensions);
    } else {
        cout << "Invalid size, defaulting to 9." << endl;
        b = Board(9);
    }
}

void Game::choose() {
    char side;
    cout << "Choose your side (R/B): ";
    cin >> side;

    if (side == 'R' || side == 'r') {
        player = Player::Red;
        ai = Player::Blue;
    } else {
        player = Player::Blue;
        ai = Player::Red;
    }
}

bool Game::playerTurn() {
    int x, y;
    cout << "Your turn (enter coordinates x y): ";
    cin >> x >> y;

    if (b.place(x, y, player)) {
        b.printBoard();
        return true;
    } else {
        cout << "Invalid move. Try again." << endl;
        return false;
    }
}

bool Game::aiTurn() {
    auto move = bot.next(b, ai);
    cout << "AI's turn: " << move.first << " " << move.second << endl;
    b.place(move.first, move.second, ai);
    b.printBoard();
    return true;
}

int main() {
    srand(static_cast<unsigned int>(time(0)));
    AI gai;
    Game hexgame(gai);
    hexgame.play();
    return 0;
}