#include <cstdio>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// app_main needs C linkage because ESP-IDF startup expects it
extern "C" void app_main() {
    
    while (true) {
        printf("Hello World from Wizards Chess!\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS); // wait 1 second
    }
}
