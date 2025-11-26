mode: agent

Quiero que realices la migración de la funcionalidad legada de frontend del Catálogo de Proveedores desde C++ Builder hacia React.

Contexto:

- La interfaz de usuario está en FormCatalogoProveedores.cpp, FormCatalogoProveedores.h y FormCatalogoProveedores.dfm. Úsalos para comprender el flujo legado, pero aplica patrones de UI moderna en React.
- El backend legado reside en ServidorCatalogos::GrabaProveedor, ServidorCatalogos::BajaProveedor y ServidorCatalogos::ConsultaProveedor (ClassServidorCatalogos.cpp). Revísalos solo como referencia funcional para comprender validaciones y transiciones de estado.
- El backend moderno ya está disponible vía REST en los endpoints documentados en .\docs\api-docs.json bajo la etiqueta "Catálogo de Proveedores". Debes consumir estos servicios desde el nuevo frontend:
	- GET /api/catalogo/proveedores (lista con paginación via query pageable)
	- GET /api/catalogo/proveedores/{proveedor} (detalle por clave)
	- POST /api/catalogo/proveedores (alta/modificación usando ProveedorRequest)
	- DELETE /api/catalogo/proveedores/{proveedor} (baja lógica)
	- GET /api/catalogo/proveedores/buscar/razon-social y /buscar/avanzada (búsquedas adicionales)
- Las estructuras de request/response (Page, ProveedorRequest, ProveedorResponse, Pageable, etc.) están descritas en el OpenAPI; al construir el frontend respeta esos contratos sin modificarlos.

Reglas y lineamientos:

- Sigue en lo posible las reglas definidas en .\github\frontend-general.md, .\github\frontend-testing.md y .\github\modulos-tipo-catalogos.md. Si necesitas desviarte, documenta el motivo y elige la mejor práctica disponible.
- Usa como contexto adicional:
	- .\docs\proceso-legacy-catalogo-proveedores.md
	- .\docs\spec-legacy-catalogo-proveedores.md
	- .\docs\claseslegadas\*.md (especialmente clases relacionadas con proveedores y teléfonos)
- No generes ni modifiques código backend; el alcance es exclusivamente frontend. Cualquier lógica de negocio nueva debe apoyarse en los endpoints existentes.

Consideraciones:

- Este módulo es de tipo catálogos (ver .github\modulos-tipo-catalogos.md) con características clave:
	0. Utiliza un TPageControl con pestañas para agrupar datos del proveedor; refleja esta organización en React.
	1. Incluye barra de acciones CRUD (buscar, alta, baja, guardar, cancelar); replica el comportamiento usando componentes reutilizables del ecosistema actual.
	2. Gestiona teléfonos del proveedor; asegúrate de mapear las operaciones disponibles en la API moderna y de sincronizar estados locales/remotos.
	3. No migres el botón de concentrados de pedidos automáticos.
	4. Restringe la interacción con datos a través de los endpoints REST; no emitas consultas SQL directas ni crees endpoints nuevos.
	5. Respeta etiquetas, validaciones y flujo de navegación descritos en el legado, mejorando UX cuando exista una recomendación explícita en frontend-general.md.
- La migración debe incluir:
	- Implementación en React de la interfaz completa (pestañas, filtros, grid principal y secciones auxiliares) consumiendo los endpoints listados.
	- Integración de paginación, filtros avanzados y búsquedas basadas en los parámetros definidos por la API (ej. pageable, razonSocial, rfc, estado, tipoProveedor).
	- Alta, edición, baja lógica y consulta de proveedores orquestadas con las respuestas ProveedorResponse; maneja mensajes de éxito/error y estados transaccionales coherentes.
	- Actualización del menú o rutas del frontend para acceder al catálogo de proveedores.
	- Pruebas (unitarias/componentes) siguiendo las recomendaciones de .\github\frontend-testing.md, cubriendo al menos el flujo feliz y validaciones críticas.
- Si falta alguno de los archivos de contexto mencionados, detén la generación y notifícalo.
- Mantén un enfoque modular, claro y extensible alineado con lo realizado en el catálogo de clientes migrado.