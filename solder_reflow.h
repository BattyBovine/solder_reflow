#ifndef SOLDER_REFLOW_H
#define SOLDER_REFLOW_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <util/delay.h>

#include "globals.h"
#include "lcd_menu.h"

#define PROGRAM_NAME	"Solder Reflow"
#define PROGRAM_VER		"1.0"
#define PROGRAM_DEV		"BattyBovine"

/* Interrupt macros */
#define HEAT_ENABLE						(PORTD |= (1<<4))
#define HEAT_DISABLE					(PORTD &= ~(1<<4))

#define INPUT_ENABLE					(PCICR |= (1<<PCIE2))
#define INPUT_DISABLE					(PCICR &= ~(1<<PCIE2))

#define BUZZER_ENABLE					(TCCR2A |= (1<<COM2B1))
#define BUZZER_DISABLE				(TCCR2A &= ~(1<<COM2B1))
#define BUZZER_TOGGLE					(TCCR2A ^= (1<<COM2B1))
#define BUZZER_ENABLED				(TCCR2A &= (1<<COM2B1))

#define ADC_ENABLE						(ADCSRA |= ((1<<ADSC)|(1<<ADIE)))
#define ADC_DISABLE						(ADCSRA &= ~((1<<ADSC)|(1<<ADIE)))

#define TEMPREP_BUZZ_ENABLE		(TIMSK1 |= (1<<OCIE1A))
#define TEMPREP_BUZZ_DISABLE	(TIMSK1 &= ~(1<<OCIE1A))

#define DEBOUNCE_ENABLE				(TIMSK0 |= (1<<OCIE0A))
#define DEBOUNCE_DISABLE			(TIMSK0 &= ~(1<<OCIE0A))
#define DEBOUNCE_ENABLED			(TIMSK0&(1<<OCIE0A))

#define CANCEL_TIMER_ENABLE		(TIMSK0 |= (1<<TOIE0))
#define CANCEL_TIMER_DISABLE	(TIMSK0 &= ~(1<<TOIE0))
#define CANCEL_TIMER_ENABLED	(TIMSK0&(1<<TOIE0))



/* Interrupt flags */
volatile uint8_t isrflags = 0x00;
#define ISRF(f)								(isrflags&(1<<ISRF_##f))
#define ISRF_SET(f)						(isrflags|=(1<<ISRF_##f))
#define ISRF_BTN()						(isrflags&((1<<ISRF_ENTER)|(1<<ISRF_NEXT)|\
															(1<<ISRF_PREV)|(1<<ISRF_CANCEL)))
#define ISRF_CLR(f)						(isrflags&=~(1<<ISRF_##f))
#define ISRF_CLRBTN()					(isrflags&=~((1<<ISRF_ENTER)|(1<<ISRF_NEXT)|\
															(1<<ISRF_PREV)|(1<<ISRF_CANCEL)))
#define ISRF_CLRALL()					(isrflags=0x00)
#define ISRF_DOOR_OPEN				0
#define ISRF_DOOR_CLOSED			1
#define	ISRF_PROFILE_START		2
#define	ISRF_REPORT_TEMP			3
#define	ISRF_ENTER						4
#define	ISRF_PREV							5
#define	ISRF_NEXT							6
#define	ISRF_CANCEL						7

/* Program status flags */
volatile uint16_t statusflags = 0x0000;
#define STAT(f)								(statusflags&(1<<STAT_##f))
#define STAT_ANY()						(statusflags)
#define STAT_PFSTAGE()				(statusflags&(\
																(1<<STAT_PROFILE_PREHEAT)|\
																(1<<STAT_PROFILE_SOAK)|\
																(1<<STAT_PROFILE_RAMPUP)|\
																(1<<STAT_PROFILE_PEAK)|\
																(1<<STAT_PROFILE_RAMPDOWN)))
#define STAT_PFEND()					(statusflags&(\
																(1<<STAT_PROFILE_COMPLETE)|\
																(1<<STAT_PROFILE_CANCEL)|\
																(1<<STAT_PROFILE_NO_TC)))
#define STAT_SET(f)						(statusflags|=(1<<STAT_##f))
#define STAT_MENU()						(statusflags&((1<<STAT_MAIN_MENU)|\
															(1<<STAT_SETTINGS_MENU)|(1<<STAT_UNITS_MENU)))
#define STAT_CLR(f)						(statusflags&=~(1<<STAT_##f))
#define STAT_CLRPFSTAGE()			(statusflags&=~(\
																(1<<STAT_PROFILE_PREHEAT)|\
																(1<<STAT_PROFILE_SOAK)|\
																(1<<STAT_PROFILE_RAMPUP)|\
																(1<<STAT_PROFILE_PEAK)|\
																(1<<STAT_PROFILE_RAMPDOWN)))
#define STAT_CLRPFEND()				(statusflags&=~(\
																(1<<STAT_PROFILE_COMPLETE)|\
																(1<<STAT_PROFILE_CANCEL)|\
																(1<<STAT_PROFILE_NO_TC)))
