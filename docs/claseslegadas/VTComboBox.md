# VTComboBox

## Resumen general
`VTComboBox` amplía `TComboBox` para ofrecer:
- Dibujo owner-draw condicionado a especificación de anchos de columna (`AnchoColumnas`).
- Propiedad lógica `Clave` para obtener/establecer un valor asociado (probablemente el código interno de un ítem).
- Control de modificación (`Modificado`), bloqueo (`Bloqueado`), solo lectura (`ReadOnly`).
- Funciones de apoyo a búsqueda, selección y manipulación (uso de `FuncionesGenericas`).
- Gestión de colores alternos y resaltado.

## Objetivo de la clase
Permitir presentar listas con múltiples columnas visuales en un combo (estilo lookup), manejando simultáneamente un código clave y un texto mostrado, con lógica consistente de edición/bloqueo.

## Atributos privados
- `TColor color_original`: Color base inicial para restaurar tras efectos de foco.
- `TColor color_alterno, color_resaltado`: Colores para alternar fondos y resaltar selección.
- `Classes::TStrings* ancho_columnas`: Colección de anchos usados para dividir el ítem en columnas (activa `csOwnerDrawFixed`).
- `bool FModificado`: Señala que el valor cambió.
- `FuncionesGenericas mFg`: Utilidades auxiliares (búsquedas, conversiones, etc.).
- `bool FBloqueado`: Control de bloqueo lógico.
- `bool mRespaldoEnabled`: Respaldo del estado `Enabled` original.
- `bool FReadOnly`: Modo solo lectura.
- `int mRespaldoIndice`: Índice seleccionado previo para revertir si se cancela interacción.
- `UnicodeString Clave (propiedad interna)`: Representa un identificador asociado al ítem seleccionado (implementada con getter/setter privados).

## Métodos privados relevantes
- `SetClave(UnicodeString Value)`: Ubica y selecciona el ítem cuya clave coincida (criterio deducido): podría buscar en la lista (tal vez almacenada en formato "CLAVE\tDESCRIPCION").
- `GetClave() -> UnicodeString`: Devuelve la clave del ítem activo.
- `GetAnchoColumnas()` / `SetAnchoColumnas(TStrings*)`: Gestiona anchos y activa modo owner-draw.

## Métodos protegidos
- `DoEnter()/DoExit()`: Manejan foco y alteraciones visuales.
- `DrawItem(...)`: Dibuja cada elemento según los anchos y colores para múltiples columnas.
- `KeyDown(...)`: Captura navegación, búsqueda incremental o confirmaciones.
- `GetEnabled()/SetEnabled(bool)`: Sincronizan habilitación respetando bloqueo.
- `GetBloqueado()/SetBloqueado(bool)`: Control de bloqueo lógico.
- `KeyPress(wchar_t &Key)`: Filtra caracteres y quizá implementa búsqueda incremental.
- `Select()`: Evento al cambiar selección (utilizado para marcar `Modificado`).
- `SetItemIndex(const int Value)`: Overridden para controlar efectos secundarios y respaldo.
- `DropDown()`: Overridden para preparar datos/estilos antes de desplegar la lista.

## Métodos públicos
- `VTComboBox(TComponent* Owner)`: Constructor: inicializa buffers, estilos y estado.
- `~VTComboBox()`: Libera `ancho_columnas`.
- `Change()`: Evento de cambio; se sobreescribe para actualizar `Modificado`.
- `Click()`: Maneja selección por clic y respeta `Bloqueado/ReadOnly`.

## Propiedades publicadas
- `AnchoColumnas (TStrings*)`: Anchos en pixeles para segmentar columnas.
- `Modificado (bool)`: Bandera de cambio.
- `Bloqueado (bool)`: Estado lógico de bloqueo.
- `ReadOnly (bool)`: Modo consulta sin edición.
- `Clave (UnicodeString)`: (Propiedad pública no publicada) Facilita binding de código interno vs. texto.

## Notas de implementación inferidas
Formato típico de ítems multi-columna: "CLAVE\tDESCRIPCION\tOTRO"; `DrawItem` particiona usando los anchos cumulativos. `Clave` probablemente extrae el primer segmento.

## Ejemplo hipotético
```cpp
VTComboBox *cbClientes = new VTComboBox(this);
cbClientes->AnchoColumnas->Add("60");  // Clave
cbClientes->AnchoColumnas->Add("200"); // Nombre
cbClientes->Items->Add("C001\tCliente Demo S.A.");
cbClientes->Clave = "C001"; // Selecciona la fila cuyo primer campo coincide
```

## Casos de uso
- Selección de entidades (clientes, productos) donde se muestra clave + descripción.
- Listas lookup con necesidad de alineación visual de columnas.

## Integración en el proyecto
Provee un patrón uniforme con otros controles para alternar entre modos de edición/consulta y reforzar consistencia visual y lógica en formularios.

---
