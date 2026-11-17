idf_component_register(
    # SRCS "main_hw.c" "main.c"
    SRCS "main_hw.c"
    INCLUDE_DIRS "."
    REQUIRES esp_driver_gpio esp_driver_i2c esp_timer log freertos
)
