#include <STC15F2K60S2.H>
#define __REG52_H__
#include "ds1302.h"
#include "onewire.h"

#define YY 	P2=(P2&0x1f)
#define Y4	YY|0x80
#define Y5	YY|0xa0
#define Y6	YY|0xc0
#define Y7	YY|0xe0

typedef unsigned char u8;
typedef	unsigned int 	u16;

#define SYS	P0=0xff;Y4;YY;P0=0x00;Y5;YY;

u8	JM_flag=0,SJ_flag=0;
u16 JM_1s,temperature_ms;
u8 Time1ms,Time10ms;
code u16 LED_duan[]={0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,0xff,0xbf,0xc6,
										0xC0&0x7f,0xF9&0x7f,0xA4&0x7f,0xB0&0x7f,0x99&0x7f,0x92&0x7f,0x82&0x7f,
										0xF8&0x7f,0x80&0x7f,0x90&0x7f,0xff&0x7f,0xbf&0x7f,0xc6&0x7f};
u8 LED_buff[]={10,10,10,10,10,10,10,10};
//-----------------------------------------定时器----------------------------------------
void Timer0_Init(void)
{
	AUXR |= 0x80;
	TMOD &= 0xF0;
	TL0 = 0x20;
	TH0 = 0xD1;
	TF0 = 0;
	TR0 = 1;
	ET0=1;
	EA=1;
}

//------------------------------------------数码管----------------------------------------
void LED_display()
{
	static u8 i;
	Y7;P0=0xff;
	Y6;P0=0x01<<i;
	Y7;P0=LED_duan[LED_buff[i]];
	if(++i==8){i=0;};
}

void LED_XSBUFF(u8 d1,d2,d3,d4,d5,d6,d7,d8)
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

//-----------------------------------------实时时钟------------------------------------------
code u8 RTC_R[7]={0x81,0x83,0x85,0x87,0x89,0x8b,0x8d};
code u8 RTC_W[8]={0x80,0x82,0x84,0x86,0x88,0x8a,0x8c,0x8e};
int RTC_HC[7]={50,59,23,05,11,05,22};
u8	Clock_HC[8]={0,0,0,0,0,0,0,0};
void RTC_read()
{
	u8 i,temp;
	for(i=0;i<7;i++)
	{
		temp=Read_Ds1302_Byte(RTC_R[i]);
		RTC_HC[i]=temp/16*10+temp%16;
	}
}

void RTC_write()
{
	u8 i;
	Write_Ds1302_Byte(RTC_W[8],0x00);
	for(i=0;i<7;i++)
	{
		Write_Ds1302_Byte(RTC_W[i],RTC_HC[i]/10*16+RTC_HC[i]%10);
	}
	Write_Ds1302_Byte(RTC_W[8],0x80);
}

void RTC_byte_write(u8 addr,HC)
{
	Write_Ds1302_Byte(RTC_W[7],0x00);
	Write_Ds1302_Byte(RTC_W[addr],RTC_HC[HC]/10*16+RTC_HC[HC]%10);
	Write_Ds1302_Byte(RTC_W[7],0x80);
}
//------------------------------------------温度读取-------------------------------------------
u16 WD_B=0;
void DSi8B20_R()
{
	u8	L,H;
	u16	TEMP;
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0x44);
	
	Delay_OneWire(200);
	
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0xbe);
	
	L=Read_DS18B20();
	H=Read_DS18B20();
	
	TEMP=(H&0x0F)<<8;
	TEMP|=L;
	
	WD_B=(TEMP*0.625);
}

//--------------------------------------------按键输入-----------------------------------------
u8 key_state=0;
u8 key_value=0;
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
		key_state=0;
		key_value=0;
	}
}

