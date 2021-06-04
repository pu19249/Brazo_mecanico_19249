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
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit 
//(Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit 
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
char contador, valor;
char pot2, pot3, pot4;
char pot2_nuevo, pot3_nuevo, pot4_nuevo;
char estado_motor;
char estado_motor_nuevo;
char pasar_a_uart;
/*==============================================================================
                               INTERRUPCIONES Y PROTOTIPOS
 =============================================================================*/
void setup(void);
void servo_1_1(void);
void servo_1_2(void);
void servo_1_3(void);
void servo_1_4(void);
void servo_1_5(void);
void servo_1_6(void);
void servo_2_1(void);
void servo_2_2(void);
void servo_2_3(void);
void servo_2_4(void);
void servo_2_5(void);
void servo_3_1(void);
void servo_3_2(void);
void servo_3_3(void);
void servo_3_4(void);
void servo_3_5(void);
void motor_1(void);
void motor_2(void);
void motor_detenido(void);
void putch(char dato);
void mensaje(void);

void escribir_eeprom(char data, char address);
char leer_eeprom(char address);


void __interrupt() isr(void){
    if (ADIF == 1){
        //revisar cual es el que termino de convertirse
        if (ADCON0bits.CHS == 0){
            ADCON0bits.CHS = 1;
            if (RB2 == 0){
                motor_detenido();
                estado_motor = 0b11;
            }
            else if (RB0 == 0){
                motor_1();
                estado_motor = 0b01;
            }
            else if (RB1 == 0){
                motor_2();
                estado_motor = 0b10;
            }
            else {
                RE0 = 0;
                RE1 = 0;
                RE2 = 0;
            }    
        }
        else if (ADCON0bits.CHS == 1){
            ADCON0bits.CHS = 2;
            pot2 = ADRESH;
            if (pot2<=50){ //de 0 a 50 en el pot va a tener -90 grados
                servo_1_1();
            }
            if ((pot2<=101) && (pot2>=51)){
                servo_1_2();
              
            }
            if ((pot2<=152) && (pot2>=102)){
                servo_1_3();
            }
            if ((pot2<=203) && (pot2>=153)){
                servo_1_4();
            }
            if (pot2>=204){
                servo_1_5();
            }
        }
        else if (ADCON0bits.CHS == 2){
            ADCON0bits.CHS = 3;
            pot3 = ADRESH;
            if (pot3<=50){ //de 0 a 50 en el pot va a tener -90 grados
                servo_2_1();
            }
            if ((pot3<=101) && (pot3>=51)){
                servo_2_2();
            }
            if ((pot3<=152) && (pot3>=102)){
                servo_2_3();
            }
            if ((pot3<=203) && (pot3>=153)){
                servo_2_4();
            }
            if (pot3>=204){
                servo_2_5();
            }
        }
        
        else if (ADCON0bits.CHS == 3){
            ADCON0bits.CHS = 0;
            pot4 = ADRESH;
            if (pot4<=50){ //de 0 a 50 en el pot va a tener -90 grados
                servo_3_1();
            }
            if ((pot4<=101) && (pot4>=51)){
                servo_3_2();
            }
            if ((pot4<=152) && (pot4>=102)){
                servo_3_3();
            }
            if ((pot4<=203) && (pot4>=153)){
                servo_3_4();
            }
            if (pot4>=204){
               servo_3_5();
            }
        }
        ADIF = 0;           //apaga la bandera
        __delay_us(100);
    }
    
    if (RBIF == 1){ //alguno de los dos botones que tengo se presiono
        if (RB3 == 0){ //porque es pull-up
            RB6 = 0;
            RB5 = 1;
            escribir_eeprom(pot2, 0x16);
            escribir_eeprom(pot3, 0x17);
            escribir_eeprom(pot4, 0x18);
            escribir_eeprom(estado_motor, 0x19);
            __delay_ms(500);
            RB5 = 0;
        }
        if (RB4 == 0){
            //ADCON0bits.GO = 0;
            //ADCON0bits.ADON = 0;
            RB5 = 0;
            RB6 = 1;
            pot2_nuevo = leer_eeprom(0x16);
            pot3_nuevo = leer_eeprom(0x17);
            pot4_nuevo = leer_eeprom(0x18);
            estado_motor_nuevo = leer_eeprom(0x19);
            if (pot2_nuevo<=50){ //de 0 a 50 en el pot va a tener -90 grados
                servo_1_1();
                pot2 = 30; //cargo manualmente el rango para que no haga
            }
            if ((pot2_nuevo<=101) && (pot2_nuevo>=51)){
                servo_1_2();
                pot2 = 70; //conflicto entre la posicion guardada
            }
            if ((pot2_nuevo<=152) && (pot2_nuevo>=102)){
                servo_1_3();
                pot2 = 130; //y la posicion actual del servo
            }
            if ((pot2_nuevo<=203) && (pot2_nuevo>=153)){
                servo_1_4();
                pot2 = 170;
            }
            if (pot2_nuevo>=204){
                servo_1_5();
                pot2 = 205;
            }
            if (pot3_nuevo<=50){ //de 0 a 50 en el pot va a tener -90 grados
                servo_2_1();
                pot3 = 30;
            }
            if ((pot3_nuevo<=101) && (pot3_nuevo>=51)){
                servo_2_2();
                pot3 = 70;
            }
            if ((pot3_nuevo<=152) && (pot3_nuevo>=102)){
                servo_2_3();
                pot3 = 110;
            }
            if ((pot3_nuevo<=203) && (pot3_nuevo>=153)){
                servo_2_4();
                pot3 = 170;
            }
            if (pot3_nuevo>=204){
                servo_2_5();
                pot3 = 205;                        
            }
            if (pot4_nuevo<=50){ //de 0 a 50 en el pot va a tener -90 grados
                servo_3_1();
                pot4 = 30;
            }
            if ((pot4_nuevo<=101) && (pot4_nuevo>=51)){
                servo_3_2();
                pot4 = 70;
            }
            if ((pot4_nuevo<=152) && (pot4_nuevo>=102)){
                servo_3_3();
                pot4 = 130;
            }
            if ((pot4_nuevo<=203) && (pot4_nuevo>=153)){
                servo_3_4();
                pot4 = 170;
            }
            if (pot4_nuevo>=204){
               servo_3_5();
               pot4 = 205;
            }
            if (estado_motor_nuevo == 0b11){
                motor_detenido();
                
            }
            if (estado_motor_nuevo == 0b01){
                motor_1();
                
            }
            if (estado_motor_nuevo == 0b10){
                motor_2();
                
            }
        __delay_ms(2000);
        //ADCON0bits.GO = 1;
        RB6 = 0;
    }
        if (RB7 == 0){
            pasar_a_uart = 1;
            while(pasar_a_uart == 1){
          
                mensaje();
            }
//            if (RB7 == 0){ //si se volvio a presionar el boton
//                pasar_a_uart = 0; //que salga de modo uart
//            }
        }
        RBIF = 0; //limpio la interrupcion para que salga de aqui
        
        //__delay_ms(100);
        //ADCON0bits.ADON = 1;
        
    }

}
/*==============================================================================
                                LOOP PRINCIPAL
 =============================================================================*/
