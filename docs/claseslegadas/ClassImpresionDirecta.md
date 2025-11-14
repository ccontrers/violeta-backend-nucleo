# ClassImpresionDirecta

## 1. Resumen
`ImpresionDirecta` encapsula operaciones de bajo nivel para enviar secuencias de bytes directamente al spool de una impresora Windows y controlar dispositivos de impresión matricial / térmica usando comandos ESC/P y ESC/POS (Epson). Facilita la impresión de tickets, formularios sencillos y manejo de periféricos asociados (cajón de dinero) sin pasar por drivers genéricos de alto nivel ni motores de reporte.

## 2. Objetivos
- Proveer una API unificada para inicializar, escribir y finalizar trabajos de impresión cruda.
- Exponer un subconjunto consistente de comandos ESC/P y ESC/POS necesarios en tickets (alineación, fuentes, estilos, cortes, códigos de barras, interlineado, feed inverso). 
- Realizar conversión de caracteres con acentos/ñ al conjunto de códigos 850 utilizado por impresoras configuradas en ese modo.

## 3. Alcance / No Alcance
Alcance:
- Inicio y cierre de documento (`StartDocPrinter` / `EndDocPrinter`).
- Envío de buffers arbitrarios (texto y comandos) directamente a la cola de impresión.
- Comandos específicos ESC/P y ESC/POS para formato básico (negritas, itálica, subrayado, tamaño/fuente A/B/C, énfasis, inverso, interlineado, alineaciones, corte, expulsión de cajón, tabla de caracteres, feed inverso, salto de línea, fin de página, impresión EAN13).
- Conversión puntual Windows ANSI -> Code Page 850.

No Alcance:
- Detección de estado de impresora (online/offline, papel, errores mecánicos).
- Manejo de colas múltiples simultáneas (la instancia maneja un único `HANDLE`).
- Soporte para UTF-8 o tablas de caracteres diferentes a CP850.
- Renderizado gráfico, imágenes o QR (eso se delega a otras clases o frameworks).

## 4. Dependencias
- API Win32 de impresión: `OpenPrinter`, `StartDocPrinter`, `WritePrinter`, `EndDocPrinter`, `ClosePrinter` (winspool.h).
- Configuración externa de impresora / nombre obtenida de `ClassClienteVioleta` (`gClienteVioleta->ObtieneImpresoraTickets()`).
- C runtime (`sprintf`, `strlen`).
- Definido para entorno C++ Builder / VCL pero su lógica central es Win32 pura.

## 5. Atributos
- `mNombreImpresora[1024]`: buffer con nombre spool (printer share / nombre lógico Windows).
- `mNombreDispositivo[1024]`: salida directa (LPT1:, COM1:) si se usa (normalmente vacío en spool moderno).
- `mNombreDocumento[1024]`: etiqueta del job en cola.
- `hPrinter`: `HANDLE` a la impresora (válido entre InicializaDocumento y FinalizaDocumento).
- `mDocumentoIniciado`: bandera para validar estado activo.
- (DEBUG) `FILE *fp`: archivo de volcado de texto para diagnóstico en compilaciones `_DEBUG`.

## 6. Métodos Principales
### 6.1 Gestión de Documento
- `InicializaDocumento()`: Abre impresora, inicia job (DOC_INFO_1), envía secuencia reset ESC @ (27,64). Si un documento previo estaba abierto lo cierra primero.
- `FinalizaDocumento()`: Llama `EndDocPrinter` y `ClosePrinter`. Limpia bandera.

### 6.2 Escritura de Datos
- `Imprime(char *Cadena)`: Envía cadena ASCII tal cual.
- `ImprimeEn850(char *Cadena)`: Convierte a CP850 y luego imprime.
- `ImprimeBuffer(int longitud, char *buffer)`: Envío crudo con validación de bytes escritos.
- `ImprimeCodigo(char codigo)`: Atajo para un byte.

### 6.3 Asignación de Metadatos
- `AsignaNombreImpresora`, `AsignaNombreDispositivo`, `AsignaNombreDocumento`: Configuran buffers previos a `InicializaDocumento`.

### 6.4 Conversión de Caracteres
- `Copia850(dest, origen)`: Copia origen -> destino reemplazando vocales acentuadas, ñ/Ñ, diéresis, signos ¿ ¡, °, ª y @ a sus códigos CP850 específicos; retorna inicio de destino.
- `Convierte850(cadena)`: In-place; mismo mapeo.

