#include "BoardParser.hpp"
#include "PieceFactory.hpp"
#include "GameController.hpp"
#include <sstream>
#include <algorithm>

// מסיר רווחים מההתחלה ומהסוף


std::vector<std::string> BoardParser::split(const std::string& str)
{
    std::vector<std::string> tokens;
    std::istringstream stream(str);
    std::string token;

    while (stream >> token)
        tokens.push_back(token);

    return tokens;
}

BoardParser::ParseResult BoardParser::parse(std::istream& input)
{
    ParseResult result;

    std::vector<std::string> lines;
    std::string line;

    while (std::getline(input, line))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        if (trim(line) == "Commands:")
            break;

        lines.push_back(line);
    }

    std::vector<std::string> boardLines;
    bool foundBoard = false;

    for (const auto& l : lines)
    {
        if (trim(l) == "Board:")
        {
            foundBoard = true;
            continue;
        }
        if (trim(l) == "Commands:")
            break;
        if (foundBoard)
            boardLines.push_back(l);
    }

    if (!foundBoard || boardLines.empty())
        return result;

    while (!boardLines.empty() && boardLines.back().empty())
        boardLines.pop_back();

    if (boardLines.empty())
        return result;

    std::vector<std::string> firstRow = split(boardLines[0]);
    size_t expectedWidth = firstRow.size();

    if (expectedWidth == 0)
        return result;

    std::vector<std::vector<std::string>> stringGrid;

    for (const auto& lineStr : boardLines)
    {
        std::vector<std::string> tokens = split(lineStr);

        if (tokens.size() != expectedWidth)
        {
            result.error = "ERROR ROW_WIDTH_MISMATCH";
            result.hasError = true;
            return result;
        }

        for (const auto& token : tokens)
        {
            if (token != "." && !PieceFactory::isValidToken(token))
            {
                result.error = "ERROR UNKNOWN_TOKEN";
                result.hasError = true;
                return result;
            }
        }

        stringGrid.push_back(tokens);
    }

    result.board = Board(stringGrid);
    return result;
}
std::string BoardParser::trim(const std::string& str)
{
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
        return "";

    size_t end = str.find_last_not_of(" \t\r\n");

    return str.substr(
        start,
        end - start + 1
    );
}