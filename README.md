# Entregable LAB 1

**Autores:** Matías García, Vicente Miranda  

## Acerca del Proyecto

Este proyecto consiste en la implementación de un motor de procesamiento y manipulación de imágenes RGB. El sistema no solo permite aplicar transformaciones espaciales a una imagen como desplazamientos y rotaciones, sino que implementa un sistema completo de **gestión de estados** utilizando Estructuras de Datos como pilas y colas construidas desde cero.

El objetivo principal es demostrar el manejo eficiente de memoria dinámica mediante punteros, junto con la correcta aplicación teórica y práctica de pilas (Stacks) y colas (Queues) para simular el comportamiento de un historial de acciones.

---

## Características y Funcionalidades

El proyecto se divide en dos módulos principales:

### Parte 1: Transformaciones Espaciales
La clase `moving_image` maneja tres matrices dinámicas (capas Red, Green, Blue o mas bien conocidas RGB). Se implementaron los siguientes movimientos optimizados:
* `move_left(d)` / `move_right(d)`: Desplazamiento horizontal circular continuo de `d` píxeles.
* `move_up(d)` / `move_down(d)`: Desplazamiento vertical circular, optimizado mediante la reasignación de punteros de fila.
* `rotate()`: Rotación matemática de la imagen en 90 grados en sentido antihorario.

### Parte 2: Gestión de Memoria y Estados (Estructuras de Datos)
Para el control del historial, se implementaron Nodos dinámicos que alimentan las siguientes estructuras:
* **Undo (`undo()`):** Utiliza una **Pila (Stack)** para revertir el último movimiento aplicado, ejecutando la transformación matemática inversa.
* **Redo (`redo()`):** Utiliza una segunda **Pila (Stack)** para recuperar acciones revertidas, siempre y cuando no se haya ejecutado un movimiento nuevo.
* **Repeat (`repeat()`):** Repite la última acción registrada en el sistema.
* **Repetir todo (`repeat_all()`):** Utiliza una **Cola (Queue)** para almacenar cronológicamente cada acción desde la instanciación del objeto. Al llamarse, reconstruye el historial paso a paso generando un fotograma `.png` por cada movimiento, permitiendo visualizar la secuencia completa como una "película".

---

## Estructuras Utilizadas

* **Gestión de Memoria:** Manejo estricto de `new` y `delete[]` para evitar *Memory Leaks* durante la creación de Nodos temporales y matrices bidimensionales.
* **Estructuras de Datos:**
    * `struct Node`: Contenedor base de las acciones.
    * `class Stack`: Lógica LIFO para el control de *Undo/Redo*.
    * `class Queue`: Lógica FIFO para el *Tracking* cronológico del historial.
* **Librería Externa:** `svpng.inc` (para la codificación y volcado de las matrices RGB a archivos `.png`).

---

## Compilación y Ejecución

1. Clonamos el repositorio en nuestro dispositivo con el comando de git bash: 
git clone https://github.com/Matygp/Tarea1_ED

2. Entrar a la carpeta Tarea1_ED

3. Para compilar el proyecto en un entorno basado en Unix/Linux, utiliza el compilador `g++`:

```bash
g++ -test.cpp -o test
./test.exe

