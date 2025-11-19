#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

static const char *TAG = "ADC_MIC";

// GPIO4 = ADC1_CHANNEL_3 on ESP32-S3
#define ADC_UNIT          ADC_UNIT_1
#define ADC_CHANNEL       ADC_CHANNEL_3
#define ADC_ATTEN_LEVEL   ADC_ATTEN_DB_12   // ~0–3.3V
#define ADC_BITWIDTH      ADC_BITWIDTH_12

adc_oneshot_unit_handle_t adc_handle;
adc_cali_handle_t cali_handle;
bool cali_enabled = false;

void init_adc()
{
    // ----- ADC UNIT CONFIG -----
    adc_oneshot_unit_init_cfg_t unit_cfg = {
        .unit_id = ADC_UNIT,
        .clk_src = ADC_RTC_CLK_SRC_DEFAULT,    // required field in IDF 5.3
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_cfg, &adc_handle));

    // ----- CHANNEL CONFIG -----
    adc_oneshot_chan_cfg_t channel_cfg = {
        .atten = ADC_ATTEN_LEVEL,
        .bitwidth = ADC_BITWIDTH,
    };
    ESP_ERROR_CHECK(
        adc_oneshot_config_channel(adc_handle, ADC_CHANNEL, &channel_cfg)
    );

    // ----- CALIBRATION CONFIG -----
    adc_cali_curve_fitting_config_t cali_cfg = {
        .unit_id = ADC_UNIT,
        .chan = ADC_CHANNEL,
        .atten = ADC_ATTEN_LEVEL,
        .bitwidth = ADC_BITWIDTH,
    };

    if (adc_cali_create_scheme_curve_fitting(&cali_cfg, &cali_handle) == ESP_OK)
    {
        cali_enabled = true;
        ESP_LOGI(TAG, "ADC calibration enabled");
    }
    else
    {
        ESP_LOGW(TAG, "ADC calibration not supported / not available");
    }
}

extern "C" void app_main()
{
    init_adc();

    while (true)
    {
        int raw = 0;
        ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, ADC_CHANNEL, &raw));

        if (cali_enabled) {
            int mv = 0;
            adc_cali_raw_to_voltage(cali_handle, raw, &mv);
            ESP_LOGI(TAG, "Raw: %d   Voltage: %d mV", raw, mv);
        } else {
            ESP_LOGI(TAG, "Raw: %d", raw);
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
