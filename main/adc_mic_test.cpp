extern "C" {
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s_std.h"
#include "esp_log.h"
}

#define I2S_NUM     (I2S_NUM_0)
#define I2S_BCLK   GPIO_NUM_8
#define I2S_LRCLK  GPIO_NUM_9
#define I2S_DATA   GPIO_NUM_4

static const char *TAG = "I2S_MIC";

extern "C" void app_main(void)
{
    i2s_chan_handle_t rx_handle;
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM, I2S_ROLE_MASTER);
    i2s_new_channel(&chan_cfg, NULL, &rx_handle);


    // Standard I2S configuration
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(16000),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = I2S_BCLK,
            .ws = I2S_LRCLK,
            .dout = I2S_GPIO_UNUSED,
            .din = I2S_DATA,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false
            }
        }
    };

    ESP_ERROR_CHECK(i2s_channel_init_std_mode(rx_handle, &std_cfg));
    ESP_ERROR_CHECK(i2s_channel_enable(rx_handle));

    ESP_LOGI(TAG, "I2S Microphone started");

    int32_t sample_buffer[256];

    while (true) {
        size_t bytes_read = 0;
        ESP_ERROR_CHECK(i2s_channel_read(rx_handle, sample_buffer, sizeof(sample_buffer), &bytes_read, portMAX_DELAY));

        // Print first sample so you know it's alive:
        int32_t sample = sample_buffer[0] >> 14; // MSB align
        ESP_LOGI(TAG, "Sample: %d", sample);

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
