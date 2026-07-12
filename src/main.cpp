#include "Board.hpp"
#include "BoardParser.hpp"
#include "GameEngine.hpp"
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

    if (!result.board.has_value() || result.board->isEmpty())
        return 0;

    GameEngine engine(std::move(*result.board));

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
            engine.handleCellClick(y / 100, x / 100);
        }
        else if (cmd == "jump")
        {
            int x, y;
            iss >> x >> y;
            // jump פקודת: קליק כפול — בחר וקפוץ
            engine.handleCellClick(y / 100, x / 100);
            engine.handleCellClick(y / 100, x / 100);
        }
        else if (cmd == "print")
        {
            std::string target;
            iss >> target;
            if (target == "board")
                engine.getBoard().print();
        }
        else if (cmd == "wait")
        {
            int ms;
            iss >> ms;
            engine.advanceTime(ms);
        }
    }

    return 0;
}
