// AVR ATMEGA8 - INTERNAL 8MHz CLK
// Auhtor: Zaid Pirwani
// Developer at Ejaad
// email@zaidpirwani.com
// 21-02-2012

#include <avr/pgmspace.h>
#include "globals.h"
#include "lcd_menu.h"

const char menusel_left PROGMEM = '\176';
const char menusel_right PROGMEM = '\177';
const char global_back[] PROGMEM = "\177 Back";

// Main Menu
const char mm_pb[] PROGMEM =			"Leaded Profile";
const char mm_rohs[] PROGMEM =		"RoHS Profile";
const char mm_opts[] PROGMEM =		"Settings";
const char mm_about[] PROGMEM =		"About Software";
PGM_P main_menu[MENU_LENGTH_main] PROGMEM =
{ mm_pb, mm_rohs, mm_opts, mm_about };

// Settings
const char sm_tempunits[] PROGMEM = "Temp. Units";
PGM_P settings_menu[MENU_LENGTH_settings] PROGMEM =
{ global_back, sm_tempunits };

// Temperature Units
const char um_c[] PROGMEM = "Celsius";
const char um_f[] PROGMEM = "Fahrenheit";
const char um_k[] PROGMEM = "Kelvin";
const char um_r[] PROGMEM = "Rankine";
const char um_d[] PROGMEM = "Delisle";
const char um_n[] PROGMEM = "Newton";
const char um_e[] PROGMEM = "R\11aumur";
const char um_o[] PROGMEM = "R\02mer";
PGM_P units_menu[MENU_LENGTH_units] PROGMEM =
{ um_c, um_f, um_k, um_r, um_d, um_n, um_e, um_o };

volatile uint8_t menuitem = 0, menuitem_prev = 0;

void menu_init_func(PGM_P *menu, uint8_t len) {
	if(activemenu==menu) return;
	lcd_clrscr();
	activemenu = menu;
	activemenulen = len;
	menuitem = menuitem_prev = 0;
	menu_display(menuitem);
}

void menu_redraw(void) {
	menu_display(menuitem);
}

inline void menu_uninit(void) {
	if(!activemenu) return;
	lcd_clrscr();
	activemenu = 0x0000;
	activemenulen = menuitem = menuitem_prev = 0;
}

void menu_next(void) {
	menuitem_prev = menuitem;
	menuitem += (menuitem>=(activemenulen-1))?0:1;
	menu_display(menuitem);
}

void menu_prev(void) {
	menuitem_prev = menuitem;
	menuitem -= (menuitem<1)?0:1;
	menu_display(menuitem);
}

uint8_t menu_selected(void) {
	return menuitem;
}

// Display Options / Update LCD Display
void menu_display(uint8_t item) {
	if(menuitem>=menuitem_prev) {
		
		// If we're going forward and not scrolling the list up
 		if(menuitem<LCD_LINES-1) {
			for(uint8_t i=0; i<LCD_LINES && i<activemenulen; i++) {
				lcd_set_cursor(i+1,3);
				lcd_print_p((PGM_P)pgm_read_word(&activemenu[i]));
			}
			// Clear previous selection markers
			lcd_set_cursor(menuitem,1);
			lcd_putc(' ');
			lcd_set_cursor(menuitem,LCD_DISP_LENGTH);
			lcd_putc(' ');
			// Draw the new markers
			lcd_set_cursor(menuitem+1,1);
			lcd_putc(pgm_read_byte(&menusel_left));
			lcd_set_cursor(menuitem+1,LCD_DISP_LENGTH);
			lcd_putc(pgm_read_byte(&menusel_right));
		// If we reach the end of the list
		} else if (menuitem>=activemenulen-1) {
			for(uint8_t i=0; i<LCD_LINES; i++) {
				lcd_set_cursor(i+1,3);
				lcd_print_p((PGM_P)pgm_read_word(&activemenu[activemenulen-(LCD_LINES-i)]));
			}
			// Clear previous selection markers
			lcd_set_cursor(LCD_LINES-1,1);
			lcd_putc(' ');
			lcd_set_cursor(LCD_LINES-1,LCD_DISP_LENGTH);
			lcd_putc(' ');
			// Draw the new markers
			lcd_set_cursor(LCD_LINES,1);
			lcd_putc(pgm_read_byte(&menusel_left));
			lcd_set_cursor(LCD_LINES,LCD_DISP_LENGTH);
			lcd_putc(pgm_read_byte(&menusel_right));
		// If we're scrolling
		} else {
			lcd_clrscr();
			for(uint8_t i=0; i<LCD_LINES; i++) {
				lcd_set_cursor(i+1,3);
				lcd_print_p((PGM_P)pgm_read_word(&activemenu[i+(menuitem-(LCD_LINES-2))]));
			}
			lcd_set_cursor(LCD_LINES-1,1);
			lcd_putc(pgm_read_byte(&menusel_left));
			lcd_set_cursor(LCD_LINES-1,LCD_DISP_LENGTH);
			lcd_putc(pgm_read_byte(&menusel_right));
		}
	
	} else if(menuitem<menuitem_prev) {
		uint8_t i = (activemenulen<LCD_LINES)?activemenulen:LCD_LINES;
		
		// If we reach the beginning of the list
		if (menuitem==0) {
			for(; i>0; i--) {
				lcd_set_cursor(i,3);
				lcd_print_p((PGM_P)pgm_read_word(&activemenu[i-1]));
			}
			// Clear previous selection markers
			lcd_set_cursor(2,1);
			lcd_putc(' ');
			lcd_set_cursor(2,LCD_DISP_LENGTH);
			lcd_putc(' ');
			// Draw the new markers
			lcd_set_cursor(1,1);
			lcd_putc(pgm_read_byte(&menusel_left));
			lcd_set_cursor(1,LCD_DISP_LENGTH);
			lcd_putc(pgm_read_byte(&menusel_right));
		// If we're going backward and not scrolling the list down
		} else if(menuitem>activemenulen-LCD_LINES) {
			for(; i>0; i--) {
				lcd_set_cursor(i,3);
				lcd_print_p((PGM_P)pgm_read_word(&activemenu[activemenulen-(LCD_LINES-(i-1))]));
			}
			// Clear previous selection markers
			lcd_set_cursor(LCD_LINES-(activemenulen-menuitem)+2,1);
			lcd_putc(' ');
			lcd_set_cursor(LCD_LINES-(activemenulen-menuitem)+2,LCD_DISP_LENGTH);
			lcd_putc(' ');
			// Draw the new markers
			lcd_set_cursor(LCD_LINES-(activemenulen-menuitem)+1,1);
			lcd_putc(pgm_read_byte(&menusel_left));
			lcd_set_cursor(LCD_LINES-(activemenulen-menuitem)+1,LCD_DISP_LENGTH);
			lcd_putc(pgm_read_byte(&menusel_right));
		// If we're scrolling
		} else {
			lcd_clrscr();
			for(; i>0; i--) {
				lcd_set_cursor(i,3);
				lcd_print_p((PGM_P)pgm_read_word(&activemenu[(i-1)+(menuitem-1)]));
			}
			lcd_set_cursor(2,1);
			lcd_putc(pgm_read_byte(&menusel_left));
			lcd_set_cursor(2,LCD_DISP_LENGTH);
			lcd_putc(pgm_read_byte(&menusel_right));
		}
	
	}
}
