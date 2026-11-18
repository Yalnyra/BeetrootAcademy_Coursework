idf_component_register(

    SRCS "main_5_12hr_menu_lvgl.c"
    # SRCS "main_4_lvgl.c"
    # SRCS "main_3_rtc.c"
    # SRCS "main_hw.c"
    # SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES esp_driver_gpio esp_driver_i2c esp_timer log freertos encoder
)
