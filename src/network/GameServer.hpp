#pragma once

#include <boost/asio.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <set>
#include <vector>

class ClientSession;
class GameController;

class GameServer
{
public:

    GameServer(uint16_t port, GameController& controller);

    void run();

    void onSessionReady(std::shared_ptr<ClientSession> session);

    void onMessage(std::shared_ptr<ClientSession> session,
                   const std::string& message);

    void onSessionClosed(std::shared_ptr<ClientSession> session);


private:

    void acceptNext();

    void startGameLoop();

    void tick();


private:

    boost::asio::io_context ioContext_;

    boost::asio::ip::tcp::acceptor acceptor_;

    GameController& controller_;

    boost::asio::steady_timer tickTimer_;

    static constexpr int TICK_MS = 16;

    std::set<std::shared_ptr<ClientSession>> sessions_;

    int nextColor_ = 0;             // 0=White, 1=Black, 2+=Spectator
};

