#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "pwm_timer.h"
#include "adc.h"

int main() {

	// setup input IO pins (PB0 / PB1 are inputs by default, just set PB1 pull-up)
	PORTB |= 0x02;


	pwm_timer_setup();

	adc_setup();

	run_adc_timer();
	
	OCR0A = 127;
	OCR0B = 127;
	
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
