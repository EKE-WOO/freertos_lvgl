#include <stdio.h>
#include <string.h>
#include "zsbm800x_drv.h"
#include "zsbm800x_user_cfg.h"
#include "math.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "queue.h"
#include "semphr.h"


//extern char new_filename[MAX_FILENAME_LENGTH];
//#include "lcd_drv.h"

ZSBM_SENSOR_STATUS_T zsbm_status = RESET_STATUS ;
WORK_MODE_T workmode ;

volatile uint8_t zsbm_int_flag = 0;
uint16_t zsbm_clk_frq = 32000;

WORK_MODE_T workmode ;
static const uint16_t gains[8] = {TIA_12K5_CAP_8P86,TIA_25K_CAP_6P32,TIA_50K_CAP_4P74,TIA_100K_CAP_3P16,\
																	TIA_200K_CAP_1P58,TIA_400K_CAP_960F,TIA_800K_CAP_480F,TIA_1M6_CAP_320F};

//int32_t rawforalgin[200];
volatile uint16_t ppgdatready = 0;
uint8_t zsbm_raw_dat_ird[TSD_FIFO_SIZE] = {0} ;//???????
uint8_t zsbm_raw_dat_green[1024];
uint8_t zsbm_raw_dat_red[1024];
uint8_t zsbm_raw_dat_ir[1024];

uint8_t zsbm_raw_dat_ecg[1024];

extern int32_t TEST_BIN_ADDR[];
extern uint32_t TEST_BIN_SIZE;
extern uint8_t *zspd_algo_buffer;
extern void usb_printf(const char *format, ...) ;

ZSBM_LED_CFG_T led_cfg[3] = { \
														{AGC_IR_LED_CURREN_MAX,  IR_GAIN_MAX_LV - 1, 0}, \
														{AGC_GREEN_LED_CURREN_MAX,  GREEN_GAIN_MAX_LV - 1, 0}, \
														{AGC_RED_LED_CURREN_MAX,  RED_GAIN_MAX_LV - 1, 0} \
														} ;

void ZSBM800X_SoftReset(void)
{
	uint16_t tmp = SOFT_RESET ;
	zsbm_status = RESET_STATUS;
//	ZSBM_WRITE_REG16(0x00, &tmp, 1) ;
	tmp = 0x80 ;
	ZSBM_WRITE_REG16(0x02, &tmp, 1) ;
//	ZSBM800X_DelayMs(500) ;
}

/*clear fifo*/
void ZSBM800X_ClearFifo(void)
{
//	uint16_t tmp = FIFO_CLR ;
//	ZSBM_WRITE_REG16(0x00, &tmp, 1);
}

ZSBM_ERROR_CODE_T ZSBM800X_IDCheck(void)
{
    uint8_t retry = 5 ; 
		uint16_t  reg_value[2] ;
    while(retry--)
		{
			ZSBM_READ_REG16(0, reg_value, 2) ;
			if (reg_value[0] == 0x410) 
			{
				ZSBM_PRINTF("Chip Version = %4x!\n", reg_value[1]) ;
				return ZSBM_OK;			
			}	
		}
		ZSBM_PRINTF("Chip ID check fail %4x!\n", reg_value[0]) ;
    return ZSBM_ID_FAIL;
}

/**
*function:Enable 32K OSC and Set sample rate
*parameter: sample rate;
*return:none;
*date:2022/04/11
**/
void ZSBM800X_SetSampleRate(uint16_t samplerate) 
{
	uint16_t tmp[3];
	
	/*open OSC32K and stop all timeslots */
	tmp[0] = OSC32K_EN ;		// reg 0x02

	/*set time slot cycle according to sample rate */
	tmp[1] = (zsbm_clk_frq / samplerate ) >> 16 ;			// reg 0x03
	tmp[2] = (zsbm_clk_frq / samplerate ) & 0xFFFF ;		// reg 0x04
	ZSBM_WRITE_REG16(0x02, tmp, 3) ;
	return ;
}

/**
*function:Time slot sel
*parameter: timeslot;
*return:none;
*date:2022/04/11
**/
void ZSBM800X_SelectTimeSlot( uint8_t timeslot)
{
	uint16_t tmp;
	tmp = timeslot ;
	
	ZSBM_WRITE_REG16(0x0F, &tmp, 1);
}

/**
*function:Set ZSBM FIFO mode/depth
*parameter: depth;
*return:none;
*date:2022/04/11
**/
void ZSBM800X_SetFifoAdrSize(uint16_t addr, uint16_t size)
{
	uint16_t tmp;
	tmp = addr;							
	ZSBM_WRITE_REG16(0x2d, &tmp, 1);	// fifo start address
	tmp = size - 1;	
	ZSBM_WRITE_REG16(0x2e, &tmp, 1);	// fifo total size	
}

/**
*function:Set ZSPD FIFO mode/depth
*parameter: depth;
*return:none;
*date:2022/04/11
**/
void ZSBM800X_SetFifoDepth( uint16_t depth)
{
	uint16_t tmp;
	
//	/*auto clear fifo after reading, overwrite old data when the fifo full*/
//	tmp = INT_FIFO_THRHD_EN | INT_ACLEAR_FIFO_EN|FIFO_OF_CONTINUE_WR_NEW ;
//	ZSBM_WRITE_REG16(0x06, &tmp, 1);

	/*config FIFO interupt threshold*/
	tmp = ((depth & 0x03FF) -1) | INT_FIFO_DEPTH_THRD_EN ;
	ZSBM_WRITE_REG16(0x2f, &tmp, 1);
}

/**
*function:config fifo word size
*parameter: depth;
*return:none;
*date:2022/04/11
**/
void ZSBM800X_SetFifoWordSizeEnPds(uint16_t fifowordsize, uint8_t pdchannels)
{
	uint16_t tmp;
	switch(pdchannels)
	{
		case 8:
			tmp = fifowordsize | 0xff;
			break;
		
		case 7:
			tmp = fifowordsize | 0x7f;
			break;
		
		case 6:
			tmp = fifowordsize | 0x3f;
			break;
		
		case 5:
			tmp = fifowordsize | 0x1f;
			break;
		
		case 4:
			tmp = fifowordsize | 0x0f;
			break;
		
		case 3:
			tmp = fifowordsize | 0x07;
			break;
		
		case 2:
			tmp = fifowordsize | 0x06;
			break;
		
		case 1:
		default:
			tmp = fifowordsize | 0x02;
			break;
	}	
	ZSBM_WRITE_REG16(0x10, &tmp, 1);
}

/**
*function:config number of repeat tsx
*parameter: repeattsx;
*return:none;
*date:2022/04/11
**/
void ZSBM800X_SetSubPeriod_NumIntTsx_NumRepeatTsx(uint8_t subperiod, uint8_t numinttsx, uint8_t numberrepeattsx)
{
	uint16_t tmp;
	tmp = ((subperiod & 0xf ) << 12) ;
	tmp |= ((numinttsx & 0xf ) << 8 );
	tmp |= (numberrepeattsx & 0xff);
	ZSBM_WRITE_REG16(0x23, &tmp, 1);
}
/**
*function:Set ZSBM TIA_GAIN
*parameter: depth;
*return:none;
*date:2022/04/11
**/
void ZSBM800X_SetTIAGain(uint16_t tiagaingap)
{
	uint16_t tmp;
	
	tmp = (tiagaingap & 0xFF70) | TIA_VREF | TIA_EN;
	ZSBM_WRITE_REG16(0x27, &tmp, 1);
}


/**
*function:Set ZSBM TIA_VREF
*parameter: depth;
*return:none;
*date:2022/04/11
**/
void ZSBM800X_SetVcSel(uint16_t vcsel)
{
	uint16_t tmp;
	
	tmp = vcsel ;
//	ZSBM_WRITE_REG16(0x25, &tmp, 1);
}

/**
*function:Enable time slot and start Cycle
*parameter: timeslot;
*return:none;
*date:2022/04/11
**/
void ZSBM800X_TimerSlotStart( uint16_t timeslot)
{
	uint16_t tmp;
	if(timeslot & 0x0100) // ECG channel, align with Reg 0x0f
		tmp = (timeslot & 0x000F) | OP_EN | OSC32K_EN | ECG_EN;
	else
		tmp = (timeslot & 0x000F) | OP_EN | OSC32K_EN ;

	/*enable time slot, go!*/
	ZSBM_WRITE_REG16(0x02, &tmp, 1);
}

/**
*function:Config ZSBM GPIO Interrupt output
*parameter: gpiointoutputmode;
*return:none;
*date:2022/04/11
**/
void ZSBM800X_ConfigIntOutputPin( uint16_t gpiointoutputmode)
{
	uint16_t tmp;
	
	/*GPIO0 set push-pull output, interrupt output, active low, low drive strength*/
//	tmp = GPIO0_CFG_OUT_PP | GPIO0_POL_NEG | GPIO0_DS_12MA | GPIO0_OUT_SOURCE_INT;
	tmp = gpiointoutputmode;	
	ZSBM_WRITE_REG16(0x0b, &tmp, 1);
}

/**
*function:Config ZSBM PD Connection
*parameter: PDconnect;
*return:none;
*date:2022/04/11
**/
void ZSBM800X_ConfigPDConnection( ZSBM_PD_CONN_T *PDconnect)
{
//	uint16_t *tmp;
//	tmp = (uint16_t *)PDconnect ;
	uint16_t xxxx = 0x4343;	// 先全部按单端配置
	ZSBM_WRITE_REG16(0x19, &xxxx, 1);
//	xxxx = 0x0043 ;
	ZSBM_WRITE_REG16(0x1a, &xxxx, 1);
	
	xxxx = 0x4141;
	ZSBM_WRITE_REG16(0x1b, &xxxx, 1);
//	xxxx = 0x0041 ;
	ZSBM_WRITE_REG16(0x1c, &xxxx, 1);
	xxxx = 0x0FF0;
	ZSBM_WRITE_REG16(0x25, &xxxx, 1);	
}

/**
*function:Set LED current
*parameter: led channel , led current;
Note: Green always set to timeslot A
			Red always set to timeslot B
			IR always set to timeslot C
*return:none;
*date:2022/04/11
**/
void ZSBM800X_SetLedCurrent( ZSBM_LED_T ledchannel, uint16_t ledcurrent)
{
	uint16_t tmp;

	switch(ledchannel)
	{
		case Z_GREEN_CH :
			tmp = TSA_REG_SEL ;
			ZSBM_WRITE_REG16(0x0f, &tmp, 1);
			if(LED_GREEN_USED_DRV_PINS & 0x003f)	// led drv0
			{
				tmp = (ledcurrent & 0x007F) | ((LED_GREEN_USED_DRV_PINS & 0x003f)<<8);
				ZSBM_WRITE_REG16(0x14, &tmp, 1);	
			}				
			if(LED_GREEN_USED_DRV_PINS & 0x3f00)	// led drv1
			{
				tmp = (ledcurrent & 0x007F) | ((LED_GREEN_USED_DRV_PINS & 0x3f00)<<0);
				ZSBM_WRITE_REG16(0x15, &tmp, 1);
			}
//		tmp = (ledcurrent & 0x007F) | ((2 & 0x003f)<<8);
//		ZSBM_WRITE_REG16(0x14, &tmp, 1);	
//		tmp = (0 & 0x007F) | ((3 & 0x003f)<<8);
//		ZSBM_WRITE_REG16(0x15, &tmp, 1);
		break;
		
		case Z_RED_CH :
			tmp = TSB_REG_SEL ;
			ZSBM_WRITE_REG16(0x0f, &tmp, 1);
			if(LED_RED_USED_DRV_PINS & 0x003f)	// led drv0
			{
				tmp = (ledcurrent & 0x007F) | ((LED_RED_USED_DRV_PINS & 0x003f)<<8);
				ZSBM_WRITE_REG16(0x14, &tmp, 1);	
			}				
			if(LED_RED_USED_DRV_PINS & 0x3f00)	// led drv1
			{
				tmp = (ledcurrent & 0x007F) | ((LED_RED_USED_DRV_PINS & 0x3f00)<<0);
				ZSBM_WRITE_REG16(0x15, &tmp, 1);
			}
//			tmp = (0 & 0x007F) | ((3 & 0x003f)<<8);
//			ZSBM_WRITE_REG16(0x14, &tmp, 1);	
//			tmp = (ledcurrent & 0x007F) | ((1 & 0x003f)<<8);
//			ZSBM_WRITE_REG16(0x15, &tmp, 1);
		break;
		
		case Z_IR_CH :
			tmp = TSC_REG_SEL ;
			ZSBM_WRITE_REG16(0x0f, &tmp, 1);
			if(LED_IR_USED_DRV_PINS & 0x003f)	// led drv0
			{
				tmp = (ledcurrent & 0x007F) | ((LED_IR_USED_DRV_PINS & 0x003f)<<8);
				ZSBM_WRITE_REG16(0x14, &tmp, 1);	
			}				
			if(LED_IR_USED_DRV_PINS & 0x3f00)	// led drv1
			{
				tmp = (ledcurrent & 0x007F) | ((LED_IR_USED_DRV_PINS & 0x3f00)<<0);
				ZSBM_WRITE_REG16(0x15, &tmp, 1);
			}
//		tmp = (ledcurrent & 0x007F) | ((2 & 0x003f)<<8);
//		ZSBM_WRITE_REG16(0x14, &tmp, 1);	
//		tmp = (0 & 0x007F) | ((3 & 0x003f)<<8);
//		ZSBM_WRITE_REG16(0x15, &tmp, 1);
		break;		

		case Z_IR_DETECD_CH :
			tmp = TSD_REG_SEL ;
			ZSBM_WRITE_REG16(0x0f, &tmp, 1);
			if(LED_IR_USED_DRV_PINS & 0x003f)	// led drv0
			{
				tmp = (ledcurrent & 0x007F) | ((LED_IR_USED_DRV_PINS & 0x003f)<<8);
				ZSBM_WRITE_REG16(0x14, &tmp, 1);	
			}				
			if(LED_IR_USED_DRV_PINS & 0x3f00)	// led drv1
			{
				tmp = (ledcurrent & 0x007F) | ((LED_IR_USED_DRV_PINS & 0x3f00)<<0);
				ZSBM_WRITE_REG16(0x15, &tmp, 1);
			}
//		tmp = (ledcurrent & 0x007F) | ((2 & 0x003f)<<8);
//		ZSBM_WRITE_REG16(0x14, &tmp, 1);	
//		tmp = (0 & 0x007F) | ((3 & 0x003f)<<8);
//		ZSBM_WRITE_REG16(0x15, &tmp, 1);
		break;	

		default:
			break;
	}
}

