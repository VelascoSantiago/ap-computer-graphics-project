# T68 Sci-Fi Tribute — Manual técnico

Universidad Nacional Autónoma de México · Facultad de Ingeniería  
Computación Gráfica  
Integrantes, grupo, profesor y fecha académica: **pendientes de proporcionar**.

Versión inspeccionada: `36edb9352e6053eb2f9f0341e9351ab7027c6a80`. Revisión documental: 6 de noviembre de 2026. Las afirmaciones de implementación corresponden a lectura estática de `proyecto_lab` y sus dependencias compartidas. No se cuenta con una ejecución gráfica ni con mediciones de rendimiento.

## 1. Introducción y justificación

El desarrollo consiste en integrar una representación de la Plaza de las Tres Culturas con personajes de ciencia ficción dentro de una aplicación gráfica interactiva. El escenario procede de un OBJ compuesto que reúne arquitectura, vestigios y mobiliario. CUAU-68, SENTINEL y MAC se importan desde archivos separados y se colocan mediante matrices de modelo. Esta separación permite conservar la plaza como entorno y modificar la posición de los personajes durante la ejecución.

El problema técnico no termina al importar geometría. Los modelos tienen materiales, imágenes y orígenes locales distintos; para mostrarlos juntos es necesario cargar sus recursos, transformar sus coordenadas y proporcionar al shader datos de cámara e iluminación. MAC requiere compensar el origen de su cuerpo y colocar las sierras respecto de él. SENTINEL comparte geometría entre tres instancias, pero necesita una transformación y un estado de disparo distintos para cada una. CUAU combina una posición controlada por el usuario con un desplazamiento calculado en función del tiempo.

La elección de este escenario da un referente identificable para revisar proporciones, distribución espacial y materiales. No existe evidencia de levantamiento métrico ni de una reconstrucción histórica validada; la comparación con fotografías forma parte de la documentación pendiente. En términos de la asignatura, la implementación permite estudiar cómo las transformaciones locales llegan al espacio de pantalla y cómo una actualización temporal modifica la escena sin reconstruir las mallas. Este manual explica esas relaciones y registra las discrepancias encontradas entre el código de aplicación y sus shaders, para que la evaluación distinga lo implementado de lo que todavía debe comprobarse en pantalla.

## 2. Objetivos

### 2.1. Objetivo general

Construir un entorno tridimensional interactivo de Tlatelolco mediante OpenGL, con modelos importados, navegación de cámara y tres animaciones programadas, cuyo comportamiento pueda verificarse mediante controles y casos de prueba definidos.

### 2.2. Objetivos específicos

1. Representar el escenario contenido en `plaza.obj` conservando su organización geométrica y referencias de materiales.
2. Integrar CUAU-68, tres instancias de SENTINEL con railgun y MAC con tres sierras mediante transformaciones independientes.
3. Implementar navegación con teclado y ratón, así como selección y traslación de los grupos de personajes.
4. Controlar el giro de sierras, la secuencia de disparo y el esquive mediante tiempo transcurrido y banderas independientes.
5. Integrar iluminación direccional y un destello puntual en el cálculo de color de las superficies.
6. Documentar la carga de recursos y verificar el comportamiento con una matriz de pruebas, dejando identificadas las ejecuciones pendientes.

Estos objetivos describen características existentes y una actividad de verificación prevista; no certifican que todas hayan superado pruebas de ejecución.

## 3. Alcance

La escena dibuja una plaza compuesta y seis tipos de recurso geométrico importado: plaza, CUAU, cuerpo de SENTINEL, railgun, cuerpo de MAC y aspa. SENTINEL y su arma se dibujan tres veces; el aspa se reutiliza en tres alturas. No se carga una copia de archivo por instancia. El inventario nominal de la plaza incluye iglesia exterior e interior, muebles de iglesia, convento, Chihuahua, vestigios, lápida, árboles, bancas y otros elementos urbanos. Los nombres de objetos no demuestran por sí mismos que su aspecto corresponda al lugar real.

La navegación es libre y usa la orientación de la cámara. Los grupos se trasladan sobre los ejes del mundo y sus posiciones pueden imprimirse en consola. Las animaciones comienzan activadas y se alternan con N, B y V. No hay interfaz gráfica de edición ni almacenamiento de cambios.

La apariencia se calcula con una luz direccional y una puntual intermitente. El shader también contiene un foco, pero la aplicación envía sus intensidades en cero. Los materiales se convierten en texturas de imagen, texturas teñidas o colores planos de 1 × 1 píxel. No se implementa un sistema PBR aunque Assimp permita consultar texturas BASE_COLOR. El FBX de CUAU y los shaders `lamp` y `modelLoading` no participan en esta versión.

Se prevén como trabajo documental la ejecución de pruebas, las capturas y la comparación fotográfica. No se presentan como funciones futuras del programa ni como tareas terminadas.

## 4. Limitaciones observables

