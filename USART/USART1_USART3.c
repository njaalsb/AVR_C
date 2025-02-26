/* 
 * File:   newmain.c
 * Author: bruhe
 *
 * Created on January 20, 2025, 12:05 PM
 */
#define F_CPU 4000000UL
#define USART3_BAUD_RATE(BAUD_RATE) ((float)(F_CPU * 64 / (16 *(float)BAUD_RATE)) + 0.5)
#define USART1_BAUD_RATE(BAUD_RATE) ((float)(F_CPU * 64 / (16 *(float)BAUD_RATE)) + 0.5)

#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>
#include <string.h>

/*
 * 
 */

void USART3_init(void);
void USART3_sendChar(char c);
void USART3_sendString(char *str);
uint8_t USART3_read();
static int USART3_printChar(char c, FILE *stream);

void USART1_init(void);
void USART1_sendChar(char c);
void USART1_sendString(char *str);
uint8_t USART1_read();
static int USART1_printChar(char c, FILE *stream);

void USART3_init(void)
{
    PORTB.DIR &= ~PIN1_bm;
    PORTB.DIR |= PIN0_bm;

    USART3.BAUD = (uint16_t)USART3_BAUD_RATE(9600);
    USART3.CTRLB |= USART_TXEN_bm;
    USART3.CTRLB |= USART_RXEN_bm;
 
    static FILE USART_stream = FDEV_SETUP_STREAM(USART3_printChar, NULL, _FDEV_SETUP_WRITE);
    //stdout = &USART_stream;
}

void USART1_init(void)
{
    PORTC.DIR &= ~PIN1_bm;
    PORTC.DIR |= PIN0_bm;

    USART1.BAUD = (uint16_t)USART1_BAUD_RATE(9600);
    USART1.CTRLB |= USART_TXEN_bm;
    USART1.CTRLB |= USART_RXEN_bm;
 
    static FILE USART_stream = FDEV_SETUP_STREAM(USART1_printChar, NULL, _FDEV_SETUP_WRITE);
    stdout = &USART_stream;
}

void USART3_sendChar(char c){
    while (!(USART3.STATUS & USART_DREIF_bm))
    {
        ;
    }
    USART3.TXDATAL = c;
}

void USART1_sendChar(char c)
{
    while (!(USART1.STATUS & USART_DREIF_bm))
    {
        ;
    }
    USART1.TXDATAL = c;
}
uint8_t USART3_read(){
    while (!(USART3.STATUS & USART_RXCIF_bm)) 
    {
        ;
    }
    return USART3.RXDATAL;
}

uint8_t USART1_read(){
    while (!(USART1.STATUS & USART_RXCIF_bm))
    {
        ;
    }
    return USART1.RXDATAL;
}
static int USART3_printChar(char c, FILE *stream)
{
    USART3_sendChar(c);
    return 0;
}
static int USART1_printChar(char c, FILE *stream){
    USART1_sendChar(c);
    return 0;
}

void USART3_sendString(char *str){
    for(size_t i = 0; i < strlen(str); i++){
        USART3_sendChar(str[i]);
    }
}

void USART1_sendString(char *str){
    for(size_t i = 0; i < strlen(str); i++){
        USART1_sendChar(str[i]);
    }
}


int main(void){
    USART3_init();
    USART1_init();
    PORTB.DIRSET = PIN2_bm; //led
    PORTB.OUT |= PIN2_bm;
    char bokstav;
    while (1)
    {
        USART3_sendChar('f');
        bokstav = USART1_read();
        if (bokstav ==  'f'){
            //PORTB.DIRTGL = PIN3_bm;
            PORTB.OUT ^= PIN2_bm;
        }
        _delay_ms(1000); 
    }
}
