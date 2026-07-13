#include "BoardParser.hpp"
#include "GameEngine.hpp"
#include "GameController.hpp"
#include "TextTestRunner.hpp"

#include <iostream>
#include <utility>


int main()
{
    auto result = BoardParser::parse(std::cin);

    if (result.hasError)
    {
        std::cout << result.error << '\n';
        return 1;
    }

    if (!result.board.has_value())
    {
        return 1;
    }


    GameEngine engine(
        std::move(*result.board)
    );


    GameController controller(engine);


    TextTestRunner runner(controller);

    runner.run(std::cin);


    return 0;
}