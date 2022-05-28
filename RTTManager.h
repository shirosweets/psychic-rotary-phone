#ifndef __RTTMANAGER_H
#define __RTTMANAGER_H

#include <omnetpp.h>
#include "Volt.h"

class RTTManager {
private:
	double stdDesviation;
	double rtt;
public:
	RTTManager();
    virtual ~RTTManager();

    /* RTo = Retransmission Timeout */
	double getCurrentRTo();

    /* rtMeasurement = Round Trip Measurement */
    void updateEstimation(double rtMeasurement);

    double getCurrentRTT();
protected:
	//
};

#endif // ifndef __RTTMANAGER_H

