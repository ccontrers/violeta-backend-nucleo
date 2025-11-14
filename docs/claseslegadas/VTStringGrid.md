# VTStringGrid

## Resumen general
`VTStringGrid` extiende `TStringGrid` agregando un amplio conjunto de funcionalidades: control de estados (`Bloqueado`, `Modificado`, `ReadOnly`), formateo numérico por columna, soporte de hipervínculos lógicos, control de decimales, mecanismos de totalización (por fila/columna), dibujo personalizado de celdas con íconos y alineación/colores configurables, y funciones de ordenamiento y búsqueda. Constituye un componente central para presentación y edición tabular avanzada en el proyecto.

## Objetivo de la clase
Proporcionar una grilla versátil para datos tabulares que cubra necesidades comunes (formato, sumatorias, hipervínculos, búsqueda, resaltado, iconografía) unificando patrones de control de edición y detección de cambio.

## Enumeraciones relacionadas
- `TipoFormatoColumna`: `FORMATO_NORMAL`, `FORMATO_NUMERICO_COMAS` — determina formateo visual numérico.
- `TipoColumnaOrdenar`: `ORDENA_NUMERO`, `ORDENA_FECHAHORA`, `ORDENA_CADENA` — define estrategia de comparación al ordenar.
- `HiperLinkColumna`: Enum extenso para identificar semántica de enlaces (documentos, pagos, auditorías, etc.).
- `EstadoModifica`: `MODIFICA`, `NO_MODIFICA` — indica si la columna es editable.

## Atributos privados
- `TColor color_original`: Color base del control para efectos de foco.
- `double ConvertToDoubleDef(UnicodeString sValueString, double dlDefValue)`: Función auxiliar para parsing seguro de números con valor por defecto.
- `bool FBloqueado`: Bloqueo lógico general.
- `bool mRespaldoEnabled`: Respaldo de habilitación real.
- `bool FReadOnly`: Modo consulta sin modificaciones.
- `int mRespaldoRow, mRespaldoCol`: Posición previa (para restaurar tras eventos o pérdida de foco).
- `bool FModificado`: Indica si hubo alteraciones (contenido / selección) relevantes.
- `TFont *FFontCelda`: Fuente usada en celdas (personalizable).
- `TAlignment FAlineacionCelda`: Alineación default para dibujo de celdas.
- `TColor FColorCeldaNormal, FColorCeldaFija, FColorCeldaResaltada`: Paleta de colores para diferentes estados.
- `bool FDibujarTexto`: Bandera que decide si el texto se pinta manualmente (permite usar íconos/fondos custom antes).
- `void (*mFuncionFormateadora)(int , int , TGridDrawState)`: Callback para formateo/dibujo adicional por celda.
- `TImageList *mImageListIcono`: Lista de íconos para celdas.
- `int mIndiceIcono`: Índice de ícono a usar en dibujo.
- `UnicodeString mValorFormateado`: Último valor formateado producido (para acceso externo).
- `FuncionesGenericas mFg`: Utilidades de soporte (conversión, etc.).
- `TipoFormatoColumna FormatosColumnas[MAX_COLUMNAS_FORMATEADAS]`: Arreglo de formato por columna.
- `int DecimalesColumnas[MAX_COLUMNAS_FORMATEADAS]`: Precisión decimal por columna.
- `HiperLinkColumna TipoHiperLinkColumna[MAX_COLUMNAS_FORMATEADAS]`: Semántica de hipervínculo por columna.
- `EstadoModifica EsModificable[MAX_COLUMNAS_FORMATEADAS]`: Marcador de columnas editables/no editables.

## Métodos protegidos
- `DoEnter()/DoExit()`: Control de foco y restauración de estado visual.
- `KeyDown(Word &Key, TShiftState Shift)`: Navegación y atajos; puede interceptar teclas de edición y validar `EsModificable`.
- `GetEnabled()/SetEnabled(bool)`: Ajusta habilitación respetando bloqueo.
- `GetBloqueado()/SetBloqueado(bool)`: Control lógico del bloqueo.
- `KeyPress(wchar_t &Key)`: Filtrado de caracteres para edición directa en celdas.
- Métodos de asignación:
  - `AsignaFontCelda(TFont*)`
  - `AsignaAlineacionCelda(TAlignment)`
  - `AsignaColorCeldaNormal(TColor)`
  - `AsignaColorCeldaFija(TColor)`
  - `AsignaColorCeldaResaltada(TColor)`
  - `AsignaDibujarTexto(bool)`
  Estos encapsulan cambios en atributos y disparan redibujado.

