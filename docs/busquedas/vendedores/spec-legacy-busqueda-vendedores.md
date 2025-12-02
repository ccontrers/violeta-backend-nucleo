# Especificación Técnica Legada: Módulo de Búsqueda de Vendedores

Documento técnico que describe el comportamiento actual del módulo de búsqueda de vendedores del sistema legado (cliente C++ Builder + servidor propietario + MySQL). Este material únicamente explica el funcionamiento existente sin proponer cambios.

---
## 1. Introducción técnica general
- **Propósito**: permitir que el usuario localice y seleccione registros de vendedores para otras operaciones (asignaciones, catálogos, etc.) a partir de criterios básicos (nombre, apellidos, tipo de comisión o clave interna).
- **Capas involucradas**:
  - **Interfaz**: formulario modal `TFormBusqVendedores` (archivos `FormBusquedaVendedores.cpp/.h/.dfm`).
  - **Cliente**: método `TFormBusqVendedores::BuscaVendedor` que prepara parámetros y consume la respuesta vía `BufferRespuestas`.
  - **Servidor**: procedimiento `ServidorBusquedas::BuscaVendedores` (`ClassServidorBusquedas.cpp`) que construye y ejecuta SQL.
  - **Datos**: tablas `vendedores` y `empleados` de MySQL (con relaciones a `terminales` y a la misma `vendedores`).
- **Flujo principal**:
  1. Usuario captura criterio y pulsa el botón Buscar en la pestaña correspondiente.
  2. El cliente envía una petición `ID_BUSQ_VENDEDOR` con tres parámetros (tipo de búsqueda, bandera de activos, valor buscado).
  3. El servidor decodifica parámetros, arma el `SELECT` con `JOIN` entre `empleados` y `vendedores`, aplica `LIMIT` (`NUM_LIMITE_RESULTADOS_BUSQ = 501`) y devuelve el buffer serializado.
  4. El cliente llena el `VTStringGrid` con la respuesta y permite seleccionar la fila deseada.
- **Diagrama textual**:
  ```
  Usuario → [TFormBusqVendedores] → (ID_BUSQ_VENDEDOR) → ServidorBusquedas::BuscaVendedores → MySQL
                                     ↑                                                          ↓
                                   BufferRespuestas ←---------- Respuesta tabular empaquetada ←-
  ```

---
## 2. Estructura de la interfaz (`TFormBusqVendedores`)
- **Descripción general**: formulario modal con cuatro pestañas de criterio, un grid de resultados y botones de acción. Se instancia una única vez (objeto global `FormBusqVendedores`).
- **Componentes relevantes**:

| Tipo VCL | Nombre | Ubicación (dfm) | Función | Observaciones |
|----------|--------|-----------------|---------|---------------|
| `TPageControl` | `tpgBusqueda` | nodo raíz | Contenedor de pestañas de búsqueda. `ActivePage` inicial = `TabSheetClave`. |
| `TTabSheet` | `TabSheetNombre` | pestaña 0 | Criterio por nombre. |
| `VTLabeledEdit` | `EditNombre` | dentro `TabSheetNombre` | Entrada de texto (uppercase, `MaxLength=50`). |
| `VTBitBtn` | `ButtonBuscarNombre` | `TabSheetNombre` | Ejecuta búsqueda tipo `"NOM"`. |
| `TTabSheet` | `TabSheetApellido` | pestaña 1 | Criterio por apellido paterno/materno. |
| `VTLabeledEdit` | `EditApellido` | `MaxLength=50` | Texto uppercase de apellidos. |
| `VTBitBtn` | `ButtonBuscarApellido` | | Ejecuta búsqueda tipo `"APE"`. |
| `TTabSheet` | `TabSheetNomneg` | pestaña 2 | Renombrado en UI como "Tipo de comisión". |
| `VTLabeledEdit` | `EditTipoComi` | `MaxLength=50` | Permite filtrar por clave exacta de `v.tipocomi`. |
| `VTBitBtn` | `ButtonBuscarNomnegocio` | | Ejecuta búsqueda tipo `"COMI"`. |
| `TTabSheet` | `TabSheetClave` | pestaña 3 | Criterio por clave de vendedor. |
| `VTLabeledEdit` | `EditClave` | `MaxLength=11` | Captura prefijo de `empleados.empleado`. |
| `VTBitBtn` | `ButtonBuscarClave` | | Ejecuta búsqueda tipo `"CLA"`. |
| `TLabel` | `LabelResultados` | encima del grid | Título fijo “Resultados”. |
| `VTStringGrid` | `StringGridResultados` | cuerpo | Muestra columnas `empleado`, `nombre`, `localidad`, `tipocomi`. `DefaultDrawing=false` para personalización. |
| `VTBitBtn` | `ButtonSeleccionar` | pie | Devuelve selección y cierra. `Default` se activa dinámicamente cuando hay filas. |
| `VTBitBtn` | `ButtonCancelar` | pie | Cancela y cierra (`ModalResult`). |

