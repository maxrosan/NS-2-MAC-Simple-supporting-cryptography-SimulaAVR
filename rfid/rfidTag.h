
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
#include "../mobile/energy-model.h"
#include "node.h"

#include "rfidCommon.h"
#include "rfidPacket.h"

class CollectResponseTimer;
class TagWakeUpTimer;
class TagSleepTimer;
class RfidTagAgent;
class TagWaitWindow;
class TagLogTimer;
class TagCheckStateTimer;

enum { TAG_IDLE, TAG_WAITING_SLOT, TAG_SLEEPING, TAG_WAITING_WINDOW };

char* namesOfStates[] = { "TAG_IDLE", "TAG_WAITING_SLOT", "TAG_SLEEPING", "TAG_WAITING_WINDOW" };
char* namesOfStatesOfRadio[] = { "WAITING", "POWERSAVING", "INROUTE"};
typedef int (RfidTagAgent::*funcCall)(int, const char* const*);
#define pointer_map std::map<std::string, funcCall>

class RfidTagAgent : public Agent {
public:
	RfidTagAgent();
	int command(int argc, const char*const* argv);
	virtual void recv(Packet*, Handler*);
	void responseCollect(Packet *packet, hdr_rfid *hdr, int slot);
	void sleep(Packet *packet, hdr_rfid *hdr);
	void wakeUp();
	void startToWakeUp(Packet *pkt);
	void tryToSleep();
	int start(int argc, const char* const* argv);
	void waitWindowCall();
	void log();
	void check();
protected:
	CollectResponseTimer *collectResponseTimer;
	TagWakeUpTimer *wakeUpTimer;
	TagSleepTimer *sleepTimer;
	TagWaitWindow *waitWindow;
	TagLogTimer *logTimer;
	TagCheckStateTimer *checkStateTimer;

	uint16_t tagID;
	pointer_map commandsMap;
	int state;
	int numberOfTimesRecognized;
	bool mustSleep;
	double firstTimeReceivedWakeUp;
	double intervalToWaitToSleepAgain;

	void collectId(Packet *p, hdr_rfid *hdr);
	EnergyModel* em() { return Node::get_node_by_address(addr())->energy_model(); }
	void consume(double energy);
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

class TagWakeUpTimer: public Handler {
private:
	RfidTagAgent *tag;
public:

	Event intr;

	TagWakeUpTimer(RfidTagAgent *tag) : Handler(){
		this->tag = tag;
	}

	virtual void handle(Event *e) {
		tag->wakeUp();
	}

	void schedule(double delay) {
		Scheduler &sched = Scheduler::instance();
		sched.schedule(this, &intr, delay);
	}
};

class TagSleepTimer: public TimerHandler {
private:
	RfidTagAgent *tag;
public:

	TagSleepTimer(RfidTagAgent *tag) : TimerHandler() {
		this->tag = tag;
	}

	virtual void expire(Event *e) {
		tag->tryToSleep();
	}
};

class TagWaitWindow: public Handler {
private:
	RfidTagAgent *tag;
public:

	Event intr;

	TagWaitWindow(RfidTagAgent *tag) {
		this->tag = tag;
	}

	virtual void handle(Event *e) {
		tag->waitWindowCall();
	}

	void schedule(double delay) {
		Scheduler &sched = Scheduler::instance();
		sched.schedule(this, &intr, delay);
	}
};

class TagLogTimer: public TimerHandler {
private:
	RfidTagAgent *tag;

public:

	TagLogTimer(RfidTagAgent *tag) : TimerHandler(){
		this->tag = tag;
	}

	void schedule() {
		resched(10.);
	}

	virtual void expire(Event *e) {
		tag->log();
		resched(10.);
	}


};

class TagCheckStateTimer: public TimerHandler {
private:
	RfidTagAgent *tag;

public:

	TagCheckStateTimer(RfidTagAgent *tag) : TimerHandler(){
		this->tag = tag;
	}

	void schedule() {
		resched(.1);
	}

	virtual void expire(Event *e) {
		tag->check();
		resched(.1);
	}


};

#endif
