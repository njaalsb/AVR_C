/*
 * main.c
 * 
 * Designprosjekt 10
 *
 * Sist oppdatert: 7/5/2025
 */ 

#include <xc.h>
#include "C:\Users\bruhe\Downloads\kiss_fft_D10.X\filter\kiss_fftr.h"
#include "C:\Users\bruhe\Downloads\i2c_display_demo.X\i2c_display_demo.X\config.h"
#include "C:\Users\bruhe\Downloads\i2c_display_demo.X\i2c_display_demo.X\display\ssd1306.h"
#include "peripherals/clock/clkctrl.h"
#include <avr/cpufunc.h>
#include <math.h>
#include "ADC.h"

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <avr/cpufunc.h>

#include <avr/interrupt.h>

#define NFFT 256

volatile int out = (NFFT/2)+1;

volatile bool active_buffer;

volatile kiss_fft_scalar cpx_in[NFFT]; 

volatile kiss_fft_scalar cpx_in_2[NFFT];

volatile uint16_t k = 0;

ISR(ADC0_RESRDY_vect){
    
    ADC0.INTFLAGS = ADC_RESRDY_bm;
            
    if(k < NFFT && !active_buffer){
        
        cpx_in[k] = ADC0_RES;
           
    }
    else if(k < NFFT && active_buffer){

        cpx_in_2[k] = ADC0_RES;

    }
    else {
        ADC0.INTFLAGS &= ~ADC_RESRDY_bm;
    }
    
    k++;
}

void FFT(kiss_fft_scalar cpx[],
        kiss_fftr_cfg config,
        kiss_fft_cpx cpx_out[],
        volatile uint16_t *cnt,
        volatile int16_t *watch_real,
        volatile int16_t *watch_imag,
        volatile uint32_t *pwr)
{
    ADC0.INTFLAGS = ADC_RESRDY_bm;
    
    //PORTD.OUTSET = PIN4_bm;
    kiss_fftr(config, cpx , cpx_out);      // The actual FFT operation
    //PORTD.OUTCLR = PIN4_bm;
    
    volatile uint16_t counter = 129;
    volatile uint32_t amp = 0;
    
    SSD1306_ClearScreen();
    for(int l=0;l<out;l++){
        //putting cpx_out.r  into watchable variables
        *cnt = l;
        *watch_real = cpx_out[l].r;
        *watch_imag = cpx_out[l].i;

         //Calculating the power spectrum
        *pwr = sqrt(cpx_out[l].r * cpx_out[l].r + cpx_out[l].i * cpx_out[l].i);

        // Tegner en linje med høyde 'pwr' for hver iterasjon
        amp = 20*log10(*pwr);
        SSD1306_DrawLine (counter,counter,0, amp);
        counter--;
    }
    SSD1306_UpdateScreen (SSD1306_ADDR);
    memset(cpx_out, 0, sizeof(cpx_out)/sizeof(cpx_out[0]));
    active_buffer = !active_buffer;
    k = 0;
}

int main(void)
{
    clkctrl_init();
    ADC0_init();
    
    ADC0.COMMAND = ADC_STCONV_bm; // **START første konvertering**
    
    PORTD.DIRSET = PIN4_bm;
    
    sei();
    
    // Initialiserer displayet:
    uint32_t retVal = SSD1306_Init(SSD1306_ADDR);

    kiss_fftr_cfg cfg = kiss_fftr_alloc(NFFT, 0, NULL, NULL);  //kiss_fft_alloc( nfft ,is_inverse_fft ,0,0 );

    kiss_fft_cpx cpx_out[out];

    volatile int16_t watch_real=4;
    volatile int16_t watch_imag = 35;
    volatile uint32_t pwr;
    volatile uint16_t cnt=0;

    while(1) {
        //PORTD.OUTSET = PIN4_bm;
        if(!active_buffer && k >= NFFT){
            PORTD.OUTSET = PIN4_bm;
            FFT(cpx_in, cfg, cpx_out, &cnt, &watch_real, &watch_imag, &pwr);
        } 
        if(active_buffer && k >= NFFT){  
            FFT(cpx_in_2, cfg, cpx_out, &cnt, &watch_real, &watch_imag, &pwr);
            PORTD.OUTCLR = PIN4_bm;
        }
        
    }
    kiss_fft_cleanup();                         // Tidy up after you.
}
