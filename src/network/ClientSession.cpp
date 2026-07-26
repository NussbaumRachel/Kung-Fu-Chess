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


            self->server_.onSessionReady(self);


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

                self->server_.onSessionClosed(self);

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


            self->server_.onMessage(self, message);


            self->buffer_.consume(
                self->buffer_.size()
            );


            self->read();
        }
    );
}


void ClientSession::send(const std::string& message)
{
    auto msgPtr = std::make_shared<std::string>(message);
    bool shouldWrite = false;
    {
        std::lock_guard<std::mutex> lk(writeMutex_);
        writeQueue_.push(msgPtr);
        if (!writing_)
        {
            writing_ = true;
            shouldWrite = true;
        }
    }
    if (shouldWrite)
        writeNext();
}


void ClientSession::writeNext()
{
    auto self = shared_from_this();

    std::shared_ptr<std::string> msgPtr;
    {
        std::lock_guard<std::mutex> lk(writeMutex_);
        if (writeQueue_.empty())
        {
            writing_ = false;
            return;
        }
        msgPtr = writeQueue_.front();
        writeQueue_.pop();
    }

    // msgPtr is captured by the lambda, keeping the string data alive
    // for the duration of the async_write.
    ws_.async_write(
        boost::asio::buffer(*msgPtr),
        [self, msgPtr](boost::system::error_code ec,
               std::size_t bytes)
        {
            if (ec)
            {
                std::cout
                    << "Write failed: "
                    << ec.message()
                    << std::endl;
            }

            bool hasMore = false;
            {
                std::lock_guard<std::mutex> lk(self->writeMutex_);
                hasMore = !self->writeQueue_.empty();
                if (!hasMore)
                    self->writing_ = false;
            }
            if (hasMore)
                self->writeNext();
        }
    );
}
