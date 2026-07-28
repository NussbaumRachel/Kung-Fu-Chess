# KUNG_FU_CHESS

A real-time multiplayer **Kung Fu Chess** application written in **C++17**.

The project combines:

- An authoritative multiplayer server
- Real-time game simulation
- Multiple isolated game rooms
- A graphical OpenCV client
- Asynchronous WebSocket communication
- Clean Architecture and Single Responsibility principles
- Docker-based server deployment

---

## Table of Contents

- [Overview](#overview)
- [Core Principles](#core-principles)
- [High-Level Architecture](#high-level-architecture)
- [Game Logic Layers](#game-logic-layers)
- [Networking Layer](#networking-layer)
- [Graphics Layer](#graphics-layer)
- [Configuration Layer](#configuration-layer)
- [Text and Test Utilities](#text-and-test-utilities)
- [Communication Flow](#communication-flow)
- [Room Architecture](#room-architecture)
- [Supported Protocol Messages](#supported-protocol-messages)
- [Technologies](#technologies)
- [Project Structure](#project-structure)
- [Local Build](#local-build)
- [Docker](#docker)
- [Testing](#testing)
- [Roadmap](#roadmap)

---

## Overview

KUNG_FU_CHESS is a real-time chess system in which game actions are handled by an authoritative server.

Unlike a traditional turn-based chess system, the game engine supports real-time state updates through an independent game loop.

Each active room contains its own:

- GameEngine
- GameController
- GameLoop
- White player
- Black player
- Spectators
- Local selection state
- Room-specific broadcast scope

Rooms are isolated from each other. Commands, snapshots, selections, players, and spectators belonging to one room do not affect another room.

---

## Core Principles

### Authoritative Server

The server owns the official game state.

The client:

- Sends user commands
- Receives snapshots
- Renders the latest state

The client does not decide:

- Whether a move is legal
- Whether a collision is valid
- Whether a game has ended
- What the authoritative board state is

### Clean Architecture

The game domain is separated from:

- Networking
- Serialization
- Rendering
- Infrastructure
- Transport concerns

### Single Responsibility

Each component has one primary responsibility.

Examples:

- `GameEngine` manages game state transitions.
- `GameController` is the entry point into game operations.
- `MessageRouter` validates and routes protocol messages.
- `ChessRenderer` renders the client-side game view.
- `JsonProtocol` performs JSON and DTO conversion.

### Room Isolation

Every Room is an independent game instance.

A Room owns its game-related components and broadcasts updates only to sessions that belong to that Room.

---

## High-Level Architecture

```text
Client
│
├── NetworkClient
├── ClientMessageProcessor
├── SharedState
└── ChessRenderer
        │
        │ WebSocket + JSON
        ▼
GameServer
│
├── SessionManager
│   └── ClientSession
│
├── MessageRouter
│
└── RoomManager
    └── Room
        ├── GameController
        ├── GameEngine
        └── GameLoop
```

---

# Game Logic Layers

The game logic is divided into several focused layers.

```text
Controller Layer
        │
        ▼
Game Engine Layer
        │
        ├── Rule Engine
        ├── Movement
        ├── Arbiter
        └── Model
```

---

## Model Layer

Location:

```text
src/model/
```

The model layer contains the core game entities and board representation.

Main components:

```text
Board
Piece
King
Queen
Rook
Bishop
Knight
Pawn
Position
Types
```

### Responsibilities

- Represent the chess board
- Represent chess pieces
- Store piece positions
- Store piece-specific state
- Provide domain-level board access
- Define shared game types

### Main Files

```text
src/model/Board.cpp
src/model/Piece.cpp
src/model/King.cpp
src/model/Queen.cpp
src/model/Rook.cpp
src/model/Bishop.cpp
src/model/Knight.cpp
src/model/Pawn.cpp
src/model/Position.hpp
src/model/types.hpp
```

The model layer does not manage:

- WebSocket communication
- JSON
- Rendering
- Room membership
- Client sessions

---

## Movement Layer

Location:

```text
src/movement/
```

Main components:

```text
Move
PieceFactory
```

### Responsibilities

- Represent movement operations
- Create piece instances through the piece factory
- Support the game engine's movement workflow
- Keep movement-related behavior separate from networking and rendering

### Main Files

```text
src/movement/Move.cpp
src/movement/PieceFactory.cpp
```

---

## Rule Engine Layer

Location:

```text
src/rule_engine/
```

Main component:

```text
RuleEngine
```

### Responsibilities

- Evaluate game rules
- Validate rule-related conditions
- Provide rule decisions to the game engine
- Keep rule evaluation outside the networking layer

### Main File

```text
src/rule_engine/RuleEngine.cpp
```

The `RuleEngine` is part of the authoritative server-side game logic.

---

## Arbiter Layer

Location:

```text
src/arbiter/
```

Main components:

```text
RealTimeArbiter
CollisionResolver
```

### Responsibilities

- Resolve real-time interactions
- Coordinate arbitration between simultaneous game events
- Resolve collisions
- Provide deterministic decisions to the game engine

### Main Files

```text
src/arbiter/RealTimeArbiter.cpp
src/arbiter/CollisionResolver.cpp
```

This layer separates real-time conflict resolution from:

- Rendering
- Transport
- Session management
- JSON parsing

---

## Game Engine Layer

Location:

```text
src/game_engine/
```

The game engine layer owns the authoritative game simulation.

Main components:

```text
GameEngine
GameStateMachine
BoardController
MoveCompletionService
ClickPreparationService
ScoreTracker
MoveRecorder
CooldownService
SnapshotBuilder
```

### GameEngine

`GameEngine` coordinates the main game state and simulation logic.

It is the authoritative source of game state for a Room.

### GameStateMachine

Responsible for managing transitions between game states.

### BoardController

Coordinates board-related operations used by the game engine.

### MoveCompletionService

Handles the completion phase of active moves.

### ClickPreparationService

Prepares click-related game operations before they are applied.

### ScoreTracker

Tracks game scoring state.

### MoveRecorder

Records game movements within the engine workflow.

### CooldownService

Manages cooldown-related game state.

### SnapshotBuilder

Builds a serializable representation of the current game state.

### Main Files

```text
src/game_engine/GameEngine.cpp
src/game_engine/GameStateMachine.cpp
src/game_engine/BoardController.cpp
src/game_engine/MoveCompletionService.cpp
src/game_engine/ClickPreparationService.cpp
src/game_engine/ScoreTracker.cpp
src/game_engine/MoveRecorder.cpp
src/game_engine/CooldownService.cpp
src/game_engine/SnapshotBuilder.cpp
```

---

## Controller Layer

Location:

```text
src/controllerClick/
```

Main component:

```text
GameController
```

### Responsibilities

- Act as the single entry point into game logic
- Receive prepared game commands
- Coordinate operations with the GameEngine
- Prevent networking code from accessing internal game services directly

### Main File

```text
src/controllerClick/GameController.cpp
```

The intended command path is:

```text
MessageRouter
    │
    ▼
Room
    │
    ▼
GameController
    │
    ▼
GameEngine
```

---

# Networking Layer

Location:

```text
src/network/
```

The networking layer manages WebSocket connections, sessions, protocol routing, rooms, and server-side broadcasting.

---

## GameServer

Main file:

```text
src/network/GameServer.cpp
```

### Responsibility

`GameServer` is the server orchestrator.

It connects the main server components but does not contain game rules or database logic.

---

## SessionManager

Main file:

```text
src/network/SessionManager.cpp
```

### Responsibilities

- Accept incoming connections
- Create ClientSession instances
- Manage active sessions
- Perform basic login handling
- Enforce unique usernames
- Remove disconnected sessions

---

## ClientSession

Main file:

```text
src/network/ClientSession.cpp
```

### Responsibilities

- Own one WebSocket connection
- Perform the WebSocket handshake
- Execute asynchronous reads
- Execute asynchronous writes
- Maintain an outgoing message queue
- Store username
- Store authentication state
- Store PlayerRole

Each connected client is represented by one `ClientSession`.

---

## MessageRouter

Main file:

```text
src/network/MessageRouter.cpp
```

### Responsibilities

- Parse incoming messages
- Validate protocol input
- Validate authentication
- Route commands to the correct component
- Keep routing logic outside ClientSession

A single `MessageRouter` instance is used at server level.

---

## RoomManager

Main file:

```text
src/network/RoomManager.cpp
```

### Responsibilities

- Create rooms
- Join rooms
- Leave rooms
- Delete rooms
- Track session-to-room membership
- Locate the correct Room for a session

---

## Room

Main file:

```text
src/network/Room.cpp
```

A Room represents one independent multiplayer game.

### Room Membership

Each Room supports:

- One White player
- One Black player
- An unlimited number of spectators

### Room Responsibilities

- Own the Room-specific GameEngine
- Own the Room-specific GameController
- Own the Room-specific GameLoop
- Maintain Room membership
- Maintain Room-local selection state
- Enforce Room-local game permissions
- Broadcast only to Room members

---

## GameLoop

Main file:

```text
src/network/GameLoop.cpp
```

### Responsibilities

- Execute periodic game updates
- Advance the Room's game simulation
- Build the current snapshot
- Publish the snapshot through the Room

The current loop interval is:

```text
16 milliseconds
```

The GameLoop is independent for every Room.

---

## JsonProtocol

Main file:

```text
src/network/JsonProtocol.cpp
```

### Responsibilities

- Convert JSON into protocol DTOs
- Convert protocol DTOs into JSON
- Centralize protocol serialization
- Prevent manual JSON construction throughout the codebase

All JSON conversion should remain inside `JsonProtocol`.

---

# Graphics Layer

Location:

```text
demo/
```

The graphics layer is responsible for presenting the latest client state.

It does not own the authoritative game state.

Main components:

```text
ChessRenderer
BoardRenderer
PieceRenderer
AnimatedSprite
AnimationManager
AssetManager
MouseHandler
DemoConfig
img
```

The graphics implementation uses OpenCV.

---

## ChessRenderer

Main file:

```text
demo/ChessRenderer.cpp
```

### Responsibilities

- Coordinate rendering of the complete chess scene
- Use the current client-side shared state
- Delegate board rendering
- Delegate piece rendering
- Coordinate graphical updates

---

## BoardRenderer

Main file:

```text
demo/BoardRenderer.cpp
```

### Responsibilities

- Render the chess board
- Draw board cells
- Present board-level visual state

---

## PieceRenderer

Main file:

```text
demo/PieceRenderer.cpp
```

### Responsibilities

- Render chess pieces
- Draw piece positions based on client state
- Use loaded graphical assets

---

## AnimatedSprite

Main file:

```text
demo/AnimatedSprite.cpp
```

### Responsibilities

- Represent an animated graphical object
- Manage sprite animation state
- Support frame-based rendering

---

## AnimationManager

Main file:

```text
demo/AnimationManager.cpp
```

### Responsibilities

- Coordinate active animations
- Update animation progress
- Provide animation data to the rendering layer

---

## AssetManager

Main file:

```text
demo/AssetManager.cpp
```

### Responsibilities

- Load graphical assets
- Store reusable asset resources
- Provide images to renderers
- Centralize asset access

The server Docker image currently copies assets from:

```text
demo/assets/
```

into:

```text
/app/assets/
```

---

## MouseHandler

Main file:

```text
demo/MouseHandler.cpp
```

### Responsibilities

- Process mouse input
- Convert graphical input into board-related actions
- Forward user actions toward the client command flow

The mouse handler does not determine whether a move is legal.

---

## Demo Configuration

Main file:

```text
demo/DemoConfig.hpp
```

### Responsibilities

- Store graphics-related configuration
- Provide rendering constants used by the demo layer

---

# Client-Side Architecture

The client separates transport, message processing, shared state, and rendering.

```text
NetworkClient
      │
      ▼
ClientMessageProcessor
      │
      ▼
SharedState
      │
      ▼
ChessRenderer
```

---

## NetworkClient

Main file:

```text
src/network/NetworkClient.cpp
```

### Responsibilities

- Connect to the WebSocket server
- Send messages
- Receive messages
- Handle transport operations

`NetworkClient` does not contain game rules.

---

## ClientMessageProcessor

Main file:

```text
src/network/ClientMessageProcessor.cpp
```

### Responsibilities

- Parse incoming server messages
- Process the latest snapshot
- Process `room_result`
- Update client-side state

---

## SharedState

`SharedState` stores state shared between client networking and rendering.

Its role is to allow:

- The network side to update received state
- The graphics side to render the latest available state

---

# Configuration Layer

Location:

```text
src/config/
```

Main components:

```text
PieceSpeedConfig
PieceConfigReader
```

### Responsibilities

- Represent piece speed configuration
- Read piece-related configuration
- Keep configuration parsing separate from game logic

### Main Files

```text
src/config/PieceSpeedConfig.cpp
src/config/PieceConfigReader.cpp
```

---

# Text and Test Utilities

Location:

```text
src/text_io/
```

Main components:

```text
BoardParser
TextTestRunner
```

### Responsibilities

- Parse textual board representations
- Support non-graphical test workflows
- Provide reusable text-based testing utilities

### Main Files

```text
src/text_io/BoardParser.cpp
src/text_io/TextTestRunner.cpp
src/text_io/main.cpp
```

---

# Communication Flow

## Client Command Flow

```text
MouseHandler
    │
    ▼
NetworkClient
    │
    ▼
WebSocket
    │
    ▼
ClientSession
    │
    ▼
MessageRouter
    │
    ▼
RoomManager
    │
    ▼
Room
    │
    ▼
GameController
    │
    ▼
GameEngine
```

## Snapshot Flow

```text
GameEngine
    │
    ▼
SnapshotBuilder
    │
    ▼
GameLoop
    │
    ▼
Room
    │
    ▼
ClientSession
    │
    ▼
WebSocket
    │
    ▼
ClientMessageProcessor
    │
    ▼
SharedState
    │
    ▼
ChessRenderer
```

---

# Room Architecture

```text
Room
│
├── White Player
├── Black Player
├── Spectators
├── Local Selection State
├── GameController
├── GameEngine
└── GameLoop
```

Each Room has:

- Independent game state
- Independent update loop
- Independent player assignments
- Independent spectator list
- Independent message broadcasting
- Independent selection and permission state

---

# Supported Protocol Messages

The current protocol supports:

```text
login
click
create_room
join_room
leave_room
room_result
snapshot
error
```

---

# Technologies

## Core

- C++17
- CMake
- Boost.Asio
- Boost.Beast
- nlohmann/json

## Graphics

- OpenCV
- Custom renderer components

## Testing

- GoogleTest

## Deployment

- Docker
- Docker Desktop
- Linux containers

---

# Project Structure

```text
KUNG_FU_CHESS/
│
├── src/
│   ├── model/
│   │   ├── Board
│   │   ├── Piece
│   │   └── Concrete piece classes
│   │
│   ├── movement/
│   │   ├── Move
│   │   └── PieceFactory
│   │
│   ├── rule_engine/
│   │   └── RuleEngine
│   │
│   ├── arbiter/
│   │   ├── RealTimeArbiter
│   │   └── CollisionResolver
│   │
│   ├── game_engine/
│   │   ├── GameEngine
│   │   ├── GameStateMachine
│   │   ├── BoardController
│   │   ├── MoveCompletionService
│   │   ├── ClickPreparationService
│   │   ├── ScoreTracker
│   │   ├── MoveRecorder
│   │   ├── CooldownService
│   │   └── SnapshotBuilder
│   │
│   ├── controllerClick/
│   │   └── GameController
│   │
│   ├── network/
│   │   ├── GameServer
│   │   ├── SessionManager
│   │   ├── ClientSession
│   │   ├── MessageRouter
│   │   ├── RoomManager
│   │   ├── Room
│   │   ├── GameLoop
│   │   ├── JsonProtocol
│   │   ├── NetworkClient
│   │   └── ClientMessageProcessor
│   │
│   ├── config/
│   │   ├── PieceSpeedConfig
│   │   └── PieceConfigReader
│   │
│   └── text_io/
│       ├── BoardParser
│       └── TextTestRunner
│
├── demo/
│   ├── assets/
│   ├── ChessRenderer
│   ├── BoardRenderer
│   ├── PieceRenderer
│   ├── AnimatedSprite
│   ├── AnimationManager
│   ├── AssetManager
│   ├── MouseHandler
│   └── DemoConfig
│
├── tests/
├── external/
├── Dockerfile.server
├── docker-compose.yml
├── .dockerignore
├── CMakeLists.txt
└── README.md
```

---

# Local Build

## Configure

```powershell
cmake -B build
```

When using vcpkg:

```powershell
cmake -B build `
  -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
```

## Build

```powershell
cmake --build build --config Debug
```

## Build Server Target

```powershell
cmake --build build --config Debug --target kungfuchess_server
```

## Build Client Target

```powershell
cmake --build build --config Debug --target kungfuchess_client
```

---

# Docker

## Build Server Image

```powershell
docker build `
  -f Dockerfile.server `
  -t kung-fu-chess-server:dev `
  .
```

## Run Server Container

```powershell
docker run --rm `
  --name kung-fu-chess-server `
  -p 8080:8080 `
  kung-fu-chess-server:dev
```

The server is exposed on:

```text
localhost:8080
```

## Check Running Containers

```powershell
docker ps
```

## View Server Logs

```powershell
docker logs -f kung-fu-chess-server
```

## Test Port Availability

```powershell
Test-NetConnection localhost -Port 8080
```

Expected result:

```text
TcpTestSucceeded : True
```

---

# Testing

Build the tests:

```powershell
cmake --build build --config Debug --target chess_tests
```

Run tests through CTest:

```powershell
ctest --test-dir build -C Debug --output-on-failure
```

---

# Current Deployment State

The current Docker deployment contains:

```text
Docker Image
└── kung-fu-chess-server:dev

Running Container
└── kung-fu-chess-server
    └── Port 8080
```

The graphical client currently runs locally on Windows and connects to the server container through:

```text
127.0.0.1:8080
```

---

# Roadmap

## Containerization

- Server Docker image
- Docker Compose development configuration
- Environment-based configuration
- Basic health checks
- Client container

## Infrastructure

- PostgreSQL
- Redis
- NATS
- Database migrations
- Structured logging
- Metrics

## Distributed Architecture

- API Service
- WebSocket Gateway
- Matchmaker
- Game Allocator
- Multiple Game Server shards
- Active Room Directory
- Reconnect support

## Production

- Kubernetes or K3s
- Horizontal scaling
- Readiness probes
- Liveness probes
- Rolling updates
- Load testing

---

# Architecture Constraint

The following components remain free of direct PostgreSQL, Redis, and message-broker logic:

```text
GameServer
Room
MessageRouter
GameEngine
GameController
```

Infrastructure access is introduced through interfaces and adapters.

---

# License

This project is developed for educational and engineering purposes.