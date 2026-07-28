#include "network/ClientMessageProcessor.hpp"

#include "network/JsonProtocol.hpp"
#include "network/Messages.hpp"
#include "network/SharedState.hpp"

#include "../../demo/ChessRenderer.hpp"

#include <exception>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace
{

bool isAnimatedState(PieceState state)
{
    return
        state == PieceState::Moving ||
        state == PieceState::Jumping;
}

} // namespace

ClientMessageProcessor::ClientMessageProcessor(
    SharedState& sharedState,
    ChessRenderer& renderer
)
    : sharedState_(sharedState),
      renderer_(renderer)
{
}

void ClientMessageProcessor::processPendingMessages()
{
    std::queue<std::string> pendingMessages;

    /*
     * החלפת התורים היא פעולה מהירה.
     * אין parsing או rendering בזמן שה-mutex נעול.
     */
    {
        std::lock_guard<std::mutex> lock(
            sharedState_.mtx
        );

        pendingMessages.swap(
            sharedState_.incomingMessages
        );
    }

    std::string latestSnapshotMessage;
    std::vector<std::string> controlMessages;

    while (!pendingMessages.empty())
    {
        std::string message =
            std::move(
                pendingMessages.front()
            );

        pendingMessages.pop();

        /*
         * רק ה-snapshot האחרון נשמר.
         */
        if (
            message.find(
                "\"type\":\"snapshot\""
            ) != std::string::npos
        )
        {
            latestSnapshotMessage =
                std::move(message);
        }
        else
        {
            controlMessages.push_back(
                std::move(message)
            );
        }
    }

    for (
        const std::string& message :
        controlMessages
    )
    {
        processControlMessage(message);
    }

    if (!latestSnapshotMessage.empty())
    {
        processSnapshotMessage(
            latestSnapshotMessage
        );
    }
}

void ClientMessageProcessor::processControlMessage(
    const std::string& message
)
{
    try
    {
        const MessageType type =
            JsonProtocol::getMessageType(
                message
            );

        switch (type)
        {
            case MessageType::Welcome:
            {
                const WelcomeMessage welcome =
                    JsonProtocol::
                        deserializeWelcome(
                            message
                        );

                std::cout
                    << "Assigned role: "
                    << welcome.color
                    << std::endl;

                break;
            }

            case MessageType::Error:
            {
                const ErrorMessage error =
                    JsonProtocol::
                        deserializeError(
                            message
                        );

                std::cerr
                    << "Server error: "
                    << error.message
                    << std::endl;

                break;
            }

            case MessageType::LoginResult:
            {
                const LoginResultMessage result =
                    JsonProtocol::
                        deserializeLoginResult(
                            message
                        );

                {
                    std::lock_guard<std::mutex> lock(
                        sharedState_.mtx
                    );

                    sharedState_.authenticated =
                        result.success;

                    if (result.success)
                    {
                        sharedState_.username =
                            result.username;
                    }
                    else
                    {
                        sharedState_.username.clear();
                    }
                }

                if (result.success)
                {
                    std::cout
                        << "Logged in as: "
                        << result.username
                        << std::endl;
                }
                else
                {
                    std::cerr
                        << "Login failed: "
                        << result.message
                        << std::endl;
                }

                break;
            }

            case MessageType::RoomResult:
            {
                const RoomResultMessage result =
                    JsonProtocol::
                        deserializeRoomResult(
                            message
                        );

                if (result.success)
                {
                    {
                        std::lock_guard<std::mutex> lock(
                            sharedState_.mtx
                        );

                        if (
                            result.action == "create" ||
                            result.action == "join"
                        )
                        {
                            sharedState_.currentRoomId =
                                result.roomId;
                        }
                        else if (
                            result.action == "leave"
                        )
                        {
                            sharedState_
                                .currentRoomId
                                .clear();
                        }
                    }

                    std::cout
                        << "Room action succeeded"
                        << " [action="
                        << result.action
                        << ", roomId="
                        << result.roomId
                        << "]: "
                        << result.message
                        << std::endl;
                }
                else
                {
                    std::cerr
                        << "Room action failed"
                        << " [action="
                        << result.action
                        << ", roomId="
                        << result.roomId
                        << "]: "
                        << result.message
                        << std::endl;
                }

                break;
            }

            case MessageType::Login:
            case MessageType::Click:
            case MessageType::Snapshot:
            case MessageType::CreateRoom:
            case MessageType::JoinRoom:
            case MessageType::LeaveRoom:
            case MessageType::Unknown:
            default:
            {
                std::cerr
                    << "Unexpected server message"
                    << std::endl;

                break;
            }
        }
    }
    catch (const std::exception& exception)
    {
        std::cerr
            << "Control message parse error: "
            << exception.what()
            << std::endl;
    }
}

void ClientMessageProcessor::processSnapshotMessage(
    const std::string& message
)
{
    try
    {
        SnapshotMessage snapshotMessage =
            JsonProtocol::deserializeSnapshot(
                message
            );

        GameSnapshot& snapshot =
            snapshotMessage.snapshot;

        resetStartedAnimations(snapshot);

        renderer_.render(snapshot);
        renderer_.display();

        lastSnapshot_ =
            std::move(snapshot);
    }
    catch (const std::exception& exception)
    {
        std::cerr
            << "Snapshot parse error: "
            << exception.what()
            << std::endl;
    }
}

void ClientMessageProcessor::resetStartedAnimations(
    const GameSnapshot& snapshot
)
{
    if (!lastSnapshot_.has_value())
    {
        return;
    }

    std::unordered_map<int, PieceState>
        previousStates;

    previousStates.reserve(
        lastSnapshot_->pieces.size()
    );

    for (
        const PieceInfo& oldPiece :
        lastSnapshot_->pieces
    )
    {
        previousStates.emplace(
            oldPiece.pieceId,
            oldPiece.state
        );
    }

    for (
        const PieceInfo& piece :
        snapshot.pieces
    )
    {
        if (
            !isAnimatedState(piece.state) ||
            piece.progress != 0.0
        )
        {
            continue;
        }

        const auto previous =
            previousStates.find(
                piece.pieceId
            );

        if (
            previous == previousStates.end() ||
            previous->second == piece.state
        )
        {
            continue;
        }

        renderer_
            .getAnimMgr()
            .reset(piece.pieceId);
    }
}