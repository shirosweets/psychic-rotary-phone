#ifndef SINK
#define SINK

#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

class Sink : public cSimpleModule {
private:
    cStdDev delayStats;
    cOutVector delayVector;
public:
    Sink();
    virtual ~Sink();
protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(Sink);

Sink::Sink() {
}

Sink::~Sink() {
}

void Sink::initialize(){
    delayStats.setName("TotalDelay");
    delayVector.setName("Delay");
}

void Sink::finish(){
    // Stats record at the end of simulation
    recordScalar("Avg delay", delayStats.getMean());
    recordScalar("Number of packets", delayStats.getCount());
}

void Sink::handleMessage(cMessage * msg) {
    // Compute queuing delay
    simtime_t delay = simTime() - msg->getCreationTime();

    // Update stats
    delayStats.collect(delay);
    delayVector.record(delay);

    // Delete msg
    delete(msg);
}

#endif /* SINK */
