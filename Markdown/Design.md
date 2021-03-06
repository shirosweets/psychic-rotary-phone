# Diseño de capa de transporte con control de flujo y congestión

Se presenta en detalle la especificación e implementación (en Omnet++) de nuestra solución para los problemas de control y flujo en la capa de transporte.

# Integrantes
- Carrizo, Ernesto.
- Domínguez, Agustín.
- Vispo, Valentina.

---

**[ENUNCIADO](../Markdown/Assignment.md) | [README](../README.md) | [ANÁLISIS](../Markdown/Analysis.md)**

---

## Contenido

- [Introducción a TLCP](#introducción-a-tlcp)
  - [¿Qué es TLCP?](#qué-es-tlcp)
  - [Arquitectura TLCP](#arquitectura-tlcp)
- [Iteraciones de diseño](#iteraciones-de-diseño)
  - [Ideas Preliminares](#ideas-preliminares)
  - [Evolución del modelo base](#evolución-del-modelo-base)
    - [Primera iteración o etapa del diseño](#primera-iteración-o-etapa-del-diseño)
    - [Segundo iteración o etapa del diseño](#segundo-iteración-o-etapa-del-diseño)
    - [Tercer iteración o etapa del diseño](#tercer-iteración-o-etapa-del-diseño)
    - [Cuarta iteración o etapa del diseño](#cuarta-iteración-o-etapa-del-diseño)
- [Ventana de congestión](#ventana-de-congestión)
- [Controlador de congestión](#controlador-de-congestión)
- [](#)
- [Primera versión](#primera-versión)
- [Estructura final de TLCP](#estructura-final-de-tlcp)
  - [`Volt`](#volt)
    - [seqNumber](#seqnumber)
    - [Flags](#flags)
    - [windowSize](#windowsize)
- [Control de flujo](#control-de-flujo)
- [Control de congestión](#control-de-congestión)
- [RTT](#rtt)
  - [Karn](#karn)
- [Cosas no implementadas de TCP o TCP-Reno](#cosas-no-implementadas-de-tcp-o-tcp-reno)
  - [Reordenamiento de paquetes](#reordenamiento-de-paquetes)
  - [ACK duplicados](#ack-duplicados)
- [Problemas que surgieron](#problemas-que-surgieron)
  - [IDE](#ide)
  - [Omnet y sus librerías](#omnet-y-sus-librerías)
  - [simtime_t](#simtime_t)

# Introducción a TLCP

## ¿Qué es TLCP?

**Transport Limited Control Protocol** (o TLCP), es el protocolo de capa de transporte que implementamos, es una aproximación limitada a *TCP-Reno*, por lo que comparte varias de sus características generales. Tiene control de congestión sin tener acceso directo a la subred, y control de flujo mediante una negociación entre hosts.

Las características del algoritmo son las siguientes:

 * Segmentos intercambiados de mismo tipo.
   > El tipo de mensaje que se intercambian los host (llamados en nuestra implementación `Sender` y `Receiver`) tanto para mensajes de data como para mensaje de control con el mismo. Lo llamamos **`Volt`**.
 * Ventana de control.
 * Ventana de congestión Reno
   * Arranque lento al inicializar
   * Seteo de la *VC* (ventana de congestión) a la mitad luego de un timeout
   * Aumento linear de la *VC* (cada RTT de congestión) luego de un timeout
 * Temporizador de Retransmisión basado en RTT
 * Estimación suave de RTT basado en tiempo de respuesta.
   > Se implementó algoritmo de Jacobson (1988) y una variante del algoritmo de Karn
 * Tamaño de header del `Volt` de 9 bytes.
   > **flags**: 1 byte (flags: `ACK`, `RET`)
   >
   > **número de secuencia**: 4 bytes
   >
   > **tamaño de ventana de control**: 4 bytes

## Arquitectura TLCP

![Arquitectura TLCP](/documents/assets/TLCP_Architecture.png)

# Iteraciones de diseño

## Ideas Preliminares

Desde un principio decidimos que conceptos iba a satisfacer nuestro protocolo:
  - retransmisión,
  - control de flujo,
  - control de congestión,
  - formato de paquetes,

La primera idea fue el metodo de parada y espera (*stop-and-wait*), ya que nos aseguraba que la información no se pierda y que los paquetes se reciban en orden, pero dado que tiene un uso pobre de la red y el rendimiento era peor que el diseño proporcionado por el [kickstart](../documents/lab3-kickstarter.tar.gz) de la cátedra descartamos esta implementación a pesar de que resolvía el problema de flujo e indirectamente el problema de congestión debido al extremo desuso de la red.
Además no nos servía para continuar nuestro camino a las funcionalidades de TCP-Reno.

## Evolución del modelo base

Luego de haber descartado parada y espera, se decidió evaluar cómo dividir las funcionalidades de TCP-Reno en diferentes etapas para su implementación.

### Primera iteración o etapa del diseño
- Mensajes para ambos tipos de mensajes (datos y acks).
- `cPacket` propio: `Volt` v1.
  - `N° de seq`.
  - `ACK` flag.
- `BufferSize` en cada cola.
- Ventana de congestión (`CW`) v1.
  - `maxSize`.
  - `size`

### Segundo iteración o etapa del diseño
- Timer -> Timeout.
  - Evento.
- Volt v2
  - RET flag.
- `RTT` fijo.
- Retransmisión.
- Ventana corrediza (`SW`) v1.
- `CW` v2.
  - Cantidad de mensajes envíandose.
  - Stages
    - arranque lento al inicio, y
    - luego recuperación rápido.


### Tercer iteración o etapa del diseño
- Control de flujo.
- `RTT` dinámico v1.
  - simtime_t en segundos.
  - RTO.
  - Desviación estándar.
  - Misma inicialización siempre.

### Cuarta iteración o etapa del diseño
- `RTT` dinámico v2.
  - si es el primer ACK recibo: inicializa `rto = rtt * 3.0`.

**Decisiones de diseño**

- Solo actualizamos el RTT cuando no sean acks duplicados ni retransmitidos.
- Nunca vamos a tener acks duplicados que no seas paquetes retransmitidos.

# Ventana de congestión

Lo primero que apuntamos fue tener registro de los paquetes de la red eso dio lugar a la implementacion de `CongestionWindow.h` que luego paso a tener mas funcionalidades por lo que paso a llamarse [`RenoManager.h`](../RenoManager.h).

- Con las adiciones de estos modulos y lógica del lado del emisor conseguimos retransmitir los paquetes perdidos pero no aun el control de congestión. Esas fueron agregados posteriores cuando implementamos arranque lento.

Este módulo nos permite organizar cuales son los paquetes que estaá en la red en todo momento, y es utilizado por la capa del transmisor.

Un detalle de la implementación es que cuando un paquete es agregado a la ventana inicia el temporizador de un evento del `timeout` de ese paquete, el cual nos sirve como un valor "time-to-live" para saber cuando retransmitir paquetes. Otra característica es la posibilidad de recibir `feedback` de la capa de receptor a través de ACKs.

**RenoManager.h**

```C++
class RenoManager {
private:
  int maxSize;
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

# Estructura final de TLCP

## `Volt`

El paquete que se intercambian los hosts se llama *Volt*, y fue generado con la herramienta de template que ofrece omnet (`opp_msgc`) con el siguiente template:

[**Volt.msg**](../Volt.msg)

```C++
packet Volt {
  bool flags = false;
  int seqNumber;
  int windowSize;
};
```

La primer versión del Volt solo consistía del mensaje y número de secuencia, posteriormente se agregraron las flags.

[**Volt.h**](../Volt.h)

```cpp
class Volt : public ::omnetpp::cPacket
{
  protected:
    bool flags;
    int seqNumber;
    int windowSize;
  public:
    virtual Volt *dup() const override {return new Volt(*this);}
    // Nuestros métodos
    virtual bool getFlags() const;
    virtual void setFlags(bool flags);
    bool getAckFlag();
    void setAckFlag(bool ackFlag);
    bool getRetFlag();  // Retransmission Flag
    void setRetFlag(bool retFlag);
    virtual int getSeqNumber() const;
    virtual void setSeqNumber(int seqNumber);
    virtual int getWindowSize() const;
    virtual void setWindowSize(int windowSize);
```

### seqNumber

El número de secuencia indica el orden de los paquetes. A diferencia de TCP, no implementamos números de secuencia por posición de byte, por lo que los números de secuencia simplemente **enumeran cada paquete**, y todos los paquetes tienen tamaño fijo (de *12500 b*).

Por cuestión de tiempo, simplemente seteamos el `MAX_SEQ_N = 1000`

> *Nota:* Planeabamos hacer el cambio a la implementación por bytes, y en gran parte la estructura actual del Sender ya permite eso, por lo que la transición no sería tan costosa. Se puede ver en [**mejoras posibles**](../Markdown/Analysis.md/#mejoras-posibles) para ver las posibles mejoras respecto a esto.

### Flags

Implementamos dos **FLAGS** actualmente. `ACK` y `RET`. Ambas encodeadas en el byte de flags. Para setear y obtener esos valores, hacemos *operaciones bitwise* y uso de *máscara de bits*.

**`ACK`** : Indica que el Volt actual es un volt de tipo ACK, no de datos.

**`RET`** : Indica que el Volt actual fue retransmitido. En caso de un Volt de tipo ACK, indica que es un ACK de un paquete que fue retransmitido. Esta información nos sirve para actualizar acordemente el RTT dinámico. (Ver seccion [*Karn*](#karn))

### windowSize

Por último este dato nos sirve para comunicar al emisor el tamaño de la ventana actual del receptor. (Ver sección [*Control de Flujo*](#control-de-flujo))

El windowSize máximo permitido es `2147483640`, lo cual es muy cercano al MAX_INT que se puede tener en **C**.

# EventTimeout

```cpp
class EventTimeout : public ::omnetpp::cMessage
{
  protected:
    int seqN;
    int packetSize;
  public:
    virtual int getSeqN() const;
    virtual void setSeqN(int seqN);
    virtual int getPacketSize() const;
    virtual void setPacketSize(int packetSize);
```

# Control de Flujo

Para control de flujo simplemente guardamos la ventana de control actual en un entero directamente en el Sender, llamada `currentControlWindowSize`. Cuando se consulta si se puede enviar un Volt, se compara este número con la cantidad actual de mensajes en el *aire*, y si esa cantidad más el paquete que se desea enviar no supera la ventana, podemos enviar el paquete actual.

# Control de Congestión

Implementamos Reno con ventana corrediza y control por timeouts con RTT dinámico.

## `SlidingWindow`

TAD Utilizado:

```cpp
struct __packetMetadata {
	Volt * volt;
	int ackCounter;
    double sendTime = 0;
    bool retransmitted = false;
};
```

```C++
class SlidingWindow {
private:
	int baseWindow = 0;
  int bytesInFlight = 0;
	std::map<int,packetMetadata> slidingWindow;
public:
    int getAck(int seqN);
    void addAck(int seqN);
    void addVolt(Volt * volt);
    Volt * popVolt(int seqN);
    Volt * dupVolt(int seqN);
    bool isVoltInWindow(int seqN);
    double getSendTime(int seqN);
    void addSendTime(int seqN, double time);
    int getBaseWindow();
    void setBaseWindow(int base);
    int amountBytesInFlight();
};
```

La `SlidingWindow` utiliza principalmente un diccionario de Int (Numeros de secuencia) -> packetMetadata que es nuestro TAD donde guardamos el paquete `volt`, la cantidad de ACKs que llego de ese paquete `ackCounter`, el tiempo de enviado `sendTime` y una flag de retransmision `retransmitted`.

Esto sirve para conservar paquetes que potencialmente puedan ser retransmitidos de ser necesario.

## `RenoManager`

Esta clase se encarga de administrar y almacenar los `timeouts` de los paquetes en la red.

```C++
class RenoManager {
private:
	int maxSize;
	int size;
	int msgSendingAmount;
	bool isSlowStartStage = true;
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
};
```

Administra los paquetes a través de un diccionario Int (Numero de secuencia) -> EventTimeout, la clase `EventTimeout` como `Volt` fueron generados por el template de Omnet++ de `cMessage`.

En nuesta implementación utilizamos **arranque lento** para control de flujo, representamos esa instancia con la flag `isSlowStartStage`.

Con `size` y `msgSendingAmount` se puede calcular la disponibilidad del diccionario.

# `RTT`

La implementación del RTT está en el modulo [`RTTManager.h`](../RTTManager.h), este nos permine manejar la estimación del RTT, RTO y la desviación estándar.

```cpp
class RTTManager {
private:
	  double stdDesviation;
	  double rtt;, por ello se
    double rto;
    bool isFirstAckReceived;
    void updateSmoothRTT(double rtMeasurement);
public:
    /* RTo = Retransmission Timeout */
	  double getCurrentRTo();
    void updateTimeoutRTo();
    /* rtMeasurement = Round Trip Measurement */
    void updateEstimation(double rtMeasurement);
    double getCurrentRTT();
```

- `isFirstAckReceived` lo utilizamos para saber si es necesario realizar la primera inicialización o realizar la actualización suavizada.
- Para calcular el `RTT` y `RTO` se utilizo el algoritmo de `Jacobson (1988)`.
  - Se inicializa el `RTT = 1` usando como referencia el consejo del estándar `RFC 6298`, la desviación estándar en 0 y `RTO = 1`.
- Se actualiza la estimación por cada ACK de paquete que no a sido retransmitido.
- Este modulo se utiliza principalmente para estimar el tiempo dinámico de timeout para los paquetes que se crean.

## Karn

Para calcular el RTT y RTO se utilizo el algoritmos de Jacobson.

Si bien nuestra intención original era seguir el algoritmo de Karn, por cuestiones de arquitectura accidentalmente caímos en un protocol que no lo hace *completamente*.

> Nota: Inicialmente cuando un paquete se asumía perdido, lo quitábamos de la **SW** (Sliding Window) temporalmente, y lo poníamos en la punta de la cola común de transmisión. De esa manera nos conservar la función de envio exactamente igual, lo cual simplificaba las cosas. Esta implementación terminó abandonandose ya que se descubrió un caso donde esto generaba problemas.
>
> Si uno estaba justo transmitiendo un paquete cuando ocurría el timeout, debía esperar un tiempo no trivial hasta poder hacer la retransmisión. Entonces entre ese tiempo, podría llegar un ACK del paquete por retransmitir, y como todavía no estaba de nuevo en la SW, generaba toda clase de problemas con la estimación de paquetes en vuelo, lo cual generaba un **deathlock**.
>
> Como inicialmente se quitaba de la ventana corrediza, no teníamos forma de saber si un paquete entrante era retransmitido o no, y necesitabamos actualizar el SRTT. La solución fue que el volt llevara esa información en sigo mismo y que el receptor se encargue de agregar esa flag acorde al volt que recibió.

Karn es necesario porque no sabemos si un ACK entrante de un paquete que fue retransmitido pertenece al paquete original o al retransmitido, y cualquiera de esas dos asunciones llevaría a ensuciar la estimación de SRTT. Pero en nuestro caso, como tenemos la **RET flag**, podemos saber cuales ACK provienen del paquete original y cuales no. Por lo que podemos **solo ignorar los ACKs siguientes al original**.

# Cosas no implementadas de TCP o TCP-Reno

## Reordenamiento de paquetes

El **receptor** asume que siempre recibe en orden.

## ACK duplicados

Descartamos **ACKs duplicados** porque dentro de nuestra red no sería algo que ocurra.

# Problemas que surgieron

## IDE

El principal incoveniente que nos surgió fue que nunca logramos que el IDE lance la simulación con el botón correspondiente, tuvimos que realizarlo a través del [`Makefile`](../Makefile), pero descubrimos que de esta forma lanza la simulación más rápido.

Por esta misma razón no pudimos utilizar la herramienta de debugeo que nos brinda el IDE, por lo que tuvimos que usar impresiones directas en consola para debugear. También esto explica porqué utilizamos el comando:

```bash
make clean && make run > /dev/null
```

Otro inconveniente respecto al IDE es que no podíamos que funcionara con la extensión Live Share de Visual Studio Code para pair programming, por lo que al querer realizar múltiples cambios de diferentes personas para probarlos en una única branch teníamos que estar actualizando cada computadora individualmente, resolviendo conflictos. Sino tenía que uno estar escribiendo mientras comparte pantalla y todos realizan esos cambios localmente sin comitear.

## Omnet y sus librerías

No podíamos abstraer completamente la [WC](#ventana-de-congestión) porque no podíamos importar subtipos de omnet++, a pesar que la misma documentación explicaba cómo hacerlo. En nuestro caso los `simtime_t` y los `cMessage`, no podíamos crear ambos de manera interna sin heredarlos.

## simtime_t

Tuvimos varias dificultades para implementar nuestro propio `simtime_t`, nombrando algunas:

- Al declararlo en el `.ini` no nos reconocía el tipo.
- Cuando el `.ini` reconocía el tipo, no la unidad.
- Cuando `.ini` reconocía la unidad el `.ned` y los módulos no lo reconocían.
- Cuando los módulos lo reconocían, al inicio no funcionaban los operadores para tiempo.

---

**[ENUNCIADO](../Markdown/Assignment.md) | [README](../README.md) | [ANÁLISIS](../Markdown/Analysis.md)**

---