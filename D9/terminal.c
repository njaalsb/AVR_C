/* 
 * File:   main.c
 * Author: bruhe
 *
 * Created on February 26, 2025, 10:45 AM
 */

 #include "terminal.h"
 #include "peripherals.h"
 
 // Funksjonsprototyper
 void USART3_init(void);
 void USART3_sendChar(char c);
 static int USART3_printChar(char c, FILE *stream);
 char USART3_read();
 
 /**
  * @brief Initialiserer hovedklokkka (F_CPU) til maksimal hastighet: 24MHz. 
  * Det er mulig Ã¥ endre til andre klokkefrekvenser i funksjonen.
  */
 
 
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
 char USART3_read()
 {
      //venter på at avbruddsflagget for mottak aktiveres
     while (!(USART3.STATUS & USART_RXCIF_bm))
     {
         ;
     }
     char src = USART3.RXDATAL;
     return src;
 }
 
 uint16_t input1(void)
 {
     USART3_init(); // Initialiser USART3
     uint16_t print_var;
     char input_values[7]; // Buffer for input
     char c; 
     
     //printf("&s\r\n", "Skriv inn et heltall: ");
     
     uint16_t i = 0;
 
     while (1)
     {
         c = USART3_read();
         //printf("%c\r\n", c);
 
         // Avslutt når Enter trykkes
         if (c == '\n' || c == '\r') {
             input_values[i] = '\0'; // Nullterminér strengen
             break;
         }
 
         // Legg til i bufferen hvis det er plass
         if (i < (sizeof(input_values) - 1)) {
             input_values[i++] = c;
         } else {
 
         }
     }
     if(strlen(input_values)>0){
         // Konverter string til heltall
         print_var = atoi(input_values);
 
         // Send tallet tilbake til terminalen
         printf("Du skrev: %i\n", print_var);
         return print_var;
     }
 }
 