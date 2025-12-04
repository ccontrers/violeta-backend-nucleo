# Especificación técnica del módulo legacy de bitácora de modificaciones de privilegios

## 1. Introducción técnica general
- El formulario `TFormBitacoraModPrivi` permite consultar la trazabilidad de cambios sobre privilegios asignados a usuarios y roles dentro del sistema legado. El usuario final puede filtrar por fechas, empleado operador, rol modificado, tipo de contexto (usuarios o roles) y tipo de entidad afectada, obteniendo un listado tabular con origen de datos en MySQL.
- La arquitectura combina una capa de presentación VCL (componentes personalizados prefijados con `VT*`), una capa de servicios cliente (`gClienteVioleta`, `Interfaz`, `ControladorInterfaz`, `PrivilegiosDeObjeto`) y la capa de datos en tablas `bitacoramodprivusu`, `bitacoramodprivrol` y catálogos asociados (`empleados`, `usuarios`, `rolessistema`, `privilegios`, `objetossistema`).
- El flujo de ejecución es estrictamente controlado en el cliente; las consultas SQL se arman en tiempo de ejecución en `MostrarBitacora()` y se ejecutan a través de `gClienteVioleta->Interfaz`. No existe lógica de negocio adicional en el servidor para esta pantalla aparte de las restricciones de vistas/llaves.
- Flujo general (ASCII):
```
Usuario (Botón "Mostrar")
  ↓ eventos VCL (ButtonMuestraReporteClick)
FormBitacoraModPrivi::MostrarBitacora()
  ↓ construye filtros (condicionUsuario/Rol/Tipo/Entidad)
  ↓ arma SELECT con UNION ALL sobre bitacoramodprivusu/bitacoramodprivrol
  ↓ gClienteVioleta->Interfaz->LlenaStringGrid(query, StringGridReporte, true, false)
  ↓ VTStringGrid renderiza filas (StringGridReporteDrawCell)
  ↓ ExportadorDatos habilita impresión/Excel vía menú contextual
Base de datos MySQL (tablas de bitácora y catálogos)
```

## 2. Estructura de la interfaz (Form)
### 2.1 Componentes visibles y layout
- `StringGridReporte : VTStringGrid`
  - Posición `Left=8`, `Top=88`, tamaño 1015x569, 8 columnas visibles, 2 filas iniciales.
  - ColWidth inicial reajustado en constructor: `[80, 80, 230, 230, 120, 120, 120, 200]`.
  - Encabezados asignados en tiempo de ejecución: `Fecha`, `Hora`, `Usuario`, `Usuario modificado`, `Rol modificado`, `Operación`, `Entidad involucrada`, `Nombre de la entidad`.
  - Propiedad `PopupMenu` enlazada a `PopupMenuExportar`.
- `ComboBoxEmpleado : VTComboBox`
  - Carga dinámica vía `LlenaComboBox` con catálogo de usuarios-actores (`empleados` vinculados a `usuarios`).
  - Ancho de columnas (`AnchoColumnas`) preconfigurado 90/180.
- `ComboBoxRol : VTComboBox`
  - Se alimenta con `SELECT claverol, nombre FROM rolessistema ORDER BY nombre`.
- `ComboBoxTipo : VTComboBox`
  - Lista fija de opciones configurada en .dfm: `USUARIOS`, `ROLES`. Determina el contexto del union (`usurol.contexto`).
- `ComboBoxEntidad : VTComboBox`
  - Lista fija en .dfm: `GRUPO`, `OBJETO`, `PRIVILEGIO`, `SUCURSAL`, `ROL`, `USUARIO`. Filtra `entidad_mod`.
- `FechaIni`, `FechaFin : VTDateTimePicker`
  - Inicializados en `BlanqueaFormulario()` con la fecha actual del servidor (`gClienteVioleta->ObtieneFechaDeHoy()`).
