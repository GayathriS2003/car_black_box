
/*
 * File:    password_screen.c
 * Author:
 *
 * Created on October 19, 2026
 */

#include <xc.h>
#include <string.h>
#include "main.h"


extern unsigned char og_password[5];
extern unsigned char operation;
void password(unsigned char key)
{
    // System password
    static char user_password[5];
    static unsigned char index = 0;
    static unsigned char attempts = 3;

    // SW3 pressed: Return to dashboard
    if (key == SW3)
    {
        index = 0;
        operation = DASHBOARD;
        return;
    }

    // Check account lock
    if (attempts == 0)
    {
        clcd_print("Account Locked  ", LINE1(0));
        clcd_print("Wait 1 minute   ", LINE2(0));

        for (unsigned char i = 0; i < 60; i++)
        {
            __delay_ms(1000);
        }

        attempts = 3;
        index = 0;

        clcd_print("                ", LINE1(0));
        clcd_print("                ", LINE2(0));

        return;
    }

    // Display password screen
    if (index == 0)
    {
        clcd_print("Enter Password: ", LINE1(0));
        clcd_print("                ", LINE2(0));
    }

    // SW1 = 1, SW2 = 0
    if (key == SW1 || key == SW2)
    {
        if (index < 4)
        {
            if (key == SW1)
            {
                user_password[index] = '1';
            }
            else
            {
                user_password[index] = '0';
            }

            clcd_putch('*', LINE2(index));
            index++;
        }

        // Check password after entering 4 digits
        if (index == 4)
        {
            user_password[4] = '\0';

            if (strcmp(user_password, og_password) == 0)
            {
                clcd_print("Login Successful", LINE1(0));
                clcd_print("Opening Menu... ", LINE2(0));

                __delay_ms(2000);

                // Clear password screen
                clcd_print("                ", LINE1(0));
                clcd_print("                ", LINE2(0));

                operation = 2;
                attempts = 3;
                index = 0;

                return;
            }
            else
            {
                attempts--;

                clcd_print("Wrong Password  ", LINE1(0));

                if (attempts > 0)
                {
                    clcd_print("Attempts left:  ", LINE2(0));
                    clcd_putch(attempts + '0', LINE2(15));
                }
                else
                {
                    clcd_print("Account Locked  ", LINE2(0));
                }

                __delay_ms(2000);

                // Reset password input
                index = 0;

                // Clear LCD
                clcd_print("                ", LINE1(0));
                clcd_print("                ", LINE2(0));
            }
        }
    }

}
