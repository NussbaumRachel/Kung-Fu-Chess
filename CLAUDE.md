# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Complete Click-to-Render Flow (Networked Mode)

### Directory of Key Files
| File | Role |
|---|---|
| `src/network/network_main_client.cpp` | Client entry: 2 threads (render+network), main loop, mouse→click, snapshot→render |
| `src/network/network_main_server.cpp` | Server entry: creates Board+Engine+Controller+GameServer |
| `src/network/GameServer.cpp` | Accepts connections, tick loop (16ms), routes clicks/messages, broadcasts snapshots |
| `src/network/ClientSession.cpp` | Per-connection WebSocket transport: async read/write queues |
| `src/network/JsonProtocol.cpp` | Pure serialization: `GameSnapshot` ↔ JSON (all static, stateless) |
| `src/controllerClick/GameController.cpp` | Gateway: `handleCellClick`, `handleWait`, `getSnapshot` — wraps `GameEngine` |
| `src/game_engine/GameEngine.cpp` | Core facade: click handling, decision execution, time advance, snapshot building |
| `src/game_engine/GameStateMachine.cpp` | Pure function: `ClickContext + GameState + selectedCell → GameDecision` |
| `src/game_engine/SnapshotBuilder.cpp` | Builds `GameSnapshot` from Board + Arbiter + StateMachine + Score + Moves |
| `demo/ChessRenderer.cpp` | Renders `GameSnapshot` to OpenCV canvas (board, pieces, panels, HUD) |
| `demo/PieceRenderer.cpp` | Computes pixel positions, draws sprite frames (handles Idle/Moving/Jumping) |
| `demo/BoardRenderer.cpp` | Draws board background + highlight overlays |
| `demo/MouseHandler.cpp` | OpenCV mouse callback → pixel-to-cell conversion → fires `ClickCallback` |
| `demo/img.cpp` | OpenCV HighGUI wrappers: `imshow`, `waitKey`, `setMouseCallback`, `draw_on` |

### Full Flow: Click → Server → Render

**Phase 1 — Mouse click (client, render thread):**
1. User clicks OpenCV window during `cv::waitKey(16)` in main loop
2. OpenCV fires `MouseHandler::onMouse` (static C callback) on the **render thread**
3. Subtracts `PANEL_WIDTH_PX` from x, calls `GameController::pixelsToCell` → `Position{row,col}`
4. Fires callback: builds `{"type":"click","row":R,"col":C}` JSON
5. Pushes JSON to `SharedState::outgoingMessages` (under `outgoingMtx`)
6. `wait_key` finishes (timeout or after event processing), main loop iterates

**Phase 2 — Network send (client, network thread):**
7. `doDrain` timer fires every 5ms on Asio thread, checks `outgoingMessages`
8. Takes one message, wraps in `shared_ptr<string>`, calls `ws->async_write()`
9. **CRITICAL**: `shared_ptr` is captured in completion lambda, keeping buffer alive for async operation
10. Write completes → timer rescheduled for next drain check

**Phase 3 — Server receive + process (server, Asio thread):**
11. `ClientSession::read()` async_read completes, extracts message
12. `GameServer::onMessage(session, message)` — parses JSON, checks `type == "click"`
13. `controller_.handleCellClick(row, col)` → `GameEngine::handleCellClick()`
14. `ClickPreparationService::prepare()` gathers context (isEmpty, isInvolved, hasFriendly, moveIsValid)
15. `GameStateMachine::evaluate(ctx, currentState, selectedCell)` → returns `GameDecision`
16. `executeDecision(d)` — SelectPiece/SwitchPiece/StartMove/StartJump/CancelSelection/NoOp
17. For StartMove: `arbiter_.startMove(piece, from, to, moveTime)`, piece state → `Moving`

**Phase 4 — Server game loop tick (server, Asio thread, every 16ms):**
18. `controller_.handleWait(16)` → `engine_.advanceTime(16)`
19. `arbiter_.advanceTime(16, board_)` — updates move/jump progress, detects completions
20. Completed moves → `MoveCompletionService::completeMove()` → captures, scoring, game-over check
21. Completed jumps → piece state → `Short_rest`, cooldown started
22. `cooldownService_.advanceRestTimers(16)` — advances rest timers
23. `maybeReturnToSelection()` — if no active moves/jumps, transitions to `WAITING_SELECTION`
24. `controller_.getSnapshot()` → `SnapshotBuilder::build()` → `GameSnapshot`
25. `JsonProtocol::serializeSnapshot(snap)` → JSON
26. For each connected session: `session->send(json)` → `ClientSession::writeNext()` → `async_write`

