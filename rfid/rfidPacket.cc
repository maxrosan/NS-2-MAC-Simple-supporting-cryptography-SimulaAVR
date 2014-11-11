/*
 * rfidPacket.cpp
 *
 *  Created on: 11/11/2014
 *      Author: max
 */

#include "rfidPacket.h"

int hdr_rfid::offset_;
static class RfidPacketHeaderClass : public PacketHeaderClass {
public:
	RfidPacketHeaderClass() : PacketHeaderClass("PacketHeader/RfidPacket",
					      sizeof(hdr_rfid)) {
		bind_offset(&hdr_rfid::offset_);
	}
} class_rfidPackethdr;
