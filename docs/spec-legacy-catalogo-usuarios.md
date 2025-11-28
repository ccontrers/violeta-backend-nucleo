# Especificación técnica del catálogo legado de usuarios

## 1. Introducción técnica general
- **Propósito**: el formulario `cpp/FormCatalogoUsuarios.{cpp,h,dfm}` permite administrar el catálogo de usuarios de la plataforma de escritorio. Opera sobre la tabla `usuarios` y refleja el estado de los empleados que tienen acceso al sistema. Soporta consulta, altas, bajas y modificaciones básicas (activo/fechas/passwords asociados a sistemas externos).
- **Arquitectura vigente**:
  - **Cliente VCL**: formulario `TFormCatUsuarios`, controles personalizados VT*, `FrameBarraNavegacion` para navegación CRUD y utilidades `ClassControladorInterfaz`, `ClassPrivilegiosDeObjeto`, `ClassFuncionesGenericas`.
  - **Capa de transporte**: `ClassClienteVioleta` serializa peticiones (`ID_CON_USUARIO`, `ID_GRA_USUARIO`, `ID_BAJ_USUARIO`) y consume resultados `BufferRespuestas`.
  - **Servidor**: clase monolítica `ServidorCatalogos` en `cpp/ClassServidorCatalogos.cpp` atiende los IDs anteriores con métodos `ConsultaUsuario`, `GrabaUsuarios`/`GrabaUsuario` y `BajaUsuario` sobre MySQL.
- **Flujo resumido**:
  1. El usuario interactúa con el formulario (búsqueda, selección en grid, botones del frame).
  2. El cliente arma el buffer de parámetros en `gClienteVioleta` y lanza la petición.
  3. `ServidorCatalogos` deserializa, ejecuta SQL y empaqueta hasta tres result sets consecutivos.
  4. El cliente crea uno o varios `BufferRespuestas` para poblar controles (grid, combo, controlador de datos).
- **Consideraciones**: no existe capa de validación adicional; el formulario confía en privilegios `SISUSU` para habilitar modificación. Las transacciones solo están presentes en altas/modificaciones, sin `ROLLBACK` ante fallo intermedio.

## 2. Estructura de la interfaz (Form)
### 2.1 Componentes declarados
| Componente | Tipo | Propósito / binding |
|------------|------|----------------------|
| `VTStringGrid *StringGridUsuarios` | Grid de cuatro columnas (Clave, Nombre, Último acceso, Activo). Usa `PopupMenuExportarUsuarios` para imprimir o exportar. El evento `OnDrawCell` pinta íconos de estado activo usando `ImageListIconos`. |
| `TFrameNavegacion *FrameNav` | Encapsula botones Agregar/Quitar/Grabar/Cancelar/Buscar/Modificar. Los manejadores en `FormCatalogoUsuarios.cpp` delegan en el frame y ajustan el estado del `ControladorInterfaz`. |
| `VTComboBox *ComboBoxEmpleado` | Lista de empleados disponibles (llena con el tercer result set de `ConsultaUsuario`). Asociada al campo `usuarios.empleado`; deshabilitada durante edición para evitar cambio de clave. Tecla F5 abre `FormCatalogoEmpleados`. |
| `VTDateTimePicker *DtpFechaAlta`, `*DtpFechaBaja` | Se inicializan con fecha actual (`mFg.DateToAnsiString(gClienteVioleta->ObtieneFechaDeHoy())`) y 01/01/2099 respectivamente y están vinculados a `usuarios.fechaalta` y `usuarios.fechabaja`. |
| `VTCheckBox *CheckBoxActivo` | Enlazado a `usuarios.activo`. `CheckBoxMostrarInactivos` controla el filtro enviado al backend (`activos = "AND u.activo=1 "` o cadena vacía). |
| `VTLabeledEdit *EditBuscarUsuario` | Campo de búsqueda incremental; a partir de 2 caracteres invoca `CargaUsuario(" ")` para refrescar con filtro parcial en nombres/apellidos. |
| `VTBitBtn *ButtonAsignarClave` | Abre `FormSistemaAsignarClave` para redefinir contraseñas siempre que exista un usuario seleccionado y persistido. |
| `TPopupMenu *PopupMenuExportarUsuarios` + `ImprimirUsuarios`, `ExportaraExcelUsuarios` | Usan `ClassExportadorDatos` para exportar el grid a impresora o Excel. |
| `ControladorInterfaz mControlador` | En el constructor se registran bindings (`Inserta` asociando controles VT con `usuarios`). Provee validación (`Valida`), respaldo/restore de datos (`Respalda`, `Restaura`) y llenado desde `BufferRespuestas`. |

