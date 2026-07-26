#include "network/JsonProtocol.hpp"
#include "network/Messages.hpp"
#include "network/NetworkClient.hpp"
#include "network/SharedState.hpp"
#include "../../demo/ChessRenderer.hpp"
#include "../../demo/DemoConfig.hpp"
#include <chrono>
#include <filesystem>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <thread>
#include <utility>
#include <vector>


int main(int argc, char* argv[])
{
    namespace fs = std::filesystem;

    std::string assetsPath = "assets";

    if (argc >= 2)
        assetsPath = argv[1];

    if (!fs::exists(assetsPath))
    {
        const std::string alternatePath =
            (
                fs::path(__FILE__)
                    .parent_path()
                    .parent_path()
                    .parent_path() /
                "demo" /
                "assets"
            ).string();

        if (fs::exists(alternatePath))
            assetsPath = alternatePath;
    }

    std::cout
        << "Assets path: "
        << assetsPath
        << std::endl;

    ChessRenderer renderer;

    if (!renderer.initialize(assetsPath))
    {
        std::cerr
            << "Failed to initialize renderer."
            << std::endl;

        return 1;
    }

    SharedState shared;

    NetworkClient networkClient(
    shared,
    "127.0.0.1",
    "8080"
);

std::thread networkThread(
    [&networkClient]()
    {
        networkClient.run();
    }
);
    while (true)
    {
        bool connected = false;
        bool running = false;

        {
            std::lock_guard<std::mutex> lock(
                shared.mtx
            );

            connected = shared.connected;
            running = shared.running;
        }

        if (connected || !running)
            break;

        std::this_thread::sleep_for(
            std::chrono::milliseconds(50)
        );
    }

    {
        std::lock_guard<std::mutex> lock(
            shared.mtx
        );

        if (!shared.running)
        {
            networkClient.stop();

            if (networkThread.joinable())
                networkThread.join();

            std::cerr
                << "Failed to connect"
                << std::endl;

            return 1;
        }
    }

    renderer.setClickCallback(
        [&shared](int row, int col)
        {
            const ClickMessage click{
                row,
                col
            };

            const std::string serializedClick =
                JsonProtocol::serializeClick(click);

            {
                std::lock_guard<std::mutex> lock(
                    shared.outgoingMtx
                );

                shared.outgoingMessages.push(
                    serializedClick
                );
            }

            std::cout
                << "CLICK: ("
                << row
                << ","
                << col
                << ")"
                << std::endl;
        }
    );

    renderer.attachMouse();

    std::optional<GameSnapshot> lastSnapshot;

    while (true)
    {
        bool running = false;

        {
            std::lock_guard<std::mutex> lock(
                shared.mtx
            );

            running = shared.running;
        }

        if (!running)
            break;

        std::string latestSnapshotMessage;
        std::vector<std::string> controlMessages;

        {
            std::lock_guard<std::mutex> lock(
                shared.mtx
            );

            while (!shared.incomingMessages.empty())
            {
                std::string message =
                    std::move(
                        shared.incomingMessages.front()
                    );

                shared.incomingMessages.pop();

                /*
                 * Snapshot throttling remains intentionally cheap:
                 * do not parse JSON while holding shared.mtx.
                 */
                if (
                    message.find("\"type\":\"snapshot\"") !=
                    std::string::npos
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
        }

        /*
         * Parse non-snapshot messages outside shared.mtx.
         */
        for (const std::string& message : controlMessages)
        {
            try
            {
                const MessageType type =
                    JsonProtocol::getMessageType(message);

                switch (type)
                {
                    case MessageType::Welcome:
                    {
                        const WelcomeMessage welcome =
                            JsonProtocol::
                                deserializeWelcome(message);

                        std::cout
                            << "Assigned color: "
                            << welcome.color
                            << std::endl;

                        break;
                    }

                    case MessageType::Error:
                    {
                        const ErrorMessage error =
                            JsonProtocol::
                                deserializeError(message);

                        std::cerr
                            << "Server error: "
                            << error.message
                            << std::endl;

                        break;
                    }

                    case MessageType::Click:
                    case MessageType::Snapshot:
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

        /*
         * Parse and render only the latest snapshot,
         * outside shared.mtx.
         */
        if (!latestSnapshotMessage.empty())
        {
            try
            {
                SnapshotMessage snapshotMessage =
                    JsonProtocol::deserializeSnapshot(
                        latestSnapshotMessage
                    );

                GameSnapshot& snapshot =
                    snapshotMessage.snapshot;

                if (lastSnapshot.has_value())
                {
                    const auto& oldPieces =
                        lastSnapshot->pieces;

                    for (const PieceInfo& piece :
                         snapshot.pieces)
                    {
                        if (
                            piece.state ==
                                PieceState::Moving &&
                            piece.progress == 0.0
                        )
                        {
                            bool animationStarted = false;

                            for (const PieceInfo& oldPiece :
                                 oldPieces)
                            {
                                if (
                                    oldPiece.pieceId ==
                                        piece.pieceId &&
                                    oldPiece.state !=
                                        PieceState::Moving
                                )
                                {
                                    animationStarted = true;
                                    break;
                                }
                            }

                            if (animationStarted)
                            {
                                renderer
                                    .getAnimMgr()
                                    .reset(piece.pieceId);
                            }
                        }
                        else if (
                            piece.state ==
                                PieceState::Jumping &&
                            piece.progress == 0.0
                        )
                        {
                            bool animationStarted = false;

                            for (const PieceInfo& oldPiece :
                                 oldPieces)
                            {
                                if (
                                    oldPiece.pieceId ==
                                        piece.pieceId &&
                                    oldPiece.state !=
                                        PieceState::Jumping
                                )
                                {
                                    animationStarted = true;
                                    break;
                                }
                            }

                            if (animationStarted)
                            {
                                renderer
                                    .getAnimMgr()
                                    .reset(piece.pieceId);
                            }
                        }
                    }
                }

                lastSnapshot = snapshot;

                renderer.render(snapshot);
                renderer.display();
            }
            catch (const std::exception& exception)
            {
                std::cerr
                    << "Snapshot parse error: "
                    << exception.what()
                    << std::endl;
            }
        }

        if (!renderer.isWindowOpen())
        {
            std::lock_guard<std::mutex> lock(
                shared.mtx
            );

            shared.running = false;
            break;
        }

        const int key = Img::wait_key(16);

        if (key == 27)
        {
            std::lock_guard<std::mutex> lock(
                shared.mtx
            );

            shared.running = false;
            break;
        }
    }

    Img::destroy_all_windows();

    {
        std::lock_guard<std::mutex> lock(
            shared.mtx
        );

        shared.running = false;
    }

networkClient.stop();

if (networkThread.joinable())
    networkThread.join();
    return 0;
}