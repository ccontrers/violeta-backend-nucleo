# ClassParametros

## Resumen General
`Parametros` carga en memoria (cache local) los registros de la tabla lógica de parámetros (según código: `parametrosemp` al consultar individualmente y `ID_CON_PARAMETROS` para la carga masiva) para acceso rápido desde el cliente, evitando hits repetidos a la base de datos. Ofrece lectura de valores en formato texto o entero y, opcionalmente, una vía de consulta directa a BD para parámetros críticos que puedan cambiar en caliente.

## Objetivos
1. Reducir latencia y carga del servidor reutilizando valores ya obtenidos al inicio de la sesión cliente.
2. Proveer interfaz simple de recuperación (`Valor`, `ValorEntero`).
3. Permitir forzar lectura directa desde BD (`ConsultaBaseDatos=true`) para parámetros dinámicos o sensitivos.

## Dependencias
- `ClienteVioleta`: Para ejecutar la petición masiva y consultas puntuales.
- `BufferRespuestas`: Parseo de resultados binarios devueltos por el servidor.
- `FuncionesGenericas`: Sólo para utilidades (mostrar mensajes en error inicial de carga).
- Variable global `gClienteVioleta` usada en consultas directas (acoplamiento global).

## Atributos Internos
| Atributo | Tipo | Descripción |
|----------|------|-------------|
| `mParametros` | `TStringList*` | Lista de nombres de parámetros (mantiene orden paralelo a valores). |
| `mValores` | `TStringList*` | Lista de valores correspondientes. |
| `mFg` | `FuncionesGenericas` | Helper para UI / mensajes. |

## Flujo de Construcción
1. Inicializa listas vacías.
2. Ejecuta petición `ID_CON_PARAMETROS` vía `ClienteVioleta` (buffer serializado).
3. Recorre el `BufferRespuestas` llenando pares (parametro, valor) en memoria.
4. Si la ejecución falla: muestra cuadro de mensaje (no lanza excepción en constructor, el cache queda potencialmente incompleto si se ignora el error).

## Métodos Públicos
### `AnsiString Valor(AnsiString NomParametro, bool ConsultaBaseDatos=false)`
- Modo cache (`ConsultaBaseDatos=false`): búsqueda case-insensitive en `mParametros`; si existe retorna el valor; si no, lanza excepción.
- Modo directo (`ConsultaBaseDatos=true`): construye SELECT a `parametrosemp` filtrando por sucursal del cliente activo; si encuentra registro devuelve el valor; si no, lanza excepción.
- Errores: No diferencia entre ?no encontrado? y error de ejecución (ambos terminan en excepción genérica). No hay logging estructurado.

### `int ValorEntero(AnsiString NomParametro, bool ConsultaBaseDatos=false)`
- Delegado de `Valor` y luego `StrToInt`; propaga excepción si el valor no es convertible.

## Caso de Uso Típico
```cpp
Parametros *par = cliente.Param; // Ya construido en ClienteVioleta
AnsiString serie = par->Valor("SERIE_FACTURA");
int dias = par->ValorEntero("DIAS_VIGENCIA", true); // Forzar lectura directa
```

## Consideraciones y Riesgos
1. Acoplamiento Global: Uso de `gClienteVioleta` dentro de `Valor` cuando se consulta BD directamente; dificulta test aislado.
2. Ausencia de Invalidación: No hay mecanismo para refrescar el cache completo salvo reconstruir el objeto.
3. Falta de Normalización de Errores: Mensajes de excepción genéricos; recomendación: prefijar códigos (`PAR_NO_ENCONTRADO`, `PAR_SQL_FALLO`).
4. Posible Duplicidad: Si la tabla permite duplicados del campo `parametro`, el último insertado dominará (TStringList `IndexOf` retorna primera coincidencia). No se controla unicidad.
5. Seguridad SQL: SELECT se construye por concatenación directa; riesgo teórico de inyección si `NomParametro` viene de entrada externa (mitigado si se internaliza el uso). Debe validarse contra un whitelist.
6. Costo de SELECT Directo: Repetidos `ConsultaBaseDatos=true` sobre parámetros usados con frecuencia pueden revertir el beneficio del cache; se sugiere política clara para cuáles son ?dinámicos?.
7. Thread Safety: No hay sincronización; si múltiples hilos acceden y uno modifica listas (no sucede hoy) habría condiciones de carrera. Lecturas concurrentes actuales son seguras sólo porque no se modifica tras construcción.

## Recomendaciones de Mejora
- Introducir método `Refrescar()` para recargar todos los parámetros.
- Reemplazar `TStringList` por un `std::unordered_map` (o `TDictionary`) para búsqueda O(1) y claridad semántica.
- Añadir validación/escape de `NomParametro` o parametrizar el query para evitar riesgo de inyección.
- Incorporar cache parcial para lecturas directas con TTL (por parámetro) para balancear frescura vs. performance.
- Emitir eventos o logging estructurado cuando un parámetro no se encuentra o cambia de valor frente al cache.
- Unificar obtención de sucursal sin variable global (inyectar referencia al cliente en método o almacenar puntero en la instancia).

## Contrato Simplificado
- Precondición: Constructor ejecutado; petición masiva pudo (idealmente) poblar listas.
- Postcondición (`Valor`): Retorna valor si existe; si no existe lanza excepción.
- Postcondición (`ValorEntero`): Igual que anterior con conversión numérica válida.

## Métricas Potenciales (no implementadas)
- Tiempo de carga inicial (ms).
- Ratio de aciertos en cache vs. consultas directas.
- Parámetros más solicitados.

## Resumen
`Parametros` ofrece un cache simple y directo para configuración operativa del sistema. Su simplicidad facilita uso inmediato, pero mejoras en refresco, seguridad y modelado de errores incrementarían robustez y escalabilidad.

---
© Documentación técnica generada automáticamente.
