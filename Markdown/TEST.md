> **Referencia:**
>
> *Itv:* Intervalo de Generación
>
> *Gen:* Paquetes Generados
>
> *Del:* Paquetes entregados
>
> *Drop:* Paquetes perdidos **en la cola intermedia**
>
> *AvDel:* Retraso de entrega promedio

----

FIXTHIS
1. ¿Cómo cree que se comporta su algoritmo de control de flujo y congestión ? ¿Funciona para el caso de estudio 1 y 2 por igual? ¿Por qué?

# Primer Diseño

- [x] Mensajes para ambos tipos de mensajes (datos y acks).
- [/] Parada y espera. *(Descartado)*
- [x] N° de seq.
- [x] ACK flag.
- [x] BufferSize.
- [x] Ventana de congestión v1.

# Segundo diseño

- [x] Ventana corrediza.
- [ ] Ventana de congestión v2.
- [x] Arranque lento.
- [/] Threshold. *(Descartado)*
- [ ] Retransmisión.
- [ ] Arranque lento al inicio y luego recuperación rápido.
- [x] Timer -> Timeout.

# Tercer diseño

- [ ] Control de flujo.

# Modificaciones

La ventana de congestión mide su disponibilidad en cantidad de bytes (no en cantidad de segmentos).

> .ini


Network.sender.senderAppLayer.packetByteSize = 12500

** nos dice que forma parte de todos como "global"
Network.**.packetByteSize = 12500

Reno Controller

- ~umbral~
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

# Mejoras posibles

Se asume que la cola del Sender es arbitrariamente grande, por lo que no nos concierne su espacio para las retransmisiones. Sino fuese el caso se podría hacer lo siguiente para reducirlo:
- setear un tamaño fijo en la cola para los paquetes de retransmisión. Este espacio no puede ser ocupado por lo paquetes normales.