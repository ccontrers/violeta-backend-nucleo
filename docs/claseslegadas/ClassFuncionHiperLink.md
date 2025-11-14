# ClassFuncionHiperLink

## 1. Resumen
`FuncionHiperLink` actúa como un despachador de navegación contextual: dado un `VTStringGrid` y la celda activa revisa metadatos (tipo de hipervínculo configurado por columna) y abre la ventana/formulario correspondiente (ventas, compras, notas de crédito, movimientos de almacén, pagos, cartas porte, tickets de soporte, auditorías, catálogo de artículos, etc.). Encapsula la lógica de branching que tradicionalmente estaría dispersa en múltiples manejadores de eventos de la UI.

## 2. Objetivos
- Centralizar la lógica de apertura de formularios a partir de datos tabulares hipervinculados.
- Reducir duplicación de código al crear y mostrar formularios modales/pop-up homogéneamente.
- Abstraer la detección de tipo de documento (venta vs pedido vs nota de crédito) de la capa visual.

## 3. Alcance / No Alcance
Alcance:
- Determinación del formulario a instanciar según código de hiperlink (`LINK_*`) configurado en la columna del grid.
- Extracción de información adicional de otras columnas para distinguir subtipos (p.ej. tipo de nota de crédito, tipo de movimiento, si es venta ?super?).
- Creación, parametrización (`AsignaReferencia`, `Carga*`, `SetReferencia`, etc.) y apertura (`Abrir()`) de formularios hijos.
- Limpieza de instancias con `try/__finally` garantizando destrucción.

No Alcance:
- Manejo de errores de base de datos internos de cada formulario.
- Cache de formularios (siempre crea uno nuevo).
- Validación de permisos/privilegios (asumido gestionado en otros niveles).

## 4. Dependencias
- `VTStringGrid`: métodos `Cells`, `ObtieneHiperLinkColumna`, `ColCount`.
- Múltiples formularios (ventas, compras, notas crédito/cargo, auditoría, catálogo, tickets, cartas porte) ? fuerte acoplamiento a la capa visual.
- `ClassClienteVioleta` para consultas SQL auxiliares (determinar venta ?super?, cartas porte referenciadas).
- `FuncionesGenericas` (instancia `mFg`, no explotada en el fragmento visible salvo posible uso futuro).

## 5. Atributos
- `FuncionesGenericas mFg`: utilidades genéricas (poco usado aquí).
- Métodos privados especializados: `notaCreditoProve`, `notaCreditoClie`, `ventas`, `ventasSuper`, `compras`, `pagoProv`, `pagoCliente`, `entrada`, `pedidos`, `pedidosfill`, `notacargocli`, `notacargoprov`, `pedidoscli`, `pedidosCompras`.

## 6. Método Público Clave
### `MostrarFormulario(VTStringGrid *Grid, TCustomForm* Owner)`
Flujo principal:
1. Verifica que la celda activa no contenga ?NULL?, espacio ni cadena vacía.
2. Obtiene código de hiperlink de la columna activa `a = Grid->ObtieneHiperLinkColumna(Grid->Col)`.
3. Gran cadena de `if/else if` evaluando constantes `LINK_*` (ejemplos: `LINK_MOVI`, `LINK_TRNF`, `LINK_VNTA`, `LINK_COMP`, `LINK_PEDIDOS`, `LINK_GASTOS_COMPRA`, `LINK_PAGO_CLI`, `LINK_NCRE_CLI`, `LINK_CART_POR`, `LINK_NCAR`, `LINK_PAGO_GAST`, `LINK_VNTA_NCRECLI`, `LINK_PRECALCULOS`, `LINK_DOCMOD`, `LINK_REPCFD`, `LINK_AUDMOV`, `LINK_REPCOSEXT`, `LINK_REPSEGART`, `LINK_PAGO_PROV`, `LINK_MOVI_ALMA`, `LINK_COMP_NCLPRO`, `LINK_CATA_ARTI`, `LINK_PED_COM_REL`, `LINK_SOL_TICKET_SOP`, `LINK_AUD_SURTIDO`, `LINK_AUD_RECEPCION`).
4. Para cada código invoca método helper o construye formulario en línea, establece propiedades (`PopupMode=pmExplicit`, `PopupParent=Owner`, capturas de referencia) y llama `Abrir()`.
5. Selección contextual adicional:
   - Determina venta ?super? consultando DB (`ventasuper`) para decidir `ventas` vs `ventasSuper`.
   - Extrae tipo de nota de crédito de columnas ?Tipo Nota?.
   - Interpreta tipos genéricos (CO, VE, ES, SA, DC, BC, DV) en reportes de costos.
   - Resuelve carta porte a partir de campos disponibles (venta, pedido, pedido compra) mediante queries si se usa `CAR2`.
   - Maneja listas múltiples (pedidos de compras concatenados con coma) en `pedidosCompras` construyendo un `TStringList`.