- **Eventos principales**:
  - `FormShow`: llama `Inicializa()`, limpia campos, borra filas existentes, y fuerza el foco a `EditNombre` (usando `mControlador.ForceFocus`, que también activa la pestaña apropiada).
  - `FormPaint`: delega en `gClienteVioleta->GradienteForma(this)` para repintar con el gradiente corporativo.
  - `FormKeyDown`: atajos `Ctrl+1..4` para enfocar rápidamente cada campo (`EditNombre`, `EditApellido`, `EditTipoComi`, `EditClave`).
  - Botones Buscar (`OnClick`): validan que el campo no esté vacío y luego invocan `BuscaVendedor` con el código correspondiente. Si el campo está vacío, muestran `AppMessageBox` con mensaje de error y reposicionan el foco.
  - `ButtonSeleccionarClick`: copia valores de la fila activa (`Cells[0]` y `Cells[1]`) a `mClaveVendedor`/`mNombreVendedor`, marca que hay selección y cierra.
  - `StringGridResultadosDrawCell`: centra encabezados, usa estilos personalizados y evita texto literal en la columna de tipo de comisión si requiere íconos (no aplica íconos específicos en este formulario pero mantiene la misma infraestructura que otros grids).
  - `StringGridResultadosClick`: habilita el botón Seleccionar cuando hay fila válida.
  - `Edit*Enter/Exit`: alternan cuál botón `Default` recibe el Enter, evitando que `ButtonSeleccionar` capture la tecla antes de ejecutar la búsqueda.

- **Variables y banderas internas**:
  - `ControladorInterfaz mControlador`: utilitario que administra foco/tab y validaciones.
  - `FuncionesGenericas mFg`: usado para mensajes y utilidades de cadena.
  - `bool mSoloActivos`: bandera enviada a servidor. Inicialmente `false`, se puede sobrescribir mediante `AsignaSoloActivos(bool)`. No hay control visual para esta bandera; depende de quién abra el formulario.
  - `AnsiString mClaveVendedor`, `mNombreVendedor`: valores retornados al llamante después de `ButtonSeleccionar`.
  - `Abrir()`: método público que muestra el modal (`ShowModal()`). Retorna `bool` pero actualmente no devuelve valor explícito (comportamiento indefinido) tal como está en el código legado.

---
## 3. Detalle por pestaña (criterios de búsqueda)
Cada pestaña dispara la misma rutina `BuscaVendedor`, cambiando el parámetro `tipo_busqueda` y el campo que se evalúa en el servidor. Todas las entradas aplican `CharCase = ecUpperCase` antes de enviar.

### 3.1 TabSheetNombre – criterio `"NOM"`
- **Campos**: `EditNombre` (texto libre, requerido, hasta 50 caracteres).
- **Validaciones**: si está vacío se muestra “Debe escribir al menos parte del nombre...”.
- **SQL resultante**:
  ```sql
  SELECT e.empleado,
         CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS nombre,
         e.localidad,
         v.tipocomi
  FROM empleados e
  INNER JOIN vendedores v ON e.empleado = v.empleado
  WHERE e.nombre LIKE '<dato_buscado>%'
    <CONDICION_ACTIVOS>
  ORDER BY e.nombre
  LIMIT 501;
  ```
  `CONDICION_ACTIVOS` se reemplaza por `AND v.activo=1` cuando `solo_activos="1"`; en caso contrario se usa `AND v.activo=0`.
- **Ordenamiento**: alfabético por `e.nombre`.

