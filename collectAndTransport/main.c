/*** ������� ***/

#include "temperature.h"
#include "gprs.h"

void main()
{
    uint count = 0;
    while (1)
    {
        // 1. �¶����ݲɼ�
        DataTrans(Ds18b20ReadTemp());

        // 2. �¶�������ʾ
        DigDisplay();

        // ��������ִ����ϴ����ʱ 200us, 1s = 1000000us
        count++;
        if (count == 5000) // ԼΪ1s
        {
            // 3. �¶����ݴ���
            DataTransmission();
            count = 0;
        }
    }
}
