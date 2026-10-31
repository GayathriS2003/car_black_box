#include <xc.h>
#include "main.h"
#include "digital_keypad.h"
#include "clcd.h"
#include "adc.h"

extern unsigned char operation;

char *menu[] = {"view log", "download log", "clear log", "set time", "change pswd"};

void menu_screen(unsigned char key)
{
    static char index = 0; // 0 to 4
    static char top = 0;
    static char prev = 99;
    
    // SCROLL DOWN - DKP5 (SW5 is pressed)
    if(key == SW5) 
    {
        if(index < 4)
        {
            index++;
        }
        if(index == 0) 
            top = 0;        //show vie log/ download log
        else 
            top = index - 1;
        if(top > 3) 
            top = 3;        //show set time / change passwd
    }
    
   // SCROLL UP - DKP4 (SW4 pressed)
    else if(key == SW4) 
    {
        if(index == 4)   // If at last item (change pswd)
        {
            index = 3;
            top = 3;     // Show set time / change pswd
        }
        else if(index > 0)  // If not at first item, move up
        {
            index--;
            top = index;
        }
        if(top > 3) top = 3;
    }
    
   // DISPLAY UPDATE - Only update if selection changed (to prevent flicker)
    if(prev != index)
    {
        prev = index;
        clcd_print("                ", LINE1(0));
        clcd_print("                ", LINE2(0));
        
        unsigned char bottom = top + 1;    // Bottom line item is top + 1
        
        //if cursor is on top line
        if(index == top)
        {
            clcd_putch('*', LINE1(0));         // Show '*' at top line
            clcd_print(menu[top], LINE1(1));   // Print top menu item
            clcd_print(" ", LINE2(0));         //clear star on bottom
            clcd_print(menu[bottom], LINE2(1)); // Print bottom menu item
        }
        //if cursor is on bottom line
        else
        {
            clcd_print(" ", LINE1(0));          // Clear star on top
            clcd_print(menu[top], LINE1(1));
            clcd_putch('*', LINE2(0));             // Show '*' at bottom line
            clcd_print(menu[bottom], LINE2(1));
        }
    }
    
    // ENTER SELECTION - DKP3 (SW3 pressed)
    if(key == SW3) 
    {
        if(index == 0)
            operation = VIEW_LOG;        // select view log
        if(index == 1)
            operation = DOWNLOAD_LOG;     // select download log
        if(index == 2)
            operation = CLEAR_LOG;        //select clear log
        if(index == 3)
            operation = SET_TIME;          //select set time
        if(index == 4)
            operation = CHANGE_PSWD;       //select change password
        
        operation = index + 3; // view log = 3, download = 4...
        prev = 99;          //reset prev for next time
        index = 0;          //reset menu position
        top = 0;
        __delay_ms(300);
      
    }
   // EXIT MENU - DKP2 (SW2 pressed) - Back to Dashboard
    else if(key == SW2) 
    {
        
        clcd_print("                ", LINE1(0)); //clear display
        clcd_print("                ", LINE2(0));
        operation = DASHBOARD;     // go back to dashboard
        index = 0;                //reset menu
        top = 0;
        prev = 99;
        __delay_ms(300);
        
    }
}