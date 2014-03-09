/*************************************************************************
© Copyright 2006 Peter Fleury <pfleury@gmx.ch>

This file is part of Solder Reflow.

Solder Reflow is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Solder Reflow is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Solder Reflow.  If not, see <http://www.gnu.org/licenses/>.
***************************************************************************/

/*
 * Library to drive a HD44780 compatible display connected to I2C (aka TWI) bus
 * using a PCF8574 remote 8-bit I/O expander.
 *
 * The display should be connected like this:
 *
 * PCF8574 | HD44780
 *      P0 = RS
 *      P1 = RW
 *      P2 = EN
 *      P3 = Backlight switch
 *      P4 = D4
 *      P5 = D5
 *      P6 = D6
 *      P7 = D7
 *
 * Commands / data is sent to the I/O expander and the display is driven in
 * 4-bit mode. This library does NOT implement check of the busy flag!
 *
 * Partly based on Peter Fleury's LCD library.
 */

#include <inttypes.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>

#include "lcd_i2c.h"
#include "i2cmaster.h"

/*
 * local functions
 */

/*************************************************************************
 delay loop for small accurate delays: 16-bit counter, 4 cycles/loop
*************************************************************************/
static inline void _delayFourCycles(unsigned int __count)
{
	if ( __count == 0 )
		__asm__ __volatile__( "rjmp 1f\n 1:" );    // 2 cycles
	else
		__asm__ __volatile__ (
			"1: sbiw %0,1" "\n\t"
			"brne 1b"                              // 4 cycles/loop
			: "=w" (__count)
			: "0" (__count)
		);
}


/*************************************************************************
delay for a minimum of <us> microseconds
the number of loops is calculated at compile-time from MCU clock frequency
*************************************************************************/

#define _delay_us_asm(us)		_delayFourCycles( ( ( 1*(F_CPU/4000) )*us)/1000 )


/* Pin assignment for control lines on the PCF8574 */
#define RS      0x01
#define RW      0x02
#define EN      0x04
#define BL      0x08

/*************************************************************************
Sends one byte to the display using I2C bus.
Input:   data = byte to be sent, df = flag if data (1) or command (0)
Returns: none
*************************************************************************/
static void
lcd_write(unsigned char data, unsigned char df)
{
	uint8_t dataBits;
	
	// i2c_send_start();
	// i2c_send_adr(LCD_TWI_ADDR);
	i2c_start((LCD_TWI_ADDR<<1)|I2C_WRITE);
	
	dataBits = ((data&0xF0)|BL);					// Output high nybble
	if(df)
		dataBits |= RS;
	// if(!OCR0A)
		// dataBits |= BL;
	i2c_write(dataBits);
	i2c_write(EN | dataBits);
	i2c_write(dataBits);
	
	dataBits = (((data<<4)&0xF0)|BL);			// Output low nybble
	if(df)
		dataBits |= RS;
	// if(!OCR0A)
		// dataBits |= BL;
	i2c_write(dataBits);
	i2c_write(EN | dataBits);
	i2c_write(dataBits);
	
	// i2c_send_stop();
	i2c_stop();
	// _delay_us_asm(40);														// Minimum execution time
	lcd_busy_wait();
}


/*************************************************************************
Send LCD controller instruction command
Input:   instruction to send to LCD controller, see HD44780 data sheet
Returns: none
*************************************************************************/
void lcd_command(unsigned char cmd)
{
	lcd_write(cmd, 0);
}

/*************************************************************************
Send data byte to LCD controller
Input:   data to send to LCD controller, see HD44780 data sheet
Returns: none
*************************************************************************/
void lcd_data(unsigned char data)
{
	lcd_write(data, 1);
}

/*************************************************************************
Clear display
*************************************************************************/
void lcd_clrscr(void)
{
	lcd_command(1<<LCD_CLR);
	// _delay_us_asm(1500);
	lcd_busy_wait();
}

/*************************************************************************
Display string without auto linefeed
Input:    string to be displayed
Returns:  none
*************************************************************************/
void lcd_print(const char *s)
{
	while ( (*s) )
		lcd_data(*s++);
}

/*************************************************************************
Display string from program memory without auto linefeed
Input:     string from program memory be be displayed
Returns:   none
*************************************************************************/
void lcd_print_p(const char *progmem_s)
{
	register char c;
	
	while ( (c = pgm_read_byte(progmem_s++)) ) {
		lcd_data(c);
	}
}


