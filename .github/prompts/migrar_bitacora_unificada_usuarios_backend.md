Quiero que realices la migración de una funcionalidad de backend legada desde C++ Builder hacia Java Spring Boot con un endpoint REST.

Contexto:

- La interfaz de usuario está en FormBitacoraUnificada.cpp, FormBitacoraUnificada.h, FormBitacoraUnificada.dfm,
	tómalo como referencia para establecer la forma en que se usa el backend legado que debe ser similar en el API migrada que se genere.
- Las funciones que serán migradas se encuentran en el archivo ServidorAdminSistema, y son:
	- ServidorAdminSistema::ConsultaBitacoraUnificada

Reglas y lineamientos:

- Sigue las reglas de migración definidas en los documentos siguientes: .\github\backend-general.md, .\github\backend-testing.md
- Usa como contexto adicional:
	- Documentación técnica de la funcionalidad legada: \docs\spec-legacy-bitacora-unificada-usuarios.md
	- Definición de tablas SQL en .\db\*.sql, tomando en cuenta que las tablas usadas y analizando las relacionadas a través de sus llaves foráneas.

Consideraciones:

- Este módulo es de tipo bitácora, con las siguientes características:
- La migración debe incluir:
	- Creación de un endpoint REST en Spring Boot que ejecute la lógica de cada función utilizando los diferentes métodos HTTP según su caso(ruta recomendada: `/api/v1/bitacora-unificada`).
	- Documentar lo necesario para cumplir con la especificación OpenAPI 3 (Swagger).
- Si no encuentras alguno de los archivos de contexto mencionados, detén la generación y notifícalo, ya que son muy importantes para la migración.
- Asegúrate de mantener un enfoque modular, claro y extensible, de forma consistente con lo ya hecho 
- Expone un endpoint que reciba explícitamente `fechaInicio`, `fechaFin`, sucursal (no es obligatorio que se llene, puede ir vacia) y el usuario a consultar, alineado a la firma actual del backend legado.
- Se debe diseñar pruebas de API cubriendo al menos: rango válido con datos, rango válido sin coincidencias, y validación de parámetros obligatorios.