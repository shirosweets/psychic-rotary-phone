#ifndef RENOMANAGER
#define RENOMANAGER

#include <string.h>
#include <omnetpp.h>
#include "RenoManager.h"

using namespace omnetpp;

typedef std::map<int, EventTimeout*>::iterator cwIterator;

RenoManager::RenoManager() {
	maxSize = 2147483640;
	size = 0;
	msgSendingAmount = 0;
}

RenoManager::~RenoManager() {
}

int RenoManager::getMaxSize(){
	return maxSize;
}

int RenoManager::getSize() {
	return size;
}

void RenoManager::logAvailableWin() {
	std::cout << "RM :: Current Available Window: " << getAvailableWin();
	std::cout << "/" << size << " bytes\t";
	std::cout << "(map size = " << window.size() << ")\n";
}

int RenoManager::getAvailableWin(){
	int availableWindow = size - msgSendingAmount;
	availableWindow = availableWindow >= 0 ? availableWindow : 0;
	return availableWindow;
}

void RenoManager::setSize(int newSize){
	if(newSize <= this->maxSize){
		logAvailableWin();
		std::cout << "RM :: Changing window size\n";
		size = newSize;
		logAvailableWin();
	}
}

void RenoManager::addTimeoutMsg(EventTimeout * event){
	if(event == NULL) {
		std::cout << "RM :: WARNING :: call to addTimeoutMsg(NULL)\n";
		return;
	} else if(getAvailableWin() >= event->getPacketSize()) {
		int seqN = event->getSeqN();
		window[seqN] = event;
		logAvailableWin();
		std::cout << "RM :: Adding seqN " << seqN << " to the window.";
		std::cout << "\t\t(map size = " << window.size() << ")\n";
		msgSendingAmount += event->getPacketSize();
		logAvailableWin();
	} else {
		std::cout << "RM :: WARNING :: Window has no space for new timeout\n";
	}
}

EventTimeout * RenoManager::popTimeoutMsg(int seqN){
	cwIterator timeoutIterator = window.find(seqN);
	if (timeoutIterator == window.end()) {
		std::cout << "RM :: WARNING :: Tried popTimeoutMsg(" << seqN;
		std::cout << ") but no event in window. returning NULL\n";
		return NULL;
	}
	EventTimeout * event = timeoutIterator->second;
	logAvailableWin();
	msgSendingAmount -= event->getPacketSize();
	logAvailableWin();
	window.erase(timeoutIterator);
	std::cout << "RM :: Removed seqN " << seqN << " from window.";
	std::cout << "\t\t(map size = " << window.size() << ")\n";
	return event;
}

bool RenoManager::getSlowStart(){
	return isSlowStartStage;
}

void RenoManager::setSlowStart(bool state) {
	isSlowStartStage = state;
}

#endif /* RENOMANAGER */
