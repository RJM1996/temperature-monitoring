/*** ������ ***/

#define uint unsigned int

// ��ʱ��������С���ȴ����1s
void Delay1s(int sec)
{
    uint i, j = 0;
    for (i = 0; i < sec; i++)
    {
        for (j = 0; j < 65535; j++)
        {
            ;
        }
    }
}

// ��ʱ���������1ms
void Delay1ms(uint y)
{
    uint x;
    for (; y > 0; y--)
    {
        for (x = 110; x > 0; x--)
            ;
    }
}
