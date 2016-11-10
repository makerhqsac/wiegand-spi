/**
* Wiegand-SPI fimreware for ATTINY25/45/85
*
* main.h - main header file
*
* (c) 2016 Sam Thompson <git@samt.us>
* The MIT License
*/
#ifndef _MAIN_H
#define _MAIN_H

/**
 * Wiegand I/O
 * These are parameterized should we want to support other MCUs
 */
#define WGD_DIR_REG             DDRB
#define WGD_OUT_REG             PORTB
#define WGD_IN_REG              PINB
#define WGD_D0                  PB3
#define WGD_D1                  PB4
#define WGD_IRQ                 PB5
#define WGD_PCINT_D0            PCINT3
#define WGD_PCINT_D1            PCINT4

/** Timeouts to end listening for a Wiegand transmission */
#define T1_OFFSET               64
#define T1_PRESCALE             (_BV(CS12) | _BV(CS11))

/**
 * USI I/O
 * These are parameterized should we want to support other MCUs
 */
#define USI_DIR_REG	            DDRB
#define USI_OUT_REG             PORTB
#define USI_IN_REG	            PIN
#define USI_CLOCK_PIN	          PB2
#define USI_DATAIN_PIN	        PB0
#define USI_DATAOUT_PIN	        PB1

/** The status flag has a 6-bit counter and two flags it can raise */
#define FLAG_COUNTER_MSK        0x3F
#define FLAG_ERROR              6
#define FLAG_HAS_DATA           7

/** Just like _BV() but for 64-bit integers */
#define _BV_ULL(b)              (1ULL<<(b))

#endif
