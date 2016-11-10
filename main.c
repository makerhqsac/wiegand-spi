/**
* Wiegand-SPI fimreware for ATTINY25/45/85
*
* main.c - main file
*
* (c) 2016 Sam Thompson <git@samt.us>
* The MIT License
*/
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "main.h"

/**
 * State flag
 *
 * flag[0:5]: 6-bit counter (0-63)
 * flag[6]: Error state
 * flag[7]: There is data to send
 */
volatile uint8_t flag;

/**
 * Raw input buffer from the Wiegand reader (reverse order)
 *
 * Wiegand transmits the MSB first, but the last byte in the ID first. We
 * accept the ID, storing each byte reversed, and consequently fixing the byte
 * order. When read back, we shift off each byte, reverse the order, and send
 * it down the USIDR.
 */
volatile uint64_t buffer;

/**
 * Pin Change0 Interrupt
 *
 * Register incomming bits from the Wiegand reader
 */
ISR(PCINT0_vect)
{
    if (flag & _BV(FLAG_HAS_DATA)) {
        flag = 0;
        buffer = 0ULL;
        WGD_OUT_REG &= ~_BV(WGD_IRQ);
    }

    if (bit_is_clear(WGD_IN_REG, WGD_D0) ^ bit_is_clear(WGD_IN_REG, WGD_D1)) {
        if (bit_is_clear(WGD_IN_REG, WGD_D1)) {
            buffer |= _BV_ULL(flag & FLAG_COUNTER_MSK);
        }

        // Reset timer, clock starts ticking again
        TCNT1 = T1_OFFSET;
        TCCR1 = T1_PRESCALE;

        flag++;

        return;
    }

    // When both bits are clear, the D1 and D0 lines are not high and
    // can therefore be considered disconnected.
    if (bit_is_clear(WGD_IN_REG, WGD_D0) && bit_is_clear(WGD_IN_REG, WGD_D1)) {
        // Turn off timer
        TCCR1 = 0;
        flag = 0;
        WGD_OUT_REG &= ~_BV(WGD_IRQ);
    }
}

/**
 * Timer1 Overflow Interrupt
 *
 * Used to timeout the Wiegand transmission
 */
ISR(TIM1_OVF_vect)
{
    // Turn off timer, fix USI
    TCCR1 = 0;
    USISR = _BV(USIOIF);

    flag &= FLAG_COUNTER_MSK;

    if (flag != 34 && flag != 26) {
        flag = 0;
        buffer = 0ULL;
        return;
    }

    // todo: check parity bits before discarding them
    buffer >>= 1;
    buffer &= ~_BV_ULL(flag - 2);

    // Transmission complete
    USIDR = flag; // Store bit count in register
    USIDR >>= 3; // Divide by 8 to get byte count

    // Signal that the transmission has completed
    flag = _BV(FLAG_HAS_DATA);
    WGD_OUT_REG |= _BV(WGD_IRQ);
}

/**
 * USI Overflow Interrupt
 *
 * Send the last byte of the buffer var to the USIDR.
 */
ISR(USI_OVF_vect)
{
    // Clear IRQ, USI counter
    WGD_OUT_REG &= ~_BV(WGD_IRQ);
    USISR = _BV(USIOIF);

    // Buffer to send (still backwards bits)
    USIDR = (uint8_t) (buffer & 0xFF);

    // Flip the order of bits in register
    USIDR = (USIDR & 0xF0) >> 4 | (USIDR & 0x0F) << 4;
    USIDR = (USIDR & 0xCC) >> 2 | (USIDR & 0x33) << 2;
    USIDR = (USIDR & 0xAA) >> 1 | (USIDR & 0x55) << 1;

    // Shift off the byte we've just read
    buffer >>= 8;
    flag = 0;
}

int main(void)
{
    // Wiegand Setup
    WGD_DIR_REG |= _BV(WGD_IRQ);                 // Outputs
    WGD_DIR_REG &= ~(_BV(WGD_D0) | _BV(WGD_D1)); // Inputs
    WGD_OUT_REG &= ~_BV(WGD_IRQ);              // Pull-ups

    PCMSK |= _BV(WGD_PCINT_D0) | _BV(WGD_PCINT_D1); // PCINT0 enable
    GIMSK |= _BV(PCIE); // Pin Change Interrupt Enable
    TIMSK |= _BV(TOIE1); // Timer0 Overflow Interrupt Enable

    TCCR1 = 0; // initialize timer, stopped

    // USI Setup
    USI_DIR_REG |= _BV(USI_DATAOUT_PIN);                        // Outputs
    USI_DIR_REG &= ~(_BV(USI_DATAIN_PIN) | _BV(USI_CLOCK_PIN)); // Inputs
    USI_OUT_REG |= _BV(USI_DATAIN_PIN) | _BV(USI_CLOCK_PIN);    // Pull-ups

    USICR = _BV(USIOIE)  // USI Overflow interrupt
        | _BV(USIWM0)    // 3 wire (SPI)
        | _BV(USICS1);   // Sample positive edge (SPI mode 0)

    // Initialize our state
    sei();
    flag = 0;
    buffer = 0ULL;

    // nop forever
    for (;;) {
        asm("nop");
    }

    return 0;
}
