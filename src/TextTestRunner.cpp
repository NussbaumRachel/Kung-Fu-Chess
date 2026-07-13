#include "TextTestRunner.hpp"
#include <sstream>
#include <string>


TextTestRunner::TextTestRunner(GameController& controller)
    : controller_(controller)
{
}

void TextTestRunner::run(std::istream& input)
{
    std::string line;

    while (std::getline(input, line))
    {
        std::istringstream iss(line);

        std::string command;
        iss >> command;


        if (command == "CLICK" || command == "click")
        {
            int x;
            int y;

            iss >> x >> y;

            controller_.handleClick(x, y);
        }
        else if (command == "JUMP" || command == "jump")
        {
            int x;
            int y;

            iss >> x >> y;

            controller_.handleJump(x, y);
        }
        else if (command == "WAIT" || command == "wait")
        {
            int ms;

            iss >> ms;

            controller_.handleWait(ms);
        }
        else if (command == "PRINT" || command == "print")
        {
            std::string target;
            iss >> target;
            // target is ignored — "print" or "print board" both print the board
            controller_.handlePrint(std::cout);
        }
    }
}

// void TextTestRunner::handleClick(std::istringstream& command)
// {
//     int x;
//     int y;

//     command >> x >> y;

//     controller_.handleClick(x, y);
// }


// void TextTestRunner::handleJump(std::istringstream& command)
// {
//     int x;
//     int y;

//     command >> x >> y;

//     controller_.handleJump(x, y);
// }


// void TextTestRunner::handleWait(std::istringstream& command)
// {
//     int milliseconds;

//     command >> milliseconds;

//     controller_.handleWait(milliseconds);
// }


// void TextTestRunner::handlePrint(std::istringstream& command)
// {
//     std::string target;

//     command >> target;

//     if (target == "board")
//     {
//         controller_.getBoard().print();
//     }
// }


// void TextTestRunner::normalizeLine(std::string& line)
// {
//     if (!line.empty() && line.back() == '\r')
//         line.pop_back();
// }