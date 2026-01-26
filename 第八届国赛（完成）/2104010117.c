/*
LED与EEPROM未完成
*/

#include <STC15F2K60S2.H>
#include "intrins.h"
#include "iic.h"

#define YY P2=(P2&0x1f)
#define Y4 YY|0x80
#define Y5 YY|0xa0
#define Y6 YY|0xc0
#define Y7 YY|0xe0

typedef unsigned char u8;
typedef unsigned int  u16;

#define SYS P0=0x00;Y5;YY;P0=0xff;Y4;YY;

u8 	Time10ms,LED_V=0xff;
char CS=0;
u16 Time1S,distance,Time100ms,old_distance;
u16  old_distance2[4];
//																																		10		11c	 12f	
code u8 LED_duan[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xff,0xc6,0x8e};
u8 LED_buff[]={10,10,10,10,10,10,10,10};

void LED(){P0=LED_V;Y4;YY;}

void Timer1_Init(void)		//1毫秒@12.000MHz
{
	AUXR &= 0xBF;			//定时器时钟12T模式
	TMOD &= 0x0F;			//设置定时器模式

	TL1 = 0x18;				//设置定时初始值
	TH1 = 0xFC;				//设置定时初始值

	TF1 = 0;				//清除TF1标志
	TR1 = 1;				//定时器1开始计时
	ET1 = 1;				//使能定时器1中断
}


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

void DAC(u8 dat)
{
	float date=0;
	date=dat/5.0*255;
	if(date>255)date=255;
	I2CStart();
	I2CSendByte(0x90);
	I2CWaitAck();
	I2CSendByte(0x40);
	I2CWaitAck();
	I2CSendByte(date);
	I2CWaitAck();
	I2CStop();
}

u8 EEPROM;
void EEPROM_R(u8 addr)
{
	I2CStart();
	I2CSendByte(0xa0);
	I2CWaitAck();
	I2CSendByte(addr);
	I2CWaitAck();

	I2CStart();
	I2CSendByte(0xa1);
	I2CWaitAck();
	EEPROM=I2CReceiveByte();
	I2CSendAck(1);
	I2CStop();
}

void EEPROM_W(u8 addr,dat)
{
	I2CStart();
	I2CSendByte(0xa0);
	I2CWaitAck();
	I2CSendByte(addr);
	I2CWaitAck();
	I2CSendByte(dat);
	I2CWaitAck();
	I2CStop();
}

void Delay13us()		//@12.000MHz
{
	unsigned char i;

	_nop_();
	_nop_();
	i = 36;
	while (--i);
}



u16 PACSonic()
{
	int distance=-1;
	u16 Time=0;
	CMOD&=0x00;
	CH=0x3C;
	CL=0xB0;
	CF=0;

	P10=1;Delay13us();P10=0;Delay13us();
	P10=1;Delay13us();P10=0;Delay13us();
	P10=1;Delay13us();P10=0;Delay13us();
	P10=1;Delay13us();P10=0;Delay13us();
	P10=1;Delay13us();P10=0;Delay13us();
	P10=1;Delay13us();P10=0;Delay13us();
	P10=1;Delay13us();P10=0;Delay13us();
	P10=1;Delay13us();P10=0;Delay13us();
	
	CR=1;
	while((P11==1)&&(CF==0));
	CR=0;
	
	if(CF==0)
	{
		Time=CL|(CH<<8)-15535;
		distance=(Time*0.17F);
		distance=distance%10000/10;
	}
	return distance;
	
}

u8 key_value,key_state,distance_flag,key_flag;
void key_scan()
{
	P3|=0x0f;
	if(~P3&0x0f)
	{
		if(key_state<255)key_state++;
		if(P30==0)key_value=7;
		if(P31==0)key_value=6;
		if(P32==0)key_value=5;
		if(P33==0)key_value=4;
	}
	else
	{
		key_value=0;
		key_state=0;
	}
}

u8 FY_flag,CS_flag;
void key_disposs()
{
	Time10ms=0;
	key_scan();
	if(key_value==4&&key_state==5)
	{
		distance_flag=1;
		key_flag=0;;
	}
	if(key_value==5&&key_state==5)
	{
		key_flag=!key_flag;
	}
	if(key_value==6&&key_state==5)
	{
		if(++CS_flag==3)CS_flag=1;
		if(CS_flag==1){key_flag=2;}
		if(CS_flag==2){key_flag=0;}
		
	}
	
	
	if(key_value==7&&key_state==5&&key_flag==1)
	{
		if(++FY_flag==4)FY_flag=0;
	}
	if(key_value==7&&key_state==5&&key_flag==2)
	{
		CS+=10;
		if(CS>30){CS=0;}
		if(CS<0){CS=30;}
	}
	
}

void JM_QH()
{
	switch(key_flag)
	{
		case 0:
			LED_displaybuff(11,10,distance/100,distance/10%10,distance%10,old_distance/100,old_distance/10%10,old_distance%10);
		break;
		case 1:
			switch(FY_flag)
			{
				case 0:
					LED_displaybuff(2,10,10,10,10,old_distance2[0]/100,old_distance2[0]/10%10,old_distance2[0]%10);
				break;
				case 1:
					LED_displaybuff(2,10,10,10,10,old_distance2[1]/100,old_distance2[1]/10%10,old_distance2[1]%10);
				break;
				case 2:
					LED_displaybuff(2,10,10,10,10,old_distance2[2]/100,old_distance2[2]/10%10,old_distance2[2]%10);
				break;
				case 3:
					LED_displaybuff(2,10,10,10,10,old_distance2[3]/100,old_distance2[3]/10%10,old_distance2[3]%10);
				break;
			}
				break;
		case 2:
			LED_displaybuff(12,10,10,10,10,10,CS/10,CS%10);
		break;
	}
}
u8 CL_falg,i=3,LED_flag;
void LED_XS()
{
	
	if(key_flag==2)LED_V&=0xbf;else LED_V|=~0xbf;
	if(key_flag==1)LED_V&=0x7f;else LED_V|=~0x7f;
	
	
}


u8 i,LED_flag;
void main()
{
	SYS;
	EA=1;
	Timer1_Init();
	while(1)
	{
			if(distance_flag==1)
			{
				Time100ms=0;
				old_distance=distance;
				old_distance2[i]=distance;
				if(i++==3)i=0;;
				distance=PACSonic();
				distance_flag=0;
				CL_falg=1;
			}
			if(distance<=CS){DAC(0);}
			if(distance>CS){DAC((distance-CS)*0.02);}
			LED_XS();
			JM_QH();
	}
	
	
	
}

u8 l;
void Timer1_Isr() interrupt 3
{
	if(++l==5){l=0;LED();}
	if(++Time10ms==10)key_disposs();
	++Time100ms;
	if(++Time1S==1000){Time100ms=0;LED_flag=!LED_flag;}
	LED_display();
	
}


