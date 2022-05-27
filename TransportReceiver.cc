#ifndef TRANSPORTRECEIVER
#define TRANSPORTRECEIVER

#include <string.h>
#include <omnetpp.h>
#include "Volt.h"

using namespace omnetpp;

class TransportReceiver : public cSimpleModule {
private:
    // Stats
	cStdDev bufferSizeStdRec;
	cOutVector bufferSizeRec;
	cOutVector packetDropRec;

	// Events
	cMessage *endServiceEvent;

    cQueue buffer;

	// Methods
	void handleSelfMessage(cMessage * msg);
	void handleVolt(Volt * volt);
	int getCurrentWindowSize();
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
	bufferSizeRec.setName("bufferSizeRec");
	bufferSizeStdRec.setName("bufferSizeStdRec");
	packetDropRec.setName("packetDropRec");
	packetDropRec.record(0);

	buffer.setName("buffer");

	endServiceEvent = new cMessage("endService");
}

void TransportReceiver::finish(){
    // Stats record at the end of simulation
	recordScalar("Avg Buffer Size Rec", bufferSizeStdRec.getMean());
}

void TransportReceiver::handleMessage(cMessage * msg) {
	if(msg->isSelfMessage()) {
		this->handleSelfMessage(msg);
	} else {
		this->handleVolt((Volt*) msg);
	}
}

void TransportReceiver::handleSelfMessage(cMessage * msg){
	if(msg == endServiceEvent) {
		// Podemos enviar un nuevo mensaje
		if(!buffer.isEmpty()){
			Volt * volt = (Volt *) buffer.pop();

			bufferSizeStdRec.collect(buffer.getLength());
			bufferSizeRec.record(buffer.getLength());

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
		ackVolt->setSeqNumber(volt->getSeqNumber());
		ackVolt->setWindowSize(getCurrentWindowSize());
		send(ackVolt, "subnetwork$o");

		// Hay espacio en el buffer, por lo que
		// ponemos el paquete en la cola de envío
		buffer.insert(volt);

		bufferSizeStdRec.collect(buffer.getLength());
		bufferSizeRec.record(buffer.getLength());

		// Si no estamos enviando un mensaje ahora mismo
		if (!endServiceEvent->isScheduled()) {
			// Empezamos el envío
			scheduleAt(simTime() + 0, endServiceEvent);
		}
	} else {
		delete(volt);
		this->bubble("volt-dropped");
		packetDropRec.record(1);
	}
}

int TransportReceiver::getCurrentWindowSize() {
	int remainingBuffer = par("bufferSize").intValue() - buffer.getLength() - 2;
	remainingBuffer = remainingBuffer >= 0 ? remainingBuffer : 0;
	std::cout << "Receiver :: Remaining space in buffer " << remainingBuffer << "\n";
	return par("packetByteSize").intValue() * remainingBuffer;
}

#endif /* TRANSPORTRECEIVER */
