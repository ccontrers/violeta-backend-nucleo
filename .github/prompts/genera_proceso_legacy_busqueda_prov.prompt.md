---
mode: agent
---
Genera un documento en formato Markdown con el nombre y path:
	 .\docs\proceso_legacy_busqueda_proveedores.md
El objetivo es describir de la forma más detallada posible la interfaz de búsqueda de proveedores legada, con un enfoque dirigido a usuarios no técnicos cuyo interés es documentar procesos y no detalles de implementación técnica.

Instrucciones específicas:

0. Explora el código de la interfaz legada en los archivos:
	- FormBusquedaProveedores.cpp
	- FormBusquedaProveedores.h
	- FormBusquedaProveedores.dfm
1. para identificar cómo está organizada la interfaz de usuario.
2. Analiza la función backend ServidorBusquedas::BuscaProveedores para comprender:
	- Qué datos devuelve la búsqueda.
	- Qué queries ejecuta.
	- Cómo aplica los filtros de cada pestaña/página de búsqueda.
3. Usa como contexto adicional:
	- La documentación del código legado en los archivos .\docs\claseslegadas\*.md
	- La definición de tablas en .\db\*.sql, tomando en cuenta que:
		- La tabla central es proveedores.sql.
		- Analiza las tablas relacionadas a través de sus llaves foráneas para explicar datos adicionales que puedan mostrarse en los resultados o en grids auxiliares.

Contenido esperado en el documento:

- Una introducción general de cómo funcionan las búsquedas en el sistema (usa la explicación general de las búsquedas que tienen un TPageControl, grids principales VTStringGrid y grids adicionales).
- Una descripción página por página (TTabSheet) de búsqueda, detallando:
	- Criterio principal de búsqueda.
	- Filtros disponibles, con una explicación clara de cómo se utilizan en el proceso.
	- Datos que deben mostrarse en el grid de resultados principal, en qué orden, con qué formato.
	- Datos en grids adicionales (si los hay), explicando qué relación tienen con el resultado principal.
- Usa un estilo claro, descriptivo y orientado a procesos (sin tecnicismos como nombres de funciones o clases), para que pueda ser entendido por personal de negocio, analistas o responsables de procesos.