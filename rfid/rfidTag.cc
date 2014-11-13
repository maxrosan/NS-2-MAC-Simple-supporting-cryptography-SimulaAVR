
#include "rfidTag.h"

#include <cstdlib>
#include <ctime>

#define ATMEGA328P_SUPPLY 5.0 // V
#define ATMEGA328P_LOW_FREQ_Icc (0.9 * 1e-3) // amp (1 Mhz)
#define ATMEGA328P_FUL_FREQ_Icc (10.0 * 1e-3) // amp ( 16 Mhz )

#define INTERVAL_TO_BE_SCANNED_AGAIN 3
#define INTERVAL_TO_TRY_SLEEP_AGAIN 5

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
	wakeUpTimer = new TagWakeUpTimer(this);
	sleepTimer = new TagSleepTimer(this);
	waitWindow = new TagWaitWindow(this);

	commandsMap["start"] = &RfidTagAgent::start;

	bind("packetSize_", &size_);

}

void RfidTagAgent::consume(double energy) {

	if (em() == NULL) {
		return;
	}

	em()->setenergy(em()->energy() - energy);

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

	if (em()) {
		em()->set_node_sleep(1);
	}

	delete collectResponseTimer;
	collectResponseTimer = new CollectResponseTimer(this);
	collectResponseTimer->schedule(p, hdr, slotChosen, delay);

	delete waitWindow;
	waitWindow = new TagWaitWindow(this);
	waitWindow->schedule(windowSize);

}

void RfidTagAgent::responseCollect(Packet *packet, hdr_rfid *hdr, int slot) {

	hdr_ip* intIpHeader = HDR_IP(packet);

	Packet* pkt = allocpkt();
	hdr_ip* ipHeader = HDR_IP(pkt);
	hdr_rfid *rfidHeader = hdr_rfid::access(pkt);

	if (em()) {
		em()->set_node_sleep(0);
	}

	state = TAG_WAITING_WINDOW;

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

	Packet::free(packet);

}

void RfidTagAgent::sleep(Packet *pkt, hdr_rfid *hdr) {

	hdr_rfid *rfidHeader = hdr_rfid::access(pkt);

	if (rfidHeader->tagID != tagID) {
		return;
	}

	state = TAG_SLEEPING;

	if (em()) {
		em()->set_node_sleep(1);
	}

	fprintf(stderr, "Tag [%u] sleep\n", rfidHeader->tagID);

	delete wakeUpTimer;
	wakeUpTimer = new TagWakeUpTimer(this);
	wakeUpTimer->schedule(INTERVAL_TO_BE_SCANNED_AGAIN);

	Packet::free(pkt);

}

void RfidTagAgent::recv(Packet *pkt, Handler *h) {

	hdr_ip* hdrip = hdr_ip::access(pkt);
	hdr_rfid* hdr = hdr_rfid::access(pkt);

	if (state != TAG_IDLE) {
		Packet::free(pkt);
		return;
	}

	if (hdrip->daddr() == IP_BROADCAST) {

		if (hdr->commandPrefix == 31) {

			switch (hdr->commandCode) {
			case 10:
				collectId(pkt, hdr); break;
			case 15:
				sleep(pkt, hdr); break;
			}
		}

	}

}

int RfidTagAgent::command(int argc, const char*const* argv) {

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

void RfidTagAgent::wakeUp() {

	if (state != TAG_SLEEPING) return;

	state = TAG_IDLE;

	if (em()) {
		em()->set_node_sleep(0);
	}

}

void RfidTagAgent::tryToSleep() {

	if (state == TAG_IDLE) {
		state = TAG_SLEEPING;

		if (em()) {
			em()->set_node_sleep(1);
			delete wakeUpTimer;
			wakeUpTimer = new TagWakeUpTimer(this);
			wakeUpTimer->schedule(INTERVAL_TO_BE_SCANNED_AGAIN);
			//sleepTimer->schedule(INTERVAL_TO_TRY_SLEEP_AGAIN + INTERVAL_TO_BE_SCANNED_AGAIN);
			sleepTimer->resched(INTERVAL_TO_TRY_SLEEP_AGAIN + INTERVAL_TO_BE_SCANNED_AGAIN);
			fprintf(stderr, "Tag %u sleeping\n", tagID);
		}
	}
}


int RfidTagAgent::start(int argc, const char* const* argv) {

	//wakeUp();

	state = TAG_IDLE;
	//sleepTimer->schedule(INTERVAL_TO_TRY_SLEEP_AGAIN);
	sleepTimer->resched(INTERVAL_TO_TRY_SLEEP_AGAIN);

	return (TCL_OK);
}

void RfidTagAgent::waitWindowCall() {
	state = TAG_IDLE;
	em()->set_node_sleep(0);
	sleepTimer->resched(INTERVAL_TO_TRY_SLEEP_AGAIN);

	//Tcl& tcl=Tcl::instance();
	//tcl.evalf("%s delete-mark m2", Node::get_node_by_address(addr())->name());
	//sleepTimer->schedule(5);

	Tcl& tcl=Tcl::instance();
	tcl.evalf("%s label recognized", Node::get_node_by_address(addr())->name());
}