### 2.2 Eventos relevantes
- `FormShow` → `Inicializa()`: ejecuta consulta sin filtros, deshabilita edición y prepara el frame.
- `StringGridUsuariosClick` carga el registro de la fila seleccionada (si la clave difiere de la actual).
- `FrameNavButtonAgregarClick` bloquea grid/búsqueda, habilita controles, limpia datos y marca tarea activa `tfnAgregando`.
- `FrameNavButtonGrabarClick` llama `GrabaUsuario()` y, si el backend responde OK, vuelve a cargar el usuario recién editado, restaura el respaldo y re habilita UI.
- `FrameNavButtonQuitarClick` confirma con mensaje y ejecuta `BorraUsuario` (ID_BAJ_USUARIO). Maneja errores 1451/1452 (`ER_ROW_IS_REFERENCED*`) mostrando instrucción al usuario.
- `FrameNavButtonCancelarClick` revierte mediante `mControlador.Restaura()` si era modificación o reinicia si era alta.
- `CheckBoxMostrarInactivosClick` actualiza la variable `activos` y vuelve a consultar.
- `EditBuscarUsuarioChange` dispara nueva consulta cuando el texto tiene 0 o 2+ caracteres (minimiza spam de peticiones).
- `ImprimirUsuariosClick`/`ExportaraExcelUsuariosClick` usan `ExportadorDatos` con configuración fija de tipografía, márgenes y alineaciones.

### 2.3 Variables y estructuras cliente
- `AnsiString activos`: contiene el fragmento SQL enviado como segundo parámetro del ID de consulta. Valores posibles: `"AND u.activo=1 "` (default) o `" "` (cuando se solicitan inactivos).
- `AnsiString mUsuario`: almacena el último usuario abierto; reutilizado al cerrar el formulario para restablecer selección en combos externos (`LlenaVTComboBox`).
- `PrivilegiosDeObjeto mPrivilegios`: consulta `SISUSU`, determina si el frame permite consultar/modificar y si `ButtonAsignarClave` queda bloqueado (requiere privilegio `"PAS"`).
- `ControladorInterfaz mControlador`: coordina la carga desde `BufferRespuestas resp_usuario` y aplica `AsignaSoloLectura(true)` cuando `mPermitidoModificar` es falso.

## 3. Lógica de negocios en ServidorCatalogos
### 3.1 Protocolo de peticiones
| ID | Parámetros (en orden) | Descripción |
|----|-----------------------|-------------|
| `ID_CON_USUARIO` | 1) `clave_usuario` (puede ser `""` o `" "`); 2) cadena de condición `activos` (`"AND u.activo=1 "` o `" "`); 3) `filtrousuario` (espacio simple sin filtro o texto en mayúsculas). | Devuelve tres `SELECT` concatenados. |
| `ID_GRA_USUARIO` | 1) `tarea` (`"A"` para alta, `"M"` para modificación); 2) `clave` (empleado); 3) campos serializados de `usuarios` en el orden definido en metadatos de `DatosTabla`. | Ejecuta `INSERT` o `UPDATE` dentro de transacción e inserta la relación en `usuariosucursal` durante altas. |
| `ID_BAJ_USUARIO` | 1) `clave` del empleado. | Ejecuta `DELETE FROM usuarios WHERE empleado='<clave>'`. |

Los buffers no incluyen contadores explícitos; el servidor consume secuencialmente usando `FuncionesGenericas::ExtraeStringDeBuffer`.