void main(void){
    setup();
     
    while(1){
            
        ADCON0bits.GO = 1; //inicia la conversion otra vez
        
        }
    
    
}

/*==============================================================================
                                    FUNCIONES
 =============================================================================*/

void escribir_eeprom(char data, char address){
    EEADR = address;        //la direccion de memoria que voy a escribir
    EEDAT = data;           //el valor que voy a escribir en la memoria
    
    EECON1bits.EEPGD = 0;   //apuntar a la DATA memory
    EECON1bits.WREN = 1;    //habilita la escritura
    
    INTCONbits.GIE = 0;     //deshabilita las interrupciones globales
    //rutina default
    EECON2 = 0x55;          //por la secuencia de 1-0 que da este valor
    EECON2 = 0xAA;          //y este en binario
    
    EECON1bits.WR = 1;      //inicia la escritura
    
    while(PIR2bits.EEIF == 0);  //Para que espere el final de la escritura
    PIR2bits.EEIF = 0;          //vuelve a apagar la bandera
    
    EECON1bits.WREN = 0;        //Para asegurar que no se esta escribiendo
    INTCONbits.GIE = 0;         //vuelve a habilitar las interrupciones globales
    return;
}

char leer_eeprom(char address){
    EEADR = address;            //el byte de la memoria que va a leer
    EECON1bits.EEPGD = 0;       //apuntar a la PROGRAM memory
    EECON1bits.RD = 1;          //se indica que es lectura
    char data = EEDATA;         //guarda en la variable el dato
    return data;                //la funcion regresa el char
}