### 3.2 TabSheetApellido – criterio `"APE"`
- **Campos**: `EditApellido` (50 caracteres, requerido).
- **SQL resultante**:
  ```sql
  SELECT e.empleado,
         CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS nombre,
         e.localidad,
         v.tipocomi
  FROM empleados e
  INNER JOIN vendedores v ON e.empleado = v.empleado
  WHERE (e.appat LIKE '<dato_buscado>%' OR e.apmat LIKE '<dato_buscado>%')
    <CONDICION_ACTIVOS>
  ORDER BY e.appat
  LIMIT 501;
  ```
- **Ordenamiento**: por apellido paterno (`e.appat`).

### 3.3 TabSheetNomneg – criterio `"COMI"`
- **Propósito**: filtrar vendedores por código de tipo de comisión exacto.
- **Campos**: `EditTipoComi` (50 caracteres, requerido). El texto se envía sin comodines, exige coincidencia exacta.
- **SQL resultante**:
  ```sql
  SELECT e.empleado,
         CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS nombre,
         e.localidad,
         v.tipocomi
  FROM empleados e
  INNER JOIN vendedores v ON e.empleado = v.empleado
  WHERE v.tipocomi = '<dato_buscado>'
    <CONDICION_ACTIVOS>
  ORDER BY v.tipocomi
  LIMIT 501;
  ```
- **Ordenamiento**: por `v.tipocomi`.

### 3.4 TabSheetClave – criterio `"CLA"`
- **Campos**: `EditClave` (11 caracteres máximo, requerido). Se interpreta como prefijo.
- **SQL resultante**:
  ```sql
  SELECT e.empleado,
         CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS nombre,
         e.localidad,
         v.tipocomi
  FROM empleados e
  INNER JOIN vendedores v ON e.empleado = v.empleado
  WHERE e.empleado LIKE '<dato_buscado>%'
    <CONDICION_ACTIVOS>
  ORDER BY e.empleado
  LIMIT 501;
  ```
- **Ordenamiento**: por clave de empleado (alfanumérico).

### 3.5 Grid de resultados (`VTStringGrid`)
- **Columnas visuales**: 4 (Clave, Nombre completo, Localidad, Tipo de comisión). Los encabezados se establecen en el constructor y son sobrescritos por `BufferRespuestas::LlenaStringGrid(...)` cuando llega la respuesta (primer fila = nombres de campo devueltos por el servidor).
- **Relleno**: `LlenaStringGrid(StringGridResultados, true, false)` — primer parámetro indica inclusión de encabezados de columna; el segundo evita agregar índice numérico. Cada fila del buffer se asigna a `Cells[col][row]`.
- **Selección**: se marca `Row=1` al cargar resultados y se posiciona el foco en el grid. El botón Seleccionar se vuelve `Default` únicamente cuando hay registros (`num_encontrados > 0`).
- **Colores**: el formulario toma el color corporativo desde `gClienteVioleta->Param->Valor("COLORVENTANAS")` convertido a `TColor`.

---
## 4. Lógica servidor (`ServidorBusquedas::BuscaVendedores`)
- **Ubicación**: `cpp/ClassServidorBusquedas.cpp`, líneas 3895–3990 aprox.
- **Parámetros recibidos** (orden fijo, extraídos con `FuncionesGenericas::ExtraeStringDeBuffer`):
  1. `tipo_busqueda` (`"NOM"`, `"APE"`, `"COMI"`, `"CLA"`).
  2. `solo_activos` (`"1"` para activos, cualquier otro valor → se fuerza `v.activo=0`).
  3. `dato_buscado` (prefijo o valor exacto según caso).
- **Inicialización**: se llama `mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ);` para reutilizar el buffer circular del servidor. `TAM_MAX_BUFFER_RESPUESTA_BUSQ` es una constante global del servidor (valor definido en otro módulo, tamaño máximo para consultas de búsqueda).
- **Construcción de condición de activos**:
  ```cpp
  if (solo_activos == "1")
      condicion_activos = " AND v.activo=1 ";
  else
      condicion_activos = " AND v.activo=0 ";
  ```
  No existe opción para traer ambos estados simultáneamente; si el formulario no configura la bandera, el servidor filtrará por `v.activo=0` (solo inactivos).
