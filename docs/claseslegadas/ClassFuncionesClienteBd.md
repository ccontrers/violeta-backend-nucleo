# ClassFuncionesClienteBd

## Resumen General
`FuncionesClienteBd` es un contenedor mínimo que ofrece utilidades puntuales de acceso a base de datos desde el cliente. Actualmente sólo implementa un método: `ObtieneParametro`, que consulta directamente el valor de un parámetro en la tabla `parametrosemp` filtrando por la sucursal activa.

## Objetivos
1. Proveer una forma rápida y directa de leer parámetros sin pasar por el cache de `Parametros` cuando se requiere el valor más reciente.
2. Encapsular la construcción del SELECT y el parseo de respuesta (`BufferRespuestas`).

## Dependencias
- `gClienteVioleta` (global): Para ejecutar el SELECT y obtener la sucursal actual.
- `BufferRespuestas`: Interpretación del buffer devuelto por el servidor.

## Método Público
### `AnsiString ObtieneParametro(AnsiString Parametro)`
- Construye: `SELECT valor FROM parametrosemp WHERE parametro='<Parametro>' AND sucursal='<sucursal_actual>'`.
- Ejecuta `EjecutaSqlSelect` mediante el cliente global.
- Crea `BufferRespuestas` sobre el resultado y devuelve el primer valor (`ObtieneDato()` por defecto el primer campo del primer registro).
- No valida si existen registros; si la consulta no retorna filas, el resultado será una cadena vacía (dependiendo de implementación de `BufferRespuestas`), sin lanzar excepción.

## Ejemplo de Uso
```cpp
FuncionesClienteBd fcb;
AnsiString corte = fcb.ObtieneParametro("FORZAR_CORTE");
if (corte == "1") {
    // aplicar lógica de corte forzado
}
```

## Comparación con `Parametros`
| Aspecto | `FuncionesClienteBd` | `Parametros` |
|---------|----------------------|--------------|
| Cache | No (consulta cada vez) | Sí (listas en memoria) |
| Refresco | Siempre actual | Requiere reconstruir o modo directo `ConsultaBaseDatos=true` |
| Acoplamiento global | Usa directamente `gClienteVioleta` | También usa global indirectamente en modo directo |
| Manejo de ausencia | Valor vacío silencioso | Lanza excepción al no encontrar en cache (modo no directo) |

## Riesgos / Limitaciones
1. Inyección SQL potencial si `Parametro` viene de entrada no controlada (concatenación directa). Debe validarse contra un whitelist o escaparse.
2. Falta de manejo de errores: no se comprueba resultado de `EjecutaSqlSelect`; si falla podría intentar parsear buffer nulo provocando acceso inválido (aunque en código actual siempre construye `BufferRespuestas`).
3. Dependencia en variable global; dificulta pruebas unitarias aisladas.
4. Estructura mínima: al crecer número de utilidades podría volverse clase ?bolsa? sin cohesión.

## Recomendaciones de Mejora
- Validar retorno de `EjecutaSqlSelect` y lanzar excepción o retornar estado claro.
- Parametrizar query o usar función central de obtención de parámetros para unificar lógica con `Parametros` (evitar duplicación).
- Introducir sobrecarga que indique si se debe lanzar excepción cuando no existan registros.
- Añadir método batch para recuperar múltiples parámetros en una sola consulta si se van a leer varios en cadena.

## Contrato Simplificado
- Precondición: Conexión activa y usuario con privilegio de lectura de `parametrosemp`.
- Postcondición: Devuelve el valor del campo `valor` de la primera fila encontrada o cadena vacía si no hay registros.

## Resumen
`FuncionesClienteBd` es una clase utilitaria minimalista enfocada a consultas directas de parámetros; es útil para lecturas frescas pero debería reforzarse en validación de errores, seguridad y posible integración con el mecanismo de cache existente.

---
© Documentación técnica generada automáticamente.
