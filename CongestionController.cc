#ifndef CONGESTIONWINDOW
#define CONGESTIONWINDOW

#include <string.h>
#include <omnetpp.h>
#include "CongestionController.h"

using namespace omnetpp;

CongestionController::CongestionController() {
	slidingWindow = new pairPacketData[134217727]; // FIXME 134MB
}

CongestionController::~CongestionController() {
	delete[] slidingWindow;
}

bool CongestionController::getSlowStart(){
	return true;
}

#endif /* CONGESTIONCONTROLLER */
