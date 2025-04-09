#include "key.h"
#include "delay.h"
//#include "user_TasksInit.h"
#include "cmsis_os.h"
#include "queue.h"
#include <stdio.h>
#include "power.h"
#include "bt.h"
#include "zsbm800x_drv.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "semphr.h"

// uint8_t first_open = 1;

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
extern SemaphoreHandle_t xKeySemaphore;
extern TaskHandle_t 		key_scan_handler;

extern uint8_t key_flag;
// ʱ�����
#define DEBOUNCE_TIME      20    // ����ʱ��20ms
#define SHORT_PRESS_TIME   100   // �̰�ʱ��500ms
#define LONG_PRESS_TIME    2000  // ����ʱ��2000ms
uint32_t testi = 0;
// void KeyScanTask(void *pvParameters)
// {
//     TickType_t xLastWakeTime = xTaskGetTickCount();
//     uint32_t keyPressTime = 0;
//     uint8_t keyState = GPIO_PIN_SET;

//     for(;;) {

//         // // �ȴ��ź�����������ָ��
//         // if(xSemaphoreTake(xKeySemaphore, 0) == pdTRUE) {
//         //     vTaskSuspend(NULL); // ���ҹ���
//         // }
//         // if(key_flag == 1) {
//         //     vTaskSuspend(NULL); // ���ҹ���
//         // }

//         uint8_t currentState = HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin);
//         printf("into_KeyScanTask\r\n");
 
//         // ��ⰴ�����£�����͵�ƽ��Ч��

// /////////////////////////////////use/////////////////////////////////////////////////////////////
// //         keyPressTime = xTaskGetTickCount();
        
// //         // ������ⰴ��״̬
// //         while(HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == GPIO_PIN_RESET) {
// // //                    vTaskDelay(pdMS_TO_TICKS(20));
// //             vTaskDelay(1);
// // //                     // ��ⳤ��
// //             if((xTaskGetTickCount() - keyPressTime) >= pdMS_TO_TICKS(LONG_PRESS_TIME)) {
// //                 PowerOn();
// //                 xQueueSend(xKeyQueue, &(KeyEvent_t){EV_LONG_PRESS}, 0);
// //                 while(HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == GPIO_PIN_RESET) {
// //                     vTaskDelay(20);  // �ȴ������ͷ�
// //                     // vTaskSuspend(key_scan_handler); 
// //                 }
// //                 break;
// //             }
// //         }

// //         // ��ⳤ��
// // //         if((xTaskGetTickCount() - keyPressTime) >= pdMS_TO_TICKS(LONG_PRESS_TIME)) {
// // //             PowerOn();
// // //             // first_open = 0;
// // // //											testi = xTaskGetTickCount() - keyPressTime;
// // //             xQueueSend(xKeyQueue, &(KeyEvent_t){EV_LONG_PRESS}, 0);
// // //             vTaskDelay(20);  // �ȴ������ͷ�

// // //         }
// // //         else{
// //             if(key_flag >= 3){
// //                 xQueueSend(xKeyQueue, &(KeyEvent_t){EV_SHORT_PRESS}, 0);
// //                 // key_flag = 1;
// //                 vTaskDelay(20); 
// //             }
// //             // xQueueSend(xKeyQueue, &(KeyEvent_t){EV_SHORT_PRESS}, 0);
// //             // // key_flag = 1;
// //             // vTaskDelay(20); 
// //         // }                

// //             // ���̰�
// // //                 if((xTaskGetTickCount() - keyPressTime) < pdMS_TO_TICKS(LONG_PRESS_TIME)) {
// // // //									testi = xTaskGetTickCount() - keyPressTime;
// // //                     if((xTaskGetTickCount() - keyPressTime) >= pdMS_TO_TICKS(SHORT_PRESS_TIME)) {
// // //                         xQueueSend(xKeyQueue, &(KeyEvent_t){EV_SHORT_PRESS}, 0);
// // //                         key_flag = 1;
// // //                         vTaskDelay(20); 
// // //                         // vTaskSuspend(key_scan_handler); 
// // //                     }
// // //                 }
// //             // vTaskSuspend(key_scan_handler);
                
// /////////////////////////////////����use��һ��ʼ�İ汾////////////////////////////////////////////////////////////
//         // ��ⰴ�����£�����͵�ƽ��Ч��
//         if(currentState == GPIO_PIN_RESET && keyState == GPIO_PIN_SET) {
//             vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_TIME));  // ��������
//             currentState = HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin);
            
//             if(currentState == GPIO_PIN_RESET) {
//                 keyPressTime = xTaskGetTickCount();
                
