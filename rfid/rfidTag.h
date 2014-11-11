
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

#include "rfidPacket.h"

class CollectResponseTimer;

class RfidTagAgent : public Agent {
public:
	RfidTagAgent();
	int command(int argc, const char*const* argv);
	virtual void recv(Packet*, Handler*);
	void responseCollect(hdr_rfid *hdr);
protected:
	uint16_t tagID;
	CollectResponseTimer *collectResponseTimer;

	void collectId(hdr_rfid *hdr);
};

class CollectResponseTimer: public Handler {
private:
	RfidTagAgent *tag;
	hdr_rfid *hdr;
public:

	Event intr;

	CollectResponseTimer(RfidTagAgent *tag) {
		this->tag = tag;
		hdr = NULL;
	}

	virtual void handle(Event *e) {
		tag->responseCollect(hdr);
	}

	void schedule(hdr_rfid *hdr, double delay) {
		Scheduler &sched = Scheduler::instance();

		this->hdr = hdr;
		sched.schedule(this, &intr, delay);
	}


};


#endif
