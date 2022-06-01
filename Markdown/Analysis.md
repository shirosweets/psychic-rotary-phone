# Control de Congestión y Flujo en la Capa de Transporte con OMNET++

Informe de la tarea de análisis del laboratorio 3 de Redes y Sistemas Distribuidos - Primer cuatrimestre, FAMaF 2022.

# Integrantes
- Carrizo, Ernesto.
- Domínguez, Agustín.
- Vispo, Valentina.

---

**[ENUNCIADO](../Markdown/Assignment.md) | [README](../README.md) | [DISEÑO](../Markdown/Design.md)**

---

## Contenido

- [Análisis de la red sin control de congestión ni flujo](#análisis-de-la-red-sin-control-de-congestión-ni-flujo)
  - [Introducción a la red básica](#introducción-a-la-red-básica)
  - [Introducción a la red con TLCP](#introducción-a-la-red-con-tlcp)
  - [Presentación del Caso I](#presentación-del-caso-i)
  - [Presentación del Caso II](#presentación-del-caso-ii)
  - [> `RecAppLayer`: Capa de aplicación del receptor](#-recapplayer-capa-de-aplicación-del-receptor)
  - [Caso I](#caso-i)
    - [Hipótesis](#hipótesis)
    - [Mediciones](#mediciones)
    - [Análisis de los resultados Caso I](#análisis-de-los-resultados-caso-i)
  - [Caso II](#caso-ii)
    - [Hipótesis](#hipótesis-1)
    - [Mediciones](#mediciones-1)
    - [Análisis de los resultados Caso II](#análisis-de-los-resultados-caso-ii)
  - [Conclusión parcial](#conclusión-parcial)
    - [¿Qué diferencia observa entre el caso de estudio I y II? ¿Cuál es la fuente limitante en cada uno?](#qué-diferencia-observa-entre-el-caso-de-estudio-i-y-ii-cuál-es-la-fuente-limitante-en-cada-uno)
    - [General](#general)
    - [Diferencia entre control de flujo y control de congestión](#diferencia-entre-control-de-flujo-y-control-de-congestión)
- [Análisis de la red con *TLCP*](#análisis-de-la-red-con-tlcp)
  - [Pantallazo a *Transport Limited Control Protocol*](#pantallazo-a-transport-limited-control-protocol)
  - [TLCP Caso I](#tlcp-caso-i)
    - [Hipótesis](#hipótesis-2)
    - [Mediciones](#mediciones-2)
    - [Análisis de los resultados TLCP Caso I](#análisis-de-los-resultados-tlcp-caso-i)
  - [TLCP Caso II](#tlcp-caso-ii)
    - [Hipótesis](#hipótesis-3)
    - [Mediciones](#mediciones-3)
    - [Análisis de los resultados TLCP Caso II](#análisis-de-los-resultados-tlcp-caso-ii)
  - [Comparación con la red previa](#comparación-con-la-red-previa)
    - [**¿Cómo creen que se comporta su algoritmo de control de flujo y congestión?**](#cómo-creen-que-se-comporta-su-algoritmo-de-control-de-flujo-y-congestión)
    - [**¿Funciona para el caso de estudio I y II por igual? ¿Por qué?**](#funciona-para-el-caso-de-estudio-i-y-ii-por-igual-por-qué)
- [Conclusión](#conclusión)
- [Mejoras posibles](#mejoras-posibles)
  - [TCP VEGAS](#tcp-vegas)
- [Referencias](#referencias)

# Abstract

En este laboratorio se estudia el comportamiento de las redes frente a problemas de congestión y flujo con la herramienta de simulación por eventos discretos *OMNETT++*. Se tomó una red simplificada y se realizó un análisis con el objetivo de identificar de qué escenario se trataba, dónde se produce el cuello de botella y en qué módulo ocurre. Un escenario presenta problemas por congestión y otro por problemas de flujo.

Posteriormente se realizó un diseño e implementación de control de ambos problemas y se estudió la efectividad en dichos escenarios.

# Análisis de la red sin control de congestión ni flujo

## Introducción a la red básica

La red simplificada tiene un `sender` y un `receiver`, y un nodo intermedio `queue` que simboliza la subred entre el transmisor y receptor.

![Red Básica](../documents/assets/case_1_network.png)

- El **emisor** genera paquetes de *12500 bytes* bajo una distribucción exponencial centrada en un tiempo **`T`** (s) y tanto la `queue` intermedia como el receptor tienen un buffer interno para manejar paquetes entrantes.

- El **receptor** tiene un *datarate* interno para su `Sink`. El Sink modela lo que sería la capa de aplicación del lado del receptor, y este datarate existe para simular problemas de flujo propios del receptor.

Por razones de conveniencia con respecto a la segunda parte de este análisis, se modificó la simulación original en los siguientes aspectos:

 * El tamaño del buffer del nodo intermedio `queue` se cambio de **200** a **100** paquetes.
    > Esto para facilitar las mediciones del control de congestión.
 * El largo de la simulación se aumentó de **200 segundos** a **300 segundos**
    > Esto para normalizar aun más los datos estadísticos provenientes de la generación azarosa de paquetes.

## Introducción a la red con TLCP

La implementación de nuestro algoritmo requiere un canal de vuelta entre el receptor y emisor, por lo que se agregó ese canal con las mismas características (`datarate` y `delay`) del **nodo intermedio**. También en cada caso los parámetros de los enlaces de envío al receptor son los mismos que en la red básica para que sus mediciones sean comparables.

![TLCP Red Básica](../documents/assets/TLCP_basic_network.png)

## Presentación del Caso I

| Conexión                           | Datarate      |
|------------------------------------|---------------|
| entre `TransRx` y `RecAppLayer`    | **`0.5Mbps`** |
| entre `emisor` y `nodo intermedio` | **`1.0Mbps`** |

## Presentación del Caso II

| Conexión                           | Datarate      |
|------------------------------------|---------------|
| entre `TransRx` y `RecAppLayer`    | **`1.0Mbps`** |
| entre `emisor` y `nodo intermedio` | **`0.5Mbps`** |

---
> `TransRx` : Capa de transporte del Receptor
>
> `RecAppLayer`: Capa de aplicación del receptor
---

## Caso I

### Hipótesis

Con esos datos podemos ver a simple vista que existe un **cuello de botella** entre las dos capas de aplicación que se encuentra en el receptor mismo.

El *receptor* va a recibir mensajes más rápidos de lo que puede procesarlos, eventualmente generando **droppeos** de paquetes.

Esto es un ejemplo de problemas de *flujo*.

### Mediciones

Se análizaron varias mediciones entontrando los límites de la red, desde donde la red no experimenta ningún tipo de pérdida de paquetes ni retrasos hasta el punto donde la red alcanza su límite natural. De este rango se tomaron datos para análisis.

Se realizó también una tabla sobre la transmisión basándonos en cambiar la media de intervalo de creación de paquetes.

Primero se tomó un intervalo de generación relativamente grande (**`T = 2s`**). Si bien el análisis teórico del problema dice que el receptor tiene un problema de flujo, si se ocupa poco la red no debería haber pédida de paquetes o una mínima pérdida.

Caso I tabla

| Itv      | Gen      | Del      | Drop Q | Drop R   | AvDel [s] |
|----------|----------|----------|--------|----------|-----------|
| **2.0**  | **144**  | **144**  | **0**  | **0**    | **0.40**  |
| 0.8      | 367      | 367      | 0      | 0        | 0.43      |
| 0.4      | 754      | 754      | 0      | 0        | 0.49      |
| 0.3      | 989      | 989      | 0      | 0        | 0.58      |
| 0.25     | 1190     | 1187     | 0      | 0        | 0.72      |
| 0.23     | 1280     | 1279     | 0      | 0        | 0.94      |
| 0.22     | 1344     | 1344     | 0      | 0        | 1.11      |
| 0.21     | 1418     | 1401     | 0      | 0        | 1.49      |
| 0.2      | 1494     | 1469     | 0      | 0        | 2.41      |
| **0.18** | **1664** | **1494** | **0**  | **0**    | **14.52** |
| 0.175    | 1710     | 1496     | 0      | 15       | 18.07     |
| 0.17     | 1763     | 1497     | 0      | 65       | 21.54     |
| 0.16     | 1867     | 1497     | 0      | 166      | 26.45     |
| 0.15     | 1979     | 1498     | 0      | 278      | 29.16     |
| 0.14     | 2136     | 1498     | 0      | 438      | 30.99     |
| **0.1**  | **2933** | **1498** | **0**  | **1231** | **35.83** |

> **Referencia**
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

### Análisis de los resultados Caso I

- Se puede notar que hasta el rango de aproximadamente `0.25` de intervalo la red no tiene ningún tipo de retraso. Las colas están prácticamente todo el tiempo vacías.
- Luego de eso hasta el rango `0.18` las colas comienzan a llenarse pero todavía no se pierden paquetes. El problema comienza a vislumbrarse, lo cual se manifiesta en que retraso promedio que suba en 2 órdenes de magnitud (de `0.4` -> `14.52`). Aún así, no hay pérdida de paquetes, como se puede vislumbrar en el siguiente gráfico

**Generación de paquetes con distribución exponencial de media = 0.18s**

![Caso I Buffer Size intv = 0.18](/documents/assets/case_I_intv_018/case_I_intv_018_bufferStacked.png)

- Total de paquetes generados y almacenados en el buffer del emisor: `1664` (`166.4. Mb`)
- Total de paquetes recibidos y almacenados en el buffer de la subred: `1494` (`149.4 Mb`)
- Total de paquetes recibidos y almacenados en el buffer del receptor: `1494` (`149.4 Mb`)

De ahí en más aumentar la generación de paquetes finalmente genera la pérdida de los mismos además de más aumento en el retraso.

Se puede notar que la carga efectiva que de ahí en más el receptor puede recibir es de `1498` y se mantiene en esa cifra.

Con estos datos, se realizaron los siguientes gráficos:

**Generación de paquetes con distribución exponencial de media = 0.1s**

<!-- - 1 paquete cada 10pps -->

![Caso I Buffer Size intv = 0.1](/documents/assets/case_I/case_I_bufferStacked.png)

- Total de paquetes generados y almacenados en el buffer del emisor: `2933` (`293.3 Mb`)
- Total de paquetes recibidos y almacenados en el buffer de la subred: `1498` (`149.8 Mb`)
- Total de paquetes recibidos y almacenados en el buffer del receptor: `1498` (`149.8 Mb`)

<!-- **Generación de paquetes con distribución exponencial de media = 2.0s**
![Caso I Buffer Size intv = 2.0](/documents/assets/case_I_intv_20/case_I_intv_2_0_bufferStacked.png)

- Total de paquetes generados y almacenados en el buffer del emisor: `144` (`14.4 Mb`)
- Total de paquetes recibidos y almacenados en el buffer de la subred: `144` (`14.4 Mb`)
- Total de paquetes recibidos y almacenados en el buffer del receptor: `144` (`14.4 Mb`) -->

---

## Caso II

### Hipótesis

En este caso se percibe que la velocidad de generación es más rápida que la velocidad de las cola en recibir y enviar al paquete al Sink. Igual que el caso I exceptuando que el cuello de botella se encuentra en la subred (dataQueue).

De esta manera la cola paulativamente se llenará y empezará a dropear paquetes.

Este es un claro problema de congestión, la interred no puede manejar la velocidad del generador y al no haber caminos alternativos siempre habrá problemas de congestión.

Las mediciones en este caso son análogas al anterior:

### Mediciones

Caso II tabla

| Itv      | Gen      | Del      | Drop Q   | Drop R | AvDel [s] |
|----------|----------|----------|----------|--------|-----------|
| **2.0**  | **144**  | **144**  | **0**    | **0**  | **0.40**  |
| 0.8      | 367      | 367      | 0        | 0      | 0.43      |
| 0.4      | 754      | 754      | 0        | 0      | 0.49      |
| 0.3      | 989      | 989      | 0        | 0      | 0.58      |
| 0.25     | 1190     | 1187     | 0        | 0      | 0.72      |
| 0.23     | 1280     | 1279     | 0        | 0      | 0.94      |
| 0.22     | 1344     | 1334     | 0        | 0      | 1.11      |
| 0.21     | 1418     | 1401     | 0        | 0      | 1.49      |
| 0.2      | 1494     | 1469     | 0        | 0      | 2.41      |
| **0.18** | **1664** | **1494** | **67**   | **0**  | **13.25** |
| 0.175    | 1710     | 1496     | 115      | 0      | 14.41     |
| 0.17     | 1763     | 1497     | 165      | 0      | 15.25     |
| 0.16     | 1867     | 1497     | 266      | 0      | 16.59     |
| 0.15     | 1979     | 1498     | 379      | 0      | 17.40     |
| 0.14     | 2136     | 1498     | 538      | 0      | 17.96     |
| **0.1**  | **2933** | **1498** | **1332** | **0**  | **19.98** |

### Análisis de los resultados Caso II

- **Drop Q**: En este caso el datarate entre la cola intermedia y la capa del deceptor se divide a la mitad (de 1Mbps a 0.5Mbps), se mantiene el datarate del generador con la cola intermedia (1Mbps) lo que provoca el llenado de la cola intermedia, generando paquetes dropeados.

En cuestión de progresión del cuello de botella notamos una velocidad similar (comienza a congestionarse la red y perder paquetes relativamente al mismo timepo). Cabe mencionar que como el tamaño del buffer intermedio es más chico que el del receptor del caso anterior (es decir donde ocurre el cuello de botella), la pérdida de paquetes que sufre es mayor y más temprana. Fuera de esto, el cuello de botella es muy similar.

- **Capa de receiver**: En este caso no hay dropeo en la cola del receptor debido a que el datarate es el doble de rápido y por este mismo motivo el delay baja.

A partir de los datos tomados se graficaron las siguientes métricas:

**Generación de paquetes con distribución exponencial de media = 0.1s**

En el siguiente gráfico podemos apreciar

![Caso II Buffer Size intv = 0.1](/documents/assets/case_II/case_II_bufferStacked.png)

- Total de paquetes generados y almacenados en el buffer del emisor: `2933` (`293.3 Mb`)
- Total de paquetes recibidos y almacenados en el buffer de la subred: `1498` (`149.8 Mb`)
- Total de paquetes recibidos y almacenados en el buffer del receptor: `1498` (`149.8 Mb`)

**Generación de paquetes con distribución exponencial de media = 0.18s**

![Caso II Buffer Size intv = 0.18](/documents/assets/case_II_intv_018/case_II_intv_018_bufferStacked.png)

- Total de paquetes generados y almacenados en el buffer del emisor: `1664` (`166.4 Mb`)
- Total de paquetes recibidos y almacenados en el buffer de la subred: `1494` (`149.4 Mb`)
- Total de paquetes recibidos y almacenados en el buffer del receptor: `1494` (`149.4 Mb`)

<!-- **Generación de paquetes con distribución exponencial de media = 2.0s**

![Caso II Buffer Size intv = 2.0](/documents/assets/case_II_intv_20/case_I_intv_2_0_bufferStacked.png)

- Total de paquetes generados y almacenados en el buffer del emisor: `144` (`14.4 Mb`)
- Total de paquetes recibidos y almacenados en el buffer de la subred: `144` (`14.4 Mb`)
- Total de paquetes recibidos y almacenados en el buffer del receptor: `144` (`14.4 Mb`) -->

## Conclusión parcial
### ¿Qué diferencia observa entre el caso de estudio I y II? ¿Cuál es la fuente limitante en cada uno?

Comparando los dos casos podemos ver lo siguiente:

- El cuello de botella sigue existiendo de manera similar, ambas redes tienen la misma carga útil máxima (1498 paquetes), lo que cambia es el lugar donde ocurre el cuello de botella. Como había sido supuesto.
- El caso I es un problema de flujo mientras que el caso II es un problema de congestión.
- Por la diferente de tamaño de buffer (la subred tiene menos buffer que el receptor) el colapso del caso II ocurre ligeramente antes.
- La fuente limitante en cada caso es la diferencia de datarate en las conexiones.

### General

El principal problema que tiene la red es que en ambos casos es que existe un cuello de botella:
- En el primer caso es la línea interna de la capa del receptor que conecta el sink con su cola.
- En el segundo caso es la línea que conecta la capa del receptor con la cola intermeedia.

Respectivamente las colas dropean paquetes en el momento que se llenan.

Posibles soluciones para dejar de perder paquetes:
- Aumentar el data rate de dichos canales en cada caso.
- Retransmitir los paquetes que se pierden.
- Implementar control de congestión para evitar perder paquetes.

### Diferencia entre control de flujo y control de congestión

| Nombre                | Ubicación del problema                         | Impide                                                   |
|-----------------------|------------------------------------------------|----------------------------------------------------------|
| Control de Congestión | red entre medio del emisor y receptor (subred) | Que un conjunto de transmisiones sobrecarguen la red     |
| Control de Flujo      | entre emisor y receptor                        | Que un transmisor rápido sobrecargue a un receptor lento |

----
# Análisis de la red con *TLCP*

## Pantallazo a *Transport Limited Control Protocol*

**Transport Limited Control Protocol** (TLCP) implementa *control de flujo* y *control de congestión*, con un receptor manteniendo un feedback constante al emisor mediante paquetes de confirmación (ACK).

El tamaño del header de cada paquete, llamado `Volt`, tiene tan solo **9** bytes, lo cual agrega muy poco overhead a cada paquete.

Para una explicación de las especificaciones e implementación de TLCP dirigirse a [**DISEÑO**](Design.md).

Los detalles de la simulación son idénticos para cada caso de la red anterior sin control de flujo ni control, por lo que saltaremos directamente a la hipótesis.

# TLCP Caso I

### Hipótesis

Como en este caso el cuello de botella ocurre al nivel host, tenemos acceso directo al estado del buffer y TLCP comunica este dato al emisor que puede ajustar su tranmisión de manera acorde. Dado esto, si bien seguimos teniendo un cuello de botella, esperamos resolver el problema de flujo de manera satisfactoria. TLCP utilizaría la conexión al máximo posible sin pérdida de paquetes.

### Mediciones

Para TLCP tenemos acceso a más métricas que agregamos al análisis:
- **Ret**: Cantidad de paquetes que fueron retransmitidos.
- **RTT**: Promedio del tiempo que tarda en llegar un ACK para cada paquete.
- **ACKt**: Promedio del tiempo que tarda en llegar un ACK desde que se creó un paquete.

TLCP Caso I Tabla

| Itv      | Gen      | Del      | Drop Q | Drop R | AvDel [s]  | RTT [s]     | ACKt (+-stdev) [s] | Ret   |
|----------|----------|----------|--------|--------|------------|-------------|--------------------|-------|
| **2.0**  | **144**  | **144**  | **0**  | **0**  | **0.40**   | **0.2 (0)** | **0.2** (0)        | **0** |
| 0.8      | 367      | 367      | 0      | 0      | 0.43       | 0.2 (0)     | 0.2  (0)           | 0     |
| 0.4      | 754      | 754      | 0      | 0      | 0.49       | 0.2 (0)     | 0.21 (0)           | 0     |
| 0.3      | 989      | 989      | 0      | 0      | 0.58       | 0.2 (0)     | 0.22 (0)           | 0     |
| 0.25     | 1190     | 1187     | 0      | 0      | 0.72       | 0.2 (0)     | 0.23 (0)           | 0     |
| 0.23     | 1280     | 1279     | 0      | 0      | 0.94       | 0.2 (0)     | 0.23 (0)           | 0     |
| 0.22     | 1344     | 1334     | 0      | 0      | 1.11       | 0.2 (0)     | 0.23 (0)           | 0     |
| 0.21     | 1418     | 1401     | 0      | 0      | 1.49       | 0.2 (0)     | 0.24 (0.05)        | 0     |
| 0.2      | 1494     | 1469     | 0      | 0      | 2.41       | 0.2 (0)     | 0.25 (0.05)        | 0     |
| **0.18** | **1664** | **1494** | **0**  | **0**  | **14.52**  | **0.2 (0)** | **0.26 (0.1)**     | **0** |
| 0.175    | 1710     | 1496     | 0      | 0      | 18.07      | 0.2 (0)     | 0.33 (0.4)         | 0     |
| 0.17     | 1763     | 1497     | 0      | 0      | 21.68      | 0.2 (0)     | 1.18 (2.3)         | 0     |
| 0.16     | 1867     | 1497     | 0      | 0      | 29.15      | 0.2 (0)     | 5.35 (8.0)         | 0     |
| 0.15     | 1979     | 1498     | 0      | 0      | 36.64      | 0.2 (0)     | 11.4 (3.9)         | 0     |
| 0.14     | 2136     | 1498     | 0      | 0      | 44.17      | 0.2 (0)     | 18 (13.9)          | 0     |
| **0.1**  | **2933** | **1498** | **0**  | **0**  | **74.41**  | **0.2 (0)** | **49 (19)**        | **0** |
| **0.05** | **5847** | **1498** | **0**  | **0**  | **112.27** | **0.2 (0)** | **92 (66)**        | **0** |


### Análisis de los resultados TLCP Caso I

Como se esperaba, TLCP maneja de manera óptima, incluso en el caso más estresante de la red, la cola del receptor se llena hasta casi el punto de colapso, pero entonces el receptor ajusta su datarate y manda solamente paquetes cuando el receptor le avisa que puede procesar más.

Es decir que el receptor tiene siempre acceso a la mayor cantidad posible de data y cada vez que procesa otro paquete, recibe otro más.

Se debe mencionar que no hay droppeo de paquetes, y el RTT medido siempre se mantiene fijo y muy bajo (0.2 s), esto es gracias a la interred no congestionada.

Otra mención imporante es el delay, que es bastante mayor al caso I sin control. Esta comparación se hará en el análisis íntegro.

**Generación de paquetes con distribución exponencial de media = 0.05s**

![TLCP Caso I Buffer Size intv = 0.05](/documents/assets/tlcp_case_I_intv_005/tlcp_case_I_intv_0_05_bufferSize_Stacked.png)

- Total de paquetes generados y almacenados en el buffer del emisor: `5847` (`149.8 Mb`)
- Total de paquetes recibidos y almacenados en el buffer de la subred: `1498` (`149.8 Mb`)
- Total de paquetes recibidos y almacenados en el buffer del receptor: `1498` (`149.8 Mb`)

<!-- **Generación de paquetes con distribución exponencial de media = 0.1s**

![TLCP Caso I Buffer Size intv = 0.1](/documents/assets/tlcp_case_I_intv_01/tlcp_case_I_intv_0_1_bufferSize_Stacked.png)

- Total de paquetes generados y almacenados en el buffer del emisor: `2933` (`293.3 Mb`)
- Total de paquetes recibidos y almacenados en el buffer de la subred: `1498` (`149.8 Mb`)
- Total de paquetes recibidos y almacenados en el buffer del receptor: `1498` (`149.8 Mb`)

**Generación de paquetes con distribución exponencial de media = 0.18s**

![TLCP Caso I Buffer Size intv = 0.18](/documents/assets/tlcp_case_I_intv_018/tlcp_case_I_intv_0_18_bufferSize_Stacked.png)

- Total de paquetes generados y almacenados en el buffer del emisor: `1664` (`166.4 Mb`)
- Total de paquetes recibidos y almacenados en el buffer de la subred: `1494` (`149.4 Mb`)
- Total de paquetes recibidos y almacenados en el buffer del receptor: `1494` (`149.4 Mb`) -->

**Generación de paquetes con distribución exponencial de media = 2.0s**

![TLCP Caso I Buffer Size intv = 2.0](/documents/assets/tlcp_case_I_intv_02/tlcp_case_I_intv_2_0_bufferSize_Stacked.png)

- Total de paquetes generados y almacenados en el buffer del emisor: `144` (`14.4 Mb`)
- Total de paquetes recibidos y almacenados en el buffer de la subred: `144` (`14.4 Mb`)
- Total de paquetes recibidos y almacenados en el buffer del receptor: `144` (`14.4 Mb`)

## TLCP Caso II

| Conexión                           | Datarate      |
|------------------------------------|---------------|
| entre `TransRx` y `RecAppLayer`    | **`1.0Mbps`** |
| entre `emisor` y `nodo intermedio` | **`0.5Mbps`** |

### Hipótesis

Como no tenemos acceso directo a la subred, TLCP debe tomar una estimación de la congestión y ajustar su datarate en base a eso. Por esta razón, se estima que la solución no va a ser tan óptima como en el caso I, y vaya a haber menos carga útil que en el caso II sin control. Sin embargo, también se espera que no haya pérdida de paquetes, aunque sí puede haber retransmisiones innecesarias causadas por timeouts.

El RTT debería ser mucho menos estable que en [TLCP Caso I](#tlcp-caso-i), pero aún así no debería crecer demasiado.

Otra cosa que se espera que ocurra con TLCP es la llegada completa de la transmisión, en orden.

### Mediciones

TLCP Caso II Tabla

| Itv      | Gen      | Del      | Drop Q | Drop R | AvDel [s] | RTT (+-stdev) [s] | ACKt (+-stdev) [s] | Ret   |
|----------|----------|----------|--------|--------|-----------|-------------------|--------------------|-------|
| **2.0**  | **144**  | **144**  | **0**  | **0**  | **0.42**  | **0.3 (0.02)**    | **0.3 (0.03)**     | **8** |
| 0.8      | 367      | 367      | 0      | 0      | 0.46      | 0.32 (0.05)       | 0.33 (0.08)        | 20    |
| 0.4      | 754      | 754      | 0      | 0      | 0.55      | 0.39 (0.15)       | 0.41 (0.1 41)       | 7    |
| 0.3      | 989      | 989      | 0      | 0      | 0.67      | 0.48 (0.24)       | 0.53 (0.3)         | 47    |
| 0.25     | 1190     | 1185     | 0      | 0      | 0.89      | 0.66 (0.3)        | 0.76 (0.4)         | 41    |
| 0.23     | 1280     | 1277     | 0      | 0      | 1.32      | 0.86 (0.5)        | 1.02 (0.92)        | 33    |
| 0.22     | 1344     | 1334     | 0      | 0      | 2.0       | 0.94 (0.6)        | 1.28 (0.92)        | 32    |
| 0.21     | 1418     | 1400     | 0      | 0      | 2.41      | 1.38 (0.84)       | 2.62 (1.76)        | 11    |
| 0.2      | 1494     | 1463     | 0      | 0      | 3.37      | 1.54 (0.31)       | 3.2 (1.64)         | 3     |
| **0.18** | **1664** | **1406** | **0**  | **0**  | **23.3**  | **1.89 (0.92)**   | **23.25 (13.63)**  | **2** |
| 0.175    | 1710     | 1421     | 0      | 0      | 26.14     | 1.85 (0.91)       | 26.0 (14)          | 0     |
| 0.17     | 1763     | 1438     | 0      | 0      | 26.18     | 1.83 (0.9)        | 26.1 (15)          | 0     |
| 0.16     | 1867     | 1405     | 0      | 0      | 37.8      | 1.9 (0.93)        | 37 (22)            | 2     |
| 0.15     | 1979     | 1438     | 0      | 0      | 42.26     | 1.7 (0.8)         | 42 (24)            | 1     |
| 0.14     | 2136     | 1473     | 0      | 0      | 45.9      | 1.6 (0.8)         | 45 (26)            | 1     |
| **0.1**  | **2933** | **1457** | **0**  | **0**  | **78.4**  | **1.9 (0.8)**     | **78 (44)**        | **1** |
| **0.05** | **5847** | **1480** | **0**  | **0**  | **118.3** | **1.88 (0.8)**    | **114 (64)**       | **0** |


### Análisis de los resultados TLCP Caso II

Se percibe un aumento del delay medio de los diferentes casos de `intervalo de generación` debido a la implementación de control de congestión.

Como los paquetes quedan en la red más tiempo pero en consecuencia no se pierden, aunmentan el promedio del delay, que en la red sin control simplemente no llegaban a medirse.

![Retraso](/documents/comparative_algorithms/delay_all.png)

En cuantro a la productividad de la red misma, primero notamos que en casos de poca carga, la red se comporta correctamente, aunque el nivel de retranmisiones es más alto del esperado.

Aún así, llegan todos los paquetes esperados:

**Generación de paquetes con distribución exponencial de media = 2.0s**

![TLCP Caso II Buffer Size intv = 2.0](/documents/assets/tlcp_case_II_intv_20/tlcp_case_II_intv_2_0_bufferSize_Stacked.png)

- Total de paquetes generados y almacenados en el buffer del emisor: `144` (`14.4 Mb`)
- Total de paquetes recibidos y almacenados en el buffer de la subred: `144` (`14.4 Mb`)
- Total de paquetes recibidos y almacenados en el buffer del receptor: `144` (`14.4 Mb`)

En el caso de `Intv = 0.18`, podemos ver que la red ajusta varias veces su datarate, aumentando hasta cierto umbral, en este caso 15 paquetes en el buffer de la subred, y TLCP detecta el inicio de congestión y temporalmente baja su datarate. De esta manera la red nunca termina congestionandose.

Viendo el gráfico de buffers, podemos notar la forma de serrucho que tanto caracteriza a Reno

**Generación de paquetes con distribución exponencial de media = 0.18s**

Notar que el buffer del receptor varía de `0` a `1`.

![TLCP Caso II Buffer Size intv = 0.18](/documents/assets/tlcp_case_II_intv_018/tlcp_case_II_intv_0_18_bufferSize_Stacked.png)

- Total de paquetes generados y almacenados en el buffer del emisor: `1664` (`166.4 Mb`)
- Total de paquetes recibidos y almacenados en el buffer de la subred: `1406` (`140.6 Mb`)
- Total de paquetes recibidos y almacenados en el buffer del receptor: `1406` (`140.6 Mb`)

En intervalos de generación más alto la red también va ajustando el datarate:

**Generación de paquetes con distribución exponencial de media = 0.1s**

Notar que el buffer del receptor varía de `0` a `1`.

![TLCP Caso II Buffer Size intv = 0.05](/documents/assets/tlcp_case_II_intv_005/tlcp_case_II_intv_0_05_bufferSize_Stacked.png)

- Total de paquetes generados y almacenados en el buffer del emisor: `5847` (`584.7 Mb`)
- Total de paquetes recibidos y almacenados en el buffer de la subred: `1480` (`148.0 Mb`)
- Total de paquetes recibidos y almacenados en el buffer del receptor: `1480` (`148.0 Mb`)

**Generación de paquetes con distribución exponencial de media = 0.1s**

Notar que el buffer del receptor varía de `0` a `1`.

![TLCP Caso II Buffer Size intv = 0.1](/documents/assets/tlcp_case_II_intv_01/tlcp_case_II_intv_0_1_bufferSize_Stacked.png)

- Total de paquetes generados y almacenados en el buffer del emisor: `2933` (`293.3 Mb`)
- Total de paquetes recibidos y almacenados en el buffer de la subred: `1457` (`145.7 Mb`)
- Total de paquetes recibidos y almacenados en el buffer del receptor: `1457` (`145.7 Mb`)


## Comparación con la red previa

Lo primero que existe en comparación con la otra red, es que **TLCP nunca pierde paquetes**. Como se ve en los anteriores gráficos, los paquetes generados que la red no puede manejar se mantienen en el emisor sin congestionar a la misma.

![Paquetes perdidos](/documents/comparative_algorithms/loss_all.png)

### **¿Cómo creen que se comporta su algoritmo de control de flujo y congestión?**

Se observó que se comporta como era esperado:

- No pierde paquetes.
- No pierde mucho performance.

En cuanto a control de flujo, el algoritmo tiene la misma performance que la red anterior, solo que sin droppear paquetes, mientras que el caso de congestión si tiene menos carga útil. Acá podemos ver el gráfico de carga util:

![Carga ofrecida vs Carga Util](/documents/comparative_algorithms/ofrecida_vs_util_all.png)

Una característica rara que notamos es que TLCP en el caso iI mientras más carga tiene menos retransmisiones hace. El peak de retransmisiones ocurre con un intv bastante alto.

![TLCP Retransmitidos](/documents/comparative_tlcp_alogithm/retransmissions_tlcp.png)

En cuanto al RTT, en el **caso I permanece fijo en el mímimo de la red**, ya que la subred no tiene cuello de botella, mientras que el caso II si bien crece, se estabiliza por el valor `~1.8`, lo cual habla del control de congestion que posee TLCP.

![TLCP RTT](/documents/comparative_tlcp_alogithm/rtt_tlcp.png)

El ACK Time sigue un trayecto muy similar al delay promedio
![TLCP Ack Time](/documents/comparative_tlcp_alogithm/ack_time_tlcp.png)

### **¿Funciona para el caso de estudio I y II por igual? ¿Por qué?**

En el caso I, la red funciona de manera óptima, se nota la mejora con la implementación. El emisor manda exactamente lo que el receptor puede soportar/manejar.

En cambio, para el caso II con nuestra implementación consigue una mejora considerable teniendo en cuenta que no pierde paquetes, pero no óptima ya que no usa la red en su totalidad.

# Mejoras posibles

- Se asume que la cola del Sender es arbitrariamente grande, por lo que no nos concierne su espacio para las retransmisiones. Sino fuese el caso se podría hacer lo siguiente para reducirlo:
   - setear un tamaño fijo en la cola para los paquetes de retransmisión. Este espacio no puede ser ocupado por lo paquetes normales.

- Descartamos **ACKs duplicados** porque dentro de nuestra red no sería algo que ocurra, pero de implementarse se debería:
  - modificar también del lado del receptor.
  - modicar `congestionController.addAck(seqN)`, puede flag o bool.

- Reordenamiento.

- Reducir los llamados innecesarios a `dupVolt()` ya que este genera una copia del Volt.

- Modificar el `1000` del siguiente código de manera dinámica:
  ```cpp
  congestionController.setBaseWindow((currentBaseOfSlidingWindow + 1) % 1000);
  ```

- Implementar TCP VEGAS.

## TCP VEGAS
**TCP VEGAS** propone mecanismos para evitar la congestión que difieren de aquellos usados por TCP SACK y RENO. Mientras que TCP RENO requiere tener la pérdida de paquetes para censar el ancho de banda disponible, TCP VEGAS tiene como objetivo detectar la congestión en sus etapas iniciales y reducir su tasa de envío de datos (throughput) para prevenir que haya pérdida de paquetes.

Una de las ventajas de VEGAS es que tiene un comportamiento de inicio lento diferente con respecto a los otros TCP. Para lograrlo VEGAS revisa a diff (Número estimado de paquetes transitando por la red) cada vez que hay un RTT (round trip time).

En el modo de inicio lento el tamaño de la ventana es doblado cada vez que se recibe un RTT para ver el retardo de los paquetes con cada tamaño de ventana. Este retarde es usado para calcular diff y decidir si continuar en el modo de inicio lento. Específicamente si diff supera el valor del umbral γ=(α+ß)/2, (en donde α y ß representan umbrales predeterminados para la longitud de la ventana), VEGAS incrementa el tamaño de la ventana por un paquete [*Referencia 3*].

![TCP VEGAS - EC 3](/documents/assets/tcp_vegas/ec_0_tcp_vegas.jpg)

Como en TCP RENO tres reconocimientos duplicados resultan en la transmisión de paquetes, pero los temporizadores implementados ayudan a detectan la pérdida de paquetes antes, haciendo necesario solamente uno o dos reconocimientos.

Cuando hay una retransmisión debido a un reconocimiento duplicado, el tamaño de la ventana se reduce a 3w/4 en comparación a la reducción de RENO que es de w/2.

**Modelo analítico**

Para que VEGAS detecte congestión, se hace el cálculo de diff (*ecuación (6)*) cada vez que hay un RTT; para ello se usa el tamaño de la ventana actual W, el último RTT, y el RTT más pequeño observado hasta el momento.

![TCP VEGAS - EC 6](/documents/assets/tcp_vegas/ec_1_tcp_vegas.jpg)

El objetivo del algoritmo de control de congestión de VEGAS es mantener a diff dentro de un rango predefinido por dos límites; α y β, de esa maneta cada vez que hay un RTT nuevo y si no está en el modo de inicio lento, VEGAS ajustan el tamaño de su ventana mediante la *ecuación (7)* [*Referencia 5*].

![TCP VEGAS - EC 7](/documents/assets/tcp_vegas/ec_2_tcp_vegas.jpg)

# Referencias

- [1] Tanenbaum A. S. (2012). Redes de computadores (5ta ed.) Pearson.
- [2] Paxson & Allman (2000, Noviembre) Computing TCP's Retransmission Timer - RFC 2988 [https://datatracker.ietf.org/doc/html/rfc2988](https://datatracker.ietf.org/doc/html/rfc2988)
- [3] A. Wierman, T. Osogami, J. Olsén; A unified framework for modeling TCP-Vegas, TCP-SACK, and TCP-Reno. s.l. : School of Computer Science Carnegie Mellon University Pittsburgh, PA 15213,2003.
- [4] OMNeT Simulation Library: OMNeT API Reference, [doc.omnetpp.org/omnetpp/api/index.html](https://doc.omnetpp.org/omnetpp/api/index.html)
- [5] B. Thomas; Comparison of TCP Reno and TCP Vegas via fluid approximation. s.l. : Institut National de Recherche En Informatique Et En Automatique, 1998.
- [6] C. Samios, M. Vernon; Modeling the throughput of TCP Vegas. s.l. : Department of Computer Sciences, University of Wisconsin. Madison, Wisconsin 53706, 2003.
- [7] F. Kevin; Simulation-based comparisons of Tahoe, Reno, and SACK TCP. s.l. : Lawrence Berkeley National Laboratory One Cyclotron Road, Berkeley, CA 94720, 2003.

---

**[ENUNCIADO](../Markdown/Assignment.md) | [README](../README.md) | [DISEÑO](../Markdown/Design.md)**

---