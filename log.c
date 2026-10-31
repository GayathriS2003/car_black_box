#include <xc.h>
#include "main.h"
#include "clcd.h"
#include "digital_keypad.h"
#include "external_eeprom.h"
#include "uart.h"

char e_logs_a[4][17]; // 0-3
char e_logs_b[3][17]; // 4-6
char e_logs_c[3][17]; // 7-9


//Function to get log pointer from segmented storage 
//Since PIC has limited RAM, logs are stored in 3 separate arrays 
char* get_log(unsigned char i)
{
    if(i < 4) 
        return e_logs_a[i];         // If index 0-3, return from first array (a)
    else if(i < 7) 
        return e_logs_b[i-4];       // If index 4-6, return from second array (b)
    else 
        return e_logs_c[i-7];        // If index 7-9, return from third array (c)
}

void event_read(void)
{
    for(unsigned char i = 0; i < 10; i++)
    {
        unsigned char addr = i * 12;         // Each log occupies 12 bytes in EEPROM
        char *p = get_log(i);                // Get RAM pointer for this log index
        
        // Read time from EEPROM
        unsigned char hr  = read_external_eeprom(addr);
        unsigned char mn  = read_external_eeprom(addr+1);
        unsigned char sec = read_external_eeprom(addr+2);
        
        // Check if slot is empty 
        if(hr == 0xFF || hr > 23)
        {
            p[0] = i + '0';
            p[1] = ' ';
            p[2] = 'N'; 
            p[3]='O'; 
            p[4]=' ';
            p[5]='L'; 
            p[6]='O'; 
            p[7]='G';
            
            for(char j=8; j<16; j++) 
                p[j]=' ';
            p[16]='\0';          //null terminator
            continue;            // continue next log
        }

        // Read Event and Speed data
        char ev1 = read_external_eeprom(addr+3);
        char ev2 = read_external_eeprom(addr+4);
        char sp1 = read_external_eeprom(addr+5);
        char sp2 = read_external_eeprom(addr+6);
        
        
        // Handle erased values (0xFF) with defaults
        if(ev1 == 0xFF) 
            ev1 = ' ';
        if(ev2 == 0xFF) 
            ev2 = ' ';
        if(sp1 == 0xFF) 
            sp1 = '0';
        if(sp2 == 0xFF) 
            sp2 = '0';

        // Format final string: "0 12:00:00 ON 00" (16 chars + null)
        p[0] = i + '0';              // Log index number
        p[1] = ' ';                  // Space separator
        p[2] = (hr / 10) + '0';      //hour tens
        p[3] = (hr % 10) + '0';      //hour ones
        p[4] = ':';
        p[5] = (mn / 10) + '0';      //minutes tens
        p[6] = (mn % 10) + '0';      //minutes ones
        p[7] = ':';
        p[8] = (sec / 10) + '0';     //sec tens
        p[9] = (sec % 10) + '0';     //sec ones
        p[10] = ' ';
        p[11] = ev1;                 //event
        p[12] = ev2;
        p[13] = ' ';
        p[14] = sp1;                //speed
        p[15] = sp2;
        p[16] = '\0';
    }
}

