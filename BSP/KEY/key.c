#include "key.h"
#include "delay.h"
//#include "user_TasksInit.h"
#include "cmsis_os.h"
#include "queue.h"
#include <stdio.h>
#include "power.h"
#include "bt.h"

void Key_Port_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = KEY1_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(KEY1_PORT, &GPIO_InitStruct);


	
  /* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}

uint8_t KeyScan(uint8_t mode)
{
	static uint8_t key_up = 1;
	uint8_t keyvalue=0;
	if(mode) key_up = 1;
	if( key_up && (!KEY1))
	{
		//delay_ms(3);//ensure the key is down
		osDelay(3);
		if(!KEY1) keyvalue = 1;
		if(keyvalue) key_up = 0;
	}
	else
	{
		//delay_ms(3);//ensure the key is up
		osDelay(3);
		if(KEY1)
			key_up = 1;
	}
	return keyvalue;
}

// ��������
#define KEY_PRESSED_STATE  GPIO_PIN_RESET  // �͵�ƽ��ʾ����

//�ⲿ��������
extern QueueHandle_t xKeyQueue;

// ʱ�����
#define DEBOUNCE_TIME      20    // ����ʱ��20ms
#define SHORT_PRESS_TIME   100   // �̰�ʱ��500ms
#define LONG_PRESS_TIME    2000  // ����ʱ��2000ms
uint32_t testi = 0;
void KeyScanTask(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    uint32_t keyPressTime = 0;
    uint8_t keyState = GPIO_PIN_SET;

    for(;;) {
        uint8_t currentState = HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin);
        
        // ��ⰴ�����£�����͵�ƽ��Ч��
        if(currentState == GPIO_PIN_RESET && keyState == GPIO_PIN_SET) {
            vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_TIME));  // ��������
            currentState = HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin);
            
            if(currentState == GPIO_PIN_RESET) {
                keyPressTime = xTaskGetTickCount();
                
                // ������ⰴ��״̬
                while(HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == GPIO_PIN_RESET) {
//                    vTaskDelay(pdMS_TO_TICKS(20));
                    
                    // ��ⳤ��
                    if((xTaskGetTickCount() - keyPressTime) >= pdMS_TO_TICKS(LONG_PRESS_TIME)) {
//											testi = xTaskGetTickCount() - keyPressTime;
                        xQueueSend(xKeyQueue, &(KeyEvent_t){EV_LONG_PRESS}, 0);
                        while(HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == GPIO_PIN_RESET) {
                            vTaskDelay(20);  // �ȴ������ͷ�
                        }
                        break;
                    }
                }
                
                // ���̰�
                if((xTaskGetTickCount() - keyPressTime) < pdMS_TO_TICKS(LONG_PRESS_TIME)) {
//									testi = xTaskGetTickCount() - keyPressTime;
                    if((xTaskGetTickCount() - keyPressTime) >= pdMS_TO_TICKS(SHORT_PRESS_TIME)) {
                        xQueueSend(xKeyQueue, &(KeyEvent_t){EV_SHORT_PRESS}, 0);
                    }
                }
            }
        }
        keyState = currentState;
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(10));  // 10msɨ������
    }
}
/////////////////////////////////��ת��Ļ///////////////////////////////////////////////
void ToggleScreen(void)
{
	printf("%s++\r\n", __FUNCTION__);
	static uint8_t screenState = 1;
	screenState ^= 1;
//	printf("System close lcd\r\n");
//    SetScreenState(screenState);  // ���Ʊ������ʾ
	printf("%s--\r\n", __FUNCTION__);
}
///////////////////////////////////���ػ�//////////////////////////////////////////////
void KeyEventHandlerTask(void *pvParameters)
{
    KeyEvent_t receivedEvent;
    
    for(;;) {
        if(xQueueReceive(xKeyQueue, &receivedEvent, portMAX_DELAY)) {
            switch(receivedEvent) {
                case EV_SHORT_PRESS:
                    ToggleScreen();  // ��Ļ״̬�л�����
                    break;
                    
                case EV_LONG_PRESS:
//										printf("HandlePowerManagement\r\n");
                    HandlePowerManagement();  // ��Դ������
//										printf("HandlePowerManagemented\r\n");
                    break;
                    
                default:
                    break;
            }
        }
    }
}

void HandlePowerManagement(void) {
	printf("%s++\r\n", __FUNCTION__);
	
	static uint8_t isPowerOn = 0;
  
	if(isPowerOn){
		// �ػ�����
		printf("System Shutdown Initiated\r\n");
			
			/* ��ȫ�ػ����� */
			// 1. �ر���������
//        MX_GPIO_DeInit();
//        MX_USART1_UART_DeInit();
			// �����������رմ���...
			
			// 2. ����ؼ�����
//        SaveSystemStateToFlash();
			
			// 3. �жϵ�Դ�����һ����
//        HAL_GPIO_WritePin(PWR_CTRL_GPIO_PORT, PWR_CTRL_PIN, PWR_OFF_STATE);
		PowerOff();
		BT_Power_Off();
			
			// 4. ������ѭ���ȴ��ϵ�
		isPowerOn = 0;
//        while(1) { 
//            __NOP();  // ���뱣���ڴ�ѭ��
//        }
	}else{
		PowerOn();
		BT_Power_On();
    // �������̣���Ӳ���Զ���ɣ�
		printf("System open Initiated\r\n");
    isPowerOn = 1;
//      SystemReboot();
	}
	
}