void servo_1_1(void){           //rango de posicion 1 para el servo1
    RD0 = 1;
    __delay_ms(0.7);
    RD0 = 0;
    __delay_ms(19.3);
}

void servo_1_2(void){           //rango de posicion 2 para el servo1
    RD0 = 1;
    __delay_ms(1.25);
    RD0 = 0;
    __delay_ms(18.75);
}

void servo_1_3(void){           //rango de posicion 3 para el servo1
    RD0 = 1;
    __delay_ms(1.5);
    RD0 = 0;
    __delay_ms(18.5);
}

void servo_1_4(void){           //rango de posicion 4 para el servo1
    RD0 = 1;
    __delay_ms(1.75);
    RD0 = 0;
    __delay_ms(18.25);
}

void servo_1_5(void){           //rango de posicion 5 para el servo1
    RD0 = 1;
    __delay_ms(2);
    RD0 = 0;
    __delay_ms(18);
}

void servo_2_1(void){           //rango de posicion 1 para el servo2
    RD1 = 1;
    __delay_ms(0.7);
    RD1 = 0;
    __delay_ms(19.3);
}

void servo_2_2(void){           //rango de posicion 2 para el servo2
    RD1 = 1;
    __delay_ms(1.25);
    RD1 = 0;
    __delay_ms(18.75);
}

void servo_2_3(void){           //rango de posicion 3 para el servo2
    RD1 = 1;
    __delay_ms(1.5);
    RD1 = 0;
    __delay_ms(18.5);
}

void servo_2_4(void){           //rango de posicion 4 para el servo2
    RD1 = 1;
    __delay_ms(1.75);
    RD1 = 0;
    __delay_ms(18.25);
}

void servo_2_5(void){           //rango de posicion 5 para el servo2
    RD1 = 1;
    __delay_ms(2);
    RD1 = 0;
    __delay_ms(18);
}

void servo_3_1(void){           //rango de posicion 1 para el servo3
    RD2 = 1;
    __delay_ms(0.7);
    RD2 = 0;
    __delay_ms(19.3);
}

void servo_3_2(void){           //rango de posicion 2 para el servo3
    RD2 = 1;
    __delay_ms(1.25);
    RD2 = 0;
    __delay_ms(18.75);
}

void servo_3_3(void){           //rango de posicion 3 para el servo3
    RD2 = 1;
    __delay_ms(1.5);
    RD2 = 0;
    __delay_ms(18.5);
}

void servo_3_4(void){           //rango de posicion 4 para el servo3
    RD2 = 1;
    __delay_ms(1.75);
    RD2 = 0;
    __delay_ms(18.25);
}

void servo_3_5(void){           //rango de posicion 5 para el servo3
    RD2 = 1;
    __delay_ms(2);
    RD2 = 0;
    __delay_ms(18);
}

void motor_detenido(void){
    CCPR1L = 0;
    CCPR2L = 0;
    RE2 = 1;
}

void motor_1(void){
    CCPR1L = ADRESH/4;
    CCPR2L = 0;
    RE0 = 1;
}

