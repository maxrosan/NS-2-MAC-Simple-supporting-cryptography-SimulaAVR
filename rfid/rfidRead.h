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
#include <map>
#include <list>
#include <algorithm>

#include "agent.h"
#include "tclcl.h"
#include "packet.h"
#include "address.h"
#include "ip.h"

#include "rfidCommon.h"
#include "rfidPacket.h"


class RfidReadAgent;

typedef int (RfidReadAgent::*funcCall)(int, const char* const*);
#define pointer_map std::map<std::string, funcCall>

class WindowTimer;

enum READER_STATE {
	READER_SENDING_SLEEP,
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

	void endOfWindow();

protected:
	pointer_map commandsMap;
	uint16_t windowSize;
	int numberOfSlots;
	uint16_t interrogatorID;
	WindowTimer *windowTimer;
	int state;
	std::list<TagRecognized> tagsRecognized;
	int currentSlot;

	int start(int argc, const char*const* argv);

	void sendCollection();
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

#endif /* RFID_RFIDREAD_H_ */
