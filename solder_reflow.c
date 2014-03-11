#include <string.h>

#include "solder_reflow.h"



// The temperature/time profile as {secs, temp}
// This profile is linearly interpolated to get the required temperature at any time.
#define PROFILE_LENGTH 6
#define PROFILE_DATLEN 2
uint16_t profile_pb[PROFILE_LENGTH][PROFILE_DATLEN] PROGMEM =
	{ {0, 20}, {60, 120}, {150, 130}, {220, 185}, {240, 195}, {300, 20} };
uint16_t profile_rohs[PROFILE_LENGTH][PROFILE_DATLEN] PROGMEM =
	{ {0, 20}, {60, 180}, {150, 190}, {220, 220}, {240, 230}, {300, 20} };
uint16_t *activeprofile;



int main(void)
{
	// Scale the processor to the default speed
	CPU_DEFAULT();
	
	// Reset all pins
	DDRB = DDRC = DDRD = 0xFF;
	
	// Configure pin D2 (door switch) as input with pull-ups
	DDRD &= ~(1<<2);
	PORTD |= (1<<2);
	
	// Configure pin D3 (piezo alarm) as output (off by default)
	DDRD |= (1<<3);
	PORTD &= ~(1<<3);
	
	// Configure pin D4 (heating elements) as output
	DDRD |= (1<<4);
	PORTD &= ~(1<<4);
	
	// Configure pin D5-7 (rotary encoder and button) as input with pull-ups
	DDRD &= ~((1<<7)|(1<<6)|(1<<5));
	PORTD |= ((1<<7)|(1<<6)|(1<<5));
	
	// Initialise the LCD display
	lcd_init();
	
	// Enable pin change interrupts for door switch and rotary encoder
	PCMSK2 |= ((1<<PCINT7)|(1<<PCINT6)|	// Enable interrupt on D2 and D5-7
						(1<<PCINT5)|(1<<PCINT2));
	INPUT_ENABLE;
	
	// Configure ADC for temperature readings
	ADMUX &= ~((1<<MUX3)|(1<<MUX2)|			// Clear MUX
		(1<<MUX1)|(1<<MUX0));
	ADMUX |= (1<<REFS0);								// Internal Vcc as reference
	ADCSRB &= ~((1<<ADTS2)|(1<<ADTS1)|	// Free-running Mode
		(1<<ADTS0));
	ADCSRA |= ((1<<ADPS2)|(1<<ADPS1)|		// Clock/128
		(1<<ADPS0));
	ADCSRA |= ((1<<ADEN)|(1<<ADATE));		// Enable ADC, auto-trigger
	
	// Configure timer interrupt for debounce and cancel delay
	TCCR0A |= (1<<WGM01);								// CTC
	TCCR0B |= ((1<<CS02)|(1<<CS00));		// Clock/128
	OCR0A = 0xFF;												// 0.00204s at 16MHz / 128
	
	// Configure timer interrupt for the temperature reporter and piezo buzzer
	TCCR1B |= (1<<WGM12);								// CTC mode (clear timer on compare match)
	TCCR1B |= (1<<CS12);								// Clock/256
	OCR1A = 3125;												// 0.05s at 16MHz / 256
	
	// Configure PWM for the piezo buzzer
	TCCR2A |= ((1<<WGM21)|(1<<WGM20)|(1<<COM2B1));
	TCCR2B |= ((1<<CS22)|(1<<CS21)|(1<<CS20));
	OCR2B = 0x2B;
	BUZZER_DISABLE;
	
	// Load settings from EEPROM and initialise if necessary
	EEPROM_LOAD();
	if(EEPROM_UNINIT())	EEPROM_CLRALL();
	
	// Enable interrupts
	sei();
	
	lcd_write_cgram_defaults();
	
	// Check if door switch is high (door is open)
	if(PIND&(1<<2)) {
		STAT_SET(DOOR_OPEN);
	// If not, reset to a known state
	} else {
		reset_all();
	}
	
	while(1) {
		
		if(STAT(DOOR_OPEN)|STAT(TC_ERROR)) {
			if(STAT(TC_ERROR)) {
				show_thermocouple_error();
				while(STAT(TC_ERROR)){};
			} else if(STAT(DOOR_OPEN)) {
				show_door_open();
				while(STAT(DOOR_OPEN)){};
			}
			if(STAT(DOOR_OPEN)|STAT(TC_ERROR)) continue;
			reset_all();
		} else {
			/* If there is an ISR flag, check which ones are set */
			if(isrflags) {
				
				/* If we need to report the temperature */
				if(ISRF(REPORT_TEMP)) {
					show_temp_report();
				}
				
				/* If we need to respond to button presses */
				if(ISRF_BTN() && !DEBOUNCE_ENABLED) {
				
					if(ISRF(ENTER)) {				// Enter button
						ISRF_CLR(CANCEL);			// Ignore cancel
						STAT_CLR(CANCEL);
						if(MENU_ANY()) {
							if(MENU(MAIN)) {
								uint8_t sel = menu_selected();
								switch(sel) {
									case 0:									// Leaded Profile
									case 1:									// RoHS Profile
										if(activeprofile) free(activeprofile);
										activeprofile = malloc(sizeof(uint16_t)*PROFILE_LENGTH*PROFILE_DATLEN);
										if(activeprofile) {
											MENU_CLR();
											memcpy_P(activeprofile,(sel==1?profile_rohs:profile_pb),sizeof(uint16_t)*PROFILE_LENGTH*PROFILE_DATLEN);
											TEMPREP_BUZZ_ENABLE;
											ADC_ENABLE;
											STAT_SET(PROFILE_RUNNING);
										}
										break;
									case 2:									// Settings Menu
										MENU_SET(SETTINGS);
										break;
									case 3:									// About Software
										MENU_CLR();
										STAT_SET(ABOUT);
										break;
									default:
										MENU_CLR();
										STAT_SET(COMING_SOON);
								}
							} else if(MENU(SETTINGS)) {
								switch(menu_selected()) {
									case 0:
										MENU_SET(MAIN);
										break;
									case 1:
										MENU_SET(UNITS);
										break;
								}
							} else if(MENU(UNITS)) {
								EEPROM_CLR(TEMPERATURE);	// Celsius
								EEPROM_SETVAL(menu_selected());
								MENU_SET(SETTINGS);
							}
						} else if(STAT(PROFILE_COMPLETE) ||
											STAT(PROFILE_CANCEL) ||
											STAT(TC_ERROR)) {
							reset_all();
						} else if(STAT(ABOUT)) {
							MENU_SET(MAIN);
							STAT_CLR(ABOUT);
						} else if(STAT(COMING_SOON)) {
							MENU_SET(MAIN);
							STAT_CLR(COMING_SOON);
						}
					}
					
					if(ISRF(CANCEL)) {				// Cancel button
						STAT_SET(CANCEL);
					}
					
					if(ISRF(NEXT)) {					// Next button
						if(MENU_ANY())
							menu_next();
					}
					
					if(ISRF(PREV)) {					// Previous button
						if(MENU_ANY())
							menu_prev();
					}
					
					ISRF_CLRBTN();
				}
				
			}
			
			if(MENU_ANY()) {
				show_menu();
			} else {
				menu_uninit();
				
				if(STAT(PROFILE_COMPLETE) ||
					 STAT(PROFILE_CANCEL) ||
					 STAT(TC_ERROR)) {
					show_profile_completion();
				}
				if(STAT(PROFILE_RUNNING)) {
					if(STAT(CANCEL))	show_cancel_timer();
					else							reset_cancel_timer();
				}
				if(STAT(ABOUT)) {
					show_about();
				}
				if(STAT(COMING_SOON)) {
					show_coming_soon();
				}
			}
		}
	}
}



