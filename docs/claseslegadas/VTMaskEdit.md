# VTMaskEdit

## Resumen general
`VTMaskEdit` extiende `TMaskEdit` agregando validación automática opcional, mensaje de error personalizable, seguimiento de modificaciones y control de bloqueo lógico. Permite integrar campos con formato (máscaras) dentro del modelo uniforme de estados del proyecto.

## Objetivo de la clase
Facilitar la validación de entradas con máscara (fechas, códigos, estructuras específicas) agregando un flujo consistente para bloquear, marcar cambios y emitir un mensaje de error común cuando la validación falla.

## Atributos privados
- `TColor color_original`: Color base para restaurar tras efectos de foco.
- `bool auto_validar`: Indica si la validación se ejecuta automáticamente (probablemente al perder foco o al cambiar el contenido).
- `UnicodeString mensaje_error`: Mensaje a mostrar o registrar cuando la validación falla.
- `bool FModificado`: Bandera de cambio en el contenido.
- `bool FBloqueado`: Estado lógico de bloqueo.
- `bool mRespaldoEnabled`: Respaldo del estado original de `Enabled`.

## Métodos protegidos
- `DoEnter()/DoExit()`: Manejan efectos visuales y disparan validación si corresponde.
- `KeyDown(Word &Key, TShiftState Shift)`: Captura navegación y posibles teclas de borrado o confirmación, respetando bloqueo.
- `GetEnabled()/SetEnabled(bool)`: Ajuste de habilitación con bloqueo.
- `GetBloqueado()/SetBloqueado(bool)`: Estado de bloqueo lógico.

## Métodos públicos
- `VTMaskEdit(TComponent* Owner)`: Constructor inicializa valores y color.
- `ValidateEdit()`: Sobreescritura del proceso de validación que evalúa la máscara y reglas adicionales.
- `Validar() -> bool`: Método explícito que ejecuta/verifica la validación y devuelve resultado (puede usar `mensaje_error`).
- `Change()`: Marca `FModificado` y puede disparar validación si `auto_validar`.

## Propiedades publicadas
- `Bloqueado (bool)`: Impide edición lógica.
- `AutoValidar (bool)`: Habilita ejecución automática de validación.
- `MensajeError (UnicodeString)`: Texto mostrado o registrado al fallar validación.
- `Modificado (bool)`: Bandera de cambio del contenido.

## Ejemplo hipotético
```cpp
VTMaskEdit *meFecha = new VTMaskEdit(this);
meFecha->EditMask = "!99/99/0000;1;_"; // dd/mm/aaaa
meFecha->AutoValidar = true;
meFecha->MensajeError = "Fecha inválida";
```

## Notas de implementación inferidas
`ValidateEdit()` probablemente verifica cumplimiento estricto de la máscara y reglas adicionales de negocio (fecha válida, etc.). `Validar()` facilita invocaciones manuales antes de procesar formularios.

## Integración
Complementa componentes numéricos y de texto permitiendo uso de máscaras sin perder consistencia en el control de cambios y el ciclo de validación global.

---
