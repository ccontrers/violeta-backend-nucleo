# ClassBufferRespuestas

## Resumen General
`BufferRespuestas` encapsula el acceso secuencial y aleatorio a un bloque de memoria (buffer) que representa el resultado tabular de una consulta (SELECT). El formato interno incluye: tamaño total, indicador de error, definición de campos (nombre, tipo, longitud, decimales) y luego los registros codificados campo a campo como cadenas (o representaciones específicas). La clase abstrae la navegación por campos y registros, ofrece búsquedas por nombre o índice, extracción de valores (como `AnsiString` o puntero crudo), indexación para saltos O(1) a registros, y métodos utilitarios para volcar los datos a controles visuales VCL (grids, list boxes, checklist, combobox, listview, edits, mask edits, date/time pickers). También permite exportar a CSV y ajustar dinámicamente los decimales de presentación de campos numéricos.

Su objetivo principal es desacoplar la lógica de iteración y parsing del buffer binario / estructurado, simplificando el consumo de resultados en la capa de presentación o lógica superior.

## Estructura Interna del Buffer (Formato Esperado)
Orden en memoria del bloque apuntado por `buffer`:
1. (int) Tamaño total de la respuesta en bytes.
2. Cadena (con longitud embebida) -> indicador de error ("0" = sin error, distinto = error).
3. Cadena -> número de campos (N).
4. Para cada campo i en [0..N-1]:
   - Cadena: nombre.
   - Cadena (1 char efectivo) : tipo (por ejemplo 'C'=caracter, 'N'=numérico, 'D'=fecha, etc.).
   - Cadena -> longitud declarada.
   - Cadena -> decimales.
5. Cadena -> número de registros (R).
6. Luego, para cada registro r y cada campo c, una cadena codificada secuencialmente (el cursor avanza mediante `ExtraePCharDeBuffer` / `ExtraeStringDeBuffer`).

La clase asume que si el indicador de error es distinto de 0 lanza una excepción y no permite uso posterior.

## Atributos Principales
- `DefCampo *mCampos`: Arreglo de metadatos de cada campo (nombre, tipo, longitud, decimales).
- `FuncionesGenericas mFg`: Utilidades de extracción/formato desde el buffer crudo.
- `int mTamRespuesta`: Tamaño total en bytes.
- `int mNumCampos`, `mNumRegistros`: Cardinalidad de la tabla lógica.
- `int mNumCampoActivo`, `mNumRegistroActivo`: Posición actual del cursor interno.
- `int mHuboError`: Indicador de estado (0 = OK).
- Punteros de navegación: `mPunteroDefCampos`, `mPunteroDatos`, `mPunteroDatoActivo`, `mPunteroRegistroActivo`.
- `char *mBuffer`: Puntero base al bloque de datos.
- `bool mLiberar`: Indica si el destructor debe liberar la memoria del buffer.
- `char **mPunteroRegistros`: Índice opcional a inicio de cada registro para saltos rápidos.

## Tipos y Constantes
- `LIBERAR_BUFFER_AL_DESTRUIR` / `NO_LIBERAR_BUFFER_AL_DESTRUIR` (banderas para control de ciclo de vida).
- Tipos de campo previstos (implícitos en el código):
  - 'N' numérico (permite ajuste de decimales).
  - 'D' fecha (se normaliza quitando `01/01/1900`).
  - 'C' carácter / cadena (por defecto para else).

## Ciclo de Vida
1. Construcción: Parsea encabezado y metadatos. Si `mHuboError != 0` lanza excepción.
2. Inicializa cursor (`IrAlPrimerDato`).
3. Uso: iteración secuencial (`ObtieneDato`, `IrAlSiguienteDato`, `IrAlSiguienteRegistro`) o aleatoria (`ObtieneIndiceCampo`, `IrAlCampoNumero`, `IrAlRegistroNumero`).
4. (Opcional) `IndexarRegistros` construye `mPunteroRegistros` para saltos O(1) a cualquier registro (de lo contrario coste proporcional al desplazamiento relativo).
5. Destrucción: libera `mCampos`, (posiblemente) `mBuffer`, e índice de registros si fueron asignados.

