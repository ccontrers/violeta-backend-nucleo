# Proceso de Búsqueda de Proveedores (Sistema Legado)

## 1. Objetivo del proceso
Permitir al usuario localizar rápidamente un proveedor existente para **consultarlo** o **seleccionarlo** como dato de entrada en otros procesos (compras, órdenes, configuración). El enfoque es puramente de **localización y selección**: no se edita información aquí. El formulario funciona como un cuadro modal de apoyo.

## 2. Alcance
Incluye el uso de la pantalla "Búsqueda de proveedores" del sistema legado (aplicación de escritorio C++ Builder / VCL). No cubre el alta, modificación ni baja de proveedores (esos ocurren en módulos de catálogo). Aplica a todos los usuarios con permisos para operaciones que requieren seleccionar un proveedor: compras, registros de cuentas por pagar, órdenes, cotizaciones, etc.

## 3. Estructura general de la pantalla
La ventana presenta:
- Título: "Búsqueda de proveedores".
- Pestañas de criterios (una sola activa a la vez):
  1. "Por razón social"
  2. "Por RFC"
  3. "Por nombre del representante legal"
  4. "Por clave" (código interno del proveedor)
- Controles de entrada según la pestaña (un campo de texto principal + botón "Buscar").
- Filtros globales adicionales:
  - Checkbox: "Mostrar inactivos" (por defecto NO marcado → sólo activos).
  - Filtros de tipo de proveedor (en cada pestaña):
    - "Mostrar solo proveedor de Gastos"
    - "Mostrar solo proveedor de Mercancías"
- Sección de resultados: etiqueta "Resultados" + una cuadrícula (tabla) donde se listan coincidencias.
- Botones de acción al pie:
  - "Seleccionar" (confirma y devuelve el proveedor elegido)
  - "Cancelar" (cierra sin selección)

## 4. Pestañas y lógica de criterio principal
| Pestaña | Campo que el usuario escribe | Qué representa | Tipo de coincidencia | Orden base de resultados |
|---------|------------------------------|----------------|----------------------|---------------------------|
| Por razón social | Razón social | Nombre fiscal registrado | Contiene (parcial) | Razón social |
| Por RFC | RFC | Registro Federal de Contribuyentes | Contiene (parcial) | RFC, Razón social |
| Por nombre del representante legal | Representante legal | Nombre del apoderado / firmante | Contiene (parcial) | Razón social |
| Por clave | Clave de proveedor | Identificador interno (código) | Contiene (parcial) | RFC, Razón social |

Notas:
- En todos los casos se permite escribir sólo una parte inicial o interna del texto (el sistema rodea internamente con comodines). 
- Si el usuario no escribe nada y presiona "Buscar" se muestra un mensaje (legado) solicitando capturar algo antes de lanzar la búsqueda.
- Cada búsqueda reemplaza los resultados previos; no hay acumulación ni paginación visible (se limita internamente a un máximo predefinido de filas).

## 5. Filtros disponibles
### 5.1 Estado (activos / inactivos)
- Checkbox: "Mostrar inactivos".
- Desmarcado (valor por defecto): sólo proveedores activos.
- Marcado: incluye también los inactivos.
- El sistema legado realmente aplica el filtro como: si NO está marcado, agrega la condición de actividad; al marcar se elimina la restricción.

### 5.2 Tipo de proveedor (Gastos vs Mercancías)
En cada pestaña aparecen dos checkboxes independientes pero **mutuamente excluyentes lógicamente** (marcar uno desmarca el otro):
- "Mostrar solo proveedor de Gastos"
- "Mostrar solo proveedor de Mercancías"

Comportamiento:
- Si ninguno está marcado: se muestran ambos tipos (comportamiento por defecto).
- Si se marca uno: el listado se limita sólo a ese tipo.
- Marcar uno desactiva el otro para evitar resultados vacíos o confusos.

Uso esperado:
- Gasto: proveedores de servicios generales, facturación de gastos operativos.
- Mercancía: proveedores que abastecen productos / inventario comercializable.

## 6. Cuadrícula de resultados
La tabla muestra una fila por proveedor encontrado. Columnas visibles (orden del sistema legado):
1. Clave (código interno)
2. Razón social
3. Representante legal
4. RFC
5. Estado (abreviado: código de la entidad federativa)
6. Localidad (ciudad / población)
7. Calle
8. Colonia
9. Redondeo (indicador de política de redondeo; ver nota abajo)
10. Prov. Gastos ("Si" / "No")
11. Prov. Mercancía ("Si" / "No")

(Existe internamente un campo de vigencia / días de vigencia, pero no se muestra en esta pantalla para la selección.)

### 6.1 Interpretación de campos clave
- Clave: Identificador maestro; se utiliza para referencias en otros procesos.
- Razón social: Nombre fiscal oficial.
- Representante legal: Persona autorizada para actos formales (puede estar vacío si no fue capturado).
- RFC: Identificador fiscal; puede contener homoclave.
- Estado / Localidad / Calle / Colonia: Segmentos principales del domicilio. Sirven para reconocimiento rápido; no son editables aquí.
- Redondeo: Corresponde al campo interno `redondeocptecho`. Indica si el proveedor aplica una política de redondeo en ciertos cálculos (ej. comprobantes). No se modifica aquí; sólo orienta al usuario.
- Prov. Gastos / Prov. Mercancía: Clasificación operativa usada para filtrar y dirigir procesos posteriores (órdenes, compras de inventario vs servicios, etc.).

