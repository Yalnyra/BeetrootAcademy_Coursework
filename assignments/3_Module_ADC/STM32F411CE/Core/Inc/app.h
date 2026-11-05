/** @author Yevhenii Vinokur @ 2026
* @brief Button press handler Initialization code emulating Arduino loop 
 */
#ifndef APP_H          /* захист від повторного імпорту */
#define APP_H
#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h> /* Add type sizes to declarations */
#define DEBOUNCE_MS 50U /* час дебаунсингу під час переривань */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */


int serial_print(char *ptr, int len); /* вивід буферу у usb_otg */
void setup(void);      /* запуск один раз після HAL_Init + MX_*_Init */
void loop(void);       /* вічно крутиться у циклі while(1)     */
/*  */
#ifdef __cplusplus
}
#endif

#endif /* APP_H */