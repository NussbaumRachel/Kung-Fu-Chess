#pragma once

#include <boost/asio.hpp>

#include <cstdint>
#include <memory>
#include <string>


class ClientSession;
class GameController;


class GameServer
{
public:

    GameServer(uint16_t port, GameController& controller);

    void run();

    void onSessionReady(std::shared_ptr<ClientSession> session);


private:

    void acceptNext();


private:

    boost::asio::io_context ioContext_;

    boost::asio::ip::tcp::acceptor acceptor_;

    GameController& controller_;
};