### Helpers Principales (Privados)
- `compras(referencia)`: abre `TFormaCompras` (modo compra).
- `pedidos(referencia)`: `TFormaCompras` en modo pedido (constructor param distinto).
- `ventas(referencia)` / `ventasSuper(referencia)`: abre formularios de venta estándar o ?super?.
- `notaCreditoProve(tipo, ref)` / `notaCreditoClie(tipo, ref)`: abren notas crédito proveedor / cliente con título según tipo (devolución, bonificación, descuento global).
- `pagoProv(ref)`, `pagoCliente(ref)`: abren formularios de pagos.
- `entrada(ref)`: abre movimiento de almacén.
- `notacargocli(ref)`, `notacargoprov(ref)`.
- `pedidoscli(ref)`, `pedidosfill(ref)` (variantes para pedidos cliente).
- `pedidosCompras(refs)`: parsea referencias separadas por coma y abre un formulario para lote.

## 7. Flujo de Interacción Típico
1. Usuario hace clic (doble clic) o acción asociada sobre celda hipervinculada en un grid.
2. UI invoca `MostrarFormulario(grid, this)` desde el formulario contenedor.
3. `FuncionHiperLink` identifica el tipo y abre el formulario correspondiente, modal o en popup sobre el mismo Owner.
4. Usuario cierra formulario emergente; el objeto se destruye inmediatamente (no retención en memoria).

## 8. Mapeo Simplificado LINK_* ? Acción
| LINK_* (ejemplos) | Acción | Notas |
|------------------|--------|-------|
| LINK_VNTA / LINK_VNTA_NCRECLI | Venta o derivado (venta, pago, nota crédito) | Distinción por columna ?Tipo? |
| LINK_COMP / LINK_GASTOS_COMPRA | Compra o gasto | Determina tipo buscando cabecera ?Tipo? |
| LINK_PED / LINK_PEDIDOS / LINK_PED_CLI | Pedido (compra/cliente) | Diferentes formularios según contexto |
| LINK_NCRE_CLI / LINK_NCRE_PRO | Nota crédito cliente / proveedor | Deduce tipo (0,1,2) por columna ?Tipo Nota? |
| LINK_PAGO_CLI / LINK_PAGO_PRO / LINK_PAGO_GAST | Pagos | Distinción proveedor vs cliente vs gastos |
| LINK_MOVI / LINK_MOVI_ALMA / LINK_AUDMOV | Movimiento almacen / transformación | Puede abrir transformaciones |
| LINK_CART_POR / CAR2 | Carta porte | Construye referencia y tipo vía queries |
| LINK_CATA_ARTI | Catálogo artículos | Muestra ficha artículo |
| LINK_REPCFD / LINK_PRECALCULOS / LINK_DOCMOD | Mixto (CFDI, costos, documentos modificados) | Lógica condicional extensa |
| LINK_SOL_TICKET_SOP | Tickets soporte/área (RH, MER, DES, etc.) | Selección por valor en columna 17 |
| LINK_AUD_SURTIDO / LINK_AUD_RECEPCION | Auditorías surtido/recepción | Abre formularios específicos |
| LINK_PED_COM_REL | Pedidos relacionados (múltiples) | Parseo lista coma ? StringList |

