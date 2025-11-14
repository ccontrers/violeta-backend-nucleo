# VTDateTimePicker

## Resumen general
`VTDateTimePicker` extiende `TDateTimePicker` añadiendo control de estado (`Bloqueado`, `ReadOnly`), seguimiento de modificación (`Modificado`) y respaldo de valores para revertir cambios. Implementa manejo adicional de teclado, ratón y foco para gestionar cuándo disparar eventos de cambio.

## Objetivo de la clase
Ofrecer un selector de fecha/hora que pueda bloquearse lógicamente sin deshabilitarse físicamente, detectar si el usuario modificó el valor y administrar cambios diferidos (evitando notificaciones prematuras de `Change`).

## Atributos privados
- `TColor color_original`: Color base previo a cualquier efecto visual.
- `bool FModificado`: Bandera de que el valor cambió (comparado con respaldo inicial).
- `bool FBloqueado`: Bloqueo lógico de interacción.
- `bool mRespaldoEnabled`: Respaldo del estado `Enabled` original.
- `bool FReadOnly`: Modo de solo lectura.
- `TDateTime mRespaldoDate`: Respaldo de la fecha si en modo solo fecha.
- `TDateTime mRespaldoDateTime`: Respaldo completo fecha-hora.
- `TDateTime mRespaldoTime`: Respaldo de la hora si en modo hora.
- `bool mEjecutarChange`: Bandera para controlar si debe ejecutarse `Change` (evitar eventos múltiples mientras se escribe/navega).
- `bool mFocoEnControl`: Indica si actualmente el control tiene foco (usado para posponer procesamiento hasta `DoExit`).

## Métodos protegidos
- `DoEnter()/DoExit()`: Gestión de foco, respaldo de valores y actualización visual.
- `KeyDown(Word &Key, TShiftState Shift)`: Navegación y validación al presionar teclas (Enter, Esc, Tab...).
- `KeyPress(wchar_t &Key)`: Filtra caracteres según modo (fecha/hora) y puede activar la bandera de modificación.
- `GetEnabled()/SetEnabled(bool)`: Sincroniza habilitación real con estado de bloqueo.
- `GetBloqueado()/SetBloqueado(bool)`: Acceso/mutación del bloqueo lógico.
- `MouseDown(...)`: Intercepta clicks en el calendario para validar permisos y setear `Modificado`.

## Métodos públicos
- `VTDateTimePicker(TComponent* Owner)`: Constructor que inicializa respaldos y estados.
- `Change()`: Evento de cambio sobreescrito; se dispara condicionado por `mEjecutarChange` y actualiza `Modificado`.

## Propiedades publicadas
- `Bloqueado (bool)`: Control lógico de interacción.
- `Modificado (bool)`: Señala cambio de valor.
- `ReadOnly (bool)`: Impide modificar aunque esté visible.

## Notas de implementación inferidas
Los múltiples respaldos (`mRespaldoDate`, `mRespaldoTime`, etc.) permiten revertir según el modo del control (solo fecha, solo hora o ambos). `mEjecutarChange` evita múltiples notificaciones mientras el usuario navega por componentes internos del picker.

## Ejemplo hipotético
```cpp
VTDateTimePicker *dtp = new VTDateTimePicker(this);
dtp->Kind = dtkDate;
dtp->Date = Now();
dtp->Bloqueado = false;
dtp->ReadOnly = false;
```

## Integración
Sigue el patrón de estados uniforme, facilitando su uso en formularios que alternan "ver" / "editar" y requieren detectar cambios para validación y persistencia condicional.

---
