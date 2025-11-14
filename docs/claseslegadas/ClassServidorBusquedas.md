# ClassServidorBusquedas

## Resumen General
`ServidorBusquedas` encapsula la lógica de consultas rápidas (lookups) y listados filtrados de entidades operativas del sistema (artículos, clientes, proveedores, ventas, compras, pagos, movimientos, logística, e?commerce, cartas porte, gastos, etc.). Cada método construye dinámicamente una sentencia SQL (mayormente SELECT) a partir de parámetros serializados recibidos en un buffer, y escribe el/los result sets directamente en el `BufferResultado` de un objeto `RespuestaServidor`.

Su objetivo es devolver conjuntos de datos acotados (paginados de forma fija vía `LIMIT`) para alimentar interfaces de búsqueda/autocompletado o selección dentro de la aplicación cliente.

## Responsabilidades Clave
- Interpretar parámetros compactos (buffer lineal) usando `FuncionesGenericas::ExtraeStringDeBuffer`.
- Aplicar filtros condicionales (tipo de búsqueda, estatus activo, existencia, rangos de fechas, claves, RFC, códigos de barras, etc.).
- Limitar el volumen mediante constantes `NUM_LIMITE_RESULTADOS_BUSQ` y `NUM_LIMITE_RESULTADOS_BUSQ2` evitando saturar red/cliente.
- Incorporar lógica condicional de negocio (mostrar existencias, incluir joins a inventario, marcas, clasificaciones, sucursal).
- Escribir resultados en el buffer de respuesta reutilizable gestionado por `RespuestasServidor`.

## Patrón de Funcionamiento Típico
1. Se recibe una petición con identificador de búsqueda en el servidor principal (router externo no mostrado aquí).
2. Se obtiene/crea un `RespuestaServidor` y una conexión `MYSQL*` ya abierta (pool/conexión administrada por `ServidorVioleta`).
3. El método específico (`BuscaXxx`) desempaca parámetros en orden predeterminado.
4. Construye fragmentos de condiciones según parámetros no vacíos.
5. Llama a `mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ)` para preparar escritura.
6. Ejecuta uno o varios `EjecutaSelectSql` concatenando result sets en el mismo buffer.
7. El cliente interpretará secuencialmente los bloques recibidos.

## Categorías de Búsqueda (Métodos Principales)
- Inventario / Productos: `BuscaArticulos`, `BuscaKits`, `BuscaLoteInventario`, `BuscaArticulosCandidatosAgregar`.
- Clientes / Ventas: `BuscaClientes`, `BuscaVentas`, `BuscaPedidosCli`, `BuscaNotasCredCli`, `BuscaNotasCargCli`, `BuscaPagosCli`, `BuscaChequesCli`, `BuscaPrePagosCli`, `BuscaPedidosEcom`, `BuscaRecepcion`.
- Proveedores / Compras: `BuscaProveedores`, `BuscaCompras`, `BuscaPedidosProv`, `BuscaNotasCredProv`, `BuscaNotasCargProv`, `BuscaPagosProv`, `BuscaChequesProv`.
- Almacén / Movimientos: `BuscaMovimientosAlmacen`, `BuscaInventarios`, `BuscaTransformacion`, `BuscaRecepciones`, `BuscaSurtido`.
- Personal / Organización: `BuscaUsuarios`, `BuscaEmpleados`, `BuscaVendedores`.
- Catálogos específicos adicionales: `BuscaMarcas`, `BuscaFormasImpresion`, `BuscaEmbarques`, `BuscaCartasPorte`, `BuscaCartaPorte20`, `BuscaProdServ`.
- Gastos y otros: `BuscaPagosGastos`, `BuscaNotasCredGasto`, `BuscaGastos`.
- Mensajería / móviles: `BuscaMsgsMoviles`.

## Ejemplo Resumido de Flujo (BuscaArticulos)
1. Lee sucursal y bandera de existencias.
2. Consulta parámetro `EXISTENCIASPV` para decidir si debe añadir joins a tablas de existencias / almacenes.
3. Lee `tipo_busqueda` y arma condiciones específicas (por código, nombre, marca, clasificación, código de barras, etc.).
4. Prepara buffer de salida y ejecuta la sentencia principal (si una clasificación vacía se devuelven catálogos auxiliares: marcas, clasificaciones).
5. Libera buffers auxiliares.

