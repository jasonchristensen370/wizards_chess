idf_component_register(
    # SRCS "adc_mic_test.cpp" "analog_adc_mic_test.cpp"
    # SRCS "main.cpp"
    SRCS "adc_mic_test.cpp"
    INCLUDE_DIRS "."
    PRIV_REQUIRES esp_driver_i2s
    REQUIRES esp_adc
)
