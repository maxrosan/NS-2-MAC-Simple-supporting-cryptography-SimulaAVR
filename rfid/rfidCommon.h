/*
 * rfidCommon.h
 *
 *  Created on: 11/11/2014
 *      Author: max
 */

#ifndef RFID_RFIDCOMMON_H_
#define RFID_RFIDCOMMON_H_

#define P2P_COMMAND 2
#define ACK_VALUE 1
#define NACK_VALUE 0
#define TAG_VALUE 2
#define GOOD_BATT 1
#define BAD_BATT 0

#define MASK(N) ((1 << N) - 1)
#define MODE_FIELD(STATUS) ((STATUS >> 12) & MASK(4))
#define ACK(STATUS) ((STATUS >> 8) & 1)
#define TAG_TYPE(STATUS) ((STATUS >> 3) & MASK(3))
#define BATTERY(STATUS) (STATUS & 1)


#endif /* RFID_RFIDCOMMON_H_ */
