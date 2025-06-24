Hex Game in C++
Welcome to the Hex Game repository!
This project showcases a full-featured implementation of the classic two-player Hex game, written in modern C++. Hex is a connection-based board game in which two players compete to form an unbroken chain of their pieces from one side of the board to the opposite side on a hexagonal grid. This implementation offers a customizable, interactive, and strategy-rich gameplay experience, suitable for both casual play and AI experimentation.

🎮 Features
Dynamic Board Configuration
Easily set the board size to your preference (e.g., 7x7, 11x11, 13x13), offering flexible gameplay and scalability for different difficulty levels.

Multiple Game Modes

Player vs Player: Classic face-off between two human players.

Player vs AI: Challenge a smart computer opponent that uses pathfinding and heuristic strategies to block and counter human moves.

User-Friendly Console Interface
Designed for clarity and simplicity, the interface guides the user through game setup, move selection, and victory detection with colored output and clear instructions.

Cross-Platform Compatibility
Built with standard C++ and no platform-dependent libraries, the game runs seamlessly on Windows, macOS, and Linux.

Smart AI Opponent (Optional)
The AI leverages basic search strategies and evaluation functions (e.g., depth-limited DFS, Monte Carlo simulations) to provide a competitive opponent. Great for educational purposes and expanding into more advanced AI techniques.

🕹️ How to Play
Objective
Each player is assigned a pair of opposite sides of the board. The goal is to form an unbroken path of your colored tiles connecting your two sides.

Turns
Players alternate turns, placing one tile per move onto an unoccupied hex.

Winning Condition
A player wins by creating a continuous chain of connected tiles from one side to the other. The game automatically checks for victory after each turn.

🧠 Educational Value
This project is perfect for:

Learning Game Development in C++: Understand core concepts like data structures, input handling, and real-time state updates.

Practicing Algorithms: Graph traversal (DFS/BFS), union-find, and shortest path logic.

Exploring AI in Board Games: Build and extend AI opponents with Minimax, Monte Carlo Tree Search, or Reinforcement Learning.

Systems Programming: Offers insight into terminal-based UI, memory management, and cross-platform development.

📦 Build & Run Instructions
bash
Copy
Edit
# Clone the repository
git clone https://github.com/YourUsername/HexGameCPP.git
cd HexGameCPP

# Compile (for example, using g++)
g++ -std=c++17 -o hexgame main.cpp board.cpp game.cpp ai.cpp

# Run the game
./hexgame
Note: The actual build command and files may vary depending on your structure.
