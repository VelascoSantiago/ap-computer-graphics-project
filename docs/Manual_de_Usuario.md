# T68 Sci-Fi Tribute

**Universidad Nacional Autónoma de México**  
**Facultad de Ingeniería**  
Asignatura: Computación Gráfica  
**Manual de usuario**

Integrantes: [pendiente]  
Grupo: [pendiente]  
Profesor: [pendiente]  
Fecha de entrega: [pendiente de confirmar por el equipo]

## 1. Introducción

T68 Sci-Fi Tribute representa una versión tridimensional de la Plaza de las Tres Culturas de Tlatelolco. El escenario reúne modelos de la iglesia de Santiago, el Edificio Chihuahua, vestigios arqueológicos y una lápida, junto con árboles y mobiliario urbano. La representación permite recorrer el conjunto desde una cámara libre; no sustituye un levantamiento arquitectónico ni ofrece información histórica interactiva.

Dentro de la plaza se integran CUAU-68, tres instancias de SENTINEL y MAC. El usuario puede cambiar su punto de observación, seleccionar uno de estos grupos y desplazarlo. Las animaciones programadas muestran el giro de las sierras de MAC, una secuencia de barrido y disparo de SENTINEL y el movimiento evasivo de CUAU-68. Cada animación tiene una tecla de activación independiente y las tres comienzan habilitadas.

Este manual explica los requisitos, el procedimiento de compilación previsto y los controles identificados en la implementación. Las instrucciones se basan en los archivos de `proyecto_lab`; la instalación y la visualización todavía requieren validación en un equipo con las bibliotecas gráficas disponibles. Los espacios reservados para capturas deberán completarse con imágenes obtenidas de esa ejecución. La consola forma parte del uso del programa: allí se muestran errores de carga, el grupo seleccionado y las coordenadas solicitadas mediante el teclado.

## 2. Requisitos

| Requisito | Motivo y situación |
|---|---|
| Linux con entorno gráfico | Es el entorno descrito por el README raíz, con paquetes de Debian/Ubuntu. No se validó una distribución específica en esta revisión. |
| Compilador C++17 y compilador C | CMake fija C++17; GLAD y SOIL2 contienen fuentes C. |
| CMake 3.10 o posterior | Versión mínima declarada en el CMake raíz. |
| OpenGL y controlador gráfico con GLSL 330 | Los shaders activos requieren `#version 330 core`. El programa no solicita explícitamente esa versión al crear el contexto. |
| GLFW y Assimp | Se buscan como dependencias externas mediante CMake. Assimp debe exponer `assimp::assimp` y la API BASE_COLOR usada por el código. |
| GLAD, GLM y SOIL2 del repositorio | Se compilan o incluyen desde `dependencias/`; GLM incluido es 0.9.9.3. |
| Teclado, ratón y terminal | Permiten recorrer la escena, mover grupos y consultar mensajes. |
| Recursos completos | Deben conservarse los OBJ, MTL, imágenes y shaders con sus rutas relativas. |

No se dispone de mediciones para establecer memoria RAM, VRAM o frecuencia de fotogramas mínimas. Windows y macOS son alternativas pendientes de comprobar: no hay una configuración de Visual Studio validada. En Windows, la creación de enlaces simbólicos de recursos también depende de los permisos del sistema.

## 3. Instalación y ejecución

**Procedimiento previsto para Debian/Ubuntu; pendiente de validación completa.** En el entorno de esta revisión, `cmake --version` falló porque CMake no estaba disponible. No se compiló ni ejecutó el programa.

1. Instalar las herramientas y bibliotecas. La lista del README se complementa con Assimp, requerido por el CMake actual:

   ```bash
   sudo apt update
   sudo apt install build-essential cmake git libgl1-mesa-dev libx11-dev \
       libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev \
       libglfw3-dev libassimp-dev
   ```

2. Obtener el repositorio y entrar a su raíz:

   ```bash
   git clone https://github.com/VelascoSantiago/ap-computer-graphics.git
   cd ap-computer-graphics
   ```

3. Configurar desde la raíz. `proyecto_lab` depende de las bibliotecas definidas en el CMake principal y no debe configurarse por separado:

   ```bash
   cmake -S . -B build
   ```

4. Compilar el target correspondiente a Tlatelolco:

   ```bash
   cmake --build build --target proyecto_lab_main --parallel
   ```

   La configuración raíz también examina otras carpetas con CMake. Un error durante esa fase puede proceder de otro subdirectorio, aunque se solicite compilar únicamente este target. Conservar el mensaje exacto para diagnosticarlo.

