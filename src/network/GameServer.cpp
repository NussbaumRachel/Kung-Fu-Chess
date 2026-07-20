#include "GameServer.hpp"
#include "ClientSession.hpp"

#include <iostream>


GameServer::GameServer(uint16_t port)
    :
    ioContext_(),
    acceptor_(
        ioContext_,
        boost::asio::ip::tcp::endpoint(
            boost::asio::ip::tcp::v4(),
            port
        )
    )
{
    std::cout
        << "Server listening on port "
        << port
        << std::endl;
}


void GameServer::run()
{
    acceptNext();

    ioContext_.run();
}


void GameServer::acceptNext()
{
    acceptor_.async_accept(
        [this](boost::system::error_code ec,
               boost::asio::ip::tcp::socket socket)
        {
            if (!ec)
            {
                std::cout
                    << "New TCP connection"
                    << std::endl;


                auto session =
                    std::make_shared<ClientSession>(
                        std::move(socket),
                        *this
                    );


                session->start();
            }
            else
            {
                std::cout
                    << "Accept error: "
                    << ec.message()
                    << std::endl;
            }


            // ממשיכים להאזין לחיבורים נוספים
            acceptNext();
        }
    );
}