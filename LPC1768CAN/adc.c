#include "stdutils.h"
#include "adc.h"



void adcInit(void)
{
    LPC_SC->PCONP |= (1 << 12); //ADC power/clock control bit
    LPC_ADC->ADCR |= (1 << 21); //enable ADC
    LPC_SC->PCLKSEL0 |= (1 << 23); //enable clock
    LPC_PINCON->PINSEL1 |= (1 << 18); // enable P0.2 (standart potentiometr)
    
 
}

uint16_t adcgetValue(void)
{
    LPC_ADC->ADCR  = (LPC_ADC->ADCR  & 0xFFFFFF00) | (0x01 << 2 ); //set the 2 channel
    util_BitSet(LPC_ADC->ADCR,24); //start ADC conversion now
    while(util_GetBitStatus(LPC_ADC->ADGDR,31) == 0); //wait when ADC conversion will be done
    return ((LPC_ADC->ADGDR >> 4) & 0xfff); // (15:4 -> 11:0)   4 right-shifts  and set this value to ch
}
