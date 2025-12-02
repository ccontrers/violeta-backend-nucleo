# Especificación Técnica Legada: Bitácora Unificada de Usuarios

Documento técnico que describe el comportamiento actual del módulo de bitácora unificada de usuarios dentro del cliente C++ Builder y el servidor propietario que operan sobre MySQL. Se detalla el estado existente sin proponer cambios ni mejoras.

---

## 1. Introducción técnica general
- **Propósito**: centralizar en una sola vista las operaciones que los usuarios realizan sobre ventas, compras, notas, movimientos de almacén, pagos y pedidos, permitiendo auditar altas, modificaciones y cancelaciones con su referencia temporal.
- **Ubicación de código**: formulario `TFormBitacoraUnif` (`cpp/FormBitacoraUnificada.cpp/.h/.dfm`) en el cliente legado y procedimiento `ServidorAdminSistema::ConsultaBitacoraUnificada` (`cpp/ClassServidorAdminSistema.cpp`) en el servidor.
- **Arquitectura funcional**: la interfaz desktop captura filtros, el cliente `gClienteVioleta` prepara la petición `ID_CON_BITACORAUNIFICADA`, el servidor compone una consulta SQL unificada mediante múltiples `UNION` y MySQL devuelve el conjunto resultante empaquetado en `BufferRespuestas`.
- **Flujo de ejecución**: el usuario elige sucursal, usuario y rango de fechas; se construye la lista de empleados asociados al nombre seleccionado; se envía la petición con el rango y la lista; el servidor ejecuta la consulta y retorna el dataset que se vuelca sobre el grid.
- **Diagrama textual**:
  ```
  Usuario → [TFormBitacoraUnif] → gClienteVioleta (ID_CON_BITACORAUNIFICADA) → ServidorAdminSistema::ConsultaBitacoraUnificada → MySQL
                                       ↑                                                                        ↓
                             BufferRespuestas ←-------------- Resultado serializado (referencia, operación, fechas) ←-
  ```

---

## 2. Estructura de la interfaz (`TFormBitacoraUnif`)
Formulario modal con controles de filtrado, un grid de resultados y utilidades de exportación. Todos los componentes están definidos en `FormBitacoraUnificada.dfm`.

| Tipo VCL | Nombre | Ubicación (dfm) | Función | Observaciones |
|----------|--------|-----------------|---------|---------------|
| `VTComboBox` | `ComboBoxSucursal` | coordenadas `(8,26)` | Selecciona sucursal; muestra clave y nombre. | Propiedad `AnchoColumnas = {40,260}`; eventos `OnChange` y `OnExit` invocan `actualizaUsuarios`. |
| `VTComboBox` | `ComboBoxUsuario` | `(8,73)` | Lista usuarios activos de la sucursal (o todos si no se eligió sucursal). | Propiedad `AnchoColumnas = {80,120}`; relleno manual vía SQL. |
| `VTDateTimePicker` | `FechaInicial` | `(643,19)` | Límite inferior del rango de fechas (inclusive). | Valor inicial = fecha actual - 3 meses (`IncMonth`). |
| `VTDateTimePicker` | `FechaFinal` | `(797,19)` | Límite superior del rango de fechas (inclusive). | Valor inicial = fecha actual (`gClienteVioleta->ObtieneFechaDeHoy`). |
| `VTBitBtn` | `ButtonMuestraReporte` | `(674,57)` | Dispara la consulta y llena el grid. | Texto “Mostrar Reporte”; ícono embebido en `Glyph`. |
| `VTStringGrid` | `StringGridBitacora` | `(8,104)` | Visualiza resultados con ocho columnas. | `ColCount=8`, `FixedCols=0`, `DefaultDrawing=false`, cabeceras configuradas en el constructor. |
| `TPopupMenu` | `PopupMenuExportar` | lig. al grid | Opciones de salida “Imprimir” y “Exportar a Excel”. | Items llaman a `Imprimir1Click` y `ExportaraExcel1Click`. |
| `TImageList` | `ImageListIconos` | recurso asociado | Íconos para celdas de estado activo/cancelado en la columna 3. | Índice 4 = activo, índice 1 = cancelado. |
| `TLabel` | `Label1`, `LabelUsuario`, `LabelFechaInicialFech`, `LabelFechaFinalFech` | Encabezados | Guían la captura de filtros. | Texto de `LabelUsuario` enfatiza que agrupa usuarios con el mismo nombre. |

