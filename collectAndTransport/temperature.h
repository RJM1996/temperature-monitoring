/*** �¶Ȳɼ���� ͷ�ļ� ***/

#ifndef __TEMP_H_
#define __TEMP_H_

#include <reg51.h>

//---�ض���ؼ���---//
#ifndef uchar
#define uchar unsigned char
#endif

#ifndef uint
#define uint unsigned int
#endif

//--����ʹ�õ�IO��--//
sbit DSPORT = P3 ^ 7;
sbit LSA = P2 ^ 2;
sbit LSB = P2 ^ 3;
sbit LSC = P2 ^ 4;

uchar DisplayData[8];
// ����������ܶ�ѡ
uchar code smgduan[10] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f};
// �¶ȵ�λ�� 100-10-1-0.1-0.01
int hundred = 0;
int decade = 0;
int unit = 0;
int zeroPointOne = 0;
int zeroPointZeroOne = 0;

//--����ȫ�ֺ���--//
uchar Ds18b20Init();
void Ds18b20WriteByte(uchar com);
uchar Ds18b20ReadByte();
void Ds18b20ChangTemp();
void Ds18b20ReadTempCom();
int Ds18b20ReadTemp();
void DigDisplay();
void DataTrans(int temp);
#endif
