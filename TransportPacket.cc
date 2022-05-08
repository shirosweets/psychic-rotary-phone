#ifndef TRANSPORTPACKET
#define TRANSPORTPACKET

#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

class TransportPacket: public cPacket {
public:
    TransportPacket();
    virtual ~TransportPacket();
    // Check 6 Message Definitions
    virtual TransportPacket *dup();
    virtual string str();
protected:
//    virtual void get(cMessage *msg);
};

Define_Module(TransportPacket);

TransportPacket::TransportPacket() {
//    endServiceEvent = NULL;
}

TransportPacket::~TransportPacket() {
//    cancelAndDelete(endServiceEvent);
}

TransportPacket* TransportPacket::dup() {
	// FIXME
	return new TransportPacket(*this);
}

string TransportPacket::str() {
	return "UwU Packet"
}

#endif /* TRANSPORTPACKET */
