import { useEffect, useRef, useState } from 'react'
import { createGame, GameSocket, type Difficulty, type GameState, type Side } from './api'
import { HexBoard } from './components/HexBoard'

const DIFFICULTY_LABELS: Record<Difficulty, string> = {
  easy: 'Easy — Random',
  medium: 'Medium — Flat Monte Carlo',
  hard: 'Hard — MCTS (UCT)',
}

export default function App() {
  const [boardSize, setBoardSize] = useState(9)
  const [humanSide, setHumanSide] = useState<Side>('blue')
  const [difficulty, setDifficulty] = useState<Difficulty>('medium')
  const [state, setState] = useState<GameState | null>(null)
  const [thinking, setThinking] = useState(false)
  const [error, setError] = useState<string | null>(null)
  const socketRef = useRef<GameSocket | null>(null)

  useEffect(() => () => socketRef.current?.close(), [])

  async function startGame() {
    setError(null)
    socketRef.current?.close()
    try {
      const initial = await createGame(boardSize, humanSide, difficulty)
      setState(initial)
      setThinking(false)
      socketRef.current = new GameSocket(initial.game_id, (s) => {
        setState(s)
        setThinking(false)
        if (s.error) setError(s.error)
      })
    } catch {
      setError('Could not reach the backend. Is it running?')
    }
  }

  function handlePlay(row: number, col: number) {
    if (!state || !socketRef.current) return
    if (state.status !== 'in_progress' || state.to_move !== state.human_side) return
    setError(null)
    setThinking(true)
    socketRef.current.sendMove(row, col)
  }

  const humanTurn = !!state && state.status === 'in_progress' && state.to_move === state.human_side

  return (
    <div className="app">
      <header className="app-header">
        <h1>Game of Hex</h1>
        <p className="subtitle">
          Connect your two sides of the board before your opponent connects theirs. Board logic, win
          detection, and every AI opponent run in C++ behind a FastAPI/WebSocket backend.
        </p>
      </header>

      <section className="panel setup-panel">
        <div className="controls-row">
          <label>
            Board size
            <select value={boardSize} onChange={(e) => setBoardSize(Number(e.target.value))}>
              {[5, 7, 9, 11, 13].map((n) => (
                <option key={n} value={n}>
                  {n} × {n}
                </option>
              ))}
            </select>
          </label>
          <label>
            Play as
            <select value={humanSide} onChange={(e) => setHumanSide(e.target.value as Side)}>
              <option value="blue">Blue (left ↔ right)</option>
              <option value="red">Red (top ↔ bottom)</option>
            </select>
          </label>
          <label>
            Opponent
            <select value={difficulty} onChange={(e) => setDifficulty(e.target.value as Difficulty)}>
              {(Object.keys(DIFFICULTY_LABELS) as Difficulty[]).map((d) => (
                <option key={d} value={d}>
                  {DIFFICULTY_LABELS[d]}
                </option>
              ))}
            </select>
          </label>
          <button onClick={startGame}>New Game</button>
        </div>
        {error && <p className="error">{error}</p>}
      </section>

      {state && (
        <section className="panel board-panel">
          <div className="board-status">
            <span className={`side-tag blue${state.human_side === 'blue' ? ' you' : ''}`}>
              Blue {state.human_side === 'blue' ? '(you)' : `(${state.ai_name})`}
            </span>
            <span className={`side-tag red${state.human_side === 'red' ? ' you' : ''}`}>
              Red {state.human_side === 'red' ? '(you)' : `(${state.ai_name})`}
            </span>
            {state.status === 'in_progress' ? (
              <span className="turn-indicator">
                {thinking ? `${state.ai_name} is thinking…` : humanTurn ? 'Your move' : "Opponent's move"}
              </span>
            ) : (
              <span className="turn-indicator winner">
                {state.winner === state.human_side
                  ? 'You win!'
                  : `${state.winner === 'blue' ? 'Blue' : 'Red'} wins`}
              </span>
            )}
          </div>

          <HexBoard
            grid={state.grid}
            winningPath={state.winning_path}
            lastAiMove={state.last_ai_move}
            disabled={!humanTurn || thinking}
            onPlay={handlePlay}
          />
        </section>
      )}
    </div>
  )
}
