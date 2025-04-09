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

// 按键配置
#define KEY_PRESSED_STATE  GPIO_PIN_RESET  // 低电平表示按下

//外部声明队列
extern QueueHandle_t xKeyQueue;
extern SemaphoreHandle_t xKeySemaphore;
extern TaskHandle_t 		key_scan_handler;

extern uint8_t key_flag;
// 时间参数
#define DEBOUNCE_TIME      20    // 消抖时间20ms
#define SHORT_PRESS_TIME   100   // 短按时间500ms
#define LONG_PRESS_TIME    2000  // 长按时间2000ms
uint32_t testi = 0;
// void KeyScanTask(void *pvParameters)
// {
//     TickType_t xLastWakeTime = xTaskGetTickCount();
//     uint32_t keyPressTime = 0;
//     uint8_t keyState = GPIO_PIN_SET;

//     for(;;) {

//         // // 等待信号量触发挂起指令
//         // if(xSemaphoreTake(xKeySemaphore, 0) == pdTRUE) {
//         //     vTaskSuspend(NULL); // 自我挂起
//         // }
//         // if(key_flag == 1) {
//         //     vTaskSuspend(NULL); // 自我挂起
//         // }

//         uint8_t currentState = HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin);
//         printf("into_KeyScanTask\r\n");
 
//         // 检测按键按下（假设低电平有效）

// /////////////////////////////////use/////////////////////////////////////////////////////////////
// //         keyPressTime = xTaskGetTickCount();
        
// //         // 持续监测按键状态
// //         while(HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == GPIO_PIN_RESET) {
// // //                    vTaskDelay(pdMS_TO_TICKS(20));
// //             vTaskDelay(1);
// // //                     // 检测长按
// //             if((xTaskGetTickCount() - keyPressTime) >= pdMS_TO_TICKS(LONG_PRESS_TIME)) {
// //                 PowerOn();
// //                 xQueueSend(xKeyQueue, &(KeyEvent_t){EV_LONG_PRESS}, 0);
// //                 while(HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == GPIO_PIN_RESET) {
// //                     vTaskDelay(20);  // 等待按键释放
// //                     // vTaskSuspend(key_scan_handler); 
// //                 }
// //                 break;
// //             }
// //         }

// //         // 检测长按
// // //         if((xTaskGetTickCount() - keyPressTime) >= pdMS_TO_TICKS(LONG_PRESS_TIME)) {
// // //             PowerOn();
// // //             // first_open = 0;
// // // //											testi = xTaskGetTickCount() - keyPressTime;
// // //             xQueueSend(xKeyQueue, &(KeyEvent_t){EV_LONG_PRESS}, 0);
// // //             vTaskDelay(20);  // 等待按键释放

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

// //             // 检测短按
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
                
// /////////////////////////////////下面use是一开始的版本////////////////////////////////////////////////////////////
//         // 检测按键按下（假设低电平有效）
//         if(currentState == GPIO_PIN_RESET && keyState == GPIO_PIN_SET) {
//             vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_TIME));  // 消抖处理
//             currentState = HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin);
            
//             if(currentState == GPIO_PIN_RESET) {
//                 keyPressTime = xTaskGetTickCount();
                
//                 // 持续监测按键状态
//                 while(HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == GPIO_PIN_RESET) {
//                    vTaskDelay(pdMS_TO_TICKS(20));
//                     // vTaskDelay(1);
//                     // 检测长按
//                     if((xTaskGetTickCount() - keyPressTime) >= pdMS_TO_TICKS(LONG_PRESS_TIME)) {
// //											testi = xTaskGetTickCount() - keyPressTime;
//                         xQueueSend(xKeyQueue, &(KeyEvent_t){EV_LONG_PRESS}, 0);
//                         while(HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == GPIO_PIN_RESET) {
//                             vTaskDelay(20);  // 等待按键释放
//                             // vTaskSuspend(key_scan_handler); 
//                             key_flag = 1;
//                         }
//                         break;
//                     }
//                 }

