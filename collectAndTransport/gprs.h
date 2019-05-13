/*** 数据传输相关代码 头文件 ***/

#ifndef __GPRS_H_
#define __GPRS_H_

#include <reg51.h>
#include <string.h>
#include <intrins.h>

//---重定义关键词---//
#ifndef uchar
#define uchar unsigned char
#endif

#ifndef uint
#define uint unsigned int
#endif

// 以下是51单片机的晶振大小
#define FOSC_110592M
// #define FOSC_12M

//以下用于保存单片机收到模块发来的AT指令，通过这些指令单片机可以判断模块的状态
uchar GsmRcv[50] = {0};
uchar GsmRcvAt[50] = {0};
uchar GsmRcvCnt = 0;
uchar GsmAtFlag = 0;

//--定义使用的IO口--//
sbit Signal = P1 ^ 0;

//--声明全局函数--//
void SerialInit();
void DataTransmission();
void UartSend(uchar *str);
void SerialInterrupt() interrupt 4;
unsigned char Hand(unsigned char *data_source, unsigned char *ptr);

#endif
