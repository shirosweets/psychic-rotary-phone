//
// Generated file, do not edit! Created by nedtool 5.3 from Volt.msg.
//

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#ifndef __RENOMANAGER_H
#define __RENOMANAGER_H

#include <omnetpp.h>
#include "EventTimeout.h"

#define EVENT_TIMEOUT_KIND 1

// nedtool version check
#define MSGC_VERSION 0x0503
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of nedtool: 'make clean' should help.
#endif


/* RenoController */
class RenoManager {
private:
	int maxSize; // INT_MAX
	int size;
	int msgSendingAmount;
	bool isSlowStartStage = true;

	std::map<int, EventTimeout*> window;
	void logAvailableWin();
public:
	RenoManager();
    virtual ~RenoManager();

    /* Retorna el tamaño máximo de la ventana de congestión */
    int getMaxSize();

    /* Retorna el tamaño actual de la ventana de congestión */
    int getSize();

    /* Dado el tamaño de la ventana y cuanto ya está ocupada
     * devuelve la cantidad de mensajes que se pueden agregar */
	int getAvailableWin();

	/* Setea el tamaño de la ventana de congestión */
	void setSize(int newSize);

	/* Agrega un evento que representa que un mensaje fue enviado y está
	 * en la sub red. */
	void addTimeoutMsg(EventTimeout * msg);

	/* Quita un mensaje de la ventana de congestión y lo devuelve */
	EventTimeout * popTimeoutMsg(int seqN);

	/* Devuelve si el estado actual de Reno es arranque lento */
    bool getSlowStart();
	/* Setea si el estado de Reno es arranque lento */
    void setSlowStart(bool state);
protected:
	//
};

#endif // ifndef __RENOMANAGER_H
