# Backend — reglas y guardrails

Alcance: backend Java / Spring Boot del proyecto VioletaServer.

## Estado del Proyecto

### Módulos Implementados
- **Login:** Autenticación básica
- **Catálogos:** Clientes completo (CRUD + validaciones)
- **Búsquedas:** Artículos y Clientes (6 tipos de filtros)
- **Configuración:** Multi-empresa, base de datos unificada

### Estructura modelo del proyecto (Separado por entidades)
```
src/main/java/com/lavioleta/desarrollo/violetaserver/
├── config
├── security/            # Filtros y utilidades de autenticación/JWT
│   ├── JwtAuthenticationFilter.java
│   └── JwtTokenProvider.java
├── cliente/
│   ├── controller/          # REST Controllers
│   │   └── ClienteController.java
│   ├── service/             # Business logic
│   │   ├── ClienteService.java
│   │   └── impl/            # Service implementations
│   ├── repository/          # Data access (SQL only)
│   │   └── ClienteRepository.java
│   ├── dto/                 # Data Transfer Objects
│   │   ├── request/
│   │   └── response/
│   ├── entity/              # Entidades JPA (cuando aplica)
│   ...
├── proveedor/
│   ├── controller/          # REST Controllers
│   │   └── ProveedorController.java
│   ├── service/             # Business logic
│   │   ├── ProveedorService.java
│   │   └── impl/            # Service implementations
│   ├── repository/          # Data access (SQL only)
│   │   └── ProveedorRepository.java
│   ├── dto/                 # Data Transfer Objects
│   │   ├── request/
│   │   └── response/
|   ├── entity/              # Entidades JPA (cuando aplica)
│   ...
├── acceso/
│   ├── controller/          # REST Controllers
│   │    ├── AccesoController.java
│   │    ├── BitAccesoController.java
│   │    │   ├── ...
│   │    │   └── ...
│   ├── service/             # Business logic
│   │    │   ├── AccesoService.java
│   │    │   ├── BitAccesoService.java
│   │    │   ├── ...
│   │    │   └── impl/            # Service implementations
│   ├── repository/          # Data access (SQL only)
│   ├─── dto/                 # Data Transfer Objects
│   │    ├── request/
│   │    └── response/
│   ├── entity/              # Entidades JPA (cuando aplica)
│   ...
|── common/              # Clases comunes (utils, exception, etc.)
|    ├── exception/           # Global exception handling
|    └── constant/            # App constants 
|
└── config/              # Configuration beans (CORS, Security, etc.)

src/main/resources/
├── application-*.properties  # Configuración por perfil (dev/test/prod)
└── db/                       # Scripts y definiciones SQL de referencia
```

---

## Guardrails Obligatorios

### Tecnología y Arquitectura
- ✅ **Spring JDBC Client** como primera opción (no JdbcTemplate)
- ✅ **JPA** como segunda opción donde sea evidente que JPA ya tiene funcionalidad que reduce significativamente la complejidad del código.
- ✅ **SQL solo en Repositories** - Controllers/Services sin SQL inline
- ✅ **Validación en DTOs** (jakarta.validation) y Controllers
- ✅ **Logging con @Slf4j** en Services/Controllers
- ⚠️ **Transacciones** cuando corresponda (`@Transactional` en ServiceImpl)

### Base de Datos
- ✅ **No usar Flyway** (BD ya configurada)
- ✅ **Mantener** `spring.jpa.hibernate.ddl-auto=none`
- ✅ **Schema:** `20250910_multiempresa` en MariaDB 11.2.x
- ✅ **Port:** 3308

### Seguridad y Validación
- ✅ **Autenticación híbrida**: cookies HTTP (`/api/v1/auth/login`) y JWT (`/api/v1/auth/login/jwt`)
- ✅ **Parámetros SQL** - nunca concatenar entradas de usuario
- ✅ **Defaults explícitos** para columnas NOT NULL al insertar
- ✅ **Validar FK** antes de insertar para evitar violaciones
- ✅ **Mapear a DTOs** en Repository (no exponer ResultSet)

---

## Convenciones de Nombres y Estructura

### Paquetes y Clases
- **Controllers:** `*Controller`
- **Services:** `*Service` + `*ServiceImpl`
- **Repositories:** `*Repository`
- **DTOs:** `request/response` suffixes

### Orden para Nuevos Módulos
1. **DTOs** (request/response)
2. **Repository** (consultas SQL)
3. **Service** (interfaz) + **ServiceImpl**
4. **Controller** (endpoints REST)

---

## Patrones y Buenas Prácticas

