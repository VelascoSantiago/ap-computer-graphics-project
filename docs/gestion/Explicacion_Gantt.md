# Explicación del cronograma

El [Gantt editable en Mermaid](Gantt_Proyecto.mmd) organiza una **propuesta de cierre**, no reconstruye el calendario real de desarrollo. El [CSV](Cronograma.csv) conserva también las actividades históricas sin fecha, para que el equipo pueda completarlas sin convertir estimaciones en hechos.

## Evidencia parcial del historial

Se consultó `git log --format='%h | %aI | %s' -- proyecto_lab` y el resumen de cambios de cada commit. Las marcas siguientes son fechas de autor con zona −06:00; no se interpretan como horas de una sesión de trabajo.

| Commit | Registro | Evidencia que aporta | Lo que no permite concluir |
|---|---|---|---|
| `3563e24` | 2026-11-05 00:53:39 −06:00 · `proyecto: started` | Incorporación de `proyecto_lab`, plaza, clases y shaders, junto con recursos que después fueron sustituidos. | Fecha inicial de modelado, duración de preparación o pruebas superadas. |
| `36edb93` | 2026-11-05 23:46:34 −06:00 · `project: animations` | Modificación de `main.cpp` e incorporación de CUAU, SENTINEL, railgun y MAC; retiro de modelos anteriores. | Que todas las animaciones se hayan desarrollado ese día o que se haya concluido la integración visual. |

No se asignaron fechas históricas de inicio o término a partir de esos registros. También falta conocer si hubo trabajo fuera de Git, importación de modelos ya preparados o cambios no registrados antes de estos commits.

## Lectura de las barras

Las barras del 6 al 8 de noviembre son ventanas propuestas para revisar planeación, recursos, programación, integración, animaciones, pruebas y documentos. No representan que esas actividades originales deban realizarse desde cero ni que ya estén terminadas. Sus duraciones son días naturales de calendario, sin estimación de horas de esfuerzo y sin responsables asignados.

Para actividades con duración se usa intervalo con fin exclusivo: por ejemplo, del 6 al 8 abarca los días 6 y 7. Los hitos tienen duración cero y fecha puntual. No se excluyen fines de semana porque los hitos previstos incluyen sábado y domingo. Las actividades pueden coincidir, pero el diagrama no acredita disponibilidad de personas para ejecutarlas en paralelo.

## Hitos proporcionados

| Fecha prevista | Hito | Condición de preparación propuesta |
|---|---|---|
| Sábado 7 de noviembre de 2026 | Borrador | Manuales y diagramas disponibles para revisión, con pendientes visibles. |
| Domingo 8 de noviembre de 2026 | Versión final y grabación | Incorporar correcciones, resultados de pruebas y capturas que el equipo haya reunido. |
| Lunes 9 de noviembre de 2026 | Entrega prevista | Revisar archivos y requisitos de recepción de la asignatura. |

Las condiciones de preparación son propuestas documentales; el usuario proporcionó los hitos, pero no horarios ni un mecanismo formal de aceptación. No se marcó ninguna barra como `done`.

## Datos que faltan

El equipo debe indicar fechas reales, responsables y evidencia de planeación, modelado, programación, integración y pruebas; confirmar la fecha académica de portada, y precisar el horario y medio de entrega. Al actualizar el CSV deben conservarse separados el historial confirmado y la propuesta. Si se desea representar el historial en el Gantt, hacerlo únicamente después de completar esas fechas.
