// C:\Users\bruhe\Downloads\avr_demodulator_template_test.X (1)\avr_demodulator_template.X\main.c
/* Ferdig versjon
* INFO: 
* Denne koden implementerer et demoduleringssystem for å demodulere et FSK-signal til UART. 
* Systemet inn punktprøver med ADCen, filtrerer disse gjennom to 2. ordens IIR-filter, videre brukes det to midlingsfilter for å gi et moving average. 
* til slutt tas absoluttverdien av resultatet før dette sendes ut igjen på DACen. Resultatet er også tilgjengelig i hovedløkka og skrives der til terminalen med UART. 
*
* Det er også lagt inn funksjonalitet som sjekker om avbruddsrutninen kjører for tregt ved å se på verdiene til telleren underveis i rutinen. Om telleren
* rekker en hel periode før avbruddsrutinen er ferdig er prossesen for treg og systemet går inn i et feilmodus. Merk at dette ikke er en idéell løsning.
*
* PINOUT
* - PortD pinne 1 (PD1): ADC inn (med mulighet for å bruke andre pinner) 
* - PortD pinne 6 (PD6): DAC ut  
* - PortB pinne 3 (PB3): LED0 ut (LEDen på kortet)  
*  
* Notes on the implementation: 
* - ADC har en oppløsning på 10 bit.  
* - Systemklokka er 24MHz, som er den maksimale hastigheten. 
* - ADC-klokka er 2MHz, som er dens maksimale hastighet (see datablad for AVR128DB48). 
* - Detaljer rundt periferienhetimplentasjoner finner man i usart.h/.c og peripherals.h/.c. 
* - Filen config.h inneholder sentrale definisjoner som blir brukt i hele prosjektet. 
* 
* Created: 17/06/2024
* Sist modifisert: 10/5/2025
* Author : Njål Bruheim, utgangspunkt i kodeeksempel skrevet av: Carl Richard Steen Fosse. 
*/

/* Global configurations for the project */

#include "config.h" 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "usart.h"  
#include "peripherals.h" 

//-------------------Brukerdefinerte parametere-------------------  

#define SAMPLING_PERIOD 1000 //us 
#define N 2 //For 2^N
#define CIRCULAR_BUFFER_SIZE (1 << N) //Gir bufferstørrelse 2^N
#define NUM_SAMPLES_DELAY 1

/* Skaleringsfaktor for å unngå numerisk overflyt*/  
#define A_FLOAT (float) 0.8
#define A (int16_t)(A_FLOAT*(1<<6)) 

/* Definisjon av filterorden*/ 
#define FILTER_ORDER 2

/* Parameter for midlingsfilteret*/ 

#define N_MID 6 // For N^2 for middling
#define CIRC_MID_BUFF_SIZE (1 << N_MID) //Gir bufferstørrelse 2^N

/* Filterkoeffsienter for de to høypassfiltrene:*/ 
static const int16_t BP1[FILTER_ORDER] = {(float)(-0.49)*(1<<6), (float)(0.64)*(1<<6)}; 
static const int16_t BP0[FILTER_ORDER] = {(float)(-0.49)*(1<<6), (float)(-0.64)*(1<<6)}; 

//-------------------------------------------------------------

/* Global summasjonsvariabel */
static volatile int32_t summa_summarum = 0;

/* Globale variabel for å holde på absluttverdien av filtrert data*/
volatile int16_t bp1_filt_abs = 0; 
volatile int16_t bp0_filt_abs = 0; 
volatile bool result_ready = false; 
volatile bool error_mode = false; 

/* Typedefinisjon for ringbufferen*/ 
typedef struct {
    int16_t * const buffer;
    unsigned writeIndex;
    unsigned size;
} circular_buffer_t; 

/* Funksjonsprototyper */
void circular_buffer_write(circular_buffer_t *c, int16_t data); 
int16_t circular_buffer_read(circular_buffer_t *c, unsigned Xn); 

/* Funksjonsprototyper filter */
int16_t bp1_filter(int16_t sample); 
int16_t bp0_filter(int16_t sample0); 
int16_t md1_filter(int16_t filt_samp); 
int16_t md0_filter(int16_t filt_samp0); 

/*
 * @brief ISR for sampling og D/A-konvertering, med et eksempel på signalbehandling. 
 * ISRen utløses etter TCA0-telleren maksverdien sin (overflow med avbruddsvektor TCA0_OVF_vect). 
 */