/**
*function:Get LED current and tia setting
*parameter: led channel , led current, tia setting;
Note: Green always set to timeslot A
			Red always set to timeslot B
			IR always set to timeslot C
*return:none;
*date:2022/12/20
**/
void ZSBM800X_GetLedCurrentAndTiaSetting( ZSBM_LED_T ledchannel, uint16_t *current, uint16_t *tiagain)
{
	uint16_t tmp, cur14, cur15, tia;

	switch(ledchannel)
	{
		case Z_GREEN_CH :
			tmp = TSA_REG_SEL ;
			ZSBM_WRITE_REG16(0x0f, &tmp, 1);
			ZSBM_READ_REG16(0x14, &cur14, 1);			
			ZSBM_READ_REG16(0x15, &cur15, 1);
			ZSBM_READ_REG16(0x27, &tia, 1);

			if(LED_GREEN_USED_DRV_PINS & 0x003f)
			{
				cur14 &= 0x7F ;
				if(cur14<16)
					cur14 = cur14 ;
				else
					cur14 = (cur14-16) * 19 / 10 + 16 ;
			}
			else
				cur14 = 0;
			

			if(LED_GREEN_USED_DRV_PINS & 0x3f00)
			{
				cur15 &= 0x7F ;
				if(cur15<16)
					cur15 = cur15 ;
				else
					cur15 = (cur15-16) * 19 / 10 + 16 ;
			}
			else
				cur15 = 0;
				
			break;
		
		case Z_RED_CH :
			tmp = TSB_REG_SEL ;
			ZSBM_WRITE_REG16(0x0f, &tmp, 1);
			ZSBM_READ_REG16(0x14, &cur14, 1);			
			ZSBM_READ_REG16(0x15, &cur15, 1);
			ZSBM_READ_REG16(0x27, &tia, 1);

			if(LED_RED_USED_DRV_PINS & 0x003f)
			{
				cur14 &= 0x7F ;
				if(cur14<16)
					cur14 = cur14 ;
				else
					cur14 = (cur14-16) * 19 / 10 + 16 ;
			}
			else
				cur14 = 0;
			

			if(LED_RED_USED_DRV_PINS & 0x3f00)
			{
				cur15 &= 0x7F ;
				if(cur15<16)
					cur15 = cur15 ;
				else
					cur15 = (cur15-16) * 19 / 10 + 16 ;
			}
			else
				cur15 = 0;
			
			break;
		
		case Z_IR_CH :
			tmp = TSC_REG_SEL ;
			ZSBM_WRITE_REG16(0x0f, &tmp, 1);
			ZSBM_READ_REG16(0x14, &cur14, 1);			
			ZSBM_READ_REG16(0x15, &cur15, 1);
			ZSBM_READ_REG16(0x27, &tia, 1);

			if(LED_IR_USED_DRV_PINS & 0x003f)
			{
				cur14 &= 0x7F ;
				if(cur14<16)
					cur14 = cur14 ;
				else
					cur14 = (cur14-16) * 19 / 10 + 16 ;
			}
			else
				cur14 = 0;
			

			if(LED_IR_USED_DRV_PINS & 0x3f00)
			{
				cur15 &= 0x7F ;
				if(cur15<16)
					cur15 = cur15 ;
				else
					cur15 = (cur15-16) * 19 / 10 + 16 ;
			}
			else
				cur15 = 0;

			break;	
		
		default:
			break;
	}
	
	*current = cur14 + cur15 ;
	*tiagain = tia & 0xFF70 ;
	
	ZSBM_PRINTF("CURRENT = %d, TIA = 0x%x\n", *current, *tiagain) ;
}

/**
*function:read fifo data number
*parameter:none;
*return:fifo_num_temp:fifo data size;
*date:2022/04/11
**/
uint16_t ZSBM800X_ReadFifoCount(uint16_t timeslot) 
{
	uint16_t fifo_num_temp;
	switch(timeslot)
	{
		case TSA_REG_SEL:
			ZSBM_READ_REG16(0x51, &fifo_num_temp, 1);
			break;
		
		case TSB_REG_SEL:
			ZSBM_READ_REG16(0x52, &fifo_num_temp, 1);
			break;

		case TSC_REG_SEL:
			ZSBM_READ_REG16(0x53, &fifo_num_temp, 1);
			break;
			
		case TSD_REG_SEL:
			ZSBM_READ_REG16(0x54, &fifo_num_temp, 1);
			break;
			
		case ECG_REG_SEL:
			ZSBM_READ_REG16(0x50, &fifo_num_temp, 1);
			break;
	}
	return (fifo_num_temp&0x03FF);	//
}

/**
*function: read fifo data
*parameter:read_fifo_size,fifo size
*parameter:buf,read fifo data first address
*return:none
*date:2022/07/04
**/
void ZSBM800X_ReadFifoData(uint8_t timeslot, uint8_t *buf, uint16_t read_fifo_size)
{
	switch(timeslot)
	{
		case TSA_REG_SEL:
			ZSBM_READ_FIFO8(0x61, buf, read_fifo_size) ;
			break;
		
		case TSB_REG_SEL:
			ZSBM_READ_FIFO8(0x62, buf, read_fifo_size) ;
			break;

		case TSC_REG_SEL:
			ZSBM_READ_FIFO8(0x63, buf, read_fifo_size) ;
			break;
			
		case TSD_REG_SEL:
			ZSBM_READ_FIFO8(0x64, buf, read_fifo_size) ;
			break;
			
		case ECG_REG_SEL:
			ZSBM_READ_FIFO8(0x60, buf, read_fifo_size) ;
			break;
	}
}

/**
*function:clear fifo and interrupt_flg
*parameter:none
*return:none
*date:2022/05/26
**/
void ZSBM800X_ClearIntAndFifo(void) 
{
	uint8_t size, tmp[200];
	
	size = ZSBM800X_ReadFifoCount(TSA_REG_SEL) ;
	if(size)
		ZSBM800X_ReadFifoData(TSA_REG_SEL, tmp, size) ;
	
	size = ZSBM800X_ReadFifoCount(TSB_REG_SEL) ;
	if(size)
		ZSBM800X_ReadFifoData(TSB_REG_SEL, tmp, size) ;
	
	size = ZSBM800X_ReadFifoCount(TSC_REG_SEL) ;
	if(size)
		ZSBM800X_ReadFifoData(TSC_REG_SEL, tmp, size) ;
	
	size = ZSBM800X_ReadFifoCount(TSD_REG_SEL) ;
	if(size)
		ZSBM800X_ReadFifoData(TSD_REG_SEL, tmp, size) ;
	
	size = ZSBM800X_ReadFifoCount(ECG_REG_SEL) ;
	if(size)
		ZSBM800X_ReadFifoData(ECG_REG_SEL, tmp, size) ;
}


/**
*function:gipo interrupt config
*parameter:	en,	!=0 -> enable; 0 -> disable
*return:none
*date:2022/01/21
**/
void ZSBM800X_GpioIntConfig(uint8_t en)
{
//	uint16_t tmp;
	
	// enable/disable host CPU gpio interrupt
	if(en)
		ZSBM800X_GpioIntEnable();
	else
		ZSBM800X_GpioIntDisable();

	// clear ZSBM800X fifo count & fifo data
	/*
	tmp = INT_FIFO_THRHD_CLR ;
	ZSBM_WRITE_REG16(0x08, &tmp, 1) ;
	tmp = FIFO_CLR ;
	ZSBM_WRITE_REG16(0x00, &tmp, 1);
	*/
}

/**
*function:heart rate ppg adc offset
*parameter:none
*return:0,success
*date:2022/04/16
**/
ZSBM_ERROR_CODE_T ZSBM800X_SetPpgAdcOffset(uint8_t length, int16_t *buf)
{
	uint8_t i;
	// time slot A + B + C enabled
	for(i=3; i > 0; i--)
	{
		ZSBM800X_SelectTimeSlot(0x1<<(3-i)) ;
		ZSBM_WRITE_REG16(0x22, (uint16_t *)&buf[length-1-i], 1);
		return ZSBM_OK;
	}
	return ZSBM_AGC_OFFSET_FAIL;
}

__inline uint16_t ZSBM800X_ReadIntStatus(void)
{
	uint16_t tmp;
	ZSBM_READ_REG16(0x06, &tmp, 1) ;
	return tmp;
}

uint16_t ZSBM800X_ReadECGIntStatus(void)
{
	uint16_t tmp;
	ZSBM_READ_REG16(0x07, &tmp, 1) ;
	return tmp;
}

uint16_t ZSBM800X_ClrECGRLDGlobalIntbit(void)
{
	uint16_t tmp = (1<<3);
	ZSBM_WRITE_REG16(0x07, &tmp, 1) ;
	return tmp;
}

uint16_t ZSBM800X_ReadECGRLDStatus(void)
{
	uint16_t tmp;
	ZSBM_READ_REG16(0x3a, &tmp, 1) ;
	return tmp&0x1E00;
}
/**
*function:heart rate ppg adc offset
*parameter:none
*return:0,success
*date:2022/04/16
**/
void ZSBM800X_CommonInit(void)
{
	ZSBM_ERROR_CODE_T ret;
	ZSBM_PD_CONN_T PDconnect ;
	
	zsbm_status = RESET_STATUS;
	// can remove this softreset
	ZSBM800X_SoftReset() ;
//	ZSBM800X_DelayMs(500) ;
	ZSBM800X_GpioIntConfig(0);	// Disable GPIO IRQ
	
	ret = ZSBM800X_IDCheck();
	if(ret != ZSBM_OK)
		ZSBM_PRINTF("ZSBM800X ID check fail!\n") ;
	
//	ZSBM800X_SetSampleRate(SAMPLE_FS_10HZ) ;	// start with IR @ 10Hz
	ZSBM800X_ConfigIntOutputPin( GPIO0_CFG_OUT_PP | GPIO0_POL_NEG | GPIO0_OUT_SOURCE_LOW | GPIO0_OUT_SOURCE_INT) ;
//	ZSBM800X_SetFifoDepth(FIFO_DEPTH_AGC_CAL_IR_DETECT) ;	
/*
	
	ZSBM800X_SetLedCurrent(Z_GREEN_CH, 0) ;
	ZSBM800X_SetLedCurrent(Z_RED_CH, 0) ;
	ZSBM800X_SetLedCurrent(Z_IR_CH, 0) ;
*/	
	
	ZSBM800X_SelectTimeSlot	(TSA_REG_SEL | TSB_REG_SEL  | TSC_REG_SEL  | TSD_REG_SEL) ;
	
	uint16_t tmp = 0x4606;
	ZSBM_WRITE_REG16(0x0C, &tmp, 1) ;
	tmp = 0x3E00;
	ZSBM_WRITE_REG16(0x0E, &tmp, 1) ;	
//	tmp = 0x13FF;
//	ZSBM_WRITE_REG16(0x18, &tmp, 1) ;	
//	tmp = 0xFF00;
//	ZSBM_WRITE_REG16(0x1F, &tmp, 1) ;
	tmp = 0x1000;
	ZSBM_WRITE_REG16(0x39, &tmp, 1) ;
	
	/*config fifo word size*/
//	ZSBM800X_SetFifoWordSizeEnPds(FIFO_WORD_SIZE_16, USED_PD_NUMBER);
//	ZSBM800X_SetSubPeriod_NumIntTsx_NumRepeatTsx	(9, 0, 0) ;
	
	/* PD connection config */
//#ifdef USE_INTERNAL_PD
//	PDconnect.REG18 = 0x4207;
////	PDconnect.REG19 = 0x1111;
////	PDconnect.REG1A = 0x1111;
//#else
//	PDconnect.REG18 = 0x0007;
//	PDconnect.REG19 = PD0_CONNECTION;
//	PDconnect.REG1A = PD1_CONNECTION;
//#endif	

	ZSBM800X_ConfigPDConnection(&PDconnect);
	ZSBM800X_SetFifoWordSizeEnPds(FIFO_WORD_SIZE_16, USED_PD_NUMBER);
	
	/*TIA gain = TIA_GAIN, feedback cap = TIA_GAIN_CAP, TIA Vref = TIA_VREF, TIA enable*/
	ZSBM800X_SetTIAGain(TIA_GAIN_CAP);
	
	/*VC = TIA_VREF + 250mV*/
	ZSBM800X_SetVcSel(VC0_SEL_TIA_VREF_250 | VC1_SEL_TIA_VREF_250);	

	ZSBM_PRINTF("ZSBM800X_CommonInit Done!\n") ;

/*
	ZSBM800X_ClearFifo();
	ZSBM800X_TimerSlotStart(TSA_EN | TSB_EN | TSC_EN);	
	
	zsbm_satus = AGC_OFFSET_CAL ;
*/
	ZSBM800X_GpioIntConfig(1);	// Enable GPIO IRQ
}

void ZSBM_IRDeteLowFs(void)
{
	ZSBM800X_SetSampleRate(SAMPLE_FS_10HZ) ;	// start with IR @ 5Hz
	
	ZSBM800X_SetLedCurrent(Z_GREEN_CH, 0) ;
	ZSBM800X_SetFifoWordSizeEnPds(FIFO_WORD_SIZE_16, USED_PD_NUMBER);
	ZSBM800X_SetSubPeriod_NumIntTsx_NumRepeatTsx	(0, 0, 0) ;

	ZSBM800X_SetLedCurrent(Z_RED_CH, 0) ;
	ZSBM800X_SetFifoWordSizeEnPds(FIFO_WORD_SIZE_16, USED_PD_NUMBER);
	ZSBM800X_SetSubPeriod_NumIntTsx_NumRepeatTsx	(0, 0, 0) ;

	
	ZSBM800X_SetLedCurrent(Z_IR_CH, AGC_CAL_IR_INIT_CURR) ;
	ZSBM800X_SetTIAGain(AGC_CAL_IR_TIA_GAIN_CAP);
	ZSBM800X_SetFifoWordSizeEnPds(FIFO_WORD_SIZE_16, USED_PD_NUMBER);
	ZSBM800X_SetSubPeriod_NumIntTsx_NumRepeatTsx	(0, 0, 0) ;

	ZSBM800X_SetLedCurrent(Z_IR_DETECD_CH, AGC_CAL_IR_INIT_CURR) ;
	ZSBM800X_SetTIAGain(AGC_CAL_IR_TIA_GAIN_CAP);
	ZSBM800X_SetFifoWordSizeEnPds(FIFO_WORD_SIZE_16, USED_PD_NUMBER);
	ZSBM800X_SetSubPeriod_NumIntTsx_NumRepeatTsx	(0, 0, 0) ;
	ZSBM800X_SetFifoAdrSize(TSD_FIFO_START_ADDR, TSD_FIFO_SIZE) ;
	ZSBM800X_SetFifoDepth(16) ;	
	
//	ZSBM800X_ClearFifo();
//	uint16_t tmp = 0xf;
//	ZSBM_WRITE_REG16(0x0f, &tmp, 1);
	ZSBM800X_DumpRegister();
	ZSBM800X_TimerSlotStart(TSD_EN );	
}

