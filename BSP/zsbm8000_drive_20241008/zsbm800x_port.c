#include "zsbm800x_port.h"
#include "i2c.h"
//#include "public.h"

//#define COMMU_USE_SPI

// big_endian to little_endian
static uint16_t Bswap_16 (uint16_t n)
{
	return (((uint16_t)(n) & 0x00ff ) << 8 ) | \
				(((uint16_t)(n) & 0xff00 ) >> 8 ) ;
}

/**
*function:millisecond delay, used for power up reset delay
*parameter:ms, time in millisecond
*return:none
*date:2022/01/23
**/
void ZSBM800X_DelayMs(uint32_t ms)
{
		HAL_Delay(ms);		
}

#ifdef COMMU_USE_SPI

#define ZSBM_SPI_CS_L  HAL_GPIO_WritePin(ZS_CS_GPIO_Port, ZS_CS_Pin, GPIO_PIN_RESET)
#define ZSBM_SPI_CS_H  HAL_GPIO_WritePin(ZS_CS_GPIO_Port, ZS_CS_Pin, GPIO_PIN_SET)

extern SPI_HandleTypeDef hspi2;

uint16_t ZSBM800X_ReadMultyWord (uint8_t regaddr , uint16_t *rcvbuf, uint16_t length)
{
	uint16_t tmplen = length;
	uint8_t address = regaddr<<1;
	
	ZSBM_SPI_CS_L;
	HAL_SPI_Transmit(&hspi2, &address, 1, 200);
	HAL_SPI_Receive(&hspi2, (uint8_t *)rcvbuf, tmplen*2, 200);
	ZSBM_SPI_CS_H;

	/* 大端序改成小端序 */
	while(tmplen--)
	{
		*rcvbuf = Bswap_16(*rcvbuf);
		rcvbuf++ ;
	}
	return length;	
}

uint16_t ZSBM800X_ReadMultyByte (uint8_t regaddr , uint8_t *rcvbuf, uint16_t length)
{
	uint16_t tmplen = length;
	uint8_t address = regaddr<<1;
	
	ZSBM_SPI_CS_L;
	HAL_SPI_Transmit(&hspi2, &address, 1, 200);
	HAL_SPI_Receive(&hspi2, rcvbuf, tmplen, 200);
	ZSBM_SPI_CS_H;

	return length;	
}

uint16_t ZSBM800X_WriteMultyWord (uint8_t regaddr , uint16_t *regdat, uint16_t length)
{
	// Max 38 regs once
	uint8_t i, tmp[80];
	
	tmp[0] = (regaddr<<1) + 1; 
	
	/* 大端序数据写入 */
	for(i=0; i< length; i++)
	{
		tmp[2*i+1] = *regdat >> 8 ;
		tmp[2*i + 2] = *regdat & 0xff ;		
		regdat++ ;
	}

	ZSBM_SPI_CS_L;
	HAL_SPI_Transmit(&hspi2, tmp, length*2 + 1, 200);
	ZSBM_SPI_CS_H;

	return length;	
}

#else
/**
*function:Read multy word (16bit register) from ZSBM8000
*parameter:regaddr, register address
*parameter:rcvbuf, read data first address
*parameter:length, data length
*return: data length
*date:2022/10/23
**/
uint16_t ZSBM800X_ReadMultyWord (uint8_t regaddr , uint16_t *rcvbuf, uint16_t length)
{
	uint16_t tmplen = length;
	HAL_I2C_Master_Transmit(&hi2c2, ZSBM8XXX7BITADDR << 1, &regaddr, 1, 1000) ;
	HAL_I2C_Master_Receive(&hi2c2, ZSBM8XXX7BITADDR << 1, (uint8_t *)rcvbuf, tmplen*2, 1000) ;

	while(tmplen--)
	{
		*rcvbuf = Bswap_16(*rcvbuf);
		rcvbuf++ ;
	}

	return length;	
}

uint16_t ZSBM800X_ReadMultyByte (uint8_t regaddr , uint8_t *rcvbuf, uint16_t length)
{
	uint16_t tmplen = length;
	HAL_I2C_Master_Transmit(&hi2c2, ZSBM8XXX7BITADDR << 1, &regaddr, 1, 1000) ;
	HAL_I2C_Master_Receive(&hi2c2, ZSBM8XXX7BITADDR << 1, (uint8_t *)rcvbuf, tmplen, 1000) ;
/*
	while(tmplen--)
	{
		*rcvbuf = Bswap_16(*rcvbuf);
		rcvbuf++ ;
	}
*/
	return length;	
}
/**
*function:Write multy word (16bit register) to ZSBM8000
*parameter:regaddr, register address
*parameter:regdat, reg values to be write
*parameter:length, data length
*return: data length
*date:2022/10/23
**/
uint16_t ZSBM800X_WriteMultyWord (uint8_t regaddr , uint16_t *regdat, uint16_t length)
{
	// Max 38 regs once
	uint8_t i, tmp[80];
	
	tmp[0] = regaddr ; 
	
	for(i=0; i< length; i++)
	{
		tmp[2*i+1] = *regdat >> 8 ;
		tmp[2*i + 2] = *regdat & 0xff ;		
		regdat++ ;
	}

	HAL_I2C_Master_Transmit(&hi2c2, ZSBM8XXX7BITADDR << 1, tmp, length*2 + 1, 1000) ;
	return length;	
}
#endif

/**
*function:Clear host CPU pending GPIO interrupt then enable GPIO interrupt
*parameter:none
*return: none
*date:2022/10/23
**/
void ZSBM800X_GpioIntEnable(void)
{
	HAL_NVIC_ClearPendingIRQ(EXTI9_5_IRQn);
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

/**
*function:Disable host CPU GPIO interrupt
*parameter:none
*return: none
*date:2022/10/23
**/
void ZSBM800X_GpioIntDisable(void)
{
  	HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
}
