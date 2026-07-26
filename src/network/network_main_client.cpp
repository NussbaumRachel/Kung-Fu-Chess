#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include "../../demo/ChessRenderer.hpp"
#include "../../demo/DemoConfig.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <queue>
#include <optional>


// ── Shared state between network thread and render thread ──
struct SharedState
{
    std::mutex mtx;
    std::queue<std::string> incomingMessages;
    bool connected = false;
    bool running   = true;
    
    // Outgoing direction (drained by network thread)
    std::mutex outgoingMtx;
    std::queue<std::string> outgoingMessages;
};
void networkThread(SharedState& state, const std::string& host, const std::string& port)
{
    try
    {
        boost::asio::io_context ioCtx;
        boost::asio::ip::tcp::resolver resolver(ioCtx);
        auto results = resolver.resolve(host, port);

        using WSStream = boost::beast::websocket::stream<boost::asio::ip::tcp::socket>;
        auto ws = std::make_shared<WSStream>(ioCtx);

        boost::asio::connect(ws->next_layer(), results.begin(), results.end());
        ws->handshake(host, "/");
        std::cout << "Connected to server" << std::endl;

        {
            std::lock_guard<std::mutex> lk(state.mtx);
            state.connected = true;
        }

        // ── Async read loop ──
        auto doRead = std::make_shared<std::function<void()>>();
        *doRead = [ws, &state, doRead]()
        {
            auto buf = std::make_shared<boost::beast::flat_buffer>();
            ws->async_read(*buf, [ws, buf, &state, doRead](boost::system::error_code ec, std::size_t)
            {
                if (ec == boost::beast::websocket::error::closed ||
                    ec == boost::asio::error::eof)
                {
                    std::cout << "Server closed connection" << std::endl;
                    std::lock_guard<std::mutex> lk(state.mtx);
                    state.running = false;
                    return;
                }
                if (ec)
                {
                    std::cerr << "Read error: " << ec.message() << std::endl;
                    std::lock_guard<std::mutex> lk(state.mtx);
                    state.running = false;
                    return;
                }

                std::string msg = boost::beast::buffers_to_string(buf->data());
                {
                    std::lock_guard<std::mutex> lk(state.mtx);
                    state.incomingMessages.push(std::move(msg));
                }

                (*doRead)();  // continue reading
            });
        };

        // ── Async drain for outgoing messages ──
        // auto timer = std::make_shared<boost::asio::steady_timer>(ioCtx);
        // auto doDrain = std::make_shared<std::function<void()>>();
        // *doDrain = [ws, &state, timer, doDrain]()
        // {
        //     // Drain all outgoing messages
        //     while (true)
        //     {
        //         std::string msg;
        //         {
        //             std::lock_guard<std::mutex> lk(state.outgoingMtx);
        //             if (state.outgoingMessages.empty())
        //                 break;
        //             msg = std::move(state.outgoingMessages.front());
        //             state.outgoingMessages.pop();
        //         }
        //         std::cout << "Sending: " << msg << std::endl;
        //         ws->write(boost::asio::buffer(msg));
        //         std::cout << "Sent: " << msg << std::endl;
        //     }

        //     // Schedule next drain
        //     timer->expires_after(std::chrono::milliseconds(5));
        //     timer->async_wait([doDrain](boost::system::error_code)
        //     {
        //         if (doDrain)
        //             (*doDrain)();
        //     });
        // };
        // ── Async drain for outgoing messages ──
        auto timer = std::make_shared<boost::asio::steady_timer>(ioCtx);
        auto doDrain = std::make_shared<std::function<void()>>();
        *doDrain = [ws, &state, timer, doDrain]()
        {
            // Use shared_ptr<string> so the buffer stays alive for async_write
            std::shared_ptr<std::string> msgPtr;
            {
                std::lock_guard<std::mutex> lk(state.outgoingMtx);
                if (state.outgoingMessages.empty())
                {
                    // Nothing to send — schedule next drain check
                    timer->expires_after(std::chrono::milliseconds(5));
                    timer->async_wait([doDrain](boost::system::error_code) {
                        if (doDrain) (*doDrain)();
                    });
                    return;
                }
                msgPtr = std::make_shared<std::string>(
                    std::move(state.outgoingMessages.front()));
                state.outgoingMessages.pop();
            }

            std::cout << "Sending: " << *msgPtr << std::endl;

            // Write asynchronously — msgPtr captured keeps string data alive
            ws->async_write(boost::asio::buffer(*msgPtr),
                [ws, &state, timer, doDrain, msgPtr](boost::system::error_code ec, std::size_t)
                {
                    if (ec)
                    {
                        std::cerr << "Write error: " << ec.message() << std::endl;
                        std::lock_guard<std::mutex> lk(state.mtx);
                        state.running = false;
                        return;
                    }
                    std::cout << "Sent async" << std::endl;

                    // After write completes — continue draining
                    timer->expires_after(std::chrono::milliseconds(5));
                    timer->async_wait([doDrain](boost::system::error_code) {
                        if (doDrain) (*doDrain)();
                    });
                });
        };
        // Start both loops
        (*doRead)();
        (*doDrain)();

        // Run the event loop
        ioCtx.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Network error: " << e.what() << std::endl;
        std::lock_guard<std::mutex> lk(state.mtx);
        state.running = false;
    }
}

