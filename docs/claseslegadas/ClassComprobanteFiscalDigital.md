# ClassComprobanteFiscalDigital

## Resumen General
`ComprobanteFiscalDigital` es el orquestador central para la generación, sellado, timbrado, almacenamiento, reintento y cancelación de CFDI (3.3 y 4.0), así como de complementos asociados (Pagos 1.0 / 2.0, Carta Porte 2.0 / 3.0 internalizada como 30, Factura Global, Notas de Crédito/Cargo). Integra:
- Construcción de nodos XML mediante bindings (clases generadas `cfdiv32`, `cfdv33`, `cfdi40`, `Pagos10`, `Pagos20`, `CartaPorte30`, addenda de Carta Porte).
- Generación de cadenas originales (3.3 y 4.0) según contexto (retenciones, pagos, carta porte, factura global).
- Firmado (sello digital) a través de `FacturacionElectronica` / `FuncionesOpenssl`.
- Timbrado con PAC (Edicom y Comercio Digital) incluyendo failover y bitácoras.
- Persistencia de XML y metadatos en tablas `cfd`, `cfdxml`, bitácoras de timbrado y facturación web.
- Cancelación de CFDI (con regeneración de PFX y envío a PAC correspondiente).

## Objetivos Funcionales
1. Emitir distintos tipos de CFDI y sus variantes de versión (3.3 / 4.0) con condiciones y parámetros empresariales.
2. Mantener consistencia de folios, UUID y sellos aplicando transacciones MySQL y commits explícitos.
3. Tolerar fallos de un PAC intentando con el siguiente proveedor configurado (Edicom ? Comercio Digital).
4. Proveer reintentos controlados de timbrado y marcar registros pendientes.
5. Construir cadenas originales correctas alineadas al XSLT oficial (incluso manteniendo compatibilidad con errores conocidos del SAT mientras subsisten).
6. Gestionar addendas y complementos (Pagos, Carta Porte, Factura Global) según flags y parámetros.

## Componentes y Dependencias Clave
- `ServidorVioleta`: Ejecución de SQL, logging, mensajes, gestión de memoria, utilidades de sistema.
- `FuncionesGenericas (mFg)`: Limpieza de espacios y formateos.
- `FuncionesOpenssl (mFuncOpenssl)`: Base criptográfica y codificación Base64.
- `FacturacionElectronica`: Carga de llave y firma de cadena original.
- Bindings XML: `_di_IXMLCFDI`, `_di_IXMLCFDI33`, `_di_IXMLCFDI40`, `_di_IXMLPagos`, `_di_IXMLPagos20`, `_di_IXMLCartaPorte30`, `_di_IXMLt_AddendaCartaPorte`.
- PAC Abstraction: Clase `Timbrar` (invoca servicios SOAP / HTTP para timbrado y cancelación).

## Atributos Destacados
- Estructuras de acumulación: Arrays dinámicos de conceptos (`Array_Of_CfdConcepto`), impuestos (transladados/ retenidos), impuestos CFDI 3.3/4.0, totales, pagos.
- `mRootNodeInternetXX`: Árbol XML base según versión.
- `mCadenaOriginal`, variantes para Pagos, Carta Porte y Factura Global.
- `mSello`: Resultado del firmado.
- Bandera `factWeb`: Activa estrategias alternativas (p.ej. llenar campos obligatorios faltantes con ?Pendiente? en vez de lanzar excepción).
- Parámetros `mParamNuevoMetodo`, `mParamNuevaClave`, etc.: Ajustes contextuales de emisión (forma de pago, uso de CFDI, relación, etc.).
- `mResultadoCancelacion`: Resultado booleano de procesos de cancelación.

## Flujo General de Emisión (Venta / Nota / Pago / Carta Porte)
1. `InicializarInternet()` si no se ha hecho (COM + instancias de nodos + OpenSSL helper).
2. Determinar versión a emitir: `EmitirCFDI40` y/o `EmitirCFDIWeb44` consultan tablas de parámetros (empresa / web) y validan tipo.
3. Construir nodos XML: se llenan atributos de Emisor, Receptor, Conceptos, Impuestos, Complementos y, si aplica, CFDI Relacionados.
4. Generar cadena original: `GeneraCadenaOriginalInternet33` o `GeneraCadenaOriginalInternet40` (o variante Factura Global) dependiendo del caso, con banderas (datos de exportación, retenciones, pagos, carta porte).
5. Generar sello: `GeneraSello(LlaveB64, Password, "SHA256")` ? usa `FacturacionElectronica` y actualiza `mSello`.
6. Insertar / actualizar tablas `cfd` y `cfdxml` con XML (Base64) y datos del folio, controlado por transacciones (`START TRANSACTION`/`COMMIT`).
7. Timbrado: `TimbrarXML()` ? instancia `Timbrar`, decide orden de PAC según parámetro `pacseleccionado`, realiza hasta 2 intentos:
   - PAC 0: Edicom
   - PAC 1: Comercio Digital
   - Registra en bitácoras errores (`bitacoracfditimbrado`, `errorestimbradofactweb`) y decide si el error es de forma o conectividad.