#define STAT_CLRMENU()				(statusflags&=~((1<<STAT_MAIN_MENU)|\
															(1<<STAT_SETTINGS_MENU)|(1<<STAT_UNITS_MENU)))
#define STAT_CLRALL()					(statusflags=0x0000)
#define STAT_DOOR_OPEN				0
#define STAT_TC_ERROR					1
#define STAT_CANCEL						2
#define STAT_PROFILE_START		3
#define STAT_PROFILE_RUNNING	4
#define STAT_PROFILE_PREHEAT	5
#define STAT_PROFILE_SOAK			6
#define STAT_PROFILE_RAMPUP		7
#define STAT_PROFILE_PEAK			8
#define STAT_PROFILE_RAMPDOWN	9
#define STAT_PROFILE_COMPLETE	10
#define STAT_PROFILE_CANCEL		11
#define STAT_ABOUT						12
#define STAT_COMING_SOON			13
                              
/* Menu status flags */
volatile uint8_t menuflag = 0x00;
#define MENU(f)								(menuflag==MENU_##f)
#define MENU_ANY()						(menuflag)
#define MENU_SET(f)						(menuflag=MENU_##f)
#define MENU_CLR()						(menuflag=0x00)
#define MENU_MAIN							1
#define MENU_PROFILES					2
#define MENU_SETTINGS					3
#define MENU_UNITS						4
#define MENU_SOUNDS						5

/* EEPROM flags */
#define EEPROM_START_ADDR		(uint8_t*)0x00
volatile uint8_t eepromflags = 0x00;
#define EEPROM(f)						(eepromflags&EEPROM_##f)
#define EEPROM_UNINIT()			(eepromflags==0xFF)
#define EEPROM_LOAD()				(eepromflags=eeprom_read_byte(EEPROM_START_ADDR))
#define EEPROM_SAVE()				(eeprom_update_byte(EEPROM_START_ADDR,eepromflags))
#define EEPROM_SET(f)				{(eepromflags|=EEPROM_##f);EEPROM_SAVE();}
#define EEPROM_SETVAL(f)		{(eepromflags|=(f));EEPROM_SAVE();}
#define EEPROM_CLR(f)				{(eepromflags&=~EEPROM_##f);EEPROM_SAVE();}
#define EEPROM_CLRALL()			{(eepromflags=0x00);EEPROM_SAVE();}
#define EEPROM_TEMPERATURE	(0b0000111)
#define EEPROM_CELSIUS			(0b0000000)
#define EEPROM_FAHRENHEIT		(0b0000001)
#define EEPROM_KELVIN				(0b0000010)
#define EEPROM_RANKINE			(0b0000011)
#define EEPROM_DELISLE			(0b0000100)
#define EEPROM_NEWTON				(0b0000101)
#define EEPROM_REAUMUR			(0b0000110)
#define EEPROM_ROMER				(0b0000111)
#define EEPROM_BUZZER				(0b0011000)
#define EEPROM_BUZZER_OFF		(0b0000000)
#define EEPROM_BUZZER_LOW		(0b0001000)
#define EEPROM_BUZZER_MED		(0b0010000)
#define EEPROM_BUZZER_HIGH	(0b0011000)

// Button states for PORTD
volatile uint8_t pd_prev = 0xFF;



#define ctof(x)		(x*9/5+32)				// Celsius to Fahrenheit
#define ctok(x)		(x+273)						// Celsius to Kelvin
#define ctor(x)		(ctof(x)+459.67)	// Celsius to Rankine
#define ctod(x)		((100-x)*1.5)			// Celsius to Delisle
#define cton(x)		(x*0.333)					// Celsius to Newton
#define ctore(x)	(x*0.8)						// Celsius to Reaumur
#define ctoro(x)	(x*(0.525)+7.5)		// Celsius to Romer

#define BUZZER_TIME_MENU					100
#define BUZZER_TIME_CANCEL				150
#define BUZZER_TIME_COMPLETE			500
#define BUZZER_TIME_DOOR_TC_ERROR	1000



static inline void show_temp_report(void);
static inline void show_menu(void);
static inline void show_thermocouple_error(void);
static inline void show_door_open(void);
static inline void show_cancel_timer(void);
static inline void show_profile_state(void);
static inline void show_profile_completion(void);
static inline void show_about(void);
static inline void show_coming_soon(void);

static inline void start_buzzer(uint8_t cnt, uint16_t ms);

static inline void reset_profile_state(void);
static inline void reset_cancel_timer(void);
static inline void reset_all(void);



#define NUM_AVERAGE 16
static volatile uint16_t adc_average[NUM_AVERAGE];
static volatile uint8_t average_count = 0;

static volatile uint16_t temperature = 0;
static volatile double targettemp = 0;
static volatile uint32_t time_ms = 0;
static volatile uint8_t ctovf_count = 0;
static volatile uint8_t debounce_count = 0;
static volatile uint8_t buzzer_count = 0;
static volatile uint8_t buzzer_time = 0;

#endif // SOLDER_REFLOW_H
