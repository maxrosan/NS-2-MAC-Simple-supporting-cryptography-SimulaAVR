
#include "rfidTag.h"

#include <cstdlib>
#include <ctime>

#define ATMEGA328P_SUPPLY 5.0 // V
#define ATMEGA328P_LOW_FREQ_Icc (0.9 * 1e-3) // amp ( 1 Mhz )
#define ATMEGA328P_FUL_FREQ_Icc (10.0 * 1e-3) // amp ( 16 Mhz )

#define INTERVAL_TO_BE_SCANNED_AGAIN 2
#define INTERVAL_TO_TRY_SLEEP_AGAIN 2

static FILE *fpLog = NULL;

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
	//logTimer = new TagLogTimer(this);
	checkStateTimer = new TagCheckStateTimer(this);
	windowTimer = new TagWindowTimer(this);

	commandsMap["start"] = &RfidTagAgent::start;

	bind("packetSize_", &size_);

	numberOfTimesRecognized = 0;

	mustSleep = true;
	firstTimeReceivedWakeUp = 0;
	intervalToWaitToSleepAgain = 5;

}

void RfidTagAgent::sleepCommand() {
	if (em() == NULL || em()->sleep()) return;
	em()->set_node_sleep(1);
}

void RfidTagAgent::wakeUpCommand() {

	if (em() == NULL || !em()->sleep()) return;
	em()->set_node_sleep(0);

}


void RfidTagAgent::log() {

	int radioState = 0;

	if (em()) radioState = em()->state();

	fprintf(stderr, "[%f] LOG:: Tag [%u] [%s] [%s]\n",
			Scheduler::instance().clock(),
			tagID,
			namesOfStates[state],
			namesOfStatesOfRadio[radioState]);

}

void RfidTagAgent::check() {

	if (em() == NULL) return;

	if (em()->sleep()) {
		fprintf(stderr, "%f tag %u waking up\n", tagID, Scheduler::instance().clock());
		wakeUpCommand();
	} else {
		fprintf(stderr, "%f tag %u sleeping\n", tagID, Scheduler::instance().clock());
		sleepCommand();
	}

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
	Tcl& tcl=Tcl::instance();

	if (state != TAG_IDLE) {
		fprintf(stderr, "Tag [%u] [%d]\n", hdr->tagID, state);
		Packet::free(p);
		return;
	}

	if ((hdr->commandType & (1 << 1)) &&
			hdr->tagID != tagID) {

		fprintf(stderr, "Not to tag [%u]\n", hdr->tagID);

		Packet::free(p);
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

	delete collectResponseTimer;
	collectResponseTimer = new CollectResponseTimer(this);
	collectResponseTimer->schedule(p, hdr, slotChosen, delay);

	wakeUpCommand();
}


void RfidTagAgent::responseCollect(Packet *packet, hdr_rfid *hdr, int slot) {

	double windowSize = 0;
	double slotSize = 0;
	double delay;

	hdr_ip* intIpHeader = HDR_IP(packet);

	Packet* pkt = allocpkt();
	hdr_ip* ipHeader = HDR_IP(pkt);
	hdr_rfid *rfidHeader = hdr_rfid::access(pkt);

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

	ipHeader->daddr() = IP_BROADCAST;
	ipHeader->dport() = intIpHeader->sport();
	ipHeader->saddr() = here_.addr_; //Source: reader ip
	ipHeader->sport() = here_.port_;

	send(pkt, (Handler*) 0);

	windowSize = ((double) hdr->windowSize)  * 1e-3;
	slotSize = windowSize / ((double) hdr->numberOfSlots);
	delay = slotSize * ((double) slot);

	logEvent("%f response_collect %u %u", Scheduler::instance().clock(), tagID, hdr->interrogatorID);

	Packet::free(packet);

	windowTimer->resched(windowSize - delay);

	//checkStateTimer->stop();
	sleepCommand();

}

void RfidTagAgent::logEvent(const char *message, ...) {
	char buffer[512];
	va_list ap;

	va_start(ap, message);
	vsprintf(buffer, message, ap);
	va_end(ap);

	fwrite(buffer, strlen(buffer), 1, fpLog);
	fwrite("\n", 1, 1, fpLog);
}

void RfidTagAgent::sleep(Packet *pkt, hdr_rfid *hdr) {

	hdr_rfid *rfidHeader = hdr_rfid::access(pkt);
	TracedInt tracedInt (123);
	Tcl& tcl=Tcl::instance();

	if (rfidHeader->tagID != tagID) {
		return;
	}

	numberOfTimesRecognized++;

	tcl.evalf("%s label recognized#%d", Node::get_node_by_address(addr())->name(), numberOfTimesRecognized);

	logEvent("%f sleep %u %u", Scheduler::instance().clock(), tagID, hdr->interrogatorID);

	Packet::free(pkt);

}

void RfidTagAgent::startToWakeUp(Packet *pkt) {
	checkStateTimer->stop();
	//wakeUpCommand();
}

void RfidTagAgent::recv(Packet *pkt, Handler *h) {

	hdr_ip* hdrip = hdr_ip::access(pkt);
	hdr_rfid* hdr = hdr_rfid::access(pkt);

	if (hdrip->daddr() == IP_BROADCAST) {

		if (hdr->commandPrefix == 31 && state == TAG_IDLE) {
			switch (hdr->commandCode) {
			case 10:
				collectId(pkt, hdr); break;
			case 15:
				sleep(pkt, hdr); break;
			}
		} else if (hdr->commandPrefix == 0xFF) {
			startToWakeUp(pkt);
		} else {
			Packet::free(pkt);
		}

	} else {
		Packet::free(pkt);
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


int RfidTagAgent::start(int argc, const char* const* argv) {

	if (em()) {
		em()->start_powersaving();
	}

	state = TAG_IDLE;
	//wakeUpCommand();
	checkStateTimer->start();

	//logTimer->schedule();

	if (fpLog == NULL) {
		fpLog = fopen("rfid.txt", "w");
	}

	return (TCL_OK);
}

void RfidTagAgent::waitWindowCall() {

	state = TAG_IDLE;
	sleepCommand();

	checkStateTimer->start();

	logEvent("%f end_window %u", Scheduler::instance().clock(), tagID);

}