**Phase 5 — Client receive + render (client, render thread):**
27. Network thread `doRead` completion: pushes JSON to `SharedState::incomingMessages` (under `mtx`)
28. Main loop: locks `mtx`, drains queue — **keeps only LATEST snapshot** (drops intermediate ones)
29. Releases lock immediately — all JSON parsing + rendering happens OUTSIDE the lock
30. Parses snapshot JSON → `GameSnapshot` (pieces, scores, selectedCell, move history)
31. Animation detection: compares with `lastSnapshot`, resets `AnimationManager` for new moves/jumps
32. `renderer.render(snap)`:
    - Fills canvas grey → creates `boardCanvas` (800×800×4ch)
    - `BoardRenderer::draw()` — blits background, draws highlight overlay
    - `PieceRenderer::drawPieces()` — computes pixel positions per piece state/progress, draws sprite frames
    - Blits `boardCanvas` onto main canvas at offset `PANEL_WIDTH_PX`
    - `drawMoveHistoryPanel()` — left (Black) and right (White) scrollable lists
    - `drawScoreHUD()` — semi-transparent top bar
33. `renderer.display()` → `cv::imshow(windowName_, canvas_)` — queues window update
34. `cv::waitKey(16)` — pumps event loop, actually renders to screen, processes new mouse events → back to Phase 1

### Thread Architecture
```
┌─ Render Thread (main) ─────────────────────┐  ┌─ Network Thread (std::thread) ────────────┐
│  while(running):                           │  │  ioCtx.run():                              │
│    drain queue → keep latest snapshot      │  │    doRead → async_read → push incoming     │
│    parse → render → display                │  │    doDrain → timer(5ms) → async_write      │
│    wait_key(16) ← mouse callback fires!    │  │    (Boost.Asio single-threaded event loop) │
└────────────────────────────────────────────┘  └────────────────────────────────────────────┘
         ↕ outgoingMtx                                  ↕ mtx
         └── outgoingMessages ──→                       └── incomingMessages ←──
```

### CRITICAL: Deadlock/Freeze Prevention

**Bug #1 — async_write buffer lifetime (FIXED):**
The `doDrain` lambda used a stack-allocated `std::string msg` with `boost::asio::buffer(msg)` which creates a non-owning pointer. When the lambda returned, `msg` was destroyed and `async_write` read freed memory. **Fix**: Use `shared_ptr<string>` captured in the completion lambda (same pattern as `ClientSession::writeNext()`).

**Bug #2 — Snapshot queue overflow causing window freeze (FIXED):**
The server sends snapshots at 16ms intervals (~62/sec). The client was processing ALL queued snapshots while holding `shared.mtx`, including JSON parsing and full rendering for each one. This caused:
- Render thread: mutex held for seconds processing backlog → `wait_key` rarely called → window unresponsive, mouse clicks never registered
- Network thread: blocked trying to push new messages (mutex contention) → `async_read` stalled → TCP backpressure

**Fix**: The main loop now drains the queue quickly (cheap string search for `"snapshot"`), keeps only the **last** snapshot, drops the rest, and releases the lock immediately. JSON parsing and rendering happen outside the lock. Each iteration takes ~constant time regardless of queue depth.

### Server Tick Loop
```
GameServer::tick() [every 16ms, on Asio thread]:
  1. controller_.handleWait(16)     → advances game time, completes moves/jumps
  2. controller_.getSnapshot()      → builds GameSnapshot from current state
  3. JsonProtocol::serializeSnapshot() → JSON
  4. for each session: session->send(json) → async_write (queued)
  5. Reschedule tick timer
```

### Server Message Routing
```
GameServer::onMessage(session, message):
  parse JSON → check "type":
    "click" → controller_.handleCellClick(row, col)

GameServer::onSessionReady(session):
  assign color (White/Black/Spectator) → send welcome → send initial snapshot

GameServer::onSessionClosed(session):
  erase from sessions_ set
```

