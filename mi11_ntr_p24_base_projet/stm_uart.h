/*
 * stm_uart.h
 *
 *  Created on: 1 avr. 2021
 *      Author: bonnetst
 */

#ifndef STM_UART_H_
#define STM_UART_H_

#include <stdint.h>

typedef struct _tagUSART {
    volatile uint32_t cr1;
    volatile uint32_t cr2;
    volatile uint32_t cr3;
    volatile uint32_t brr;
    volatile uint32_t gtpr;
    volatile uint32_t rtor;
    volatile uint32_t rqr;
    volatile uint32_t isr;
    volatile uint32_t icr;
    volatile uint32_t rdr;
    volatile uint32_t tdr;
    volatile uint32_t presc;
} usart_t;

#define USART1 ((usart_t *) 0x40011000)

void usart_init(uint32_t baudrate);
void usart_write(char c);
int usart_read(void);

#endif /* STM_UART_H_ */
