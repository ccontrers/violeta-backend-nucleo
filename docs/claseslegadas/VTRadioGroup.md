# VTRadioGroup

## Resumen general
`VTRadioGroup` extiende `TRadioGroup` para añadir control de modificación (`Modificado`), bloqueo lógico (`Bloqueado`), modo de sólo lectura (`ReadOnly`) y respaldo del índice seleccionado para revertir cambios si es necesario.

## Objetivo de la clase
Gestionar un conjunto de opciones mutuamente excluyentes con la capacidad de saber si el usuario alteró la selección, y controlar globalmente la posibilidad de cambiar el estado del grupo sin deshabilitarlo visualmente.

## Atributos privados
- `TColor color_original`: Color base antes de efectos de foco.
- `bool FModificado`: Señala cambio en la selección.
- `bool FBloqueado`: Estado lógico de bloqueo del grupo.
- `bool mRespaldoEnabled`: Guarda valor original de `Enabled`.
- `bool FReadOnly`: Modo consulta sin permitir cambios.
- `int mRespaldoIndex`: Índice seleccionado previo.

## Métodos protegidos
- `DoEnter()/DoExit()`: Manejan realce visual y respaldo de estado.
- `KeyDown(Word &Key, TShiftState Shift)`: Navegación entre botones internos con flechas; valida bloqueo.
- `GetEnabled()/SetEnabled(bool)`: Sincronización de habilitación con bloqueo.
- `GetBloqueado()/SetBloqueado(bool)`: Control lógico de interacción.

## Métodos públicos
- `VTRadioGroup(TComponent* Owner)`: Constructor inicializa estados y respaldo.
- `Click()`: Maneja cambios de selección, marcando `FModificado`.

## Propiedades publicadas
- `Bloqueado (bool)`
- `Modificado (bool)`
- `ReadOnly (bool)`

## Ejemplo hipotético
```cpp
VTRadioGroup *rgModo = new VTRadioGroup(this);
rgModo->Items->Add("Manual");
rgModo->Items->Add("Automático");
rgModo->ReadOnly = true; // Mantiene selección sin permitir cambios
```

## Integración
Permite a controladores de formulario aplicar políticas uniformes de edición a grupos de opciones y detectar alteraciones para guardado condicional.

---
