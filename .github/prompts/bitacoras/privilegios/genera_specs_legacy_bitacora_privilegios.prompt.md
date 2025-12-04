Genera un documento en formato Markdown con el nombre y path:
	.\docs\spec-legacy-bitacora-privilegios.md
	
El objetivo es describir con el máximo detalle técnico posible la implementación actual del módulo de bitácora de modificaciones de privilegios en el sistema legado, de forma que un programador pueda entender exactamente cómo funciona el código existente y reprogramarlo fielmente en otra tecnología. 

El documento no debe incluir sugerencias de mejora, migración o modernización, sino describir con precisión el comportamiento actual y todos los elementos que lo soportan.

Instrucciones específicas:

0. Explora el código fuente de la interfaz legada en los archivos:
	- FormBitacoraModPrivilegios.cpp
	- FormBitacoraModPrivilegios.h
	- FormBitacoraModPrivilegios.dfm
  Para identificar:
	- La estructura completa del formulario (componentes, eventos, bindings).
	- El flujo de ejecución desde la acción del usuario hasta la obtención y despliegue de resultados.
	- Las funciones o métodos internos que gestionan la lógica del catálogo, filtrado, carga y visualización, incluyendo `Abrir`, `FormShow`, `BlanqueaFormulario`, `MostrarBitacora`, validaciones de fechas, flujos de exportación (`Imprimir1Click`, `ExportaraExcel1Click`) y la inicialización de combos.
1. Documenta las consultas SQL construidas en el cliente:
	- Describe las sentencias que generan los combos de usuarios y roles, así como la consulta principal armada en `MostrarBitacora`.
	- Explica cada condición dinámica (`condicionUsuario`, `condicionRol`, `condicionTipo`, `condicionEntidad`) y cuándo se adjunta al query.
	- Detalla cómo se mapearon los resultados a los componentes del `VTStringGrid` (orden de columnas, encabezados, tipos) y qué listas se cargan de forma estática (`ComboBoxTipo`, `ComboBoxEntidad`).
2. Revisa las tablas y vistas relevantes en .\db\*.sql, al menos:
	- `bitacoramodprivusu.sql`
	- `bitacoramodprivrol.sql`
	- `asignacionprivilegios.sql`
	- `asignacionprivrol.sql`
	- `privilegios.sql`
	- Cualquier otra tabla enlazada mediante claves foráneas o campos referenciados en las consultas (por ejemplo `usuarios.sql`, `empleados.sql`, `rolessistema.sql`).
  Para cada tabla citada en el documento, detalla campos, llaves, relaciones y su papel dentro del módulo.
3. Utiliza como contexto adicional:
	- La documentación del código legado en los archivos .\docs\claseslegadas\*.md (por ejemplo `ClassClienteVioleta.md`, `ClassPrivilegiosDeObjeto.md`, `ClassControladorInterfaz.md`, `ClassFuncionesGenericas.md`, `ClassFuncionesClienteBd.md`, `VTStringGrid.md`, `ClassExportadorDatos.md`).
	- Describe todas las dependencias relevantes que participan, incluyendo manejo de privilegios (`mPrivilegios`), formato de fechas (`mFg`), carga de combos (`gClienteVioleta->Interfaz`) y exportaciones (`ExportadorDatos`).


Contenido esperado en el documento:


1. Introducción técnica general

- Descripción del propósito del módulo de bitácora de modificaciones de privilegios de usuarios dentro del sistema.
- Arquitectura funcional actual (interfaz, capa lógica, capa de datos).
- Diagrama o flujo de ejecución entre cliente (incluye la interfaz y las consultas que se realizan en el cliente).

2. Estructura de la interfaz (Form)

- Componentes principales definidos en el .dfm (PageControl, Tabs, Grids, Campos de entrada, Botones, etc.).
- Eventos relevantes y funciones asociadas (OnClick, OnChange, OnKeyUp, etc.).
- Variables o estructuras internas utilizadas para almacenar filtros o resultados.


3. Lógica de negocios de cada función (ubicadas en FormBitacoraModPrivilegios.cpp)

- Descripción paso a paso de cómo se procesa cada función de la bitácora (apertura del formulario, inicialización de filtros, validaciones, generación del reporte, exportación):
	- Recepción de parámetros desde el cliente.
	- Construcción dinámica del query.
	- Ejecución en base de datos.
	- Transformación de resultados a estructuras de salida (celdas del grid, títulos, exportación).
	- Validaciones previas (rangos de fechas, privilegios, visibilidad del grid) y manejo de mensajes.
- Detalle de validaciones, conversiones de tipos, manejo de valores nulos o vacíos.
- Identificación de dependencias con otras funciones o módulos.

4. Estructura de datos y relaciones

- Cómo se relacionan los datos generados por el backend con los componentes de la interfaz gráfica.
- Tablas involucradas (nombre exacto del archivo .sql).
- Para cada tabla:
	- Campos con nombre, tipo, longitud, restricciones, índices.
	- Claves primarias y foráneas.
	- Relación con otras tablas (1:N, N:N, etc.).
- Diagramas o descripciones relacionales relevantes para entender los joins del módulo.

5. Ejemplos de ejecución

- Escenarios completos de uso del módulo (por ejemplo filtros por usuario, por rol, por tipo, combinaciones de filtros y rangos de fechas válidos/ inválidos) con los parámetros reales y la consulta SQL resultante.
- Ejemplo del resultado devuelto (estructura tabular tal como se despliega en `StringGridReporte`) indicando la correspondencia con los campos mostrados en la interfaz y las acciones de exportación disponibles.

Estilo del documento:

- Claro, exhaustivo y puramente técnico.
- Se deben incluir nombres exactos de funciones, clases, variables, tablas y campos.
- Sin lenguaje orientado a procesos o usuarios finales.
- Sin sugerencias de mejora, solo descripción del estado actual del sistema.