## Build Commands

```bash
# Configure and build (from project root)
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build .

# Build specific targets
cmake --build . --target kungfuchess_demo     # OpenCV GUI demo
cmake --build . --target kungfuchess_server   # WebSocket game server
cmake --build . --target kungfuchess_client   # WebSocket game client
cmake --build . --target chess_tests          # GoogleTest suite

# Run tests
ctest
# or directly:
./Debug/chess_tests

# Run (from build/Debug, OpenCV DLL must be in PATH or copied to output dir)
export PATH="/c/Kung-Fu-Chess/OpenCV_451/bin:$PATH"  # or: cp OpenCV_451/bin/opencv_world451d.dll build/Debug/
```

### Targets

| Target | Source | Description |
|---|---|---|
| `kungfuchess` | `src/text_io/main.cpp` | Text-based test runner |
| `kungfuchess_demo` | `demo/main.cpp` | Standalone OpenCV GUI (single-process) |
| `kungfuchess_server` | `src/network/network_main_server.cpp` | WebSocket game server (port 8080) |
| `kungfuchess_client` | `src/network/network_main_client.cpp` | WebSocket game client with OpenCV GUI |
| `chess_tests` | `tests/*.cpp` | GoogleTest suite |

OpenCV 4.5.1 must be at `C:\Kung-Fu-Chess\OpenCV_451\` — the demo and client need `opencv_world451d.dll`. Copy it to `build/Debug/` or add to PATH.

## Architecture

The project follows a layered architecture with strict single-responsibility:

### Flow (standalone demo)
```
click → GameController (adapter: pixels→cells)
      → GameEngine (facade)
          → ClickPreparationService → GameStateMachine (pure function: ClickContext→Decision)
          → executeDecision → arbiter.startMove / arbiter.startJump
          → advanceTime(16ms) → RealTimeArbiter → MoveCompletionService → BoardController
      → GameSnapshot (immutable DTO) → ChessRenderer (demo/)
```

### Flow (networked)
```
Client (OpenCV window + WebSocket)
  → {"type":"click","row":R,"col":C}  →
                                          ClientSession (WebSocket transport)
                                            → GameServer (orchestration)
                                              → GameController (gateway)
                                                → GameEngine
                                              ← GameSnapshot
                                            ← JsonProtocol::serializeSnapshot()
                                          ← {"type":"snapshot","data":{...}}
  ← JSON snapshot → ChessRenderer