- **Branching por tipo**: cuatro bloques `if/else if` arman la sentencia `SELECT` usando `AnsiString::sprintf`. No hay `else` de escape; si el tipo no coincide con ninguno, no se ejecuta consulta.
- **Ejecución**: cada bloque termina con `mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);`, que ejecuta la instrucción y empaqueta el conjunto de resultados en el buffer de respuesta (`Respuesta->TamBufferResultado` se actualiza con la longitud real).
- **Columnas devueltas** (orden fijo):
  1. `empleado` (PK de `empleados`, coincide con PK de `vendedores`).
  2. `nombre` (cadena concatenada con espacios).
  3. `localidad` (texto libre del empleado).
  4. `tipocomi` (clave de tipo de comisión definida en `vendedores`).
- **Límites**: todas las sentencias añaden `LIMIT NUM_LIMITE_RESULTADOS_BUSQ`, donde `NUM_LIMITE_RESULTADOS_BUSQ` está definido como literal `"501"` en `ClassServidorBusquedas.h`.
- **Serialización de resultados**: el contenido devuelto respeta el formato de `BufferRespuestas` (metadatos de campos seguido de registros). El cliente interpreta el orden directamente al llenar el grid, sin transformaciones adicionales.
- **Manejo de errores**: no hay validaciones extra sobre el contenido de `dato_buscado`. Si el búfer de entrada está vacío o el servidor no construye consulta, el buffer de respuesta queda en blanco y el cliente interpreta “0 registros”.

---
## 5. Estructura de datos y relaciones
- **Tabla `vendedores`** (`db/vendedores.sql`):

| Campo | Tipo | Restricciones | Uso en módulo |
|-------|------|---------------|---------------|
| `empleado` | `varchar(10)` | PK, FK → `empleados.empleado`, `ON UPDATE CASCADE` | Clave devuelta en la columna “Clave”. |
| `tipocomi` | `varchar(2)` | Puede ser NULL | Mostrado en columna “Tipo de comisión”; criterio exacto en pestaña COMI. |
| `tipovend` | `varchar(1)` | Puede ser NULL | No se usa en la búsqueda. |
| `activo` | `tinyint(1)` | 0/1 | Filtrado mediante `condicion_activos`. |
| `fechaalta` | `date` | `NOT NULL` | No visible. |
| `porccomi` | `decimal(5,2)` | `NOT NULL` | No visible. |
| `tolercomision` | `int(3)` | Default 0 | No visible. |
| `fechabaja` | `date` | `NOT NULL` | No visible. |
| `terminalmovil` | `varchar(10)` | FK → `terminales.terminal` | No se consulta; relevante para integraciones móviles. |
| `vendedor_imitar` | `varchar(10)` | FK auto-referenciada → `vendedores.empleado` | No utilizado en la búsqueda actual. |

  Índices complementarios: `KEY vendedores_ibfk_2 (terminalmovil)` y `KEY vendedor_imitar (vendedor_imitar)`.

- **Tabla `empleados`** (`db/empleados.sql`):

| Campo | Tipo | Restricciones | Uso en módulo |
|-------|------|---------------|---------------|
| `empleado` | `varchar(10)` | PK | Se usa en filtro `LIKE` y como primera columna del resultado. |
| `nombre` | `varchar(40)` | `NOT NULL` | Prefijo evaluado en criterio NOM; contribuye al `CONCAT`. |
| `appat` | `varchar(40)` | `NOT NULL` | Prefijo evaluado en criterio APE; parte del `CONCAT`. |
| `apmat` | `varchar(40)` | `NOT NULL` | Prefijo opcional en criterio APE; parte del `CONCAT`. |
| `localidad` | `varchar(40)` | Puede ser NULL | Mostrada tal como viene en la tercera columna. |
| `sucursal` | `varchar(2)` | FK → `sucursales.sucursal` | No participa en filtros pero determina sucursal asociada al vendedor. |
| `activo` | `tinyint(1)` | 0/1 | No se usa directamente; el filtro de activos se aplica sobre `vendedores.activo`. |
| Otros campos (calle, colonia, teléfonos, RFC, etc.) | varios | | No intervienen en esta búsqueda. |

- **Tabla `terminales`** (`db/terminales.sql`): relevante por la FK `vendedores.terminalmovil`. No se consulta directamente, pero su existencia explica la restricción referencial cuando `terminalmovil` está poblado.

