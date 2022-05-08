#ifndef TRANSPORTSENDER
#define TRANSPORTSENDER

#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

class TransportSender : public cSimpleModule {
private:
    //cStdDev delayStats;
    //cOutVector delayVector;
public:
    TransportSender();
    virtual ~TransportSender();
protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(TransportSender);

TransportSender::TransportSender() {
}

TransportSender::~TransportSender() {
}

void TransportSender::initialize(){
    // stats and vector names
    // delayStats.setName("TotalDelay");
    // delayVector.setName("Delay");
}

void TransportSender::finish(){
    // stats record at the end of simulation
    // recordScalar("Avg delay", delayStats.getMean());
    // recordScalar("Number of packets", delayStats.getCount());
}

void TransportSender::handleMessage(cMessage * msg) {
    // compute queuing delay
    simtime_t delay = simTime() - msg->getCreationTime();
    // update stats
    // delayStats.collect(delay);
    // delayVector.record(delay);
    // delete msg
    // delete(msg);
    send(cMessage, "subnetwork$o");
}

#endif /* TRANSPORTSENDER */
