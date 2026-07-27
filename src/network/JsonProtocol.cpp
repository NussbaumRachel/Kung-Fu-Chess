#include "network/JsonProtocol.hpp"

#include "model/Constants.hpp"
#include "model/Piece.hpp"
#include "model/Position.hpp"

#include <nlohmann/json.hpp>

#include <optional>
#include <stdexcept>
#include <string>

namespace
{

using Json = nlohmann::json;

std::string colorToString(Color color)
{
    switch (color)
    {
        case Color::White:
            return "White";

        case Color::Black:
            return "Black";
    }

    throw std::runtime_error("Unsupported Color value");
}

Color stringToColor(const std::string& value)
{
    if (value == "White")
        return Color::White;

    if (value == "Black")
        return Color::Black;

    throw std::runtime_error("Invalid color value: " + value);
}

std::string pieceTypeToString(PieceType type)
{
    switch (type)
    {
        case PieceType::King:
            return "King";

        case PieceType::Queen:
            return "Queen";

        case PieceType::Rook:
            return "Rook";

        case PieceType::Bishop:
            return "Bishop";

        case PieceType::Knight:
            return "Knight";

        case PieceType::Pawn:
            return "Pawn";
    }

    throw std::runtime_error("Unsupported PieceType value");
}

PieceType stringToPieceType(const std::string& value)
{
    if (value == "King")
        return PieceType::King;

    if (value == "Queen")
        return PieceType::Queen;

    if (value == "Rook")
        return PieceType::Rook;

    if (value == "Bishop")
        return PieceType::Bishop;

    if (value == "Knight")
        return PieceType::Knight;

    if (value == "Pawn")
        return PieceType::Pawn;

    throw std::runtime_error("Invalid piece type value: " + value);
}

std::string pieceStateToString(PieceState state)
{
    switch (state)
    {
        case PieceState::Idle:
            return "Idle";

        case PieceState::Moving:
            return "Moving";

        case PieceState::Jumping:
            return "Jumping";

        case PieceState::long_rest:
            return "long_rest";

        case PieceState::Short_rest:
            return "Short_rest";

        case PieceState::Captured:
            return "Captured";
    }

    throw std::runtime_error("Unsupported PieceState value");
}

PieceState stringToPieceState(const std::string& value)
{
    if (value == "Idle")
        return PieceState::Idle;

    if (value == "Moving")
        return PieceState::Moving;

    if (value == "Jumping")
        return PieceState::Jumping;

    if (value == "long_rest")
        return PieceState::long_rest;

    if (value == "Short_rest")
        return PieceState::Short_rest;

    if (value == "Captured")
        return PieceState::Captured;

    throw std::runtime_error("Invalid piece state value: " + value);
}

Json positionToJson(const Position& position)
{
    return Json{
        {"row", position.row},
        {"col", position.col}
    };
}

Position positionFromJson(const Json& json)
{
    if (!json.is_object())
        throw std::runtime_error("Position must be a JSON object");

    if (!json.contains("row") || !json.contains("col"))
        throw std::runtime_error("Position must contain row and col");

    return Position{
        json.at("row").get<int>(),
        json.at("col").get<int>()
    };
}

Json optionalPositionToJson(const std::optional<Position>& position)
{
    if (!position.has_value())
        return nullptr;

    return positionToJson(position.value());
}

std::optional<Position> optionalPositionFromJson(const Json& json)
{
    if (json.is_null())
        return std::nullopt;

    return positionFromJson(json);
}

Json optionalColorToJson(const std::optional<Color>& color)
{
    if (!color.has_value())
        return nullptr;

    return colorToString(color.value());
}

std::optional<Color> optionalColorFromJson(const Json& json)
{
    if (json.is_null())
        return std::nullopt;

    return stringToColor(json.get<std::string>());
}

Json pieceInfoToJson(const PieceInfo& piece)
{
    return Json{
        {"kind", pieceTypeToString(piece.kind)},
        {"color", colorToString(piece.color)},
        {"pieceId", piece.pieceId},
        {"cell", positionToJson(piece.cell)},
        {"state", pieceStateToString(piece.state)},
        {"progress", piece.progress},
        {"targetCell", optionalPositionToJson(piece.targetCell)}
    };
}

PieceInfo pieceInfoFromJson(const Json& json)
{
    if (!json.is_object())
        throw std::runtime_error("PieceInfo must be a JSON object");

    PieceInfo piece;

    piece.kind = stringToPieceType(
        json.at("kind").get<std::string>()
    );

    piece.color = stringToColor(
        json.at("color").get<std::string>()
    );

    piece.pieceId = json.at("pieceId").get<int>();
    piece.cell = positionFromJson(json.at("cell"));

    piece.state = stringToPieceState(
        json.at("state").get<std::string>()
    );

    piece.progress = json.value("progress", 0.0);

    if (json.contains("targetCell"))
        piece.targetCell = optionalPositionFromJson(json.at("targetCell"));
    else
        piece.targetCell = std::nullopt;

    return piece;
}

Json moveRecordToJson(const MoveRecord& record)
{
    return Json{
        {"minutes", record.minutes},
        {"seconds", record.seconds},
        {"milliseconds", record.milliseconds},
        {"pieceType", pieceTypeToString(record.pieceType)},
        {"color", colorToString(record.color)},
        {"from", positionToJson(record.from)},
        {"to", positionToJson(record.to)},
        {"isJump", record.isJump},
        {"isCapture", record.isCapture},
        {"givesCheck", record.givesCheck}
    };
}

MoveRecord moveRecordFromJson(const Json& json)
{
    if (!json.is_object())
        throw std::runtime_error("MoveRecord must be a JSON object");

    MoveRecord record;

    record.minutes = json.value("minutes", 0);
    record.seconds = json.value("seconds", 0);
    record.milliseconds = json.value("milliseconds", 0);

    record.pieceType = stringToPieceType(
        json.at("pieceType").get<std::string>()
    );

    record.color = stringToColor(
        json.at("color").get<std::string>()
    );

    record.from = positionFromJson(json.at("from"));
    record.to = positionFromJson(json.at("to"));

    record.isJump = json.value("isJump", false);
    record.isCapture = json.value("isCapture", false);
    record.givesCheck = json.value("givesCheck", false);

    return record;
}

void requireMessageType(
    const Json& json,
    const std::string& expectedType)
{
    if (!json.is_object())
        throw std::runtime_error("Network message must be a JSON object");

    const std::string actualType = json.value("type", "");

    if (actualType != expectedType)
    {
        throw std::runtime_error(
            "Expected message type '" +
            expectedType +
            "', received '" +
            actualType +
            "'"
        );
    }
}

} // namespace

