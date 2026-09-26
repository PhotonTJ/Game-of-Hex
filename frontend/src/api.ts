export type Side = 'red' | 'blue'
export type Difficulty = 'easy' | 'medium' | 'hard'

export type GameState = {
  game_id: string
  board_size: number
  grid: number[][] // 0 empty, 1 red, 2 blue
  to_move: Side
  status: 'in_progress' | 'finished'
  winner: Side | null
  winning_path: number[][]
  human_side: Side
  ai_side: Side
  ai_name: string
  last_ai_move: number[] | null
  error?: string | null
}

export async function createGame(
  boardSize: number,
  humanSide: Side,
  difficulty: Difficulty,
): Promise<GameState> {
  const res = await fetch('/api/games', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ board_size: boardSize, human_side: humanSide, difficulty }),
  })
  if (!res.ok) throw new Error(`create game failed: ${res.status}`)
  return res.json() as Promise<GameState>
}

function wsUrl(path: string): string {
  const proto = window.location.protocol === 'https:' ? 'wss' : 'ws'
  return `${proto}://${window.location.host}${path}`
}

export class GameSocket {
  private ws: WebSocket

  constructor(gameId: string, onState: (s: GameState) => void, onOpen?: () => void) {
    this.ws = new WebSocket(wsUrl(`/ws/games/${gameId}`))
    this.ws.onmessage = (ev) => onState(JSON.parse(ev.data) as GameState)
    if (onOpen) this.ws.onopen = onOpen
  }

  sendMove(row: number, col: number) {
    this.ws.send(JSON.stringify({ row, col }))
  }

  close() {
    this.ws.close()
  }
}
