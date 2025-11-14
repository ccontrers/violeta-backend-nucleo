# ClassFacturacionElectronica

## Resumen General
`FacturacionElectronica` es un contenedor ligero que encapsula las operaciones criptográficas necesarias para:
1. Cargar la llave privada del contribuyente (formato PKCS#8) desde archivo o contenido Base64.
2. Generar el sello digital (firma) de la ?cadena original? de un CFDI usando actualmente el algoritmo SHA256 con RSA.

Se apoya totalmente en la clase `FuncionesOpenssl` para el manejo de llaves, firma y codificación Base64.

## Responsabilidades
- Abstraer la carga de la llave privada (archivo .key o buffer Base64) protegiendo al resto del código de detalles de OpenSSL.
- Firmar la cadena original y devolver el sello en Base64.
- Gestionar el ciclo de vida del objeto helper `FuncionesOpenssl`.

## Atributos Internos
- `FuncionesOpenssl *openssl`: Instancia auxiliar que concentra la lógica de:
  - Lectura/decodificación de PKCS#8.
  - Operación de firmado RSA con SHA256.
  - Conversión a Base64 y liberación de memoria.

## Métodos Públicos
- `FacturacionElectronica()`: Constructor; reserva la instancia `FuncionesOpenssl`.
- `~FacturacionElectronica()`: Destructor; libera la instancia.
- `bool CargaLlavePrivada(char *nombre_archivo, char *password)`: Abre y desencripta el archivo `.key` (PKCS#8) devolviendo `true` si fue exitoso.
- `bool CargaLlavePrivadaSrcBase64(unsigned char *srcbase64, char *password)`: Igual que el anterior pero tomando el contenido ya codificado en Base64 (útil cuando se almacena en BD o configuración sin archivo físico).
- `AnsiString GeneraSello(char *cadena_original, char *algoritmo)`: Firma la cadena original. Flujo:
  1. Valida el algoritmo; sólo acepta `"SHA256"`.
  2. Llama a `RSAWithSHA256Sign` para obtener firma binaria.
  3. Transforma firma a Base64 con `ToBase64`.
  4. Libera buffers temporales y retorna el sello.
  5. Si el algoritmo no es soportado o la firma falla, devuelve un mensaje de error embebido en el `AnsiString`.

## Flujo de Generación de Sello
1. (Previo) Cargar llave privada: `CargaLlavePrivada` o `CargaLlavePrivadaSrcBase64`.
2. Preparar la cadena original conforme a especificación SAT (se construye externamente, p.ej. en `ComprobanteFiscalDigital`).
3. Invocar `GeneraSello(cadena, "SHA256")`.
4. Insertar el sello resultante en el XML CFDI.

## Errores y Manejo Actual
- Algoritmo no soportado: retorna texto `"ALGORITMO NO SOPORTADO ..."`.
- Fallo en firmado: retorna texto `"ERROR EN LA FUNCION RSAWITH..."`.
- No se utilizan códigos estructurados ni excepciones: el consumidor debe validar si el resultado comienza con `"ERROR"` o `"ALGORITMO"`.

## Limitaciones Identificadas
- Sólo soporta SHA256; no hay extensibilidad configurada para futuros algoritmos (ej. SHA384/SHA512) que aunque no sean estándar en CFDI podrían evaluarse.
- Mensajes de error mezclados con la ruta ?éxito? en el mismo tipo de retorno; riesgo de insertar un mensaje de error en el CFDI si no se valida.
- No valida que la llave haya sido cargada antes de firmar (confía en que el llamado es correcto); si no lo fue, la firma retornará NULL y se emitirá mensaje de error genérico.
- El parámetro `algoritmo` se usa solo para comparar literal `"SHA256"`; el resto de la cadena se utiliza parcialmente al construir mensajes (solo primer carácter por `AnsiString(*algoritmo)`), lo que hace que la parte dinámica del error sea incompleta.

## Recomendaciones de Mejora
1. Cambiar retorno de `GeneraSello` a un tipo resultado estructurado (struct con campos `exito`, `sello`, `codigoError`, `mensaje`).
2. Normalizar códigos de error (`ERR_LLAVE_NO_CARGADA`, `ERR_FIRMA_NULL`, `ERR_ALGORITMO_NO_SOPORTADO`).
3. Validar explícitamente que la llave esté cargada antes de firmar y retornar error específico.
4. Extender soporte a un enumerado de algoritmos (aunque CFDI actual exija SHA256) para facilitar pruebas o migraciones.
5. Mejorar construcción de mensajes de error: usar cadena completa del algoritmo y contexto (por ejemplo, nombre de función fallida real de OpenSSL si se obtiene).
6. Implementar limpieza segura de memoria sensible (password) si en algún punto se almacena temporalmente.

## Contrato Simplificado
- Precondición: Llave privada cargada correctamente; `cadena_original` no nula y en formato CFDI.
- Postcondición: Retorna sello Base64 válido o mensaje de error explícito.
- Invariantes: La instancia de `FuncionesOpenssl` vive durante toda la vida de `FacturacionElectronica` y se libera exactamente una vez.

## Ejemplo de Uso
```cpp
FacturacionElectronica fe;
if (!fe.CargaLlavePrivada("c\\ruta\\llave.key", "miclave")) {
    // manejar error de carga
}
AnsiString sello = fe.GeneraSello(cadenaOriginal.c_str(), (char*)"SHA256");
if (sello.Pos("ERROR") == 1 || sello.Pos("ALGORITMO") == 1) {
    // manejar error de firmado
}
// Colocar sello en el nodo <Sello>
```

## Integración con Otras Clases
- Usada por `ComprobanteFiscalDigital` para insertar el sello dentro del XML antes del timbrado (stamping) con el PAC.
- Depende de `FuncionesOpenssl` para todo el backend criptográfico; cualquier mejora o cambio de librería se realiza allí sin tocar esta capa.

## Riesgos Operativos
- Si el consumidor no valida el resultado, un mensaje de error puede terminar como contenido de `<Sello>` invalidando el CFDI.
- Falta de soporte para llaves dañadas o formatos inesperados produce mensajes genéricos difíciles de diagnosticar.

## Resumen
`FacturacionElectronica` es una fachada minimalista sobre la capa OpenSSL cuyo foco es reducir fricción al cargar llaves y firmar la cadena original CFDI. Centraliza muy poca lógica pero resulta punto crítico de fallos; robustecer su manejo de errores y validaciones elevaría la confiabilidad global.

---
© Documentación técnica generada automáticamente.