void ZSBM_GreenNormal(uint16_t samplerate)
{
	uint16_t cur, tia ;
	uint16_t tmp ;
	ZSBM800X_SetSampleRate(samplerate) ;	// 
	ZSBM800X_SetLedCurrent(Z_GREEN_CH, led_cfg[1].led_cur) ;
	ZSBM800X_SetTIAGain(gains[led_cfg[1].tia_cap_group]);
	
	ZSBM800X_GetLedCurrentAndTiaSetting(Z_GREEN_CH, &cur, &tia);
	
	ZSBM800X_SelectTimeSlot(TSA_REG_SEL) ;
#if (DEFAULT_HR_MODE_GREEN_PULSE>1)	
	ZSBM800X_SetFifoWordSizeEnPds(FIFO_WORD_SIZE_24, USED_PD_NUMBER);
	ZSBM800X_SetSubPeriod_NumIntTsx_NumRepeatTsx	(0, 0, DEFAULT_HR_MODE_GREEN_PULSE - 1 ) ;
#else
	ZSBM800X_SetFifoWordSizeEnPds(FIFO_WORD_SIZE_16, USED_PD_NUMBER);
	ZSBM800X_SetSubPeriod_NumIntTsx_NumRepeatTsx	(0, 0, 0) ;
#endif
	
	ZSBM800X_SetFifoAdrSize(HR_HRV_TSA_FIFO_START_ADDR, HR_HRV_TSA_FIFO_SIZE) ;
	ZSBM800X_SetFifoDepth(HR_HRV_TSA_FIFO_INT_THRD) ;	
	
	// Config LPF
	tmp = 0x0110;
	ZSBM_WRITE_REG16(0x3E, &tmp, 1) ;
//	tmp = 0x0101;
//	ZSBM_WRITE_REG16(0x3F, &tmp, 1) ;
	
//	ZSBM800X_ClearFifo();
	ZSBM800X_DumpRegister();

	ZSBM800X_TimerSlotStart(TSA_EN );	
}

void ZSBM_IrRedNormal(uint16_t samplerate)
{
	uint16_t cur, tia ;
	uint16_t tmp;
	ZSBM800X_SetSampleRate(samplerate) ;	// 
	ZSBM800X_SetLedCurrent(Z_RED_CH, led_cfg[2].led_cur) ;
	ZSBM800X_SetTIAGain(gains[led_cfg[2].tia_cap_group]);	
	ZSBM800X_SetLedCurrent(Z_IR_CH, led_cfg[0].led_cur );
	ZSBM800X_SetTIAGain(gains[led_cfg[0].tia_cap_group]);
	
	ZSBM800X_GetLedCurrentAndTiaSetting(Z_RED_CH, &cur, &tia);
	ZSBM800X_GetLedCurrentAndTiaSetting(Z_IR_CH, &cur, &tia);

	
	ZSBM800X_SelectTimeSlot(TSB_REG_SEL | TSC_REG_SEL) ;
#if	(DEFAULT_SPO2_MODE_IR_RED_PULSE>1)
	// Multy pos
	ZSBM800X_SetFifoWordSizeEnPds(FIFO_WORD_SIZE_24, USED_PD_NUMBER);
	ZSBM800X_SetSubPeriod_NumIntTsx_NumRepeatTsx	(0, 0, DEFAULT_SPO2_MODE_IR_RED_PULSE - 1) ;
#else
	ZSBM800X_SetFifoWordSizeEnPds(FIFO_WORD_SIZE_16, USED_PD_NUMBER);
	ZSBM800X_SetSubPeriod_NumIntTsx_NumRepeatTsx	(0, 0, 0) ;
#endif
	
	ZSBM800X_SelectTimeSlot(TSB_REG_SEL ) ;
	ZSBM800X_SetFifoAdrSize(SPO2_TSB_FIFO_START_ADDR, SPO2_TSB_FIFO_SIZE) ;
//	ZSBM800X_SetFifoDepth(HR_HRV_TSA_FIFO_INT_THRD) ;	
	ZSBM800X_SelectTimeSlot(TSC_REG_SEL ) ;
	ZSBM800X_SetFifoAdrSize(SPO2_TSC_FIFO_START_ADDR, SPO2_TSC_FIFO_SIZE) ;
	ZSBM800X_SetFifoDepth(SPO2_TSC_FIFO_INT_THRD) ;	

//	// Config LPF
//	tmp = 0x0114;
//	ZSBM_WRITE_REG16(0x3E, &tmp, 1) ;
//	tmp = 0x011A;
//	ZSBM_WRITE_REG16(0x3F, &tmp, 1) ;

//	ZSBM800X_ClearFifo();
	ZSBM800X_TimerSlotStart(TSB_EN | TSC_EN );	
}
uint8_t g_current = 30 ;
uint8_t r_current = 30 ;
uint8_t i_current = 30 ;

uint16_t g_tia = 0x6600;
uint16_t r_tia = 0x6600;
uint16_t i_tia = 0x6600;

//uint8_t g_current = 30 ;
//uint8_t r_current = 95 ;
//uint8_t i_current = 40 ;

//uint16_t g_tia = TIA_25K_CAP_6P32;//0x3810
//uint16_t r_tia = TIA_12K5_CAP_8P86;
//uint16_t i_tia = TIA_25K_CAP_6P32;

void ZSBM_IrRedGreenNormal(uint16_t samplerate)
{
	uint16_t cur, tia ;
	uint16_t tmp;
	ZSBM800X_SetSampleRate(samplerate) ;	// 
	
	ZSBM800X_SetLedCurrent(Z_GREEN_CH, g_current) ;
	ZSBM800X_SetTIAGain(g_tia);
	
	ZSBM800X_SetLedCurrent(Z_RED_CH, r_current) ;
	ZSBM800X_SetTIAGain(r_tia);	
	
	ZSBM800X_SetLedCurrent(Z_IR_CH, i_current );
	ZSBM800X_SetTIAGain(i_tia);
	
	ZSBM800X_GetLedCurrentAndTiaSetting(Z_GREEN_CH, &cur, &tia);
	ZSBM800X_GetLedCurrentAndTiaSetting(Z_RED_CH, &cur, &tia);
	ZSBM800X_GetLedCurrentAndTiaSetting(Z_IR_CH, &cur, &tia);

	ZSBM800X_SelectTimeSlot(TSA_REG_SEL) ;
#if (DEFAULT_HR_MODE_GREEN_PULSE>1)	
	ZSBM800X_SetFifoWordSizeEnPds(FIFO_WORD_SIZE_24, USED_PD_NUMBER);
	ZSBM800X_SetSubPeriod_NumIntTsx_NumRepeatTsx	(0, 0, DEFAULT_HR_MODE_GREEN_PULSE - 1 ) ;
#else
	ZSBM800X_SetFifoWordSizeEnPds(FIFO_WORD_SIZE_16, USED_PD_NUMBER);
	ZSBM800X_SetSubPeriod_NumIntTsx_NumRepeatTsx	(0, 0, 0) ;
#endif

	ZSBM800X_SelectTimeSlot(TSB_REG_SEL | TSC_REG_SEL) ;
#if	(DEFAULT_SPO2_MODE_IR_RED_PULSE>1)
	// Multy pos
	ZSBM800X_SetFifoWordSizeEnPds(FIFO_WORD_SIZE_24, USED_PD_NUMBER);
	ZSBM800X_SetSubPeriod_NumIntTsx_NumRepeatTsx	(0, 0, DEFAULT_SPO2_MODE_IR_RED_PULSE - 1) ;
#else
	ZSBM800X_SetFifoWordSizeEnPds(FIFO_WORD_SIZE_16, USED_PD_NUMBER);
	ZSBM800X_SetSubPeriod_NumIntTsx_NumRepeatTsx	(0, 0, 0) ;
#endif

	ZSBM800X_SelectTimeSlot(TSA_REG_SEL ) ;
	ZSBM800X_SetFifoAdrSize(0, 0x150) ;
	
	ZSBM800X_SelectTimeSlot(TSB_REG_SEL ) ;
	ZSBM800X_SetFifoAdrSize(0x155, 0x150) ;
//	ZSBM800X_SetFifoDepth(HR_HRV_TSA_FIFO_INT_THRD) ;	
	ZSBM800X_SelectTimeSlot(TSC_REG_SEL ) ;
	ZSBM800X_SetFifoAdrSize(0x2B0, 0x150) ;
	ZSBM800X_SetFifoDepth(240) ;	
	
	ZSBM800X_TimerSlotStart(TSA_EN | TSB_EN | TSC_EN );	
}

ZSBM_ERROR_CODE_T ZSBM_IRDetecProcess(uint16_t data_size, int16_t *buf)
{
	uint8_t i;
	for(i=1; i< 4; i++){
		if(ABS(buf[data_size - i]) < IR_DETEC_THRESHOLD_MIN)
			return ZSBM_IR_DETEC_CONTINUE ;
	}
	// IR value OK, turn off IR  
	// HRS/HRV turn Green LED, SPO2 turn on IR+RED
	return ZSBM_OK;
}

ZSBM_ERROR_CODE_T	ZSBM_AgcCalSearchCurrentSetting(uint8_t timeslot, ZSBM_LED_CFG_T *led)	// Timeslot B green
{

	if(timeslot & TSA_REG_SEL)
	{
		ZSBM800X_SetLedCurrent(Z_GREEN_CH, led[1].led_cur) ;
		ZSBM800X_SetTIAGain(gains[led[1].tia_cap_group]);		
	}

	if(timeslot & TSB_REG_SEL)
	{
		ZSBM800X_SetLedCurrent(Z_RED_CH, led[2].led_cur) ;
		ZSBM800X_SetTIAGain(gains[led[2].tia_cap_group]);		
	}

	if(timeslot & TSC_REG_SEL)
	{
		ZSBM800X_SetLedCurrent(Z_IR_CH, led[0].led_cur) ;
		ZSBM800X_SetTIAGain(gains[led[0].tia_cap_group]);		
	}
	
//	ZSBM800X_ClearFifo();
//	ZSBM800X_DumpRegister();
	ZSBM800X_TimerSlotStart(timeslot & 0x7 );	
	return ZSBM_OK;
}

