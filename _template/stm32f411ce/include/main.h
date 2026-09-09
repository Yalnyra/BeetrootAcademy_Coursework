#ifndef MAIN_H
#define MAIN_H

#include "stm32f4xx_hal.h"

/* On-board user LED of the STM32F411CEU6 "Black Pill": PC13, active LOW. */
#define LED_PIN               GPIO_PIN_13
#define LED_GPIO_PORT         GPIOC
#define LED_GPIO_CLK_ENABLE() __HAL_RCC_GPIOC_CLK_ENABLE()

void SystemClock_Config(void);
void Error_Handler(void);

#endif /* MAIN_H */
