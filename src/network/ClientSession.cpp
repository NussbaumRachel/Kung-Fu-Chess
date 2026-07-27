#include "network/ClientSession.hpp"

#include <boost/beast/core/buffers_to_string.hpp>

#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <utility>

ClientSession::ClientSession(
    boost::asio::ip::tcp::socket socket,
    ReadyHandler onReady,
    MessageHandler onMessage,
    ClosedHandler onClosed
)
    : ws_(std::move(socket)),
      onReady_(std::move(onReady)),
      onMessage_(std::move(onMessage)),
      onClosed_(std::move(onClosed))
{
}

void ClientSession::start()
{
    doHandshake();
}

void ClientSession::send(
    const std::string& message)
{
    auto messagePtr =
        std::make_shared<std::string>(message);

    bool shouldStartWriting = false;

    {
        std::lock_guard<std::mutex> lock(
            writeMutex_
        );

        writeQueue_.push(messagePtr);

        if (!writing_)
        {
            writing_ = true;
            shouldStartWriting = true;
        }
    }

    if (shouldStartWriting)
        writeNext();
}

void ClientSession::setRole(PlayerRole role)
{
    role_ = role;
}

PlayerRole ClientSession::role() const
{
    return role_;
}

void ClientSession::doHandshake()
{
    const auto self = shared_from_this();

    ws_.async_accept(
        [self](
            boost::system::error_code error)
        {
            if (error)
            {
                std::cerr
                    << "Handshake failed: "
                    << error.message()
                    << std::endl;

                self->notifyClosed();
                return;
            }

            std::cout
                << "Client connected"
                << std::endl;

            if (self->onReady_)
                self->onReady_(self);

            self->read();
        }
    );
}

void ClientSession::read()
{
    const auto self = shared_from_this();

    ws_.async_read(
        readBuffer_,
        [self](
            boost::system::error_code error,
            std::size_t)
        {
            if (error)
            {
                if (
                    error !=
                    boost::beast::websocket::
                        error::closed
                )
                {
                    std::cerr
                        << "Client read failed: "
                        << error.message()
                        << std::endl;
                }

                self->notifyClosed();
                return;
            }

            std::string message =
                boost::beast::buffers_to_string(
                    self->readBuffer_.data()
                );

            self->readBuffer_.consume(
                self->readBuffer_.size()
            );

            if (self->onMessage_)
            {
                self->onMessage_(
                    self,
                    message
                );
            }

            self->read();
        }
    );
}

void ClientSession::writeNext()
{
    const auto self = shared_from_this();

    std::shared_ptr<std::string> messagePtr;

    {
        std::lock_guard<std::mutex> lock(
            writeMutex_
        );

        if (writeQueue_.empty())
        {
            writing_ = false;
            return;
        }

        messagePtr = writeQueue_.front();
        writeQueue_.pop();
    }

    ws_.async_write(
        boost::asio::buffer(*messagePtr),
        [self, messagePtr](
            boost::system::error_code error,
            std::size_t)
        {
            if (error)
            {
                std::cerr
                    << "Write failed: "
                    << error.message()
                    << std::endl;

                {
                    std::lock_guard<std::mutex> lock(
                        self->writeMutex_
                    );

                    self->writing_ = false;

                    while (
                        !self->writeQueue_.empty()
                    )
                    {
                        self->writeQueue_.pop();
                    }
                }

                self->notifyClosed();
                return;
            }

            bool hasMoreMessages = false;

            {
                std::lock_guard<std::mutex> lock(
                    self->writeMutex_
                );

                hasMoreMessages =
                    !self->writeQueue_.empty();

                if (!hasMoreMessages)
                    self->writing_ = false;
            }

            if (hasMoreMessages)
                self->writeNext();
        }
    );
}

void ClientSession::notifyClosed()
{
    if (closedNotified_)
        return;

    closedNotified_ = true;

    if (onClosed_)
        onClosed_(shared_from_this());
}