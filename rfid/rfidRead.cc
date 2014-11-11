/*
 * rfid_tag.cc
 *
 *  Created on: 11/11/2014
 *      Author: max
 */

#include "rfidRead.h"

static class RfidReadClass : public TclClass {
public:
	RfidReadClass() : TclClass("Agent/RfidReader") {}
	TclObject* create(int, const char*const*) {
		return (new RfidReadAgent());
	}
} class_rfidReader;

RfidReadAgent::RfidReadAgent() : Agent(PT_RFID) {

	interrogatorID = 0x1;
	windowSize = 53;
	windowTimer = new WindowTimer(this);
	numberOfSlots = 10;

	bind("packetSize_", &size_);

	commandsMap["start"] = &RfidReadAgent::start;

	state = READER_IDLE;
}

void RfidReadAgent::recv(Packet *pkt, Handler *h) {

//	hdr_ip* hdrip = hdr_ip::access(pkt);
//	hdr_rfid* hdr = hdr_rfid::access(pkt);

}

int RfidReadAgent::start(int argc, const char*const* argv) {

	sendCollection();

	return (TCL_OK);
}

int RfidReadAgent::command(int argc, const char*const* argv) {

	if (argc > 1) {
		std::string nameCommand = std::string(argv[1]);
		pointer_map::const_iterator it = commandsMap.find(nameCommand);

		if (it != commandsMap.end()) {
			funcCall call = it->second;
			return (this->*call)(argc, argv);
		} else {

			fprintf(stderr, "Command %s not found\n", argv[1]);

		}

	}

	return (Agent::command(argc, argv));
}

void RfidReadAgent::sendCollection() {

	Packet* pkt = allocpkt();
	hdr_ip* ipHeader = HDR_IP(pkt);
	hdr_rfid *rfidHeader = hdr_rfid::access(pkt);

	memset(rfidHeader, 0, sizeof(hdr_rfid));

	rfidHeader->commandPrefix = 31;
	rfidHeader->commandType = 00; // 1: broadcast or p2p 0: owner not present
	rfidHeader->interrogatorID = interrogatorID;
	rfidHeader->commandCode = 10;
	rfidHeader->windowSize = windowSize;
	rfidHeader->numberOfSlots = numberOfSlots;
	rfidHeader->CRC = 0; // not calculated yet

	ipHeader->daddr() = IP_BROADCAST;
	ipHeader->dport() = ipHeader->sport();
    ipHeader->saddr() = here_.addr_; //Source: reader ip
    ipHeader->sport() = here_.port_;

    state = READER_WINDOW;

    windowTimer->schedule(((double) windowSize) * 1e-3);

    send(pkt, (Handler*) 0);
}

void RfidReadAgent::endOfWindow() {

	state = READER_SENDING_SLEEP;

}
