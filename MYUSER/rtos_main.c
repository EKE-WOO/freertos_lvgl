#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "rtos_main.h"
#include "usart.h"
#include <stdio.h>
#include "queue.h"
#include "key.h"

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
#define LED_FLASHING_PRIO        3
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
#define KEY_SCAN_PRIO        4
//任务堆栈大小
#define KEY_SCAN_STACK_SIZE  128 //注意128是字 要*4才等于字节 =128*4个字节
//任务句柄
TaskHandle_t 		key_scan_handler;
////任务函数
//void key_scan(void *pvParameters);

/*按键事件等待 任务 配置
 *包括:任务句柄 任务优先级 堆栈大小 创建任务
 */
//任务优先级
#define KEY_EVENT_PRIO        5
//任务堆栈大小
#define KEY_EVENT_STACK_SIZE  128 //注意128是字 要*4才等于字节 =128*4个字节
//任务句柄
TaskHandle_t 		key_event_handler;
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
void Queue_init(void)
{
	xKeyQueue = xQueueCreate(3, sizeof(KeyEvent_t));  // 按键，调整为3级缓冲
	if (xKeyQueue == NULL) {
    printf("队列创建失败！\r\n");
    Error_Handler();
	}else{
	printf("队列创建成功！\r\n");
	}
	
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
		vTaskDelay(500);
	}
}
