/**************************************************************************************************/

/*
 * File: finalproject.c
 * Team: Lambda^3
 * Members: Chris Houseman
 *          Randy Martinez
 *          Rachel Powers
 *          Chris Sanford
 *
 * Date: November 14, 2014
 *
 * Description: Code that allows robot to follow line
 *
 */

// ******************************************************************************************* //
// ******************************************************************************************* //

#include "p24fj64ga002.h"
#include <stdio.h>
#include "lcd.h"
#include <stdlib.h>
#include "soundboard.h"

_CONFIG1( JTAGEN_OFF & GCP_OFF & GWRP_OFF &
          BKBUG_ON & COE_ON & ICS_PGx1 &
          FWDTEN_OFF & WINDIS_OFF & FWPSA_PR128 & WDTPS_PS32768 )

_CONFIG2( IESO_OFF & SOSCSEL_SOSC & WUTSEL_LEG & FNOSC_PRIPLL & FCKSM_CSDCMD & OSCIOFNC_OFF &
          IOL1WAY_OFF & I2C1SEL_PRI & POSCMOD_XT )

// ******************************************************************************************* //

// variables that are accessed in multiple functions

volatile int state = 0;
volatile int buttonPress=0;
volatile int timerFlag=0;
volatile int playing = 0;
volatile int ADC_value;      // variable to store the binary value in the ADC buffer
volatile int ADC_left;
volatile int ADC_right;
volatile int ADC_reader;
volatile int count = 0;

// ******************************************************************************************* //
void ScanSensors(){
    //Sensor (inputs)
    //sensor1
    TRISBbits.TRISB2 = 1;
    AD1PCFGbits.PCFG4 = 0;

    
    //sensor 3
    TRISAbits.TRISA0 = 1;
    AD1PCFGbits.PCFG0 = 0;

    //sensor 4-barcode reader
    TRISBbits.TRISB3 = 1;
    AD1PCFGbits.PCFG5 = 0;

    //sensor 2
    TRISAbits.TRISA1 = 1;
    AD1PCFGbits.PCFG1 = 0;


    //ADCON REGISTER SetUP
    AD1CON2 = 0x0;
    AD1CON3 = 0x0101;    
    AD1CON1 = 0x20E4;   
    AD1CHS = 1;         // positive input is AN1
    AD1CSSL = 0;        // low reference set to 0

    AD1CON1bits.ADON = 1; // A/D operating mode set to A/D converter module is operating
    IFS0bits.AD1IF = 0;   // clear the A/D 1 interrupt flag

    //scan sensors
    AD1CON1bits.ADON = 1; // A/D operating mode set to A/D converter module is operating
    AD1CHS = 1;         // positive input is AN1
    AD1CON1bits.SAMP=1;
    while(!IFS0bits.AD1IF);  // wait while the A/D 1 interrupt flag is low
    IFS0bits.AD1IF = 0;     // clear the A/D 1 interrupt flag
    ADC_value = ADC1BUF0;   // stores the current value in the A/D 1 buffer in the ADC_value variable
    AD1CON1bits.SAMP=0;
    AD1CON1bits.ADON = 0; // A/D operating mode set to A/D converter module is operating

    AD1CHS = 0;         // positive input is AN0
    AD1CON1bits.ADON = 1; // A/D operating mode set to A/D converter module is operating
    AD1CON1bits.SAMP=1;
    while(!IFS0bits.AD1IF);  // wait while the A/D 1 interrupt flag is low
    IFS0bits.AD1IF = 0;     // clear the A/D 1 interrupt flag
    ADC_right = ADC1BUF0;   // stores the current value in the A/D 1 buffer in the ADC_value variable
    AD1CON1bits.SAMP=0;
    AD1CON1bits.ADON = 0; // A/D operating mode set to A/D converter module is operating

    AD1CHS = 4;         // positive input is AN4
    AD1CON1bits.ADON = 1; // A/D operating mode set to A/D converter module is operating
    AD1CON1bits.SAMP=1;
    while(!IFS0bits.AD1IF);  // wait while the A/D 1 interrupt flag is low
    IFS0bits.AD1IF = 0;     // clear the A/D 1 interrupt flag
    ADC_left = ADC1BUF0;   // stores the current value in the A/D 1 buffer in the ADC_value variable
    AD1CON1bits.SAMP=0;
    AD1CON1bits.ADON = 0; // A/D operating mode set to A/D converter module is operating

    AD1CHS = 5;         // positive input is AN4
    AD1CON1bits.ADON = 1; // A/D operating mode set to A/D converter module is operating
    AD1CON1bits.SAMP=1;
    while(!IFS0bits.AD1IF);  // wait while the A/D 1 interrupt flag is low
    IFS0bits.AD1IF = 0;     // clear the A/D 1 interrupt flag
    ADC_reader = ADC1BUF0;   // stores the current value in the A/D 1 buffer in the ADC_value variable
    AD1CON1bits.SAMP=0;
    AD1CON1bits.ADON = 0; // A/D operating mode set to A/D converter module is operating
}

