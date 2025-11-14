# ClassClienteVioleta

## Resumen General
`ClienteVioleta` centraliza toda la comunicación del cliente de escritorio con el ecosistema servidor de la plataforma (CORBA/TAO, TCP propietario y HTTP/REST). Expone un API uniforme para construir peticiones con parámetros serializados en un buffer binario, enviarlas por el canal activo y recibir respuestas empaquetadas. Además administra:
- Contexto operativo (empresa, sucursal, sección, departamento, almacén, terminal, usuario, privilegios, tokens JWT, parámetros globales).
- Slots concurrentes de petición (hasta `MAX_SLOTS_SOLICITUDES_CLIENTE` = 20) con buffers independientes para evitar colisiones en escenarios multi?hilo o reentrantes.
- Errores por slot (código, línea, mensaje) y control de bloqueo temporal del cliente.
- Detección/adaptación de protocolo (prioridad HTTP > TCP > CORBA) según parámetros de inicialización.
- Funciones utilitarias de soporte: consulta de parámetros, fechas/hora del servidor, validaciones de códigos de barras, impresión, selección de impresoras, privilegios, timers de validación de versión.

Es la puerta de entrada estándar: otras clases de negocio no hablan directamente con CORBA/PAC/HTTP sino a través de una instancia de `ClienteVioleta`.

## Objetivos
1. Unificar transporte heterogéneo detrás de una misma interfaz de peticiones/respuestas.
2. Proveer serialización compacta de parámetros (buffer contiguo de cadenas nulas) para minimizar overhead de red.
3. Aislar detalles de inicialización y reconexión CORBA / sockets / REST.
4. Mantener cache inmediato de metadata corporativa (empresa/terminal) para no repetir queries.
5. Exponer métodos de conveniencia de alto nivel (ejecución SQL simplificada, obtención de parámetros y configuraciones de impresión, códigos de barras únicos, privilegios).

## Dependencias Principales
- `FuncionesGenericas` (serialización de cadenas y helpers).
- `BufferRespuestas` (parsear resultados SELECT iniciales y otros).
- `EnlaceInterfaz`, `TablasBdCliente`, `Parametros` (puentes de UI y metadatos locales).
- `FuncionesOpenssl` (Base64 para canal HTTP, eventualmente otras transformaciones).
- Indy (`TIdTCPClient`, `TIdStack`) para transporte TCP y descubrimiento de IP.
- Embarcadero REST components (`TRESTClient`, `TRESTRequest`, `TRESTResponse`).
- CORBA TAO (`CORBA::ORB`, Naming Service) para modo legado distribuido.
- `ClassHelperFunctions.h` (asumido: constantes de IDs de solicitud, helpers de construcción).

## Estructura de Datos Interna
- Buffers por slot: `mBufferParametros[i]` (2MB c/u según `TAM_MAX_BUFFER_PARAMETROS` implícito), cursor `mPosBufferParametros[i]`.
- Estado de ocupación: `mSlotsOcupacion[i]` + contador `mNumSlotsOcupados` (protegidos por `SlotsSeccionCritica`).
- Errores por slot: arrays paralelos `mErrorNo`, `mErrorLinea`, `mErrorMsg`.
- Identidad y contexto: múltiples `AnsiString` (usuario, empresa, sucursal, terminal, etc.).
- Transporte activo: punteros / objetos (`mOrb_`, `mVioleta_`, `TCPClient`, `REST*`) y banderas.
- Cronometría: `TStopwatch sw` + `mSegundosValidacion` para control de validación de versión.
- Tokens JWT: arreglo `tokens[3]` indexado por tipo de servicio.

## Flujo de Vida
1. Constructor:
   - Reserva buffers de parámetros por slot.
   - Selecciona canal según argumentos (HTTP > TCP > CORBA).
   - Crea objetos auxiliares (`EnlaceInterfaz`, `TablasBdCliente`, `Parametros`).
   - Ejecuta SELECT inicial para poblar contexto corporativo (empresa, sucursal, almacén default, etc.).
   - Detecta IPs locales (preferencia IPv6 si presente, luego IPv4) mediante Indy.