8. Al éxito: recibe UUID, valida longitud (36), marca bitácora sin pendiente y actualiza CFD.
9. Si falla con problemas ?de forma? y es primer intento: aborta emisión (en web podría cancelar la factura provisional). Reintentos usan `ReintentaTimbrarXML()`.

## Generación de Cadenas Originales
- Métodos: `GeneraCadenaOriginalInternet33`, `GeneraCadenaOriginalInternet40`, `GeneraCadenaOriginalInternet40FacturaGlobal`.
- Patrón: Inician con "||", concatenan atributos requeridos/ opcionales con validaciones y fallback `Pentiente` cuando `factWeb` está activo para campos obligatorios vacíos.
- Se incluyen secciones condicionadas: Relacionados, Conceptos (detalle de impuestos trasladados y retenidos), Totales de impuestos.
- Para 4.0 se añade atributo `Exportacion` y reglas específicas de receptor/domicilio.
- Mantiene comentarios sobre XSLT incorrecto del SAT (se ajusta a la práctica vigente evitando romper timbrado).

## Timbrado (Pac Failover y Bitácoras)
- Selección de orden PAC: según campo `pacseleccionado` en parámetros.
- Reintentos: Mientras no se obtenga UUID y no existan errores de forma.
- Errores: Limpieza de `'` en mensajes (`ReplaceRegExpr`). Clasificación de errores 404 (conectividad) vs. otros (forma).
- Bitácoras: Inserta/actualiza en `bitacoracfditimbrado` y detalla errores específicos para facturación web (`bitacorafacturacionwebdetalles`, `errorestimbradofactweb`).

## Cancelación de CFDI
- Método `cancelarCFDI(...)`:
  - Reobtiene configuración (rfc, certificado, PFX, credenciales PAC) según el comprobante.
  - Reconstruye / carga certificado y llave (posible conversión a PFX en memoria a stream `TMemoryStream`).
  - Invoca funciones de la clase `Timbrar` para solicitar cancelación al PAC adecuado.
  - Actualiza banderas (`mResultadoCancelacion`) y registra resultado.
  - Aplica condiciones por tipo de comprobante y sucursal.

## Factura Global y Pagos
- Usa versión especializada de la cadena: `GeneraCadenaOriginalInternet40FacturaGlobal` (manejo de agregados y totales consolidables).
- Pagos 1.0/2.0: Ajuste de cadena original mediante banderas `AplicaCompPagos` y nodos complementarios (`mPagosNode`, `mPagosNode20`).

## Mecanismos de Control y Validación
- Excepciones: Uso extensivo de `throw Exception` ante inconsistencias (falta de parámetros, errores de timbrado crítico, fallos transaccionales).
- Transacciones: Explicitas para asegurar atomicidad (inicio, commit; falta visible de rollback manual en algunos paths puede implicar dependencia de autocommit o confiabilidad de flujo feliz).
- Sello/UUID: Validación de longitud UUID, base64 de XML y cadena original antes de persitir.

## Flags y Parámetros Externos
- `factWeb`: Ajusta modo tolerante (rellena ?Pendiente?) vs. modo estricto (lanza excepción).
- Parámetros de relación (`AsignaValores`) permiten: forma de pago nueva, clave método, dígitos, uso CFDI, UUID relacionado, campo fiscal relacionado.
- Campos temporales: `forma_cancelar_ticket`, `referencia_ticket`, `muuid_insert_return` usados en flujos especiales (tickets, cancelaciones, facturación web).