## Métodos (Resumen Funcional)
### Navegación y Posicionamiento
- `IrAlPrimerDato()`: Resetea cursores a inicio de datos.
- `IrAlSiguienteDato()`: Avanza por un campo; al final de un registro avanza al siguiente.
- `IrAlSiguienteRegistro()`: Salta el resto del registro actual y posiciona en el primero del siguiente.
- `IrAlCampoNumero(indice)`: Posiciona dentro del registro actual en el campo indicado (retrocede al inicio del registro si el índice es menor que el campo activo).
- `IrAlRegistroNumero(indice)`: Salta a un registro específico (si se indexó usa acceso directo; si no, recorre secuencialmente desde posición actual o reinicia según convenga).
- `IndexarRegistros()`: Construye arreglo de punteros a cada registro para acelerar accesos aleatorios posteriores.

### Metadatos
- `ObtieneNumCampos()`, `ObtieneNumRegistros()`, `ObtieneTamRespuesta()`.
- `ObtieneCampo(indice, nombre, tipo, longitud, decimales)`: Copia la definición de un campo.
- `ObtieneIndiceCampo(nombre)`: Busca índice (case-insensitive); lanza excepción si no existe.
- `ExisteCampo(nombre)`: Versión booleana no-excepcional.
- `CambiaIndicadorDecimales(campo, decimales)`: Modifica presentación de decimales para campos numéricos ('N'), validando tipo.

### Extracción de Datos
- `ObtieneDato()`: Devuelve el valor del campo activo (sin avanzar el cursor base; usa copia de puntero temporal para extraer la cadena). Mantiene estado de posición.
- `ObtieneDato(nombreCampo)`: Salta al campo indicado y retorna su valor.
- `ObtieneDatoEnIndice(indiceCampo)`: Similar pero por índice.
- `ObtienePunteroDato(nombre)` / `ObtienePunteroDatoEnIndice(indice)`: Devuelven puntero crudo al dato (inicio de la representación codificada); permite extracción personalizada externa.

### Llenado de Controles VCL
(Abstracciones para enlazar resultado tabular con UI):
- `LlenaStringGrid(grid, titulo_vacio, incluir_numreg, max_columnas)`: Construye encabezados opcionales, controla inclusión de número de registro y límite de columnas. Normaliza fechas nulas (01/01/1900 ? vacío).
- `LlenaListBox(lista)`: Cada registro en una línea con campos separados por '|'.
- `LlenaCheckListBox(lista)` y overload (indices de campo de nombre y bandera): Usa primera columna booleana para checks.
- `LlenaComboBox(combo)`: Cada item = concatenación de campos con '|'; reemplaza cadenas vacías por espacio simple.
- `LlenaListView(listview, incluir_nomcamp)`: Opcionalmente agrega columnas con nombres y luego filas.
- `LlenaEdit(indiceCampo, indiceRegistro, edit)`, `LlenaMaskEdit(indCampo, indReg, mask, maskedit)`: Copian datos simples.
- `LlenaDateTimePicker(indCampo, indReg, kind, picker)`: Carga fecha o hora, usando conversión MySQL para fechas (vía `FuncionesGenericas::MySqlDateToTDate`).

### Búsqueda y Exportación
- `BuscaDato(indiceCampo, datoBuscado)`: Retorna índice de registro o -1 si no lo encuentra.
- `EnviaCsv(nombreArchivo)`: Exporta todos los registros a un archivo tipo texto con tabuladores (TSV); normaliza fechas "01/01/1900" a vacío; agrega salto de línea por registro.

### Destructor
- Libera memoria de metadatos y opcionalmente del buffer (según `mLiberar`). Libera índice de registros si fue creado.

