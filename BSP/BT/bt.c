#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "main.h"
#include "gpio.h"
#include "bt.h"

// ������Դ��������
void BT_Power_On(void) {
//    GPIO_SetBits(BT_PWR_GPIO_Port, BT_PWR_Pin);  // ����PA0Ϊ�ߵ�ƽ
	HAL_GPIO_WritePin(BT_PWR_GPIO_Port, BT_PWR_Pin, GPIO_PIN_RESET); // ����͵�ƽ���򿪵�Դ
	  
}

// ������Դ�رպ���
void BT_Power_Off(void) {
//    GPIO_ResetBits(BT_PWR_GPIO_Port, BT_PWR_Pin);  // ����PA0Ϊ�͵�ƽ
	HAL_GPIO_WritePin(BT_PWR_GPIO_Port, BT_PWR_Pin, GPIO_PIN_SET); // ����ߵ�ƽ���رյ�Դ
}
