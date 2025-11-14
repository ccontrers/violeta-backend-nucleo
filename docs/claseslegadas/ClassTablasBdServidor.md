# ClassTablasBdServidor

## Resumen General
`TablasBdServidor` extiende `TablasBd` para poblar dinámicamente la metadata de todas las tablas directamente desde un servidor MySQL activo en tiempo de arranque del servidor Violeta. Ejecuta una consulta de catálogo (`mysql_list_tables`) y para cada tabla realiza un `SELECT * ... LIMIT 0` con el fin de recuperar las definiciones de columnas (`mysql_fetch_fields`). Traduce tipos MySQL a códigos internos (C,D,T,I,F,N,B,S) y valida que el número de tablas no exceda los límites predefinidos. Sirve como fuente primaria de verdad de la estructura para el resto de la aplicación del lado servidor.

## Responsabilidades Clave
- Descubrir todas las tablas existentes en el esquema configurado.
- Mapear tipos MySQL a tipos internos uniformes usados por `BufferRespuestas`, `DatosTabla`, etc.
- Asegurar límites (número máximo de tablas) y abortar si se exceden (`Application->Terminate()` tras mostrar alerta).
- Mantener integridad de metadatos en memoria hasta el fin de vida del servidor.

## Flujo de Inicialización
1. Conecta a MySQL usando `mServidorVioleta->mFuncionesMySql->Conectarse()`.
2. Lista tablas: `mysql_list_tables(MySQL, NULL)`.
3. Itera filas para copiar nombres a `mTablasVioleta[i].nombre` y establece `mNumTablas`.
4. Para cada tabla forma sentencia `SELECT * FROM <tabla> LIMIT 0`.
5. Ejecuta la sentencia y obtiene result set vacío con solo metadata de columnas.
6. Por cada columna mapea tipo MySQL ? tipo interno y guarda longitud y decimales.
7. Libera result sets y al final desconecta.

## Mapeo de Tipos (Resumen)
| Tipo MySQL | Interno | Notas |
|------------|---------|-------|
| STRING / VAR_STRING | C | Texto variable |
| DATE / NEWDATE / DATETIME | D | Fecha (DATETIME también etiquetado como D) |
| TIME | T | Hora |
| TINY / SHORT / INT24 / LONG / LONGLONG | I | Entero |
| FLOAT / DOUBLE | F | Flotante |
| DECIMAL / NEWDECIMAL | N | Decimal (precisión exacta) |
| BLOB (todas variantes) | B | Binario / texto largo |
| GEOMETRY | C | Normalizado a caracter longitud 1 |
| TIMESTAMP | S | Marca de tiempo |

Errores en tipos no mapeados lanzan excepción con mensaje detallando tabla, columna y código MySQL.

## Atributos Específicos
- `ServidorVioleta *mServidorVioleta`: Acceso a utilidades de conexión y mensajería.
- Hereda de `TablasBd`:
  - `mNumTablas`
  - `mTablasVioleta[]` (estructura completa de tablas y campos)
  - `FuncionesGenericas mFg` para formateo/mensajes.

## Manejo de Errores y Alertas
- Si el número de tablas descubiertas ? `MAX_NUM_TABLAS_VIOLETA`: muestra mensaje y termina aplicación (enfoque fuerte ? se podría mejorar a un modo degradado o registro de advertencia).
- Tipos desconocidos ? excepción: evita continuar con esquema inconsistente.
- Uso de bloque `try / __finally` garantiza desconexión del servidor MySQL.

## Consideraciones de Desempeño
- Realiza un `SELECT ... LIMIT 0` por cada tabla; en esquemas muy grandes puede impactar el arranque. Alternativa: consultar `information_schema.COLUMNS` en una sola pasada.
- No realiza caching persistente; cada instancia vuelve a consultar todo en construcción.

## Posibles Mejoras
- Reemplazar múltiples SELECT por una consulta única a `information_schema` para reducir round-trips.
- Añadir campos adicionales: nullable, default, clave primaria, índices, comentarios.
- Estrategia de actualización incremental (detectar cambios sin recargar todo el catálogo).
- Registro de checksum de esquema para sincronización y validación en clientes.
- Manejo configurable ante exceso de tablas (log + continuar parcial, en vez de terminar proceso).

## Interacción con Otras Clases
- Depende de `ServidorVioleta` (por `mFuncionesMySql`) para conexión.
- Provee metadatos a: `DatosTabla`, `FuncionesMySql` (indirectamente a través de capas), generación de reportes, validaciones.

## Ejemplo Simplificado
```cpp
ServidorVioleta *srv = /* ya inicializado */;
TablasBdServidor catalogo(srv);
int num = catalogo.ObtieneNumTablas();
int idxClientes = catalogo.ObtieneIndiceTabla("clientes");
int camposClientes = catalogo.ObtieneNumCampos(idxClientes);
```

## Riesgos / Edge Cases
- Esquemas con tablas > límite interrumpen el arranque: requiere supervisión al crecer la base.
- Tipos nuevos introducidos en versiones recientes de MySQL provocarán excepción hasta adicionar mapeo.
- No se filtran tablas de sistema (dependiendo del usuario/base seleccionada). Si la base incluye tablas temporales o utilitarias podrían inflar mNumTablas.

## Archivo Fuente
Basado en `ClassTablasBdServidor.h` y `ClassTablasBdServidor.cpp` (líneas 1?400). Cambios posteriores al proceso de descubrimiento deben reflejarse aquí.

---
© Documentación técnica generada automáticamente.
