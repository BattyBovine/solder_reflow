/*************************************************************************
© Copyright 2012 Peter Fleury <pfleury@gmx.ch>
TWI compatibility © Copyright 2012 Markus Dolze <bsdfan@nurfuerspam.de>
Further changes © Copyright 2014 Jamie Greunbaum <jamie.greunbaum@gmail.com>

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

#ifndef LCD_H
#define LCD_H

/*************************************************************************
 Title:     C include file for the HD44780U LCD library (lcd.c)
 Author:    Peter Fleury <pfleury@gmx.ch>  http://jump.to/fleury
 File:      $Id: lcd.h,v 1.4 2012/06/01 06:00:34 mmdolze Exp $
 Software:  AVR-GCC 3.3
***************************************************************************/

#include <inttypes.h>
#include <avr/pgmspace.h>

/**
 * Definitions for LCD command instructions
 * The constants define the various LCD controller instructions which can be passed to the
 * function  (), see HD44780 data sheet for a complete description.
 */
/* instruction register bit positions, see HD44780U data sheet */
#define LCD_CLR               0      /* DB0: clear display                  */
#define LCD_HOME              1      /* DB1: return to home position        */
#define LCD_ENTRY_MODE        2      /* DB2: set entry mode                 */
#define LCD_ENTRY_INC         1      /*   DB1: 1=increment, 0=decrement     */
#define LCD_ENTRY_SHIFT       0      /*   DB2: 1=display shift on           */
#define LCD_ON                3      /* DB3: turn lcd/cursor on             */
#define LCD_ON_DISPLAY        2      /*   DB2: turn display on              */
#define LCD_ON_CURSOR         1      /*   DB1: turn cursor on               */
#define LCD_ON_BLINK          0      /*     DB0: blinking cursor ?          */
#define LCD_MOVE              4      /* DB4: move cursor/display            */
#define LCD_MOVE_DISP         3      /*   DB3: move display (0-> cursor) ?  */
#define LCD_MOVE_RIGHT        2      /*   DB2: move right (0-> left) ?      */
#define LCD_FUNCTION          5      /* DB5: function set                   */
#define LCD_FUNCTION_8BIT     4      /*   DB4: set 8BIT mode (0->4BIT mode) */
#define LCD_FUNCTION_2LINES   3      /*   DB3: two lines (0->one line)      */
#define LCD_FUNCTION_10DOTS   2      /*   DB2: 5x10 font (0->5x7 font)      */
#define LCD_CGRAM             6      /* DB6: set CG RAM address             */
#define LCD_DDRAM             7      /* DB7: set DD RAM address             */
#define LCD_BUSY              7      /* DB7: LCD is busy                    */

/* set entry mode: display shift on/off, dec/inc cursor move direction */
#define LCD_ENTRY_DEC            0x04   /* display shift off, dec cursor move dir */
#define LCD_ENTRY_DEC_SHIFT      0x05   /* display shift on,  dec cursor move dir */
#define LCD_ENTRY_INC_           0x06   /* display shift off, inc cursor move dir */
#define LCD_ENTRY_INC_SHIFT      0x07   /* display shift on,  inc cursor move dir */

/* display on/off, cursor on/off, blinking char at cursor position */
#define LCD_DISP_OFF             0x08   /* display off                            */
#define LCD_DISP_ON              0x0C   /* display on, cursor off                 */
#define LCD_DISP_ON_BLINK        0x0D   /* display on, cursor off, blink char     */
#define LCD_DISP_ON_CURSOR       0x0E   /* display on, cursor on                  */
#define LCD_DISP_ON_CURSOR_BLINK 0x0F   /* display on, cursor on, blink char      */

/* move cursor/shift display */
#define LCD_MOVE_CURSOR_LEFT     0x10   /* move cursor left  (decrement)          */
#define LCD_MOVE_CURSOR_RIGHT    0x14   /* move cursor right (increment)          */
#define LCD_MOVE_DISP_LEFT       0x18   /* shift display left                     */
#define LCD_MOVE_DISP_RIGHT      0x1C   /* shift display right                    */

/* function set: set interface data length and number of display lines */
#define LCD_FUNCTION_4BIT_1LINE  0x20   /* 4-bit interface, single line, 5x7 dots */
#define LCD_FUNCTION_4BIT_2LINES 0x28   /* 4-bit interface, dual line,   5x7 dots */
#define LCD_FUNCTION_8BIT_1LINE  0x30   /* 8-bit interface, single line, 5x7 dots */
#define LCD_FUNCTION_8BIT_2LINES 0x38   /* 8-bit interface, dual line,   5x7 dots */

/* Shortcuts for positioning the cursor on the screen */
#define LCD_LINE_1					0x00
#define LCD_LINE_2					0x40
#define LCD_LINE_3					0x14
#define LCD_LINE_4					0x54
// #define LCD_CURSOR(x,y)			lcd_command((1<<LCD_DDRAM)|(x+y))

#define LCD_DISP_LENGTH	20
#define LCD_LINES				4
/* Address of the I2C port expander for the LCD */
#define LCD_TWI_ADDR		0x3F

#define LCD_CUSTOMCHARS_LEN	7
static const uint8_t custom_chars[LCD_CUSTOMCHARS_LEN][8] PROGMEM = {
	{ 0x02, 0x05, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00 },	// Degree symbol (\10)
	{ 0x02, 0x04, 0x0E, 0x11, 0x1F, 0x10, 0x0E, 0x00 },	// e-acute (\11)
	{ 0x00, 0x01, 0x0E, 0x13, 0x15, 0x19, 0x0E, 0x10 },	// Slashed o (\12)
	{ 0x00, 0x0E, 0x11, 0x15, 0x0C, 0x1C, 0x00, 0x00 },	// Rotate left (\13)
	{ 0x00, 0x0E, 0x11, 0x15, 0x06, 0x07, 0x00, 0x00 },	// Rotate right (\14)
	{ 0x0E, 0x13, 0x11, 0x0E, 0x00, 0x1F, 0x0E, 0x04 },	// Press dial (\15)
	{ 0x00, 0x0E, 0x17, 0x19, 0x17, 0x0E, 0x00, 0x00 },	// Copyright Symbol (\16)
};

/**
 * Functions
 */

/* Init for display */
extern void lcd_init(void);

/* Clear screen */
extern void lcd_clrscr(void);

/* Print string on display (no auto linefeed) */
extern void lcd_print(const char *s);

/* Display string from program memory */
extern void lcd_print_p(const char *progmem_s);
#define lcd_print_P(__s) lcd_print_p(PSTR(__s))

/* Send command to display */
extern void lcd_command(unsigned char cmd);

/* Send data to display */
extern void lcd_data(unsigned char data);
#define lcd_putc(c) lcd_data(c)

/* Wait for the busy flag to clear */
extern void lcd_busy_wait(void);

/* Set the position of the LCD cursor */
extern void lcd_set_cursor(uint8_t y, uint8_t x);

/* Clear a line by writing a string of spaces */
extern void lcd_clrline(uint8_t y);

/* Store hard-coded custom characters in CGRAM */
extern void lcd_write_cgram_defaults(void);

/* Define a custom character at the given memory location */
extern void lcd_write_cgram(uint8_t memoff, uint8_t c);

#endif
