#include <msp430g2233.h>
#include <legacymsp430.h>

#include "Uart.h"

extern unsigned char uart_buffer[32]; // Save Received Data, At most 32 Bytes
extern int buffer_counter;            // Buffer Counter of UART
extern unsigned char function;        // Represent the function that is used
extern unsigned char but_press;       // Represent the button that is pressed

void ConfigureClocks()
{
    if (CALBC1_1MHZ==0xFF || CALDCO_1MHZ==0xFF)
    {
        while(1);    // If calibration constants erased
                     // do not load, trap CPU!!
    }
    /* Configure Clocks */
    BCSCTL1 = CALBC1_1MHZ; // Set DCO
    DCOCTL = CALDCO_1MHZ;
}

void ConfigureUART()
{
    /* Configure UART */
    P1SEL = BIT1 + BIT2;  // P1.1 = RXD, P1.2=TXD
    P1SEL2 = BIT1 + BIT2; // P1.1 = RXD, P1.2=TXD
    UCA0CTL1 |= UCSSEL_2; // SMCLK
    UCA0BR0 = 104;        // 1MHz 9600
    UCA0BR1 = 0;          // 1MHz 9600
    UCA0MCTL = UCBRS0;    // Modulation UCBRSx = 1
    UCA0CTL1 &= ~UCSWRST; // **Initialize USCI state machine**
    IE2 |= UCA0RXIE;      // Enable USCI_A0 RX interrupt
}


void ConfigureSwitches()
{
    /* Configure Switches on P2.1 & P2.2 - Enroll & Identify */

    P2REN |= (E_BTN + PT_BTN);    //P2.1 & P2.2 Enable pullup/pulldown resistor
    P2OUT |= (E_BTN + PT_BTN);    //P2.1 & P2.2 Pullup
    P2IE |= (E_BTN + PT_BTN);     //P2.1 & P2.2 interrupt enabled
    P2IES |= (E_BTN + PT_BTN);    //P2.1 & P2.2 Hi/lo edge
    P2IFG &= ~(E_BTN + PT_BTN);   //P2.1 & P2.2 IFG cleared just in case


    /* Configure Switch on P1.0 - Delete All */
    P1REN |= R_BTN;    //P1.0 Enable pullup/pulldown resistor
    P1OUT |= R_BTN;    //P1.0 Pullup
    P1IE |= R_BTN;     //P1.0 interrupt enabled
    P1IES |= R_BTN;    //P1.0 Hi/lo edge
    P1IFG &= ~R_BTN;   //P1.0 IFG cleared just in case
}

void ConfigureBuzzer()
{
    /* Configure Buzzer */
    P2DIR |= SPK;
    P2OUT |= SPK;
}

void ConfigurePowerControl()
{
    /* FVCC */
    P1DIR |= PW_CTR;
    P1OUT &= ~PW_CTR; // Turn FVCC off

    /* PD1*/
    P2DIR |= PD1;
    P2OUT &= ~PD1;
}

void ActivateFPM()
{
    /* Power Control Turn on (Activate Fingerprint Module) */
    buffer_counter = 0;
    P1OUT |= PW_CTR;
    ActivateBeep();
}

void StopFPM()
{
    /* Power Control Turn off (Stop Fingerprint Module) */
    StopBeep();
    P1OUT &= ~PW_CTR;
}

void InterruptEnable()
{
    P2IFG &= ~(E_BTN + PT_BTN);
    P1IFG &= ~R_BTN;  //P1.0 IFG cleared just in case

    P2IE |= (E_BTN + PT_BTN);
    P1IE |= R_BTN;    //P1.0 interrupt enabled
}

void InterruptDisable()
{
    P2IE &= ~(E_BTN + PT_BTN);
    P1IE &= ~R_BTN;
}

void ActivateBeep()
{
    int i;
    // 0.1 second
    for (i=0; i<500; i++) // Frequency 2500Hz, 85dB
    {
        P2OUT ^= SPK;
        __delay_cycles(200);
    }
    P2OUT |= SPK;
}

