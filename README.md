# Plataforma de Streaming - Proyecto Final (2026-0)

**Curso:** Programación III
**Proyecto:** Motor de Búsqueda y Recomendación de Películas

## Integrantes
* JOHN BRANDON QUINONES ESPINAL
* FABRIZIO LEIVA NOVOA
* ARMANDO ANDRES RUESTA (no trabajo)

**[Enlace al Video de Presentación]** *https://drive.google.com/drive/folders/1Z_BDn4P2AMKapo_dmdUMzuZCggClKg0o?usp=sharing*

---

## Descripción del Proyecto
Este proyecto es una plataforma de streaming en C++ que permite la búsqueda eficiente de películas por título, frases, palabras parciales o etiquetas (tags). Implementa procesamiento concurrente para la carga de datos, un sistema de ranking personalizado mediante Heaps, y un motor de recomendaciones basado en Teoría de Grafos.

---

## 1. Arquitectura y Patrones de Diseño

Para garantizar que el código sea modular, escalable y mantenible, implementamos los siguientes patrones de diseño (SOLID):

* **Singleton (`DatabaseManager`):** Garantiza que la enorme base de datos (miles de películas cargadas desde el CSV) se instancie una sola vez en memoria, proporcionando un punto de acceso global y evitando copias innecesarias que colapsarían la memoria RAM.
* **Strategy (`IBusquedaStrategy`):** Separa los algoritmos de búsqueda. Actualmente permite `BusquedaGlobalStrategy` y `BusquedaPorTituloStrategy`. Si en el futuro se desea agregar búsqueda por director o año, el código está abierto a la extensión sin modificar el motor principal (Open/Closed Principle).

---

## 2. Pre-procesamiento y Concurrencia (Multi-threading)

El grupo es responsable del pre-procesamiento del dataset `mpst_full_data.csv`. Para ello, implementamos:
1. **Limpieza de Datos:** Eliminación de signos de puntuación y conversión de todo el texto a minúsculas para asegurar búsquedas *case-insensitive*.
2. **Carga Concurrente:** Dado el volumen de datos, la lectura del archivo se divide en "lotes" (batches) de 1000 filas. Utilizamos la librería `<future>` y `std::async` para delegar el procesamiento de cada lote a **hilos en paralelo** (Threads). 
3. **Sincronización:** Utilizamos `std::mutex` y `std::lock_guard` para evitar condiciones de carrera (*Data Races*) al momento en que los hilos escriben los datos procesados en la Hash Table global.

---

## 3. Justificación de Estructuras de Datos

Tal como se solicita, la elección de las estructuras fundamentales ha sido documentada y justificada en base a su eficiencia computacional:

### Árbol de Sufijos (Suffix Trie)
* **Requisito:** Búsqueda rápida por prefijos, palabras completas o **subcadenas** (Ej: buscar "bar" y encontrar "barco").
* **Justificación:** Un Árbol Binario de Búsqueda (BST) o un Hash Map tradicional obligan a buscar coincidencias exactas de palabras. Elegimos un **Suffix Trie** porque indexa cada sufijo de cada palabra. Esto permite encontrar cualquier fragmento de texto en tiempo $O(M)$, donde $M$ es la longitud de la consulta del usuario, independientemente de cuántas películas existan en la base de datos.
* **Inserción:** $O(L^2)$ por palabra, donde $L$ es la longitud de la palabra.

### Hash Table (`unordered_map`)
* **Justificación:** Utilizado como la base de datos principal (`DatabaseManager`), mapeando el `id` de la película a su estructura. Permite acceso directo a la información completa de cualquier película en tiempo constante $O(1)$.

### Max-Heap (`priority_queue`)
* **Justificación:** Utilizado tanto para la **Paginación** como para el **Top N de Recomendaciones**. En lugar de ordenar todo el arreglo de resultados (lo cual tomaría $O(N \log N)$), un Max-Heap nos permite extraer los $K$ elementos más importantes de un conjunto de $N$ elementos en tiempo $O(N + K \log N)$.

### Colas (`queue`)
* **Justificación:** Utilizado para la funcionalidad **"Ver más tarde"**. Sigue una lógica FIFO (First In, First Out) con complejidad $O(1)$ para inserciones, ideal para una lista de reproducción pendiente.

---

## 4. Algoritmos Core Implementados

### A. Algoritmo de Ranking e Importancia (Fase de Búsqueda)
Al realizar una búsqueda, el sistema asigna un **Score** a cada película para determinar su importancia. Los pesos son:
1. **Coincidencia Exacta en Título:** +100 puntos.
2. **Coincidencia Parcial en Título:** +50 puntos.
3. **Frecuencia en Tags:** +30 puntos por cada vez que aparece el término.
4. **Coincidencia en Sinopsis:** +5 puntos (menor peso para evitar falsos positivos largos).
Los resultados se insertan en el Max-Heap y se extraen en bloques de 5 (Paginación).

### B. Motor de Recomendaciones (Grafo Bipartito + BFS)
Al iniciar la plataforma, se recomiendan películas basadas en los "Likes" previos del usuario.
* **Modelo:** Construimos un Grafo Bipartito implícito conectando `Películas <-> Tags`.
* **Algoritmo:** Ejecutamos una **Búsqueda en Anchura (BFS)** de 2 niveles:
  1. Partimos de las películas que el usuario le dio Like.
  2. *Nivel 1:* Encontramos todos los Tags asociados a esas películas.
  3. *Nivel 2:* Desde esos Tags, encontramos nuevas películas asociadas.
* Cada vez que se llega a una película destino a través de un tag compartido, se suma +1 a su peso (peso de arista). Finalmente, usamos un Max-Heap para extraer las películas con mayor cantidad de caminos conectados.

---

## 5. Tabla de Complejidad Computacional (Big-O)

| Operación | Estructura / Algoritmo | Complejidad Promedio |
| :--- | :--- | :--- |
| Búsqueda de Película por ID | Hash Table (`unordered_map`) | $O(1)$ |
| Búsqueda de Texto (Subcadenas) | Suffix Trie | $O(M)$ |
| Inserción en Índice | Suffix Trie | $O(L^2)$ por palabra |
| Paginación (Top N Resultados) | Max-Heap | Extraer: $O(N \log K)$ |
| Añadir a "Ver más tarde" | Cola (`queue`) | $O(1)$ |
| Recomendaciones (Similitud) | Grafo + BFS | $O(V + E)$ |

---

## Instrucciones de Ejecución

1. Clonar el repositorio.
2. Asegurarse de tener el archivo `mpst_full_data.csv` en la raíz del proyecto.
3. Compilar el proyecto usando cualquier compilador de C++ (requiere estándar C++11 o superior para soporte de hilos):
   ```bash
   g++ main.cpp -o streaming_app -pthread