void view_log(unsigned char key)
{
    
    // If no logs are stored, show message and return to menu
    if(event_count == 0)
    {
        clcd_print("no logs       ", LINE1(0));
        clcd_print("found         ", LINE2(0));
        __delay_ms(3000);
        operation = MENU;
        return;
    }
    static unsigned char index = 0;  // Current log index being displayed
    static char first = 0;           // Flag to check if first entry into function

    if(first == 0)
    {
        event_read();          // Load logs from external EEPROM
        first = 1;             // Set flag so it won't read again until exit
        index = 0;             // Start from first log
    }

    clcd_print("L TIME     EV SP", LINE1(0)); // Gap correct!
    
    // Calculate actual log position for circular buffer
    unsigned char once;
    if(event_count < 10)
        once = index;
    else
        once = (eeprom_index + index) % 10;
    clcd_print(get_log(index), LINE2(0));

    
    // UP key - scroll up
    if(key == SW1) 
    {
        if(index > 0) 
            index--;      // Decrease index if not at top
        __delay_ms(250);
    }
    
     // DOWN key - scroll down through logs
    else if(key == SW2) 
    {
        if(event_count > 0)           // Only scroll if logs exist
        {
            if(event_count >= 10)
            {
                if(index < 9) index++; // allow 0-9 full!
            }
            else
            {
                if(index < event_count-1) index++; // Scroll only up to available logs
            }
        }
        __delay_ms(250);
    }
    else if(key == SW5) 
    {
        index = 0;                   // Reset index
        first = 0;                   // Reset first entry flag for next time
        
        operation = MENU;
        
        clcd_print("                ", LINE1(0)); 
        clcd_print("                ", LINE2(0));
        
        __delay_ms(300);
    }
}

void download_log(unsigned char key) 
{
    // Read event count and index from EEPROM
    event_read();
    
    // Check if any logs are available
    if(event_count == 0)
    {
        clcd_print("no logs       ", LINE1(0));     
        clcd_print("found         ", LINE2(0));
        puts("no logs found\r\n");      //send same message via URAT
        
        __delay_ms(3000);
        
        operation = MENU;              //go back to menu
        return; 
    }
    
    clcd_print("Download Log    ", LINE1(0));
    clcd_print("Success fully   ", LINE2(0)); 
    
    puts("\r\nLogs  Time     EV SP\r\n");
    
    
    for(unsigned char i = 0; i < event_count && i < 10; i++)
    {
        // Logic for circular buffer: if less than 10 logs, start from 0
        // If more than 10, start from oldest (eeprom_index) and wrap around
        unsigned char first;
        if(event_count < 10) 
            first = i;
        else 
            first = (eeprom_index + i) % 10; 
        
        puts(get_log(first));          // Send log entry via UART
        puts("\r\n");                 // New line after each log
    }
    
    __delay_ms(3000); 
    
    operation = MENU;          //return to menu
    clcd_print("                ", LINE1(0)); 
    clcd_print("                ", LINE2(0));
}


void clear_log(unsigned char key)
{
    
     // Display success message on CLCD
    clcd_print("clear log     ", LINE1(0));
    clcd_print("successfully  ", LINE2(0));
    
    
    event_count = 0;   // Reset total event count to zero
    eeprom_index = 0;  // Reset EEPROM write address pointer to start
    
    // EEPROM full clear
    for(int i=0; i<100; i++)
        eeprom_write(i, 0xFF);  //Erase each loaction
    
    __delay_ms(3000); // after some time
    operation = MENU; // back to menu screen
}


unsigned char h_tens = 1, h_ones = 2;         //Default Hour = 12
unsigned char m_tens = 0, m_ones = 1;         //Default minute = 01
unsigned char s_tens = 1, s_ones = 2;         //Default seconds = 12
unsigned char field = 0;
unsigned int blink_cnt = 0;                   // Counter for blinking timing
unsigned char blink_show = 1;                 // Flag for blink state

