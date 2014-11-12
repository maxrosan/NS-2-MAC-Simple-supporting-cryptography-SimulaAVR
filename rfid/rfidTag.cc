
#include "rfidTag.h"

#include <cstdlib>
#include <ctime>

static class RfidTagClass : public TclClass {
public:
	RfidTagClass() : TclClass("Agent/RfidTag") {}
	TclObject* create(int, const char*const*) {
		return (new RfidTagAgent());
	}
} class_rfidReader;

RfidTagAgent::RfidTagAgent() : Agent(PT_RFID) {

	tagID = rand();
	collectResponseTimer = new CollectResponseTimer(this);
	state = TAG_IDLE;

	bind("packetSize_", &size_);

}

void RfidTagAgent::collectId(Packet *p, hdr_rfid *hdr) {

	double windowSize = 0;
	double slotSize = 0;
	double delay;
	int slotChosen = 0;

	if (state != TAG_IDLE) {
		return;
	}

	if ((hdr->commandType & (1 << 1)) &&
			hdr->tagID != tagID) {

		return;
	}

	windowSize = ((double) hdr->windowSize)  * 1e-3;
	slotSize = windowSize / ((double) hdr->numberOfSlots);

	slotChosen = (rand() % hdr->numberOfSlots);
	delay = slotSize * ((double) slotChosen);

	fprintf(stderr, "Collect my ID [%u]\n", tagID);
	fprintf(stderr, "windowSize = %u\n", hdr->windowSize);
	fprintf(stderr, "number of slots = %u\n", hdr->numberOfSlots);
	fprintf(stderr, "slot chosen = %u\n", slotChosen);
	fprintf(stderr, "delay = %f\n", delay);

	state = TAG_WAITING_SLOT;

	collectResponseTimer->schedule(p, hdr, slotChosen, delay);

}

void RfidTagAgent::responseCollect(Packet *packet, hdr_rfid *hdr, int slot) {

	hdr_ip* intIpHeader = HDR_IP(packet);

	Packet* pkt = allocpkt();
	hdr_ip* ipHeader = HDR_IP(pkt);
	hdr_rfid *rfidHeader = hdr_rfid::access(pkt);

	state = TAG_IDLE;

	memset(rfidHeader, 0, sizeof(hdr_rfid));

	rfidHeader->tagStatus.modeField = P2P_COMMAND;
	rfidHeader->tagStatus.ack = ACK_VALUE;
	rfidHeader->tagStatus.battery = GOOD_BATT;
	rfidHeader->tagStatus.tagType = TAG_VALUE;
	rfidHeader->tagStatus.reserved = 0;

	rfidHeader->messageLength = 0; // not calculated yet
	rfidHeader->interrogatorID = hdr->interrogatorID;
	rfidHeader->tagID = tagID;
	rfidHeader->ownerID = 0;
	rfidHeader->CRC = 0; // not calculated yet
	rfidHeader->slotChosen = slot;

	ipHeader->daddr() = intIpHeader->saddr();
	ipHeader->dport() = intIpHeader->sport();
	ipHeader->saddr() = here_.addr_; //Source: reader ip
	ipHeader->sport() = here_.port_;

	send(pkt, (Handler*) 0);

}

void RfidTagAgent::recv(Packet *pkt, Handler *h) {

	hdr_ip* hdrip = hdr_ip::access(pkt);
	hdr_rfid* hdr = hdr_rfid::access(pkt);

	fprintf(stderr, "Reader [%u] identified\n", hdrip->saddr());

	if (hdrip->daddr() == IP_BROADCAST) {

		if (hdr->commandPrefix == 31) {

			switch (hdr->commandCode) {
			case 10:
				collectId(pkt, hdr); break;
			}
		}

	}

}

int RfidTagAgent::command(int argc, const char*const* argv) {
	return (Agent::command(argc, argv));
}
