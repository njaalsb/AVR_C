/* 
 * File:   peripherals.h
 * Author: bruhe
 *
 * Created on February 21, 2025, 1:44 PM
 */

 #ifndef PERIPHERALS_H
 #define	PERIPHERALS_H
 
 #ifdef	__cplusplus
 extern "C" {
 #endif
    
     
 #define F_CPU 24000000UL
 #define F_SAMPLE 16384 /* Hz - Samplingsfrekvens */
 #define M_SAMPLES 8192 /* Antall punktprÃ¸ver i oppslagstabellen */
     
 /* Parametere for frekvenssveip. Justering av disse endrer oppfÃ¸rselen til sveipet. */
 //#define F_0 200  /* Hz - Startfrekvens */ 
 //#define F_1 5000 /* Hz - Sluttfrekvens */ 
 //#define T_SWEEP 10 /* s - sveipperiode */ 
 
 /* Parametere for frekvenssveip. Justering av disse endrer oppfÃ¸rselen til sveipet. */
 #define F_0 100  /* Hz - Startfrekvens */ 
 #define F_1 5000 /* Hz - Sluttfrekvens */ 
 #define T_SWEEP 10 /* s - sveipperiode */ 
 
 /* Beregning av initialverdi for d_k og konstanten dd_k. */
 #define K_ZERO (float)M_SAMPLES*F_0/(float)F_SAMPLE*((uint32_t)1<<16) /* Ligning 16 i [1] */
 #define DELTA_DELTA_K (float)M_SAMPLES*(F_1-F_0)/((float)T_SWEEP*F_SAMPLE*F_SAMPLE)*((uint32_t)1<<16) /* Ligning 19 i [1]*/
 
 #include <avr/io.h>
 #include <avr/cpufunc.h>
 #include <util/delay.h>
 #include <avr/interrupt.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <time.h>
 #include <util/delay.h>
 #include "terminal.h"
     
 extern volatile uint16_t meny;
 extern volatile uint32_t F_TESTER;
 extern volatile uint32_t i;
 
 extern volatile uint16_t gjett;
 extern volatile uint16_t svar;
 
 
 void TCA0_init(void);
 void CLK_configuration(void);
 void run_sweep();
 void DAC0_set_val(uint16_t val);
 void DAC0_init(void);
 
 uint16_t resultat(uint16_t verdi_1, uint16_t verdi_2);
 uint16_t randomizer(uint16_t seed);
 
 #ifdef	__cplusplus
 }
 #endif
 
 #endif	/* PERIPHERALS_H */
 
 