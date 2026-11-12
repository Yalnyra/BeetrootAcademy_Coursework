#idf_component_register(SRCS "lesson_3.4/main_3.1_passive_buzzer.c"
#                     PRIV_REQUIRES esp_driver_gpio esp_timer)
                       
# # TODO Split into CMakelist for every folder 
# idf_component_register(SRCS "lesson_3.1/main_2_potentiotemer_adc_calbiration.c"
#                        PRIV_REQUIRES esp_driver_gpio esp_timer esp_adc)
# idf_component_register(SRCS "lesson_3.5/main_3.5_2_pot_to_motor_map.c"
#                           PRIV_REQUIRES esp_driver_gpio esp_driver_ledc esp_adc)

# idf_component_register(SRCS "main.c"   
#                     INCLUDE_DIRS "."
#                     REQUIRES esp_adc esp_driver_gpio esp_timer esp_driver_ledc log freertos)

# idf_component_register(SRCS "lesson_3.6/main_4_pcnt_front.c"
#                           REQUIRES esp_adc esp_driver_gpio esp_timer esp_driver_ledc log esp_driver_pcnt freertos)


idf_component_register(SRCS "miniproject_3/src/main.c"
                          REQUIRES esp_adc esp_driver_gpio esp_timer esp_driver_ledc log esp_driver_pcnt freertos)

# Miniproject 3 encoder decoding mode (compile-time flag ENCODER_QUADRATURE_X4):
#   idf.py -DENCODER_X4=ON  build   -> X4 quadrature (2 PCNT channels, 4 counts per click)
#   idf.py -DENCODER_X4=OFF build   -> X1 (lesson 3.6, default)
# The value is stored in the CMake cache, so it stays until you set it again.
option(ENCODER_X4 "Miniproject 3: X4 quadrature encoder decoding instead of X1" OFF)
if(ENCODER_X4)
    target_compile_definitions(${COMPONENT_LIB} PRIVATE ENCODER_QUADRATURE_X4=1)
endif()
