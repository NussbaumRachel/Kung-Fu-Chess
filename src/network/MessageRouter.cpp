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
            << "Message processing error: "
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
    std::string& rejectionReason
)
{
    if (!session)
    {
        rejectionReason =
            "Invalid client session";

        return false;
    }

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

    /*
     * אין כלי נבחר:
     * הלחיצה יכולה לבחור רק כלי של השחקן.
     */
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

    /*
     * יש כלי נבחר:
     * רק ה-session שיצר את הבחירה רשאי להמשיך.
     */
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

    /*
     * בדיקת הגנה נוספת:
     * גם הכלי הנבחר עצמו חייב להיות בצבע השחקן.
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

    if (
        selectedPiece->color !=
        playerColor.value()
    )
    {
        rejectionReason =
            "The selected piece does not belong to you";

        return false;
    }

    /*
     * מכאן הלחיצה יכולה להיות:
     *
     * - יעד ריק
     * - כלי יריב לצורך לכידה
     * - הכלי הנבחר עצמו לצורך Jump
     * - כלי נוסף של אותו שחקן לצורך SwitchPiece
     *
     * החוקיות עצמה נשארת באחריות מנוע המשחק.
     */
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

void MessageRouter::updateSelectionOwner(
    const std::shared_ptr<ClientSession>& session,
    const GameSnapshot& snapshotBefore,
    const GameSnapshot& snapshotAfter
)
{
    const bool hadSelection =
        snapshotBefore.selectedCell.has_value();

    const bool hasSelection =
        snapshotAfter.selectedCell.has_value();

    /*
     * נוצרה בחירה חדשה.
     */
    if (!hadSelection && hasSelection)
    {
        selectionOwner_ = session;
        return;
    }

    /*
     * הבחירה בוטלה או שהמהלך התחיל.
     */
    if (hadSelection && !hasSelection)
    {
        selectionOwner_.reset();
        return;
    }

    /*
     * עדיין קיימת בחירה.
     *
     * זה יכול להיות:
     * - בחירה שלא השתנתה
     * - SwitchPiece
     *
     * הבעלות נשארת של אותו session.
     */
    if (hasSelection)
    {
        const std::shared_ptr<ClientSession> owner =
            selectionOwner_.lock();

        if (!owner)
            selectionOwner_ = session;

        return;
    }

    selectionOwner_.reset();
}

void MessageRouter::clearExpiredSelectionOwner(
    const GameSnapshot& snapshot
)
{
    if (!snapshot.selectedCell.has_value())
    {
        selectionOwner_.reset();
        return;
    }

    /*
     * אם ה-session נותק וה-weak_ptr פג,
     * לא משייכים אוטומטית את הבחירה לשחקן אחר.
     */
    if (selectionOwner_.expired())
    {
        std::cerr
            << "Selected piece belongs to a disconnected session"
            << std::endl;
    }
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