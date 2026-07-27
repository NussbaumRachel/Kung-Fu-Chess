#pragma once

#include <string>

struct LoginAttemptResult
{
    bool success = false;
    std::string message;
};