# VTLabeledEdit

## Resumen general
`VTLabeledEdit` amplía `TLabeledEdit` (un `TEdit` con etiqueta asociada) agregando capacidades avanzadas de validación, alineación, control numérico (entero/flotante), precisión decimal, seguimiento de modificaciones y bloqueo lógico, replicando el patrón funcional de `VTEdit` con la ventaja de una etiqueta integrada para contexto del usuario.

## Objetivo de la clase
Ofrecer un campo etiquetado listo para usar que unifique validación de contenido, control de alineación, soporte de conversiones numéricas y administración de estados de edición/bloqueo.

## Atributos privados
- `Word ultima_tecla_control`: Última tecla de control manejada (navegación, edición).
- `int pos_anterior_cursor`: Posición del cursor antes de un cambio validable.
- `int longitud_anterior_text`: Longitud previa; ayuda a determinar si hubo cambio efectivo.
- `int digitos_entero`: Cantidad de dígitos enteros permitidos (inferido por nombre).
- `UnicodeString texto_respaldo`: Texto guardado para revertir entradas inválidas.
- `bool mRespaldoEnabled`: Respaldo de habilitación ante bloqueo.
- `TColor color_original`: Color base usado para retroalimentación en foco.
- `EditAlineacion FAlign`: Tipo de alineación horizontal configurada.
- `EditContenido FCont`: Tipo de contenido permitido.
- `int FPrecision`: Decimales máximos para valores flotantes.
- `bool FModificado`: Bandera de cambio.
- `bool FActivarOnChange`: Controla emisión de eventos `Change`.
- `bool FBloqueado`: Estado lógico de bloqueo de edición.

## Métodos protegidos
- `DoEnter()/DoExit()`: Manejo de foco, respaldo y visual.
- `DoSetMaxLength(int)`: Limita longitud según reglas del contenido.
- `KeyDown(Word &Key, TShiftState)`: Navegación y atajos (tab, flechas, suprimir) evitando acciones inválidas.
- `KeyPress(wchar_t &Key)`: Filtra caracteres de acuerdo a `FCont` y formato numérico.
- `SetAlign(EditAlineacion)`: Cambia alineación (estilos de ventana).
- `SetCont(EditContenido)`: Ajusta modo de validación.
- `SetPrecision(int)`: Configura decimales para flotantes.
- `GetEntero()/GetFlotante()`: Conversión de texto a número controlando formato.
- `SetEntero(int)/SetFlotante(Extended)`: Asigna texto formateado desde números.
- `GetEnabled()/SetEnabled(bool)`: Sincroniza con bloqueo.
- `GetBloqueado()/SetBloqueado(bool)`: Control de bloqueo lógico.

## Métodos públicos
- `VTLabeledEdit(TComponent* Owner)`: Constructor que inicializa parámetros y estados.
- `CreateParams(TCreateParams &Params)`: Ajuste de estilos de ventana (alineación).
- `Change()`: Eventos de cambio condicionados por `FActivarOnChange` y que marcan `FModificado`.

## Propiedades publicadas
- `Bloqueado (bool)`: Impide edición.
- `ActivarOnChange (bool)`: Suprime o permite disparar `Change` automáticamente.
- `Alineacion (EditAlineacion)`: Alineación del texto.
- `Contenido (EditContenido)`: Tipo semántico del contenido.
- `Precision (int)`: Decimales permitidos.
- `ValorEntero (long)`: Acceso/edición como entero.
- `ValorFlotante (Extended)`: Acceso/edición como flotante.
- `Modificado (bool)`: Bandera de cambio.

## Notas de implementación inferidas
La similitud con `VTEdit` sugiere reutilización conceptual para garantizar consistencia. El agregado de la etiqueta permite reducir la necesidad de un `TLabel` separado y así simplificar maquetado.

## Ejemplo hipotético
```cpp
VTLabeledEdit *leMonto = new VTLabeledEdit(this);
leMonto->Label->Caption = "Monto:";
leMonto->Contenido = ecFlotante;
leMonto->Precision = 2;
leMonto->ValorFlotante = 2500.00;
```

## Casos de uso
- Formularios donde se requiere claridad inmediata (etiqueta) y validación numérica.
- Entradas donde se debe detectar cambios antes de guardar.

## Integración
Mantiene coherencia con otros componentes personalizados de la suite, facilitando el manejo global de estados (`Bloqueado`, extracción de sólo modificados, etc.).

---