#define HIGH_PERFORMANCE
#ifdef HIGH_PERFORMANCE
ZSBM_ERROR_CODE_T ZSBM_AgcCalProcess(int16_t latest_dat, uint8_t timeslot, ZSBM_LED_CFG_T *led)
{
	uint8_t currentmin, currentmax, tiagainmax ,tiagainmin ;
	static uint8_t GreenTiaStep = 0, RedTiaStep = 0, IrTiaStep = 0;
	static uint8_t GreenFirst = 1, RedFirst = 1, IrFirst = 1;
	static uint8_t GreenCurBefMin = 0, GreenCurBefMax = 0, RedCurBefMin = 0, RedCurBefMax = 0, IrCurBefMin = 0, IrCurBefMax = 0;
	uint8_t *tia_step, *first, *bef_led_currnet_min, *bef_led_currnet_max;

	if(timeslot&0x1)	
	{
		currentmin = AGC_GREEN_LED_CURREN_MIN;
		currentmax = AGC_GREEN_LED_CURREN_MAX;
		tiagainmin = 0;//AGC_GREEN_GAIN_MIN_LV -1 ;
		tiagainmax = GREEN_GAIN_MAX_LV - 1;
		tia_step = &GreenTiaStep;		
		first = &GreenFirst;
		bef_led_currnet_min = &GreenCurBefMin;
		bef_led_currnet_max = &GreenCurBefMax;
		if(*first) {
			*first = 0;
			led->tia_cap_group = AGC_GREEN_GAIN_BEST_LV - 1;	//?????????
		}
	}
	if(timeslot&0x2)	
	{
		currentmin = AGC_RED_LED_CURREN_MIN;
		currentmax = AGC_RED_LED_CURREN_MAX;
		tiagainmin = 0;//ZSBM_AGC_RED_GAIN_MIN_LV -1 ;
		tiagainmax = RED_GAIN_MAX_LV - 1;
		tia_step = &RedTiaStep;		
		first = &RedFirst;
		bef_led_currnet_min = &RedCurBefMin;
		bef_led_currnet_max = &RedCurBefMax;
		if(*first) {
			*first = 0;
			led->tia_cap_group = AGC_RED_GAIN_BEST_LV - 1;	//?????????
		}
	}
	if(timeslot&0x4)	
	{
		currentmin = AGC_IR_LED_CURREN_MIN;
		currentmax = AGC_IR_LED_CURREN_MAX;
		tiagainmin = 0;//ZSBM_AGC_IR_GAIN_MIN_LV -1 ;
		tiagainmax = IR_GAIN_MAX_LV - 1;
		tia_step = &IrTiaStep;
		first = &IrFirst;
		bef_led_currnet_min = &IrCurBefMin;
		bef_led_currnet_max = &IrCurBefMax;
		if(*first) {
			*first = 0;
			led->tia_cap_group = AGC_IR_GAIN_BEST_LV - 1;	//?????????
		}
	}	
	ZSBM_PRINTF("AAA ppg = %d, Current = %d, TIA = 0x%x\n", ABS(latest_dat), led->led_cur, gains[led->tia_cap_group] ) ;
	
	if((ABS(latest_dat) > AGC_PPG_THRESHOLD_LV2) && (ABS(latest_dat) < AGC_PPG_THRESHOLD_LV3))
	{
		led->status = 3;
		*first = 1;
		*tia_step = 0;
		return ZSBM_OK ;			
	}

	if((led->status & 0x1) != 1) {	//??????????	
		if(*tia_step == 0) {
			led->led_cur = currentmin;
			(*tia_step)++;
		}
		else if(*tia_step == 1) {
			if(ABS(latest_dat) > AGC_PPG_THRESHOLD_LV3) {	//?????????
				*tia_step = 0;
				if(led->tia_cap_group > tiagainmin) {	//tia????
					led->tia_cap_group--;					
				}
				else {	//tia????,??agc
					*first = 1;
					*tia_step = 0;
					return ZSBM_AGC_REACH_CURRENT_MAX;	
				}
			}
			else {
				*bef_led_currnet_min = led->led_cur;
				led->led_cur = currentmax;
				(*tia_step)++;
				ZSBM_PRINTF("\r\n");
				ZSBM_PRINTF("\r\n");
				ZSBM_PRINTF("min_cur = %d\r\n",led->led_cur);
			}
		}
		else if(*tia_step == 2) {
			if(ABS(latest_dat) < AGC_PPG_THRESHOLD_LV2) {	//?????????
				*tia_step = 0;
				if(led->tia_cap_group < tiagainmax) {		//tia????
					led->tia_cap_group ++;				
				}
				else {	//tia????,??agc
					*first = 1;
					*tia_step = 0;
					return ZSBM_AGC_REACH_CURRENT_MIN;	
				}
			}
			else {
				(*tia_step)++;
				*bef_led_currnet_max = led->led_cur;
				ZSBM_PRINTF("\r\n");
				ZSBM_PRINTF("\r\n");
				ZSBM_PRINTF("max_cur = %d\r\n",led->led_cur);
				led->led_cur = (*bef_led_currnet_max + *bef_led_currnet_min)/2; 	//???????????
				led->status |= 0x1;	//???????????
				ZSBM_PRINTF("led->led_cur = %d\r\n",led->led_cur);
				ZSBM_PRINTF("\r\n");
				ZSBM_PRINTF("TIA CAL Done!\n");
			}
		}
	}
	else		// TIA??,????????
	{
		ZSBM_PRINTF("Current CAL start... %d\n", latest_dat);
		if(ABS(latest_dat) > AGC_PPG_THRESHOLD_LV3) {	//?????????
			*bef_led_currnet_max = led->led_cur;
			ZSBM_PRINTF("\r\n");
			ZSBM_PRINTF("AGC_PPG_MAX_1 = %d\r\n",*bef_led_currnet_max);
			ZSBM_PRINTF("AGC_PPG_MAX_2 = %d\r\n",*bef_led_currnet_min);
			led->led_cur = (*bef_led_currnet_max + *bef_led_currnet_min)/2; 	//?????????????
			ZSBM_PRINTF("AGC_PPG_MAX_3 = %d\r\n",led->led_cur);
			ZSBM_PRINTF("\r\n");
		}
		if(ABS(latest_dat) < AGC_PPG_THRESHOLD_LV2) {	//?????????
			*bef_led_currnet_min = led->led_cur;

			ZSBM_PRINTF("\r\n");
			ZSBM_PRINTF("AGC_PPG_MIN_1 = %d\r\n",*bef_led_currnet_max);
			ZSBM_PRINTF("AGC_PPG_MIN_2 = %d\r\n",*bef_led_currnet_min);
			led->led_cur = (*bef_led_currnet_max + *bef_led_currnet_min)/2; 	//?????????????
			ZSBM_PRINTF("AGC_PPG_MIN_3 = %d\r\n",led->led_cur);
			ZSBM_PRINTF("\r\n");
		}
		if(ABS(*bef_led_currnet_max-*bef_led_currnet_min) <= 1) {
			*first = 1;
			*tia_step = 0;
			return ZSBM_AGC_REACH_CURRENT_MIN;
		}
	}

	ZSBM_PRINTF("Next Current = %d, TIA = 0x%x\n", led->led_cur, gains[led->tia_cap_group] ) ;
	return ZSBM_AGC_CAL_CONTINUE ;
}

ZSBM_ERROR_CODE_T ZSBM_AgcCalProcess_test(int16_t latest_dat, uint8_t timeslot, ZSBM_LED_CFG_T *led)
{
	uint8_t currentmin, currentmax, tiagainmax ,tiagainmin ;
	static uint8_t GreenTiaStep = 0, RedTiaStep = 0, IrTiaStep = 0;
	static uint8_t GreenFirst = 1, RedFirst = 1, IrFirst = 1;
	static uint8_t GreenCurBefMin = 0, GreenCurBefMax = 0, RedCurBefMin = 0, RedCurBefMax = 0, IrCurBefMin = 0, IrCurBefMax = 0;
	uint8_t *tia_step, *first, *bef_led_currnet_min, *bef_led_currnet_max;

	uint8_t *bef_min, *bef_max;

	if(timeslot&0x2)	
	{
		currentmin = AGC_RED_LED_CURREN_MIN;
		currentmax = AGC_RED_LED_CURREN_MAX;
		first = &RedFirst;
		bef_min = &RedCurBefMin;
		bef_max = &RedCurBefMax;
		led->tia_cap_group = 0;
	}
	if(timeslot&0x4)	
	{
		currentmin = AGC_IR_LED_CURREN_MIN;
		currentmax = AGC_IR_LED_CURREN_MAX;
		first = &IrFirst;
		bef_min = &IrCurBefMin;
		bef_max = &IrCurBefMax;
		led->tia_cap_group = 0;
	}	
	ZSBM_PRINTF("AAA ppg = %d, Current = %d, TIA = 0x%x\n", ABS(latest_dat), led->led_cur, gains[led->tia_cap_group] ) ;

	//--- 初始化：首次运行时设置电流最小值 ---
	if (*first) {
		*first = 0;
		led->led_cur = currentmin;   // 初始电流设为最小值
		*bef_min = currentmin;
		*bef_max = currentmax;
		ZSBM_PRINTF("[Init] Cur=%d\n", led->led_cur);
		return ZSBM_AGC_CAL_CONTINUE;
	}
	

    //--- 信号范围检查：若已在目标范围则完成校准 ---
    int16_t abs_dat = ABS(latest_dat);
    if (abs_dat > AGC_PPG_THRESHOLD_LV2 && abs_dat < AGC_PPG_THRESHOLD_LV3) {
        led->status = 3; // 标记为已完成
		*first = 1;
        ZSBM_PRINTF("[OK] Cur=%d, PPG=%d\n", led->led_cur, latest_dat);
        return ZSBM_OK;
    }

    //--- 动态电流调整（二分法）---
    if (abs_dat > AGC_PPG_THRESHOLD_LV3) { 
        // 信号过强：降低电流（记录当前值为新上限）
        *bef_max = led->led_cur;
        led->led_cur = (*bef_min + led->led_cur) / 2;
        ZSBM_PRINTF("[HIGH] NewCur=%d\n",  led->led_cur);
    } else if (abs_dat < AGC_PPG_THRESHOLD_LV2) { 
        // 信号过弱：增加电流（记录当前值为新下限）
        *bef_min = led->led_cur;
        led->led_cur = (led->led_cur + *bef_max) / 2;
        ZSBM_PRINTF("[LOW]  NewCur=%d\n",  led->led_cur);
    }

    //--- 终止条件：电流范围无法继续缩小 ---
    if (*bef_max - *bef_min <= 1) { 
        ZSBM_PRINTF("[FAIL]  Min=%d, Max=%d\n", *bef_min, *bef_max);
		*first = 1;
        return ZSBM_AGC_REACH_CURRENT_MIN;
    }

	ZSBM_PRINTF("Next Current = %d, TIA = 0x%x\n", led->led_cur, gains[led->tia_cap_group] ) ;
	return ZSBM_AGC_CAL_CONTINUE ;
}

ZSBM_ERROR_CODE_T ZSBM_AgcCalProcess_test1(int16_t latest_dat, uint8_t timeslot, ZSBM_LED_CFG_T *led)
{
	uint8_t currentmin, currentmax, tiagainmax ,tiagainmin ;
	static uint8_t GreenTiaStep = 0, RedTiaStep = 0, IrTiaStep = 0;
	static uint8_t GreenFirst = 1, RedFirst = 1, IrFirst = 1;
	static uint8_t GreenCurBefMin = 0, GreenCurBefMax = 0, RedCurBefMin = 0, RedCurBefMax = 0, IrCurBefMin = 0, IrCurBefMax = 0;
	uint8_t *tia_step, *first, *bef_led_currnet_min, *bef_led_currnet_max;

	if(timeslot&0x2)	
	{
		currentmin = AGC_RED_LED_CURREN_MIN;
		currentmax = AGC_RED_LED_CURREN_MAX;
		tiagainmin = 0;//ZSBM_AGC_RED_GAIN_MIN_LV -1 ;
		tiagainmax = RED_GAIN_MAX_LV - 1;
		tia_step = &RedTiaStep;		
		first = &RedFirst;
		bef_led_currnet_min = &RedCurBefMin;
		bef_led_currnet_max = &RedCurBefMax;
		if(*first) {
			*first = 0;
			led->tia_cap_group = 1 - 1;	//?????????
		}
	}
	if(timeslot&0x4)	
	{
		currentmin = AGC_IR_LED_CURREN_MIN;
		currentmax = AGC_IR_LED_CURREN_MAX;
		tiagainmin = 0;//ZSBM_AGC_IR_GAIN_MIN_LV -1 ;
		tiagainmax = IR_GAIN_MAX_LV - 1;
		tia_step = &IrTiaStep;
		first = &IrFirst;
		bef_led_currnet_min = &IrCurBefMin;
		bef_led_currnet_max = &IrCurBefMax;
		if(*first) {
			*first = 0;
			led->tia_cap_group = 1 - 1;	//?????????
		}
	}	
	ZSBM_PRINTF("AAA ppg = %d, Current = %d, TIA = 0x%x\n", ABS(latest_dat), led->led_cur, gains[led->tia_cap_group] ) ;
	
	if((ABS(latest_dat) > AGC_PPG_THRESHOLD_LV2) && (ABS(latest_dat) < AGC_PPG_THRESHOLD_LV3))
	{
		led->status = 3;
		*first = 1;
		*tia_step = 0;
		return ZSBM_OK ;			
	}

	
	ZSBM_PRINTF("Current CAL start... %d\n", latest_dat);
	if(ABS(latest_dat) > AGC_PPG_THRESHOLD_LV3) {	//?????????
		*bef_led_currnet_max = led->led_cur;
		ZSBM_PRINTF("\r\n");
		ZSBM_PRINTF("AGC_PPG_MAX_1 = %d\r\n",*bef_led_currnet_max);
		ZSBM_PRINTF("AGC_PPG_MAX_2 = %d\r\n",*bef_led_currnet_min);
		led->led_cur = (*bef_led_currnet_max + *bef_led_currnet_min)/2; 	//?????????????
		ZSBM_PRINTF("AGC_PPG_MAX_3 = %d\r\n",led->led_cur);
		ZSBM_PRINTF("\r\n");
	}
	if(ABS(latest_dat) < AGC_PPG_THRESHOLD_LV2) {	//?????????
		*bef_led_currnet_min = led->led_cur;

		ZSBM_PRINTF("\r\n");
		ZSBM_PRINTF("AGC_PPG_MIN_1 = %d\r\n",*bef_led_currnet_max);
		ZSBM_PRINTF("AGC_PPG_MIN_2 = %d\r\n",*bef_led_currnet_min);
		led->led_cur = (*bef_led_currnet_max + *bef_led_currnet_min)/2; 	//?????????????
		ZSBM_PRINTF("AGC_PPG_MIN_3 = %d\r\n",led->led_cur);
		ZSBM_PRINTF("\r\n");
	}
	if(ABS(*bef_led_currnet_max-*bef_led_currnet_min) <= 1) {
		*first = 1;
		*tia_step = 0;
		return ZSBM_AGC_REACH_CURRENT_MIN;
	}


	ZSBM_PRINTF("Next Current = %d, TIA = 0x%x\n", led->led_cur, gains[led->tia_cap_group] ) ;
	return ZSBM_AGC_CAL_CONTINUE ;
}

#else
ZSBM_ERROR_CODE_T ZSBM_AgcCalProcess(int16_t latest_dat, uint8_t timeslot, ZSBM_LED_CFG_T *led)
{
	uint8_t currentmin, currentmax, tiagainmax ;
	if(timeslot&0x4)	
	{
		currentmin = AGC_IR_LED_CURREN_MIN;
		currentmax = AGC_IR_LED_CURREN_MAX;
		tiagainmax = IR_GAIN_MAX_LV - 1;
	}
	if(timeslot&0x1)	
	{
		currentmin = AGC_GREEN_LED_CURREN_MIN;
		currentmax = AGC_GREEN_LED_CURREN_MAX;
		tiagainmax = GREEN_GAIN_MAX_LV - 1;
	}
	if(timeslot&0x2)	
	{
		currentmin = AGC_RED_LED_CURREN_MIN;
		currentmax = AGC_RED_LED_CURREN_MAX;
		tiagainmax = RED_GAIN_MAX_LV - 1;
	}
	
	ZSBM_PRINTF("AAA ppg = %d, Current = %d, TIA = 0x%x\n", ABS(latest_dat), led->led_cur, gains[led->tia_cap_group] ) ;
	
	if((ABS(latest_dat) > AGC_PPG_THRESHOLD_LV2) && (ABS(latest_dat) < AGC_PPG_THRESHOLD_LV3))
	{
		led->status = 3;
		return ZSBM_OK ;			
	}

	// The latest data
	if((led->status & 0x1) != 1)	// no proper tia 
	{
		if(ABS(latest_dat) < AGC_PPG_THRESHOLD_LV2)
		{
				if(	led->tia_cap_group < (tiagainmax - 1) )
				{
					// TIA 不是最高档，超过PPG上限，调高一档TIA
					led->tia_cap_group ++ ;
				}
				else
				{
					// TIA 已是最高档，固定在最高档，结束TIA调整，准备调整电流
					ZSBM_PRINTF("ppg = %d, Current = %d, TIA = 0x%x\n", ABS(latest_dat), led->led_cur, gains[led->tia_cap_group] ) ;
					return ZSBM_AGC_REACH_GAIN_MIN;
				}
		}
		else
		{
				led->status |= 0x1;
				ZSBM_PRINTF("TIA CAL Done!\n");
//				return ZSBM_AGC_REACH_GAIN_MAX;				
		}
	}
	else		// TIA OK, fine tune current
	{
		ZSBM_PRINTF("Current CAL start... %d\n", ABS(latest_dat));
		if((ABS(latest_dat) > AGC_PPG_THRESHOLD_LV2) && (ABS(latest_dat) < AGC_PPG_THRESHOLD_LV3))
		{
				led->status |= 0x2 ;
				return ZSBM_OK ;			
		}

		if(ABS(latest_dat) > AGC_PPG_THRESHOLD_LV3)
		{
			if(led->led_cur > currentmin )
				led->led_cur -=2 ;
			else
			{
				ZSBM_PRINTF("ppg = %d, Current = %d, TIA = 0x%x\n", ABS(latest_dat), led->led_cur, gains[led->tia_cap_group] ) ;
				return ZSBM_AGC_REACH_CURRENT_MIN ;	
			}		
		}
		if(ABS(latest_dat) < AGC_PPG_THRESHOLD_LV2) 
		{
			if(led->led_cur < currentmax)
				led->led_cur +=2 ;
			else
			{
				ZSBM_PRINTF("ppg = %d, Current = %d, TIA = 0x%x\n", ABS(latest_dat), led->led_cur, gains[led->tia_cap_group] ) ;
				return ZSBM_AGC_REACH_CURRENT_MAX ;	
			}				
		}		
	}

	ZSBM_PRINTF("Next Current = %d, TIA = 0x%x\n", led->led_cur, gains[led->tia_cap_group] ) ;
	return ZSBM_AGC_CAL_CONTINUE ;
}
#endif

