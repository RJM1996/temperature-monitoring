#ifndef __GPRS_H_
#define __GPRS_H_

// 温度全局变量
extern char tempData[6];

void SerialInit();
void Sim900Init();
void HttpSend();
void Call();
void Delay1ms(unsigned int y);
void Delay1s(unsigned int sec);

#endif
