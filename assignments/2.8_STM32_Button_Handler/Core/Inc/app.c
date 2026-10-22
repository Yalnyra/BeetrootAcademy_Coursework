#include "app.h"
#include "main.h"  /* HAL + LED_Pin / LED_GPIO_Port labels from the .ioc */
#include "usbd_cdc_if.h"   /* CDC_Transmit_FS */
#include <stdio.h>         /* snprintf */
/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
/*UART buffer variables */
char buf[64];
int value = 0; // Input char
static volatile uint8_t button_flag = 0;
/* USER CODE END PV */

int serial_print(char *ptr, int len){
    CDC_Transmit_FS((uint8_t*)ptr, len);
    HAL_Delay(1);
    return len;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) /* overrides HAL __weak */
{
    static uint32_t last_interrupt_time = 0;

    uint32_t current_time = HAL_GetTick();


    if (GPIO_Pin == BTN_Pin)
    {

        if (current_time - last_interrupt_time > DEBOUNCE_MS) /* 50 ms debounce */
        {
            button_flag = 1;
            last_interrupt_time = current_time;
        }
    }
}


void setup(){
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);  /* PC13 is active-LOW: SET = off */
}

void loop(){
/* USER CODE BEGIN 3 */


  	  if (button_flag == 1)
  	       {
  	           button_flag = 0;

  	           int len = snprintf(buf, sizeof(buf), "Button pushed.\r\n");

  	           serial_print(buf, len);

  	           HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
  	       }

  	       // int len = snprintf(buf, sizeof(buf), "Counter = %d\r\n", value);
  	       // serial_print(buf, len);
  	       // value++;
  	       HAL_Delay(500);
/* USER CODE END 3 */
    }
  