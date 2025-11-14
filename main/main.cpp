#include <cstdio>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "game.h"

extern "C" void app_main() {
    printf("Starting full-legality chess engine (embedded) on ESP32-S3\n");
    Chess::Game game;
    game.newGame();

    while (true) {
        game.debugPrintBoard();
        // Example: list legal moves for side to move
        std::vector<Chess::Move> moves;
        game.legalMoves(moves);
        printf("Legal moves: %zu\n", moves.size());
        // For demonstration: if any move exists, play the first one
        if (!moves.empty()) {
            game.playMoveUCI(Chess::moveToUCI(moves[0]));
        }
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

    // while (true) {
    //     printf("Hello World from Wizards Chess!\n");
    //     vTaskDelay(1000 / portTICK_PERIOD_MS); // wait 1 second
    // }
}
