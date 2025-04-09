#include <stdio.h>
#include <string.h>
#include "zsbm800x_drv.h"
#include "zsbm800x_port.h"
#include "usart.h"
//#include "delay.h"
#include "zsbm800x_ecg_ra.h"
//#include "hc04ble.h"


#define ECG_MEDICAL_16BIT_250HZ			(0x0660)
#define ECG_MEDICAL_16BIT_300HZ			(0x0668)
#define ECG_MEDICAL_16BIT_500HZ			(0x0650)
#define ECG_MEDICAL_16BIT_600HZ			(0x0658)
#define ECG_MEDICAL_16BIT_1000HZ		(0x0640)
#define ECG_MEDICAL_16BIT_1200HZ		(0x0648)
#define ECG_MEDICAL_24BIT_250HZ			(0x0620)	
#define ECG_MEDICAL_24BIT_300HZ			(0x0628)
#define ECG_MEDICAL_24BIT_500HZ			(0x0610)
#define ECG_MEDICAL_24BIT_600HZ			(0x0618)
#define ECG_MEDICAL_24BIT_1000HZ		(0x0600)
#define ECG_MEDICAL_24BIT_1200HZ		(0x0608)

#define PPG_SAMPLE_RATE	(250)
//#define	ECG_PLUS_PPG	1	//PPG + ECG MODE, 300Hz sample rate

uint8_t	readbuff[600];//300*2

extern uint16_t zsbm_clk_frq ;
extern uint16_t gre_led_cur, gre_led_tia;

extern uint8_t g_current, r_current, i_current;
extern uint16_t g_tia, r_tia, i_tia;

void Reg_W_Add_Data(uint8_t add, uint16_t data)
{
	/*
	uint8_t data_write[2];

	data_write[0] = data>>8;
	data_write[1] = data;
	*/
	ZSBM_WRITE_REG16(add, &data, 1);
}

uint16_t Reg_R_Data(uint8_t add)
{
	uint16_t data;
	
	ZSBM_READ_REG16(add, &data, 1);
		
	return data;
}


