# ClassDatosTabla (DatosTabla)

## Resumen General
`DatosTabla` abstrae la construcción y manipulación de conjuntos campo?valor asociados a una única tabla lógica (descrita por `TablasBd`). Permite poblar dichos pares desde buffers serializados o desde un `BufferRespuestas`, consultar tipos, reemplazar valores (incluyendo asignación explícita a NULL), y generar dinámicamente sentencias SQL `INSERT`, `REPLACE`, `UPDATE` así como un bloque de datos formateados para transferencia o logging. Internamente mantiene listas paralelas de nombres y valores junto con arreglos de tipos y banderas que indican si un valor se interpretará como variable MySQL (sin comillas) o literal.

## Objetivos Clave
- Facilitar generación segura y consistente de instrucciones SQL sin reescribir lógica de quoting y conversión por tipo.
- Permitir carga rápida de valores desde distintos formatos (buffer binario propio o resultado tabular).
- Asegurar que solo se trabajen campos pertenecientes al esquema oficial (`TablasBd`).
- Proveer utilidades de serialización / deserialización para integrarse con otras capas de transporte.

## Atributos Principales
- `mCampos` (`TStringList*`): Nombres de campos agregados.
- `mValores` (`TStringList*`): Valores crudos asociados posición a posición con `mCampos`.
- `mTipos[MAX_NUM_CAMPOS_VIOLETA]`: Código de tipo por campo (proveniente de `TablasBd`):
  - 'I' entero / numérico integral
  - 'N' numérico decimal (precisión)
  - 'F' flotante
  - 'D' fecha (convertida a formato MySQL mediante `StrToMySqlDate`)
  - 'T' hora (convertida con `TimeToMySqlTime`)
  - (Otros posibles códigos según definición en `TablasBd`)
- `mEsVariable[]`: Marca si el valor se debe insertar sin comillas (p.ej. variables @userVar, funciones SQL, expresiones) en lugar de un literal.
- `mNombreTabla`: Nombre ASCII de la tabla objetivo.
- `mIndiceTabla`: Índice interno de la tabla dentro de `TablasBd` (?1 si no se asignó).
- `TablasBd *Tablas`: Referencia al descriptor estructural (metadatos) de todas las tablas.
- `FuncionesGenericas mFg`: Utilidades de parsing / formateo (manejo de buffer y conversión de fechas/tiempos).

## Flujo de Uso Típico
1. Construcción: `DatosTabla dt(&tablas);`
2. Asignación de tabla: `dt.AsignaTabla("clientes");` (valida existencia en metadatos).
3. Inserción de campos: `dt.InsCampo("nombre", "Juan"); dt.InsCampo("limite_credito", "1500.50");`
4. Modificación: `dt.ReemplazaCampo("limite_credito", "2000.00");`
5. Generación SQL: `AnsiString sql = dt.GenerarSqlInsert();`
6. Ejecución en capa inferior (por ejemplo `FuncionesMySql`).

## Métodos y Responsabilidades
### Configuración
- `DatosTabla(TablasBd *DirTablas)`: Inicializa listas y referencia al catálogo.
- `void AsignaTabla(char *Nombre)`: Valida y registra tabla; limpia campos previos.

### Inserción y Reemplazo
- `InsCampo(NomCampo, Valor, EsVariable=0)`: Agrega nuevo campo; valida existencia y tipo en `TablasBd`.
- `ReemplazaCampo(NomCampo, Valor, EsVariable=0)`: Si ya existe actualiza valor/bandera; si no, inserta.
- `ReemplazaCampoValorNull(NomCampo)`: Azúcar para asignar valor lógico NULL (`(NULL)` internamente -> convertido a `NULL` en SQL final).

### Carga Masiva
- `InsCamposDesdeBufferRespuestas(BufferRespuestas *Buffer)`: Toma campos del registro activo (usa nombres de metadatos del buffer y valores). Inserta cada par.
- `int InsCamposDesdeBuffer(char *Buffer)`: Deserializa estructura: [num_campos][campo1][valor1]...[campoN][valorN]; retorna bytes consumidos para permitir encadenamiento en buffer mayor.

### Consulta / Mutación
- `char ObtieneTipoCampo(NomCampo)`: Obtiene tipo desde metadatos (sin necesidad de haber insertado antes el campo).
- `void AsignaValorCampo(NomCampo, Valor, EsVariable=0)`: Cambia valor de un campo ya agregado (silencioso si no existe).
- `AnsiString ObtieneValorCampo(NomCampo, GenerarExepcion=true)`: Recupera valor; opcionalmente lanza excepción si no existe.
- `int ObtieneIndiceCampo(NomCampo)`: Índice interno si se ha insertado; -1 si no (tras validación de existencia en tabla).
- `int ObtieneNumCamposInsertados()`: Conteo actual.