| Restricción | Consecuencia y evidencia |
|---|---|
| Dependencias externas y enlaces simbólicos | La compilación depende de OpenGL, GLFW y Assimp; CMake enlaza recursos por ruta. Linux es el entorno descrito; no se verificó Windows o macOS. |
| Contexto no solicitado por versión | `main()` no llama a `glfwWindowHint`; GLSL 330 requiere un contexto compatible, que no queda garantizado por una solicitud explícita. |
| Proyección sin conversión angular | Se pasa 45 a `glm::perspective` sin `glm::radians`; GLM 0.9.9.3 interpreta el argumento en radianes. No se acredita un campo visual de 45°. |
| Ventana sin actualización de tamaño | Viewport y proyección solo se calculan al inicio; no hay callback de framebuffer. |
| Interfaz de samplers inconsistente | C++ escribe nombres que no corresponden con `material.diffuse` y `material.specular`; consultar sección 7.5. |
| Animación rígida y predefinida | Se transforman modelos completos y piezas; no hay skinning, física, detección de impactos o colisiones. |
| Orientación de SENTINEL | Apunta a la base `posCuau`, no a la posición animada ni a una predicción de trayectoria. |
| Una luz de disparo | Si coincide más de un destello, se toma el primero en orden de índice; no se suman tres luces. |
| Importador restringido | Lee normales sin comprobar su existencia; no solicita generarlas ni aplica `aiNode::mTransformation`. Otros archivos podrían no importarse como se espera. |
| Manejo parcial de errores | Fallos de shaders o modelos se imprimen y no abortan necesariamente; `TextureFromFile` no comprueba imagen nula. |
| Gestión de recursos | No se definen destructores para liberar VAO/VBO/EBO, texturas y programa; la salida normal termina GLFW y el proceso. |

No hay pruebas que permitan atribuir caídas de rendimiento a la complejidad de la plaza. Tampoco se establecen límites de memoria o una tasa de fotogramas esperada sin medición.

## 5. Metodología de desarrollo propuesta

Se propone una metodología incremental e iterativa. Cada incremento debe dejar una escena que pueda abrirse y revisarse antes de añadir otra fuente de cambios. El historial disponible permite identificar incorporación de recursos y animaciones, pero no reconstruye reuniones, responsables ni la secuencia completa de trabajo del equipo.

| Incremento propuesto | Trabajo y criterio de revisión |
|---|---|
| Preparación del escenario | Revisar OBJ/MTL de plaza, orientación y escala; obtener una vista general sin personajes. |
| Integración de modelos | Incorporar los seis recursos usados y comprobar sus orígenes, posiciones y materiales. |
| Programación de animaciones | Revisar una animación a la vez; alternar su bandera y verificar transformaciones locales. |
| Pruebas e integración | Ejecutar la matriz de la sección 11, combinando movimientos manuales y animaciones. Registrar errores reproducibles. |
| Documentación | Contrastar cada control y ecuación con el código; añadir capturas y resultados observados. |
| Presentación final | Preparar un recorrido que muestre escenario, controles y animaciones; grabar y revisar la entrega. |

Este orden facilita localizar si una anomalía procede de la importación, de una matriz o del shader. La iteración permite volver al incremento correspondiente cuando una prueba detecta una discrepancia, sin atribuir el problema a toda la escena. La revisión visual es necesaria porque una compilación correcta no demuestra que un pivote o un material se hayan representado correctamente. Las fechas del cronograma se separan de esta propuesta; véase [Explicación del Gantt](gestion/Explicacion_Gantt.md).

## 6. Arquitectura del software

La aplicación concentra entrada, estado y renderizado en `main.cpp`. No hay clases independientes llamadas sistema de animación o administrador de escena: esos papeles los cumplen variables globales, funciones y bloques del ciclo principal. `Model` administra colecciones de `Mesh`; cada malla conserva sus datos de CPU y sus identificadores de GPU. `Shader` pertenece a las dependencias compartidas.

| Módulo real | Archivo | Responsabilidad e intercambio de datos |
|---|---|---|
| Programa principal | `proyecto_lab/main.cpp` | Crea contexto, carga recursos, actualiza tiempo y emite el dibujo. Posee posiciones y banderas globales. |
| Entrada | Callbacks y `DoMovement()` en `main.cpp` | GLFW informa eventos; `keys` conserva estados; las funciones modifican cámara, selección y posiciones. |
| Animación | `Animation()` y bloques de `main()` | Actualizan ángulos y tiempos; calculan desplazamientos, matrices, retrocesos y luz. |
| Cámara | `proyecto_lab/Camera.h` | Mantiene base de orientación y posición; produce la matriz de vista con GLM. |
| Modelos y materiales | `proyecto_lab/Model.h` | Assimp transforma datos importados en mallas; selecciona imágenes, tintes y colores. |
| Mallas | `proyecto_lab/Mesh.h` | Carga atributos e índices en GPU y enlaza texturas antes de dibujar. |
| Programa de shaders | `dependencias/Shader.h` | Lee, compila y enlaza GLSL; expone `Program` y `Use()`. |
| Transformación de vértices | `proyecto_lab/Shader/lighting.vs` | Calcula posición de clip, posición de mundo y normal transformada. |
| Iluminación | `proyecto_lab/Shader/lighting.frag` | Combina componentes ambiental, difusa y especular con luces y texturas. |
| Configuración | CMake raíz y `proyecto_lab/CMakeLists.txt` | Construye dependencias, define `proyecto_lab_main` y enlaces a recursos. |

Los materiales no son objetos persistentes propios con propiedades completas del MTL. El importador extrae los datos que necesita y los traduce a una lista de `Texture`; el shader usa un brillo global por malla. Esta decisión de implementación limita qué propiedades originales llegan al dibujo.

## 7. Documentación detallada del código

### 7.1. Estado compartido

