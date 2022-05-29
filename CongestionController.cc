#ifndef CONGESTIONWINDOW
#define CONGESTIONWINDOW

#include <string.h>
#include <omnetpp.h>
#include "CongestionController.h"

using namespace omnetpp;

typedef std::map<int,pairPacketData>::iterator windowIterator;

SlidingWindow::SlidingWindow() {
}

SlidingWindow::~SlidingWindow() {
}

int SlidingWindow::getAck(int seqN) {
	windowIterator pairIterator = slidingWindow.find(seqN);
	if (pairIterator == slidingWindow.end()) {
		// No existe en el diccionario
		std::cout << "CC :: WARNING :: getAck(" << seqN << ") didn't found any pairPacketData\n";
		return -1;
	}
	return pairIterator->second->ackCounter;
}

void SlidingWindow::addAck(int seqN) {
	windowIterator pairIterator = slidingWindow.find(seqN);
	if (pairIterator != slidingWindow.end()) {
		pairPacketData pair = pairIterator->second;
		(pair->ackCounter)++;
		std::cout << "CC :: ackCounter[" << seqN << "]++ " << pair->ackCounter << "\n";
		slidingWindow[seqN] = pair;
	}
}

void SlidingWindow::addVolt(Volt * volt) {
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
	std::cout << "CC :: !bytesInFlight " << bytesInFlight << "\n";  // FIXME
	std::cout << "CC :: Volt " << seqN << " added successfully\n";
}

Volt * SlidingWindow::popVolt(int seqN) {
	Volt * volt = NULL;
	windowIterator pairIterator = slidingWindow.find(seqN);
	if (pairIterator != slidingWindow.end()) {
		std::cout << "CC :: Removing Volt " << seqN << " from congestion controller\n";
		volt = pairIterator->second->volt;
		delete(pairIterator->second);
		slidingWindow.erase(pairIterator);
		bytesInFlight -= volt->getByteLength();
		std::cout << "CC :: !bytesInFlight " << bytesInFlight << "\n";  // FIXME
	} else {
		std::cout << "CC :: WARNING :: popVolt could not find Volt " << seqN << "\n";
	};
	return volt;
}

Volt * SlidingWindow::dupVolt(int seqN) {
	Volt * volt = NULL;
	windowIterator pairIterator = slidingWindow.find(seqN);
	if(pairIterator != slidingWindow.end()) {
		std::cout << "CC :: Copying Volt " << seqN << "\n";
		volt = pairIterator->second->volt->dup();
	} else {
		std::cout << "CC :: WARNING :: dupVolt could not find Volt " << seqN << "\n";
	}
	return volt;
}

bool SlidingWindow::isVoltInWindow(int seqN){
	windowIterator pairIterator = slidingWindow.find(seqN);
	return pairIterator != slidingWindow.end();
}

double SlidingWindow::getSendTime(int seqN){
    windowIterator pairIterator = slidingWindow.find(seqN);
    if (pairIterator == slidingWindow.end()) {
        std::cout << "CC :: WARNING :: getSendTime(" << seqN << ") didn't found any pairPacketData\n";
        return -1;
    }
    return pairIterator->second->sendTime;
}

void SlidingWindow::addSendTime(int seqN, double time){
    windowIterator pairIterator = slidingWindow.find(seqN);
    if (pairIterator != slidingWindow.end()) {
        pairPacketData pair = pairIterator->second;
        pair->sendTime = time;
        slidingWindow[seqN] = pair;
    }
}

int SlidingWindow::getBaseWindow() {
	return baseWindow;
}

void SlidingWindow::setBaseWindow(int base) {
	std::cout << "CC :: Moving Sliding Window from " << baseWindow << " to " << base << "\n";
	baseWindow = base;
}

int SlidingWindow::amountBytesInFlight() {
	std::cout << "CC :: bytesInFlight " << bytesInFlight << "\n";
	return bytesInFlight;
}

#endif /* CONGESTIONCONTROLLER */

