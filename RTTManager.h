#ifndef __RTTMANAGER_H
#define __RTTMANAGER_H

#include <omnetpp.h>
#include "Volt.h"

class RTTManager {
private:
	//
public:
	CongestionController();
    virtual ~CongestionController();

    /* RTo = Retransmission Timeout */
	simtime_t getCurrentRTo();

    /* rtMeasurement = Round Trip Measurement */
    void updateEstimation(simtime_t rtMeasurement);
protected:
	//
};

#endif // ifndef __RTTMANAGER_H