| Variables de `main.cpp` | Papel y modificación |
|---|---|
| `posCuau`, `posSentinel`, `posMac` | Bases iniciales (0,0,0), (0,0,−30), (12,0,6); `DoMovement()` modifica la seleccionada. |
| `grupoSel`, `grupos`, `nombreGrupo` | Índice 0..2, punteros a posiciones y nombres; TAB cambia el índice y permite reutilizar el mismo código de movimiento. |
| `offSentinel` | Desplazamientos fijos (−16,0,0), (0,0,−6), (16,0,0) respecto de la base del grupo. |
| `animSierras`, `animScarface`, `animEsquive` | Banderas verdaderas al inicio; N/B/V las invierten. `animScarface` es el nombre interno de la secuencia SENTINEL. |
| `rotAspa`, `tScar`, `tEsq` | Ángulo acumulado en grados y tiempos propios en segundos; `Animation()` los actualiza cuando su bandera lo permite. |
| `cuauOffset`, `cuauYaw`, `cuauTilt` | Resultados del esquive aplicados al dibujar CUAU. |
| `camera`, `keys`, `lastX`, `lastY`, `firstMouse` | Estado de navegación y entrada. Los callbacks y `DoMovement()` lo actualizan. |
| `deltaTime`, `lastFrame` | Diferencia de tiempo y marca anterior; calculadas al comienzo de cada fotograma. |
| `rotMac` | Yaw del cuerpo inicializado en cero, sin control que lo cambie. |

`PLAZA_ESCALA`, `CUAU_ESC`, `SENT_ESC` y `MAC_ESC` valen 1. `MAC_CENTRO=(16,0,0)`, `MAC_SUELO=5.8` y `ASPA_Y={0.30,2.85,5.40}` corrigen la colocación de MAC. `bocaLocal=(9,9,0)` define el punto local usado como boca del railgun; no se infiere automáticamente de la malla.

### 7.2. Funciones del programa principal

#### `int main()` — `proyecto_lab/main.cpp`

No recibe argumentos. Inicializa GLFW sin comprobar el retorno de `glfwInit()`, solicita la ventana y comprueba su puntero. Si falla, imprime un mensaje, termina GLFW y devuelve `EXIT_FAILURE`. Tras activar el contexto, consulta el framebuffer, registra callbacks y captura el cursor. GLAD se inicializa con `glfwGetProcAddress`; si falla devuelve `EXIT_FAILURE`, sin llamar a `glfwTerminate()` en esa rama.

Construye `lightingShader` y seis `Model`, lo que dispara lectura de archivos y creación de recursos gráficos. La cámara ya fue construida globalmente antes de entrar a `main()`. Calcula una proyección una sola vez con planos 0.1 y 1000. En cada iteración modifica `deltaTime` y `lastFrame`, procesa eventos, llama a movimiento y animación, limpia buffers, configura iluminación y matrices, dibuja y presenta el resultado. Las matrices SENTINEL se calculan aquí, no dentro de `Animation()`.

Sus dependencias directas son GLFW, GLAD, GLM, `Shader`, `Model`, `SetModel()` y las funciones de actualización. Su salida normal es 0 tras terminar GLFW. No hay guardado del estado ni liberación explícita de cada recurso OpenGL.

#### `void Animation()` — `proyecto_lab/main.cpp`

Sin parámetros ni valor de retorno: consume `deltaTime` y banderas globales. Incrementa `rotAspa` a 540 grados por segundo y `tScar` a razón de un segundo por segundo cuando están activos. Con esquive activo incrementa `tEsq`, calcula tres componentes de `cuauOffset` y actualiza yaw e inclinación. No modifica posiciones base, no recibe eventos y no envía uniformes.

Sus salidas son estado global para la siguiente construcción de matrices. Depende de seno, coseno, valor absoluto, `atan2`, módulo y conversiones de GLM. Si V está apagada no recalcula el offset ni la orientación; los valores previos permanecen. Las fórmulas y su interpretación están en la sección 9.

#### `void DoMovement()` — `proyecto_lab/main.cpp`

Lee `keys`, `deltaTime` y `grupoSel`. Para WASD o flechas llama a `camera.ProcessKeyboard(dirección, deltaTime)`. Una alternativa de tecla dentro de la misma dirección usa OR, por lo que W y arriba simultáneas no duplican ese avance. Direcciones diferentes se procesan por separado y no se normaliza el movimiento diagonal.

Después obtiene una referencia a `*grupos[grupoSel]` y aplica incrementos de `10*deltaTime` sobre X, Y o Z con J/L, U/O e I/K. No devuelve datos; modifica cámara o una posición global. Las teclas opuestas se cancelan aritméticamente. No calcula colisiones ni limita el volumen de movimiento.

#### `void KeyCallback(GLFWwindow*, int key, int scancode, int action, int mode)`

Función de `main.cpp` registrada ante GLFW. `window` permite solicitar cierre con Escape; `key` identifica la tecla y `action` distingue pulsación, repetición y liberación. `scancode` y `mode` no se utilizan. Para códigos 0..1023 activa o desactiva `keys[key]` en PRESS y RELEASE.

Solo en PRESS invierte las banderas N/B/V, cambia TAB mediante `(grupoSel+1)%3` o imprime posiciones con P. La repetición de tecla no vuelve a alternar las animaciones. Sus efectos son cambios de estado, salida en consola y señal de cierre; no devuelve un valor. `DoMovement()` consume después el estado de teclas mantenidas.

#### `void MouseCallback(GLFWwindow*, double xPos, double yPos)`

Función de `main.cpp` que recibe posición del cursor. La primera llamada inicializa `lastX` y `lastY` para evitar un salto por la referencia inicial. Calcula `xPos-lastX` y `lastY-yPos`, guarda la posición nueva y envía los desplazamientos a `camera.ProcessMouseMovement()`. Modifica las referencias globales y la orientación de cámara; no devuelve valor. `window` no se usa.

#### `static void SetModel(GLuint P, const glm::mat4& m)`

Auxiliar de `main.cpp`. Busca el uniforme `model` del programa `P` y copia la matriz `m` con `glUniformMatrix4fv`, sin transposición (`GL_FALSE`). No cambia la matriz de CPU ni activa el programa: depende de que `lightingShader.Use()` se haya llamado. Su efecto es actualizar estado del programa gráfico vigente para el próximo dibujo. No almacena la ubicación del uniforme entre llamadas.

