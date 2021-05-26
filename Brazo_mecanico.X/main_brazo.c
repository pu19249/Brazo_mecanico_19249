/*
 * Archivo:   main_brazo.c
 * Autor: Jonathan Pu c.19249
 * Descripcion de hardware: 3 servomotores para el control de los brazos, 
 * 1 motor reductor para mover la base del brazo, potenciometros para el control
 * actuadores digitales para grabar posicion default
 * Descripcion de software: control de servos con funciones de bitbanging,
 * control de motor DC con modulo PWM y comunicacion serial
 * Created on May 19, 2021, 12:10 AM
 */

#include <xc.h>
#include <stdint.h>
#include <stdio.h> //libreria para mandar str en comunicacion serial
#include <pic16f887.h> 
#define _XTAL_FREQ 4000000
/*=============================================================================
                        BITS DE CONFIGURACION
 =============================================================================*/
// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO 
//oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/
//CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and 
//can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR 
//pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code 
//protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code 
//protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal
///External Switchover mode is disabled)
#pragma config FCMEN = ON      // Fail-Safe Clock Monitor Enabled bit 
//(Fail-Safe Clock Monitor is disabled)
#pragma config LVP = ON         // Low Voltage Programming Enable bit 
//(RB3/PGM pin has PGM function, low voltage programming enabled)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit 
//(Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits 
//(Write protection off)

/*==============================================================================
                                VARIABLES
 =============================================================================*/
char motor, direccion;
/*==============================================================================
                               INTERRUPCIONES Y PROTOTIPOS
 =============================================================================*/
void setup(void);

//void __interrupt() isr(void){
//    if (ADIF == 1){
//        //revisar cual es el que termino de convertirse
//        if (ADCON0bits.CHS == 0){
//            motor = ADRESH;
//            CCPR1L = motor;}
//    
//        if (ADCON0bits.CHS == 1){ //el otro canal de PWM
//            CCPR2L = ADRESH;
//        }
//    }
//        ADIF = 0;           //apaga la bandera
//        
//    }

/*==============================================================================
                                LOOP PRINCIPAL
 =============================================================================*/
void main(void){
    setup();
    direccion = 0;
    motor = 0;
    
    
    //while(1){
        for(;;){
        if (RB0 == 0){
                if (direccion == 0){
                    direccion = 1;
                    P1M1 = 1;
                    P1M0 = 1;
                }
                else {
                        direccion = 0;
                        P1M1 = 0;
                        P1M0 = 0;
                    }
                }
        GO = 1;
        while(GO);
        motor = ADRESH;
        CCPR1L = motor;
//        //cambio de canales para la conversin analogica
//        if (ADCON0bits.GO == 0){       //si estaba en el canal0
//            
//            if (ADCON0bits.CHS == 1){    
//                ADCON0bits.CHS = 0;     //
//                
//            }
//            if (ADCON0bits.CHS == 2){
//                ADCON0bits.CHS = 3;   //si es otro cambiamos el canal de nuevo
//                
//            }
//            if (ADCON0bits.CHS == 3){
//                ADCON0bits.CHS = 0;
//            }
//        __delay_us(100);
//        ADCON0bits.GO = 1; //inicia la conversion otra vez
//        }
//    }
//}
    }   }
/*==============================================================================
                                    FUNCIONES
 =============================================================================*/


/*==============================================================================
                            CONFIGURACION DE PIC
 =============================================================================*/