void ZSPD800X_EcgPpgInit(void)
{	
	zsbm_status = RESET_STATUS;
// config ppg
	Reg_W_Add_Data(0x00,0x0001); //��λ
	
	Reg_W_Add_Data(0x00,0x0001);
	
	Reg_W_Add_Data(0x02,0x0080);	// Sample rate
	Reg_W_Add_Data(0x03,(zsbm_clk_frq/PPG_SAMPLE_RATE)>>16);
	Reg_W_Add_Data(0x04,(zsbm_clk_frq/PPG_SAMPLE_RATE)&0xffff);
	
	Reg_W_Add_Data(0x0A,0x0008);  //GPIO0_LO_INT �������ж����

	Reg_W_Add_Data(0x0B,0x0041);  //GPIO1/0   0100:�ж�,01�����ģʽ��Push-Pull�� OK  ����IO�ն����
	
	
	Reg_W_Add_Data(0x0F,0x000F);	//Sel all channel	
	
	Reg_W_Add_Data(0x19,0x4343);	
	
	Reg_W_Add_Data(0x1A,0x4343);	
	
	Reg_W_Add_Data(0x1B,0x4141);	
	
	Reg_W_Add_Data(0x1C,0x4141);

	Reg_W_Add_Data(0x25,0x0FF0);	
	

	Reg_W_Add_Data(0x0F,0x0001);	//Sel TSA Green channel	

	Reg_W_Add_Data(0x10,0x8002);	// 24 BIT WORD SIZE, 1x PD USE IN1

	Reg_W_Add_Data(0x27,(g_tia & 0xFF70) | TIA_VREF | TIA_EN);	// TIA
	
	Reg_W_Add_Data(0x14,(g_current & 0x007F) | (((1<<0) & 0x003f)<<8));

	Reg_W_Add_Data(0x15,(g_current & 0x007F) | (((1<<3) & 0x003f)<<8));

	Reg_W_Add_Data(0x23,0x0007);		// 8x pulse	

	Reg_W_Add_Data(0x2D,0x0100);	//GREEN��ʼ��ַһ����1Kƽ�ֵ�

	Reg_W_Add_Data(0x2E,0xff);	//FIFO��� һ����1Kƽ�ֵ� 255+1 byte


	Reg_W_Add_Data(0x0F,0x0002);	//Sel TSB Green channel	

	Reg_W_Add_Data(0x10,0x8002);	// 24 BIT WORD SIZE, 1x PD USE IN1

	Reg_W_Add_Data(0x27,(r_tia & 0xFF70) | TIA_VREF | TIA_EN);	// TIA
	
	Reg_W_Add_Data(0x14,(r_current & 0x007F) | (((1<<1) & 0x003f)<<8));

	Reg_W_Add_Data(0x15,(r_current & 0x007F) | (((1<<4) & 0x003f)<<8));

	Reg_W_Add_Data(0x23,0x001f);		// 16x pulse	

	Reg_W_Add_Data(0x2D,0x0200);	//GREEN��ʼ��ַһ����1Kƽ�ֵ�

	Reg_W_Add_Data(0x2E,0xff);	//FIFO��� һ����1Kƽ�ֵ� 255+1 byte
	

	Reg_W_Add_Data(0x0F,0x0004);	//Sel TSC Green channel	

	Reg_W_Add_Data(0x10,0x8002);	// 24 BIT WORD SIZE, 1x PD USE IN1

	Reg_W_Add_Data(0x27,(i_tia & 0xFF70) | TIA_VREF | TIA_EN);	// TIA
	
	Reg_W_Add_Data(0x14,(i_current & 0x007F) | (((1<<2) & 0x003f)<<8));

	Reg_W_Add_Data(0x15,(i_current & 0x007F) | (((1<<5) & 0x003f)<<8));

	Reg_W_Add_Data(0x23,0x001f);		// 16x pulse	

	Reg_W_Add_Data(0x2D,0x0300);	//GREEN��ʼ��ַһ����1Kƽ�ֵ�

	Reg_W_Add_Data(0x2E,0xff);	//FIFO��� һ����1Kƽ�ֵ� 255+1 byte
	
//	Reg_W_Add_Data(0x2F,0x112B);	//������С  �ж���ֵ��С	239+1 byte

// config ecg
	
//	Reg_W_Add_Data(0x07,0x0001);  //
	
	//Reg_W_Add_Data(0x0C,0x4240);	//BIT15:disable_ldoana=0  BIT14:bgldo_en=1     4240 ECG 1000
	//Reg_W_Add_Data(0x0C,0x4270);	//BIT15:disable_ldoana=0  BIT14:bgldo_en=1   ������250
	//Reg_W_Add_Data(0x0C,0x4250);	//BIT15:disable_ldoana=0  BIT14:bgldo_en=1   ������500
//	Reg_W_Add_Data(0x0C,0x4628);	//BIT15:disable_ldoana=0  BIT14:bgldo_en=1   ������500
	
	Reg_W_Add_Data(0x0C,ECG_MEDICAL_24BIT_250HZ); 		// 24bit & 250 Hz 
	
	Reg_W_Add_Data(0x0F,0x0010);	////ѡ��д��ECG�ļĴ���ҳ
	
	Reg_W_Add_Data(0x2D,0x0000);	//ECG��ʼ��ַһ����1Kƽ�ֵ�

	Reg_W_Add_Data(0x2E,0xff);	//FIFO��� һ����1Kƽ�ֵ� 255+1 byte

	Reg_W_Add_Data(0x2F,0x1095);	//������С  �ж���ֵ��С	239+1 byte

//	Reg_W_Add_Data(0x43,0x3F08);	//3.3V����

#if 1	//ECG_DC_CONNECT

	Reg_W_Add_Data(0x37,0x8310);  //ECG_INPUTRC=01 DCLO  ������Ĵ���

	Reg_W_Add_Data(0x39,0x2300);  //ʹ��������   RLD

//	Reg_W_Add_Data(0x37,0x4B10);  //ECG_INPUTRC=01 DCLO  ������Ĵ���

//	Reg_W_Add_Data(0x39,0x207E);  //ʹ��������   RLD
#else

	//Reg_W_Add_Data(0x37,(0x7408|0x0007));  //AC ģʽ������ʹ��  �����λΪ��������趨ֵ
	Reg_W_Add_Data(0x37,(0x5408|0x0007));  //AC ģʽ������ʹ��  �����λΪ��������趨ֵ

	Reg_W_Add_Data(0x38,500);  //ECG AC���ģʽ���趨������ֵ

	Reg_W_Add_Data(0x39,0x2071);  //ACʹ��������   RLD
	
#endif

	Reg_W_Add_Data(0x3E,0x0101);  //lpfch0_cfg=ECG lpfch1_cfg=ECG
	
	Reg_W_Add_Data(0x3F,0x0101);  //lpfch0_cfg=ECG lpfch1_cfg=ECG

	Reg_W_Add_Data(0x02,0x0396);   //24Mʱ��ʹ�ܣ�ECG_ENʹ�ܣ�ECG_EN_OP_EN  ʹ��32Kʱ��  TSA/TSB/TSC��

//	zsbm_status = ECG_GREEN_NORMAL;
}


