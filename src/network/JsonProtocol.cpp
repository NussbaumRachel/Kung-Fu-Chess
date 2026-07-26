#include "network/JsonProtocol.hpp"
#include "game_engine/GameSnapshot.hpp"
#include "model/Constants.hpp"
#include "model/Position.hpp"
#include "model/Piece.hpp"

#include <nlohmann/json.hpp>


namespace {

std::string colorToString(Color c)
{
    return (c == Color::White) ? "White" : "Black";
}

std::string pieceTypeToString(PieceType t)
{
    switch (t) {
        case PieceType::King:   return "King";
        case PieceType::Queen:  return "Queen";
        case PieceType::Rook:   return "Rook";
        case PieceType::Bishop: return "Bishop";
        case PieceType::Knight: return "Knight";
        case PieceType::Pawn:   return "Pawn";
        default:                return "?";
    }
}

std::string pieceStateToString(PieceState s)
{
    switch (s) {
        case PieceState::Idle:       return "Idle";
        case PieceState::Moving:     return "Moving";
        case PieceState::Jumping:    return "Jumping";
        case PieceState::long_rest:  return "long_rest";
        case PieceState::Short_rest: return "Short_rest";
        case PieceState::Captured:   return "Captured";
        default:                     return "?";
    }
}

nlohmann::json positionToJson(const Position& p)
{
    return {{"row", p.row}, {"col", p.col}};
}

nlohmann::json optionalPositionToJson(const std::optional<Position>& opt)
{
    if (opt.has_value())
        return positionToJson(opt.value());
    return nullptr;
}

nlohmann::json pieceInfoToJson(const PieceInfo& pi)
{
    nlohmann::json j;
    j["kind"]       = pieceTypeToString(pi.kind);
    j["color"]      = colorToString(pi.color);
    j["pieceId"]    = pi.pieceId;
    j["cell"]       = positionToJson(pi.cell);
    j["state"]      = pieceStateToString(pi.state);
    j["progress"]   = pi.progress;
    j["targetCell"] = optionalPositionToJson(pi.targetCell);
    return j;
}

nlohmann::json moveRecordToJson(const MoveRecord& mr)
{
    nlohmann::json j;
    j["minutes"]      = mr.minutes;
    j["seconds"]      = mr.seconds;
    j["milliseconds"] = mr.milliseconds;
    j["pieceType"]    = pieceTypeToString(mr.pieceType);
    j["color"]        = colorToString(mr.color);
    j["from"]         = positionToJson(mr.from);
    j["to"]           = positionToJson(mr.to);
    j["isJump"]       = mr.isJump;
    j["isCapture"]    = mr.isCapture;
    j["givesCheck"]   = mr.givesCheck;
    return j;
}

nlohmann::json optionalColorToJson(const std::optional<Color>& opt)
{
    if (opt.has_value())
        return colorToString(opt.value());
    return nullptr;
}

} // anonymous namespace


std::string JsonProtocol::serializeSnapshot(const GameSnapshot& snap)
{
    nlohmann::json j;

    j["type"] = "snapshot";

    nlohmann::json& data = j["data"];

    data["boardWidth"]  = snap.boardWidth;
    data["boardHeight"] = snap.boardHeight;

    // pieces
    nlohmann::json piecesArr = nlohmann::json::array();
    for (const auto& pi : snap.pieces)
        piecesArr.push_back(pieceInfoToJson(pi));
    data["pieces"] = piecesArr;

    data["selectedCell"] = optionalPositionToJson(snap.selectedCell);
    data["gameOver"]     = snap.gameOver;
    data["winner"]       = optionalColorToJson(snap.winner);
    data["whiteScore"]   = snap.whiteScore;
    data["blackScore"]   = snap.blackScore;

    // move history
    nlohmann::json wmArr = nlohmann::json::array();
    for (const auto& mr : snap.whiteMoves)
        wmArr.push_back(moveRecordToJson(mr));
    data["whiteMoves"] = wmArr;

    nlohmann::json bmArr = nlohmann::json::array();
    for (const auto& mr : snap.blackMoves)
        bmArr.push_back(moveRecordToJson(mr));
    data["blackMoves"] = bmArr;

    return j.dump();
}
