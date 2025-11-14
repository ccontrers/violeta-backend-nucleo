# ClassLienzoImpresionFormas

## 1. Resumen
`LienzoImpresionFormas` provee un buffer lógico ("lienzo") multidimensional para componer texto paginado destinado a impresoras de matriz de puntos / tickets mediante impresión directa. Permite definir un número de hojas, ancho y largo (en caracteres monoespaciados), pintar texto alineado o libremente sobre coordenadas X,Y y finalmente volcar cada hoja a una impresora usando la clase `ImpresionDirecta`. El objetivo es desacoplar el formateo posicional del envío de comandos crudos ESC/P o ESC/POS.

## 2. Objetivos
- Simplificar la maquetación de documentos de texto fijo (formularios, pólizas, tickets extendidos) sin depender de motores gráficos.
- Facilitar alineación horizontal (izquierda, centro, derecha) dentro de un ancho acotado.
- Permitir reutilización del mismo lienzo para múltiples páginas (hojas) uniformes.

## 3. Alcance / No Alcance
Alcance:
- Buffer 2D por página (caracteres ASCII/ANSI) de tamaño discreto.
- Escritura controlada con recorte automático de excedentes horizontales.
- Limpieza y reutilización del lienzo.
- Exportación secuencial de cada hoja a la impresora (con conversión CP850 y recorte de espacios finales por línea).

No Alcance:
- Manejo de estilos tipográficos (negritas, tamaño) ? se delega a comandos enviados externamente si se requiere.
- Imágenes, códigos de barras, gráficos.
- Ajuste automático de salto de línea o word wrapping inteligente (solo truncamiento simple si sobrepasa el ancho asignado en `PintaTexto` o argumento `Ancho`).

## 4. Dependencias
- `FuncionesGenericas` (`mFg`) para utilidades como `RTrim`.
- `ImpresionDirecta` para la materialización del buffer (`Convierte850`, `Imprime`).
- VCL/ANSI (`AnsiString`).

## 5. Atributos
- `mFg`: utilidades genéricas (recorte de espacios finales).
- `mNumHojas`: número de páginas definidas.
- `mHojaActual`: índice de la hoja en edición.
- `mAncho`: columnas por hoja (capado a `MAX_ANCHO_LINEA_LIENZO_IMPRESION = 256`).
- `mLargo`: filas por hoja.
- `mLienzo`: puntero dinámico a bloque contiguo `mNumHojas * mAncho * mLargo` inicializado con espacios.

## 6. Métodos
### 6.1 Construcción / Configuración
- `LienzoImpresionFormas()`: por defecto crea 1 hoja de 100x50.
- `DefineLienzo(NumHojas, Ancho, Largo)`: reserva nuevo bloque (previo delete si existía), aplica límite ancho, limpia y setea hoja activa 0.
- `LimpiaLienzo()`: rellena todo el bloque con espacios.
- `AsignaHoja(Hoja)`: cambia `mHojaActual` (sin verificación explícita de rango; asume cliente correcto).

### 6.2 Pintado
- `PintaTexto(AnsiString Texto, int X, int Y)`: Copia secuencia de caracteres desde `Texto` comenzando en `(X,Y)` recortando si excede el ancho restante de la fila. No valida Y fuera de rango (riesgo si externo envía valor inválido).
- `PintaTextoAlineado(Texto, X, Y, Ancho, Alineacion)`: Calcula desplazamiento horizontal según:
  - Izquierda: X
  - Centro: X + Ancho/2 - longitud/2
  - Derecha: X + Ancho - longitud
  Recorta texto a `Ancho` antes del cálculo y delega a `PintaTexto`.

### 6.3 Salida
- `ImprimeHoja(ImpresionDirecta *imp, int LineasIgnorar)`: Recorre filas 0..`mLargo-1`; si `i >= LineasIgnorar` toma la porción correspondiente, aplica `RTrim`, convierte CP850 y la imprime seguida de `\n`. Cada línea ocupa exactamente una salida textual. Ignora líneas iniciales (útil para formularios pre-impresos con márgenes superiores).

### 6.4 Accesores
- `ObtieneNumHojas()`, `ObtieneAncho()`, `ObtieneLargo()`: getters simples.
- Destructor: libera `mLienzo` si no es nulo.

## 7. Flujo de Uso Típico
1. `DefineLienzo(paginas, ancho, largo)` acorde a formato de formulario.
2. Para cada hoja: `AsignaHoja(i)` -> `PintaTexto` / `PintaTextoAlineado` para colocar campos.
3. Inicializar `ImpresionDirecta` y su documento.
4. Para cada hoja: `ImprimeHoja(&imp, lineasIgnorar)`.
5. Finalizar documento.

