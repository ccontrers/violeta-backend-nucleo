Quiero que realices la migración de una funcionalidad legada de frontend desde C++ Builder hacia React.

Contexto:

- La interfaz de usuario está en FormBusquedaVendedores.cpp, FormBusquedaVendedores.h y FormBusquedaVendedores.dfm,
	tómalo como referencia para establecer la forma en que funciona el frontend legado que debe ser similar en objetivo al frontend que se genere, 
	pero el nuevo frontend generado debe tener la mejores prácticas de una UI moderna.
- El backend legado está en la función ServidorBusquedas::BuscaVendedores (de C++ ubicada en ClassServidorBusquedas.cpp).
	tomalo como referencia para que tengas el contexto de como se integraba con el frontend legado.
- El backend actual (moderno) que debes usar para el frontend está en un API Rest (spring boot) en la ruta "/api/v1/busqueda/vendedores" 
	cuya estructura se encuentra en la especificación OpenAPI en el archivo .\docs\api-docs.json. El request debe seguir el contrato:
	```json
	{
		"tipoBusqueda": "NOM|APE|COMI|CLA",
		"valor": "<texto_de_busqueda>",
		"soloActivos": true|false,
		"limite": 501 // opcional; default deseado
	}
	```
	y el response expone `success`, `message`, `totalResultados` y la lista `vendedores` con campos `empleado`, `nombre`, `localidad`, `tipocomi`, `activo`.
- Debes migrar lo necesario de dichas funciones legadas para obtener como resultado un frontend en React, 
	con interfaz intuitiva, amigable y moderna.

Reglas y lineamientos:

- Sigue en lo posible las reglas de migración definidas en los documentos .\github\frontend-general.md y .\github\frontend-testing.md 
  y cuando no sea posible seguir estas reglas, elegir alguna opción con la mejor práctica posible.
- Usa como contexto adicional:
	- Documentación técnica de la funcionalidad legada: .\docs\spec-legacy-busqueda-vendedores.md
	- Al mapear comportamientos, respeta la equivalencia entre pestañas y `tipoBusqueda`: Nombre → `NOM`, Apellidos → `APE`, Tipo de comisión → `COMI` y Clave → `CLA`. Mantén validaciones de prefijo exactas descritas en el documento legado (upper-case, longitudes máximas, coincidencia exacta para comisión).
	
Consideraciones:

- Este módulo es de tipo búsquedas, con las siguientes características:
	1. Tiene un TPageControl que organiza las búsquedas en páginas (TTabSheet), cada una con un conjunto de filtros y un botón de ejecución.
	2. El grid principal de resultados es un VTStringGrid (no se utilizan grids secundarios en este formulario específico).
- La migración debe incluir:
	- Uso de las APIS REST del endpoint /api/v1/busqueda/vendedores como backend para que se ejecute la lógica de búsqueda de vendedores.
	- Un control visual (switch/toggle) que permita alternar el valor de `soloActivos` (activos, inactivos o ambos si la API lo permite) ya que en el legado era configurado externamente.
- Si no encuentras alguno de los archivos de contexto mencionados, detén la generación y notifícalo, ya que son muy importantes para la migración.
- Asegúrate de mantener un enfoque modular, claro y extensible.
- Se debe diseñar pruebas con base a lo especificado en frontend-testing.md. Se puede usar el vendedor con clave 'CAMT' 
	de nombre 'CARLOS MAGOS TAPIA' el cual ya existe en la base de datos configurada con el proyecto (campo vendedores.empleado='CAMT').
- Cuando presentes resultados, muestra al menos las columnas heredadas (`empleado`, `nombre`, `localidad`, `tipocomi`) y define explícitamente cómo se visualizará el estado `activo` devuelto por la API (columna, badge o justificación si se omite).
- Ajusta la paginación/límite usando el parámetro `limite` del request (por defecto 501) asegurando mensajes claros cuando `totalResultados` alcance el tope para incentivar refinamiento de filtros.