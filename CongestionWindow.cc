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

int CongestionWindow::getAvailableWin(){
//	int test = ((this->size / sizeof(EventTimeout *)) - (this->msgSendingAmount * sizeof(EventTimeout *)));
	int availableWindow = size - msgSendingAmount;
	availableWindow = availableWindow >= 0 ? availableWindow : 0;
	std::cout << "CW :: Current Available Window: " << availableWindow;
	std::cout << "/" << size << " (available/winSize)\n";
	return availableWindow;
}

void CongestionWindow::setSize(int newSize){
	if(newSize <= this->maxSize){
		size = newSize;
	}
}

void CongestionWindow::addTimeoutMsg(int seqN, EventTimeout * event){
	if(event == NULL) {
		return;
	} else if(getAvailableWin() > 0) {
		window[seqN] = event;
		msgSendingAmount += event->getPacketSize();
	}
}

EventTimeout * CongestionWindow::popTimeoutMsg(int seqN){
	EventTimeout * event = window[seqN];
	if(event == NULL) {
		return NULL;
	}
	msgSendingAmount -= event->getPacketSize();
	window[seqN] = NULL;
	return event;
}

#endif /* CONGESTIONWINDOW */
