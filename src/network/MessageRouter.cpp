#include "network/MessageRouter.hpp"

#include "network/ClientSession.hpp"
#include "network/JsonProtocol.hpp"
#include "network/Messages.hpp"
#include "network/PlayerRole.hpp"

#include "controllerClick/GameController.hpp"
#include "game_engine/GameSnapshot.hpp"

#include <exception>
#include <iostream>
#include <memory>
#include <optional>
#include <string>

MessageRouter::MessageRouter(
    GameController& controller
)
    : controller_(controller)
{
}

void MessageRouter::route(
    std::shared_ptr<ClientSession> session,
    const std::string& message
)
{
    if (!session)
    {
        std::cerr
            << "MessageRouter received a null session"
            << std::endl;

        return;
    }

    try
    {
        const MessageType messageType =
            JsonProtocol::getMessageType(message);

        switch (messageType)
        {
            case MessageType::Click:
            {
                handleClick(session, message);
                break;
            }

            case MessageType::Welcome:
            case MessageType::Snapshot:
            case MessageType::Error:
            {
                sendError(
                    session,
                    "Message type is not accepted from clients"
                );

                break;
            }

            case MessageType::Unknown:
            default:
            {
                sendError(
                    session,
                    "Unknown message type"
                );

                break;
            }
        }
    }
    catch (const std::exception& exception)
    {
        std::cerr
            << "Message parse error: "
            << exception.what()
            << std::endl;

        sendError(
            session,
            exception.what()
        );
    }
}

void MessageRouter::handleClick(
    const std::shared_ptr<ClientSession>& session,
    const std::string& message
)
{
    const ClickMessage click =
        JsonProtocol::deserializeClick(message);

    if (click.row < 0 || click.col < 0)
    {
        sendError(
            session,
            "Click coordinates must be non-negative"
        );

        return;
    }

    std::string rejectionReason;

    if (
        !authorizeClick(
            session,
            click.row,
            click.col,
            rejectionReason
        )
    )
    {
        sendError(
            session,
            rejectionReason
        );

        return;
    }

    controller_.handleCellClick(
        click.row,
        click.col
    );
}

bool MessageRouter::authorizeClick(
    const std::shared_ptr<ClientSession>& session,
    int row,
    int col,
    std::string& rejectionReason
) const
{
    if (!session)
    {
        rejectionReason =
            "Invalid client session";

        return false;
    }

    const PlayerRole role =
        session->role();

    const std::optional<Color> playerColor =
        playerRoleToColor(role);

    if (!playerColor.has_value())
    {
        rejectionReason =
            "Spectators cannot perform game actions";

        return false;
    }

    const GameSnapshot snapshot =
        controller_.getSnapshot();

    /*
     * No piece is currently selected:
     * this click must select one of the player's
     * own pieces.
     */
    if (!snapshot.selectedCell.has_value())
    {
        const PieceInfo* clickedPiece =
            findPieceAt(snapshot, row, col);

        if (clickedPiece == nullptr)
        {
            rejectionReason =
                "Select one of your own pieces first";

            return false;
        }

        if (clickedPiece->color != playerColor.value())
        {
            rejectionReason =
                "You cannot select an opponent's piece";

            return false;
        }

        return true;
    }

    /*
     * A piece is already selected globally.
     * Only the owner of that selected piece may
     * complete, cancel or switch the action.
     */
    const Position& selectedCell =
        snapshot.selectedCell.value();

    const PieceInfo* selectedPiece =
        findPieceAt(
            snapshot,
            selectedCell.row,
            selectedCell.col
        );

    if (selectedPiece == nullptr)
    {
        rejectionReason =
            "The selected piece no longer exists";

        return false;
    }

    if (selectedPiece->color != playerColor.value())
    {
        rejectionReason =
            "Another player's action is currently in progress";

        return false;
    }

    /*
     * When clicking another friendly piece,
     * it must also belong to this player.
     *
     * An empty cell or an opponent piece may be
     * a legal destination; GameController and the
     * game engine decide whether the move itself
     * is valid.
     */
    const PieceInfo* clickedPiece =
        findPieceAt(snapshot, row, col);

    if (
        clickedPiece != nullptr &&
        clickedPiece->color ==
            selectedPiece->color
    )
    {
        return
            clickedPiece->color ==
            playerColor.value();
    }

    return true;
}

const PieceInfo* MessageRouter::findPieceAt(
    const GameSnapshot& snapshot,
    int row,
    int col
) const
{
    for (const PieceInfo& piece : snapshot.pieces)
    {
        if (
            piece.state != PieceState::Captured &&
            piece.cell.row == row &&
            piece.cell.col == col
        )
        {
            return &piece;
        }
    }

    return nullptr;
}

void MessageRouter::sendError(
    const std::shared_ptr<ClientSession>& session,
    const std::string& errorMessage
)
{
    if (!session)
        return;

    const ErrorMessage error{
        errorMessage
    };

    session->send(
        JsonProtocol::serializeError(error)
    );
}