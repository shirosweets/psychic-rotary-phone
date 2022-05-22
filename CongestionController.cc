#ifndef CONGESTIONWINDOW
#define CONGESTIONWINDOW

#include <string.h>
#include <omnetpp.h>
#include "CongestionController.h"

using namespace omnetpp;

CongestionController::CongestionController() {
	slidingWindow = new pairPacketData[1000]; // FIXME 134MB 134217727
}

CongestionController::~CongestionController() {
	delete[] slidingWindow;
}

int CongestionController::getAck(int seqN) {
	pairPacketData pair = slidingWindow[seqN];
	return pair->ackCounter;
}

void CongestionController::addAck(int seqN) {
	pairPacketData pair = slidingWindow[seqN];
	(pair->ackCounter)++;
	slidingWindow[seqN] = pair;
}

int CongestionController::getThreshold() {
	return threshold;
}

void CongestionController::setThreshold(int newValue) {
	threshold = newValue;
}

void CongestionController::addVolt(Volt * volt) {
	int seqN = volt->getSeqNumber();
	std::cout << "CC :: Adding Volt " << seqN << " to congestion controller\n";
	pairPacketData newPair = new _pairPacketData();
	newPair->ackCounter = 0;
	newPair->volt = volt;
	slidingWindow[seqN] = newPair;
}

Volt * CongestionController::popVolt(int seqN) {
	std::cout << "CC :: Volt " << seqN << " added successfully\n";
	pairPacketData pair = slidingWindow[seqN];
	Volt * volt = pair->volt;
	slidingWindow[seqN] = NULL;
	delete(pair);
	return volt;
}

int CongestionController::getBaseWindow() {
	return baseWindow;
}

void CongestionController::setBaseWindow(int base) {
	std::cout << "CC :: Moving Sliding Window from " << baseWindow << " to " << base << "\n";
	baseWindow = base;
}

bool CongestionController::getSlowStart(){
	return isSlowStartStage;
}

void CongestionController::setSlowStart(bool state) {
	isSlowStartStage = state;
}

#endif /* CONGESTIONCONTROLLER */

