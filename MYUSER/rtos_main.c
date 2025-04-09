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

////////////////////////////////////////////���� ����///////////////////////////////////////////////////////////
/*TASK1 ���� ����
 *����:������ �������ȼ� ��ջ��С ��������
 */
//�������ȼ�
#define TASK1_PRIO        2
//�����ջ��С
#define TASK1_STACK_SIZE  128 //ע��128���� Ҫ*4�ŵ����ֽ� =128*4���ֽ�
//������
TaskHandle_t 		task1_handler;
//������
void task1(void *pvParameters);

/*led_flashing ���� ����
 *����:������ �������ȼ� ��ջ��С ��������
 */
//�������ȼ�
#define LED_FLASHING_PRIO        2
//�����ջ��С
#define LED_FLASHING_STACK_SIZE  128 //ע��128���� Ҫ*4�ŵ����ֽ� =128*4���ֽ�
//������
TaskHandle_t 		led_flashing_handler;
//������
void led_flashing(void *pvParameters);

/*����ɨ�� ���� ����
 *����:������ �������ȼ� ��ջ��С ��������
 */
//�������ȼ�
#define KEY_SCAN_PRIO        5
//�����ջ��С
#define KEY_SCAN_STACK_SIZE  128*2 //ע��128���� Ҫ*4�ŵ����ֽ� =128*4���ֽ�
//������
TaskHandle_t 		key_scan_handler;
////������
//void key_scan(void *pvParameters);

/*�����¼��ȴ� ���� ����
 *����:������ �������ȼ� ��ջ��С ��������
 */
//�������ȼ�
#define KEY_EVENT_PRIO        3
//�����ջ��С
#define KEY_EVENT_STACK_SIZE  128*2 //ע��128���� Ҫ*4�ŵ����ֽ� =128*4���ֽ�
//������
TaskHandle_t 		key_event_handler;
////������
//void key_event(void *pvParameters);

/*��Ļ���� ���� ����
 *����:������ �������ȼ� ��ջ��С ��������
 */
//�������ȼ�
#define LCD_PRIO        3
//�����ջ��С
#define LCD_STACK_SIZE  128 //ע��128���� Ҫ*4�ŵ����ֽ� =128*4���ֽ�
//������
TaskHandle_t 		lcd_handler;
////������

/*������ ���� ����
 *����:������ �������ȼ� ��ջ��С ��������
 */
//�������ȼ�
#define ZSBM_PRIO        4
//�����ջ��С
#define ZSBM_STACK_SIZE  128 //ע��128���� Ҫ*4�ŵ����ֽ� =128*4���ֽ�
//������
TaskHandle_t 		zsbm_handler;
////������
//void key_event(void *pvParameters);

/*LCD��ʾ���� ���� ����
 *����:������ �������ȼ� ��ջ��С ��������
 */
//�������ȼ�
#define LCD_DISPLAY_PRIO        3
//�����ջ��С
#define LCD_DISPLAY_STACK_SIZE  128*4 //ע��128���� Ҫ*4�ŵ����ֽ� =128*4���ֽ�
//������
TaskHandle_t 		lcd_display_handler;
////������
//void key_event(void *pvParameters);


///////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Ӳ����ʼ��
void Hart_init(void)
{
	//���е�Ӳ����ʼ�������Է�������
	HAL_Delay(50);
}
////////////////////////���г�ʼ�������е��¶��ж�������//////////////////////////////////////////////////////
QueueHandle_t xKeyQueue;
QueueHandle_t sensorDataQueue; // ���ݶ���
void Queue_init(void)
{
	// ��������
	xKeyQueue = xQueueCreate(3, sizeof(KeyEvent_t));  // ����������Ϊ3������
	if (xKeyQueue == NULL) {
    printf("���д���ʧ�ܣ�\r\n");
    	Error_Handler();
	}else{
		printf("�������д����ɹ���\r\n");
	}

	// ����z8000���У�����Ϊ10�����ݵ�
    sensorDataQueue = xQueueCreate(10, sizeof(SensorData_t));
    if (sensorDataQueue == NULL) {
        Error_Handler();
    }else{
		printf("���������д����ɹ���\r\n");
	}
	
}
////////////////////////�ź�����ʼ�������е����ź�����������//////////////////////////////////////////////////////
SemaphoreHandle_t xzsbmSemaphore;
SemaphoreHandle_t xKeySemaphore = NULL;
int test;
void Semaphore_init(void)
{
	xzsbmSemaphore = xSemaphoreCreateBinary();	
	if (xzsbmSemaphore == NULL) {
    	printf("zsbm��ֵ�ź�������ʧ�ܣ�\r\n");
		test = 0;
    	Error_Handler();
	}else{
		printf("zsbm��ֵ�ź��������ɹ���\r\n");
		test = 1;
	}
	
	xKeySemaphore = xSemaphoreCreateBinary();
}
////////////////////////�����ʼ�������е�������������//////////////////////////////////////////////////////
void Soft_init(void)
{
	taskENTER_CRITICAL(); /*�����ٽ���,�ر��жϣ���ֹ����������ִ���Ƚ��д����ĵ����ȼ���task1����
												 *������������л����˳��ٽ����Ż������л�*/

//////////////////test����//////////////////////////////////
	xTaskCreate((TaskFunction_t ) task1,
							(const char *   ) "task1",
							(uint16_t 			) TASK1_STACK_SIZE,  
							(void *         ) NULL,
							(UBaseType_t    ) TASK1_PRIO,
							(TaskHandle_t * ) &task1_handler ); 
	
////////////////led_flashing����////////////////////////////
	xTaskCreate((TaskFunction_t ) led_flashing,
							(const char *   ) "led_flashing",
							(uint16_t 			) LED_FLASHING_STACK_SIZE,  
							(void *         ) NULL,
							(UBaseType_t    ) LED_FLASHING_PRIO,
							(TaskHandle_t * ) &led_flashing_handler ); 
							
////////////////����ɨ������////////////////////////////
	xTaskCreate((TaskFunction_t ) KeyScanTask,
							(const char *   ) "key_scan",
							(uint16_t 			) KEY_SCAN_STACK_SIZE,  
							(void *         ) NULL,
							(UBaseType_t    ) KEY_SCAN_PRIO,
							(TaskHandle_t * ) &key_scan_handler ); 
			
////////////////�����¼�����////////////////////////////
	xTaskCreate((TaskFunction_t ) KeyEventHandlerTask,
							(const char *   ) "key_event",
							(uint16_t 			) KEY_EVENT_STACK_SIZE,  
							(void *         ) NULL,
							(UBaseType_t    ) KEY_EVENT_PRIO,
							(TaskHandle_t * ) &key_event_handler ); 

////////////////��Ļ��������////////////////////////////
	// xTaskCreate((TaskFunction_t ) LCD_Draw_Task,
	// 						(const char *   ) "lcd_event",
	// 						(uint16_t 			) LCD_STACK_SIZE,  
	// 						(void *         ) NULL,
	// 						(UBaseType_t    ) LCD_PRIO,
	// 						(TaskHandle_t * ) &lcd_handler ); 

////////////////z8000��������////////////////////////////
	xTaskCreate((TaskFunction_t ) sensorTask,
							(const char *   ) "zsbm_event",
							(uint16_t 			) ZSBM_STACK_SIZE,  
							(void *         ) NULL,
							(UBaseType_t    ) ZSBM_PRIO,
							(TaskHandle_t * ) &zsbm_handler ); 

// ////////////////��Ļ��ʾ����////////////////////////////
	// xTaskCreate((TaskFunction_t ) lcdTask,
	// 						(const char *   ) "lcd_display_event",
	// 						(uint16_t 			) LCD_DISPLAY_STACK_SIZE,  
	// 						(void *         ) NULL,
	// 						(UBaseType_t    ) LCD_DISPLAY_PRIO,
	// 						(TaskHandle_t * ) &lcd_display_handler ); 

	taskEXIT_CRITICAL(); //�˳��ٽ������˳���ŻῪʼ������ȣ������ȼ� ��ִ��	
}

