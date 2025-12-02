Genera un documento en formato Markdown con el nombre y path:
	.\docs\spec-legacy-cambio-clave-usuario.md

El objetivo es describir con el máximo detalle técnico posible la implementación actual del módulo de cambio de clave de usuario en el sistema legado, de forma que un programador pueda entender exactamente cómo funciona el código existente y reprogramarlo fielmente en otra tecnología. 

El documento no debe incluir sugerencias de mejora, migración o modernización, sino describir con precisión el comportamiento actual y todos los elementos que lo soportan.

Instrucciones específicas:

1. Explora el código fuente de la interfaz legada en los archivos (paths relativos a la raíz del repositorio `cpp/`):
	- cpp\FormSistemaCambiarClave.cpp
	- cpp\FormSistemaCambiarClave.h
	- cpp\FormSistemaCambiarClave.dfm (abrir en modo texto para inspeccionar todos los componentes VCL declarados)
	  Para identificar:
	- La estructura completa del formulario (componentes, eventos, bindings).
	- El flujo de ejecución desde la acción del usuario hasta la obtención y despliegue de resultados.
	- Las funciones o métodos internos que gestionan la lógica del módulo.
2. Analiza las funciónes backend ServidorAdminSistema::CambiaClave, ServidorAdminSistema::AsignaPassword y cualquier rutina auxiliar que invoquen (por ejemplo hashing, validación de sesión, cifrado) para documentar:
	- Las consultas SQL exactas que ejecuta.
	- La lógica de filtros (qué condiciones se aplican según si se está modificando o haciendo un alta).
	- Cómo se construye el query dinámicamente.
	- Cómo se mapean los resultados a estructuras o clases del cliente.  
	- Como se envía los datos, como de la contraseña, que si se revisa se envía cifrada.
3. Utiliza como contexto adicional:
	- La documentación del código legado en los archivos .\docs\claseslegadas\*.md.
	- Las definiciones de tablas en .\db\*.sql, considerando que:
		- La tabla central es usuarios.sql y debe describirse completa.
		- Se deben analizar las tablas de sucursales (ej. db\sucursales*.sql) vinculadas mediante claves foráneas para detallar todos los campos que se consultan o muestran en los resultados.

Contenido esperado en el documento:

1. Introducción técnica general
- Descripción del propósito del módulo de cambio de clave del usuario dentro del sistema.
- Arquitectura funcional actual (interfaz, capa lógica, capa de datos).
- Diagrama o flujo de ejecución entre cliente (formulario) y servidor (ServidorAdminSistema).

2. Estructura de la interfaz (Form)
- Componentes principales definidos en el .dfm (PageControl, Tabs, Grids, Campos de entrada, Botones, etc.).
- Eventos relevantes y funciones asociadas (OnClick, OnChange, OnKeyUp, etc.).
- Variables o estructuras internas utilizadas para almacenar filtros o resultados.

3. Lógica de negocios de cada función (ServidorAdminSistema::CambiaClave, ServidorAdminSistema::AsignaPassword)
- Descripción paso a paso de cómo se procesa cada función del módulo (cambio de clave, asignación de contraseña, consultas, etc.):
    - Recepción de parámetros desde el cliente.
    - Construcción dinámica del query.
    - Ejecución en base de datos.
    - Transformación de resultados a estructuras de salida.
- Detalle de validaciones, conversiones de tipos, manejo de valores nulos o vacíos.
- Identificación de dependencias con otras funciones o módulos.

4. Estructura de datos y relaciones
- Como se relacionan los datos generados por el backend con los componentes de la interfaz gráfica.
- Tablas involucradas (nombre exacto del archivo .sql), al menos db\usuarios.sql y las tablas de sucursales presentes en db\sucursales*.sql.
- Para cada tabla:
	- Campos con nombre, tipo, longitud, restricciones, índices.
	- Claves primarias y foráneas.
	- Relación con otras tablas (1:N, N:N, etc.).
- Diagramas o descripciones relacionales relevantes para entender los joins del módulo.

5. Consideraciones de seguridad
- Ejemplos concretos de funciones del modulo (cambio de clave, asignación de contraseña, etc.) con parámetros reales y las consultas SQL resultantes.
- Ejemplos concretos de cómo se maneja la seguridad en el módulo (cifrado de contraseñas, validaciones, etc.).
- Ejemplo del resultado devuelto describiendo el dataset/recordset mostrado en el grid y un JSON equivalente que represente exactamente los mismos campos y valores.

Estilo del documento:
- Lenguaje técnico preciso, evitando ambigüedades.
- Se deben incluir nombres exactos de funciones, clases, variables, tablas y campos.
- Sin lenguaje orientado a procesos o usuarios finales.
- Sin sugerencias de mejora, solo descripción del estado actual del sistema.