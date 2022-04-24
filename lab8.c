/*
 * File:   lab.c
 * Author: Melanie Samayoa
 *
 * Creado en abril 23, 2021, 03:44 PM
 */


// PIC16F887 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is enabled)
#pragma config LVP = ON         // Low Voltage Programming Enable bit (RB3/PGM pin has PGM function, low voltage programming enabled)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.


#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <stdint.h>  // Para poder usar los int de 8 bits

#define _XTAL_FREQ 500000

char unidades;     
char decenas;     // Variable decenas
char centenas;      // Variable centenas
char tmr0;    // Variable de bandera de timer
int potenciometro;  
int sen1;     
int sen2;    

char valores[10] = {0B00111111, 0B00000110, 0B01011011, 0B01001111, 
0B01100110, 0B01101101, 0B01111101, 0B00000111, 0B01111111, 0B01100111};

void setup(void);
void calculos(void);

void __interrupt() isr(void){
    if(INTCONbits.T0IF){        //Interrupcion de timer0
        INTCONbits.T0IF = 0;
        TMR0 = 255;             // Valor de timer0
        
        switch(tmr0){              //Multiplexeo de displays
            case 0:
                PORTE = 0;
                PORTC = valores[centenas];   // Display de centenas
                PORTEbits.RE0 = 1;
                PORTCbits.RC7 = 1;
                tmr0++;
                break;
                
            case 1:
                PORTE = 0;
                PORTC = valores[decenas];  // Display de decenas
                PORTEbits.RE1 = 1;
                tmr0++;
                break;
                
            case 2:
                PORTE = 0;
                PORTC = valores[unidades];       // Display de unidades
                PORTEbits.RE2 = 1;
                tmr0 = 0;
                break;
        }
        INTCONbits.T0IF = 0;        // Reinicio del timer0
    }
    if(PIR1bits.ADIF){              
        if(ADCON0bits.CHS == 11){   
            PORTA = ADRESH;        
        }
        else
            PORTD = ADRESH;         
        PIR1bits.ADIF = 0;
    }
}

void main(void){
    setup();
    __delay_us(50);
    ADCON0bits.GO = 1;          
    while(1){
        if(ADCON0bits.GO == 0){
            if(ADCON0bits.CHS == 11)
                ADCON0bits.CHS = 10;
            else 
                ADCON0bits.CHS = 11;
            __delay_us(50);
            ADCON0bits.GO = 1;      
        }
        calculos();
    }
}

void calculos(void){
    sen1 = PORTD;          
    potenciometro = (sen1 << 2);     // Mover bits, mutiplicar por 4
    sen2 = potenciometro >> 1;        // Mover a la derecha para divir entre 2 
    
    centenas = sen2/100;                   
    decenas = (sen2-(centenas*100))/10;       
    unidades = (sen2-(centenas*100+decenas*10));   
    return;
}

void setup(void){
    ANSEL = 0;
    ANSELH = 0b1100;    
    
    TRISA = 0;          // Puerto A como salida
    TRISB = 1;          // Puerto B como entrada
    
    TRISC = 0;          // Puerto C como salida
    TRISD = 0;          // Puerto D como salida
    TRISE = 0;          // Puerto E como salida
    
    PORTA = 0;          // Se limpia el puerto A
    PORTB = 0;          // Se limpia el puerto B
    PORTC = 0;          // Se limpia el puerto C
    PORTD = 0;          // Se limpia el puerto D
    PORTE = 0;          // Se limpia el puerto E
    
    OSCCONbits.IRCF = 0b0011; // Oscilador configurdo a 500KHz
    OSCCONbits.SCS = 1; 
    
     //Timer0 Registers Prescaler= 256 - TMR0 Preset = 60 - Freq = 9.96 Hz - Period = 0.100352 seconds
    OPTION_REGbits.T0CS = 0;  
    OPTION_REGbits.T0SE = 0;  
    OPTION_REGbits.PSA = 0;   
    OPTION_REGbits.PS2 = 1;   
    OPTION_REGbits.PS1 = 1;
    OPTION_REGbits.PS0 = 1;
    TMR0 = 255;
    
    INTCONbits.T0IF = 0;        // Limpiar bandera del timer0
    INTCONbits.T0IE = 1;        // Se habilita la interrupcion del timer0
    
    ADCON0bits.ADCS = 0b00;     
    ADCON0bits.CHS = 10;        
    
    ADCON1bits.ADFM = 0;        
    ADCON1bits.VCFG0 = 0;      
    ADCON1bits.VCFG1 = 0;       
        
    PIR1bits.ADIF = 0;          
    PIE1bits.ADIE = 1;          // Interrupciones analogicas habilitadas
    INTCONbits.PEIE = 1;        // Interrupciones externas habilitadas
    INTCONbits.GIE = 1;         // Interrupciones globales habilitadas
    
    __delay_us(50);             
    ADCON0bits.ADON = 1;         
    
    return;
}