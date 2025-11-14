# VTListView

## Resumen general
`VTListView` extiende `TListView` incorporando el estado de bloqueo lógico (`Bloqueado`) y la gestión visual del foco usando un color original. Ofrece una base homogénea con el resto de componentes personalizados para aplicar políticas globales de edición.

## Objetivo de la clase
Centralizar y simplificar el control de habilitación/bloqueo sin perder el estado visual original, facilitando que formularios complejos cambien su modo operativo (edición vs consulta) de forma uniforme.

## Atributos privados
- `TColor color_original`: Color previo a efectos de foco o resaltado.
- `bool FBloqueado`: Estado lógico que evita interacción (selección, edición de subitems) sin requerir deshabilitar completamente el control.
- `bool mRespaldoEnabled`: Respalda `Enabled` para restaurar al quitar bloqueo.

## Métodos protegidos
- `DoEnter()/DoExit()`: Ajustes visuales y restauración de colores.
- `KeyDown(Word &Key, TShiftState Shift)`: Control de navegación, posible bloqueo de teclas que alteren selección si está bloqueado.
- `GetEnabled()/SetEnabled(bool)`: Ajuste de habilitación física respetando bloqueo.
- `GetBloqueado()/SetBloqueado(bool)`: Acceso y modificación del estado de bloqueo.

## Métodos públicos
- `VTListView(TComponent* Owner)`: Constructor que guarda estado visual y establece valores iniciales.

## Propiedades publicadas
- `Bloqueado (bool)`: Control lógico de interacción.

## Ejemplo hipotético
```cpp
VTListView *lv = new VTListView(this);
lv->ViewStyle = vsReport;
lv->Columns->Add()->Caption = "Código";
```

## Integración
Aunque `TListView` ya ofrece rica funcionalidad, la adición de `Bloqueado` mantiene un contrato común con otros controles que simplifica la gestión global de modos de edición en la interfaz.

---
