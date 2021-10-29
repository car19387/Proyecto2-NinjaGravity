/*
 * File:   funciones.c
 * Author: angel
 *
 * Created on 11 de octubre de 2021, 05:52 PM
 */

#include <xc.h>
#include <stdint.h>
#include "funciones.h"
#define  _XTAL_FREQ 8000000

// funcion encargada de programar el oscilador interno el PIC y seleccionar la 
// frecuencia de uso deseada. 
void conf_OSCCON(uint8_t frecuencia){
    
    switch (frecuencia) {
        case (1):
            OSCCONbits.IRCF = 0b000;     // para frec = 31 Khz 
            break;
        case (2):
            OSCCONbits.IRCF = 0b001;     // para frec = 125 Khz
            break;
        case (3):
            OSCCONbits.IRCF = 0b010;     // para frec = 250 Khz
            break;
        case (4):
            OSCCONbits.IRCF = 0b011;     // para frec = 500 Khz
            break;
        case (5):
            OSCCONbits.IRCF = 0b100;     // para frec = 1 Mhz 
            break;
        case (6):
            OSCCONbits.IRCF = 0b101;     // para frec = 2 Mhz
            break;
        case (7):
            OSCCONbits.IRCF = 0b110;     // para frec = 4 Mhz
            break;
        case (8):
            OSCCONbits.IRCF = 0b111;     // para frec = 8 Mhz
            break;
    }       
    OSCCONbits.SCS = 1;    // seleccionando el oscilador interno del PIC
    return;
}

///////////////// configuracion comunicacion USART /////////////////////////////

void conf_USART() {
    
    TXSTAbits.TX9 = 0; 
    TXSTAbits.TXEN = 1;   // configuracion de registro TXSTA 
    TXSTAbits.SYNC = 0; 
    TXSTAbits.BRGH = 1; 

    RCSTAbits.SPEN = 1; 
    RCSTAbits.RX9 = 0;    // configuracion de registro RCSTA 
    RCSTAbits.CREN = 1; 

    BAUDCTLbits.BRG16 = 1;   // configuracion de registro BAUDCTL

    SPBRG = 207; 
    SPBRGH = 0;           // configurando que opere a 9600 BAULIOS 
    
    return;
}
