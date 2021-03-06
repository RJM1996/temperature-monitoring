#ifndef __TEMP_H_
#define __TEMP_H_

#include <reg51.h>

//---重定义关键词---//
#ifndef uchar
#define uchar unsigned char
#endif

#ifndef uint
#define uint unsigned int
#endif

// LED灯
sbit Signal = P1 ^ 0;

//--定义使用的IO口--//
sbit DSPORT = P3 ^ 7;

//--声明全局函数--//
void Delay1ms(uint);
uchar Ds18b20Init();
void Ds18b20WriteByte(uchar com);
uchar Ds18b20ReadByte();
void Ds18b20ChangTemp();
void Ds18b20ReadTempCom();
int Ds18b20ReadTemp();

#endif
