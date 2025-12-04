Quiero que realices la migración de una funcionalidad de backend legada desde C++ Builder hacia Java Spring Boot con un endpoint REST.

Contexto:

- La interfaz de usuario está en FormBitacoraModPrivilegios.cpp, FormBitacoraModPrivilegios.h y FormBitacoraModPrivilegios.dfm,
	tómalo como referencia para establecer la forma en que se usa el backend legado que debe ser similar en el API migrada que se genere.
- Las funciones que serán migradas se encuentran en el archivo FormBitacoraModPrivilegios.cpp, y son:
	- MostrarBitacora
	- Llenado de los combo box que se utilizan en la parte legada

Reglas y lineamientos:

- Sigue las reglas de migración definidas en los documentos siguientes: .\.github\backend-general.md, .\.github\backend-testing.md
- Usa como contexto adicional:
	- Documentación técnica de la funcionalidad legada: docs\bitacoras\privilegios\spec-legacy-bitacora-privilegios.md
	- Definición de tablas SQL en .\db\*.sql:
	- Tablas de bitácora:
		- `bitacoramodprivusu.sql`
		- `bitacoramodprivrol.sql`
	- Tablas de asignación de privilegios:
		- `asignacionprivilegios.sql`
		- `asignacionprivrol.sql`
		- `privilegios.sql`
	- Tablas auxiliares usadas en JOINs:
		- `usuarios.sql`
		- `empleados.sql`
		- `rolessistema.sql`
		- `objetossistema.sql`

Consideraciones:

- Este módulo es de tipo bitácora, con las siguientes características:
	- Solo lectura (consulta): No permite inserción, modificación ni eliminación de registros; únicamente consulta histórica.
	- Filtrado por rango de fechas obligatorio: fecha inicial ≤ fecha final, con rango máximo de 2 años.
	- Filtros opcionales: por usuario/empleado (operador o usuario modificado), por rol modificado, por tipo de contexto (`USUARIOS` o `ROLES`), por entidad involucrada (`GRUPO`, `OBJETO`, `PRIVILEGIO`, `SUCURSAL`, `ROL`, `USUARIO`).
	- Unión de dos fuentes de datos: combina registros de `bitacoramodprivusu` (cambios a usuarios) y `bitacoramodprivrol` (cambios a roles) mediante `UNION ALL`.
	- Ordenamiento descendente: los resultados se ordenan por fecha y hora (más recientes primero).
	- **Nota:** No se requiere migrar la validación de privilegios (control de acceso sobre el objeto `BITMPUR`); esto se manejará en una fase posterior o mediante otro mecanismo.
- La migración debe incluir:
	- Creación de un endpoint REST en Spring Boot usando método `POST` para la consulta principal (consistente con `/api/v1/bitacora-unificada`), ruta recomendada: `/api/v1/bitacora-privilegios`.
	- Estructura de archivos siguiendo el patrón del proyecto:
		- `src\main\java\com\lavioleta\desarrollo\violetaserver\bitacora_privilegios\controller\BitacoraPrivilegiosController.java`
		- `src\main\java\com\lavioleta\desarrollo\violetaserver\bitacora_privilegios\service\BitacoraPrivilegiosService.java`
		- `src\main\java\com\lavioleta\desarrollo\violetaserver\bitacora_privilegios\service\impl\BitacoraPrivilegiosServiceImpl.java`
		- `src\main\java\com\lavioleta\desarrollo\violetaserver\bitacora_privilegios\repository\BitacoraPrivilegiosRepository.java`
		- `src\main\java\com\lavioleta\desarrollo\violetaserver\bitacora_privilegios\dto\request\BitacoraPrivilegiosRequest.java`
		- `src\main\java\com\lavioleta\desarrollo\violetaserver\bitacora_privilegios\dto\response\BitacoraPrivilegiosResponse.java`
	- Documentar lo necesario para cumplir con la especificación OpenAPI 3 (Swagger).
	- Para cada combo box del formulario legacy (`ComboBoxEmpleado`, `ComboBoxRol`, `ComboBoxTipo`, `ComboBoxEntidad`):
		1. Verificar si ya existe un endpoint que provea los datos requeridos.
		2. Si existe, evaluar si se puede reutilizar sin afectar otros módulos que lo consuman.
		3. Si no existe o no es compatible, crear un nuevo endpoint específico para ese combo box.
		4. Documentar en el código la decisión tomada para cada caso.
- Si no encuentras alguno de los archivos de contexto mencionados, detén la generación y notifícalo, ya que son muy importantes para la migración.
- Asegúrate de mantener un enfoque modular, claro y extensible, de forma consistente con lo ya hecho en otra búsqueda que ya se migró como es bitacora unificada (/api/v1/bitacora-unificada)
- Se debe diseñar pruebas de API por cada una de las formas de búsqueda, se puede usar el usuario con clave 'CRCP' el cual ya existe en la base de datos configurada con el proyecto (campo usuarios.empleado='CRCP')