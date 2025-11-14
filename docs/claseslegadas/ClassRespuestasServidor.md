# ClassRespuestasServidor

## Resumen General
El conjunto `RespuestaServidor` / `RespuestasServidor` gestiona resultados (buffers) generados por el servidor para solicitudes de clientes. Cada respuesta posee memoria dinámica para almacenar el resultado potencialmente grande de una operación (consultas, reportes, etc.), junto con su caducidad y un índice de logging incremental. `RespuestasServidor` administra un pool estático de tamaño fijo (`MAX_RESPUESTAS_SIMULTANEAS = 25`) y aplica sincronización (sección crítica) para operaciones concurrentes.

## Objetivos
- Reutilizar estructuras de respuesta evitando asignaciones/desasignaciones caóticas.
- Controlar memoria ocupada y caducidad para liberar respuestas expiradas automáticamente.
- Exponer API simple para crear, recuperar, borrar y monitorear respuestas.
- Proveer soporte a bitácora por respuesta mediante índice incremental (`PosLog`).

## Estructuras
### RespuestaServidor
Campos principales:
- `bool Ocupada`: Marca si la respuesta está asignada a una solicitud activa.
- `short Id`: Identificador dentro del pool.
- `char *BufferResultado`: Memoria donde se almacena el contenido de salida generado.
- `long TamBufferResultado`: Cantidad actualmente utilizada (bytes válidos escritos) dentro de `BufferResultado`.
- `long PosBufferResultado`: Offset desde el cual faltan datos por transmitir (para envíos segmentados).
- `long MemoriaOcupadaBufferResultado`: Capacidad asignada actualmente.
- `TDateTime Caducidad`: Momento en que la respuesta debe considerarse vencida y liberarse.
- `int PosLog`: Contador de eventos/entradas de log asociados.
- `FuncionesGenericas mFg`: Utilidades varias (formato, conversiones).

Métodos clave:
- `char *ObtieneDirLibreBuffer()`: Devuelve puntero al área libre inmediata para continuar escribiendo.
- `void InicializaBuffer(int Tamanio)`: (Re)asigna buffer con nueva capacidad; libera anterior si existe y reinicia contadores.

### RespuestasServidor
Campos privados:
- `RespuestaServidor mRespuestas[25]`: Arreglo estático de slots.
- Contadores agregados: `mNumRespuestasCreadas`, `mNumRespuestasOcupadas`, `mNumRespuestasCaducadasLiberadas`.
- `TCriticalSection *RespuestasSeccionCritica`: Sincronización para operaciones mutantes.

## Flujo de Uso Típico
1. Cliente genera petición ? capa superior solicita slot llamando `CrearRespuesta()`.
2. Código de negocio obtiene puntero con `Respuesta(id)` y reserva/llena buffer (usando `InicializaBuffer`).
3. Se fija caducidad con `FijarCaducidad` cuando el resultado está listo.
4. Proceso de envío al cliente lee incrementos del buffer usando `PosBufferResultado` hasta completarse.
5. Se libera explicitamente con `BorrarRespuesta(id)` o automáticamente al expirar (detectado en `ObtieneEstado`).

## Métodos de RespuestasServidor
- `RespuestasServidor()`: Inicializa contadores, marca todos los slots libres y crea la sección crítica.
- `~RespuestasServidor()`: Libera buffers ocupados aún activos y la sección crítica.
- `short CrearRespuesta()`: Busca primer slot libre; marca como ocupado, inicializa campos (caducidad fija a Now()+10 años como placeholder) y actualiza contadores.
- `void BorrarRespuesta(short)`: Envuelve `BorrarRespuestaPrivado` bajo lock.
- `void FijarCaducidad(short, TDateTime)`: Ajusta fecha de expiración de un slot ocupado.
- `RespuestaServidor *Respuesta(short)`: Valida rango y ocupado; devuelve puntero directo al slot o lanza excepción.
- `void ObtieneEstado(...)`: Recorre todos los slots, libera los caducados (log mediante `FormServidor->violeta_servant->MuestraMensaje`), totaliza memoria y produce enumeración de índices ocupados.
- `int IncPosLog(short)`: Devuelve valor actual de `PosLog` para la respuesta y lo incrementa (no valida ocupación al incrementar, solo rango).

