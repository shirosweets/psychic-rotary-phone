simple Queue
{
    parameters:
        int bufferSize;
        @display("i=block/queue;q=buffer");
    gates:
        input in;
        output out;
}

simple Generator // Sender App Layer
{
    parameters:
        volatile double generationInterval; // sec
        int packetByteSize;
        @display("i=block/source");
    gates:
        output out;
}

simple Sink // Receiver App Layer
{
    parameters:
        @display("i=block/sink");
    gates:
        input in;
}

// TransportTx
simple TransportSender
{
    parameters:
        int bufferSize;
        int packetByteSize;
        double rtt @unit(s);  // sec
        double timeout @unit(s);  // sec
        @display("i=block/queue;q=buffer");
    gates:
        input appLayerIn; // toApp
        inout subnetwork; // toOut
}

// TransportRx
simple TransportReceiver
{
    parameters:
        int bufferSize;
        int packetByteSize;
        @display("i=block/queue;q=buffer");
    gates:
        inout subnetwork;
        output appLayerOut;
}

// NodeTx
module Sender
{
    parameters:
        @display("i=block/routing");
    gates:
        input in;
        output out;
    submodules:
        senderAppLayer: Generator {
            parameters:
                @display("p=100,30");
        }
        transpSend: TransportSender {
            parameters:
                @display("p=100,137");
        }
    connections:
        in --> transpSend.subnetwork$i; // Conexion que maneja los ACKs entrantes
        senderAppLayer.out --> transpSend.appLayerIn; // Desde el generador de paquetes a la queue de capa de transporte
        transpSend.subnetwork$o --> out; // Salida de la capa de transportes a paquetes
}

// NodeRx
module Receiver
{
    parameters:
        @display("i=block/routing");
    gates:
        input in;
        output out;
    submodules:
        recAppLayer: Sink {
            parameters:
                @display("p=100,30");
        }
        transpRec: TransportReceiver {
            parameters:
                @display("p=100,137");
        }
    connections:
        in --> transpRec.subnetwork$i; // Conexion que maneja los paquetes de data entrantes del Sender
        // ** Start Study Cases ** //
        // Case I 
        transpRec.appLayerOut --> { datarate = 0.5Mbps; } --> recAppLayer.in; // Desde la queue del receptor a la capa de aplicacion
        // Case II
//         transpRec.appLayerOut --> { datarate = 1Mbps; } --> recAppLayer.in; // Desde la queue del receptor a la capa de aplicacion
        // ** End Study Cases ** //
        transpRec.subnetwork$o --> out; // Salida para mandar ACKs
}

network Network
{
    @display("bgl=2");
    submodules:
        sender: Sender {
            @display("p=30,70");
        }
        dataQueue: Queue {
            @display("p=130,30");
        }
        ackQueue: Queue {
            @display("p=130,101");
        }
        rec: Receiver {
            @display("p=230,70");
        }
    connections:
        sender.out --> { datarate = 1Mbps; delay = 100us; } --> dataQueue.in;
        // ** Start Study Cases ** //
        // Case I
        dataQueue.out --> { datarate = 1Mbps; delay = 100us; } --> rec.in;
        // Case II
//         dataQueue.out --> { datarate = 0.5Mbps; delay = 100us; } --> rec.in;
        // ** End Study Cases ** //
        rec.out --> { datarate = 1Mbps; delay = 100us; } --> ackQueue.in;
        ackQueue.out --> { datarate = 1Mbps; delay = 100us; } --> sender.in;
}
