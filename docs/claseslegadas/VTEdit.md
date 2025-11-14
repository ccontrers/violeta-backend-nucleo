# VTEdit

## Resumen general
`VTEdit` extiende `TEdit` para soportar validación y formateo de contenido numérico, alineación configurable, control de precisión para valores flotantes, estados de modificación, bloqueo y activación selectiva de eventos `OnChange`. Implementa lógica detallada de teclado para controlar inserciones y formato.

## Objetivo de la clase
Ofrecer una caja de texto robusta para entrada de números y texto con:
- Alineación (izquierda, derecha, centro) según `EditAlineacion`.
- Restricción de tipo de contenido (texto, entero, flotante, etc.) vía `EditContenido`.
- Conversión rápida a/b de tipos (`ValorEntero`, `ValorFlotante`).
- Control de precisión decimal.
- Estados de bloqueo (`Bloqueado`) y cambio (`Modificado`).
- Activación diferida de eventos (`ActivarOnChange`).

## Atributos privados
- `Word ultima_tecla_control`: Última tecla de control procesada (para lógica de repetición/navegación).
- `int pos_anterior_cursor`: Posición del cursor antes de una edición (para restaurar tras validación).
- `int longitud_anterior_text`: Para comparar si hubo cambio real en longitud.
- `int digitos_entero`: Límite o registro de dígitos enteros permitidos (inferencia por nombre).
- `UnicodeString texto_respaldo`: Texto previo a una edición usada para revertir si se invalida la entrada.
- `bool mRespaldoEnabled`: Respaldo de habilitación real.
- `TColor color_original`: Color base del control.
- `EditAlineacion FAlign`: Alineación configurada del texto.
- `EditContenido FCont`: Tipo semántico del contenido (p.ej., Numérico, Texto, etc.).
- `int FPrecision`: Número de decimales permitidos para contenido flotante.
- `bool FModificado`: Indica si el usuario alteró el valor original.
- `bool FActivarOnChange`: Si `false`, el evento `Change` puede suprimirse temporalmente.
- `bool FBloqueado`: Estado lógico de bloqueo.

## Métodos protegidos
- `DoEnter()/DoExit()`: Gestionan color, respaldo y validación final.
- `DoSetMaxLength(int Value)`: Permite ajustar longitud máxima posiblemente considerando tipo de contenido.
- `KeyDown(Word &Key, TShiftState Shift)`: Maneja navegación, atajos y bloqueo de ciertas teclas.
- `KeyPress(wchar_t &Key)`: Filtra caracteres no válidos según `FCont` y controla formato numérico / precisión.
- `SetAlign(EditAlineacion)`: Aplica alineación (probablemente ajustando estilos de ventana en `CreateParams`).
- `SetCont(EditContenido)`: Cambia el modo de validación de entrada.
- `SetPrecision(int)`: Ajusta decimales máximos manejados.
- `GetEntero()/SetEntero(long)`: Conversión entre texto y valor entero.
- `GetFlotante()/SetFlotante(Extended)`: Conversión entre texto y valor flotante con formateo.
- `GetEnabled()/SetEnabled(bool)`: Sincronización con bloqueo.
- `GetBloqueado()/SetBloqueado(bool)`: Mecanismo de bloqueo lógico.

## Métodos públicos
- `VTEdit(TComponent* Owner)`: Constructor que establece valores iniciales.
- `CreateParams(TCreateParams &Params)`: Permite modificar estilos de ventana (para alineación o filtrado IME).
- `Change()`: Llamado al modificarse el texto; respeta `FActivarOnChange` y marca `FModificado`.

## Propiedades publicadas
- `ActivarOnChange (bool)`: Controla si se emite el evento `Change` automáticamente.
- `Alineacion (EditAlineacion)`: Alineación horizontal del contenido.
- `Contenido (EditContenido)`: Define reglas de validación (numérico, texto, etc.).
- `Precision (int)`: Decimales soportados para flotantes.
- `ValorEntero (long)`: Acceso numérico entero al contenido.
- `ValorFlotante (Extended)`: Acceso numérico flotante.
- `Modificado (bool)`: Bandera de cambio.
- `Bloqueado (bool)`: Control lógico para impedir edición temporal.

## Notas de implementación inferidas
El filtro en `KeyPress` más la conversión en los getters permite centralizar validación evitando parsing disperso en la capa de negocio. `CreateParams` probablemente modifica `ES_RIGHT/ES_CENTER` según la alineación.

## Ejemplo hipotético
```cpp
VTEdit *edPrecio = new VTEdit(this);
edPrecio->Contenido = ecFlotante; // Ejemplo: enumeración definida en ComponentesComunes.h
edPrecio->Precision = 2;
edPrecio->ValorFlotante = 123.45;
```

## Casos de uso
- Entrada cuantitativa (precios, cantidades) con validación y formateo.
- Campos que requieren saber si el usuario alteró el dato respecto al original.

## Integración
Estándar del framework para campos de edición: ofrece API homogénea para controladores que necesiten bloquear todo un formulario (`Bloqueado`) o recopilar únicamente valores modificados (`Modificado`).

---
