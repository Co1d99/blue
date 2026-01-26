#include <STC15F2K60S2.H>
#define __REG52_H__
#include "iic.h"
#include "onewire.h"

#define YY P2=(P2&0x1f)
#define Y4 YY|0x80
#define Y5 YY|0xa0
#define Y6 YY|0xc0
#define Y7 YY|0xe0

typedef unsigned char u8;
typedef unsigned int u16;

#define SYS P0=0xff;Y4;YY;P0=0x00;Y5;YY;

u8  Time10ms,ADC_Time10ms,WD_Time10ms,LED_V=0xff;
u16 Time1S,NE555;

//                                                                     10   11p 12n  13u  14-  15f  16c
u8 code LED_duan[]={0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,0xff,0x8c,0xc8,0xc1,0xbf,0x8e,0xc6,
0xC0&0x7f,0xF9&0x7f,0xA4&0x7f,0xB0&0x7f,0x99&0x7f,0x92&0x7f,0x82&0x7f,0xF8&0x7f,0x80&0x7f,0x90&0x7f,0x89};
u8 LED_buff[]={10,10,10,10,10,10,10,10};

void LED(){P0=LED_V;Y4;YY;}
void Timer0_Init(void)
{
	AUXR |= 0x80;
	TMOD &= 0xF0;
	TMOD |= 0x05;
	TL0 = 0x00;		
	TH0 = 0x00;		
	TF0 = 0;			
	TR0 = 1;			
}

void Timer1_Init(void)		//1??@12.000MHz
{
	AUXR |= 0x40;			//?????1T??
	TMOD &= 0x0F;			//???????
	TL1 = 0x20;				//???????
	TH1 = 0xD1;				//???????
	TF1 = 0;				//??TF1??
	TR1 = 1;				//???1????
	EA=1;
	ET1=1;
}

void Delay5ms()		//@12.000MHz
{
	unsigned char i, j;

	i = 59;
	j = 90;
	do
	{
		while (--j);
	} while (--i);
}


void LED_display()
{
	static u8 i;
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

u16 ADCR,ADC;
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
	ADCR=IIC_RecByte();
	IIC_SendAck(1); 
	IIC_Stop();
	
	ADC=ADCR*50.0/255;
}

void DAC_W(u8 dat)
{
	IIC_Start();
	IIC_SendByte(0x90); 
	IIC_WaitAck();  
	IIC_SendByte(0x40); 
	IIC_WaitAck(); 
	IIC_SendByte(dat);
	IIC_WaitAck(); 
	IIC_Stop();
}

u16 WD;
void DS18B20_R()
{
	u8 L,H;
	u16 Temp;
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0x44);
	
	Delay_OneWire(200);
	
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0xbe);
	
	L=Read_DS18B20();
	H=Read_DS18B20();
	
	Temp=(H&0x0f)<<8;
	Temp|=L;
	
	WD=Temp*6.25;
}

void EEPROM_W(u8 addr,dat)
{
	IIC_Start();
	IIC_SendByte(0xa0);
	IIC_WaitAck(); 
	IIC_SendByte(addr);
	IIC_WaitAck();
	IIC_SendByte(dat); 
	IIC_WaitAck();
	IIC_Stop();
}
u16 EEPR;
void EEPROM_R(u8 addr)
{
	IIC_Start();
	IIC_SendByte(0xa0);
	IIC_WaitAck(); 
	IIC_SendByte(addr);
	IIC_WaitAck();
	
	IIC_Start();
	IIC_SendByte(0xa1);
	IIC_WaitAck();
	EEPR=IIC_RecByte();
	IIC_Stop();
}