int main(int argc, char* argv[])
{
    namespace fs = std::filesystem;

    // Assets path
    std::string assetsPath = "assets";
    if (argc >= 2)
        assetsPath = argv[1];
    if (!fs::exists(assetsPath))
    {
        std::string altPath =
            fs::path(__FILE__).parent_path().parent_path().parent_path().string()
            + "/demo/assets";
        if (fs::exists(altPath))
            assetsPath = altPath;
    }
    std::cout << "Assets path: " << assetsPath << std::endl;

    // Initialize renderer
    ChessRenderer renderer;
    if (!renderer.initialize(assetsPath))
    {
        std::cerr << "Failed to initialize renderer." << std::endl;
        return 1;
    }

    // Shared state
    SharedState shared;

    // Start network thread
    std::thread net([&shared]() {
        networkThread(shared, "127.0.0.1", "8080");
    });

    // Wait for connection
    while (!shared.connected && shared.running)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    if (!shared.running)
    {
        net.join();
        std::cerr << "Failed to connect" << std::endl;
        return 1;
    }

    // ── Mouse callback: send clicks over WebSocket ──
    // We use a shared_ptr<boost::beast::websocket::stream<...>> so the
    // callback can access it.  Actually simpler: post clicks to a queue
    // that the network thread sends.
    renderer.setClickCallback([&shared](int row, int col) {
    nlohmann::json clickMsg;
    clickMsg["type"] = "click";
    clickMsg["row"]  = row;
    clickMsg["col"]  = col;
    {
        std::lock_guard<std::mutex> lk(shared.outgoingMtx);
        std::cout << "CLICK: (" << row << "," << col << ")" << std::endl;
        shared.outgoingMessages.push(clickMsg.dump());
    }
    });


    renderer.attachMouse();

    // Setup AnimationManager hook
    // The click callback can't access renderer directly in the same
    // closure (would be circular). Instead, after each snapshot render,
    // we check for new animations.

    // Store last snapshot for animation detection
    std::optional<GameSnapshot> lastSnapshot;

    // Main loop
    while (shared.running)
    {
        // Drain queue but keep only the LATEST snapshot message
        // This prevents the client from falling behind when processing
        // is slower than the server's 16ms tick rate.
        std::string latestSnapshotMsg;
        {
            std::lock_guard<std::mutex> lk(shared.mtx);
            while (!shared.incomingMessages.empty())
            {
                std::string msg = std::move(shared.incomingMessages.front());
                shared.incomingMessages.pop();

                // Quick check: is this a snapshot? (cheap, no full parse)
                // We just check if the string contains "snapshot" — good enough
                if (msg.find("\"snapshot\"") != std::string::npos)
                    latestSnapshotMsg = std::move(msg);  // keep only latest
                // Non-snapshot messages (e.g. welcome) are processed inline
                else
                {
                    try {
                        auto j = nlohmann::json::parse(msg);
                        std::string type = j.value("type", "");
                        if (type == "welcome")
                            std::cout << "Assigned color: " << j.value("color", "") << std::endl;
                    } catch (...) {}
                }
            }
        }

        // Process the latest snapshot (if any) OUTSIDE the lock
        if (!latestSnapshotMsg.empty())
        {
            try
            {
                auto j = nlohmann::json::parse(latestSnapshotMsg);
                std::string type = j.value("type", "");

                if (type == "snapshot" && j.contains("data"))
                {
                    auto& data = j["data"];

                    GameSnapshot snap;
                    snap.boardWidth  = data.value("boardWidth", 0);
                    snap.boardHeight = data.value("boardHeight", 0);
                    snap.whiteScore  = data.value("whiteScore", 0);
                    snap.blackScore  = data.value("blackScore", 0);
                    snap.gameOver    = data.value("gameOver", false);

                    // winner
                    if (data.contains("winner") && !data["winner"].is_null())
                    {
                        std::string w = data["winner"].get<std::string>();
                        snap.winner = (w == "White") ? Color::White : Color::Black;
                    }

                    // selectedCell
                    if (data.contains("selectedCell") && !data["selectedCell"].is_null())
                    {
                        auto& sc = data["selectedCell"];
                        snap.selectedCell = Position{sc["row"], sc["col"]};
                    }

                    // pieces
                    if (data.contains("pieces"))
                    {
                        for (const auto& pj : data["pieces"])
                        {
                            PieceInfo pi;
                            pi.pieceId  = pj.value("pieceId", 0);
                            pi.progress = pj.value("progress", 0.0);
                            pi.cell     = Position{
                                pj["cell"]["row"],
                                pj["cell"]["col"]
                            };

                            if (pj.contains("targetCell") && !pj["targetCell"].is_null())
                            {
                                pi.targetCell = Position{
                                    pj["targetCell"]["row"],
                                    pj["targetCell"]["col"]
                                };
                            }

                            std::string kindStr  = pj.value("kind", "");
                            std::string colorStr = pj.value("color", "");
                            std::string stateStr = pj.value("state", "");

                            if (kindStr == "King")   pi.kind = PieceType::King;
                            else if (kindStr == "Queen")  pi.kind = PieceType::Queen;
                            else if (kindStr == "Rook")   pi.kind = PieceType::Rook;
                            else if (kindStr == "Bishop") pi.kind = PieceType::Bishop;
                            else if (kindStr == "Knight") pi.kind = PieceType::Knight;
                            else                          pi.kind = PieceType::Pawn;

                            pi.color = (colorStr == "White") ? Color::White : Color::Black;

                            if      (stateStr == "Idle")       pi.state = PieceState::Idle;
                            else if (stateStr == "Moving")     pi.state = PieceState::Moving;
                            else if (stateStr == "Jumping")    pi.state = PieceState::Jumping;
                            else if (stateStr == "long_rest")  pi.state = PieceState::long_rest;
                            else if (stateStr == "Short_rest") pi.state = PieceState::Short_rest;
                            else                               pi.state = PieceState::Idle;

                            snap.pieces.push_back(pi);
                        }
                    }

                    // Animation detection
                    if (lastSnapshot.has_value())
                    {
                        const auto& oldPieces = lastSnapshot->pieces;
                        for (const auto& p : snap.pieces)
                        {
                            if (p.state == PieceState::Moving && p.progress == 0.0)
                            {
                                bool found = false;
                                for (const auto& op : oldPieces)
                                {
                                    if (op.pieceId == p.pieceId &&
                                        op.state != PieceState::Moving)
                                    {
                                        found = true;
                                        break;
                                    }
                                }
                                if (found)
                                    renderer.getAnimMgr().reset(p.pieceId);
                            }
                            else if (p.state == PieceState::Jumping && p.progress == 0.0)
                            {
                                bool found = false;
                                for (const auto& op : oldPieces)
                                {
                                    if (op.pieceId == p.pieceId &&
                                        op.state != PieceState::Jumping)
                                    {
                                        found = true;
                                        break;
                                    }
                                }
                                if (found)
                                    renderer.getAnimMgr().reset(p.pieceId);
                            }
                        }
                    }
                    lastSnapshot = snap;

                    renderer.render(snap);
                    renderer.display();
                }
            }
            catch (const std::exception& e)
            {
                std::cerr << "Snapshot parse error: " << e.what() << std::endl;
            }
        }

        // Check window
        if (!renderer.isWindowOpen())
        {
            shared.running = false;
            break;
        }

        // Check key
        int key = Img::wait_key(16);
        // std::cerr << "[DIAG] wait_key returned: " << key << std::endl;
        if (key == 27) // ESC
        {
            shared.running = false;
            break;
        }

        // wait_key(16) above already provides the delay
    }

    // Cleanup
    Img::destroy_all_windows();
    shared.running = false;
    net.join();

    return 0;
}
