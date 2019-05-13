/*** ���ݴ��� ��ش��� ***/

#include "gprs.h"
#include "utils.c"

// ע��: ���۽��յ��źŻ��Ƿ������źţ���������жϷ������
// ��ʼ�����򣨱���ʹ�ã������޷��շ���
void SerialInit()
{
    TMOD = 0x20; // ��ʱ��1 ����ģʽ2:8λ�Զ����ض�ʱ��

    // ��������ݾ����С���ò�ͬ����ֵ��ʼ������
#ifdef FOSC_12M
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

// �����ж���Ӧ�Ƿ���ȷ
unsigned char Hand(unsigned char *data_source, unsigned char *ptr)
{
    if (Hand(data_source, ptr) != NULL)
        return 1;
    else
        return 0;
}

// �����жϣ��շ���ɶ�������ó���
void SerialInterrupt() interrupt 4
{
    uchar i = 0;

    if (RI == 1) // �յ���Ϣ
    {

        RI = 0; // �����ж��ź����㣬��ʾ����������

        GsmRcv[GsmRcvCnt] = SBUF;
        // Uart1Send(tmp);
        GsmRcvCnt++;
        // �յ���������ATָ�������ATָ������0x0a 0x0d��β�ģ��������жϣ��ڽ��յĹ������Ƿ��յ�0x0a 0x0d
        if (GsmRcv[GsmRcvCnt - 2] == 0x0d && GsmRcv[GsmRcvCnt - 1] == 0x0a && GsmRcvCnt >= 2)
        {
            // һ���յ�0x0a 0x0d���ͽ����ݱ����������û����������ж�
            for (i = 0; i < GsmRcvCnt; i++)
            {
                GsmRcvAt[i] = GsmRcv[i];
                GsmRcv[i] = 0;
            }
            GsmRcvCnt = 0;
            GsmAtFlag = 1; // �յ���������atָ�ͨ�������־λ��1��������������֪��ȥ�ж���
        }
        else if (GsmRcvCnt >= 50) // ��Ϊ�ڴ����ޣ��յ���50���ַ�����û�п���0x0a 0x0d�Ļ��������¿�ʼ���հ�
        {
            GsmRcvCnt = 0;
        }
    }
}

// ���п���������char�����飬������ֹ�� \0 ��ֹͣ
// ������ATָ��ĺ���
void UartSend(uchar *str)
{
    while (*str != '\0')
    {
        SBUF = *str;
        while (!TI)
            ; //�ȴ���������źţ�TI=1������
        TI = 0;
        str++;
    }
    DelaySec(1);
}

// ���ݴ���
void DataTransmission()
{
    // 1. ���ڳ�ʼ��
    SerialInit();

    // ����led��Ϩ��
    Signal = 0;

    // 2. ����ָ��

    // (1) ��ʼ��HTTP����
    UartSend("AT+HTTPINIT\r\n");
    // (2) ������������Ϊ GPRS
    UartSend("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r\n");
    // (3) ���� APN �й��ƶ����� CMNET
    UartSend("AT+SAPBR=3,1,\"APN\",\"CMNET\"\r\n");
    // (4) ����������
    UartSend("AT+SAPBR=1,1\r\n");
    // (5) ���ýӿڵ�URL��ַ
    UartSend("AT+HTTPPARA=\"URL\",\"http://47.93.189.83/api/device/savetemperature\"\r\n");

    if (GsmAtFlag == 1 && Hand(GsmRcvAt, "OK"))
    {
        // (6) ��Ϊ������POST��������Ҫ����POST���ݵĴ�С������ʱ��
        UartSend("AT+HTTPDATA=147,10000"); // ��post���ݴ�СΪ147�ֽ� ����ʱ��Ϊ10000ms(10s)

        // (7) �������������յ�����Ϊ download ʱ���Ϳ��Կ�ʼ�������ǵ�post������
        if (GsmAtFlag == 1 && Hand(GsmRcvAt, "DOWNLOAD"))
        {
            // ����post����
            UartSend("{\"did\": \"83457508\",\"verify_code\": \"6K9F99MI\",\"temperature\":{\"hundred\" : 0,\"decade\" : 3,\"unit\" : 8,\"zeroPointOne\" : 7,\"zeroPointZeroOne\" : 5}}\r\n");
        }
    }

    // (8) �յ�����Ϊ OK ֮�󣬾Ϳ��Լ��� post ������
    if (GsmAtFlag == 1 && Hand(GsmRcvAt, "OK"))
    {
        // ����post���� get:0 post:1 head:2
        UartSend("AT+HTTPACTION=1");
        if (GsmAtFlag == 1 && Hand(GsmRcvAt, "OK"))
        {
            DelaySec(5);
            // �ж���Ӧ�Ƿ�ɹ�
            for (int i = 1; i <= 3 && GsmAtFlag == 1 && Hand(GsmRcvAt, "+HTTPACTION:1,200"); i++)
            {
                // ��Ӧ�ɹ�����ʼ��ȡ��Ӧ
                UartSend("HTTPREAD:0,52");
                if (GsmAtFlag == 1 && Hand(GsmRcvAt, "\"code\":200, \"status\":\"success\""))
                {
                    // ��ȷ��Ӧ����Ϊ��{"code":200, "status":"success", "uid":0, "bizdata":""}
                    // ʹLED����˸������ʾ��Ӧ�ɹ�
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
            // ֮��ر� HTTP ����
            UartSend("AT+HTTPTERM\r\n");
        }
    }
}