```

**Network layer** (`src/network/`):
- `GameServer` — accepts connections (Boost.Beast), owns game loop timer (16ms ticks), routes clicks to `GameController`, broadcasts snapshots to all connected sessions. **Must access game only through `GameController` — never `GameEngine` directly.**
- `ClientSession` — **transport only**: WebSocket handshake, async read/write, message queue. No game logic, no serialization. Write queue serializes `async_write` calls (only one in-flight, enforced by `writing_` flag).
- `JsonProtocol` — pure serialization: `GameSnapshot` ↔ JSON. Stateless, all static methods.
- `network_main_server.cpp` — creates Board+PieceSpeedConfig+GameEngine+GameController, passes controller to `GameServer`
- `network_main_client.cpp` — two threads: render (main) + network (std::thread). Connects via Boost.Beast WebSocket, renders with OpenCV ChessRenderer, sends click messages via outgoing queue. **Snapshot throttling**: keeps only the latest snapshot per frame to prevent backlog.

**CRITICAL — Client outgoing drain**:
- `doDrain` uses `shared_ptr<string>` for the async_write buffer — same pattern as `ClientSession::writeNext()`. A stack-allocated `std::string` + `boost::asio::buffer()` is use-after-free.
- `doRead` and `doDrain` run concurrently on the same `ioContext` thread (Boost.Asio serializes them). This is safe — Boost.Beast supports concurrent async_read + async_write on the same stream.

**CRITICAL — Client main loop (snapshot throttling)**:
- Queue drain holds `shared.mtx` ONLY for the minimal time needed to scan messages (cheap string search for `"snapshot"`). All JSON parsing, snapshot reconstruction, and rendering happen outside the lock.
- Only the **latest** snapshot is kept per iteration; intermediate snapshots are dropped. This prevents unbounded queue growth when processing is slower than the server's 16ms tick rate.
- `wait_key(16)` is called every iteration, ensuring OpenCV event loop is pumped and mouse clicks are registered.

**CRITICAL — ClientSession write queue**:
- `writeQueue_` uses `std::queue<std::shared_ptr<std::string>>` — NOT `std::string`. `boost::asio::buffer(*msgPtr)` points to the string's internal data; the `shared_ptr` captured in the lambda keeps it alive for the duration of `async_write`.
- `send()` and `writeNext()` must NEVER call each other while holding `writeMutex_` — this causes deadlock. `send()` releases the lock before calling `writeNext()`; the completion handler also releases before recursing.
- Only one `async_write` is in flight at a time (enforced by `writing_` flag).

**CRITICAL — Forward declarations**: `GameSnapshot` is a `struct` (not `class`). Forward-declare it as `struct GameSnapshot;`. Using `class GameSnapshot;` causes MSVC linker errors because MSVC mangles `class` vs `struct` differently.

## Key layers

**Logic Core** (`src/`):
- `GameEngine` — the central facade; owns everything, orchestrates the game loop
- `GameStateMachine` — **pure decision function**: `evaluate(ClickContext, GameState, selectedCell) → GameDecision`. Stateless in logic; state (`selectedCell`, `winner`, current `GameState`) is mutated by `GameEngine`
- `ClickPreparationService` — gathers context for each click (is empty? has friendly piece? legal move?) into a `ClickContext` struct
- `RealTimeArbiter` — real-time movement coordinator: tracks active moves/jumps, resolves collisions (3 phases: target collisions → jump interceptions → path collisions)
- `MoveCompletionService` — executes completed moves on the board, checks win conditions, handles promotion
- `BoardController` — raw board manipulations
- `Move` — path-based movement: each move computes a `path_` vector of Position, interpolates progress by time

**Gateway** (`src/controllerClick/`):
- `GameController` — **the only entry point for external systems to the game**. Wraps `GameEngine` with methods: `handleCellClick()`, `handleJump()`, `handleWait()` (→ advanceTime), `getSnapshot()`, `getBoard()`. Network layer must go through this — never `GameEngine` directly.

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

## Dependencies

| Library | Usage | How |
|---|---|---|
| Boost (system) | WebSocket server (Beast) | `find_package(Boost CONFIG REQUIRED COMPONENTS system)` |
| nlohmann_json | JSON serialization in network layer | `FetchContent` from GitHub v3.11.3 |
| OpenCV 4.5.1 | GUI rendering (demo + client) | Manual path at `OpenCV_451/` in project root |
| GoogleTest 1.17.0 | Unit tests | Bundled in `external/googletest-1.17.0/` |

## Known issues / current state

- `PieceState` enum uses inconsistent casing: `long_rest`, `Short_rest` — should be unified
- ~18 tests fail with SEH exceptions (0xc0000005 access violation) — these are pre-existing runtime bugs, not related to scoring/move-history/ui changes
- The `build.bat` at the root is a legacy script that only compiles a handful of files with MSVC directly — prefer CMake
- `AnimatedSprite::pieceStateToFolder()` maps by ordinal position — adding/reordering `PieceState` values will break folder resolution
- Client requires OpenCV DLL in PATH or copied to the binary directory
- MSVC mangles `class` vs `struct` differently in forward declarations — always match the actual definition (`GameSnapshot` is `struct`)
- Boost.Beast `async_write` buffers must outlive the async operation — use `shared_ptr<string>` captured in the completion lambda (see `ClientSession::send()` and `network_main_client.cpp` `doDrain` for correct pattern; a plain stack `std::string` + `boost::asio::buffer()` is use-after-free)
- Network client main loop uses snapshot throttling: only the latest snapshot is processed per frame; intermediate snapshots are dropped to prevent queue backlog and window freeze. Do NOT revert to the "process all messages" pattern — it causes the render thread to starve `wait_key` and the window becomes unresponsive.
- The `draw_on` method in `img.cpp` was previously broken in its 4-channel alpha blending path (used `.col(c)` instead of `cv::split()`). The fix uses `cv::split`/`cv::merge` with per-channel float blending. Do NOT revert to the column-based approach.