- `ButtonMuestraReporte : VTBitBtn`
  - Dispara `ButtonMuestraReporteClick`, icono embebido, etiqueta "Mostrar".
- Etiquetas `Label1..Label6`
  - Indican filtros activos en español, fuente en negritas.
- `PopupMenuExportar`
  - Opciones `Imprimir1` y `ExportaraExcel1` conectadas a métodos homónimos.

### 2.2 Eventos asociados y estado interno
- `FormShow` → `BlanqueaFormulario()` para inicializar controles cada vez que se presenta el formulario.
- `FormPaint` → `gClienteVioleta->GradienteForma(this)` para repintar con degradado corporativo.
- `StringGridReporte.OnDrawCell` → `StringGridReporteDrawCell` aplica alineación centrada/negritas en encabezados.
- `Imprimir1.OnClick` → `Imprimir1Click`: configura `ExportadorDatos` orientado a impresión.
- `ExportaraExcel1.OnClick` → `ExportaraExcel1Click`: exportación rápida a Excel mediante `ExportadorDatos::EnviaExcel()`.
- `ButtonMuestraReporte.OnClick` → `ButtonMuestraReporteClick`: valida fechas, limpia grid y lanza la consulta.
- Variables miembro relevantes:
  - `mPrivilegios : PrivilegiosDeObjeto` para validación de permisos del objeto `BITMPUR`.
  - `mControlador : ControladorInterfaz` no se utiliza en este código específico pero se instancia (consistencia con otros formularios).
  - `mFg : FuncionesGenericas` provee conversiones de fechas, mensajes y utilidades variopintas.
  - `mFechaini`, `mFechafin` almacenan las fechas en formato ANSI tras validación para reutilizarlas en títulos de exportación.

## 3. Lógica de negocios por función (FormBitacoraModPrivilegios.cpp)
### 3.1 Constructor `TFormBitacoraModPrivi::TFormBitacoraModPrivi`
- Define anchos de columnas y títulos del grid.
- Obtiene el color corporativo `COLORVENTANAS` mediante `gClienteVioleta->Param->Valor` y lo aplica a la forma.

### 3.2 `bool Abrir()`
- Ejecuta `mPrivilegios.ConsultaPrivilegios` con el usuario autenticado y el identificador de objeto `BITMPUR` (bitácora de modificaciones de privilegios).
- Si el usuario es administrador (`gClienteVioleta->ObtienePrivAdminSistema()`), agrega la clave del objeto al `Caption` del formulario.
- Si el privilegio de consulta (`mPrivilegios.mPermitidoConsultar`) es falso, muestra un mensaje modal mediante `mFg.AppMessageBox` y no abre la ventana.
- Cuando se poseen privilegios, la forma se muestra modal (`ShowModal()`).
- Retorna `mPrivilegios.TieneAlgunPrivilegio()` para que la capa llamante conozca si existe acceso.

### 3.3 `FormShow`
- Invoca `BlanqueaFormulario()` cada vez que el formulario se hace visible.

### 3.4 `BlanqueaFormulario()`
- Resetea `FechaIni` y `FechaFin` al día actual del servidor.
- Construye la consulta para `ComboBoxEmpleado`:
  ```sql
  SELECT e.empleado,
         CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS nombre
  FROM usuarios u
  INNER JOIN empleados e ON e.empleado = u.empleado
  ORDER BY nombre
  ```
  `LlenaComboBox` interpreta la primera columna como `Clave` y la segunda como descripción mostrada.
- Construye la consulta para `ComboBoxRol`:
  ```sql
  SELECT claverol, nombre
  FROM rolessistema
  ORDER BY nombre
  ```
- Las listas de `ComboBoxTipo` y `ComboBoxEntidad` permanecen tal como se definieron en diseño.

### 3.5 `FormPaint`
- Redibuja el gradiente usando la utilidad `gClienteVioleta->GradienteForma`, consistente con la guía visual de la aplicación.