### Autenticación (sesión + JWT)
- El backend habilita sesiones tradicionales **y** tokens JWT.
- Login web: usar `/api/v1/auth/login` (recibe `JSESSIONID` + cookies, requiere `withCredentials` en axios/fetch).
- Integraciones/automatización: usar `/api/v1/auth/login/jwt` y enviar `Authorization: Bearer <token>` en cada request.
- Tests `@ActiveProfiles("test")` ya deshabilitan filtros de seguridad; no dupliques lógica.
- Documenta nuevos endpoints protegidos con `@Operation(security = { @SecurityRequirement(name = "bearerAuth") })` y mantén `cookieAuth` vigente en Swagger.

### Consultas SQL
```java
// ✅ Patrón QueryBuilder recomendado
String sql = """
    SELECT c.*, ce.vendedor, ce.cobrador
    FROM clientes c
    LEFT JOIN clientesemp ce ON c.cliente = ce.cliente
    WHERE c.codigo = ? AND c.activo = 1
    """;

return jdbcClient.sql(sql)
    .param(codigo)
    .query(this::mapToClienteResponse)
    .optional();
```

### Validación y Defaults
```java
// ✅ Defaults explícitos para NOT NULL
INSERT INTO clientesemp (cliente, idempresa, enviarcfd, digitosdef, digitossup)
VALUES (?, ?, ?, COALESCE(?, ''), COALESCE(?, ''))
```

### Manejo de Errores
```java
// ✅ Logging estructurado + excepciones específicas
@Slf4j
@Service
public class CatalogoClientesServiceImpl {
    public void guardarCliente(ClienteRequest request) {
        try {
            validarRequest(request);
            repository.guardarCliente(request);
        } catch (Exception e) {
            log.error("Error guardando cliente {}: {}", request.getCodigo(), e.getMessage());
            throw new RuntimeException("Error al procesar cliente");
        }
    }
}
```

---

## Testing Strategy

Tests unitarios backend (ver [`backend-testing.md`](backend-testing.md))

### Tests E2E Existentes
✅ **Playwright** para catálogo de clientes:
- Alta de cliente nuevo
- Modificación preservando campos no enviados
- Validaciones FK constraints
- Manejo errores 400/404

### Próxima Prioridad
```java
// REQUERIDO: Tests unitarios
src/test/java/.../repository/CatalogoClientesRepositoryTest.java
src/test/java/.../service/CatalogoClientesServiceTest.java  
src/test/java/.../dto/ClienteRequestValidationTest.java
```

---

## Tecnologías Utilizadas

### Stack Principal
- **Spring Boot:** 3.5.4
- **Java:** 21
- **Spring JDBC Client** (acceso datos principal)
- **Spring Data JPA** (casos complejos)
- **MariaDB:** 11.2.6 (producción)
- **H2 Database** (testing)
- **Gradle** (build system)

### Testing y Calidad
- **JUnit 5** + **Mockito** (unit tests)
- **Playwright** (E2E tests)
- **@Slf4j** (logging)

---

## Documentación Relacionada

### Módulos Específicos
- **📋 [Módulos tipo Catálogos](modulos-tipo-catalogos.md)** - Patrones CRUD, validaciones
- **🔍 [Módulos tipo Búsquedas](modulos-tipo-busquedas.md)** - Filtros dinámicos, paginación
- **🔐 [Security General](security-general.md)** - Autenticación y autorización

### Testing y Desarrollo
- **🧪 [Backend Testing](backend-testing.md)** - Estrategia tests, ejemplos, checklist
- **⚙️ [Frontend General](frontend-general.md)** - Integración frontend-backend

### Configuración
- **🗄️ Esquema BD:** `src/db/*.sql` (estructura tablas y relaciones)
- **📝 API Docs:** `docs/BUSQUEDA_ARTICULOS_API.md`, `docs/CATALOGO_CLIENTES.md`

### 📘 Guía OpenAPI / Swagger (Resumen)
Principios clave:
- La especificación es un contrato revisable en PR.
- Todo endpoint debe entenderse sin leer código.
- Sólo documentar lo estable; marcar `deprecated` si aplica.

Anotaciones mínimas:
- `@Tag` por dominio (evitar genéricos tipo "Misc").
- `@Operation(summary=..., description=...)` con summary ≤ 90 chars.
- `@ApiResponses` con códigos: éxito + principales errores (200, 400, 401/403, 404, 409, 422, 500).
- `@Parameter` cuando el nombre no sea claro o haya formatos/enums.
- `@Schema` en DTOs clave y en `ApiError`.

Convenciones de rutas:
- Prefijo versión `/api/v1`.
- Plurales para colecciones (`/clientes`).
- Subrecursos jerárquicos (`/clientes/{id}/credito`).
- No usar verbos en la ruta.

