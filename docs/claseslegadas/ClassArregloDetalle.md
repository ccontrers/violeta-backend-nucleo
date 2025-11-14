# ClassArregloDetalle y ElementoDetalle

## Resumen General
La clase `ArregloDetalle` gestiona un conjunto de partidas (líneas) de documentos comerciales (pedidos, facturas, notas de crédito de compra y de venta) encapsulando el cálculo de importes, descuentos, impuestos múltiples (hasta 4 por partida más IEPS de cuota), subtotales segmentados por tratamientos fiscales (tasa 0, gravado, combinaciones IVA/IEPS) y lógica de redondeo/ajuste. Cada línea se modela con la estructura/objeto `ElementoDetalle`, que concentra datos del artículo, cantidades, precios/costos unitarios (con y sin impuestos), porcentajes y tipos de impuestos, cuotas IEPS, así como valores calculados (bases e importes de impuestos compuestos). 

La clase también sincroniza estos datos con componentes visuales (grids y labels VCL), maneja inserciones/modificaciones de líneas en modos compra/venta, carga masiva desde buffers (consultas SQL empaquetadas), calcula totales desagregados/agrupados de impuestos (incluyendo desglose por tipo y combinaciones IVA+IEPS para CFDI 3.3/4.0) y soporta reglas especiales para notas de crédito (descuento general, devoluciones, desglose, disposición) y ofertas (unificación de precios de artículos similares). 

## Contexto de Uso
`ArregloDetalle` opera en escenarios diferenciados por constantes de uso (`PEDI_COMPRA`, `FACT_COMPRA`, `PEDI_VENTA`, `FACT_VENTA`, `NCRE_*`) y posición (`ARRIBA` / `ABAJO`) que afectan:
- Cómo se recalculan y ajustan redondeos de totales.
- Qué columnas del grid contienen costos vs precios.
- Si los costos/precios unitarios almacenados incluyen o no impuestos.
- Cómo se interpretan descuentos globales vs por partida.

## Principales Responsabilidades
- Insertar / modificar partidas (compra y venta) validando UI (`VerificaFrameLleno`).
- Calcular importes por partida: costo/precio total, con/ sin descuento, con/ sin impuestos.
- Calcular base e importe de cada impuesto (hasta 4) considerando encadenamiento (impuestos sobre subtotal que ya incluye anteriores) y modalidad compra/venta.
- Invertir precios/costos para obtener valores sin impuestos partiendo de un valor con impuestos (incluye lógica IEPS cuota + IVA sobre base incrementada).
- Acumular totales globales: subtotales gravados, tasa 0, descuentos, ISR retenido, totales por tipo de impuesto, totales combinados IVA+IEPS.
- Manejar redondeo configurable: modo simple vs ajuste iterativo con delta para cuadrar diferencias (< tolerancias definidas).
- Cargar partidas desde buffers (compras y ventas) traduciendo registros a elementos y populando grids.
- Aplicar descuentos globales en notas de crédito y recalcular proporcionalmente.
- Uniformizar precios de oferta entre artículos "similares" (misma clave de producto + forma de compra + precio oferta) y revertirlos si aplica.
- Calcular comisión por venta basada en porcentaje por artículo o porcentaje general del vendedor.

## Estructura de Datos (Atributos Clave)
### ElementoDetalle (por partida)
- Identificación: `CveProducto`, `CveArticulo`, `NombreProducto`, `Presentacion`, `FormaCompra`.
- Cantidad: `Cantidad`.
- Precios/Costos: `CostoUnitarioSinDescuento`, `PrecioUnitarioSinDescuento`, `PrecioUnitarioConImpuestos`, `CostoBase`.
- Descuentos: (implícitos vía `PorcentajeDescuento` del arreglo contenedor y métodos de cálculo).
- Impuestos porcentuales (hasta 4): `PorcentajeImp1..4`, `TipoImp1..4` (nombre), `CveImp1..4` (clave), `CveTipoImp1..4` (clave de tipo SAT). 
- IEPS Cuota: `IepsCuota` (monto fijo por unidad que altera base de IVA si existe).
- Otros: `ISRRetenido` (retenido acumulado, usa índice 0), `PorcentajeComision`.

