#include <STC15F2K60S2.H>
#define __REG52_H__
#include "iic.h"

#define YY P2=(P2&0x1f)
#define Y4 YY|0x80
#define Y5 YY|0xa0
#define Y6 YY|0xc0
#define Y7 YY|0xe0

typedef unsigned char u8;
typedef	unsigned int  u16;

#define SYS	P0=0xff;Y4;YY;P0=0x00;Y5;YY;

u8 Time_10ms,Time_20ms,Time_1000ms,LED_SS,ADC_flag,tempture,tempture_flag;
u16 Time;
u8 LED_LZ[4]={120,120,120,120};
code u8	LED_duan[]={0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,0xff,0xbf};
u8 LED_buff[]={10,10,10,10,10,10,10,10};
//-----------------------定时器初始化----------------------------
void Timer0_Init(void)		//1毫秒@12.000MHz
{
	AUXR |= 0x80;			//定时器时钟1T模式
	TMOD &= 0xF0;			//设置定时器模式
	TL0 = 0x20;				//设置定时初始值
	TH0 = 0xD1;				//设置定时初始值
	TF0 = 0;				//清除TF0标志
	TR0 = 1;				//定时器0开始计时
	EA=1;
	ET0=1;
}

void Timer1_Init(void)		//1微秒@12.000MHz
{
	AUXR |= 0x40;			//定时器时钟1T模式
	TMOD &= 0x0F;			//设置定时器模式
	TL1 = 0xF4;				//设置定时初始值
	TH1 = 0xFF;				//设置定时初始值
	TF1 = 0;				//清除TF1标志
	TR1 = 1;				//定时器1开始计时
	ET1=1;
	EA=1;
}


//-----------------------数码管显示-----------------------------
void LED_display()
{
	static u8 i;
	P0=0xff;Y7;YY;
	P0=0x01<<i;Y6;YY;
	P0=LED_duan[LED_buff[i]];Y7;YY;
	if(++i==8)i=0;
}

void LED_display_buff(u8 d1,d2,d3,d4,d5,d6,d7,d8)
{
	LED_buff[0]=d1;
	LED_buff[1]=d2;
	LED_buff[2]=d3;
	LED_buff[3]=d4;
	LED_buff[4]=d5;
	LED_buff[5]=d6;
	LED_buff[6]=d7;
	LED_buff[7]=d8;
}
//-----------------------------EEPROM初始化--------------------------------
void EEPROM_R(u8 addr,u8* dat,u8 len) 
{
	IIC_Start();
	IIC_SendByte(0xa0);
	IIC_WaitAck();
	IIC_SendByte(addr);
	IIC_WaitAck();

	IIC_Start();
	IIC_SendByte(0xa1);
	IIC_WaitAck();
	while(len--)
	{
		*dat++=IIC_RecByte();
		if(len)IIC_SendAck(0);
		else	 IIC_SendAck(1);
	}
	IIC_Stop();
}

void EEPROM_W(u8 addr,u8* dat,u8 len)
{
	IIC_Start();
	IIC_SendByte(0xa0);
	IIC_WaitAck();
	IIC_SendByte(addr);
	IIC_WaitAck();
	while(len--)
	{
		IIC_SendByte(*dat++);
		IIC_WaitAck();
		IIC_Delay(200); 
	}
	IIC_Stop();
}
//----------------------------------ADC--------------------------------------
u8 ADC;
void ADC_R(u8 channle)
{
	IIC_Start();
	IIC_SendByte(0x90); 
	IIC_WaitAck();
	
	IIC_SendByte(channle); 
	IIC_WaitAck();
	
	IIC_Start();
	
	IIC_SendByte(0x91); 
	IIC_WaitAck();
	
	ADC = IIC_RecByte();
  IIC_SendAck(1);
	
  IIC_Stop();
}

