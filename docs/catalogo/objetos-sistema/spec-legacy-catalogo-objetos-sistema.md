# Especificación Técnica Legacy: Catálogo de Objetos del Sistema

## 1. Introducción
El módulo de **Catálogo de Objetos del Sistema** tiene como finalidad la administración de los "objetos" que componen la aplicación. Un objeto del sistema representa típicamente una pantalla, un módulo o una funcionalidad específica sobre la cual se pueden asignar permisos. Este catálogo permite registrar estos objetos y definir sus propiedades básicas, así como generar automáticamente los privilegios estándar asociados.

## 2. Pantallas
La interacción con el usuario se realiza a través del formulario `FormCatalogoObjetosSistema` (archivos `FormCatalogoObjetosSistema.cpp`, `.h`, `.dfm`).

### 2.1. Elementos de la Interfaz
- **Campos de Texto**:
  - `dbeObjeto`: Para capturar la clave única del objeto.
  - `dbeNombre`: Para capturar el nombre descriptivo del objeto.
- **Listas de Selección (ComboBox/Lookup)**:
  - `dblcGrupo`: Para seleccionar el grupo al que pertenece el objeto (basado en `gruposobjetos`).
- **Grids**:
  - Un grid principal para listar los objetos existentes.
  - Un grid secundario para mostrar los privilegios asociados al objeto seleccionado.
- **Botones de Acción**:
  - Botones estándar para **Agregar**, **Modificar**, **Borrar** y **Guardar**.

### 2.2. Comportamiento
- Al seleccionar un objeto del grid, se cargan sus detalles en los campos de edición y se listan sus privilegios.
- La comunicación con el servidor se realiza mediante identificadores de comando:
  - `ID_CON_OBJETOSISTEMA`: Para consultar datos.
  - `ID_GRA_OBJETOSISTEMA`: Para guardar (altas y modificaciones).
  - `ID_BAJ_OBJETOSISTEMA`: Para eliminar.

## 3. Modelo de Datos
El módulo interactúa principalmente con las siguientes tablas de la base de datos MySQL:

### 3.1. Tabla `objetossistema`
Almacena la definición de los objetos.
| Campo | Tipo | Descripción |
|-------|------|-------------|
| `objeto` | VARCHAR | Clave primaria. Identificador único del objeto. |
| `nombre` | VARCHAR | Nombre descriptivo del objeto. |
| `grupo` | VARCHAR | Clave del grupo al que pertenece. |

### 3.2. Tabla `privilegios`
Almacena los permisos específicos disponibles para cada objeto.
| Campo | Tipo | Descripción |
|-------|------|-------------|
| `objeto` | VARCHAR | Clave foránea hacia `objetossistema`. |
| `privilegio` | VARCHAR | Código del privilegio (ej. CON, ALT, BAJ, MOD). |
| `descripcion` | VARCHAR | Descripción legible del privilegio. |

### 3.3. Tablas Relacionadas
- `gruposobjetos`: Catálogo de grupos para clasificar los objetos.
- `asignacionprivilegios`: Tabla donde se vinculan los privilegios de los objetos con los usuarios o roles (utilizada durante la baja para mantener integridad).

## 4. Lógica de Negocio
La lógica reside en la clase `ServidorCatalogos` (archivo `ClassServidorCatalogos.cpp`), específicamente en los métodos `ConsultaObjetoSistema`, `GrabaObjetoSistema` y `BajaObjetoSistema`.

### 4.1. Consulta (`ConsultaObjetoSistema`)
Recibe la clave del objeto a consultar.
1. **Obtiene datos del objeto**: Ejecuta `SELECT * FROM objetossistema WHERE objeto = ...`.
2. **Lista todos los objetos**: Ejecuta `SELECT ... FROM objetossistema ORDER BY objeto, grupo, nombre` para poblar el grid de navegación.
3. **Lista grupos**: Obtiene el catálogo de `gruposobjetos` para llenar el combo de selección.
4. **Lista privilegios**: Obtiene los privilegios asociados al objeto (`SELECT ... FROM privilegios WHERE objeto = ...`).

### 4.2. Alta y Modificación (`GrabaObjetoSistema`)
Recibe los parámetros del formulario y una bandera de tarea (`A` para Alta, `M` para Modificación).

- **Alta (`A`)**:
  1. Inserta el registro en `objetossistema`.
  2. **Generación Automática de Privilegios**: Inserta automáticamente 4 privilegios estándar en la tabla `privilegios`:
     - `CON`: CONSULTAS
     - `MOD`: MODIFICACIONES
     - `ALT`: ALTAS
     - `BAJ`: BAJAS
- **Modificación (`M`)**:
  1. Actualiza los campos `nombre` y `grupo` en la tabla `objetossistema` para la clave especificada.

### 4.3. Baja (`BajaObjetoSistema`)
Recibe la clave del objeto a eliminar. Realiza una limpieza en cascada manual:
1. **Elimina asignaciones**: Borra registros en `asignacionprivilegios` asociados al objeto.
2. **Elimina privilegios**: Borra registros en `privilegios` asociados al objeto.
3. **Elimina objeto**: Borra el registro en `objetossistema`.

Todas las operaciones de escritura se agrupan en un buffer de transacciones para asegurar la atomicidad (aunque en el código analizado no se observó un `START TRANSACTION` explícito en este método específico, se envían como un lote).
