# Especificación Técnica Legacy: Catálogo de Objetos del Sistema

## 1. Introducción Técnica General
El módulo de **Catálogo de Objetos del Sistema** permite la administración de los recursos (pantallas, módulos, reportes) que componen la aplicación y sobre los cuales se pueden asignar permisos de seguridad.

### 1.1. Arquitectura Funcional
El módulo sigue una arquitectura cliente-servidor "Thick Client" utilizando C++ Builder (VCL) en el cliente y un servidor de aplicaciones propio (`ServidorVioleta`) que gestiona la conexión a MySQL.

- **Capa de Presentación (Cliente)**: `FormCatalogoObjetosSistema` gestiona la interacción, validación de entrada y visualización de datos. Utiliza componentes visuales propietarios (`VTStringGrid`, `VTLabeledEdit`, `VTComboBox`) y un controlador de interfaz (`ControladorInterfaz`) para el binding de datos.
- **Capa de Comunicación**: La clase `ClassClienteVioleta` (instancia global `gClienteVioleta`) empaqueta las peticiones con identificadores numéricos (IDs) y parámetros en un buffer binario/texto.
- **Capa de Negocio (Servidor)**: La clase `ServidorCatalogos` recibe la petición, decodifica los parámetros, construye las sentencias SQL dinámicamente y ejecuta las transacciones.
- **Capa de Datos**: Base de datos MySQL.

### 1.2. Flujo de Ejecución
1. **Inicio**: El usuario abre la pantalla. Se ejecuta `Inicializa()`, enviando una petición de consulta vacía (`ID_CON_OBJETOSISTEMA`) para llenar los grids y combos.
2. **Selección**: Al hacer clic en el grid de objetos, se envía una nueva consulta con la clave del objeto seleccionado para llenar los campos de detalle.
3. **Edición**: El usuario modifica datos o agrega uno nuevo. El `ControladorInterfaz` gestiona el estado de los controles.
4. **Persistencia**: Al guardar, se envían los datos al servidor (`ID_GRA_OBJETOSISTEMA`). El servidor ejecuta las inserciones/actualizaciones y devuelve éxito o error.

## 2. Estructura de la Interfaz (Form)
Archivo fuente: `cpp/FormCatalogoObjetosSistema.cpp` / `.h` / `.dfm`

### 2.1. Componentes Principales
| Componente | Tipo | Propósito | Binding (Tabla.Campo) |
|------------|------|-----------|-----------------------|
| `EditObjeto` | `VTLabeledEdit` | Captura la clave única del objeto. | `objetossistema.objeto` |
| `EditNombre` | `VTLabeledEdit` | Captura el nombre descriptivo. | `objetossistema.nombre` |
| `ComboBoxGrupo` | `VTComboBox` | Selección del grupo del objeto. | `objetossistema.grupo` |
| `StringGridObjetos` | `VTStringGrid` | Lista todos los objetos registrados. | N/A (Llenado manual) |
| `StringGridPrivilegios`| `VTStringGrid` | Lista los privilegios del objeto actual. | N/A (Llenado manual) |
| `FrameNav` | `TFrameNavegacion` | Botonera estándar (Agregar, Grabar, etc.). | N/A |

### 2.2. Variables y Estructuras Internas
- `mControlador` (`ControladorInterfaz`): Gestiona el mapeo entre componentes visuales y campos de base de datos, así como el estado (Solo lectura, Modificado).
- `mPrivilegios` (`PrivilegiosDeObjeto`): Almacena los permisos del usuario actual sobre este módulo ("SISOBJ").
- `mObjeto` (`AnsiString`): Almacena la clave del objeto actualmente seleccionado.

### 2.3. Eventos Relevantes
- `FormShow`: Llama a `Inicializa()` para cargar la lista inicial.
- `StringGridObjetosClick`: Invoca `CargaObjeto()` con la celda seleccionada para traer el detalle.
- `FrameNavButtonAgregarClick`: Prepara la interfaz para un nuevo registro (limpia campos, habilita edición).
- `FrameNavButtonGrabarClick`: Invoca `GrabaObjeto()`.
- `ComboBoxGrupoKeyDown` (F5): Abre el catálogo de Grupos (`FormCatGruposObjetos`) para dar de alta nuevos grupos al vuelo.
- `StringGridPrivilegiosKeyDown` (F5): Abre el catálogo de Privilegios (`FormCatPrivilegios`) para gestionar permisos específicos.

## 3. Lógica de Negocios (Backend)
Clase: `ServidorCatalogos` en `cpp/ClassServidorCatalogos.cpp`

### 3.1. Consulta (`ID_CON_OBJETOSISTEMA`)
Método: `ConsultaObjetoSistema`
**Parámetros de Entrada**: `clave_objeto` (String).

**Proceso**:
1. **Consulta Detalle**: Si se recibe una clave, busca sus datos.
   ```sql
   select * from objetossistema where objeto='<clave_objeto>'
   ```
2. **Consulta Lista General**: Obtiene todos los objetos para el grid de navegación.
   ```sql
   select objeto AS Objeto, nombre AS Nombre, grupo AS Grupo from objetossistema order by objeto, grupo, nombre
   ```
3. **Consulta Catálogo de Grupos**: Para llenar el ComboBox.
   ```sql
   select grupo, nombre from gruposobjetos
   ```
4. **Consulta Privilegios**: Obtiene los privilegios asignados al objeto.
   ```sql
   select privilegio, descripcion from privilegios where objeto='<clave_objeto>'
   ```

**Salida**: 4 ResultSets secuenciales en el buffer de respuesta.

