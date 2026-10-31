/*
 * File:   main.c
 * Author: Gayathri
 *
 * Created on OCTOBER 19 2026
 */

#pragma config WDTE = OFF
#include <xc.h>
#include "main.h"
#include <string.h>
#include "uart.h"

unsigned char og_password[5] = "1110";
unsigned char key, operation = DASHBOARD;
unsigned char eeprom_index = 5;
unsigned char event_count = 0;

void init_config()
{
    init_clcd();
    init_digital_keypad();
    init_i2c(100000);
    init_ds1307();
    init_adc();
    init_uart(9600);
    event_read();
    
    // Read saved password from external EEPROM (address 0x00 to 0x03)
    // Loop 4 times to read 4 characters of password
    for(int i=0;i<4;i++)
    { 
        og_password[i] = read_external_eeprom(0x00+i);
    }
    og_password[4]='\0';  // Add null terminator

    
    // Check if EEPROM is empty (first time programming)
    // Empty EEPROM reads as 0xFF (all bits 1)
    // If empty, set default password as "1110"
    if(og_password[0]==0xFF) 
    { 
        og_password[0]='1'; 
        og_password[1]='1'; 
        og_password[2]='1'; 
        og_password[3]='0'; 
    }
    
    // Save the default password into external EEPROM
    // This runs only first time or when EEPROM is cleared
    for(int i = 0; i < 4; i++)
    {
        write_external_eeprom(0x00+i, og_password[i]);
        __delay_ms(20);         // Delay for EEPROM write cycle to complete
    }
}

void main(void) {
    init_config();
    
    while(1)
    {
        key = read_digital_keypad(STATE);
        
       
        if(operation == DASHBOARD)       //dashboard
        {
            dashboard(key);
        }
        else if(operation == PASSWORD)   //password
        {
            password(key);
        } 
        else if(operation == MENU)      //menu 
        {
            menu_screen(key);
        }
        else if(operation == VIEW_LOG)    //view log
        {
            view_log(key);
        
        } 
        else if(operation == DOWNLOAD_LOG)     //download log
        {
            download_log(key);
        }
        else if(operation == CLEAR_LOG)        //clear log
        {
            clear_log(key);
        }
        else if(operation == SET_TIME)         //set time
        {
            set_time(key);
        }
        else if(operation == CHANGE_PSWD)      //change password
        {
            change_password(key);
        }

       
        
    }
}