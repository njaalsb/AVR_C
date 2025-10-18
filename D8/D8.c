/* 
 * File:   main.c
 * Author: bruhe
 *
 * Created on January 20, 2025, 12:05 PM
 * 
 * Info:
 * Denne koden er en testbenk for UART kommunikasjon, USART1 sender tilfeldige bitsekvenser
 * til USART3. USART0 skal skrive resultatene fra kommunikasjonen til terminalen, men litt
 * usikker på om dette er nødvendig. Mulig USART3 kan brukes i steden. 
 * 
 * Pinout: 
 * 
 * PB0 -> R_x (USART3)
 * PC1 -> T_x (USART1)
 * 
 * Sist oppdatert: 18/10/25
 */


 // Makroer for klokkefrekvens og baud rate 
#define F_CPU 4000000UL
#define TIMEOUT_MS 1000 // Timeout i et sekund 
#define USART3_BAUD_RATE(BAUD_RATE) ((float)(F_CPU * 64 / (16 *(float)BAUD_RATE)) + 0.5)
#define USART1_BAUD_RATE(BAUD_RATE) ((float)(F_CPU * 64 / (16 *(float)BAUD_RATE)) + 0.5)
#define USART0_BAUD_RATE(BAUD_RATE) ((float)(F_CPU * 64 / (16 *(float)BAUD_RATE)) + 0.5)
#define COMMON_BAUD_RATE 9600  

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <time.h>
#include <util/delay.h>
#include <string.h>


/*
 * Denne makroen brukes for å gjøre om en byte til et bitmønster.
 *
 * F.eks: 0xAF => '0b10101111' 
 * 
 * Dette brukes for å sammenligne bytes bitvis.
 * 
 * Kilde: https://stackoverflow.com/questions/111928/is-there-a-printf-converter-to-print-in-binary-format
 */

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
    ((byte) & 0x80 ? '1' : '0'), \
    ((byte) & 0x40 ? '1' : '0'), \
    ((byte) & 0x20 ? '1' : '0'), \
    ((byte) & 0x10 ? '1' : '0'), \
    ((byte) & 0x08 ? '1' : '0'), \
    ((byte) & 0x04 ? '1' : '0'), \
    ((byte) & 0x02 ? '1' : '0'), \
    ((byte) & 0x01 ? '1' : '0')


// Prototyper USART1
void USART1_init(void);
void USART1_sendChar(char c);
void USART1_sendString(char *str);
uint8_t USART1_read();
static int USART1_printChar(char c, FILE *stream);

// Protoyper USART2
void USART0_init(void);
void USART0_sendChar(char c);
void USART0_sendString(char *str);
static int USART0_printChar(char c, FILE *stream);

// Prototyper USART3
void USART3_init(void);
void USART3_sendChar(char c);
void USART3_sendString(char *str);
uint8_t USART3_read();
static int USART3_printChar(char c, FILE *stream);

// Prototype for randomizer funksjon
char randomizer();

void USART0_sendChar(char c) {
    while (!(USART0.STATUS & USART_DREIF_bm)) 
    {
        ;
    }
    USART0.TXDATAL = c;
}

static int USART0_printChar(char c, FILE *stream) {
    USART0_sendChar(c);
    return 0;
}

static FILE USART_stream = FDEV_SETUP_STREAM(USART3_printChar, NULL, _FDEV_SETUP_WRITE);

void USART0_init(void){
    // Disse linjene setter pinne 1 til Rx og pinne 0 til Tx:
    //PORTA.DIR &= ~PIN1_bm;
    PORTA.DIR |= PIN0_bm;
    
    // Her settes baud-raten på USART3
    USART0.BAUD = (uint16_t)USART0_BAUD_RATE(COMMON_BAUD_RATE);
    
    USART0.CTRLB |= USART_TXEN_bm; 
    
    stdout = &USART_stream;
}

void USART1_init(void)
{
    PORTC.DIR &= ~PIN1_bm;
    PORTC.DIR |= PIN0_bm;

    USART1.BAUD = (uint16_t)USART1_BAUD_RATE(COMMON_BAUD_RATE);
    USART1.CTRLB |= USART_TXEN_bm;
    USART1.CTRLB |= USART_RXEN_bm;   
}

void USART3_init(void)
{
    PORTB.DIR |= PIN0_bm;

    USART3.BAUD = (uint16_t)USART3_BAUD_RATE(COMMON_BAUD_RATE);
    USART3.CTRLB |= USART_TXEN_bm;
    USART3.CTRLB |= USART_RXEN_bm;
}

