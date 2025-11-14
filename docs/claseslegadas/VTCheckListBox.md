# VTCheckListBox

## Resumen general
`VTCheckListBox` extiende `TCheckListBox` para soportar:
- Control de bloqueo (`Bloqueado`) y modo de solo lectura (`ReadOnly`).
- Seguimiento de modificación (`Modificado`).
- Dibujo personalizado de ítems (owner-draw) condicionado por configuración de anchos de columna.
- Gestión de colores alternos y de resaltado (inferido por campos privados).
- Conteo rápido de elementos marcados / desmarcados.

## Objetivo de la clase
Proveer una lista con casillas que pueda presentar datos tabulares (varias columnas simuladas) mediante dibujo manual, controlar edición y detectar cambios de forma centralizada.

## Atributos privados
- `TColor color_original`: Color base previo a efectos de foco.
- `TColor color_alterno, color_resaltado`: Colores para alternar filas y resaltar selección/foco (usados en `DrawItem`).
- `Classes::TStrings* ancho_columnas`: Lista de anchos (en pixeles) para segmentar el texto de cada ítem en pseudo-columnas.
- `bool FModificado`: Indica si ha habido cambios de selección/marcado.
- `bool FBloqueado`: Estado lógico que impide interacción.
- `bool mRespaldoEnabled`: Guarda `Enabled` real al aplicar bloqueo.
- `bool FReadOnly`: Impide cambiar checks sin alterar visibilidad.
- `int mRespaldoIndex`: Índice previo seleccionado para restaurar si se cancela una acción.

## Accesores privados inline
- `GetAnchoColumnas()`: Devuelve la lista de anchos.
- `SetAnchoColumnas(TStrings* Value)`: Copia y activa modo owner-draw si hay columnas definidas (`Style=lbOwnerDrawFixed`).

## Métodos protegidos
- `DoEnter()/DoExit()`: Manejan foco y posiblemente alteran colores.
- `KeyDown(...)`: Captura teclas (espacio, navegación, selección múltiple) con lógica de bloqueo.
- `GetEnabled()/SetEnabled(bool)`: Sincronizan habilitación con bloqueo.
- `GetBloqueado()/SetBloqueado(bool)`: Acceso y mutación del estado de bloqueo.
- `SetItemData(int Index, int AData)`: Permite asociar datos extra a ítems (podría usarse para IDs).
- `MouseDown(...)`: Intercepta clicks para validar bloqueo/readonly y gestionar checks.
- `KeyPress(wchar_t &Key)`: Tratamiento de caracteres (búsqueda incremental, prevención de toggles no deseados).
- `DrawItem(int Index, const TRect &Rect, TOwnerDrawState State)`: Dibujo personalizado, utilizando `ancho_columnas`, colores e indicador de check.

## Métodos públicos
- `VTCheckListBox(TComponent* Owner)`: Constructor: inicializa lista de anchos y colores.
- `ClickCheck()`: Evento específico de cambio de estado de un check.
- `CuentaMarcados() -> int`: Devuelve cuántos ítems están marcados.
- `CuentaDesmarcados() -> int`: Devuelve el número de ítems no marcados.
- `~VTCheckListBox()`: Libera `ancho_columnas`.

## Propiedades publicadas
- `Modificado (bool)`: Bandera de cambio.
- `Bloqueado (bool)`: Control lógico de interacción.
- `AnchoColumnas (TStrings*)`: Define segmentación de columnas para owner-draw.
- `ReadOnly (bool)`: Permite visualizar sin cambiar checks.

## Notas de implementación inferidas
Para simular columnas, cada ítem puede contener texto delimitado (p.ej. por tabuladores) y `DrawItem` divide y pinta cada segmento usando los anchos en orden.

## Ejemplo hipotético
```cpp
VTCheckListBox *clb = new VTCheckListBox(this);
clb->Items->Add("ID1\tDescripción uno\tActivo");
clb->AnchoColumnas->Add("60");
clb->AnchoColumnas->Add("180");
clb->AnchoColumnas->Add("70");
clb->Bloqueado = false;
```

## Casos de uso
- Listas de selección múltiple con datos tabulares compactos.
- Interfaz donde se requiera contar rápidamente elementos seleccionados (para validar límites o mostrar totales).

## Integración en el proyecto
Mantiene la convención de `Bloqueado`/`ReadOnly` para permitir a controladores de formulario aplicar políticas de edición globales sin lógica ad-hoc por control.

---
