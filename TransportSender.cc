#ifndef TRANSPORTSENDER
#define TRANSPORTSENDER

#include <string.h>
#include <omnetpp.h>
#include "Volt.h"
#include "CongestionWindow.h"
#include "CongestionController.h"

#define DUPLICATE_ACK_LIMIT 3

using namespace omnetpp;

class TransportSender : public cSimpleModule {
private:
	// Stats
	cStdDev bufferSizeStdDev;
	cOutVector bufferSizeVector;
	//cOutVector ackTime;

	// Events
	cMessage *rttEvent;
	cMessage *endServiceEvent;

	simtime_t rtt;
	cQueue buffer;

	// TADs
	CongestionWindow congestionWindow;
	CongestionController congestionController;

	// Methods
	void handleVoltToSend(Volt * msg);
	void handleSelfMsg(cMessage * msg);
	void handleVoltReceived(Volt * msg);
	void handlePacketLoss(int seqN);
	void handleStartNextTransmission();
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
	rttEvent = NULL;
	endServiceEvent = NULL;
}

TransportSender::~TransportSender() {
	cancelAndDelete(rttEvent);
	cancelAndDelete(endServiceEvent);
}

void TransportSender::initialize(){
	bufferSizeVector.setName("bufferSizeVector");
	bufferSizeStdDev.setName("bufferSizeStdDev");

	buffer.setName("Buffer");
//	ackTime.setName("AckTime");

	rttEvent = new cMessage("rttEvent");
	scheduleAt(simTime() + par("rtt"), rttEvent);
	endServiceEvent = new cMessage("endService");

	congestionWindow = CongestionWindow();
	congestionWindow.setSize(par("packetByteSize").intValue());

	congestionController = CongestionController();
}

void TransportSender::finish(){
	// Stats record at the end of simulation
	recordScalar("Avg Buffer Size Send", bufferSizeStdDev.getMean());
}

void TransportSender::handleMessage(cMessage * msg) {
	std::cout << "Sender :: [" << msg->getFullName() << "] :: " << msg->str() << "\n";
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
		if(!buffer.isEmpty()) {
			// Chequeamos el tamaño del packet
			// sin quitarlo de la cola
			Volt * volt = (Volt*) buffer.front();

			int packetSize = volt->getByteLength();
			bool hasCWEnoughSpace = congestionWindow.getAvailableWin() >= packetSize;
			if (hasCWEnoughSpace) {
				// Enviamos el Volt que está al frente de la lista
				handleStartNextTransmission();
			}
		}
	} else if (msg == rttEvent) {
		std::cout << "Sender :: RTT Event\n";

		if (!congestionWindow.getSlowStart()) {
			// Cada RTT Aumentamos en un paquete la VC
			congestionWindow.setSize(congestionWindow.getSize() + par("packetByteSize").longValue());
		}

		scheduleAt(simTime() + par("rtt"), rttEvent);
	} else if (msg->getKind() == EVENT_TIMEOUT_KIND) {
		EventTimeout * timeout = (EventTimeout*) msg;
		handlePacketLoss(timeout->seqN);
	}
}

void TransportSender::handleStartNextTransmission() {
	Volt * volt = (Volt*) buffer.pop();
	std::cout << "Sender :: Current buffer has " << buffer.getLength() << " elements\n";
	int packetSize = volt->getByteLength();

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
	scheduleAt(simTime() + par("timeout"), timeout);
	congestionWindow.addTimeoutMsg(timeout);

	// Guardamos una copia del Volt en caso que haya que retransmitirlo
	Volt * voltCopy = volt->dup();
	congestionController.addVolt(voltCopy);
}

void TransportSender::handleVoltReceived(Volt * volt) {
	if(volt->getAckFlag()){
		//ackTime.record();
		int seqN = volt->getSeqNumber();
		std::cout << "Sender :: handling ACK of Volt " << seqN << "\n";
		EventTimeout * timeout = congestionWindow.popTimeoutMsg(seqN);
		if(timeout != NULL && !timeout->isScheduled()) {
			std::cout << "Sender :: Timeout cancelled due to ACK\n";
			cancelEvent(timeout);
		}

		if(congestionWindow.getSlowStart()) {
			// Estamos en arranque lento aumentamos la VC a maxSize(Packet)
			congestionWindow.setSize(congestionWindow.getSize() + par("packetByteSize").longValue());
		}

		congestionController.addAck(seqN);
		int currentBaseOfSlidingWindow = congestionController.getBaseWindow();
		if (currentBaseOfSlidingWindow == seqN) {
			// El ACK que llegó es el que esperábamos
			// por lo que avanzamos la ventana corrediza
			// y eliminamos el volt guardado (que hubieramos usado para retrasmitir)
			Volt * savedPkt = congestionController.popVolt(seqN);
			delete(savedPkt);
			congestionController.setBaseWindow((currentBaseOfSlidingWindow + 1) % 1000);
			std::cout << "Sender :: Removed Volt " << seqN << " from sliding window\n";
		} else if (congestionController.getAck(seqN) >= DUPLICATE_ACK_LIMIT) {
			handlePacketLoss(seqN);
		}
		delete(volt);
	}
}

void TransportSender::handlePacketLoss(int seqN) {
	std::cout << "Sender :: SeqNumber" << seqN << " from handlePacketLoss\n";

	// Cancelar el timeout actual
	EventTimeout * timeout = CongestionWindow.popTimeoutMsg(seqN);

	if(timeout != NULL){
		cancelEvent(timeout);
	}

	// Obtener la copia del Volt y eliminar de la queue de copias
	Volt * volt = CongestionController.popVolt(seqN);

	if(volt == NULL) {
		std::cout << "Sender :: ERROR :: function handlePacketLoss can't find copy of volt \n";
	}

	// Insertar el Volt al inicio de la queue de envíos
	buffer.insertBefore(buffer.front(), volt);

	// Si no estamos enviando un mensaje ahora mismo
	if (!endServiceEvent->isScheduled()) {
		// Empezamos el envío
		scheduleAt(simTime() + 0, endServiceEvent);
	}

	// Actualizar la VC
	int newCWSize = congestionWindow.getSize() / 2;
	congestionWindow.setSize(newCWSize);
	congestionWindow.setSlowStart(false);
}

#endif /* TRANSPORTSENDER */
