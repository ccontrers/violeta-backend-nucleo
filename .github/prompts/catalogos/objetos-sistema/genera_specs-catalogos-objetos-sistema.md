# Generar especificaciones técnicas
Genera un documento en formato Markdown con el nombre y path: `docs/catalogo/objetos-sistema/spec-legacy-catalogo-objetos-sistema.md`
	
El objetivo es describir con el máximo detalle técnico posible la implementación actual del módulo de `catalogo` de `objetos-sistema` en el sistema legado, de forma que un programador pueda entender exactamente cómo funciona el código existente y reprogramarlo fielmente en otra tecnología. 

El documento no debe incluir sugerencias de mejora, migración o modernización, sino describir con precisión el comportamiento actual y todos los elementos que lo soportan.

Instrucciones específicas:

1. Explora el código fuente de la interfaz legada y el backend en los archivos:
	- `cpp/FormCatalogoObjetosSistema.cpp`
	- `cpp/FormCatalogoObjetosSistema.h`
	- `cpp/FormCatalogoObjetosSistema.dfm`
	- `cpp/ClassServidorCatalogos.cpp`
	- `cpp/ClassServidorCatalogos.h`
	  Para identificar:
	- La estructura completa del formulario (componentes, eventos, bindings).
	- El flujo de ejecución desde la acción del usuario hasta la obtención y despliegue de resultados.
	- Las funciones o métodos internos que gestionan la lógica del `catalogo`, filtrado, carga y visualización.
2. Analiza las funciones backend de `ClassServidorCatalogos` que son ejecutadas llamando al servidor buscando "gClienteVioleta->InicializaPeticion()", "gClienteVioleta->EjecutaSqlSelect()" o del tipo "gClienteVioleta->Interfaz->Llena*". Para documentar:
	- Las consultas SQL exactas que ejecuta.
	- La lógica de filtros (qué condiciones se aplican según si se está modificando o haciendo una función específica).
	- Cómo se construye el query dinámicamente.
	- Cómo se mapean los resultados a estructuras o clases del cliente.
3. Utiliza como contexto adicional:
	- La documentación del código legado en los archivos .\docs\claseslegadas\*.md.
	- Las definiciones de tablas en .\db\*.sql, considerando que:
		- La tabla central es `objetossistema.sql`
		- Se deben analizar las tablas relacionadas mediante claves foráneas para detallar todos los campos que se consultan o muestran en los resultados.


## Contenido esperado en el documento:
1. Introducción técnica general
- Descripción del propósito del módulo de `catalogo` de `objetos-sistema` dentro del sistema.
- Arquitectura funcional actual (interfaz, capa lógica, capa de datos).
- Diagrama o flujo de ejecución entre cliente (formulario) y servidor (`ClassServidorCatalogos`).

2. Estructura de la interfaz (Form)
- Componentes principales definidos en el .dfm
- Eventos relevantes y funciones asociadas (OnClick, OnChange, OnKeyUp, etc.).
- Variables o estructuras internas utilizadas para almacenar filtros o resultados.

3. Lógica de negocios de cada función identificada al buscar "gClienteVioleta->InicializaPeticion()", "gClienteVioleta->EjecutaSqlSelect()" o del tipo "gClienteVioleta->Interfaz->Llena*"

- Descripción paso a paso de cómo se procesa cada función del `catalogo` de `objetos-sistema`
	- Recepción de parámetros desde el cliente.
	- Construcción dinámica del query.
	- Ejecución en base de datos.
	- Transformación de resultados a estructuras de salida.
- Detalle de validaciones, conversiones de tipos, manejo de valores nulos o vacíos.
- Identificación de dependencias con otras funciones o módulos.

4. Estructura de datos y relaciones
- Como se relacionan los datos generados por el backend con los componentes de la interfaz gráfica.
- Tablas involucradas (nombre exacto del archivo .sql).
- Para cada tabla:
	- Campos con nombre, tipo, longitud, restricciones, índices.
	- Claves primarias y foráneas.
	- Relación con otras tablas (1:N, N:N, etc.).
- Diagramas o descripciones relacionales relevantes para entender los joins del módulo.

5. Ejemplos de ejecución
- Ejemplos concretos de funciones del `catalogo` de `objetos-sistema` con parámetros reales y las consultas SQL resultantes.
- Ejemplo del resultado devuelto (estructura JSON, recordset o grid) indicando la correspondencia con los campos mostrados en la interfaz.

# Estilo del documento:
- Claro, exhaustivo y puramente técnico.
- Se deben incluir nombres exactos de funciones, clases, variables, tablas y campos.
- Sin lenguaje orientado a procesos o usuarios finales.
- Sin sugerencias de mejora, solo descripción del estado actual del sistema.
