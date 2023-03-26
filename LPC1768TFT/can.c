#include "can.h"

void can1Init(void)
{
    LPC_SC->PCONP |= (1 << 13); //CAN Controller 1 power/clock control bit.
    //LPC_SC->PCLKSEL0 |= (1 << 27) | (1 << 26); // PCLK_CAN1 = sysckl / 6 (11 = 3)
    LPC_PINCON->PINSEL0 |= (1 << 0) | (1 << 2); //Enable RD1 and TD1 on P0.0 and P0.1
    
	LPC_CAN1->MOD = 1; 
	LPC_CAN1->IER = 0; 
	LPC_CAN1->GSR = 0; 
    //LPC_CAN1->CMR = (1 << 1) | (1 << 2) | (1 << 3);
    LPC_CAN1->BTR = 0x25c018; // 100kbps   0x7F0009   0x1C0009
	LPC_CAN1->MOD = 0; 

    //NVIC_EnableIRQ(CAN_IRQn); 
	LPC_CAN1->IER = 0x03;
}

void can2Init(void)
{
    LPC_SC->PCONP |= (1 << 14); //CAN Controller 2 power/clock control bit.
    //LPC_SC->PCLKSEL0 |= (1 << 29) | (1 << 28); // PCLK_CAN2 = sysckl / 2
    LPC_PINCON->PINSEL4 |= (1 << 14) | (1 << 16); //Enable RD2 and TD2 on P2.7 and P2.8
    
	LPC_CAN2->MOD = 1; 
	LPC_CAN2->IER = 0; 
	LPC_CAN2->GSR = 0; 
    //LPC_CAN2->CMR = (1 << 1) | (1 << 2) | (1 << 3);    	
    LPC_CAN2->BTR = 0x25c018; // 100kbps  
	LPC_CAN2->MOD = 0;  

    NVIC_EnableIRQ(CAN_IRQn); 
	//LPC_CAN1->IER = 0x03;
    LPC_CAN2->IER = 0x03;
}

uint8_t can1Tx(canMessage* dataTx)
{
    uint32_t status = LPC_CAN1->SR;
    
	if (status & 0x0000004) //TS1 Transmit Status 1
	{
		LPC_CAN1->TFI1 = dataTx->frame & 0x000F0000; 
		LPC_CAN1->TID1 = dataTx->id; 
		LPC_CAN1->TDA1 = dataTx->dataA; 
		LPC_CAN1->TDB1 = dataTx->dataB; 
        LPC_CAN1->CMR |= (1 << 5) | (1 << 0); //STB1 Select Tx Buffer 1
        return 1;
	} //tx buf 1 
	else if (status & 0x00000400) //TS2 Transmit Status 2
	{
		LPC_CAN1->TFI2 = dataTx->frame & 0xC00F0000; 
		LPC_CAN1->TID2 = dataTx->id; 
		LPC_CAN1->TDA2 = dataTx->dataA; 
		LPC_CAN1->TDB2 = dataTx->dataB; 
        LPC_CAN1->CMR |= (1 << 6) | (1 << 0); //STB2 Select Tx Buffer 2
		return 1;
	} //tx buf 2 
	else if (status & 0x00040000) //TS3 Transmit Status 3
	{
		LPC_CAN1->TFI3 = dataTx->frame & 0xC00F0000; 
		LPC_CAN1->TID3 = dataTx->id; 
		LPC_CAN1->TDA3 = dataTx->dataA; 
		LPC_CAN1->TDB3 = dataTx->dataB; 
        LPC_CAN1->CMR |= (1 << 7) | (1 << 0); //STB3 Select Tx Buffer 3
		return 1;
	}	//tx buf 3
	else return 0; 
}

uint8_t can2Tx(canMessage* dataTx)
{
    uint32_t status = LPC_CAN2->SR;

	if (status & 0x0000004) //TS1 Transmit Status 1
	{
		LPC_CAN2->TFI1 = dataTx->frame & 0x000F0000; 
		LPC_CAN2->TID1 = dataTx->id; 
		LPC_CAN2->TDA1 = dataTx->dataA; 
		LPC_CAN2->TDB1 = dataTx->dataB; 
        LPC_CAN2->CMR |= (1 << 5) | (1 << 0); //STB1 Select Tx Buffer 1
		return 1; 
	} //tx buf 1 
	else if (status & 0x00000400) //TS2 Transmit Status 2
	{
		LPC_CAN2->TFI2 = dataTx->frame & 0xC00F0000; 
		LPC_CAN2->TID2 = dataTx->id; 
		LPC_CAN2->TDA2 = dataTx->dataA; 
		LPC_CAN2->TDB2 = dataTx->dataB; 
        LPC_CAN2->CMR |= (1 << 6) | (1 << 0); //STB2 Select Tx Buffer 2
		return 1;
	} //tx buf 2 
	else if (status & 0x00040000) //TS3 Transmit Status 3
	{
		LPC_CAN2->TFI3 = dataTx->frame & 0xC00F0000; 
		LPC_CAN2->TID3 = dataTx->id; 
		LPC_CAN2->TDA3 = dataTx->dataA; 
		LPC_CAN2->TDB3 = dataTx->dataB; 
        LPC_CAN2->CMR |= (1 << 7) | (1 << 0); //STB3 Select Tx Buffer 3
		return 1;
	}	//tx buf 3
	else return 0; 
}


void can1Rx(void)
{
    can1RxData.frame = LPC_CAN1->RFS;
    can1RxData.id = LPC_CAN1->RID;
    can1RxData.dataA = LPC_CAN1->RDA;
    can1RxData.dataB = LPC_CAN1->RDB;

    LPC_CAN1->CMR = (1 << 2);
    
    can1RxDone = 1;
    
    return; 
}

void can2Rx(void)
{
    can2RxData.frame = LPC_CAN2->RFS;
    can2RxData.id = LPC_CAN2->RID;
    can2RxData.dataA = LPC_CAN2->RDA;
    can2RxData.dataB = LPC_CAN2->RDB;

    LPC_CAN2->CMR = (1 << 2);
    
    can2RxDone = 1;
    
    return; 
}


void CAN_IRQHandler (void)
{	
    uint32_t rxStatus = LPC_CANCR->CANRxSR;
    
    uint32_t rxStatus2 = LPC_CAN1->ICR;
    uint32_t rxStatus4 = LPC_CAN2->ICR;
    
    //can1Rx();
    if (rxStatus & (1 << 8) || LPC_CAN1->RDB != 0) 
    {
        can1Rx();
    } //CAN1
    
    if (rxStatus & (1 << 9) || LPC_CAN2->RDB != 0) 
    {
        can2Rx();
    } //CAN2 

    if ( LPC_CAN1->GSR & (1 << 6 ) )
    {
        can1ErrorCnt = LPC_CAN1->GSR >> 16;
    }
    if ( LPC_CAN2->GSR & (1 << 6 ) )
    {
        can2ErrorCnt = LPC_CAN2->GSR >> 16;
    }

    return; 
}