void StopBeep()
{
    int i;
    // 0.005 second
    /*
    for (i=0; i<50; i++) // Frequency 5000Hz, 85dB
    {
        P2OUT ^= SPK;
        __delay_cycles(100);
    }
    */
    P2OUT |= SPK;
}

void SuccessBeep() //So La Si Do
{
    int i;
    // 0.5 second (0.1 + 0.1 + 0.1 + 0.2)
    for (i=0; i<627; i++) // Frequency 3136Hz, 76dB
    {
        P2OUT ^= SPK;
        __delay_cycles(159);
    }
    for (i=0; i<704; i++) // Frequency 3520Hz, 76dB
    {
        P2OUT ^= SPK;
        __delay_cycles(142);
    }
    for (i=0; i<790; i++) // Frequency 3951Hz, 76dB
    {
        P2OUT ^= SPK;
        __delay_cycles(127);
    }
    for (i=0; i<837*2; i++) // Frequency 4186Hz, 76dB
    {
        P2OUT ^= SPK;
        __delay_cycles(119);
    }
    P2OUT |= SPK;
}

void FailBeep()
{
    int i;
    // 0.25 second (0.1 + 0.05 + 0.1)
    for (i=0; i<546; i++) // Frequency 2730Hz, 90dB
    {
        P2OUT ^= SPK;
        __delay_cycles(183);
    }
    __delay_cycles(50000);
    for (i=0; i<546; i++) // Frequency 2730Hz, 90dB
    {
        P2OUT ^= SPK;
        __delay_cycles(183);
    }
    P2OUT |= SPK;
}

void DeleteBeep()
{
    int i;
    //  0.7 second
    for (i=0; i<7000; i++) // Frequency 5000Hz, 85dB
    {
        P2OUT ^= SPK;
        __delay_cycles(100);
    }
    P2OUT |= SPK;
}