### ArregloDetalle (colección)
- Arreglo de partidas: `Elemento[MAX_DETALLES]`.
- Dimensiones y control: `NumDetalles`, `UsoDelArreglo`, `PosicionArreglo`, `PorcentajeDescuento`, `PorcentajeDescuentoGeneral`.
- Totales acumulados compra: `CostoTotalSinDescuento`, `CostoTotalConDescuentoTasa0`, `CostoTotalConDescuentoGravado`, subtotales segmentados 2014 (`CostoSubtotalIVA0_2014`, etc.).
- Totales acumulados venta: `PrecioTotalSinDescuento`, `PrecioTotalConDescuentoTasa0`, `PrecioTotalConDescuentoGravado`, `PrecioSubtotalIVA*`.
- Descuentos e ISR: `DescuentoTotal`, `Total`, `SumatoriaNCredito`, `ValorOriginal`, `ValorBonificacionNCredito`.
- Redondeo / ajuste: `DeltaActual`, `modo_ajuste_en_redondeo`, tolerancias (`MAX_DIF_SOPORTADA`, `DELTA_DECIMAL`).
- Impuestos agregados: `TotalImpuestos[]`, `TiposDeImpuestos[]`, desglose detallado (`TotalImpuestosDesg[]`, `BaseImpuestosDesg[]`, `PorcImpuestosDesg[]`, `TiposDeImpuestosDesg[]`).
- Impuestos combinados IVA+IEPS: `PorcIvaImpCombinados[]`, `PorcIepsImpCombinados[]`, bases y totales (`IvaImpCombinados[]`, `IepsImpCombinados[]`, `BaseIvaImpCombinados[]`, `BaseIepsImpCombinados[]`).
- UI asociada: `GridDetallesEnUso`, `GridTotalesEnUso`, `LabelTotalOperacion`.
- Contenedores de totales para venta: `ArregloContenedorTotales[3][MAX_INF_TOTALES]`.
- Dependencias: instancia utilitaria `mFg` (FuncionesGenericas) para formato, parsing, comparaciones y mensajes.

## Flujo de Cálculo de Impuestos (por partida)
1. Se parte de costo/precio unitario sin descuento (campo base) y cantidad.
2. Se aplica descuento porcentual global (`PorcentajeDescuento`) para obtener base con descuento.
3. Para cada impuesto 1..4 se obtiene porcentaje y tipo; se calcula base incremental:
   - Compra: cada impuesto puede gravar subtotal previo + impuestos anteriores (encadenamiento).
   - Venta: lógica similar adaptada a precio.
4. Si hay IEPS cuota: se suma a la base antes de IVA (dependiendo reglas) y se separa para cálculo inverso.
5. Resultado: importe del impuesto por partida y por unidad, y acumulación en estructuras globales.

## Métodos Principales (Agrupados)
### Inserción / Modificación de Detalles
- `InsertaElementosCompra(...)` / `InsertaElementosVenta(...)`: Alta o actualización de línea. Valida datos del frame, detecta duplicados (misma clave+presentación+forma), actualiza grid sin disparar eventos. Ajusta precio/costo base sin impuestos aplicando descuento inverso si corresponde al modo pedido.
- `CambiarPrecioElementoVenta(...)`: Reemplaza precio y recalcula representación en grid, luego recalcula precio sin impuestos real.
- `ReAsignaConsecutivoEnGrid()`: Renumera filas tras eliminaciones.
- `EliminaDetalle(...)` (en código anterior a 800): Shifteo del arreglo y ajuste de grid.

### Carga desde Buffers / Exportación
- `CargaBufferEnGridYDetalleCompras(...)`, `CargaBufferEnGridYDetalleVentas(...)`: Transfiere registros (BufferRespuestas) a detalle + grid. Interpreta campos estandarizados (cantidad, multiplo, producto, impuestos...). Para notas de crédito de devolución/descuento ajusta presentación de costos/precios con impuestos.
- `CargaBufferEnArregloDetalleVentas(...)`: Sólo llena el arreglo sin grid.
- Apoyo a notas de crédito: `AplicaDescuentoGeneralNCredito(...)` genera arreglo reducido con precios/costos escalados por porcentaje de descuento global.

### Cálculo por Partida (ElementoDetalle)
- `CalculaCostoTotalSinDescuentoPorPartida()`, `CalculaPrecioTotalSinDescuentoPorPartida()`.
- `CalculaCostoTotalConDescuentoPorPartida(factorCantidad, porcDesc)` / `CalculaPrecioTotalConDescuentoPorPartida(...)`.
- `CalculaImpuestoPorPartida(indice, porcDesc)` y variantes de unidad: `CalculaImpuestoPorUnidadPorPartida`, `CalculaTotalDeImpuestosPorUnidadPorPartida`.
- `CalculaBasePorPartida(indice, porcDesc)`.
- Inversión impuestos: `CalculaCostoSinImpuestosPorUnidadPorPartida()`, `CalculaPrecioSinImpuestosPorUnidadPorPartida()`. Incluye extracción IEPS cuota previo a descomponer IVA.

