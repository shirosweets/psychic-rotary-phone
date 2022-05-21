#ifndef CONGESTIONWINDOW
#define CONGESTIONWINDOW

#include <string.h>
#include <omnetpp.h>
#include "CongestionController.h"

using namespace omnetpp;

CongestionController::CongestionController() {
	slidingWindow = new pairPacketData[2147483647]; // FIXME
}

CongestionController::~CongestionController() {
	delete[] slidingWindow;
}

#endif /* CONGESTIONCONTROLLER */
