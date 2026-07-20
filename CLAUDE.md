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
- `GameSnapshot` — complete immutable game state for the renderer

**Shared config** (`src/config/`):
- `PieceConfigReader` — **stateless JSON reader** (all static methods): `readDouble()`, `readString()`, `readBool()`. Both `AnimatedSprite` (UI) and `PieceSpeedConfig` (logic) use it instead of hand-rolling JSON parsing
- `PieceSpeedConfig` — loads `speed_m_per_sec` from all 12 piece configs, provides `getMoveSpeed(type, color)` / `getJumpSpeed(type, color)`

**Assets** (`demo/assets/pieces2/`):
- 12 piece dirs (`KW`, `KB`, `QW`, `QB`, …), each with 5 states: `idle`, `move`, `jump`, `short_rest`, `long_rest`
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
- `GameEngine.hpp` was updated to accept `PieceSpeedConfig&` but `GameEngine.cpp` still uses the old constructor signature with hardcoded `JUMP_DURATION_MS` constant — the `.cpp` needs to be brought in sync with the header
- `PieceSpeedConfig` and `PieceConfigReader` source files are not listed in `CMakeLists.txt`'s `ALL_SOURCES` — they need to be added for compilation
- `AnimatedSprite::parseConfig()` was refactored to use `PieceConfigReader` but the `#include` only exists in `.cpp` not `.hpp`
- Tests construct `GameEngine(Board)` — need a default `PieceSpeedConfig` or updated calls
- The `build.bat` at the root is a legacy script that only compiles a handful of files with MSVC directly — prefer CMake
