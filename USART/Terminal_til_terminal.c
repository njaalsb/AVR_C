/* 
 * File:   main.c
 * Author: bruhe
 *
 * Created on February 26, 2025, 10:45 AM
 */

// Setter CPU frekvens og regner ut baud-rate basert på frekvensen:
#define F_CPU 4000000UL
#define USART3_BAUD_RATE(BAUD_RATE) ((float)(F_CPU * 64 / (16 * (float)BAUD_RATE)) + 0.5)

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>

// Funksjonsprototyper
void USART3_init(void);
void USART3_sendChar(char c);
static int USART3_printChar(char c, FILE *stream);
uint8_t USART3_read();

// Definerer USART_stream
static FILE USART_stream = FDEV_SETUP_STREAM(USART3_printChar, NULL, _FDEV_SETUP_WRITE);

void USART3_init(void){
    PORTB.DIRSET = PIN0_bm; //TX-pinne som inngang
    PORTB.DIRCLR = PIN1_bm; //RX-pin som utgang
        
    USART3.BAUD = (uint16_t)USART3_BAUD_RATE(9600);
    USART3.CTRLB |= USART_TXEN_bm | USART_RXEN_bm; //Aktiverer både sending og mottak
    
    stdout = &USART_stream; //Erstatter standard output streamen med USART stream.
}

void USART3_sendChar(char c){
    //venter på at dataregisteret til USART3 tømmes før ny karakter sendes
     while (!(USART3.STATUS & USART_DREIF_bm))
    {
        ;
    }
    USART3.TXDATAL = c; // Send c
}

// Pakker USART3_sendChar inn i en funksjon som kan brukes med FDEV_SETUP_STREAM
static int USART3_printChar(char c, FILE *stream)
{
    USART3_sendChar(c);
    return 0;
}

// Funksjon for å lese det mottatte tegnet (char).
uint8_t USART3_read()
{
     //venter på at avbruddsflagget for mottak aktiveres
    while (!(USART3.STATUS & USART_RXCIF_bm))
    {
        ;
    }
    uint8_t src = USART3.RXDATAL;
    return src;
}

int main(void)
{
    USART3_init(); // Intitialiserer USART3
    
    char c;
    
    while (1)
    {
        c = USART3_read();
        USART3_sendChar(c);
    }   
}