#include "peripherals.h"
#include "cosine_table.h"

 /**
 * @brief Initialiserer telleren TCA0. Telleren aktiveres ikke.
 */

volatile uint16_t meny = 1;
volatile uint32_t F_TESTER; // = randomizer(222);
volatile uint32_t i = 0;

volatile uint16_t gjett;
volatile uint16_t svar;

void TCA0_init() {
    /* Konfigrerer tellerperioden (hvor mange klokkesykluser den skal telle).
    *  Formel: F_CPU/(F_CNT*SKALERING)
    */
	TCA0.SINGLE.PER = (uint16_t)(F_CPU/(F_SAMPLE*1));
    
    /* Aktiverer avbrudd fra telleroverflyt */
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;
	
    /* Setter telleren i 16-bits modus med en klokkefrekvens pÃ¥ F_FPU/1. */
  	TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1_gc; 

}

 /**
  * @brief Avbruddsrutine for telleren. Denne hÃ¥ndterer frekvenssveipet.
  */

ISR(TCA0_OVF_vect) {
    
    /* Definerer variabler for Ã¥ holde punktprÃ¸ver*/
    static int16_t curr_sample = 0; /* Verdier med fortegn fra cosinustabellen */
    static uint16_t dac_val = 0; /* Verdier uten fortegn som skal skrives til DACen*/

    /* Definerer variabler for Ã¥ hÃ¥ndtere sveipet og frekvensgenrerering. 
    *  Se notatet [1] for detaljer.
    */
    static uint16_t k = 0;
    static uint32_t d_k = K_ZERO;
    volatile uint32_t dd_k;
    F_TESTER = randomizer(i);
    svar = F_TESTER;
    if(meny == 2){
        dd_k = 0;
        d_k = (float)M_SAMPLES*F_TESTER/(float)F_SAMPLE*((uint32_t)1<<16);
    } else if(meny == 3){
        dd_k = DELTA_DELTA_K;
        d_k = K_ZERO;
    }
  
    
    /* Bruker modulo for å sette d_k til 0 når frekvensen F_1 er nÃ¥dd. */
    d_k = (d_k + dd_k)  % ((uint32_t)F_1 << (15)); 
    
    /* Bruker modulo for Ã¥ sette k til 0 etter en gjennomgang av oppslagstabellen */
    k = (k+(d_k>>16)) % M_SAMPLES;
        
    curr_sample = sineLookupTable[k];

    /* Konverterer til positiv 10-bits verdi*/
    dac_val = (curr_sample >> 6)+512; 
    DAC0_set_val(dac_val);
    
    /* NÃ¥r d_k er 0 er et sveip gjennomfÃ¸rt. */
    if(d_k==0) {
        /* Slukker LED0 */
        PORTB.OUT = PIN3_bm;

        /* SlÃ¥r av telleren */
        TCA0.SINGLE.CTRLA &= ~TCA_SINGLE_ENABLE_bm; 

        /* Setter d_k til initialverdi */
        d_k = K_ZERO;
    }
    
    /* Tilbakestiller avbruddsflagget for TCA0 */
    gjett = (d_k >> 16) * (float)F_SAMPLE / (float)M_SAMPLES;
	TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
}

/**
 * @brief Initialiserer hovedklokkka (F_CPU) til maksimal hastighet: 24MHz. 
 * Det er mulig Ã¥ endre til andre klokkefrekvenser i funksjonen.
 */
void CLK_configuration(void)
{
	/* Setter OSCHF-klokka til 24 MHz */
	ccp_write_io ((uint8_t *) & CLKCTRL.OSCHFCTRLA, CLKCTRL_FRQSEL_24M_gc 
                                                   | CLKCTRL_AUTOTUNE_bm);
}

 /**
 * @brief Initialiserer DACen. Den bruker pinne PD6.
 */
 void DAC0_init(void)
 {
     /* Konfigurerer pinne PD6*/
	 /* Deaktiverer digital input buffer */
	 PORTD.PIN6CTRL &= ~PORT_ISC_gm;
	 PORTD.PIN6CTRL |= PORT_ISC_INPUT_DISABLE_gc;
     
	 /* Deaktiverer opptrekksmotstand */
	 PORTD.PIN6CTRL &= ~PORT_PULLUPEN_bm;
     
     VREF.DAC0REF |= VREF_REFSEL_VDD_gc;

	 /* Aktiverer DACen, utgangsbuffer */
	 DAC0.CTRLA = DAC_ENABLE_bm | DAC_OUTEN_bm;
 }

  /**
 * @brief Setter Ã¸nsket verdi pÃ¥ DACen.
 *
 * @Param val 10 bits verdi som skal konverters.
 */
 void DAC0_set_val(uint16_t val)
 {
	 /* Lagrer de to LSbene i DAC0.DATAL */
	 DAC0.DATAL = (val & (0x03)) << 6;
	 /* Lagrer de Ã¥tte MSbene i DAC0.DATAH */
	 DAC0.DATAH = val >> 2;
 }

 /**
 * @brief Setter i gang et sveip ved Ã¥ aktivere telleren.
 */
void run_sweep() {
    /* Tenner LED0 for å markere start på sveip */
    PORTB.OUT &= ~PIN3_bm;

    /* Aktiverer telleren for Ã¥ starte sveip*/
    TCA0.SINGLE.CTRLA = TCA_SINGLE_ENABLE_bm; 
}

ISR(PORTA_PORT_vect){
    PORTD.DIRSET = PIN5_bm;
    if(PORTA.INTFLAGS & PIN2_bm){
        PORTD.OUT = PIN5_bm;
        _delay_ms(100);
        meny = 3;
        PORTD.OUTCLR = PIN5_bm;
        
    }
    PORTA.INTFLAGS &= PIN2_bm;
}

uint16_t randomizer(uint16_t seed){
    srand(seed);
    int i = F_0 + rand() % (F_0 - F_1);
    return i;
}


uint16_t resultat(uint16_t verdi_1, uint16_t verdi_2){
    uint16_t res = abs(verdi_2 - verdi_1);
    return res;
}
