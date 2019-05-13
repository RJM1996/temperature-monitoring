/*** 程序入口 ***/

#include "temperature.h"
#include "gprs.h"

void main()
{
    uint count = 0;
    while (1)
    {
        // 1. 温度数据采集
        DataTrans(Ds18b20ReadTemp());

        // 2. 温度数据显示
        DigDisplay();

        // 上面两句执行完毕大概用时 200us, 1s = 1000000us
        count++;
        if (count == 5000) // 约为1s
        {
            // 3. 温度数据传输
            DataTransmission();
            count = 0;
        }
    }
}