ZSBM_ERROR_CODE_T ZSBM_GreenAgcCalProcess(uint16_t data_size, int16_t *greenbuf)
{
	ZSBM_ERROR_CODE_T ret;
	
	// the latest green PPG data
	int16_t rawdat = greenbuf[data_size-1];
	ret = ZSBM_AgcCalProcess(rawdat, TSA_REG_SEL, &led_cfg[1]) ;
	if(ret != ZSBM_OK)
		ZSBM_AgcCalSearchCurrentSetting(TSA_REG_SEL,led_cfg) ;
	return ret;
}

// 电流调节主函数
ZSBM_ERROR_CODE_T ZSBM_IrRedCurrentAdjust_test(
    uint16_t data_size, 
    int16_t *red_buf, 
    int16_t *ir_buf,
    ZSBM_LED_CFG_T *red_cfg,
    ZSBM_LED_CFG_T *ir_cfg) 
{
    // 参数校验
    if(data_size == 0 || !red_buf || !ir_buf) 
        return ZSBM_AGC_CAL_CONTINUE;

    // // 滑动窗口滤波（4点平均）
    // static int32_t red_history[4] = {0};
    // static int32_t ir_history[4] = {0};
    // static uint8_t filter_idx = 0;
    
    // // 更新滤波缓冲区
    // red_history[filter_idx] = red_buf[data_size-1];
    // ir_history[filter_idx] = ir_buf[data_size-1];
    // filter_idx = (filter_idx + 1) % 4;

    // // 计算滤波值
    // int32_t red_sum = 0, ir_sum = 0;
    // for(uint8_t i=0; i<4; i++){
    //     red_sum += red_history[i];
    //     ir_sum += ir_history[i];
    // }
	// int16_t red_avg = (int16_t)(red_sum >> 2);
    // int16_t ir_avg = (int16_t)(ir_sum >> 2);

    int16_t red_avg = red_buf[data_size-1];
    int16_t ir_avg = ir_buf[data_size-1];


	ZSBM_PRINTF("\r\n");
	ZSBM_PRINTF("red_avg = %d,ir_avg = %d\r\n",red_avg,ir_avg);

    // 红光通道调节
	static uint8_t red_over_flag = 1;
    int32_t red_delta = red_avg - AGC_PPG_TARGET_LEVEL;
    uint8_t red_step = (abs(red_delta) > 5000) ? 2 : AGC_CURRENT_STEP;

	ZSBM_PRINTF("red_delta = %d,red_step = %d\r\n",red_delta,red_step);
 
	if((abs(red_delta) < 500) && red_delta > 0) {
		red_over_flag = 0;
		red_cfg->led_cur = red_cfg->led_cur;
		ZSBM_PRINTF("red_is_ok\r\n");
	}

    if(abs(red_delta) > 500 && red_over_flag == 1) {
        if(red_delta > 0) {
            red_cfg->led_cur = (red_cfg->led_cur > red_step) ? 
                             red_cfg->led_cur - red_step : AGC_CURRENT_MIN;
        } else {
            red_cfg->led_cur = (red_cfg->led_cur < (AGC_CURRENT_MAX - red_step)) ? 
                             red_cfg->led_cur + red_step : AGC_CURRENT_MAX;
        }
		// if((red_delta) < 1000) {
		// 	red_over_flag = 0;
		// 	red_cfg->led_cur = red_cfg->led_cur;
		// 	ZSBM_PRINTF("red_is_ok\r\n");
		// }
    }
	ZSBM_PRINTF("red_cfg->led_cur = %d\r\n",red_cfg->led_cur);	

    // 红外通道调节
	static uint8_t ir_over_flag = 1;
    int32_t ir_delta = ir_avg - AGC_PPG_TARGET_LEVEL;
    uint8_t ir_step = (abs(ir_delta) > 5000) ? 2 : AGC_CURRENT_STEP;
	ZSBM_PRINTF("ir_delta = %d,ir_step = %d\r\n",ir_delta,ir_step);

	if((abs(ir_delta) < 500) && (ir_delta > 0)) {
		ir_over_flag = 0;
		ir_cfg->led_cur = ir_cfg->led_cur;
		ZSBM_PRINTF("ir_is_ok\r\n");
	}

    if(abs(ir_delta) > 500 && ir_over_flag == 1) {
        if(ir_delta > 0) {
            ir_cfg->led_cur = (ir_cfg->led_cur > ir_step) ? 
                            ir_cfg->led_cur - ir_step : AGC_CURRENT_MIN;
        } else {
            ir_cfg->led_cur = (ir_cfg->led_cur < (AGC_CURRENT_MAX - ir_step)) ? 
                            ir_cfg->led_cur + ir_step : AGC_CURRENT_MAX;
        }
		// if((ir_delta) < 500) {
		// 	ir_over_flag = 0;
		// 	ir_cfg->led_cur = ir_cfg->led_cur;
		// 	ZSBM_PRINTF("ir_is_ok\r\n");
		// }
    }
	ZSBM_PRINTF("ir_cfg->led_cur = %d\r\n",ir_cfg->led_cur);

    // 边界保护
    red_cfg->led_cur = CLAMP(red_cfg->led_cur, AGC_CURRENT_MIN, AGC_CURRENT_MAX);
    ir_cfg->led_cur = CLAMP(ir_cfg->led_cur, AGC_CURRENT_MIN, AGC_CURRENT_MAX);

    // 更新硬件寄存器
    ZSBM800X_SetLedCurrent(Z_RED_CH, red_cfg->led_cur);
    ZSBM800X_SetLedCurrent(Z_IR_CH, ir_cfg->led_cur);

    // 收敛判断
    // if((abs(red_delta) < 1000) && (abs(ir_delta) < 1000)) {
    //     return ZSBM_OK;
    // }
	if(red_over_flag == 0 && ir_over_flag == 0 ){
		return ZSBM_OK;
	}

    return (red_cfg->led_cur == AGC_CURRENT_MAX || 
            red_cfg->led_cur == AGC_CURRENT_MIN ||
            ir_cfg->led_cur == AGC_CURRENT_MAX || 
            ir_cfg->led_cur == AGC_CURRENT_MIN) ? 
            ZSBM_AGC_REACH_LIMIT : ZSBM_AGC_CAL_CONTINUE;
}
// LED电流设置实现
void ZSBM800X_SetLedCurrent_test(ZSBM_LED_T led_ch, uint8_t current) {
    uint16_t reg_value = 0;
    
    switch(led_ch) {
        case Z_GREEN_CH:
            reg_value = (current & 0x7F) | (LED_GREEN_USED_DRV_PINS << 8);
            ZSBM_WRITE_REG16(0x14, &reg_value, 1);
            break;
            
        case Z_RED_CH:
            reg_value = (current & 0x7F) | (LED_RED_USED_DRV_PINS << 8);
            ZSBM_WRITE_REG16(0x15, &reg_value, 1);
            break;
            
        case Z_IR_CH:
            reg_value = (current & 0x7F) | (LED_IR_USED_DRV_PINS << 8);
            ZSBM_WRITE_REG16(0x16, &reg_value, 1);
            break;
            
        default:
            break;
    }
}

ZSBM_ERROR_CODE_T ZSBM_IrRedAgcCalProcess(uint16_t data_size, int16_t *redbuf, int16_t *irbuf)
{
	ZSBM_ERROR_CODE_T retr,reti;
	// the latest IR data
	int16_t rawdat = irbuf[data_size-1];
	if(led_cfg[0].status != 3)
	{
		ZSBM_PRINTF("\r\n");
		ZSBM_PRINTF("ZSBM_AgcCalProcess for IR\r\n");		
		reti = ZSBM_AgcCalProcess(rawdat, TSC_REG_SEL, &led_cfg[0]) ;
		ZSBM_PRINTF("rawdat = %d,IR_cur = %d\r\n",rawdat,led_cfg[0].led_cur);	
	}
	else
		reti = ZSBM_OK;
	ZSBM_PRINTF("IR AGC...reti = %d\n", reti);
	
	// the latest RED PPG data
	rawdat = redbuf[data_size-1];
	if(led_cfg[2].status != 3)
	{
		ZSBM_PRINTF("\r\n");	
		ZSBM_PRINTF("ZSBM_AgcCalProcess for RED\r\n");		
		retr = ZSBM_AgcCalProcess(rawdat, TSB_REG_SEL, &led_cfg[2]) ;
		ZSBM_PRINTF("rawdat = %d,RED_cur = %d\r\n",rawdat,led_cfg[2].led_cur);	
	}
	else
		retr = ZSBM_OK;
	ZSBM_PRINTF("RED AGC...retr = %d\n", retr);
	
	if((reti == ZSBM_OK)  && (retr == ZSBM_OK))	return ZSBM_OK;	
	
	if( ((reti == ZSBM_AGC_CAL_CONTINUE) && (retr == ZSBM_OK)) || \
		((retr == ZSBM_AGC_CAL_CONTINUE) && (reti == ZSBM_OK)) || \
		((retr == ZSBM_AGC_CAL_CONTINUE) && (reti == ZSBM_AGC_CAL_CONTINUE)) )
	{
		ZSBM_AgcCalSearchCurrentSetting(TSB_REG_SEL | TSC_REG_SEL, led_cfg) ;
		return ZSBM_AGC_CAL_CONTINUE ;
	}
	
	if((retr != ZSBM_AGC_CAL_CONTINUE) && (retr != ZSBM_OK))	return retr;	// red led agc fail
	if((reti != ZSBM_AGC_CAL_CONTINUE) && (reti != ZSBM_OK))	return reti;	// ir agc fail	
	
	// add to remove compile errors
	return ZSBM_OK ;
}

ZSBM_ERROR_CODE_T ZSBM_IrRedAgcCalProcess_test(uint16_t data_size, int16_t *redbuf, int16_t *irbuf)
{
	ZSBM_ERROR_CODE_T retr,reti;
	// the latest IR data
	int16_t rawdat = irbuf[data_size-1];
	if(led_cfg[0].status != 3)
	{
		ZSBM_PRINTF("\r\n");
		ZSBM_PRINTF("ZSBM_AgcCalProcess for IR\r\n");		
		reti = ZSBM_AgcCalProcess_test(rawdat, TSC_REG_SEL, &led_cfg[0]) ;//AI二分法
		// reti = ZSBM_AgcCalProcess_test1(rawdat, TSC_REG_SEL, &led_cfg[0]) ;//改的例程二分法
		ZSBM_PRINTF("rawdat = %d,IR_cur = %d\r\n",rawdat,led_cfg[0].led_cur);	
	}
	else
		reti = ZSBM_OK;
	ZSBM_PRINTF("IR AGC...reti = %d\n", reti);
	
	// the latest RED PPG data
	rawdat = redbuf[data_size-1];
	if(led_cfg[2].status != 3)
	{
		ZSBM_PRINTF("\r\n");	
		ZSBM_PRINTF("ZSBM_AgcCalProcess for RED\r\n");		
		retr = ZSBM_AgcCalProcess_test(rawdat, TSB_REG_SEL, &led_cfg[2]) ;//AI二分法
		// reti = ZSBM_AgcCalProcess_test1(rawdat, TSC_REG_SEL, &led_cfg[2]) ;//改的例程二分法
		ZSBM_PRINTF("rawdat = %d,RED_cur = %d\r\n",rawdat,led_cfg[2].led_cur);	
	}
	else
		retr = ZSBM_OK;
	ZSBM_PRINTF("RED AGC...retr = %d\n", retr);
	
	if((reti == ZSBM_OK)  && (retr == ZSBM_OK))	return ZSBM_OK;	
	
	if( ((reti == ZSBM_AGC_CAL_CONTINUE) && (retr == ZSBM_OK)) || \
		((retr == ZSBM_AGC_CAL_CONTINUE) && (reti == ZSBM_OK)) || \
		((retr == ZSBM_AGC_CAL_CONTINUE) && (reti == ZSBM_AGC_CAL_CONTINUE)) )
	{
		ZSBM_AgcCalSearchCurrentSetting(TSB_REG_SEL | TSC_REG_SEL, led_cfg) ;
		return ZSBM_AGC_CAL_CONTINUE ;
	}
	
	if((retr != ZSBM_AGC_CAL_CONTINUE) && (retr != ZSBM_OK))	return retr;	// red led agc fail
	if((reti != ZSBM_AGC_CAL_CONTINUE) && (reti != ZSBM_OK))	return reti;	// ir agc fail	
	
	// add to remove compile errors
	return ZSBM_OK ;
}

