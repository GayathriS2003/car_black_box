/*
 * File:   dashboard.c
 * Author: 25014
 *
 * Created on 31 December, 2025
 */
#include <xc.h>
#include "main.h"
#include "external_eeprom.h"
unsigned char clock_reg[3], speed[3] = {0};
char  *event[ ] = { "ON",  "GN", "G1", "G2", "G3", "G4", "G5" , "GR", "C_ "};  //store events in an array
int pos = 0;
unsigned int adc_val;


void dashboard(unsigned char key) {
    
        clcd_print(" TIME     EV  SP", LINE1(0));
        
        // display time
        get_time(clock_reg);
        display_time(clock_reg);
        
        
        //check sw1 is pressed increment the pos++
        if(key == SW1)
        {
            
            if(pos < 7)
                pos++;
            
            else if(pos == 8)
                pos = 1;               //started with GN
            event_store();             //call the event store function
            event_count++;             //increment the event count
            if(event_count > 10)       //event count is greater than 10
                event_count = 10;      //set is 10
            
        }
        
        //check sw2 is pressed increment the pos--
        else if(key == SW2)
        {
            
            if(pos == 8)
                pos = 1;              //started with GN
            else if(pos > 1)
                pos--;                //decrement the pos
            event_store();            //call the event store function
            event_count++;            //increment the event count
        }
        
       //check sw2 is pressed 
        else if(key == SW3)
        {
            pos = 8;                  //restart with GN
            event_store();            //call the event store function
            event_count++;            //increment the event count
        }
        //if sw2 is pressed than decrement the pos--
        
        clcd_print(event[pos], LINE2(10 ));
        
        //display speed
        
        //read adc value from pot
        adc_val = read_adc(CHANNEL0); //0-1023
        
        //compress the value to 00 - 99
        adc_val = adc_val / 10.25;
        
        //convert the speed into string
        speed[0] = (adc_val / 10) + '0';
        speed[1] = (adc_val % 10) + '0';
        speed[2] = '\0';
        
        //display on clcd
        clcd_print(speed, LINE2(14));
        
        if(key == SW4)
        {
            operation = PASSWORD;                  // go to password
            clcd_print("               ", LINE1(0));
            clcd_print("               ", LINE2(0));
        }
        
        
        
        
        
}


void event_store()
{
    //calculate EEPROM address -it takes 12 bytes
    unsigned int addr = eeprom_index * 12;
    
    // Convert BCD to Decimal - RTC gives time in BCD format
    // >> 4 gets upper nibble (tens), & 0x0F gets lower nibble (units)
    unsigned char hr = ((clock_reg[0] >> 4) * 10) + (clock_reg[0] & 0x0F);  //hours
    unsigned char mn = ((clock_reg[1] >> 4) * 10) + (clock_reg[1] & 0x0F);  //minutes
    unsigned char sec = ((clock_reg[2] >> 4) * 10) + (clock_reg[2] & 0x0F);  //seconds

    // Store hours into EEPROM to base address
    write_external_eeprom(addr, hr);
    __delay_ms(10);
    
    // Store Minutes into next location (addr+1)
    write_external_eeprom(addr+1, mn);
    __delay_ms(10);
    
    // Store Seconds into next location (addr+2)
    write_external_eeprom(addr+2, sec);
    __delay_ms(10);
    
    // Store Event type - first character 
    write_external_eeprom(addr+3, event[pos][0]);
    __delay_ms(10);
    
    // Store Event type - second character 
    write_external_eeprom(addr+4, event[pos][1]);
    __delay_ms(10);
    
    // Store Speed - first digit (tens place)
    write_external_eeprom(addr+5, speed[0]);
    __delay_ms(10);
    
    // Store Speed - second digit (units place)
    write_external_eeprom(addr+6, speed[1]);
    __delay_ms(10);

    // Move to next index for next event
    eeprom_index++;
    
    // This keeps only last 10 events in memory
    if(eeprom_index >= 10) 
        eeprom_index = 0;

    // Save current index position into internal EEPROM (address 0x00)
    // So that after power off, we know where to write next
    eeprom_write(0x00, eeprom_index);
    __delay_ms(10);
    
    // Increase total event count, max 10
    // Used to know how many logs to display in View Log
    if(event_count < 10) 
        event_count++;
}