Manual de migración


Definir las variables de entorno para cada backend.
VIOLETABACKENDNUCLEODB
	localhost:3308/20251118_multiempresa
VIOLETABACKENDNUCLEOPORT
	6820
VIOLETAFRONTENDNUCLEOPORT
	3120
VITE_API_BASE_URL
	http://localhost:6820


De preferencia evitar hacer cambios en el légacy en módulos que se estén migrando, para evitar que se migre con diferencias.

Cuidar los desfases de lo que se está migrando actualizando los códigos fuente originales y estructura de la bd (SQL).

En nombres de repositorios y spring.application.name usar guiones medios para separar palabras. Esar el siguiente formato:
violeta-<tipo de aplicacion>-<dominio>-subdominio>
inicialmente:
	violeta-backend-nucleo
	violeta-frontend-nucleo

Los proyectos de backend el paquete debe quedar en la siguiente URL inversa:
	com.lavioleta.desarrollo.violetaserver;

Asignación de Puertos para desarrollo y pruebas
* Backends: 6820–6839
* Frontends: 3120–3139
* Bases de datos: 3300–3319

Asignación de Puertos para Producción
* Backends: 7820–7839
* Frontends: 8120–8139
* Bases de datos: 3320–3339


Buenas prácticas y recomendaciones:

Usar archivos markdown (.md) en vez de documentos de word, porque es IA fiendly.

