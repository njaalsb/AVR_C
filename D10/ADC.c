/* 
 * File:   main.c
 * Author: bruhe
 *
 * Created on February 10, 2025, 3:11 PM
 * 
 * PINOUT:
 * PD5 = Signal 2 inn
 * 
 * Kilder: 
 * - Getting Started with Analog-to-Digital Converter (ADC). Microchip 10/2/2025
 * - ERT 23. Carl Richard Steen Fosse, NTNU, 10/2/2025
 */


 #include "ADC.h"
 
void ADC0_init(void)
{
    /* Disable digital input buffer */
    PORTD.PIN6CTRL &= ~PORT_ISC_gm;
    PORTD.PIN6CTRL |= PORT_ISC_INPUT_DISABLE_gc;
    
    /* Disable pull-up resistor */
    PORTD.PIN6CTRL &= ~PORT_PULLUPEN_bm;
    
    ADC0.CTRLC = ADC_PRESC_DIV256_gc; /* CLK_PER divided by 256 */
    ADC0.CTRLA = ADC_ENABLE_bm /* ADC Enable: enabled */
               | ADC_RESSEL_10BIT_gc; /* 10-bit mode */
    
    /* Select ADC channel */
    ADC0.MUXPOS = ADC_MUXPOS_AIN5_gc;
    VREF.ADC0REF |= VREF_REFSEL_VDD_gc; // Bruk heller lavere ref ved lavere spenninger: VREF_REFSEL_2V048_gc
    
    ADC0.INTCTRL |= ADC_RESRDY_bm; // Aktiver interrupt
    ADC0.CTRLA |= ADC_FREERUN_bm; // Sett freerun-modus
}