u8 key_ON()
{
		P3|=0x0f;
	if(~P3&0x0f)
	{
		if(P33==0){return 8;}
		
	}
	return 0;
}
//--------------------------------------------界面显示---------------------------------------
u8 key_RTC_flag,key_flag;//时钟加减标志位//时钟选择标志位
u8 key_Clock_flag;
u8 temperature,temperature_flag;
void RTC_JM_XS()
{
	if(temperature==0)
	{
		switch(key_flag)
		{
			case 0:
				temperature_flag=1;
				SJ_flag=1;//时钟暂停标志位
				key_RTC_flag=0;//时钟加减标志位
				LED_XSBUFF(RTC_HC[2]/10,RTC_HC[2]%10,11,RTC_HC[1]/10,RTC_HC[1]%10,11,RTC_HC[0]/10,RTC_HC[0]%10);
				break;
			case 1:
				temperature_flag=0;
				SJ_flag=0;//时钟暂停标志位
				key_RTC_flag=1;//时钟加减标志位
				if(JM_flag==0)
				{
					LED_XSBUFF(RTC_HC[2]/10,RTC_HC[2]%10,11,RTC_HC[1]/10,RTC_HC[1]%10,11,RTC_HC[0]/10,RTC_HC[0]%10);			
				}
				else
				{
					LED_XSBUFF(10,10,11,RTC_HC[1]/10,RTC_HC[1]%10,11,RTC_HC[0]/10,RTC_HC[0]%10);
				}
				break;
			case 2:
				key_RTC_flag=2;//时钟加减标志位
			if(JM_flag==0)
				{
					LED_XSBUFF(RTC_HC[2]/10,RTC_HC[2]%10,11,RTC_HC[1]/10,RTC_HC[1]%10,11,RTC_HC[0]/10,RTC_HC[0]%10);			
				}
				else
				{
					LED_XSBUFF(RTC_HC[2]/10,RTC_HC[2]%10,11,10,10,11,RTC_HC[0]/10,RTC_HC[0]%10);
				}
				break;
			case 3:
				key_RTC_flag=3;//时钟加减标志位
			if(JM_flag==0)
				{
					LED_XSBUFF(RTC_HC[2]/10,RTC_HC[2]%10,11,RTC_HC[1]/10,RTC_HC[1]%10,11,RTC_HC[0]/10,RTC_HC[0]%10);
				}
				else
				{
					LED_XSBUFF(RTC_HC[2]/10,RTC_HC[2]%10,11,RTC_HC[1]/10,RTC_HC[1]%10,11,10,10);
				}
				break;
			case 4:
				temperature_flag=0;
				key_Clock_flag=0;
				LED_XSBUFF(Clock_HC[2]/10,Clock_HC[2]%10,11,Clock_HC[1]/10,Clock_HC[1]%10,11,Clock_HC[0]/10,Clock_HC[0]%10);
				break;
			case 5:
				key_Clock_flag=1;
				if(JM_flag==0)
				{
					LED_XSBUFF(Clock_HC[2]/10,Clock_HC[2]%10,11,Clock_HC[1]/10,Clock_HC[1]%10,11,Clock_HC[0]/10,Clock_HC[0]%10);
				}
				else
				{
					LED_XSBUFF(10,10,11,Clock_HC[1]/10,Clock_HC[1]%10,11,Clock_HC[0]/10,Clock_HC[0]%10);
				}
				break;
			case 6:
				key_Clock_flag=2;
				if(JM_flag==0)
				{
					LED_XSBUFF(Clock_HC[2]/10,Clock_HC[2]%10,11,Clock_HC[1]/10,Clock_HC[1]%10,11,Clock_HC[0]/10,Clock_HC[0]%10);
				}
				else
				{
					LED_XSBUFF(Clock_HC[2]/10,Clock_HC[2]%10,11,10,10,11,Clock_HC[0]/10,Clock_HC[0]%10);
				}
				break;
			case 7:
				key_Clock_flag=3;
				if(JM_flag==0)
				{
					LED_XSBUFF(Clock_HC[2]/10,Clock_HC[2]%10,11,Clock_HC[1]/10,Clock_HC[1]%10,11,Clock_HC[0]/10,Clock_HC[0]%10);
				}
			else
				{
					LED_XSBUFF(Clock_HC[2]/10,Clock_HC[2]%10,11,Clock_HC[1]/10,Clock_HC[1]%10,11,10,10);
				}
			break;	
		}
	}else if(temperature==8&&temperature_flag==1)
		{
			LED_XSBUFF(10,10,10,10,10,WD_B/100,(WD_B%100/10)+13,WD_B%10);
		}
}

