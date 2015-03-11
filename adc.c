// PWM timer setup 

#include <avr/io.h>
#include <avr/interrupt.h>

#include "spi.h"

// ADC latest values
struct adc_data {
	uint8_t attack, level_off, decay, level_on ;
} adc ;

// state machine tracking
#define ATTACK	1
#define DECAY	2
#define LOW		3
#define HIGH	4
volatile uint8_t env_complete = 0;

// current output env value
uint16_t currentlevel; 


void adc_setup() {
	// enable ADC, clock prescaler = /64
	ADCSRA |= (1<<ADEN) | (1<<ADPS2) | (1<<ADPS0) ;
	
	// left adjust result
	ADCSRB |= (1<<ADLAR);
	
	// disable digital IO on used ADC pins
	DIDR0 = 0b00001111;	
	
	// Go! =)
	ADCSRA |= (1<<ADSC) ;
}

void run_adc_timer() {	// enable 16bit timer1 + interrupts
	
	// OCR1AH = 0x1f; // 8MHz, divider 8000 -> 1KHz timer
	// OCR1AL = 0x40;
	
	OCR1AH = 0x03; // 8MHz, divider 1000 -> 8KHz timer
	OCR1AL = 0xe8;
	
	// clear on OCR1A, clock prescaler 1. 
	TCCR1B |= (1<<WGM12) | (1<<CS10) ;
	// TCCR1B |= (1<<WGM12) | (1<<CS10) | (1<<CS11) ;  // presc. /64
	
	// enable OCR1A interrupt
	TIMSK1 |= (1<<OCIE1A);
}

inline void setpwm(uint8_t value) {  // update PWM output registers
	if ( value == 0) {
		value = 1;
	}
	if ( value == 255 ) {
		value = 254;
	}
	OCR0A=value;
	OCR0B=value;
	spi_write_word(value << 4);
}

void env_update(uint8_t rate, uint8_t targetlevel_8) {
	
	uint16_t chrate;
	uint16_t targetlevel = (uint16_t)targetlevel_8 * 256 ;
	
	chrate=(uint16_t)rate; // make change rate 16bit
	
	// accelerate envelope change rate expnentially
	if (chrate == 0) {
		chrate=1;
	}
	if (chrate > 128) {
		chrate = chrate+((chrate-128)*2);
	}
	if (chrate > 192) {
		chrate = chrate+((chrate-192)*2);
	}
	if (chrate > 220) {
		chrate = chrate+((chrate-220)*2);
	}


	if ( currentlevel == targetlevel) {
		env_complete=1;
	} else {
		if ( currentlevel < targetlevel ) {	// env going up 
			if ( currentlevel <= (targetlevel-chrate)) {  // check, pass target level?
				currentlevel += chrate;
			} else {
				currentlevel = targetlevel;
				env_complete=1;
			} 
		} 
		if ( currentlevel > targetlevel ) { // env going down
			if ( currentlevel >= (targetlevel+chrate) ) { // check, pass target level?
				currentlevel -= chrate;
			} else {
				currentlevel = targetlevel;
				env_complete=1;
			}
		}
	}
	setpwm( (uint8_t) (currentlevel >> 8) );
}

void state_env() {
	
	uint8_t gate_went_high=0 ;	// event: gate went high
	uint8_t gate_went_low=0 ;	// event: gate went low
	
	static uint8_t prev_gate_level ; // saves previous gate level 
	
	uint8_t gate ; // gate input value
	uint8_t mode_gate ;	// gated or triggered mode?
	static uint8_t stage = LOW ; 	// curremt ENV stage
	
	gate = ( ~PINB & 0x01 );	// check gate input pin
	mode_gate = ( PINB & 0x02 ); // 0=trig, 1=gate
	
	if ( prev_gate_level == 0 ) {	
		if (gate) { // gate went low->high
			gate_went_high = 1 ;
			prev_gate_level = 1;
		}
	} else {	// prev state high, low now... 
		if (! gate) {
			gate_went_low = 1 ; 
			prev_gate_level = 0 ;
		}
	}
	
	// copy gate state to GATE THRU output (PA7)
	if (gate) {
		PORTA |= _BV(PA7);
	} else {
		PORTA &= ~_BV(PA7);
	}
	
	
	// state machine 
	switch (stage) {
	
		case LOW:
			if (gate_went_high) {
				env_complete=0;
				stage=ATTACK;
			} 
			setpwm(adc.level_off);
			currentlevel=adc.level_off*256;
			break;
			
		case ATTACK:
			env_update(adc.attack, adc.level_on);
			if (mode_gate) {	// gated mode
				if (env_complete) {	
					stage=HIGH;	// go to HIGH if env timer completed
					env_complete=0;
				}
				if (gate_went_low) {	// always go directly to DECAY if gate->low
					stage=DECAY;
					env_complete=0;
				}
			} else {  	// trigger mode
				if (env_complete) {
					stage=DECAY;	// go to DECAY if env timer completed
					env_complete=0;
				}
			}

			break;
			
		case HIGH:
			setpwm(adc.level_on);
			currentlevel=adc.level_on*256;
			if (gate_went_low) {	// go to DECAY if gate->low
				env_complete=0;
				stage=DECAY;
			}
			break;
			
		case DECAY:
			env_update(adc.decay, adc.level_off);
			if (gate_went_high) {	// always back to ATTACK if gate->high.
				stage=ATTACK;
				env_complete=0;
			}
				
			break;
	}	
	
}


ISR(TIM1_COMPA_vect) {
	
	static uint8_t adc_chan=0;

	
	uint8_t chan_mask=0x03; // use 4 channels (0-3).
	uint8_t adc_in = ADCH;
	
	switch (adc_chan) {
		case 0 :	// chan 0
			// adc.attack = adc_in;
			adc.level_on = adc_in;
			break;
		case 1 :	// chan 1
			// adc.level_off = adc_in;
			adc.attack = adc_in;
			break;
		case 2 :	// chan 2
			// adc.decay = adc_in;
			adc.level_off = adc_in;
			break;
		case 3 :	// chan 3
			// adc.level_on = adc_in;
			adc.decay = adc_in;
			break;
	}
	
	// OCR0B = adc.attack;

	adc_chan = ((adc_chan+1) & chan_mask ) ;
	
	// update ADMUX register
	ADMUX &= 0xf0;	// reset channel bits
	ADMUX |= adc_chan ;
	
	// start next conversion
	ADCSRA |= (1<<ADSC) ;
	
	// update ENV state machine 
	state_env();

}