### Totales Globales / Subtotales
- Compra: `RecalculaTotalesCompra()`, usa `CalculaCostoTotalSinDescuentoGeneral()`, `CalculaDescuentoTotal()`, `CalculaSubtotalTasa0()`, `CalculaSubtotalGravado()`, segmentaciones 2014 (`CalculaSubtotalIVA*_2014()`), `CalculaISRRetenido()`.
- Venta: `RecalculaTotalesVenta()` homólogo para precios y llenado de `ArregloContenedorTotales` + combinaciones IVA/IEPS.
- Apoyo: `CalculaTotalGridTotalesCompra()`, `CalculaTotalGridDetallesCompra()`, `CalculaTotalGridTotalesVenta()`, `CalculaTotalGridDetallesVenta()`.
- Impuestos agregados: `CalculaTotalDeImpuestosPorPartida()`, `ObtieneTotalImpuesto(cve, porc)`, `LlenaGridTotalesAsignado()`.
- Unitarios con impuestos: `CalculaCostoUnitarioConImpuestosPorPartida()`, `CalculaPrecioUnitarioConImpuestosPorPartida()`.

### Ofertas y Precios Similares (solo cliente)
- `ConsultarPrecioOferta(articulo, empresa)` retorna precio vigente en tabla de ofertas.
- `ConsultarPrecio(detalle, tipoPrecio, consVerVentMay)` obtiene precio base segun tablas y autorizaciones locales.
- `UniformizarOferta(...)` y `UniformizarOfertaEliminar(...)` propagan o retiran precios de oferta a artículos "similares".
- `ObtieneCantidadDeProductosSimilares(...)` (dos overloads) suma cantidades de artículos ofertados equivalentes.

### Búsqueda y Consulta
- `ObtienePosicionDeArticuloContenido(Frame)` y `ObtieneCantidadDeArticuloExistente(Frame)` / `ObtienePrecioDeArticuloExistente(Frame)` devuelven posición, cantidad o precio ya registrado del artículo.

### Validación / Interacción UI
- `VerificaFrameLleno(Frame)` centraliza validaciones de captura mostrando mensajes adecuados y focos.
- Uso extensivo de `mFg.AppMessageBox`, conversión y formato numérico.

### Comisiones
- `CalculaComisionTotalEnVenta(porcVendedor)` suma comisiones por partida usando porcentaje por artículo o el general.

## Lógica de Redondeo y Ajustes
Dos modos:
1. Modo simple (`modo_ajuste_en_redondeo==0`): se redondean totales directamente a 2 decimales.
2. Modo ajuste (`==1`): bucle iterativo ajustando subtotal gravado con `DELTA_DECIMAL` hasta que diferencia entre suma de partidas y total calculado cae dentro de tolerancia (`MAX_DIF_SOPORTADA`). Aplica condiciones diferentes según tipo de documento y posición (arriba/abajo en notas de crédito) para determinar objetivo de cuadratura (cuerpo vs valor original vs bonificación). Se protege con contador límite (lanza excepción si excede 100000 iteraciones).

## Impuestos Combinados (CFDI 3.3 / 4.0)
Se almacenan parejas (porcentaje IVA, porcentaje IEPS) y sus importes y bases agregadas para exportaciones fiscales. Se corrige base de IVA en escenarios tasa 0 con IEPS o exentos gravados solo por IEPS agregando IEPS a base antes de IVA si corresponde. IEPS cuota ajusta cálculo inverso para derivar precio/costo sin impuestos.

## Notas de Crédito
Distintos modos (`NCRE_DEVO_*`, `NCRE_DESG_*`, `NCRE_DESP_*`) afectan:
- Qué base se usa para ajustes y comparaciones en redondeo.
- Cómo se presentan costos/precios (con impuestos ya aplicados vs sin impuestos).
- Aplicación de descuentos generales proporcionalmente (`AplicaDescuentoGeneralNCredito`).

## Interacción con IEPS Cuota
Cuando `IepsCuota>0` por unidad:
- Se suma a la base previa antes de IVA (dependiendo de reglas en métodos de inversión).
- En cálculo inverso se resta primero la cuota para luego dividir por (1+IVA%) evitando distorsión.

