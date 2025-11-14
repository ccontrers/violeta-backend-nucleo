# ClassCodigoBarras

## 1. Resumen
`CodigoBarras` centraliza funciones relacionadas con:
- Generación de códigos QR (para CFDI y genérico) y su asignación a objetos `TfrxPictureView` (FastReport).
- Cálculo de dígitos verificadores para estándares: EAN8, EAN13, EAN14, UPC-A (12), UPC-E.
- Validación estructural de códigos de barras según longitud y regla de dígito verificador.
- Conversión de un EAN13 numérico a la cadena ASCII que renderiza el código usando la fuente especializada "EAN-13.ttf".
- Codificación y decodificación de EAN13 con precio (prefijos 25, 26, 20) o peso (prefijo 28).

Es una clase utilitaria (sin estado persistente) que agrupa algoritmos de codificación y verificación.

## 2. Objetivos
- Evitar duplicación de lógica de validación y cálculo de dígitos verificadores en el sistema.
- Proveer una API concreta para generación de QR en reportes fiscales (CFDI 3.2/4.0+).
- Facilitar interpretación de códigos pesables / con precio integrados para punto de venta.

## 3. Alcance / No Alcance
Alcance:
- Lógica algorítmica de verificación/transformación textual.
- Interoperación con librería QR (`QRcode_encodeString`) y FastReport (`TfrxPictureView`).
- Normalización de formato monetario en parámetros CFDI.

No Alcance:
- Persistencia o registro histórico de códigos.
- Manejo de errores robusto (lanza códigos de error, usa -1 en verificadores, no excepciones formales).
- Abstracción desacoplada de framework VCL (uso directo de `AnsiString`, clases gráficas).

## 4. Dependencias
- VCL / C++ Builder: `TImage`, `Graphics::TBitmap`, `TMemoryStream`, `AnsiString`.
- FastReport: `TfrxPictureView`.
- Librería QR externa (headers: `common.h`, `qrspec.h`, `qrinput.h`, `mask.h`, `rscode.h`).
- `FuncionesGenericas` (instancia `mFg`) aunque en este archivo no se usa explícitamente; probable residuo/conveniencia futura.

## 5. Métodos Principales
### 5.1 Generación de QR
- `AsignaQRcodeTfrxPictureView(pv, EmisorRfc, ReceptorRfc, Total, UUID, Sello, CfdiVersion)`
  - Construye cadena conforme a versión CFDI:
    - 3.2: ?re=?&rr=?&tt=?&id=? (Total formateado a 0000000000.000000)
    - >=4.0 (por convención detectada): URL SAT + parámetros &fe=últimos 8 chars del sello.
  - Codifica QR en nivel de corrección M, módulo tamaño fijo (ancho=5 px por celda).
  - Pinta manualmente pixeles negros mediante `Rectangle` sobre `TBitmap`.
  - Asigna resultado a `pv->Picture`.
  - Libera recursos y llama `QRcode_free`.

- `AsignaQRcode(pv, parametro)`
  - Variante genérica: mismo proceso pero usando el parámetro directo.

Riesgo: Falta de manejo de excepciones, potenciales fugas si se produce error intermedio. Uso redundante de `img->Free(); img->Destroying();` (doble intención innecesaria).

### 5.2 Validación y Utilidades Numéricas
- `SoloDigitos(cadena)`: Verificación manual carácter por carácter con `isdigit`.
- Generadores de dígito verificador: `GeneraDigitoVerificadorEAN8`, `EAN13`, `EAN14`, `UPC12`, `UPCE` (este último con fórmula distinta ajustada a estándar compacto; devuelve valor ASCII en una rama: `(SumaEvenOdd%10)+48`).

### 5.3 Conversión para Renderizado
- `CodigoToFontEAN13(CodigoNumerico)`:
  - Verifica longitud=13 y dígitos, valida dígito verificador.
  - Mappea cada carácter a secuencias ASCII específicas según patrón de paridad dictado por primer dígito (tablas A/B), añadiendo guardas centrales (char 124) y offsets en rangos 33?42, 96?105, 48/64, 80?89, 112?121.
  - Devuelve cadena lista para impresión con fuente EAN13.

### 5.4 Decodificación de Códigos Variables
- `DecodificaEAN13Precio(Codigo, producto&, precio&, digitosprod=5)`
  - Prefijos soportados: 25, 26, 20.
  - Extrae `producto` de dígitos 3..(2+digitosprod), precio de los 5 siguientes (centavos, divide entre 100).

- `DecodificaEAN13Peso(Codigo, producto&, peso&, digitosprod=5)`
  - Prefijo 28.
  - Peso extraído como gramos (longitud variable según `digitosprod`), divide entre 1000.

- `CodificaEAN13Peso(int producto, double peso)`
  - Formato: 28 + 5 dígitos producto + 5 dígitos peso(en gramos) + dígito verificador.
  - Rellena con `sprintf` formateado y valida longitudes.

### 5.5 Verificaciones Finales
- `verificarEAN8/UPC12/EAN13/EAN14/UPCE`: Revalidan estructura y dígito verificador.
- `validarCodigoBarras(Codigo)`:
  - Recorta espacios, retorna false si es EAN13 con prefijo 28 (tratado aparte quizás para peso).
  - Según longitud delega a verificador o aprueba directamente longitudes 3 y 4 (flexibilidad legacy para códigos internos cortos).
  - Lógica especial para longitud 8 dependiendo primer dígito (0 => UPC-E). 