### Internos privados
- `void BorrarRespuestaPrivado(short)`: Limpia un slot (marca desocupado, pone tamaños en 0 y libera memoria). Requiere ejecutarse dentro de la sección crítica.
- `void Totalizar()`: Declarada pero no implementada (posible remanente o plan futuro).

## Caducidad
Mientras una respuesta esté activa, `ObtieneEstado` compara `Now()` con `Caducidad`; si ya venció:
- Registra mensaje.
- Libera inmediatamente con `BorrarRespuestaPrivado`.
- Incrementa `mNumRespuestasCaducadasLiberadas`.

El valor inicial de caducidad (+10 años) evita expiración prematura antes de que el resultado esté completamente generado y se defina la caducidad real.

## Manejo de Memoria
- Buffers de respuesta se crean con `new char[]` dentro de `InicializaBuffer`.
- Se destruyen con `delete` (no `delete[]`) tanto en destructor como en liberaciones; esto es un BUG (uso correcto debería ser `delete[]`). Riesgo de comportamiento indefinido.
- Reasignación siempre limpia puntero previo.

## Concurrencia
- Operaciones mutantes (`CrearRespuesta`, `BorrarRespuesta`, `FijarCaducidad`, `ObtieneEstado`) protegen el acceso con `Acquire`/`Release` de la sección crítica.
- Acceso a `Respuesta(id)` no usa lock (lectura bajo supuesta estabilidad de estructura; esto implica que el llamador debe asegurar que el slot no sea liberado concurrentemente).

## Errores y Excepciones
- `Respuesta(short)` lanza excepción si id inválido o slot no ocupado.
- `InicializaBuffer` lanza excepción si `new` devuelve NULL (en C++ moderno esto lanza `std::bad_alloc` automáticamente; chequeo redundante).
- Falta manejo centralizado de errores (solo excepciones directas en puntos aislados).

## Posibles Mejoras
1. Sustituir memoria cruda por `std::vector<char>` o `unique_ptr<char[]>` para seguridad y evitar fugas / mismatched delete.
2. Corregir `delete` ? `delete[]` para buffers dinámicos.
3. Añadir verificación de ocupación en `IncPosLog` y quizás lock para atomicidad.
4. Exponer tamaño libre restante y capacidad para validaciones de escritura.
5. Implementar reciclaje LRU o priorización si se alcanza el límite de 25 slots (actualmente solo falla devolviendo -1 en `CrearRespuesta`).
6. Parametrizar valor placeholder de caducidad en vez de 10 años fijo.
7. Añadir métricas acumulativas de bytes transmitidos vs generados (aprovechando `PosBufferResultado`).

## Ejemplo Simplificado
```cpp
RespuestasServidor pool;
short id = pool.CrearRespuesta();
if (id >= 0) {
    RespuestaServidor *r = pool.Respuesta(id);
    r->InicializaBuffer(1024);
    strcpy(r->ObtieneDirLibreBuffer(), "RESULTADO");
    r->TamBufferResultado = strlen("RESULTADO");
    pool.FijarCaducidad(id, Now() + (1.0/24.0)); // expira en 1 hora
    // ... envío parcial usando r->PosBufferResultado ...
    pool.BorrarRespuesta(id);
}
```

## Contrato Resumido
- Máx. 25 respuestas simultáneas.
- Identificador válido: 0..24 y slot marcado ocupado.
- El llamador gestiona escritura y actualización de `TamBufferResultado` / `PosBufferResultado`.

## Riesgos / Edge Cases
- Uso de `delete` en lugar de `delete[]` para arrays: comportamiento indefinido.
- Sin reintentos si pool lleno; se devuelve -1 sin explicación detallada.
- Potencial carrera: acceso a `Respuesta(id)` mientras otro hilo libera el slot después de `ObtieneEstado`.
- Caducidades largas no gestionadas podrían acumular memoria si no se fija una real.

## Observaciones de Integración
- Depende de `FormServidor->violeta_servant->MuestraMensaje` para logging cuando expira una respuesta (acoplamiento UI/servidor).
- Utiliza `FuncionesGenericas` para formatear enteros en reportes de estado.

---
© Documentación técnica generada automáticamente.
