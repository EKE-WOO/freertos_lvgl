#ifndef _ZSBM800X_DRV_H_
#define _ZSBM800X_DRV_H_

#include <stdbool.h>
#include "zsbm800X_port.h"
#include "zsbm800x_regcfg.h"
//#include "Public.h"
//#include "zsbm800X_ppg.h"
#include "zsbm800X_ecg_ra.h"

#define LED_DRV_PIN0	(1<<0)
#define LED_DRV_PIN1	(1<<1)
#define LED_DRV_PIN2	(1<<2)
#define LED_DRV_PIN3	(1<<3)
#define LED_DRV_PIN4	(1<<4)
#define LED_DRV_PIN5	(1<<5)

#define LED_DRV_PIN6	(1<<8)
#define LED_DRV_PIN7	(1<<9)
#define LED_DRV_PIN8	(1<<10)
#define LED_DRV_PIN9	(1<<11)
#define LED_DRV_PIN10	(1<<12)
#define LED_DRV_PIN11	(1<<13)

#define SAMPLE_FS_1HZ   	1 	/* for calibration only */
#define SAMPLE_FS_5HZ   	5 	/* IR wearalbe detect */
#define SAMPLE_FS_10HZ   	10 	/* IR wearalbe detect */
#define SAMPLE_FS_25HZ   	25 	/* Factory test, HRS*/
#define SAMPLE_FS_50HZ  	50 	/* SPO2/HR */
#define SAMPLE_FS_100HZ  	100 /* SPO2/HRV*/
#define SAMPLE_FS_200HZ  	200 /* HR RRI*/
#define SAMPLE_FS_250HZ		250	/* PPG */
#define SAMPLE_FS_300HZ		300	/* PPG */

#define SAMPLE_ECG_FS_250HZ		250	/* PPG */
#define SAMPLE_ECG_FS_300HZ		300	/* PPG */
#define SAMPLE_ECG_FS_500HZ		500	/* ECG */
#define SAMPLE_ECG_FS_600HZ		600	/* ECG */
#define SAMPLE_ECG_FS_1000HZ	1000	/* ECG */
#define SAMPLE_ECG_FS_1200HZ	1200	/* ECG */

#define ZSBM800X_FIFO_ADDR		0x0000
#define ZSBM800X_FIFO_SIZE		(1024)	// Bytes

////////////////////////////
// 以下为FIFO起始地址及大小，将会根据以上用户配置自动计算，无需用户参于
////////////////////////////

// 中断阈值按 90% FIFO size配置，且 PPG按 3 byte * channels aligned, ECG 按 3 byte aligned
#define FIFO_DEPTH_THRD_PERC		(90)		

//	TSD 用于接近检测，实际采样率不超过15Hz，适用于所有模式。且允许丢FIFO数据
#define TSD_FIFO_SIZE				(0x20)
#define TSD_FIFO_START_ADDR			(ZSBM800X_FIFO_ADDR + ZSBM800X_FIFO_SIZE - TSD_FIFO_SIZE)


// AGC过程: FIFO depth thrd 3 Byte, Use the last enabled TS trig INT
#define AGC_TSA_FIFO_SIZE			(0xC8)
#define AGC_TSA_FIFO_START_ADDR		(ZSBM800X_FIFO_ADDR)
#define AGC_TSB_FIFO_SIZE			(0xC8)
#define AGC_TSB_FIFO_START_ADDR		(AGC_TSA_FIFO_START_ADDR + AGC_TSA_FIFO_SIZE)
#define AGC_TSC_FIFO_SIZE			(0xC8)
#define AGC_TSC_FIFO_START_ADDR		(AGC_TSB_FIFO_START_ADDR + AGC_TSB_FIFO_SIZE)
#define AGC_TSD_FIFO_SIZE			(0xC8)
#define AGC_TSD_FIFO_START_ADDR		(AGC_TSC_FIFO_START_ADDR + AGC_TSC_FIFO_SIZE)

#define HR_HRV_TSABCD_FIFO_INT_THRD	(16)



// HR/HRV: 只开启Green
#define HR_HRV_TSA_FIFO_SIZE		(ZSBM800X_FIFO_SIZE - TSD_FIFO_SIZE)
#define HR_HRV_TSA_FIFO_START_ADDR	(ZSBM800X_FIFO_ADDR)

//#define HR_HRV_TSA_FIFO_INT_THRD	(HR_HRV_TSA_FIFO_SIZE*FIFO_DEPTH_THRD_PERC/100/4*4)	// 4 Byte aligned	
#define HR_HRV_TSA_FIFO_INT_THRD	300	// Limited to ACC fifo size, MAX fifo size for ACC is 32 BYTE


// SPO2: Red + IR
#define SPO2_TSB_FIFO_SIZE			((ZSBM800X_FIFO_SIZE - TSD_FIFO_SIZE)/2/16*16)	// 16 Byte aligned
#define SPO2_TSB_FIFO_START_ADDR	(ZSBM800X_FIFO_ADDR )
#define SPO2_TSC_FIFO_SIZE			(SPO2_TSB_FIFO_SIZE)
#define SPO2_TSC_FIFO_START_ADDR	(SPO2_TSB_FIFO_START_ADDR +  SPO2_TSB_FIFO_SIZE)