void motor_2(void){
    CCPR1L = 0;
    CCPR2L = ADRESH/4;
    RE1 = 1;
}

void putch(char dato){
    while(TXIF == 0);
    TXREG = dato; //transmite los datos al recibir un printf en alguna  parte 
    return;
}

void mensaje(void){
    __delay_ms(500); //para que despliegue los datos en el tiempo correcto
    printf("\r Que accion desea ejecutar \r");
    __delay_ms(250);
    printf("(1) Mover servos \r");
    __delay_ms(250);
    printf("(2) Mover motor \r");
    __delay_ms(250);
    printf("(3) Salir del modo UART \r");
    while (RCIF == 0);
    if (RCREG == '1'){
        __delay_ms(500);
        printf("\r Cual de los servos desea mover?\r");
        __delay_ms(250);
        printf("\r a. Servo 1");
        __delay_ms(250);
        printf("\r b. Servo 2");
        __delay_ms(250);
        printf("\r c. Servo 3");
        while (RCIF == 0); //esperar una respuesta
        if (RCREG == 'a'){
            printf("\r Seleccione entre los siguiente para dar la posicion:");
            __delay_ms(250);
            printf("\r q: -45 ");
            __delay_ms(250);
            printf("\r w: -90 ");
            __delay_ms(250);
            printf("\r e: 0 ");
            __delay_ms(250);
            printf("\r r: 90 ");
            __delay_ms(250);
            printf("\r t: 45 ");
            __delay_ms(250);
            while (RCIF == 0);
            if (RCREG == 'q'){
                servo_1_1();                                  
            }
            if (RCREG == 'w'){
                servo_1_2();
                
            }
            if (RCREG == 'e'){
                servo_1_3();
                
            }
            if (RCREG == 'r'){
                servo_1_4();
                
            }
            if (RCREG == 't'){
                servo_1_5();
                
            }
            //while (RCIF == 0);
        }
        if (RCREG == 'b'){
            printf("\r Seleccione entre los siguiente para dar la posicion:");
            __delay_ms(250);
            printf("\r z: -45 ");
            __delay_ms(250);
            printf("\r x: -90 ");
            __delay_ms(250);
            printf("\r g: 0 ");
            __delay_ms(250);
            printf("\r v: 90 ");
            __delay_ms(250);
            printf("\r b: 45 ");
            __delay_ms(250);
            while (RCIF == 0);
            if (RCREG == 'z'){
                servo_2_1();
            }
            if (RCREG == 'x'){
                servo_2_2();
            }
            if (RCREG == 'g'){
                servo_2_3();
            }
            if (RCREG == 'v'){
                servo_2_4();
            }
            if (RCREG == 'b'){
                servo_2_5();
            }
            //while (RCIF == 0);
        }
        if (RCREG == 'c'){
            printf("\r Seleccione entre los siguiente para dar la posicion:");
            __delay_ms(250);
            printf("\r p: -45 ");
            __delay_ms(250);
            printf("\r o: -90 ");
            __delay_ms(250);
            printf("\r i: 0 ");
            __delay_ms(250);
            printf("\r u: 90 ");
            __delay_ms(250);
            printf("\r y: 45 ");
            __delay_ms(250);
            while (RCIF == 0);
            if (RCREG == 'p'){
                servo_3_1();
            }
            if (RCREG == 'o'){
                servo_3_2();
            }
            if (RCREG == 'i'){
                servo_3_3();
            }
            if (RCREG == 'u'){
                servo_3_4();
            }
            if (RCREG == 'y'){
                servo_3_5();
            }
            //while (RCIF == 0);
        }
    }
    if (RCREG == '2'){ //segunda opcion del menu
        printf("\r Hacia donde desea mover el motor? \r");
        __delay_ms(250);
        printf("\r r: Derecha");
        __delay_ms(250);
        printf("\r l: Izquierda");
        __delay_ms(250);
        printf("\r s: Detener");
        __delay_ms(250);
        while (RCIF == 0);
        if (RCREG == 'r'){
            motor_1();
        }
        if (RCREG == 'l'){
            motor_2();
        }
        if (RCREG == 's'){
            motor_detenido();
        }
    }
    if (RCREG == '3'){ //tercera opcion del menu
        pasar_a_uart = 0;
        printf("\r Ha finalizado la comunicacion UART");
        __delay_ms(250);
    }
    else{ //cualquier otra opcion que no este en el menu
        NULL;
    }
    return;
}    

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
    
    //TRISCbits.TRISC2 = 0; //full bridge P1A
    TRISDbits.TRISD5 = 0; //full bridge P1B
    TRISDbits.TRISD6 = 0; //full bridge P1C
    TRISDbits.TRISD7 = 0; //full bridge P1D
    
    //Configuracion de internal pull-ups para los botones
    OPTION_REGbits.nRBPU = 0; //internal pull-ups are enabled
    WPUBbits.WPUB0 = 1;   //boton1
    WPUBbits.WPUB1 = 1;   //boton2
    WPUBbits.WPUB2 = 1;   //boton3
    WPUBbits.WPUB3 = 1;   //boton4
    WPUBbits.WPUB7 = 1;   //para que no se encienda el rb7   
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
    CCP1CONbits.P1M = 0b00;    //single output
    CCP2CONbits.CCP2M = 0b1111; //para que sea PWM
    CCP1CONbits.CCP1M = 0b00001100; //PWM mode, P1A, P1C active-high
    
    
    CCPR1L = 0x0F;          //ciclo de trabajo
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
//    INTCONbits.T0IE = 1;    //habilita la interrupcion del timer0
//    INTCONbits.T0IF = 0;    //limpia bit de int del timer 0
//    
//    //configurar el timer0
//    OPTION_REGbits.T0CS = 0;     //oscilador interno
//    OPTION_REGbits.PSA = 0;      //prescaler asignado al timer0
//    OPTION_REGbits.PS0 = 1;      //prescaler tenga un valor 1:16
//    OPTION_REGbits.PS1 = 1;
//    OPTION_REGbits.PS2 = 0;
//    TMR0 = 176;
    
    //Configuracion de interrupcion del puerto B
    IOCBbits.IOCB3 = 1;     //Boton de escritura
    IOCBbits.IOCB4 = 1;     //Boton de lectura
    IOCBbits.IOCB7 = 1;     //Boton de UART
    INTCONbits.RBIE = 1;
    INTCONbits.RBIF = 0;    //limpiar bandera de interrupcion
    
    //configurar transmisor y receptor asincrono
    SPBRG = 103;         //para el baud rate de 600
    SPBRGH = 0;
    BAUDCTLbits.BRG16 = 1; //8bits baud rate generator is used
    TXSTAbits.BRGH = 1; //high speed
    
    TXSTAbits.SYNC = 0; //asincrono
    //serial port enabled (Configures RX/DT and TX/CK pins as serial)
    RCSTAbits.SPEN = 1; 
    RCSTAbits.CREN = 1; //habilitar la recepcion
    
    TXSTAbits.TX9 = 0; //transmision de 8bits
    TXSTAbits.TXEN = 1; //enable the transmission
    RCSTAbits.RX9 = 0; //recepcion de 8 bits
      
    //PIE1bits.TXIE = 1; //porque quiero las interrupciones de la transmision
    INTCONbits.GIE = 1; //enable de global interrupts
    INTCONbits.PEIE = 1;
    //PIE1bits.RCIE = 1; //interrupciones del receptor
    PIR1bits.TXIF = 0;  //limpiar interrupciones
    PIR1bits.RCIF = 0;
    
    //Limpiar puertos
    PORTA = 0x00;
    PORTB = 0x00;
    PORTC = 0x00;
    PORTD = 0x00;
    PORTE = 0x00;
    
    return;
}