static inline void show_temp_report(void)
{
	/* Display current profile step as necessary */
	if(STAT(PROFILE_RUNNING) && !STAT(PROFILE_CANCEL)) {
		show_profile_state();
	}
	char buf[LCD_DISP_LENGTH];
	uint16_t convertedtemp;
	double convertedtarget;
	char tempsymbol[4];
	switch(EEPROM(TEMPERATURE)) {
		case EEPROM_FAHRENHEIT:
			convertedtemp = ctof(temperature);
			convertedtarget = ctof(targettemp);
			strcpy_P(tempsymbol, fsymbol);
			break;
		case EEPROM_KELVIN:
			convertedtemp = ctok(temperature);
			convertedtarget = ctok(targettemp);
			strcpy_P(tempsymbol, ksymbol);
			break;
		case EEPROM_RANKINE:
			convertedtemp = ctor(temperature);
			convertedtarget = ctor(targettemp);
			strcpy_P(tempsymbol, rsymbol);
			break;
		case EEPROM_DELISLE:
			convertedtemp = ctod(temperature);
			convertedtarget = ctod(targettemp);
			strcpy_P(tempsymbol, dsymbol);
			break;
		case EEPROM_NEWTON:
			convertedtemp = cton(temperature);
			convertedtarget = cton(targettemp);
			strcpy_P(tempsymbol, nsymbol);
			break;
		case EEPROM_REAUMUR:
			convertedtemp = ctore(temperature);
			convertedtarget = ctore(targettemp);
			strcpy_P(tempsymbol, resymbol);
			break;
		case EEPROM_ROMER:
			convertedtemp = ctoro(temperature);
			convertedtarget = ctoro(targettemp);
			strcpy_P(tempsymbol, rosymbol);
			break;
		default:	// Celsius
			convertedtemp = temperature;
			convertedtarget = targettemp;
			strcpy_P(tempsymbol, csymbol);
	}
	sprintf_P(buf, tempmsg, convertedtemp, tempsymbol);
	lcd_set_cursor(3,3);
	lcd_print(buf);
	sprintf_P(buf, targetmsg, convertedtarget, tempsymbol);
	lcd_set_cursor(4,1);
	lcd_print(buf);
	
	ISRF_CLR(REPORT_TEMP);
}

