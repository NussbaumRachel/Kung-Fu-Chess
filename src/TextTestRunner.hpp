#pragma once

#include "GameController.hpp"
#include <istream>

class TextTestRunner
{
public:
    explicit TextTestRunner(GameController& controller);

    void run(std::istream& input);

private:
    GameController& controller_;
};