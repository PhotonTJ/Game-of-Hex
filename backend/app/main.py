"""FastAPI backend for Game-of-Hex.

Thin wrapper around the compiled `hex_engine` pybind11 module (built from
`engine/`, see engine/build.sh). All game rules, win detection, and AI
strategies live in C++; this file only tracks per-game sessions and moves
JSON in and out over REST + a WebSocket.
"""
import asyncio
import uuid
from typing import Dict, Optional, Tuple

from fastapi import FastAPI, HTTPException, WebSocket, WebSocketDisconnect
from fastapi.middleware.cors import CORSMiddleware

from . import hex_engine as hx
from .schemas import CreateGameRequest, GameState

app = FastAPI(
    title="Game-of-Hex API",
    description=(
        "Hex board game with Random / Flat Monte Carlo / MCTS-UCT AI "
        "opponents, all implemented in C++ and exposed via pybind11."
    ),
    version="1.0.0",
)

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["*"],
    allow_headers=["*"],
)

_SIDE_TO_PLAYER = {"red": hx.Player.RED, "blue": hx.Player.BLUE}
_PLAYER_TO_SIDE = {hx.Player.RED: "red", hx.Player.BLUE: "blue"}


def _make_ai(difficulty: str) -> hx.AIStrategy:
    if difficulty == "easy":
        return hx.RandomAI()
    if difficulty == "hard":
        return hx.MctsAI(iterations=4000)
    return hx.FlatMonteCarloAI(trials=150)


class GameSession:
    """In-memory session: this is a portfolio-scale demo (single process,
    no persistence), not a production matchmaking service."""

    def __init__(self, board_size: int, human_side: str, difficulty: str) -> None:
        self.human_side = _SIDE_TO_PLAYER[human_side]
        self.ai_side = hx.opponent(self.human_side)
        # Blue always moves first, matching the original console version.
        self.game = hx.Game(board_size=board_size, first_to_move=hx.Player.BLUE)
        self.ai = _make_ai(difficulty)
        self.last_ai_move: Optional[Tuple[int, int]] = None


_sessions: Dict[str, GameSession] = {}


def _play_ai_move(session: GameSession) -> None:
    move = session.ai.select_move(session.game.board, session.ai_side)
    session.game.apply_move(move[0], move[1], session.ai_side)
    session.last_ai_move = move


def _state(game_id: str, session: GameSession, error: Optional[str] = None) -> GameState:
    game = session.game
    winner_player = game.winner
    winner_side = _PLAYER_TO_SIDE.get(winner_player)
    winning_path = game.board.winning_path(winner_player) if winner_side is not None else []
    return GameState(
        game_id=game_id,
        board_size=game.board.size,
        grid=game.board.grid(),
        to_move=_PLAYER_TO_SIDE[game.to_move],
        status="finished" if game.status == hx.GameStatus.FINISHED else "in_progress",
        winner=winner_side,
        winning_path=[list(cell) for cell in winning_path],
        human_side=_PLAYER_TO_SIDE[session.human_side],
        ai_side=_PLAYER_TO_SIDE[session.ai_side],
        ai_name=session.ai.name,
        last_ai_move=list(session.last_ai_move) if session.last_ai_move else None,
        error=error,
    )


@app.get("/api/health")
def health() -> dict:
    return {"status": "ok"}


@app.post("/api/games", response_model=GameState)
async def create_game(req: CreateGameRequest) -> GameState:
    game_id = uuid.uuid4().hex[:12]
    session = GameSession(req.board_size, req.human_side, req.difficulty)
    _sessions[game_id] = session

    # If the human chose Red, Blue (the AI) moves first.
    if session.game.to_move == session.ai_side:
        await asyncio.to_thread(_play_ai_move, session)

    return _state(game_id, session)


@app.get("/api/games/{game_id}", response_model=GameState)
def get_game(game_id: str) -> GameState:
    session = _sessions.get(game_id)
    if session is None:
        raise HTTPException(status_code=404, detail="game not found")
    return _state(game_id, session)


@app.websocket("/ws/games/{game_id}")
async def play(websocket: WebSocket, game_id: str) -> None:
    await websocket.accept()
    session = _sessions.get(game_id)
    if session is None:
        await websocket.send_json({"error": "game not found"})
        await websocket.close()
        return

    await websocket.send_json(_state(game_id, session).model_dump())

    try:
        while True:
            msg = await websocket.receive_json()
            row, col = int(msg["row"]), int(msg["col"])

            if session.game.status != hx.GameStatus.IN_PROGRESS:
                await websocket.send_json(_state(game_id, session, error="game already finished").model_dump())
                continue
            if session.game.to_move != session.human_side:
                await websocket.send_json(_state(game_id, session, error="not your turn").model_dump())
                continue

            if not session.game.apply_move(row, col, session.human_side):
                await websocket.send_json(_state(game_id, session, error="illegal move").model_dump())
                continue

            await websocket.send_json(_state(game_id, session).model_dump())

            if session.game.status == hx.GameStatus.IN_PROGRESS:
                # MCTS can take real CPU time; run it off the event loop so
                # other games' connections stay responsive (and the pybind11
                # binding releases the GIL for the duration too).
                await asyncio.to_thread(_play_ai_move, session)
                await websocket.send_json(_state(game_id, session).model_dump())
    except WebSocketDisconnect:
        pass
