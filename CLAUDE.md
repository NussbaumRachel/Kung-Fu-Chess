# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
# Configure and build (from project root)
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build .

# Build the demo (OpenCV-based GUI)
cmake --build . --target kungfuchess_demo

# Build and run tests
cmake --build . --target chess_tests
ctest
# or directly:
./chess_tests
```

The project has two targets:
- `kungfuchess` — text-based test runner (`src/text_io/main.cpp`)
- `kungfuchess_demo` — OpenCV GUI (`demo/main.cpp`), requires OpenCV_451 in project root
- `chess_tests` — GoogleTest test suite

## Architecture

The project follows a layered architecture with strict single-responsibility:

### Flow
```
click → GameController (adapter: pixels→cells)
      → GameEngine (facade)
          → ClickPreparationService → GameStateMachine (pure function: ClickContext→Decision)
          → executeDecision → arbiter.startMove / arbiter.startJump
          → advanceTime(16ms) → RealTimeArbiter → MoveCompletionService → BoardController
      → GameSnapshot (immutable DTO) → ChessRenderer (demo/)
```

### Key layers

**Logic Core** (`src/`):
- `GameEngine` — the central facade; owns everything, orchestrates the game loop
- `GameStateMachine` — **pure decision function**: `evaluate(ClickContext, GameState, selectedCell) → GameDecision`. Stateless in logic; state (`selectedCell`, `winner`, current `GameState`) is mutated by `GameEngine`
- `ClickPreparationService` — gathers context for each click (is empty? has friendly piece? legal move?) into a `ClickContext` struct
- `RealTimeArbiter` — real-time movement coordinator: tracks active moves/jumps, resolves collisions (3 phases: target collisions → jump interceptions → path collisions)
- `MoveCompletionService` — executes completed moves on the board, checks win conditions, handles promotion
- `BoardController` — raw board manipulations
- `Move` — path-based movement: each move computes a `path_` vector of Position, interpolates progress by time

**DTOs** (`src/game_engine/GameSnapshot.hpp`):
- `PieceInfo` — lightweight view of a piece: kind, color, pieceId, cell, state, progress (0–1), targetCell
- `MoveRecord` — captures a completed move/jump entry: timestamp (minutes/seconds/milliseconds), pieceType, color, from, to, isJump, isCapture, givesCheck
- `GameSnapshot` — complete immutable game state for the renderer: board dimensions, pieces, selectedCell, gameOver, winner, whiteScore, blackScore, whiteMoves, blackMoves

**Shared config** (`src/config/`):
- `PieceConfigReader` — **stateless JSON reader** (all static methods): `readDouble()`, `readString()`, `readBool()`. Both `AnimatedSprite` (UI) and `PieceSpeedConfig` (logic) use it instead of hand-rolling JSON parsing
- `PieceSpeedConfig` — loads `speed_m_per_sec` from all 12 piece configs, provides `getMoveSpeed(type, color)` / `getJumpSpeed(type, color)`

**Points system** (`src/model/Constants.hpp`):
- `pieceValue(PieceType)` — `constexpr` free function: Pawn=1, Knight/Bishop=3, Rook=5, Queen=9, King=0
- Score aggregation happens in `GameEngine::advanceTime()` via `MoveCompletionResult::pointsAwarded` and `scoringColor`
- Scores are exposed through `GameSnapshot::whiteScore`/`blackScore`

**Move history**:
- `GameEngine` accumulates `gameTimeMs_` and records each completed move/jump into `whiteMoves_`/`blackMoves_` vectors of `MoveRecord`
- `recordMove()` called for normal moves, `recordJump()` for jumps — both populate timestamp from `gameTimeMs_`
- Exposed via `GameSnapshot::whiteMoves`/`blackMoves`

**Assets** (`demo/assets/`):
- `board.png` — 822×828 RGBA PNG, loaded with `cv::IMREAD_UNCHANGED`
- `pieces/` — 12 piece dirs (`KW`, `KB`, `QW`, `QB`, …), each with 5 states: `idle`, `move`, `jump`, `short_rest`, `long_rest`
- Each state has `config.json` + `sprites/1.png…5.png`
- JSON schema: `{"physics": {"speed_m_per_sec": 1.5, "next_state_when_finished": "long_rest"}, "graphics": {"frames_per_sec": 12, "is_loop": true}}`

### Key enums
- `PieceState`: `Idle`, `Moving`, `Jumping`, `long_rest`, `Short_rest`, `Captured` — note inconsistent casing
- `GameState`: `WAITING_SELECTION`, `WAITING_TARGET`, `MOVE_IN_PROGRESS`, `JUMP_IN_PROGRESS`, `GAME_OVER`
- `ActionType`: `SelectPiece`, `SwitchPiece`, `StartMove`, `StartJump`, `CancelSelection`, `NoOp`

### Piece types and colors
- `PieceType`: `King`, `Queen`, `Rook`, `Bishop`, `Knight`, `Pawn`
- `Color`: `White`, `Black`
- 2-letter key: first letter of piece type + `W`/`B` (e.g., `"KW"` = King White, `"RB"` = Rook Black)
- `Piece::isValidMove(fromRow, fromCol, toRow, toCol, board)` — virtual, overridden per piece type in `src/model/`

## UI Layer (demo/)

### Canvas layout
- `DemoConfig::CANVAS_WIDTH_PX = BOARD_WIDTH_PX + PANEL_WIDTH_PX * 2` (800 + 150*2 = 1100px)
- `BOARD_HEIGHT_PX = 800`
- Left panel (x=0..149): Black move history
- Center (x=150..949): chessboard
- Right panel (x=950..1099): White move history
- Top bar: score HUD (semi-transparent dark bar, 32px)
- Canvas is **4-channel BGRA** — required for proper alpha blending of sprites

### Mouse coordinates
- OpenCV sends window-relative pixel coordinates (x, y)
- `MouseHandler::onMouse` subtracts `DemoConfig::PANEL_WIDTH_PX` from x before passing to `GameController::pixelsToCell`
- `GameController::pixelsToCell` is a pure function: `Position{pixelY/cellSize, pixelX/cellSize}` — no UI knowledge
- Window uses `cv::WINDOW_AUTOSIZE` for 1:1 pixel mapping

### Rendering order (per frame)
1. Clear canvas to grey background
2. Create temp `boardCanvas` (800×800, 4ch)
3. `BoardRenderer::draw()` — ensures boardCanvas is 4-channel, blits background via `draw_on`
4. `PieceRenderer::drawPieces()` — draws pieces onto boardCanvas via `draw_on` (alpha blending)
5. `boardCanvas.draw_on(canvas_, PANEL_WIDTH_PX, 0)` — blit to main canvas at offset
6. `drawMoveHistoryPanel()` — left/right panels with scrollable move lists
7. `drawScoreHUD()` — top bar with scores
8. `drawGameOverlay()` — if game over, semi-transparent overlay

### Critical: `Img::draw_on` (img.cpp)
The `draw_on` method was **broken** in its 4-channel alpha blending path. It used `.col(c)` (returns column c of the 2D matrix) instead of `cv::split()` (returns actual color channels). This was fixed by:
- Using `cv::split(roi, roiChannels)` to separate the ROI into per-channel Mats
- Blending each channel with alpha: `roiChannels[c] = srcChannels[c].mul(alpha) + roiChannels[c].mul(1.0 - alpha)`
- Using `cv::merge(roiChannels, roi)` to write back
- Also fixed `cvtColor` to output to a separate Mat instead of in-place on a shallow copy (was mutating the original image)

### Critical: `BoardRenderer::draw`
- Must ensure the canvas is always 4-channel before accessing pixels as `cv::Vec4b` in `drawCellHighlight`
- Creates a fresh 4-channel canvas if dimensions don't match, then uses `draw_on` to blit the background (rather than `clone()` which could produce 3-channel)

## Working with Assets

Asset folder names use lowercase (e.g., `idle`, `move`, `jump`, `long_rest`). The `AnimatedSprite::pieceStateToFolder()` function maps `PieceState` enum values by ordinal position to folder names:
- `case 0` → `"idle"`, `case 1` → `"move"`, `case 2` → `"jump"`, `case 3` → `"long_rest"`, `default` → `"idle"`
This means `.cpp` code references `PieceState::long_rest` but the folder is `long_rest`, and the `Short_rest` (case 4) falls to `"idle"`.

When constructing paths to config JSONs, use the pattern:
`<piecesPath>/<2-letter-key>/states/<stateFolderName>/config.json`

## Testing

Tests use GoogleTest (`external/googletest-1.17.0`). Test files:
- `tests/GameTests.cpp` — basic movement, rests, multi-piece scenarios
- `tests/GameOverTests.cpp` — king capture, game-over state
- `tests/PieceMovementTests.cpp` — per-piece legal move validation

Tests call `engine.advanceTime(ms)` to simulate time passing. A rook moving 2 cells takes `2 * 1000 = 2000ms` to arrive.

## Known issues / current state

- `PieceState` enum uses inconsistent casing: `long_rest`, `Short_rest` — should be unified
- ~18 tests fail with SEH exceptions (0xc0000005 access violation) — these are pre-existing runtime bugs, not related to scoring/move-history/ui changes
- The `build.bat` at the root is a legacy script that only compiles a handful of files with MSVC directly — prefer CMake
- `AnimatedSprite::pieceStateToFolder()` maps by ordinal position — adding/reordering `PieceState` values will break folder resolution
