/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include "stdio.h"
#include "stdlib.h"
#include "zsbm800X_drv.h"
/* USER CODE END Includes */


int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

/* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */
  
  while(1)
  {
	  
	// ZSBM8000 中断处理程序
	ZSBM800X_DataHandle();
	
	switch (...)	// 串口指令
	{
		case "PPG":
			ZSBM800X_Init(TOOLING_MODE) ;
			break;

		case "ECG":
			ZSBM800X_Init(ECG_MODE) ;
			break;	  
			
		case "P+E":
			ZSBM800X_Init(ECG_PPG_MODE) ;
			break;	  
			
		....
		// 调节电流、TIA
		case ...:
		ZSBM800X_SetLedCurrent(Z_GREEN_CH, g_current) ;	// 绿灯电流、TIA			
		ZSBM800X_SetTIAGain(g_tia);	
		
		ZSBM800X_SetLedCurrent(Z_RED_CH, r_current) ;	// 绿灯电流、TIA			
		ZSBM800X_SetTIAGain(r_tia);	

		ZSBM800X_SetLedCurrent(Z_IR_CH, i_current) ;	// 绿灯电流、TIA			
		ZSBM800X_SetTIAGain(i_tia);
		
		// 其它指令
		...
		
		}		
	}
}

 
/* USER CODE BEGIN 1 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == EXT_INT0_Pin)	//ZSBM8000外部中断输入脚
	{
		zsbm_int_flag = 1;		
//		LOG_OUT("Int...\n");
	}		
}
/* USER CODE END 1 */