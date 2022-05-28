#ifndef TRANSPORTSENDER
#define TRANSPORTSENDER

#include <string.h>
#include <omnetpp.h>
#include "Volt.h"
#include "CongestionWindow.h"
#include "CongestionController.h"
#include "RTTManager.h"

#define DUPLICATE_ACK_LIMIT 3

using namespace omnetpp;

class TransportSender : public cSimpleModule {
private:
	// Stats
	cStdDev bufferSizeStdSend;
	cOutVector bufferSizeSend;
	cOutVector packetDropSend;
	cOutVector ackTime;

	// Events
	cMessage *rttEvent;
	cMessage *endServiceEvent;

	cQueue buffer;
	std::list<int> retransmissionQueue;  // seqN

	// TADs
	int currentControlWindowSize;
	RTTManager rttManager;
	CongestionWindow congestionWindow;
	CongestionController congestionController;

	// Methods
	void handleVoltToSend(Volt * msg);
	void handleSelfMsg(cMessage * msg);
	void handleVoltReceived(Volt * msg);
	void handleAck(Volt * msg);
	void handlePacketLoss(int seqN);
	void handleStartNextTransmission();
	void scheduleServiceIfIdle();
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
	bufferSizeSend.setName("bufferSizeSend");
	bufferSizeStdSend.setName("bufferSizeStdSend");
	packetDropSend.setName("packetDropSend");
	packetDropSend.record(0);
	ackTime.setName("ackTime");

	buffer.setName("Buffer");

	rttManager = RTTManager();
	rttEvent = new cMessage("rttEvent");
	scheduleAt(simTime() + rttManager.getCurrentRTT(), rttEvent);
	endServiceEvent = new cMessage("endService");

	congestionWindow = CongestionWindow();
	congestionWindow.setSize(par("packetByteSize").intValue());

	congestionController = CongestionController();
	currentControlWindowSize = par("packetByteSize").intValue();
}

void TransportSender::finish(){
	// Stats record at the end of simulation
	recordScalar("Avg Buffer Size Send", bufferSizeStdSend.getMean());
}

void TransportSender::handleMessage(cMessage * msg) {
//	std::cout << "Sender :: [" << msg->getFullName() << "] :: " << msg->str() << "\n";
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
		packetDropSend.record(1);
	} else {
		// Hay espacio en el buffer, por lo que
		// ponemos el paquete en la cola de envío
		buffer.insert(msg);

		bufferSizeStdSend.collect(buffer.getLength());
		bufferSizeSend.record(buffer.getLength());

		scheduleServiceIfIdle();
	}
}

/* */
void TransportSender::handleSelfMsg(cMessage * msg) {
	if (msg == endServiceEvent) {
		if(!buffer.isEmpty() || !retransmissionQueue.empty()) {
			// Chequeamos el tamaño del packet
			// sin quitarlo de la cola
			Volt * volt = (Volt*) buffer.front();

			int packetSize = volt->getByteLength();
			bool hasCongestionWinEnoughSpace = congestionWindow.getAvailableWin() >= packetSize;
			int bytesInFlight = congestionController.amountBytesInFlight();
			std::cout << "Sender ::  currentControlWindowSize " << currentControlWindowSize << "\n";
			bool hasControlWinEnoughSpace = currentControlWindowSize - bytesInFlight >= packetSize;
			std::cout << "Sender ::  Enough Congestion Window: " << hasCongestionWinEnoughSpace;
			std::cout << " Enough Control Window: " << hasControlWinEnoughSpace << "\n";
			if (hasCongestionWinEnoughSpace && hasControlWinEnoughSpace) {
				// Enviamos el Volt que está al frente de la lista
				handleStartNextTransmission();
			}
		}
	} else if (msg == rttEvent) {
		std::cout << "\nSender :: RTT Event\n";  // FIXME

		if (!congestionWindow.getSlowStart()) {
			// Cada RTT Aumentamos en un paquete la VC
			congestionWindow.setSize(congestionWindow.getSize() + par("packetByteSize").longValue());
		}

		scheduleAt(simTime() + rttManager.getCurrentRTT(), rttEvent);
	} else if (msg->getKind() == EVENT_TIMEOUT_KIND) {
		std::cout << "\nSender :: EVENT_TIMEOUT_KIND\n";
		EventTimeout * timeout = (EventTimeout*) msg;
		std::cout << "Sender :: Timeout has passed. SeqN = " << timeout->getSeqN() << "\n";
		handlePacketLoss(timeout->getSeqN());
	}
}

/* */
void TransportSender::handleStartNextTransmission() {
	Volt * voltToSend = NULL;

	if(!retransmissionQueue.empty()) {
		// Retransmitimos el primer mensaje de la queue
		int retSeqN = retransmissionQueue.front();
		retransmissionQueue.pop_front();

		voltToSend = congestionController.dupVolt(retSeqN);
	} else {
		// No hay que retransmitir mensajes
		voltToSend = (Volt*) buffer.pop();

		// Guardamos una copia del voltToSend en caso que haya que retransmitirlo
		Volt * voltCopy = voltToSend->dup();
		congestionController.addVolt(voltCopy);  // FIXME
		congestionController.addSendTime(voltToSend->getSeqNumber(), simTime().dbl());
	}

	std::cout << "Sender :: Current buffer has " << buffer.getLength() << " elements\n";

	bufferSizeStdSend.collect(buffer.getLength());
	bufferSizeSend.record(buffer.getLength());

	send(voltToSend, "subnetwork$o");
	simtime_t serviceTime = voltToSend->getDuration();
	scheduleAt(simTime() + serviceTime, endServiceEvent);

	// Añadimos un nuevo evento timeout para el voltToSend que se está enviando
	EventTimeout * timeout = new EventTimeout("timeout", EVENT_TIMEOUT_KIND);
	timeout->setSeqN(voltToSend->getSeqNumber());
	timeout->setPacketSize(voltToSend->getByteLength());
	scheduleAt(simTime() + rttManager.getCurrentRTo(), timeout);
	congestionWindow.addTimeoutMsg(timeout);
}

