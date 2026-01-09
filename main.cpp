#include "chess.h"
#include <iostream>
#include <string>

int main() {
    try {
        std::string mode;
        std::cout << "Choose mode: [1] Player vs Player, [2] Player vs CPU: ";
        std::getline(std::cin, mode);

        if (mode == "2") {
            std::string color;
            std::cout << "Should CPU play as white or black? [w/b]: ";
            std::getline(std::cin, color);

            Color cpuColor = (color == "w" || color == "W") ? Color::WHITE : Color::BLACK;
            ChessGame game(true, cpuColor);  // Enable CPU mode
            game.startGame();
        } else {
            ChessGame game(false);  // Player vs Player
            game.startGame();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