**Variables y dependencias internas**: `mFg` (`FuncionesGenericas`) para mensajes, `mPrivilegios` (`PrivilegiosDeObjeto`) para validar acceso al objeto “BTU”, y el recurso global `gClienteVioleta` para ejecutar SQL y peticiones de servidor.

**Flujo de eventos relevante**:
- `Abrir()`: consulta privilegios del objeto “BTU”; si `mPermitidoConsultar` es `false` se muestra mensaje y no abre el modal, de lo contrario ejecuta `ShowModal()`. Si el usuario tiene privilegio administrador, concatena la clave del objeto en la barra de título.
- `FormShow`: ejecuta `llenaSucursales` para poblar `ComboBoxSucursal`, fija el rango de fechas por defecto y llama `BlanqueaGridBitacora` para reiniciar el grid.
- `llenaSucursales`: arma `SELECT s.sucursal, s.nombre FROM sucursales WHERE idempresa = <empresa> ORDER BY s.nombre` y delega en `gClienteVioleta->Interfaz->LlenaComboBox`.
- `actualizaUsuarios`: dependiendo de la sucursal seleccionada arma `SELECT u.empleado AS Usuario, CONCAT(e.nombre,' ',e.appat,' ',apmat) AS Nombre FROM usuarios u, empleados e WHERE e.empleado=u.empleado [AND e.sucursal='<sucursal>'] ORDER BY nombre` y llena `ComboBoxUsuario`.
- `ButtonMuestraReporteClick`: limpia el grid, valida que se haya seleccionado usuario, construye la cadena `usuarios` con todas las claves de empleados que comparten el mismo nombre (consulta a `empleados e INNER JOIN empleados e2`), inicializa la petición `ID_CON_BITACORAUNIFICADA` con tres parámetros (`fechaini`, `fechafin`, lista de empleados) y ejecuta la petición. Al recibir el buffer, recorre `BufferRespuestas` fila por fila y asigna las celdas `Cells[0..7][row]` incrementando `RowCount`. Si no hay registros muestra “No hay registros que coincidan...”.
- `StringGridBitacoraDrawCell`: restaura estilos por defecto, pone encabezados en negritas, omite texto para columna 3 (`cancelado`) y dibuja el ícono correspondiente según el valor (`"0"` = icono activo, distinto = icono cancelado). Usa `ImageListIconos->Draw`.
- `Imprimir1Click`: instancia `ExportadorDatos`, configura fuente Arial Narrow 9, márgenes fijos y ancho de columnas, fija orientación vertical y envía a impresión con título “Bitácora unificada de las actividades de los usuarios”.
- `ExportaraExcel1Click`: reutiliza `ExportadorDatos` para enviar la rejilla a Excel.
- `BlanqueaGridBitacora`: borra todas las filas (`BorrarFilas`) y limpia las celdas de datos dejando la fila de encabezados intacta.

El color de fondo del formulario se sincroniza con el parámetro `COLORVENTANAS` almacenado en `gClienteVioleta->Param`.

---

## 3. Lógica de negocios en `ServidorAdminSistema::ConsultaBitacoraUnificada`
### 3.1 Preparación de parámetros y buffer
El servidor recibe la petición `ID_CON_BITACORAUNIFICADA` con los parámetros serializados como cadenas nulas. `FuncionesGenericas::ExtraeStringDeBuffer` se utiliza para extraer en orden: fecha inicial, fecha final y la lista de usuarios delimitada por comas y apóstrofes. Las fechas se normalizan a formato MySQL con `StrToMySqlDate`. El buffer de respuesta se inicializa mediante `mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ*2)` para duplicar la capacidad estándar de consultas de búsqueda.

