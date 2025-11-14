# ClassFuncionesMySql

## Resumen General
`FuncionesMySql` centraliza la ejecución de instrucciones MySQL (SELECT y acciones: INSERT, UPDATE, DELETE, DDL) y la construcción de buffers binarios estructurados consumibles por `BufferRespuestas`. Provee sobrecargas para distintas estrategias de gestión de memoria (buffer interno asignado vs buffer provisto externamente), manejo de múltiples sentencias en bloque, conversión tipificada de columnas MySQL a un formato serial unificado y utilidades de conexión/desconexión con sincronización de hilos mediante sección crítica (`TCriticalSection`). También canaliza mensajes e información de errores hacia un objeto servidor (`ServidorVioleta`) si está disponible.

## Objetivos
- Simplificar y homogeneizar la interacción con la API C de MySQL.
- Serializar resultados en un formato compacto reutilizable por otras capas (UI / lógica) sin exponer directamente la API MySQL.
- Asegurar acceso thread-safe a operaciones de conexión/desconexión.
- Proporcionar construcción flexible de `BufferRespuestas` (propiedad del buffer vs buffer externo).
- Manejar casos especiales de tipos (fechas, decimales, timestamp, geometry) y normalización de valores NULL.

## Atributos Principales
- Credenciales/conexión: `mHostMysql`, `mDbMysql`, `mUsuMysql`, `mPwdMysql`, `mPortMysql`.
- Sincronización: `ServidorSeccionCriticaMySQL` (protege secciones críticas de conexión/desconexión).
- Integración servidor: `ServidorVioleta *mServidor` (opcional para logging / notificación).
- Utilidades parsing/formato: `FuncionesGenericas mFg`.
- Estado de conexión: `bool mErrorMysql` (true si la última conexión falló).

## Constantes
- `TAM_MAX_RESULTADO_EJECUTA_ACCIONES` (512) para tamaño máximo de respuesta en acciones simples.
- `TAM_MAX_BUFFER_RESPUESTA` (definida externamente, usada como default en SELECT con buffer asignado dinámicamente).

## Formato de Serialización de SELECT
Coincide con el consumido por `BufferRespuestas`:
1. (int) Tamaño total del resultado en bytes (al inicio del buffer).
2. Indicador de error ("0" = sin error; si falla se lanza excepción antes de construir formato alterno).
3. Número de columnas (como cadena).
4. Para cada columna: nombre truncado a `MAX_ANCHO_NOMBRE_CAMPO`, tipo interno mapeado (C,D,T,I,F,N,B,S), longitud, decimales.
5. Número de registros.
6. Datos fila a fila, campo a campo, con valores en formato cadena (fechas normalizadas, decimales sin ceros no significativos, nulos sustituidos por valores sentinela según tipo).

## Mapeo de Tipos MySQL ? Código Interno
| Tipo MySQL | Código | Notas |
|-----------|--------|-------|
| STRING / VAR_STRING / GEOMETRY | C | Texto genérico (GEOMETRY se reduce a '0') |
| DATE / NEWDATE / DATETIME | D | Fechas convertidas a formato dd/mm/yyyy; DATETIME misma conversión |
| TIME | T | Formato hh:mm:ss |
| TINY / SHORT / INT24 / LONG / LONGLONG | I | Entero |
| FLOAT / DOUBLE | F | Valor flotante (no se eliminan ceros no significativos) |
| DECIMAL / NEWDECIMAL | N | Se eliminan ceros no significativos al final |
| BLOB variants | B | Longitud registrada, contenido tratado como cadena (sin decodificación adicional) |
| TIMESTAMP | S | Convertido con `AgregaPCharABufferMySqlTimeStampToStrDate` |

Valores NULL se sustituyen por:
- Texto / Blob / Geometry: ""
- Date/Datetime: "01/01/1900"
- Time: "00:00:00"
- Timestamp: "01/01/1900 00:00:00"
- Numérico (I,F,N): "0"

## Principales Métodos
### Constructores y Configuración
- `FuncionesMySql(host, db, usuario, pwd, port)` y sobrecarga con `ServidorVioleta*` para logging.
- `AsignaServidor(ServidorVioleta*)`: Inyección tardía del servidor.

### Conexión
- `MYSQL* Conectarse()`: Inicializa y abre conexión. Protegido por sección crítica. Ajusta `mErrorMysql`.
- `void Desconectarse(MYSQL*)`: Cierre seguro bajo sección crítica.

### Ejecución de SELECT
Sobrecargas:
- `EjecutaSelect(Id, MySQL, instruccion, BufferRespuestas* &respuesta, int TamBuffer)`: Asigna buffer dinámico, construye y envuelve en `BufferRespuestas` (liberación automática).
- `EjecutaSelect(Id, MySQL, instruccion, BufferRespuestas* &respuesta, char *buffer)`: Usa buffer externo (no liberado por la clase `BufferRespuestas`).
- `EjecutaSelect(Id, MySQL, instruccion, char *respuesta)`: Serializa resultado directamente en `respuesta`.
- `EjecutaSelectNulo(...)`: Ejecuta SELECT ignorando contenido (para validar existencia o efectos secundarios de triggers / locks).

### Ejecución de Acciones (No SELECT)
- `EjecutaAccionNulo(...)`: Ejecuta instrucción sin serializar respuesta (solo levanta excepción si falla).
- `EjecutaAccion(Id, MySQL, instruccion, char *respuesta, int &tam_respuesta)`: Genera buffer mínimo con indicador de error ("0" o excepción).
- `EjecutaBufferAcciones(Id, MySQL, BufferSQL, respuesta, tam_respuesta)`: Procesa múltiples instrucciones empaquetadas en un buffer (primera entrada = número de instrucciones). Lanza excepción con mensaje específico si detecta violación de constraint de clave foránea `sku_ibk1` para ofrecer contexto al usuario.