u8 key_steta,key_value;
void key_scan()
{
	P3=0x0f;
	if(~P3&0x0f)
	{
		if(key_steta<255)key_steta++;
		if(P30==0)key_value=7;
		if(P31==0)key_value=6;
		if(P32==0)key_value=5;
		if(P33==0)key_value=4;
		
	}else
	{
		key_steta=0;
		key_value=0;
	}
}
u8 key_flag,key_H_flag,ADC_value,HX_flag,key_HX_flag;
u16 EEPR_WD,EEPR_ADC,EEPR_NE555;
void key_dispose()
{
	key_scan();
	if(key_value==7&&key_steta==2)
	{
		key_flag=3;
		key_HX_flag=!key_HX_flag;
	}
	if(key_value==6&&key_steta==2&&HX_flag==0)
	{
		if(ADC_value>=50||ADC_value<=0){ADC_value=1;}else{++ADC_value;}
	}
	if(key_value==6&&key_steta==2&&HX_flag==1)
	{
		key_H_flag=!key_H_flag;
		EEPROM_R(0x00);
		EEPR_ADC=EEPR;
		EEPROM_R(0x01);
		EEPR_WD=EEPR*100;
		EEPROM_R(0x02);
		EEPR_WD=EEPR_WD+EEPR;
		EEPROM_R(0x03);
		EEPR_NE555=EEPR*1000;
		EEPROM_R(0x04);
		EEPR_NE555=EEPR_NE555+EEPR*10;
		EEPROM_R(0x05);
		EEPR_NE555=EEPR_NE555+EEPR;
	}
	
	
	if(key_value==5&&key_steta==2)
	{
		EEPROM_W(0x00,ADC);Delay5ms();
		EEPROM_W(0x01,WD/100);Delay5ms();
		EEPROM_W(0x02,WD%100);Delay5ms();
		EEPROM_W(0x03,NE555/1000);Delay5ms();
		EEPROM_W(0x04,NE555%1000/10);Delay5ms();
		EEPROM_W(0x05,NE555%10);Delay5ms();
	}
	if(key_value==4&&key_steta==2)
	{
		if(++key_flag==3){key_flag=0;}
	}

}

void JM_QH()
{
	switch(key_flag)
	{
		case 0:
			HX_flag=1;
			if(key_H_flag==0)
			{
				LED_V&=0xfb;
				LED_V|=~0xfd;
				LED_V|=~0xfe;
				LED_displaybuff(13,10,10,10,10,10,ADC/10+17,ADC%10);
			}
			else
			{
				
				LED_displaybuff(27,13,10,10,10,10,EEPR_ADC/10+17,EEPR_ADC%10);
			}
		break;
		case 1:
			HX_flag=1;
			if(key_H_flag==0)
			{
				LED_V&=0xfd;
				LED_V|=~0xfe;
				LED_V|=~0xfb;
				LED_displaybuff(15,10,10,NE555/10000,NE555%10000/1000,NE555%10000%1000/100,NE555%10000%1000%100/10,NE555%10);
			}
			else
			{	
				
				LED_displaybuff(27,15,10,EEPR_NE555/10000,EEPR_NE555%10000/1000,EEPR_NE555%10000%1000/100,EEPR_NE555%10000%1000%100/10,EEPR_NE555%10);
			}
		break;
		case 2:
			HX_flag=1;
			if(key_H_flag==0)
			{
				LED_V&=0xfe;
				LED_V|=~0xfd;
				LED_V|=~0xfb;
				LED_displaybuff(16,10,10,10,WD/1000,WD%1000/100+17,WD%1000%100/10,WD%10);
			}
			else
			{
				LED_displaybuff(27,16,10,10,EEPR_WD/1000,EEPR_WD%1000/100+17,EEPR_WD%1000%100/10,EEPR_WD%10);
			}
		break;
		case 3:
				HX_flag=0;
				LED_displaybuff(11,10,10,10,10,10,ADC_value/10+17,ADC_value%10);
				if(key_HX_flag==0){key_flag=0;}
		break;
	}
}

void main()
{
	SYS;
	Timer0_Init();
	Timer1_Init();
	
	while(1)
	{
		if(Time1S%20==0){ADC_R(0x03);ADC_Time10ms=0;}
		if(Time1S%800==0){DS18B20_R();WD_Time10ms=0;}
		JM_QH();
	}
	

}

void Time1_interrupt() interrupt 3
{
	++Time1S;
	if(++Time10ms==10){Time10ms=0;key_dispose();}
	if(Time1S%5){LED();}
	if(Time1S==1000)
	{
		Time1S=0;
		TR0=0;
		NE555=TH0<<8;
		NE555|=TL0;
		TL0 = 0x00;		
		TH0 = 0x00;
		TR0=1;
	}
	
	
	LED_display();
}