/******DS18B20温度传感器******/

#include "temp.h"
#include "GPRS.c" // 通信模块 数据传输

typedef unsigned int u16; //对数据类型进行声明定义
typedef unsigned char u8;

sbit LSA = P2 ^ 2;
sbit LSB = P2 ^ 3;
sbit LSC = P2 ^ 4;

char num = 0;
u8 DisplayData[8];
// 共阴极数码管段选
u8 code smgduan[10] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f};
// 温度的位数 100-10-1-0.1-0.01
int hundred = 0;
int decade = 0;
int unit = 0;
int zeroPointOne = 0;
int zeroPointZeroOne = 0;

/*******************************************************************************
* 函 数 名         : delay
* 函数功能		   : 延时函数，i=1时，大约延时10us
*******************************************************************************/
void delay(u16 i)
{
	while (i--)
		;
}

/*******************************************************************************
* 函 数 名         : datapros()
* 函数功能		   : 温度读取处理转换函数
* 输    入         : temp
* 输    出         : 无
*******************************************************************************/
void datapros(int temp)
{
	float tp;
	if (temp < 0) //当温度值为负数
	{
		DisplayData[0] = 0x40; //   -
		//因为读取的温度是实际温度的补码，所以减1，再取反求出原码
		temp = temp - 1;
		temp = ~temp;
		tp = temp;
		temp = tp * 0.0625 * 100 + 0.5;
		//留两个小数点就*100，+0.5是四舍五入，因为C语言浮点数转换为整型的时候把小数点
		//后面的数自动去掉，不管是否大于0.5，而+0.5之后大于0.5的就是进1了，小于0.5的就
		//算加上0.5，还是在小数点后面。
	}
	else
	{
		DisplayData[0] = 0x00;
		tp = temp; //因为数据处理有小数点所以将温度赋给一个浮点型变量
		//如果温度是正的那么，那么正数的原码就是补码它本身
		temp = tp * 0.0625 * 100 + 0.5;
		//留两个小数点就*100，+0.5是四舍五入，因为C语言浮点数转换为整型的时候把小数点
		//后面的数自动去掉，不管是否大于0.5，而+0.5之后大于0.5的就是进1了，小于0.5的就
		//算加上0.5，还是在小数点后面。
	}
	hundred = temp / 10000;
	decade = temp % 10000 / 1000;
	unit = temp % 1000 / 100;
	zeroPointOne = temp % 100 / 10;
	zeroPointZeroOne = temp % 10;
	// 100-10-1.-0.1-0.01
	DisplayData[1] = smgduan[hundred];
	DisplayData[2] = smgduan[decade];
	DisplayData[3] = smgduan[unit] | 0x80;
	DisplayData[4] = smgduan[zeroPointOne];
	DisplayData[5] = smgduan[zeroPointZeroOne];
}

/*******************************************************************************
* 函数名         :DigDisplay()
* 函数功能       :数码管显示函数
* 输入           : 无
* 输出         	 : 无
*******************************************************************************/
void DigDisplay()
{
	u8 i;
	for (i = 0; i < 6; i++)
	{
		switch (i) //位选，选择点亮的数码管，
		{
		case (0):
			LSA = 0;
			LSB = 0;
			LSC = 0;
			break; //显示第0位
		case (1):
			LSA = 1;
			LSB = 0;
			LSC = 0;
			break; //显示第1位
		case (2):
			LSA = 0;
			LSB = 1;
			LSC = 0;
			break; //显示第2位
		case (3):
			LSA = 1;
			LSB = 1;
			LSC = 0;
			break; //显示第3位
		case (4):
			LSA = 0;
			LSB = 0;
			LSC = 1;
			break; //显示第4位
		case (5):
			LSA = 1;
			LSB = 0;
			LSC = 1;
			break; //显示第5位
		}
		P0 = DisplayData[i]; //发送数据
		delay(100);			 //间隔一段时间扫描
		P0 = 0x00;			 //消隐
	}
}

/*******************************************************************************
* 函 数 名       : main
* 函数功能		 : 主函数
* 输    入       : 无
* 输    出    	 : 无
*******************************************************************************/
void main()
{
	int count = 0;
	// sim900初始化
	sim900Init();
	while (1)
	{
		// 一、温度采集
		datapros(Ds18b20ReadTemp()); //数据处理函数
		DigDisplay();				 //数码管显示函数
		// 上面两句执行完毕大概用时 200us, 1s = 1000000us
		count++;
		if (count == 5000) // 约为1s
		{
			// 二、温度数据传输
			dataTransmission();
			count = 0;
		}
	}
}