5. Comprobar los recursos y ejecutar desde el directorio del proyecto compilado:

   ```bash
   cd build/proyecto_lab
   ls Shader/lighting.vs Shader/lighting.frag Models/plaza/plaza.obj
   ./proyecto_lab_main
   ```

   `Shader` y `Models` son enlaces simbólicos creados por CMake hacia los recursos originales. Mover solo el ejecutable a otra carpeta rompe esta organización. No borrar ni sustituir modelos para resolver un problema de rutas.

6. Al terminar, pulsar Escape o usar el cierre de la ventana. La condición de cierre se comprueba en el siguiente inicio del ciclo principal.

## 4. Interfaz y navegación

La aplicación solicita una ventana de 1280 × 720 titulada **T68 Sci-Fi Tribute**. El área de dibujo tiene un fondo azul claro y contiene la plaza y los modelos. No hay menú, indicador gráfico de selección ni panel de instrucciones. La cámara empieza en `(0, 10, 30)` y mira hacia el eje Z negativo. Esta descripción procede de la inicialización; la composición exacta de la primera imagen necesita una captura real.

El cursor queda capturado y oculto. Mover el ratón cambia la dirección de observación; el giro vertical está limitado a ±89°. WASD o las flechas desplazan la cámara respecto de su orientación. Mirar hacia arriba y avanzar también cambia la altura: la cámara no está restringida al piso y puede atravesar objetos. No hay zoom con la rueda ni control de velocidad implementado.

| Tecla o entrada | Acción | Resultado esperado |
|---|---|---|
| W / flecha arriba | Avanzar | La cámara avanza hacia donde mira. |
| S / flecha abajo | Retroceder | La cámara se aleja en la dirección opuesta. |
| A / flecha izquierda | Desplazarse a la izquierda | Movimiento lateral relativo a la cámara. |
| D / flecha derecha | Desplazarse a la derecha | Movimiento lateral relativo a la cámara. |
| Ratón | Cambiar orientación | Gira la vista horizontal y verticalmente. |
| Escape | Solicitar cierre | Termina el ciclo y se cierra la aplicación. |

La velocidad configurada de cámara es 6 unidades de escena por segundo. Las unidades no están acreditadas como metros. Conviene mantener el tamaño inicial: el programa no recalcula la proyección ni el viewport cuando cambia el tamaño de ventana.

## 5. Interacción con los modelos

Al iniciar está seleccionado CUAU. Cada pulsación de TAB cambia la selección en el orden CUAU → SENTINEL → MAC → CUAU. La consola imprime `Moviendo:` seguido del nombre. Seleccionar SENTINEL afecta a las tres instancias juntas; no existe selección individual ni selección con el ratón.

| Tecla | Acción | Resultado esperado |
|---|---|---|
| TAB | Cambiar el grupo seleccionado | La consola identifica el nuevo grupo. |
| J | Disminuir X del grupo | Desplazamiento hacia X negativo del escenario. |
| L | Aumentar X del grupo | Desplazamiento hacia X positivo. |
| I | Disminuir Z del grupo | Desplazamiento hacia Z negativo. |
| K | Aumentar Z del grupo | Desplazamiento hacia Z positivo. |
| U | Aumentar Y del grupo | El grupo sube. |
| O | Disminuir Y del grupo | El grupo baja, incluso bajo el piso. |
| P | Imprimir posiciones base | La terminal muestra `posCuau`, `posSentinel` y `posMac`. |

Las teclas de traslación actúan mientras se mantienen presionadas, a 10 unidades por segundo en cada eje. X y Z son ejes del mundo, no de la cámara. P no guarda cambios en archivos: muestra coordenadas base, por lo que la posición visible de CUAU puede diferir durante su esquive. Al reiniciar se recuperan las posiciones iniciales. No hay una tecla para rotar manualmente MAC aunque exista una variable interna para su rotación.

## 6. Animaciones

| Tecla | Acción | Resultado esperado |
|---|---|---|
| N | Alternar el giro de sierras de MAC | Las tres aspas giran o conservan su ángulo actual. |
| B | Alternar la secuencia de SENTINEL | Se activa o elimina el barrido con retroceso e iluminación intermitente. |
| V | Alternar el esquive de CUAU-68 | El movimiento se actualiza o queda congelado en la pose actual. |

Las sierras de MAC giran alrededor de sus centros, con la sierra central en sentido contrario a las otras dos. El cuerpo permanece en su posición base, salvo cuando se mueve con los controles de grupo. El giro no produce el efecto de iluminación del disparo.

Los SENTINEL giran alrededor de su eje vertical mientras sus railguns retroceden en intervalos cortos. Una luz puntual cálida cambia de posición hacia una boca de arma activa. El programa representa el disparo mediante estas transformaciones y la iluminación: no dibuja balas ni calcula impactos. Al desactivar B, el grupo vuelve a su orientación base y las armas dejan de retroceder. Al reactivar continúa su tiempo interno, sin reiniciar la secuencia.

