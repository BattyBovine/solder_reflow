#ifndef LCD_MENU_H
#define LCD_MENU_H

#define MENU_LABEL_LENGTH LCD_DISP_LENGTH-4

#define MENU_LENGTH_main 4
PGM_P main_menu[MENU_LENGTH_main];
#define MENU_LENGTH_settings 2
PGM_P settings_menu[MENU_LENGTH_settings];
#define MENU_LENGTH_units 8
PGM_P units_menu[MENU_LENGTH_units];

PGM_P *activemenu;
uint8_t activemenulen;

void menu_init_func(PGM_P *menu, uint8_t len);
#define menu_init(m) menu_init_func(m##_menu, MENU_LENGTH_##m)
void menu_redraw(void);
void menu_uninit(void);
void menu_next(void);
void menu_prev(void);
uint8_t menu_selected(void);
void menu_display(uint8_t menuitem);

#endif // LCD_MENU_H