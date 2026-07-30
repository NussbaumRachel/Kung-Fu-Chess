#include "network/Room.hpp"
#include "game_result/GameResult.hpp"
#include "network/ClientSession.hpp"
#include "network/JsonProtocol.hpp"
#include "network/PlayerRole.hpp"
#include <exception>
#include <cstddef>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <utility>

Room::Room(
    std::string id,
    boost::asio::io_context& ioContext,
    Board board,
    PieceSpeedConfig speedConfig,
    IGameResultRepository& gameResultRepository
)
    : id_(std::move(id)),
      speedConfig_(std::move(speedConfig)),
      engine_(
          std::move(board),
          speedConfig_
      ),
      controller_(engine_),
      gameLoop_(
          ioContext,
          controller_,
          [this](const std::string& message)
          {
              broadcast(message);
          },
          [this](const GameSnapshot& snapshot)
          {
              handleGameOver(snapshot);
          }
      ),
      gameResultRepository_(
          gameResultRepository
      )
{
}

void Room::start()
{
    gameLoop_.start();

    std::cout
        << "Room '"
        << id_
        << "' started"
        << std::endl;
}

void Room::stop()
{
    gameLoop_.stop();

    std::cout
        << "Room '"
        << id_
        << "' stopped"
        << std::endl;
}

void Room::addSession(
    const SessionPtr& session)
{
    if (!session)
    {
        return;
    }

    if (contains(session))
    {
        return;
    }

    const PlayerRole role =
        assignRole();

    session->setRole(role);
    sessions_.insert(session);

    const std::string roleName{
        playerRoleToString(role)
    };

    std::cout
        << "Client joined room '"
        << id_
        << "' as "
        << roleName
        << std::endl;

    const WelcomeMessage welcome{
        roleName
    };

    session->send(
        JsonProtocol::serializeWelcome(
            welcome
        )
    );

    const GameSnapshot snapshot =
        controller_.getSnapshot();

    session->send(
        JsonProtocol::serializeSnapshot(
            snapshot
        )
    );
}

void Room::removeSession(
    const SessionPtr& session)
{
    if (!session)
    {
        return;
    }

    const std::size_t removed =
        sessions_.erase(session);

    if (removed == 0)
    {
        return;
    }

    const std::shared_ptr<ClientSession>
        selectionOwner =
            selectionOwner_.lock();

    if (
        selectionOwner &&
        selectionOwner.get() == session.get()
    )
    {
        selectionOwner_.reset();
    }

    session->setRole(
        PlayerRole::Spectator
    );

    std::cout
        << "Client left room '"
        << id_
        << "'"
        << std::endl;
}

void Room::handleClick(
    const SessionPtr& session,
    const ClickMessage& click)
{
    if (!session)
    {
        return;
    }

    if (!contains(session))
    {
        sendError(
            session,
            "Client does not belong to this room"
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

bool Room::contains(
    const SessionPtr& session) const
{
    if (!session)
    {
        return false;
    }

    return sessions_.find(session) !=
           sessions_.end();
}

bool Room::empty() const
{
    return sessions_.empty();
}

std::size_t Room::sessionCount() const
{
    return sessions_.size();
}

const std::string& Room::id() const
{
    return id_;
}

void Room::broadcast(
    const std::string& message)
{
    for (const SessionPtr& session :
         sessions_)
    {
        if (session)
        {
            session->send(message);
        }
    }
}

void Room::sendError(
    const SessionPtr& session,
    const std::string& errorMessage) const
{
    if (!session)
    {
        return;
    }

    const ErrorMessage error{
        errorMessage
    };

    session->send(
        JsonProtocol::serializeError(
            error
        )
    );
}

PlayerRole Room::assignRole() const
{
    if (!isRoleOccupied(PlayerRole::White))
    {
        return PlayerRole::White;
    }

    if (!isRoleOccupied(PlayerRole::Black))
    {
        return PlayerRole::Black;
    }

    return PlayerRole::Spectator;
}

bool Room::isRoleOccupied(
    PlayerRole role) const
{
    for (const SessionPtr& session :
         sessions_)
    {
        if (
            session &&
            session->role() == role
        )
        {
            return true;
        }
    }

    return false;
}

bool Room::authorizeClick(
    const SessionPtr& session,
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

const PieceInfo* Room::findPieceAt(
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

void Room::updateSelectionOwner(
    const SessionPtr& session,
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
        {
            selectionOwner_ = session;
        }

        return;
    }

    selectionOwner_.reset();
}

void Room::clearExpiredSelectionOwner(
    const GameSnapshot& snapshot)
{
    if (!snapshot.selectedCell.has_value())
    {
        selectionOwner_.reset();
    }
}
void Room::handleGameOver(
    const GameSnapshot& snapshot)
{
    if (gameResultSaveAttempted_)
    {
        return;
    }

    gameResultSaveAttempted_ = true;

    if (!snapshot.winner.has_value())
    {
        std::cerr
            << "Game ended in room '"
            << id_
            << "' without a winner; "
            << "result was not saved"
            << std::endl;

        return;
    }

    const std::optional<UserId> whiteUserId =
        findUserIdByRole(
            PlayerRole::White
        );

    const std::optional<UserId> blackUserId =
        findUserIdByRole(
            PlayerRole::Black
        );

    if (
        !whiteUserId.has_value() ||
        !blackUserId.has_value()
    )
    {
        std::cerr
            << "Game ended in room '"
            << id_
            << "' without two authenticated "
            << "players; result was not saved"
            << std::endl;

        return;
    }

    GameWinner winner;

    if (
        snapshot.winner.value() ==
        Color::White
    )
    {
        winner = GameWinner::White;
    }
    else if (
        snapshot.winner.value() ==
        Color::Black
    )
    {
        winner = GameWinner::Black;
    }
    else
    {
        std::cerr
            << "Game ended in room '"
            << id_
            << "' with an unsupported winner; "
            << "result was not saved"
            << std::endl;

        return;
    }

    const GameResult result{
        whiteUserId.value(),
        blackUserId.value(),
        winner
    };

    try
    {
        const GameResultId resultId =
            gameResultRepository_.save(
                result
            );

        std::cout
            << "Game result "
            << resultId
            << " saved for room '"
            << id_
            << "'"
            << std::endl;
    }
    catch (const std::exception& exception)
    {
        std::cerr
            << "Failed to save game result "
            << "for room '"
            << id_
            << "': "
            << exception.what()
            << std::endl;
    }
}

std::optional<UserId>
Room::findUserIdByRole(
    PlayerRole role) const
{
    for (const SessionPtr& session :
         sessions_)
    {
        if (
            !session ||
            session->role() != role
        )
        {
            continue;
        }

        const std::optional<UserId> userId =
            session->userId();

        if (userId.has_value())
        {
            return userId;
        }
    }

    return std::nullopt;
}