void CheckButton()
{
    unsigned int i;

    switch(but_press)
    {
        case 'E': // Enroll
            function = 'e';

            while (!(IFG2&UCA0TXIFG)); // USCI_A0 TX buffer ready?
            UCA0TXBUF = ENROLL;
            /*
            while (!(IFG2&UCA0TXIFG)); // USCI_A0 TX buffer ready?
            UCA0TXBUF = ENROLL;
            while (!(IFG2&UCA0TXIFG)); // USCI_A0 TX buffer ready?
            UCA0TXBUF = ENROLL;
            while (!(IFG2&UCA0TXIFG)); // USCI_A0 TX buffer ready?
            UCA0TXBUF = ENROLL;
            while (!(IFG2&UCA0TXIFG)); // USCI_A0 TX buffer ready?
            UCA0TXBUF = ENROLL;
            while (!(IFG2&UCA0TXIFG)); // USCI_A0 TX buffer ready?
            UCA0TXBUF = 0x9A;
            while (!(IFG2&UCA0TXIFG)); // USCI_A0 TX buffer ready?
            UCA0TXBUF = 0x85;
            while (!(IFG2&UCA0TXIFG)); // USCI_A0 TX buffer ready?
            UCA0TXBUF = end_code1;
            while (!(IFG2&UCA0TXIFG)); // USCI_A0 TX buffer ready?
            UCA0TXBUF = end_code2;
            */
            break;

        case 'I': // Identify
            function = 'i';
            while (!(IFG2&UCA0TXIFG)); // USCI_A0 TX buffer ready?
            UCA0TXBUF = IDENTIFY;
            /*
            while (!(IFG2&UCA0TXIFG)); // USCI_A0 TX buffer ready?
            UCA0TXBUF = IDENTIFY;
            while (!(IFG2&UCA0TXIFG)); // USCI_A0 TX buffer ready?
            UCA0TXBUF = IDENTIFY;
            while (!(IFG2&UCA0TXIFG)); // USCI_A0 TX buffer ready?
            UCA0TXBUF = IDENTIFY;
            while (!(IFG2&UCA0TXIFG)); // USCI_A0 TX buffer ready?
            UCA0TXBUF = IDENTIFY;
            while (!(IFG2&UCA0TXIFG)); // USCI_A0 TX buffer ready?
            UCA0TXBUF = 0x96;
            while (!(IFG2&UCA0TXIFG)); // USCI_A0 TX buffer ready?
            UCA0TXBUF = end_code1;
            while (!(IFG2&UCA0TXIFG)); // USCI_A0 TX buffer ready?
            UCA0TXBUF = end_code2;
            */
            break;

        case 'N': // Product Test
            function = 'n';
            while (!(IFG2&UCA0TXIFG)); // USCI_A0 TX buffer ready?
            UCA0TXBUF = PRODUCT_TEST;
            /*
            while (!(IFG2&UCA0TXIFG)); // USCI_A0 TX buffer ready?
            UCA0TXBUF = PRODUCT_TEST;
            while (!(IFG2&UCA0TXIFG)); // USCI_A0 TX buffer ready?
            UCA0TXBUF = PRODUCT_TEST;
            while (!(IFG2&UCA0TXIFG)); // USCI_A0 TX buffer ready?
            UCA0TXBUF = PRODUCT_TEST;
            while (!(IFG2&UCA0TXIFG)); // USCI_A0 TX buffer ready?
            UCA0TXBUF = PRODUCT_TEST;
            while (!(IFG2&UCA0TXIFG)); // USCI_A0 TX buffer ready?
            UCA0TXBUF = 0x91;
            while (!(IFG2&UCA0TXIFG)); // USCI_A0 TX buffer ready?
            UCA0TXBUF = end_code1;
            while (!(IFG2&UCA0TXIFG)); // USCI_A0 TX buffer ready?
            UCA0TXBUF = end_code2;
            */
            break;

        case 'P': // Delete
            function = 'p';
            while (!(IFG2&UCA0TXIFG)); // USCI_A0 TX buffer ready?
            UCA0TXBUF = DELETE;
            /*
            while (!(IFG2&UCA0TXIFG)); // USCI_A0 TX buffer ready?
            UCA0TXBUF = DELETE;
            while (!(IFG2&UCA0TXIFG)); // USCI_A0 TX buffer ready?
            UCA0TXBUF = DELETE;
            while (!(IFG2&UCA0TXIFG)); // USCI_A0 TX buffer ready?
            UCA0TXBUF = DELETE;
            while (!(IFG2&UCA0TXIFG)); // USCI_A0 TX buffer ready?
            UCA0TXBUF = DELETE;
            while (!(IFG2&UCA0TXIFG)); // USCI_A0 TX buffer ready?
            UCA0TXBUF = 0x8F;
            while (!(IFG2&UCA0TXIFG)); // USCI_A0 TX buffer ready?
            UCA0TXBUF = end_code1;
            while (!(IFG2&UCA0TXIFG)); // USCI_A0 TX buffer ready?
            UCA0TXBUF = end_code2;
            */
            break;
    }
}

void CheckResult()
{
    switch (function)
    {
        case 'p': //Delete All
            if (uart_buffer[0] == 0x53)
            {
	      	   DeleteBeep();
            }
            else if (uart_buffer[0] == 0x66)
            {
                FailBeep();
	            __delay_cycles(100000);
            }
            StopFPM();
            InterruptEnable();
            break;

        case 'e': //Enroll
            if (uart_buffer[0] == 0x53)
            {
                SuccessBeep();
                __delay_cycles(100000);
            }
            else	//0x66 or 0x67
            {
                FailBeep();
                __delay_cycles(100000);
            }
            StopFPM();
            InterruptEnable();
            break;

        case 'i': //Identify
            if (uart_buffer[0] == 0x73)
            {
                SuccessBeep();
                __delay_cycles(100000);
                StopFPM();
            }
            else
            {
                FailBeep();
                __delay_cycles(100000);
                StopFPM();
            }
            InterruptEnable();
            break;

        case 'n': // Product Test
            if (uart_buffer[0] == 0x73)
            {
                SuccessBeep();
                __delay_cycles(100000);
                StopFPM();
            }
            else if (uart_buffer[0] == 0x66)
            {
                FailBeep();
                __delay_cycles(100000);
                StopFPM();
            }
            else
            {
                DeleteBeep();
                __delay_cycles(100000);
                StopFPM();
            }
            InterruptEnable();
            break;
    }
}
