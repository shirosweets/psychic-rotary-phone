#ifndef CONGESTIONWINDOW
#define CONGESTIONWINDOW

#include <string.h>
#include <omnetpp.h>
#include "CongestionWindow.h"

using namespace omnetpp;

CongestionWindow::CongestionWindow() {
	maxSize = 2147483647;
	size = 0;
	msgSendingAmount = 0;

	int lenWin = (sizeof(window)/sizeof(window[0]));

	for(int i = 0; i<lenWin; i++){
		window[i] = NULL;
	}
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
	std::cout << "/" << size << " bytes\n";
}

int CongestionWindow::getAvailableWin(){
//	int test = ((this->size / sizeof(EventTimeout *)) - (this->msgSendingAmount * sizeof(EventTimeout *)));
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
		return;
	} else if(getAvailableWin() >= event->getPacketSize()) {
		int seqN = event->getSeqN();
		window[seqN] = event;
		logAvailableWin();
		std::cout << "CW :: @addTimeoutMsg :: Adding seqN " << seqN << " to the window\n";
		msgSendingAmount += event->getPacketSize();
		logAvailableWin();
	}
}

EventTimeout * CongestionWindow::popTimeoutMsg(int seqN){
	EventTimeout * event = window[seqN];
	if(event == NULL) {
		std::cout << "CW :: Tried pipTimeoutMsg(" << seqN << ") but no event in window. Return NULL\n";
		return NULL;
	}
	logAvailableWin();
	std::cout << "CW :: @popTimeoutMsg :: Removing seqN " << seqN << " from window\n";
	msgSendingAmount -= event->getPacketSize();
	logAvailableWin();
	window[seqN] = NULL;
	return event;
}

#endif /* CONGESTIONWINDOW */
