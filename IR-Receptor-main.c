/* 
 * File:   IR-Receptor-main.c
 * Author: Mickaël Tansorier <mickatans@orange.fr>
 *
 * Created on 2 mars 2015, 18:09
 * 
 * License CC-BY-NC-SA
 * https://creativecommons.org/licenses/by-nc-sa/3.0/fr/
 * 
 */

/* CONFIGUTATION */
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)
#pragma config CCPMX = RB3      // CCP1 Pin Selection bit
#pragma config DEBUG = OFF      // In-Circuit Debugger Mode bit
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits
#pragma config CPD = OFF        // Data EE Memory Code Protection bit (Data memory code protection off)
#pragma config LVP = OFF        // Low-Voltage Programming Enable bit (RB4/PGM pin has PGM function, low-voltage programming enabled)
#pragma config BOREN = ON       // Brown-out Detect Enable bit (BOD enabled)
#pragma config MCLRE = OFF      // RA5/MCLR/VPP Pin Function Select bit (RA5/MCLR/VPP pin function is MCLR)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config FOSC = INTOSCIO

#define _XTAL_FREQ 8000000 //set your internal(or)external oscillator speed

#include <htc.h>
#include <xc.h>
#include <pic16f87.h>

/* PARAM */
#pragma config CCPMX = RB3         // set PWM pin on RB3 (RB0 by default))

/* ALIAS */
#define BUTTON2 PORTBbits.RB4
#define BUTTON1 PORTBbits.RB5

/* FONCTION */
void init_oscillator    (void);
void init_PWM_Frequency (void);
void init_PWM_duty      (void);
void timer2_start       (void);
void start_PWM          (void);
void stop_PWM           (void);

/*
 * MAIN
 */
main(void) { 
    
    init_oscillator();
    
    /* DEFINE PORTS */
    TRISBbits.TRISB3 = 0;
    TRISBbits.TRISB4 = 1; // button 2 - input
    TRISBbits.TRISB5 = 1; // button 1 - input 

    /* init port */
    PORTBbits.RB3 = 0;

    unsigned signal_precedent_1 = 0;
    unsigned signal_precedent_2 = 0;
    
    /* INIT PWM */
    init_PWM_Frequency();
    init_PWM_duty();
    timer2_start();
        
    /* ACTIVE INTERRUPT */
    INTCONbits.GIE = 1;
    
    
    while(1) 
    { 
        
        /* code: 01010101 */
        if(BUTTON1==1){
            __delay_ms(10);
            if(BUTTON1==1 && signal_precedent_1==0){
                /* start bit */
                start_PWM();
                __delay_ms(10);
                __delay_ms(1);
                /* code */
                
                stop_PWM();
                __delay_ms(1);
                start_PWM();
                __delay_ms(1);
                stop_PWM();
                __delay_ms(1);
                start_PWM();
                __delay_ms(1);
                stop_PWM();
                __delay_ms(1);
                start_PWM();
                __delay_ms(1);
                stop_PWM();
                __delay_ms(1);
                start_PWM();
                __delay_ms(1);
                
                /* stop bit */
                stop_PWM();
                __delay_ms(1);
                
                signal_precedent_1=1;
            }
            else{stop_PWM();}
        }
        else{stop_PWM(); signal_precedent_1=0; __delay_ms(10);}

        
        /* code: 00011000 */
        if(BUTTON2==1){
            __delay_ms(10);
            if(BUTTON2==1 && signal_precedent_2==0){
                /* start bit */
                start_PWM();
                
                /* code */
                __delay_ms(10);
                __delay_ms(1);
                stop_PWM();
                __delay_ms(1);
                stop_PWM();
                __delay_ms(1);
                stop_PWM();
                __delay_ms(1);
                start_PWM();
                __delay_ms(1);
                start_PWM();
                __delay_ms(1);
                stop_PWM();
                __delay_ms(1);
                stop_PWM();
                __delay_ms(1);
                stop_PWM();
                __delay_ms(1);
                
                /* stop bit */
                stop_PWM();

                signal_precedent_2=1;
            }
            else{stop_PWM();}
        }
        else{stop_PWM(); signal_precedent_2=0; __delay_ms(10);}

    } 

}

void init_oscillator(void){
    OSCCONbits.IRCF = 0b111; //8MHz
    OSCCONbits.SCS = 0b00;   //Oscillator mode defined by FOSC
    OSCCONbits.IOFS = 1;     //Frequency is stable
}

void init_PWM_Frequency(void){
    // for 38kHz
    // PR2 = ( PWM Period / ( TOsc x 4 x TMR2 Prescaler ) ) - 1   
    // PR2 = 8 000 000 / (38 000 * 4 * 1) - 1
    // PR2 = 52.6 - 1 = 51.6 = 52   
    // define predivisor T2CKPS1 and T2CKPS0 to 1
    T2CONbits.T2CKPS0 = 0;
    T2CONbits.T2CKPS1 = 0;
    // défine postscaler to 1
    T2CONbits.TOUTPS0 = 0;
    T2CONbits.TOUTPS1 = 0;
    T2CONbits.TOUTPS2 = 0;
    T2CONbits.TOUTPS3 = 0;
    // define PR2
    PR2 = 52; // <=> 0x34   
}

void init_PWM_duty(void){
    // for duty cycle 50%
    // duty cycle with CCPR1L register and to the CCP1CON<5:4> bits
    // CCPR1L:CCP1CON<5:4>
    // PWM Duty Cycle = (CCPR1L:CCP1CON<5:4>) ? TOSC ? (TMR2 Prescale Value)
    // for 50% duty cycle, frequency 38kHz
    // PWM Duty Cycle = 50/100*1/38000 = 0.0000132
    // TOSC = 1/8MHz = 0.000000125
    // CCPR1L:CCP1CON<5:4> = PWM Duty Cycle / TOSC / TMR2 Prescale Value
    // CCPR1L:CCP1CON<5:4> = 0.0000132 / 0.000000125 /1 = 105.6
    // 106 = 0b01101010
    // CCPR1L = 0b11010 and CCP1CON<5:4> = 0b10
    CCP1CONbits.CCP1Y = 0; //CCP1CON<4>
    CCP1CONbits.CCP1X = 1; //CCP1CON<5>
    CCPR1L = 0b11010;
}

void timer2_start(void){
    T2CONbits.TMR2ON = 1; // Timer2 On bit
    PIE1bits.TMR2IE = 1;  // TMR2 to PR2 Match Interrupt Enable bit
    PIR1bits.TMR2IF = 0;  // TMR2 to PR2 Interrupt Flag bit
}

void start_PWM(void){
    CCP1CONbits.CCP1M = 0b1111; //11xx = PWM mode
}

void stop_PWM(void){
    CCP1CONbits.CCP1M = 0b0000;
}
   
