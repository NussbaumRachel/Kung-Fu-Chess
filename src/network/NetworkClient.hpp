#pragma once

#include <memory>
#include <string>

struct SharedState;

class NetworkClient
{
public:
    NetworkClient(
        SharedState& state,
        std::string host,
        std::string port
    );

    ~NetworkClient();

    NetworkClient(const NetworkClient&) = delete;
    NetworkClient& operator=(const NetworkClient&) = delete;

    void run();
    void stop();

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};