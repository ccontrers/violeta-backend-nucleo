# ClassExportadorDatos

## 1. Resumen
`ExportadorDatos` centraliza la salida de información tabular (principalmente desde un `VTStringGrid`) hacia distintos destinos: impresión física (a través de una abstracción `Impresora`), archivos Excel (dos variantes: generación manual de XLSX "sin formato" y con celdas ocultas, y exportación con formato usando la librería SME/SMExport), además de archivos CSV en dos sabores (simple con tabulaciones y compatible con Excel usando comillas y coma/tablador). También gestiona la inclusión de una segunda rejilla de totales (`mGridTotales`) y soporta configuración granular de: anchos por columna, colores, fuente, orientación de página, márgenes, columnas visibles/exportables y columnas que deben totalizarse.

## 2. Objetivos
- Proporcionar un único punto de exportación/impresión para datos mostrados en la UI.
- Uniformar formato visual (márgenes, fuentes, colores, bordes) y lógica de totales en los distintos destinos.
- Optimizar flujos de usuario ofreciendo varias modalidades de exportación según cantidad de datos y necesidad de formato (rápido CSV vs Excel formateado vs Excel sin formato de gran volumen).
- Evitar duplicación de código de recorrido y renderizado de celdas.

## 3. Alcance / No Alcance
Alcance:
- Renderizado a impresora página por página con salto automático y encabezados.
- Serialización básica XLSX (OpenXML) construyendo manualmente la estructura mínima (carpetas, [Content_Types], relationships, sharedStrings, sheet1) sin dependencias externas para modalidad "sin formato".
- Exportación XLSX con formato mediante SMExport (incluyendo colores y eventos de personalización de celdas).
- Exportación CSV (tabulaciones) y CSV especial para Excel (entre comillas, separaciones y preservación de decimales formateados).
- Soporte de columnas ocultas (Excel365Ocultas) basadas en ancho (<1) y exclusión de éstas.

No Alcance:
- Fórmulas complejas, autofiltros, estilos avanzados o múltiples hojas.
- Manejo de grandes volúmenes > límites codificados (p.ej. >50k filas Excel sin formato, >10k filas Excel con formato).
- Streaming incremental verdaderamente eficiente (los XML se arman en memoria en segmentos, pero no hay pipeline/flush configurable más allá de threshold 20,000 celdas).
- Internacionalización de separadores decimales (usa formateador interno `FormateaCantidad`).

## 4. Dependencias
- UI/VCL: `VTStringGrid`, `TFont`, colores `TColor`, diálogos `TSaveDialog`.
- Impresión: clase `Impresora` (métodos `BeginDocument`, `PrepareDocument`, `Text`, `Cell`, `Line`, etc.).
- Utilidades: `FuncionesGenericas mFg` para formato numérico, mensajes y cadenas (`FormateaCantidad`, `FiltraCadenaNumeroDecimal`, `AppMessageBox`, etc.).
- Librería SMExport (SME): `TSMEWizardDlg`, `TSMEStringGridDataEngine` para Excel con formato.
- Sistema de archivos: `System::Ioutils::TPath`, `TFile`, `TDirectory`, `TZipFile`, `ShellExecute`.
- Constantes y enumeraciones gráficas: alineaciones (aLeft/aRight/aCenter), estilos de fuente, colores `clWhite`, etc.

## 5. Atributos Principales
(Se derivan del header `ClassExportadorDatos.h`)
- Configuración de página:
  - `mMargenIzq/Der/Arr/Aba` (double): márgenes (cm o unidad lógica usada por `Impresora`).
  - `mOrient`: orientación (portrait/landscape).
  - `max_lineas_por_pagina`: límite manual de líneas antes de forzar salto.
- Grids:
  - `mGridOrigen`: datos base.
  - `mGridTotales`: grid de totales (se imprime después) ? puede compartir columnas.
  - `mGridActivo`: índice 0 o 1 para referenciar arrays paralelos (origen vs totales).
- Formato por columna (doble índice [col][0|1]):
  - `mAnchosColumnasImpresion`: ancho lógico en impresión.
  - `mEstadoColumnasAExportar`: bool visible/exportable.
  - `mTotalizarColumnas`: bool si se suma en la fila de total.
- Apariencia:
  - `mExportarColorCeldas[2]`: habilita colores de celda reales vs color default.
  - `mColorDefaultCeldas[2]`: fallback de color si se deshabilita color real.
  - `mColorContornoCeldas[2]`: color de bordes.
  - `mContornoColumnas[2]`: bitmask de qué bordes dibujar.
  - `mFont[2]`: fuente usada (origen y totales).
