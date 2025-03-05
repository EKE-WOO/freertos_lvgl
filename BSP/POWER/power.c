#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "main.h"
#include "gpio.h"
#include "power.h"

// 打开电源
void PowerOn(void) {
    HAL_GPIO_WritePin(POWER_GPIO_Port, POWER_Pin, GPIO_PIN_SET); // 输出高电平，打开电源
}

// 关闭电源
void PowerOff(void) {
    HAL_GPIO_WritePin(POWER_GPIO_Port, POWER_Pin, GPIO_PIN_RESET); // 输出低电平，关闭电源
}

// 读取KRY
uint8_t ReadKey(void) {
    return  HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin); // 读取KEY的实际电平来控制
}