## 8. Detalles de Implementación
- Índice base para celda (X,Y,Hoja) = `(mAncho*mLargo*mHojaActual) + (mAncho*Y) + X`.
- No hay padding final: el buffer es plano; limpieza rellena todos los bytes con espacio para garantizar que se impriman blancos cuando no se sobrescriben.
- `RTrim` previa a impresión reduce ancho real escrito (optimiza ancho de línea en spool / elimina trailing blanks innecesarios).
- Sin control de overflow vertical: si Y>=mLargo se escribe fuera del bloque (UB). Igual para Hoja inválida en `AsignaHoja`. Se asume disciplina externa.

## 9. Riesgos / Issues
| Área | Descripción | Impacto | Mitigación |
|------|-------------|---------|-----------|
| Falta validación coordenadas | No se comprueba Y ni hoja en rango | Escritura fuera de límites (UB / corrupción memoria) | Añadir asserts / checks y retornar error |
| Uso de `new char[]` manual | Sin RAII | Pérdida memoria si se lanza excepción | Reemplazar por `std::vector<char>` |
| Copias inseguras | `strcpy` en buffer local alineación | Posible overflow si texto >256 | Usar `strncpy` y garantizar terminación |
| Encapsulación mínima | Cliente manipula flujo sin saber límites reales | Errores sutiles formateo | Exponer metadatos y helpers (ClampX/Y) |
| Conversión CP850 tardía | Se hace por línea en impresión | Duplicación si se reimprime | Opcional: conversión al pintar o capa caché |

## 10. Mejoras Propuestas
1. Validar rangos en `PintaTexto` y retornar bool (false si fuera de área).
2. Implementar word-wrap opcional para bloques multi-línea dentro de un ancho.
3. Cambiar almacenamiento a `std::vector<std::string>` por hoja para claridad y facilidad de manipulación.
4. Añadir API `DrawBox(x,y,w,h,char)` para marcos ASCII.
5. Integrar plantillas (`LienzoTemplate`) que definan posiciones de campos y sustitución parametrizada.
6. Soporte para juego de caracteres configurable (no sólo CP850) mediante callback de conversión.
7. Opción de exportar a texto plano acumulado (para logs o vista previa textual) antes de imprimir.

## 11. Contrato (Propuesto)
Precondiciones:
- `DefineLienzo` llamado antes de pintar si se requiere dimensiones distintas al default.
- Coordenadas X,Y dentro de rango (0<=X<mAncho, 0<=Y<mLargo).

Postcondiciones:
- Tras `ImprimeHoja`, se envían `mLargo - LineasIgnorar` líneas (cada una con `\n`).
- El lienzo no se altera durante impresión (operación de solo lectura exceptuando conversión en copia local).

Errores sugeridos futuros:
- Enum `LienzoError { Ok, FueraRango, SinMemoria }` retornado por métodos de edición.

## 12. Ejemplo Rápido
```cpp
LienzoImpresionFormas lienzo;
lienzo.DefineLienzo(2, 40, 60);
// Hoja 0
lienzo.PintaTextoAlineado("FACTURA", 0, 0, 40, la_centro);
lienzo.PintaTexto("Cliente: Juan Pérez", 0, 2);
// Hoja 1
lienzo.AsignaHoja(1);
lienzo.PintaTexto("Observaciones:", 0, 0);
ImpresionDirecta imp;
if (imp.InicializaDocumento()) {
  for (int h=0; h<lienzo.ObtieneNumHojas(); ++h) {
    lienzo.AsignaHoja(h);
    lienzo.ImprimeHoja(&imp, 0);
  }
  imp.FinalizaDocumento();
}
```

## 13. Impacto en Otros Módulos
- Complementa `ImpresionDirecta` brindando capa de layout previo.
- Útil en formularios donde se requiere alinear columnas sin lógica compleja.
- Evita dependencia de componentes visuales para impresión de plantillas ASCII.

## 14. Observaciones de Calidad
- Implementación sencilla y comprensible; mínimo de  funciones.
- Carece de defensividad (sin validaciones de entrada) lo cual limita robustez en escenarios no controlados.
- Posible expansión fácil hacia rectángulos, separadores y secciones.

## 15. Resumen Ejecutivo
`LienzoImpresionFormas` ofrece una abstracción ligera para construir páginas de texto fijo antes de imprimirlas. Su simplicidad permite rápida adopción pero requiere refuerzo en validaciones y gestión de memoria para prevenir errores silenciosos. Es un buen candidato a modernización con contenedores estándar y API más segura.

---
Última revisión: 2025-09-23
