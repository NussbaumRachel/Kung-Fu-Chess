#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio.hpp>

#include <iostream>
#include <string>


int main()
{
    try
    {
        boost::asio::io_context ioContext;


        boost::asio::ip::tcp::resolver resolver(ioContext);

        auto results =
            resolver.resolve(
                "127.0.0.1",
                "8080"
            );


        boost::beast::websocket::stream<
            boost::asio::ip::tcp::socket
        > ws(ioContext);


        boost::asio::connect(
            ws.next_layer(),
            results.begin(),
            results.end()
        );


        ws.handshake(
            "127.0.0.1",
            "/"
        );


        std::cout
            << "Connected to server"
            << std::endl;


        std::string message;


        while (true)
        {
            std::cout
                << "> ";

            std::getline(
                std::cin,
                message
            );


            if (message == "exit")
                break;


            ws.write(
                boost::asio::buffer(message)
            );
        }
    }
    catch (const std::exception& e)
    {
        std::cerr
            << "Error: "
            << e.what()
            << std::endl;

        return 1;
    }


    return 0;
}