#define SPO2_TSC_FIFO_INT_THRD		(SPO2_TSC_FIFO_SIZE*FIFO_DEPTH_THRD_PERC/100/8*8)	// 8 Byte aligned

// FreeRun: Green + Red + IR
#define FREE_R_TSA_FIFO_SIZE		((ZSBM800X_FIFO_SIZE - TSD_FIFO_SIZE)/3/16*16)	// 16 Byte aligned
#define FREE_R_TSA_FIFO_START_ADDR	(ZSBM800X_FIFO_ADDR)
#define FREE_R_TSB_FIFO_SIZE		FREE_R_TSA_FIFO_SIZE
#define FREE_R_TSB_FIFO_START_ADDR	(FREE_R_TSA_FIFO_SIZE +  FREE_R_TSA_FIFO_SIZE)
#define FREE_R_TSC_FIFO_SIZE		FREE_R_TSA_FIFO_SIZE
#define FREE_R_TSC_FIFO_START_ADDR	(FREE_R_TSB_FIFO_SIZE +  FREE_R_TSB_FIFO_SIZE)

#define FREE_R_TSC_FIFO_INT_THRD	(SPO2_TSC_FIFO_SIZE*FIFO_DEPTH_THRD_PERC/100/12*12)	// 12 Byte aligned



// ECG: ECG only, 24Bbit data width
#define ECG_FIFO_SIZE				(ZSBM800X_FIFO_SIZE - TSD_FIFO_SIZE)
#define ECG_FIFO_START_ADDR			(ZSBM800X_FIFO_ADDR)

#define ECG_FIFO_INT_THRD			(ECG_FIFO_SIZE*FIFO_DEPTH_THRD_PERC/100/3*3)	// 3 Byte aligned	

// ECG+PPG: ECG 24Bit, PPG(Green) 16Bit 
#define ECGPPG_SAMPLES_PPGDAT_SIZE	(DEFAULT_ECG_PPG_MODE_PPG_SAMPLE_FS*2)
#define ECGPPG_SAMPLES_TT_DAT_SIZE	((DEFAULT_ECG_MODE_ECG_SAMPLE_FS*3) + (DEFAULT_ECG_PPG_MODE_PPG_SAMPLE_FS*2))

#define ECGPPG_TSA_FIFO_SIZE		((ZSBM800X_FIFO_SIZE - TSD_FIFO_SIZE)*ECGPPG_SAMPLES_PPGDAT_SIZE/ECGPPG_SAMPLES_TT_DAT_SIZE/16*16)	// 16 Byte aligned
#define ECGPPG_TSA_FIFO_START_ADDR	(ZSBM800X_FIFO_ADDR)

#define ECGPPG_ECG_FIFO_SIZE		(ZSBM800X_FIFO_SIZE - ECGPPG_TSA_FIFO_SIZE - TSD_FIFO_SIZE)	// Rest all for ECG
#define ECGPPG_ECG_FIFO_START_ADDR	(ECGPPG_TSA_FIFO_START_ADDR + ECGPPG_TSA_FIFO_SIZE )

#define ECGPPG_TSA_FIFO_INT_THRD	(ECGPPG_TSA_FIFO_SIZE*FIFO_DEPTH_THRD_PERC/100/4*4)	// use ppg to trig INT


#define ABS(x) 	( (x)>0?(x):(-(x)) )
/**************************************
Note: Green always set to timeslot A 
	Ree always set to timeslot B
	IR always set to timeslot C
	IRD (used for proximity detect) always set to timeslot D
	ledchannel 1 -> Green
	ledchannel 2 -> RED
	ledchannel 3 -> IR
	ledchannel 4 -> IR(IR detect)
**************************************/
#define TIA_GAIN_CAP					TIA_12K5_CAP_8P86
#define TIA_VREF						TIA_VREF_1P26V//TIA_VREF_0P85V

#define IR_GAIN_MAX_LV					4
#define GREEN_GAIN_MAX_LV				4
#define RED_GAIN_MAX_LV					4

#define AGC_GREEN_GAIN_BEST_LV	2
#define AGC_RED_GAIN_BEST_LV	2
#define AGC_IR_GAIN_BEST_LV	2
#define AGC_IR_LED_CURREN_MIN			5
#define AGC_IR_LED_CURREN_MAX			80
#define AGC_GREEN_LED_CURREN_MIN		5
#define AGC_GREEN_LED_CURREN_MAX		100
#define AGC_RED_LED_CURREN_MIN			5 
#define AGC_RED_LED_CURREN_MAX			80

#define HRS_NORMAL_GAIN					TIA_50K_CAP_4P74
	
#define ZSBM_READ_REG16(reg, buf, num)		ZSBM800X_ReadMultyWord(reg, buf, num)
#define ZSBM_READ_FIFO8(reg, buf, num)		ZSBM800X_ReadMultyByte(reg, buf, num)
#define ZSBM_WRITE_REG16(reg, buf, num)		ZSBM800X_WriteMultyWord(reg, buf, num)

