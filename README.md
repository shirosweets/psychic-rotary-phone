# Laboratorio N°3: Capa de Transporte

Redes y Sistemas Distribuidos - Primer cuatrimestre, FAMAF - 2022.

# Integrantes
- Carrizo, Ernesto.
- Domínguez, Agustín.
- Vispo, Valentina.

# Objetivos

- Leer, comprender y generar modelos de red en Omnet++.
- Analizar tráfico de red bajo tasas de datos acotadas y tamaño de buffers limitados.
- Diseñar y proponer soluciones de control de congestión y flujo.

# Enunciado

El enunciado completo en Markdown se encuentra en [Enunciado.md](Enunciado.md).

# Simulación

# Experimentos y preguntas: análisis

Se deberá correr simulaciones paramétricas para cada caso de estudio, variando el intervalo de
generación de paquetes (`generationInterval`) entre 0.1 y 1 en los pasos que el grupo crea
adecuado para responder las preguntas planteadas.
Se deberá generar algunas gráficas representativas de la utilización de cada una de las 3
queue para los caso de estudios planteados.
Se sugiere crear una gráfica de carga transmitida (eje x) vs. carga recibida (eje y), ambas
expresadas en paquetes por segundo (ver Figura 6-19 del libro Tanenbaum). En caso de que
haya pérdidas de paquetes también se sugiere medir y comparar estadísticas de ese
fenómeno.

## Responda y justifique

1. ¿Qué diferencia observa entre el caso de estudio 1 y 2? ¿Cuál es la fuente limitante en
cada uno? Investigue sobre la diferencia entre control de flujo y control de congestión
(ver Figura 6-22 del libro Tanenbaum).

# Experimentos y preguntas: diseño
Utilice los mismos parámetros de los experimentos de la tarea 1, genere las curvas necesarias,
y responda:

1. ¿Cómo cree que se comporta su algoritmo de control de flujo y congestión ? ¿Funciona para el caso de estudio 1 y 2 por igual? ¿Por qué?

# Primer Diseño

- [x] Mensajes para ambos tipos de mensajes (datos y acks).
- [/] Parada y espera. *(Descartado)*
- [x] N° de seq.
- [x] ACK flag.
- [x] BufferSize.
- [ ] Ventana de congestión v1.

# Segundo diseño

- [ ] Ventana corrediza.
- [ ] Ventana de congestión v2.
- [ ] Arranque lento.
- [ ] Threshold.
- [ ] Retransmisión.
- [ ] Arranque lento al inicio y luego recuperación rápido.
- [ ] Timer -> Timeout.

# Tercer diseño

- [ ] Control de flujo.

# Modificaciones

La ventana de congestión mide su disponibilidad en cantidad de bytes (no en cantidad de segmentos).

> .ini


Network.sender.senderAppLayer.packetByteSize = 12500

** nos dice que forma parte de todos como "global"
Network.**.packetByteSize = 12500

Reno Controller

- umbral
- packetDropped
- stage

Congestion Window
- Que el emisor sepa distinguir [(state, timer), etc , ]

- seqNumber
- acks

---

# Realizar

Generar el evento rtt cuando se inicializa

Re sched del rtt cuando se recibe

Hacer el schedAt del timeout en la función handleSelfMsg

Arreglar el simtime_t del rtt

Implementar los métodos del CongestionController




# OMNet++

https://doc.omnetpp.org/omnetpp/manual/

https://stackoverflow.com/questions/52445993/omnet-on-windows-or-linux
https://stackoverflow.com/questions/7020069/make-library-not-found

## Imagen de docker con Omnet++ (opcional)

Instrucciones para descargar y ejecutar Omnet++ usando Docker.

https://github.com/mmodenesi/omnetpy

## cPacket
https://doc.omnetpp.org/omnetpp/api/classomnetpp_1_1cPacket.html

### Cómo crear class template de packet

1) Crear el archivo `testPacket.msg`

```cpp
packet testPacket
{
     int srcAddress;
     int destAddress;
     int remainingHops = 32;
};
```

2) Correr el comando

```bash
opp_msgc testPacket.msg
```

3) Se habrán generado los archivos `testPacket_m.h` y `testPacket_m.cc`

### Cómo saber de qué gate viene un mensaje

bool arrivedOn()

## 

`cancelEvent()`

---

**[Enunciado.md](Enunciado.md)**
