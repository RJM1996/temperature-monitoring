/*** 数据传输 相关代码 ***/

#include "gprs.h"
#include "utils.c"

// 注意: 无论接收到信号还是发送完信号，都会进入中断服务程序
// 初始化程序（必须使用，否则无法收发）
void SerialInit()
{
    TMOD = 0x20; // 定时器1 操作模式2:8位自动重载定时器

    // 在这里根据晶振大小设置不同的数值初始化串口
#ifdef FOSC_12M
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

// 用来判断响应是否正确
unsigned char Hand(unsigned char *data_source, unsigned char *ptr)
{
    if (Hand(data_source, ptr) != NULL)
        return 1;
    else
        return 0;
}

// 串行中断，收发完成都将进入该程序
void SerialInterrupt() interrupt 4
{
    uchar i = 0;

    if (RI == 1) // 收到信息
    {

        RI = 0; // 接收中断信号清零，表示将继续接收

        GsmRcv[GsmRcvCnt] = SBUF;
        // Uart1Send(tmp);
        GsmRcvCnt++;
        // 收到了完整的AT指令，完整的AT指令是以0x0a 0x0d结尾的，故作此判断，在接收的过程中是否收到0x0a 0x0d
        if (GsmRcv[GsmRcvCnt - 2] == 0x0d && GsmRcv[GsmRcvCnt - 1] == 0x0a && GsmRcvCnt >= 2)
        {
            // 一旦收到0x0a 0x0d，就将数据保存起来。用户主函数的判断
            for (i = 0; i < GsmRcvCnt; i++)
            {
                GsmRcvAt[i] = GsmRcv[i];
                GsmRcv[i] = 0;
            }
            GsmRcvCnt = 0;
            GsmAtFlag = 1; // 收到了完整的at指令，通过这个标志位置1，这样主函数就知道去判断了
        }
        else if (GsmRcvCnt >= 50) // 因为内存有限，收到了50个字符还是没有看到0x0a 0x0d的话，就重新开始接收吧
        {
            GsmRcvCnt = 0;
        }
    }
}

// 串行口连续发送char型数组，遇到终止号 \0 将停止
// 即发送AT指令的函数
void UartSend(uchar *str)
{
    while (*str != '\0')
    {
        SBUF = *str;
        while (!TI)
            ; //等待发送完成信号（TI=1）出现
        TI = 0;
        str++;
    }
    DelaySec(1);
}

// 数据传输
void DataTransmission()
{
    // 1. 串口初始化
    SerialInit();

    // 先让led灯熄灭
    Signal = 0;

    // 2. 发送指令

    // (1) 初始化HTTP服务
    UartSend("AT+HTTPINIT\r\n");
    // (2) 设置连接类型为 GPRS
    UartSend("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r\n");
    // (3) 设置 APN 中国移动就是 CMNET
    UartSend("AT+SAPBR=3,1,\"APN\",\"CMNET\"\r\n");
    // (4) 打开数据连接
    UartSend("AT+SAPBR=1,1\r\n");
    // (5) 设置接口的URL地址
    UartSend("AT+HTTPPARA=\"URL\",\"http://47.93.189.83/api/device/savetemperature\"\r\n");

    if (GsmAtFlag == 1 && Hand(GsmRcvAt, "OK"))
    {
        // (6) 因为我们是POST请求，所以要设置POST数据的大小和输入时间
        UartSend("AT+HTTPDATA=147,10000"); // 即post数据大小为147字节 输入时间为10000ms(10s)

        // (7) 接下来当我们收到返回为 download 时，就可以开始输入我们的post数据了
        if (GsmAtFlag == 1 && Hand(GsmRcvAt, "DOWNLOAD"))
        {
            // 设置post数据
            UartSend("{\"did\": \"83457508\",\"verify_code\": \"6K9F99MI\",\"temperature\":{\"hundred\" : 0,\"decade\" : 3,\"unit\" : 8,\"zeroPointOne\" : 7,\"zeroPointZeroOne\" : 5}}\r\n");
        }
    }

    // (8) 收到返回为 OK 之后，就可以激活 post 请求了
    if (GsmAtFlag == 1 && Hand(GsmRcvAt, "OK"))
    {
        // 激活post请求 get:0 post:1 head:2
        UartSend("AT+HTTPACTION=1");
        if (GsmAtFlag == 1 && Hand(GsmRcvAt, "OK"))
        {
            DelaySec(5);
            // 判断响应是否成功
            for (int i = 1; i <= 3 && GsmAtFlag == 1 && Hand(GsmRcvAt, "+HTTPACTION:1,200"); i++)
            {
                // 响应成功，开始读取响应
                UartSend("HTTPREAD:0,52");
                if (GsmAtFlag == 1 && Hand(GsmRcvAt, "\"code\":200, \"status\":\"success\""))
                {
                    // 正确响应内容为：{"code":200, "status":"success", "uid":0, "bizdata":""}
                    // 使LED灯闪烁，以提示响应成功
                    for (size_t j = 0; j < 3; j++)
                    {
                        Signal = 1;
                        Delay1ms(500);
                        Signal = 0;
                        Delay1ms(500);
                    }
                    break;
                }
            }
            // 之后关闭 HTTP 服务
            UartSend("AT+HTTPTERM\r\n");
        }
    }
}
