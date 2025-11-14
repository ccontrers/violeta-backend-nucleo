---
mode: agent
---

Tarea:
Generar un documento en formato Markdown en la ruta: `docs/proceso_legacy_catalogo_proveedores.md`.

Objetivo principal:
Describir de forma clara, estructurada y exhaustiva el funcionamiento del catálogo legado de proveedores (interfaz y flujo de uso) orientado a usuarios no técnicos (negocio, analistas de procesos, control interno). Debe enfocarse en qué hace el catálogo y cómo se usan los datos, evitando tecnicismos (no incluir nombres de clases, métodos o detalles de implementación salvo cuando sea imprescindible para justificar una regla de negocio).

Alcance (qué sí y qué no):
- Sí: campos visibles, organización por secciones/pestañas, validaciones, reglas de negocio, procesos de alta/baja/modificación/consulta, relaciones de datos relevantes, mensajes o estados funcionales importantes.
- No: detalles de código, estructuras internas de clases, explicaciones de sintaxis C++ o SQL específicas, optimizaciones técnicas.

Fuentes de información a analizar (en este orden):
1. Código de la interfaz (formularios legados):
   - `cpp/FormCatalogoProveedores.cpp`
   - `cpp/FormCatalogoProveedores.h`
   - `cpp/FormCatalogoProveedores.dfm`
2. Funciones backend involucradas en operaciones clave (solo para extraer reglas funcionales):
   - `ServidorCatalogos::GrabaProveedor`
   - `ServidorCatalogos::BajaProveedor`
   - `ServidorCatalogos::ConsultaProveedor`
   Identificar para cada una (expresado en lenguaje funcional):
   - Datos que retorna (describirlos como conceptos de negocio).
   - Efectos sobre el estado (alta, baja lógica, actualización, consulta).
   - Validaciones o precondiciones detectables.
3. Documentación heredada: `docs/claseslegadas/*.md` (solo lo que aporte significado funcional).
4. Definición de tablas en `db/*.sql`, con foco en:
   - Tabla central: `proveedores` (archivo `db/proveedores.sql`).
   - Tablas relacionadas vía llaves foráneas (explicar de manera conceptual qué complementan: domicilios, condiciones de pago, clasificaciones, estatus, etc.).

Metodología sugerida para estructurar la información:
1. Identificar las secciones o pestañas (TTabSheets) del formulario. Si no existen pestañas, definir bloques lógicos por agrupación visual (paneles, group boxes, grids, etc.).
2. Para cada sección, listar los campos visibles y agruparlos por propósito (identificación, contacto, condiciones comerciales, fiscales, seguimiento, auditoría, otros).
3. Para cada campo, documentar (cuando aplique): nombre funcional, descripción, tipo lógico (texto, numérico, fecha, catálogo, bandera), obligatoriedad, validaciones (rango, formato, unicidad, relaciones), origen/destino de datos y impacto en procesos.
4. Describir los procesos (alta, baja, modificación, consulta) indicando pasos funcionales, decisiones y reglas clave.
5. Incluir un diagrama textual simple (opcional) de flujo para cada proceso si aporta claridad.
6. Incorporar una sección de relaciones de datos: explicar en lenguaje de negocio cómo otras tablas complementan la información del proveedor.
7. Señalar riesgos o puntos de control (ej.: campos críticos, dependencias, validaciones silenciosas, posibles inconsistencias).

Contenido mínimo obligatorio del documento:
- Título y breve introducción del propósito del catálogo.
- Resumen ejecutivo (2–4 párrafos) para lectura rápida de negocio.
- Sección "Estructura de la Interfaz" (pestañas/secciones y su propósito).
- Sección "Campos y Validaciones" organizada por sección.
- Sección "Procesos" (alta, baja, modificación, consulta) con pasos numerados.
- Sección "Relaciones de Datos" (tablas clave y qué aportan).
- Sección "Reglas de Negocio Destacadas" (bullets claras).
- Sección "Riesgos y Controles Recomendados".
- Glosario (cuando haya términos potencialmente ambiguos).

Estilo redaccional:
- Lenguaje claro, en español neutro, orientado a procesos y negocio.
- Evitar tecnicismos (si se menciona un término técnico, añadir breve explicación entre paréntesis).
- Frases cortas; usar listas para mejorar legibilidad.
- No incluir fragmentos de código ni SQL literal (parafrasear la lógica).

Criterios de éxito (el entregable se considerará completo si):
- Cubre todas las secciones mínimas listadas.
- No aparecen nombres crudos de métodos/clases salvo en una sección opcional de referencias técnicas (si se agrega).
- Cada proceso tiene pasos claros y criterio de finalización.
- Cada campo crítico (identificación fiscal, razón social, estatus, etc.) tiene sus validaciones o reglas descritas.
- Las relaciones de datos relevantes están explicadas sin depender de conocer el modelo físico.
- No hay faltas ortográficas notorias ni inconsistencias terminológicas (ej.: usar siempre "proveedor", no alternar con "supplier").

Suposiciones y límites:
- Si alguna validación no es deducible de forma razonable, indicar "No inferible con la evidencia disponible" en lugar de inventar.
- Si se detectan campos no utilizados o duplicados, mencionarlos en una subsección "Campos Observados con Uso Dudoso".

Formato final:
- Archivo: `docs/proceso_legacy_catalogo_proveedores.md` (codificación UTF-8, encabezados Markdown estándar `#`, `##`, etc.).
- Evitar tablas Markdown demasiado anchas; preferir listas estructuradas para campos (solo usar tablas si realmente mejora la claridad).

Salida:
Generar únicamente el contenido del archivo solicitado (no envolver en explicaciones adicionales). Si falta información esencial para completar una sección, incluir un bloque clearly marcado como "INFORMACIÓN INCOMPLETA" con los puntos faltantes.

Nota de confidencialidad (incluir al final):
"Este documento describe un sistema legado y se entrega exclusivamente para fines de análisis funcional y documentación interna. No debe difundirse externamente sin autorización." 