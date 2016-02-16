/*******************************************************************
 * Processor:       PIC18F4520
 * Compiler:        MPLAB C18
 *
 * This file does the following....
 *
 * Author (Your name here)
 ********************************************************************/

/** Header Files ***************************************************/
#include <p18f4520.h>
#include <stdio.h>
#include <delays.h>
#include <timers.h>
#include <usart.h>


void low_isr(void);
void high_isr(void);


/** Configuration Bits *********************************************/
#pragma config OSC = INTIO67
#pragma config WDT = OFF
#pragma config LVP = OFF
#pragma config BOREN = OFF
#pragma config XINST = OFF


#pragma code high_vector=0x08
void interrupt_at_high_vector(void)
{
   _asm goto high_isr _endasm
}

#pragma code low_vector=0x18

void interrupt_at_low_vector(void) {
    _asm goto low_isr _endasm
}


/** Define Constants Here ******************************************/
#define SAMPLE 100

/** Local Function Prototypes **************************************/
void stepOnceForward(void);
void stepOnceBackward(void);
void stepTo(int);

/** Global Variables ***********************************************/
#define STEP1 0b00010100
#define STEP2 0b00001100
#define STEP3 0b00001010
#define STEP4 0b00010010

//Maybe was good
#define STEP5 0b00001100
#define STEP6 0b00010100
#define STEP7 0b00010010
#define STEP8 0b00001010


#define PRESSED 0
#define UNPRESSED 1
#define DELAY_MS 1
int i =0;
int j;
int buttonState = 0;
int which = 0;
int x = 240;
int stepPos = 0;
int stepTarget = 0;
int stepDigits[3];
int servoPos = 145;
int servoDigits[3];
int rcvCount = 0;
char data;
/*******************************************************************
 * Function:        void main(void)
 ********************************************************************/
#pragma code

void main(void) {
    // Set the clock to 4 MHz
    OSCCONbits.IRCF2 = 1;
    OSCCONbits.IRCF1 = 1;
    OSCCONbits.IRCF0 = 0;

    // Pin IO Setup
//    OpenADC(ADC_FOSC_8 & ADC_RIGHT_JUST & ADC_12_TAD,
//            ADC_CH0 & ADC_INT_OFF & ADC_REF_VDD_VSS,
//            0x0B); // Four analog pins
    
    // Open timer for Servo
    OpenTimer0( TIMER_INT_ON & T0_16BIT & T0_SOURCE_INT & T0_PS_1_2 );
    ADCON1 = 0x0F;
    TRISA = 0xFF; // All of PORTA input
    TRISB = 0xFF; // All of PORTB input
    TRISC = 0x00; // All of PORTC output
    TRISD = 0x00; // All of PORTD output
    PORTC = 0x00; // Turn off all 8 Port C outputs
    
    RCONbits.IPEN = 1;

    INTCONbits.GIEH = 1;		//  Enable High priority interrupt
    INTCONbits.GIEL = 1;		//  Enable Low priority interrupt

    INTCONbits.TMR0IE = 1;
    INTCON2bits.TMR0IP = 0;
    // This area happens once
    // Good for initializing and things that need to happen once
        OpenUSART(USART_TX_INT_OFF &
            USART_RX_INT_ON &
            USART_ASYNCH_MODE &
            USART_EIGHT_BIT &
            USART_CONT_RX &
            USART_BRGH_HIGH,
            25);
        PIE1bits.RCIE = 1;
        IPR1bits.RCIP = 1;
    WriteTimer0(60535); // 50 hz 60535
    
    PORTD = 0;
    PORTC = STEP1;
    buttonState = 0;
    while (1) {
        if(PORTAbits.RA2 == PRESSED && buttonState == 0){
            buttonState = 1;
            printf("%c", '0');
        } if(PORTAbits.RA3 == PRESSED && buttonState == 0){
            buttonState = 1;
            printf("%c", '1');
        }if(PORTAbits.RA4 == PRESSED && buttonState == 0){
            buttonState = 1;
            printf("%c", '2');
        }if(PORTAbits.RA5 == PRESSED && buttonState == 0){
            buttonState = 1;
            printf("%c", '3');
        }else if(PORTAbits.RA2 == UNPRESSED &&
                PORTAbits.RA3 == UNPRESSED &&
                PORTAbits.RA4 == UNPRESSED &&
                PORTAbits.RA5 == UNPRESSED ) {
            buttonState = 0;
        }
        stepTo(stepTarget);
        PORTDbits.RD2 = buttonState;
        PORTC = 0;
        
    }
}

/*******************************************************************
 * Additional Helper Functions
 ********************************************************************/

/*******************************************************************
 * Function:			void sample(void)
 * Input Variables:	none
 * Output Return:	none
 * Overview:			Use a comment block like this before functions
 ********************************************************************/
void stepOnceForward() {
    
    PORTC = STEP2;
    Delay10KTCYx(DELAY_MS);
    PORTC = STEP3;
    Delay10KTCYx(DELAY_MS);
    PORTC = STEP4;
    Delay10KTCYx(DELAY_MS);
    PORTC = STEP1;
    Delay10KTCYx(DELAY_MS);


}

void stepOnceBackward() {
    // Some function that does a specific task
    PORTC = STEP5;
    Delay10KTCYx(DELAY_MS);
    PORTC = STEP6;
    Delay10KTCYx(DELAY_MS);
    PORTC = STEP7;
    Delay10KTCYx(DELAY_MS);
    PORTC = STEP8;
    Delay10KTCYx(DELAY_MS);
//    PORTC = STEP5;
//    Delay10KTCYx(DELAY_MS);
}

void stepTo(int target){
    if(stepPos == target){
        PORTC = 0;
        return;
    }
    // Go Backwards
    if(target < stepPos){
        for(j=target; j < stepPos; j++){
            stepOnceForward();
        }
    } else {
        for(j=stepPos; j < target; j++){
            stepOnceBackward();
        }
    }
    PORTC = 0;
    stepPos = target;
}

#pragma interrupt high_isr			// declare function as high priority isr
void high_isr(void)
{
    if(PIR1bits.RCIF){
        PIR1bits.RCIF = 0; //clear interrupt flag
        data = RCREG;
        
            
        //stepper vs servo
        if(data == 'A'){
//            stepOnceForward();
            rcvCount = 0;
            which = 1;
            return;
        } else if(data == 'E'){
            rcvCount = 0;
            which = 2;
            return;
        }
        
        //receive loop
        if(rcvCount < 3 && which >0){
            if(which == 1){
                stepDigits[rcvCount] = data -48;
                
            } else{
                servoDigits[rcvCount] = data -48;
            }
            rcvCount++;
        } 
        if(rcvCount == 3){
            // End loop, set value
            if(which == 1){
                stepTarget = stepDigits[0] *100 + stepDigits[1] *10 + stepDigits[2];
                stepTarget = stepTarget/4;
            } else{
                servoPos = servoDigits[0] *100 + servoDigits[1] *10 + servoDigits[2];

            }
            which = 3;
            rcvCount = 0;
        }
 
    }


    
	
}

#pragma interruptlow low_isr

void low_isr(void) {
    // Add code here for the low priority Interrupt Service Routine (ISR)
    if(INTCONbits.TMR0IF)  // Check whether it was the timer interrupt that got us here (better be since that's the only interrupt right now)
	{		
        INTCONbits.TMR0IF = 0;	// Clear interrupt flag for TIMER Zero
        WriteTimer0(59535); // 1000Hz 60535
        PORTDbits.RD3 = 1;
        Delay10TCYx(servoPos); // 1.5ms pulse
        PORTDbits.RD3 = 0;
	} 
}
