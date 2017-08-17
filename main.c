#include <msp430g2233.h>

#include "Uart.h"

#deinfe XXXXX                -1
#define NOKEY                 0
#define PUSH_KEY              1
#define LONG_KEY              2

unsigned char KEY = 0;
unsigned char KEY_State = 0;

unsigned char uart_buffer[32] = {0}; // Save Received Data, At most 32 Bytes (*List)
int buffer_counter = 0;              // Buffer Counter of UART
unsigned char function = 0;          // Represent the function that is used
unsigned char but_press = 0;         // Represent the button that is pressed
int trans_uart_test = 0;

int main(void)
{
    WDTCTL = WDTPW + WDTHOLD; // Stop WDT

    ConfigureClocks();
    ConfigureUART();
    ConfigureSwitches();
    ConfigureBuzzer();
    ConfigurePowerControl();

    __bis_SR_register(LPM0_bits + GIE); // Enter Low Power Mode 4(LPM4) w/interrupt
    // LPM4 Disables CPU and All Clocks, and the only interrupts that can activate the system from sleep are GPIO interrupts
}

#pragma vector=PORT2_VECTOR
__interrupt void Port_2()
{
    InterruptDisable(); // Prevent other interrupts

    /*
    if (P2IFG&E_BTN)
    {
        __delay_cycles(5000);

        if (!(P2IN&E_BTN))
        {
            while ((P2IN&E_BTN) == 0);

            but_press = 'E';

            ActivateFPM();      // Activate Fingerprint Module
            CheckButton();
        }
    }
    */

    if (P2IFG&PT_BTN)   // Product Test
    {
        __delay_cycles(5000);

        if (!(P2IN&PT_BTN))
        {
            while ((P2IN&PT_BTN) == 0);

            but_press = 'E';

            ActivateFPM();      // Activate Fingerprint Module
            CheckButton();
        }
    }
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1()
{
    static unsigned int KeyTime1 = 0, KeyTime2 = 0, LongClick = 0;

    InterruptDisable(); // Prevent other interrupts

    LongClick = 0;

    if (P1IFG&R_BTN)
    {
        __delay_cycles(5000);

        while((P1IN&R_BTN) == 0)
        {
            KeyTime1++;

            if (KeyTime1 > 60000)
            {
                KeyTime2++;
                KeyTime1 = 0;

                if (KeyTime2 > 5)
                {
                    LongClick = 1;

                    but_press = 'P';
                    ActivateFPM(); // Activate Fingerprint Module
                    CheckButton();

                    KeyTime2 = 0;
                }
            }
        }

        if(LongClick == 0)
        {
            but_press = 'I';
            ActivateFPM(); // Activate Fingerprint Module
            CheckButton();
        }
    }
}


#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
    if (UCA0RXBUF == SUCCESS)
    {
        buffer_counter = 0; // Reset buffer counter
        uart_buffer[buffer_counter] = UCA0RXBUF;
        if (uart_buffer[0] != AGAIN_BYTE) CheckResult();
    }
    else if (UCA0RXBUF == I_SUCCESS)
    {
        trans_uart_test = 0;
        buffer_counter = 0; // Reset buffer counter
        uart_buffer[buffer_counter] = UCA0RXBUF;
        if (uart_buffer[0] != AGAIN_BYTE) CheckResult();
    }
    else if ((UCA0RXBUF == ALL_FAIL)||(UCA0RXBUF == ENROLL_FAIL))
    {
        buffer_counter = 0; // Reset buffer counter
        uart_buffer[buffer_counter] = UCA0RXBUF;
        if (uart_buffer[0] != AGAIN_BYTE) CheckResult();
    }
    else
    {
        uart_buffer[buffer_counter] = UCA0RXBUF;
        if (UCA0RXBUF == (UART_TEST + trans_uart_test))
        {
            trans_uart_test++;
            buffer_counter = 0;
            UCA0TXBUF = UCA0RXBUF;
        }
        buffer_counter++;
    }
}

