/* 
 * File:   ADC.h
 * Author: bruhe
 *
 * Created on March 18, 2025, 10:40 AM
 */

 #ifndef CONFIG_H
 #define	CONFIG_H
 
 #ifdef	__cplusplus
 extern "C" {
 #endif

 #include <stdio.h>
 #include <stdlib.h>
 #include <avr/io.h>
 #include <string.h>
 
 #define F_CPU 4000000UL
 
 #include <util/delay.h>

 uint16_t adcVal;
 void ADC0_init(void);
 uint16_t ADC0_read(void);

 #ifdef	__cplusplus
 }
 #endif

 #endif	/* CONFIG_H */