### 7.3. Clase `Camera` — `proyecto_lab/Camera.h`

| Método | Entradas, salida y estado afectado |
|---|---|
| `Camera(vec3 position, vec3 up, GLfloat yaw, GLfloat pitch)` | Inicializa posición, vertical del mundo y ángulos; fija velocidad 6, sensibilidad 0.15 y zoom 45; llama a `updateCameraVectors()`. |
| `Camera(posX,posY,posZ,upX,upY,upZ,yaw,pitch)` | Alternativa escalar con el mismo efecto; no se usa para la cámara global actual. |
| `GetViewMatrix()` | Sin entradas; devuelve `lookAt(position, position+front, up)` sin modificar estado. |
| `ProcessKeyboard(direction, deltaTime)` | Calcula velocidad por tiempo y suma o resta `front` o `right` a `position`; sin retorno. |
| `ProcessMouseMovement(xOffset,yOffset,constrainPitch=true)` | Escala desplazamientos por sensibilidad; modifica yaw/pitch, limita pitch y reconstruye la base. |
| `ProcessMouseScroll(yOffset)` | Cuerpo vacío: no modifica zoom ni otra variable. No se registra callback de rueda. |
| `GetZoom()`, `GetPosition()`, `GetFront()` | Devuelven copias de zoom, posición y dirección; no alteran estado. |
| `updateCameraVectors()` | Método privado; convierte yaw/pitch a radianes, calcula `front`, normaliza y obtiene `right` y `up` mediante productos cruz. |

Los vectores base evitan que el movimiento lateral dependa de la magnitud de un producto cruz sin normalizar. El límite de pitch evita llegar a una orientación exactamente paralela a `worldUp`. No se impone altura fija.

### 7.4. Clase `Model` y funciones de textura — `proyecto_lab/Model.h`

| Elemento | Contrato, efectos y dependencias |
|---|---|
| `Model(GLchar *path)` | Recibe ruta relativa; llama a `loadModel`; llena colecciones internas y crea recursos de GPU durante la importación. Requiere contexto activo. |
| `Draw(Shader shader)` | Recorre `meshes` y llama a `Mesh::Draw`; recibe el wrapper por valor; no activa por sí mismo el shader. |
| `loadModel(string path)` | `ReadFile` usa Triangulate y FlipUVs. Ante escena inválida imprime error y retorna; de lo contrario calcula `directory` y procesa la raíz. Modifica mallas y cachés indirectamente. |
| `processNode(aiNode*, const aiScene*)` | Agrega mallas referidas por el nodo y recorre hijos recursivamente. No acumula transformaciones de nodos. Sin retorno. |
| `processMesh(aiMesh*, const aiScene*)` | Copia posición y normal de cada vértice; copia UV0 o asigna (0,0); concatena índices de caras; interpreta material y devuelve un `Mesh`. Modifica cachés de texturas. |
| `loadMaterialTextures(aiMaterial*, aiTextureType, string typeName)` | Consulta rutas del material; reutiliza `textures_loaded` por ruta o llama a `TextureFromFile`; devuelve una lista de `Texture`. |
| `TextureFromFile(const char*, string directory)` | Concatena directorio y ruta; SOIL2 carga RGBA; crea textura, mipmaps, repetición y filtros lineales. Libera imagen de CPU y devuelve identificador. No comprueba fallo de carga. |
| `TexturaColorPlano(glm::vec3 kd)` | Convierte Kd a una aproximación sRGB y crea un píxel RGBA opaco con filtrado nearest; devuelve GLuint. |
| `TexturaTenida(filename,tinte,f)` | Carga RGBA, transforma RGB píxel a píxel y crea textura con mipmaps. Ante fallo imprime `ERROR textura:` y devuelve 0. Conserva alfa. |
| `aLineal(float)`, `aSRGB(float)` | Funciones auxiliares: potencias 2.2 y 1/2.2; la segunda limita a [0,1]. Devuelven componentes de color. |

`processMesh()` prioriza BASE_COLOR si existe y, en otro caso, DIFFUSE. Busca el nombre del material en `TINTES`, una tabla de 12 recetas de la plaza. Si hay receta e imagen, produce una textura teñida y la guarda en `cacheMat` por nombre. Sin receta usa las imágenes ordinarias; sin imagen crea color plano desde Kd, con respaldo (0.8,0.8,0.8). Después agrega mapas especulares.

El tintado aproxima una conversión a espacio lineal con potencia 2.2. Calcula gris ponderado `0.2126R+0.7152G+0.0722B`, reduce la separación respecto de ese gris a 0.82, multiplica por 1.1 y mezcla con un color fijo según el factor de la receta. Aunque un comentario menciona HSV, el procedimiento real es una mezcla RGB basada en luminancia, sin conversión explícita a HSV. El resultado se convierte de regreso, se limita y se almacena como RGBA de 8 bits. No hay una corrección sRGB de framebuffer configurada en `main()`.

`textures_loaded` evita recargar rutas dentro de un mismo Model, y `cacheMat` evita regenerar colores o tintes por nombre. No hay un caché global entre Model diferentes. `loadModel()` comprueba las banderas de escena con igualdad frente a INCOMPLETE, no con una máscara de bits; esta comprobación no cubre todas las combinaciones posibles. `processMesh()` presupone normales presentes, porque no solicita `GenNormals` ni comprueba `mNormals`.

### 7.5. Clase `Mesh` y enlace de materiales — `proyecto_lab/Mesh.h`

