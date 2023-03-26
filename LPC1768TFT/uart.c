#include "uart.h"


/* Function to initialize the UART0 at specifief baud rate */
void uart0Init(void)
{
    uint32_t var_RegValue_u32 = SystemCoreClock / (16 * 9600 );
    
    // Initialize UART0
    LPC_PINCON->PINSEL0 |= (1 << 4) | (1 << 6); // Enable TXD0 and RXD0 on P0.2 and P0.3
    LPC_UART0->LCR = (1 << 7); // Enable DLAB to set baud rate
    LPC_UART0->DLL = var_RegValue_u32 & 0xFFu; // Set baud rate to 9600
    LPC_UART0->DLM = (var_RegValue_u32 >> 0x08) & 0xFF;
    LPC_UART0->LCR = 3; // Disable DLAB and set 8-bit data, no parity, 1 stop bit

    /*LPC_UART0->LCR = (0x03<<0x00u) | (1<<0x07u);  */
}

/* Function to initialize the UART1 at specifief baud rate */
void uart1Init(void)
{
    uint32_t var_RegValue_u32 = SystemCoreClock / (16 * 9600 );
    
    // Initialize UART1    
    LPC_PINCON->PINSEL4 |= (1 << 1) | (1 << 3); // Enable TXD1 and RXD1 on P2.0 and P2.1
    LPC_UART1->LCR = (1 << 7); // Enable DLAB to set baud rate
    LPC_UART1->DLL = var_RegValue_u32 & 0xFFu; // Set baud rate to 9600
    LPC_UART1->DLM = (var_RegValue_u32 >> 0x08) & 0xFF;
    LPC_UART1->LCR = 3; // Disable DLAB and set 8-bit data, no parity, 1 stop bit
}

char uartReceiveChar(uint8_t selectUart)
{
    char ch = 0;
    switch (selectUart)
    {
        case 0:
        {
            while (!(LPC_UART0->LSR & (1 << 0))); // Wait until UART0 is ready to transmit
            ch = LPC_UART0->RBR; // Read a character from UART0
            break;
        }
        case 1:
        {
            while (!(LPC_UART1->LSR & (1 << 0))); // Wait until UART1 is ready to transmit
            ch = LPC_UART1->RBR; // Read a character from UART1
            break;
        }
    }
    return ch;
}

void uartTransmitChar(char ch, uint8_t selectUart)
{
    switch (selectUart)
    {
        case 0:
        {
            while (!(LPC_UART0->LSR & (1 << 5))); // Wait until UART0 is ready to transmit
            LPC_UART0->THR = ch; // Transmit the character to UART0
            break;
        }
        case 1:
        {
            while (!(LPC_UART1->LSR & (1 << 5))); // Wait until UART1 is ready to transmit
            LPC_UART1->THR = ch; // Transmit the character to UART1
            break;
        }
    }
}