### 3.6 `StringGridReporteDrawCell`
- Llama a `StringGridReporte->AsignarAtributosDefaultCelda()` (resetea alineación, colores y fuente a los valores base definidos en `VTStringGrid`).
- Para la fila 0 (encabezado) fuerza alineación centrada y fuente en negritas (`TFontStyles() << fsBold`).
- Delegado final a `StringGridReporte->DibujarCelda` para respetar toda la lógica de formato del componente personalizado.

### 3.7 `BlanqueaGrid()`
- Elimina filas de datos utilizando `VTStringGrid::BorrarFilas` desde la fila 1 hasta el final, conservando el encabezado.
- Limpia cada celda de la fila 1 asignándole cadena vacía para evitar residuos visuales.

### 3.8 `ButtonMuestraReporteClick`
- Validaciones previas a la consulta:
  - Si `FechaIni` es mayor que `FechaFin`, muestra error y aborta.
  - Si el rango es mayor a 2 años (`IncYear(FechaFin, -2)`), muestra error y aborta.
- Limpia el grid (`BlanqueaGrid`) y oculta el control (`StringGridReporte->Visible=false`) mientras la consulta se ejecuta.
- Llama a `MostrarBitacora()` y, al finalizar, vuelve a mostrar el grid.

### 3.9 `MostrarBitacora()`
- Convierte las fechas seleccionadas a cadenas (`mFg.DateToAnsiString`) y después a formato MySQL (`mFg.StrToMySqlDate`).
- Construye condiciones dinámicas sólo cuando el filtro tiene valor:
  - `condicionUsuario`: agrega `AND (usurol.usuario = '<empleado>' OR bpu.usuario_mod = '<empleado>')`. Permite ubicar registros donde el empleado actuó como operador (`usurol.usuario`) o fue el usuario modificado (`bpu.usuario_mod`).
  - `condicionRol`: agrega `AND bpr.rol_mod = '<claveRol>'`. Solo impacta las filas provenientes de la tabla de roles (las filas de usuarios conservan `bpr` en NULL tras el `LEFT JOIN`).
  - `condicionTipo`: agrega `AND usurol.contexto = '<USUARIOS|ROLES>'`. El campo `contexto` se determina en la subconsulta UNION.
  - `condicionEntidad`: agrega `AND entidad_mod = '<GRUPO|OBJETO|...>'` y se adjunta tanto al bloque `bitacoramodprivusu` como al bloque `bitacoramodprivrol` dentro del `UNION ALL`.
- La instrucción SQL resultante es:
  ```sql
  SELECT
      usurol.fecha,
      usurol.hora,
      CONCAT(e.nombre, ' ', e.appat, ' ', e.apmat) AS usuario,
      CONCAT(emod.nombre, ' ', emod.appat, ' ', emod.apmat) AS usuario_mod,
      bpr.rol_mod,
      usurol.tipo_mod,
      usurol.entidad_mod,
      usurol.entidad_nombre
  FROM (
      SELECT
          idbitacprivusu AS ID,
          fecha,
          hora,
          usuario,
          tipo_mod,
          entidad_mod,
          entidad_nombre,
          'USUARIOS' AS contexto
      FROM bitacoramodprivusu bpu
      WHERE fecha BETWEEN '<fecha_ini>' AND '<fecha_fin>' <condicionEntidad>
      UNION ALL
      SELECT
          idbitacprivrol AS ID,
          fecha,
          hora,
          usuario,
          tipo_mod,
          entidad_mod,
          entidad_nombre,
          'ROLES' AS contexto
      FROM bitacoramodprivrol bpr
      WHERE fecha BETWEEN '<fecha_ini>' AND '<fecha_fin>' <condicionEntidad>
  ) usurol
  LEFT JOIN bitacoramodprivusu bpu
         ON bpu.idbitacprivusu = usurol.ID AND usurol.contexto = 'USUARIOS'
  LEFT JOIN bitacoramodprivrol bpr
         ON bpr.idbitacprivrol = usurol.ID AND usurol.contexto = 'ROLES'
  LEFT JOIN empleados e
         ON e.empleado = usurol.usuario
  LEFT JOIN empleados emod
         ON emod.empleado = bpu.usuario_mod
  WHERE 1
        <condicionUsuario>
        <condicionRol>
        <condicionTipo>
  ORDER BY usurol.fecha DESC, usurol.hora DESC
  ```
  Los literales `<fecha_ini>` y `<fecha_fin>` corresponden al formato `YYYY-MM-DD` generado por `StrToMySqlDate`.