/*************************************************************************
Initialize I2C and display
*************************************************************************/
void lcd_init(void)
{
	i2c_init();

	_delay_us_asm(16000);               /* wait 16ms after power-on */

	// i2c_send_start();
	// i2c_send_adr(LCD_TWI_ADDR);
	i2c_start((LCD_TWI_ADDR<<1)|I2C_WRITE);

	/*
	 * Send 0x30 a couple of times which is the same as
	 * (1<<LCD_FUNCTION | 1<<LCD_FUNCTION_8BIT)
	 */
	i2c_write(1<<LCD_FUNCTION | 1<<LCD_FUNCTION_8BIT);
	i2c_write(EN | (1<<LCD_FUNCTION | 1<<LCD_FUNCTION_8BIT));
	i2c_write(1<<LCD_FUNCTION | 1<<LCD_FUNCTION_8BIT);
	_delay_us_asm(4200);
	i2c_write(1<<LCD_FUNCTION | 1<<LCD_FUNCTION_8BIT);
	i2c_write(EN | (1<<LCD_FUNCTION | 1<<LCD_FUNCTION_8BIT));
	i2c_write(1<<LCD_FUNCTION | 1<<LCD_FUNCTION_8BIT);
	_delay_us_asm(64);
	i2c_write(1<<LCD_FUNCTION | 1<<LCD_FUNCTION_8BIT);
	i2c_write(EN | (1<<LCD_FUNCTION | 1<<LCD_FUNCTION_8BIT));
	i2c_write(1<<LCD_FUNCTION | 1<<LCD_FUNCTION_8BIT);
	_delay_us_asm(64);
	/* Switch to 4 bit mode */
	i2c_write(1<<LCD_FUNCTION);
	i2c_write(EN | (1<<LCD_FUNCTION));
	i2c_write(1<<LCD_FUNCTION);

	// i2c_send_stop();
	i2c_stop();

	lcd_command(LCD_FUNCTION_4BIT_2LINES);  /* function set: display lines */
	lcd_command(LCD_DISP_ON);   /* Display on, Cursor on, Blink off */
	lcd_command(LCD_ENTRY_INC_);   /* Display on, Cursor on, Blink off */
	lcd_clrscr();
	lcd_busy_wait();
}

void lcd_busy_wait(void)
{
	i2c_start((LCD_TWI_ADDR<<1)|I2C_READ);
	uint8_t c = 0x00;
	while(!(c&(1<<LCD_BUSY))) {
		i2c_write((EN|RW|0x00)&~RS);
		c |= (i2c_readNak()<<4)&0xF0;
		i2c_write((RW|0x00)&~RS);
		_delay_us_asm(1);
		i2c_write((EN|RW|0x00)&~RS);
		c |= i2c_readNak()&0x0F;
		i2c_write((RW|0x00)&~RS);
		_delay_us_asm(2);
	};
	i2c_stop();
}



uint8_t lcd_line(uint8_t y)
{
	switch(y) {
		case 2:
			return LCD_LINE_2;
		case 3:
			return LCD_LINE_3;
		case 4:
			return LCD_LINE_4;
		default:
			return LCD_LINE_1;
	}
}

void lcd_set_cursor(uint8_t y, uint8_t x)
{
	lcd_command((1<<LCD_DDRAM)|(lcd_line(y)+(x-1)));
}

void lcd_clrline(uint8_t y)
{
	lcd_command((1<<LCD_DDRAM)|(lcd_line(y)));
	for(uint8_t i=0; i<LCD_DISP_LENGTH; i++) lcd_data(' ');
}

void lcd_write_cgram_defaults(void) {
	lcd_command(1<<LCD_CGRAM);
	for(uint8_t i=0; i<LCD_CUSTOMCHARS_LEN; i++) {
		lcd_data(pgm_read_byte(&(custom_chars[i][0])));
		lcd_data(pgm_read_byte(&(custom_chars[i][1])));
		lcd_data(pgm_read_byte(&(custom_chars[i][2])));
		lcd_data(pgm_read_byte(&(custom_chars[i][3])));
		lcd_data(pgm_read_byte(&(custom_chars[i][4])));
		lcd_data(pgm_read_byte(&(custom_chars[i][5])));
		lcd_data(pgm_read_byte(&(custom_chars[i][6])));
		lcd_data(pgm_read_byte(&(custom_chars[i][7])));
	}
	lcd_command(1<<LCD_HOME);
}

void lcd_write_cgram(uint8_t memoff, uint8_t c) {
	lcd_command((1<<LCD_CGRAM)+(memoff*8));
	for(uint8_t i=0; i<8; i++)
		lcd_data(pgm_read_byte(&(custom_chars[c][i])));
	lcd_command(1<<LCD_HOME);
}