uint16_t gre_led_cur, gre_led_tia;
/**
*function:ZSBM800X data processing
*parameter:none
*return:none
*date:2022/05/20
**/

void ZSBM800X_DataHandle(void)
{
	ZSBM_ERROR_CODE_T ret;
	uint16_t i ;
	int16_t *ptr, intstatus=0 ;
	uint16_t fifosize[5]; // TSA-D, ECG
	
	static uint8_t firstin = 0 ;
	
	if(zsbm_int_flag) // buf not empty
	{
		intstatus = ZSBM800X_ReadIntStatus();
		// ZSBM_PRINTF("intstatus1 = %x, %d\r\n", intstatus, zsbm_status);
		zsbm_int_flag = 0 ;
		switch(zsbm_status)
		{
/*			case AGC_OFFSET_CAL:
					break;
*/			
			case IR_DETEC_LOW_FS:
				if(intstatus & Z_IR_DETECD_CH) // TSD FIFO THRD interrupt
				{
					firstin = 0 ;
					fifosize[3] = ZSBM800X_ReadFifoCount(Z_IR_DETECD_CH)/2*2;
					if(fifosize[3])
						ZSBM800X_ReadFifoData(Z_IR_DETECD_CH, zsbm_raw_dat_ird, fifosize[3]);
				}
				ZSBM_PRINTF("IRD size = %d, %d\n", fifosize[3], (int16_t)(zsbm_raw_dat_ird[1]<<8 | zsbm_raw_dat_ird[0] ));
				ZSBM_PRINTF("IRD size = %d, %d\n", fifosize[3], (int16_t)(zsbm_raw_dat_ird[3]<<8 | zsbm_raw_dat_ird[2] ));
				if(ZSBM_OK == ZSBM_IRDetecProcess(fifosize[3]/2, (int16_t *)zsbm_raw_dat_ird))
				{
					if((HR_MODE == workmode) || (HRV_MODE == workmode) )
					{
#ifdef HIGH_PERFORMANCE
						led_cfg[1].led_cur = AGC_GREEN_LED_CURREN_MAX;
						led_cfg[1].tia_cap_group = 1 - 1 ;
						led_cfg[1].status = 0;
#else
						led_cfg[1].led_cur = AGC_GREEN_LED_CURREN_MAX;
						led_cfg[1].tia_cap_group = GREEN_GAIN_MAX_LV - 1 ;
						led_cfg[1].status = 0;
#endif						
						ZSBM800X_CommonInit();
						ZSBM800X_SetSampleRate(DEFAULT_AGC_CAL_SAMPLE_FS) ;	// start with IR @ 100Hz
						ZSBM800X_SelectTimeSlot(TSA_REG_SEL) ;
						ZSBM800X_SetFifoAdrSize(AGC_TSA_FIFO_START_ADDR, AGC_TSA_FIFO_SIZE) ;
						ZSBM800X_SetFifoDepth(HR_HRV_TSABCD_FIFO_INT_THRD) ;	
						ZSBM_AgcCalSearchCurrentSetting(TSA_REG_SEL, led_cfg);	// Timeslot A green
						zsbm_status = GREEN_AGC_CAL;
					}
					if(SPO2_MODE == workmode)
					{
#ifdef HIGH_PERFORMANCE
						led_cfg[0].led_cur = AGC_IR_LED_CURREN_MAX;
						led_cfg[0].tia_cap_group = 1 - 1 ;
						led_cfg[0].status = 0;
						led_cfg[2].led_cur = AGC_RED_LED_CURREN_MAX;
						led_cfg[2].tia_cap_group = 1 - 1 ;
						led_cfg[2].status = 0;
#else						
						led_cfg[0].led_cur = AGC_IR_LED_CURREN_MAX;
						led_cfg[0].tia_cap_group = IR_GAIN_MAX_LV - 1 ;
						led_cfg[0].status = 0;
						led_cfg[2].led_cur = AGC_RED_LED_CURREN_MAX;
						led_cfg[2].tia_cap_group = RED_GAIN_MAX_LV - 1 ;
						led_cfg[2].status = 0;
#endif
						ZSBM800X_CommonInit();
						ZSBM800X_SetSampleRate(DEFAULT_AGC_CAL_SAMPLE_FS) ;	// start with IR @ 100Hz
						ZSBM800X_SelectTimeSlot(TSB_REG_SEL) ;
						ZSBM800X_SetFifoAdrSize(AGC_TSB_FIFO_START_ADDR, AGC_TSB_FIFO_SIZE) ;
//						ZSBM800X_SetFifoDepth(HR_HRV_TSABCD_FIFO_INT_THRD) ;	
						ZSBM800X_SelectTimeSlot(TSC_REG_SEL) ;
						ZSBM800X_SetFifoAdrSize(AGC_TSC_FIFO_START_ADDR, AGC_TSC_FIFO_SIZE) ;
						ZSBM800X_SetFifoDepth(HR_HRV_TSABCD_FIFO_INT_THRD) ;	
						ZSBM_AgcCalSearchCurrentSetting(TSB_REG_SEL | TSC_REG_SEL, led_cfg);	// Timeslot A green
						zsbm_status = IR_RED_AGC_CAL;
					}
					if((ECG_PPG_MODE == workmode))
					{
#ifdef HIGH_PERFORMANCE
						led_cfg[0].led_cur = AGC_IR_LED_CURREN_MAX;
						led_cfg[0].tia_cap_group = 1 - 1 ;
						led_cfg[0].status = 0;
						led_cfg[2].led_cur = AGC_RED_LED_CURREN_MAX;
						led_cfg[2].tia_cap_group = 1 - 1 ;
						led_cfg[2].status = 0;
#else						
						led_cfg[0].led_cur = AGC_IR_LED_CURREN_MAX;
						led_cfg[0].tia_cap_group = IR_GAIN_MAX_LV - 1 ;
						led_cfg[0].status = 0;
						led_cfg[2].led_cur = AGC_RED_LED_CURREN_MAX;
						led_cfg[2].tia_cap_group = RED_GAIN_MAX_LV - 1 ;
						led_cfg[2].status = 0;
#endif
						ZSBM800X_CommonInit();
						ZSBM800X_SetSampleRate(DEFAULT_AGC_CAL_SAMPLE_FS) ;	// start with IR @ 100Hz
						ZSBM800X_SelectTimeSlot(TSB_REG_SEL) ;
						ZSBM800X_SetFifoAdrSize(AGC_TSB_FIFO_START_ADDR, AGC_TSB_FIFO_SIZE) ;
//						ZSBM800X_SetFifoDepth(HR_HRV_TSABCD_FIFO_INT_THRD) ;	
						ZSBM800X_SelectTimeSlot(TSC_REG_SEL) ;
						ZSBM800X_SetFifoAdrSize(AGC_TSC_FIFO_START_ADDR, AGC_TSC_FIFO_SIZE) ;
						ZSBM800X_SetFifoDepth(HR_HRV_TSABCD_FIFO_INT_THRD) ;	
						ZSBM_AgcCalSearchCurrentSetting(TSB_REG_SEL | TSC_REG_SEL, led_cfg);	// Timeslot A green
						zsbm_status = IR_RED_AGC_CAL;
					}

				}
				break;
				
			case GREEN_AGC_CAL:	
				if(intstatus & Z_GREEN_CH) // TSD FIFO THRD interrupt
				{
					fifosize[0] = ZSBM800X_ReadFifoCount(Z_GREEN_CH)/2*2;
					ZSBM800X_ReadFifoData(Z_GREEN_CH, zsbm_raw_dat_green, fifosize[0]);
				}
				ret = ZSBM_GreenAgcCalProcess(fifosize[0]/2, (int16_t *)zsbm_raw_dat_green);
				ZSBM_PRINTF("ZSBM_GreenAgcCalProcess = %d\n", ret);
				if( ZSBM_OK == ret){
					if(HR_MODE == workmode)
					{
						ZSBM800X_CommonInit();
						ZSBM_GreenNormal(DEFAULT_HRV_MODE_SAMPLE_FS);	// HRV 100Hz
						zsbm_status = GREEN_NORMAL;			
					}
					else if(HRV_MODE == workmode)
					{
						ZSBM800X_CommonInit();
						ZSBM_GreenNormal(DEFAULT_HR_MODE_SAMPLE_FS);	// D HR 25Hz
						zsbm_status = GREEN_NORMAL;			
					}		
					else if(ECG_PPG_MODE == workmode)
					{
						gre_led_cur = led_cfg[1].led_cur ;
						gre_led_tia = gains[led_cfg[1].tia_cap_group] ;
						ZSPD800X_EcgPpgInit();
						zsbm_status = ECG_GREEN_NORMAL;			
					}
				}						
				// no wear, turn off green, turn on IR, then goto low FS IR detec mode
				else if(ZSBM_AGC_CAL_CONTINUE != ret)
				{
					ZSBM800X_CommonInit();
					ZSBM_IRDeteLowFs() ;
					zsbm_status = IR_DETEC_LOW_FS;
				}
				break;
				
			case IR_RED_AGC_CAL:	
				if(intstatus & Z_IR_CH) // TSD FIFO THRD interrupt
				{
					fifosize[1] = ZSBM800X_ReadFifoCount(Z_RED_CH)/2*2;
					fifosize[2] = ZSBM800X_ReadFifoCount(Z_IR_CH)/2*2;
					
					// 长短取短
					if(fifosize[1] > fifosize[2]) fifosize[1] = fifosize[2] ;
					if(fifosize[2] > fifosize[1]) fifosize[2] = fifosize[1] ;
					
					ZSBM800X_ReadFifoData(Z_RED_CH, zsbm_raw_dat_red, fifosize[1]);
					ZSBM800X_ReadFifoData(Z_IR_CH, zsbm_raw_dat_ir, fifosize[2]);
				}
				// ret = ZSBM_IrRedAgcCalProcess(fifosize[1]/2, (int16_t *)zsbm_raw_dat_red, (int16_t *)zsbm_raw_dat_ir);
				ret = ZSBM_IrRedAgcCalProcess_test(fifosize[1]/2, (int16_t *)zsbm_raw_dat_red, (int16_t *)zsbm_raw_dat_ir);
				// ret = ZSBM_IrRedCurrentAdjust_test(
				// 	fifosize[1]/2, 
				// 	(int16_t *)zsbm_raw_dat_red, 
				// 	(int16_t *)zsbm_raw_dat_ir,
				// 	&led_cfg[2],  // 红光配置
				// 	&led_cfg[0]   // 红外配置
				// );


					ZSBM_PRINTF("ZSBM_IrRedAgcCalProcess = %d\n", ret);
					if( ZSBM_OK == ret){
						if(SPO2_MODE == workmode)
						{
							ZSBM800X_CommonInit();
							ZSBM_IrRedNormal(SAMPLE_FS_100HZ);	// SPO2 100Hz
							zsbm_status = IR_RED_NORMAL;		

							
							r_current = led_cfg[2].led_cur ;
							r_tia = gains[led_cfg[2].tia_cap_group] ;

							i_current = led_cfg[0].led_cur ;
							i_tia = gains[led_cfg[0].tia_cap_group] ;
						}
						else if(ECG_PPG_MODE == workmode)
						{
							r_current = led_cfg[2].led_cur ;
							r_tia = r_tia ;										//固定tia
							// r_tia = gains[led_cfg[2].tia_cap_group] ;

							i_current = led_cfg[0].led_cur ;
							i_tia = i_tia ;										//固定tia
							// i_tia = gains[led_cfg[0].tia_cap_group] ;
							
							ZSPD800X_EcgPpgInit();
							// test_reg();
							zsbm_status = ECG_GREEN_RED_IRD_NORMAL;	
						}
			
					}						
					// no wear, turn off green, turn on IR, then goto low FS IR detec mode
					else if(ZSBM_AGC_CAL_CONTINUE != ret)
					{
						ZSBM800X_CommonInit();
						ZSBM_IRDeteLowFs() ;
						zsbm_status = IR_DETEC_LOW_FS;
					}
				// }
				break;				
				
			case GREEN_NORMAL:	// 
				if(intstatus & Z_GREEN_CH) // TSD FIFO THRD interrupt
				{
					fifosize[0] = ZSBM800X_ReadFifoCount(Z_GREEN_CH)/6*6;	
					if( HRV_MODE == workmode && fifosize[0] >= HR_HRV_TSA_FIFO_INT_THRD)		// Limited to ACC fifo size, MAX 32 samples
					{
						fifosize[0] = 75;//HR_HRV_TSA_FIFO_INT_THRD;
						ZSBM800X_ReadFifoData(Z_GREEN_CH, zsbm_raw_dat_green, fifosize[0]);
						if(firstin)
							ppgdatready = fifosize[0] ;
					}	
					if( HR_MODE == workmode && fifosize[0] >= HR_HRV_TSA_FIFO_INT_THRD)
					{
						fifosize[0] = 300;//HR_HRV_TSA_FIFO_INT_THRD;
						ZSBM800X_ReadFifoData(Z_GREEN_CH, zsbm_raw_dat_green, fifosize[0]);
						if(firstin)
							ppgdatready = fifosize[0] ;
					}		
				}						
				for(i=0;i<fifosize[0]/3;i++)
				{
					// 1X pd
					ZSBM_PRINTF("%d\n", ((int32_t )((zsbm_raw_dat_green[3*i+2]<<16 | zsbm_raw_dat_green[3*i+1]<<8 | zsbm_raw_dat_green[3*i+0]<<0)<<8))/256 );
//					ZSBM_PRINTF("%d\n", (int16_t )(zsbm_raw_dat_green[2*i+1]<<8 | zsbm_raw_dat_green[2*i+0]<<0 ));
//					// 2X pd
//					ZSBM_PRINTF("%d %d\n", ((int32_t )((zsbm_raw_dat_green[6*i+2]<<16 | zsbm_raw_dat_green[6*i+1]<<8 | zsbm_raw_dat_green[6*i+0]<<0)<<8))/256, \
//					((int32_t )((zsbm_raw_dat_green[6*i+5]<<16 | zsbm_raw_dat_green[6*i+4]<<8 | zsbm_raw_dat_green[6*i+3]<<0)<<8))/256 );
					// 4X pd
//					ZSBM_PRINTF("%d %d %d %d\n", ((int32_t )((zsbm_raw_dat_green[12*i+2]<<16 | zsbm_raw_dat_green[12*i+1]<<8 | zsbm_raw_dat_green[12*i+0]<<0)<<8))/256, \
//																			((int32_t )((zsbm_raw_dat_green[12*i+5]<<16 | zsbm_raw_dat_green[12*i+4]<<8 | zsbm_raw_dat_green[12*i+3]<<0)<<8))/256, \
//																			((int32_t )((zsbm_raw_dat_green[12*i+8]<<16 | zsbm_raw_dat_green[12*i+7]<<8 | zsbm_raw_dat_green[12*i+6]<<0)<<8))/256, \
//																			((int32_t )((zsbm_raw_dat_green[12*i+11]<<16 | zsbm_raw_dat_green[12*i+10]<<8 | zsbm_raw_dat_green[12*i+9]<<0)<<8))/256 );
				}
//				Send_To_Hc04(zsbm_raw_dat_red, fifosize[1]);
//				HAL_UART_Transmit(&huart3, (uint8_t *)&zsbm_raw_dat_green, fifosize[0], 5000) ;
				firstin = 1;
				break;
				
			case IR_RED_NORMAL:						
				if(intstatus & Z_IR_CH) // TSD FIFO THRD interrupt
				{
					fifosize[1] = ZSBM800X_ReadFifoCount(Z_RED_CH)/6*6;
					fifosize[2] = ZSBM800X_ReadFifoCount(Z_IR_CH)/6*6;
					
					// 长短取短
					if(fifosize[1] > fifosize[2]) fifosize[1] = fifosize[2] ;
					if(fifosize[2] > fifosize[1]) fifosize[2] = fifosize[1] ;
					if(firstin)
						ppgdatready = fifosize[1] ;
										
					ZSBM800X_ReadFifoData(Z_RED_CH, zsbm_raw_dat_red, fifosize[1]);
					ZSBM800X_ReadFifoData(Z_IR_CH, zsbm_raw_dat_ir, fifosize[2]);
				}
				for(i=0;i<fifosize[1]/6;i++)
//				// 1X pd
//				ZSBM_PRINTF("%d %d\n", ((int32_t )((zsbm_raw_dat_red[3*i+2]<<16 | zsbm_raw_dat_red[3*i+1]<<8 | zsbm_raw_dat_red[3*i+0]<<0)<<8))/256, \
//															((int32_t )((zsbm_raw_dat_ir[3*i+2]<<16 | zsbm_raw_dat_ir[3*i+1]<<8 | zsbm_raw_dat_ir[3*i+0]<<0)<<8))/256 );
				// 2X pd
				ZSBM_PRINTF("%d %d\n", ((int32_t )((zsbm_raw_dat_red[6*i+2]<<16 | zsbm_raw_dat_red[6*i+1]<<8 | zsbm_raw_dat_red[6*i+0]<<0)<<8))/256, \
															((int32_t )((zsbm_raw_dat_ir[6*i+2]<<16 | zsbm_raw_dat_ir[6*i+1]<<8 | zsbm_raw_dat_ir[6*i+0]<<0)<<8))/256 );
//				Send_To_Hc04(zsbm_raw_dat_red, fifosize[1]);
				firstin = 1;
				break; 
			
			case ECG_NORMAL:		
				intstatus = ZSBM800X_ReadECGIntStatus();
//				ZSBM_PRINTF("intstatus2 = %x\n", intstatus);
				if(intstatus & 0x08)	// Clr ECG RLD global Int
					ZSBM800X_ClrECGRLDGlobalIntbit();
				if(intstatus & 0x01) // ECG FIFO THRD interrupt
				{
					fifosize[4] = ZSBM800X_ReadFifoCount(ECG_REG_SEL)/6*6;
//					ZSBM_PRINTF("fifosize[4] = %d\n", fifosize[4]);
					if(fifosize[4])
						ZSBM800X_ReadFifoData(ECG_REG_SEL, zsbm_raw_dat_ecg, fifosize[4]);
					intstatus = ZSBM800X_ReadECGRLDStatus()>>9;
//					ZSBM_PRINTF("intstatus3 = %x\n", intstatus);
					
//					LCD_Set_TextColor(Green) ;
					switch(intstatus)
					{
						case 0x0:
//							LCD_Display_String(2, 4, 2,"All On ");
							break ;
						
						case 0x0b:
//							LCD_Display_String(2, 4, 2,"RA Off ");
							fifosize[4] = 0 ;
							break ;
												
						case 0x0d:
//							LCD_Display_String(2, 4, 2,"LA Off ");
							fifosize[4] = 0 ;
							break ;
						
						case 0x0f:
						default:
//							LCD_Display_String(2, 4, 2,"All Off");
							fifosize[4] = 0 ;
							break ;
					}
				}
				ppgdatready = fifosize[4] ;
				for(i=0;i<fifosize[4]/3;i++)
				{
//				ZSBM_PRINTF("%d, %d, %d\n",  zsbm_raw_dat_ecg[3*i+0], zsbm_raw_dat_ecg[3*i+1], zsbm_raw_dat_ecg[3*i+2]);
//				ZSBM_PRINTF("%d\n", (int16_t )(zsbm_raw_dat_ecg[2*i+1]<<8 | zsbm_raw_dat_ecg[2*i+0]<<0));// | zsbm_raw_dat_ecg[3*i+0]<<0)<<8) );
					// 24bit
					printf("%d\n", (int32_t )(zsbm_raw_dat_ecg[3*i+2]<<24 | zsbm_raw_dat_ecg[3*i+1]<<16 | zsbm_raw_dat_ecg[3*i+0]<<8)>>8);				

//					// 16bit
//					ZSBM_PRINTF("%d\n", (int16_t)(zsbm_raw_dat_ecg[2*i+1]<<8 | zsbm_raw_dat_ecg[2*i+0] ));// , intstatus);				
//				Send_To_Hc04(zsbm_raw_dat_green, fifosize[0]);
				}

				break; 

			case ECG_GREEN_NORMAL:		
				intstatus = ZSBM800X_ReadECGIntStatus();
//				ZSBM_PRINTF("intstatus2 = %x\n", intstatus);
				if(intstatus & 0x08)	// Clr ECG RLD global Int
					ZSBM800X_ClrECGRLDGlobalIntbit();
				if(intstatus & 0x01) // ECG FIFO THRD interrupt
				{
					fifosize[4] = ZSBM800X_ReadFifoCount(ECG_REG_SEL)/2*2;
					fifosize[0] = ZSBM800X_ReadFifoCount(TSA_REG_SEL)/3*3;
					
					ppgdatready = fifosize[4]/2 ;	// 依照ECG对齐，表样本点个数
					
//					ZSBM_PRINTF("fifosize[4] = %d\n", fifosize[4]);
					if(fifosize[0])
						ZSBM800X_ReadFifoData(TSA_REG_SEL, zsbm_raw_dat_green, fifosize[0]);
					if(fifosize[4])
						ZSBM800X_ReadFifoData(ECG_REG_SEL, zsbm_raw_dat_ecg, fifosize[4]);
					
					intstatus = ZSBM800X_ReadECGRLDStatus()>>9;
//					ZSBM_PRINTF("intstatus3 = %x\n", intstatus);
					
					if(fifosize[0]/3 >= ppgdatready)
						for(i=0;i<ppgdatready;i++)
						{
						ZSBM_PRINTF("%d %d\n", ((int32_t )(zsbm_raw_dat_green[3*i+2]<<24 | zsbm_raw_dat_green[3*i+1]<<16 | zsbm_raw_dat_green[3*i+0]<<8))/256, \
							(int16_t)(zsbm_raw_dat_ecg[2*i+1]<<8 | zsbm_raw_dat_ecg[2*i+0]));				
						}
					else
					{
						for(i=0;i<fifosize[0]/3;i++)
						{
						ZSBM_PRINTF("%d %d\n", ((int32_t )(zsbm_raw_dat_green[3*i+2]<<24 | zsbm_raw_dat_green[3*i+1]<<16 | zsbm_raw_dat_green[3*i+0]<<8))/256, \
							(int16_t)(zsbm_raw_dat_ecg[2*i+1]<<8 | zsbm_raw_dat_ecg[2*i+0]));				
						}	
						for(;i<ppgdatready;i++)
						{
						ZSBM_PRINTF("%d %d\n", ((int32_t )(zsbm_raw_dat_green[fifosize[0]-1]<<24 | zsbm_raw_dat_green[fifosize[0]-2]<<16 | zsbm_raw_dat_green[fifosize[0]-3]<<8))/256, \
							(int16_t)(zsbm_raw_dat_ecg[2*i+1]<<8 | zsbm_raw_dat_ecg[2*i+0]));				
						}	
					}
				}
				ppgdatready = fifosize[4] ;
				break;
				
			case ECG_GREEN_RED_IRD_NORMAL:
				intstatus = ZSBM800X_ReadECGIntStatus();
				// ZSBM_PRINTF("intstatus2 = %x\n", intstatus);
				if(intstatus & 0x08)	// Clr ECG RLD global Int
					ZSBM800X_ClrECGRLDGlobalIntbit();
				if(intstatus & 0x01) // ECG FIFO THRD interrupt
				{
					fifosize[4] = ZSBM800X_ReadFifoCount(ECG_REG_SEL)/6*6;
					fifosize[2] = ZSBM800X_ReadFifoCount(TSC_REG_SEL)/6*6;
					fifosize[1] = ZSBM800X_ReadFifoCount(TSB_REG_SEL)/6*6;
//					fifosize[0] = ZSBM800X_ReadFifoCount(TSA_REG_SEL)/6*6;
//					ZSBM_PRINTF("%d %d %d %d\n", fifosize[4], fifosize[0], fifosize[1], fifosize[2]);
					ZSBM800X_ReadFifoData(ECG_REG_SEL, zsbm_raw_dat_ecg, fifosize[4]);
					ZSBM800X_ReadFifoData(TSC_REG_SEL, zsbm_raw_dat_ird, fifosize[2]);
					ZSBM800X_ReadFifoData(TSB_REG_SEL, zsbm_raw_dat_red, fifosize[1]);
//					ZSBM800X_ReadFifoData(TSA_REG_SEL, zsbm_raw_dat_green, fifosize[0]);
					
					if(fifosize[1] > fifosize[2]) fifosize[1] = fifosize[2] ;
//					if(fifosize[4] > fifosize[0]) fifosize[4] = fifosize[0] ;
//					
					if(fifosize[4] > fifosize[1]) fifosize[4] = fifosize[1] ;
					
//  					for(i=0; i<fifosize[4]/3; i++ ){
//  							ZSBM_PRINTF("%d %d %d\n", ((int32_t )((zsbm_raw_dat_ecg[3*i+2]<<24 | zsbm_raw_dat_ecg[3*i+1]<<16 | zsbm_raw_dat_ecg[3*i+0]<<8)))>>8, \
//  																						((int32_t )((zsbm_raw_dat_red[3*i+2]<<24 | zsbm_raw_dat_red[3*i+1]<<16 | zsbm_raw_dat_red[3*i+0]<<8)))>>8, \
//  																						((int32_t )((zsbm_raw_dat_ird[3*i+2]<<24 | zsbm_raw_dat_ird[3*i+1]<<16 | zsbm_raw_dat_ird[3*i+0]<<8)))>>8);// \
//  //																						((int32_t )((zsbm_raw_dat_green[3*i+2]<<24 | zsbm_raw_dat_green[3*i+1]<<16 | zsbm_raw_dat_green[3*i+0]<<8)))>>8);
					
//  					}

					extern QueueHandle_t sensorDataQueue; // 数据队列
					SensorData_t data;
 					for(i=0; i<fifosize[4]/3; i++ ){
						data.ecg_data = ((int32_t)((zsbm_raw_dat_ecg[3*i+2] << 24 | 
                                                   zsbm_raw_dat_ecg[3*i+1] << 16 | 
                                                   zsbm_raw_dat_ecg[3*i+0] << 8))) >> 8;
                        data.red_data = ((int32_t)((zsbm_raw_dat_red[3*i+2] << 24 | 
                                                   zsbm_raw_dat_red[3*i+1] << 16 | 
                                                   zsbm_raw_dat_red[3*i+0] << 8))) >> 8;
                        data.ird_data = ((int32_t)((zsbm_raw_dat_ird[3*i+2] << 24 | 
                                                   zsbm_raw_dat_ird[3*i+1] << 16 | 
                                                   zsbm_raw_dat_ird[3*i+0] << 8))) >> 8;

                        // 发送到队列
                        // xQueueSend(sensorDataQueue, &data, portMAX_DELAY);

                        // 通过UART发送
                        char uart_buf[50];
//////////////////////////如果要在QTQ上位机显示，不要打印回车就好
                        int len = snprintf(uart_buf, sizeof(uart_buf), "%ld %ld %ld\r\n", 
                                         (long)data.ecg_data, (long)data.red_data, (long)data.ird_data);
						// printf("Sending...\r\n"); // 如果有其他调试 UART
                        HAL_UART_Transmit(&huart1, (uint8_t*)uart_buf, len, 100); // 替换huartx为您的UART实例
						// printf("Sent.\r\n");
					}

//					uint8_t ecg_data,red_data,ird_data;
//					
//					for(i=0; i<fifosize[4]/3; i++ ){
//							ecg_data = ((int32_t )((zsbm_raw_dat_ecg[3*i+2]<<24 | zsbm_raw_dat_ecg[3*i+1]<<16 | zsbm_raw_dat_ecg[3*i+0]<<8)))>>8;
//							red_data = ((int32_t )((zsbm_raw_dat_red[3*i+2]<<24 | zsbm_raw_dat_red[3*i+1]<<16 | zsbm_raw_dat_red[3*i+0]<<8)))>>8;
//							ird_data = ((int32_t )((zsbm_raw_dat_ird[3*i+2]<<24 | zsbm_raw_dat_ird[3*i+1]<<16 | zsbm_raw_dat_ird[3*i+0]<<8)))>>8;
//							// printf("%d %d %d\n", ecg_data, red_data, ird_data);
//							ZSBM_PRINTF("%d %d %d\n",ecg_data,red_data,ird_data);// \
////																						((int32_t )((zsbm_raw_dat_green[3*i+2]<<24 | zsbm_raw_dat_green[3*i+1]<<16 | zsbm_raw_dat_green[3*i+0]<<8)))>>8);
////						TF_Write_ECG_PPG_Data(new_filename,ecg_data, red_data, ird_data);
//					}
				}					
				break;
				
			case IR_RED_GREEN_NORMAL:
				if(intstatus & Z_IR_CH) // TSD FIFO THRD interrupt
				{
					fifosize[0] = ZSBM800X_ReadFifoCount(Z_GREEN_CH)/12*12;
					fifosize[1] = ZSBM800X_ReadFifoCount(Z_RED_CH)/12*12;
					fifosize[2] = ZSBM800X_ReadFifoCount(Z_IR_CH)/12*12;
					
//					ZSBM_PRINTF("G_size=%d, R_size=%d, IR_size=%d\n", fifosize[0], fifosize[1], fifosize[2]) ;
					
//					// 长短取短
//					if(fifosize[1] > fifosize[2]) fifosize[1] = fifosize[2] ;
//					if(fifosize[2] > fifosize[1]) fifosize[2] = fifosize[1] ;
										
					ZSBM800X_ReadFifoData(Z_GREEN_CH, zsbm_raw_dat_green, fifosize[0]);
					ZSBM800X_ReadFifoData(Z_RED_CH, zsbm_raw_dat_red, fifosize[1]);
					ZSBM800X_ReadFifoData(Z_IR_CH, zsbm_raw_dat_ir, fifosize[2]);
					
//					if(fifosize[0]>fifosize[1])
//						ppgdatready = fifosize[1] ;
//					else
						ppgdatready = fifosize[0] ;
				}
				// convert to 32bit data width
//				memset((uint8_t *)zspd_algo_buffer, 0 ,ppgdatready*4);
//				for(i=0; i<ppgdatready/12; i++)
//				{
//					memcpy((uint8_t *)(zspd_algo_buffer + 48*i +  1), (uint8_t *)(zsbm_raw_dat_green + 12*i + 0), 3);
//					memcpy((uint8_t *)(zspd_algo_buffer + 48*i +  5), (uint8_t *)(zsbm_raw_dat_green + 12*i + 3), 3);
//					memcpy((uint8_t *)(zspd_algo_buffer + 48*i +  9), (uint8_t *)(zsbm_raw_dat_green + 12*i + 6), 3);
//					memcpy((uint8_t *)(zspd_algo_buffer + 48*i + 13), (uint8_t *)(zsbm_raw_dat_green + 12*i + 9), 3);

//					memcpy((uint8_t *)(zspd_algo_buffer + 48*i + 17), (uint8_t *)(zsbm_raw_dat_red + 12*i + 0), 3);
//					memcpy((uint8_t *)(zspd_algo_buffer + 48*i + 21), (uint8_t *)(zsbm_raw_dat_red + 12*i + 3), 3);
//					memcpy((uint8_t *)(zspd_algo_buffer + 48*i + 25), (uint8_t *)(zsbm_raw_dat_red + 12*i + 6), 3);
//					memcpy((uint8_t *)(zspd_algo_buffer + 48*i + 29), (uint8_t *)(zsbm_raw_dat_red + 12*i + 9), 3);
//					
//					memcpy((uint8_t *)(zspd_algo_buffer + 48*i + 33), (uint8_t *)(zsbm_raw_dat_ir + 12*i + 0), 3);
//					memcpy((uint8_t *)(zspd_algo_buffer + 48*i + 37), (uint8_t *)(zsbm_raw_dat_ir + 12*i + 3), 3);
//					memcpy((uint8_t *)(zspd_algo_buffer + 48*i + 41), (uint8_t *)(zsbm_raw_dat_ir + 12*i + 6), 3);
//					memcpy((uint8_t *)(zspd_algo_buffer + 48*i + 45), (uint8_t *)(zsbm_raw_dat_ir + 12*i + 9), 3);
//					
//					int32_t *pppptr = (int32_t *)(zspd_algo_buffer + 48*i + 0);
//					printf("%d %d %d %d %d %d %d %d %d %d %d %d\n",*(pppptr+0)>>8, *(pppptr+1)>>8, *(pppptr+2)>>8, *(pppptr+3)>>8,\
//																													*(pppptr+4)>>8, *(pppptr+5)>>8, *(pppptr+6)>>8, *(pppptr+7)>>8,\
//																													*(pppptr+8)>>8, *(pppptr+9)>>8, *(pppptr+10)>>8, *(pppptr+11)>>8);
//				}

				for(i=0; i<ppgdatready/3; i++)
				{
					printf("%d %d %d\n", ((int32_t )(zsbm_raw_dat_green[3*i+2]<<24 | zsbm_raw_dat_green[3*i+1]<<16 | zsbm_raw_dat_green[3*i+0]<<8))>>8, \
																((int32_t )(zsbm_raw_dat_red[3*i+2]<<24 | zsbm_raw_dat_red[3*i+1]<<16 | zsbm_raw_dat_red[3*i+0]<<8))>>8, \
																((int32_t )(zsbm_raw_dat_ir[3*i+2]<<24 | zsbm_raw_dat_ir[3*i+1]<<16 | zsbm_raw_dat_ir[3*i+0]<<8))>>8) ;
				}
				
// using DMA to send char for less data transfer
//				HAL_UART_Transmit_DMA(&huart3, (uint8_t *)zspd_algo_buffer, ppgdatready * 4) ;
				break;
			
			default:
				break ;
		}
	}
}

