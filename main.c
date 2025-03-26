/*
 * main.c
 *
 * Created: 7/5/2022 9:48:26 AM
 * Author: Microchip Technology
 *
 * Fungerende kode til D10, mangler optimalisering, dette er en backup
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
#include "filter/sine.h"
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

 #define NFFT 256

 volatile int out = (NFFT/2)+1;
 
 kiss_fft_scalar cpx_in[NFFT]; 
 
 void fill_buffer(void){
    for(int m =0;m<NFFT;m++){               // fill cpx_in with nfft data points from sinewave[]
     	cpx_in[m] = ADC0_read();   
    };
 }
 
 int main(void)
 {
     ADC0_init();
     clkctrl_init();
     USART3_Initialize();
     
    uint32_t retVal = 0;
 
    retVal = SSD1306_Init(SSD1306_ADDR);
    
     //Configuring pin for time performance measurement
     PORTD.DIRSET = PIN4_bm;
     
     
     kiss_fftr_cfg cfg = kiss_fftr_alloc(NFFT, 0, NULL, NULL);  //kiss_fft_alloc( nfft ,is_inverse_fft ,0,0 );
 
     kiss_fft_cpx cpx_out[out];
     
     fill_buffer();

     volatile int16_t watch_real=4;
     volatile int16_t watch_imag = 35;
     volatile uint32_t pwr;
     volatile uint16_t cnt=0;

     PORTD.OUTSET = PIN4_bm; // Make PD4 output logic high
     kiss_fftr(cfg, cpx_in , cpx_out);      // The actual FFT operation
     PORTD.OUTCLR = PIN4_bm; // Make PD4 output logic low

     while(1) {
        SSD1306_ClearScreen();  
        fill_buffer();
        kiss_fftr(cfg, cpx_in , cpx_out);      // The actual FFT operation
        
        uint8_t counter = out -2;
        
        for(int n=0;n<out;n++)
        {
            //putting cpx_out.r  into watchable variables
            cnt = n;
            watch_real = cpx_out[n].r;
            watch_imag = cpx_out[n].i;
 
            //Calculating the power spectrum
            pwr = 20*log10(sqrt(cpx_out[n].r * cpx_out[n].r + cpx_out[n].i * cpx_out[n].i));
            
            // Tegner en linje med høyde 'pwr' for hver iterasjon
            SSD1306_DrawLine (counter,counter,0, pwr);
            counter--;
            //Writing four variables to USART, so they can be read by MPLAB DV:
            // 0. Start token
            // 1. watch_real (int16_t)
            // 2. watch_imag (int16_t)
            // 3. cnt (uint16_t))
            // 4. pwr (uint_32_t))
            // .. and the end token
            //variableWrite_SendFrame(watch_real, watch_imag, cnt, pwr);
         }
         SSD1306_UpdateScreen (SSD1306_ADDR);
         memset(cpx_in, 0, sizeof cpx_in);
     }
     kiss_fft_cleanup();                         // Tidy up after you.
 }