CUAU-68 oscila lateralmente y hacia adelante y atrás, sube mediante pequeños saltos e inclina el cuerpo. Su orientación intenta seguir la dirección horizontal de su trayectoria. Se transforma todo el modelo; no se articulan piernas o brazos. Al desactivar V conserva incluso la altura de un salto en curso. No existe relación de colisión entre este movimiento y los disparos.

## 7. Problemas frecuentes

| Síntoma | Cómo identificarlo | Acción disponible |
|---|---|---|
| CMake no reconoce una biblioteca | La configuración menciona OpenGL, glfw3 o assimp ausente | Instalar el paquete de desarrollo correspondiente y repetir la configuración. Conservar el error si falta el target `assimp::assimp`. |
| No se crea ventana | Consola: `Failed to create GLFW window` | Comprobar sesión gráfica y controlador de GPU; ejecutar desde una terminal para conservar el diagnóstico. |
| Falla la carga de funciones gráficas | Consola: `Failed to initialise GLAD` | Comprobar el contexto y controlador OpenGL. No basta con tener las cabeceras instaladas. |
| Shaders sin compilar o pantalla sin objetos | Mensajes `ERROR::SHADER::...` | Ejecutar en `build/proyecto_lab`, comprobar los dos archivos `lighting.*` y guardar el registro de compilación GLSL. |
| Falta uno o varios modelos | Mensaje `ERROR::ASSIMP::` | Revisar `Models`, enlaces simbólicos, OBJ y MTL; respetar mayúsculas y nombres. Un error de modelo no necesariamente detiene la aplicación. |
| Colores inesperados o textura ausente | Puede aparecer `ERROR textura:` en la ruta de tintado; la otra ruta no comprueba el fallo de imagen | Revisar imágenes referidas por los MTL. Si existen, reportar también la discrepancia de samplers registrada en el manual técnico. |
| Cambiar el tamaño de ventana altera la vista | El encuadre o la proporción no se ajustan | Cerrar y volver a abrir con el tamaño inicial. No hay ajuste dinámico implementado. |
| Se atraviesan paredes o piso | La cámara o un grupo cruza geometría | Volver usando los controles inversos o reiniciar. No existe detección de colisiones ni restablecimiento por tecla. |

El campo de visión también requiere revisión técnica: el código pasa el valor 45 directamente a una función que recibe radianes. No hay un ajuste de usuario para corregirlo.

## 8. Capturas pendientes

Los siguientes espacios corresponden a imágenes reales por obtener. No se incluyeron imágenes sintéticas ni capturas de una ejecución ajena.

| Identificador | Espacio reservado y contenido |
|---|---|
| CAP-01 | **[PENDIENTE: vista general]** Plaza, iglesia y edificio desde un punto donde se reconozca la distribución. |
| CAP-02 | **[PENDIENTE: MAC]** Cuerpo y tres sierras; complementar con dos fotogramas que permitan observar su giro. |
| CAP-03 | **[PENDIENTE: SENTINEL]** Grupo completo y detalle de un railgun durante su retroceso y luz activa. |
| CAP-04 | **[PENDIENTE: CUAU-68]** Dos posiciones del esquive, incluyendo un salto. |
| CAP-05 | **[PENDIENTE: consola]** Mensajes de TAB y coordenadas producidas con P, sin información personal. |

Las siete capturas de comparación arquitectónica se especifican en [Comparativa Tlatelolco](referencias/Comparativa_Tlatelolco.md). Registrar resolución y condiciones de ejecución al incorporarlas.

## 9. Glosario

| Término | Significado en este manual |
|---|---|
| Cámara libre | Punto de observación que puede desplazarse y girar sin estar sujeto al piso. |
| Grupo | Conjunto controlable: CUAU, los tres SENTINEL o MAC. |
| Modelo OBJ | Archivo que contiene la geometría importada de un objeto. |
| Material MTL | Archivo asociado al OBJ que describe colores y referencias a texturas. |
| Textura | Imagen aplicada a una superficie para definir su apariencia. |
| Shader | Programa ejecutado por la GPU para transformar vértices o calcular el color. |
| Enlace simbólico | Entrada del sistema de archivos que apunta a los recursos originales. |
| Fotograma | Imagen producida en una iteración del ciclo de dibujo. |

## 10. Base documental

Controles y comportamiento: [`main.cpp`](../proyecto_lab/main.cpp) y [`Camera.h`](../proyecto_lab/Camera.h). Instalación: [README raíz](../README.md), [CMake raíz](../CMakeLists.txt) y [CMake del proyecto](../proyecto_lab/CMakeLists.txt). Versión inspeccionada y verificaciones: [Resumen técnico](Resumen_Tecnico.md).
