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

/* Serial baud rate */
#define BAUD_RATE 9600


/* Global string constants */
static const char csymbol[] PROGMEM = "\10C";
static const char fsymbol[] PROGMEM = "\10F";
static const char ksymbol[] PROGMEM = "K";
static const char rsymbol[] PROGMEM = "\10R";
static const char dsymbol[] PROGMEM = "\10De";
static const char nsymbol[] PROGMEM = "\10N";
static const char resymbol[] PROGMEM = "\10R\11";
static const char rosymbol[] PROGMEM = "\10R\02";
static const char tempmsg[] PROGMEM = "Temp: %d%s   ";
static const char targetmsg[] PROGMEM = "Target: %.1f%s   ";

static const char aboutcopyrightmsg[] PROGMEM = "\16 2014 ";
static const char aboutlicensemsg[] PROGMEM = "Licensed under GPLv3";
static const char canceltimermsg[] PROGMEM = "Cancelling in %d";
static const char comingsoonmsg[] PROGMEM = "Coming soon!";
static const char dooropenmsg[] PROGMEM = "Door open!";
static const char pleaseclosedoormsg[] PROGMEM = "Please close door!";
static const char tcerrormsg[] PROGMEM = "Thermocouple error!";
static const char checktcmsg[] PROGMEM = "Check thermocouple!";
static const char presstocontinuemsg[] PROGMEM = "Press \15 to continue.";
static const char reflowcancelledmsg[] PROGMEM = "Reflow cancelled!";
static const char reflowcompletemsg[] PROGMEM = "Reflow complete!";

#endif // GLOBAL_H