ISR(TCA0_OVF_vect) {
    // Tilbakestiller TCA0 avbruddsflagget 
    TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm; 
    // Variabel for å lagre nye punktprøver fra ADCen. 
    static volatile uint16_t input_sample = 0; 
    //Variabel til bruk under signalbehandlingen. 
    static int16_t normalized_sample = 0; 
    
    //Variabel for lagring av filterresultater. 
    static int16_t bp1_filt_result = 0; 
    static int16_t bp0_filt_result = 0; 
    
    // Variabler for lagring av midling: 
    static int16_t bp1_filt_mid = 0; 
    static int16_t bp0_filt_mid = 0; 
    //Punktprøving med ADCen
    input_sample = ADC0_read(); 
    /*--Signalbehandling starter--*/ 
    /* Justerer punktprøvet verdi til å variere rundt 0 */ 
    normalized_sample = (int16_t)input_sample - 512;
    
    /* Skalerer for økt nøyaktighet */ 
    normalized_sample = normalized_sample << 6; 
    /* Skalerer for å hindre numerisk overflyt*/ 
    normalized_sample = normalized_sample - ((int32_t)A*normalized_sample>>6); 
    
    /* Høypassfiltrering av punktprøven */ 
    bp1_filt_result = bp1_filter(normalized_sample); 
    bp0_filt_result = bp0_filter(normalized_sample); 

    /* Energiestimat med absoluttverdien av punktprøven. Se math.h*/ 
    bp1_filt_abs = abs(bp1_filt_result);
    bp0_filt_abs = abs(bp0_filt_result); 
    
    /* Midling av absoluttverdien til de filtrerte punktprøvene */ 
    bp1_filt_mid = md1_filter(bp1_filt_abs); 
    bp0_filt_mid = md0_filter(bp0_filt_abs); 
    
    // Sjekker hvilken av filtrene som gir størst utslag
    if (bp0_filt_mid > bp1_filt_mid){
    // signalet er på 200Hz 
    summa_summarum = 1023;
    }
    else if (bp0_filt_mid < bp1_filt_mid){
    // signalet er på 300Hz 
    summa_summarum = 300;
    }
    result_ready = true;
    
    /*--Signalbehandling slutter--*/
    DAC0_set_val(summa_summarum);
    
    /* Sjekker om ISRen er for treg ved å se om telleren har gått én runde. 
    * Hvis dette er tilfellet deaktiverer vi telleren. Systemet går så i feilmodus. 
    */
    if (TCA0.SINGLE.INTFLAGS & TCA_SINGLE_OVF_bm) {
        error_mode = true;
        TCA0.SINGLE.CTRLA = ~TCA_SINGLE_ENABLE_bm;
    }
}
/*
 * 
 * @brief Et andreordens rekursivt båndpassfilter.
 * 
 *  @param sample En 16-bits verdi som skal filtreres. 
 */ 

int16_t bp1_filter(int16_t sample) {
    //Deklarerer en ringbuffer med dataområde gitt av CIRCULAR_BUFFER_SIZE
    static int16_t bp1_buffer_data_space[CIRCULAR_BUFFER_SIZE]; 
    static circular_buffer_t bp1_filt_buff = {
        .buffer = bp1_buffer_data_space,
        .writeIndex = 0,
        .size = CIRCULAR_BUFFER_SIZE
    };

    // Oppretter variabler for lagring av punktprøver
    static int16_t delayed_sample = 0;
    static int16_t result = 0;
    static int16_t delayed_sample2 = 0;

    // Leser punktprøver fra buffer    
    delayed_sample = circular_buffer_read(&bp1_filt_buff, NUM_SAMPLES_DELAY);
    delayed_sample2 = circular_buffer_read(&bp1_filt_buff, 2*NUM_SAMPLES_DELAY);
    
    // Selve filtreringsoperasjonen
    result = sample + ((int32_t)BP1[0]*delayed_sample>>6) + ((int32_t)BP1[1]*delayed_sample2>>6);
    
    // Skriver resultatet tilbake i bufferen
    circular_buffer_write(&bp1_filt_buff, result);
    return result;
}

int16_t bp0_filter(int16_t sample0) {
    //Deklarerer en ringbuffer med dataområde gitt av CIRCULAR_BUFFER_SIZE
    static int16_t bp0_buffer_data_space[CIRCULAR_BUFFER_SIZE]; 
    static circular_buffer_t bp0_filt_buff = {
        .buffer = bp0_buffer_data_space, 
        .writeIndex = 0,
        .size = CIRCULAR_BUFFER_SIZE
    };

    // Oppretter variabler for lagring av punktprøver
    static int16_t delayed_sample1 = 0;
    static int16_t result2 = 0;
    static int16_t delayed_sample12 = 0;
    
    // Leser punktprøver fra buffer
    delayed_sample1 = circular_buffer_read(&bp0_filt_buff, NUM_SAMPLES_DELAY);
    delayed_sample12 = circular_buffer_read(&bp0_filt_buff, 2*NUM_SAMPLES_DELAY);
    
    // Selve filtreringsoperasjonen
    result2 = -(sample0) + ((int32_t)BP0[0]*delayed_sample1>>6) + ((int32_t)BP0[1]*delayed_sample12>>6);
    
    // Skriver resultatet tilbake i bufferen
    circular_buffer_write(&bp0_filt_buff, (result2));
    
    return result2;
}

