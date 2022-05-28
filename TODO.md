- Descartamos ACKs duplicados porque dentro de nuestra red no sería algo que ocurra.
    - Se debería modificar también del lado del receptor.
    - Mantuvimos `congestionController.addAck(seqN)` para modicar su implementación. Puede flag o bool.

- El receptor nunca manda datos (no responde).

- El receptor asume que siempre recibe en orden.