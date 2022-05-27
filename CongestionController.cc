#ifndef CONGESTIONWINDOW
#define CONGESTIONWINDOW

#include <string.h>
#include <omnetpp.h>
#include "CongestionController.h"

using namespace omnetpp;

typedef std::map<int,pairPacketData>::iterator windowIterator;

CongestionController::CongestionController() {
}

CongestionController::~CongestionController() {
}

int CongestionController::getAck(int seqN) {
	windowIterator pairIterator = slidingWindow.find(seqN);
	if (pairIterator == slidingWindow.end()) {
		// No existe en el diccionario
		std::cout << "CC :: WARNING :: getAck( " << seqN << ") didn't found any pairPacketData";
		return -1;
	}
	return pairIterator->second->ackCounter;
}

void CongestionController::addAck(int seqN) {
	windowIterator pairIterator = slidingWindow.find(seqN);
	if (pairIterator != slidingWindow.end()) {
		pairPacketData pair = pairIterator->second;
		(pair->ackCounter)++;
		slidingWindow[seqN] = pair;
	}
}

void CongestionController::addVolt(Volt * volt) {
	int seqN = volt->getSeqNumber();
	std::cout << "CC :: Adding Volt " << seqN << " to congestion controller\n";
	pairPacketData newPair = new _pairPacketData();
	newPair->ackCounter = 0;
	newPair->volt = volt;
	if(slidingWindow.find(seqN) != slidingWindow.end()) {
		std::cout << "CC :: WARNING :: addVolt is overwriting a previous instance\n";
	}
	slidingWindow[seqN] = newPair;
	bytesInFlight += volt->getByteLength();
	std::cout << "CC :: Volt " << seqN << " added successfully\n";
}

Volt * CongestionController::popVolt(int seqN) {
	Volt * volt = NULL;
	windowIterator pairIterator = slidingWindow.find(seqN);
	if (pairIterator != slidingWindow.end()) {
		std::cout << "CC :: Removing Volt " << seqN << " from congestion controller\n";
		volt = pairIterator->second->volt;
		delete(pairIterator->second);
		slidingWindow.erase(pairIterator);
		bytesInFlight -= volt->getByteLength();
	} else {
		std::cout << "CC :: WARNING :: popVolt could not find Volt " << seqN << "\n";
	};
	return volt;
}

double CongestionController::getSendTime(int seqN){
    windowIterator pairIterator = slidingWindow.find(seqN);
    if (pairIterator == slidingWindow.end()) {
        std::cout << "CC :: WARNING :: getSendTime( " << seqN << ") didn't found any pairPacketData";
        return -1;
    }
    return pairIterator->second->sendTime;
}

void CongestionController::addSendTime(int seqN, double time){
    windowIterator pairIterator = slidingWindow.find(seqN);
    if (pairIterator != slidingWindow.end()) {
        pairPacketData pair = pairIterator->second;
        pair->sendTime = time;
        slidingWindow[seqN] = pair;
    }
}

int CongestionController::getBaseWindow() {
	return baseWindow;
}

void CongestionController::setBaseWindow(int base) {
	std::cout << "CC :: Moving Sliding Window from " << baseWindow << " to " << base << "\n";
	baseWindow = base;
}

int CongestionController::amountBytesInFlight() {
	return bytesInFlight;
}

#endif /* CONGESTIONCONTROLLER */

