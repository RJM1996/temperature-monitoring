#include <reg51.h>
#include <string.h>
#include "GPRS.h"
#include "temp.h"

// 51单片机的晶振大小
#define FOSC_110592M

// 类型重定义
#define uchar unsigned char
#define uint unsigned int

//以下用于保存单片机收到模块发来的AT指令，通过这些指令单片机可以判断模块的状态
uchar GsmRcv[50] = {0};
uchar GsmRcvAt[50] = {0};
uchar GsmRcvCnt = 0;
uchar GsmAtFlag = 0;

// 延时函数:1s
void Delay1s(uint sec)
{
    uint i, j;
    for (i = 0; i < sec; i++)
    {
        for (j = 0; j < 65535; j++)
            ;
    }
}

// 延时函数:1ms
void Delay1ms(uint y)
{
    uint x;
    for (; y > 0; y--)
    {
        for (x = 110; x > 0; x--)
            ;
    }
}

// 初始化程序（必须使用，否则无法收发）
void SerialInit()
{
    TMOD = 0x20; //定时器1 操作模式2:8位自动重载定时器

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

// 串行通信中断程序，收发完成都将进入该程序
void Serial_interrupt() interrupt 4
{
    uchar i = 0;
    // RI 是接收中断的意思,是串口接收中断标志
    // 当RI=1时表示串口接收完成,SBUF中保存好了接收的数据,此时申请中断,如果ES=1,就进入中断服务程序了
    if (RI == 1) // 收到信息
    {
        RI = 0; // 接收中断信号清零，表示将继续接收
        GsmRcv[GsmRcvCnt] = SBUF;
        // Uart1Send(tmp);
        GsmRcvCnt++;
        // 收到了完整的AT指令，完整的AT指令是以0x0a 0x0d结尾的。故作此判断，在接收的过程中是否收到0x0a 0x0d
        if (GsmRcvCnt >= 2 && GsmRcv[GsmRcvCnt - 2] == 0x0d && GsmRcv[GsmRcvCnt - 1] == 0x0a)
        {
            // 一旦收到0x0a 0x0d，就将数据保存起来，在主函数中进行判断
            for (i = 0; i < GsmRcvCnt; i++)
            {
                GsmRcvAt[i] = GsmRcv[i];
                GsmRcv[i] = 0;
            }
            GsmRcvCnt = 0;
            GsmAtFlag = 1; // 收到了完整的AT指令，标志位置1，这样主函数就知道去判断了
        }
        else if (GsmRcvCnt >= 50) // 因为内存有限，收到了50个字符还是没有看到0x0a 0x0d的话，就重新开始接收
        {
            GsmRcvCnt = 0;
        }
    }
}

// 串行口连续发送char型数组，遇到终止号/0将停止
void Uart1Sends(uchar *str)
{
    while (*str != '\0')
    {
        SBUF = *str;
        // 等待发送完成信号（TI=1）出现
        while (!TI)
            ;
        TI = 0;
        str++;
    }
}

// led闪烁 提示作用
void LEDBlink(uchar k)
{
    uchar j;
    Signal = 0;
    for (j = 1; j <= k; j++)
    {
        Signal = 1;
        Delay1ms(400);
        Signal = 0;
        Delay1ms(400);
    }
}

void Sim900Init()
{
    uchar i = 0;
    // 判断是否启动完成
    GsmAtFlag = 0;
    while (GsmAtFlag == 0)
    {
        // 设置sim900模块的波特率
        Uart1Sends("ATI\r\n");
        Delay1s(1);
    }
    GsmAtFlag = 0;

    // 检测信号
    while (1)
    {
        Uart1Sends("AT+COPS?\r\n");
        Delay1s(2); // 延时2秒
        while (GsmAtFlag == 0)
            ;
        // strstr(str1, str2) 用于判断 str2 是否为 str1 的子串
        if (strstr(GsmRcvAt, "CHINA")) // 检测是否收到 CHINA MOBILE 服务商信息. 如果收到证明已连接上网络
        {
            break;
        }
    }
}

void HttpSend()
{
    uchar dat[100] = "AT+HTTPPARA=\"URL\",";
    uchar *dat1 = "\"http://47.93.189.83/api/device/savetemperature";
    // 其中d代表设备编号，v代表设备验证码
    uchar *dat2 = "?d=83457508&v=6K9F99MI";
    // t即为温度
    uchar *t = "&t=";
    uchar *dat3 = "\"\r\n";
    char *temper = tempData;

    // 拼接URL
    strcat(dat, dat1);
    strcat(dat, dat2);
    strcat(dat, t);
    strcat(dat, temper);
    strcat(dat, dat3);

    // 设置连接类型为 GPRS
    Uart1Sends("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r\n");
    Delay1s(1);
    // 设置 APN 中国移动就是 CMNET
    Uart1Sends("AT+SAPBR=3,1,\"APN\",\"CMNET\"\r\n");
    Delay1s(1);
    // 打开数据连接
    Uart1Sends("AT+SAPBR=1,1\r\n");
    Delay1s(2);

    // 初始化HTTP服务
    Uart1Sends("AT+HTTPINIT\r\n");
    Delay1s(1);
    // 设置URL
    Uart1Sends(dat);
    Delay1s(1);

    LEDBlink(2);
    // 发送GET请求
    Uart1Sends("AT+HTTPACTION=0\r\n");
    Delay1s(2);

    // 判断请求是否发送成功
    if (GsmAtFlag == 1 && strstr(GsmRcvAt, "OK"))
    {
        // 3下闪烁提示
        LEDBlink(3);
    }
}

// 拨打电话 用于测试
void Call()
{
    // 拨打之前先闪烁提示
    LEDBlink(2);
    Uart1Sends("ATD17730282451\r\n");
    Delay1s(2);
    // 挂断
    Uart1Sends("ATH\r\n");
    Delay1s(1);
}