## Consideraciones de Implementación
- Lanzamiento temprano de excepción si el buffer trae indicador de error ? 0: evita operar sobre datos inconsistentes.
- Uso de `ExtraePCharDeBuffer`/`ExtraeStringDeBuffer` (de `FuncionesGenericas`) hace que la clase dependa fuertemente del formato de serialización (longitud prefijada/terminación controlada). Cambios en el protocolo requieren actualización coordinada.
- `ObtieneDato()` no avanza el puntero activo principal, lo que podría generar lecturas repetidas si se asume comportamiento consumible; la iteración secuencial correcta necesita combinar `ObtieneDato` con `IrAlSiguienteDato`.
- `IndexarRegistros` almacena direcciones al primer campo del registro. Cambios posteriores en estructura no permiten recompactar sin reconstruir índice.
- No hay verificación explícita de `indice_campo` o `indice_registro` fuera de rango antes de loops (se infiere confianza en llamadas internas). Para robustez se podrían agregar asserts/excepciones.
- Fechas sentinela se limpian (hardcode "01/01/1900"). Recomendable parametrizar.
- Exportación a CSV realmente produce TSV (tabulaciones). Podría renombrarse o permitir separador configurable.

## Ejemplo de Uso Básico
```cpp
char *raw = EjecutaConsultaComoBuffer("SELECT id, nombre, fecha FROM clientes");
BufferRespuestas buf(raw, LIBERAR_BUFFER_AL_DESTRUIR);
int campos = buf.ObtieneNumCampos();
int registros = buf.ObtieneNumRegistros();
for (int r=0; r<registros; ++r) {
    buf.IrAlRegistroNumero(r);
    AnsiString id = buf.ObtieneDato("id");
    AnsiString nombre = buf.ObtieneDato("nombre");
    AnsiString fecha = buf.ObtieneDato("fecha");
    // ... procesar
}
```

## Escenario con Indexación
```cpp
BufferRespuestas buf(raw, NO_LIBERAR_BUFFER_AL_DESTRUIR);
buf.IndexarRegistros(); // Accesos O(1)
int pos = buf.BuscaDato(buf.ObtieneIndiceCampo("codigo"), "A123");
if (pos>=0) {
    buf.IrAlRegistroNumero(pos);
    auto precio = buf.ObtieneDato("precio");
}
```

## Posibles Mejoras
- Añadir verificación de límites y lanzar excepciones claras en accesos fuera de rango.
- Separar responsabilidad de serialización/deserialización en una capa independiente (Strategy o Parser externo) para permitir múltiples formatos.
- Implementar iterador STL-like para range-based for.
- Permitir extracción tipada (int/double/fecha) directamente, evitando conversiones repetidas `AnsiString` ? numérico.
- Configurar separador y codificación (UTF-8) en exportación TSV/CSV; escapado de caracteres especiales.
- Caching opcional de conversiones numéricas y normalización de fechas para mejorar rendimiento en recorridos múltiples.
- Parametrizar valor sentinela de fecha nula en lugar de hardcode.
- Soporte para lazy indexado (solo si se detectan múltiples saltos hacia atrás).

## Riesgos / Edge Cases
- Buffer malformado (metadatos inconsistentes) puede causar lecturas fuera de límites; actualmente no se valida integridad global.
- Uso concurrente no es seguro (estado mutable de cursores). Para multihilo, duplicar instancia o proteger con mutex.
- `ObtienePunteroDato*` expone puntero interno: si se modifica externamente se corrompe el estado. Debe tratarse como solo lectura.
- Si se cambia `mCampos[indice].decimales` no se re-formatean datos ya volcados a UI (solo afecta futura presentación condicional si se aplicara formateo numérico adicional).

## Glosario
- Campo activo: Campo donde apunta el cursor dentro del registro actual.
- Registro activo: Registro actual sobre el que se realizan operaciones de campo.
- Indexar: Pre-calcular punteros de inicio de cada registro para accesos O(1).
- TSV: Tab Separated Values (archivo de texto con tabuladores).

## Archivo Fuente
Derivado del análisis de `ClassBufferRespuestas.h` y `ClassBufferRespuestas.cpp` (líneas 1?568).

---
© Documentación técnica generada automáticamente.
