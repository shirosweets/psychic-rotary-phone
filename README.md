# Laboratorio N°3: Transporte

# Integrantes
- Carrizo, Ernesto.
- Domínguez, Agustín.
- Vispo, Valentina.

# Objetivos

- Leer, comprender y generar modelos de red en Omnet++.
- Analizar tráfico de red bajo tasas de datos acotadas y tamaño de buffers limitados.
- Diseñar y proponer soluciones de control de congestión y flujo.

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

1. ¿Cómo cree que se comporta su algoritmo de control de flujo y congestión ? ¿Funciona4
para el caso de estudio 1 y 2 por igual? ¿Por qué?

# OMNet++

https://doc.omnetpp.org/omnetpp/manual/

https://stackoverflow.com/questions/52445993/omnet-on-windows-or-linux
https://stackoverflow.com/questions/7020069/make-library-not-found

## Imagen de docker con Omnet++ (opcional)

Instrucciones para descargar y ejecutar Omnet++ usando Docker.

https://github.com/mmodenesi/omnetpy

