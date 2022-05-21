#ifndef CONGESTIONWINDOW
#define CONGESTIONWINDOW

#include <string.h>
#include <omnetpp.h>
#include "CongestionWindow.h"

using namespace omnetpp;

CongestionWindow::CongestionWindow() {
	size = 0;
}

CongestionWindow::~CongestionWindow() {
}

int CongestionWindow::getSize() {
	return this->size;
}

int CongestionWindow::getAvailableWin(){
	return ((this->size / sizeof(EventTimeout *)) - (this->msgSendingAmount * sizeof(EventTimeout *)));
}

void CongestionWindow::setSize(int newSize){
	if(newSize <= this->maxSize){
		this->size = newSize;
	}
}

void CongestionWindow::addTimeoutMsg(int seqN, EventTimeout * event){
	if(this->getAvailableWin() > 0){
		this->window[seqN] = event;
		this->msgSendingAmount++;
	}
}

EventTimeout * CongestionWindow::popTimeoutMsg(int seqN){
	EventTimeout * event = window[seqN];
	window[seqN] = NULL;
	return event;
}

#endif /* CONGESTIONWINDOW */
