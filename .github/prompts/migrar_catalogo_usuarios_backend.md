Quiero que realices la migración de una funcionalidad de backend legada desde C++ Builder hacia Java Spring Boot con un endpoint REST.

Contexto:

- La interfaz de usuario está en FormCatalogoUsuarios.cpp, FormCatalogoUsuarios.h y FormCatalogoUsuarios.dfm,
	tómalo como referencia para establecer la forma en que se usa el backend legado que debe ser similar en el API migrada que se genere.
- Las funciones que serán migradas se encuentran en el archivo ServidorCatalogos, y son:
	- ServidorCatalogos::ConsultaUsuario
	- ServidorCatalogos::GrabaUsuarios
	- ServidorCatalogos::BajaUsuario

Reglas y lineamientos:

- Sigue las reglas de migración definidas en los documentos siguientes: .\github\backend-general.md, .\github\backend-testing.md
- Usa como contexto adicional:
	- Documentación técnica de la funcionalidad legada: \docs\spec-legacy-catalogo-usuarios.md
	- Definición de tablas SQL en .\db\*.sql, tomando en cuenta que la tabla central es
		usuarios.sql y analizando las relacionadas a través de sus llaves foráneas.

Consideraciones:

- Este módulo es de tipo catálogo, con las siguientes características:
- La migración debe incluir:
	- Creación de un endpoint REST en Spring Boot que ejecute la lógica de cada función utilizando los diferentes métodos HTTP según su caso(ruta recomendada: `/api/v1/usuarios`).
	- Documentar lo necesario para cumplir con la especificación OpenAPI 3 (Swagger).
- Si no encuentras alguno de los archivos de contexto mencionados, detén la generación y notifícalo, ya que son muy importantes para la migración.
- Asegúrate de mantener un enfoque modular, claro y extensible, de forma consistente con lo ya hecho en otra búsqueda que ya se migró como es búsqueda de proveedores (/api/v1/busqueda/proveedores)
- Se debe diseñar pruebas de API por cada una de las formas de búsqueda, se puede usar el usuario con clave 'CRCP' el cual ya existe en la base de datos configurada con el proyecto (campo usuarios.clave='CRCP')