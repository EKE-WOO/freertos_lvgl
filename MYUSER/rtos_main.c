#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "rtos_main.h"
#include "usart.h"
#include <stdio.h>
#include "queue.h"
#include "key.h"

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
#define LED_FLASHING_PRIO        3
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
#define KEY_SCAN_PRIO        4
//�����ջ��С
#define KEY_SCAN_STACK_SIZE  128 //ע��128���� Ҫ*4�ŵ����ֽ� =128*4���ֽ�
//������
TaskHandle_t 		key_scan_handler;
////������
//void key_scan(void *pvParameters);

/*�����¼��ȴ� ���� ����
 *����:������ �������ȼ� ��ջ��С ��������
 */
//�������ȼ�
#define KEY_EVENT_PRIO        5
//�����ջ��С
#define KEY_EVENT_STACK_SIZE  128 //ע��128���� Ҫ*4�ŵ����ֽ� =128*4���ֽ�
//������
TaskHandle_t 		key_event_handler;
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
void Queue_init(void)
{
	xKeyQueue = xQueueCreate(3, sizeof(KeyEvent_t));  // ����������Ϊ3������
	if (xKeyQueue == NULL) {
    printf("���д���ʧ�ܣ�\r\n");
    Error_Handler();
	}else{
	printf("���д����ɹ���\r\n");
	}
	
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
		vTaskDelay(500);
	}
}
