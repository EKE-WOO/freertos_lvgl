#ifndef _ZSBM800X_PORT_H_
#define _ZSBM800X_PORT_H_

#include <stdio.h>

#include "stm32l4xx_hal.h"
#include "zsbm800x_regcfg.h"
#include "main.h"

#define ZSBM_DEBUG	1

#define ZSBM8XXX7BITADDR			(0x5B)

#ifdef ZSBM_DEBUG
//#define  ZSPD_PRINTF(x, args ...)   printf(" [%s()]\n"  x, __func__, ## args)
#define  ZSBM_PRINTF(...)     printf(__VA_ARGS__)
#else
//#define	 ZSBM_PRINTF(x, args ...)  
#define	 ZSBM_PRINTF(...)  
#endif

#define BSWAP_16(n)		(uint16_t)(((uint16_t)(n) & 0x00ff << 8 ) | \
						(((uint16_t)(n) & 0xff00 >> 8 )))
typedef enum {    
		NULL_MODE,
		HR_MODE, 
		HRV_MODE,
		SPO2_MODE,	
		ECG_MODE,
		WEAR_MODE,		
		FT_HR_MODE,	 
		FT_HRV_MODE,		
		FT_SPO2_MODE,	

		ECG_PPG_MODE,
		TOOLING_MODE
} WORK_MODE_T;

typedef struct {    
		int16_t *ppgi;
		int16_t *ppgg;
		int16_t *ppgr;
} ZSBM_SPO2_DATA_T;


extern uint8_t zsbm_raw_dat_ird[]  ;//Êý¾Ý´æ´¢»º³åÇø
extern uint8_t zsbm_raw_dat_green[];
extern uint8_t zsbm_raw_dat_red[];
extern uint8_t zsbm_raw_dat_ir[];

extern uint8_t zsbm_raw_dat_ecg[];

extern volatile uint8_t zsbm_int_flag;

extern void ZSBM800X_DelayMs(uint32_t ms);
extern uint16_t ZSBM800X_ReadMultyWord (uint8_t regaddr , uint16_t *rcvbuf, uint16_t length) ;
extern uint16_t ZSBM800X_ReadMultyByte (uint8_t regaddr , uint8_t *rcvbuf, uint16_t length) ;
extern uint16_t ZSBM800X_WriteMultyWord (uint8_t regaddr , uint16_t *regdat, uint16_t length) ;
extern void ZSBM800X_GpioIntEnable(void) ;
extern void ZSBM800X_GpioIntDisable(void) ;

#endif
