/*** ���ݴ�����ش��� ͷ�ļ� ***/

#ifndef __GPRS_H_
#define __GPRS_H_

#include <reg51.h>
#include <string.h>
#include <intrins.h>

//---�ض���ؼ���---//
#ifndef uchar
#define uchar unsigned char
#endif

#ifndef uint
#define uint unsigned int
#endif

// ������51��Ƭ���ľ����С
#define FOSC_110592M
// #define FOSC_12M

//�������ڱ��浥Ƭ���յ�ģ�鷢����ATָ�ͨ����Щָ�Ƭ�������ж�ģ���״̬
uchar GsmRcv[50] = {0};
uchar GsmRcvAt[50] = {0};
uchar GsmRcvCnt = 0;
uchar GsmAtFlag = 0;

//--����ʹ�õ�IO��--//
sbit Signal = P1 ^ 0;

//--����ȫ�ֺ���--//
void SerialInit();
void DataTransmission();
void UartSend(uchar *str);
void SerialInterrupt() interrupt 4;
unsigned char Hand(unsigned char *data_source, unsigned char *ptr);

#endif