- Ejecuta la consulta mediante `gClienteVioleta->Interfaz->LlenaStringGrid(Select, StringGridReporte, true, false)`. La bandera `true` indica que el grid debe integrar encabezados/formatos devueltos y `false` deshabilita autoajustes adicionales (comportamiento propio del componente).
- Si la primera fila de datos (`Cells[0][1]`) queda vacía, detona un mensaje informativo "No hay datos" mediante `mFg.AppMessageBox`.

### 3.10 `Imprimir1Click`
- Instancia `ExportadorDatos` con el grid actual como origen.
- Configura fuente por defecto (Arial Narrow 9), colores negros, activación de contornos y márgenes (0.5" laterales, 1" superior e inferior).
- Ajusta anchos de columna para el documento impreso en pulgadas: `[1.7, 1.5, 5.5, 1.5, 5.5, 2.4, 1.5]` (la columna 7 conserva el ancho por defecto del componente ExportadorDatos).
- Define orientación vertical (`poPortrait`).
- Establece títulos:
  - Principal: `Bitácora de modificaciones de privilegios`.
  - Secundario: `De la fecha <FechaIni> al <FechaFin>` usando `mFg.DateToAnsiString` para formateo `DD/MM/YYYY`.
  - Título inferior vacío.
- Envía a impresión (`EnviaImpresion`) y libera instancias auxiliares.

### 3.11 `ExportaraExcel1Click`
- Crea `ExportadorDatos` temporal y llama a `EnviaExcel()` con la configuración interactiva por defecto (diálogo de opciones de exportación). Tras enviar el archivo, destruye la instancia.

## 4. Estructura de datos y relaciones
### 4.1 Tablas de bitácora principales
- `bitacoramodprivusu` (`db/bitacoramodprivusu.sql`)
  - Campos: `idbitacprivusu` (PK, autoincremental), `fecha`, `hora`, `usuario`, `usuario_mod`, `tipo_mod`, `entidad_mod`, `entidad_nombre`.
  - Llaves foráneas: `usuario` y `usuario_mod` referencian `usuarios.empleado`.
  - Uso en módulo: aporta cambios sobre privilegios asignados a usuarios; `usuario_mod` se utiliza para mostrar el "Usuario modificado".
- `bitacoramodprivrol` (`db/bitacoramodprivrol.sql`)
  - Campos: `idbitacprivrol` (PK), `fecha`, `hora`, `usuario` (operador), `rol_mod`, `tipo_mod`, `entidad_mod`, `entidad_nombre`.
  - Llaves foráneas: `usuario` → `usuarios.empleado`, `rol_mod` → `rolessistema.claverol`.
  - Uso en módulo: registra cambios en privilegios asignados a roles; el campo `rol_mod` se despliega en la columna homónima del grid.

### 4.2 Catálogos y asignaciones
- `usuarios` (`db/usuarios.sql`)
  - Campos principales: `empleado` (PK), credenciales y banderas de estado.
  - Llave foránea: `empleado` → `empleados.empleado`.
  - Uso: determina la lista de operadores (`ComboBoxEmpleado`) y soporta integridad referencial con bitácoras.
- `empleados` (`db/empleados.sql`)
  - Campos: identificadores, nombres (`nombre`, `appat`, `apmat`), datos de contacto, referencias a departamentos/puestos, etc.
  - LLaves: `empleado` (PK), múltiples índices secundarios y FKs a catálogos de recursos humanos.
  - Uso: provee textos mostrados en el grid (concatenación para operador y usuario modificado) y en combos.
