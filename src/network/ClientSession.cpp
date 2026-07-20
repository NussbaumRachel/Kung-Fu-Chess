#include "ClientSession.hpp"
#include "GameServer.hpp"
#include <boost/beast/core/buffers_to_string.hpp>
#include <iostream>


ClientSession::ClientSession(
    boost::asio::ip::tcp::socket socket,
    GameServer& server
)
    :
    ws_(std::move(socket)),
    server_(server)
{
}


void ClientSession::start()
{
    doHandshake();
}


void ClientSession::doHandshake()
{
    auto self = shared_from_this();


    ws_.async_accept(
        [self](boost::system::error_code ec)
        {
            if (ec)
            {
                std::cout
                    << "Handshake failed: "
                    << ec.message()
                    << std::endl;

                return;
            }


            std::cout
                << "Client connected"
                << std::endl;


            self->read();
        }
    );
}


void ClientSession::read()
{
    auto self = shared_from_this();


    ws_.async_read(
        buffer_,
        [self](boost::system::error_code ec,
               std::size_t bytes)
        {
            if (ec)
            {
                std::cout
                    << "Client disconnected: "
                    << ec.message()
                    << std::endl;

                return;
            }


            std::string message =
                boost::beast::buffers_to_string(
                    self->buffer_.data()
                );


            std::cout
                << "Received: "
                << message
                << std::endl;


            self->buffer_.consume(
                self->buffer_.size()
            );


            self->read();
        }
    );
}


void ClientSession::send(
    const std::string& message
)
{
    auto self = shared_from_this();


    ws_.async_write(
        boost::asio::buffer(message),
        [self](boost::system::error_code ec,
               std::size_t bytes)
        {
            if (ec)
            {
                std::cout
                    << "Write failed: "
                    << ec.message()
                    << std::endl;
            }
        }
    );
}