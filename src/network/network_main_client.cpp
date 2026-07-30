#include "network/ClientMessageProcessor.hpp"
#include "network/JsonProtocol.hpp"
#include "network/Messages.hpp"
#include "network/NetworkClient.hpp"
#include "network/SharedState.hpp"

#include "../../demo/ChessRenderer.hpp"

#include <chrono>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>

namespace
{

enum class StartupRoomActionType
{
    None,
    Create,
    Join,
    Leave
};

struct StartupRoomAction
{
    StartupRoomActionType type =
        StartupRoomActionType::None;

    std::string roomId;
};

void enqueueSerializedMessage(
    SharedState& shared,
    std::string message
)
{
    std::lock_guard<std::mutex> lock(
        shared.outgoingMtx
    );

    shared.outgoingMessages.push(
        std::move(message)
    );
}

void enqueueLogin(
    SharedState& shared,
    const std::string& username
)
{
    const LoginMessage login{
        username
    };

    enqueueSerializedMessage(
        shared,
        JsonProtocol::serializeLogin(login)
    );
}

void enqueueClick(
    SharedState& shared,
    int row,
    int col
)
{
    const ClickMessage click{
        row,
        col
    };

    enqueueSerializedMessage(
        shared,
        JsonProtocol::serializeClick(click)
    );

    std::cout
        << "CLICK: ("
        << row
        << ","
        << col
        << ")"
        << std::endl;
}

void enqueueRoomAction(
    SharedState& shared,
    const StartupRoomAction& action
)
{
    switch (action.type)
    {
        case StartupRoomActionType::Create:
        {
            const CreateRoomMessage message{
                action.roomId
            };

            enqueueSerializedMessage(
                shared,
                JsonProtocol::serializeCreateRoom(
                    message
                )
            );

            return;
        }

        case StartupRoomActionType::Join:
        {
            const JoinRoomMessage message{
                action.roomId
            };

            enqueueSerializedMessage(
                shared,
                JsonProtocol::serializeJoinRoom(
                    message
                )
            );

            return;
        }

        case StartupRoomActionType::Leave:
        {
            enqueueSerializedMessage(
                shared,
                JsonProtocol::serializeLeaveRoom(
                    LeaveRoomMessage{}
                )
            );

            return;
        }

        case StartupRoomActionType::None:
        default:
        {
            return;
        }
    }
}

std::string resolveAssetsPath(
    int argc,
    char* argv[]
)
{
    namespace fs = std::filesystem;

    std::string assetsPath = "assets";

    if (argc >= 2)
    {
        assetsPath = argv[1];
    }

    if (fs::exists(assetsPath))
    {
        return assetsPath;
    }

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
    {
        return alternatePath;
    }

    return assetsPath;
}

StartupRoomAction parseRoomAction(
    int argc,
    char* argv[]
)
{
    if (argc == 5)
    {
        return {};
    }

    const std::string action =
        argv[5];

    if (action == "create")
    {
        if (argc != 7)
        {
            throw std::invalid_argument(
                "The create action requires a room ID"
            );
        }

        return {
            StartupRoomActionType::Create,
            argv[6]
        };
    }

    if (action == "join")
    {
        if (argc != 7)
        {
            throw std::invalid_argument(
                "The join action requires a room ID"
            );
        }

        return {
            StartupRoomActionType::Join,
            argv[6]
        };
    }

    if (action == "leave")
    {
        if (argc != 6)
        {
            throw std::invalid_argument(
                "The leave action does not accept a room ID"
            );
        }

        return {
            StartupRoomActionType::Leave,
            {}
        };
    }

    throw std::invalid_argument(
        "Unknown room action: " + action
    );
}

bool waitForConnection(
    SharedState& shared
)
{
    while (true)
    {
        bool connected = false;
        bool running = false;

        {
            std::lock_guard<std::mutex> lock(
                shared.mtx
            );

            connected =
                shared.connected;

            running =
                shared.running;
        }

        if (connected)
        {
            return true;
        }

        if (!running)
        {
            return false;
        }

        std::this_thread::sleep_for(
            std::chrono::milliseconds(50)
        );
    }
}
bool isRunning(
    SharedState& shared
)
{
    std::lock_guard<std::mutex> lock(
        shared.mtx
    );

    return shared.running;
}

bool waitForLoginResult(
    SharedState& shared,
    ClientMessageProcessor& messageProcessor
)
{
    while (isRunning(shared))
    {
        messageProcessor.processPendingMessages();

        if (messageProcessor.hasLoginResult())
        {
            return messageProcessor.loginSucceeded();
        }

        const int key =
            Img::wait_key(16);

        if (key == 27)
        {
            return false;
        }
    }

    return false;
}


void requestStop(
    SharedState& shared
)
{
    std::lock_guard<std::mutex> lock(
        shared.mtx
    );

    shared.running = false;
}

void stopClient(
    SharedState& shared,
    NetworkClient& networkClient,
    std::thread& networkThread
)
{
    Img::destroy_all_windows();

    requestStop(shared);
    networkClient.stop();

    if (networkThread.joinable())
    {
        networkThread.join();
    }
}

void printUsage()
{
    std::cerr
        << "Usage:\n"
        << "  kungfuchess_client "
        << "<assets-path> <username> <host> <port>\n"
        << "  kungfuchess_client "
        << "<assets-path> <username> <host> <port> "
        << "create <room-id>\n"
        << "  kungfuchess_client "
        << "<assets-path> <username> <host> <port> "
        << "join <room-id>\n"
        << "  kungfuchess_client "
        << "<assets-path> <username> <host> <port> "
        << "leave"
        << std::endl;
}
} // namespace

