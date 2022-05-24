#ifndef CONGESTIONWINDOW
#define CONGESTIONWINDOW

#include <string.h>
#include <omnetpp.h>
#include "CongestionWindow.h"

using namespace omnetpp;

typedef std::map<int, EventTimeout*>::iterator cwIterator;

CongestionWindow::CongestionWindow() {
	maxSize = 2147483647;
	size = 0;
	msgSendingAmount = 0;
}

CongestionWindow::~CongestionWindow() {
}

int CongestionWindow::getMaxSize(){
	return maxSize;
}

int CongestionWindow::getSize() {
	return size;
}

void CongestionWindow::logAvailableWin() {
	std::cout << "CW :: Current Available Window: " << getAvailableWin();
	std::cout << "/" << size << " bytes\t";
	std::cout << "(map size = " << window.size() << ")\n";
}

int CongestionWindow::getAvailableWin(){
	int availableWindow = size - msgSendingAmount;
	availableWindow = availableWindow >= 0 ? availableWindow : 0;
	return availableWindow;
}

void CongestionWindow::setSize(int newSize){
	if(newSize <= this->maxSize){
		logAvailableWin();
		std::cout << "CW :: Changing window size\n";
		size = newSize;
		logAvailableWin();
	}
}

void CongestionWindow::addTimeoutMsg(EventTimeout * event){
	if(event == NULL) {
		std::cout << "CW :: WARNING :: call to addTimeoutMsg(NULL)\n";
		return;
	} else if(getAvailableWin() >= event->getPacketSize()) {
		int seqN = event->getSeqN();
		window[seqN] = event;
		logAvailableWin();
		std::cout << "CW :: Adding seqN " << seqN << " to the window.";
		std::cout << "\t\t(map size = " << window.size() << ")\n";
		msgSendingAmount += event->getPacketSize();
		logAvailableWin();
	} else {
		std::cout << "CW :: WARNING :: Window has no space for new timeout\n";
	}
}

EventTimeout * CongestionWindow::popTimeoutMsg(int seqN){
	cwIterator timeoutIterator = window.find(seqN);
	if (timeoutIterator == window.end()) {
		std::cout << "CW :: WARNING :: Tried popTimeoutMsg(" << seqN;
		std::cout << ") but no event in window. returning NULL\n";
		return NULL;
	}
	EventTimeout * event = timeoutIterator->second;
	logAvailableWin();
	msgSendingAmount -= event->getPacketSize();
	logAvailableWin();
	window.erase(timeoutIterator);
	std::cout << "CW :: Removed seqN " << seqN << " from window.";
	std::cout << "\t\t(map size = " << window.size() << ")\n";
	return event;
}

bool CongestionWindow::getSlowStart(){
	return isSlowStartStage;
}

void CongestionWindow::setSlowStart(bool state) {
	isSlowStartStage = state;
}

#endif /* CONGESTIONWINDOW */