MessageType JsonProtocol::getMessageType(
    const std::string& jsonText)
{
    const Json json = Json::parse(jsonText);
    const std::string type =
        json.value("type", "");

    if (type == "welcome")
        return MessageType::Welcome;

    if (type == "login")
        return MessageType::Login;

    if (type == "login_result")
        return MessageType::LoginResult;

    if (type == "click")
        return MessageType::Click;

    if (type == "snapshot")
        return MessageType::Snapshot;

    if (type == "error")
        return MessageType::Error;

    return MessageType::Unknown;
}

std::string JsonProtocol::serializeWelcome(
    const WelcomeMessage& message)
{
    const Json json{
        {"type", "welcome"},
        {"color", message.color}
    };

    return json.dump();
}

WelcomeMessage JsonProtocol::deserializeWelcome(
    const std::string& jsonText)
{
    const Json json = Json::parse(jsonText);
    requireMessageType(json, "welcome");

    WelcomeMessage message;
    message.color = json.at("color").get<std::string>();

    return message;
}
std::string JsonProtocol::serializeLogin(
    const LoginMessage& message)
{
    const Json json{
        {"type", "login"},
        {"username", message.username}
    };

    return json.dump();
}

LoginMessage JsonProtocol::deserializeLogin(
    const std::string& jsonText)
{
    const Json json = Json::parse(jsonText);

    requireMessageType(
        json,
        "login"
    );

    LoginMessage message;
    message.username =
        json.at("username").get<std::string>();

    return message;
}

std::string JsonProtocol::serializeLoginResult(
    const LoginResultMessage& message)
{
    const Json json{
        {"type", "login_result"},
        {"success", message.success},
        {"username", message.username},
        {"message", message.message}
    };

    return json.dump();
}

