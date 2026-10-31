/* 
 * File:   main.h
 * Author: 25014
 *
 * Created on 26 November, 2025, 5:53 PM
 */

#ifndef MAIN_H
#define	MAIN_H
#include "adc.h"
#include "digital_keypad.h"
#include "clcd.h"
#include "i2c.h"
#include "ds1307.h"
#include "external_eeprom.h"

//define macros
#define           DASHBOARD      0
#define           PASSWORD       1
#define           MENU           2
#define           VIEW_LOG       3
#define           DOWNLOAD_LOG   4
#define           CLEAR_LOG      5
#define           SET_TIME       6
#define           CHANGE_PSWD    7

//dashboard
void dashboard(unsigned char key);

//password
void password(unsigned char key);

//menu_screen
void menu_screen(unsigned char key);

//store event
void event_store();

//read event
void event_read();

//view log
void view_log(unsigned char key);

//get log
char *get_log(unsigned char i);

//download log
void download_log(unsigned char key);

//clear log
void clear_log(unsigned char key);

//set time
void set_time(unsigned char dummy);

//change password
void change_password(unsigned char dummy);


extern unsigned char key;
extern unsigned char operation;
extern unsigned char eeprom_index;
extern unsigned char og_password[5]; 
extern unsigned char event_count;
extern char e_logs_a[4][17];
extern char e_logs_b[3][17];
extern char e_logs_c[3][17];





#endif	/* MAIN_H */

