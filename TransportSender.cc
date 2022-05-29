#ifndef TRANSPORTSENDER
#define TRANSPORTSENDER

#include <string.h>
#include <omnetpp.h>
#include "Volt.h"
#include "RenoManager.h"
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
	RenoManager renoManager;
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
	std::cout << std::boolalpha;
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

	renoManager = RenoManager();
	renoManager.setSize(par("packetByteSize").intValue());

	congestionController = CongestionController();
	currentControlWindowSize = par("packetByteSize").intValue();
}

void TransportSender::finish(){
	// Stats record at the end of simulation
	recordScalar("Avg Buffer Size Send", bufferSizeStdSend.getMean());
}

/* Handler general que OMNET llama para manejar todos los eventos del modulo */
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

/* Maneja un Volt que viene de la propia capa de aplicacion.
 * Este volt se desea enviar al receptor */
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

/* Maneja los mensajes creados en el Sender */
void TransportSender::handleSelfMsg(cMessage * msg) {
	if (msg == endServiceEvent) {
		if(!buffer.isEmpty() || !retransmissionQueue.empty()) {
			// Chequeamos el tamaño del packet sin quitarlo de la cola
			int packetSize;
			
			if (buffer.isEmpty()) {
				int retSeqN = retransmissionQueue.front();
				Volt * auxVolt = congestionController.dupVolt(retSeqN);
				packetSize = auxVolt->getByteLength();
				delete(auxVolt);
			} else {
				packetSize = ((Volt*) buffer.front())->getByteLength();
			}

			bool hasCongestionWinEnoughSpace = renoManager.getAvailableWin() >= packetSize;
			int bytesInFlight = congestionController.amountBytesInFlight();

			std::cout << "Sender :: currentControlWindowSize " << currentControlWindowSize << " bytes\n";
			bool hasControlWinEnoughSpace = currentControlWindowSize - bytesInFlight >= packetSize;
			std::cout << "Sender :: Enough Congestion Window: " << hasCongestionWinEnoughSpace;
			std::cout << " Enough Control Window: " << hasControlWinEnoughSpace << "\n";

			if (hasCongestionWinEnoughSpace && hasControlWinEnoughSpace) {
				// Enviamos el Volt que está al frente de la lista
				handleStartNextTransmission();
			}
		}
	} else if (msg == rttEvent) {
		std::cout << "Sender :: RTT Event\n";

		if (!renoManager.getSlowStart()) {
			// Cada RTT Aumentamos en un paquete la VC
			renoManager.setSize(renoManager.getSize() + par("packetByteSize").longValue());
		}

		scheduleAt(simTime() + rttManager.getCurrentRTT(), rttEvent);
	} else if (msg->getKind() == EVENT_TIMEOUT_KIND) {
		EventTimeout * timeout = (EventTimeout*) msg;
		std::cout << "\nSender :: EVENT_TIMEOUT_KIND. SeqN = " << timeout->getSeqN() << "\n";
		handlePacketLoss(timeout->getSeqN());
	}
}

/* Maneja el envío del próximo Volt
   - Si la queue de retransmisión está vacía, envía el primer Volt del buffer
   - Sino está vacía, retransmite el Volt de esta queue
   - Añade el timeout
 */
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
		congestionController.addVolt(voltCopy);
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
	renoManager.addTimeoutMsg(timeout);
}

/* */
void TransportSender::handleVoltReceived(Volt * volt) {
	if(volt->getAckFlag()){
		handleAck(volt);
	} else {
		std::cout << "Sender :: ERROR :: Received message through subnetwork that is not ACK\n";
	}
}

/*
	Maneja un mensaje de ACK entrante:
	- Cancelamos el timeout de retransmisión
	- Actualizamos la ventana de control
	- Si está en la cola de retransmisión, lo quitamos
	- Si estamos en slow start, aumenta la VC
	- Aumentamos el contador de ACK de la SW (Sliding Window)
	- Si paquete no fue retransmitido antes, actualizamos la estimacion de RTT
	- Si el ACK es el de la base de la SW, la movemos
 */
void TransportSender::handleAck(Volt * volt) {
//	    ackTime.record(volt->getDuration());  // Revisar
	int seqN = volt->getSeqNumber();
	std::cout << "Sender :: handling ACK of Volt " << seqN << "\n";

	std::cout << "Sender :: Current Control WindowSize = " << volt->getWindowSize() << "\n";
	std::cout << "MIRROR :: Check Volt " << volt << "\n";  // FIXME

	/* ----------------------- CANCEL TIMEOUT ----------------------- */
	scheduleServiceIfIdle();
	EventTimeout * timeout = renoManager.popTimeoutMsg(seqN);
	if(timeout != NULL) {
		std::cout << "Sender :: Timeout cancelled due to ACK\n";
		cancelEvent(timeout);
		delete(timeout);
	}

	/* ------------------- UPDATE CONTROL WINDOW --------------------- */
	currentControlWindowSize = volt->getWindowSize();

	/* ----------------- POP RETRANSMISSION QUEUE ------------------ */
	retransmissionQueue.remove(seqN);

	/* ----------------------- SLOW START --------------------------- */
	if(renoManager.getSlowStart()) {
		// Estamos en arranque lento aumentamos la VC a maxSize(Packet)
		renoManager.setSize(renoManager.getSize() + par("packetByteSize").longValue());
	}

	/* ----------------------- +1 ACK COUNTER ---------------------- */
	congestionController.addAck(seqN);


	/* --------------- UPDATE RTT IF NOT RETRANSMITTED --------------*/
	double sendTime = congestionController.getSendTime(seqN);  // ?

	Volt * auxVolt = congestionController.dupVolt(seqN);

	if (!auxVolt->getRetFlag()) {
		// Actualizamos la estimación de RTT
		double newRtt = (simTime().dbl() - sendTime);
		rttManager.updateEstimation(newRtt);
	}

	/* -------------------- MOVE SW IF ACK BASE -------------------*/
	// Mientras el volt de la base del SW tenga al menos una confirmación de recepción...
	while (congestionController.getAck(congestionController.getBaseWindow()) > 0){
		// La base actual del congestionController ya fue confirmada como recibida
		// eliminamos el volt guardado (que hubieramos usado para retrasmitir)
		int currentBaseOfSlidingWindow = congestionController.getBaseWindow();
		Volt * savedPkt = congestionController.popVolt(currentBaseOfSlidingWindow);
		delete(savedPkt);
		
		// Movemos la SW
		congestionController.setBaseWindow((currentBaseOfSlidingWindow + 1) % 1000);  // FIXME 1000
	}

	delete(auxVolt);
	delete(volt);
}

void TransportSender::handlePacketLoss(int seqN) {
	std::cout << "\nSender :: SeqNumber " << seqN << " from handlePacketLoss\n";

	// Cancelar el timeout actual
	EventTimeout * timeout = renoManager.popTimeoutMsg(seqN);

	if(timeout != NULL){
		cancelEvent(timeout);
		delete(timeout);
	}

	// Lo agregamos a la queue de retransmisión
	retransmissionQueue.push_back(seqN);
	rttManager.updateTimeoutRTo();

	scheduleServiceIfIdle();

	// Actualizar la VC
	int newCWSize = renoManager.getSize() / 2;
	renoManager.setSize(newCWSize);
	renoManager.setSlowStart(false);
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
