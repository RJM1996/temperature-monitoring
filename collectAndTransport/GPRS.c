/************************************************************
程序说明：
本程序运行后如果GPRS模块找到服务商信号，就会向服务器发送信息
1.将51单片机的串口1连接到GSM的232接口
2.找到程序中前面的#define处，根据说明修改好自己的单片机配置.
3.下载程序
4.启动模块，等待信号灯闪烁变慢，如果模块和手机卡正常工作，服务器将收到模块发来的信息
*************************************************************/
#include <reg52.h>
#include <string.h>

#define uchar unsigned char #define uint unsigned int

//以下是你的51单片机的晶振大小
#define FOSC_110592M
//#define FOSC_12M

//以下用于保存单片机收到模块发来的AT指令，通过这些指令单片机可以判断模块的状态
uchar GsmRcv[50] = {0};
uchar GsmRcvAt[50] = {0};
uchar GsmRcvCnt = 0;
uchar GsmAtFlag = 0;

// 初始化程序（必须使用，否则无法收发），次程序将会使用定时器1
void SerialInit() //初始化程序（必须使用，否则无法收发）
{
	TMOD = 0x20; //定时器1操作模式2:8位自动重载定时器

#ifdef FOSC_12M //在这里根据晶振大小设置不同的数值初始化串口
	TH1 = 0xf3; //装入初值，波特率2400
	TL1 = 0xf3;
#else
	TH1 = 0xfd; //装入初值，波特率9600
	TL1 = 0xfd;
#endif
	TR1 = 1; //打开定时器
	SM0 = 0; //设置串行通讯工作模式，（10为一部发送，波特率可变，由定时器1的溢出率控制）
	SM1 = 1; //(同上)在此模式下，定时器溢出一次就发送一个位的数据
	REN = 1; //串行接收允许位（要先设置sm0sm1再开串行允许）
	EA = 1;  //开总中断
	ES = 1;  //开串行口中断
}

/*
 * 串行通讯中断
 * 无论接收到信号还是发送完信号，都会进入该中断服务程序  
 */
void Serial_interrupt() interrupt 4
{
	uchar i = 0;

	if (RI == 1) // 收到信息
	{

		RI = 0; // 接收中断信号清零，表示将继续接收

		GsmRcv[GsmRcvCnt] = SBUF;
		// Uart1Send(tmp);
		GsmRcvCnt++;
		// 收到了完整的AT指令，完整的AT指令是以0x0a 0x0d结尾的。故作此判断，在接收的过程中是否收到0x0a 0x0d
		if (GsmRcv[GsmRcvCnt - 2] == 0x0d && GsmRcv[GsmRcvCnt - 1] == 0x0a && GsmRcvCnt >= 2)
		{
			// 一旦收到0x0a 0x0d，就将数据保存起来。用户主函数的判断。
			for (i = 0; i < GsmRcvCnt; i++)
			{
				GsmRcvAt[i] = GsmRcv[i];
				GsmRcv[i] = 0;
			}
			GsmRcvCnt = 0;
			GsmAtFlag = 1; // 收到了完整的at指令，通过这个标志位置1，这样主函数就知道去判断了。
		}
		else if (GsmRcvCnt >= 50) // 因为内存有限，收到了50个字符还是没有看到0x0a 0x0d的话，就重新开始接收吧。
		{
			GsmRcvCnt = 0;
		}
	}
}

void Uart1Send(uchar c)
{
	SBUF = c;
	while (!TI)
		; // 等待发送完成信号（TI=1）出现
	TI = 0;
}

// 串行口连续发送char型数组，遇到终止号/0将停止
void Uart1Sends(uchar *str)
{
	while (*str != '\0')
	{
		SBUF = *str;
		while (!TI)
		{
			; // 等待发送完成信号（TI=1）出现
		}

		TI = 0;
		str++;
	}
	// 延时1s
	DelaySec(1);
}

// 延时函数大概是1s，不过延时大的话不准
void DelaySec(int sec)
{
	uint i, j = 0;

	for (i = 0; i < sec; i++)
	{
		for (j = 0; j < 65535; j++)
		{
		}
	}
}

// sim900的初始化工作
void sim900Init()
{
	uchar i = 0;
	// 1.串口初始化
	SerialInit();

	// 2.判断是否启动完成
	GsmAtFlag = 0;
	while (GsmAtFlag == 0)
	{
		Uart1Sends("ATI\r\n"); // 设置sim900模块的波特率
		DelaySec(1);		   // 延时1秒
	}
	GsmAtFlag = 0;

	// 3.检测信号
	while (1)
	{
		Uart1Sends("AT+COPS?\r\n");
		DelaySec(2); // 延时2秒
		while (GsmAtFlag == 0)
		{
			;
		}
		// strstr(str1, str2) 用于判断 str2 是否为 str1 的子串
		if (strstr(GsmRcvAt, "CHINA")) // 检测是否收到 CHINA MOBILE 服务商信息。如果收到证明是连接上网络了
		{
			break;
		}
	}
}

// 数据传输
void dataTransmission()
{
	// 4.发送指令，启动HTTP服务

	// (1) 初始化HTTP服务
	Uart1Sends("AT+HTTPINIT\r\n");

	// (2) 设置连接类型为 GPRS
	Uart1Sends("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r\n");

	// (3) 设置 APN 中国移动就是 CMNET
	Uart1Sends("AT+SAPBR=3,1,\"APN\",\"CMNET\"\r\n");

	// 打开数据连接
	Uart1Sends("AT+SAPBR=1,1\r\n");

	// (4) 设置接口的URL地址
	Uart1Sends("AT+HTTPPARA=\"URL\",\"http://47.93.189.83/api/device/savetemperature\"\r\n");

	if (GsmAtFlag == 1 && strstr(GsmRcvAt, "OK"))
	{
		// (5) 因为我们是POST请求，所以要设置POST数据的大小和输入时间
		Uart1Sends("AT+HTTPDATA=147,10000"); // 即post数据大小为147字节 输入时间为10000ms(10s)

		// (6) 接下来当我们收到返回为 download 时，就可以开始输入我们的post数据了
		if (GsmAtFlag == 1 && strstr(GsmRcvAt, "DOWNLOAD"))
		{
			// 设置post数据
			Uart1Sends("{\"did\": \"83457508\",\"verify_code\": \"6K9F99MI\",\"temperature\":{\"hundred\" : 0,\"decade\" : 3,\"unit\" : 8,\"zeroPointOne\" : 7,\"zeroPointZeroOne\" : 5}}\r\n");
		}
	}

	// (7) 收到返回为 OK 之后，就可以激活 post 请求了
	if (GsmAtFlag == 1 && strstr(GsmRcvAt, "OK"))
	{
		// 激活post请求 get:0 post:1 head:2
		Uart1Sends("AT+HTTPACTION=1");
		if (GsmAtFlag == 1 && strstr(GsmRcvAt, "OK"))
		{
			DelaySec(5);
			// 判断响应是否成功
			for (int i = 1; i <= 3 && GsmAtFlag == 1 && strstr(GsmRcvAt, "+HTTPACTION:1,200,52"); i++)
			{
				// 响应成功，开始读取响应
				Uart1Sends("HTTPREAD:0,52");
				if (GsmAtFlag == 1 && RcvIsSuccess(GsmRcvAt))
				{
					// 正确响应内容为：{"code":200, "status":"success", "uid":0, "bizdata":""}
				}
			}
			// 之后关闭 HTTP 服务
			Uart1Sends("AT+HTTPTERM\r\n");
		}
	}
}