### 3.2 Composición de la consulta SQL
Se construye un único `AnsiString` `instruccion` con una serie de `SELECT` entre paréntesis unidos mediante `UNION`. Cada bloque proyecta las mismas ocho columnas:

1. `referencia`
2. `tipodocumento` (literal asociado al tipo de entidad)
3. `operacion` (literal que describe alta, modificación o cancelación, en algunos casos diferenciado por subtipo)
4. `cancelado`
5. `fechadoc` (fecha propia del documento comercial)
6. `usuariooper` (usuario responsable)
7. `fechaoper` (fecha en la que se realizó la operación)
8. `horaoper` (hora complementaria a `fechaoper`)

Las columnas provienen directamente de las tablas involucradas. Los filtros comunes en todos los bloques son:
- El campo de fecha correspondiente debe estar dentro de `BETWEEN '<fechaini>' AND '<fechafin>'`.
- El usuario asociado se restringe a `IN (<usuarios>)`, donde `<usuarios>` es la cadena formada en el cliente (por ejemplo `'USR001','USR045'`).

A continuación se describen los bloques agrupados por entidad:

#### Ventas (`ventas`)
| `tipodocumento` | `operacion` | Fecha evaluada | Condiciones adicionales |
|-----------------|-------------|----------------|-------------------------|
| `VENT` | `ALTA` | `fechaalta` | `cancelado=0` y el usuario corresponde a `usualta`; `fechadoc = fechavta`, `horaoper = horaalta`. |
| `VENT` | `MODI` | `fechamodi` | Requiere que `fechaalta<>fechamodi` o `horaalta<>horamodi`, `cancelado=0`, usuario = `usumodi`. |
| `VENT` | `CANC` | `fechamodi` | `cancelado=1`, usuario = `usumodi`. |

#### Compras (`compras`)
| `tipodocumento` | `operacion` | Fecha evaluada | Condiciones adicionales |
|-----------------|-------------|----------------|-------------------------|
| `COMP` | `ALTA` | `fechaalta` | `cancelado=0`, usuario = `usualta`, `fechadoc = fechacom`. |
| `COMP` | `MODI` | `fechamodi` | Diferencia entre alta y modi (`fechaalta<>fechamodi` o `horaalta<>horamodi`), `cancelado=0`, usuario = `usumodi`. |
| `COMP` | `CANC` | `fechamodi` | `cancelado=1`, usuario = `usumodi`. |

#### Notas de crédito a clientes (`notascredcli`)
| `tipodocumento` | `operacion` | Fecha evaluada | Condiciones adicionales |
|-----------------|-------------|----------------|-------------------------|
| `NCREDCLI` | `ALTA` | `fechaalta` | `cancelado=0`, usuario = `usualta`, `fechadoc = fechanot`. |
| `NCREDCLI` | `MODI` | `fechamodi` | Cambios detectados en fecha u hora de alta, `cancelado=0`, usuario = `usumodi`. |
| `NCREDCLI` | `CANC` | `fechamodi` | `cancelado=1`, usuario = `usumodi`. |

#### Notas de crédito a proveedores (`notascredprov`)
| `tipodocumento` | `operacion` | Fecha evaluada | Condiciones adicionales |
|-----------------|-------------|----------------|-------------------------|
| `NCREDPROV` | `ALTA` | `fechaalta` | `cancelado=0`, usuario = `usualta`, `fechadoc = fechanot`. |
| `NCREDPROV` | `MODI` | `fechamodi` | Usa alias `ncp` con condición de diferencia entre alta y modificada, `cancelado=0`, usuario = `usumodi`. |
| `NCREDPROV` | `CANC` | `fechamodi` | `cancelado=1`, usuario = `usumodi`. |

