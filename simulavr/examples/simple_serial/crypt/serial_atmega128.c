#include "serial.h"
#include "uart.h"

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#define BAUD 19200

// initialise serial subsystem
void serial_init() {
	uart_init( BAUD );
}


// check how many characters can be read
uint8_t serial_rxchars() {
	return uart_available();
}

// read one character
uint8_t serial_popchar() {
	uint8_t c = 0;

	c = uart_getc();

	return c;
}

// send one character
void serial_writechar(uint8_t data) {
	uart_putc(data);
}

// send a string
void serial_writestr(uint8_t *data) {
	uint8_t i = 0, r;

	while ((r = data[i++]))
		serial_writechar(r);
}

void serial_writestr_P(PGM_P data) {
	uint8_t r, i = 0;
	// Yes, this is *supposed* to be assignment rather than comparison, so we
	// break when r is assigned zero.
	while ((r = pgm_read_byte(&data[i++])))
		serial_writechar(r);
}