////////////////���������ת��//////////////////////
int add( int a)
{
  a += 1;
	return a;
}

/*test����ʵ��LED0ÿ500ms��תһ��*/
void task1(void *pvParameters)
{
	uint8_t task1_num = 10;
		
	while(1)
	{
//		task1_num = add(task1_num);
//				
//		printf("test������������ %d\r\n",task1_num);
////		HAL_GPIO_WritePin(LED_FLASHING_GPIO_Port,LED_FLASHING_Pin,GPIO_PIN_SET);
		vTaskDelay(10);
	}
}

/*����led_flashing��ʵ��LED0ÿ500ms��תһ��*/
void led_flashing(void *pvParameters)
{
	uint8_t task1_num = 0;
	
	while(1)
	{
		task1_num = add(task1_num);
		
//		printf("led_flashing������������ %d\r\n",task1_num);
//		HAL_GPIO_WritePin(LED_FLASHING_GPIO_Port,LED_FLASHING_Pin,GPIO_PIN_SET);
		HAL_GPIO_TogglePin(LED_FLASHING_GPIO_Port,LED_FLASHING_Pin);
		// printf("test = %d\r\n",test);
		vTaskDelay(500);
	}
}


extern volatile uint8_t zsbm_int_flag; // ��zsbm800x_drv.c������
// ȫ�ֱ�����¼����ʱ��
// TickType_t xPressTime = 0;
// TickType_t xReleaseTime = 0;
uint8_t key_flag = 0;
// extern uint8_t first_open;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	// printf("%s++\r\n", __FUNCTION__);
/////////////////////////////////z8000/////////////////////////////////////////////////////
    // if (GPIO_Pin == GPIO_INT_Pin) // PA8��ӦEXTI8
    // {
    //     zsbm_int_flag = 1; // ���ñ�־�Դ������ݴ���
	// 	printf("into zsbm_int_flag = %d\r\n",zsbm_int_flag);
    // }
//  �ź�������
	if (GPIO_Pin == GPIO_INT_Pin) // PA8��ӦEXTI8
	{
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		zsbm_int_flag = 1; // ���ñ�־�Դ������ݴ���
		// printf("into zsbm_int_flag = %d\r\n",zsbm_int_flag);
        xSemaphoreGiveFromISR(xzsbmSemaphore, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken); // ����и������ȼ����񱻻��ѣ������л�
    }
//  �¼�����
	// if (GPIO_Pin == GPIO_INT_Pin) // PA8��ӦEXTI8
	// {
    //     zsbm_int_flag = 1;  // �����жϱ�־
    //     BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	// 	printf("into zsbm_int_flag = %d\r\n",zsbm_int_flag);

    //     // ��������֪ͨ�������ݴ�������
    //     vTaskNotifyGiveFromISR(zsbm_handler, &xHigherPriorityTaskWoken);
    //     portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    // }
/////////////////////////////////����/////////////////////////////////////////////////////
//use�汾
	if (GPIO_Pin == KEY_Pin) // PA8��ӦEXTI8
	{
		// printf("into KEY_Pin\r\n");
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;

		if(key_flag <= 5){
			key_flag++;
			if(key_flag >= 5)
				key_flag = 6;
		}

		xTaskResumeFromISR(key_scan_handler);  // �ж��лָ�
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

    }

	// if (GPIO_Pin == KEY_Pin) // PA8��ӦEXTI8
	// {
	// 	printf("into KEY_Pin\r\n");
	// 	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	// 	if(HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == GPIO_PIN_RESET){
	// 		// key_flag = 1;
	// 		xTaskResumeFromISR(key_scan_handler);  // �ж��лָ�
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
    //         xPressTime = xTaskGetTickCountFromISR(); // ����ʱ��
    //     } else {
    //         xReleaseTime = xTaskGetTickCountFromISR(); // �ͷ�ʱ��
    //         xSemaphoreGiveFromISR(xKeySemaphore, &xHigherPriorityTaskWoken);
    //     }
    //     portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    // }

	// printf("%s--\r\n", __FUNCTION__);
}