## 6. Algoritmos de Dígito Verificador (Resumen)
- EAN13: Suma posiciones pares (2,4,..,12) *3 + suma impares (1,3,..,11); dígito = (10 - (suma % 10)) % 10.
- EAN8 / UPC12 / EAN14: Variantes del mismo patrón con ajustes en posición inicial.
- UPC-E: Fórmula distinta (Odd*7 + Even*9 mod 10) => se ajusta añadiendo 48 en una versión; conviene normalizar para coherencia (devolver entero 0..9).

## 7. Flujo de Uso Típico (QR CFDI)
1. Reunir datos timbrados (RFC emisor, receptor, total, UUID, sello, versión).
2. Llamar `AsignaQRcodeTfrxPictureView` dentro del evento de preparación de reporte.
3. FastReport renderiza imagen en documento impreso/PDF.
4. Usuario escanea QR para verificación en SAT.

## 8. Riesgos / Issues
| Área | Descripción | Impacto | Mitigación |
|------|-------------|---------|-----------|
| Manejo memoria | Creación manual de `TBitmap`, `TMemoryStream`, `TImage` sin RAII | Fugas en excepciones | Encapsular en smart pointers / wrappers o `try/finally` | 
| Duplicación código | `AsignaQRcodeTfrxPictureView` y `AsignaQRcode` comparten 95% de lógica | Mantenimiento oneroso | Extraer función interna `GeneraQRBitmap(cadena)` | 
| Cálculo UPCE | Devuelve `(SumaEvenOdd%10)+48` (ASCII) cuando otros devuelven entero | Inconsistencia / errores al comparar | Unificar salidas numéricas | 
| Validación laxa | `validarCodigoBarras` acepta longitudes 3 y 4 sin verificar | Falsos positivos | Parametrizar reglas o restringir | 
| Magic numbers | Offsets ASCII embebidos (33,48,64,80,96,112...) | Dificultad comprensión | Constantes simbólicas / tabla estructurada | 
| Encoding | Uso `AnsiString` | Problemas multi-lenguaje | Migrar a UTF-8 (`std::string`) | 
| Error handling | Retornos -1 y bool sin diferenciación causa | Depuración difícil | Enum de error (`enum class BarcodeError`) | 
| Dependencia UI | Mezcla lógica de codificación y rendering VCL | Acoplamiento alto | Separar en servicio lógico + adaptador UI | 

## 9. Mejoras Propuestas
1. Refactor modular:
   - `BarcodeAlgorithms` (puro, STL, sin VCL).
   - `QrRenderer` (recibe interfaz abstracta de superficie dibujo).
2. Implementar pruebas unitarias para cada estándar con casos oficiales.
3. Tabla de patrones EAN13: sustituir switch por arreglo de paridad (A/B) para escalabilidad.
4. Manejo de excepciones específicas (por ejemplo, argumentos inválidos) en lugar de retornar vacío.
5. Reutilizar un único `TBitmap` externalizado para reducir asignaciones en lotes masivos.
6. Integrar soporte para QR nivel configurable (L/M/Q/H) y tamaño adaptativo a DPI destino.
7. Normalizar retorno de verificadores a tipo `int` 0..9 consistentemente.
8. Parametrizar prefijos especiales (25,26,20,28) mediante configuración externa.
9. Agregar función inversa `CodigoFromFontEAN13` para diagnóstico (si aplica).
10. Documentar referencia estándar (GS1) y versión empleada.

## 10. Contrato (Propuesto)
- Precondiciones:
  - Para verificadores: longitud y caracteres válidos según estándar.
  - Para `CodigoToFontEAN13`: cadena de 13 dígitos con dígito verificador correcto.
  - Para decodificación precio/peso: prefijo válido y dígito verificador correcto.
- Postcondiciones:
  - Verificadores devuelven -1 en caso de entrada inválida.
  - QR asignado deja `pv->Picture` poblado o sin modificar si falla internamente.

## 11. Ejemplos Rápidos
```cpp
CodigoBarras cb;
// EAN13 verificador
auto dv = cb.GeneraDigitoVerificadorEAN13("750123456789"); // -> calcula dígito 0..9
// Validar
bool ok = cb.verificarEAN13("750123456789" + AnsiString(dv));
// Codificar peso
AnsiString cod = cb.CodificaEAN13Peso(12345, 2.350); // 2.350 kg => gramos=2350
// Decodificar precio
int prod; double precio;
bool esPrecio = cb.DecodificaEAN13Precio("2512345009993", prod, precio, 5);
```

## 12. Impacto en Otros Módulos
- Usado en emisión CFDI (QR) y en módulos de punto de venta (lectura precio/peso).
- El formato correcto del QR incide en validación fiscal por terceros.
- Validación flexible (longitudes 3/4) probablemente soporta códigos internos de inventario.

## 13. Observaciones de Calidad
- Código funcional pero con mezcla de presentación y lógica.
- Falta consistencia en estilo (nombres variables locales no siempre autoexplicativos).
- Carece de comentarios sobre origen de las tablas ASCII (conocimiento implícito de especificación EAN13).

## 14. Resumen Ejecutivo
`CodigoBarras` provee utilidades críticas para codificación y verificación de códigos de barras estándar y generación de QR para CFDI. Es candidato a refactor para separar responsabilidades, mejorar manejabilidad y robustecer validación. La lógica algorítmica es correcta en esencia pero requiere normalización e instrumentación de pruebas.

---
Última revisión: 2025-09-22
