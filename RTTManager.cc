#ifndef RTTMANAGER
#define RTTMANAGER

#include <string.h>
#include <omnetpp.h>
#include <cmath>
#include "RTTManager.h"

using namespace omnetpp;

RTTManager::RTTManager() {
	rtt = 1; //RFC 6298
	stdDesviation = 0;
}

RTTManager::~RTTManager() {

}

double RTTManager::getCurrentRTo(){
	return rtt + (4 * stdDesviation);
}

void RTTManager::updateEstimation(double rtMeasurement){
	double alpha = 0.875;
	double beta = 0.75;
	std::cout << "RTTManager :: WARNING :: updateEstimation( " << rtMeasurement << ") ";
	stdDesviation = beta * stdDesviation + (1-beta) * abs(rtt - rtMeasurement);
	rtt = alpha * rtt + (1-alpha) * rtMeasurement;
}

#endif
