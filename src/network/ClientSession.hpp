#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio.hpp>

#include <memory>
#include <string>


class GameServer;


class ClientSession :
    public std::enable_shared_from_this<ClientSession>
{
public:

    ClientSession(
        boost::asio::ip::tcp::socket socket,
        GameServer& server
    );


    void start();

    void send(const std::string& message);


private:

    void doHandshake();

    void read();


private:

    boost::beast::websocket::stream<
        boost::asio::ip::tcp::socket
    > ws_;


    boost::beast::flat_buffer buffer_;

    GameServer& server_;
};