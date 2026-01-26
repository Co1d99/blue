#include <STC15F2K60S2.H>
#define __REG52_H__
#include "iic.h"
#define YY P2=(P2&0x1f)
#define Y4 YY|0x80
#define Y5 YY|0xa0
#define Y6 YY|0xc0
#define Y7 YY|0xe0

typedef unsigned char u8;
typedef unsigned int  u16;

#define SYS P0=0x00;Y5;YY;P0=0xff;Y4;YY;

u8 LED_flag;
u16 Time10ms,Time5s;

//                                                                     11   12-u p    n
code u8 LED_duan[]={0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,0xff,0xc1,0x8c,0xc8,
										0xC0&0x7f,0xF9&0x7f,0xA4&0x7f,0xB0&0x7f,0x99&0x7f,0x92&0x7f,0x82&0x7f,0xF8&0x7f,0x80&0x7f,0x90&0x7f};
u8 LED_buff[]={10,10,10,10,10,10,10,10};

void Timer1_Init(void)
{
	AUXR |= 0x40;
	TMOD &= 0x0F;
	TL1 = 0x20;		
	TH1 = 0xD1;		
	TF1 = 0;			
	TR1 = 1;
	EA=1;
	ET1=1;
}
//-----------------------------数码管----------------------------------
void LED_display()
{
	static u8 i=0;
	P0=0xff;Y7;YY;
	P0=0x01<<i;Y6;YY;
	P0=LED_duan[LED_buff[i]];Y7;YY;
	if(++i==8)i=0;
}

void LED_displaybuff(u8 d1,d2,d3,d4,d5,d6,d7,d8)
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
u8 LED_V=0xff;
void LED()
{
		P0=LED_V;Y4;YY;
}



//------------------------------ADC转换----------------------------------
u8 ADC,Time_ADC,ADC_flag;
u16 temp_ADC;
void ADC_R(u8 channl)
{
	IIC_Start();
	IIC_SendByte(0x90);
	IIC_WaitAck();
		
	IIC_SendByte(channl);
	IIC_WaitAck();
		
	IIC_Start();
	IIC_SendByte(0x91);
	IIC_WaitAck();

	ADC=IIC_RecByte();
	IIC_SendAck(1);
	IIC_Stop();
	
}

//-------------------------------AT24C02初始化-------------------------

u8 EEPORM_R(u8 addr)
{
	u8 dat;
	IIC_Start();
	IIC_SendByte(0xa0);
	IIC_WaitAck();
	IIC_SendByte(addr);
	IIC_WaitAck();
		
	IIC_Start();
	IIC_Send  ;
	
}
//---------------------------------------矩阵按键----------------------------------------
u8 key_values[4][4]=
{
	7,11,15,19,
	6,10,14,18,
	5,9,13,17,
	4,8,12,16	
};
u8 key_state,key_value,x,y;
void key_scan()
{
	P3=0x0f;
	P4=0x00;
	if(~P3&0x0f)
	{
		if(key_state<255)key_state++;
		
		if(P30==0)x=0;
		if(P31==0)x=1;
		if(P32==0)x=2;
		if(P33==0)x=3;
		
		P3=0xf0;
		P4=0xff;
		if(P44==0)y=0;
		if(P42==0)y=1;
		if(P35==0)y=2;
		if(P34==0)y=3;
		
		key_value=key_values[x][y];
	}
	else
	{
		key_state=0;
		key_value=0;
	}
}
//---------------------------------------按键处理---------------------------------------
u8  key_flag=0,ADC_CS_flag,key_num,key_17,ADC_JS_flag,num;
u16 ADC_CS;
void key_dispose()
{
	if(Time10ms==10)
		{
			key_scan();
			Time10ms=0;
			if(key_value==12&&key_state==2)
			{
				if(++key_flag>2){key_flag=0;}
			
			}
			else if(key_value==16&&key_state==2&&ADC_CS_flag==1)
			{
				if(ADC_CS>=500){ADC_CS=0;}else{ADC_CS+=50;}
				EEPORM_W(0x00,ADC_CS/10);
			}
			else if(key_value==17&&key_state==2&&ADC_CS_flag==1)
			{
				if(ADC_CS<=0){ADC_CS=500;}else{ADC_CS-=50;}
				EEPORM_W(0x00,ADC_CS/10);
			}else if(key_value==13&&key_state==2&&ADC_JS_flag==1)
			{
				num=0;
			}
		}
}

//---------------------------------------计数器---------------------------------------
u8 t;
void ADC_num()
{
	switch(t)
	{
		case 0:
		if(temp_ADC>ADC_CS)
		{
			t=1;
		}
		
		break;
		case 1:
	if(temp_ADC<ADC_CS)
		{
			t=0;
			num++;
		}
		break;
	}	
}
//--------------------------------------LED显示---------------------------------------
void LED_XS()
{
	if(temp_ADC<ADC_CS)
	{
		++Time5s;
		if(Time5s>5000)
		{
			LED_flag=1;
		}
	}
	else
	{
		LED_flag=0;Time5s=0;
	}
	
			
}
//-----------------------------------界面切换----------------------------------
void JM_QH()
{
		switch(key_flag)
		{
			case 0:
				ADC_JS_flag=0;
				ADC_CS_flag=0;
				LED_displaybuff(11,10,10,10,10,(temp_ADC/100)+14,temp_ADC%100/10,temp_ADC%10);
			break;
			case 1:
				ADC_JS_flag=0;
				ADC_CS_flag=1;
				LED_displaybuff(12,10,10,10,10,(ADC_CS/100)+14,ADC_CS%100/10,ADC_CS%10);
			break;
			case 2:
				ADC_CS_flag=0;
				ADC_JS_flag=1;
				LED_displaybuff(13,10,10,10,10,10,num/10,num%10);
			break;
		}
}



//-------------------------------------主函数------------------------------------------

void main()
{
	SYS;
	Timer1_Init();
	ADC_CS=EEPORM_R(0x00)*10;
	while(1)
	{
		
		if(ADC_flag==1){ADC_R(0x03);temp_ADC=ADC*(500.0/255);EA=1;ADC_flag=0;}
		key_dispose();
		JM_QH();
		ADC_num();
		if(LED_flag==1){LED_V&=0xfe;}else{LED_V|=0xff;}
    if((num%2)!=0){LED_V&=0xfd;}else{LED_V|=0x02;}
	}
}

//------------------------------------定时器1中断--------------------------------------
void Time1_interrupt() interrupt 3
{
	Time10ms++;
	if(Time10ms%5==0){LED();}
	LED_XS();
	if(++Time_ADC==10){ADC_flag=1;EA=0;}
	LED_display();
}