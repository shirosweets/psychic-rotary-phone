# Diseño

## Introducción a TLCP

**Transport Limited Control Protocol**

**TLCP**, el protocolo de capa de transporte que implementamos, es una aproximación limitada a *TCP-Reno*, por lo que comparte varias de sus características generales. Tiene control de congestión sin tener acceso directo a la subred, y control de flujo mediante una negociación entre hosts.

Las características del algoritmo son las siguientes:

 * Segmentos intercambiados de mismo tipo.
   > El tipo de mensaje que se intercambian los host (llamados en nuestra implementación `Sender` y `Receiver`) tanto para mensajes de data como para mensaje de control con el mismo. Lo llamamos **`Volt`**.
 * Ventana de control.
 * Ventana de congestión Reno
   * Arranque lento al inicializar
   * Seteo de la *VC* (ventana de congestión) a la mitad luego de un timeout
   * Aumento linear de la *VC* cada RTT de congestión) luego de un timeout
 * Temporizador de Retransmisión basado en RTT
 * Estimación suave de RTT basado en tiempo de respuesta.
   > Se implementó **Algoritmo de Jacobson (1988)** y **Algoritmo de Karn**
 * Tamaño de header del `Volt` de 9 bytes
   > **flags**: 1 byte (flags: `ACK`, `RET`)
   >
   > **número de secuencia**: 4 bytes
   >
   > **tamaño de ventana de control**: 4 bytes

# Iteraciones de diseño

## Ideas Preliminares

Desde un principio decidimos que conceptos iba a satisfacer nuestro protocolo:
  - retransmisión,
  - control de flujo,
  - control de congestión,
  - formato de paquetes, 

El primer acercamiento a nuestra implementación fue el metodo de parada y espera (*stop-and-wait*). Este metodo asegura que la información no se pierda y que los paquetes se reciban en orden pero dada su metodología, pero el rendimiento era peor que el diseño proporcionado por el kickstart de la cátedra. Por esta razón descartamos esta implementación.concluye con un uso pobre de la red.

Si bien resuelve el problema de flujo e indirectamente el problema de congestión debido al extremo desuso de la red.

## Evolución del modelo base

# Ventana de congestión
La primera adición a nuestro modelo fue `RenoManager.h` (originalmente llamado *CongestionWindow.h*) esta nos permite organizar cuales son los paquetes que estan en la red en todo momento. Un detalle de la implementación es que cuando un paquete es agregado a la ventana inicia el temporizador de un evento del `timeout` de ese paquete, el cual nos sirve para retransmitir paquetes, este modulo es utilizado por la capa del transmisor. Otra característica es la posibilidad de recibir `feedback` de la capa de receptor a través de ACKs.

```C++
class RenoManager {
private:
	int maxSize; // INT_MAX
	int size;
	int msgSendingAmount;
	bool isSlowStartStage;
	std::map<int, EventTimeout*> window;
	void logAvailableWin();
public:
  int getMaxSize();
  int getSize();
	int getAvailableWin();
	void setSize(int newSize);
	void addTimeoutMsg(EventTimeout * msg);
	EventTimeout * popTimeoutMsg(int seqN);
  bool getSlowStart();
  void setSlowStart(bool state);
}
```

# Controlador de congestión

**Decisiones de diseño**

- Solo actualizamos el RTT cuando no sean acks duplicados ni retransmitidos.
- Nunca vamos a tener acks duplicados que no seas paquetes retransmitidos.

# 

# Primera versión

Lo primero que apuntamos

----

# Iteración de implemenación TEMPLATE

Volt
CongestionWindow
CongestionController
Slow start
Retransmission
RTT Dynamic

# Estructura final de la capa de transporte

## `Volt`

El paquete que se intercambian los hosts se llama *Volt*, y fue generado con la herramienta de template que ofrece omnet (`opp_msgc`) con el siguiente template:

```C++
packet Volt
{
     bool ackFlag = false;
     int seqNumber;
     int windowSize;
};
```

POR LO QUE 

El número de secuencia sigue el mismo uso que TCP, donde indica el byte inicial de la tranmisión y el siguiente byte esperado en el ack.

### Flags

Existen las siguientes flags:

 * 
*ACK*:

 * **RET**:

