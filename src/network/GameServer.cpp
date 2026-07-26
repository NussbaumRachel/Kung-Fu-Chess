#include "GameServer.hpp"
#include "ClientSession.hpp"
#include "controllerClick/GameController.hpp"
#include "game_engine/GameSnapshot.hpp"
#include "network/JsonProtocol.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
#include <chrono>


GameServer::GameServer(uint16_t port, GameController& controller)
    :
    ioContext_(),
    acceptor_(
        ioContext_,
        boost::asio::ip::tcp::endpoint(
            boost::asio::ip::tcp::v4(),
            port
        )
    ),
    controller_(controller),
    tickTimer_(ioContext_)
{
    std::cout
        << "Server listening on port "
        << port
        << std::endl;
}


void GameServer::run()
{
    acceptNext();
    startGameLoop();
    ioContext_.run();
}


void GameServer::onSessionReady(std::shared_ptr<ClientSession> session)
{
    sessions_.insert(session);

    // Assign color: first=White, second=Black, rest=spectator
    std::string colorStr;
    if (nextColor_ == 0) {
        colorStr = "White";
    } else if (nextColor_ == 1) {
        colorStr = "Black";
    } else {
        colorStr = "Spectator";
    }
    nextColor_++;

    std::cout << "Player joined as " << colorStr << std::endl;

    // Send initial welcome + snapshot
    {
        nlohmann::json welcome;
        welcome["type"] = "welcome";
        welcome["color"] = colorStr;
        session->send(welcome.dump());
    }

    GameSnapshot snap = controller_.getSnapshot();
    std::string json = JsonProtocol::serializeSnapshot(snap);
    session->send(json);
}


void GameServer::onMessage(std::shared_ptr<ClientSession> session,
                           const std::string& message)
{
    std::cout << "SERVER RECEIVED: " << message << std::endl;
    try
    {
        auto j = nlohmann::json::parse(message);
        std::string type = j.value("type", "");

        if (type == "click")
        {
            int row = j.value("row", -1);
            int col = j.value("col", -1);
            if (row >= 0 && col >= 0)
            {
                std::cout << "Click: (" << row << ", " << col << ")" << std::endl;
                controller_.handleCellClick(row, col);
            }
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Message parse error: " << e.what() << std::endl;
    }
}


void GameServer::onSessionClosed(std::shared_ptr<ClientSession> session)
{
    sessions_.erase(session);
    std::cout << "Player disconnected" << std::endl;
}


void GameServer::startGameLoop()
{
    tickTimer_.expires_after(std::chrono::milliseconds(TICK_MS));
    tickTimer_.async_wait([this](boost::system::error_code ec) {
        if (!ec)
            tick();
    });
}


void GameServer::tick()
{
    controller_.handleWait(TICK_MS);

    // Broadcast snapshot to all sessions
    GameSnapshot snap = controller_.getSnapshot();
    std::string json = JsonProtocol::serializeSnapshot(snap);

    for (const auto& session : sessions_)
        session->send(json);

    // Reschedule
    tickTimer_.expires_after(std::chrono::milliseconds(TICK_MS));
    tickTimer_.async_wait([this](boost::system::error_code ec) {
        if (!ec)
            tick();
    });
}


void GameServer::acceptNext()
{
    acceptor_.async_accept(
        [this](boost::system::error_code ec,
               boost::asio::ip::tcp::socket socket)
        {
            if (!ec)
            {
                std::cout
                    << "New TCP connection"
                    << std::endl;


                auto session =
                    std::make_shared<ClientSession>(
                        std::move(socket),
                        *this
                    );


                session->start();
            }
            else
            {
                std::cout
                    << "Accept error: "
                    << ec.message()
                    << std::endl;
            }


            acceptNext();
        }
    );
}
