#ifndef _UART_H_
#define _UART_H_


//-------------------- define ---------------------
#define    PW_CTR        BIT5    // FVCC Power Control
#define    E_BTN         BIT1    // Enroll Button
#define    PT_BTN        BIT2    // Product Test Button
#define    R_BTN         BIT0    // Delete All Button
#define    SPK           BIT3    // Buzzer Control
#define    PD1           BIT4    // Reed Switch Control

#define    ENROLL        0x65
#define    IDENTIFY      0x69
#define    DELETE        0x70
#define    PRODUCT_TEST  0x6E

#define    AGAIN_BYTE    0x61
#define    SUCCESS       0x53
#define    ALL_FAIL      0x66
#define    ENROLL_FAIL   0x67
#define    I_SUCCESS     0x73

#define    end_code1     0x0d
#define    end_code2     0x0a

#define    PT_ID_FAIL       0x66
#define    PT_FAIL          0x66
#define    PT_SUCCESS       0x73

#define    UART_TEST        0x7E


//-------------------- prototype ---------------------
void ConfigureClocks();
void ConfigureUART();
void ConfigureSwitches();
void ConfigureBuzzer();
void ConfigurePowerControl();

void ActivateFPM(); // Power Control Turn on (Activate Fingerprint Module)
void StopFPM();     // Power Control Turn off (Stop Fingerprint Module)
void InterruptEnable();
void InterruptDisable();

void ActivateBeep();
void StopBeep();
void SuccessBeep();
void FailBeep();
void DeleteBeep();

void CheckButton();
void CheckResult();

#endif
