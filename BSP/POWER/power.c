#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "main.h"
#include "gpio.h"
#include "power.h"

// �򿪵�Դ
void PowerOn(void) {
    HAL_GPIO_WritePin(POWER_GPIO_Port, POWER_Pin, GPIO_PIN_SET); // ����ߵ�ƽ���򿪵�Դ
}

// �رյ�Դ
void PowerOff(void) {
    HAL_GPIO_WritePin(POWER_GPIO_Port, POWER_Pin, GPIO_PIN_RESET); // ����͵�ƽ���رյ�Դ
}

// ��ȡKRY
uint8_t ReadKey(void) {
    return  HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin); // ��ȡKEY��ʵ�ʵ�ƽ������
}
