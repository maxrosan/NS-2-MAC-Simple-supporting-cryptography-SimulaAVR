 /*
 ****************************************************************************
 *
 * simple_serial - A demo for the SimulAVR simulator.
 * Copyright (C) 2013 Markus Hitter <mah@jump-ing.de>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 ****************************************************************************
 *
 *  $Id$
 *
 * Super trivial example exercising the UART serial communications line.
 *
 * It's purpose is to show how SimulAVR can redirect serial communications
 * in a way useful for running in a simulator, while requiring NO code
 * modifications which would change it's behaviour compared to running
 * on real hardware or disallowing to run the very same compiled binary
 * on that hardware. The compiled binary should work in the simulator just
 * as fine as on hardware.
 */

#include "serial.h"

// AES
#include "aes.h"

#include <avr/interrupt.h>

void aes256_test(void) {
    // aes256 is 32 byte key
    uint8_t key[32] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20, 21,22,23,24,25,26,27,28,29,30,31};

    // aes256 is 16 byte data size
    uint8_t data[16] = "text to encrypt";

    // declare the context where the round keys are stored
    aes256_ctx_t ctx;

    // Initialize the AES256 with the desired key
    aes256_init(key, &ctx);

    // Encrypt data 
    // "text to encrypt" (ascii) -> '9798D10A63E4E167122C4C07AF49C3A9'(hexa)
    aes256_enc(data, &ctx);

    // Decrypt data
    // '9798D10A63E4E167122C4C07AF49C3A9'(hexa) -> "text to encrypt" (ascii)
    aes256_dec(data, &ctx);
}

// This is all we need:
int main (void) {

  uint8_t key[32] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20, 21,22,23,24,25,26,27,28,29,30,31};
  uint8_t data[16];
  uint8_t size;
  uint8_t cmd = 'N'; // N = nop; E = encrypt; D = decrypt
  uint8_t i, j, k;

  data[15] = 0;

  serial_init();
  sei();

 aes256_ctx_t ctx;
 aes256_init(key, &ctx);

  //serial_writestr_P(PSTR("Hello, world!\n\nNow, please type:\n"));

  for (;;) {

    if (serial_rxchars() != 0) {
      uint8_t c = serial_popchar();
      //serial_writestr_P(PSTR("received: <"));

      if (cmd == 'N') {
        if (c == 'E' || c == 'D') {
		cmd = c;
        }
      } else if (cmd == 'E') {
	size = c;
        i = 0;
	k = 0;

        while (k < size) {
	  if (serial_rxchars() == 0) continue;
	  uint8_t m = serial_popchar();
	  data[i] = m;
	  i++; k++;

          if (i == 15) {
		data[15] = 0;
		aes256_enc(data, &ctx);
		for (j = 0; j < 16; j++) serial_writechar(data[j]);
		i = 0;

          }
        }

	if (i > 0) {
		for (; i < 15; i++) data[i] = ' ';
		data[15] = 0;
		aes256_enc(data, &ctx);
		for (j = 0; j < 16; j++) serial_writechar(data[j]);
	}
	
	serial_writestr_P(PSTR("END"));
	
	cmd = 'N';

      } else if (cmd == 'D') {

	size = c;
        i = 0;
	k = 0;

        while (k < size) {
		if (serial_rxchars() == 0) continue;
		uint8_t m = serial_popchar();
		data[i] = m;
		i++; k++;

		if (i == 16) {
			aes256_dec(data, &ctx);
			for (j = 0; j < 15; j++) serial_writechar(data[j]);
			i = 0;
		}

	}

	cmd = 'N';
	serial_writestr_P(PSTR("END"));

      }
      
      //serial_writechar(c);
      //serial_writestr_P(PSTR("\n"));

  }
 }
}
