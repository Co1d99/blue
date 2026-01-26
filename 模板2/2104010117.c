#include <STC15F2K60S2.H>
#include "onewire.h"
#include "iic.h"
#include "ds1302.h"

#define YY P2=(P2&0x1f)
#define Y4 YY|0x80
#define Y5 YY|0xa0
#define Y6 YY|0xc0
#define Y7 YY|0xe0

typedef unsigned char u8;
typedef unsigned int  u16;

#define SYS P0=0x00;Y5;YY;P0=0xff;Y4;YY;

u8  EPR[]={150};
u8  LED_V=0xff,Time10ms,num;
u16 Time1s;

//                                                                 9   10   11a 12b  13c   14d  15e  16f
code u8 LED_duan[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xff,0x88,0x83,0xc6,0xa1,0x86,0x8e,
										0xc0&0x7f,0xf9&0x7f,0xa4&0x7f,0xb0&0x7f,0x99&0x7f,0x92&0x7f,0x82&0x7f,0xf8&0x7f,0x80&0x7f,0x90&0x7f};
u8 LED_buff[]={10,10,10,10,10,10,10,10};

void LED(){P0=LED_V;Y4;YY;}

void Timer0Init(void)	
{
	AUXR |= 0x80;
	TMOD &= 0xF0;
	TMOD |= 0x04;
	TL0 = 0x00;	
	TH0 = 0x00;	
	TF0 = 0;		
	TR0 = 1;		
}

void Timer1Init(void)	
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



void LED_Display()
{
	static u8 i;
	P0=0xff;Y7;YY;
	P0=0x01<<i;Y6;YY;
	P0=LED_duan[LED_buff[i]];Y7;YY;
	if(++i==8)i=0;
}

void LED_Disbuff(u8 d1,d2,d3,d4,d5,d6,d7,d8)
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

u16 WD;
void DS18B20_R()
{
	u8 	L,H;
	u16 temp;
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0x44);
	 
	Delay_OneWire(200);
	
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0xbe);
	
	L=Read_DS18B20();
	H=Read_DS18B20();
	
	temp=(H&0x0f)<<8;
	temp|=L;
	
	WD=temp*0.625;
}

u16 ADC,ADCR;
void ADC_R(u8 chennl)
{
	I2CStart();
	I2CSendByte(0x90);
	I2CWaitAck();
	
	I2CSendByte(chennl);
	I2CWaitAck();
	
	I2CStart();
	I2CSendByte(0x91);
	I2CWaitAck();

	ADCR=I2CReceiveByte();
	I2CSendAck(1);
	I2CStop();
	
	ADC=ADCR*500.0/255;
}

u8 DAC;
void DAC_W(u8 dat)
{
	I2CStart();
	I2CSendByte(0x90);
	I2CWaitAck();
	
	I2CSendByte(0x40);
	I2CWaitAck();
	
	I2CSendByte(dat);
	I2CWaitAck();
	I2CStop();
	
	DAC=dat/255.0*500;
}

u8 EEPRM;
void EEPROM_R(u8 addr,u8* dat,u8 len)
{
	I2CStart();
	I2CSendByte(0xa0);
	I2CWaitAck();
	
	I2CSendByte(addr);
	I2CWaitAck();
	
	I2CStart();
	I2CSendByte(0xa1);
	I2CWaitAck();
	while(len--)
	{
		*dat++=I2CReceiveByte();
		if(len)I2CSendAck(0);
		else   I2CSendAck(1);
	}
	I2CStop();

}

void EEPROM_W(u8 addr,u8* dat,u8 len)
{
	
	I2CStart();
	I2CSendByte(0xa0);
	I2CWaitAck();
	
	I2CSendByte(addr);
	I2CWaitAck();
	while(len--)
	{
		I2CSendByte(*dat++);
		I2CWaitAck();
		I2C_Delay(200);
	}
	I2CStop();
}

u8 RTC_R[7]={0x81,0x83,0x85,0x87,0x89,0x8b,0x8d};
u8 RTC_W[8]={0x80,0x82,0x84,0x86,0x88,0x8a,0x8c,0x8e};
int RTC_HC[]={50,59,23,30,11,6,23};

void DS1302_R()
{
	u8 temp,i;
	for(i=0;i<7;i++)
	{
		temp=Read_Ds1302_Byte(RTC_R[i]);
		RTC_HC[i]=temp/16*10+temp%16;		
	}
}

void DS1302_W()
{
	u8 i;
	Write_Ds1302_Byte(RTC_W[7],0x00);
	for(i=0;i<7;i++)
	{
		Write_Ds1302_Byte(RTC_W[i],RTC_HC[i]/10*16+RTC_HC[i]%10);
	}
	Write_Ds1302_Byte(RTC_W[7],0x80);
}

//u8 key_steta,key_value;
//void key_scan()
//{
//	P3=0x0f;
//	if(~P3&0x0f)
//	{
//		if(key_steta<255)key_steta++;
//		if(P30==0)key_value=7;
//		if(P31==0)key_value=6;
//		if(P32==0)key_value=5;
//		if(P33==0)key_value=4;
//	}
//	else
//	{
//		key_value=0;
//		key_steta=0;
//	}
//}

u8 key_steta,key_value,x,y;
u8 key_values[4][4]=
{
	7,11,15,19,
	6,10,14,18,
	5,9,13,17,
	4,8,12,16
};
void key_scan()
{
	P3=0x0f;
	P4=0x00;
	if(~P3&0x0f)
	{
		if(key_steta<255)key_steta++;
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
		key_value=0;
		key_steta=0;
	}
}

void key_dispose()
{
	key_scan();
	if(key_value==4&&key_steta==2)
	{
		num++;
	}	
}



void main()
{
	SYS;
	Timer0Init();
	Timer1Init();
	DAC_W(102);
	DS1302_W();
//	EEPROM_W(0x00,EPR,1);
	while(1)
	{
		
//	if(Time1s%500==0)DS18B20_R();
//	if(Time1s%200==0)ADC_R(0x43);
//	if(Time1s%20==0)EEPROM_R(0x00,EPR,1);
//	LED_Disbuff(1,10,10,10,10,WD/100,WD%100/10+17,WD%10);
//	LED_Disbuff(1,10,10,10,10,ADC/100+17,ADC%100/10,ADC%10);
//	LED_Disbuff(1,10,10,10,10,DAC/100+17,DAC%100/10,DAC%10);
//	LED_Disbuff(1,10,10,10,10,EPR[0]/100,EPR[0]%100/10,EPR[0]%10);
//		if(Time1s%800==0)DS1302_R();
//		LED_Disbuff(RTC_HC[2]/10,RTC_HC[2]%10,10,RTC_HC[1]/10,RTC_HC[1]%10,10,RTC_HC[0]/10,RTC_HC[0]%10);
		LED_Disbuff(num,10,10,10,10,10,10,10);
		
	}
}




void Time1_interrupt() interrupt 3
{
	++Time10ms;
	++Time1s;
	if(Time10ms==10){key_dispose();Time10ms=0;}
	if(Time1s==1000){Time1s=0;}
	LED_Display();
	
	
}