ZSBM_ERROR_CODE_T ZSBM800X_Init(WORK_MODE_T mode)
{
	workmode = mode;
	if(NULL_MODE == workmode){
		ZSBM800X_GpioIntConfig(0);
		ZSBM800X_SoftReset();	
		return ZSBM_OK;
	}
	
	if(ECG_MODE == workmode)
	{
		ZSPD800X_EcgInit();
		zsbm_status = ECG_NORMAL ;
		ZSBM800X_DumpRegister();
		return ZSBM_OK;
	}

	 if(ECG_PPG_MODE == workmode)
	 {
	 	ZSPD800X_EcgPpgInit();
	 	zsbm_status = ECG_GREEN_RED_IRD_NORMAL ;
	 	ZSBM800X_DumpRegister();
	 	return ZSBM_OK;
	 }
	/////////////////////////修改为ppg自适应模式///////////////////////////////////////////
	// if(ECG_PPG_MODE == workmode)
	// {
	// 	ZSBM800X_CommonInit();
	// 	zsbm_status = IR_DETEC_LOW_FS ;
	// 	ZSBM_IRDeteLowFs() ;
	// 	return ZSBM_OK;
	// }

	if(TOOLING_MODE == workmode)
	{
		ZSBM800X_CommonInit();
		ZSBM_IrRedGreenNormal(SAMPLE_FS_200HZ) ;
		zsbm_status = IR_RED_GREEN_NORMAL;
		return ZSBM_OK;
	}
	
	ZSBM800X_CommonInit();
	zsbm_status = IR_DETEC_LOW_FS ;
	ZSBM_IRDeteLowFs() ;	
	return ZSBM_OK;
}