- Estado de impresión:
  - `mPaginas`, `mPaginaActual`, `mLineaActual`, `posy`: tracking vertical.
  - `largoAdicional`: factor para espaciar líneas (multiplicador).
  - `pos_linea_agrupada`: contador para limitar líneas por página.
- Excel / CSV:
  - `NombreCsvExportado`: último archivo CSV generado.
  - Bufferes temporales locales a métodos (no persistentes) para construcción de XML.
- Varios:
  - `gLetrasExcel[]`: arreglo estático de etiquetas de columna (A..Z, AA.., etc.).
  - Títulos: `mTitulo1/2/3` impresos en encabezado.
  - `cont_SMEExcelGetCellParams`: contador incremental para callback de formateo SME.

## 6. Métodos Relevantes
### 6.1 Impresión
- `EnviaImpresion()`:
  - Valida límites (<=2000 columnas, <20000 filas). Si excede informa y aborta.
  - Crea `Impresora`, prepara documento con márgenes/orientación.
  - Llama `IniciaPagina` (encabezados y títulos), exporta grid origen y luego totales usando `EnviaImpresionUnGrid`.
  - Muestra vista previa (`DVSCREEN`) y finaliza (`EndDocument`).

- `EnviaImpresionUnGrid(VTStringGrid*, Impresora*, flagContinuacion)` (privado):
  - Itera filas + (opcional) fila adicional de totales.
  - Aplica OnDrawCell del grid para forzar estilo (reutiliza lógica UI).
  - Determina alineación, colores (según `mExportarColorCeldas`), aplica contornos.
  - Calcula acumulados de columnas marcadas con `mTotalizarColumnas`.
  - Imprime cada celda con `Prn->Cell` manejando salto de página si excede altura disponible o `max_lineas_por_pagina`.
  - Renderiza fila de totales con fuente en negritas y fondo gris claro.

- `IniciaPagina(Impresora*)`:
  - Configura fuente, limpia estilos, imprime títulos centrados y numeración de página.
  - Traza línea horizontal y reajusta `posy` antes de contenido.

### 6.2 Excel (Manual OpenXML)
- `EnviaExcel365(nombre)`:
  - Modo "sin formato" (todas las columnas visibles) ? construye estructura básica de un XLSX: carpetas `_rels`, `docProps`, `xl/...` y archivos XML (`[Content_Types].xml`, `.rels`, `core.xml`, `app.xml`, `styles.xml`, `workbook.xml`, `workbook.xml.rels`, `theme1.xml`, `sharedStrings.xml`, `worksheets/sheet1.xml`).
  - Recolecta textos no numéricos en un `TStringList` (sorted + dupIgnore) para sharedStrings; las celdas numéricas se escriben con estilos según decimales (estilos 2,3,4 interpretando 0,2,3 decimales).
  - Fragmenta escritura cada 20,000 celdas para no generar un único string gigante.
  - Comprime con `TZipFile::ZipDirectoryContents` y elimina directorio temporal (que marcó como oculto).

- `EnviaExcel365Ocultas(nombre)`:
  - Igual que anterior pero excluye columnas con ancho `<1` (`mGridOrigen->ColWidths[i]`), recalculando `countColTotal` y mappeando secuencia de letras sin huecos para columnas visibles.

### 6.3 Excel con Formato (SMExport)
- `EnviaExcel(NombreArchivo)`:
  - Lanza diálogo modal `FormExportExcel` (opciones: CSV, Excel sin formato, Excel con formato, Excel sin formato con ocultas).
  - Según selección ejecuta uno de: `EnviaCsvExcel`, `EnviaExcel365`, `EnviaExcel365Ocultas` o exportación con SMExport.
  - Modalidad formato:
    - Limita a <10,000 filas; renombra archivo con sufijo `_ExcelFmt.xlsx`.
    - Configura `TSMEWizardDlg` y `TSMEStringGridDataEngine`.
    - Activa opciones de colores/fuentes, deshabilita la mayoría de configuraciones para el usuario.
    - Usa callback `SMEExcelGetCellParams` para ajustar fuente, alineación, color de fondo y tipo de celda.

### 6.4 CSV
- `EnviaCsv(NombreArchivo)`:
  - Genera archivo tab-delimited (usa `\t`) ignorando columnas de ancho 0 (solo incluye visibles >0).
  - Cada fila termina con `\n`, sin comillas.

- `EnviaCsvExcel(NombreArchivo)`:
  - Genera CSV entrecomillado con `"` para cada campo y separador `","`.
  - Formatea numéricos (FORMATO_NUMERICO_COMAS) con decimales definidos por grid para filas de datos (no cabeceras).
  - Diseñado para abrir directamente en Excel sin pérdida de formato decimal.

