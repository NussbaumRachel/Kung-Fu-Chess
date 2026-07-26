#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio.hpp>

#include <memory>
#include <string>
#include <queue>
#include <mutex>


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

    void writeNext();


private:

    boost::beast::websocket::stream<
        boost::asio::ip::tcp::socket
    > ws_;


    boost::beast::flat_buffer buffer_;

    GameServer& server_;

    // Write queue prevents overlapping async_write calls.
    // shared_ptr<string> keeps data alive during async_write.
    std::mutex writeMutex_;
    std::queue<std::shared_ptr<std::string>> writeQueue_;
    bool writing_ = false;
};
