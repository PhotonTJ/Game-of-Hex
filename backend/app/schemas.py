from typing import List, Literal, Optional

from pydantic import BaseModel, Field

Difficulty = Literal["easy", "medium", "hard"]
Side = Literal["red", "blue"]


class CreateGameRequest(BaseModel):
    board_size: int = Field(default=9, ge=3, le=15)
    human_side: Side = "blue"
    difficulty: Difficulty = "medium"


class GameState(BaseModel):
    game_id: str
    board_size: int
    grid: List[List[int]]  # 0 = empty, 1 = red, 2 = blue
    to_move: Side
    status: Literal["in_progress", "finished"]
    winner: Optional[Side] = None
    winning_path: List[List[int]] = []
    human_side: Side
    ai_side: Side
    ai_name: str
    last_ai_move: Optional[List[int]] = None
    error: Optional[str] = None
