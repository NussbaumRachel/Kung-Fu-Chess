#include "network/ClientSession.hpp"

#include <boost/asio/error.hpp>
#include <exception>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <utility>

namespace
{
    constexpr auto SESSION_REFRESH_INTERVAL =
        std::chrono::seconds(30);
}

ClientSession::ClientSession(
    boost::asio::ip::tcp::socket socket,
    ReadyHandler onReady,
    MessageHandler onMessage,
    RefreshHandler onRefresh,
    ClosedHandler onClosed
)
    : ws_(std::move(socket)),
      sessionRefreshTimer_(
          ws_.get_executor()
      ),
      onReady_(std::move(onReady)),
      onMessage_(std::move(onMessage)),
      onRefresh_(std::move(onRefresh)),
      onClosed_(std::move(onClosed)),
      sessionId_(generateSessionId())
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
    {
        writeNext();
    }
}

void ClientSession::setRole(PlayerRole role)
{
    role_ = role;
}

PlayerRole ClientSession::role() const
{
    return role_;
}

void ClientSession::authenticate(
    UserId userId,
    std::string username)
{
    userId_ = userId;
    username_ = std::move(username);

    startSessionRefresh();
}

void ClientSession::clearAuthentication()
{
    cancelSessionRefresh();

    userId_.reset();
    username_.clear();
}

bool ClientSession::isAuthenticated() const
{
    return userId_.has_value();
}

std::optional<UserId>
ClientSession::userId() const
{
    return userId_;
}

const std::string&
ClientSession::username() const
{
    return username_;
}

const std::string&
ClientSession::sessionId() const
{
    return sessionId_;
}

std::string ClientSession::generateSessionId()
{
    std::random_device randomDevice;

    std::mt19937_64 generator(
        randomDevice()
    );

    std::uniform_int_distribution<
        unsigned long long
    > distribution;

    const unsigned long long first =
        distribution(generator);

    const unsigned long long second =
        distribution(generator);

    std::ostringstream stream;

    stream
        << std::hex
        << std::setfill('0')
        << std::setw(16)
        << first
        << std::setw(16)
        << second;

    return stream.str();
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
            {
                self->onReady_(self);
            }

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
                {
                    self->writing_ = false;
                }
            }

            if (hasMoreMessages)
            {
                self->writeNext();
            }
        }
    );
}

void ClientSession::startSessionRefresh()
{
    if (
        refreshStarted_ ||
        !isAuthenticated()
    )
    {
        return;
    }

    refreshStarted_ = true;

    sessionRefreshTimer_.expires_after(
        SESSION_REFRESH_INTERVAL
    );

    const auto self = shared_from_this();

    sessionRefreshTimer_.async_wait(
        [self](
            const boost::system::error_code& error)
        {
            self->refreshStarted_ = false;

            if (
                error ==
                boost::asio::error::operation_aborted
            )
            {
                return;
            }

            if (error)
            {
                std::cerr
                    << "Session refresh timer failed: "
                    << error.message()
                    << std::endl;

                self->notifyClosed();
                return;
            }

            if (
                !self->isAuthenticated() ||
                !self->onRefresh_
            )
            {
                return;
            }

            bool refreshed = false;

            try
            {
                refreshed =
                    self->onRefresh_(self);
            }
            catch (const std::exception& exception)
            {
                std::cerr
                    << "Session refresh failed with exception"
                    << " for session_id="
                    << self->sessionId_
                    << ": "
                    << exception.what()
                    << std::endl;

                self->notifyClosed();
                return;
            }
            catch (...)
            {
                std::cerr
                    << "Session refresh failed with unknown exception"
                    << " for session_id="
                    << self->sessionId_
                    << std::endl;

                self->notifyClosed();
                return;
            }

            if (!refreshed)
            {
                std::cerr
                    << "Session refresh rejected for session_id="
                    << self->sessionId_
                    << std::endl;

                self->notifyClosed();
                return;
            }

            self->startSessionRefresh();
        }
    );
}
void ClientSession::cancelSessionRefresh()
{
    refreshStarted_ = false;

    try
    {
        sessionRefreshTimer_.cancel();
    }
    catch (const boost::system::system_error& exception)
    {
        std::cerr
            << "Failed to cancel session refresh timer: "
            << exception.what()
            << std::endl;
    }
}
void ClientSession::notifyClosed()
{
    if (closedNotified_)
    {
        return;
    }

    closedNotified_ = true;

    cancelSessionRefresh();

    if (onClosed_)
    {
        onClosed_(shared_from_this());
    }
}