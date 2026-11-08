# Datos y verificaciones pendientes

Este registro distingue información que debe aportar el equipo de comprobaciones que necesitan una ejecución. Los apartados técnicos redactados se basan en la versión `36edb9352e6053eb2f9f0341e9351ab7027c6a80`.

## Información del equipo

| ID | Dato requerido | Dónde se utiliza |
|---|---|---|
| EQ-01 | Integrantes, grupo y profesor | Portadas de manuales. |
| EQ-02 | Fecha académica de portada y confirmación de nombre provisional | Identificación final de entrega; no confundir con fecha de inspección. |
| EQ-03 | Fechas y responsables reales de planeación, modelado, programación e integración | CSV y cronograma histórico. |
| EQ-04 | Evidencia de pruebas ya realizadas, si existe | Matriz técnica; no llenar observados con recuerdos sin registro suficiente. |
| EQ-05 | Reflexión del equipo sobre decisiones y aprendizaje | Complemento de metodología y conclusiones sin inventar experiencias. |
| EQ-06 | Procedencia, autores y licencias de modelos y texturas | Créditos de recursos; los nombres en archivos no bastan para atribuir autoría. |
| EQ-07 | Horario, formato y medio de entrega; requisitos de grabación | Cierre de la propuesta de cronograma. |

## Validación del programa

| ID | Acción necesaria | Situación en esta revisión |
|---|---|---|
| VAL-01 | Configurar y compilar en Linux con bibliotecas requeridas | No ejecutada: CMake no está disponible en el entorno inspeccionado. |
| VAL-02 | Registrar GPU, controlador, versión OpenGL/GLSL y dependencias | No disponibles a partir de una ejecución del programa. |
| VAL-03 | Ejecutar PT-02 a PT-14 y completar resultados observados | Casos diseñados; sin aprobación funcional. |
| VAL-04 | Comprobar proyección y tamaño de ventana | Detectados argumento angular sin conversión y ausencia de actualización al redimensionar. |
| VAL-05 | Comprobar materiales y especular | Detectada discrepancia entre samplers de C++ y GLSL; no se corrigió el código. |
| VAL-06 | Comprobar pivotes MAC, boca de railgun y alineación de personajes | Matrices documentadas; falta inspección visual. |
| VAL-07 | Validar Windows o macOS si se requiere | Sin configuración de Visual Studio o compatibilidad certificada. |

Los hallazgos VAL-04 y VAL-05 no autorizan cambios al programa en esta tarea. Deben contrastarse en ejecución y, si el equipo decide corregirlos después, actualizar documentación y versión de referencia.

## Evidencia visual y referencias

- CAP-01 a CAP-05: vistas de uso, animaciones y consola del manual de usuario.
- C-01 a C-07: capturas comparables de los siete elementos de la plaza.
- F-05 y F-06: fotografías de banca y reja con ubicación y derechos identificados.
- F-04: confirmar referencia de vegetación del sector representado; la fuente localizada es contextual.
- F-07: aclarar licencia o seleccionar otra fotografía del memorial con ficha consistente.
- Confirmar que `barda` corresponde a las rejas solicitadas y que `lapida` representa el memorial de referencia.
- Completar comparación de silueta, proporciones, detalle y materiales después de obtener imágenes equivalentes.

## Presentación de documentos

Los archivos draw.io y Mermaid son editables. Se comprobó que el XML de draw.io se puede analizar, pero no se abrió en diagrams.net. Los diagramas Mermaid no se renderizaron con un motor local; revisar la presentación final en el visor elegido. No se generaron DOCX: no se encontró Pandoc o LibreOffice y no se instalaron dependencias para convertirlos. La versión Markdown permanece como fuente editable.
