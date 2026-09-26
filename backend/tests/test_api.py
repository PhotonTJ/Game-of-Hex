from fastapi.testclient import TestClient

from app.main import app

client = TestClient(app)


def test_health():
    resp = client.get("/api/health")
    assert resp.status_code == 200
    assert resp.json() == {"status": "ok"}


def test_create_game_human_moves_first_by_default():
    resp = client.post("/api/games", json={"board_size": 7, "human_side": "blue", "difficulty": "easy"})
    assert resp.status_code == 200
    body = resp.json()
    assert body["board_size"] == 7
    assert body["to_move"] == "blue"  # human is blue, blue always moves first
    assert body["human_side"] == "blue"
    assert body["ai_side"] == "red"
    assert body["last_ai_move"] is None
    assert body["status"] == "in_progress"
    assert len(body["grid"]) == 7 and len(body["grid"][0]) == 7


def test_create_game_ai_moves_first_when_human_is_red():
    resp = client.post("/api/games", json={"board_size": 7, "human_side": "red", "difficulty": "easy"})
    assert resp.status_code == 200
    body = resp.json()
    assert body["human_side"] == "red"
    assert body["ai_side"] == "blue"
    # Blue (the AI here) always moves first, so it should have already played.
    assert body["last_ai_move"] is not None
    assert body["to_move"] == "red"


def test_get_unknown_game_returns_404():
    resp = client.get("/api/games/does-not-exist")
    assert resp.status_code == 404


def _first_empty_cell(grid):
    for r, row in enumerate(grid):
        for c, val in enumerate(row):
            if val == 0:
                return r, c
    return None


def test_websocket_full_playthrough_on_tiny_board():
    create = client.post("/api/games", json={"board_size": 3, "human_side": "blue", "difficulty": "easy"})
    game_id = create.json()["game_id"]

    with client.websocket_connect(f"/ws/games/{game_id}") as ws:
        state = ws.receive_json()  # initial state pushed on connect
        assert state["status"] == "in_progress"

        for _ in range(9):  # a 3x3 board has at most 9 total moves
            if state["status"] == "finished":
                break
            move = _first_empty_cell(state["grid"])
            assert move is not None, "board full but game not finished -- Hex should never draw"
            ws.send_json({"row": move[0], "col": move[1]})
            state = ws.receive_json()  # after human move
            if state["status"] == "finished":
                break
            state = ws.receive_json()  # after AI's reply move

        assert state["status"] == "finished"
        assert state["winner"] in {"red", "blue"}
        assert len(state["winning_path"]) > 0


def test_websocket_rejects_illegal_move():
    create = client.post("/api/games", json={"board_size": 5, "human_side": "blue", "difficulty": "easy"})
    game_id = create.json()["game_id"]

    with client.websocket_connect(f"/ws/games/{game_id}") as ws:
        ws.receive_json()  # initial state
        ws.send_json({"row": 0, "col": 0})
        ws.receive_json()  # human move accepted
        ws.receive_json()  # AI reply
        ws.send_json({"row": 0, "col": 0})  # occupied now
        state = ws.receive_json()
        assert state["error"] == "illegal move"
