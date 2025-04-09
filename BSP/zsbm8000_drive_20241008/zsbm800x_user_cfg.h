#ifndef _ZSBM800X_USER_CFG_H_
#define _ZSBM800X_USER_CFG_H_

#include <stdbool.h>
#include "zsbm800x_regcfg.h"
#include "zsbm800X_port.h"
#include "zsbm800x_drv.h"
//#include "Public.h"
//#include "zsbm800X_ppg.h"
#include "zsbm800X_ecg_ra.h"

////////////////////////////
////////////////////////////
// 以下配置与硬件连接强相关，请务必正确配置
// 以下配置与硬件连接强相关，请务必正确配置
// 以下配置与硬件连接强相关，请务必正确配置
////////////////////////////
////////////////////////////

// 配置使用的PD个数，最多8个PD，
#define USED_PD_NUMBER		1	// Max support 8x PDs

//// LED 灯的连接配置，需与实际硬件保持一致
////// 1x4437/3227, 6 LEDs
#define LED_GREEN_USED_DRV_PINS			(LED_DRV_PIN0 | LED_DRV_PIN9)
#define LED_RED_USED_DRV_PINS			(LED_DRV_PIN1 | LED_DRV_PIN10)
//#define LED_RED_USED_DRV_PINS			(LED_DRV_PIN0 | LED_DRV_PIN9)
#define LED_IR_USED_DRV_PINS			(LED_DRV_PIN2 | LED_DRV_PIN11)//(LED_DRV_PIN2 | LED_DRV_PIN9)

//// new 4437 6 led board
//#define LED_GREEN_USED_DRV_PINS		(LED_DRV_PIN0 | LED_DRV_PIN1 | LED_DRV_PIN6 | LED_DRV_PIN7)
//#define LED_RED_USED_DRV_PINS			(LED_DRV_PIN10 )
//#define LED_IR_USED_DRV_PINS			(LED_DRV_PIN11 )

////// 8001, 6 LEDs
//#define LED_GREEN_USED_DRV_PINS			(LED_DRV_PIN1 | LED_DRV_PIN9)
//#define LED_RED_USED_DRV_PINS			(LED_DRV_PIN2 | LED_DRV_PIN10)
//#define LED_IR_USED_DRV_PINS			(LED_DRV_PIN3 | LED_DRV_PIN11)//(LED_DRV_PIN2 | LED_DRV_PIN9)

////// 4x 3227, 5 LEDs
//#define LED_GREEN_USED_DRV_PINS			(LED_DRV_PIN0 | LED_DRV_PIN1 | LED_DRV_PIN6 | LED_DRV_PIN7)
//#define LED_RED_USED_DRV_PINS			(LED_DRV_PIN10 )
//#define LED_IR_USED_DRV_PINS			(LED_DRV_PIN9 )//| LED_DRV_PIN0 | LED_DRV_PIN1 | LED_DRV_PIN6 | LED_DRV_PIN7)//(LED_DRV_PIN2 | LED_DRV_PIN9)

////// 2x 4437, 3 LEDs
//#define LED_GREEN_USED_DRV_PINS		(LED_DRV_PIN2 | LED_DRV_PIN7)
//#define LED_RED_USED_DRV_PINS			(LED_DRV_PIN1 )
//#define LED_IR_USED_DRV_PINS			(LED_DRV_PIN9 )//| LED_DRV_PIN1 )//(LED_DRV_PIN2 | LED_DRV_PIN9)

//////// 2x 4437, 2g LED+ 2R&IR LEDs BJ
//#define LED_GREEN_USED_DRV_PINS		(LED_DRV_PIN0 | LED_DRV_PIN1 | LED_DRV_PIN6 | LED_DRV_PIN7)
//#define LED_RED_USED_DRV_PINS			(LED_DRV_PIN10 )
//#define LED_IR_USED_DRV_PINS			(LED_DRV_PIN11 )//| LED_DRV_PIN1 )//(LED_DRV_PIN2 | LED_DRV_PIN9)

//// 1x 3227, 3X LEDs, YUANYU
//#define LED_GREEN_USED_DRV_PINS		(LED_DRV_PIN0 | LED_DRV_PIN7)
//#define LED_RED_USED_DRV_PINS			(LED_DRV_PIN6 )
//#define LED_IR_USED_DRV_PINS			(LED_DRV_PIN2 )

