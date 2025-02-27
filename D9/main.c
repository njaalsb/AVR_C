/*
 * File:   main.c
 * Author: crfosse
 * 
 * Dette programmet gjennomfÃ¸rer et frekvenssveip ved hjelp av direkte digital syntese (DDS) på DACen til AVR128DB48. Sveipet gjennomføres ved hjelp av telleren TCA0 og er avbruddsstyrt for å få en jevn
 * punktprÃ¸vingsfrekvens. Prinsippene for DDS og frekvenssveip, samt noen praktiske detaljer, er beskrevet det det tekniske notatet [1]. Sveipet bruker en cosinus-oppslagstabell pÃ¥ 2^13 punkter, lagret i "cosine_table.h".
 * Noen detaljer:
 * - Utgangspinne for DAC: pinne PD6.
 * - Prossessorfrekvens: 24MHz.
 * - PunktprÃ¸vingsfrekvens: 16384 Hz.
 *
 * Kilder:
 * [1] - L. Lundheim: Generering av frekvenssveip, internt notat, NTNU, 2025
 * Created on 6. februar 2025, 14:42
 */

 #include "peripherals.h"
 #include "terminal.h"
 //#include "cosine_table.h"
 
 uint16_t res_estimat;
 
 int main(void) {
 
     /* Setter hovedklokka til 24MHz for Ã¥ fÃ¥ hÃ¸y samplingsfrekvens. */
     CLK_configuration();
 
     /* Initialiserer DACen*/
     DAC0_init();
 
     /* Intitaliserer telleren. Merk: den aktiveres ikke.*/
     TCA0_init();
     
     USART3_init();
     
     PORTA.DIR &= ~ PIN2_bm;
     PORTA.PIN2CTRL |= !PORT_PULLUPEN_bm | PORT_ISC_RISING_gc; 
     
     /* Setter pinne PB2, BTN0 pÃ¥ kortet, som inngang. */
     PORTB.DIR &= ~ PIN2_bm;
     PORTB.PIN2CTRL |= PORT_PULLUPEN_bm | PORT_ISC_FALLING_gc;    
 
     /* Setter pinne PB3, LED0 pÃ¥ kortet, som utgang. */
     PORTB.DIRSET = PIN3_bm;
     PORTB.OUT = PIN3_bm;
     
     sei();
     
     while (1) {
         i++;
             switch(meny){
         case 1:
             printf("meny nummer: %i\r\n", meny);
             printf("Det vil spilles av en tilfeldig frekvens mellom %d", F_0);
             printf(" og %d\r\n", F_1);
             something = input1();
             while(USART3_read() != '\n');
             //printf("Input2: %i\r\n", something);
             _delay_ms(10);
             sweep = input1();
             while(USART3_read() != '\n');
             printf("Input2: %i\r\n", sweep);
             _delay_ms(10);
             meny = 2;
             sei();
             break;
         case 2:
             printf("meny nummer: %i\r\n", meny);
             printf("Kjorer tone med frekvens: %d\r\n", svar);
             PORTD.OUT ^= PIN5_bm;
             
             run_sweep();
             _delay_ms(2000);
             cli();
             PORTD.OUT ^= PIN5_bm;
             meny = 3;
             break;
         case 3:
             while(USART3_read() != '\n');
             printf("meny nummer: %i\r\n", meny);
             sei();
             run_sweep();
             _delay_ms(2000);
             cli();
             meny = 4;
             break;
         case 4:
             res_estimat = resultat(svar, gjett);
             printf("Du var  %d", res_estimat);
             printf("unna den korrekte frekvensen: %d\r\n", svar);
         default:
             // something something ....
             ;
             break;
         }
     }
 }