/* 
 * File:   main.c
 * Author: bruhe
 *
 * Created on February 10, 2025, 3:11 PM
 * 
 * PINOUT:
 * PD6 = DAC
 * PD7 = Signal 1 inn
 * PD5 = Signal 2 inn
 * 
 * Kilder: 
 * - Getting Started with Analog-to-Digital Converter (ADC). Microchip 10/2/2025
 * - ERT 23. Carl Richard Steen Fosse, NTNU, 10/2/2025
 * 
 * Beskrivelse:
 * Denne koden leser to signaler ved hjelp av ADC på PD7 og PD5, signalene legges sammen og skrives deretter 
 * ut ved hjelp av DAC på pinne PD6. ADC'en bruker to forskjellige kanaler her, kanal 5 og 7, dette samsvarer
 * med pinne-nummerene. ADC-en kan ikke lese fra to kanaler samtidig og bruker derfor ADC_MUXPOS_AINX_gc til å bytte 
 * mellom de to forskjellige kanalene.
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <string.h>

#define F_CPU 4000000UL

#include <util/delay.h>

uint16_t adcVal;
void ADC0_init(void);
uint16_t ADC0_read(void);

void DAC0_init(void);
void DAC0_set_val(uint16_t val);

int main() {
    ADC0_init();
    DAC0_init();
    while(1){
        // Channel 7
        ADC0.MUXPOS = ADC_MUXPOS_AIN7_gc;
        adcVal = ADC0_read();
        
        // Channel 5
        ADC0.MUXPOS = ADC_MUXPOS_AIN5_gc;
        adcVal += ADC0_read();
        
        DAC0_set_val(adcVal);
    }
    return (EXIT_SUCCESS);
}

void ADC0_init(void)
{
    /* Disable digital input buffer */
    PORTD.PIN6CTRL &= ~PORT_ISC_gm;
    PORTD.PIN6CTRL |= PORT_ISC_INPUT_DISABLE_gc;
    
    /* Disable pull-up resistor */
    PORTD.PIN6CTRL &= ~PORT_PULLUPEN_bm;
    ADC0.CTRLC = ADC_PRESC_DIV2_gc; /* CLK_PER divided by 2 */
    ADC0.CTRLA = ADC_ENABLE_bm /* ADC Enable: enabled */
               | ADC_RESSEL_10BIT_gc; /* 10-bit mode */
    
    /* Select ADC channel */
    ADC0.MUXPOS = ADC_MUXPOS_AIN5_gc;
    VREF.ADC0REF |= VREF_REFSEL_VDD_gc;
}

uint16_t ADC0_read(void)
{
    ADC0.COMMAND = ADC_STCONV_bm;
    /* Wait until ADC conversion done */
    while ( !(ADC0.INTFLAGS & ADC_RESRDY_bm) ){
        ;
    }
    
    ADC0.INTFLAGS = ADC_RESRDY_bm;
    return ADC0.RES;
}


void DAC0_init(void)
{
        /*PIN D6*/
    /* Disable digital input buffer */
    PORTD.PIN6CTRL &= ~PORT_ISC_gm;
    PORTD.PIN6CTRL |= PORT_ISC_INPUT_DISABLE_gc;
    /* Disable pull-up resistor */
    PORTD.PIN6CTRL &= ~PORT_PULLUPEN_bm;
    VREF.DAC0REF |= VREF_REFSEL_VDD_gc;
    /* Enable DAC, Output Buffer */
    DAC0.CTRLA = DAC_ENABLE_bm | DAC_OUTEN_bm;
}

void DAC0_set_val(uint16_t val){
    /* Store the two LSbs in DAC0.DATAL */
    DAC0.DATAL = (val & (0x03)) << 6;
    /* Store the eight MSbs in DAC0.DATAH */
    DAC0.DATAH = val >> 2;
}