/* */
void TransportSender::handleVoltReceived(Volt * volt) {
	if(volt->getAckFlag()){
		handleAck(volt);
	} else {
		std::cout << "Sender :: ERROR :: Received message through subnetwork that is not ACK\n";
	}
}

/* */
void TransportSender::handleAck(Volt * volt) {
//	    ackTime.record(volt->getDuration());  // Revisar
	int seqN = volt->getSeqNumber();
	std::cout << "Sender :: handling ACK of Volt " << seqN << "\n";

	currentControlWindowSize = volt->getWindowSize();
	std::cout << "MIRROR :: Check Volt " << volt << "\n";  // FIXME

	scheduleServiceIfIdle();
	std::cout << "Sender :: Current Control WindowSize = " << currentControlWindowSize << "\n";
	EventTimeout * timeout = congestionWindow.popTimeoutMsg(seqN);
	if(timeout != NULL) {
		std::cout << "Sender :: Timeout cancelled due to ACK\n";
		cancelEvent(timeout);
		delete(timeout);
	}

	if(congestionWindow.getSlowStart()) {
		// Estamos en arranque lento aumentamos la VC a maxSize(Packet)
		congestionWindow.setSize(congestionWindow.getSize() + par("packetByteSize").longValue());
	}

	congestionController.addAck(seqN);

	int currentBaseOfSlidingWindow = congestionController.getBaseWindow();
	bool voltInSlidingWindow = congestionController.isVoltInWindow(seqN);

	if (currentBaseOfSlidingWindow == seqN && voltInSlidingWindow) {
		// El ACK que llegó es el que esperábamos
		// eliminamos el volt guardado (que hubieramos usado para retrasmitir)
		double sendTime = congestionController.getSendTime(seqN);
		Volt * savedPkt = congestionController.popVolt(seqN); // *
		std::cout << "\nSender :: Corrupted Volt " << savedPkt << "\n";  // FIXME
		std::cout << "Sender :: Removed Volt " << seqN << " from sliding window\n";
		delete(savedPkt);

		// por lo que avanzamos la ventana corrediza
		congestionController.setBaseWindow((currentBaseOfSlidingWindow + 1) % 1000);

		if (!volt->getRetFlag()) {
			// Actualizamos la estimación de RTT
			double newRtt = (simTime().dbl() - sendTime);
			rttManager.updateEstimation(newRtt);
		}
	} else if (congestionController.getAck(seqN) >= DUPLICATE_ACK_LIMIT) {
		std::cout << "\nSender :: DUPLICATE_ACK_LIMIT\n";
		handlePacketLoss(seqN);
	} else if(!voltInSlidingWindow) {
		std::cout << "\nSender :: WARNING :: BORDER CASE\n";
	} else {
		std::cout << "\nSender :: WARNING :: Ack out of order\n";
	}
	delete(volt);
}

void TransportSender::handlePacketLoss(int seqN) {
	std::cout << "\nSender :: SeqNumber " << seqN << " from handlePacketLoss\n";

	// Cancelar el timeout actual
	EventTimeout * timeout = congestionWindow.popTimeoutMsg(seqN);

	if(timeout != NULL){
		cancelEvent(timeout);
		delete(timeout);
	}

	//! Error del timeout
	//! Obtener la copia del Volt y eliminar de la queue de copias
	//! Volt * volt = congestionController.popVolt(seqN); // !
	// std::cout << "Sender :: Check popVolt " << volt << std::endl;
	// if(volt == NULL) {
	// 	std::cout << "Sender :: ERROR :: function handlePacketLoss can't find copy of volt\n";
	// }
	// // Insertar el Volt al inicio de la queue de envíos
	// std::cout << "Buffer size " << buffer.getLength() << "\t front: " << buffer.front() << std::endl;
	// Volt * firstVolt = (Volt*)buffer.front();
	// std::cout << "Sender :: Check front Volt " << firstVolt << std::endl;
	// if(firstVolt != NULL) {
	// 	buffer.insertBefore(firstVolt, volt);
	// } else {
	// 	buffer.insert(volt);
	// }
	//! -----------------------------------------------

	// Lo agregamos a la queue de retransmisión
	retransmissionQueue.push_back(seqN);

	scheduleServiceIfIdle();

	// Actualizar la VC
	int newCWSize = congestionWindow.getSize() / 2;
	congestionWindow.setSize(newCWSize);
	congestionWindow.setSlowStart(false);
}

/* */
void TransportSender::scheduleServiceIfIdle() {
	// Si no estamos enviando un mensaje ahora mismo
	if (!endServiceEvent->isScheduled()) {
		// Scheduleamos el envío
		scheduleAt(simTime() + 0, endServiceEvent);
	}
}

#endif /* TRANSPORTSENDER */
