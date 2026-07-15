#ifndef BOARD_PARSER_H
#define BOARD_PARSER_H
#include "model/Board.hpp"
#include "controllerClick/GameController.hpp"
#include <string>
#include <iostream>
#include <optional>

// אחראי על פרסור לוח מקלט טקסטואלי
// קורא מ-istream, מזהה את אזור ה-"Board:", ומפיק אובייקט Board
class BoardParser
{
public:
// BoardParser.hpp — שינוי
    struct ParseResult
    {
        std::optional<Board> board;
        bool hasError = false;
        std::string error;
    };

    // פרסור מתוך istream כללי (למשל std::cin, קובץ, stringstream)
    static ParseResult parse(std::istream& input);

private:
    // פיצול שורה למילים
    static std::vector<std::string> split(const std::string& str);
    //הסרת רווחים
    static std::string trim(const std::string& str);

};

#endif
