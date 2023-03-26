#ifndef __UART_H__
#define __UART_H__

#include <lpc17xx.h>

//prototypes
void uart0Init(void);
void uart1Init(void);
void uartTransmitChar(char ch, uint8_t selectUart);
char uartReceiveChar(uint8_t selectUart);


#endif /* __UART_H__ */