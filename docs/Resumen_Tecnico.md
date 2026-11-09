# Resumen técnico de la inspección

Inspección del 6 de noviembre de 2026. Base local: commit `36edb9352e6053eb2f9f0341e9351ab7027c6a80`. Este inventario se elaboró antes de los manuales mediante lectura de archivos e historial; no equivale a una ejecución.

| Elemento | Hallazgo confirmado | Evidencia |
|---|---|---|
| Inicio | Ventana de 1280 × 720, título T68 Sci-Fi Tribute; contexto actual antes de GLAD | `proyecto_lab/main.cpp`, `main()` |
| API gráfica | GLAD carga funciones; shaders activos declaran GLSL 330 core; no hay `glfwWindowHint` que solicite explícitamente un contexto 3.3 | `main.cpp`, `Shader/lighting.*` |
| Programa gráfico | Se usa únicamente la pareja `lighting.vs` y `lighting.frag` | Construcción de `lightingShader` |
| Recursos | Se construyen seis Model: plaza, CUAU, SENTINEL, railgun, cuerpo MAC y aspa | `main()` |
| Importación | Assimp triangula e invierte UV; recorre nodos; copia normales existentes, sin generarlas ni aplicar matrices de nodo | `Model.h` |
| Materiales | Prioriza BASE_COLOR sobre DIFFUSE; aplica tintes por nombre, imágenes sin tinte o textura 1 × 1 desde Kd; carga mapas especulares | `Model.h` |
| Dibujo | VAO, VBO y EBO por malla; atributos posición, normal y UV; `glDrawElements(GL_TRIANGLES, ...)` | `Mesh.h` |
| Cámara | Posición inicial (0,10,30), yaw −90°, pitch 0°; velocidad 6 unidades/s; sensibilidad 0.15; sin colisiones | `Camera.h`, globales de `main.cpp` |
| Entrada | WASD/flechas, ratón capturado, Escape; TAB, J/L, I/K, U/O, P; N/B/V | Callbacks y `DoMovement()` |
| Tiempo | `deltaTime = currentFrame - lastFrame`; tres banderas inicialmente verdaderas | `main()`, `Animation()` |
| MAC | Tres aspas con giro local Z, sentidos +/−/+, 540°/s; cuerpo desplazado para compensar su origen | `main()`, `Animation()`, `Models/MAC/aspas.txt` |
| SENTINEL | Tres instancias; orientación hacia la posición base de CUAU, barrido ±60°, ciclos de 0.45 s, retroceso y una luz puntual compartida | Bucle de matrices `mSent` |
| CUAU | Traslación sinusoidal X/Z, salto absoluto sinusoidal, yaw suavizado e inclinación Z; sin animación esquelética | `Animation()` y matriz de CUAU |
| Finalización | Salida normal con `glfwTerminate()` y retorno 0; error de ventana termina GLFW; error GLAD retorna sin esa limpieza | `main()` |

## Escenario e inventario

`Models/plaza/plaza.obj` incluye objetos `iglesia_por_fuera`, `iglesia_interior`, `iglesia_muebles`, `convento`, prefijos `PASEO_CHIHUAHUA_` y `PASEO_VESTIGIOS_`, `lapida`, `arbol` y sus copias, `banca`, `banca.001`, `barda`, `piso`, `paloma`, `bote`, `jardinera`, `puestos` y mobiliario numerado. La plaza se dibuja como un solo Model compuesto por mallas. Los nombres respaldan la presencia de estos recursos, pero no acreditan fidelidad arquitectónica. No se identificó un objeto llamado literalmente reja: la correspondencia de `barda` y el mobiliario con las rejas solicitadas requiere inspección visual.

El FBX de CUAU, sus recursos `.fbm`, los `.bak`, `RedDog.obj` y las parejas `lamp.*` y `modelLoading.*` están presentes, pero no se cargan en el programa actual.

## Diferencias y restricciones relevantes

- N congela el ángulo de aspas y V conserva el desplazamiento y la orientación actuales. B conserva `tScar`, pero elimina barrido, vibración, retroceso y luz; el grupo vuelve a su orientación base. No son tres pausas equivalentes.
- No existen balas, impactos, daño ni física; el salto es una función de tiempo. SENTINEL apunta a `posCuau`, sin sumar `cuauOffset`.
- `Mesh::Draw()` intenta escribir `texture_diffuse1` y `texture_specular1`, mientras el shader activo usa `material.diffuse` y `material.specular`. `main()` escribe `Material.difuse`, `Material.specular` y `diffuse`, también distintos. Los samplers efectivos quedan sin asignación explícita válida y pueden compartir la unidad 0. El resultado de materiales necesita validación gráfica.
- `Mesh::Draw()` fija `material.shininess` en 16, sobrescribiendo el 5 enviado desde `main()`.
- `camera.GetZoom()` devuelve 45 y se pasa directamente a `glm::perspective`. GLM incluido (0.9.9.3) calcula `tan(fovy/2)` sin convertir grados. No debe describirse la proyección efectiva como un campo de visión validado de 45°.
- Proyección y viewport se calculan una vez; no hay callback de redimensionamiento. No hay control de rueda: `ProcessMouseScroll()` está vacío.

## Compilación y evidencia disponible

El CMake raíz requiere C++17, OpenGL, GLFW y Assimp; construye GLAD y SOIL2 locales e incorpora subdirectorios. El target de este proyecto es `proyecto_lab_main`. Los recursos se exponen mediante enlaces simbólicos. La inclusión de `stb_image.h` se resuelve por el directorio público de SOIL2 propagado al enlazar `soil2`; no hay llamadas directas a stb en `main.cpp`.

Se intentó `cmake --version`: PowerShell informó que el comando no está disponible. No se configuró ni compiló el proyecto; tampoco se ejecutó una ventana gráfica. Pandoc no apareció en la consulta de herramientas. No se instalaron dependencias ni se modificó configuración.

El historial específico de `proyecto_lab` contiene `3563e24` (5 de noviembre de 2026, 00:53:39 −06:00, incorporación inicial) y `36edb93` (5 de noviembre, 23:46:34 −06:00, animaciones y sustitución de recursos). Estas fechas son registros de commits, no fechas demostradas de trabajo completo.
