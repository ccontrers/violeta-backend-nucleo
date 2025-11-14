# ClassServidorCatalogos

## Resumen General
`ServidorCatalogos` gestiona operaciones CRUD y procesos auxiliares sobre una amplia gama de catálogos y entidades maestras del sistema (clientes, proveedores, empleados, almacenes, parámetros, impuestos, precios, clasificaciones, artículos, rutas de embarque, banca, privilegios, configuraciones e?commerce, folios, plantillas CFDI, etc.). Centraliza la traducción de un buffer de parámetros lineal en instrucciones SQL (frecuentemente múltiples y transaccionales) escribiendo los resultados o acuses en `RespuestaServidor`.

La clase se organiza en bloques lógicos (comentarios por sección) pero todo reside en un solo archivo muy extenso; esto la convierte en un ?service façade? de alto acoplamiento con el esquema de base de datos.

## Responsabilidades Clave
- Desempaquetar parámetros (orden estricto) mediante `FuncionesGenericas`.
- Construir SQL dinámico para INSERT, UPDATE, DELETE y SELECT; a veces con composición detallada manual y otras usando `DatosTabla` para generar fragmentos.
- Gestionar transacciones explícitas (uso de `SET AUTOCOMMIT=0`, `START TRANSACTION`, `COMMIT`).
- Generar claves secuenciales/foliadores (ej. clientes) usando tablas de folios + variables de sesión (@folio, @foliosig).
- Insertar datos relacionados en cascada (teléfonos, direcciones de entrega, datos de crédito, empresas asociadas, rutas de embarque, componentes de artículos, tags, etc.).
- Manejar lógica condicional de negocio (geo?ubicaciones GIS, validaciones de existencia de parámetros, configuración multi?empresa, privilegios, precios bloqueados, segmentaciones, reclasificaciones masivas).
- Serializar múltiples result sets consecutivos para consumo del cliente.

## Patrones Recurrentes
1. Lectura de tarea: `A` (alta) o `M` (modificación) para decidir INSERT vs UPDATE.
2. Uso de `DatosTabla`:
   - `AsignaTabla()` establece contexto de metadatos.
   - `InsCamposDesdeBuffer()` consume valores en el orden definido por el protocolo.
   - `GenerarSqlInsert()` / `GenerarSqlUpdate(condición)` producen sentencias completas.
3. Ensamblaje de batch SQL en un buffer manual (`buffer_sql`) con conteo de instrucciones al inicio (protocolo propio para `EjecutaBufferAccionesSql`).
4. Variables de sesión MySQL (@folio, @ubicacion, @ubicacionent) para compartir valores entre instrucciones del mismo batch.
5. Geolocalización: conversión de coordenadas a `POINT(lat,lon)` almacenado en columnas `ubicaciongis`.

## Ejemplos Destacados
### Alta/Edición de Cliente (`GrabaCliente`)
- Lee tarea, clave (si modifica), latitud, longitud.
- Genera folio nuevo: bloquea fila de `foliosemp`, incrementa, construye código con sucursal y padding.
- Inserta registro en `clientes` (fechaalta, sucursal, posible ubicación GIS).
- Inserta o actualiza relación multi?empresa en `clientesemp` (usando parámetros por cada empresa y defaults consultados en tablas de parámetros globales).
- Procesa teléfonos (`telefonosclientes`), direcciones de entrega (`direccionesentregaclientes`) y datos de crédito (`datoscredito`).
- Envuelve todo en transacción.
- Devuelve el folio asignado mediante SELECT final `select @folio as folio`.

### Ruta de Embarque (`GrabaRutaEmbarque`)
- Recibe identificador de embarque + número variable de direcciones.
- Reemplaza completamente la ruta previa (`delete from embarquesruta`).
- Inserta cada punto con hasta tres ubicaciones GIS (ubicación entrega / llegada / salida) según longitudes recibidas.

### Genéricos (`GrabaGenerico`, `BajaGenerico`, `ConsultaGenerico`)
- Factorizan crud simple: reciben nombre de tabla y campo clave.
- Delegan parsing a `DatosTabla` y construyen sentencia adecuada.
- Restringen tipo de clave (formateo numérico vs string con comillas) basados en tipo de campo consultado vía metadatos.

