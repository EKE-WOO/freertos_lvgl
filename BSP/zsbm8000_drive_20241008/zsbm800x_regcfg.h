/******************************************************************************
* 文件名称：zsbm800x_regcfg.h
* 摘 要：支持IIC和SPI通讯
 
* 当前版本：V1.0

* 创建：张建强
* 修订：
* 创建日期：2024年01月26日
* 完成日期：
******************************************************************************/

#ifndef _ZSBM800X_REGCFG_H
#define _ZSBM800X_REGCFG_H

  //reg 0x00
  //芯片复位
  #define SOFT_RESET				      		0x0001

  //reg 0x01
  #define OVER_TEMP_FLAG              0x0080
  #define SPI_MODE_SPI_HALF           0x0100
  #define SPI_MODE_SPI_FULL           0x0000                
  #define SDA_PULLUP                  0x0200
  #define GPIO_DS_10MA                0x0000
  #define GPIO_DS_20MA                0x0400
  #define GPIO_DS_30MA                0x0800
  #define GPIO_DS_40MA                0x0C00
  #define TIMER_INT_MODE_4CP          0x8000

  //reg 0x02
  #define TSA_EN							0x0001
  #define TSB_EN							0x0002
  #define TSC_EN							0x0004
  #define TSD_EN							0x0008
  #define OP_EN								0x0010
  #define OP_DIS							0x0000
  #define EXT_SYNC_EN				  0x0020
  #define EXT_SYNC_GPIO0			0x0000
  #define EXT_SYNC_GPIO1			0x0040
  #define OSC32K_EN						0x0080
  #define ECG_EN              0x0100
  #define OSC24M_MODE_HIGH		0x0200
  #define OSC24M_MODE_LOW  		0x0000
  #define CLK_SEL(N) 					((N & 0x0003) << 10)
  #define EXT_CLK_SEL_GPIO0 	0x0000
  #define EXT_CLK_SEL_GPIO1 	0x1000
  #define OSC24M_CAL_EN				0x2000
  #define POWER_MODE          0x4000
  #define TIMER_EN            0x8000

  //reg 0x03
  #define TIMER_SUBPERIOD(N)          N 
  #define TIMESLOT_PERIOD_H_50HZ			0x0000	
  //32K时隙工作周期分频数高八位
  #define TIMESLOT_PERIOD_H(N) 				(N & 0x00FF)
  
  //reg 0x04
  #define TIMESLOT_PERIOD_L_200HZ     0x00A0
  #define TIMESLOT_PERIOD_L_50HZ			0x0280
  //32K时隙工作周期分频数低16位
  #define TIMESLOT_PERIOD_L(N) 				(N & 0xFFFF)	
  
  //reg 0x05
  #define OSC32K_CAL_DEFAULT				  0x0168
  #define OSC32K_CAL(N) 					    ((N & 0x03FF))
  #define OSC24M_AUTO_CAL_DEFAULT     0x8000
  #define OSC24M_AUTO_CAL(N)          ((N & 0x003F) << 10)

  //reg 0x06
  //TSA FIFO 数据深度中断标志位
  #define INT_FIFO_DEPTH_THRD_TSA     0x0001
  #define INT_FIFO_DEPTH_THRD_TSB     0x0002
  #define INT_FIFO_DEPTH_THRD_TSC     0x0004
  #define INT_FIFO_DEPTH_THRD_TSD     0x0008
  //FIFO 向上溢出中断标志位
  #define INT_FIFO_OFLOW_TSA          0x0010
  #define INT_FIFO_OFLOW_TSB          0x0020
  #define INT_FIFO_OFLOW_TSC          0x0040
  #define INT_FIFO_OFLOW_TSD          0x0080
  //FIFO 向下溢出中断标志位
  #define INT_FIFO_UFLOW_TSA          0x0100
  #define INT_FIFO_UFLOW_TSB          0x0200
  #define INT_FIFO_UFLOW_TSC          0x0400
  #define INT_FIFO_UFLOW_TSD          0x0800
  //ECG 中数据过上阈值中断标志位
  #define INT_HLEV_ECG                0x1000
  //ECG 中数据过下阈值中断标志位
  #define INT_LLEV_ECG                0x2000
  //定时器中断标志位
  #define INT_TIMER                   0x4000
  //温度过高中断标志位
  #define INT_OVER_TEMP               0x8000

  //reg 0x07 
  //ECG FIFO 数据深度中断标志
  #define INT_FIFO_DEPTH_THRD_ECG     0x0001
  //ECG FIFO 向上溢出中断标志位
  #define INT_FIFO_OFLOW_ECG          0x0002
  //ECG FIFO 向下溢出中断标志位
  #define INT_FIFO_UFLOW_ECG          0x0004
  //脱落检测中断标志位
  #define INT_ECG_LO                  0x0008
  //时隙A 中TIA 输出饱和中断标志位
  #define INT_TIA_OV_TSA              0x0010
  //时隙B 中TIA 输出饱和中断标志位
  #define INT_TIA_OV_TSB              0x0020
  //时隙C 中TIA 输出饱和中断标志位
  #define INT_TIA_OV_TSC              0x0040
  //时隙D 中TIA 输出饱和中断标志位
  #define INT_TIA_OV_TSD              0x0080

  //reg 0x09
  //TSA FIFO 数据深度中断IO 选择
  #define INT_FIFO_DEPTH_THRD_TSA_IO0       0x0000
  #define INT_FIFO_DEPTH_THRD_TSA_IO1       0x0001
  //TSB FIFO 数据深度中断IO 选择
  #define INT_FIFO_DEPTH_THRD_TSB_IO0       0x0000
  #define INT_FIFO_DEPTH_THRD_TSB_IO1       0x0002
  //TSB FIFO 数据深度中断IO 选择
  #define INT_FIFO_DEPTH_THRD_TSC_IO0       0x0000
  #define INT_FIFO_DEPTH_THRD_TSC_IO1       0x0004
  //TSB FIFO 数据深度中断IO 选择
  #define INT_FIFO_DEPTH_THRD_TSD_IO0       0x0000
  #define INT_FIFO_DEPTH_THRD_TSD_IO1       0x0008
  //TSA FIFO 向上溢出中断IO 选择
  #define INT_FIFO_OFLOW_TSA_IO0            0x0000
  #define INT_FIFO_OFLOW_TSA_IO1            0x0010
  //TSB FIFO 向上溢出中断IO 选择
  #define INT_FIFO_OFLOW_TSB_IO0            0x0000
  #define INT_FIFO_OFLOW_TSB_IO1            0x0020
  //TSC FIFO 向上溢出中断IO 选择
  #define INT_FIFO_OFLOW_TSC_IO0            0x0000
  #define INT_FIFO_OFLOW_TSC_IO1            0x0040
  //TSD FIFO 向上溢出中断IO 选择
  #define INT_FIFO_OFLOW_TSD_IO0            0x0000
  #define INT_FIFO_OFLOW_TSD_IO1            0x0080
  //TSA FIFO 向下溢出中断IO 选择
  #define INT_FIFO_UFLOW_TSA_IO0            0x0000
  #define INT_FIFO_UFLOW_TSA_IO1            0x0100
  //TSB FIFO 向下溢出中断IO 选择
  #define INT_FIFO_UFLOW_TSB_IO0            0x0000
  #define INT_FIFO_UFLOW_TSB_IO1            0x0200
  //TSB FIFO 向下溢出中断IO 选择
  #define INT_FIFO_UFLOW_TSC_IO0            0x0000
  #define INT_FIFO_UFLOW_TSC_IO1            0x0400
  //TSB FIFO 向下溢出中断IO 选择
  #define INT_FIFO_UFLOW_TSD_IO0            0x0000
  #define INT_FIFO_UFLOW_TSD_IO1            0x0800
  //ECG 数据过上阈值中断IO 选择
  #define INT_HLEV_ECG_IO0                  0x0000
  #define INT_HLEV_ECG_IO1                  0x1000 
  //ECG 数据过下阈值中断IO 选择
  #define INT_LLEV_ECG_IO0                  0x0000
  #define INT_LLEV_ECG_IO1                  0x2000 
  //定时器中断IO 选择
  #define INT_TIMER_IO0                     0x0000
  #define INT_TIMER_IO1                     0x4000
  //温度过高中断IO 选择
  #define INT_OVER_TEMP_IO0                 0x0000
  #define INT_OVER_TEMP_IO1                 0x8000

  //reg 0x0A
  //ECG FIFO 数据深度中断IO 选择
  #define INT_FIFO_DEPTH_THRD_ECG_IO0  0x0000 //此中断通过GPIO0 输出
  #define INT_FIFO_DEPTH_THRD_ECG_IO1  0x0001 //此中断通过GPIO1 输出
  //ECG FIFO 向上溢出中断IO 选择 （选项同上）
  #define INT_FIFO_OFLOW_ECG_IO0       0x0000 
  #define INT_FIFO_OFLOW_ECG_IO1       0x0002 
  //ECG FIFO 向下溢出中断IO 选择（选项同上）
  #define INT_FIFO_UFLOW_ECG_IO0       0X0000
  #define INT_FIFO_UFLOW_ECG_IO1       0X0004
  //脱落检测中断标志IO 选择。（选项同上）
  #define INT_ECG_LO_IO0               0x0000
  #define INT_ECG_LO_IO1               0x0008 

  //reg 0x0B
  //GPIO0 模式配置
  #define GPIO0_CFG_DIS						    0x0000
  #define GPIO0_CFG_OUT_PP				    0x0001
  #define GPIO0_CFG_OUT_OD				    0x0002
  #define GPIO0_CFG_INTO					    0x0003
  #define GPIO0_POL_NEG						    0x0000
  #define GPIO0_POL_REVERSAL			    0x0004
  #define GPIO0_OUT_SOURCE_LOW				0x0000
  #define GPIO0_OUT_SOURCE_HIGH				0x0010
  #define GPIO0_OUT_SOURCE_LOW_CLK		0x0020
  #define GPIO0_OUT_SOURCE_HIGH_CLK		0x0030
  #define GPIO0_OUT_SOURCE_INT				0x0040
  #define GPIO1_CFG_DIS						    0x0000
  #define GPIO1_CFG_OUT_PP					  0x0100
  #define GPIO1_CFG_OUT_OD					  0x0200
  #define GPIO1_CFG_INTO					    0x0300
  #define GPIO1_POL_NEG						    0x0000
  #define GPIO1_POL_REVERSAL				  0x0400
  #define GPIO1_OUT_SOURCE_LOW				0x0000
  #define GPIO1_OUT_SOURCE_HIGH				0x1000
  #define GPIO1_OUT_SOURCE_LOW_CLK		0x2000
  #define GPIO1_OUT_SOURCE_HIGH_CLK		0x3000
  #define GPIO1_OUT_SOURCE_INT				0x4000

  //reg 0x0C
  //TIA 输出饱和中断使能控制
  #define INT_TIA_OV_EN               0x0001
  //TIA 输出饱和修改数据模式0 使能控制
  #define TIA_OV_CHGD_M0_EN           0x0002
  //TIA 输出饱和修改数据模式1 使能控制
  #define TIA_OV_CHGD_M1_EN           0x0004
  //ECG 数据采样速率选择
  #define ECG_SP_RATE(N)              ((N & 0x0007) << 3) 
  //数据模式选择
  #define ECG_DATA_MODE(N)            ((N & 0x0003) << 6)   
  //ECG ADC 输入短路并连接VREF
  #define ECG_ADC_INPUT_SHORT         0x0100

  #define RXBG_FBUF_EN                0x0200

  #define RXBG_FLT_SEL                0x0400
  //读FIFO 自动清中断配置
  #define INT_FIFO_AUTOCLR_DISABLE    0x0800
  //芯片温度检测使能控制
  #define INT_TEMP_EN                 0x8000

  //reg 0x0E
  //IN0 和IN1 端口在时隙周期的睡眠期间状态连接
  #define IN01_SLP_CON(N)             (N & 0x0007)
  #define IN23_SLP_CON(N)             ((N & 0x0007) << 4)
  //INTER VC 在睡眠状态时的选择
  #define INTVC_SLP(N)                ((N & 0x0003) << 8)
  //VC0PAD 在睡眠时的开关选择
  #define VC0_SLP_SEL(N)              ((N & 0x0003) << 10)
  //VC1PAD 在睡眠时的开关选择
  #define VC1_SLP_SEL(N)              ((N & 0x0003) << 14)


  /***********************多页寄存器0x10~0x2F*****************/
  //reg 0x0F
  //选定控制寄存器映射使能控制
  #define TSA_REG_SEL						0x0001
  #define TSB_REG_SEL						0x0002
  #define TSC_REG_SEL						0x0004
  #define TSD_REG_SEL						0x0008
  #define ECG_REG_SEL						0x0010  
  
  //reg 0x10
  //通道选择寄存器
  #define CH0_0_EN              0x0001
  #define CH0_1_EN              0x0002
  #define CH0_2_EN              0x0004
  #define CH0_3_EN              0x0008
  //时隙周期预备状态时合并输入选择，需结合IN3_PRE_CON_TSX 进行配置。
  #define IN1_CON_IN3_PRE_ALL_CH0  0x0100
  //时隙周期活动状态时合并输入选择
  #define IN1_CON_IN3_ACT_ALL_CH0   0x0200
	
  #define FIFO_WORD_SIZE_16					0x0000
  #define FIFO_WORD_SIZE_24					0x8000
	
  //reg 0x14
  //LED 驱动器0 电流设置
  #define LED0_CURRENT_TSX(N)    (N & 0x007F)
  //对应LED0~LED5 的输出使能
  #define LED0_OUTX_EN_TSX(N)    ((N & 0x003F)<<8)
 
  //reg 0x15
  //LED 驱动器1 电流设置
  #define LED1_CURRENT_TSX(N)    (N & 0x007F)
  //对应LED6~LED11 的输出使能
  #define LED1_OUTX_EN_TSX(N)    ((N & 0x003F)<<8)
  //LED MASK 作用域选择
  #define LED_MASK_SEL_TSX_16BIT      0x0000
  #define LED_MASK_SEL_TSX_8BIT       0x4000  
  //TIA_ADC 模式LED 脉宽控制
  #define TIA_ADC_MODE_TSX_LED_KEEP   0x8000

  //reg 0x19
  //IN0 端口预备状态连接
  #define IN0_PRE_CON_TSX(N)    (N & 0x0007)
  //IN0 端口活动状态连接
  #define IN0_ACT_CON_TSX(N)    ((N & 0x0007)<<4)
  //IN2 端口预备状态连接
  #define IN2_PRE_CON_TSX(N)    ((N & 0x0007)<<8)
  //IN2 端口活动状态连接
  #define IN2_ACT_CON_TSX(N)    ((N & 0x0007)<<12)

  //reg 0x1A
  //IN1 端口预备状态连接
  #define IN1_PRE_CON_TSX(N)    (N & 0x000F)
  //IN1 端口活动状态连接
  #define IN1_ACT_CON_TSX(N)    ((N & 0x000F)<<8)
  //IN3 端口预备状态连接
  #define IN3_PRE_CON_TSX(N)    ((N & 0x000F)<<8)
  //IN3 端口活动状态连接
  #define IN3_ACT_CON_TSX(N)    ((N & 0x000F)<<12)

  //reg 0x1B
  //VC0PAD 在预备状态时开关选择
  #define VC0_INPUT_PRE_CON_TSX(N)  (N & 0x0007)
  //VC0PAD 在活动状态时开关选
  #define VC0_INPUT_ACT_CON_TSX(N)  ((N & 0x0007)<<4)
  //VC2PAD 在预备状态时开关选择
  #define VC2_INPUT_PRE_CON_TSX(N)  ((N & 0x0007)<<8)
  //VC2PAD 在活动状态时开关选
  #define VC2_INPUT_ACT_CON_TSX(N)  ((N & 0x0007)<<12)

  //reg 0x1C
  //VC1PAD 在预备状态时开关选择
  #define VC1_INPUT_PRE_CON_TSX(N)  (N & 0x0007)
  //VC1PAD 在活动状态时开关选
  #define VC1_INPUT_ACT_CON_TSX(N)  ((N & 0x0007)<<4)
  //VC3PAD 在预备状态时开关选择
  #define VC3_INPUT_PRE_CON_TSX(N)  ((N & 0x0007)<<8)
  //VC3PAD 在活动状态时开关选
  #define VC3_INPUT_ACT_CON_TSX(N)  ((N & 0x0007)<<12)
  

  //reg 0x22
  //ADC 失调校正
  #define ADC_ADJUST_TSX(N)         (N & 0xFFFF)
  
  //reg 0x23
  //每时隙转换次数
  #define NUM_REPEAT_TSX(N)         (N & 0x00FF)
  //每次转换积分次数
  #define NUM_INT_TSX(N)            ((N & 0x00FF) << 8)
  //时隙次级分频系数
  #define SUB_PERIOD_TSX(N)         ((N & 0x00FF) << 12)

  //reg 0x25
  //时隙内VC 电压选择
  #define INTVC_SEL_TSX(N)          (N & 0x0003)
  //调制模式时，调制窗口期间的VC 电压选择
  #define INTVC_SEL_MOD_TSX(N)      ((N & 0x0003) << 2)
  //VC0PAD 在活动状态时开关选择
  #define VC0_ACT_SEL_TSX(N)        ((N & 0x0003) << 4)  
  //调制模式下，VC0PAD 在调制窗口时开关选择
  #define VC0_MOD_SEL_TSX(N)        ((N & 0x0003) << 6)  
  //VC1PAD 在活动状态时开关选择
  #define VC1_ACT_SEL_TSX(N)        ((N & 0x0003) << 8)  
  //调制模式下，VC1PAD 在调制窗口时开关选择
  #define VC1_MOD_SEL_TSX(N)        ((N & 0x0003) << 10)  

  #define VC0_SEL_AVDD						0x0000
  #define VC0_SEL_TIA_VREF					0x0010
  #define VC0_SEL_TIA_VREF_250				0x0020
  #define VC0_SEL_GND						0x0030
  
  #define VC0_SEL_MOD_AVDD					0x0000
  #define VC0_SEL_MOD_TIA_VREF				0x0040
  #define VC0_SEL_MOD_TIA_VREF_250			0x0800
  #define VC0_SEL_MOD_GND					0x00C0
	
  #define VC1_SEL_AVDD						0x0000
  #define VC1_SEL_TIA_VREF					0x0100
  #define VC1_SEL_TIA_VREF_250				0x0200
  #define VC1_SEL_GND						0x0300
  
  #define VC1_SEL_MOD_AVDD					0x0000
  #define VC1_SEL_MOD_TIA_VREF				0x0400
  #define VC1_SEL_MOD_TIA_VREF_250			0x0800
  #define VC1_SEL_MOD_GND					0x0C00
	
  //reg 0x27
  //TSX 时隙周期内TIA 使能寄存器
  #define TIA_EN_TSX                0x0001
  //TIA_VREF 电压选择
  #define TIA_VREF_TSX(N)           ((N & 0x0003) << 1)
  //跨阻放大增益选择
  #define TIA_GAIN_TSX(N)           ((N & 0x0007) << 4)
  //跨阻放大器反馈电容选择
  #define TIA_CAP_TSX(N)            ((N & 0x00FF) << 8)

  #define TIA_EN							0x0001
  #define TIA_VREF_0P64V					0x0000
  #define TIA_VREF_0P85V					0x0002
  #define TIA_VREF_1P06V					0x0004
  #define TIA_VREF_1P26V					0x0006
  #define TIA_12K5_CAP_8P86					0x6600
  #define TIA_25K_CAP_6P32					0x3810
  #define TIA_50K_CAP_4P74					0x4020
  #define TIA_100K_CAP_3P16					0x2030
  #define TIA_200K_CAP_1P58					0x1040
  #define TIA_400K_CAP_960F					0x0650
  #define TIA_800K_CAP_480F					0x0360
  #define TIA_1M6_CAP_320F					0x0270
	
  //reg 0x2D
  //FIFO 起始地址 ECG和TSX共用1KFIFO
  #define FIFO_STA_ADR(N)         (N & 0x03FF)

  //reg 0x2E
  //FIFO 大小，共用1KFIFO
  #define FIFO_SIZE(N)            (N & 0x03FF)

  //reg 0x2F
  //FIFO 数据深度中断阈值设置为FIFO+1，以字节为单位
  #define FIFO_DEPTH_THRD(N)      (N & 0x03FF)
  //FIFO 数据深度中断使能控制
  #define INT_FIFO_DEPTH_THRD_EN   0x1000
  //FIFO 向上溢出中断使能控制
  #define INT_FIFO_OFLOW_EN        0x2000
  //FIFO 向下溢出中断使能控制
  #define INT_FIFO_UFLOW_EN        0x4000


  /***********************************************************/
  //reg 0x37
  //ECG 交流脱落检测电流辐值
  #define ECG_ACLO_MAG(N)          (N & 0x000F)
  //ECG 直流脱落检测电流辐值
  #define ECG_DCLO_MAG(N)          ((N & 0x000F) << 4)
  //ECG 直流脱落检测负输入电极电流方向配置
  #define ECG_DCLO_POLARITY_IN_R   0x0100
  //ECG 直流脱落检测正输入电极电流方向配置
  #define ECG_DCLO_POLARITY_IP_R   0x0200
  //ECG AC 耦合或两电极模式使能控制
  #define ECG_ACCOUPLE_EN          0x1000
  //ECG 三电极AC 耦合或两电极模式下，输入管脚连接到VREF 的电阻选择
  #define ECG_ACRES_SEL_200M       0x0000
  #define ECG_ACRES_SEL_2_5G       0x2000
  //AC 脱落检测使能控制
  #define ECG_ACLO_EN              0x4000
  //DC 脱落检测使能控制
  #define ECG_DCLO_EN              0x8000

  //reg 0x38
  //ECG 交流脱落检测比较阈值设定
  #define ECG_ACLO_AMP_THRD(N)     (N & 0x7FFF)

  //reg 0x39
  //ECG 交流脱落检测中断使能
  #define INT_ECG_ACLO_EN         0x0001
  //ECG 直流脱落检测中断使能
  #define INT_ECG_DCLO_EN         0x0002
  //ECG 负输入电极直流脱落检测高阈值使能
  #define INT_ECGIN_DCLO_HI_EN    0x0004
  //ECG 正输入电极直流脱落检测高阈值使能
  #define INT_ECGIP_DCLO_HI_EN    0x0008
  //ECG 右腿驱动直流脱落检测低阈值中断使能配置
  #define INT_ECG_RLD_DCLO_LO_EN  0x0010
  //ECG 右腿驱动直流脱落检测高阈值中断使能配置
  #define INT_ECG_RLD_DCLO_HI_EN  0x0020
  //ECG ADC 满量程检测中断
  #define INT_ECG_ADC_OV_EN       0x0040
  //ECG 右腿驱动配置
  #define ECG_RLD_CONFIG(N)       ((N & 0x001F)<<8)
  //右腿驱动使能控制
  #define ECG_RLD_EN              0x2000
  //ECG 输入管脚对地或差分电容配置
  #define ECG_INPUTRC(N)          ((N & 0x0003)<<14)

  //reg 0x3A
  //ECG 交流脱落检测中断
  #define INT_ECG_ACLO            0x0001
  //ECG 直流脱落检测中断
  #define INT_ECG_DCLO            0x0002
  //ECG 负输入电极直流脱落检测高阈值中断
  #define INT_ECGIN_DCLO_HI       0x0004
  //ECG 正输入电极直流脱落检测高阈值中断
  #define INT_ECGIP_DCLO_HI       0x0008
  //ECG 右腿驱动直流脱落检测低阈值中断
  #define INT_ECG_RLD_DCLO_LO     0x0010
  //ECG 右腿驱动直流脱落检测高阈值中断
  #define INT_ECG_RLD_DCLO_HO     0x0020 
  //ECG ADC 满量程检测中断
  #define INT_ECG_ADC_OV          0x0040
  //ECG 交流脱落检测标志位
  #define STA_ECG_ACLO            0x0100
  //ECG 直流脱落检测标志位
  #define STA_ECG_DCLO            0x0200 
  //ECG 负输入电极直流脱落检测高阈值标志位
  #define STA_ECGIN_DCLO_HI       0x0400
  //ECG 正输入电极直流脱落检测高阈值标志位
  #define STA_ECGIP_DCLO_HI       0x0800
  //ECG 右腿驱动直流脱落检测低阈值标志位
  #define STA_ECG_RLD_DCLO_LO     0x1000
  //ECG 右腿驱动直流脱落检测高阈值标志位
  #define STA_ECG_RLD_DCLO_HI     0x2000  
  //ECG ADC 满量程检测标志位
  #define STA_ECG_ADC_OV          0x4000

  //reg 0x3B
  #define ECG_ACLO_AMP_DET(N)     (N&0xFFFF)
  
  //reg 0x3E
  #define LPFCH0_CFG(N)           (N & 0x001F)
  #define LPFCH1_CFG(N)           ((N & 0x001F) << 8)

  //reg 0x3F
  #define LPFCH2_CFG(N)           (N & 0x001F)
  #define LPFCH3_CFG(N)           ((N & 0x001F) << 8)
  
  //reg 0x50
  //ECG FIFO 数据深度指示，以字节为单位
  #define FIFO_BYTE_COUNT_ECG(N)  (N & 0x03FF)
  #define FIFO_BYTE_COUNT_ECG_REG  0x50

  //reg 0x51
  //TSA FIFO 数据深度指示，以字节为单位
  #define FIFO_BYTE_COUNT_TSA(N)  (N & 0x03FF)
  #define FIFO_BYTE_COUNT_TSA_REG  0x51

  //reg 0x52
  //TSB FIFO 数据深度指示，以字节为单位
  #define FIFO_BYTE_COUNT_TSB(N)  (N & 0x03FF)
  #define FIFO_BYTE_COUNT_TSB_REG  0x52

  //reg 0x53
  //TSC FIFO 数据深度指示，以字节为单位
  #define FIFO_BYTE_COUNT_TSC(N)  (N & 0x03FF)
  #define FIFO_BYTE_COUNT_TSC_REG  0x53

  //reg 0x54
  //TSD FIFO 数据深度指示，以字节为单位
  #define FIFO_BYTE_COUNT_TSD(N)  (N & 0x03FF)
  #define FIFO_BYTE_COUNT_TSD_REG  0x54

  //reg 0x60
  //FIFO 数据，采用小端模式输出。连续读不会自加寄存器地址，而会依先进先出次序读出转换数据。
  #define FIFO_DATA_ECG_REG  0x60
 
  //reg 0x61
  #define FIFO_DATA_TSA_REG  0x61
  
  //reg 0x62
  #define FIFO_DATA_TSB_REG  0x62
  
  //reg 0x63
  #define FIFO_DATA_TSC_REG  0x63
  
  //reg 0x64
  #define FIFO_DATA_TSD_REG  0x64 


#endif