#### Notas de cargo a clientes (`notascarcli`)
| `tipodocumento` | `operacion` | Fecha evaluada | Condiciones adicionales |
|-----------------|-------------|----------------|-------------------------|
| `NCARGOCLI` | `ALTA` | `fechaalta` | `cancelado=0`, usuario = `usualta`, `fechadoc = fechanot`. |
| `NCARGOCLI` | `MODI` | `fechamodi` | Compara fecha/hora contra alta, `cancelado=0`, usuario = `usumodi`. |
| `NCARGOCLI` | `CANC` | `fechalta` | Utiliza `fechaalta` en el filtro pese a representar una cancelación; usuario = `usualta`. |

#### Notas de cargo a proveedores (`notascarprov`)
| `tipodocumento` | `operacion` | Fecha evaluada | Condiciones adicionales |
|-----------------|-------------|----------------|-------------------------|
| `NCARGOPROV` | `ALTA` | `fechaalta` | `cancelado=0`, usuario = `usualta`, `fechadoc = fechanot`. |
| `NCARGOPROV` | `MODI` | `fechamodi` | Valida diferencia con alta, `cancelado=0`, usuario = `usumodi`. |
| `NCARGOPROV` | `CANC` | `fechamodi` | `cancelado=1`, usuario = `usumodi`. |

#### Movimientos de almacén (`movalma`)
| `tipodocumento` | `operacion` | Tipo (`movalma.tipo`) | Fecha evaluada | Condiciones adicionales |
|-----------------|-------------|-----------------------|----------------|-------------------------|
| `MOVALMA` | `ENT-ALTA` | `E` | `fechaalta` | `cancelado=0`, usuario = `usualta`, `fechadoc = fechamov`. |
| `MOVALMA` | `ENT-MODI` | `E` | `fechamodi` | Diferencia con alta, `cancelado=0`, usuario = `usumodi`. |
| `MOVALMA` | `ENT-CANC` | `E` | `fechamodi` | `cancelado=1`, usuario = `usumodi`. |
| `MOVALMA` | `SAL-ALTA` | `S` | `fechaalta` | `cancelado=0`, usuario = `usualta`. |
| `MOVALMA` | `SAL-MODI` | `S` | `fechamodi` | Diferencia con alta, `cancelado=0`, usuario = `usumodi`. |
| `MOVALMA` | `SAL-CANC` | `S` | `fechamodi` | `cancelado=1`, usuario = `usumodi`. |
| `MOVALMA` | `TRASP-ALTA` | `T` | `fechaalta` | `cancelado=0`, usuario = `usualta`. |
| `MOVALMA` | `TRASP-MODI` | `T` | `fechamodi` | Diferencia con alta, `cancelado=0`, usuario = `usumodi`. |
| `MOVALMA` | `TRASP-CANC` | `T` | `fechamodi` | `cancelado=1`, usuario = `usumodi`. |

#### Pagos de clientes (`pagoscli`)
| `tipodocumento` | `operacion` | Fecha evaluada | Condiciones adicionales |
|-----------------|-------------|----------------|-------------------------|
| `PAGOSCLI` | `ALTA` | `fecha` | `cancelado=0`, usuario = `usualta`, `fechadoc = fecha`. |
| `PAGOSCLI` | `MODI` | `fechamodi` | Requiere que `fecha <> fechamodi` **y** `hora <> horamodi`, `cancelado=0`, usuario = `usumodi`. |
| `PAGOSCLI` | `CANC` | `fechamodi` | `cancelado=1`, usuario = `usumodi`. |

#### Pagos a proveedores (`pagosprov`)
| `tipodocumento` | `operacion` | Fecha evaluada | Condiciones adicionales |
|-----------------|-------------|----------------|-------------------------|
| `PAGOSPROV` | `ALTA` | `fecha` | `cancelado=0`, usuario = `usualta`, `fechadoc = fecha`. |
| `PAGOSPROV` | `MODI` | `fechamodi` | Comparación `fecha <> fechamodi` y `hora <> horamodi`, `cancelado=0`, usuario = `usumodi`. |
| `PAGOSPROV` | `CANC` | `fechamodi` | `cancelado=1`, usuario = `usumodi`. |