### Mensajería / Logging
- `MuestraMensaje(msg, id_respuesta)` y `MuestraMensajeError(msg)`: Encaminan a `mServidor` si está configurado; si no, silenciosos.

## Flujo de Ejecución de un SELECT (Detalle)
1. Se escribe indicador de tamaño reservado (luego se actualiza con offset final).
2. Se agrega indicador de éxito "0".
3. Se consultan columnas (`mysql_fetch_fields`).
4. Se serializan definiciones (nombre truncado, tipo mapeado, longitud, decimales).
5. Se serializa número de filas (`row_count`).
6. Se itera cada fila (`mysql_fetch_row`) y cada columna, normalizando nulos y formatos.
7. Se libera `MYSQL_RES` y se calcula tamaño final (`*aux_tam = bytes_escritos`).
8. Devuelve 1 si éxito; en caso contrario lanza excepción (no rellena buffer de error alterno en la implementación actual ?bloque comentado permanece como referencia histórica?).

## Manejo de Errores
- Estrategia principal: lanzar `Exception` inmediata ante error MySQL (SELECT, acción o buffer de acciones). Esto evita ambigüedad de estados intermedios.
- `EjecutaBufferAcciones` detecta caso específico de FK `sku_ibk1` para generar mensaje contextual más amigable.
- No se implementa actualmente reintento backoff ni reconexión automática; tareas que podrían agregarse.

## Thread-Safety
- Solo las operaciones de conexión/desconexión están protegidas por `ServidorSeccionCriticaMySQL`. Las ejecuciones de queries asumen que el `MYSQL*` es usado en un contexto serializado externo o por hilo dedicado.
- Para múltiples hilos compartiendo instancia: cada hilo debe obtener su propio `MYSQL*` vía `Conectarse()`.

## Ejemplo de Uso
```cpp
FuncionesMySql db(host, schema, user, pass, port);
MYSQL *conn = db.Conectarse();
try {
    BufferRespuestas *res = nullptr;
    if (db.EjecutaSelect(1, conn, "SELECT id, nombre FROM clientes", res, 1024*64)) {
        for (int r=0; r<res->ObtieneNumRegistros(); ++r) {
            res->IrAlRegistroNumero(r);
            auto id = res->ObtieneDato("id");
            auto nombre = res->ObtieneDato("nombre");
            // ... procesar
        }
        delete res; // Liberará también el buffer interno.
    }
} catch (const Exception &e) {
    // Manejo de error
}
db.Desconectarse(conn);
```

## Consideraciones de Rendimiento
- Conversión por campo implica múltiples llamadas a utilidades de copia y normalización; para consultas masivas podría evaluarse un modo streaming sin materialización completa.
- Uso de `row_count` requiere que el resultado se almacene completamente (`mysql_store_result`), lo cual consume memoria proporcional al número total de filas. Para datasets muy grandes evaluar `mysql_use_result` + serialización incremental.
- Eliminación de ceros no significativos en decimales reduce payload, pero consume CPU adicional; se podría hacer opcional.

## Limitaciones / Riesgos
- Sin límite explícito de crecimiento: si `TamBuffer` es insuficiente se escribirá fuera de rango (no hay verificación). Debería agregarse cálculo previo o redimensionamiento dinámico.
- Ausencia de sanitización de `instruccion`: SQL injection depende de capas externas (espera queries ya construidos de forma segura).
- Falta de logging detallado para auditoría (solo mensaje plano). Se podría integrar nivel (INFO/WARN/ERROR) y timestamp estructurado.
- No se reconecta automáticamente al detectar `CR_SERVER_LOST` o `CR_SERVER_GONE_ERROR`.

## Posibles Mejoras
- Implementar verificación de capacidad antes de escribir en el buffer (o usar un builder dinámico con expansión).
- Añadir métricas (tiempo por query, bytes serializados, filas afectadas) para tuning.
- Soporte opcional de compresión del buffer para respuestas muy grandes.
- Modo streaming (`mysql_use_result`) + callback para procesado en tiempo real.
- Reintentos con backoff para errores transitorios.
- Pool de conexiones encapsulado (actualmente responsabilidad externa). 
- Tipificación directa (leer enteros/dobles sin pasar por `AnsiString`).
- Parametrizar valor sentinela de fecha (?01/01/1900?).

## Interacción con Otras Clases
- Produce buffers consumidos por `BufferRespuestas`.
- Depende de `FuncionesGenericas` para serialización segura (agregar PChar/String a buffer y formateos especiales de fechas, decimales y timestamps).
- Opcionalmente notifica a `ServidorVioleta` para mostrar mensajes y errores centralizados.

## Glosario
- Acción: Instrucción SQL que no devuelve dataset (INSERT/UPDATE/DELETE/DDL).
- Selección (SELECT): Instrucción que retorna filas y columnas.
- Buffer SQL de acciones: Estructura serial que lista N instrucciones consecutivas a ejecutar.

## Archivo Fuente
Basado en `ClassFuncionesMySql.h` y `ClassFuncionesMySql.cpp` (líneas 1?491). Cualquier cambio en tipos soportados o formato de serialización debe reflejarse en este documento.

---
© Documentación técnica generada automáticamente.