## Riesgos y Observaciones Técnicas
1. Acoplamiento extenso: Clase monolítica (>11k líneas) difícil de testear y mantener. Violación del principio de responsabilidad única.
2. Repetición de código entre versiones 3.3 y 4.0 (creación de nodos, armado de cadena original) sugiere necesidad de factorizar en estrategias por versión.
3. Manejo de errores: Mezcla excepciones genéricas con retornos y banderas; falta un modelo uniforme (códigos + mensajes + origen). Riesgo de commits parciales si falla antes de actualizar todas las tablas relacionadas.
4. Comentarios que preservan comportamientos incorrectos por dependencias externas (XSLT SAT) podrían quedar obsoletos sin auditoría periódica.
5. Seguridad: Uso de concatenación de SQL mediante `sprintf` con parámetros de entrada (riesgo inyección si no hay saneamiento previo aunque parezca controlado por fuentes internas). Falta parametrización sistemática.
6. Recursos COM y memory management: Liberación manual parcial comentada?posible fuga no crítica si rely on proceso global; ideal asegurar `Release()` en destructor bajo control.
7. Failover PAC: Lógica secuencial con heurística simple; no registra métricas de latencia ni circuit breaker para evitar saturar servicios en caso de fallo sistemático.
8. Ausencia de logging estructurado (solo mensajes de texto) dificulta análisis automático.
9. Validación de integridad: No se observan hash/firmas internas sobre XML persistido tras Base64 (solo sello), por lo que corrupción posterior no se detectaría fácilmente.
10. Escalabilidad: Construcción de grandes XML y múltiples conversiones Base64 pueden impactar memoria; no hay streaming parcial.

## Recomendaciones de Refactor y Mejora
- Segmentar en módulos:
  - Builder de XML (por versión y tipo de comprobante).
  - Generador de cadena original (estrategia + plantillas).
  - Servicio de timbrado (cliente PAC con política de reintentos configurable y métricas).
  - Repositorio de persistencia CFDI (operaciones CRUD y transacciones).
  - Servicio de cancelación independiente.
- Introducir objeto de resultado estándar (`struct ResultadoOperacion`) con campos: `exito`, `codigo`, `mensaje`, `detalles`, `uuid`.
- Parametrizar SQL con prepared statements evitando formateos directos.
- Implementar capa de abstracción para PACs (interface IPAC { timbrar(xml) , cancelar(uuid) }) permitiendo registrar más proveedores.
- Reutilizar árboles de nodos mediante factoría y pools para reducir costo de inicialización repetida.
- Extraer configuración (timeouts, orden de PAC, modo prueba) a un archivo JSON/YAML cargado una vez.
- Añadir pruebas unitarias sobre cadenas originales predecibles comparando contra fixtures.
- Agregar validación XSD previa al timbrado para reducir fallos de forma en PAC.
- Centralizar manejo de errores en un mapeo (código PAC ? categoría ? acción).

## Contrato Simplificado
- Precondiciones: Parámetros de facturación cargados, folios vigentes, llave privada válida, nodos XML correctamente poblados.
- Postcondición de emisión exitosa: Registro en `cfd` y `cfdxml` con UUID distinto de NULL, XML Base64 persistido, bitácoras sin pendientes.
- Postcondición de fallo de forma: Excepción con mensaje significativo; no debe quedar registro incompleto (revisar atomocidad actual).
- Postcondición de cancelación: `mResultadoCancelacion == true` y bitácoras actualizadas.

## Ejemplo de Secuencia (Resumen Venta 4.0)
1. `InicializarInternet()`
2. `EmitirCFDI40(...)` ? determina versión = true.
3. Llenar nodos Emisor/Receptor/Conceptos.
4. `mCadenaOriginal = GeneraCadenaOriginalInternet40(...)`.
5. `GeneraSello(llaveB64, password, "SHA256")`.
6. Persistir preliminarmente (insert cfd + cfdxml cadena original base64 si aplica).
7. `TimbrarXML(...)` ? recibe UUID.
8. Actualizar `cfd` con UUID + PAC.

## Integración con Otras Clases
- `FacturacionElectronica`: Firma cadena original (intermediario de OpenSSL).
- `FuncionesGenericas`: Limpieza de campos y formateo antes de enviar a PAC / persistir.
- `BufferRespuestas`: Acceso estructurado a resultados de queries.
- `Timbrar`: Cliente a PAC (timbrado/cancelación).
- `FuncionesOpenssl`: Base64 encode/decode y utilidades criptográficas directas (XML, cadena original).

## Mantenimiento y Testing Sugerido
- Crear batería de casos con combinaciones: (Venta / Nota Crédito / Pago / Carta Porte / Global) × (3.3 / 4.0) × (con / sin retenciones) × (factWeb on/off).
- Validar diferencias de cadena original contra XSLT SAT oficial; mantener un watch de cambios normativos.
- Mock de PAC para pruebas (respuestas de error simuladas: forma, conexión, duplicado de folio, certificado inválido).

## Resumen
`ComprobanteFiscalDigital` concentra todo el ciclo de vida fiscal del CFDI dentro del sistema, ofreciendo funcionalidad completa pero con alto acoplamiento y complejidad accidental. Una refactorización modular orientada a responsabilidad, más un modelo uniforme de errores y parametrización segura, elevarían mantenibilidad, testabilidad y robustez operativa.

---
© Documentación técnica generada automáticamente.