void USART1_sendChar(char c)
{
    while (!(USART1.STATUS & USART_DREIF_bm))
    {
        ;
    }
    USART1.TXDATAL = c;
}

void USART3_sendChar(char c){
    while (!(USART3.STATUS & USART_DREIF_bm))
    {
        ;
    }
    USART3.TXDATAL = c;
}

uint8_t USART1_read(){
    uint32_t timeout = 0;
    while (!(USART1.STATUS & USART_RXCIF_bm))
    {
        // Timer 
        timeout++;
        if(timeout >= 100){
            return 0;
        }
    }
    return USART1.RXDATAL;
}

uint8_t USART3_read(){
    while (!(USART3.STATUS & USART_RXCIF_bm)) 
    {
        ;
    }
    return USART3.RXDATAL;
}

static int USART1_printChar(char c, FILE *stream){
    USART1_sendChar(c);
    return 0;
}

// Randomizer funksjon
char randomizer(){
    int i = rand() % 20;
    char c = i + '0';
    return c;
}

static int USART3_printChar(char c, FILE *stream)
{
    USART3_sendChar(c);
    return 0;
}

void USART1_sendString(char *str){
    for(size_t i = 0; i < strlen(str); i++){
        USART1_sendChar(str[i]);
    }
}

void USART3_sendString(char *str){
    for(size_t i = 0; i < strlen(str); i++){
        USART3_sendChar(str[i]);
    }
}

void USART0_sendString(char *str){
    for(size_t i = 0; i < strlen(str); i++){
        USART0_sendChar(str[i]);
    }
}

void purge_buffer(void){
    while(USART1.STATUS & USART_RXCIF_bm){
        volatile int8_t flusher = USART1.RXDATAL;
    }
}

void BER_calculator(uint8_t tx, uint8_t rx) {
    // Bitvis XOR på motatt og sendt bitsekvens (7 bit lengde ish, siden ASCII brukes)
    uint8_t forskjell = tx ^ rx;
    static uint16_t feil_teller;
    static uint16_t iterasjoner;

    printf("%d\r\n", iterasjoner++);

    if (forskjell == 0) {
        printf("Ingen bitfeil: 0b"BYTE_TO_BINARY_PATTERN"\n\r", BYTE_TO_BINARY(tx));
    } 
    else {
        printf("Bitfeil funnet!\n\r");
        printf("Sendt    : 0b"BYTE_TO_BINARY_PATTERN"\n\r", BYTE_TO_BINARY(tx));
        printf("Mottatt  : 0b"BYTE_TO_BINARY_PATTERN"\n\r", BYTE_TO_BINARY(rx));
        printf("Forskjell: 0b"BYTE_TO_BINARY_PATTERN"\n\r", BYTE_TO_BINARY(forskjell));

        // Itererer gjennom bitsekvensen og inkrementerer feil_teller hver gang en feil oppdages
        for (int i = 0; i < 8; i++) {
            if (forskjell & (1 << i)) { 
                printf("Bit %d er feil!\n\r", i);
                printf("%d\r\n", feil_teller++);
            }
        }
    }
    if(iterasjoner >= 1000 & feil_teller >= 0){
        double rate;

        // Etter 1000 iterasjoner beregnes feilraten
        // Antall iterasjoner kan økes for mer nøyaktig beregning 
        rate = 100.0 * ((float)feil_teller/(iterasjoner*8));
        printf("BER = %f\r\n", rate);
        printf("Antall iterajoner %d\r\n", iterasjoner);
        printf("Antall Bitfeil %d\r\n", feil_teller);
        _delay_ms(10000);
        iterasjoner = 0;
        feil_teller = 0;
    }
}


int main(void) {
    // Initialiserer
    USART3_init();
    USART0_init();
    USART1_init();

    PORTB.DIRSET = PIN2_bm; //led
    PORTB.OUT |= PIN2_bm;
    char* sendstring = "Starter...\r\n";
    USART3_sendString(sendstring);
    _delay_ms(1000);
    char tx_char;
    char rx_char;
    while (1) {
        // Tømmer mottaks bufferen til USART1
        purge_buffer();
        
        _delay_ms(10);
        
        // Generer random char 
        tx_char = randomizer();
       
        USART3_sendChar(tx_char);
        
        _delay_ms(10);
        
        rx_char = USART1_read();
        
        // Linjeskift
        printf("%s\r\n", ""); 
        
        BER_calculator(tx_char, rx_char);
        
        // Toggler LED
        if(rx_char == tx_char){
            PORTB.OUT ^= PIN2_bm;
        } 
        _delay_ms(10);
    }
}       
