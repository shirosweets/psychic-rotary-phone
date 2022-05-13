#ifndef TRANSPORTSENDER
#define TRANSPORTSENDER

#include <string.h>
#include <omnetpp.h>
#include "Volt.h"

using namespace omnetpp;

class TransportSender : public cSimpleModule {
private:
    //cStdDev delayStats;
    //cOutVector delayVector;
	cQueue buffer;
	cMessage *endServiceEvent;
	void handleVolt(Volt * msg);
public:
    TransportSender();
    virtual ~TransportSender();
protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(TransportSender);

TransportSender::TransportSender() {
	endServiceEvent = NULL;
}

TransportSender::~TransportSender() {
}

void TransportSender::initialize(){
    // stats and vector names
    // delayStats.setName("TotalDelay");
    // delayVector.setName("Delay");

	buffer.setName("Buffer");
	endServiceEvent = new cMessage("endService");
}

void TransportSender::finish(){
    // stats record at the end of simulation
    // recordScalar("Avg delay", delayStats.getMean());
    // recordScalar("Number of packets", delayStats.getCount());
}

void TransportSender::handleMessage(cMessage * msg) {
    // Compute queuing delay
    simtime_t delay = simTime() - msg->getCreationTime();

    //
    handleVolt((Volt *)msg);

    // update stats
    // delayStats.collect(delay);
    // delayVector.record(delay);
    // delete msg
    // delete(msg);

    send(msg, "subnetwork$o");
}

void TransportSender::handleVolt(Volt * msg) {
	if(msg->isSelfMessage()){
		// Evento propio
		if (!buffer.isEmpty()) {
			// dequeue packet
			Volt *pkt = (Volt*) buffer.pop();
			// send packet
			send(pkt, "subnetwork$o");
			// start new service
			simtime_t serviceTime = pkt->getDuration();
			scheduleAt(simTime() + serviceTime, endServiceEvent);
		}
	} else if (msg->arrivedOn("appLayerIn")){
		// Mensaje del generador
		if (buffer.getLength() >= par("bufferSize").longValue()) {
			// Drop the packet
			delete(msg);
			this->bubble("packet-dropped");
			// packetDropVector.record(1);
		}
		else {
			// Enqueue the packet
			buffer.insert(msg);
			// bufferSizeVector.record(buffer.getLength());
			// if the server is idle
			if (!endServiceEvent->isScheduled()) {
				// start the service
				scheduleAt(simTime() + 0, endServiceEvent);
			}
		}
	} else {
		// Mensaje de la subnetwork
		Volt *volt = (Volt*) msg;

		if(volt->getAckFlag()){
			// Mensaje de ACK
		}
	}
}

#endif /* TRANSPORTSENDER */
