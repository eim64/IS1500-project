#include "io.h"
#include <pic32mx.h>

uint32_t getbtns(){
    return ((PORTD >> 4) & 0xE) | ((PORTF & 0x2) >> 1);
}

uint32_t getswitches(){
    return (PORTD >> 8) & 0xF;
}

unsigned int read_potentiometer(){
    // Start sample
    AD1CON1SET = 0x2;
    // Wait for DONE
    while(!(AD1CON1 & 0x1));
    // Return the value 
    return ADC1BUF0;
}