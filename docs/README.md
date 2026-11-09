# Documentación de T68 Sci-Fi Tribute

Documentación de la implementación de Tlatelolco en `proyecto_lab`, Facultad de Ingeniería, UNAM. Base: commit `36edb9352e6053eb2f9f0341e9351ab7027c6a80`. Inspección: 6 de noviembre de 2026. Nombre del proyecto provisional; datos académicos pendientes.

## Documentos

| Archivo | Contenido y estado |
|---|---|
| [Resumen técnico](Resumen_Tecnico.md) | Inventario previo a la redacción, fuentes inspeccionadas y discrepancias confirmadas. |
| [Manual de usuario](Manual_de_Usuario.md) | Requisitos, instalación prevista, controles, animaciones, diagnóstico y espacios para capturas. Redacción completa; compilación y uso pendientes de validar. |
| [Manual técnico](Manual_Tecnico.md) | Objetivos, alcance, limitaciones, metodología propuesta, arquitectura, funciones, matemáticas, pipeline y pruebas. Análisis estático completo; matriz de ejecución sin resultados inventados. |
| [Pendientes](PENDIENTES.md) | Datos del equipo, pruebas, imágenes y derechos que faltan para cerrar la entrega. |
| [Flujo draw.io](diagramas/flujo_software.drawio) | Nodos y conectores editables; no es una imagen incrustada. |
| [Flujo Mermaid](diagramas/flujo_software.mmd) · [Vista y explicación](diagramas/README.md) | Secuencia real, ramas de error y ciclo de renderizado. |
| [Gantt](gestion/Gantt_Proyecto.mmd) | Ventanas propuestas de revisión y los tres hitos proporcionados. |
| [Cronograma CSV](gestion/Cronograma.csv) · [Explicación](gestion/Explicacion_Gantt.md) | Fechas pendientes separadas de propuesta e historial de commits. |
| [Comparativa de Tlatelolco](referencias/Comparativa_Tlatelolco.md) | Siete elementos, fuentes fotográficas, créditos, datos geométricos y comparación visual pendiente. |

## Criterios de lectura

**Confirmado en código** significa que existe una declaración, operación o recurso identificable en los archivos locales. **Pendiente de validación** indica una instrucción o resultado esperado que no se ha observado ejecutando el programa. **Propuesta** identifica la metodología o las ventanas de trabajo que no se acreditan como historial. Los espacios **[PENDIENTE]** requieren datos o imágenes reales.

Las tres animaciones y los controles indicados por el equipo existen. SENTINEL se representa con tres instancias; su disparo combina barrido, retroceso, vibración y una sola luz puntual compartida. No hay proyectiles ni impactos. Se registraron diferencias entre desactivar cada animación, la inconsistencia de nombres de samplers, el brillo sobrescrito y la perspectiva con un ángulo sin conversión. Los documentos describen estos hallazgos sin modificar su implementación.

## Revisión y límites de la evidencia

La revisión cotejó los controles con callbacks y movimiento, las ecuaciones con `Animation()` y los bloques de dibujo, las rutas con los recursos, el target con CMake y las fechas con `git log`. Se revisaron redundancias, lenguaje, jerarquía de encabezados y separación entre propuesta y resultado. Se verificaron enlaces relativos locales, estructura CSV, XML editable y correspondencia de nodos entre los dos formatos de flujo mediante comprobaciones documentales.

Las comprobaciones estructurales encontraron 29 nodos de flujo y 31 conectores, además de la nota explicativa del XML; el cronograma contiene 16 registros. Se confirmó que el 7, 8 y 9 de noviembre de 2026 corresponden a sábado, domingo y lunes. La revisión de los seis OBJ cargados encontró sus seis MTL y las 15 referencias de imagen existentes en disco. Esta comprobación de rutas no demuestra que Assimp o SOIL2 los decodifiquen correctamente. Git no mostró diferencias en archivos originales del proyecto.

No fue posible compilar: `cmake --version` devolvió comando no disponible. No se ejecutaron ventana, navegación o animaciones; tampoco se generaron capturas o mediciones. El renderizado de Mermaid y la apertura visual de draw.io quedan pendientes. Las búsquedas fotográficas conservaron URLs y créditos; no se atribuyó licencia donde no se confirmó.

Todos los archivos creados se encuentran en `docs/`. No se modificaron fuentes, shaders, recursos, CMake o dependencias; no se hizo push ni se publicó documentación. No se generaron DOCX porque no se encontraron las herramientas de conversión consultadas. Antes de entregar, completar [PENDIENTES](PENDIENTES.md) y repetir la revisión contra la versión que efectivamente se presente.
