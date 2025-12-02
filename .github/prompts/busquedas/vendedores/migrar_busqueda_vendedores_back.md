Quiero que realices la migración de una funcionalidad legada desde C++ Builder hacia Java Spring Boot con un endpoint REST.

Contexto:

- La interfaz de usuario está en FormBusquedaVendedores.cpp, FormBusquedaVendedores.h y FormBusquedaVendedores.dfm,
	tómalo como referencia para establecer la forma en que se usa el backend legado que debe ser similar en el API migrada que se genere.
- El backend legado está en la función ServidorBusquedas::BuscaVendedores (de C++ ubicada en ClassServidorBusquedas.cpp).

Reglas y lineamientos:

- Sigue las reglas de migración definidas en los documentos siguientes: .\github\backend-general.md, .\github\backend-testing.md
- Usa como contexto adicional:
	- Documentación técnica de la funcionalidad legada: .\docs\spec-legacy-busqueda-vendedores.md
	- Definición de tablas SQL en .\db\*.sql, tomando en cuenta que la tabla central es
		vendedores.sql y analizando las relacionadas a través de sus llaves foráneas.

Consideraciones:

- Este módulo es de tipo búsquedas, con las siguientes características:
	1. Tiene un TPageControl que organiza las búsquedas en páginas (TTabSheet), cada una con un conjunto de filtros y un botón de ejecución.
	2. El grid principal de resultados es un VTStringGrid (no se utilizan grids secundarios en este formulario específico).
- La migración debe incluir:
	- Creación de un endpoint REST en Spring Boot que ejecute la lógica de búsqueda de vendedores (ruta recomendada: `/api/v1/busqueda/vendedores`).
	- Agregar una opción en el menú de la aplicación para acceder al nuevo módulo de búsqueda de vendedores.
	- Documentar lo necesario para cumplir con la especificación OpenAPI 3 (Swagger).
- Si no encuentras alguno de los archivos de contexto mencionados, detén la generación y notifícalo, ya que son muy importantes para la migración.
- Asegúrate de mantener un enfoque modular, claro y extensible, de forma consistente con lo ya hecho en otra búsqueda que ya se migró como es búsqueda de proveedores (/api/v1/busqueda/proveedores)
- Se debe diseñar pruebas de API por cada una de las formas de búsqueda, se puede usar el vendedor con clave 'CAMT' de nombre 'CARLOS MAGOS TAPIA' el cual ya existe en la base de datos configurada con el proyecto (campo vendedores.empleado='CAMT')