#### Pedidos de venta (`pedidosventa`)
| `tipodocumento` | `operacion` | Fecha evaluada | Condiciones adicionales |
|-----------------|-------------|----------------|-------------------------|
| `PEDVENTA` | `ALTA` | `fechamodi` | Se toma `cancelado=0`, usuario = `usumodi`, `fechadoc = fechaped`. |
| `PEDVENTA` | `MODI` | `fechamodi` | Requiere diferencia con alta y `cancelado=1`, usuario = `usumodi`. |
| `PEDVENTA` | `CANC` | `fechamodi` | `cancelado=1`, usuario = `usumodi`. |

#### Pedidos de compra (`pedidos`)
| `tipodocumento` | `operacion` | Fecha evaluada | Condiciones adicionales |
|-----------------|-------------|----------------|-------------------------|
| `PEDCOMPRA` | `ALTA` | `fechamodi` | `cancelado=0`, usuario = `usumodi`, `fechadoc = fechaped`. |
| `PEDCOMPRA` | `MODI` | `fechamodi` | Diferencia con alta y `cancelado=1`, usuario = `usumodi`. |
| `PEDCOMPRA` | `CANC` | `fechamodi` | `cancelado=1`, usuario = `usumodi`. |

La sentencia completa termina con `ORDER BY fechaoper DESC, horaoper DESC, referencia`, priorizando las operaciones más recientes. No se usa `UNION ALL`, por lo que MySQL elimina cualquier fila duplicada resultante.

### 3.3 Ejecución y serialización
Una vez concatenado el SQL, `mServidorVioleta->EjecutaSelectSql` ejecuta la consulta y llena el buffer de respuesta respetando el formato de `BufferRespuestas` (metadatos de campos seguidos de registros). El tamaño real queda almacenado en `Respuesta->TamBufferResultado`. No se realizan paginaciones ni límites adicionales: la cantidad de filas depende únicamente del rango de fechas, el conjunto de usuarios y las condiciones de cada bloque.

---

## 4. Estructura de datos y relaciones
### 4.1 Mapeo entre resultados y grid
| Columna en `StringGridBitacora` | Campo SQL | Descripción | Tipo MySQL |
|---------------------------------|-----------|-------------|------------|
| 0 – Referencia | `referencia` | Identificador del documento o movimiento según la tabla origen. | `varchar(11)` (o equivalente según la tabla). |
| 1 – Tipo documento | `tipodocumento` | Literal que agrupa entidades (`VENT`, `COMP`, `MOVALMA`, etc.). | `char/varchar` generado en la consulta. |
| 2 – Operación | `operacion` | Literal que describe la acción (`ALTA`, `MODI`, `CANC`, variantes `ENT-*`, etc.). | `char/varchar` generado en la consulta. |
| 3 – Activo | `cancelado` | Indicador de cancelación (`0` activo, `1` cancelado). | `tinyint(1)` convertido a texto. |
| 4 – Fecha doc. | `fechadoc` | Fecha de negocio del documento (venta, compra, nota, movimiento). | `date` formateado por MySQL. |
| 5 – Usuario | `usuariooper` | Usuario (clave de empleado) responsable de la operación. | `varchar(10)` según `usuarios.empleado`. |
| 6 – Fecha oper. | `fechaoper` | Fecha en la que se registró la operación (alta/modificación/cancelación). | `date`. |
| 7 – Hora oper. | `horaoper` | Hora asociada a la operación. | `time`. |

La columna 3 se presenta con iconografía en lugar de texto gracias a `ImageListIconos`.

### 4.2 Tablas consultadas en MySQL
A continuación se listan los campos relevantes de cada tabla involucrada, con énfasis en aquellos usados por la consulta:

