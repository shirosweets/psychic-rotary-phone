//
// Generated file, do not edit! Created by nedtool 5.3 from Volt.msg.
//

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#ifndef __CONGESTIONWINDOW_H
#define __CONGESTIONWINDOW_H

#include <omnetpp.h>
#include "EventTimeout.h"

// nedtool version check
#define MSGC_VERSION 0x0503
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of nedtool: 'make clean' should help.
#endif

// Que el emisor sepa distinguir
// [(state, timer), , ]
// TODO: Ernesto
class CongestionWindow {
private:
	int maxSize = 2147483647;
	int size;
	int msgSendingAmount = 0;
	EventTimeout * window[2147483647];  // INT_MAX
public:
	CongestionWindow();
    virtual ~CongestionWindow();

    int getSize();
	int getAvailableWin();
	void setSize(int newSize);
	void addTimeoutMsg(int seqN, EventTimeout * msg);
	void cancelTimeoutMsg(int seqN);
protected:
	//
};

#endif // ifndef __CONGESTIONWINDOW_H

