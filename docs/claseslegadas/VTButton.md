# VTButton

## Resumen general
`VTButton` es una extensión de `TButton` que introduce un modelo uniforme de control de estado: bloqueo lógico (`Bloqueado`), modo de solo lectura (`ReadOnly`) y gestión de foco/teclado con posible realce visual mediante `color_original`. Busca homogeneizar el comportamiento de botones dentro del marco de componentes personalizados del proyecto.

## Objetivo de la clase
Añadir al botón estándar la capacidad de diferenciar entre estar deshabilitado físicamente (`Enabled=false`) y estar bloqueado lógicamente (evitando acciones pero manteniendo coherencia visual y posible reactivación ágil). Proveer además un estado de solo lectura diferenciable.

## Atributos privados
- `TColor color_original`: Conserva el color original para restaurar tras efectos de foco.
- `bool FBloqueado`: Estado lógico de bloqueo (independiente de `Enabled`).
- `bool mRespaldoEnabled`: Respalda el valor previo de `Enabled` al aplicar bloqueo o cambios temporales.
- `bool FReadOnly`: Indica modo solo lectura (el botón no debe ejecutar acción aun si parece activo).

## Métodos protegidos
- `DoEnter()`: Personaliza entrada de foco (p.ej. cambiar color o registrar estado inicial).
- `DoExit()`: Restablece apariencia y estados al perder el foco.
- `KeyDown(Word &Key, TShiftState Shift)`: Captura teclas (Enter, Space, Escape) y puede impedir acción si está bloqueado/readonly.
- `GetEnabled() -> bool`: Asegura que el valor expuesto de `Enabled` refleje la lógica de bloqueo.
- `SetEnabled(bool Value)`: Controla habilitación física respetando `FBloqueado`.
- `GetBloqueado() -> bool`: Devuelve el estado lógico.
- `SetBloqueado(bool Value)`: Ajusta el bloqueo y actualiza `Enabled` según corresponda.

## Métodos públicos
- `VTButton(TComponent* Owner)`: Constructor que inicializa atributos y guarda estado visual inicial.
- `Click()`: Sobrescribe el evento estándar para validar `Bloqueado` o `ReadOnly` antes de ejecutar la lógica base.

## Propiedades publicadas
- `Bloqueado (bool)`: Control lógico para permitir/impedir la acción.
- `ReadOnly (bool)`: Marca el botón como no interactivo a nivel lógico sin necesariamente alterar aspecto.

## Notas de uso
Use `Bloqueado` durante operaciones críticas (procesos en curso) para prevenir clics repetidos. Use `ReadOnly` en vistas donde el botón está presente por consistencia de layout pero no debe activar lógica.

## Ejemplo hipotético
```cpp
VTButton *btnGuardar = new VTButton(this);
btnGuardar->Caption = "Guardar";
btnGuardar->Bloqueado = true;   // Se evita el clic mientras se valida el formulario
btnGuardar->ReadOnly = false;   // Podrá reactivarse luego
```

## Rol dentro del proyecto
Contribuye a un conjunto coherente de controles que comparten mecanismos de control de edición y bloqueo, simplificando la lógica de formularios que alternan entre modos de consulta y edición.

---
