# ClassServidorReportes

## Resumen General
`ServidorReportes` centraliza la generación de reportes transaccionales, analíticos y operativos: ventas (por factura, artículo, cliente, kits, tendencias, clasificaciones), compras, notas de crédito/débito, existencias, costos, movimientos de almacén, inventarios, flujo de efectivo, prepagos, pinpad, cartas porte, bitácoras (modificaciones, pedidos, CFDi, transacciones billeto, auditorías), métricas e integraciones (envío de correo, condiciones comerciales). Cada método arma dinámicamente un bloque SQL (frecuente uso de tablas temporales, variables de sesión, agregaciones condicionales, expresiones CASE) en función de un buffer de parámetros lineal y devuelve uno o más result sets en un `RespuestaServidor`.

La clase es extensa (decenas de miles de líneas) y actúa como capa de orquestación SQL avanzada?mezcla lógica de: filtrado, derivación de métricas, expansión de impuestos dinámicos, pivot de montos, composición de joins opcionales, y construcción de secciones adicionales (XML CFDI, resumenes, detalles). 

## Responsabilidades Clave
- Interpretar parámetros de filtrado (rangos de fecha/hora, empresa, sucursal, vendedor, cliente, clasificaciones, tipo de factura, estatus, cancelaciones, cortes, medios de facturación, origen de venta, formas de pago, inclusión de devoluciones, impuestos específicos, etc.).
- Generar consultas de agregación y detalle usando múltiples condiciones compuestas.
- Adaptar columnas dinámicas (impuestos variables): construcción de expresiones sumatorias a partir de una consulta previa de catálogo de impuestos.
- Ejecutar secuencias de SQL (a veces multi?paso: creación de tablas temporales, inserciones intermedias, cálculos finales) y volcar resultados serializados.
- Integrar elementos externos (carga de cadenas originales CFDI, XML base64, correos electrónicos, bitácoras).

## Patrón General de un Método (Ejemplo: `EjecutaRepVentasXFactura`)
1. Desempaca ~40 parámetros secuenciales en `AnsiString` (horarios, fechas, filtros de cliente, vendedor, clasificaciones, impuestos, opciones de incluir XML, etc.).
2. Consulta catálogo de impuestos para generar dinámicamente segmentos como:
   - Campos de valor por impuesto (`sum(@impcolXXX:=...) as impcolXXX`).
   - Expresión sumatoria total (`@impcol001+@impcol002+...`).
3. Construye condiciones condicionales (cada filtro añade cláusula `and ...`).
4. Maneja canceladas vs no canceladas y contabilizadas con conjuntos de banderas (`condicion_canceladas`, `condicion_contabilizadas`, `fechas_canceladas`).
5. Añade joins opcionales (clientes, ecommerce, cfdiweb, kits, formas de pago, XML CFDI) según parámetros.
6. Ejecuta la consulta final con LIMIT implícito o sin límite (depende del reporte), depositando resultado(s) en el buffer.

## Categorías de Reportes (No Exhaustivo)
- Ventas: por factura, artículo, cliente, kits, mensual, rotación, tendencias, precios diferidos, pedidos vs ventas, utilidad, global IEPS.
- Compras: por factura, proveedores, bitácoras, cambios de proveedor principal, registros de compras.
- Notas de crédito / cargo: clientes, proveedores, gastos (por nota y por artículo).
- Existencias e Inventario: existencias simple/óptima/sucursal, costo de existencias, inventarios capturados, comparativos, diferencias de peso/volumen, articulos sin existencias.
- Movimientos de Almacén: por movimiento, por artículo, costos, detalle extendido, costo movimientos externos.
- Cobranzas y Pagos: pagos clientes, proveedores, prepagos, pólizas de cobranza/gastos, transacciones billeto, flujo de efectivo (detalle y concentrado).
- Comercio Electrónico: pedidos, artículos relacionados, bitácoras ecommerce, articulos supervisados, transacciones pinpad.
- Fiscal/CFDI: errores timbrado, CFD, cartas porte, pólizas contables masivas, XML y cadena original opcional.
- Logística / Embarques / Surtido: embarques (general y desglosado), cartas porte por embarque, surtido anaqueles / auditoría, recepciones y devoluciones, modificaciones almacén, ubicación por producto.
- Auditorías y Bitácoras: modificaciones clave, cambios programados, bitácora servidor, empleados, solicitudes de notas de crédito, ventas TPV, configuraciones de precios.

## Dinámica de Impuestos
- Consulta previa a catálogo `impuestos` + `tiposdeimpuestos` para construir columnas dinámicas.
- Usa variables de usuario (@impuesto1..@impuesto4) y mapeos condicionales a cada renglón para sumarizar.
- Facilidad para extender impuestos requiere mantener convención de nombres y orden.