2. Operación:
   - Para cada petición: `InicializaPeticion` ? agregar parámetros (`AgregaStringAParametros`, `AgregaPCharAParametros` / `DesplazaDirParametros`) ? `SolicitaAlServidor` (elige canal) ? `RecibeDelServidor*` según protocolo.
   - Interpretación de errores en arrays por slot.
3. Destrucción: libera buffers, objetos auxiliares, sockets/REST y opcionalmente recursos CORBA.

## Serialización de Parámetros
El encabezado de cada buffer: `[int IdSolicitud][int NumParametrosInicial (0)]` seguido de las cadenas agregadas (cada una terminada en `\0`). Al finalizar, el servidor interpreta la secuencia para reconstruir parámetros. El número de parámetros se actualiza (en la implementación completa) antes de envío o se infiere en servidor si se conoce formato.

## Canales de Comunicación
| Canal | Selección | Uso Principal | Observaciones |
|-------|-----------|---------------|---------------|
| HTTP/REST | Si se pasa `HttpUrl` | Servicios modernos (endpoint `violetalegado/ServidorLegado/solicitud`) | Params en Base64; respuesta JSON con campos `ocurrioexcepcion`, `mensajeexcepcion`, `idrespuesta`, `resultado`. |
| TCP Propietario | Si se pasa host/puerto y no HTTP | Solicitudes binarias directas | Protocolo: clave servidor (16 bytes), task code (int), longitud, buffer parámetros, flags de excepción, id_respuesta, longitud_resultado. |
| CORBA/TAO | Fallback | Infraestructura legado distribuida | Usa Naming Service, objetos `Violeta`. |

Failover entre canales no es automático; la prioridad se fija al construir.

## Manejo de Errores
- TCP/HTTP capturan excepciones (`EIdException`, `ERESTException`, genéricas) y establecen códigos negativos (-97 / -98). Mensajes incluyen recomendación de revisar conectividad y reintentar.
- Excepciones remotas (servidor) llevan un flag y longitud de mensaje; se copian a `mErrorMsg`.
- HTTP decodifica mensajes de excepción en Base64.
- Sin política de reintentos automáticos; la lógica de reintento debe estar en capas superiores.

## Métodos Principales (Resumen)
### Construcción y Slots
- `InicializaPeticion(IdSolicitud, slot)` prepara encabezado.
- `AgregaStringAParametros / AgregaPCharAParametros` serializan campos.
- `AsignarSlot()` / `LiberarSlot(slot)` (en .cpp) gestionan concurrencia de slots.

### Envío / Recepción
- `SolicitaAlServidor(...)` decide canal ? `SolicitaAlServidor_http | _tcp | _tao`.
- `RecibeDelServidor_*` obtiene buffers de respuesta (implementación no mostrada aquí pero análoga por canal) y devuelve puntero (responsabilidad de liberar).

### SQL & Datos
- `EjecutaSqlSelect`, `EjecutaSqlAccion(es)`, `EjecutaPeticionActual`, `EjecutaPeticionActual(char*& buffer)` proporcionan atajos parametrizados.
- `EjecutaConsultaTablas()` llena estructura de metadatos local `Tablas`.

### Parámetros y Configuración
- Métodos `ObtieneParam...`, `ObtieneParametrosGlobales`, `ObtieneParametroEmp`, `ObtieneParametrosGenerales` para acceder a tablas de parámetros (impuestos, impresión, cortes, versiones, etc.).

### Identidad y Contexto
- Getters para usuario, empresa, sucursal, secciones, almacén, impresoras, privilegios, tokens.
- Setters: `AsignaClaveUsuario`, `AsignaJWT`, `AsignaPrivAdminSistema`, etc.

### Utilidades Adicionales
- `CodigoBarrasYaUsado / CodigoBarrasAdicionalYaUsado` validan unicidad de código de barras contra artículos y tabla de códigos adicionales.
- `GradienteForma` (delegable a utilidades UI ? mantiene compatibilidad).
- Temporizador validación versión: inicia/detiene `sw` y calcula delta (`EmpezarTemporizadorValidacionVersion`, etc.).
- Bloqueo de cliente (`BloquearCliente` / `DesbloquearCliente`) para operaciones críticas con otro cliente temporal.

