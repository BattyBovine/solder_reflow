#ifndef GLOBAL_H
#define GLOBAL_H

#include "lcd_i2c.h"

/* CPU prescaler macros */
#define CPU_PRESCALE(n)				(CLKPR = 0x80, CLKPR = (n))
#define CPU_16M								0x00
#define CPU_8M								0x01
#define CPU_4M								0x02
#define CPU_2M								0x03
#define CPU_1M								0x04
#define CPU_500k							0x05
#define CPU_250k							0x06
#define CPU_125k							0x07
#define CPU_62k								0x08

#if F_CPU == 16000000
	#define CPU_DEFAULT()				CPU_PRESCALE(CPU_16M)
#elif F_CPU == 8000000
	#define CPU_DEFAULT()				CPU_PRESCALE(CPU_8M)
#elif F_CPU == 4000000
	#define CPU_DEFAULT()				CPU_PRESCALE(CPU_4M)
#elif F_CPU == 2000000
	#define CPU_DEFAULT()				CPU_PRESCALE(CPU_2M)
#elif F_CPU == 1000000
	#define CPU_DEFAULT()				CPU_PRESCALE(CPU_1M)
#elif F_CPU == 500000
	#define CPU_DEFAULT()				CPU_PRESCALE(CPU_500k)
#elif F_CPU == 250000
	#define CPU_DEFAULT()				CPU_PRESCALE(CPU_250k)
#elif F_CPU == 125000
	#define CPU_DEFAULT()				CPU_PRESCALE(CPU_125k)
#elif F_CPU == 62000
	#define CPU_DEFAULT()				CPU_PRESCALE(CPU_62k)
#else
	#define CPU_DEFAULT()				CPU_PRESCALE(CPU_1M)
#endif

// Serial baud rate
#define BAUD_RATE 9600

#endif // GLOBAL_H
