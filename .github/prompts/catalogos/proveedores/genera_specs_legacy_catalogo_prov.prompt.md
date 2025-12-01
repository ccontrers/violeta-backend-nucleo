---
mode: agent
---
Genera un documento en formato Markdown con el nombre y path:
	.\docs\spec-legacy-catalogo-proveedores.md
	
El objetivo es describir con el máximo detalle técnico posible la implementación actual del módulo de catálogos de proveedores en el sistema legado, de forma que un programador 
pueda entender exactamente cómo funciona el código existente y reprogramarlo fielmente en otra tecnología.
El documento no debe incluir sugerencias de mejora, migración o modernización, sino describir con precisión el comportamiento actual y todos los elementos que lo soportan.

Instrucciones específicas:

0. Explora el código fuente de la interfaz legada en los archivos:
	- FormCatalogoProveedores.cpp
	- FormCatalogoProveedores.h
	- FormCatalogoProveedores.dfm
	  Para identificar:
	- La estructura completa del formulario (componentes, eventos, bindings).
	- El flujo de ejecución desde la acción del usuario hasta la obtención y despliegue de resultados.
	- Las funciones o métodos internos que gestionan la lógica del catálogo, filtrado, carga y visualización.
1. Analiza las funciónes backend ServidorCatalogos::GrabaProveedor, ServidorCatalogos::BajaProveedor y ServidorCatalogos::ConsultaProveedor para documentar:
	- Las consultas SQL exactas que ejecuta.
	- La lógica de filtros (qué condiciones se aplican según si se está modificando o haciendo un alta.
	- Cómo se construye el query dinámicamente.
	- Cómo se mapean los resultados a estructuras o clases del cliente.
2. Utiliza como contexto adicional:
	- La documentación del código legado en los archivos .\docs\claseslegadas\*.md.
	- Las definiciones de tablas en .\db\*.sql, considerando que:
		- La tabla central es proveedores.sql.
		- Se deben analizar las tablas relacionadas mediante claves foráneas para detallar todos los campos que se consultan o muestran en los resultados.


Contenido esperado en el documento:


1. Introducción técnica general

- Descripción del propósito del módulo de catálogo de proveedores dentro del sistema.
- Arquitectura funcional actual (interfaz, capa lógica, capa de datos).
- Diagrama o flujo de ejecución entre cliente (formulario) y servidor (ServidorCatalogos).

2. Estructura de la interfaz (Form)

- Componentes principales definidos en el .dfm (PageControl, Tabs, Grids, Campos de entrada, Botones, etc.).
- Eventos relevantes y funciones asociadas (OnClick, OnChange, OnKeyUp, etc.).
- Variables o estructuras internas utilizadas para almacenar filtros o resultados.

3. Detalle por pestaña (TTabSheet)

Para cada pestaña o página del catálogo:
- Nombre de la pestaña y criterio principal.
- Campos y filtros (nombre del componente, tipo de dato, comportamiento, condiciones SQL generadas).
- Query SQL resultante, incluyendo JOIN, WHERE, ORDER BY, GROUP BY y paginación.
- Grids auxiliares (si los hay):
	- Propósito, relación con el grid principal, tablas involucradas, query usado.

4. Lógica de negocios de cada función (ServidorCatalogos::GrabaProveedor, ServidorCatalogos::BajaProveedor y ServidorCatalogos::ConsultaProveedor)

- Descripción paso a paso de cómo se procesa cada función del catálogo (búsqueda, altas, bajas, modificaciones, consultas, etc.):
	- Recepción de parámetros desde el cliente.
	- Construcción dinámica del query.
	- Ejecución en base de datos.
	- Transformación de resultados a estructuras de salida.
- Detalle de validaciones, conversiones de tipos, manejo de valores nulos o vacíos.
- Identificación de dependencias con otras funciones o módulos.

5. Estructura de datos y relaciones

- Como se relación con el formulario de búsqueda de proveedores.
- Tablas involucradas (nombre exacto del archivo .sql).
- Para cada tabla:
	- Campos con nombre, tipo, longitud, restricciones, índices.
	- Claves primarias y foráneas.
	- Relación con otras tablas (1:N, N:N, etc.).
- Diagramas o descripciones relacionales relevantes para entender los joins del módulo.

6. Ejemplos de ejecución

- Ejemplos concretos de funciónes del catálogo (búsqueda, altas, bajas, modificacione, consultas, etc) con parámetros reales y sus queries generados.
- Ejemplo del resultado devuelto (estructura JSON, recordset o grid).

Estilo del documento:

- Claro, exhaustivo y puramente técnico.
- Se deben incluir nombres exactos de funciones, clases, variables, tablas y campos.
- Sin lenguaje orientado a procesos o usuarios finales.
- Sin sugerencias de mejora, solo descripción del estado actual del sistema.