- `rolessistema` (`db/rolessistema.sql`)
  - Campos: `claverol` (PK), `nombre`.
  - Uso: alimenta `ComboBoxRol` y valida `rol_mod` en bitácora de roles.
- `asignacionprivilegios` (`db/asignacionprivilegios.sql`)
  - Campos: `usuario`, `objeto`, `privilegio`; PK compuesta.
  - Uso indirecto: fuente de cambios que caen en `bitacoramodprivusu`; mantiene integridad con `privilegios` y `usuarios`.
- `asignacionprivrol` (`db/asignacionprivrol.sql`)
  - Campos: `rol`, `objeto`, `privilegio`.
  - Uso indirecto: origen de modificaciones registradas en `bitacoramodprivrol`.
- `privilegios` (`db/privilegios.sql`)
  - Campos: `privilegio`, `objeto`, `descripcion`.
  - FK: `objeto` → `objetossistema.objeto`.
  - Uso: católogo que alimenta las asignaciones y define el alcance de `tipo_mod`/`entidad_mod`.
- `objetossistema` (`db/objetossistema.sql`)
  - Campos: `objeto`, `nombre`, `grupo`.
  - Uso: contexto semántico para `privilegios.objeto`; relevante para comprender valores de `entidad_mod`.

### 4.3 Relaciones relevantes para la consulta
- El `UNION ALL` fusiona registros de ambas bitácoras generando un alias `usurol` con atributo `contexto` que distingue el origen.
- `usurol.usuario` se resuelve contra `empleados` para obtener el nombre completo del operador.
- `bpu.usuario_mod` (disponible sólo cuando `contexto='USUARIOS'`) se resuelve contra `empleados` para mostrar el usuario afectado.
- Para entradas del contexto `ROLES`, el campo `bpr.rol_mod` se mantiene y se muestra tal cual; cuando la fila proviene de usuarios el `LEFT JOIN` deja el campo en NULL.
- `tipo_mod`, `entidad_mod` y `entidad_nombre` provienen de los registros de bitácora; `entidad_mod` se filtra por `ComboBoxEntidad`, `entidad_nombre` se despliega como texto libre.

### 4.4 Correspondencia con el grid
| Índice de columna | Campo SQL | Descripción mostrada |
|-------------------|-----------|-----------------------|
| 0 | `usurol.fecha` | Fecha del evento (ordenada DESC) |
| 1 | `usurol.hora` | Hora exacta del evento |
| 2 | `CONCAT(e.nombre, ... )` | Nombre completo del operador |
| 3 | `CONCAT(emod.nombre, ... )` | Nombre del usuario modificado (sólo filas de contexto usuarios) |
| 4 | `bpr.rol_mod` | Clave del rol modificado (sólo filas de contexto roles) |
| 5 | `usurol.tipo_mod` | Tipo de operación registrada (alta, baja, modificación, etc.) |
| 6 | `usurol.entidad_mod` | Tipo de entidad afectada (Grupo, Objeto, Privilegio, etc.) |
| 7 | `usurol.entidad_nombre` | Identificador descriptivo de la entidad |

La lógica de `VTStringGrid` permite que campos vacíos se representen como celdas vacías sin afectar el formato.

## 5. Ejemplos de ejecución
### 5.1 Consulta por usuario específico en contexto de usuarios
- Parámetros seleccionados:
  - `FechaIni = 01/01/2024`, `FechaFin = 31/01/2024`.
  - `ComboBoxEmpleado` → clave `USR001`.
  - `ComboBoxTipo` → `USUARIOS`.
  - Otros filtros vacíos.
