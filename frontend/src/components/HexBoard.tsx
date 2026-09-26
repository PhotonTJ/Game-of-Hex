import { useMemo } from 'react'

const HEX_RADIUS = 28

function hexCenter(row: number, col: number, r: number): { x: number; y: number } {
  // Pointy-top hexagons, rows shifted right by half a hex width so they
  // interlock into the classic Hex-board rhombus (Blue's edges are the
  // left/right columns, Red's edges are the top/bottom rows).
  return {
    x: Math.sqrt(3) * r * (col + row / 2),
    y: 1.5 * r * row,
  }
}

function hexCorners(cx: number, cy: number, r: number): string {
  const points: string[] = []
  for (let i = 0; i < 6; i++) {
    const angle = (Math.PI / 180) * (60 * i - 30)
    points.push(`${cx + r * Math.cos(angle)},${cy + r * Math.sin(angle)}`)
  }
  return points.join(' ')
}

type Props = {
  grid: number[][]
  winningPath: number[][]
  lastAiMove: number[] | null
  disabled: boolean
  onPlay: (row: number, col: number) => void
}

export function HexBoard({ grid, winningPath, lastAiMove, disabled, onPlay }: Props) {
  const size = grid.length
  const r = HEX_RADIUS

  const cells = useMemo(() => {
    const out: { row: number; col: number; cx: number; cy: number }[] = []
    for (let row = 0; row < size; row++) {
      for (let col = 0; col < size; col++) {
        const { x, y } = hexCenter(row, col, r)
        out.push({ row, col, cx: x, cy: y })
      }
    }
    return out
  }, [size, r])

  if (cells.length === 0) return null

  const minX = Math.min(...cells.map((c) => c.cx)) - r * 1.3
  const maxX = Math.max(...cells.map((c) => c.cx)) + r * 1.3
  const minY = Math.min(...cells.map((c) => c.cy)) - r * 1.3
  const maxY = Math.max(...cells.map((c) => c.cy)) + r * 1.3

  const winningSet = new Set(winningPath.map(([row, col]) => `${row},${col}`))
  const lastAiKey = lastAiMove ? `${lastAiMove[0]},${lastAiMove[1]}` : null

  return (
    <svg
      viewBox={`${minX} ${minY} ${maxX - minX} ${maxY - minY}`}
      className="hex-board"
      role="grid"
      aria-label={`${size} by ${size} Hex board`}
    >
      {cells.map(({ row, col, cx, cy }) => {
        const value = grid[row][col]
        const key = `${row},${col}`
        const isWinning = winningSet.has(key)
        const isLastAi = key === lastAiKey
        const fill = value === 1 ? 'var(--p-red)' : value === 2 ? 'var(--p-blue)' : 'var(--cell-empty)'
        const playable = value === 0 && !disabled
        return (
          <g
            key={key}
            onClick={() => playable && onPlay(row, col)}
            style={{ cursor: playable ? 'pointer' : 'default' }}
          >
            <polygon
              points={hexCorners(cx, cy, r - 2)}
              fill={fill}
              stroke={isWinning ? 'var(--text-primary)' : 'var(--cell-border)'}
              strokeWidth={isWinning ? 3 : 1}
            />
            {isLastAi && <circle cx={cx} cy={cy} r={r * 0.22} fill="var(--surface)" opacity={0.85} />}
          </g>
        )
      })}
    </svg>
  )
}
