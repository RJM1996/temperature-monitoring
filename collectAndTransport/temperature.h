/*** 温度采集相关 头文件 ***/

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

//--定义使用的IO口--//
sbit DSPORT = P3 ^ 7;
sbit LSA = P2 ^ 2;
sbit LSB = P2 ^ 3;
sbit LSC = P2 ^ 4;

uchar DisplayData[8];
// 共阴极数码管段选
uchar code smgduan[10] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f};
// 温度的位数 100-10-1-0.1-0.01
int hundred = 0;
int decade = 0;
int unit = 0;
int zeroPointOne = 0;
int zeroPointZeroOne = 0;

//--声明全局函数--//
uchar Ds18b20Init();
void Ds18b20WriteByte(uchar com);
uchar Ds18b20ReadByte();
void Ds18b20ChangTemp();
void Ds18b20ReadTempCom();
int Ds18b20ReadTemp();
void DigDisplay();
void DataTrans(int temp);
#endif