### 6.5 Setters / Configuración
- `AsignaAnchoColumna(col, ancho)`.
- `AsignaEstadoColumnasAExportar(col, bool)`.
- `AsignaTotalizarColumna(col, bool)`.
- `AsignaMaxLineasPorPagina(int)`.
- (Otros en header no leídos aquí pero típicos: márgenes, títulos, fuente, colores, orientación, etc.)

### 6.6 Callback SME
- `SMEExcelGetCellParams(Sender, Field, Text, AFont, Alignment, Background, CellType)`:
  - Emula un barrido lineal fila-major calculando `fila=idx/colCount` y `columna=idx%colCount` mediante contador global `cont_SMEExcelGetCellParams`.
  - Invoca `OnDrawCell` del grid para garantizar consistencia visual.
  - Decide `CellType=ctDouble` para columnas con `FORMATO_NUMERICO_COMAS` en datos (no cabeceras/fixed) tras filtrar caracteres (`FiltraCadenaNumeroDecimal`).
  - Asigna color de fondo según configuración de exportar colores o default.
  - Limita ejecución mientras `cont_SMEExcelGetCellParams < (col*row)`.

### 6.7 Internos / Defaults
- `EstableceValoresDefault()` (analizado en porción anterior): setea márgenes (1.5), orientación, fuentes Arial 9, exportar colores activado, colores base (blanco y contorno gris), anchos 2.0, totalizar=false, exportar=true para cada columna inicial.

## 7. Flujos de Uso Típicos
### 7.1 Impresión de Reporte con Totales
1. Configurar columnas a exportar y anchos.
2. Marcar columnas que deben totalizarse.
3. Asignar títulos (mTitulo1/2/3) y márgenes.
4. Llamar `EnviaImpresion()`.
5. Previsualizar y/o enviar a impresora desde interfaz generada por `Impresora`.

### 7.2 Exportación Masiva a Excel (sin formato)
1. Verificar tamaño (<5000 columnas, <50k filas).
2. `EnviaExcel365("directorioTemporal")`.
3. Sistema produce `.xlsx` y lo abre (si asociación existe).

### 7.3 Exportación con Formato (SMExport)
1. Datos <=10k filas.
2. Usuario elige opción 3 en diálogo.
3. Se genera archivo `_ExcelFmt.xlsx` aplicando colores y alineación.

### 7.4 CSV para Intercambio Rápido
- `EnviaCsv()` (tab delimited) cuando se requiere simplicidad.
- `EnviaCsvExcel()` cuando el receptor es Excel y se necesita preservar numéricos.

## 8. Algoritmos y Lógica Destacada
- Mapeo de columnas a letras: uso de gran arreglo estático `gLetrasExcel` que cubre secuencia Excel hasta longitud necesaria (evita cálculo dinámico base 26 en tiempo real).
- Segmentación de escritura de sheet XML cada 20,000 celdas para mitigar consumo de memoria en string builder.
- Detección de columnas ocultas: `ColWidths[i] < 1` considerada no exportable en modalidad Ocultas.
- Totales impresos: acumulación por columna (sólo fila de datos, excluye cabeceras) y salida formateada según decimales provistos por grid.
- Callback SME: reconstrucción del estilo sin consultar directamente al grid para cada parámetro (aplica OnDrawCell + copia de propiedades).

## 9. Límites y Validaciones Codificados
| Destino | Límite Columnas | Límite Filas | Comentario |
|---------|-----------------|--------------|------------|
| Impresión | 2000 | 20000 | Abortado con mensaje si excede |
| Excel sin formato / Ocultas | 5000 | 50000 | Sugerencia alternativa CSV si excede |
| Excel con formato (SMExport) | (implícito) | 10000 | Fila >10k aborta (orienta usar otra opción) |
| CSV | No explícito | No explícito | Limitado por memoria/IO | 

