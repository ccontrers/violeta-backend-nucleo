---
mode: agent
---

Quiero que realices la migración de una funcionalidad legada desde C++ Builder hacia Java Spring Boot (backend) y React (frontend).

Contexto y archivos a migrar:

- La interfaz de usuario está en:
	- FormCatalogoProveedores.cpp
	- FormCatalogoProveedores.h
	- FormCatalogoProveedores.dfm
- El backend legado está en las funciónes:
	- ServidorCatalogos::GrabaProveedor, ServidorCatalogos::BajaProveedor y ServidorCatalogos::ConsultaProveedor (C++).
Debes migrar lo necesario de dichas funciones legadas para el backend en Spring Boot, y la parte de la interfaz de FormCatalogoProveedores para el frontend en React, siguiendo un patrón similar al que se usó en la migración del catálogo de cliente, ya que se espera un resultado similar y con interfaz parecida.

Reglas y lineamientos:

- Sigue las reglas de migración definidas en los documentos que están en la carpeta y muestrame los documentos que estas considerando:
	- .\github\*.md
- Usa como contexto adicional:
	- Documentación del proceso legado: .\docs\proceso-legacy-catalogo-proveedores.md
	- Documentación técnica de la funcionalidad legada: .\docs\spec-legacy-catalogo-proveedores.md
	- Documentación del código fuente legado: .\docs\claseslegadas\*.md
	- Definición de tablas SQL en .\db\*.sql, tomando en cuenta que la tabla central es proveedores.sql y analizando las relacionadas a través de sus llaves foráneas.

Consideraciones:

- Este módulo es de tipo catalogos (ver .github\modulos-tipo-catalogos.md), con las siguientes características:
	0. Tiene un TPageControl que organiza los datos relacionados con un proveedor en páginas (TTabSheet).
	1. Hay una barra de botones: de búsqueda, alta, baja, guardar y cancelar modificaciones. Revisar la implementación en el catálogo de clientes migrado, para seguir un patrón similar.
	2. Hay una sección para gestionar los teléfonos del proveedor, revisar la implementación en el catálogo de clientes migrado, para seguir un patrón similar.
	3. Hay un botón de concentrados de pedidos automáticos que no se debe migrar.
	4. Existen querys en el frontend legados que se deben migrar al backend, ya que en la arquitectura moderna no se deben hacer querys SQL desde el frontend.
	5. Respetar la organización de las pestañas y los filtros definidos en el formulario legado, además de los labels y textos visibles en la interfaz.
- La migración debe incluir:
	- En backend: creación de los endpoints REST en Spring Boot que ejecuten la lógica de necesario de un catálogo de proveedores.
	- En frontend: implementación en React de la interfaz de catálogo de proveedores con pestañas, filtros, grid principal de resultados y grids adicionales.
	- Creación de una opción en el menú de la aplicación para acceder a este nuevo catálogo de proveedores y enlazarla a la nueva funcionalidad.
- Si no encuentras alguno de los archivos de contexto mencionados, detén la generación y notifícalo, ya que son muy importantes para la migración.
- Asegúrate de mantener un enfoque modular, claro y extensible, de forma consistente con lo ya hecho en el catálogo de clientes.