`Vertex` contiene `Position` (vec3), `Normal` (vec3) y `TexCoords` (vec2). `Texture` conserva identificador, tipo y ruta Assimp. El constructor `Mesh(vector<Vertex>, vector<GLuint>, vector<Texture>)` copia esas colecciones y llama a `setupMesh()`. Este método privado genera VAO, VBO y EBO, sube datos con `GL_STATIC_DRAW` y configura atributos 0, 1 y 2 con el tamaño de `Vertex` y los offsets de sus campos. No devuelve datos; deja identificadores internos listos para el dibujo.

`Draw(Shader shader)` activa unidades consecutivas, enlaza texturas y construye nombres `texture_diffuse1`, `texture_specular1`, etc. Fija `material.shininess=16`, enlaza VAO y ejecuta `glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0)`. Desenlaza VAO y texturas al terminar. El número de llamadas de dibujo depende de las mallas de cada Model, no solo de los seis recursos importados.

Hay una discrepancia comprobable en el contrato de materiales. `lighting.frag` declara `material.diffuse` y `material.specular`, pero `Mesh::Draw()` intenta asignar `texture_diffuse1` y `texture_specular1`. `main()` intenta asignar `Material.difuse`, `Material.specular` y `diffuse`, que tampoco corresponden. Para uniformes inexistentes, la ubicación −1 hace que la escritura no tenga efecto; los samplers activos no reciben así una selección explícita válida de unidades. Por su inicialización predeterminada pueden consultar ambos la unidad 0, de modo que la imagen difusa podría aparecer sin que el mapa especular correcto esté conectado. El resultado visible debe comprobarse; no se documenta una configuración de materiales plenamente validada.

El brillo efectivo enviado por malla es 16, porque esa asignación ocurre después del valor 5 enviado en `main()`. No se transfiere el exponente `Ns` de cada MTL. Estas observaciones describen el código existente; no se hicieron correcciones de fuentes durante esta tarea.

### 7.6. Clase `Shader` y funciones GLSL

En `dependencias/Shader.h`, `Shader(vertexPath,fragmentPath)` recibe dos rutas, lee los archivos, crea y compila etapas, comprueba `GL_COMPILE_STATUS`, enlaza `Program` y comprueba `GL_LINK_STATUS`. Imprime registros de hasta 512 caracteres y elimina los objetos de etapas después del enlace. No lanza una interrupción propia ante compilación fallida. La lectura habilita excepciones para `badbit`, por lo que una ruta inexistente no garantiza el mensaje específico de lectura; también puede terminar en errores de compilación de una fuente vacía. `Use()` no recibe datos ni retorna valor: activa `Program` con `glUseProgram`.

| Función del shader activo | Entradas y resultado |
|---|---|
| `lighting.vs::main()` | Atributos posición, normal y UV, matrices M/V/P. Produce `gl_Position=PVMp`, posición de mundo, normal con inversa transpuesta de M y UV. |
| `lighting.frag::main()` | Normaliza normal y dirección al observador; suma luz direccional, una puntual y foco; emite RGBA. Solo descarta alfa bajo si `transparency==1`. |
| `CalcDirLight(light,normal,viewDir)` | Usa dirección opuesta de luz, producto punto difuso y reflexión especular; devuelve RGB ambiental+difuso+especular. |
| `CalcPointLight(light,normal,fragPos,viewDir)` | Añade dirección desde el fragmento y atenuación por distancia a la suma de componentes. |
| `CalcSpotLight(light,normal,fragPos,viewDir)` | Calcula también intensidad entre conos interno y externo; devuelve RGB del foco. Su intensidad configurada está apagada. |

El cálculo especular usa `reflect`, por lo que corresponde a una formulación de tipo Phong, no al vector medio de Blinn-Phong. `transparency` se fija en 0 y no se habilita mezcla alfa en `main()`; la presencia de materiales de vidrio no demuestra transparencia visual. No hay cálculo de sombras. Las otras dos parejas GLSL presentes quedan fuera del pipeline actual.

## 8. Fundamentos matemáticos aplicados

### 8.1. Transformaciones afines y coordenadas homogéneas

Una posición local se amplía a `p=(x,y,z,1)`. La cuarta componente permite expresar traslación mediante una matriz 4 × 4; para una dirección se usa w=0 y la traslación no contribuye. El shader transforma posiciones como:

$$p_{clip}=P\,V\,M\,(x,y,z,1)^T.$$

M es `model`, que coloca cada objeto en el mundo; V es `view`, obtenida de `Camera::GetViewMatrix()`; P es `projection`, creada en `main()`. La división por w y el mapeo al viewport forman parte del procesamiento posterior de OpenGL. Las posiciones de la boca del railgun también usan w=1 para convertir `bocaLocal` a coordenadas de mundo.

En la convención de vectores columna usada aquí, `translate(m,p)`, seguido de `rotate` y `scale`, construye `M=T R S`: el vértice recibe primero S, después R y finalmente T. Cambiar ese orden altera la posición del pivote. La traslación suma componentes, el escalamiento multiplica por las escalas y la rotación conserva distancias respecto de su eje. Todas las escalas configuradas actualmente son uniformes e iguales a 1.

El shader transforma normales con `mat3(transpose(inverse(model)))`, no con la misma matriz de posición. Esta operación conserva su relación perpendicular con la superficie frente a transformaciones invertibles; posteriormente el fragment shader vuelve a normalizarlas.

### 8.2. Vista, orientación y proyección

La cámara construye su dirección a partir de yaw ψ y pitch φ:

$$f=(\cos\psi\cos\varphi,\ \sin\varphi,\ \sin\psi\cos\varphi).$$

