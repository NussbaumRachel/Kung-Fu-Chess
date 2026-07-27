#pragma once

#include "model/Constants.hpp"

#include <optional>
#include <string_view>

enum class PlayerRole
{
    White,
    Black,
    Spectator
};

[[nodiscard]]
constexpr std::string_view playerRoleToString(
    PlayerRole role)
{
    switch (role)
    {
        case PlayerRole::White:
            return "White";

        case PlayerRole::Black:
            return "Black";

        case PlayerRole::Spectator:
            return "Spectator";
    }

    return "Spectator";
}

[[nodiscard]]
constexpr std::optional<Color> playerRoleToColor(
    PlayerRole role)
{
    switch (role)
    {
        case PlayerRole::White:
            return Color::White;

        case PlayerRole::Black:
            return Color::Black;

        case PlayerRole::Spectator:
            return std::nullopt;
    }

    return std::nullopt;
}