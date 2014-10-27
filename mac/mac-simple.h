
/*
 * mac-simple.h
 * Copyright (C) 2003 by the University of Southern California
 * $Id: mac-simple.h,v 1.6 2005/08/25 18:58:07 johnh Exp $
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 * The copyright of this module includes the following
 * linking-with-specific-other-licenses addition:
 *
 * In addition, as a special exception, the copyright holders of
 * this module give you permission to combine (via static or
 * dynamic linking) this module with free software programs or
 * libraries that are released under the GNU LGPL and with code
 * included in the standard release of ns-2 under the Apache 2.0
 * license or under otherwise-compatible licenses with advertising
 * requirements (or modified versions of such code, with unchanged
 * license).  You may copy and distribute such a system following the
 * terms of the GNU GPL for this module and the licenses of the
 * other code concerned, provided that you include the source code of
 * that other code when and as the GNU GPL requires distribution of
 * source code.
 *
 * Note that people who make modified versions of this module
 * are not obligated to grant this special exception for their
 * modified versions; it is their choice whether to do so.  The GNU
 * General Public License gives permission to release a modified
 * version without this exception; this exception also makes it
 * possible to release a modified version which carries forward this
 * exception.
 *
 */

#ifndef ns_mac_simple_h
#define ns_mac_simple_h

#include <queue>
#include <algorithm>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <netdb.h>

// Added by Sushmita to support event tracing (singal@nunki.usc.edu)
#include "address.h"
#include "ip.h"

class MacSimpleWaitTimer;
class MacSimpleSendTimer;
class MacSimpleRecvTimer;
class MacSimpleDecryptTimer;
class MacSimpleEncryptTimer;
class SimulAVRClient;

// Added by Sushmita to support event tracing (singal@nunki.usc.edu)
class EventTrace;


class MacSimple : public Mac {
	//Added by Sushmita to support backoff
	friend class BackoffTimer;
public:
	MacSimple();
	void recv(Packet *p, Handler *h);
	void send(Packet *p, Handler *h);

	void waitHandler(void);
	void sendHandler(void);
	void recvHandler(void);
	double txtime(Packet *p);
	void recvUp(void);

	// Added by Sushmita to support event tracing (singal@nunki.usc.edu)
	void trace_event(char *, Packet *);
	int command(int, const char*const*);
	EventTrace *et_;

	EnergyModel* em() { return netif_->node()->energy_model(); }

private:
	Packet *	pktRx_;
	Packet *	pktTx_;
        MacState        rx_state_;      // incoming state (MAC_RECV or MAC_IDLE)
	MacState        tx_state_;      // outgoing state
        int             tx_active_;
	int             fullduplex_mode_;
	Handler * 	txHandler_;
	MacSimpleWaitTimer *waitTimer;
	MacSimpleSendTimer *sendTimer;
	MacSimpleRecvTimer *recvTimer;
	MacSimpleDecryptTimer *decryptTimer;
	MacSimpleEncryptTimer *encryptTimer;	

	int busy_ ;


};

class MacSimpleTimer: public Handler {
public:
	MacSimpleTimer(MacSimple* m) : mac(m) {
	  busy_ = 0;
	}
	virtual void handle(Event *e) = 0;
	virtual void restart(double time);
	virtual void start(double time);
	virtual void stop(void);
	inline int busy(void) { return busy_; }
	inline double expire(void) {
		return ((stime + rtime) - Scheduler::instance().clock());
	}

protected:
	MacSimple	*mac;
	int		busy_;
	Event		intr;
	double		stime;
	double		rtime;
	double		slottime;
};

// Timer to use for delaying the sending of packets
class MacSimpleWaitTimer: public MacSimpleTimer {
public: MacSimpleWaitTimer(MacSimple *m) : MacSimpleTimer(m) {}
	void handle(Event *e);
};

//  Timer to use for finishing sending of packets
class MacSimpleSendTimer: public MacSimpleTimer {
public:
	MacSimpleSendTimer(MacSimple *m) : MacSimpleTimer(m) {}
	void handle(Event *e);
};

class MacSimpleDecryptTimer: public Handler {
private:
	bool busy;
	std::queue<std::pair<Packet*, double> > pkts;
	Packet *mainPkt;
public:

	MacSimpleDecryptTimer(MacSimple *m) {
		this->macSimple = m;
		busy = false;
		mainPkt = NULL;
	}

	virtual void handle(Event *e) {

		Scheduler &sched = Scheduler::instance();
		printf("X = %lf %x\n", sched.clock(), mainPkt);

		if (pkts.size() == 0) {
			busy = false;
			mainPkt = NULL;
		} else {
			mainPkt = pkts.front().first;
			sched.schedule(this, &intr, pkts.front().second);
			pkts.pop();
		}
	}

	void schedule(Packet *pkt, double delay) {

		Scheduler &sched = Scheduler::instance();

		printf("Y = %lf\n", sched.clock());

		if (!busy) {
			busy = true;
			mainPkt = pkt;
			sched.schedule(this, &intr, delay);
		} else {
			pkts.push(std::make_pair(pkt, delay));
		}
	}