Los ángulos se guardan en grados y se convierten con `glm::radians` en `updateCameraVectors()`. `front` es la normalización de f; `right=normalize(front×worldUp)` y `up=normalize(right×front)`. Con estos vectores `lookAt` forma V.

P usa perspectiva con relación `SCREEN_WIDTH/SCREEN_HEIGHT` y planos cercano 0.1 y lejano 1000. El valor de zoom es 45, pero se omite su conversión a radianes en esta llamada. Por eso se distingue la intención aparente de un campo de 45 grados del argumento efectivo enviado a GLM. No se recalcula P en cada fotograma ni al mover el ratón.

### 8.3. Tiempo y movimiento periódico

$$\Delta t_n=t_n-t_{n-1},\qquad q_{n+1}=q_n+v\Delta t_n.$$

`t_n` es `glfwGetTime()`, Δt es `deltaTime`, q puede ser posición o ángulo y v su velocidad lineal o angular. La cámara usa v=6 unidades/s y los grupos v=10 unidades/s por eje. Multiplicar por tiempo reduce la dependencia del desplazamiento respecto de cuántos fotogramas se dibujan; no constituye una medición de rendimiento.

`lastFrame` comienza en cero y no se reinicia tras cargar modelos: el primer delta incluye el tiempo previo al primer fotograma. No hay límite superior de delta ni paso temporal fijo. Una pausa larga puede producir un incremento grande al volver.

$$s(t)=A\sin(\omega t+\phi),\qquad T=\frac{2\pi}{|\omega|}.$$

A es amplitud, ω frecuencia angular en radianes por segundo y φ fase. SENTINEL usa esta forma para el barrido; CUAU la usa en dos ejes. Los senos y cosenos de `<cmath>` reciben radianes. La velocidad de un término sinusoidal se obtiene como `Aω cos(ωt+φ)`, operación utilizada explícitamente para orientar a CUAU.

## 9. Las tres animaciones

### 9.1. MAC: rotación de sierras

N controla `animSierras`. `Animation()` realiza `rotAspa += 540.0f * deltaTime`, así que la velocidad angular es 540°/s, equivalente a 1.5 vueltas/s; una vuelta tarda 2/3 s mientras esté activa. El ángulo no se reduce módulo 360, sino que se acumula.

La matriz del cuerpo es:

$$M_b=T(posMac)\,R_y(rotMac)\,S(MAC\_ESC)\,T(-16,5.8,0).$$

`posMac` es la base controlada por el usuario; `rotMac` permanece en cero; la última traslación compensa el origen del cuerpo. Para cada sierra se usa:

$$M_i=M_b\,T(16,h_i,0)\,R_z(s_i\theta).$$

`h_i` toma 0.30, 2.85 y 5.40 desde `ASPA_Y`; θ es `rotAspa`; `s_i` vale +1, −1, +1. Se reutiliza el mismo `aspa.obj` en las tres posiciones. Su colocación presupone la geometría local de ese recurso: no se calcula un centroide durante la ejecución ni se añade una traslación inversa de pivote tras el giro. `aspas.txt` documenta los centros usados para situarlas.

`SetModel()` envía cada matriz antes de `Aspa.Draw()`. Al apagar N el ángulo se conserva y al reactivar continúa. El resultado previsto es giro local Z con sentido alternado; el contacto de las sierras con otros objetos no tiene consecuencias. Hace falta observar una ejecución para confirmar la alineación final de los pivotes.

### 9.2. SENTINEL: barrido, disparo y retroceso

B controla `animScarface` y el avance de `tScar`. Para cada índice i=0,1,2 se suma `offSentinel[i]` a `posSentinel`. La diferencia d entre `posCuau` y esa posición produce `yawBase=degrees(atan2(-d.z,d.x))`, porque el cañón se considera orientado hacia +X local.

$$b_i(t)=60\sin(1.5t+0.9i),\qquad c_i(t)=\operatorname{fmod}(t+0.15i,0.45).$$

`t` es `tScar` en segundos; b es barrido en grados, de amplitud ±60° y periodo aproximado 4.19 s; c es el tiempo dentro de un ciclo de disparo de 0.45 s. Las fases dependen del índice para desfasar las instancias. La matriz del cuerpo incluye traslación, yaw base más barrido y escala.

El destello está activo si B está habilitada y `c_i<0.07`. El retroceso es:

$$r_i=\begin{cases}0.8(1-c_i/0.12),& c_i<0.12\text{ y animación activa},\\0,&\text{en otro caso}.\end{cases}$$

r se mide en unidades locales; se aplica con `translate(mSent[i],(-r_i,0,0))` únicamente al railgun. El máximo aparece al inicio del ciclo y se recupera linealmente durante 0.12 s. Mientras hay destello, todo el cuerpo recibe vibración vertical `0.08*sin(120*tScar)`. No es una respuesta física a masa o fuerza.

La primera instancia con destello transforma `bocaLocal=(9,9,0)` usando también el retroceso. Esa posición se envía a `pointLights[0]`. La componente difusa cálida es (3,1.8,0.6), y la atenuación usa constante 1, lineal 0.022 y cuadrática 0.0019:

$$a(d)=\frac{1}{1+0.022d+0.0019d^2}.$$

d es la distancia entre fragmento y luz en `CalcPointLight`. Sin destello se envían intensidades cero. No se dibuja una malla luminosa de boca ni un proyectil; el efecto visible depende de las superficies iluminadas y sus materiales. Las ventanas temporales breves pueden no aparecer en todos los fotogramas si el intervalo entre ellos es grande.

