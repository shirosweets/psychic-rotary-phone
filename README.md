# Laboratorio N°3: Capa de Transporte

Redes y Sistemas Distribuidos - Primer cuatrimestre, FAMaF - 2022.

# Integrantes
- Carrizo, Ernesto.
- Domínguez, Agustín.
- Vispo, Valentina.

---

**[ENUNCIADO](Enunciado.md) | [ANÁLISIS](Analysis.md) | [DISEÑO](Design.md)**

---

# Objetivos

- Leer, comprender y generar modelos de red en Omnet++.
- Analizar tráfico de red bajo tasas de datos acotadas y tamaño de buffers limitados.
- Diseñar y proponer soluciones de control de congestión y flujo.

# Cómo correrlo

Se necesita la herramienta de simulación discreta **OMNET++**, [¿cómo descargar Omnet?](#descarga-de-omnet)

Luego se debe importar el proyecto. En [esta sección](#como-importarlo-en-omnet) está paso a paso cómo hacerlo.

## Correr una simulación

Tenemos un makefile que compila el codigo fuente y lanza la simulación:

```bash
make clean && make run > /dev/null
```

# Descarga de Omnet

* [Imagen de Lubuntu con Omnet instalada](https://mega.nz/file/eaJUGIQQ#3h_VvTJlkmK3KHGZPiLd6EubADPxfze2JNRtHfcc3A4)
* [Imagen de docker con Omnet++](https://github.com/mmodenesi/omnetpy)
* [Guia para ejecutar Omnet en Windows](documents/Guia_para_ejecutar_Omnet++_en_Windows.pdf)

### Documentación de OMNet++

* https://doc.omnetpp.org/omnetpp/manual/
* https://stackoverflow.com/questions/52445993/omnet-on-windows-or-linux
* https://stackoverflow.com/questions/7020069/make-library-not-found

## Como importarlo en Omnet

**File > Import**
<p align="center">
<img src="documents/assets/guide_import_project/guide_import_project_0.png" width="300" title="Omnet Guide 0">
</p>

**General > Projects from Folder or Archivr**
<p align="center">
<img src="documents/assets/guide_import_project/guide_import_project_1.png" width="600" title="Omnet Guide 1">
</p>

**Path del repositorio y le damos aceptar**
<p align="center">
<img src="documents/assets/guide_import_project/guide_import_project_2.png" width="600" title="Omnet Guide 2">
</p>

<p align="center">
<img src="documents/assets/guide_import_project/guide_import_project_3.png" width="600" title="Omnet Guide 3">
</p>

**En caso de que nos aparezca un mensaje emergente, le daremo a YES**
<p align="center">
<img src="documents/assets/guide_import_project/guide_import_project_4.png" width="500" title="Omnet Guide 4">
</p>

---

**[ENUNCIADO](Enunciado.md) | [ANÁLISIS](Analysis.md) | [DISEÑO](Design.md)**

---