static inline void show_menu(void)
{
	switch(MENU_ANY()) {
		case MENU_MAIN:
			menu_init(main);
			break;
		case MENU_SETTINGS:
			menu_init(settings);
			break;
		case MENU_UNITS:
			menu_init(units);
			break;
	}
}

static inline void show_thermocouple_error(void)
{
	lcd_clrscr();
	lcd_set_cursor(2,1);
	lcd_print_p(tcerrormsg);
	lcd_set_cursor(3,1);
	lcd_print_p(checktcmsg);
}

static inline void show_door_open(void)
{
	lcd_clrscr();
	lcd_set_cursor(2,6);
	lcd_print_p(dooropenmsg);
	lcd_set_cursor(3,2);
	lcd_print_p(pleaseclosedoormsg);
}

static inline void show_cancel_timer(void)
{
	// if(ctovf_count >= 155) {	// 155 = 2.5s
	if(ctovf_count >= 78) {	// 78 = 1.25s
		STAT_SET(PROFILE_CANCEL);
	} else {
		CANCEL_TIMER_ENABLE;
		uint8_t n = 0;
		// if(ctovf_count >= 93)		// 93 = 1.5s
		if(ctovf_count >= 47)		// 47 = 0.75s
			n = 1;
		// else if(ctovf_count >= 31)	// 31 = 0.5s
		else if(ctovf_count >= 16)	// 16 = 0.25s
			n = 2;
		if(n) {
			char buf[LCD_DISP_LENGTH];
			lcd_set_cursor(2,3);
			sprintf_P(buf, canceltimermsg, n);
			lcd_print(buf);
		}
	}
}

