#idf_component_register(SRCS "lesson_3.4/main_3.1_passive_buzzer.c"
#                     PRIV_REQUIRES esp_driver_gpio esp_timer)
                       
# TODO Split into CMakelist for every folder 
# idf_component_register(SRCS "lesson_3.1/main_2_potentiotemer_adc_calbiration.c"
#                        PRIV_REQUIRES esp_driver_gpio esp_timer esp_adc)
idf_component_register(SRCS "lesson_3.3/examples_1_taras_potentiometer.c"
                          PRIV_REQUIRES esp_driver_gpio esp_driver_ledc esp_adc)
