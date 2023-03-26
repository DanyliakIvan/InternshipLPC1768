#include "delay.h"


void delayMs(int value)
{
    for (volatile int i = 0; i < value; i++)
    {
        for (volatile int j = 0; j < 10000; j++)
        {
            
        }
    }
}
