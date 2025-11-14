# VTRadioButton

## Resumen general
`VTRadioButton` amplía `TRadioButton` añadiendo soporte a los estados compartidos del framework (`Bloqueado`, `Modificado`, `ReadOnly`) y control sobre la transición del estado marcado. Gestiona efectos de foco y teclado para garantizar una experiencia consistente.

## Objetivo de la clase
Ofrecer un radio button que pueda diferenciar entre estar bloqueado, en modo consulta o modificable, registrando además si su estado cambió por interacción del usuario.

## Atributos privados
- `TColor color_original`: Color base para restaurar tras efectos de foco.
- `bool FModificado`: Bandera que indica si el valor cambió respecto a su estado inicial.
- `bool FBloqueado`: Estado lógico que evita modificación.
- `bool mRespaldoEnabled`: Respaldo del valor `Enabled` original.
- `bool FReadOnly`: Modo de sólo lectura.

## Métodos protegidos
- `DoEnter()/DoExit()`: Ajustan estilo visual al entrar/salir de foco.
- `KeyDown(Word &Key, TShiftState Shift)`: Procesa navegación (flechas) y tecla de selección, respetando bloqueo/readonly.
- `GetEnabled()/SetEnabled(bool)`: Sincroniza habilitación con estado de bloqueo.
- `GetBloqueado()/SetBloqueado(bool)`: Control de bloqueo lógico.
- `SetChecked(bool Value)`: Controla cambios en selección, marcando `FModificado` y validando permisos.

## Métodos públicos
- `VTRadioButton(TComponent* Owner)`: Constructor estándar.
- `Click()`: Sobrescribe el evento de clic para validar estados antes de propagar el evento.

## Propiedades publicadas
- `Bloqueado (bool)`
- `Modificado (bool)`
- `ReadOnly (bool)`

## Ejemplo hipotético
```cpp
VTRadioButton *rb = new VTRadioButton(this);
rb->Caption = "Opción A";
rb->ReadOnly = true; // No permitirá cambiar su estado
```

## Integración
Compatible con el patrón de formularios que alternan modos de edición, permitiendo detectar cuáles opciones fueron modificadas realmente.

---
