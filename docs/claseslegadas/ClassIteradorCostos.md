# ClassIteradorCostos

## Resumen General
`IteradorCostos` proporciona un mecanismo de iteración en dos fases sobre un conjunto de registros de movimientos (provenientes de un `BufferRespuestas`) para cálculo/aplicación de costos. Primero recorre los registros en orden natural (iteración normal) y va marcando aquellos que no se pueden aplicar todavía, acumulándolos en una lista de ?pendientes? (no aplicados). En una segunda fase alterna hacia la iteración de pendientes para reintentar aplicarlos. Cambia dinámicamente entre fases cuando detecta que cambió el par producto/presentación o al terminar la secuencia base.

## Objetivos
- Diferir la aplicación de registros dependientes de información que aún no está consolidada en iteración inicial.
- Evitar estructuras dinámicas complejas usando un arreglo simple de índices.
- Forzar la re?evaluación de pendientes al detectar cambio de clave (producto/presentación) o fin de flujo.

## Atributos Principales
- `int *mNoAplicados`: Arreglo de índices a registros marcados como ?no aplicados? (pendientes). Tamaño máximo: `MAX_TAM_BUFFER_MOV_NO_APLICADOS`.
- `int mNumNoAplicados`: Total de espacios usados en `mNoAplicados` (incluye entradas ya anuladas).
- `int mNumRealNoAplicados`: Conteo efectivo de pendientes aún vigentes (no anulados con -1).
- `int mNumRegistros`: Número total de registros en el buffer origen.
- `int mPosIteracionNormal`: Posición actual dentro de la fase normal.
- `int mPosNoAplicados`: Posición actual dentro del arreglo de pendientes durante fase de reintento.
- `int mEstadoIteracion`: 0 = iteración normal, 1 = iteración pendientes.
- `int mNumRegistroActivo`: Índice del registro actualmente posicionado.
- `AnsiString mProducto`, `mPresent`: Valores de producto/presentación del registro activo al inicio de una fase (para detectar cambio y forzar pendientes).
- `bool mForzarPendientes`: Bandera que indica si se obliga a pasar a la fase de pendientes.
- `BufferRespuestas *mBuffResp`: Fuente de datos; se indexa para acceso por número.

## Métodos Públicos
- `IteradorCostos(BufferRespuestas *BuffResp)`: Construye el iterador, indexa el buffer y establece contexto inicial (producto/presentación del primer registro).
- `void IniciaOtroProductoPresentacion()`: Reinicia los acumuladores de pendientes y vuelve a capturar producto/presentación actual. Útil si el consumidor segmenta manualmente.
- `bool IrRegistroPendiente()`: Avanza la posición del iterador según el estado actual:
  - Estado 0 (normal): se mueve secuencialmente; si detecta que existen pendientes y cambió producto/presentación, conmuta a estado 1 y reinicia recorrido de pendientes.
  - Estado 1 (pendientes): busca el siguiente índice no anulado (>=0); si se acaban, retorna a estado 0 y llama recursivamente para continuar con registros normales.
  - Devuelve `true` si posicionó algún registro, `false` si no quedan registros en ninguna fase.
- `void MarcaComoNoAplicado()`: Marca el registro actual como pendiente según la fase:
  - En fase normal: agrega índice en `mNoAplicados`, incrementa contadores y avanza a siguiente registro normal.
  - En fase pendientes: no se re?aplica; restablece iteración a comienzo de pendientes y cambia a fase normal (para que más adelante vuelva a intentar).
- `void MarcaComoSiAplicado()`: Indica que el registro actual se procesó exitosamente:
  - Fase normal: avanza registro normal y conmuta a fase pendientes (para intentar aplicar algunos diferidos).
  - Fase pendientes: anula la entrada (`-1`), decrementa `mNumRealNoAplicados` y avanza al siguiente pendiente.
- Getters varios (consultar estados y contadores): `ObtieneNumNoAplicados`, `ObtieneNumRealNoAplicados`, `ObtieneEstadoIteracion`, `ObtieneNumRegistroActivo`, `ObtieneForzarPendientes`, `ObtieneEstaTerminadaIteracionNormal`.
- Destructor: libera arreglo `mNoAplicados` (nota: usa `delete` en vez de `delete[]` ? ver mejoras).

## Flujo de Uso Típico
1. Crear instancia pasando un `BufferRespuestas` ya cargado con movimientos.
2. Llamar repetidamente a `IrRegistroPendiente()` mientras retorne `true`.
3. Para cada registro activo, decidir si se puede aplicar; llamar a `MarcaComoSiAplicado()` o `MarcaComoNoAplicado()`.
4. El iterador alternará automáticamente entre fases hasta agotar pendientes o registros.

## Ejemplo Simplificado (Pseudocódigo)
```cpp
IteradorCostos it(bufferMovs);
while (it.IrRegistroPendiente()) {
    int idx = it.ObtieneNumRegistroActivo();
    // lógica de verificación...
    bool listo = PuedeAplicarse(idx);
    if (listo) it.MarcaComoSiAplicado();
    else it.MarcaComoNoAplicado();
}
```

## Estrategia de Fases
- La alternancia temprano (después de cada éxito en fase normal) permite intentar aplicar pendientes tan pronto como nueva información (costos base, acumulados, saldos) esté disponible.
- El forzado por cambio de producto/presentación evita dejar pendientes huérfanos cuando ya no habrá nuevos registros relevantes para resolver dependencias.

## Complejidad
- Acceso y marcaje O(1) por operación.
- Re?intentos de pendientes pueden implicar múltiples pasadas sobre `mNoAplicados`; en peor caso O(n²) si casi todos quedan como pendientes hasta el final.

## Riesgos y Observaciones
- Uso de `delete` para un arreglo asignado con `new[]` (debe corregirse a `delete[]`).
- `MAX_TAM_BUFFER_MOV_NO_APLICADOS` es muy grande (8 MiB de ints ? 32 MB) si se interpreta literalmente; revisar necesidad y posible límite real (potencial desperdicio de memoria si la mayoría de movimientos aplican a la primera).
- No hay verificación de overflow al incrementar `mNumNoAplicados`.
- Supone que campos `producto` y `present` existen en el buffer (falta validación defensiva).
- La recursión en `IrRegistroPendiente()` es poco profunda pero podría reemplazarse por bucle para claridad.

## Posibles Mejoras
1. Cambiar a estructura dinámica (std::vector<int>) para ajustar memoria real utilizada.
2. Añadir verificación de límites antes de insertar en `mNoAplicados`.
3. Implementar heurística para ordenar pendientes por probabilidad de resolverse (prioridad) en lugar de secuencia cruda.
4. Exponer método para reiniciar solo la fase de pendientes sin limpiar todo (mayor control).
5. Agregar métricas: número de reintentos por registro, tiempo total, ratio de diferidos.
6. Sustituir recursion por loop para evitar stack adicional.

## Contrato Simplificado
- Precondición: `mBuffResp` indexado y con campos requeridos.
- Postcondición: Iteración visita cada registro normal al menos una vez; pendientes reintentados hasta liberarse o hasta agotar contexto.
- Error Handling: No se lanzan excepciones en lógica central; posibles fallos silenciosos si índices salen de rango o buffer carece de campos esperados.

## Resumen
`IteradorCostos` implementa un patrón de ?doble pasada adaptativa? que separa registros aplicables inmediatamente de aquellos dependientes, optimizando la probabilidad de resolver diferidos sin una tercera estructura de grafo o dependencias explícitas.

---
© Documentación técnica generada automáticamente.
