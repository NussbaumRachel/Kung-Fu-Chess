#include "img.hpp"
#include <iostream>

int main() {
    try {
        std::cout << "Testing Img class..." << std::endl;
        
        Img img;
        img.read(R"(C:\Users\Yehudit\Downloads\code\KT_MoodleUtils\KungFu Chess\sprite_utils\Pieces\WQ.png)", {640, 480}, true);
        img.put_text("Hello, Img!", 150, 360, 1.0, {0,0,0});
        img.show();
        
        std::cout << "Img class test completed successfully!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
} 