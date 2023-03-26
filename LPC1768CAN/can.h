#ifndef __CAN_H__
#define __CAN_H__

#include "LPC17xx.h"
#include "GPIO_LPC17xx.h"

typedef struct 
{   
    uint32_t frame;
    uint32_t id;
    uint32_t dataA;
    uint32_t dataB;
} canMessage;


//prototypes
void can1Init(void);
uint8_t can1Tx(canMessage* dataTx);
void can1Rx(void);

void can2Init(void);
uint8_t can2Tx(canMessage* dataTx);
void can2Rx(void);

extern canMessage can1RxData, can2RxData;
extern volatile uint8_t can1RxDone, can2RxDone;
extern uint32_t can1ErrorCnt, can2ErrorCnt;
        
//void CAN_IRQHandler (void);

#endif /* __CAN_H__ */
