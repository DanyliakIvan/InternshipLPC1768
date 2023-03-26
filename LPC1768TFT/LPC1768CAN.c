#include "LPC17xx.h"
#include "GPIO_LPC17xx.h"
#include "can.h"
#include "delay.h"
#include "uart.h"
#include "adc.h"
#include "display.h"

int main(void)
{
    SystemInit();

    /* TEST TFT DISPLAY */
    
                                                         
    GLCD_Init();
    GLCD_Display_On();
	GLCD_Clear(DarkCyan);
    GLCD_SetBackColor(DarkCyan); //Black
    GLCD_SetTextColor(Navy); //White
    GLCD_DisplayString(1, 0, "  LAB 5 KI-46 ");
    GLCD_DisplayString(2, 0, " Danyliak Ivan" );
    while(1)
    {
    }    
    
    /* TEST TFT DISPLAY */
}
