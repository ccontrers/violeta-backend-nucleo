# VTBitBtn

## Resumen general
`VTBitBtn` extiende `TBitBtn` para añadir control de estado de bloqueo (`Bloqueado`), soporte de solo lectura (`ReadOnly`) y preservación temporal del estado `Enabled` mientras el componente recibe o pierde el foco. Además implementa lógica de retroalimentación visual (inferida por el uso de `color_original` en eventos de foco) y personaliza eventos de teclado y clic.

## Objetivo de la clase
Proporcionar un botón con imagen que pueda bloquearse lógicamente (sin depender únicamente de `Enabled`), distinguir entre un estado de solo lectura y uno interactivo, y gestionar cambios visuales al recibir / perder el foco.

## Atributos privados
- `TColor color_original`: Almacena el color original del botón para restaurarlo tras efectos de enfoque o deshabilitado.
- `bool FBloqueado`: Indica si el botón está en estado de "bloqueado" (deshabilitación lógica personalizada distinta de `Enabled`).
- `bool mRespaldoEnabled`: Guarda el valor original de `Enabled` cuando se altera temporalmente (por ejemplo, al aplicar `Bloqueado` o al gestionar foco).
- `bool FReadOnly`: Indica si el botón debe considerarse de solo lectura (se muestra activo pero podría ignorar acciones de clic en la implementación `.cpp`).

## Métodos protegidos
- `DoEnter()`: Evento al recibir el foco. Usado típicamente para cambiar color o registrar estado.
- `DoExit()`: Evento al perder el foco. Restaura color/estado previo.
- `KeyDown(Word &Key, TShiftState Shift)`: Manejo de teclas (posible captura de Enter/Escape o bloqueo de interacción si `Bloqueado`).
- `GetEnabled() -> bool`: Getter virtual que probablemente sincroniza el estado `Enabled` real con la lógica de `Bloqueado`.
- `SetEnabled(bool Value)`: Setter virtual que respeta bloqueo/readonly.
- `GetBloqueado() -> bool`: Devuelve el estado lógico de bloqueo.
- `SetBloqueado(bool Value)`: Aplica o retira el estado de bloqueo, quizá ajustando `Enabled` y guardando en `mRespaldoEnabled`.

## Métodos públicos
- `VTBitBtn(TComponent* Owner)`: Constructor: inicializa estados y guarda color inicial.
- `Click()`: Evento de clic sobreescrito. Probablemente verifica `Bloqueado` o `ReadOnly` antes de llamar a la versión base.

## Propiedades publicadas
- `Bloqueado (bool)`: Expone el control de bloqueo lógico vía `Get/SetBloqueado`.
- `ReadOnly (bool)`: Indica si el botón debe considerarse de solo lectura (no estándar en `TBitBtn`).

## Notas de uso
Utilice `Bloqueado` cuando se quiere impedir interacción temporal sin modificar semánticamente `Enabled` global (por ejemplo para validaciones intermedias). Use `ReadOnly` cuando el botón deba permanecer visible y con estilo activo pero sin ejecutar acción.

## Ejemplo hipotético de uso
```cpp
VTBitBtn *btn = new VTBitBtn(this);
btn->Caption = "Procesar";
btn->Bloqueado = true; // Visualmente puede quedar deshabilitado o con estilo distinto
btn->ReadOnly = false; // Permitirá futura interacción al quitar bloqueo
```

## Interacción prevista con el resto del proyecto
El patrón de propiedades `Bloqueado` y `ReadOnly` aparece en múltiples componentes, sugiriendo un comportamiento uniforme para formularios que controlan edición vs. visualización.

---