static inline void show_profile_state(void)
{
	uint16_t time_sec = time_ms/1000;
	uint8_t i = PROFILE_LENGTH;
	if(time_sec <= *activeprofile && !STAT(PROFILE_PREHEAT)) {
		lcd_clrline(1);
		lcd_set_cursor(1,7);
		lcd_print_P("Preheat");
		STAT_SET(PROFILE_PREHEAT);
	} else {
		while(--i) {
			if(time_sec <= *(activeprofile+(i*PROFILE_DATLEN)) &&
				 time_sec > *(activeprofile+((i-1)*PROFILE_DATLEN))) {
				switch(i) {
					case 2:
						if(!STAT(PROFILE_SOAK)) {
							lcd_clrline(1);
							lcd_set_cursor(1,9);
							lcd_print_P("Soak");
							STAT_SET(PROFILE_SOAK);
						}
						break;
					case 3:
						if(!STAT(PROFILE_RAMPUP)) {
							lcd_clrline(1);
							lcd_set_cursor(1,7);
							lcd_print_P("Ramp-up");
							STAT_SET(PROFILE_RAMPUP);
						}
						break;
					case 4:
						if(!STAT(PROFILE_PEAK)) {
							lcd_clrline(1);
							lcd_set_cursor(1,9);
							lcd_print_P("Peak");
							STAT_SET(PROFILE_PEAK);
						}
						break;
					case 5:
						if(!STAT(PROFILE_RAMPDOWN)) {
							lcd_clrline(1);
							lcd_set_cursor(1,6);
							lcd_print_P("Ramp-down");
							STAT_SET(PROFILE_RAMPDOWN);
						}
						break;
				}
			}
		}
	}
}

static inline void show_profile_completion(void)
{
	if(STAT(PROFILE_RUNNING)) {
		STAT_CLR(PROFILE_RUNNING);
		lcd_clrscr();
		if(STAT(PROFILE_CANCEL)) {
			CANCEL_TIMER_DISABLE;
			lcd_set_cursor(2,2);
			lcd_print_p(reflowcancelledmsg);
		} else {
			lcd_set_cursor(2,3);
			lcd_print_p(reflowcompletemsg);
			lcd_set_cursor(3,1);
			lcd_print_p(presstocontinuemsg);
			start_buzzer(3);
		}
	}
	reset_profile_state();
}

static inline void show_about(void)
{
	lcd_set_cursor(1,2);
	lcd_print_P(PROGRAM_NAME);
	lcd_print_P(" v");
	lcd_print_P(PROGRAM_VER);
	lcd_set_cursor(3,2);
	lcd_print_p(aboutcopyrightmsg);
	lcd_print_P(PROGRAM_DEV);
	lcd_set_cursor(4,1);
	lcd_print_p(aboutlicensemsg);
}

static inline void show_coming_soon(void)
{
	lcd_set_cursor(2,5);
	lcd_print_p(comingsoonmsg);
	lcd_set_cursor(3,1);
	lcd_print_p(presstocontinuemsg);
}



static inline void start_buzzer(uint8_t cnt) {
	buzzer_count = cnt*20;		// Beep three times for 0.5s each
	BUZZER_ENABLE;
	TEMPREP_BUZZ_ENABLE;
}



static inline void reset_profile_state(void)
{
	HEAT_DISABLE;
	STAT_CLRPFSTAGE();
	if(activeprofile) {
		free(activeprofile);
		activeprofile = 0x0000;
	}
	time_ms = 0;
}

static inline void reset_cancel_timer(void)
{
	if(CANCEL_TIMER_ENABLED) lcd_clrline(2);
	CANCEL_TIMER_DISABLE;
	ctovf_count = 0;
}

static inline void reset_all(void)
{
	HEAT_DISABLE;
	TEMPREP_BUZZ_DISABLE;
	BUZZER_DISABLE;
	ADC_ENABLE;
	ISRF_CLRALL();
	STAT_CLRALL();
	MENU_CLR();
	menu_uninit();
	MENU_SET(MAIN);
	if(activeprofile) {
		free(activeprofile);
		activeprofile = 0x0000;
	}
	ctovf_count = 0;
	time_ms = 0;
}



