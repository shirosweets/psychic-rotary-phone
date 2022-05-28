#ifndef QUEUE
#define QUEUE

#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

class Queue: public cSimpleModule {
private:
    // Stats
    cOutVector bufferSizeQueue;
    cOutVector packetDropQueue;

    cQueue buffer;
    simtime_t serviceTime;

    // Events
    cMessage *endServiceEvent;
public:
    Queue();
    virtual ~Queue();
protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(Queue);

Queue::Queue() {
    endServiceEvent = NULL;
}

Queue::~Queue() {
    cancelAndDelete(endServiceEvent);
}

void Queue::initialize() {
    buffer.setName("buffer");
    bufferSizeQueue.setName("bufferSizeQueue");
    packetDropQueue.setName("packetDropQueue");
    packetDropQueue.record(0);

    endServiceEvent = new cMessage("endService");
}

void Queue::finish() {
}

void Queue::handleMessage(cMessage *msg) {
    if (msg == endServiceEvent) {
        // if packet in buffer, send next one
        if (!buffer.isEmpty()) {
            // Dequeue packet
            cPacket *pkt = (cPacket*) buffer.pop();

            // Send packet
            send(pkt, "out");

            // Start new service
            serviceTime = pkt->getDuration();
            scheduleAt(simTime() + serviceTime, endServiceEvent);
        }
    } else { // if msg is a data packet
        if (buffer.getLength() >= par("bufferSize").longValue()) {
		    // drop the packet
		    delete(msg);
			this->bubble("packet-dropped");
			packetDropQueue.record(1);
		}
		else {
			// Enqueue the packet
			buffer.insert(msg);
			bufferSizeQueue.record(buffer.getLength());
			// if the server is idle
			if (!endServiceEvent->isScheduled()) {
				// start the service
				scheduleAt(simTime() + 0, endServiceEvent);
			}
		}
    }
}

#endif /* QUEUE */
