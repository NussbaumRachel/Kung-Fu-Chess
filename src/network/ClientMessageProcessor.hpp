#pragma once

#include "game_engine/GameSnapshot.hpp"

#include <optional>
#include <string>
#include <vector>

class ChessRenderer;
struct SharedState;

class ClientMessageProcessor
{
public:
    ClientMessageProcessor(
        SharedState& sharedState,
        ChessRenderer& renderer
    );

    void processPendingMessages();

private:
    void processControlMessage(
        const std::string& message
    );

    void processSnapshotMessage(
        const std::string& message
    );

    void resetStartedAnimations(
        const GameSnapshot& snapshot
    );

private:
    SharedState& sharedState_;
    ChessRenderer& renderer_;

    std::optional<GameSnapshot> lastSnapshot_;
};