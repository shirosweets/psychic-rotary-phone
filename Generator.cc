#ifndef GENERATOR
#define GENERATOR

#include <string.h>
#include <omnetpp.h>
#include "Volt.h"

using namespace omnetpp;

class Generator : public cSimpleModule {
private:
	int currentSeqNumber;
    cMessage *sendMsgEvent;
    cStdDev transmissionStats;
public:
    Generator();
    virtual ~Generator();
protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
};
Define_Module(Generator);

Generator::Generator() {
    sendMsgEvent = NULL;
    currentSeqNumber = 0;
}

Generator::~Generator() {
    cancelAndDelete(sendMsgEvent);
}

void Generator::initialize() {
    transmissionStats.setName("TotalTransmissions");
    // Create the send packet
    sendMsgEvent = new cMessage("sendEvent");
    // Schedule the first event at random time
    scheduleAt(par("generationInterval"), sendMsgEvent);
}

void Generator::finish() {
    recordScalar("Number of packets generated", transmissionStats.getCount());
}

void Generator::handleMessage(cMessage *msg) {
    // Create new packet
    Volt *volt = new Volt("packet");
    volt->setByteLength(par("packetByteSize"));
    volt->setAckFlag(false);
    volt->setSeqNumber(currentSeqNumber);
    currentSeqNumber = (currentSeqNumber + 1) % 1000; // FIXME

    // Stats
    transmissionStats.collect(1);

    std::cout << "Generator :: New Packet :: " << volt->getSeqNumber() << "\n";
    // Send to the output
    send(volt, "out");

    // Compute the new departure time
    simtime_t departureTime = simTime() + par("generationInterval");
    // Schedule the new packet generation
    scheduleAt(departureTime, sendMsgEvent);
}

#endif /* GENERATOR */
