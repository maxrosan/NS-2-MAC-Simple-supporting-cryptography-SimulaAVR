/*
 * rfidRead.h
 *
 *  Created on: 11/11/2014
 *      Author: max
 */

#ifndef RFID_RFIDREAD_H_
#define RFID_RFIDREAD_H_

#include <string>
#include <utility>
#include <queue>
#include <map>
#include <list>
#include <algorithm>

#include "agent.h"
#include "tclcl.h"
#include "packet.h"
#include "address.h"
#include "ip.h"
#include "node.h"
#include "../mobile/energy-model.h"

#include "rfidCommon.h"
#include "rfidPacket.h"


class RfidReadAgent;

typedef int (RfidReadAgent::*funcCall)(int, const char* const*);
#define pointer_map std::map<std::string, funcCall>

class WindowTimer;
class RecollectionTimer;
class WakeUpCommand;
class SlotSenderTimer;

enum READER_STATE {
	READER_SENDING_SLEEP,
	READER_SENDING_MEM,
	READER_IDLE,
	READER_WINDOW};

class TagRecognized {

public:

	uint32_t tagID;
	uint16_t slot;
	uint8_t battery;

	TagRecognized() {
		tagID = 0;
		slot = 0;
		battery = 0;
	}

	TagRecognized(const TagRecognized &cp) {
		tagID = cp.tagID;
		slot = cp.slot;
		battery = cp.battery;
	}

};

class RfidReadAgent : public Agent {
public:
	RfidReadAgent();
	int command(int argc, const char*const* argv);
	virtual void recv(Packet*, Handler*);
	void recvTagResponse(Packet *pkt);
	void sendCollection();
	void sendSleep(uint32_t tagID);
	void sendWakeUp();
	void sendReadMemory(uint32_t tagID);
	void endOfWindow();
	void putTagsToSleep();
protected:
	pointer_map commandsMap;
	uint16_t windowSize;
	int numberOfSlots;
	uint16_t interrogatorID;
	WindowTimer *windowTimer;
	int state;
	std::list<TagRecognized> tagsRecognized;
	int currentSlot;
	RecollectionTimer* recollectionTimer;
	WakeUpCommand *wakeUpCommand;
	SlotSenderTimer *slotSenderTimer;

	int start(int argc, const char*const* argv);
};

struct RfidReadRemoveIfCollided {

	uint16_t slot;

	bool operator()(const TagRecognized& tag) {
		return (tag.slot == slot);
	}
};

class WindowTimer: public Handler {
private:
	RfidReadAgent *reader;
public:

	Event intr;

	WindowTimer(RfidReadAgent *reader) {
		this->reader = reader;
	}

	virtual void handle(Event *e) {
		reader->endOfWindow();
	}

	void schedule(double delay) {
		Scheduler &sched = Scheduler::instance();
		sched.schedule(this, &intr, delay);
	}
};

/*class RecollectionTimer: public Handler {
private:
	RfidReadAgent *reader;
public:

	Event intr;

	RecollectionTimer(RfidReadAgent *reader) {
		this->reader = reader;
	}

	virtual void handle(Event *e) {
		reader->sendCollection();
	}

	void schedule(double delay) {
		Scheduler &sched = Scheduler::instance();
		sched.schedule(this, &intr, delay);
	}


};*/

class SlotSenderTimer : public TimerHandler {
private:
	std::queue<Packet*> packets;
	RfidReadAgent *reader;
	double interval;
public:

	SlotSenderTimer(RfidReadAgent *reader) : TimerHandler() {
		this->reader = reader;
		interval = 1.;
	}

	void setInterval(double interval) {
		this->interval = interval;
	}

	void send(Packet *p) {
		packets.push(p);
	}

	virtual void expire(Event *e) {
		if (packets.size() > 0) {
			Packet *p = packets.front();
			reader->send(p, NULL);
			packets.pop();

			fprintf(stderr, "sending slot packet\n");
		}
		resched(interval);
	}

};

class WakeUpCommand : public TimerHandler {
private:
	RfidReadAgent *reader;
	double start;
	double interval;
	double length;
public:

	WakeUpCommand(RfidReadAgent *read, double interval, double length) : TimerHandler() {
		this->reader = read;
		this->interval = interval;
		this->length = length;
	}

	void startSendingWakeUp() {
		Scheduler &sched = Scheduler::instance();
		start = sched.clock();
		reader->sendWakeUp();
		resched(interval);
	}

	void startSendingWakeUpAfter(double after) {
		Scheduler &sched = Scheduler::instance();
		start = sched.clock() + after;
		resched(interval + after);
	}

	virtual void expire(Event *e) {
		Scheduler &sched = Scheduler::instance();

		if ((sched.clock() - start) < length) {
			reader->sendWakeUp();
			resched(interval);
		} else {
			reader->sendCollection();
		}
	}
};

#endif /* RFID_RFIDREAD_H_ */