#### `ventas` (`db/ventas.sql`)
| Campo | Tipo | Restricciones | Uso |
|-------|------|---------------|-----|
| `referencia` | `varchar(11)` | PK | Proyectado como `referencia`. |
| `cancelado` | `tinyint(1)` | Puede ser NULL | Evalúa estado activo/cancelado. |
| `usualta` | `varchar(10)` | FK → `usuarios.empleado` | Filtra altas y muestra `usuariooper` en `ALTA`. |
| `usumodi` | `varchar(10)` | FK → `usuarios.empleado` | Filtra modificaciones/cancelaciones. |
| `fechavta` | `date` | `NOT NULL` | Usado como `fechadoc`. |
| `fechaalta` | `date` | `NOT NULL` | Filtro para operaciones `ALTA`. |
| `horaalta` | `time` | | Horario para `ALTA`. |
| `fechamodi` | `date` | `NOT NULL` | Filtro de `MODI` y `CANC`. |
| `horamodi` | `time` | | Horario de `MODI`/`CANC`. |

#### `compras` (`db/compras.sql`)
| Campo | Tipo | Restricciones | Uso |
|-------|------|---------------|-----|
| `referencia` | `varchar(11)` | PK | `referencia`. |
| `cancelado` | `tinyint(1)` | | Determina operación. |
| `usualta` / `usumodi` | `varchar(10)` | FK → `usuarios` | Filtro de usuario. |
| `fechacom` | `date` | | `fechadoc`. |
| `fechaalta` / `horaalta` | `date/time` | | Parámetros `ALTA`. |
| `fechamodi` / `horamodi` | `date/time` | | Parámetros `MODI`/`CANC`. |

#### `notascredcli` (`db/notascredcli.sql`)
| Campo | Tipo | Restricciones | Uso |
|-------|------|---------------|-----|
| `referencia` | `varchar(11)` | PK | `referencia`. |
| `cancelado` | `tinyint(1)` | | Estado. |
| `usualta` / `usumodi` | `varchar(10)` | FK → `usuarios` | Filtro de usuario. |
| `fechanot` | `date` | | `fechadoc`. |
| `fechaalta` / `horaalta` | `date/time` | | Fecha de alta. |
| `fechamodi` / `horamodi` | `date/time` | | Fecha/hora de cambio. |

#### `notascredprov` (`db/notascredprov.sql`)
Campos equivalentes a los de `notascredcli`; se agrega `muuid`, `impuestoret` y `totalret` pero no participan en la consulta.

#### `notascarcli` (`db/notascarcli.sql`)
| Campo | Tipo | Uso |
|-------|------|-----|
| `referencia` (`varchar(11)`) | Proyectado. |
| `cancelado` (`tinyint(1)`) | Evaluado en operaciones. |
| `usualta` / `usumodi` (`varchar(10)`) | Filtran usuarios. |
| `fechanot` (`date`) | `fechadoc`. |
| `fechaalta` / `horaalta`, `fechamodi` / `horamodi` | Determinan fechas de operación. |

#### `notascarprov` (`db/notascarprov.sql`)
Mismo patrón que `notascarcli`, con `proveedor` como FK hacia `proveedores`.

#### `movalma` (`db/movalma.sql`)
| Campo | Tipo | Uso |
|-------|------|-----|
| `movimiento` | `varchar(11)` | `referencia`. |
| `tipo` | `varchar(1)` | Diferencia `ENT`, `SAL`, `TRASP`. |
| `cancelado` | `tinyint(1)` | Determina operación. |
| `usualta` / `usumodi` | `varchar(10)` | Usuario operador. |
| `fechaalta` / `horaalta`, `fechamodi` / `horamodi` | Fechas de operación. |
| `fechamov` | `date` | `fechadoc`. |

#### `pagoscli` (`db/pagoscli.sql`) y `pagosprov` (`db/pagosprov.sql`)
Campos clave `pago`, `cancelado`, `usualta`, `usumodi`, `fecha`, `hora`, `fechamodi`, `horamodi`. Las dos tablas tienen estructura homologada para clientes y proveedores respectivamente.

