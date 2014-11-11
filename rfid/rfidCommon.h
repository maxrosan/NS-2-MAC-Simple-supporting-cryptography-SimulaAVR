/*
 * rfidCommon.h
 *
 *  Created on: 11/11/2014
 *      Author: max
 */

#ifndef RFID_RFIDCOMMON_H_
#define RFID_RFIDCOMMON_H_

class RfidReadAgent;

class WindowTimer: public Handler {
private:
	RfidReadAgent *reader;
public:

	Event intr;

	WindowTimer(RfidReadAgent *reader) {
		this->reader = reader;
	}

	virtual void handle(Event *e) {
		printf("end of window");
	}

	void schedule(double delay) {
		Scheduler &sched = Scheduler::instance();
		sched.schedule(this, &intr, delay);
	}


};


#endif /* RFID_RFIDCOMMON_H_ */
