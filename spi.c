
// SPI DAC setup & data transfers

#include <avr/io.h>

void setup_spi() {
	
	DDRA |= (1<<PA6); 	// PA6=SS (low)
	PORTA |= (1<<PA6);  // start as disabled = 1
	DDRA |= (1<<PA5);	// DO
	DDRA |= (1<<PA4);  	// USCK
}

void spi_write_word_dac(uint16_t data) {	 // int 0-4095

	uint8_t usi_noclock = (1<<USIWM0) | (1<<USITC) ;
	uint8_t usi_clock = (1<<USIWM0) | (1<<USITC) | (1<<USICLK) ;
	
	// CS -> low
	PORTA &= ~(1<<PA6);

	USIDR = data >> 8 ;		// highest 8 bits (msb)...

	USICR = usi_noclock;
	USICR = usi_clock;
	USICR = usi_noclock;
	USICR = usi_clock;
	USICR = usi_noclock;
	USICR = usi_clock;
	USICR = usi_noclock;
	USICR = usi_clock;
	USICR = usi_noclock;
	USICR = usi_clock;
	USICR = usi_noclock;
	USICR = usi_clock;
	USICR = usi_noclock;
	USICR = usi_clock;
	USICR = usi_noclock;
	USICR = usi_clock;

	USIDR = data & 0xff;	// lowest 8 bits (lsb)...

	USICR = usi_noclock;
	USICR = usi_clock;
	USICR = usi_noclock;
	USICR = usi_clock;
	USICR = usi_noclock;
	USICR = usi_clock;
	USICR = usi_noclock;
	USICR = usi_clock;
	USICR = usi_noclock;
	USICR = usi_clock;
	USICR = usi_noclock;
	USICR = usi_clock;
	USICR = usi_noclock;
	USICR = usi_clock;
	USICR = usi_noclock;
	USICR = usi_clock;
	
	// CS -> high
	PORTA |= (1<<PA6);
	;
}

void spi_write_word(uint16_t data) {
	spi_write_word_dac(data | 0x3000);
}




