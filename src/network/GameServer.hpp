#pragma once

#include <boost/asio.hpp>

#include <cstdint>
#include <memory>
#include <string>


class ClientSession;


class GameServer
{
public:

    explicit GameServer(uint16_t port);

    void run();


private:

    void acceptNext();


private:

    boost::asio::io_context ioContext_;

    boost::asio::ip::tcp::acceptor acceptor_;
};