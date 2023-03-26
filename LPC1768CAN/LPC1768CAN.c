#include "LPC17xx.h"
#include "GPIO_LPC17xx.h"
#include "can.h"
#include "delay.h"
#include "uart.h"
#include "adc.h"


canMessage can1RxData, can2RxData;
volatile uint8_t can1RxDone = 0, can2RxDone = 0;
uint32_t can1ErrorCnt = 0, can2ErrorCnt = 0;

int main(void)
{
    SystemInit();
    
    can1Init();
    can2Init();
    
    LPC_GPIO2->FIODIR = 0x0000007C;  //reset 1-3 leds
    LPC_GPIO1->FIODIR = 0xb0000000;  //reset 4-8 leds


    canMessage can1TxData;
    canMessage can2TxData;
    
    can1TxData.frame = 0x00080000; 		
    can1TxData.id = 0x100; 
    can1TxData.dataA = 0x55AA55AA;
    can1TxData.dataB = 0xAA55AA55;
    
    
    can2TxData.frame = 0x00080000; 		
    can2TxData.id = 0x100; 
    can2TxData.dataA = 0x55AA55AA;
    can2TxData.dataB = 0xAA55AA55;

    
    can1RxData.frame = 0x0;
    can1RxData.id = 0x0;
    can1RxData.dataA = 0x0;
    can1RxData.dataB = 0x0;
    
    can2RxData.frame = 0x0;
    can2RxData.id = 0x0;
    can2RxData.dataA = 0x0;
    can2RxData.dataB = 0x0;
    
    while(1)
    {   
        if (!GPIO_PinRead(2, 10))
        {
            while (!(LPC_CAN1->GSR & (1 << 3))){}
            if (!can1Tx(&can1TxData))
            {
                continue;
            }
            
            GPIO_PinWrite(2, 6, 1);
        }
        
        GPIO_PinWrite(1, 28, 1);
        
        
        if (can2RxDone)
        {
            GPIO_PinWrite(1, 29, 1);
            //delayMs(100);
		
            if (can1TxData.id == can2RxData.id && can1TxData.dataA == can2RxData.dataA && can1TxData.dataB == can2RxData.dataB)
            {
                GPIO_PinWrite(2, 4, 1);
            }
            
            if (can1TxData.id != can2RxData.id || can1TxData.dataA != can2RxData.dataA || can1TxData.dataB != can2RxData.dataB)
            {
                GPIO_PinWrite(2, 5, 1);
            }
            
            can1RxData.frame = 0;
            can1RxData.id = 0;
            can1RxData.dataA = 0;
            can1RxData.dataB = 0;
            
            can2RxData.frame = 0;
            can2RxData.id = 0;
            can2RxData.dataA = 0;
            can2RxData.dataB = 0;
            
            can1RxDone = 0;
            can2RxDone = 0;
            
            delayMs(1000);
            
            GPIO_PinWrite(1, 28, 0);
            GPIO_PinWrite(1, 29, 0);
            GPIO_PinWrite(1, 31, 0);
            GPIO_PinWrite(2, 2, 0);
            GPIO_PinWrite(2, 3, 0);
            GPIO_PinWrite(2, 4, 0);
            GPIO_PinWrite(2, 5, 0);
            GPIO_PinWrite(2, 6, 0);
        }
        
        if (can1ErrorCnt != 0 || can1ErrorCnt != 0)
             GPIO_PinWrite(2, 2, 1);
        
    }
}