int main(int argc, char* argv[])
{
    if (argc < 5 || argc > 7)
    {
        printUsage();
        return 1;
    }

    StartupRoomAction roomAction;

    try
    {
        roomAction =
            parseRoomAction(
                argc,
                argv
            );
    }
    catch (const std::exception& exception)
    {
        std::cerr
            << exception.what()
            << std::endl;

        printUsage();

        return 1;
    }

    const std::string assetsPath =
        resolveAssetsPath(
            argc,
            argv
        );

    const std::string username =
        argv[2];
    const std::string host =
        argv[3];

    const std::string port =
        argv[4];

    std::cout
        << "Server: "
        << host
        << ":"
        << port
        << std::endl;
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
        host,
        port
    );
    std::thread networkThread(
        [&networkClient]()
        {
            networkClient.run();
        }
    );

    if (!waitForConnection(shared))
    {
        stopClient(
            shared,
            networkClient,
            networkThread
        );

        std::cerr
            << "Failed to connect"
            << std::endl;

        return 1;
    }

    ClientMessageProcessor messageProcessor(
        shared,
        renderer
    );

    enqueueLogin(
        shared,
        username
    );

    /*
     * פעולת החדר אינה נשלחת עד שהשרת מאשר
     * שה-login הצליח.
     */
    if (
        !waitForLoginResult(
            shared,
            messageProcessor
        )
    )
    {
        stopClient(
            shared,
            networkClient,
            networkThread
        );

        return 1;
    }

    enqueueRoomAction(
        shared,
        roomAction
    );

    renderer.setClickCallback(
        [&shared](int row, int col)
        {
            enqueueClick(
                shared,
                row,
                col
            );
        }
    );

    renderer.attachMouse();

    while (isRunning(shared))
    {
        messageProcessor.processPendingMessages();

        if (!renderer.isWindowOpen())
        {
            requestStop(shared);
            break;
        }

        const int key =
            Img::wait_key(16);

        if (key == 27)
        {
            requestStop(shared);
            break;
        }
    }

    stopClient(
        shared,
        networkClient,
        networkThread
    );

    return 0;
}