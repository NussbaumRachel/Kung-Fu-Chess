#include "GameServer.hpp"

#include <iostream>


int main()
{
    try
    {
        GameServer server(8080);

        server.run();
    }
    catch (const std::exception& e)
    {
        std::cerr
            << e.what()
            << std::endl;
    }


    return 0;
}