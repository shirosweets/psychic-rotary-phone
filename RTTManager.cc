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
}

RTTManager::~RTTManager() {

}

double RTTManager::getCurrentRTo(){
	std::cout << "\nRTTManager :: Current RTO = " << rto << "\n";
	return rto;
}

void RTTManager::updateTimeoutRTo(){
	rto = getCurrentRTo() * 2;
	std::cout << "\nRTTManager :: New RTO = " << rto << "\n";
}

void RTTManager::updateEstimation(double rtMeasurement) {
	double alpha = 0.875;
	double beta = 0.75;
	std::cout << "\nRTTManager :: updateEstimation(" << rtMeasurement << ") \n";

	std::cout << "\nRTTManager :: rtt " << rtt << "\n";
	double absolute = std::abs(rtt - rtMeasurement);
	absolute = std::max(absolute, MIN_ABSOLUTE_DIFFERENCE);
	std::cout << "\nRTTManager :: absolute " << absolute << "\n";

	stdDesviation = beta * stdDesviation + (1-beta) * absolute;
	std::cout << "\nRTTManager :: stdDesviation " << stdDesviation << "\n";

	rtt = alpha * rtt + (1-alpha) * rtMeasurement;
	rto = rtt + (4 * stdDesviation);
}

double RTTManager::getCurrentRTT(){
    return rtt;
}

#endif
