#ifndef _iic_h_
#define _iic_h_

#include <STC15F2K60S2.H>
#include <intrins.h>

void I2CStart(void);
void I2CStop(void);
void I2CSendByte(unsigned char byt);
unsigned char I2CReceiveByte(void);
unsigned char I2CWaitAck(void);
void I2CSendAck(unsigned char ackbit);
 void I2C_Delay(unsigned char n);


#endif