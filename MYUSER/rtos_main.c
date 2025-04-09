#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "rtos_main.h"
#include "usart.h"
#include <stdio.h>
#include "queue.h"
#include "semphr.h"
#include "key.h"
#include "lcd.h"
#include "zsbm800x_drv.h"
#include "gpio.h"
#include "main.h"
#include "stm32l4xx_hal.h"

////////////////////////////////////////////任务 配置///////////////////////////////////////////////////////////
/*TASK1 任务 配置
 *包括:任务句柄 任务优先级 堆栈大小 创建任务
 */
//任务优先级
#define TASK1_PRIO        2
//任务堆栈大小
#define TASK1_STACK_SIZE  128 //注意128是字 要*4才等于字节 =128*4个字节
//任务句柄
TaskHandle_t 		task1_handler;
//任务函数
void task1(void *pvParameters);

/*led_flashing 任务 配置
 *包括:任务句柄 任务优先级 堆栈大小 创建任务
 */
//任务优先级
#define LED_FLASHING_PRIO        2
//任务堆栈大小
#define LED_FLASHING_STACK_SIZE  128 //注意128是字 要*4才等于字节 =128*4个字节
//任务句柄
TaskHandle_t 		led_flashing_handler;
//任务函数
void led_flashing(void *pvParameters);

/*按键扫描 任务 配置
 *包括:任务句柄 任务优先级 堆栈大小 创建任务
 */
//任务优先级
#define KEY_SCAN_PRIO        5
//任务堆栈大小
#define KEY_SCAN_STACK_SIZE  128*2 //注意128是字 要*4才等于字节 =128*4个字节
//任务句柄
TaskHandle_t 		key_scan_handler;
////任务函数
//void key_scan(void *pvParameters);

/*按键事件等待 任务 配置
 *包括:任务句柄 任务优先级 堆栈大小 创建任务
 */
//任务优先级
#define KEY_EVENT_PRIO        3
//任务堆栈大小
#define KEY_EVENT_STACK_SIZE  128*2 //注意128是字 要*4才等于字节 =128*4个字节
//任务句柄
TaskHandle_t 		key_event_handler;
////任务函数
//void key_event(void *pvParameters);

/*屏幕测试 任务 配置
 *包括:任务句柄 任务优先级 堆栈大小 创建任务
 */
//任务优先级
#define LCD_PRIO        3
//任务堆栈大小
#define LCD_STACK_SIZE  128 //注意128是字 要*4才等于字节 =128*4个字节
//任务句柄
TaskHandle_t 		lcd_handler;
////任务函数

/*传感器 任务 配置
 *包括:任务句柄 任务优先级 堆栈大小 创建任务
 */
//任务优先级
#define ZSBM_PRIO        4
//任务堆栈大小
#define ZSBM_STACK_SIZE  128 //注意128是字 要*4才等于字节 =128*4个字节
//任务句柄
TaskHandle_t 		zsbm_handler;
////任务函数
//void key_event(void *pvParameters);

/*LCD显示波形 任务 配置
 *包括:任务句柄 任务优先级 堆栈大小 创建任务
 */
//任务优先级
#define LCD_DISPLAY_PRIO        3
//任务堆栈大小
#define LCD_DISPLAY_STACK_SIZE  128*4 //注意128是字 要*4才等于字节 =128*4个字节
//任务句柄
TaskHandle_t 		lcd_display_handler;
////任务函数
//void key_event(void *pvParameters);


///////////////////////////////////////////////////////////////////////////////////////////////////////////////

