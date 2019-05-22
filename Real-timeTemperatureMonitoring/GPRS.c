#include <reg51.h>
#include <string.h>
#include "GPRS.h"
#include "temp.h"

// 51��Ƭ���ľ����С
#define FOSC_110592M

// �����ض���
#define uchar unsigned char
#define uint unsigned int

//�������ڱ��浥Ƭ���յ�ģ�鷢����ATָ�ͨ����Щָ�Ƭ�������ж�ģ���״̬
uchar GsmRcv[50] = {0};
uchar GsmRcvAt[50] = {0};
uchar GsmRcvCnt = 0;
uchar GsmAtFlag = 0;

// ��ʱ����:1s
void Delay1s(uint sec)
{
    uint i, j;
    for (i = 0; i < sec; i++)
    {
        for (j = 0; j < 65535; j++)
            ;
    }
}

// ��ʱ����:1ms
void Delay1ms(uint y)
{
    uint x;
    for (; y > 0; y--)
    {
        for (x = 110; x > 0; x--)
            ;
    }
}

// ��ʼ�����򣨱���ʹ�ã������޷��շ���
void SerialInit()
{
    TMOD = 0x20; //��ʱ��1 ����ģʽ2:8λ�Զ����ض�ʱ��

#ifdef FOSC_12M //��������ݾ����С���ò�ͬ����ֵ��ʼ������
    TH1 = 0xf3; //װ���ֵ��������2400
    TL1 = 0xf3;
#else
    TH1 = 0xfd; //װ���ֵ��������9600
    TL1 = 0xfd;
#endif

    TR1 = 1; //�򿪶�ʱ��
    SM0 = 0; //���ô���ͨѶ����ģʽ����10Ϊһ�����ͣ������ʿɱ䣬�ɶ�ʱ��1������ʿ��ƣ�
    SM1 = 1; //(ͬ��)�ڴ�ģʽ�£���ʱ�����һ�ξͷ���һ��λ������
    REN = 1; //���н�������λ��Ҫ������sm0sm1�ٿ���������
    EA = 1;  //�����ж�
    ES = 1;  //�����п��ж�
}

// ����ͨ���жϳ����շ���ɶ�������ó���
void Serial_interrupt() interrupt 4
{
    uchar i = 0;
    // RI �ǽ����жϵ���˼,�Ǵ��ڽ����жϱ�־
    // ��RI=1ʱ��ʾ���ڽ������,SBUF�б�����˽��յ�����,��ʱ�����ж�,���ES=1,�ͽ����жϷ��������
    if (RI == 1) // �յ���Ϣ
    {
        RI = 0; // �����ж��ź����㣬��ʾ����������
        GsmRcv[GsmRcvCnt] = SBUF;
        // Uart1Send(tmp);
        GsmRcvCnt++;
        // �յ���������ATָ�������ATָ������0x0a 0x0d��β�ġ��������жϣ��ڽ��յĹ������Ƿ��յ�0x0a 0x0d
        if (GsmRcvCnt >= 2 && GsmRcv[GsmRcvCnt - 2] == 0x0d && GsmRcv[GsmRcvCnt - 1] == 0x0a)
        {
            // һ���յ�0x0a 0x0d���ͽ����ݱ������������������н����ж�
            for (i = 0; i < GsmRcvCnt; i++)
            {
                GsmRcvAt[i] = GsmRcv[i];
                GsmRcv[i] = 0;
            }
            GsmRcvCnt = 0;
            GsmAtFlag = 1; // �յ���������ATָ���־λ��1��������������֪��ȥ�ж���
        }
        else if (GsmRcvCnt >= 50) // ��Ϊ�ڴ����ޣ��յ���50���ַ�����û�п���0x0a 0x0d�Ļ��������¿�ʼ����
        {
            GsmRcvCnt = 0;
        }
    }
}

// ���п���������char�����飬������ֹ��/0��ֹͣ
void Uart1Sends(uchar *str)
{
    while (*str != '\0')
    {
        SBUF = *str;
        // �ȴ���������źţ�TI=1������
        while (!TI)
            ;
        TI = 0;
        str++;
    }
}

// led��˸ ��ʾ����
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
    // �ж��Ƿ��������
    GsmAtFlag = 0;
    while (GsmAtFlag == 0)
    {
        // ����sim900ģ��Ĳ�����
        Uart1Sends("ATI\r\n");
        Delay1s(1);
    }
    GsmAtFlag = 0;

    // ����ź�
    while (1)
    {
        Uart1Sends("AT+COPS?\r\n");
        Delay1s(2); // ��ʱ2��
        while (GsmAtFlag == 0)
            ;
        // strstr(str1, str2) �����ж� str2 �Ƿ�Ϊ str1 ���Ӵ�
        if (strstr(GsmRcvAt, "CHINA")) // ����Ƿ��յ� CHINA MOBILE ��������Ϣ. ����յ�֤��������������
        {
            break;
        }
    }
}

void HttpSend()
{
    uchar dat[100] = "AT+HTTPPARA=\"URL\",";
    uchar *dat1 = "\"http://47.93.189.83/api/device/savetemperature";
    // ����d�����豸��ţ�v�����豸��֤��
    uchar *dat2 = "?d=83457508&v=6K9F99MI";
    // t��Ϊ�¶�
    uchar *t = "&t=";
    uchar *dat3 = "\"\r\n";
    char *temper = tempData;

    // ƴ��URL
    strcat(dat, dat1);
    strcat(dat, dat2);
    strcat(dat, t);
    strcat(dat, temper);
    strcat(dat, dat3);

    // ������������Ϊ GPRS
    Uart1Sends("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r\n");
    Delay1s(1);
    // ���� APN �й��ƶ����� CMNET
    Uart1Sends("AT+SAPBR=3,1,\"APN\",\"CMNET\"\r\n");
    Delay1s(1);
    // ����������
    Uart1Sends("AT+SAPBR=1,1\r\n");
    Delay1s(2);

    // ��ʼ��HTTP����
    Uart1Sends("AT+HTTPINIT\r\n");
    Delay1s(1);
    // ����URL
    Uart1Sends(dat);
    Delay1s(1);

    LEDBlink(2);
    // ����GET����
    Uart1Sends("AT+HTTPACTION=0\r\n");
    Delay1s(2);

    // �ж������Ƿ��ͳɹ�
    if (GsmAtFlag == 1 && strstr(GsmRcvAt, "OK"))
    {
        // 3����˸��ʾ
        LEDBlink(3);
    }
}

// ����绰 ���ڲ���
void Call()
{
    // ����֮ǰ����˸��ʾ
    LEDBlink(2);
    Uart1Sends("ATD17730282451\r\n");
    Delay1s(2);
    // �Ҷ�
    Uart1Sends("ATH\r\n");
    Delay1s(1);
}