## Conjuntos de Entidades Cubiertas (No Exhaustivo)
- Identidad Comercial: clientes, proveedores, empleados, vendedores, cobradores, transportistas, aseguradoras, remolques.
- Organización y Operación: sucursales, almacenes, secciones, terminales, departamentos, puestos.
- Fiscal / CFDI: folios, plantillas CFDI, parámetros globales, tipos y claves de productos/servicios, uso CFDI, impuestos, monedas, formas de pago.
- Clasificaciones y Segmentaciones: marcas, clasificaciones 1?3, segmentos, giros, canales, grupos de objetos, privilegios, tipos de bloqueo, tipos de precio.
- Logística: rutas de embarque, conceptos de embarque, artículos supervisados, articulos venta internet, reclasificaciones masivas.
- Comercio Electrónico: pedidos (órdenes), detalle pedidos, bitácoras, reembolsos, fracciones, Rappi y actualizaciones de catálogo.
- Inventario y Costos: movimientos, pesos promedio, fraccionamientos, configuraciones de precios, estado de artículos, bitácoras varias.
- Banca y Pagos: bancos, cuentas, orígenes de pago, pinpad, movimientos de caja.
- Marketing / Ofertas: ofertas, cancelaciones, tags de artículos.

## Lógica de Multi?Empresa
- Consultas frecuentes a `FormServidor->ObtieneClaveEmpresa()` y `ObtieneClaveSucursal()` para filtrar.
- Inserciones condicionales en `clientesemp` replicando defaults por cada empresa detectada en tabla `empresas`.
- Parámetros globales empresariales (`parametrosglobemp`) usados para default de precio, vendedor y cobrador.

## Manejo de Geolocalización
- Coordenadas recibidas como strings, validadas por longitud mínima (>3 o >5 caracteres) para decidir si se construye `POINT`.
- Almacena en campos `ubicaciongis`, `ubicaciondellegada`, `ubicaciondesalida` según caso.

## Transacciones
- Patrón: `SET AUTOCOMMIT=0`; `START TRANSACTION`; (operaciones); `COMMIT`.
- No se observan `ROLLBACK` explícitos en errores ? se asume que fallos impiden ejecución subsecuente (pero lote parcial podría haber corrido si error se produce tarde). Mejora recomendada.

## Riesgos y Oportunidades de Mejora
1. SQL Injection: Parámetros insertados por concatenación directa sin escape.
2. Tamaño monolítico: Dificulta pruebas unitarias y revisiones de seguridad.
3. Errores silenciosos: Mínimo manejo de excepciones; muchas rutas usan `throw Exception(...)` sin categorizar.
4. Mezcla de responsabilidades (negocio, persistencia, orquestación) en un único nivel.
5. Uso manual de buffers y conteos ? propenso a desajustes si se modifica protocolo.
6. Falta de `ROLLBACK` en bloques con múltiples operaciones dependientes.
7. Duplicación de patrones (alta vs modificación) que podrían refactorizarse (Strategy / Command interno).
8. Ausencia de versionado de esquema en tiempo de ejecución (asume estructura estática).

## Recomendaciones Técnicas
- Introducir capa DAO / Repository y encapsular builder SQL con escapes seguros.
- Dividir por dominios (ClientesService, ProveedoresService, ArticulosService, etc.).
- Implementar macro?transacciones con control de errores y rollback centralizado.
- Registrar métricas (duración, filas afectadas) para tuning.
- Normalizar manejo de geodatos (validador coordenadas y envoltorio). 
- Parametrizar límites y validar cardinalidades (número de teléfonos, direcciones) antes de construir arreglos en memoria.

## Ejemplo Simplificado (Alta Cliente)
```
// Pseudocódigo: construir buffer en cliente
params.add("A");            // tarea alta
params.add("");             // clave vacía (autogenera)
params.add("21.152");       // latitud
params.add("-101.711");     // longitud
// ... campos esperados por InsCamposDesdeBuffer(clientes)
// ... teléfonos, direcciones, crédito según flags
send(ID_GRA_CLIENTE, params);
// Servidor devuelve: datos + select @folio as folio
```

## Contrato Simplificado
- Entrada: buffer con secuencia de tokens; algunos sub?segmentos precedidos por flags ?1? para indicar inclusión de secciones (teléfonos, direcciones, crédito, etc.).
- Salida: result sets concatenados (para consultas) o confirmación + datos clave (folio generado) para altas.
- Errores: Excepciones C++ lanzadas; cliente debe mapear a mensaje de usuario.

## Edge Cases
- Falta de parámetros esperados ? desalineación en lectura (no hay verificación de tamaño previo).
- Folio no disponible (registro faltante en `foliosemp`) lanza excepción.
- Coordenadas geográficas mal formadas podrían insertarse e invalidar cálculos geoespaciales posteriores.
- Grandes volúmenes en lotes (ej. rutas con muchas direcciones) pueden superar tamaño buffer fijo.

## Observaciones de Integración
- Reutiliza intensivamente `DatosTabla` para mantener sincronía con definiciones de campos (reduce drift manual, aunque no evita SQL directo extenso en otras rutas).
- Interactúa con diversas tablas transversales (parámetros globales, folios, bitácoras, ecommerce) mostrando centralidad en la capa de negocio.

---
© Documentación técnica generada automáticamente.