//硬件初始化
void Hart_init(void)
{
	//所有的硬件初始化都可以放在这里
	HAL_Delay(50);
}
////////////////////////队列初始化，所有的新队列都在这里//////////////////////////////////////////////////////
QueueHandle_t xKeyQueue;
QueueHandle_t sensorDataQueue; // 数据队列
void Queue_init(void)
{
	// 按键队列
	xKeyQueue = xQueueCreate(3, sizeof(KeyEvent_t));  // 按键，调整为3级缓冲
	if (xKeyQueue == NULL) {
    printf("队列创建失败！\r\n");
    	Error_Handler();
	}else{
		printf("按键队列创建成功！\r\n");
	}

	// 创建z8000队列，容量为10个数据点
    sensorDataQueue = xQueueCreate(10, sizeof(SensorData_t));
    if (sensorDataQueue == NULL) {
        Error_Handler();
    }else{
		printf("传感器队列创建成功！\r\n");
	}
	
}
////////////////////////信号量初始化，所有的新信号量都在这里//////////////////////////////////////////////////////
SemaphoreHandle_t xzsbmSemaphore;
SemaphoreHandle_t xKeySemaphore = NULL;
int test;
void Semaphore_init(void)
{
	xzsbmSemaphore = xSemaphoreCreateBinary();	
	if (xzsbmSemaphore == NULL) {
    	printf("zsbm二值信号量创建失败！\r\n");
		test = 0;
    	Error_Handler();
	}else{
		printf("zsbm二值信号量创建成功！\r\n");
		test = 1;
	}
	
	xKeySemaphore = xSemaphoreCreateBinary();
}
////////////////////////软件初始化，所有的新任务都在这里//////////////////////////////////////////////////////
void Soft_init(void)
{
	taskENTER_CRITICAL(); /*进入临界区,关闭中断，防止调度器优先执行先进行创建的低优先级的task1任务
												 *不会进行任务切换，退出临界区才会任务切换*/

//////////////////test任务//////////////////////////////////
	xTaskCreate((TaskFunction_t ) task1,
							(const char *   ) "task1",
							(uint16_t 			) TASK1_STACK_SIZE,  
							(void *         ) NULL,
							(UBaseType_t    ) TASK1_PRIO,
							(TaskHandle_t * ) &task1_handler ); 
	
////////////////led_flashing任务////////////////////////////
	xTaskCreate((TaskFunction_t ) led_flashing,
							(const char *   ) "led_flashing",
							(uint16_t 			) LED_FLASHING_STACK_SIZE,  
							(void *         ) NULL,
							(UBaseType_t    ) LED_FLASHING_PRIO,
							(TaskHandle_t * ) &led_flashing_handler ); 
							
////////////////按键扫描任务////////////////////////////
	xTaskCreate((TaskFunction_t ) KeyScanTask,
							(const char *   ) "key_scan",
							(uint16_t 			) KEY_SCAN_STACK_SIZE,  
							(void *         ) NULL,
							(UBaseType_t    ) KEY_SCAN_PRIO,
							(TaskHandle_t * ) &key_scan_handler ); 
			
////////////////按键事件任务////////////////////////////
	xTaskCreate((TaskFunction_t ) KeyEventHandlerTask,
							(const char *   ) "key_event",
							(uint16_t 			) KEY_EVENT_STACK_SIZE,  
							(void *         ) NULL,
							(UBaseType_t    ) KEY_EVENT_PRIO,
							(TaskHandle_t * ) &key_event_handler ); 

////////////////屏幕测试任务////////////////////////////
	// xTaskCreate((TaskFunction_t ) LCD_Draw_Task,
	// 						(const char *   ) "lcd_event",
	// 						(uint16_t 			) LCD_STACK_SIZE,  
	// 						(void *         ) NULL,
	// 						(UBaseType_t    ) LCD_PRIO,
	// 						(TaskHandle_t * ) &lcd_handler ); 

////////////////z8000测试任务////////////////////////////
	xTaskCreate((TaskFunction_t ) sensorTask,
							(const char *   ) "zsbm_event",
							(uint16_t 			) ZSBM_STACK_SIZE,  
							(void *         ) NULL,
							(UBaseType_t    ) ZSBM_PRIO,
							(TaskHandle_t * ) &zsbm_handler ); 

// ////////////////屏幕显示任务////////////////////////////
	// xTaskCreate((TaskFunction_t ) lcdTask,
	// 						(const char *   ) "lcd_display_event",
	// 						(uint16_t 			) LCD_DISPLAY_STACK_SIZE,  
	// 						(void *         ) NULL,
	// 						(UBaseType_t    ) LCD_DISPLAY_PRIO,
	// 						(TaskHandle_t * ) &lcd_display_handler ); 

	taskEXIT_CRITICAL(); //退出临界区，退出后才会开始任务调度，高优先级 先执行	
}

////////////////测试任务的转换//////////////////////
int add( int a)
{
  a += 1;
	return a;
}

