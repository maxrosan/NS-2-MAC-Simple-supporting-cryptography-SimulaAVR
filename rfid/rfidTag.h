
#ifndef TAG_H
#define TAG_H

#include <string>
#include <utility>
#include <map>

#include "agent.h"
#include "tclcl.h"
#include "packet.h"
#include "address.h"
#include "ip.h"

#include "rfidCommon.h"
#include "rfidPacket.h"

class CollectResponseTimer;

enum { TAG_IDLE, TAG_WAITING_SLOT };

class RfidTagAgent : public Agent {
public:
	RfidTagAgent();
	int command(int argc, const char*const* argv);
	virtual void recv(Packet*, Handler*);
	void responseCollect(Packet *packet, hdr_rfid *hdr, int slot);
protected:
	uint16_t tagID;
	CollectResponseTimer *collectResponseTimer;
	int state;

	void collectId(Packet *p, hdr_rfid *hdr);
};

class CollectResponseTimer: public Handler {
private:
	RfidTagAgent *tag;
	hdr_rfid *hdr;
	Packet *pack;
	int slot;
public:

	Event intr;

	CollectResponseTimer(RfidTagAgent *tag) {
		this->tag = tag;
		hdr = NULL;
	}

	virtual void handle(Event *e) {
		tag->responseCollect(pack, hdr, slot);
	}

	void schedule(Packet *p, hdr_rfid *hdr, int slot, double delay) {
		Scheduler &sched = Scheduler::instance();

		this->hdr = hdr;
		this->pack = p;
		this->slot = slot;
		sched.schedule(this, &intr, delay);
	}


};


#endif
