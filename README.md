# T68 Sci-Fi Tribute

Proyecto final de Laboratorio de Computación Gráfica (OpenGL).
Recreación estilizada de la Plaza de las Tres Culturas (Tlatelolco) con los mechas CUAU-68 y SENTINEL.

## Compilar (Linux)
    cmake -S . -B build
    cmake --build build -j
    cd build/proyecto_lab && ./proyecto_lab_main

## Estructura
- proyecto_lab/  código fuente, shaders y modelos
- dependencias/  glad, SOIL2 y cabeceras del código base
- docs/          manual de usuario, manual técnico, referencias y Gantt
- Ejecutable/    binarios listos para ejecutar

## Registro de avances
- 2026-11-06: estructura inicial, plaza, CUAU, SENTINEL y MAC con animaciones por código.
