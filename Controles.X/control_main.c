/*
 * File:   control_main.c
 * Author: angel
 *
 * Created on 11 de octubre de 2021, 05:53 PM
 */

// PIC16F887 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF        // Watchdog Timer Enable bit (WDT enabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF       // RE3/MCLR pin function select bit (RE3/MCLR pin function is MCLR)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF       // Brown Out Reset Selection bits (BOR enabled)
#pragma config IESO = OFF        // Internal External Switchover bit (Internal/External Switchover mode is enabled)
#pragma config FCMEN = OFF       // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is enabled)
#pragma config LVP = OFF         // Low Voltage Programming Enable bit (RB3/PGM pin has PGM function, low voltage programming enabled)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdint.h>
#include "funciones.h"
#define  _XTAL_FREQ 8000000

///////////////// declaración de variables /////////////////////////////////////

unsigned char accion;
unsigned char jugar = '0'; 
uint8_t bloqueo = 0; 

//////////////////////// subrutina de interrupcion /////////////////////////////

void __interrupt()isr(void){
    
    // interrucion por medio de bandera del ON CHANGE.
    if (RBIF == 1){
        if (RB0 == 0) {
            accion = '1';    // star
            bloqueo = 1;  // dejo que la comunicacion serial envie los datos
        }
        else if (RB1 == 0) {
            accion = '2';    // izquierda  
            bloqueo = 1;  // dejo que la comunicacion serial envie los datos
        }
        else if (RB2 == 0) {
            accion = '3';    // derecha 
            bloqueo = 1;  // dejo que la comunicacion serial envie los datos
        }
    INTCONbits.RBIF = 0; // luego de todo esto, bajo la bandera de ON CHANGE.
    }
    // interrucion por recepcion de informacion USART 
    if (RCIF == 1) {
        jugar = RCREG;        // lo que reciba de la PC lo pongo en la variable
        RCIF = 0;             // bajo la bandera 
    }
    return; 
}

void main(void) {
    
///////////////////// configuraciones de pines del pic /////////////////////////
 
ANSEL = 0x00; 
ANSELH = 0x00;

TRISA = 0x00; 
TRISB = 0b00000111;
TRISC = 0b10000000;
TRISD = 0x00;
TRISE = 0x00;

PORTA = 0x00;
PORTB = 0x00;
PORTC = 0x00;
PORTD = 0x00;
PORTE = 0x00;

// configuracion de PULL UP internos en PORTB///////////////////////////////////

OPTION_REGbits.nRBPU = 0;       
IOCBbits.IOCB0 = 1;        // configuracion de los PULL UP internos en los
IOCBbits.IOCB1 = 1;        // pines RB0,RB1 y RB2
IOCBbits.IOCB2 = 1;        
WPUB = 0b00000111;

//////////////////////// configuracion del oscilador interno ///////////////////

/* en este caso voy a configurarlo para que opere a 8 Mhz por lo que sera la 
 opcion 8 que le ingrese a la funcion de la libreria. */

conf_OSCCON(8);

//////////////// configuracion de comunicacion USART  //////////////////////////

conf_USART();

/*  habilitando las banderas de interrupcion para USART y ON CHANGE*/

INTCONbits.GIE = 1;
INTCONbits.RBIE = 1;
INTCONbits.RBIF = 0;  // interrupciones del ON CHANGE 
INTCONbits.PEIE = 1;
PIE1bits.RCIE = 1; 
PIR1bits.RCIF = 0;   // USART

////////////////////////////// LOOP principal //////////////////////////////////

while (1) {
    
    if (jugar == '0') {
        PORTA = 0x00; // para led para indicar que el control esta deshabilitado
        accion = '0'; // dejo en cero la variable de accion 
    }
    // este bloqueo me sirve para enviar el comando un sola vez cuando haya un 
    // botonazo en el control
    else if (bloqueo == 1) {
        
        bloqueo = 0; // despues de enviar la informacion bloqueo otra vez el
        // envio de informacion hasta un nuevo botonazo.
        
        PORTA = 0x01;  // enciendo el para indicar que el control esta activo. 
 
        if (TXIF == 1) {       // mientras pueda enviar informacion le envio el
            TXREG = accion;      // valor si se mueve a la derecha o izquierda
            accion = '0';
            TXREG = accion;
        }
    }               
}
return; 
}