### 6.5 ESC/P (impresoras matriciales clásicas)
- `ESCP_SaltaLinea()`: 10,13 (LF+CR).
- `ESCP_SaltaNLineas(n)`: n repeticiones.
- `ESCP_FijaEspaciado(pulg)`: ESC 51 n (n = pulgadas * 216 => 1/216" resolution). 
- `ESCP_FijaEspaciadoNormal()`: ESC 50 (restablece 1/6").
- `ESCP_FijaFuente10CPI()`: DC2 (18). (Tradicional 10 cust).
- `ESCP_FijaFuente15CPI()`: ESC g.
- `ESCP_FijaModoCondensado()`: ESC SI (15).
- `ESCP_CancelaModoCondensado()`: DC2 (18).
- `ESCP_FinPagina()`: Form feed (12). 
- `ESCP_FijaItalica()/CancelaItalica()`: ESC 52 / ESC 53.
- `ESCP_FijaNegrita()/CancelaNegrita()`: ESC 69 / ESC 70.
- `ESCP_ReverseFeed(lineas)`: ESC 101 n (retroalimentación de papel).

### 6.6 ESC/POS (impresoras térmicas)
- `ESCPOS_InicializaImpresora()`: ESC @.
- `ESCPOS_FijaInterlineado(n)`: ESC 51 n (1/60 de pulgada base). Default 10.
- Fuentes: `FijaFontA/B/C` -> ESC M 0/1/2.
- Estilos: `FijaEnfasado` (ESC E 1) / `CancelaEnfasado` (ESC E 2), `FijaNegritas` (ESC G 1) / `CancelaNegritas` (ESC G 0), `FijaSubrayado` (ESC 45 1) / `CancelaSubrayado` (ESC 45 0), `FijaInverso` (GS B 1) / `CancelaInverso` (GS B 0), `FijaColorAlterno` (ESC 114 1) / `CancelaColorAlterno` (ESC 114 0).
- Alineación: `FijaAlineacionIzquierda/Centro/Derecha` -> ESC 97 0/1/2.
- Corte: `ESCPOS_Corta()` -> GS V 66 1 (corte parcial). 
- Cajón: `ESCPOS_ExpulsaCajon()` -> ESC p 48 60 120 (pulso).
- Tabla caracteres 850: `ESCPOS_FijaTablaCaracteres850()` -> ESC t 2.
- Código de barras EAN13: `ESCPOS_ImprimeEAN13(char *Codigo)` -> GS k 67 12 <12 dígitos> (no imprime si longitud !=12). No calcula dígito verificador; se asume provisto aparte.

## 7. Flujo de Uso Típico
1. Configurar impresora: `AsignaNombreImpresora("EPSON TM-T20")` (si difiere del default de cliente).
2. `InicializaDocumento()`.
3. `ESCPOS_InicializaImpresora()`, seleccionar fuentes/estilos según se necesite.
4. Imprimir líneas (posible `ImprimeEn850` para texto con acentos, comandos de formato intercalados).
5. (Opcional) `ESCPOS_Corta()` y `ESCPOS_ExpulsaCajon()`.
6. `FinalizaDocumento()`.

## 8. Tabla Resumen Comandos ESC/POS
| Método | Secuencia (hex) | Descripción |
|--------|-----------------|-------------|
| ESCPOS_InicializaImpresora | 1B 40 | Reset |
| ESCPOS_FijaInterlineado(n) | 1B 33 n | Interlineado |
| ESCPOS_FijaFontA/B/C | 1B 4D 00/01/02 | Selección fuente |
| ESCPOS_FijaEnfasado/Cancela | 1B 45 01 / 1B 45 02 | Enfasis (doble strike) |
| ESCPOS_FijaNegritas/Cancela | 1B 47 01 / 1B 47 00 | Negritas |
| ESCPOS_FijaSubrayado/Cancela | 1B 2D 01 / 1B 2D 00 | Subrayado |
| ESCPOS_FijaInverso/Cancela | 1D 42 01 / 1D 42 00 | Blanco sobre negro |
| ESCPOS_FijaColorAlterno/Cancela | 1B 72 01 / 1B 72 00 | Cinta color alterno |
| ESCPOS_FijaAlineacion* | 1B 61 00/01/02 | Izq / Centro / Der |
| ESCPOS_Corta | 1D 56 42 01 | Corte parcial |
| ESCPOS_ExpulsaCajon | 1B 70 30 3C 78 | Pulso cajón |
| ESCPOS_FijaTablaCaracteres850 | 1B 74 02 | Tabla CP850 |
| ESCPOS_ImprimeEAN13 | 1D 6B 43 0C + 12 dígitos | Código barras |

## 9. Riesgos / Issues
| Área | Descripción | Impacto | Mitigación |
|------|-------------|---------|-----------|
| Buffer fijo nombres | `char[1024]` sin verificación de overflow en `strcpy` | Posible corrupción memoria | Usar `strncpy_s` o `std::string` |
| Estado global impresora tickets | Obtiene nombre de `gClienteVioleta` en ctor | Dificulta pruebas / cambio dinámico | Inyectar dependencia (setter o ctor param) |
| Falta verificación errores Win32 | Ignora códigos de error de `OpenPrinter`, `StartDocPrinter`, `WritePrinter` (más allá de bytes escritos) | Diagnóstico difícil | Revisar `GetLastError` y mapear a enum |
| Sin manejo multibyte/UTF-8 | Sólo ANSI -> CP850 | Caracteres fuera del mapeo se pierden | Implementar conversión UTF-8 -> CP850 con fallback |
| Repetición mapeo acentos | `Copia850` y `Convierte850` duplican tabla | Mantenimiento duplicado | Refactor a función central / tabla estática |
| Secuencia ESC g (15CPI) | Dependiente de modelo; no se valida compatibilidad | Impresoras no compatibles ignoran | Documentar y/o consultar manual antes uso |
| EAN13 sin checksum | Exige 12 dígitos pero no calcula dígito final | Riesgo imprimir código inválido | Integrar cálculo o validar con módulo EAN13 |
| Dependencia condicional DEBUG | Archivo debug sin control de concurrencia | Colisión si múltiples instancias | Usar log rotativo o timestamp |

## 10. Mejoras Propuestas
1. Migrar a `std::string` + `std::array<uint8_t>` para construcción de comandos tipo builder.
2. Introducir `enum class EscPosCommand` y generador de secuencias para legibilidad.
3. Añadir validación de resultado Win32 (bytes vs esperado + GetLastError) y retorno estructurado.
4. Implementar soporte UTF-8 con transliteración parcial a CP850.
5. Refactor tabla de conversión a estructura centralizada (`static std::unordered_map<char,char>`).
6. Método `BeginTicket(config)` que agrupe inicialización frecuente (reset, tabla 850, font A, interlineado default).
7. Pruebas unitarias simulando spool (usar impresora virtual) o inyección de un writer stub.
8. Agregar soporte QR ESC/POS (GS ( k ... model 2) si impresora lo soporta).
9. Exponer `Feed(lineas)` genérico y `FeedReverse(lineas)` validando rango.
10. Añadir `ImprimeLinea(const std::string &txt)` que normalice final CR/LF.

## 11. Contrato (Propuesto)
Precondiciones:
- Llamar `InicializaDocumento()` antes de cualquier función de impresión ESC/P / ESC/POS.
- Cadena a `ESCPOS_ImprimeEAN13` debe contener exactamente 12 dígitos (sin checksum) según implementación actual.

Postcondiciones:
- Tras `FinalizaDocumento()` el handle se invalida y debe reabrirse para nuevo trabajo.
- Comandos retornan `false` si el documento no está iniciado o no se escribieron todos los bytes.

Errores sugeridos (futuro):
- Enumerar estados: `NotInitialized`, `SpoolOpenError`, `StartDocError`, `WriteError`, `InvalidArgument`.

## 12. Ejemplos Rápidos
```cpp
ImpresionDirecta imp;
if (imp.InicializaDocumento()) {
    imp.ESCPOS_InicializaImpresora();
    imp.ESCPOS_FijaTablaCaracteres850();
    imp.ESCPOS_FijaAlineacionCentro();
    imp.ImprimeEn850("CAFÉ LA VIOLETA\n");
    imp.ESCPOS_FijaAlineacionIzquierda();
    imp.ImprimeEn850("Producto    $12.50\n");
    imp.ESCPOS_Corta();
    imp.ESCPOS_ExpulsaCajon();
    imp.FinalizaDocumento();
}
```

## 13. Impacto en Otros Módulos
- Usado por formularios de ventas, pagos, retiros y reimpresión de vouchers para tickets rápidos.
- Complementa a `ExportadorDatos` cuando se requiere salida de bajo nivel en lugar de impresiones estructuradas.
- Dependencia del nombre de impresora de `ClienteVioleta` lo vincula a la configuración global de la aplicación.

## 14. Observaciones de Calidad
- Implementación directa, clara y corta; fácil de portar.
- Faltan abstracciones para pruebas y extensibilidad.
- Riesgo de overflow en `strcpy` y repetición de mapeos.

## 15. Resumen Ejecutivo
`ImpresionDirecta` ofrece un puente simple y eficaz para impresión cruda y control de funcionalidades típicas de impresoras térmicas/matriciales en el POS. Su simplicidad es una fortaleza para rendimiento, pero carece de robustez en manejo de errores, seguridad de buffers y extensibilidad. Refactors propuestos mejorarían resiliencia y facilidad de mantenimiento sin alterar su interfaz esencial.

---
Última revisión: 2025-09-23