/*test任务，实现LED0每500ms翻转一次*/
void task1(void *pvParameters)
{
	uint8_t task1_num = 10;
		
	while(1)
	{
//		task1_num = add(task1_num);
//				
//		printf("test任务正在运行 %d\r\n",task1_num);
////		HAL_GPIO_WritePin(LED_FLASHING_GPIO_Port,LED_FLASHING_Pin,GPIO_PIN_SET);
		vTaskDelay(10);
	}
}

/*任务led_flashing，实现LED0每500ms翻转一次*/
void led_flashing(void *pvParameters)
{
	uint8_t task1_num = 0;
	
	while(1)
	{
		task1_num = add(task1_num);
		
//		printf("led_flashing任务正在运行 %d\r\n",task1_num);
//		HAL_GPIO_WritePin(LED_FLASHING_GPIO_Port,LED_FLASHING_Pin,GPIO_PIN_SET);
		HAL_GPIO_TogglePin(LED_FLASHING_GPIO_Port,LED_FLASHING_Pin);
		// printf("test = %d\r\n",test);
		vTaskDelay(500);
	}
}


extern volatile uint8_t zsbm_int_flag; // 在zsbm800x_drv.c中声明
// 全局变量记录按键时间
// TickType_t xPressTime = 0;
// TickType_t xReleaseTime = 0;
uint8_t key_flag = 0;
// extern uint8_t first_open;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	// printf("%s++\r\n", __FUNCTION__);
/////////////////////////////////z8000/////////////////////////////////////////////////////
    // if (GPIO_Pin == GPIO_INT_Pin) // PA8对应EXTI8
    // {
    //     zsbm_int_flag = 1; // 设置标志以触发数据处理
	// 	printf("into zsbm_int_flag = %d\r\n",zsbm_int_flag);
    // }
//  信号量驱动
	if (GPIO_Pin == GPIO_INT_Pin) // PA8对应EXTI8
	{
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		zsbm_int_flag = 1; // 设置标志以触发数据处理
		// printf("into zsbm_int_flag = %d\r\n",zsbm_int_flag);
        xSemaphoreGiveFromISR(xzsbmSemaphore, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken); // 如果有更高优先级任务被唤醒，立即切换
    }
//  事件驱动
	// if (GPIO_Pin == GPIO_INT_Pin) // PA8对应EXTI8
	// {
    //     zsbm_int_flag = 1;  // 设置中断标志
    //     BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	// 	printf("into zsbm_int_flag = %d\r\n",zsbm_int_flag);

    //     // 发送任务通知唤醒数据处理任务
    //     vTaskNotifyGiveFromISR(zsbm_handler, &xHigherPriorityTaskWoken);
    //     portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    // }
/////////////////////////////////按键/////////////////////////////////////////////////////
//use版本
	if (GPIO_Pin == KEY_Pin) // PA8对应EXTI8
	{
		// printf("into KEY_Pin\r\n");
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;

		if(key_flag <= 5){
			key_flag++;
			if(key_flag >= 5)
				key_flag = 6;
		}

		xTaskResumeFromISR(key_scan_handler);  // 中断中恢复
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

    }

	// if (GPIO_Pin == KEY_Pin) // PA8对应EXTI8
	// {
	// 	printf("into KEY_Pin\r\n");
	// 	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	// 	if(HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == GPIO_PIN_RESET){
	// 		// key_flag = 1;
	// 		xTaskResumeFromISR(key_scan_handler);  // 中断中恢复
	// 		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	// 	}else{
	// 		// xSemaphoreGiveFromISR(xKeySemaphore, &xHigherPriorityTaskWoken);
	// 		// key_flag = 1;
	// 	}
	// 	// portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

    // }

    
    // if (GPIO_Pin == KEY_Pin) {
	// 	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    //     if (HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == GPIO_PIN_RESET) {
    //         xPressTime = xTaskGetTickCountFromISR(); // 按下时刻
    //     } else {
    //         xReleaseTime = xTaskGetTickCountFromISR(); // 释放时刻
    //         xSemaphoreGiveFromISR(xKeySemaphore, &xHigherPriorityTaskWoken);
    //     }
    //     portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    // }

	// printf("%s--\r\n", __FUNCTION__);
}
