# VTCheckBox

## Resumen general
`VTCheckBox` amplía `TCheckBox` agregando control de modificación (`Modificado`), bloqueo lógico (`Bloqueado`), modo de solo lectura (`ReadOnly`) y respaldo de estado `Enabled`. Personaliza la respuesta a eventos de foco, teclado, clic y cambio de selección (`Checked`).

## Objetivo de la clase
Facilitar la gestión uniforme de estados de edición para casillas de verificación, permitiendo detectar cambios, impedir modificarlos temporalmente, y distinguir entre un estado de bloqueo y otro de solo lectura.

## Atributos privados
- `TColor color_original`: Conserva el color original para efectos visuales de foco.
- `bool FModificado`: Indica si el valor fue alterado desde su estado inicial (útil para validación o guardado condicional).
- `bool FBloqueado`: Estado lógico de bloqueo que anula interacción sin necesariamente cambiar `Enabled` externo.
- `bool mRespaldoEnabled`: Guarda el valor original de `Enabled` cuando se aplica bloqueo o manipulaciones temporales.
- `bool FReadOnly`: Modo de solo lectura; impide cambios, pero puede dejar el control visualmente activo.

## Métodos protegidos
- `DoEnter() / DoExit()`: Ajustan apariencia o estado auxiliar al ganar/perder foco.
- `KeyDown(Word &Key, TShiftState Shift)`: Procesa teclas (barra espaciadora, navegación) respetando bloqueo/readonly.
- `GetEnabled() / SetEnabled(bool)`: Sincronizan habilitación física con la lógica de bloqueo.
- `GetBloqueado() / SetBloqueado(bool)`: Acceso y aplicación del estado de bloqueo.
- `SetChecked(bool Value)`: Sobrescribe el cambio de estado marcado; aquí puede interceptarse para evitar cambios si `Bloqueado` o `ReadOnly`.
- `Toggle()`: Alterna el estado; probable punto de control de modificación y validación de permisos.

## Métodos públicos
- `VTCheckBox(TComponent* Owner)`: Constructor; inicializa campos y color base.
- `Click()`: Evento clic sobreescrito para validar bloqueo, marcar `Modificado` y llamar a la implementación base.

## Propiedades publicadas
- `Modificado (bool)`: Bandera manual o automática para saber si el usuario cambió el estado.
- `Bloqueado (bool)`: Control de bloqueo lógico.
- `ReadOnly (bool)`: Impide que el usuario altere el valor aun si no está bloqueado visualmente.

## Notas de uso
Utilice `Modificado` para decidir si persistir cambios. `Bloqueado` es adecuado durante procesos intermedios (carga de datos). `ReadOnly` sirve para formularios en modo consulta.

## Ejemplo hipotético
```cpp
VTCheckBox *cbActivo = new VTCheckBox(this);
cbActivo->Caption = "Activo";
cbActivo->Checked = true;
cbActivo->ReadOnly = true;   // Visible y marcado, pero no editable.
cbActivo->Bloqueado = false; // No bloqueado lógicamente (se usa sólo ReadOnly)
```

## Integración en el proyecto
Sigue el mismo patrón de extensiones que otros controles personalizados, simplificando la implementación de un modo de edición global: un controlador externo puede iterar controles y aplicar `Bloqueado` o `ReadOnly` consistentemente.

---