void DAC_W(u8 addr)
{
	IIC_Start();
	IIC_SendByte(0x90);
	IIC_WaitAck();
	
	IIC_SendByte(0x40);
	IIC_WaitAck();
	
	IIC_SendByte(addr);
	IIC_WaitAck();
	
	IIC_Stop();
}
//---------------------------------独立按键---------------------------------
u8 key_value=0;
u8 key_state=0;
u8 LZJG_yx_flag;
void key_scan()
{
	P3|=0x0f;
	if(~P3&0x0f)
	{
		if(key_state<255)key_state++;
		if(P33==0)key_value=4;
		if(P32==0)key_value=5;
		if(P31==0)key_value=6;	
		if(P30==0)key_value=7;
	}
	else
	{
		key_value=0;
		key_state=0;
	}
}


//------------------------------LED-----------------------------------
bit LED_TZ=0;
u8 i,LED_KZ,LEDXS=0xff;
u16 LED_JS;
void LED(){P0=LEDXS;Y4;YY;}

void LED_QH()
{
		switch(LED_KZ)
		{
			case 0:
			if(LED_JS>=LED_LZ[0])
			{
				LEDXS=~(0x01<<i);
				if(++i==8){i=0;LED_KZ=1;}
				LED_JS=0;
			}
			break;		
			case 1:
				if(LED_JS>=LED_LZ[1])
			{
				LEDXS=~(0x80>>i);
				if(++i==8){i=0;LED_KZ=2;}
				LED_JS=0;
			}
			break;	
			case 2:
				if(LED_JS>=LED_LZ[2])
			{
				if(i == 0)LEDXS = ~0X81;
				if(i == 1)LEDXS = ~0X42;
				if(i == 2)LEDXS = ~0X24;
				if(i == 3)LEDXS = ~0X18;   
				if(++i==4){i=0;LED_KZ=3;}
				LED_JS=0;	
			}
			break;	
			case 3:
				if(LED_JS>=LED_LZ[3])
			{
				if(i == 3)LEDXS = ~0X81;
				if(i == 2)LEDXS = ~0X42;
				if(i == 1)LEDXS = ~0X24;
				if(i == 0)LEDXS = ~0X18;   
				if(++i==4){i=0;LED_KZ=0;}
				LED_JS=0;	
			}
			break;
		}
}
//-----------------------------LED流转间隔-----------------------------
u8 LZJG_flag,LZJG_LZ_flag,LZJG_LZ;
u8 LZJG_yx=1;
void LED_LZJG()
{
	if(tempture==0)
	{
		switch(LZJG_flag)
		{
			case 0:
				LZJG_LZ_flag=1;
			tempture_flag=0;
				LED_display_buff(11,LZJG_yx,11,10,LED_LZ[LZJG_yx-1]/100,LED_LZ[LZJG_yx-1]%100/10,LED_LZ[LZJG_yx-1]%10,0);
		//			LED_display_buff(10,10,10,10,10,10,10,10);
			break;
			case 1:
				LZJG_yx_flag=1;
				LZJG_LZ_flag=1;
				tempture_flag=1;
				if(LED_SS==0)
				{
					LED_display_buff(11,LZJG_yx,11,10,LED_LZ[LZJG_yx-1]/100,LED_LZ[LZJG_yx-1]%100/10,LED_LZ[LZJG_yx-1]%10,0);
				}
				else
				{
					LED_display_buff(10,10,10,10,LED_LZ[LZJG_yx-1]/100,LED_LZ[LZJG_yx-1]%100/10,LED_LZ[LZJG_yx-1]%10,0);
				}
			break;
			case 2:
				LZJG_yx_flag=0;
				LZJG_LZ_flag=0;
				if(LED_SS==0)
				{
					LED_display_buff(11,LZJG_yx,11,10,LED_LZ[LZJG_yx-1]/100,LED_LZ[LZJG_yx-1]%100/10,LED_LZ[LZJG_yx-1]%10,0);
				}
				else
				{
					LED_display_buff(11,LZJG_yx,11,10,10,10,10,10);
				}
			break;
		}
	}
	else
	{
		if(tempture==3&&tempture_flag==0)
		{
			if(ADC<63){LED_display_buff(10,10,10,10,10,10,11,1);}
			if(63<ADC&&ADC<=127){LED_display_buff(10,10,10,10,10,10,11,2);}
			if(127<ADC&&ADC<=191){LED_display_buff(10,10,10,10,10,10,11,3);}
			if(191<ADC&&ADC<=255){LED_display_buff(10,10,10,10,10,10,11,4);}
		}
	}
}
//-------------------------------------------------按键处理----------------------------------------------
void key_dispose()
{
	if(Time_10ms>=10)
		{
			Time_10ms=0;
			key_scan();
			//流水灯暂停
			if(key_value==7&&key_state==2)
			{
				LED_TZ=~LED_TZ;
				if(!LED_TZ)LEDXS=0xff;
			}
			//按键处理
			if(key_value==6&&key_state==2)
			{
				LZJG_flag++;
				if(LZJG_flag>=3)LZJG_flag=0;
			}
			//运行加减处理
			if(key_value==5&&key_state==2&&LZJG_yx_flag==1)
			{
				LZJG_yx++;
				if(LZJG_yx>4)LZJG_yx=1;
			}else if(key_value==4&&key_state==2&&LZJG_yx_flag==1)
			{
				LZJG_yx--;
				if(LZJG_yx<=0)LZJG_yx=4;
			}
			//流转间隔加减处理
			if(key_value==5&&key_state==2&&LZJG_LZ_flag==0)
			{
				if(LZJG_yx==1){if(LED_LZ[0]>=120){LED_LZ[0]=40;}else LED_LZ[0]+=10;}
				if(LZJG_yx==2){if(LED_LZ[1]>=120){LED_LZ[1]=40;}else LED_LZ[1]+=10;}
				if(LZJG_yx==3){if(LED_LZ[2]>=120){LED_LZ[2]=40;}else LED_LZ[2]+=10;}
				if(LZJG_yx==4){if(LED_LZ[3]>=120){LED_LZ[3]=40;}else LED_LZ[3]+=10;}
				EEPROM_W(0x00,LED_LZ,4);  
			}else if(key_value==4&&key_state==2&&LZJG_LZ_flag==0)
			{
				if(LZJG_yx==1){if(LED_LZ[0]<=40){LED_LZ[0]=120;}else LED_LZ[0]-=10;}
				if(LZJG_yx==2){if(LED_LZ[1]<=40){LED_LZ[1]=120;}else LED_LZ[1]-=10;}
				if(LZJG_yx==3){if(LED_LZ[2]<=40){LED_LZ[2]=120;}else LED_LZ[2]-=10;}
				if(LZJG_yx==4){if(LED_LZ[3]<=40){LED_LZ[3]=120;}else LED_LZ[3]-=10;}
				EEPROM_W(0x00,LED_LZ,4);
			}
		}
}