### Serialización / Exportación
- `int LlenaBufferConCampos(char *Buffer)`: Serializa pares campo/valor con prefijo de cantidad.
- `AnsiString GenerarDatosXSeparador()`: Devuelve secuencia de solo valores (sin nombres) separados por `'` (apóstrofo) como delimitador; cada valor es transformado según tipo al formato bruto (sin comillas para numéricos / variable, conversiones fecha/hora). Útil para protocolos internos.

### Generación de SQL
- `AnsiString GenerarSqlInsert(bool IncluirParte1=true, bool IncluirParte2=true)`: Permite construir por partes (lista de campos y/o valores) para composiciones multi?fila externas.
- `AnsiString GenerarSqlReplace()`: Igual a insert pero usando `REPLACE INTO`.
- `AnsiString GenerarSqlUpdate(AnsiString Condicion)`: Lista de asignaciones `campo=valor` con quoting/transformación adecuada más cláusula WHERE provista.

#### Reglas de Transformación de Valores
1. Si valor lógico interno es "(NULL)" ? `NULL` (sin comillas).
2. Si `mEsVariable[i]==1` ? se usa literalmente (funciones, expresiones, variables).
3. Según tipo:
   - Numéricos ('I','N','F'): se usa literal sin comillas; vacío o "." ? `0`.
   - Fecha ('D'): se transforma con `StrToMySqlDate` y se encierra en comillas simples.
   - Hora ('T'): se transforma con `TimeToMySqlTime(... )` y va entre comillas.
   - Otros: se encierra directamente entre comillas simples.

## Manejo de Errores
- La mayoría de operaciones que dependen de la tabla lanzan excepción si no se ha llamado `AsignaTabla`.
- Validación estricta de campo: inserción o reemplazo falla inmediatamente si el campo no existe en la estructura `TablasBd`.
- Consultas de valor pueden ser silenciosas (retornar cadena vacía) si se desactiva `GenerarExepcion`.

## Consideraciones de Integridad
- No se valida duplicidad de campos al insertar (depende del flujo usar `ReemplazaCampo` si se prevé repetición). Una inserción directa repetida resultaría en múltiples instancias del mismo nombre sin mecanismo de deduplicación (aunque las validaciones típicas del flujo parecen usar reemplazo para actualizaciones).
- No hay sanitización ni escape adicional de comillas internas en textos: se asume que los valores provistos ya están limpios o provienen de fuentes controladas. Mejorable para prevenir SQL injection si se da entrada externa.
- Arreglos `mTipos` y `mEsVariable` dependen de índice secuencial de inserción; no se recompactan ni reordenan.

## Ejemplo: Insert Dinámico
```cpp
DatosTabla dt(&tablas);
dt.AsignaTabla("clientes");
dt.InsCampo("cliente_id", "123");
dt.InsCampo("nombre", "ACME");
dt.InsCampo("limite_credito", "1500.75");
AnsiString sql = dt.GenerarSqlInsert();
// INSERT INTO clientes (cliente_id, nombre, limite_credito) VALUES (123, 'ACME', 1500.75)
```

## Ejemplo: Update Parcial
```cpp
DatosTabla dt(&tablas);
dt.AsignaTabla("clientes");
dt.InsCampo("cliente_id", "123");
dt.InsCampo("limite_credito", "1500.75");
dt.AsignaValorCampo("limite_credito", "2000.00");
AnsiString upd = dt.GenerarSqlUpdate("cliente_id=123");
// UPDATE clientes SET cliente_id=123, limite_credito=2000.00 WHERE cliente_id=123
```

## Mejores Prácticas / Mejoras Propuestas
- Implementar escape de comillas simples dentro de cadenas de texto (p.ej. reemplazar `'` por `\'`).
- Añadir método para generación de INSERT multi?fila reutilizando parte 1 (lista de campos) y acumulando parte 2 (valores).
- Ofrecer método `Clear()` público para reutilizar instancia sin re?asignar tabla.
- Soporte de prepared statements/parametrización para evitar riesgos de inyección y mejorar rendimiento.
- Validar longitud de valores frente a metadatos (longitud máxima) antes de construir SQL.
- Integrar logging opcional del SQL generado para auditoría.
- Añadir extracción tipada (`int GetInt("campo")`, etc.).

## Interacción con Otras Clases
- Depende de `TablasBd` para validar metadatos (índices y tipos).
- Puede consumir datos de `BufferRespuestas` para inicialización masiva.
- Usa `FuncionesGenericas` para serialización y conversión de formatos fecha/hora.

## Riesgos y Edge Cases
- Insertar el mismo campo varias veces con `InsCampo` generará duplicados no controlados (lógica externa debe evitarlo).
- Valor numérico vacío interpretado como 0 puede ocultar errores de captura; posible opción futura: modo estricto.
- `GenerarDatosXSeparador` usa `'` como separador; conflicto potencial si un valor sin comillas contiene `'` (no escapado). 

## Archivo Fuente
Basado en `ClassDatosTabla.h` y `ClassDatosTabla.cpp` (líneas 1?406).

---
© Documentación técnica generada automáticamente.