	Event intr;
	MacSimple *macSimple;

};

class MacSimpleEncryptTimer: public Handler {
private:
	bool busy;
	std::queue<std::pair<Packet*, double> > pkts;
	Packet *mainPkt;
public:

	MacSimpleEncryptTimer(MacSimple *m) {
		this->macSimple = m;
		busy = false;
		mainPkt = NULL;
	}

	virtual void handle(Event *e) {

		Scheduler &sched = Scheduler::instance();

		printf("Encrypt\n");

		if (pkts.size() == 0) {
			busy = false;
			mainPkt = NULL;
		} else {
			mainPkt = pkts.front().first;
			sched.schedule(this, &intr, pkts.front().second);
			pkts.pop();
		}
	}

	void schedule(Packet *pkt, double delay) {

		Scheduler &sched = Scheduler::instance();

		if (!busy) {
			busy = true;
			mainPkt = pkt;
			sched.schedule(this, &intr, delay);
		} else {
			pkts.push(std::make_pair(pkt, delay));
		}
	}

	Event intr;
	MacSimple *macSimple;

};

// Timer to use for finishing reception of packets
class MacSimpleRecvTimer: public MacSimpleTimer {
public:
	MacSimpleRecvTimer(MacSimple *m) : MacSimpleTimer(m) {}
	void handle(Event *e);
};

char *base64_encode(const unsigned char *data,
                    size_t input_length,
                    size_t *output_length);

unsigned char *base64_decode(const char *data,
                             size_t input_length,
                             size_t *output_length);

void build_decoding_table();

void base64_cleanup();

class SimulAVRClient {
private:
	int sock;
	bool connected;
	char *buffer;
	int bufferSize;
	MacSimple *macSimple;

	void makeSocketLinger(int fd) {
	        struct linger ling;
	        ling.l_onoff=1;
	        ling.l_linger=30;
        	setsockopt(fd, SOL_SOCKET, SO_LINGER, &ling, sizeof(ling));
	}
	
public:

	int port;

	SimulAVRClient(MacSimple *mac) {
		port = 8080;
		connected = false;
	
		bufferSize = 1024;
		buffer = (char*) malloc(bufferSize);

		macSimple = mac;
	}

	void sendPacketToEncrypt(Packet *packet) {

		static char* ENCRYPT_CMD = "encrypt\n";
		char *baseEnc;
		size_t size;
		char *ptr;
		PacketData *pktData;

		if (!connected) return;

		baseEnc = base64_encode(packet->accessdata(), packet->userdata()->size(), &size);

		write(sock, ENCRYPT_CMD, strlen(ENCRYPT_CMD));

		write(sock, baseEnc, size);
		write(sock, "\n", 1);

		size = read(sock, buffer, bufferSize);
		buffer[size] = 0;

		ptr = strtok(buffer, "#");
		fprintf(stderr, "msg = %s\n", ptr);

		baseEnc = (char*) base64_decode(ptr, strlen(ptr), &size);
		pktData = new PacketData(size);
		memcpy(pktData->data(), baseEnc, size);

		packet->setdata(pktData);

		ptr = strtok(NULL, "#");
		fprintf(stderr, "int = %s\n", ptr);
	}

	void sendPacketToDecrypt(Packet *packet) {

		static char* DECRYPT_CMD = "decrypt\n";
		char *baseEnc;
		size_t size;
		char *ptr;
		PacketData *pktData;

		if (!connected) return;

		baseEnc = base64_encode(packet->accessdata(), packet->userdata()->size(), &size);

		write(sock, DECRYPT_CMD, strlen(DECRYPT_CMD));

		write(sock, baseEnc, size);
		write(sock, "\n", 1);

		size = read(sock, buffer, bufferSize);
		buffer[size] = 0;

		ptr = strtok(buffer, "#");
		fprintf(stderr, "msg = %s\n", ptr);

		baseEnc = (char*) base64_decode(ptr, strlen(ptr), &size);
		pktData = new PacketData(size);
		memcpy(pktData->data(), baseEnc, size);

		packet->setdata(pktData);

		ptr = strtok(NULL, "#");
		fprintf(stderr, "int = %s\n", ptr);
	}

	bool connectSock() {

		struct sockaddr_in serv_addr;
		struct hostent *server;

		if (connected) {
			return true;
		}

		sock = socket(AF_INET, SOCK_STREAM, 0);

		if (sock < 0) {
			return false;
		}

		server = gethostbyname("localhost");
		
		if (server == NULL) {
			close(sock);
			fprintf(stderr, "Failed to get host by name\n");
			return false;
		}

		bzero((char*) &serv_addr, sizeof(serv_addr));

		serv_addr.sin_family = AF_INET;
		bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
		serv_addr.sin_port = htons(port);

		makeSocketLinger(sock);

		if (connect(sock, (const sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
			close(sock);
			fprintf(stderr, "Failed to connect to socket\n");
			return false;
		}

		connected = true;

		return true;

	}

};


#endif
