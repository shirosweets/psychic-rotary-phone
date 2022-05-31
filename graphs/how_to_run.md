
# Como correr el graficador

## Dependencias

Se requiere `python3.x`

Crear un virtual enviroment y entrar a él

```sh
python3 -m venv graph_venv
source graph_venv/bin/activate
```

Instalar los requisitos

```sh
pip install -r requirements.txt && sudo apt-get install python3-tk
```
# Como obtener los datos

Cambiar en el `.ini` el intervalo, correr la simulación y anotar los datos de la siguiente manera:

**Archivos de datos**

Los datos deben estar **estrictamente** en el siguiente formato:

```
| 2.1 | 144 | 144 | 0 | 0 | 0.40 |
| 0.9 | 367 | 367 | 0 | 0 | 0.43 |
| 0.45 | 754 | 754 | 0 | 0 | 0.49 |
| 0.31 | 989 | 989 | 0 | 0 | 0.58 |
...
```

Donde las columnas son respectivamente:

 * Intervalo de Generación exponencial
 * Paquetes generados
 * Paquetes recibidos
 * Paquetes droppeados en el nodo intermedio
 * Paquetes droppeados en el receptor
 * Retraso promedio de llegada

# Creación de gráficos

Correr el comando `python main.py` y pasar como argumento los archivos de los casos de estudio. Si se pasa más de uno, entonces se graficarán ambos en la misma imagen, separados por caso. El nombre del caso es el nombre del archivo pasado. Archivos separados implican casos separados

Por ejemplo:
Si se guardaron los datos de un caso en un archivo `case1.txt` y los de otro en `case2.txt`, y se quieren graficar juntos para comparar se puede corrar:

```sh
python3 main.py case1.txt case2.txt
```

Generará un gráfico con dos casos llamados *case1* y *case2*.