void ZSPD800X_EcgInit(void)
{	
	Reg_W_Add_Data(0x00,0x0001); //��λ
	
	Reg_W_Add_Data(0x02,0x0091);
	
	ZSBM800X_DelayMs(250);
	
	Reg_W_Add_Data(0x00,0x0001); //��λ
	
//	Reg_W_Add_Data(0x07,0x0001);  //
														
	Reg_W_Add_Data(0x0A,0x0008);  //GPIO0_LO_INT �������ж����

	Reg_W_Add_Data(0x0B,0x0041);  //GPIO1/0   0100:�ж�,01�����ģʽ��Push-Pull�� OK  ����IO�ն����
	
	Reg_W_Add_Data(0x0C,ECG_MEDICAL_24BIT_250HZ); 
	
	Reg_W_Add_Data(0x0F,0x0010);	////ѡ��д��ECG�ļĴ���ҳ
	
	Reg_W_Add_Data(0x2D,0x0000);	//ECG��ʼ��ַһ����1Kƽ�ֵ�

	Reg_W_Add_Data(0x2E,0x03DF);	//FIFO��� һ����1Kƽ�ֵ� 991+1 byte

	Reg_W_Add_Data(0x2F,0x1257);	//������С  �ж���ֵ��С	599+1 byte

//	Reg_W_Add_Data(0x43,0x3F08);	//3.3V����

#if 1	//ECG_DC_CONNECT

	Reg_W_Add_Data(0x37,0x8310);  //ECG_INPUTRC=01 DCLO  ������Ĵ���

	Reg_W_Add_Data(0x39,0x6300);  //ʹ��������   RLD	0x6300

//	Reg_W_Add_Data(0x37,0x4B10);  //ECG_INPUTRC=01 DCLO  ������Ĵ���

//	Reg_W_Add_Data(0x39,0x207E);  //ʹ��������   RLD
#else

	//Reg_W_Add_Data(0x37,(0x7408|0x0007));  //AC ģʽ������ʹ��  �����λΪ��������趨ֵ
	Reg_W_Add_Data(0x37,(0x5408|0x0007));  //AC ģʽ������ʹ��  �����λΪ��������趨ֵ

	Reg_W_Add_Data(0x38,500);  //ECG AC���ģʽ���趨������ֵ

	Reg_W_Add_Data(0x39,0x2071);  //ACʹ��������   RLD
	
#endif

	Reg_W_Add_Data(0x3E,0x0101);  //lpfch0_cfg=ECG lpfch1_cfg=ECG
	
	Reg_W_Add_Data(0x3F,0x0101);  //lpfch0_cfg=ECG lpfch1_cfg=ECG

	Reg_W_Add_Data(0x02,0x0390);   //24Mʱ��ʹ�ܣ�ECG_ENʹ�ܣ�ECG_EN_OP_EN  ʹ��32Kʱ��  TSA��
}

