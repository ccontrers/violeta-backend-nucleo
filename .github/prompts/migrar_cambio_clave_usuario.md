Quiero que realices la migración  de una funcionalidad de backend legada desde C++ Builder hacia Java Spring Boot con un endpoint REST.

Contexto:

- La interfaz de usuario está en cpp\FormSistemaCambiarClave.cpp, cpp\FormSistemaCambiarClave.h y cpp\FormSistemaCambiarClave.dfm,
    tómalo como referencia para establecer la forma en que se usa el backend legado que debe ser similar en el API migrada que se genere.
- Las funciones que serán migradas se encuentran en `cpp\ClassServidorAdminSistema.cpp` / `.h`, y son:
    - ServidorAdminSistema::CambiaClave
    - ServidorAdminSistema::AsignaPassword
    - Cualquier rutina auxiliar que invoquen (por ejemplo hashing, validación de sesión, cifrado), si es de uso común debe ir en Utils o en otra clase de servicios compartidos (common/...).
  
Reglas y lineamientos:

- Sigue las reglas de migración definidas en los documentos siguientes: .\github\backend-general.md, .\github\backend-testing.md
- Usa como contexto adicional:
    - Documentación técnica de la funcionalidad legada: \docs\spec-legacy-cambio-clave-usuario.md
    - Definición de tablas SQL en .\db\*.sql, tomando en cuenta que la tabla central es
        usuarios.sql y analizando las relacionadas a través de sus llaves foráneas (por ejemplo las tablas de sucursales db\sucursales*.sql).

Consideraciones:

- La migración debe incluir:
    - Creación de endpoints REST en Spring Boot que ejecuten la lógica de cada función utilizando los verbos HTTP y rutas especificadas:
        1. **Asignar clave inicial**: `POST /api/v1/usuarios/{id}/clave`
            - Body JSON: `{ "nuevaClave": "string" }`
            - Respuestas esperadas: `201 Created`, `400 Bad Request`, `404 Not Found`, `409 Conflict`.
        2. **Cambiar clave existente**: `PUT /api/v1/usuarios/{id}/clave`
            - Body JSON: `{ "claveActual": "string", "nuevaClave": "string" }`
            - Respuestas esperadas: `200 OK`, `400 Bad Request`, `401 Unauthorized`, `404 Not Found`, `409 Conflict`.
    - Documentar lo necesario para cumplir con la especificación OpenAPI 3 (Swagger), incluyendo ejemplos de request/response y códigos descritos arriba.
    - Ya existe el endpoint /api/v1/usuarios para gestión de usuarios, asegúrate de que la nueva funcionalidad de cambio de clave se integre correctamente con la estructura y seguridad del API existente y que no afecte el funcionamiento actual. Creo que se usa para el catalogo de usuarios. Ve como se hizo ahí. Si tienes dudas pregunta.
- Si no encuentras alguno de los archivos de contexto mencionados, detén la generación y notifícalo, ya que son muy importantes para la migración.
- Asegúrate de mantener un enfoque modular, claro y extensible, de forma consistente con lo ya hecho en otra búsqueda que ya se migró como es catalogo de usuarios (/api/v1/usuarios), aunque en este caso es algo diferente porque no es un catálogo sino una acción específica de cambio de clave.
- Se debe diseñar pruebas de API, se puede usar el usuario con clave 'CRCP' el cual ya existe en la base de datos configurada con el proyecto (campo usuarios.clave='CRCP').
- Las contraseñas se reciben ya cifradas (hash SHA-256 como en el cliente legado), se procesan en hash y se persisten en hash; el backend **no** debe manejar texto plano.