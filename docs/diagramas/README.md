# Flujo del software

```mermaid
flowchart TD
    A([Inicio]) --> C[Inicialización global de cámara y estado]
    C --> G[glfwInit y creación de ventana]
    G --> W{¿Ventana creada?}
    W -- No --> EW[Mensaje de error y glfwTerminate]
    EW --> EF([Retorno EXIT_FAILURE])
    W -- Sí --> CT[Activar contexto; consultar framebuffer; registrar callbacks; capturar cursor]
    CT --> L[Inicializar funciones OpenGL con GLAD]
    L --> OK{¿GLAD cargó?}
    OK -- No --> EL[Mensaje de error; sin glfwTerminate en esta rama]
    EL --> EF
    OK -- Sí --> VP[Configurar viewport]
    VP --> S[Cargar, compilar y enlazar lighting.vs y lighting.frag]
    S --> M[Cargar seis Model con Assimp; materiales y texturas SOIL2; buffers Mesh]
    M --> P[Activar shader; asignaciones iniciales; construir proyección]
    P --> Q{¿Se solicitó cierre?}
    Q -- Sí --> F[glfwTerminate]
    F --> Z([Retorno 0 y fin])
    Q -- No --> T[Calcular deltaTime y guardar lastFrame]
    T --> E[glfwPollEvents: teclado y ratón]
    E --> D[DoMovement: cámara y posición del grupo]
    D --> AN[Animation: ángulo de sierras, tiempos y esquive]
    AN --> CL[Limpiar color y profundidad; habilitar profundidad; activar shader]
    CL --> MS[Construir matrices SENTINEL; barrido, retroceso y destellos]
    MS --> H{¿Hay destello activo?}
    H -- Sí --> LP[Situar luz puntual en primera boca activa]
    H -- No --> LO[Enviar intensidades puntuales cero]
    LP --> U[Enviar luces, vista y proyección]
    LO --> U
    U --> DR[Construir matrices de objetos y SetModel; dibujar plaza, CUAU, SENTINEL, railguns y MAC con aspas]
    DR --> B[glfwSwapBuffers: actualizar pantalla]
    B --> Q

%% Explicación: la cámara existe antes de main. El cierre se consulta al inicio
%% del ciclo; Escape marca la solicitud dentro del procesamiento de eventos.
%% Los fallos de shader, modelo o textura no tienen una rama general de aborto.
%% La animación SENTINEL se completa en main, después de Animation().

```

La cámara se construye antes de entrar a main. Después de cargar los recursos, el programa repite la lectura de entradas, la actualización y el dibujo hasta que se solicita el cierre. Los errores de ventana y GLAD sí retornan; los de carga de modelos o shaders se informan sin una decisión general de salida. El archivo draw.io conserva nodos y conectores editables, con esta explicación debajo del flujo.
