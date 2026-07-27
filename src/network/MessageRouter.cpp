#include "network/MessageRouter.hpp"

#include "network/ClientSession.hpp"
#include "network/JsonProtocol.hpp"
#include "network/Messages.hpp"
#include "network/PlayerRole.hpp"
#include "network/SessionManager.hpp"

#include "controllerClick/GameController.hpp"
#include "game_engine/GameSnapshot.hpp"

#include <exception>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <utility>

MessageRouter::MessageRouter(
    GameController& controller,
    LoginHandler loginHandler
)
    : controller_(controller),
      loginHandler_(std::move(loginHandler))
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
            case MessageType::Login:
            {
                handleLogin(
                    session,
                    message
                );

                break;
            }

            case MessageType::Click:
            {
                handleClick(
                    session,
                    message
                );

                break;
            }

            case MessageType::Welcome:
            case MessageType::LoginResult:
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
            << "Message processing error: "
            << exception.what()
            << std::endl;

        sendError(
            session,
            exception.what()
        );
    }
}

void MessageRouter::handleLogin(
    const std::shared_ptr<ClientSession>& session,
    const std::string& message)
{
    const LoginMessage login =
        JsonProtocol::deserializeLogin(
            message
        );

    if (!loginHandler_)
    {
        sendError(
            session,
            "Login service is unavailable"
        );

        return;
    }

    const LoginAttemptResult result =
        loginHandler_(
            session,
            login.username
        );

    const LoginResultMessage response{
        result.success,
        result.success
            ? login.username
            : "",
        result.message
    };

    session->send(
        JsonProtocol::serializeLoginResult(
            response
        )
    );
}

void MessageRouter::handleClick(
    const std::shared_ptr<ClientSession>& session,
    const std::string& message)
{
    if (!session->isAuthenticated())
    {
        sendError(
            session,
            "Login is required before playing"
        );

        return;
    }

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

    const GameSnapshot snapshotBefore =
        controller_.getSnapshot();

    clearExpiredSelectionOwner(
        snapshotBefore
    );

    std::string rejectionReason;

    if (
        !authorizeClick(
            session,
            click.row,
            click.col,
            snapshotBefore,
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

    const GameSnapshot snapshotAfter =
        controller_.getSnapshot();

    updateSelectionOwner(
        session,
        snapshotBefore,
        snapshotAfter
    );
}

bool MessageRouter::authorizeClick(
    const std::shared_ptr<ClientSession>& session,
    int row,
    int col,
    const GameSnapshot& snapshot,
    std::string& rejectionReason)
{
    const std::optional<Color> playerColor =
        playerRoleToColor(
            session->role()
        );

    if (!playerColor.has_value())
    {
        rejectionReason =
            "Spectators cannot perform game actions";

        return false;
    }

    if (!snapshot.selectedCell.has_value())
    {
        const PieceInfo* clickedPiece =
            findPieceAt(
                snapshot,
                row,
                col
            );

        if (clickedPiece == nullptr)
        {
            rejectionReason =
                "Select one of your own pieces first";

            return false;
        }

        if (
            clickedPiece->color !=
            playerColor.value()
        )
        {
            rejectionReason =
                "You cannot select an opponent's piece";

            return false;
        }

        return true;
    }

    const std::shared_ptr<ClientSession> owner =
        selectionOwner_.lock();

    if (!owner)
    {
        rejectionReason =
            "The current selection has no valid owner";

        return false;
    }

    if (owner.get() != session.get())
    {
        rejectionReason =
            "Another player is currently selecting a move";

        return false;
    }

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

    if (
        selectedPiece->color !=
        playerColor.value()
    )
    {
        rejectionReason =
            "The selected piece does not belong to you";

        return false;
    }

    return true;
}

const PieceInfo* MessageRouter::findPieceAt(
    const GameSnapshot& snapshot,
    int row,
    int col) const
{
    for (const PieceInfo& piece :
         snapshot.pieces)
    {
        if (
            piece.state !=
                PieceState::Captured &&
            piece.cell.row == row &&
            piece.cell.col == col
        )
        {
            return &piece;
        }
    }

    return nullptr;
}

void MessageRouter::updateSelectionOwner(
    const std::shared_ptr<ClientSession>& session,
    const GameSnapshot& snapshotBefore,
    const GameSnapshot& snapshotAfter)
{
    const bool hadSelection =
        snapshotBefore.selectedCell.has_value();

    const bool hasSelection =
        snapshotAfter.selectedCell.has_value();

    if (!hadSelection && hasSelection)
    {
        selectionOwner_ = session;
        return;
    }

    if (hadSelection && !hasSelection)
    {
        selectionOwner_.reset();
        return;
    }

    if (hasSelection)
    {
        if (selectionOwner_.expired())
            selectionOwner_ = session;

        return;
    }

    selectionOwner_.reset();
}

void MessageRouter::clearExpiredSelectionOwner(
    const GameSnapshot& snapshot)
{
    if (!snapshot.selectedCell.has_value())
    {
        selectionOwner_.reset();
    }
}

void MessageRouter::sendError(
    const std::shared_ptr<ClientSession>& session,
    const std::string& errorMessage)
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