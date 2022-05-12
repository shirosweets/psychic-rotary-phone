#ifndef CONGESTIONWINDOW
#define CONGESTIONWINDOW

#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

CongestionWindow::CongestionWindow() {
	endServiceEvent = NULL;
}

CongestionWindow::~CongestionWindow() {
}

#endif /* CONGESTIONWINDOW */