- SQL resultante (fragmento con tokens sustituidos):
  ```sql
  ...
  FROM (
      SELECT ... FROM bitacoramodprivusu bpu
      WHERE fecha BETWEEN '2024-01-01' AND '2024-01-31'
      UNION ALL
      SELECT ... FROM bitacoramodprivrol bpr
      WHERE fecha BETWEEN '2024-01-01' AND '2024-01-31'
  ) usurol
  ...
  WHERE 1
        AND (usurol.usuario='USR001' OR bpu.usuario_mod='USR001')
        AND usurol.contexto='USUARIOS'
  ORDER BY usurol.fecha DESC, usurol.hora DESC
  ```
- Resultado típico presentado en el grid:
  | Fecha | Hora | Usuario | Usuario modificado | Rol modificado | Operación | Entidad involucrada | Nombre de la entidad |
  |-------|------|---------|--------------------|----------------|-----------|----------------------|-----------------------|
  | 2024-01-25 | 10:32:11 | Juan Pérez | María López | *(vacío)* | MODIFICACIÓN | PRIVILEGIO | BITMPUR_CON |
- Acción disponible: exportación/impresión mantiene el mismo orden de columnas.

### 5.2 Consulta por rol en contexto roles y entidad "PRIVILEGIO"
- Parámetros:
  - `FechaIni = 15/03/2024`, `FechaFin = 31/03/2024`.
  - `ComboBoxRol` → clave `ADM`.
  - `ComboBoxEntidad` → `PRIVILEGIO`.
  - Sin filtro de usuario, `ComboBoxTipo` vacío (acepta ambos contextos).
- SQL (fragmento):
  ```sql
  ...
  FROM (
      SELECT ... FROM bitacoramodprivusu bpu
      WHERE fecha BETWEEN '2024-03-15' AND '2024-03-31' AND entidad_mod='PRIVILEGIO'
      UNION ALL
      SELECT ... FROM bitacoramodprivrol bpr
      WHERE fecha BETWEEN '2024-03-15' AND '2024-03-31' AND entidad_mod='PRIVILEGIO'
  ) usurol
  ...
  WHERE 1
        AND bpr.rol_mod='ADM'
  ORDER BY usurol.fecha DESC, usurol.hora DESC
  ```
- Fila esperada en grid:
  | Fecha | Hora | Usuario | Usuario modificado | Rol modificado | Operación | Entidad involucrada | Nombre de la entidad |
  |-------|------|---------|--------------------|----------------|-----------|----------------------|-----------------------|
  | 2024-03-20 | 18:04:55 | Laura Gómez | *(vacío)* | ADM | ALTA | PRIVILEGIO | BITUSR_MOD |

### 5.3 Validación de rango de fechas superior a dos años (entrada inválida)
- Parámetros: `FechaIni = 01/01/2020`, `FechaFin = 31/01/2024`.
- `ButtonMuestraReporteClick` detecta que la diferencia supera dos años completos (`IncYear(FechaFin, -2)`), muestra mensaje "Entre la fecha inicial y final debe haber máximo un rango de 2 años." y evita la ejecución de `MostrarBitacora()`.
- El grid permanece sin datos y no se habilitan exportaciones hasta corregir el rango.

### 5.4 Exportación e impresión
- Tras una consulta válida, el usuario puede:
  - Invocar `Imprimir` desde el menú contextual para obtener un reporte paginado con títulos "Bitácora de modificaciones de privilegios" y periodo `FechaIni` / `FechaFin`. La orientación es vertical y los anchos se fijan conforme a `ExportadorDatos::AsignaAnchoColumna`.
  - Seleccionar `Exportar a Excel`, lo que dispara el asistente de `ExportadorDatos::EnviaExcel()` ofreciendo las modalidades de exportación soportadas (CSV, Excel con/ sin formato, manejadas internamente por `ClassExportadorDatos`).

---

Este documento recoge la implementación vigente en el cliente legado, describiendo la interacción entre la interfaz VCL, los servicios de privilegios y las tablas de base de datos subyacentes para la bitácora de modificaciones de privilegios de usuarios y roles.