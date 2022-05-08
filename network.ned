simple Generator
{
    parameters:
        volatile double generationInterval; // sec
        int packetByteSize;
        @display("i=block/source");
    gates:
        output out;
}

simple Queue
{
    parameters:
       	int bufferSize;
        @display("i=block/queue;q=buffer");
    gates:
        input in;
        output out;
}

simple Sink
{
    parameters:
        @display("i=block/sink");
    gates:
        input in;
}

// NodeTx
module Sender
{
    parameters:
        @display("i=block/routing");
    gates:
        output out;
    submodules:
        gen: Generator {
            parameters:
                @display("p=100,30");
        }
        queue: Queue {
            parameters:
                @display("p=100,137");
        }
    connections:
        gen.out --> queue.in;
        queue.out --> out;
}

// NodeRx
module Receiver
{
    parameters:
        @display("i=block/routing");
    gates:
        input in;
    submodules:
        sink: Sink {
            parameters:
                @display("p=100,30");
        }
        queue: Queue {
            parameters:
                @display("p=100,137");
        }
    connections:
        in --> queue.in;
        // Case I
        // queue.out --> { datarate = 0.5Mbps; } --> sink.in;
        // Case II
        queue.out --> {  datarate = 1Mbps; } --> sink.in;
}

network Network
{
    @display("bgl=2");
    submodules:
        sender: Sender {
            @display("p=30,30");
        }
        queue: Queue {
            @display("p=130,30");
        }
        rec: Receiver {
            @display("p=230,30");
        }
    connections:
        sender.out --> {  datarate = 1Mbps; delay = 100us; } --> queue.in;
        // Case I
        // queue.out --> { datarate = 1Mbps; delay = 100us;} --> rec.in;
        // Case II
        queue.out --> {  datarate = 0.5Mbps; delay = 100us; } --> rec.in;
}
