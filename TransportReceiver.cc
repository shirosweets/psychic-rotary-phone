#ifndef TRANSPORTRECEIVER
#define TRANSPORTRECEIVER

#include <string.h>
#include <omnetpp.h>
#include "Volt.h"

using namespace omnetpp;

class TransportReceiver : public cSimpleModule {
private:
    //cStdDev delayStats;
    //cOutVector delayVector;
	cQueue buffer;
	cMessage *endServiceEvent;
	void handleSelfMessage(cMessage * msg);
	void handleVolt(Volt * volt);
public:
    TransportReceiver();
    virtual ~TransportReceiver();
protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(TransportReceiver);

TransportReceiver::TransportReceiver() {
}

TransportReceiver::~TransportReceiver() {
}

void TransportReceiver::initialize(){
    // stats and vector names
    // delayStats.setName("TotalDelay");
    // delayVector.setName("Delay");
	buffer.setName("buffer");
	endServiceEvent = new cMessage("endService");
}

void TransportReceiver::finish(){
    // stats record at the end of simulation
    // recordScalar("Avg delay", delayStats.getMean());
    // recordScalar("Number of packets", delayStats.getCount());
}

void TransportReceiver::handleMessage(cMessage * msg) {
	if(msg->isSelfMessage()){
		this->handleSelfMessage(msg);
	} else {
		this->handleVolt((Volt*) msg);
	}

	// TODO
    // compute queuing delay
//    simtime_t delay = simTime() - msg->getCreationTime();
    // update stats
    // delayStats.collect(delay);
    // delayVector.record(delay);
}

void TransportReceiver::handleSelfMessage(cMessage * msg){
	if(msg == endServiceEvent){
		// Podemos enviar un nuevo mensaje
		if(!buffer.isEmpty()){
			Volt * volt = (Volt *) buffer.pop();
			send(volt, "appLayerOut");
			simtime_t serviceTime = volt->getDuration();
			scheduleAt(simTime() + serviceTime, endServiceEvent);
		}
	}
}

void TransportReceiver::handleVolt(Volt * volt){
	if(buffer.getLength() < par("bufferSize").longValue()){
		Volt *ackVolt = new Volt("packet");
		ackVolt->setByteLength(9);
		ackVolt->setAckFlag(true);
		send(ackVolt, "subnetwork$o");

		// Hay espacio en el buffer, por lo que
		// ponemos el paquete en la cola de envío
		buffer.insert(volt);
		// Si no estamos enviando un mensaje ahora mismo
		if (!endServiceEvent->isScheduled()) {
			// Empezamos el envío
			scheduleAt(simTime() + 0, endServiceEvent);
		}
	} else {
		delete(volt);
		this->bubble("volt-dropped");
	}
}

#endif /* TRANSPORTRECEIVER */
