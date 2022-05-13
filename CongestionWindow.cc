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

void CongestionWindow::addTimeoutMsg(int seqN, EventTimeout * msg){
	if(this->getAvailableWin() > 0){
		EventTimeout *event = new EventTimeout(*msg);
		this->window[seqN] = event;
		this->msgSendingAmount++;
	}
}

void CongestionWindow::cancelTimeoutMsg(int seqN){
	if(this->window[seqN]->isScheduled()){
		this->window[seqN]->cancelEvent();
		this->msgSendingAmount--;
	}
}

#endif /* CONGESTIONWINDOW */