## Ejemplo de Uso (TCP/HTTP)
```cpp
short id_resp;
short slot = cliente.AsignarSlot();
cliente.InicializaPeticion(ID_SOLICITUD_BUSCAR_ARTICULO, slot);
cliente.AgregaStringAParametros("ART123", slot);
int tam = cliente.SolicitaAlServidor(id_resp, slot);
if (tam > 0) {
    char *buf = cliente.RecibeDelServidor(tam, id_resp, slot);
    BufferRespuestas r(buf);
    // procesar r...
    delete buf;
} else {
    // manejar error con cliente.ObtieneErrorMsg(slot)
}
cliente.LiberarSlot(slot);
```

## Consideraciones de Concurrencia
- Hasta 20 slots; si se excede se requiere política de espera / cola (no mostrada). 
- `TCriticalSection` protege asignación/liberación de slots; el resto de operaciones sobre buffers asumen exclusividad por slot.
- No hay protección reentrante dentro de un slot: el llamante debe sincronizar.

## Riesgos / Limitaciones
1. Tamaño fijo de buffer (2MB por slot): consumo de memoria elevado (~40MB) aunque no se usen todos; riesgo de overflow si no se valida al agregar parámetros (no se muestra guardia de límite). 
2. Código para tres protocolos en una sola clase (violación SRP); difícil test unitarios aislados.
3. Errores negativos (-97/-98) son ?magic numbers?; falta enum/documentación centralizada.
4. Falta mecanismo de reconexión automática y backoff progresivo; reconexión manual recae en capas externas.
5. Seguridad: Clave fija `w3rf.sdgxf5sl*9` en texto claro para handshake TCP; susceptible de inspección y replay.
6. Serialización casera sin longitud por campo limita validación robusta (depende del `\0`).
7. Posible fuga si `new char[...]` para buffers no se acompaña de verificación de éxito (lanzaría bad_alloc, pero no se captura).
8. Método `GradienteForma` no pertenece conceptualmente a una clase de transporte/cliente.
9. Dependencias directas de UI (TForm) y red mezcladas: complica reuso en servicios tipo consola.
10. Selección de IP local no filtra interfaces virtuales (podría tomar IP no enrutable).

## Recomendaciones de Mejora
- Extraer adaptadores de transporte (interfaces: `ITransporteCliente { enviar(buffer); recibir(); }`).
- Reemplazar buffer plano por un builder con verificación de capacidad y método `Add<T>()` tipado.
- Introducir un objeto resultado (`ResultadoPeticion`) con estado, código, mensaje, tam_respuesta, id_respuesta.
- Externalizar configuración (clave servidor, timeouts, reintentos) en archivo seguro.
- Incorporar autenticación robusta (tokens firmados) en lugar de clave estática para TCP.
- Dividir clase en: ContextoSesion, GestorParametros, GestorTransporte, RepositorioParametrosBD.
- Añadir métricas (latencia por protocolo, tasa de errores, slots usados) para tuning.
- Sanitizar y validar longitud al agregar cadenas (evitar desbordes por entradas externas anómalas).

## Contrato Simplificado
- Precondiciones: Slot asignado, petición inicializada, parámetros agregados dentro de capacidad.
- Postcondición (éxito): Valor positivo (longitud de respuesta) y buffer recuperable por `RecibeDelServidor`. 
- Postcondición (error): Código negativo (-97/-98) y campos de error por slot poblados.

## Interacciones con Otros Módulos
- Consumido por UI / procesos batch para toda operación de negocio (ventas, inventarios, facturación) indirectamente.
- Base para clases de más alto nivel que traducen estructuras específicas a secuencias de parámetros.
- `ClassComprobanteFiscalDigital` puede apoyarse indirectamente (vía servidor) para consultas de parámetros y persistencia CFDI.

## Ejemplo de Recuperación de Error
```cpp
int tam = cliente.SolicitaAlServidor(id_resp, slot);
if (tam < 0) {
    LogError(cliente.ObtieneErrorNo(slot), cliente.ObtieneErrorMsg(slot));
    // Opcional: reintentar según tipo de error
}
```

## Resumen
`ClienteVioleta` es una clase núcleo que unifica transporte, contexto y serialización de peticiones. Su potencia viene con costos en complejidad y acoplamiento; modularizar y fortalecer validaciones/seguridad mejoraría escalabilidad, mantenibilidad y resiliencia.

---
© Documentación técnica generada automáticamente.
