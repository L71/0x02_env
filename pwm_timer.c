// PWM timer setup 

#include <avr/io.h>


void pwm_timer_setup() {
	
	// activate pins for OCR0A PWM output
	DDRB |= (1<<PB2);
	
	// setup for fast PWM
	TCCR0A = (1<<COM0A1)|(1<<WGM01)|(1<<WGM00);
	TCCR0B = (1<<CS00);
	
	// OCR0A = voltage output
	// OCR0B = LED driver
	
}