//                 // ������ⰴ��״̬
//                 while(HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == GPIO_PIN_RESET) {
//                    vTaskDelay(pdMS_TO_TICKS(20));
//                     // vTaskDelay(1);
//                     // ��ⳤ��
//                     if((xTaskGetTickCount() - keyPressTime) >= pdMS_TO_TICKS(LONG_PRESS_TIME)) {
// //											testi = xTaskGetTickCount() - keyPressTime;
//                         xQueueSend(xKeyQueue, &(KeyEvent_t){EV_LONG_PRESS}, 0);
//                         while(HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == GPIO_PIN_RESET) {
//                             vTaskDelay(20);  // �ȴ������ͷ�
//                             // vTaskSuspend(key_scan_handler); 
//                             key_flag = 1;
//                         }
//                         break;
//                     }
//                 }

// //                 // ��ⳤ��
// //                 if((xTaskGetTickCount() - keyPressTime) >= pdMS_TO_TICKS(LONG_PRESS_TIME)) {
// //                     PowerOn();
// // //											testi = xTaskGetTickCount() - keyPressTime;
// //                     xQueueSend(xKeyQueue, &(KeyEvent_t){EV_LONG_PRESS}, 0);
// //                     vTaskDelay(20);  // �ȴ������ͷ�

// //                 }
// //                 else{
// //                     xQueueSend(xKeyQueue, &(KeyEvent_t){EV_SHORT_PRESS}, 0);
// //                     key_flag = 1;
// //                     vTaskDelay(20); 
// //                 }                

//                 // ���̰�
//                 if((xTaskGetTickCount() - keyPressTime) < pdMS_TO_TICKS(LONG_PRESS_TIME)) {
// //									testi = xTaskGetTickCount() - keyPressTime;
//                     if((xTaskGetTickCount() - keyPressTime) >= pdMS_TO_TICKS(SHORT_PRESS_TIME)) {
//                         xQueueSend(xKeyQueue, &(KeyEvent_t){EV_SHORT_PRESS}, 0);
//                         key_flag = 1;
//                         vTaskDelay(20); 
//                         // vTaskSuspend(key_scan_handler); 
//                     }
//                 }
//                 vTaskSuspend(key_scan_handler);
//             }           
//         }
//         keyState = currentState;
//         vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(10));  // 10msɨ������
//     }
// }
// // �޸ĺ�İ������񣨴����̰���
void KeyScanTask(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    uint32_t keyPressTime = 0;
    uint8_t keyState = GPIO_PIN_SET;

    for(;;) {
        uint8_t currentState = HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin);
        printf("into_KeyScanTask\r\n");
 
        keyPressTime = xTaskGetTickCount();
        
        // ������ⰴ��״̬
        while(HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == GPIO_PIN_RESET) {

            vTaskDelay(1);
//       // ��ⳤ��
            if((xTaskGetTickCount() - keyPressTime) >= pdMS_TO_TICKS(LONG_PRESS_TIME)) {
                PowerOn();
                xQueueSend(xKeyQueue, &(KeyEvent_t){EV_LONG_PRESS}, 0);
                // vTaskSuspend(key_scan_handler);
                while(HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == GPIO_PIN_RESET) {
                    vTaskDelay(20);  // �ȴ������ͷ�
                    // vTaskSuspend(key_scan_handler); 
                }
                break;
            }
        }

        if(key_flag >= 3){
            xQueueSend(xKeyQueue, &(KeyEvent_t){EV_SHORT_PRESS}, 0);
            // key_flag = 1;
            vTaskDelay(20); 
        }else{
            vTaskSuspend(key_scan_handler);
        }

        keyState = currentState;
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(10));  // 10msɨ������
    }
}
/////////////////////////////////��ת��Ļ///////////////////////////////////////////////
void ToggleScreen(void)
{
    UBaseType_t stackMark = uxTaskGetStackHighWaterMark(NULL);
    printf("�����������ջʣ��: %lu �ֽ�\n", stackMark * sizeof(StackType_t));
    vTaskSuspendAll();
    ZSBM800X_Init(ECG_PPG_MODE);//��Ҫ���ֽڽϴ���������128*2,���ܲ�̫��
    xTaskResumeAll();
    vTaskSuspend(key_scan_handler); 
	printf("%s++\r\n", __FUNCTION__);
	static uint8_t screenState = 1;
	screenState ^= 1;
//	printf("System close lcd\r\n");
//    SetScreenState(screenState);  // ���Ʊ������ʾ
    stackMark = uxTaskGetStackHighWaterMark(NULL);
    printf("�����������ջʣ��: %lu �ֽ�\n", stackMark * sizeof(StackType_t));
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
                // if(first_open == 1){
                //     PowerOff();
                // }
                    ToggleScreen();  // ��Ļ״̬�л�����
                    // vTaskSuspend(key_scan_handler);
                    break;
                    
                case EV_LONG_PRESS:
//										printf("HandlePowerManagement\r\n");
                    // first_open = 0;
                    HandlePowerManagement();  // ��Դ������
                    // vTaskSuspend(key_scan_handler);
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

    // first_open = 0;
  
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
        // vTaskSuspend(key_scan_handler);
        isPowerOn = 1;
        // first_open = 0;
//      SystemReboot();
	}
	
}