/************************************************************
����˵����
���������к����GPRSģ���ҵ��������źţ��ͻ��������������Ϣ
1.��51��Ƭ���Ĵ���1���ӵ�GSM��232�ӿ�
2.�ҵ�������ǰ���#define��������˵���޸ĺ��Լ��ĵ�Ƭ������.
3.���س���
4.����ģ�飬�ȴ��źŵ���˸���������ģ����ֻ����������������������յ�ģ�鷢������Ϣ
*************************************************************/
#include <reg52.h>
#include <string.h>

#define uchar unsigned char #define uint unsigned int

//���������51��Ƭ���ľ����С
#define FOSC_110592M
//#define FOSC_12M

//�������ڱ��浥Ƭ���յ�ģ�鷢����ATָ�ͨ����Щָ�Ƭ�������ж�ģ���״̬
uchar GsmRcv[50] = {0};
uchar GsmRcvAt[50] = {0};
uchar GsmRcvCnt = 0;
uchar GsmAtFlag = 0;

// ��ʼ�����򣨱���ʹ�ã������޷��շ������γ��򽫻�ʹ�ö�ʱ��1
void SerialInit() //��ʼ�����򣨱���ʹ�ã������޷��շ���
{
	TMOD = 0x20; //��ʱ��1����ģʽ2:8λ�Զ����ض�ʱ��

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

/*
 * ����ͨѶ�ж�
 * ���۽��յ��źŻ��Ƿ������źţ����������жϷ������  
 */
void Serial_interrupt() interrupt 4
{
	uchar i = 0;

	if (RI == 1) // �յ���Ϣ
	{

		RI = 0; // �����ж��ź����㣬��ʾ����������

		GsmRcv[GsmRcvCnt] = SBUF;
		// Uart1Send(tmp);
		GsmRcvCnt++;
		// �յ���������ATָ�������ATָ������0x0a 0x0d��β�ġ��������жϣ��ڽ��յĹ������Ƿ��յ�0x0a 0x0d
		if (GsmRcv[GsmRcvCnt - 2] == 0x0d && GsmRcv[GsmRcvCnt - 1] == 0x0a && GsmRcvCnt >= 2)
		{
			// һ���յ�0x0a 0x0d���ͽ����ݱ����������û����������жϡ�
			for (i = 0; i < GsmRcvCnt; i++)
			{
				GsmRcvAt[i] = GsmRcv[i];
				GsmRcv[i] = 0;
			}
			GsmRcvCnt = 0;
			GsmAtFlag = 1; // �յ���������atָ�ͨ�������־λ��1��������������֪��ȥ�ж��ˡ�
		}
		else if (GsmRcvCnt >= 50) // ��Ϊ�ڴ����ޣ��յ���50���ַ�����û�п���0x0a 0x0d�Ļ��������¿�ʼ���հɡ�
		{
			GsmRcvCnt = 0;
		}
	}
}

void Uart1Send(uchar c)
{
	SBUF = c;
	while (!TI)
		; // �ȴ���������źţ�TI=1������
	TI = 0;
}

// ���п���������char�����飬������ֹ��/0��ֹͣ
void Uart1Sends(uchar *str)
{
	while (*str != '\0')
	{
		SBUF = *str;
		while (!TI)
		{
			; // �ȴ���������źţ�TI=1������
		}

		TI = 0;
		str++;
	}
	// ��ʱ1s
	DelaySec(1);
}

// ��ʱ���������1s��������ʱ��Ļ���׼
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

// sim900�ĳ�ʼ������
void sim900Init()
{
	uchar i = 0;
	// 1.���ڳ�ʼ��
	SerialInit();

	// 2.�ж��Ƿ��������
	GsmAtFlag = 0;
	while (GsmAtFlag == 0)
	{
		Uart1Sends("ATI\r\n"); // ����sim900ģ��Ĳ�����
		DelaySec(1);		   // ��ʱ1��
	}
	GsmAtFlag = 0;

	// 3.����ź�
	while (1)
	{
		Uart1Sends("AT+COPS?\r\n");
		DelaySec(2); // ��ʱ2��
		while (GsmAtFlag == 0)
		{
			;
		}
		// strstr(str1, str2) �����ж� str2 �Ƿ�Ϊ str1 ���Ӵ�
		if (strstr(GsmRcvAt, "CHINA")) // ����Ƿ��յ� CHINA MOBILE ��������Ϣ������յ�֤����������������
		{
			break;
		}
	}
}

// ���ݴ���
void dataTransmission()
{
	// 4.����ָ�����HTTP����

	// (1) ��ʼ��HTTP����
	Uart1Sends("AT+HTTPINIT\r\n");

	// (2) ������������Ϊ GPRS
	Uart1Sends("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r\n");

	// (3) ���� APN �й��ƶ����� CMNET
	Uart1Sends("AT+SAPBR=3,1,\"APN\",\"CMNET\"\r\n");

	// ����������
	Uart1Sends("AT+SAPBR=1,1\r\n");

	// (4) ���ýӿڵ�URL��ַ
	Uart1Sends("AT+HTTPPARA=\"URL\",\"http://47.93.189.83/api/device/savetemperature\"\r\n");

	if (GsmAtFlag == 1 && strstr(GsmRcvAt, "OK"))
	{
		// (5) ��Ϊ������POST��������Ҫ����POST���ݵĴ�С������ʱ��
		Uart1Sends("AT+HTTPDATA=147,10000"); // ��post���ݴ�СΪ147�ֽ� ����ʱ��Ϊ10000ms(10s)

		// (6) �������������յ�����Ϊ download ʱ���Ϳ��Կ�ʼ�������ǵ�post������
		if (GsmAtFlag == 1 && strstr(GsmRcvAt, "DOWNLOAD"))
		{
			// ����post����
			Uart1Sends("{\"did\": \"83457508\",\"verify_code\": \"6K9F99MI\",\"temperature\":{\"hundred\" : 0,\"decade\" : 3,\"unit\" : 8,\"zeroPointOne\" : 7,\"zeroPointZeroOne\" : 5}}\r\n");
		}
	}

	// (7) �յ�����Ϊ OK ֮�󣬾Ϳ��Լ��� post ������
	if (GsmAtFlag == 1 && strstr(GsmRcvAt, "OK"))
	{
		// ����post���� get:0 post:1 head:2
		Uart1Sends("AT+HTTPACTION=1");
		if (GsmAtFlag == 1 && strstr(GsmRcvAt, "OK"))
		{
			DelaySec(5);
			// �ж���Ӧ�Ƿ�ɹ�
			for (int i = 1; i <= 3 && GsmAtFlag == 1 && strstr(GsmRcvAt, "+HTTPACTION:1,200,52"); i++)
			{
				// ��Ӧ�ɹ�����ʼ��ȡ��Ӧ
				Uart1Sends("HTTPREAD:0,52");
				if (GsmAtFlag == 1 && RcvIsSuccess(GsmRcvAt))
				{
					// ��ȷ��Ӧ����Ϊ��{"code":200, "status":"success", "uid":0, "bizdata":""}
				}
			}
			// ֮��ر� HTTP ����
			Uart1Sends("AT+HTTPTERM\r\n");
		}
	}
}
