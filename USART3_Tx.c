/* 
 * File:   main.c
 * Author: bruhe
 *
 * Created on January 14, 2025, 2:56 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <string.h>

// Setter CPU frekvens og regner ut baud-rate basert på frekvensen:
#define F_CPU 4000000UL 
#define USART3_BAUD_RATE(BAUD_RATE) ((float)(4000000UL * 64 / (16 * (float)BAUD_RATE)) + 0.5)
// Delay filen må includes etter at klokkefrekvensen til CPU'en er satt
#include <util/delay.h>

/*
 * Funksjonsprototyper:
 */
void USART3_init(void);
void USART3_sendChar(char c);
void USART3_sendString(char *str);

// Denne funksjonen initialiserer USART3 
void USART3_init(void){
    // Disse linjene setter pinne 1 til Rx og pinne 0 til Tx:
    PORTB.DIR &= ~PIN1_bm;
    PORTB.DIR |= PIN0_bm;
    
    // Her settes baud-raten på USART3
    USART3.BAUD = (uint16_t)USART3_BAUD_RATE(9600);
    USART3.CTRLB |= USART_TXEN_bm;
}

// Denne funksjonen tar in en char og sender den:
void USART3_sendChar(char c){
    while(!(USART3_STATUS & USART_DREIF_bm)){
        // Gjør ingenting når USART3_STATUS og USART_DREIF_bm er false:
        ;
    }
    // Litt usikker på hva konkret denne linjen gjør
    USART3.TXDATAL = c;
}
// konverterer characters til en streng: 
void USART3_sendString(char* str){
    for(size_t i = 0; i < strlen(str); i++){
        USART3_sendChar(str[i]);
    }
}
// Main
int main(void) {
    // Intitialiserer:
    USART3_init();
    // Printer uendelig:
    while(1){
        USART3_sendString("Hello_World!\r\n");
        _delay_ms(500);
    }
}