### 3.2 Consulta (`ConsultaUsuario`)
```sql
SELECT * FROM usuarios WHERE empleado = '<clave>';
SELECT u.empleado AS Usuario,
       CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS nombre,
       (SELECT b.fecha FROM bitacorausuario b WHERE u.empleado=b.usuario ORDER BY fecha DESC LIMIT 1) AS fecha,
       u.activo
FROM usuarios u
JOIN empleados e ON e.empleado = u.empleado
WHERE TRUE <activos><condicion_filtrousuario>
ORDER BY nombre;
SELECT empleado AS Empleado,
       CONCAT(nombre,' ',appat,' ',apmat) AS Nombre
FROM empleados
ORDER BY nombre;
```
- `condicion_filtrousuario` se arma con `LIKE` sobre nombre y apellidos cuando `filtrousuario != " "`.
- El primer result set puede venir vacío (cuando `clave` no se envía o el usuario no existe); el cliente maneja esto cargando defaults en `mControlador`.
- No hay paginación; el grid siempre recibe todos los usuarios filtrados.

### 3.3 Alta / modificación (`GrabaUsuarios` → `GrabaUsuario`)
- Envuelve operaciones en `SET AUTOCOMMIT=0; START TRANSACTION; ...; COMMIT;`.
- `DatosTabla` consume los campos esperados por la definición de `usuarios`. Para altas genera `INSERT` completo. Para modificaciones genera `UPDATE usuarios SET ... WHERE empleado='<clave>'` respetando el tipo de dato de la clave (`varchar` → comillas).
- Después del `INSERT` se ejecuta `INSERT IGNORE INTO usuariosucursal(usuario, sucursal) VALUES('<clave>', '<sucursalServidor>')` usando la sucursal activa de `FormServidor`. Esto garantiza pertenencia mínima del usuario a la sucursal actual.
- No se recalculan privilegios ni se validan dependencias en otras tablas; el formulario asume que la clave de empleado ya existe en `empleados`.

### 3.4 Baja (`BajaUsuario`)
- Delegada a `BajaGenerico`, que elimina el registro por clave.
- No se remueven filas de `usuariosucursal` ni de referencias secundarias (`bitacorausuario`, privilegios). Si la FK `usuariosucursal.usuario` impide el borrado (ON DELETE RESTRICT), el servidor propaga error 1451, capturado en `TFormCatUsuarios::BorraUsuario` para mostrar mensaje “Imposible borrar...”.

### 3.5 Transformación y mapeo cliente
- Cada `SELECT` regresa en un bloque `BufferRespuestas`. `TFormCatUsuarios::CargaUsuario` los consume secuencialmente:
  1. `resp_usuario`: rellena `mControlador` para campos vinculados a `usuarios` y establece flags de modificación.
  2. `resp_usuarios`: `LlenaStringGrid(StringGridUsuarios, true, false)` para mostrar lista completa.
  3. `resp_empleados`: `LlenaComboBox(ComboBoxEmpleado)` para actualizar el origen del combo.
- Tras una carga exitosa, se habilitan los botones Quitar/Grabar/Cancelar y, si el usuario carece de privilegio de modificación, el controlador queda en solo lectura.

## 4. Estructura de datos y relaciones
### 4.1 Tablas principales
| Tabla | Archivo | Campos relevantes |
|-------|---------|-------------------|
| `usuarios` | `db/usuarios.sql` | `empleado (PK, FK->empleados)`, `password`, `activo` (`tinyint`), `fechaalta`, `fechabaja`, `usuariocontpaq`, `passwordcontpaq`. |
| `empleados` | `db/empleados.sql` | Catálogo maestro: `empleado (PK)`, `nombre/appat/apmat`, `puesto`, `depart`, `sucursal`, `activo`, etc. Se usa para poblar combos y para mostrar nombre completo en el grid. |
| `bitacorausuario` | `db/bitacorausuario.sql` | Historial de accesos (`usuario`, `fecha`, `hora`, `terminal`). El subquery de `ConsultaUsuario` obtiene la última `fecha`. |
| `usuariosucursal` | `db/usuariosucursal.sql` | Relación N:M usuarios ↔ sucursales. Se inserta automáticamente al dar alta un usuario asociándolo a la sucursal del servidor. |

### 4.2 Relaciones y cardinalidades
- `usuarios.empleado` **1:1** `empleados.empleado` (obligatorio; FK ON UPDATE CASCADE, ON DELETE por defecto RESTRICT).
- `usuarios -> bitacorausuario`: **1:N** (un usuario puede tener múltiples accesos). Solo se lee el último registro vía subconsulta.
- `usuarios -> usuariosucursal`: **1:N** (un usuario puede pertenecer a múltiples sucursales). El alta garantiza al menos la sucursal actual. No existen operaciones para gestionar sucursales adicionales desde este formulario.