## Formato y Manejo de Parámetros
- Orden rígido (el cliente debe enviar exactamente la secuencia esperada).
- Valores vacíos representados con cadena vacía o espacio simple " ".
- Flags binarios como "1"/"0" o tokens específicos (ej. "SI" / "NO" para existencias, códigos de tipo de búsqueda: N, C, M, E, CB, ART, etc.).
- Ausencia de validación semántica amplia (se asume capa cliente confiable).

## Lógica de Control de Volumen
- Constantes `NUM_LIMITE_RESULTADOS_BUSQ` (501) y `NUM_LIMITE_RESULTADOS_BUSQ2` (3000) con `LIMIT` en SQL.
- Evita consultas sin prefijo (búsquedas amplias) exigiendo prefijos (`like 'dato%'`).
- Algunas búsquedas retornan múltiples bloques (ej. catálogos complementarios) reutilizando mismo buffer.

## Dependencias Internas
- `ServidorVioleta`: ejecución real de SQL y gestión de buffers / recursos.
- `DatosTabla`: ocasional para normalización (aunque en búsquedas predomina SQL manual).
- `FuncionesGenericas`: extraer strings, conversión de fechas, etc.
- `BufferRespuestas`: para leer sub-consultas auxiliares (ej. parámetros).

## Seguridad y Riesgos
- Riesgo de SQL injection: concatenación directa de parámetros sin sanitización (p.ej. `... like '%s%%'`). Si el cliente no es plenamente confiable, introducir escapes (mysql_real_escape_string) es crítico.
- Falta de paginación controlada por el cliente; solo límite fijo ? no apto para conjuntos mayores.
- Uso intensivo de `sprintf` sobre `AnsiString` con potencial de error si se cambian formatos.
- Algunas rutas realizan múltiples SELECT secuenciales ? posible latencia acumulada.

## Posibles Mejoras
1. Introducir un generador de filtros con escapado centralizado para strings.
2. Agregar paginación (offset + limit) y orden explícito parametrizable.
3. Cache/short?circuit para búsquedas repetitivas de catálogos estáticos (marcas, clasificaciones).
4. Retornar metadatos (ej. número total de coincidencias) adicional al subconjunto limitado.
5. Validar longitudes máximas de parámetros antes de construir SQL.
6. Incorporar mecanismos de observabilidad (tiempos de cada búsqueda y cardinalidad).

## Ejemplo de Uso (Pseudocódigo Cliente)
```
// Empaquetar parámetros para buscar clientes por RFC activos
Buffer params;
params.add("RFC");        // tipo_busqueda
params.add("1");          // solo_activos
params.add("GOMC");       // prefijo RFC
sendRequest(ID_BUSQ_CLIENTE, params);
// Recibir buffer con columnas: cliente, rsocial, nomnegocio, rfc, ...
```

## Contrato Simplificado
- Entrada: buffer lineal de strings (en orden predefinido por tipo de búsqueda).
- Salida: uno o más result sets serializados consecutivamente.
- Errores: Generalmente silenciosos (si la SQL no trae filas el cliente recibe 0 registros). Excepciones internas podrían propagarse al marco superior.

## Edge Cases
- Parámetros vacíos generan resultados grandes si no se controla (algunos métodos envían catálogos base en ese caso intencionalmente).
- Sucursal inexistente o parámetro de existencias faltante: puede romper la lógica de join condicional.
- Límite alcanzado: el cliente no conoce si hay más resultados (no se incluye indicador de truncamiento).

## Observaciones de Diseño
- Clase monolítica: todas las búsquedas en un único archivo (dificulta mantenimiento y pruebas unitarias granulares).
- Fuerte acoplamiento a esquema MySQL (nombres de tablas y campos in?line).
- Uso de `AnsiString` + VCL (limita portabilidad).

## Recomendaciones de Refactor
- Dividir por dominios (ArticulosBusquedaService, ClientesBusquedaService, etc.).
- Implementar capa de construcción de consultas reutilizable.
- Incorporar enumeración/ID de columnas en metadata para robustez ante cambios de orden.
- Añadir logs estructurados antes/después de ejecutar la consulta.

---
© Documentación técnica generada automáticamente.
