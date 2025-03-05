#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "main.h"
#include "gpio.h"
#include "bt.h"

// 蓝牙电源启动函数
void BT_Power_On(void) {
//    GPIO_SetBits(BT_PWR_GPIO_Port, BT_PWR_Pin);  // 设置PA0为高电平
	HAL_GPIO_WritePin(BT_PWR_GPIO_Port, BT_PWR_Pin, GPIO_PIN_RESET); // 输出低电平，打开电源
	  
}

// 蓝牙电源关闭函数
void BT_Power_Off(void) {
//    GPIO_ResetBits(BT_PWR_GPIO_Port, BT_PWR_Pin);  // 设置PA0为低电平
	HAL_GPIO_WritePin(BT_PWR_GPIO_Port, BT_PWR_Pin, GPIO_PIN_SET); // 输出高电平，关闭电源
}
