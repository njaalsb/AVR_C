/*
 * main.c
 *
 * Created: 7/5/2022 9:48:26 AM
 * Author: Microchip Technology
 *
 * Info: Dette prosjektet demonstrerer KissFFT-biblioteket (et C-bibliotek for beregning av FFT) på AVR128DB48. Det er basert på et eksempel fra Microchip. 
 * Testen gjennomføres ved at det tas FFT av en sinus lagret i minne. Effekten av hver frekvenskomponent i FFT blir så beregnet før dataen sendes over UART.
 * Det er deretter mulig å visualisere dataen i Data Visualizer i MPLAB X.rosjektet er konfigurert til å bruke fixed-point beregninger på 16 bit. 
 * Se README og GitHub (https://github.com/microchip-pic-avr-examples/avr64ea48-digital-filters-studio/tree/master/fft) 
 * for detaljert info om demonstrasjonen. 
 *
 * Adjusted for AVR128DB48: March, 2025
 * Editor: Carl Richard Steen Fosse
 */ 

#include <xc.h>
#include "C:\Users\bruhe\Downloads\kiss_fft_D10.X\filter\kiss_fftr.h"
#include "C:\Users\bruhe\Downloads\i2c_display_demo.X\i2c_display_demo.X\config.h"
#include "C:\Users\bruhe\Downloads\i2c_display_demo.X\i2c_display_demo.X\display\ssd1306.h"
#include "peripherals/clock/clkctrl.h"
#include "peripherals/usart/usart3.h"
#include "peripherals/data_streamer/data_streamer.h"
#include <avr/cpufunc.h>
#include <math.h>
#include "ADC.h"

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>
#include <avr/cpufunc.h>

#include <avr/interrupt.h>

#define NFFT 256

volatile int out = (NFFT/2)+1;

volatile uint8_t active_buffer;

//volatile uint8_t buffer_number;

//volatile uint8_t ADC_sample;

volatile kiss_fft_scalar cpx_in[NFFT]; 

volatile kiss_fft_scalar cpx_in_2[NFFT];

//volatile int16_t buff[NFFT];

uint16_t k = 0;

ISR(ADC0_RESRDY_vect){
    
    ADC0.INTFLAGS = ADC_RESRDY_bm;
            
    if(k < NFFT && active_buffer == 0){
        
        cpx_in[k] = ADC0_RES;
           
    }
    else if(k < NFFT && active_buffer == 1){

        cpx_in_2[k] = ADC0_RES;

    }
    else {
        ADC0.INTFLAGS &= ~ADC_RESRDY_bm;
    }
    
    k++;
}

void FFT(kiss_fft_scalar cpx[], kiss_fftr_cfg config, kiss_fft_cpx cpx_out[], volatile uint16_t *cnt, volatile int16_t *watch_real, volatile int16_t *watch_imag, volatile uint32_t *pwr){
    ADC0.INTFLAGS = ADC_RESRDY_bm;
    
    //PORTD.OUTSET = PIN4_bm;
    kiss_fftr(config, cpx , cpx_out);      // The actual FFT operation
    //PORTD.OUTCLR = PIN4_bm;
    
    uint16_t counter = 129;
    
    SSD1306_ClearScreen();
    for(int l=0;l<out;l++){
                //putting cpx_out.r  into watchable variables
                *cnt = l;
                *watch_real = cpx_out[l].r;
                *watch_imag = cpx_out[l].i;
                
                 //Calculating the power spectrum
                *pwr = sqrt(cpx_out[l].r * cpx_out[l].r + cpx_out[l].i * cpx_out[l].i);

                // Tegner en linje med høyde 'pwr' for hver iterasjon
                SSD1306_DrawLine (counter,counter,0, 20*log10(*pwr));
                counter--;
                
                //variableWrite_SendFrame(watch_real, watch_imag, cnt, pwr);
                
    }
    SSD1306_UpdateScreen (SSD1306_ADDR);
    memset(cpx, 0, sizeof *cpx);
}

int main(void)
{
    clkctrl_init();
    ADC0_init();
    
    
    //USART3_Initialize();
    
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
        PORTD.OUTSET = PIN4_bm;
        if(active_buffer == 0 && k >= NFFT){
            
            active_buffer = 1;
            
            FFT(cpx_in, cfg, cpx_out, &cnt, &watch_real, &watch_imag, &pwr);
            
            k = 0;
        } 
        
        if(active_buffer == 1 && k >= NFFT){  
            
            active_buffer = 0;
            
            FFT(cpx_in_2, cfg, cpx_out, &cnt, &watch_real, &watch_imag, &pwr);
            
            k = 0;
        }
    }
    kiss_fft_cleanup();                         // Tidy up after you.
}
