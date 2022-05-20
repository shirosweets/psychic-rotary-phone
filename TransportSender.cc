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
	void handleVoltToSend(Volt * msg);
	void handleSelfMsg(cMessage * msg);
	void handleVoltReceived(Volt * msg);
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
	buffer.setName("Buffer");
	endServiceEvent = new cMessage("endService");
}

void TransportSender::finish(){
}

void TransportSender::handleMessage(cMessage * msg) {
	if(msg->isSelfMessage()) {
		this->handleSelfMsg(msg);
	} else if (msg->arrivedOn("appLayerIn")) {
		// Llegó desde el generador (Capa de Aplicación)
		// para enviar al receptor
		this->handleVoltToSend((Volt*)msg);
	} else {
		// Llegó de la subnetwork
		this->handleVoltReceived((Volt*)msg);

	}
}

void TransportSender::handleVoltToSend(Volt * msg) {
	if (buffer.getLength() >= par("bufferSize").longValue()) {
		// No hay espacio en el buffer: Dropeamos el paquete
		delete(msg);
		this->bubble("packet-dropped");
	} else {
		// Hay espacio en el buffer, por lo que
		// ponemos el paquete en la cola de envío
		buffer.insert(msg);
		// Si no estamos enviando un mensaje ahora mismo
		if (!endServiceEvent->isScheduled()) {
			// Empezamos el envío
			scheduleAt(simTime() + 0, endServiceEvent);
		}
	}
}

void TransportSender::handleSelfMsg(cMessage * msg) {
	if (msg == endServiceEvent) {
		// Podemos enviar un nuevo mensaje
		if (!buffer.isEmpty()) {
			Volt * volt = (Volt*) buffer.pop();
			send(volt, "subnetwork$o");
			simtime_t serviceTime = volt->getDuration();
			scheduleAt(simTime() + serviceTime, endServiceEvent);
		}
	}
}

void TransportSender::handleVoltReceived(Volt * msg) {
// FIXME
	delete(msg);
}

#endif /* TRANSPORTSENDER */