## Manejo de Parámetros y Filtros
Parámetros típicos:
- Fechas/hora: `fecha_inicial`, `fecha_final`, `hora_inicial`, `hora_final`.
- Identificadores: empresa, sucursal, referencia de venta, vendedor, cliente.
- Categorizaciones: clasif1/2/3, sector, localidad, giro, canal.
- Estatus: canceladas, contabilizadas, cortes de alta/cancelación, kits, medios de facturación.
- Tributación: tipo de impuesto, impuesto particular específico.
- Origen / ecommerce: ordenEcommerce, origenVentas, inclusión de devoluciones.
- Output Extra: incluir XML CFDI, reporte gráfico, detalle vs resumen.

Cada filtro se traduce a cadenas `condicion_*` ensambladas en concatenación final del SQL.

## Consideraciones de Rendimiento
- Queries potencialmente pesadas (múltiples joins sobre grandes tablas transaccionales) sin particionado explícito.
- Dependencia en `AnsiString` y `sprintf` para concatenación (riesgo de overhead y complejidad de mantenimiento).
- Falta de caching: cada invocación recompone SQL desde cero.
- Posible uso de tablas temporales (en otros métodos) que implica I/O adicional en disco si no está en memoria.

## Riesgos y Debilidades
1. Superficie grande de SQL injection: parámetros se interpolan sin escape/cotejo (misma vulnerabilidad estructural que en otras capas).
2. Complejidad cognitiva: métodos con más de ~200 líneas difíciles de depurar o ampliar.
3. Acoplamiento al esquema actual (renombrar campos rompe múltiples métodos simultáneamente).
4. Falta de paginación/batch streaming en reportes de alto volumen ? riesgo de buffers enormes.
5. Dinámica de impuestos frágil si se agregan más de cuatro órdenes/impuestos simultáneos.
6. Lógica de canceladas/contabilizadas dispersa; se podría normalizar en una función reutilizable.
7. Ausencia de `EXPLAIN`/profiling integrado para detectar regressiones de performance.

## Recomendaciones de Refactor y Mejora
- Introducir una capa de construcción de consultas parametrizadas (prepared statements) con bind seguro.
- Segmentar por dominios (VentasReportService, ComprasReportService, InventarioReportService...).
- Añadir capa DSL interna o generador (templating) para condiciones repetidas (canceladas, contabilizadas, rango fecha/hora, clasificaciones).
- Implementar soporte de paginación y export incremental (streaming) para grandes resultados.
- Incorporar control de tiempo máximo y monitoreo (logging de duración / filas) por reporte.
- Sustituir variables de sesión por CTEs / subconsultas claras cuando sea posible.
- Centralizar lógica de impuestos en vista o procedimiento almacenado.

## Ejemplo Simplificado (Invocación Conceptual)
```
params.add("1");         // filtrar por hora
params.add("08:00:00");  // hora inicial
params.add("18:59:59");  // hora final
params.add("2025-01-01"); // fecha inicial
params.add("2025-01-31"); // fecha final
// ... resto de filtros vacíos con " "
send(ID_EJE_REPVENTAS_X_FACTURA, params);
// Respuesta: dataset con ventas filtradas por hora y rango de fechas
```

## Contrato Simplificado
- Entrada: buffer lineal de strings interpretadas en orden fijo por cada reporte.
- Salida: uno o más result sets tabulares concatenados en el buffer.
- Errores: Excepciones en construcción o ejecución SQL (no se detalla formato de error estandarizado).

## Edge Cases
- Parámetros vacíos en masa ? consultas sin restricciones (posible sobrecarga).
- Inclusión de XML (`incluir_xml=1`) amplifica tamaño y latencia; requiere considerar compresión.
- Cambio en definiciones de impuestos sin actualizar lógica de mapeo ? columnas en cero o inconsistentes.
- Rangos de fechas extensos + joins costosos pueden exceder tamaños de buffer o agotar memoria.

## Observaciones de Integración
- Comparte infraestructura de buffers y extracción de parámetros con `ServidorCatalogos` y `ServidorBusquedas`.
- Se apoya en `ServidorVioleta` para la ejecución bruta y en `FuncionesGenericas` para parse.
- Reutiliza taxonomías y catálogos generados previamente (impuestos, clasificaciones, clientes, productos) sin caching local.

## Seguridad y Cumplimiento
- Recomendable implementar: sanitización de entradas, límites de filas configurables, lista blanca de columnas seleccionables, logging de auditoría (quién corre qué reporte con qué filtros).
- XML CFDI y datos fiscales requieren cifrado en tránsito y posible anonimización en entornos no productivos.

## Ruta de Migración Propuesta (Evolución)
1. Extraer builder de condiciones (Strategy por dominio).
2. Introducir repositorio/DAO con prepared statements.
3. Añadir capa de autorización granular (reporte vs rol/privilegios).
4. Implementar caching de catálogos de impuestos y clasificaciones.
5. Instrumentar métricas (tiempo, filas) y alertas de queries lentas.

---
© Documentación técnica generada automáticamente.
