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

#ifdef AES

#include "aes.h"
#define BLOCK_SIZE 16

#elif RC6

#include "rc5.h"
#define BLOCK_SIZE 8

#elif XTEA

#include "xtea.h"
#define BLOCK_SIZE 8

#elif DES

#include "des.h"
#define BLOCK_SIZE 8

#endif


#include <avr/interrupt.h>
#include <string.h>

int main (void) {

#ifdef AES
	uint8_t key[32] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20, 21,22,23,24,25,26,27,28,29,30,31};
#elif RC6
	uint8_t key[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
#elif XTEA
	uint8_t key[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
	uint8_t temp[8];
#elif DES
	uint8_t key[8] = {0,1,2,3,4,5,6,7};
	uint8_t temp[8];
#endif

	uint8_t data[BLOCK_SIZE];
	uint8_t size;
	uint8_t cmd = 'N'; // N = nop; E = encrypt; D = decrypt
	uint8_t i, j, k;

	data[BLOCK_SIZE - 1] = 0;

	serial_init();
	sei();

#ifdef AES
	aes256_ctx_t ctx;
	aes256_init(key, &ctx);
#elif RC6
	rc5_ctx_t ctx;
	rc5_init(key, 128, 20, &ctx);
#endif

	for (;;) {

		if (serial_rxchars() != 0) {
			uint8_t c = serial_popchar();

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

					if (i == (BLOCK_SIZE - 1)) {
						data[(BLOCK_SIZE - 1)] = 0;
#ifdef AES
						aes256_enc(data, &ctx);
#elif RC6
						rc5_enc(data, &ctx);
#elif XTEA
						xtea_enc(temp, data, key);
						memcpy(data, temp, sizeof temp);
#elif DES
						des_enc(temp, data, key);
						memcpy(data, temp, sizeof temp);
#endif
						for (j = 0; j < BLOCK_SIZE; j++) serial_writechar(data[j]);
						i = 0;

					}
				}

				if (i > 0) {
					for (; i < (BLOCK_SIZE - 1); i++) data[i] = ' ';
					data[(BLOCK_SIZE - 1)] = 0;
#ifdef AES
					aes256_enc(data, &ctx);
#elif RC6
					rc5_enc(data, &ctx);
#elif XTEA
					xtea_enc(temp, data, key);
					memcpy(data, temp, sizeof temp);
#elif DES
					des_enc(temp, data, key);
					memcpy(data, temp, sizeof temp);
#endif

					for (j = 0; j < BLOCK_SIZE; j++) serial_writechar(data[j]);
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

					if (i == BLOCK_SIZE) {
#ifdef AES
						aes256_dec(data, &ctx);
#elif RC6
						rc5_dec(data, &ctx);
#elif XTEA
						xtea_dec(temp, data, key);
						memcpy(data, temp, sizeof temp);
#elif DES
						des_dec(temp, data, key);
						memcpy(data, temp, sizeof temp);
#endif
						for (j = 0; j < (BLOCK_SIZE - 1); j++) serial_writechar(data[j]);
						i = 0;
					}

				}

				cmd = 'N';
				serial_writestr_P(PSTR("END"));

			}


		}
	}
}
