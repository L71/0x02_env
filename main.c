#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "pwm_timer.h"
#include "adc.h"
#include "spi.h"

int main() {

	// setup input IO pins (PB0 / PB1 are inputs by default, just set PB1 pull-up)
	PORTB |= 0x02;
	
	// also set PA7 as output (GATE THRU output signal)
	DDRA |= _BV(PA7);


	pwm_timer_setup();

	setup_spi();

	adc_setup();

	run_adc_timer();

    // Enable interrupts for all. GO! =)
    SREG |= 0x80 ;
	
    // main loop
    while ( 1 )  {
		// OCR0A = 0;
        _delay_ms(3);
		// OCR0A = 255;
		_delay_ms(2);
    }
  
}
