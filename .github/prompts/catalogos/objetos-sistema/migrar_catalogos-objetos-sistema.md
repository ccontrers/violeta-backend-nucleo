Quiero que realices la migración de una funcionalidad de backend legada desde C++ Builder hacia Java Spring Boot con un endpoint REST.

Contexto:
- La interfaz de usuario está en los archivos:
	- `cpp/FormCatalogoObjetosSistema.cpp`
	- `cpp/FormCatalogoObjetosSistema.h`
	- `cpp/FormCatalogoObjetosSistema.dfm`
	- `cpp/ClassServidorCatalogos.cpp`
	- `cpp/ClassServidorCatalogos.h`
	tómalo como referencia para establecer la forma en que se usa el backend legado que debe ser similar en el API migrada que se genere.
- Las funciones que serán migradas se encuentran mencionadas en la sección `Lógica de negocios` del documento  `docs/catalogo/objetos-sistema/spec-legacy-catalogo-objetos-sistema.md`

Reglas y lineamientos:

- Sigue las reglas de migración definidas en los documentos siguientes: `.github/backend-general.md`, `.github/backend-testing.md`
- Usa como contexto adicional:
	- Documentación técnica de la funcionalidad legada: `docs/catalogo/objetos-sistema/spec-legacy-catalogo-objetos-sistema.md`
	- Definición de tablas SQL en `db/*.sql`, tomando en cuenta que la tabla central es
		`objetossistema.sql` y analizando las relacionadas a través de sus llaves foráneas.

Consideraciones:

- Este módulo es de tipo `catalogo`, con las siguientes características:
- La migración debe incluir:
	- Creación de un endpoint REST en Spring Boot que ejecute la lógica de cada función utilizando los diferentes métodos HTTP según su caso (ruta recomendada: `/api/v1/objetos-sistema`).
	- Documentar lo necesario para cumplir con la especificación OpenAPI 3 (Swagger).
- Si no encuentras alguno de los archivos de contexto mencionados, detén la generación y notifícalo, ya que son muy importantes para la migración.
- Asegúrate de mantener un enfoque modular, claro y extensible, de forma consistente con los estándares del proyecto.
- Se debe diseñar pruebas de API para probar que los endpoints generados funcionan correctamente:
	- Pruebas que demuestren la funcionalidad principal de cada endpoint.
	- Pruebas que validen el manejo de errores y casos límite.
	- Usa datos de prueba relevantes para el módulo, si es posible.