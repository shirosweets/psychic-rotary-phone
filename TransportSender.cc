#ifndef TRANSPORTSENDER
#define TRANSPORTSENDER

#include <string.h>
#include <omnetpp.h>
#include "Volt.h"
#include "RenoManager.h"
#include "SlidingWindow.h"
#include "RTTManager.h"

#define DUPLICATE_ACK_LIMIT 3

using namespace omnetpp;

class TransportSender : public cSimpleModule {
private:
	// Stats
	cStdDev bufferSizeStdSend;
	cStdDev packetRetransmitted;
	cOutVector bufferSizeSend;
	cOutVector packetDropSend;
	cOutVector ackTime;
	cOutVector rtt;

	// Events
	cMessage *rttEvent;
	cMessage *endServiceEvent;

	cQueue buffer;
	std::list<int> retransmissionQueue;  // seqN

	// TADs
	int currentControlWindowSize;
	RTTManager rttManager;
	RenoManager renoManager;
	SlidingWindow slidingWindow;

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
	packetRetransmitted.setName("packetRetransmittedSend");
	packetRetransmitted.collect(0);
	ackTime.setName("ackTime");
	rtt.setName("RTTtime");

	buffer.setName("Buffer");

	rttManager = RTTManager();
	rttEvent = new cMessage("rttEvent");
	scheduleAt(simTime() + rttManager.getCurrentRTT(), rttEvent);
	endServiceEvent = new cMessage("endService");

	renoManager = RenoManager();
	renoManager.setSize(par("packetByteSize").intValue());

	slidingWindow = SlidingWindow();
	currentControlWindowSize = par("packetByteSize").intValue();
}

void TransportSender::finish(){
	// Stats record at the end of simulation
	recordScalar("Avg Buffer Size Send", bufferSizeStdSend.getMean());
	recordScalar("Amount of packets retransmitted", packetRetransmitted.getCount() - 1);
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
				Volt * auxVolt = slidingWindow.dupVolt(retSeqN);
				packetSize = auxVolt->getByteLength();
				delete(auxVolt);
			} else {
				packetSize = ((Volt*) buffer.front())->getByteLength();
			}

			bool hasCongestionWinEnoughSpace = renoManager.getAvailableWin() >= packetSize;
			int bytesInFlight = slidingWindow.amountBytesInFlight();

			std::cout << "Sender :: currentControlWindowSize " << currentControlWindowSize << " bytes\n";
			bool hasControlWinEnoughSpace = currentControlWindowSize - bytesInFlight >= packetSize;
			std::cout << "Sender :: Enough Congestion Window: " << hasCongestionWinEnoughSpace;
			std::cout << "\tEnough Control Window: " << hasControlWinEnoughSpace << "\n";

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

		voltToSend = slidingWindow.dupVolt(retSeqN);
		voltToSend->setRetFlag(true);
		std::cout << "Sender :: Sending Volt from retransmission Queue\n";
		packetRetransmitted.collect(1);
	} else {
		// No hay que retransmitir mensajes
		voltToSend = (Volt*) buffer.pop();

		// Guardamos una copia del voltToSend en caso que haya que retransmitirlo
		Volt * voltCopy = voltToSend->dup();
		slidingWindow.addVolt(voltCopy);
		slidingWindow.addSendTime(voltToSend->getSeqNumber(), simTime().dbl());
	}

	std::cout << "Sender :: SENDING :: Volt " << voltToSend->getSeqNumber() << "\n";

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

	int seqN = volt->getSeqNumber();
	std::cout << "Sender :: handling ACK of Volt " << seqN << "\n";

	/* ----------------------- CANCEL TIMEOUT ----------------------- */
	scheduleServiceIfIdle();
	EventTimeout * timeout = renoManager.popTimeoutMsg(seqN);
	if(timeout != NULL) {
		std::cout << "Sender :: Timeout cancelled due to ACK\n";
		cancelEvent(timeout);
		delete(timeout);
		timeout = NULL;
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
	slidingWindow.addAck(seqN);


	/* --------------- UPDATE RTT IF NOT RETRANSMITTED --------------*/
	double sendTime = slidingWindow.getSendTime(seqN);

	Volt * auxVolt = slidingWindow.dupVolt(seqN);
	if(auxVolt != NULL) {
		simtime_t ack_time = simTime() - auxVolt->getCreationTime();
		ackTime.record(ack_time);
		simtime_t rtt_time = simTime() - slidingWindow.getSendTime(seqN);
		rtt.record(rtt_time);
	}

	if (auxVolt != NULL && !auxVolt->getRetFlag()) {
		// Actualizamos la estimación de RTT
		double newRtt = (simTime().dbl() - sendTime);
		rttManager.updateEstimation(newRtt);
	}

	/* -------------------- MOVE SW IF ACK BASE -------------------*/
	// Mientras el volt de la base del SW tenga al menos una confirmación de recepción...
	while (slidingWindow.getAck(slidingWindow.getBaseWindow()) > 0){
		// La base actual del slidingWindow ya fue confirmada como recibida
		// eliminamos el volt guardado (que hubieramos usado para retrasmitir)
		int currentBaseOfSlidingWindow = slidingWindow.getBaseWindow();
		Volt * savedPkt = slidingWindow.popVolt(currentBaseOfSlidingWindow);
		delete(savedPkt);

		// Movemos la SW
		slidingWindow.setBaseWindow((currentBaseOfSlidingWindow + 1) % 1000);  // FIXME 1000
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
