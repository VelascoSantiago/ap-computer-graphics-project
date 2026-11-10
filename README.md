# T68 Sci-Fi Tribute

Proyecto final de Laboratorio de Computación Gráfica (OpenGL). **Equipo H:** Santiago Velasco García y Cristian Michael Aguilar Chávez.
Recreación estilizada de la Plaza de las Tres Culturas (Tlatelolco) con los mechas CUAU-68 y SENTINEL.

## Video explicativo
<https://www.youtube.com/watch?v=_mIKQs8rzkg>

## Ejecutar en Windows
Descarga el repositorio (Code → Download ZIP), descomprímelo y abre `Ejecutable/proyecto_lab_main.exe` (debe quedar junto a `Models/`, `Shader/` y `UI/`). Tarda unos 8 s en abrir. Más detalles en `Ejecutable/LEEME.txt` y `docs/Manual_de_Usuario.md`.

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
- 2026-09-29: entrega parcial de teoría "Prototipo" (concepto y primer prototipo de los personajes).
- 2026-10-13: entrega parcial "Modelado" (CUAU-68, SENTINEL, MAC y plaza modelados en Blender).
- 2026-10-27: entrega parcial "Texturizado, color e iluminación".
- 2026-11-05: entrega parcial "Animación" (animaciones en Blender).
- 2026-11-06: estructura inicial en OpenGL: plaza, CUAU-68, SENTINEL y MAC con animaciones por código (`e128f2b`).
- 2026-11-08: CMake raíz y `.gitignore` (`c4d5933`); primera versión de la documentación, aportada por Cristian (`2caa3f9`, PR #1).
- 2026-11-08: modelos con materiales completos y por partes, máquina de estados del relato (10 escenas), HUD y textos (`d4644c7`).
- 2026-11-09: ajustes de escenas y textos (`99ef023`); integración del PR #1 (`0024f94`).
- 2026-11-09: manual de usuario y manual técnico nuevos, diagramas de flujo y de estados, Gantt, capturas de referencia y opción `--sinhud` (`552c89e`, `bdba803`).
- 2026-11-09: GitHub Actions para compilar en Windows y Linux, y ejecutable estático de Windows en `Ejecutable/`.
- 2026-11-09: video explicativo publicado en YouTube (liga al inicio de este README).
