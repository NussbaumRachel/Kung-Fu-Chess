#include "network/NetworkClient.hpp"
#include "network/SharedState.hpp"

#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

#include <atomic>
#include <chrono>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <utility>

namespace
{

using Tcp = boost::asio::ip::tcp;

using WebSocket =
    boost::beast::websocket::stream<Tcp::socket>;

constexpr std::chrono::milliseconds
    OUTGOING_POLL_INTERVAL{5};

} // namespace

class NetworkClient::Impl
{
public:
    Impl(
        SharedState& state,
        std::string host,
        std::string port)
        : state_(state),
          host_(std::move(host)),
          port_(std::move(port)),
          webSocket_(
              std::make_shared<WebSocket>(ioContext_)),
          outgoingTimer_(ioContext_)
    {
    }

    void run()
    {
        try
        {
            connectToServer();

            startRead();
            scheduleOutgoingDrain();

            ioContext_.run();
        }
        catch (const std::exception& exception)
        {
            if (!stopping_)
            {
                std::cerr
                    << "Network error: "
                    << exception.what()
                    << std::endl;
            }

            markStopped();
        }
    }

    void stop()
    {
        bool expected = false;

        if (!stopping_.compare_exchange_strong(
                expected,
                true))
        {
            return;
        }

        markStopped();

        boost::asio::post(
            ioContext_,
            [this]()
            {
                boost::system::error_code ignoredError;

                outgoingTimer_.cancel();

                if (webSocket_)
                {
                    auto& socket =
                        boost::beast::get_lowest_layer(
                            *webSocket_);

                    socket.cancel(ignoredError);

                    socket.shutdown(
                        Tcp::socket::shutdown_both,
                        ignoredError);

                    socket.close(ignoredError);
                }

                ioContext_.stop();
            });
    }

private:
    void connectToServer()
    {
        Tcp::resolver resolver(ioContext_);

        const auto endpoints =
            resolver.resolve(host_, port_);

        boost::asio::connect(
            webSocket_->next_layer(),
            endpoints);

        webSocket_->handshake(
            host_,
            "/");

        {
            std::lock_guard<std::mutex> lock(
                state_.mtx);

            state_.connected = true;
            state_.running = true;
        }

        std::cout
            << "Connected to server"
            << std::endl;
    }

    void startRead()
    {
        if (stopping_)
            return;

        auto buffer =
            std::make_shared<
                boost::beast::flat_buffer>();

        webSocket_->async_read(
            *buffer,
            [this, buffer](
                boost::system::error_code error,
                std::size_t)
            {
                if (error)
                {
                    handleReadError(error);
                    return;
                }

                std::string message =
                    boost::beast::buffers_to_string(
                        buffer->data());

                {
                    std::lock_guard<std::mutex> lock(
                        state_.mtx);

                    state_.incomingMessages.push(
                        std::move(message));
                }

                if (isRunning() && !stopping_)
                    startRead();
            });
    }

    void handleReadError(
        const boost::system::error_code& error)
    {
        if (
            error ==
                boost::beast::websocket::error::closed ||
            error ==
                boost::asio::error::eof)
        {
            if (!stopping_)
            {
                std::cout
                    << "Server closed connection"
                    << std::endl;
            }
        }
        else if (
            error !=
                boost::asio::error::operation_aborted &&
            !stopping_)
        {
            std::cerr
                << "Read error: "
                << error.message()
                << std::endl;
        }

        markStopped();
        ioContext_.stop();
    }

    void scheduleOutgoingDrain()
    {
        if (stopping_)
            return;

        outgoingTimer_.expires_after(
            OUTGOING_POLL_INTERVAL);

        outgoingTimer_.async_wait(
            [this](
                boost::system::error_code error)
            {
                if (
                    error ==
                    boost::asio::error::operation_aborted)
                {
                    return;
                }

                if (error)
                {
                    if (!stopping_)
                    {
                        std::cerr
                            << "Outgoing timer error: "
                            << error.message()
                            << std::endl;
                    }

                    markStopped();
                    ioContext_.stop();
                    return;
                }

                drainOneOutgoingMessage();
            });
    }

    void drainOneOutgoingMessage()
    {
        if (stopping_ || !isRunning())
            return;

        std::shared_ptr<std::string> message;

        {
            std::lock_guard<std::mutex> lock(
                state_.outgoingMtx);

            if (!state_.outgoingMessages.empty())
            {
                message =
                    std::make_shared<std::string>(
                        std::move(
                            state_
                                .outgoingMessages
                                .front()));

                state_.outgoingMessages.pop();
            }
        }

        if (!message)
        {
            scheduleOutgoingDrain();
            return;
        }

        std::cout
            << "Sending: "
            << *message
            << std::endl;

        /*
         * message נשמר באמצעות shared_ptr עד לסיום
         * פעולת async_write, ולכן ה-buffer נשאר תקף.
         */
        webSocket_->async_write(
            boost::asio::buffer(*message),
            [this, message](
                boost::system::error_code error,
                std::size_t)
            {
                if (error)
                {
                    if (
                        error !=
                            boost::asio::error::
                                operation_aborted &&
                        !stopping_)
                    {
                        std::cerr
                            << "Write error: "
                            << error.message()
                            << std::endl;
                    }

                    markStopped();
                    ioContext_.stop();
                    return;
                }

                scheduleOutgoingDrain();
            });
    }

    bool isRunning()
    {
        std::lock_guard<std::mutex> lock(
            state_.mtx);

        return state_.running;
    }

    void markStopped()
    {
        std::lock_guard<std::mutex> lock(
            state_.mtx);

        state_.running = false;
        state_.connected = false;
    }

    SharedState& state_;

    std::string host_;
    std::string port_;

    boost::asio::io_context ioContext_;

    std::shared_ptr<WebSocket> webSocket_;

    boost::asio::steady_timer outgoingTimer_;

    std::atomic_bool stopping_{false};
};

NetworkClient::NetworkClient(
    SharedState& state,
    std::string host,
    std::string port)
    : impl_(
          std::make_unique<Impl>(
              state,
              std::move(host),
              std::move(port)))
{
}

NetworkClient::~NetworkClient()
{
    stop();
}

void NetworkClient::run()
{
    impl_->run();
}

void NetworkClient::stop()
{
    if (impl_)
        impl_->stop();
}