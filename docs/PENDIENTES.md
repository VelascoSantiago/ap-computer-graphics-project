# Datos y verificaciones pendientes

## Información del equipo
| ID | Dato requerido | Dónde se usa |
|---|---|---|
| EQ-01 | Roles — **resuelto** (2026-11-09) | Sección 5.4 del manual técnico |
| EQ-02 | Víctor (equipo de teoría) acreditado por nombre; apellidos opcionales | Créditos |
| EQ-03 | Mención de asistentes de IA — **resuelto** (Gemini, ChatGPT, Claude) | Manual técnico 5.4 y 17 |
| EQ-04 | Procedencia y licencias de texturas y modelos | Manual técnico 13.4 y 17 |
| EQ-05 | Responsables de cada fase del Gantt y fecha de video y exposición | Gantt |

## Validación del programa
| ID | Acción | Estado |
|---|---|---|
| VAL-01 | Compilar desde carpeta limpia en Linux | **Hecha** (sin errores, 3 advertencias) |
| VAL-02 | Recorrido completo de las 10 escenas | **Hecha** (~96 s) |
| VAL-03 | Medir fotogramas por segundo y probar en otras GPU | Pendiente |
| VAL-04 | Ejecutable de Windows (`.exe`) y carpeta `Ejecutable/` sin `ajustes_escenas.txt` | **Hecha** (funciona en Windows 11) |
| VAL-05 | Probar el ejecutable copiado a una carpeta limpia | **Hecha** |

## Evidencia visual
- CAP-01 a CAP-05 (capturas del manual de usuario): **hechas**.
- C-01 a C-07: capturas del modelo **hechas** (`referencias/capturas/`, con `--sinhud`). Falta que el equipo redacte la comparación visual contra las fotos (R1).
- F-05, F-06, F-07: fotos de banca, reja y memorial con derechos identificados.

## Entrega
- README del repositorio con «Registro de avances».
- Commits de lo nuevo (docs, UI, main.cpp) y push.
- Video explicativo y ensayo de la exposición (máx. 5 min).
- Portada: corregir «grafica» por «gráfica».