## 9. Riesgos / Issues
| Área | Descripción | Impacto | Mitigación |
|------|-------------|---------|-----------|
| Cadena if/else extensa | 800+ líneas de branching | Difícil de mantener / propenso a errores | Refactor a tabla de despacho (map<enum, lambda>) |
| Acoplamiento fuerte UI | Referencia a docenas de formularios concretos | Cambios de nombre rompen compilación | Introducir interfaz abstracta y fábrica |
| Duplicación lógica notas crédito | Construcción de caption repetida | Inconsistencias potenciales | Función utilitaria central `SetTituloNotaCredito(...)` |
| Acceso directo a celdas | Hardcodes índices (p.ej. Cells[16], [17]) | Fragilidad si estructura de grid cambia | Usar constantes simbólicas y validar rango |
| Consultas SQL ad-hoc | Embebidas en método principal | Mezcla presentación y datos | Extraer a servicio `DocumentoMetadataService` |
| Sin control de errores | Fallo al crear formulario no capturado | Crash silencioso | Try-catch con logging y mensaje usuario |
| Recursos fugas potenciales | Sólo `__finally` en algunos, pero riesgo en excepciones antes de `new`? | Memoria/residuos | Smart pointers (RAII) / unique_ptr |
| Internacionalización | Cadenas UI hardcode en español | Dificultad de traducción | Externalizar recursos |

## 10. Mejoras Propuestas
1. Introducir enumeración fuerte `enum class HyperLinkType` mapeada desde `ObtieneHiperLinkColumna` para claridad.
2. Reemplazar cadena `if/else` por tabla: vector de estructuras `{tipo, handler}` con lambdas capturando `this`.
3. Extraer sub-flujos especializados (notas crédito, cartas porte, tickets) a clases dedicadas (Strategy / Command).
4. Definir constantes para índices de columnas usados (e.g. `COL_TIPO=2`, `COL_MOV_TIPO=16`, etc.).
5. Unificar creación de formularios con helper templado: `withForm<T>(Owner, lambdaConfig)` asegurando RAII.
6. Cache opcional de formularios de consulta read-only para mejorar performance.
7. Registro de auditoría: cada navegación podría loguearse para análisis de uso.

## 11. Contrato (Propuesto)
Precondiciones:
- `Grid` válido y con fila/columna seleccionada dentro de rango.
- `ObtieneHiperLinkColumna(col)` retorna un código reconocido (si no, no hace nada).

Postcondiciones:
- Formulario correspondiente se muestra (modal/popup) o se informa al usuario si el tipo no coincide.
- No persisten instancias de formularios al regresar (ciclo de vida se mantiene dentro del handler).

Errores futuros definidos (sugerido):
- Retornar `bool` indicando si se abrió un formulario.
- Logging detallado al fallar creación o apertura (`Abrir()`).

## 12. Ejemplo Rápido
```cpp
// En evento OnDblClick de un VTStringGrid
FuncionHiperLink fh;
fh.MostrarFormulario(StringGridReporte, this); // Abre según hiperlink configurado
```

## 13. Impacto en Otros Módulos
- Punto de entrada crítico para navegación cruzada: cambios rompen flujos de auditoría, cobranza, logística.
- Utilizado desde numerosos formularios reportados (ventas, auditorías, reportes de costo, tickets soporte). Centralizarlo mejora consistencia de UX.

## 14. Observaciones de Calidad
- Funcional pero altamente procedural y acoplado; difícil de extender sin riesgo.
- Mezcla niveles (presentación + queries SQL + lógica negocio).
- Oportuno para refactor por patrones Command/Factory y reducción de 800 líneas a tabla declarativa.

## 15. Resumen Ejecutivo
`FuncionHiperLink` es el enrutador central de hipervínculos en grids, permitiendo abrir múltiples tipos de documentos sin que cada formulario duplique la lógica. El diseño actual cumple su cometido pero con un costo de mantenibilidad alto debido a la cadena de condicionales y acoplamientos. Un rediseño basado en despacho declarativo y separación de responsabilidades aportaría claridad y reduciría deuda técnica.

---
Última revisión: 2025-09-23