LoginResultMessage
JsonProtocol::deserializeLoginResult(
    const std::string& jsonText)
{
    const Json json = Json::parse(jsonText);

    requireMessageType(
        json,
        "login_result"
    );

    LoginResultMessage message;

    message.success =
        json.at("success").get<bool>();

    message.username =
        json.value("username", "");

    message.message =
        json.value("message", "");

    return message;
}
std::string JsonProtocol::serializeClick(
    const ClickMessage& message)
{
    const Json json{
        {"type", "click"},
        {"row", message.row},
        {"col", message.col}
    };

    return json.dump();
}

ClickMessage JsonProtocol::deserializeClick(
    const std::string& jsonText)
{
    const Json json = Json::parse(jsonText);
    requireMessageType(json, "click");

    ClickMessage message;
    message.row = json.at("row").get<int>();
    message.col = json.at("col").get<int>();

    return message;
}

std::string JsonProtocol::serializeSnapshot(
    const GameSnapshot& snapshot)
{
    Json json;
    json["type"] = "snapshot";

    Json& data = json["data"];

    data["boardWidth"] = snapshot.boardWidth;
    data["boardHeight"] = snapshot.boardHeight;
    data["selectedCell"] =
        optionalPositionToJson(snapshot.selectedCell);
    data["gameOver"] = snapshot.gameOver;
    data["winner"] = optionalColorToJson(snapshot.winner);
    data["whiteScore"] = snapshot.whiteScore;
    data["blackScore"] = snapshot.blackScore;

    data["pieces"] = Json::array();

    for (const PieceInfo& piece : snapshot.pieces)
        data["pieces"].push_back(pieceInfoToJson(piece));

    data["whiteMoves"] = Json::array();

    for (const MoveRecord& record : snapshot.whiteMoves)
        data["whiteMoves"].push_back(moveRecordToJson(record));

    data["blackMoves"] = Json::array();

    for (const MoveRecord& record : snapshot.blackMoves)
        data["blackMoves"].push_back(moveRecordToJson(record));

    return json.dump();
}

SnapshotMessage JsonProtocol::deserializeSnapshot(
    const std::string& jsonText)
{
    const Json json = Json::parse(jsonText);
    requireMessageType(json, "snapshot");

    if (!json.contains("data") || !json.at("data").is_object())
        throw std::runtime_error("Snapshot message is missing data");

    const Json& data = json.at("data");

    SnapshotMessage message;
    GameSnapshot& snapshot = message.snapshot;

    snapshot.boardWidth = data.value("boardWidth", 0);
    snapshot.boardHeight = data.value("boardHeight", 0);
    snapshot.gameOver = data.value("gameOver", false);
    snapshot.whiteScore = data.value("whiteScore", 0);
    snapshot.blackScore = data.value("blackScore", 0);

    if (data.contains("selectedCell"))
    {
        snapshot.selectedCell =
            optionalPositionFromJson(data.at("selectedCell"));
    }

    if (data.contains("winner"))
    {
        snapshot.winner =
            optionalColorFromJson(data.at("winner"));
    }

    if (data.contains("pieces"))
    {
        if (!data.at("pieces").is_array())
            throw std::runtime_error("Snapshot pieces must be an array");

        for (const Json& pieceJson : data.at("pieces"))
        {
            snapshot.pieces.push_back(
                pieceInfoFromJson(pieceJson)
            );
        }
    }

    if (data.contains("whiteMoves"))
    {
        if (!data.at("whiteMoves").is_array())
        {
            throw std::runtime_error(
                "Snapshot whiteMoves must be an array"
            );
        }

        for (const Json& moveJson : data.at("whiteMoves"))
        {
            snapshot.whiteMoves.push_back(
                moveRecordFromJson(moveJson)
            );
        }
    }

    if (data.contains("blackMoves"))
    {
        if (!data.at("blackMoves").is_array())
        {
            throw std::runtime_error(
                "Snapshot blackMoves must be an array"
            );
        }

        for (const Json& moveJson : data.at("blackMoves"))
        {
            snapshot.blackMoves.push_back(
                moveRecordFromJson(moveJson)
            );
        }
    }

    return message;
}

std::string JsonProtocol::serializeError(
    const ErrorMessage& message)
{
    const Json json{
        {"type", "error"},
        {"message", message.message}
    };

    return json.dump();
}

ErrorMessage JsonProtocol::deserializeError(
    const std::string& jsonText)
{
    const Json json = Json::parse(jsonText);
    requireMessageType(json, "error");

    ErrorMessage message;
    message.message = json.at("message").get<std::string>();

    return message;
}