#ifndef TRANSPORTSENDER
#define TRANSPORTSENDER

#include <string.h>
#include <omnetpp.h>
#include "Volt.h"
#include "CongestionWindow.h"

using namespace omnetpp;

class TransportSender : public cSimpleModule {
private:
	cQueue buffer;
	cStdDev bufferSizeStdDev;
	cOutVector bufferSizeVector;
	cMessage *endServiceEvent;
	CongestionWindow congestionWindow;
	CongestionController congestionController;
	//cOutVector ackTime;
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
	cancelAndDelete(endServiceEvent);
}

void TransportSender::initialize(){
	bufferSizeVector.setName("bufferSizeVector");
	bufferSizeStdDev.setName("bufferSizeStdDev");

	buffer.setName("Buffer");
//	ackTime.setName("AckTime");

	endServiceEvent = new cMessage("endService");
	congestionWindow = CongestionWindow();
	congestionWindow.setSize(par("packetByteSize"));

	congestionController = CongestionController();
}

void TransportSender::finish(){
	// Stats record at the end of simulation
	recordScalar("Avg Buffer Size Send", bufferSizeStdDev.getMean());
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

		bufferSizeStdDev.collect(buffer.getLength());
		bufferSizeVector.record(buffer.getLength());

		// Si no estamos enviando un mensaje ahora mismo
		if (!endServiceEvent->isScheduled()) {
			// Empezamos el envío
			scheduleAt(simTime() + 0, endServiceEvent);
		}
	}
}

void TransportSender::handleSelfMsg(cMessage * msg) {
	if (msg == endServiceEvent) {
		// Chequeamos el tamaño del packet
		// sin quitarlo de la cola
		Volt * volt = (Volt*) buffer.front();

		// Revisamos la VC
		int packetSize = volt->getByteLength();
		bool hasCWEnoughSpace = congestionWindow.getAvailableWin() >= packetSize;

		// Chequeamos si podemos enviar un nuevo mensaje
		if (!buffer.isEmpty() && hasCWEnoughSpace) {
			volt = (Volt*) buffer.pop();

			bufferSizeStdDev.collect(buffer.getLength());
			bufferSizeVector.record(buffer.getLength());

			send(volt, "subnetwork$o");
			simtime_t serviceTime = volt->getDuration();
			scheduleAt(simTime() + serviceTime, endServiceEvent);

			// Añadimos un nuevo evento timeout para
			// el volt que se está enviando
			EventTimeout * timeout = new EventTimeout("timeout", EVENT_TIMEOUT_KIND);
			timeout->setSeqN(volt->getSeqNumber());
			timeout->setPacketSize(packetSize);
			congestionWindow.addTimeoutMsg(timeout->getSeqN(), timeout);
		}
	}
}

void TransportSender::handleVoltReceived(Volt * volt) {
	if(volt->getAckFlag()){
		//ackTime.record();

		delete(volt);

		EventTimeout * timeout = congestionWindow.popTimeoutMsg(volt->getSeqNumber());
		cancelEvent(timeout);
	}
}

#endif /* TRANSPORTSENDER */
