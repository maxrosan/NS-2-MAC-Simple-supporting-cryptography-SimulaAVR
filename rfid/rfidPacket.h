/*
 * rfidPacket.h
 *
 *  Created on: 11/11/2014
 *      Author: max
 */

//#ifndef RFID_RFIDPACKET_H_
#define RFID_RFIDPACKET_H_

#include "packet.h"

#include <stdint.h>

	struct hdr_rfid {

	uint16_t tagStatus;
	uint8_t messageLength;
	uint8_t intID;
	uint32_t tagID;
	uint32_t ownerID;
	char data[256];
	uint16_t CRC;

	uint8_t commandPrefix;
	uint8_t commandType;
	uint8_t commandCode;
	uint8_t interrogatorID;
	uint16_t windowSize;
	uint16_t numberOfSlots;

	// Header access methods
	static int offset_; // required by PacketHeaderManager
	inline static int& offset() { return offset_; }
	inline static hdr_rfid* access(const Packet* p) {
		return (hdr_rfid*) p->access(offset_);
	}

};

//#endif /* RFID_RFIDPACKET_H_ */
