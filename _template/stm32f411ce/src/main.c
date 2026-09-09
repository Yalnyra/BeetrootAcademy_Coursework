/**
 * @file    main.c
 * @brief   Starting point for a Black Pill (STM32F411CEU6) assignment.
 *
 * Blinks the on-board LED on PC13 at 1 Hz. Replace the body of main() with
 * the assignment's logic; keep SystemClock_Config() and SysTick_Handler().
 */

#include "main.h"

static void LED_Init(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    LED_Init();

    while (1)
    {
        HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_PIN);
        HAL_Delay(500);
    }
}

static void LED_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    LED_GPIO_CLK_ENABLE();

    gpio.Pin   = LED_PIN;
    gpio.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio.Pull  = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_GPIO_PORT, &gpio);
}

/**
 * The Black Pill carries a 25 MHz crystal. PLL: /25 * 192 / 2 = 96 MHz SYSCLK,
 * with PLLQ = 4 giving the 48 MHz the USB peripheral needs.
 * AHB = 96 MHz, APB1 = 48 MHz, APB2 = 96 MHz, flash latency 3 WS.
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef osc = {0};
    RCC_ClkInitTypeDef clk = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    osc.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    osc.HSEState       = RCC_HSE_ON;
    osc.PLL.PLLState   = RCC_PLL_ON;
    osc.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
    osc.PLL.PLLM       = 25;
    osc.PLL.PLLN       = 192;
    osc.PLL.PLLP       = RCC_PLLP_DIV2;
    osc.PLL.PLLQ       = 4;
    if (HAL_RCC_OscConfig(&osc) != HAL_OK)
    {
        Error_Handler();
    }

    clk.ClockType      = RCC_CLOCKTYPE_HCLK   | RCC_CLOCKTYPE_SYSCLK |
                         RCC_CLOCKTYPE_PCLK1  | RCC_CLOCKTYPE_PCLK2;
    clk.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    clk.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider = RCC_HCLK_DIV2;
    clk.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_3) != HAL_OK)
    {
        Error_Handler();
    }
}

void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}

/* HAL_Delay() and the HAL timeouts depend on this tick. */
void SysTick_Handler(void)
{
    HAL_IncTick();
}