u8 key_ON()
{
		P3|=0x0f;
	if(~P3&0x0f)
	{
		if(P33==0){return 3;}
	}
	return 0;
}
//------------------------------主函数------------------------------
void main()
{
	SYS;
	Timer0_Init();
	Timer1_Init();
	EEPROM_R(0x00,LED_LZ,4);
	while(1)
	{
		tempture=key_ON();
		if(ADC_flag==0){ADC_R(0x03);ADC_flag=1;EA=1;}
		LED_QH();
		LED_LZJG();
		key_dispose();
//		ADC_R(0x03);
//		LED_display_buff(ADC/100,ADC%100/10,ADC%10,10,10,10,10,10);

	}
}
//-----------------------------定时器中断----------------------------
u8 j,k;
void Time0_interrupt() interrupt 1
{
	Time_10ms++;
	if(++Time==800){LED_SS=!LED_SS;Time=0;}
	if(LED_TZ==1&&Time_1000ms%10==0)LED_JS++;
	if(Time_1000ms++==1000){Time_1000ms=0;}
	if(++k==10){ADC_flag=0;k=0;EA=0;}
	LED_display();
}

void Time1_interrupt() interrupt 3
{
	j=++j%255;
	if(ADC>=j)
	{
		LED();
	}
	else 
	{ 
		P0 = 0XFF;Y4;YY;
	}
}