/*
 * @brief Midlingsfilter
 *
 * @param Filtrert verdi fra båndpassfiler
 * 
 * Gir et slags moving average
 */

int16_t md1_filter(int16_t filt_samp) {
    // Deklarerer ny buffer for midling av verdiene:
    static int16_t md1_buffer_data_space[CIRC_MID_BUFF_SIZE];
    static circular_buffer_t md1_filt_buff = {
        .buffer = md1_buffer_data_space,
        .writeIndex = 0,
        .size = CIRC_MID_BUFF_SIZE
    };
    
    // Midling av verdiene:
    static int32_t sum1 = 0;
    
    int16_t old_sample = abs(circular_buffer_read(&md1_filt_buff, -1));
    sum1 -= old_sample;
    
    circular_buffer_write(&md1_filt_buff, filt_samp);
    sum1 += filt_samp;
    // returnerer summen delt på lengden på bufferen:
    return (sum1>>N_MID);
}

int16_t md0_filter(int16_t filt_samp0) {
    // Deklarerer ny buffer for midling av verdiene:
    static int16_t md0_buffer_data_space[CIRC_MID_BUFF_SIZE];
    static circular_buffer_t md0_filt_buff = {
        .buffer = md0_buffer_data_space,
        .writeIndex = 0,
        .size = CIRC_MID_BUFF_SIZE
    }; 

    // Midling av verdiene:
    static int32_t sum1 = 0;
    
    int16_t old_sample = abs(circular_buffer_read(&md0_filt_buff, -1));
    sum1 -= old_sample;
    
    circular_buffer_write(&md0_filt_buff, filt_samp0);
    sum1 += filt_samp0;
    // returnerer summen delt på lengden på bufferen:
    return (sum1>>N_MID);
}

/*
 * @brief Skriver data til neste ledige plass i ringbufferet c og
 * øker writeIndex. 
 * 
 * @param c En circular_buffer_t. Må deklareres før bruk.
 * @param data Dataene som skal skrives. Må være samme størrelse som
 * datatypen brukt for circular_buffer_t. 
 */

void circular_buffer_write(circular_buffer_t *c, int16_t data) {
    c -> buffer[c->writeIndex] = data;
    c->writeIndex = (c->writeIndex+1)%(c->size);
}

/*
 * @brief Leser data fra en gitt posisjon i ringbufferen relativt til nåværende
 * writeIndex.
 * 
 * @param c En circular_buffer_t. Må deklareres før bruk.
 * @param Xn Funksjonen vil returnere dataene ved writeIndex-Xn. 
 * Xn bør være mellom 0 og CIRCULAR_BUFFER_SIZE-1. 
 */

int16_t circular_buffer_read(circular_buffer_t *c, unsigned Xn) {
    return c->buffer[(c->writeIndex-Xn+(c->size)-1)%(c->size)];
}

int main(void) 277 {
    /* Run initialization functions */

    /* Konfigurerer systemklokka til å kjøre på 24MHz */
    CLK_configuration(); 

    /* Initialiserer referansespenning for DAC and ADC.
     * VDD brukes for begge. 
     */
    VREF_init();

    /* Initialiserer DAC */
    DAC0_init(); 

    /* Initialiserer ADC */
    ADC0_init();

    /* Initialiserer TCA0 telleren med regulære avbrudd hver SAMPLING_PERIOD */
    TCA0_init(SAMPLING_PERIOD);
    
    /* Initialiserer UART med en baudrate på 115200 bits/s */
    USART3_init(9600);
    
    /* Aktiverer globale avbrudd */
    sei();
    
    /* Setter PIN B3, LED0 på AVR-kortet, som utgang. */
    PORTB.DIRSET = PIN3_bm; 301
    while (1)
    {
        if(result_ready) {
            printf("Summa_summarum = %d\r\n", summa_summarum);
        }
        /* Blinker LEDen for å vise at koden kjører. */
        PORTB.OUT ^= PIN3_bm;
        if(error_mode) {
            /* Rask eller ingen blink betyr at noe er galt*/
            _delay_ms(100);
            //printf("-- Error mode! -- \n");
        } 
        else {
            _delay_ms(500);
        }
    }
}
