# VTListBox

## Resumen general
`VTListBox` amplía `TListBox` añadiendo soporte para dibujo owner-draw de múltiples pseudo-columnas, colores alternos y de resaltado, control de bloqueo (`Bloqueado`), modo de solo lectura (`ReadOnly`) y bandera de modificación (`Modificado`). Mantiene un patrón consistente con otros controles de la suite para facilitar administración global de estados.

## Objetivo de la clase
Permitir presentar ítems con estructura tabular dentro de una lista estándar, controlando si el usuario puede cambiar selección o interactuar dependiendo de modos de edición/bloqueo.

## Atributos privados
- `TColor color_original`: Color previo a efectos de foco.
- `TColor color_alterno, color_resaltado`: Colores para alternar filas y para resaltar ítems seleccionados.
- `Classes::TStrings* ancho_columnas`: Lista de anchos (pixeles) que segmentan visualmente cada ítem en columnas.
- `bool FModificado`: Bandera que indica cambio de selección.
- `bool FBloqueado`: Bloqueo lógico.
- `bool mRespaldoEnabled`: Respaldo de `Enabled` real.
- `bool FReadOnly`: Modo de solo lectura (visualización sin alteración significativa).
- `int mRespaldoIndex`: Ítem seleccionado previamente para restauración.

## Accesores inline
- `GetAnchoColumnas()`: Devuelve la lista.
- `SetAnchoColumnas(TStrings*)`: Copia valores y establece `Style=lbOwnerDrawFixed` activando dibujo personalizado.

## Métodos protegidos
- `DoEnter()/DoExit()`: Manejan color y estado de selección en foco.
- `DrawItem(int Index, const TRect &Rect, TOwnerDrawState State)`: Dibuja ítems respetando `ancho_columnas` y colores.
- `KeyDown(Word &Key, TShiftState Shift)`: Controla navegación y evita cambios si `Bloqueado`.
- `GetEnabled()/SetEnabled(bool)`: Sincroniza habilitación con bloqueo.
- `GetBloqueado()/SetBloqueado(bool)`: Control del estado de bloqueo.

## Métodos públicos
- `VTListBox(TComponent* Owner)`: Constructor inicializa estructuras.
- `~VTListBox()`: Libera `ancho_columnas`.
- `Click()`: Sobre-escritura para validar bloqueo/readonly y marcar `Modificado`.

## Propiedades publicadas
- `Bloqueado (bool)`
- `AnchoColumnas (TStrings*)`
- `Modificado (bool)`
- `ReadOnly (bool)`

## Ejemplo hipotético
```cpp
VTListBox *lb = new VTListBox(this);
lb->AnchoColumnas->Add("80");
lb->AnchoColumnas->Add("160");
lb->Items->Add("COD001\tDescripción extensa");
```

## Integración
Sigue el diseño de los demás controles owner-draw (Combo, CheckListBox) para ofrecer consistencia visual y semántica en listados.

---
