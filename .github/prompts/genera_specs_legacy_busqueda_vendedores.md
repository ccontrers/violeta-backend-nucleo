Genera un documento técnico sobre la busqueda de vendedores del sistema legado (en c++ builder).

El objetivo es describir con el máximo detalle técnico posible la implementación actual del módulo 
de búsqueda de vendedores en el sistema legado, de forma que un programador pueda entender exactamente
cómo funciona el código existente y este pueda reprogramarlo en otra tecnología.

El documento no debe incluir sugerencias de mejora, migración o modernización, sino describir con 
precisión el comportamiento actual y todos los elementos que lo soportan.

El documento resultante debe tener el siguiente nombre .\docs\spec-legacy-busqueda-vendedores.md
	
Instrucciones específicas:

1. Explora el código fuente de la interfaz legada en los archivos 
	.\cpp\FormBusquedaVendedores.cpp, .\cpp\FormBusquedaVendedores.h y .\cpp\FormBusquedaVendedores.dfm
	  Para identificar lo siguiente:
	- La estructura completa del formulario (componentes, eventos, bindings).
	- El flujo de ejecución desde la acción del usuario hasta la obtención y despliegue de resultados.
	- Las funciones o métodos internos que gestionan la lógica de búsqueda, filtrado, carga y visualización.
2. Analiza la función backend ServidorBusquedas::BuscaVendedores para documentar:
	- Las consultas SQL exactas que ejecuta.
	- La lógica de filtros (qué condiciones se aplican según cada pestaña, campo o parámetro).
	- Cómo se construye el query dinámicamente.
	- Cómo se mapean los resultados a estructuras o clases del cliente.
3. Utiliza como contexto adicional:
	- La documentación del código legado en los archivos .\docs\claseslegadas\*.md.
	- Las definiciones de tablas en .\db\*.sql, considerando que:
		- La tabla central es vendedores.sql.
		- Se deben analizar las tablas relacionadas mediante claves foráneas para detallar todos los campos que se consultan o muestran en los resultados.
4. Si no se encuentra alguno de los archivos especificado como contexto deten la generación y reporta la causa y como continuar.




Contenido esperado en el documento:


1. Introducción técnica general

- Descripción del propósito del módulo de búsqueda dentro del sistema.
- Arquitectura funcional actual (interfaz, capa lógica, capa de datos).
- Diagrama (o descripción textual detallada del flujo) entre cliente (formulario) y servidor (ServidorBusquedas).

2. Estructura de la interfaz (Form)

- Componentes principales definidos en el .dfm (PageControl, Tabs, Grids, Campos de entrada, Botones, etc.).
- Eventos relevantes y funciones asociadas (OnClick, OnChange, OnKeyUp, etc.).
- Variables o estructuras internas utilizadas para almacenar filtros o resultados.

3. Detalle por pestaña (TTabSheet)

Para cada pestaña o página de búsqueda:
- Nombre de la pestaña y criterio principal.
- Campos y filtros (nombre del componente, tipo de dato, comportamiento, condiciones SQL generadas).
- Query SQL resultante, incluyendo JOIN, WHERE, ORDER BY, GROUP BY y paginación.
- Estructura del grid principal (VTStringGrid u otro), indicando:
	- Columnas, alias, campos de origen, tipo de dato y formato.
	- Campos calculados o transformados.
- Grids auxiliares (si los hay):
	- Propósito, relación con el grid principal, tablas involucradas, query usado.

4. Lógica de búsqueda (ServidorBusquedas::BuscaVendedores)

- Descripción paso a paso de cómo se procesa una búsqueda:
	- Recepción de parámetros desde el cliente.
	- Construcción dinámica del query.
	- Ejecución en base de datos.
	- Transformación de resultados a estructuras de salida.
- Detalle de validaciones, conversiones de tipos, manejo de valores nulos o vacíos.
- Identificación de dependencias con otras funciones o módulos.

5. Estructura de datos y relaciones

- Tablas involucradas (nombre exacto del archivo .sql).
- Para cada tabla:
	- Campos con nombre, tipo, longitud, restricciones, índices.
	- Claves primarias y foráneas.
	- Relación con otras tablas (1:N, N:N, etc.).
- Diagramas o descripciones relacionales relevantes para entender los joins del módulo.

6. Ejemplos de ejecución

- Ejemplos concretos de búsquedas con parámetros reales y sus queries generados.
- Ejemplo del resultado devuelto (estructura del buffer/recordset o grid).




Estilo del documento:

- Claro, exhaustivo y puramente técnico.
- Se deben incluir nombres exactos de funciones, clases, variables, tablas y campos.
- Sin lenguaje orientado a procesos o usuarios finales.
- Sin sugerencias de mejora, solo descripción del estado actual del sistema.
---