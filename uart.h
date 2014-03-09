#ifndef _uart_included_h_
#define _uart_included_h_

#include <stdint.h>
#include <avr/pgmspace.h>

void uart_print_PM(const char *str, int p, int nl);
#define uart_print(s)				uart_print_PM(PSTR(s),1,0)
#define uart_printmem(s)		uart_print_PM(s,0,0)
#define uart_println(s)			uart_print_PM(PSTR(s),1,1)
#define uart_printmemln(s)		uart_print_PM(s,0,1)

void uart_init(uint32_t baud);
void uart_putchar(uint8_t c);
uint8_t uart_getchar(void);
uint8_t uart_available(void);

#endif