typedef struct {
	uint16_t Chip_ID;
	uint16_t Version;
}ZSBM_INFO_t;

typedef enum {  
	Z_GREEN_CH = 1, 
	Z_RED_CH = 2,  
	Z_IR_CH = 4, 
	Z_IR_DETECD_CH = 8, 
//	Z_ECG_CH = 16,
} ZSBM_LED_T;

typedef struct {  
	uint16_t REG18;
	uint16_t REG19; 
	uint16_t REG1A;  
} ZSBM_PD_CONN_T;

typedef struct {  
	uint8_t led_cur;
	uint16_t tia_cap_group; 
	uint8_t status;  
} ZSBM_LED_CFG_T;

/*
typedef enum {  
	ZSBM_OK = 0,

	ZSBM_ID_FAIL = 101, 
	ZSBM_AGC_OFFSET_FAIL,  
	ZSBM_IR_DETEC_CONTINUE,

	ZSBM_AGC_REACH_GAIN_MAX,
	ZSBM_AGC_REACH_GAIN_MIN,
	ZSBM_AGC_REACH_CURRENT_MAX,
	ZSBM_AGC_REACH_CURRENT_MIN,	
	ZSBM_AGC_CAL_CONTINUE,
	
} ZSBM_ERROR_CODE_T;
*/
typedef enum {  
	ZSBM_OK = 0,

	ZSBM_ID_FAIL = 101, 
	ZSBM_AGC_OFFSET_FAIL,  
	ZSBM_IR_DETEC_CONTINUE,

	ZSBM_AGC_REACH_GAIN_MAX,
	ZSBM_AGC_REACH_GAIN_MIN,
	ZSBM_AGC_REACH_CURRENT_MAX,
	ZSBM_AGC_REACH_CURRENT_MIN,	
	ZSBM_AGC_CAL_CONTINUE,
	ZSBM_AGC_REACH_LIMIT
	
} ZSBM_ERROR_CODE_T;

typedef enum {
	RESET_STATUS = 0,
	CLK32K_CAL,
	
	AGC_OFFSET_CAL,

	IR_DETEC_LOW_FS,

	GREEN_AGC_CAL,		// HRS & HRV
	IR_AGC_CAL,				// testonly
	RED_AGC_CAL,			// testonly

	IR_RED_AGC_CAL,		// SPO2
	IR_RED_GREEN_AGC_CAL,	// 

	IR_NORMAL,  			// testonly
	RED_NORMAL,  
	GREEN_NORMAL,  		// HRS & HRV
	IR_GREEN_NORMAL,	
	IR_RED_NORMAL,
	IR_RED_GREEN_NORMAL,
	
	ECG_NORMAL,
	ECG_GREEN_NORMAL,
	ECG_GREEN_RED_IRD_NORMAL,
	
} ZSBM_SENSOR_STATUS_T;

typedef enum {  
	NO_WEAR= 0,
	WEAR,
} ZSBM_WEAR_STATUS_T;

typedef struct {  
	ZSBM_WEAR_STATUS_T status;
	uint8_t change;  
} ZSBM_WEAR_STATUS_CHANGE_T;

extern ZSBM_SENSOR_STATUS_T zsbm_status ;

//extern ZSBM_WEAR_STATUS_CHANGE_T wearstatus;
extern uint16_t ZSBM800X_ReadFifoCount(uint16_t timeslot) ;

extern void ZSBM800X_ReadFifoData(uint8_t timeslot, uint8_t *buf, uint16_t read_fifo_size);

extern void ZSBM800X_DataHandle(void);
extern void ZSBM800X_ConmmonInit(void);
void ZSBM800X_GetLedCurrentAndTiaSetting( ZSBM_LED_T ledchannel, uint16_t *current, uint16_t *tiagain);
void ZSBM800X_SetLedCurrent( ZSBM_LED_T ledchannel, uint16_t ledcurrent);
void ZSBM800X_SetTIAGain(uint16_t tiagaingap);

ZSBM_ERROR_CODE_T ZSBM800X_Init(WORK_MODE_T mode);
extern void ZSPD800X_EcgInit(void);
extern void ZSPD800X_EcgPpgInit(void);
extern void ZSBM800X_GpioIntConfig(uint8_t en);
extern void ZSBM_IRDeteLowFs(void);

extern void ZSBM800X_DumpRegister(void);

extern ZSBM_ERROR_CODE_T ZSBM800X_32KClkOutPutToIO0( void );

extern ZSBM_ERROR_CODE_T ZSBM800X_24MClkDivideBy750OutPutToIO0( void );
//extern void ZSBM800X_CaliClkOn(void);
//void ZSBM800X_SetCaliClkValue(uint16_t value);

//extern void Reg_W_Add_Data(uint8_t add, uint16_t data);

//extern uint16_t Reg_R_Data(uint8_t add);
void test_reg();
#define CLAMP(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

typedef struct {
    int32_t ecg_data; // ECG数据
    int32_t red_data; // Red数据
    int32_t ird_data; // IRD数据
} SensorData_t;

void sensorTask(void *pvParameters);

#endif