- **Relaciones**:
  - `vendedores.empleado` ↔ `empleados.empleado` (relación 1:1). El módulo depende de esta relación para recuperar datos personales.
  - `vendedores.vendedor_imitar` ↔ `vendedores.empleado` (recursiva opcional). No se usa en el formulario pero debe considerarse para entender posibles dependencias de datos.
  - `empleados.sucursal` ↔ `sucursales.sucursal`: aunque no se une explícitamente aquí, condiciona integridad de la fila.

---
## 6. Ejemplos de ejecución

### 6.1 Búsqueda por nombre (activos)
- **Entrada usuario**: pestaña “Por nombre”, texto `MAR`, bandera `mSoloActivos=true` (establecida desde el llamante).
- **Parámetros enviados** (en orden): `"NOM"`, `"1"`, `"MAR"`.
- **SQL ejecutada**:
  ```sql
  SELECT e.empleado,
         CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS nombre,
         e.localidad,
         v.tipocomi
  FROM empleados e
  INNER JOIN vendedores v ON e.empleado = v.empleado
  WHERE e.nombre LIKE 'MAR%'
    AND v.activo=1
  ORDER BY e.nombre
  LIMIT 501;
  ```
- **Respuesta típica** (orden de columnas):
  | empleado | nombre                | localidad | tipocomi |
  |----------|-----------------------|-----------|----------|
  | 000123   | MARIA LOPEZ GARCIA    | CD. VICTORIA | CA |
  | 000845   | MARCOS REYES PEREZ    | MONTERREY | CB |

  El grid mostrará dos filas y permitirá seleccionar cualquiera; `ButtonSeleccionar` se activa automáticamente.

### 6.2 Búsqueda por tipo de comisión (inactivos)
- **Entrada usuario**: pestaña “Tipo de comisión”, texto `CB`, `mSoloActivos` permanece en `false` (valor por defecto del formulario).
- **Parámetros enviados**: `"COMI"`, `"0"`, `"CB"`.
- **SQL ejecutada**:
  ```sql
  SELECT e.empleado,
         CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS nombre,
         e.localidad,
         v.tipocomi
  FROM empleados e
  INNER JOIN vendedores v ON e.empleado = v.empleado
  WHERE v.tipocomi = 'CB'
    AND v.activo=0
  ORDER BY v.tipocomi
  LIMIT 501;
  ```
- **Comportamiento**: solo se listan vendedores con `v.activo=0` y `tipocomi='CB'`. Si no hubiera coincidencias, el cliente mostraría el mensaje “No se encontró ningún registro de comisión = CB” y el botón Seleccionar se mantiene deshabilitado.

### 6.3 Búsqueda por clave (prefijo)
- **Entrada usuario**: pestaña “Por clave”, texto `007`, `mSoloActivos=true`.
- **Parámetros enviados**: `"CLA"`, `"1"`, `"007"`.
- **SQL generada**: `WHERE e.empleado LIKE '007%' AND v.activo=1` con el resto de la estructura estándar.
- **Resultado**: el formulario devuelve la clave completa del vendedor seleccionado al método llamante mediante `ObtieneVendedor()` y su nombre concatenado vía `ObtieneNomVendedor()`.

---
## 7. Notas de operación relevantes
- `ID_BUSQ_VENDEDOR` identifica la petición en el protocolo binario propietario administrado por `ClassClienteVioleta`. El servidor asume que los parámetros llegan en el orden descrito; no hay metadatos ni nombres explícitos dentro del buffer.
- La UI no presenta controles para alternar entre activos/inactivos; el valor se controla externamente llamando a `AsignaSoloActivos`. Si no se inicializa, el módulo devuelve únicamente vendedores con `v.activo=0`.
- `ButtonSeleccionar` no establece `ModalResult`; el cierre se realiza vía `Close()` y el llamante debe consultar `ObtieneVendedor()`/`ObtieneNomVendedor()` para conocer la selección.
- `TFormBusqVendedores::Abrir()` invoca `ShowModal()` sin retornar valor. El flujo original obtiene los datos a través de los métodos `Obtiene*` tras el cierre.

---
Fin de la documentación técnica del módulo legado de búsqueda de vendedores.