ISR(ADC_vect)
{
	adc_average[(average_count++)%NUM_AVERAGE] = round(ADC*(1000.0/0xFF));
	
	// Calculate the average from our pool of readings
	uint16_t average = 0;
	for(uint8_t i=0; i<NUM_AVERAGE; i++)
		average += adc_average[i];
	temperature = round(average/NUM_AVERAGE);
	
	if(temperature<=5 || temperature>=995)
		STAT_SET(TC_ERROR);
	else
		STAT_CLR(TC_ERROR);
}

ISR(TIMER0_COMPA_vect)
{
	// Don't respond to button presses until this timer has fired eight times
	// (i.e.: after about 16ms)
	debounce_count++;
	if(!(debounce_count&0b111)) DEBOUNCE_DISABLE;
}

ISR(TIMER0_OVF_vect)
{
	ctovf_count++;
}

ISR(TIMER1_COMPA_vect)
{
	if(activeprofile) {
		targettemp = 0;
		double time_sec = time_ms/1000.0;
		if(time_sec <= *activeprofile) {
			targettemp = *(activeprofile+1);
		} else if(time_sec >= *(activeprofile+(PROFILE_LENGTH-1)*PROFILE_DATLEN)) {
			STAT_SET(PROFILE_COMPLETE);
		} else {
			uint8_t i = PROFILE_LENGTH;
			while(--i) {
				if(time_sec <= *(activeprofile+(i*PROFILE_DATLEN)) &&
					 time_sec > *(activeprofile+((i-1)*PROFILE_DATLEN))) {
					uint16_t x0 = *(activeprofile+((i-1)*PROFILE_DATLEN));
					uint16_t x1 = *(activeprofile+(i*PROFILE_DATLEN));
					int16_t y0 = *(activeprofile+((i-1)*PROFILE_DATLEN)+1)*10;
					int16_t y1 = *(activeprofile+(i*PROFILE_DATLEN)+1)*10;
					targettemp = (y0+((y1-y0)*((time_sec-x0)/(x1-x0))))/10;
				}
			}
		}
		
		if(temperature<targettemp)	HEAT_ENABLE;
		else												HEAT_DISABLE;
		
		time_ms += 50;	// Add 0.05 seconds to the global timer
		// Report the temperature every 500ms
		if((!(time_ms%500)) && STAT_SET(PROFILE_RUNNING))
			ISRF_SET(REPORT_TEMP);
	} else if(buzzer_count) {
		buzzer_count--;
		if(!buzzer_count)	TEMPREP_BUZZ_DISABLE;
		else if(!(buzzer_count%10)) BUZZER_TOGGLE;
	}
}

ISR(PCINT2_vect)
{
	volatile static uint8_t pd_prev = 0xFF;
	
	// Check if door switch is high (door is open)
	if(PIND&(1<<2)) {
		STAT_SET(DOOR_OPEN);
	// Check if door switch is low (door is closed)
	} else {
		STAT_CLR(DOOR_OPEN);
	}
	
	// Check encoder A and B values
	static const int8_t _encoder_lookup[] PROGMEM = { 0,-1, 1, 0,
																									  1, 0, 0,-1,
																									 -1, 0, 0, 1,
																									  0, 1,-1, 0};
	static volatile uint8_t old_AB = 0;
	static volatile int8_t encoder_value = 0;
	old_AB<<=2;
	old_AB |= ((PIND&((1<<6)|(1<<5)))>>5);
	encoder_value += pgm_read_byte(&(_encoder_lookup[(old_AB&0x0F)]));
	if(encoder_value<=-12) {
		ISRF_SET(NEXT);
		encoder_value = 0;
	} else if(encoder_value>=12) {
		ISRF_SET(PREV);
		encoder_value = 0;
	}
	
	// Check if button is low, but was previously high (key-down event)
	if((PIND&(1<<7))==0x00 && (pd_prev&(1<<7))!=0x00) {
		ISRF_SET(CANCEL);
		DEBOUNCE_ENABLE;
	}
	// Check if button is high, but was previously low (key-up event)
	if((PIND&(1<<7))!=0x00 && (pd_prev&(1<<7))==0x00) {
		ISRF_SET(ENTER);
		DEBOUNCE_ENABLE;
	}
	
	pd_prev = PIND;
}