////// HARMAN, 3IN1 LED, pd connect to VC2-IN2
//#define LED_GREEN_USED_DRV_PINS			(LED_DRV_PIN1)
//#define LED_RED_USED_DRV_PINS			(LED_DRV_PIN6)
//#define LED_IR_USED_DRV_PINS			(LED_DRV_PIN2)


//////// HARMAN, 4IN1 LED, pd connect to VC2-IN2
//#define LED_GREEN_USED_DRV_PINS			(LED_DRV_PIN1 | LED_DRV_PIN7)
//#define LED_RED_USED_DRV_PINS			(LED_DRV_PIN9)
//#define LED_IR_USED_DRV_PINS			(LED_DRV_PIN2)//LED_DRV_PIN1)	// pin2 IRD, PIN9 RED

////////////////////////////
////////////////////////////
// 以下配置与产品定义相关，请谨慎配置
// 以下配置与产品定义相关，请谨慎配置
// 以下配置与产品定义相关，请谨慎配置
////////////////////////////
////////////////////////////

// PPG/ECG模式下是否保持IR接近检测全程开启，默认开启
#define LED_IR_ALWAYS_ON	0	// Use TSD， default onle use 1st PD, always 16bit data width

// 各测量模式下的默认的多pulse数量，如果pulse数大于1，将会开启32位数据位宽
#define DEFAULT_HR_MODE_GREEN_PULSE			4							// if > 1, will enable 24 bit data width
#define DEFAULT_HRV_MODE_GREEN_PULSE		DEFAULT_HR_MODE_PULSE		// if > 1, will enable 24 bit data width
#define DEFAULT_SPO2_MODE_IR_RED_PULSE		16							// if > 1, will enable 24 bit data width

// 各测量模式下的默认的采样率。如果全程开启IR接近检测（LED_IR_ALWAYS_ON = 1），TSD会根据当前采样率计算并配置实际不高于15Hz采样率
#define DEFAULT_AGC_CAL_SAMPLE_FS			SAMPLE_FS_100HZ
#define DEFAULT_HR_MODE_SAMPLE_FS			SAMPLE_FS_25HZ	// use GREEN leds
#define DEFAULT_HRV_MODE_SAMPLE_FS			SAMPLE_FS_100HZ	// use GREEN leds
#define DEFAULT_SPO2_MODE_SAMPLE_FS			SAMPLE_FS_100HZ	// use RED+IR leds

// 数据搜集模式，三组LED同时开启，调光完成后关闭IR接近检测
#define DEFAULT_FREE_RUN_MODE_SAMPLE_FS			SAMPLE_FS_200HZ	// use GREEN+RED+IR leds

#define DEFAULT_ECG_MODE_ECG_SAMPLE_FS		SAMPLE_ECG_FS_300HZ
// ECG + PPG模式支持开启绿光，但该模式下PPG不支持多pulse，若需提升信号质量请提升采样率
#define DEFAULT_ECG_PPG_MODE_PPG_SAMPLE_FS	SAMPLE_FS_50HZ

////////////////////////////
// 以下配置与结构光路相关，请谨慎配置
////////////////////////////
#define AGC_PPG_THRESHOLD_LV1				13107
#define AGC_PPG_THRESHOLD_LV2				24576//18000
#define AGC_PPG_THRESHOLD_LV3				29491//25000
#define AGC_PPG_THRESHOLD_LV4				32767

#define AGC_CAL_IR_INIT_CURR				40
#define AGC_CAL_IR_TIA_GAIN_CAP				TIA_25K_CAP_6P32

#define IR_DETEC_THRESHOLD_MIN				3000
#define IR_DETEC_THRESHOLD_MAX				10000

//////////////////////////////////////////////////////////////
// 删除TIA相关配置，新增电流调节参数
#define AGC_PPG_TARGET_LEVEL      25000   // 目标信号幅度中心值
#define AGC_CURRENT_STEP          1       // 电流调节步长
#define AGC_CURRENT_MAX           100     // 最大允许电流值
#define AGC_CURRENT_MIN           5       // 最小允许电流值

// 删除原有TIA配置
// #define AGC_CAL_IR_TIA_GAIN_CAP  TIA_25K_CAP_6P32  // 已废弃
// 采样率配置
#define DEFAULT_HR_MODE_SAMPLE_FS     SAMPLE_FS_25HZ
#define DEFAULT_SPO2_MODE_SAMPLE_FS   SAMPLE_FS_100HZ

#endif