void set_time(unsigned char dummy)
{
    clcd_print("HH:MM:SS        ", LINE1(0));
    
    unsigned char key = 0;
      // Read keypad status
    if(PORTBbits.RB0 == 0)
        key = 1;                    //increment value
    else if(PORTBbits.RB1 == 0)
        key = 2;                    //move to next field
    else if(PORTBbits.RB2 == 0)
        key = 4;                    //save and exit
    else if(PORTBbits.RB3 == 0)
        key = 5;                    //without save and exit
    
    // Handle blinking of selected field
    blink_cnt++;
    if(blink_cnt > 50)
    {
        blink_cnt = 0;
        blink_show = !blink_show;          // Invert blink state
    }
    
    // Prepare display buffer in HH:MM:SS format
    char buff[17];
    buff[0] = '0' + h_tens;   //hour tens digit
    buff[1] = '0' + h_ones;   //hour ones digit
    buff[2] = ':';
    buff[3] = '0' + m_tens;   //minutes tens digit
    buff[4] = '0' + m_ones;   //minutes ones digit
    buff[5] = ':';
    buff[6] = '0' + s_tens;   //sec tens digit
    buff[7] = '0' + s_ones;   //sec ones digit
    buff[8] = ' ';
    buff[9] = ' ';
    buff[10] = ' ';
    buff[11] = ' ';
    buff[12] = ' ';
    buff[13] = ' ';
    buff[14] = ' ';
    buff[15] = ' ';
    buff[16] = '\0';
    
    // If blink state is OFF, blank the selected field to create blink effect
    if(blink_show == 0)
    {
        if(field == 0)           
            buff[0] = ' ';       //blank hour ones
        else if(field == 1)
            buff[1] = ' ';       //blank hour tens
        else if(field == 2)
            buff[3] = ' ';      //blank minutes ones
        else if(field == 3)
            buff[4] = ' ';      //blank minutes tens
        else if(field == 4)
            buff[6] = ' ';      //blank sec ones
        else if(field == 5)
            buff[7] = ' ';      //blank sec tens
        
    }
    
    clcd_print(buff, LINE2(0));

    // Key 1: Increment current field value
    if(key == 1)
    {
        if(field == 0)           // Hour tens 0-2
        { 
            h_tens++; 
            if(h_tens > 2) 
                h_tens = 0; 
            if(h_tens == 2 && h_ones > 3)
                h_ones = 0; 
        }
        else if(field == 1)             // Hour ones logic (0-9 or 0-3 when tens is 2)
        { 
            if(h_tens == 2) 
            { 
                h_ones++; 
                if(h_ones > 3) 
                    h_ones = 0; 
            } 
            else 
            { 
                h_ones++; 
                if(h_ones > 9) 
                    h_ones = 0; 
            } 
        }
        else if(field == 2)
        { 
            m_tens++; 
            if(m_tens > 5) 
                m_tens = 0;            // Minute tens 0-5
        }
        else if(field == 3) 
        { 
            m_ones++; 
            if(m_ones > 9) 
                m_ones = 0;            // Minute ones 0-9
        }
        else if(field == 4) 
        { 
            s_tens++; 
            if(s_tens > 5) 
                s_tens = 0;            // Second tens 0-5
        }
        
        else if(field == 5) 
        { 
            s_ones++; 
            if(s_ones > 9)
                s_ones = 0;          // Second ones 0-9
        }
        
        while(PORTBbits.RB0 == 0);          // Wait until key is released
    }
    
    // Key 2: Switch to next field
    else if(key == 2) 
    { 
        field++; 
        if(field > 5)   // Loop back from 5 to 0
            field = 0; 
        
        while(PORTBbits.RB1 == 0);
    }
    
    // Key 3: Save time to DS1307 RTC
    else if(key == 4) 
    {
        unsigned char hour = h_tens*10 + h_ones;     // Combine tens and ones to decimal
        unsigned char min  = m_tens*10 + m_ones;
        unsigned char sec  = s_tens*10 + s_ones;
        
        // Convert to BCD and write to hours register
        write_ds1307(((hour/10)<<4)|(hour%10), 0x02);
        // Convert to BCD and write to minutes register
        write_ds1307(((min/10)<<4)|(min%10), 0x01);
        
        // Convert to BCD and write to second register
        write_ds1307(((sec/10)<<4)|(sec%10), 0x00);
        clcd_print("  TIME SAVED    ", LINE1(0));
        __delay_ms(800);
        
        operation = MENU;                         //back to menu
        
        while(PORTBbits.RB2 == 0);
    }
    
     // Key 4: Exit without saving
    else if(key == 5) 
    { 
        clcd_print("  EXIT NOSAVE   ", LINE1(0));
        
        __delay_ms(800);
        
        operation = MENU;                 //go back menu
        
        while(PORTBbits.RB3 == 0);
    }
}