Errores estandarizados (`ApiError`):
```
{
    "timestamp": "2025-10-07T10:20:31Z",
    "status": 400,
    "error": "Bad Request",
    "message": "Error de validación",
    "path": "/api/v1/catalogos/clientes",
    "method": "POST",
    "code": "VALIDATION_ERROR",
    "fieldErrors": [ { "field": "razonSocial", "message": "Obligatorio" } ],
    "traceId": "af23d1..."
}
```
Reglas: no stacktrace, `code` estable (UPPER_SNAKE), usar `fieldErrors` para múltiples.

Paginación estándar: parámetros `pagina` (≥1), `registrosPorPagina` (≤100). Respuesta debe incluir `items`, `pagina`, `registrosPorPagina`, `totalRegistros`, `totalPaginas`.

Seguridad:
- Definir esquema `bearerAuth` en `OpenApiConfig`.
- Añadir `@Operation(security = { @SecurityRequirement(name = "bearerAuth") })` cuando JWT esté activo.

Ejemplos (`@ExampleObject`): sólo para payloads complejos; evitar datos reales.

Deprecación: usar `@Deprecated` + `@Operation(deprecated = true, summary = "[DEPRECATED] ...")` y documentar reemplazo.

Breaking changes (requiere revisión): eliminar campos requeridos, cambiar tipos, modificar semántica de códigos HTTP, renombrar rutas. Preferir nuevo endpoint / nueva versión.

CI/CD sugerido:
```
curl -s http://localhost:5986/v3/api-docs > build/openapi.json
spectral lint build/openapi.json
openapi-diff previous.json build/openapi.json --fail-on-incompatible
```

Checklist PR OpenAPI:
- [ ] Nuevo endpoint con `@Operation` + respuestas.
- [ ] Cambios de contrato reflejados.
- [ ] Errores usan `ApiError`.
- [ ] Campos nuevos documentados.
- [ ] Sin exponer detalles internos.

HTTP codes guía rápida:
200 éxito, 201 creación, 204 sin contenido, 400 validación, 401 no autenticado, 403 sin permiso, 404 no existe, 409 conflicto negocio, 422 regla negocio compleja (opcional), 500 error inesperado.

Anti‑patrones a evitar:
- Rutas con verbos (`/getCliente`).
- 200 con `success:false` para errores (usar código correcto).
- Documentar endpoints internos sin necesidad.
- Descripciones enormes con lógica de negocio.

Flujo para nuevo endpoint:
1. Crear DTOs / modelo.
2. Añadir método con respuesta tipada.
3. Añadir `@Tag` (si no existe).
4. `@Operation` + `@ApiResponses`.
5. `@Schema` en DTOs complejos.
6. Revisar `/v3/api-docs`.
7. Checklist en PR.

Ejemplo breve:
```java
@Tag(name = "Catálogo Clientes", description = "Gestión CRUD de clientes")
@GetMapping("/{codigoCliente}")
@Operation(summary = "Consultar cliente", description = "Devuelve datos del cliente si existe")
@ApiResponses({
    @ApiResponse(responseCode = "200", description = "Cliente encontrado"),
    @ApiResponse(responseCode = "404", description = "No existe")
})
public ResponseEntity<ClienteResponse> consultarCliente(@PathVariable String codigoCliente) { /* ... */ }
```

Fecha última actualización OpenAPI: 2025-10-07

---

## Comandos Útiles

```bash
# Compilar y ejecutar
./gradlew build
./gradlew bootRun

# Testing
./gradlew test                    # Tests unitarios
cd frontend && npx playwright test  # Tests E2E

# Base de datos
# Schema: 20250910_multiempresa
# Port: 3308
# Tablas principales: clientes, articulos, sucursales
```

---

## Características Implementadas

### ✅ Funcionalidades Core
- Arquitectura en capas (Controller → Service → Repository)
- Spring JDBC Client para consultas SQL optimizadas
- DTOs tipados para separar presentación de datos
- Manejo global de excepciones con `ApiError` unificado
- Logging estructurado en todas las capas
- Configuración por perfiles (dev/test/prod)

### ✅ Módulos Operativos
- **Login:** Autenticación multi-sucursal
- **Catálogo Clientes:** CRUD completo con validaciones complejas
- **Búsqueda Artículos:** 6 tipos filtros + paginación
- **Búsqueda Clientes:** Filtros por nombre, RFC, código

### ⚠️ Pendientes Críticos
- Tests unitarios backend (Repository, Service, DTO)
- Documentación OpenAPI/Swagger
- Métricas y monitoreo
- CI/CD pipeline completo

---
