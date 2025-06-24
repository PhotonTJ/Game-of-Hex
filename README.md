# Hex Game in C++

Welcome to the Hex Game repository!  
This project features a modern C++ implementation of the classic two-player strategy game Hex, where players compete to form an unbroken path across a hexagonal board.

## Features

- **Dynamic Board Configuration**: Customizable grid sizes (e.g., 7x7, 11x11).
- **Multiple Game Modes**: Player vs Player and Player vs AI.
- **User-Friendly Console Interface**: Clear text-based interface with visual tile feedback.
- **Cross-Platform Compatibility**: Runs on Windows, macOS, and Linux.
- **Smart AI Opponent**: Uses simple heuristics and graph algorithms (expandable for research).

## How to Play

- **Objective**: Each player must connect their assigned sides of the board with a continuous chain of tiles.
- **Turns**: Players alternate placing tiles on empty hexes.
- **Winning**: A player wins by completing an uninterrupted connection across their assigned direction. The game detects and announces victory automatically.

## Educational Value

This project is ideal for:
- Learning modern C++ with real applications.
- Studying algorithms like DFS, union-find, and pathfinding.
- Implementing game logic and AI opponents.
- Exploring how console-based games are designed and structured.

## Build & Run

```bash
# Clone the repository
git clone https://github.com/YourUsername/HexGameCPP.git
cd HexGameCPP

# Compile
g++ -std=c++17 -o hexgame main.cpp board.cpp game.cpp ai.cpp

# Run
./hexgame