// //                 // 检测长按
// //                 if((xTaskGetTickCount() - keyPressTime) >= pdMS_TO_TICKS(LONG_PRESS_TIME)) {
// //                     PowerOn();
// // //											testi = xTaskGetTickCount() - keyPressTime;
// //                     xQueueSend(xKeyQueue, &(KeyEvent_t){EV_LONG_PRESS}, 0);
// //                     vTaskDelay(20);  // 等待按键释放

// //                 }
// //                 else{
// //                     xQueueSend(xKeyQueue, &(KeyEvent_t){EV_SHORT_PRESS}, 0);
// //                     key_flag = 1;
// //                     vTaskDelay(20); 
// //                 }                

//                 // 检测短按
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
//         vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(10));  // 10ms扫描周期
//     }
// }
// // 修改后的按键任务（处理长短按）
void KeyScanTask(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    uint32_t keyPressTime = 0;
    uint8_t keyState = GPIO_PIN_SET;

    for(;;) {
        uint8_t currentState = HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin);
        printf("into_KeyScanTask\r\n");
 
        keyPressTime = xTaskGetTickCount();
        
        // 持续监测按键状态
        while(HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == GPIO_PIN_RESET) {

            vTaskDelay(1);
//       // 检测长按
            if((xTaskGetTickCount() - keyPressTime) >= pdMS_TO_TICKS(LONG_PRESS_TIME)) {
                PowerOn();
                xQueueSend(xKeyQueue, &(KeyEvent_t){EV_LONG_PRESS}, 0);
                // vTaskSuspend(key_scan_handler);
                while(HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == GPIO_PIN_RESET) {
                    vTaskDelay(20);  // 等待按键释放
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
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(10));  // 10ms扫描周期
    }
}
/////////////////////////////////翻转屏幕///////////////////////////////////////////////
void ToggleScreen(void)
{
    UBaseType_t stackMark = uxTaskGetStackHighWaterMark(NULL);
    printf("传感器任务堆栈剩余: %lu 字节\n", stackMark * sizeof(StackType_t));
    vTaskSuspendAll();
    ZSBM800X_Init(ECG_PPG_MODE);//需要的字节较大现在用了128*2,可能不太够
    xTaskResumeAll();
    vTaskSuspend(key_scan_handler); 
	printf("%s++\r\n", __FUNCTION__);
	static uint8_t screenState = 1;
	screenState ^= 1;
//	printf("System close lcd\r\n");
//    SetScreenState(screenState);  // 控制背光或显示
    stackMark = uxTaskGetStackHighWaterMark(NULL);
    printf("传感器任务堆栈剩余: %lu 字节\n", stackMark * sizeof(StackType_t));
	printf("%s--\r\n", __FUNCTION__);
}
///////////////////////////////////开关机//////////////////////////////////////////////
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
                    ToggleScreen();  // 屏幕状态切换函数
                    // vTaskSuspend(key_scan_handler);
                    break;
                    
                case EV_LONG_PRESS:
//										printf("HandlePowerManagement\r\n");
                    // first_open = 0;
                    HandlePowerManagement();  // 电源管理函数
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
		// 关机流程
		printf("System Shutdown Initiated\r\n");
			
			/* 安全关机序列 */
			// 1. 关闭所有外设
//        MX_GPIO_DeInit();
//        MX_USART1_UART_DeInit();
			// 添加其他外设关闭代码...
			
			// 2. 保存关键数据
//        SaveSystemStateToFlash();
			
			// 3. 切断电源（最后一步）
//        HAL_GPIO_WritePin(PWR_CTRL_GPIO_PORT, PWR_CTRL_PIN, PWR_OFF_STATE);
		PowerOff();
		BT_Power_Off();
			
			// 4. 进入死循环等待断电
		isPowerOn = 0;
//        while(1) { 
//            __NOP();  // 必须保持在此循环
//        }
	}else{
		PowerOn();
		BT_Power_On();
    // 开机流程（由硬件自动完成）
		printf("System open Initiated\r\n");
        // vTaskSuspend(key_scan_handler);
        isPowerOn = 1;
        // first_open = 0;
//      SystemReboot();
	}
	
}