//----------------------------------------------时钟设置处理-----------------------------------------
void key_dispose()
{
	if(Time10ms>=10)
			{
				key_scan();
				Time10ms=0;
				if(key_value==7&&key_state==2)//进入时钟设置
				{
					key_flag++;
					if(key_flag>3)key_flag=0;//时钟选择标志位
				}
				else if(key_value==6&&key_state==2)//进入闹钟设置
				{
					if(key_flag<=3)key_flag=3;
					key_flag++;
					if(key_flag>7)key_flag=4;//闹钟选择标志位
				}
				/*---------------------------------时钟加减处理--------------------------------------------*/
				else if(key_value==5&&key_state==2&&SJ_flag==0&&key_RTC_flag==1)//小时加处理
				{
					if(RTC_HC[2]>=23){RTC_HC[2]=0;}
					else {RTC_HC[2]++;}
					RTC_byte_write(2,2);
				}
				else if(key_value==5&&key_state==2&&SJ_flag==0&&key_RTC_flag==2)//分钟加处理
				{
					if(RTC_HC[1]>=59){RTC_HC[1]=0;}
					else {RTC_HC[1]++;}
					RTC_byte_write(1,1);
				}
				else if(key_value==5&&key_state==2&&SJ_flag==0&&key_RTC_flag==3)//秒加处理
				{
					if(RTC_HC[0]>=59){RTC_HC[0]=0;}
					else {RTC_HC[0]++;}
					RTC_byte_write(0,0);
				}
				else if(key_value==4&&key_state==2&&SJ_flag==0&&key_RTC_flag==1)//小时减处理
				{
					if(RTC_HC[2]<=0){RTC_HC[2]=23;}
					else {RTC_HC[2]--;}
					RTC_byte_write(2,2);
				}
				else if(key_value==4&&key_state==2&&SJ_flag==0&&key_RTC_flag==2)//分钟减处理
				{
					if(RTC_HC[1]<=0){RTC_HC[1]=59;}
					else {RTC_HC[1]--;}
					RTC_byte_write(1,1);
				}
				else if(key_value==4&&key_state==2&&SJ_flag==0&&key_RTC_flag==3)//秒减处理
				{
					if(RTC_HC[0]<=0){RTC_HC[0]=59;}
					else {RTC_HC[0]--;}
					RTC_byte_write(0,0);
				}
				/*---------------------------------闹钟加减处理--------------------------------------------*/
				else if(key_value==5&&key_state==2&&key_Clock_flag==1)//小时加处理
				{
					if(Clock_HC[2]>=23){Clock_HC[2]=0;}
					else {Clock_HC[2]++;}
				}
				else if(key_value==5&&key_state==2&&key_Clock_flag==2)//分钟加处理
				{
					if(Clock_HC[1]>=59){Clock_HC[1]=0;}
					else {Clock_HC[1]++;}
				}
				else if(key_value==5&&key_state==2&&key_Clock_flag==3)//秒加处理
				{
					if(Clock_HC[0]>=59){Clock_HC[0]=0;}
					else {Clock_HC[0]++;}
				}
				else if(key_value==4&&key_state==2&&key_Clock_flag==1)//小时减处理
				{
					if(Clock_HC[2]<=0){Clock_HC[2]=23;}
					else {Clock_HC[2]--;}
				}
				else if(key_value==4&&key_state==2&&key_Clock_flag==2)//分钟减处理
				{
					if(Clock_HC[1]<=0){Clock_HC[1]=59;}
					else {Clock_HC[1]--;}
				}
				else if(key_value==4&&key_state==2&&key_Clock_flag==3)//秒减处理
				{
					if(Clock_HC[0]<=0){Clock_HC[0]=59;}
					else {Clock_HC[0]--;}
				}
			}
}

u8 Compare;
void RTC_Clock_Compare()
{
	if((Clock_HC[2]==RTC_HC[2])&&(Clock_HC[1]==RTC_HC[1])&&(Clock_HC[0]==RTC_HC[0]))
	{
		
		Compare=1;
	}
	
}

//-----------------------------------------主函数-----------------------------------------
void main()
{
	SYS;
	Timer0_Init();
	RTC_write();
	
		while(1)
		{
			key_dispose();
			RTC_JM_XS();
			if(temperature_ms==500){DSi8B20_R();temperature_ms=0;}
			temperature=key_ON();

		}
}

u8	j=0;
u16 k=0,l=0;
void Time0_interrupt() interrupt 1
{
	Time10ms++;
	temperature_ms++;
	LED_display();
	if(++JM_1s==1000){JM_flag=~JM_flag;JM_1s=0;}
	if(++j==200&&SJ_flag==1){RTC_read();j=0;}
	RTC_Clock_Compare();
	if(Compare==1)
	{
		if(key_value==4|key_value==5|key_value==6|key_value==7)
		{
			Compare=0;
			Y4;P0=0xff;
		}
		k++;
		if(k<=200){Y4;P0=0xfe;}
		if(k>=200){Y4;P0=0xff;}
		if(k==400)k=0;
		if(++l==5000){l=0;Compare=0;}
	}
}