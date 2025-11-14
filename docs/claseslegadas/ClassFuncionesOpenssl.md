# ClassFuncionesOpenssl

## Resumen General
`FuncionesOpenssl` actúa como una capa de abstracción ligera sobre un subconjunto de la API de OpenSSL requerido por el sistema para:
- Cargar llaves privadas en formato PKCS#8 (.key) y contenedores PKCS#12 (.pfx/.p12) tanto desde archivos como desde representaciones Base64.
- Firmar cadenas usando RSA + SHA1 o RSA + SHA256 según el contexto (certificados, sellos digitales CFDI, etc.).
- Codificar/decodificar datos en Base64 sin (opcionalmente) introducir saltos de línea.
- Crear resúmenes SHA1 (digests) simples.
- Obtener metadatos del certificado X509 (número de serie, emisor) y el propio certificado codificado DER.

Se centra en operaciones de firma y manejo de credenciales fiscales, evitando exponer complejidades avanzadas de OpenSSL.

## Responsabilidades Clave
- Gestión de ciclo de vida de llaves y certificados (carga, validación, liberación de memoria).
- Conversión Base64 ? binario personalizada sin CR/LF cuando así se solicita.
- Generación de firmas criptográficas RSA para cadenas ya construidas externamente.
- Extracción de información relevante de certificados X509 cargados desde PKCS#12.

## Atributos Privados (Memoria y Estado)
- `PKCS8_PRIV_KEY_INFO *p8inf`: Estructura intermedia tras descifrar PKCS#8.
- `EVP_PKEY *pkey`: Llave privada derivada de PKCS#8 (para SHA256 / pkey genérico).
- `RSA *rsaPriv`: Estructura RSA extraída de `pkey`.
- `X509_REQ *req`: (No utilizada en las funciones actuales visibles, potencial para CSRs).
- `EVP_PKEY *puKey`: Llave pública (reservado, no usada activamente en este fragmento).
- `RSA *rsaPub`: Llave pública RSA (no usada en este fragmento).
- PKCS#12:
  - `EVP_PKEY *key_12`: Llave privada extraída del contenedor PKCS#12 (usada en firma SHA1).
  - `X509 *cert_12`: Certificado X509 principal del contenedor.

## Ciclo de Vida / Destructor
El destructor libera cuidadosamente cada objeto si fue inicializado: `EVP_PKEY_free`, `X509_REQ_free`, `PKCS8_PRIV_KEY_INFO_free`, `RSA_free`, `X509_free`. Previene fugas de memoria en ejecuciones prolongadas del servicio.

## Carga de Credenciales
### PKCS#8 (archivo)
`bool OpenPKCS8(char *filename, char *password)`
1. Abre con `BIO_new_file`.
2. Decodifica estructura `X509_SIG` -> `p8`.
3. Desencripta con `PKCS8_decrypt` usando la contraseña.
4. Convierte a `EVP_PKEY` (`EVP_PKCS82PKEY`).
5. Extrae `RSA` y valida con `RSA_check_key`.
6. Devuelve `true` si todo es correcto.

### PKCS#8 (Base64 en memoria)
`bool OpenPKCS8_srcbase64(unsigned char *srcbase64, char *password)`
- Decodifica Base64 (`FromBase64`).
- Repite proceso anterior usando `BIO_new_mem_buf`.

### PKCS#12 (archivo)
`bool OpenPKCS12(char *filename, char *password)`
- Crea BIO desde archivo.
- Decodifica a `PKCS12`.
- Parsea con `PKCS12_parse` extrayendo llave y cert.
- Verifica error `PKCS12_R_MAC_VERIFY_FAILURE`.

### PKCS#12 (Base64)
`bool OpenPKCS12_srcbase64(unsigned char *srcbase64,char *password)`
- Igual que el de archivo, pero usando buffer en memoria.

