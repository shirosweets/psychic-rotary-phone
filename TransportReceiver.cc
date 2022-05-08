#ifndef TRANSPORTRECEIVER
#define TRANSPORTRECEIVER

#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

class TransportReceiver : public cSimpleModule {
private:
    //cStdDev delayStats;
    //cOutVector delayVector;
public:
    TransportReceiver();
    virtual ~TransportReceiver();
protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(TransportReceiver);

TransportReceiver::TransportReceiver() {
}

TransportReceiver::~TransportReceiver() {
}

void TransportReceiver::initialize(){
    // stats and vector names
    // delayStats.setName("TotalDelay");
    // delayVector.setName("Delay");
}

void TransportReceiver::finish(){
    // stats record at the end of simulation
    // recordScalar("Avg delay", delayStats.getMean());
    // recordScalar("Number of packets", delayStats.getCount());
}

void TransportReceiver::handleMessage(cMessage * msg) {
    // compute queuing delay
    simtime_t delay = simTime() - msg->getCreationTime();
    // update stats
    // delayStats.collect(delay);
    // delayVector.record(delay);
    // delete msg
    // delete(msg);
    send(cMessage, "appLayerOut");
}

#endif /* TRANSPORTRECEIVER */