/*
 *
 */
int main(void) {
    long i=0;
    SBInitialize();  // initialize the LCD display
    SBReset();
    SBPauseVoice();
        for(i=0;i<5000;++i){
            Delayms(1);
        }
    SBPlayVoice(0);
    for(i=0;i<10000;++i){
            Delayms(1);
        }

 /**********************************************/

    //use timer for PWM (Motor Control)
    T3CONbits.TCS = 0; // sets up to use internal clock
    T3CONbits.TGATE = 0;
    T3CONbits.TON = 0;  // Turn timer 3 off
    TMR3 = 0;           // resets timer 3 to 0

    T3CONbits.TCKPS = 3; // set a prescaler of 8 for timer 2
    PR3 = 575;
/*****************************************************/
        PR4 = 575;
	TMR4 = 0;
	IFS1bits.T4IF = 0;
	IEC1bits.T4IE = 1;
	T4CONbits.TCKPS = 3;
	T4CONbits.TON = 0;

/*****************************************************/

    //output compare stuff (motor control)
    OC1CONbits.OCM = 6; // Initialize OCx pin low, compare event forces OCx pin high,
    OC1CONbits.OCTSEL = 1; // using timer 3

    OC1R = OC1RS = PR3;

    OC2CONbits.OCM = 6; // Initialize OCx pin low, compare event forces OCx pin high,
    OC2CONbits.OCTSEL = 1; // using timer 3

    OC2R = OC2RS = PR3;

    //Ports used for output to H-bridge
    //PWM outputs
    RPOR0bits.RP0R = 18;    //10010 - OC1 (Output Compare 1)
    RPOR0bits.RP1R = 19;    //10011 - OC2 (Output Compare 2)
/*****************************************************/
    //pins that control which direction the motors turn (motor control)
    TRISBbits.TRISB11 = 0;
    LATBbits.LATB11 = 1;
    CNPU1bits.CN15PUE = 1;

    TRISBbits.TRISB10 = 0;
    LATBbits.LATB10 = 1;
    CNPU2bits.CN16PUE = 1;


/*****************************************************/
    //determines which state the car is in
// Configure TRIS register bits for switch 1 input
	TRISBbits.TRISB5 = 1;

// Configure CN register bits to enable change notifications for switch input.
	CNEN2bits.CN27IE = 1;
        IFS1bits.CNIF = 0;
        IEC1bits.CNIE = 1;
/*****************************************************/


    char value[8];
    int startBit=-1;

    int numPrinted = 0;
    int lastOnTrack = -1;
    int lastRead = -1;

    LCDInitialize();  // initialize the LCD display
    //AD1PCFG &= 0xFFDF; // Pin 7, AN5, where the POT is connected, IO6, is set to analog mode, AD module samples pin voltage

    T3CONbits.TON = 1;  // Turn timer 3 on - VVVEEERRRYYY important for comparisons

    while(1)
    {
//        while (buttonPress==0) {
//            ScanSensors();
//            sprintf(value, "%6d", ADC_right);
//            LCDMoveCursor(0,0);
//            LCDPrintString(value);
//        }
       while (buttonPress==0);
        ScanSensors();
        if (ADC_value < 175) {
//            LCDMoveCursor(0,0);
//            LCDPrintString("Go Str8 ");
//            sprintf(value, "%6d", ADC_value);
//            LCDMoveCursor(1,0);
//            LCDPrintString(value);
                OC1RS = PR3;
                OC2RS = PR3;
                LATBbits.LATB10=1;
                LATBbits.LATB11=0;
                lastOnTrack=2;
        }
        else if (ADC_right <70) {
//            LCDMoveCursor(0,0);
//            LCDPrintString("Go Right");
                OC1RS = PR3;
                OC2RS = 0;
                LATBbits.LATB10=1;
                LATBbits.LATB11=0;
                lastOnTrack=3;
        }
        else if (ADC_left<70){
        //else if (ADC_left < 20 ) {
//            LCDMoveCursor(0,0);
//            LCDPrintString("Go Left ");
                OC1RS = 0;
                OC2RS = PR3;
                LATBbits.LATB10=1;
                LATBbits.LATB11=0;
                lastOnTrack=1;
        }

        else if (lastOnTrack==2) {
//            LCDMoveCursor(0,0);
//            LCDPrintString("Go Right ");
                OC1RS = PR3;
                OC2RS = 0;
                LATBbits.LATB10=1;
                LATBbits.LATB11=0;
                lastOnTrack=2;
        }

        else if (lastOnTrack==3) {
//            LCDMoveCursor(0,0);
//            LCDPrintString("Go Right ");
                OC1RS = PR3;
                OC2RS = 0;
                LATBbits.LATB10=1;
                LATBbits.LATB11=0;
                lastOnTrack=3;
        }
        else if (lastOnTrack==1) {
//            LCDMoveCursor(0,0);
//            LCDPrintString("Go Left ");
                OC1RS = 0;
                OC2RS = PR3;
                LATBbits.LATB10=1;
                LATBbits.LATB11=0;
                lastOnTrack=1;
        }
        else {
                LATBbits.LATB10=0;
                LATBbits.LATB11=0;
        }
        
        if (startBit==-1) {

            //if (ADC_reader < 20) {
            if (ADC_reader <18) {
            startBit=1;
            lastRead=0;
            TMR4=0;
            T4CONbits.TON=1;
            }
        }
        if ((startBit==1)&&(ADC_reader>70)){
          if (lastRead == 0){
            lastRead=2;
          }
          else if (lastRead == 1) {
            lastRead=2;
          }
          if (numPrinted==4) {
              startBit=-1;
              numPrinted=0;
              T4CONbits.TON=0;
              count = 0;
              LATBbits.LATB10=0;
              LATBbits.LATB11=0;
          }
        }
        //if ((startBit == 1) && (ADC_reader <20)) {
        if ((startBit == 1) && (ADC_reader <18)) {
            if (lastRead==2) {
                LCDMoveCursor(1,numPrinted);
                LCDPrintChar('0');
                numPrinted=numPrinted+1;
                lastRead=0;
            }
            if (lastRead==1) {
                LCDMoveCursor(1,numPrinted-1);
                LCDPrintChar('0');
                numPrinted=numPrinted;
                lastRead=0;
            }
        }
        //if ((startBit == 1) && (ADC_reader<70) && (ADC_reader>20) ) {
        if ((startBit == 1) && (ADC_reader<60) && (ADC_reader>18) ) {
            if (lastRead==2) {
                LCDMoveCursor(1,numPrinted);
                LCDPrintChar('1');
                numPrinted=numPrinted+1;
                lastRead=1;
            }
        }
        if ((startBit==1) && (count==300) && (numPrinted<4)) {
            T4CONbits.TON=0;
            count = 0;
            startBit=-1;
            LCDClear();
            numPrinted=0;
        }
    }
}
//
void __attribute__((interrupt,auto_psv)) _T4Interrupt(void){
    IFS1bits.T4IF = 0;
    count=count+1;
}

void __attribute__((interrupt,auto_psv)) _CNInterrupt(void)
{
    // Clear CN interrupt flag to allow another CN interrupt to occur.

    while (PORTBbits.RB5==0);
    IFS1bits.CNIF = 0;
    buttonPress=1;
}