### 4.3 Componentes de interfaz y datos
- `ControladorInterfaz` enlaza `ComboBoxEmpleado`, `DtpFechaAlta`, `DtpFechaBaja`, `CheckBoxActivo` y etiquetas auxiliares para mostrar/editar campos de `usuarios`.
- `StringGridUsuarios` refleja `resp_usuarios`. Columna 3 (`Activo`) no muestra texto; `StringGridUsuariosDrawCell` dibuja un ícono verde o rojo según el valor (1/0) para mejorar lectura.
- `ButtonAsignarClave` no modifica la tabla `usuarios`; únicamente abre `FormSistemaAsignarClave`, el cual maneja internamente asignación de contraseñas (fuera del alcance de este documento).

## 5. Ejemplos de ejecución
### 5.1 Consulta con filtro parcial
1. `CheckBoxMostrarInactivos` desmarcado ⇒ `activos="AND u.activo=1 "`.
2. Usuario escribe “LOPE” en `EditBuscarUsuario`; al haber ≥2 caracteres se invoca:
   ```cpp
   gClienteVioleta->InicializaPeticion(ID_CON_USUARIO);
   gClienteVioleta->AgregaStringAParametros(" ");      // clave vacía
   gClienteVioleta->AgregaStringAParametros("AND u.activo=1 ");
   gClienteVioleta->AgregaStringAParametros("LOPE");    // filtro
   ```
3. `ServidorCatalogos::ConsultaUsuario` agrega `condicion_filtrousuario` = `AND ( e.nombre LIKE '%LOPE%' OR e.appat LIKE '%LOPE%' OR e.apmat LIKE '%LOPE%' )`.
4. `resp_usuarios` contiene solo coincidencias; `StringGridUsuarios` muestra usuarios activos cuyos nombres contienen “LOPE”. La columna “Último acceso” refleja la fecha devuelta por `bitacorausuario` o `NULL`.

### 5.2 Alta de usuario
1. Operador con privilegio `mPrivilegios.mPermitidoModificar = true` presiona “Agregar” en `FrameNav`.
2. Completa selección de empleado (ComboBox), fechas y marca “Activo”.
3. `FrameNavButtonGrabarClick` llama `GrabaUsuario()` que arma el buffer:
   ```cpp
   gClienteVioleta->InicializaPeticion(ID_GRA_USUARIO);
   gClienteVioleta->AgregaStringAParametros("A");                 // tarea alta
   gClienteVioleta->AgregaStringAParametros(<empleado>);
   // ControladorInterfaz serializa campos: activo, fechaalta, fechabaja, usuariocontpaq, passwordcontpaq...
   ```
4. En el servidor, `GrabaUsuario` inserta en `usuarios` y `usuariosucursal` (sucursal actual). Si la clave ya existe en `usuarios`, MySQL lanza error 1062 y el cliente muestra “Clave duplicada, se debe modificar”.
5. Al éxito, `CargaUsuario(<empleado>)` refresca la UI con el registro ya persistido y el grid vuelve a habilitarse.

### 5.3 Baja de usuario con restricciones
1. Operador selecciona usuario desde el grid y presiona “Quitar”.
2. Después de confirmar, el cliente envía `ID_BAJ_USUARIO` con la clave.
3. Si existen registros en `usuariosucursal` u otras tablas con FK `RESTRICT`, MySQL emite error `ER_ROW_IS_REFERENCED`. El manejo actual muestra “Imposible borrar, elimine los registros de otras tablas que hagan referencia a este usuario” y el registro permanece.
4. Si no hay referencias, el `DELETE` se ejecuta, `Inicializa()` repuebla el grid y se muestra mensaje “Usuario borrado!”.

### 5.4 Exportación / impresión
- Las opciones del menú contextual crean un `ExportadorDatos` basado en `StringGridUsuarios`. Para impresión se fijan márgenes (0.5" izq/der, 1" sup/inf), fuente “Arial Narrow” 9pt y orientación horizontal. Para Excel se llama `exportador->EnviaExcel()` sin parámetros adicionales. En ambos casos se exporta exactamente lo que está en el grid (respeta filtros y ordenaciones actuales).

---
Documento generado para describir fielmente el estado actual del módulo legado; no incluye propuestas de mejora ni cambios de comportamiento.
