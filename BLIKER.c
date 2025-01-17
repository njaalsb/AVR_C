/* 
 * File:   main.c
 * Author: bruhe
 *
 * Created on January 10, 2025, 2:45 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#define F_CPU=4000000UL

/*
 * Kode som blinker LED på pinne PD5
 */
int main() {
    PORTD.DIRSET = PIN5_bm;
    while(1){
        _delay_ms(500);
        //PORTD.DIRCLR = 0b00000000;
        PORTD.OUT = PIN5_bm;
        _delay_ms(500);
        PORTD.OUTCLR = PIN5_bm;
    }
    return (EXIT_SUCCESS);
}