void setup(void){
    //Configuracion de puertos de entrada digital
    TRISBbits.TRISB0 = 1; //boton para motor a la derecha
    TRISBbits.TRISB1 = 1; //boton para motor a la izquierda
    TRISBbits.TRISB2 = 1; //boton para motor detenido
    TRISBbits.TRISB3 = 1; //boton para escribir en eeprom
    TRISBbits.TRISB4 = 1; //boton para leer de eeprom
    
    //Entradas analogicas
    ANSELbits.ANS0 = 1;   //Pot para servo1
    ANSELbits.ANS1 = 1;   //Pot para servo2
    ANSELbits.ANS2 = 1;   //Pot para servo3
    ANSELbits.ANS3 = 1;   //Pot para motor
    ANSELH = 0x00;
    TRISAbits.TRISA0 = 1;
    TRISAbits.TRISA1 = 1;
    TRISAbits.TRISA2 = 1;
    TRISAbits.TRISA3 = 1;
    
    
    
    //Configuracion de salidas digitales
    TRISBbits.TRISB5 = 0; //led indicador de escritura eeprom
    TRISBbits.TRISB6 = 0; //led indicador de lectura eeprom
    TRISEbits.TRISE0 = 0; //led indicador de derecha
    TRISEbits.TRISE1 = 0; //led indicador de izquierda
    TRISEbits.TRISE2 = 0; //led indicador de parada
    
    TRISDbits.TRISD0 = 0; //salida de servo1
    TRISDbits.TRISD1 = 0; //salida de servo2
    TRISDbits.TRISD2 = 0; //salida de servo3
    
    TRISDbits.TRISD5 = 0; //full bridge P1B
    TRISDbits.TRISD6 = 0; //full bridge P1C
    TRISDbits.TRISD7 = 0; //full bridge P1D
    
    //Configuracion de internal pull-ups para los botones
    OPTION_REGbits.nRBPU = 0; //internal pull-ups are enabled
    WPUBbits.WPUB0 = 1;   //boton1
    WPUBbits.WPUB1 = 1;   //boton2
    WPUBbits.WPUB2 = 1;   //boton3
    WPUBbits.WPUB3 = 1;   //boton4
    WPUBbits.WPUB7 = 0;   //para que no se encienda el rb7   
    //Configurar reloj interno
    OSCCONbits.IRCF0 = 0;        //reloj interno de 4mhz
    OSCCONbits.IRCF1 = 1;
    OSCCONbits.IRCF2 = 1;
    OSCCONbits.SCS = 1;  //internal oscillator is used for system clock
    
     //configurar el modulo ADC
    ADCON0bits.CHS = 0;     //canal 0
    __delay_us(100);
    
    ADCON0bits.ADCS0 = 1;   //para que el clock select sea FOSC/8
    ADCON0bits.ADCS1 = 0;   
    ADCON0bits.ADON = 1;    //ADC enable bit
    ADCON1bits.ADFM = 0;    //left justified
    ADCON1bits.VCFG1 = 0;   //5 voltios
    ADCON1bits.VCFG0 = 0;   //tierra
    
    
    //configuracion del PWM junto con el TMR2
    TRISCbits.TRISC2 = 1;   //habilitar momentaneamente el pin de salida
    TRISCbits.TRISC1 = 1;
    PR2 = 125;               //queremos que sea de 20ms por el servo
    CCP1CONbits.P1M = 0b01;    //full bridge output forward  
    CCP2CONbits.CCP2M = 0b1111; //para que sea PWM
    CCP1CONbits.CCP1M = 0b00001100; //PWM mode, P1A, P1C active-high
    
    
    //CCPR1L = 0x0F;          //ciclo de trabajo
    CCP1CONbits.DC1B = 0;   //los bits menos significativos
    CCPR2L = 0x0F;
    CCP2CONbits.DC2B0 = 0; 
    CCP2CONbits.DC2B1 = 0;
    
    
    PIR1bits.TMR2IF = 0;    //limpiar la interrupcion del timer2
    T2CONbits.T2CKPS0 = 0;   //configurar el prescaler a 16
    T2CONbits.T2CKPS1 = 1;        
    T2CONbits.TMR2ON = 1;   //habilitar el tmr2on
    while (PIR1bits.TMR2IF == 0);
    PIR1bits.TMR2IF = 0;    //limpiar nuevamente
    TRISCbits.TRISC2 = 0;   //regresar el pin a salida
    TRISCbits.TRISC1 = 0;
  
    //configurar interrupciones
    PIE1bits.ADIE = 1;      //enable de la int del ADC
    PIR1bits.ADIF = 0;      //limpiar la interrupcion del ADC
    INTCONbits.GIE = 1;     //habilita las interrupciones globales
    INTCONbits.PEIE = 1;    //periferical interrupts
    
    
    //Limpiar puertos
    PORTA = 0x00;
    PORTB = 0x00;
    PORTC = 0x00;
    PORTD = 0x00;
    PORTE = 0x00;
    
    return;
}