### 6.2 Estilo visual
- El listado adopta el esquema visual estándar del sistema legado (sin resaltado dinámico avanzado).
- No hay ordenamiento interactivo por encabezado en tiempo de ejecución (el orden lo fija la consulta original según el criterio principal).

## 7. Flujo de uso típico
1. Abrir la pantalla desde un módulo que requiere seleccionar proveedor (p.ej. captura de compra).
2. Elegir la pestaña adecuada según el dato disponible (ej. si se conoce el RFC, usar "Por RFC").
3. Escribir el fragmento del texto a buscar (recomendado mínimo 3 caracteres para afinar resultados).
4. (Opcional) Marcar filtro de tipo de proveedor si se desea acotar (Gastos o Mercancías).
5. (Opcional) Marcar "Mostrar inactivos" si se requiere consultar un proveedor dado de baja.
6. Presionar "Buscar".
7. Revisar la lista y hacer clic sobre la fila deseada (la fila queda seleccionada internamente).
8. Pulsar "Seleccionar" para devolver el proveedor a la operación que originó la búsqueda.
9. Si se cancela, no se devuelve ningún valor.

## 8. Validaciones y mensajes (legado)
- Búsqueda vacía: Se impide ejecutar la consulta sin texto (evita traer un universo amplio innecesario).
- Límite máximo: El sistema limita internamente el número de resultados (para desempeño); si no aparece el proveedor esperado, se sugiere refinar el texto.
- Exclusión de filtros: Al activar un filtro de tipo (Gastos / Mercancías) se desactiva el otro para evitar combinaciones sin sentido.

## 9. Reglas y consideraciones de negocio
| Situación | Comportamiento | Implicación para el usuario |
|-----------|----------------|-----------------------------|
| Proveedor inactivo | No aparece salvo que se marque "Mostrar inactivos" | Evita selecciones accidentales |
| Proveedor sin representante legal | La columna queda vacía | No invalida la selección |
| RFC duplicado (poco común) | Ambos registros pueden mostrarse | Requiere validar por razón social |
| Texto muy corto | Puede devolver demasiadas filas | Refinar para agilizar |
| Clasificación incorrecta (Gastos vs Mercancía) | Filtrado puede ocultarlo | Revisar mantenimiento maestro |

## 10. Riesgos / Dolencias actuales (legado)
- Experiencia limitada: No hay autocompletado ni resaltado del fragmento buscado.
- Falta de paginación: El límite silencioso puede ocultar coincidencias posteriores.
- Filtros distribuidos (uno por pestaña) generan redundancia visual (los checkboxes se repiten en cada tab).
- Campo "Redondeo" carece de explicación contextual para el usuario final.
- No existe indicador visual de inactividad (más allá de depender del filtro global).

## 11. Oportunidades de mejora (para el sistema nuevo)
1. Unificar criterios en una sola barra con selección de tipo (dropdown) o búsqueda unificada multi-campo.
2. Implementar búsqueda incremental (min 2–3 caracteres) con sugerencias.
3. Añadir paginación o scroll virtual para grandes catálogos.
4. Mostrar estatus (Activo/Inactivo) con badge de color en la fila.
5. Consolidar filtros de tipo proveedor en un único control (radio group o multiselect) fuera de las pestañas.
6. Incorporar explicación contextual (tooltip) para política de redondeo.
7. Registrar métricas de uso (qué criterio se busca más) para optimizar UX.
8. Permitir ordenar columnas dinámicamente en el frontend.

## 12. Datos mínimos recomendados para una migración limpia
| Dato | Fuente en tabla `proveedores` | Necesario para selección | Prioridad |
|------|-------------------------------|--------------------------|-----------|
| Clave | proveedor | Clave de retorno | Alta |
| Razón social | razonsocial | Identificación principal | Alta |
| RFC | rfc | Validación fiscal | Alta |
| Representante legal | replegal | Referencia secundaria | Media |
| Estado | estado (FK) | Ubicación | Media |
| Localidad | localidad | Ubicación | Media |
| Colonia | colonia | Ubicación | Baja |
| Calle | calle | Ubicación | Baja |
| Tipo proveedor (Gastos/Mercancía) | provgastos / provmercancia | Filtrado | Alta |
| Activo | activo | Filtrado y validación | Alta |
| Redondeo | redondeocptecho | Regla especial | Baja |

## 13. Definición de "selección" en el proceso
El proceso concluye exitosamente cuando el usuario pulsa "Seleccionar" con una fila activa. El sistema devuelve internamente la **clave del proveedor** (y en algunos casos asociando también razón social para poblar campos visibles). Si la ventana se cierra o se pulsa "Cancelar", el proceso es nulo (sin cambios en la pantalla origen).

## 14. Métrica sugerida (futuro)
- Tiempo medio desde apertura hasta selección.
- Número de intentos de búsqueda por sesión de apertura.
- Porcentaje de búsquedas sin resultados.
- Distribución de criterios usados.

## 15. Resumen ejecutivo
La pantalla de búsqueda de proveedores del sistema legado ofrece un mecanismo funcional pero limitado, basado en pestañas y filtros repetidos, para localizar proveedores por cuatro criterios aislados. La ausencia de paginación y de una búsqueda unificada reduce eficiencia, y ciertos indicadores (como redondeo o estado inactivo) carecen de claridad contextual. La migración representa una oportunidad para simplificar la experiencia en un único panel de búsqueda enriquecido, mejorar filtrados, y añadir soporte incremental y métricas de uso.

---
Documento orientado a negocio: evita detalles de implementación técnica (consultas SQL, buffers, estructuras internas) y se centra en el flujo funcional y sus oportunidades de mejora.