## 10. Riesgos / Issues
| Área | Descripción | Impacto | Mitigación |
|------|-------------|---------|-----------|
| Código monolítico | Método `EnviaExcel365` y `EnviaExcel365Ocultas` muy largos / repetitivos | Dificultad mantenimiento | Extraer helpers: `CrearEstructuraXlsx`, `EscribirSharedStrings`, `EscribirSheet` |
| Duplicación | Lógica de generación XML casi duplicada para versión ocultas | Bug en un camino no replicado | Parametrizar conjunto de columnas visibles y reutilizar | 
| Gestión de memoria manual | Uso de `new` para objetos (dialogs, TStringList, TZipFile) sin RAII moderno | Fugas si aparece excepción fuera del finally | `std::unique_ptr` / envoltorios RAII |
| Arreglo estático letras | Gran tabla hardcodeada | Rigidez / riesgo sobre índice si se expande | Reemplazar con función generadora base 26 |
| Internacionalización | Textos y mensajes incrustados en español | Dificultad traducir | Externalizar a recurso/localización |
| Rendimiento | Recolección de sharedStrings inserta sorted+dupIgnore (O(n log n)) | Latencia en datasets grandes | Hash + orden final opcional |
| Formato numérico | Uso de `FormateaCantidad(...,2/3)` fijo para algunos estilos | Pérdida precisión si columnas requieren otros decimales | Parametrizar estilos/decimales por columna | 
| Control flujo callback SME | Depende de índice lineal global; no valida tamaño de grid dinámico | Inconsistencias si grid cambia durante exportación | Capturar snapshot de dimensiones al inicio |
| Falta streaming ZIP | Escribe todos los archivos antes de comprimir | Memoria/IO extra | Biblioteca que permita empaquetar on-the-fly |
| Mezcla responsabilidad | Conversión datos + serialización formato + UI diálogos | Acoplamiento fuerte | Separar capa servicio (core) y capa interacción UI |

## 11. Mejoras Propuestas
1. Refactorizar exportación XLSX en una clase `XlsxWriterLite` con funciones reutilizables.
2. Generar letras Excel dinámicamente (función base 26) eliminando la tabla grande.
3. Implementar estrategia de streaming (escritura a archivo incremental) para datasets >100k.
4. Unificar generación de Excel365 y Excel365Ocultas recibiendo vector de columnas visibles.
5. Introducir capa de modelado (`ExportSchema`) que describa columnas (tipo, decimales, totalizable, visible).
6. Añadir soporte multi-hoja (split si filas > límite de 1 hoja).
7. Internacionalizar mensajes y permitir configuración de separador decimal.
8. Añadir pruebas unitarias para: totales impresos, estilos numéricos, ocultación de columnas.
9. Reemplazar CSV tab por delimitador configurable y manejo de escapes robusto (celdas con comillas, tabs, saltos de línea).
10. Exponer API sin UI (sin diálogos) para automatizaciones/scripting.

## 12. Contrato (Propuesto)
Precondiciones:
- `mGridOrigen` inicializado y con dimensiones estables durante la exportación.
- Columnas configuradas (anchos/totalizar/exportar) antes de invocar métodos.
- Para impresión: límites de filas/columnas no excedidos.

Postcondiciones:
- Archivo generado existe y se intenta abrir (si asociación del SO lo permite).
- `NombreCsvExportado` actualizado tras métodos CSV.
- En impresión: al menos una página finalizada con `EndPage` antes de `EndDocument`.

Errores / Retornos:
- Métodos no retornan valor (side-effects + mensajes). Se propone refactor a enum `ExportResult` (Ok, LimiteExcedido, ErrorIO, CanceladoUsuario).

## 13. Ejemplos Rápidos
```cpp
ExportadorDatos exp;
// Configurar
exp.AsignaAnchoColumna(0, 3.0);
exp.AsignaTotalizarColumna(5, true);
exp.AsignaMaxLineasPorPagina(60);
// Imprimir
exp.EnviaImpresion();
// Exportar Excel sin formato
exp.EnviaExcel365(""); // mostrará diálogo y generará archivo
// Exportar CSV amigable Excel
exp.EnviaCsvExcel("reporte.csv");
```

## 14. Impacto en Otros Módulos
- Interactúa con grids que probablemente son alimentados por `EnlaceInterfaz` y controlados por `ControladorInterfaz`.
- Los formatos numéricos dependen de `FuncionesGenericas`; cualquier cambio en formateo repercute en la legibilidad/interpretación externa.
- Exportaciones sirven de insumo para análisis externo, por lo que estabilidad de layout y precisión numérica afecta usuarios finales.

## 15. Observaciones de Calidad
- Balance entre funcionalidad y deuda técnica: cumple requerimientos pero con complejidad ciclomática alta.
- Mezcla lógica UI (diálogos) con núcleo de exportación limita automatización.
- Comentarios escasos en secciones de XML manual; difícil de extender a otras características de OpenXML.

## 16. Resumen Ejecutivo
`ExportadorDatos` es pieza clave de salida de información; soporta múltiples canales (impresión, Excel, CSV) con configuración detallada y manejo de totales. La implementación funciona pero es extensa y repetitiva en la parte de generación OpenXML. Una modularización y separación UI/core facilitarían mantenimiento y evolución (p.ej. añadir múltiples hojas, formatos adicionales o streaming). Priorizar refactor de Excel manual y unificación de lógica de columnas visibles reduciría riesgos de divergencia.

---
Última revisión: 2025-09-23
