#ifndef RTTMANAGER
#define RTTMANAGER

#include <string.h>
#include <omnetpp.h>
#include "RTTManager.h"

using namespace omnetpp;

#define MIN_ABSOLUTE_DIFFERENCE (double)0.0001

RTTManager::RTTManager() {
	rtt = 1; // RFC 6298
	stdDesviation = 0;
	rto = 1;  // 1s
	isFirstAckReceived = false;
}

RTTManager::~RTTManager() {

}

double RTTManager::getCurrentRTo(){
	std::cout << "RTT :: Current RTO = " << rto << "\n";
	return rto;
}

void RTTManager::updateTimeoutRTo(){
	rto = getCurrentRTo() * 2;
	std::cout << "RTT TIMEOUT RTO = " << rto << "\n";
}

void RTTManager::updateSmoothRTT(double rtMeasurement) {
	double alpha = 0.875;
	double beta = 0.75;
	double absolute = std::abs(rtt - rtMeasurement);

	absolute = std::max(absolute, MIN_ABSOLUTE_DIFFERENCE);
	stdDesviation = beta * stdDesviation + (1-beta) * absolute;
	rtt = alpha * rtt + (1-alpha) * rtMeasurement;
	rto = rtt + (4 * stdDesviation);

	std::cout << "RTT :: updateEstimation(" << rtMeasurement << ")";
	std::cout << " : rtt: " << rtt << " s. stdDesviation: " << stdDesviation;
	std::cout << " s. absolute diff: " << absolute << ". rto: " << rto << " s\n";
}

void RTTManager::updateEstimation(double rtMeasurement) {
	if (isFirstAckReceived) {
		updateSmoothRTT(rtMeasurement);
	} else {
		rtt = rtMeasurement;
		stdDesviation = rtt / 2.0;
		rto = rtt * 3.0;
		isFirstAckReceived = true;
	}
}

double RTTManager::getCurrentRTT(){
    return rtt;
}

#endif