#### `pedidosventa` (`db/pedidosventa.sql`)
| Campo | Tipo | Uso |
|-------|------|-----|
| `referencia` | `varchar(11)` | `referencia`. |
| `cancelado` | `tinyint(1)` | Estado. |
| `usumodi` | `varchar(10)` | Usuario en todas las operaciones. |
| `fechaped` | `date` | `fechadoc`. |
| `fechaalta` / `horaalta`, `fechamodi` / `horamodi` | Determinan altas y cambios. |

#### `pedidos` (`db/pedidos.sql`)
Estructura similar a `pedidosventa` con campos `referencia`, `cancelado`, `usumodi`, `fechaped`, `fechaalta`, `horaalta`, `fechamodi`, `horamodi`.

#### Tablas auxiliares utilizadas por la interfaz
- `sucursales` (`db/almacenes.sql` no, actual `db/...`): se utiliza campo `sucursal` y `nombre` filtrado por `idempresa`.
- `usuarios` (`db/usuarios.sql`) y `empleados` (`db/empleados.sql`): se requiere `usuarios.empleado` y los campos de nombre (`nombre`, `appat`, `apmat`, `sucursal`) para poblar los combos y construir la lista de empleados homónimos.

Las relaciones principales se basan en claves foráneas hacia `usuarios.empleado`, lo que permite rastrear quién generó cada alta, modificación o cancelación.

---

## 5. Ejemplos de ejecución
### 5.1 Consulta de ventas recientes por un usuario
Parámetros introducidos:
- Sucursal: `SUC01`.
- Usuario elegido: empleado `USR123`; tras la consulta homónima se obtiene la lista `'USR123','USR987'`.
- Rango de fechas: 2025-01-01 a 2025-01-31.

Fragmento del SQL generado:
```sql
(SELECT v.referencia AS referencia, 'VENT' AS tipodocumento, 'ALTA' AS operacion, v.cancelado,
        v.fechavta AS fechadoc, v.usualta AS usuariooper, v.fechaalta AS fechaoper, v.horaalta AS horaoper
 FROM ventas v
 WHERE v.cancelado=0
   AND v.fechaalta BETWEEN '2025-01-01' AND '2025-01-31'
   AND v.usualta IN ('USR123','USR987'))
UNION
...
ORDER BY fechaoper DESC, horaoper DESC, referencia;
```

Ejemplo de fila recibida y mostrada en el grid:

| Columna | Valor |
|---------|-------|
| Referencia | `VTA0004567` |
| Tipo documento | `VENT` |
| Operación | `ALTA` |
| Activo | `0` |
| Fecha doc. | `2025-01-15` |
| Usuario | `USR123` |
| Fecha oper. | `2025-01-15` |
| Hora oper. | `11:23:04` |

La columna Activo se representa con el ícono de estado activo (índice 4).

### 5.2 Cancelación de movimiento de almacén
Parámetros:
- Usuario: `ALM045`.
- Fechas: 2024-11-10 a 2024-11-12.

Fragmento relevante:
```sql
(SELECT ma.movimiento AS referencia,'MOVALMA' AS tipodocumento,'ENT-CANC' AS operacion, ma.cancelado,
        ma.fechamov AS fechadoc, ma.usumodi AS usuariooper, ma.fechamodi AS fechaoper, ma.horamodi AS horaoper
 FROM movalma ma
 WHERE ma.tipo = 'E'
   AND ma.cancelado = 1
   AND ma.fechamodi BETWEEN '2024-11-10' AND '2024-11-12'
   AND ma.usumodi IN ('ALM045'))
...
```

Fila proyectada al grid:

| Columna | Valor |
|---------|-------|
| Referencia | `MOV0098123` |
| Tipo documento | `MOVALMA` |
| Operación | `ENT-CANC` |
| Activo | `1` |
| Fecha doc. | `2024-11-09` |
| Usuario | `ALM045` |
| Fecha oper. | `2024-11-11` |
| Hora oper. | `18:42:51` |

La columna Activo genera el ícono de cancelado (índice 1). El usuario puede imprimir o exportar el resultado desde el menú contextual del grid.

---
