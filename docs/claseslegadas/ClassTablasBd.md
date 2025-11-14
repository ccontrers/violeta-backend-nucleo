# ClassTablasBd

## Resumen General
`TablasBd` mantiene en memoria la descripción estructural (metadatos) de todas las tablas conocidas por la aplicación: nombre de cada tabla, número de campos y para cada campo su nombre, tipo, longitud y decimales. Sirve como catálogo central para validaciones de integridad y generación dinámica de SQL o buffers, consumido por clases como `DatosTabla` y `FuncionesMySql`/`BufferRespuestas`. Ofrece acceso por índice o por nombre y funciones de serialización/deserialización completa a/desde un buffer compacto.

## Estructuras Principales
- `DefCampo`:
  - `nombre`: (hasta `MAX_ANCHO_NOMBRE_CAMPO`)
  - `tipo`: Código: C=Caracter, D=Date, T=Time, I=Int, F=Flotante, N=Decimal (pueden existir extensiones como B, S si se amplía el catálogo en otras capas).
  - `longitud`: Longitud declarada.
  - `decimales`: Precisión decimal (solo relevante para tipos numéricos/decimales).
- `DefTabla`:
  - `nombre`: (hasta `MAX_ANCHO_NOMBRE_TABLA`).
  - `num_campos`: Conteo de campos válidos en `campos`.
  - `campos[MAX_NUM_CAMPOS_VIOLETA]`: Arreglo fijo de definiciones de campo.

## Constantes
| Constante | Significado |
|-----------|-------------|
| `MAX_NUM_CAMPOS_VIOLETA` (150) | Campos máximos por tabla. |
| `MAX_NUM_TABLAS_VIOLETA` (800) | Número máximo de tablas catalogables. |
| `MAX_ANCHO_NOMBRE_TABLA` (60) | Longitud máxima de nombre de tabla. |
| `MAX_ANCHO_NOMBRE_CAMPO` (32) | Longitud máxima de nombre de campo. |

## Atributos de Clase
- `FuncionesGenericas mFg`: Utilidades para serialización con buffers.
- `int mNumTablas`: Número real de tablas cargadas.
- `DefTabla mTablasVioleta[MAX_NUM_TABLAS_VIOLETA]`: Arreglo estático con toda la metadata.

## Métodos Públicos
### Consultas Básicas
- `int ObtieneNumTablas()`: Cantidad de tablas cargadas.
- `AnsiString ObtieneNombreTabla(indice)`: Nombre por índice.
- `int ObtieneNumCampos(indiceTabla)`: Número de campos de la tabla.
- `AnsiString ObtieneNombreCampo(indiceTabla, indiceCampo)`
- `char ObtieneTipoCampo(indiceTabla, indiceCampo)`
- `int ObtieneLongitudCampo(indiceTabla, indiceCampo)`
- `int ObtieneDecimalesCampo(indiceTabla, indiceCampo)`

### Búsqueda
- `int ObtieneIndiceCampo(indiceTabla, Nombre)`: Busca campo (case-insensitive); -1 si no existe.
- `int ObtieneIndiceTabla(Nombre)`: Índice de tabla; -1 si no existe.

### Serialización / Deserialización
- `int LlenaBufferTablas(char *buffer)`: Serializa toda la estructura. Formato:
  1. Número de tablas.
  2. Por tabla: nombre, número de campos.
  3. Por cada campo: nombre, tipo (como cadena de longitud 1), longitud, decimales.
  Retorna bytes escritos.
- `void CargaDeBuffer(char *buffer)`: Restaura estado completo desde el formato anterior.

## Patrón de Uso Común
1. Carga inicial desde fuente externa (archivo / resultado de sync): `CargaDeBuffer()`.
2. En tiempo de ejecución otras clases consultan tipos, longitudes y decimales para validar entradas y ensamblar SQL (`DatosTabla::InsCampo`, etc.).
3. Exportación opcional (sincronización hacia otro proceso/cliente) mediante `LlenaBufferTablas()`.

## Ejemplo de Serialización
```cpp
char buffer[65536];
int bytes = tablas.LlenaBufferTablas(buffer);
// Enviar 'bytes' al cliente.
```

## Ejemplo de Deserialización
```cpp
TablasBd tablas;
// buffer recibido previamente
tablas.CargaDeBuffer(bufferRecibido);
int idx = tablas.ObtieneIndiceTabla("clientes");
int campos = tablas.ObtieneNumCampos(idx);
```

## Consideraciones de Diseño
- Estructuras estáticas simplifican acceso O(1) pero requieren recompilar si se cambian límites máximos.
- No hay comprobación de overflow en `CargaDeBuffer`: se asume que el flujo origen respeta límites. Posible mejora: validar antes de escribir a `mTablasVioleta`.
- Nombres se almacenan en `char[]` sin normalizar a minúsculas; comparación se hace con `stricmp` (case-insensitive).
- Sin bloqueo de concurrencia: operaciones de lectura no están protegidas; se espera inicialización única antes de acceso multihilo.

## Posibles Mejoras
- Cambiar arreglos fijos a estructuras dinámicas (`std::vector`) con validaciones de tamaño.
- Añadir verificación de integridad (por ejemplo, campos duplicados dentro de una tabla, longitud/decimales coherentes según tipo).
- Incluir metadatos adicionales: claves primarias, índices, nullability, defaults, comentarios.
- Método de exportación a JSON/YAML para inspección humana o tooling externo.
- Cachear/emitir versión o checksum de esquema para detectar divergencias entre cliente y servidor.
- Soporte incremental (actualizar una tabla sin reserializar todas).

## Interacción con Otras Clases
- `DatosTabla`: Usa índices y tipos para validar inserciones de campos y formatting.
- `FuncionesMySql` y `BufferRespuestas`: No dependen directamente pero los consumidores de esos resultados sí emplean esta metadata.
- Herramientas de migración o sincronización podrían apoyarse en la serialización proporcionada.

## Riesgos / Edge Cases
- Acceso con índice fuera de rango no está protegido (puede producir lectura inválida). Recomendable añadir asserts o validaciones previas.
- Buffer trucado en `CargaDeBuffer` puede provocar corrupción silenciosa de memoria (faltan defensas robustas contra entrada maliciosa).

## Archivo Fuente
Basado en `ClassTablasBd.h` y `ClassTablasBd.cpp` (líneas 1?400). Cualquier cambio de formato en `LlenaBufferTablas` debe reflejarse aquí.

---
© Documentación técnica generada automáticamente.
