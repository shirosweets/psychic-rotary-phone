#ifndef __RTTMANAGER_H
#define __RTTMANAGER_H

#include <omnetpp.h>
#include "Volt.h"

class RTTManager {
private:
	double stdDesviation;
	double rtt;
    double rto;
public:
	RTTManager();
    virtual ~RTTManager();

    /* RTo = Retransmission Timeout */
	double getCurrentRTo();

    void updateTimeoutRTo();

    /* rtMeasurement = Round Trip Measurement */
    void updateEstimation(double rtMeasurement);

    /* Retorna el RTT actual */
    double getCurrentRTT();
protected:
	//
};

#endif // ifndef __RTTMANAGER_H

