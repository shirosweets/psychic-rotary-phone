//
// Generated file, do not edit! Created by nedtool 5.3 from Volt.msg.
//

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#ifndef __CONGESTIONCONTROLLER_H
#define __CONGESTIONCONTROLLER_H

#include <omnetpp.h>

//#define EVENT_TIMEOUT_KIND 1

// nedtool version check
#define MSGC_VERSION 0x0503
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of nedtool: 'make clean' should help.
#endif

struct _pairPacketData {
	Volt * volt;
	int ackCounter;
}

typedef struct _pairPacketData * pairPacketData;


class CongestionController {
private:
	int baseWindow = 0;
	int threshold = 2147483647;  // INT_MAX
	pairPacketData * slidingWindow;
public:
	CongestionController();
    virtual ~CongestionController();

    int getAck(int seqN);
    void addAck(int seqN);

    int getThreshold();
    void setThreshold(int newValue);

    void addVolt(Volt * volt);
    Volt * popVolt(int seqN);

    int getBaseWindow();
    void setBaseWindow(int base);
protected:
};

#endif // ifndef __CONGESTIONCONTROLLER_H