Al apagar B, `tScar` queda detenido pero las expresiones condicionadas suprimen barrido, retroceso, vibración y luz. Se sigue calculando yaw base, que puede cambiar si se mueven los grupos. Por tanto, B no congela la última pose completa. El seguimiento usa `posCuau` sin offset de esquive.

### 9.3. CUAU-68: desplazamiento evasivo

V controla `animEsquive`. Cuando está activa se acumula `tEsq` y se obtiene:

$$o(t)=(6\sin(2t),\ 1.2|\sin(4t)|,\ 4\sin(0.7t)).$$

`o` corresponde a `cuauOffset`; t es `tEsq`. X oscila entre −6 y 6, Z entre −4 y 4 y Y entre 0 y 1.2 unidades respecto de `posCuau`. El desplazamiento lateral tiene periodo π s; el de Z, aproximadamente 8.98 s; el salto, por el valor absoluto, π/4 s. El zigzag es una curva suave producida por senos, no una trayectoria de segmentos rectos. El salto no es parabólico y su cambio de velocidad vertical al tocar la base no se resuelve mediante física.

Las derivadas horizontales son `vx=12*cos(2t)` y `vz=2.8*cos(0.7t)`. La orientación objetivo se calcula como `degrees(atan2(vx,vz))`. Para evitar un giro largo al cruzar ±180° se calcula una diferencia angular envuelta:

$$d_\psi=\operatorname{fmod}(\psi_{obj}-\psi+540,360)-180,\quad k=\min(10\Delta t,1),\quad\psi'=\psi+k d_\psi.$$

ψ es `cuauYaw`, ψobj el objetivo y k el factor de aproximación. Esta actualización suaviza el giro sin una velocidad angular constante. La inclinación `cuauTilt=15*cos(2t)` se aplica alrededor de Z. La matriz final es `T(posCuau+cuauOffset) Ry(cuauYaw+CUAU_YAW_OFFSET) Rz(cuauTilt) S(CUAU_ESC)`.

Si V se apaga, offset y ángulos se conservan; las teclas del grupo aún cambian su base. El modelo OBJ tiene pose fija y se desplaza como unidad. La aparente evasión no consulta el estado de los railguns, por lo que no se demuestra una reacción a disparos ni una lógica de inteligencia artificial.

## 10. Pipeline de renderizado

1. Antes de `main()`, se inicializan las variables globales y la cámara con GLM. Esto no crea una ventana ni requiere dibujar.
2. GLFW se inicializa, crea la ventana y activa su contexto. Se consulta framebuffer y se registran callbacks de teclado y ratón; se captura el cursor.
3. GLAD obtiene punteros a funciones OpenGL. Se define el viewport con las dimensiones consultadas.
4. `Shader` lee `lighting.vs` y `lighting.frag`, compila, enlaza y conserva el identificador del programa. No se usa la pareja `modelLoading` por el hecho de cargar modelos.
5. Assimp lee los seis OBJ, triangula caras e invierte UV. `Model` recorre nodos, crea vértices e índices y obtiene materiales. SOIL2 decodifica imágenes; GLM aporta los vectores usados en tintes y transformaciones. `Mesh` sube geometría a buffers.
6. Se activa el shader, se intentan asignaciones iniciales de samplers y se calcula P. Las discrepancias de nombres y ángulo se describen en secciones anteriores.
7. En cada fotograma se obtiene el tiempo, se calcula delta y se llama a `glfwPollEvents()`. Los callbacks actualizan estados que consumen `DoMovement()` y `Animation()`.
8. Se establece el color de fondo y se limpian color y profundidad. Se habilita `GL_DEPTH_TEST`; el programa lo hace después de la limpieza en cada iteración. Se activa el programa de iluminación.
9. Se envían posición de cámara y luz direccional. Se construyen matrices de los SENTINEL para decidir retroceso y luz puntual; se apaga el foco y se envían vista y proyección.
10. Se dibuja plaza, CUAU, tres pares SENTINEL/railgun y cuerpo MAC con tres aspas. Antes de cada recurso se cambia M. Por cada malla se enlazan texturas y VAO y se dibujan triángulos indexados.
11. El vertex shader transforma vértices; OpenGL ensambla y rasteriza triángulos; el fragment shader evalúa luces y texturas y el test de profundidad resuelve superficies. `glfwSwapBuffers()` presenta el buffer resultante.
12. La condición `glfwWindowShouldClose()` decide si se repite el ciclo. Al salir normalmente se ejecuta `glfwTerminate()` y termina el proceso.

El [flujo editable](diagramas/flujo_software.drawio) y su [versión Mermaid](diagramas/flujo_software.mmd) muestran las ramas de error y el retorno del ciclo. Una solicitud de cierre recibida en `PollEvents()` puede permitir completar el fotograma en curso antes de comprobar la condición otra vez.

## 11. Pruebas

### 11.1. Verificaciones efectuadas

Se leyeron `main.cpp`, cabeceras locales, `dependencias/Shader.h`, shaders y CMake, se identificaron los recursos cargados y se consultó el historial específico. Se verificó en el GLM incluido que la perspectiva calcula la tangente del argumento angular sin convertirlo. La consulta `cmake --version` devolvió comando no disponible en PowerShell. Estas evidencias respaldan el análisis estático, no una aprobación funcional.

### 11.2. Matriz de ejecución pendiente

Las celdas de resultado observado se dejan vacías porque no se ejecutaron estos casos. Al completarlas, adjuntar sistema operativo, GPU/controlador, versiones de bibliotecas, commit, registro de consola y capturas. Usar aprobado, fallido o bloqueado solo con evidencia del caso correspondiente.

