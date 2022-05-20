#ifndef TRANSPORTRECEIVER
#define TRANSPORTRECEIVER

#include <string.h>
#include <omnetpp.h>
#include "Volt.h"

using namespace omnetpp;

class TransportReceiver : public cSimpleModule {
private:
	cQueue buffer;
	cStdDev bufferSizeStdDev;
	cOutVector bufferSizeVector;
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
	endServiceEvent = NULL;
}

TransportReceiver::~TransportReceiver() {
	cancelAndDelete(endServiceEvent);
}

void TransportReceiver::initialize(){
	bufferSizeVector.setName("bufferSizeVector");
	bufferSizeStdDev.setName("bufferSizeStdDev");

	buffer.setName("buffer");

	endServiceEvent = new cMessage("endService");
}

void TransportReceiver::finish(){
    // Stats record at the end of simulation
	recordScalar("Avg Buffer Size Rec", bufferSizeStdDev.getMean());
}

void TransportReceiver::handleMessage(cMessage * msg) {
	if(msg->isSelfMessage()){
		this->handleSelfMessage(msg);
	} else {
		this->handleVolt((Volt*) msg);
	}
}

void TransportReceiver::handleSelfMessage(cMessage * msg){
	if(msg == endServiceEvent){
		// Podemos enviar un nuevo mensaje
		if(!buffer.isEmpty()){
			Volt * volt = (Volt *) buffer.pop();

			bufferSizeStdDev.collect(buffer.getLength());
			bufferSizeVector.record(buffer.getLength());

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

		bufferSizeStdDev.collect(buffer.getLength());
		bufferSizeVector.record(buffer.getLength());

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