### 3.2. Alta y Modificación (`ID_GRA_OBJETOSISTEMA`)
Método: `GrabaObjetoSistema`
**Parámetros de Entrada**:
- `tarea`: "A" (Alta) o "M" (Modificación).
- `clave`: Clave del objeto.
- Campos serializados por `DatosTabla` (nombre, grupo, etc.).

**Proceso**:
1. Se utiliza la clase `DatosTabla` para parsear los parámetros y generar el SQL dinámicamente.
2. **Si es Alta ("A")**:
   - Genera `INSERT INTO objetossistema ...`
   - **Lógica Especial**: Inserta automáticamente los 4 privilegios básicos.
     ```sql
     insert into privilegios (objeto, privilegio, descripcion) values('<clave>', 'CON', 'CONSULTAS')
     insert into privilegios (objeto, privilegio, descripcion) values('<clave>', 'MOD', 'MODIFICACIONES')
     insert into privilegios (objeto, privilegio, descripcion) values('<clave>', 'ALT', 'ALTAS')
     insert into privilegios (objeto, privilegio, descripcion) values('<clave>', 'BAJ', 'BAJAS')
     ```
3. **Si es Modificación ("M")**:
   - Genera `UPDATE objetossistema SET ... WHERE objeto='<clave>'`

**Salida**: Buffer de resultados de ejecución (éxito/error).

### 3.3. Baja (`ID_BAJ_OBJETOSISTEMA`)
Método: `BajaObjetoSistema`
**Parámetros de Entrada**: `clave` (String).

**Proceso**:
Realiza un borrado en cascada manual mediante múltiples sentencias SQL agregadas a un buffer de ejecución.
1. Elimina asignaciones de privilegios a usuarios/roles.
   ```sql
   delete from asignacionprivilegios where objeto='<clave>'
   ```
2. Elimina la definición de privilegios del objeto.
   ```sql
   delete from privilegios where objeto='<clave>'
   ```
3. Elimina el objeto.
   ```sql
   delete from objetossistema where objeto='<clave>'
   ```

## 4. Estructura de Datos y Relaciones

### 4.1. Tabla `objetossistema` (Archivo: `db/objetossistema.sql`)
Tabla principal del módulo.
| Campo | Tipo | Descripción |
|-------|------|-------------|
| `objeto` | VARCHAR(10) | **PK**. Identificador único del objeto. |
| `nombre` | VARCHAR(50) | Nombre descriptivo mostrado al usuario. |
| `grupo` | VARCHAR(10) | **FK**. Clasificación del objeto. |

### 4.2. Tabla `privilegios`
Define qué acciones se pueden realizar sobre un objeto.
| Campo | Tipo | Descripción |
|-------|------|-------------|
| `objeto` | VARCHAR(10) | **PK, FK**. Referencia a `objetossistema`. |
| `privilegio` | VARCHAR(5) | **PK**. Código (ej. CON, ALT). |
| `descripcion` | VARCHAR(50) | Descripción (ej. "CONSULTAS"). |

### 4.3. Tabla `gruposobjetos`
Catálogo auxiliar para agrupar objetos.
| Campo | Tipo | Descripción |
|-------|------|-------------|
| `grupo` | VARCHAR(10) | **PK**. Clave del grupo. |
| `nombre` | VARCHAR(50) | Nombre del grupo. |

### 4.4. Tabla `asignacionprivilegios`
Tabla de relación N:M entre usuarios/roles y privilegios de objetos.
| Campo | Tipo | Descripción |
|-------|------|-------------|
| `usuario` | VARCHAR(10) | Usuario o Rol. |
| `objeto` | VARCHAR(10) | Objeto del sistema. |
| `privilegio` | VARCHAR(5) | Privilegio asignado. |

## 5. Ejemplos de Ejecución

### Ejemplo 1: Consulta de Objeto "CLIENTES"
**Entrada (Cliente -> Servidor)**:
- ID: `ID_CON_OBJETOSISTEMA`
- Param 1: "CLIENTES"

**SQL Generado (Servidor)**:
```sql
select * from objetossistema where objeto='CLIENTES';
select objeto AS Objeto, nombre AS Nombre, grupo AS Grupo from objetossistema order by objeto, grupo, nombre;
select grupo, nombre from gruposobjetos;
select privilegio, descripcion from privilegios where objeto='CLIENTES';
```

### Ejemplo 2: Alta de Objeto "REPORTES_VTA"
**Entrada**:
- ID: `ID_GRA_OBJETOSISTEMA`
- Param 1: "A" (Tarea)
- Param 2: "REPORTES_VTA" (Clave)
- Params Restantes: Campos serializados (nombre="Reportes de Ventas", grupo="REPORTES")

**SQL Generado**:
```sql
INSERT INTO objetossistema (objeto, nombre, grupo) VALUES ('REPORTES_VTA', 'Reportes de Ventas', 'REPORTES');
insert into privilegios (objeto, privilegio, descripcion) values('REPORTES_VTA', 'CON', 'CONSULTAS');
insert into privilegios (objeto, privilegio, descripcion) values('REPORTES_VTA', 'MOD', 'MODIFICACIONES');
insert into privilegios (objeto, privilegio, descripcion) values('REPORTES_VTA', 'ALT', 'ALTAS');
insert into privilegios (objeto, privilegio, descripcion) values('REPORTES_VTA', 'BAJ', 'BAJAS');
```

### Ejemplo 3: Baja de Objeto "REPORTES_VTA"
**Entrada**:
- ID: `ID_BAJ_OBJETOSISTEMA`
- Param 1: "REPORTES_VTA"

**SQL Generado**:
```sql
delete from asignacionprivilegios where objeto='REPORTES_VTA';
delete from privilegios where objeto='REPORTES_VTA';
delete from objetossistema where objeto='REPORTES_VTA';
```