## Ejemplo de Flujo (Venta)
1. Usuario captura artículo y modo de venta: `InsertaElementosVenta` valida y agrega linea.
2. Se formatea precio unitario mostrado (incluyendo impuestos al principio, luego se recalcula base sin impuestos para cálculo interno con descuento global).
3. Al terminar la captura o cambiar descuento se llama `RecalculaTotalesVenta` que:
   - Recorre líneas, acumula impuestos por tipo y por porcentaje.
   - Genera combinaciones IVA/IEPS.
   - Calcula subtotales tasa 0 / gravado.
   - Ajusta redondeo si se configuró modo con ajuste.
   - Llama `LlenaGridTotalesAsignado` para reflejar totales en UI.
4. Label de total se actualiza con suma de partidas.

## Posibles Riesgos y Observaciones
- Uso de arreglos fijos (`MAX_DETALLES`, `MAX_IMPUESTOS`) sin comprobación robusta de overflow antes de incrementar `NumDetalles`.
- Repetición de lógica de inserción (compra vs venta) podría factorizarse para reducir mantenimiento.
- Conversión y formato repetidos; podría centralizarse con helpers inline o plantillas.
- Bucle de ajuste de redondeo potencialmente costoso (hasta 100k iteraciones) en casos extremos; se podría acotar mejor.
- Mezcla de responsabilidades UI (grids/labels) con lógica de negocio dificulta test unitario aislado.

## Mejores Prácticas / Mejoras Futuras
- Separar capa de presentación (adapters para grids) de la capa de dominio (clases puras de cálculo) para pruebas automatizadas.
- Reemplazar arreglos C estáticos por `std::vector` con verificación de límites y excepciones controladas.
- Introducir tipos fuertes para monedas e impuestos (evitar mezclar double con cantidades tributarias -> riesgo de precisión binaria). Evaluar `decimal` emulado (enteros scaled). 
- Implementar estrategia de redondeo configurable (bankers, half-up) centralizada.
- Estandarizar nombres (mezcla de español/inglés y capitalización) para consistencia.
- Documentar formalmente contrato de cada método (precondiciones y efectos sobre estado global).
- Externalizar parámetros mágicos (p.ej. 0.00999, 100000) a constantes descriptivas.
- Agregar validaciones de integridad: suma de bases por tipo vs subtotal gravado, detección de impuestos inconsistentes.
- Añadir pruebas comparando resultados de impuestos con escenarios fiscales oficiales.

## Referencias Cruzadas
- Usa `FuncionesGenericas` (`mFg`) para: formato (`FormateaCantidad`), parsing (`CadenaAFlotante`), comparación tolerante (`CompararFlotantes`, `EsCero`), mensajes (`AppMessageBox`).
- Interactúa con `BufferRespuestas` para poblar datos desde consultas SQL empaquetadas.
- Depende de frames de captura `TFrameBuscaArticulos` (validación y extracción de metadatos de artículo/oferta), componentes visuales `VTStringGrid`, `VTLabeledEdit` y `TLabel`.

## Glosario Rápido
- Partida: Línea de detalle (ítem) del documento.
- Tasa 0: Partidas exentas de IVA o con IVA 0%.
- Gravado: Partidas con IVA > 0.
- IEPS: Impuesto Especial sobre Producción y Servicios; puede ser porcentaje o cuota.
- Nota de Crédito (NCredito): Documento de ajuste (devolución, descuento, bonificación).
- Oferta: Precio promocional vigente para el artículo.

## Ejemplo Simplificado de Cálculo de Impuesto
Suponga precio unitario sin descuento = 100, descuento global = 10%, IVA=16% (impuesto 1), IEPS=8% (impuesto 2) en venta:
1. Base con descuento = 100 * (1 - 0.10) = 90.
2. Impuesto IEPS (si se calcula primero) = 90 * 0.08 = 7.2.
3. Base IVA encadenada = 90 + 7.2 = 97.2.
4. IVA = 97.2 * 0.16 = 15.552.
5. Total impuestos = 7.2 + 15.552 = 22.752.
6. Precio final con impuestos = 90 + 22.752 = 112.752 (redondeo posterior según política).

## Archivo Relacionado
Este documento se deriva de la lectura de `ClassArregloDetalle.h` y `ClassArregloDetalle.cpp` (todas las secciones consultadas). Cualquier diferencia futura debe sincronizarse actualizando este archivo.

---
© Documentación técnica generada automáticamente.
