#include "Board.hpp"
#include "BoardParser.hpp"
#include "Game.hpp"
#include <iostream>
#include <sstream>

int main()
{
    auto result = BoardParser::parse(std::cin);

    if (result.hasError)
    {
        std::cout << result.error << '\n';
        return 0;
    }

    if (!result.board || result.board->isEmpty())
        return 0;

    Game game(std::move(*result.board));

    std::string commandLine;
    while (std::getline(std::cin, commandLine))
    {
        if (!commandLine.empty() && commandLine.back() == '\r')
            commandLine.pop_back();

        if (commandLine.empty())
            continue;

        std::istringstream iss(commandLine);
        std::string cmd;
        iss >> cmd;

        if (cmd == "click")
        {
            int x, y;
            iss >> x >> y;
            game.click(x, y);
        }
        else if (cmd == "print")
        {
            std::string target;
            iss >> target;
            if (target == "board")
                game.printBoard();
        }
        else if (cmd == "wait")
        {
            int ms;
            iss >> ms;

            game.wait(ms);
        }
    }

    return 0;
}
