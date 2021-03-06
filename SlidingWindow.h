//
// Generated file, do not edit! Created by nedtool 5.3 from Volt.msg.
//

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#ifndef __SLIDING_WINDOW_H
#define __SLIDING_WINDOW_H

#include <omnetpp.h>
#include "Volt.h"

// nedtool version check
#define MSGC_VERSION 0x0503
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of nedtool: 'make clean' should help.
#endif

struct __packetMetadata {
	Volt * volt;
	int ackCounter;
    double sendTime = 0;
    bool retransmitted = false;
};

typedef struct __packetMetadata _packetMetadata;
typedef _packetMetadata * packetMetadata;

/* Sliding Window */
class SlidingWindow {
private:
	int baseWindow = 0;
    int bytesInFlight = 0;
	std::map<int,packetMetadata> slidingWindow;
public:
	SlidingWindow();
    virtual ~SlidingWindow();

    /* Retorna la cantidad de ACKs que tiene ese Volt */
    int getAck(int seqN);
    /* Si hay un volt con ese seqN, incrementa en uno el contador de ACKs */
    void addAck(int seqN);

    /* Agregar el volt a la ventana */
    void addVolt(Volt * volt);
    /* Retorna el volt de la ventana eliminando su espacio reservado
     * Retorna NULL si no se encuentra un volt con ese seqN */
    Volt * popVolt(int seqN);
    /* Retorna una copia del Volt. El usuario debe encargarse de borrarlo
     * Retorna NULL si no se encuentra un volt con ese seqN */
    Volt * dupVolt(int seqN);

    /* Retorna si el volt se encuenta en la SW */
    bool isVoltInWindow(int seqN);

    /* Retorna el tiempo de envío del Volt */
    double getSendTime(int seqN);
    /* Agrega el tiempo de envío del Volt */
    void addSendTime(int seqN, double time);

    /* Retorna el seqN de la base de la SW */
    int getBaseWindow();
    /* Setea el seqN de la base de la SW */
    void setBaseWindow(int base);

    /* Retorna la cantidad de bytes que se encuentran enviado */
    int amountBytesInFlight();
protected:
};

#endif // ifndef __SLIDING_WINDOW_H

