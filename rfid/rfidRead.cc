/*
 * rfid_tag.cc
 *
 *  Created on: 11/11/2014
 *      Author: max
 */

#define INTERVAL_TO_SCAN_AGAIN 5

#include "rfidRead.h"

static class RfidReadClass : public TclClass {
public:
	RfidReadClass() : TclClass("Agent/RfidReader") {}
	TclObject* create(int, const char*const*) {
		return (new RfidReadAgent());
	}
} class_rfidReader;

RfidReadAgent::RfidReadAgent() : Agent(PT_RFID) {

	interrogatorID = rand();
	windowSize = 53 * 2;
	windowTimer = new WindowTimer(this);
	numberOfSlots = 20;
	//recollectionTimer = new RecollectionTimer(this);
	wakeUpCommand = new WakeUpCommand(this, 0.5, 2.5);

	bind("packetSize_", &size_);

	commandsMap["start"] = &RfidReadAgent::start;

	state = READER_IDLE;
}

void RfidReadAgent::recvTagResponse(Packet *pkt) {

	TagRecognized tagRecognized;
	//RfidReadRemoveIfCollided removeCollided;

	hdr_ip* hdrip = hdr_ip::access(pkt);
	hdr_rfid* hdr = hdr_rfid::access(pkt);

	/*if (currentSlot > hdr->slotChosen) {
		return;
	}*/

	//removeCollided.slot = hdr->slotChosen;

	/*if (std::find_if(tagsRecognized.begin(), tagsRecognized.end(), removeCollided)
	!= tagsRecognized.end()) {
		tagsRecognized.remove_if(removeCollided);
		return;
	}*/

	tagRecognized.tagID = hdr->tagID;
	tagRecognized.battery = hdr->tagStatus.battery;
	tagRecognized.slot = hdr->slotChosen;

	tagsRecognized.push_back(tagRecognized);

	currentSlot = hdr->slotChosen;

	Packet::free(pkt);
}

void RfidReadAgent::recv(Packet *pkt, Handler *h) {

	hdr_ip* hdrip = hdr_ip::access(pkt);
	hdr_rfid* hdr = hdr_rfid::access(pkt);

	fprintf(stderr, "READER [%u] received [%u]\n", hdr->tagID);

	if (state == READER_WINDOW && hdrip->daddr() == IP_BROADCAST) {

		if (hdr->tagStatus.modeField == P2P_COMMAND && // mode field
				hdr->tagStatus.ack == ACK_VALUE) { // ack
			recvTagResponse(pkt);
		}

	} else {

		fprintf(stderr, "READER [%u] dropping [%u]\n", hdr->tagID);

	}

}

int RfidReadAgent::start(int argc, const char*const* argv) {

	//sendCollection();
	wakeUpCommand->startSendingWakeUp();

	EnergyModel *em = Node::get_node_by_address(addr())->energy_model();

	if (em) {
		em->setenergy(1e6);
		//em->start_powersaving();
	}

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

void RfidReadAgent::sendWakeUp() {

	Packet* pkt = allocpkt();
	hdr_ip* ipHeader = HDR_IP(pkt);
	hdr_rfid *rfidHeader = hdr_rfid::access(pkt);

	fprintf(stderr, "Sending wakeup\n");

	memset(rfidHeader, 0, sizeof(hdr_rfid));

	rfidHeader->commandPrefix = 0xFF; // Wake up

	ipHeader->daddr() = IP_BROADCAST;
	ipHeader->dport() = ipHeader->sport();
	ipHeader->saddr() = here_.addr_; //Source: reader ip
	ipHeader->sport() = here_.port_;

	send(pkt, (Handler*) 0);

}

void RfidReadAgent::sendCollection() {

	Packet* pkt = allocpkt();
	hdr_ip* ipHeader = HDR_IP(pkt);
	hdr_rfid *rfidHeader = hdr_rfid::access(pkt);

	fprintf(stderr, "Sending collection\n");

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
	currentSlot = 0;

	windowTimer->schedule(((double) windowSize) * 1e-3);

	tagsRecognized.clear();

	send(pkt, (Handler*) 0);
}

void RfidReadAgent::sendSleep(uint32_t tagID) {

	Packet* pkt = allocpkt();
	hdr_ip* ipHeader = HDR_IP(pkt);
	hdr_rfid *rfidHeader = hdr_rfid::access(pkt);

	fprintf(stderr, "Sending collection\n");

	memset(rfidHeader, 0, sizeof(hdr_rfid));

	rfidHeader->commandPrefix = 31;
	rfidHeader->commandType = 1; // 1: broadcast or p2p 0: owner not present
	rfidHeader->interrogatorID = interrogatorID;
	rfidHeader->tagID = tagID;
	rfidHeader->commandCode = 15;
	rfidHeader->CRC = 0; // not calculated yet

	ipHeader->daddr() = IP_BROADCAST;
	ipHeader->dport() = ipHeader->sport();
	ipHeader->saddr() = here_.addr_; //Source: reader ip
	ipHeader->sport() = here_.port_;

	send(pkt, (Handler*) 0);
}

void RfidReadAgent::endOfWindow() {

	state = READER_IDLE;

	std::list<TagRecognized>::const_iterator it = tagsRecognized.begin();

	for (; it != tagsRecognized.end(); it++) {
		fprintf(stderr, "tag recognized = %u %u\n", it->tagID, it->slot);
		sendSleep(it->tagID);
	}

	tagsRecognized.clear();

	//recollectionTimer->schedule(INTERVAL_TO_SCAN_AGAIN);
	//wakeUpCommand->startSendingWakeUp();
	wakeUpCommand->startSendingWakeUpAfter(INTERVAL_TO_SCAN_AGAIN);

}