## Métodos públicos principales
- `VTStringGrid(TComponent* Owner)`: Constructor inicializa arrays de formato y estados.
- `AsignaFuncionFormateadora(...)`: Registra callback externa para personalizar dibujo.
- `EjecutaFuncionFormateadora(int ACol, int ARow, TGridDrawState State)`: Invoca la función registrada.
- `AsignaIcono(TImageList*, int IndiceIcono)`: Configura íconos para celdas.
- `AsignarAtributosDefaultCelda()`: Restaura valores visuales por defecto.
- `DibujarCelda(...)`: Dibuja la celda completa (texto, íconos, colores) — punto central de personalización.
- `BorrarFilas(int fila_ini, int fila_fin, int minimo_filas=2)`: Elimina un rango de filas conservando un mínimo.
- `AsignaFormatoColumna(int columna, TipoFormatoColumna Formato)`: Define formato numérico para una columna.
- `ObtieneFormatoColumna(int columna)`: Devuelve formato.
- `AsignaDecimalesColumna(int columna,int Decimales=2)`: Configura precisión.
- `ObtieneDecimalesColumna(int columna)`: Recupera precisión.
- `BorrarColumnas(int columna_ini, int columna_fin, int minimo_columnas=2)`: Elimina columnas dentro de límites.
- `Ordenar(int Columna, TipoColumnaOrdenar TipoColumna=ORDENA_CADENA, bool Ascendente=true)`: Ordena las filas según tipo de dato.
- `DibujaTexto(TCanvas *tCanvas, TRect tRect, UnicodeString Valor, TAlignment Alineacion)`: Utilidad para pintar texto alineado.
- `ObtieneValorFormateado()`: Devuelve último valor formateado (posiblemente de dibujo/edición actual).
- `TotalizaColumna(int col, int fila_ini=-1, int fila_fin=-1)`: Suma valores numéricos de una columna en un rango.
- `TotalizaFila(int fila, int col_ini=-1, int col_fin=-1)`: Suma valores de una fila en un rango.
- `Encuentra(UnicodeString TextoABuscar, int Columna, bool Seleccionar=true)`: Busca un texto en la columna dada y opcionalmente selecciona la celda.
- `AsignaHiperLinkColumna(int columna,HiperLinkColumna Tipo)`: Asigna tipo de hipervínculo (para colorear/estilizar).
- `ObtieneHiperLinkColumna(int columna)`: Devuelve el tipo asignado.
- `AsignaEstadoModificable(int columna,EstadoModifica Estado)`: Marca una columna como editable o no.
- `ObtieneEstadoModificable(int columna)`: Consulta modificabilidad.
- `Click()`: Procesa interacción (posible activación de hipervínculos) respetando estado de bloqueo.
- `~VTStringGrid()`: Destructor: libera recursos (fuente, etc.) si corresponde.

## Propiedades publicadas
- `Bloqueado (bool)`
- `Modificado (bool)`
- `ReadOnly (bool)`
- Propiedades de apariencia:
  - `FontCelda (TFont*)`
  - `AlineacionCelda (TAlignment)`
  - `ColorCeldaNormal (TColor)`
  - `ColorCeldaFija (TColor)`
  - `ColorCeldaResaltada (TColor)`
  - `DibujarTexto (bool)`

## Flujos de uso típicos
1. Configuración de formatos:
```cpp
grid->AsignaFormatoColumna(2, FORMATO_NUMERICO_COMAS);
grid->AsignaDecimalesColumna(2, 2);
```
2. Cálculos:
```cpp
double total = grid->TotalizaColumna(2, 1, grid->RowCount-1);
```
3. Búsqueda:
```cpp
int fila = grid->Encuentra("ABC123", 0);
```
4. Ordenamiento:
```cpp
grid->Ordenar(0, ORDENA_CADENA, true);
```

## Casos de uso
- Grillas de documentos con totales y formateo numérico.
- Tablas con enlaces a entidades relacionadas (ventas, compras, auditorías) usando tipos de hyperlink.
- Interfaces que requieren resaltado y personalización de celdas basada en lógica externa a través de un callback.

## Notas de implementación inferidas
Los arreglos de gran tamaño (`MAX_COLUMNAS_FORMATEADAS`) permiten configuraciones dinámicas sin realocaciones. El uso de `mFuncionFormateadora` da extensibilidad sin heredar nuevamente el componente.

## Integración
Actúa como componente clave para visores/editores de datos tabulares, integrándose con el resto mediante estados uniformes de edición y mecanismos de detección de cambios.

---