unsigned char new_passwd[5] = "0000";        // Buffer to store new password being entered
unsigned char pass_field = 0;                // Current digit position (0 to 4) - how many digits entered
unsigned int pass_blink_cnt = 0;             // Counter for cursor blinking timing
unsigned char pass_blink_show = 1;           // Flag to show/hide blinking cursor

void change_password(unsigned char dummy)
{
    clcd_print("ENTER NEW PASSWD", LINE1(0));
    
    unsigned char key = 0;
    if(PORTBbits.RB0 == 0) 
        key = 1;                     //sw1 is pressed[1]
    else if(PORTBbits.RB1 == 0) 
        key = 2;                     //sw2 is pressed[0]
    else if(PORTBbits.RB2 == 0) 
        key = 4;                     //sw4 is pressed  [save and exit]
    else if(PORTBbits.RB3 == 0) 
        key = 5;                     //sw5 is presses [cancel and exit]

    // Blinking cursor logic - toggle every 50 cycles
    pass_blink_cnt++;
    if(pass_blink_cnt > 50)
    { 
        pass_blink_cnt=0; 
        pass_blink_show=!pass_blink_show; 
    } 
     
    // Prepare display buffer for Line 2
    char buff[17];
    for(int i = 0;i < 4;i++)
    {
        if(i < pass_field) 
            buff[i] = '*';          // Show '*' for already entered digits 
        else 
            buff[i] = ' ';           // Show blank for remaining positions
    }
    
    //clear the reset of line
    for(int i = 4;i < 16;i++) 
        buff[i] = ' ';
    buff[16]='\0';

    // Hide current position if blink is off (creates blinking effect)
    // No blink needed after 4 digits are entered
    if(pass_field < 4 && pass_blink_show==0) 
        buff[pass_field]=' ';

    clcd_print(buff, LINE2(0));           // Display password entry field on line 2

     // If SW1 pressed and we have space, enter '1'
    if(key==1 && pass_field < 4)
    { 
        new_passwd[pass_field]='1'; 
        
        pass_field++; 
        
        while(PORTBbits.RB0==0);     // Wait for key release
    }
    
    // If SW2 pressed and we have space, enter '0'
    else if(key==2 && pass_field < 4)
    { 
        new_passwd[pass_field]='0';          
        
        pass_field++; 
        
        while(PORTBbits.RB1==0);      // Wait for key release
    }
    
    // If SW4 (Save) pressed and 4 digits are entered - SAVE new password
    else if(key==4 && pass_field == 4)           // Only save if all 4 digits entered
    {
        for(int i=0;i<4;i++)
        { 
            og_password[i]=new_passwd[i];           // Update password in RAM
            write_external_eeprom(0x64+i, new_passwd[i]);    // Save to external EEPROM (address 0x64)
            __delay_ms(20);
        }
        
        og_password[4]='\0';          // Add null terminator
        
        clcd_print("PASSWORD CHANGED", LINE1(0));        show success message
        clcd_print("   SAVED        ", LINE2(0));
        __delay_ms(1500);
        
        pass_field=0;                // Reset field position
        
        operation = MENU;          //Go back to menu
        
        while(PORTBbits.RB2==0);        // Wait for key release
    }
    
    // If SW5 (Cancel) pressed - Exit without saving
    else if(key==5)
    {
        clcd_print(" EXIT NO SAVE   ", LINE1(0));
        clcd_print("                ", LINE2(0));
        __delay_ms(1000);
        
        pass_field=0;                 // Reset field position
        
        for(int i=0;i<4;i++) 
            new_passwd[i]=og_password[i];  //Restore old password to buffer
        
        operation = MENU;             // Go back to menu
        
        while(PORTBbits.RB3==0);     // Wait for key release
    }
}