/* 
 * File:   newmain.c
 * Author: bruhe
 *
 * Created on January 31, 2025, 08:55 PM
 */

#define F_CPU 4000000UL
#define TIMEOUT_MS 1000 // Timeout i et sekund 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdio.h>

void TCA0_init(void){
    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV64_gc // Forhåndsskalerer: CLK_PER med /64
                        | TCA_SINGLE_ENABLE_bm; // Aktiverer timeren 
    TCA0.SINGLE.PER = 62; // Gir ca 1ms timer
}
// Vet ikke om denne funksjonen, men det inni while-løkka virker vertfall
void timeout(){
    TCA0_init();
    uint16_t prevmillis = TCA0.SINGLE.CNT;
    //error = false;
    //uint16_t i = 0;
    while (1)
    {
        // Timer 
        uint16_t curr_millis = TCA0.SINGLE.CNT - prevmillis;
        
        if(curr_millis >= TIMEOUT_MS){
            printf("%s\r\n", "Hengte seg opp...");
            break;
        }
    }
}

int main(void){
  
  while(1){
    
  timeout();
    
  }
}