ZSBM_ERROR_CODE_T ZSBM800X_32KClkOutPutToIO0( void )
{
	zsbm_status = RESET_STATUS;
	ZSBM800X_SoftReset() ;
	ZSBM800X_GpioIntConfig(0);	// Disable GPIO IRQ
	ZSBM800X_ConfigIntOutputPin( GPIO0_CFG_OUT_PP | GPIO0_POL_NEG | GPIO0_OUT_SOURCE_LOW_CLK) ;
	ZSBM800X_TimerSlotStart(0);
}

ZSBM_ERROR_CODE_T ZSBM800X_24MClkDivideBy750OutPutToIO0( void )
{
	uint16_t tmp;
	zsbm_status = RESET_STATUS;
	
	ZSBM800X_SoftReset() ;
	
	tmp = OP_EN | OSC32K_EN ;
	/*enable time slot, go!*/
	ZSBM_WRITE_REG16(0x02, &tmp, 1);
	
	ZSBM800X_DelayMs(200);
	ZSBM800X_SoftReset() ;

	ZSBM800X_GpioIntConfig(0);	// Disable GPIO IRQ
	ZSBM800X_ConfigIntOutputPin( GPIO0_CFG_OUT_PP | GPIO0_POL_NEG | GPIO0_OUT_SOURCE_HIGH_CLK) ;

	tmp = 0x2000 | OP_EN | OSC32K_EN | ECG_EN;
	
	/*enable time slot, go!*/
	ZSBM_WRITE_REG16(0x02, &tmp, 1);
}

void ZSBM800X_DumpRegister( void)
{
	uint16_t regs[0x100] ;
	uint16_t i;
	ZSBM_PRINTF("Dump register:\r\n" );
	ZSBM800X_ReadMultyWord(0, &regs[0], 0x100);
	for(i=0;i<0x57;i++)
	{
		ZSBM_PRINTF("reg[0x%x] = 0x%x \n", i, regs[i]);
	}
	ZSBM_PRINTF("Dump register done!\r\n" );
}

void test_reg(){
	ZSBM_PRINTF("\r\n" );
	ZSBM_PRINTF("\r\n" );
	ZSBM_PRINTF("0X0001 register:\r\n" );
	// ZSBM_WRITE_REG16(0x0f, &tmp, 1);
	uint16_t tmp = 0x0001; // 要写入的值
    uint16_t regs[0x45];  // 用于存储从 0x00 到 0x44 读取的值
    uint16_t i;

    // 将 0x0001 写入寄存器 0x0F
    ZSBM_WRITE_REG16(0x0F, &tmp, 1);

    // 读取从 0x00 到 0x44 的寄存器
    ZSBM800X_ReadMultyWord(0x00, &regs[0], 0x45); // 读取 0x45 个寄存器

    // 打印读取的寄存器值
    for (i = 0; i < 0x45; i++) {
        ZSBM_PRINTF("reg[0x%02x] = 0x%04x\r\n", i, regs[i]);
    }
	
	ZSBM_PRINTF("\r\n" );
	tmp = 0x0002; // 要写入的值
    // 将 0x0002 写入寄存器 0x0F
	ZSBM_PRINTF("0X0002 register:\r\n" );
    ZSBM_WRITE_REG16(0x0F, &tmp, 1);
    // 读取从 0x00 到 0x44 的寄存器
    ZSBM800X_ReadMultyWord(0x00, &regs[0], 0x45); // 读取 0x45 个寄存器
    // 打印读取的寄存器值
    for (i = 0; i < 0x45; i++) {
        ZSBM_PRINTF("reg[0x%02x] = 0x%04x\r\n", i, regs[i]);
    }

	ZSBM_PRINTF("\r\n" );
	tmp = 0x0004; // 要写入的值
    // 将 0x0004 写入寄存器 0x0F
	ZSBM_PRINTF("0X0004 register:\r\n" );
    ZSBM_WRITE_REG16(0x0F, &tmp, 1);
    // 读取从 0x00 到 0x44 的寄存器
    ZSBM800X_ReadMultyWord(0x00, &regs[0], 0x45); // 读取 0x45 个寄存器
    // 打印读取的寄存器值
    for (i = 0; i < 0x45; i++) {
        ZSBM_PRINTF("reg[0x%02x] = 0x%04x\r\n", i, regs[i]);
    }

	ZSBM_PRINTF("\r\n" );
	tmp = 0x0008; // 要写入的值
    // 将 0x0008 写入寄存器 0x0F
	ZSBM_PRINTF("0X0008 register:\r\n" );
    ZSBM_WRITE_REG16(0x0F, &tmp, 1);
    // 读取从 0x00 到 0x44 的寄存器
    ZSBM800X_ReadMultyWord(0x00, &regs[0], 0x45); // 读取 0x45 个寄存器
    // 打印读取的寄存器值
    for (i = 0; i < 0x45; i++) {
        ZSBM_PRINTF("reg[0x%02x] = 0x%04x\r\n", i, regs[i]);
    }

	
	while(1);

}

extern SemaphoreHandle_t xzsbmSemaphore;
void sensorTask(void *pvParameters)
{
//  信号量驱动
	while (1)
	{
		{
			// UBaseType_t stackMark = uxTaskGetStackHighWaterMark(NULL);
			// printf("传感器任务堆栈剩余: %lu 字节\n", stackMark * sizeof(StackType_t));
			// printf("zsbm_int_flag = %d\r\n",zsbm_int_flag);
			if (xSemaphoreTake(xzsbmSemaphore, portMAX_DELAY) == pdTRUE)
			{
				ZSBM800X_DataHandle(); // 处理中断数据
				// printf("ZSBM800X_DataHandle\r\n");
			}
		}
	}
//  裸机驱动	
    // while (1)
    // {
	// 	// UBaseType_t stackMark = uxTaskGetStackHighWaterMark(NULL);
	// 	// printf("传感器任务堆栈剩余: %lu 字节\n", stackMark * sizeof(StackType_t));
	// 	// printf("sensorTask++\r\n");
	// 	printf("zsbm_int_flag = %d\r\n",zsbm_int_flag);
    //     // if (zsbm_int_flag) // 检查中断标志
    //     // {
	// 	// taskENTER_CRITICAL();
	// 	vTaskSuspendAll();
    //     ZSBM800X_DataHandle(); // 处理ZSBM800X数据
	// 	// taskEXIT_CRITICAL();
	// 	xTaskResumeAll();
    //     // }
    //     vTaskDelay(1000); // 短暂延迟，避免忙等待
	// 	// printf("sensorTask--\r\n");
    // }

//  事件驱动
    // while (1)
    // {
	// 	// UBaseType_t stackMark = uxTaskGetStackHighWaterMark(NULL);
	// 	// printf("传感器任务堆栈剩余: %lu 字节\n", stackMark * sizeof(StackType_t));
	// 	// printf("sensorTask++\r\n");

	// 	ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1000));

	// 	printf("zsbm_int_flag = %d\r\n",zsbm_int_flag);
    //     // if (zsbm_int_flag) // 检查中断标志
    //     // {
	// 	taskENTER_CRITICAL();
    //     ZSBM800X_DataHandle(); // 处理ZSBM800X数据
	// 	taskEXIT_CRITICAL();
    //     // }
    //     // vTaskDelay(1000); // 短暂延迟，避免忙等待
	// 	// printf("sensorTask--\r\n");
    // }
	
}
