//
// Generated file, do not edit! Created by nedtool 5.3 from Volt.msg.
//

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#ifndef __CONGESTIONCONTROLLER_H
#define __CONGESTIONCONTROLLER_H

#include <omnetpp.h>
#include "Volt.h"

// nedtool version check
#define MSGC_VERSION 0x0503
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of nedtool: 'make clean' should help.
#endif

struct __pairPacketData {
	Volt * volt;
	int ackCounter;
    double sendTime = 0;
};

typedef struct __pairPacketData _pairPacketData;
typedef _pairPacketData * pairPacketData;


class CongestionController {
private:
	int baseWindow = 0;
    int bytesInFlight = 0;
	std::map<int,pairPacketData> slidingWindow;
public:
	CongestionController();
    virtual ~CongestionController();

    int getAck(int seqN);
    void addAck(int seqN);

    void addVolt(Volt * volt);
    Volt * popVolt(int seqN);
    Volt * dupVolt(int seqN);

    bool isVoltInWindow(int seqN);

    double getSendTime(int seqN);
    void addSendTime(int seqN, double time);

    int getBaseWindow();
    void setBaseWindow(int base);

    int amountBytesInFlight();
protected:
};

#endif // ifndef __CONGESTIONCONTROLLER_H

