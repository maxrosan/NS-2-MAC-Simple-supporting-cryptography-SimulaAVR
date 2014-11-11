
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

	bind("packetSize_", &size_);

}

void RfidTagAgent::collectId(hdr_rfid *hdr) {

	double windowSize = 0;
	double slotSize = 0;
	double delay;
	int slotChosen = 0;

	windowSize = ((double) hdr->windowSize)  * 1e-3;
	slotSize = windowSize / ((double) hdr->numberOfSlots);

	slotChosen = (rand() % hdr->numberOfSlots);
	delay = slotSize * ((double) slotChosen);

	fprintf(stderr, "Collect my ID [%u]\n", tagID);
	fprintf(stderr, "windowSize = %u\n", hdr->windowSize);
	fprintf(stderr, "number of slots = %u\n", hdr->numberOfSlots);
	fprintf(stderr, "slot chosen = %u\n", slotChosen);
	fprintf(stderr, "delay = %f\n", delay);

	collectResponseTimer->schedule(hdr, delay);

}

void RfidTagAgent::responseCollect(hdr_rfid *hdr) {
	fprintf(stderr, "reponse\n");

}

void RfidTagAgent::recv(Packet *pkt, Handler *h) {

	  hdr_ip* hdrip = hdr_ip::access(pkt);
	  hdr_rfid* hdr = hdr_rfid::access(pkt);

	  fprintf(stderr, "Reader [%u] identified\n", hdrip->saddr());

	  if (hdrip->daddr() == IP_BROADCAST) {

		  if (hdr->commandPrefix == 31) {
			  switch (hdr->commandCode) {
			  case 10:
				  collectId(hdr); break;
			  }
		  }

	  }

}

int RfidTagAgent::command(int argc, const char*const* argv) {
	return (Agent::command(argc, argv));
}