## Firmas Digitales
### RSA + SHA256
`unsigned char *RSAWithSHA256Sign(unsigned char *data)`
- Usa `pkey` (derivado de PKCS#8) y `rsaPriv`.
- Inicializa contexto `EVP_MD_CTX_create` con `EVP_sha256()`.
- Produce firma (`EVP_SignFinal`).
- Reserva buffer de salida con `OPENSSL_malloc(RSA_size(rsaPriv)+1)`.
- Devuelve puntero (el llamador debe liberar con `OPENSSL_free`).

### RSA + SHA1
`unsigned char *RSAWithSHA1Sign(unsigned char *data)`
- Usa `key_12` (llave de PKCS#12).
- Misma lógica que SHA256 pero con `EVP_sha1()`.

### Tamaños de Firma
- `int RSA_encryptedSize()` ? `RSA_size(rsaPriv)`.
- `int EvpPKeySize()` ? `EVP_PKEY_size(key_12)`.

## Base64
### Codificar
`unsigned char *ToBase64(unsigned char *input, int size, bool newline=false)`
- Construye cadena Base64 sin saltos de línea si `newline=false` (aplica flag `BIO_FLAGS_BASE64_NO_NL`).
- Agrega terminador nulo.

### Decodificar
`unsigned char *FromBase64(unsigned char *input, int *size=NULL, bool newline=false)`
- Decodifica cadena Base64; incluye terminador nulo al final del resultado.
- Si `size` no es NULL almacena longitud (incluyendo terminador).

### Liberación
`void Liberar(unsigned char *addr)` y sobrecarga para `char *` encapsulan `OPENSSL_free`.

## Digest SHA1
`unsigned char *createDigestWithSHA1(unsigned char *input)`
- Aplica `SHA1` sobre la cadena (usa `strlen` ? asume datos tipo texto sin bytes nulos internos).
- Devuelve buffer de `SHA_DIGEST_LENGTH` (20 bytes). El llamador debe liberar con `delete[]` (uso inconsistente de allocadores: ver mejoras).

## Certificado X509
- `char *getSerialNumberCertificateX509()`
  - Obtiene número de serie (`ASN1_INTEGER` -> `BIGNUM` -> decimal).
  - Convierte con `BN_bn2dec` (usa memoria de OpenSSL), pero el código mostrado libera `asciiDec` antes de retornarlo (posible bug). Documentado en Mejoras.
- `char *getIssuerCertificateX509()`
  - Retorna cadena producida por `X509_NAME_oneline` (liberable con `OPENSSL_free` usualmente, aquí se indica no liberar de inmediato; revisar consistencia en ciclo de vida).
- `unsigned char *certificateEncoded()`
  - Serializa certificado (`i2d_X509`).
  - Copia a `bufout` pero regresa `buf` (probable error: retorna puntero asignado por OpenSSL sin copia adicional y pierde referencia a `bufout`).

## Errores / Inconsistencias Detectadas
1. Memoria:
   - `createDigestWithSHA1` usa `new[]` mientras resto de métodos usan `OPENSSL_malloc`/`OPENSSL_free`. Riesgo de mezcla de allocadores.
2. `getSerialNumberCertificateX509` libera `asciiDec` ANTES de retornarlo ? retorno colgante (dangling pointer).
3. `certificateEncoded` hace copia a `bufout` pero retorna `buf` sin liberar ni devolver `bufout` ? `bufout` se pierde (fuga) y semántica confusa.
4. Falta verificación de errores exhaustiva (muchos retornos `false` sin logging/contexto).
5. Ausencia de inicialización explícita de algoritmos (comentado `OpenSSL_add_all_algorithms()`) podría requerirse en entornos donde no se llame globalmente.
6. Asume cadenas ASCII/UTF-8 sin NUL en digest y firmas (usa `strlen`).

## Recomendaciones de Mejora
- Unificar estrategia de memoria: usar siempre `OPENSSL_malloc/OPENSSL_free` o RAII wrappers (C++ smart pointers personalizados).
- Corregir funciones con retornos de punteros liberados (`getSerialNumberCertificateX509`) y punteros incorrectos (`certificateEncoded`).
- Añadir verificación y propagación de códigos de error detallados (`ERR_get_error`, `ERR_error_string`).
- Permitir firmas sobre buffers binarios (recibir longitud explícita en vez de depender de `strlen`).
- Exponer función para liberar resultados de firma/Base64 de forma uniforme (documentar claramente responsabilidad del llamador).
- Documentar codificación esperada (UTF-8) y tamaño máximo de entrada.

## Ejemplo de Uso Básico
```cpp
FuncionesOpenssl fx;
fx.InicializarAlgoritmos();
if (!fx.OpenPKCS8("c:\\certs\\llave.key", "password")) {
    // manejar error
}
unsigned char *firma = fx.RSAWithSHA256Sign((unsigned char*)"CADENA_ORIGINAL");
if (firma) {
    unsigned char *b64 = fx.ToBase64(firma, fx.RSA_encryptedSize());
    // usar b64 -> sello digital
    fx.Liberar(firma); // usando OPENSSL_free
    fx.Liberar(b64);
}
```

## Contrato Simplificado
- Precondición: para `RSAWithSHA256Sign` debe haberse cargado PKCS#8; para `RSAWithSHA1Sign`, PKCS#12.
- Postcondición: Punteros devueltos requieren liberación apropiada (`OPENSSL_free` o `delete[]` según la función; pendiente de homogeneizar).
- Errores: Devuelve `false` o `NULL` sin detalle; se sugiere ampliar.

## Riesgos y Seguridad
- Manejo de contraseñas en texto plano en memoria.
- Falta de limpieza segura (no se hace `OPENSSL_cleanse` de buffers sensibles tras uso).
- Potencial uso posterior a liberación y memory leaks descritos.
- Sin validación de longitud de entrada en Base64 (inputs muy grandes podrían impactar memoria).

## Próximas Extensiones Posibles
- Soporte para SHA384/SHA512 según requerimientos futuros de normativas.
- Encapsular en interfaz más declarativa (Builder o clase de contexto de firma).
- Exportar huella digital (fingerprint) del certificado (SHA1/SHA256) para validación cruzada.
- Comprobación de vigencia (fechas NotBefore / NotAfter) y CRL/OCSP opcional.

---
© Documentación técnica generada automáticamente.
