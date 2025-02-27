/* 
 * File:   terminal.h
 * Author: bruhe
 *
 * Created on February 26, 2025, 6:20 PM
 */

 #ifndef TERMINAL_H
 #define	TERMINAL_H
 
 #ifdef	__cplusplus
 extern "C" {
 #endif
     
     
 #include "peripherals.h"
 #include <stdlib.h>
 #include <avr/io.h>
 #include <string.h>
 #include <stdio.h>
 
 #include <avr/cpufunc.h>
 #include <avr/interrupt.h>
 #include <time.h>
 // Setter CPU frekvens og regner ut baud-rate basert på frekvensen:
 //#define F_CPU 24000000UL
 #define USART3_BAUD_RATE(BAUD_RATE) ((float)(F_CPU * 64 / (16 * (float)BAUD_RATE))+ 0.5)
 
 #include <util/delay.h>
 
 // Funksjonsprototyper
 void USART3_init(void);
 void USART3_sendChar(char c);
 static int USART3_printChar(char c, FILE *stream);
 char USART3_read();
 uint16_t input1(void);
 
 
 #ifdef	__cplusplus
 }
 #endif
 
 #endif	/* TERMINAL_H */
 
 