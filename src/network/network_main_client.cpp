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
#include <string>
#include <thread>

namespace
{

std::string resolveAssetsPath(
    int argc,
    char* argv[]
)
{
    namespace fs = std::filesystem;

    std::string assetsPath = "assets";

    if (argc >= 2)
        assetsPath = argv[1];

    if (fs::exists(assetsPath))
        return assetsPath;

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
        return alternatePath;

    return assetsPath;
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

            connected = shared.connected;
            running = shared.running;
        }

        if (connected)
            return true;

        if (!running)
            return false;

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

void requestStop(
    SharedState& shared
)
{
    std::lock_guard<std::mutex> lock(
        shared.mtx
    );

    shared.running = false;
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

    std::string serializedClick =
        JsonProtocol::serializeClick(click);

    {
        std::lock_guard<std::mutex> lock(
            shared.outgoingMtx
        );

        shared.outgoingMessages.push(
            std::move(serializedClick)
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

} // namespace

int main(int argc, char* argv[])
{
    const std::string assetsPath =
        resolveAssetsPath(argc, argv);

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

    if (!waitForConnection(shared))
    {
        networkClient.stop();

        if (networkThread.joinable())
            networkThread.join();

        std::cerr
            << "Failed to connect"
            << std::endl;

        return 1;
    }

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

    ClientMessageProcessor messageProcessor(
        shared,
        renderer
    );

    while (isRunning(shared))
    {
        messageProcessor.processPendingMessages();

        if (!renderer.isWindowOpen())
        {
            requestStop(shared);
            break;
        }

        const int key = Img::wait_key(16);

        if (key == 27)
        {
            requestStop(shared);
            break;
        }
    }

    Img::destroy_all_windows();

    requestStop(shared);
    networkClient.stop();

    if (networkThread.joinable())
        networkThread.join();

    return 0;
}