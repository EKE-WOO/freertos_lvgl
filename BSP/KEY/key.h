#ifndef __KEY_H__
#define __KEY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

// 事件类型
typedef enum {
    EV_NONE,
    EV_SHORT_PRESS,
    EV_LONG_PRESS
} KeyEvent_t;
//uint8_t power_flag = 0;

//KEY1
#define KEY1_PORT	GPIOA
#define KEY1_PIN	GPIO_PIN_0
#define KEY1 HAL_GPIO_ReadPin(KEY1_PORT,KEY1_PIN)
	
void Key_Port_Init(void);
void Key_Interrupt_Callback(void);
uint8_t KeyScan(uint8_t mode);
void HandlePowerManagement(void);
void ToggleScreen(void);
void KeyScanTask(void *pvParameters);
void KeyEventHandlerTask(void *pvParameters);





#ifdef __cplusplus
}
#endif
#endif

