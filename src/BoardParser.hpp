#ifndef BOARD_PARSER_H
#define BOARD_PARSER_H

#include "Board.hpp"
#include <memory>
#include <string>
#include <iostream>

// אחראי על פרסור לוח מקלט טקסטואלי
// קורא מ-istream, מזהה את אזור ה-"Board:", ומפיק אובייקט Board
class BoardParser
{
public:
    struct ParseResult
    {
        std::unique_ptr<Board> board;
        std::string error;
        bool hasError;

        ParseResult() : board(nullptr), error(""), hasError(false) {}
    };

    // פרסור מתוך istream כללי (למשל std::cin, קובץ, stringstream)
    static ParseResult parse(std::istream& input);

private:
    // פיצול שורה למילים
    static std::vector<std::string> split(const std::string& str);
};

#endif