| Identificador | Prueba | Procedimiento | Resultado esperado | Resultado observado | Estado |
|---|---|---|---|---|---|
| PT-01 | Configuración y compilación | Seguir comandos del manual de usuario y compilar el target | CMake encuentra bibliotecas y produce `proyecto_lab_main` | | Pendiente |
| PT-02 | Creación de ventana | Ejecutar desde `build/proyecto_lab` | Ventana solicitada 1280 × 720, título correcto y sin error GLFW | | Pendiente |
| PT-03 | Inicialización OpenGL | Ejecutar con controlador compatible; conservar registro | GLAD carga y GLSL 330 puede compilarse | | Pendiente |
| PT-04 | Carga de shaders | Revisar consola al arrancar y comprobar ambos `lighting.*` | Sin errores de compilación o enlace; objetos con iluminación | | Pendiente |
| PT-05 | Carga de modelos | Revisar consola y localizar seis recursos en escena | Sin errores Assimp; plaza y personajes presentes | | Pendiente |
| PT-06 | Renderizado de plaza | Recorrer iglesia, edificio, vestigios y lápida | Geometría visible, profundidad coherente; registrar diferencias de material | | Pendiente |
| PT-07 | Cámara | Probar WASD, flechas y ratón, incluyendo vista elevada | Traslación relativa y giro limitado en pitch; sin zoom de rueda | | Pendiente |
| PT-08 | Selección y traslación | Ciclar TAB; probar J/L, I/K, U/O en cada grupo y P | Orden CUAU/SENTINEL/MAC; traslaciones por ejes y posiciones en consola | | Pendiente |
| PT-09 | MAC | Observar sierras; pulsar N, esperar y reactivar | Giro +/−/+; pausa conserva ángulo y luego continúa | | Pendiente |
| PT-10 | SENTINEL | Observar varios ciclos; alternar B y mover base CUAU | Barrido, retroceso y luz; apagado elimina efectos y conserva seguimiento base | | Pendiente |
| PT-11 | CUAU-68 | Observar recorrido; desactivar V durante salto y mover su grupo | Oscilación e inclinación; pausa conserva pose y permite mover base | | Pendiente |
| PT-12 | Controles simultáneos | Mover cámara y un grupo con animaciones habilitadas | Entradas independientes; documentar cualquier desplazamiento inesperado | | Pendiente |
| PT-13 | Cierre | Ejecutar una vez con Escape y otra con botón de cierre | Finaliza ciclo y desaparece ventana sin bloqueo | | Pendiente |
| PT-14 | Recursos y diagnóstico | En copia de prueba, ejecutar desde directorio sin recursos, sin modificar originales | Registrar mensajes reales de shader/Assimp; no asumir recuperación automática | | Pendiente |

No se ensayaron deliberadamente fallos de controlador ni se midieron FPS. La revisión editorial y estructural de estos documentos se registra en [README](README.md), separada de las pruebas del programa.

## 12. Conclusiones

La inspección permite identificar una integración concreta entre geometría importada y transformaciones calculadas durante el ciclo de dibujo. Los personajes no dependen de volver a cargar sus archivos para cambiar de posición: conservan sus mallas y reciben matrices distintas antes de dibujarse. La reutilización del railgun, del cuerpo SENTINEL y del aspa muestra cómo un recurso puede participar en varias instancias con estados espaciales diferentes. La plaza, en cambio, se mantiene como un conjunto estático de mallas.

Las animaciones permiten analizar tres relaciones distintas entre tiempo y geometría. MAC acumula un ángulo a velocidad constante y hereda la matriz del cuerpo para colocar sus sierras. SENTINEL combina una orientación base con una oscilación y divide el tiempo en ventanas de retroceso e iluminación. CUAU calcula una trayectoria a partir de senos y obtiene de sus derivadas una orientación aproximada. Estas relaciones explican el movimiento previsto sin atribuirlo a una simulación física o a una reacción autónoma. Constituyen contenido de aprendizaje observable en la implementación; no sustituyen la reflexión personal que todavía debe aportar el equipo.

La conexión entre materiales y shaders requiere una revisión especial. Importar correctamente una imagen no basta para demostrar que el sampler adecuado la consulte. La discrepancia de nombres de uniformes, el brillo sobrescrito y el argumento angular de la proyección son hallazgos que una descripción basada únicamente en la apariencia podría omitir. Su registro permite plantear pruebas dirigidas y delimita el estado real de la versión documentada.

No se concluye que el programa haya superado compilación, ejecución o evaluación visual en este entorno. Para cerrar esa validación se necesitan los resultados de la matriz de pruebas, capturas reales y datos del equipo de ejecución. La comparación arquitectónica también requiere vistas equivalentes del lugar y del modelo. Con esas evidencias podrá evaluarse la correspondencia visual y completar las conclusiones del equipo sin convertir expectativas de funcionamiento en resultados comprobados.

## 13. Fuentes de implementación

- [`main.cpp`](../proyecto_lab/main.cpp): estado, entrada, animación y ciclo de dibujo.
- [`Camera.h`](../proyecto_lab/Camera.h), [`Model.h`](../proyecto_lab/Model.h) y [`Mesh.h`](../proyecto_lab/Mesh.h): clases utilizadas.
- [`Shader.h`](../dependencias/Shader.h), [`lighting.vs`](../proyecto_lab/Shader/lighting.vs) y [`lighting.frag`](../proyecto_lab/Shader/lighting.frag): carga y ejecución gráfica.
- [Perspectiva en GLM](../dependencias/glm/glm/ext/matrix_clip_space.inl), [configuración raíz](../CMakeLists.txt) y [configuración del proyecto](../proyecto_lab/CMakeLists.txt).
- [Resumen de inspección](Resumen_Tecnico.md): alcance de evidencia y diferencias identificadas.
