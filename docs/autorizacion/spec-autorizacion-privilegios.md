# Especificación técnica – Subsistema de autorización y privilegios

> Fuente de verdad de la implementación realizada durante la migración descrita en `.github/prompts/plan_migrar_autorizacion.prompt.md`. Usa este documento para portar el mismo diseño a otro backend, mantenerlo o auditarlo.

## 1. Objetivo y alcance
- Centralizar la resolución de privilegios en el backend Spring Boot 3, alineado con el esquema legado.
- Exponer un API REST versionada (`/api/v1/security/privilegios/**`) que soporte lectura masiva, consultas puntuales y verificación booleana.
- Integrar las decisiones con Spring Security (autoridades, filtros y `@PreAuthorize`).
- Consumir los privilegios desde React (TanStack Query) mediante un contexto único (`AuthContext`) y hooks (`usePrivilege`).
- Entregar un módulo de demostración (`EJEMPLOAUT`) que enseña cómo reaccionar a los privilegios en la UI.

## 2. Flujo extremo a extremo
1. `POST /api/v1/auth/login` (`LoginController.java`) autentica, inicializa `SecurityContext` y crea sesión/cookie.
2. En la primera petición autenticada, `AuthorityPopulatorFilter.java` consulta `AuthorizationService` y agrega autoridades `OBJETO_PRIVILEGIO` al contexto.
3. El frontend (tras login) invoca `GET /api/v1/security/privilegios/self` usando `AuthorizationService` (TypeScript). El backend responde con `ObjetoPrivilegiosDTO` + parámetros de versión.
4. `AuthContext.tsx` (actualmente en `src/context/autorizacion/AuthContext.tsx`) normaliza la respuesta y la cachea en React Query; `usePrivilege` y los componentes (p. ej. `EjemploAutorizacion.tsx`) consumen `hasPrivilege` para habilitar/ocultar UI.
5. Cuando se modifican privilegios de un usuario, el backend puede llamar `AuthorizationService.evictUsuario(usuario)` o `AuthorizationController.flushCache()` para forzar que la siguiente petición regenere el cache.
6. Los endpoints protegidos en otros módulos usan `@PreAuthorize("hasAuthority('OBJ_PRIV')")` o el `PrivilegePermissionEvaluator` para validar en el backend antes de ejecutar lógica de negocio.

## 3. Backend Spring Boot

### 3.1 Acceso a datos (`src/main/java/com/lavioleta/desarrollo/violetaserver/repository/AuthorizationRepository.java`)
- Implementa la consulta `UNION` de privilegios directos, por rol y por puesto usando `JdbcClient`. El SQL se encuentra embebido en `PRIVILEGE_SQL_TEMPLATE`.
- Métodos relevantes:
  - `findAllByUsuario(String usuario)` devuelve todos los privilegios del usuario.
  - `findAllByUsuarioAndObjeto(String usuario, String objeto)` filtra por objeto (usado por cache miss puntuales).
  - `findVersionRequirements()` lee `parametrosemp` (`VERSIONMINIMA`, `SUBVERSIONMIN`, `TIEMVALIDVERS`).
- El row mapper `PrivilegeRowMapper` construye `PrivilegeRecord` (usuario,objeto,privilegio,descripcion), base del resto del pipeline.

### 3.2 DTOs (`src/main/java/com/lavioleta/desarrollo/violetaserver/dto/security/*.java`)
- `PrivilegioDTO`: objeto + privilegio + descripción, usado para listados específicos.
- `ObjetoPrivilegiosDTO`: agrupación `{ objeto, List<String> privilegios }` que alimenta el frontend.
- `PrivilegeCheckResponse`: resultado booleano `allowed` para `GET /{objeto}/{privilegio}`.
- `VersionRequirementDTO`: versionado mínimo/gracia enviado a clientes.

### 3.3 Servicio y cache (`src/main/java/com/lavioleta/desarrollo/violetaserver/service/AuthorizationService.java`)
- Usa Caffeine (`privilegeCache`) con TTL y `maximumSize` definidos en `AuthorizationProperties`.
- Responsabilidades:
  - `getPrivilegios(usuario,objeto)` y `getPrivilegiosAgrupados(usuario)` sirven al controller, rellenando cache por usuario.
  - `hasPrivilege(usuario,objeto,privilegio)` reutiliza el cache y respeta la bandera `security.authorization.enforcement-enabled` para ambientes locales.
  - `getAuthoritiesForUsuario(usuario)` transforma cada privilegio a autoridad (`DomainPermissionMapper`).
  - `revisarPrivilegio` y `ensureExamplePrivilegios` facilitan el módulo `EJEMPLOAUT` agregando privilegios ficticios si el usuario no los tiene.
  - Métodos de invalidación: `evictUsuario` y `evictAll` usados por el endpoint de flush o procesos administrativos.

### 3.4 Configuración externa (`src/main/java/com/lavioleta/desarrollo/violetaserver/config/AuthorizationProperties.java` + `src/main/resources/application*.properties`)
- Prefijo `security.authorization.*` permite ajustar TTL (`cache.ttl`), tamaño (`cache.maximum-size`), versión mínima (`version.min/sub/grace-minutes`) y la bandera `enforcement-enabled`.
- `application.properties` define valores por defecto (TTL 5 min); `application-dev.properties` reduce el TTL (2 min) y habilita logs detallados para depuración.

### 3.5 Seguridad e integración con Spring Security
- `AuthorityPopulatorFilter.java`: filtro `OncePerRequestFilter` registrado después de `UsernamePasswordAuthenticationFilter` en `SecurityConfig`. Lee el usuario autenticado y agrega nuevas autoridades basadas en `AuthorizationService.getAuthoritiesForUsuario`.
- `DomainPermissionMapper.java`: convención `OBJETO_PRIVILEGIO` usada tanto por el filtro como por los `@PreAuthorize`.
- `PrivilegePermissionEvaluator.java` + `MethodSecurityConfiguration.java`: habilitan expresiones como `@PreAuthorize("hasPermission('CATPROV','MOD')")`, delegando de nuevo en `AuthorizationService`.
- `SecurityConfig.java`: activa `@EnableMethodSecurity`, registra `JwtAuthenticationFilter` y `AuthorityPopulatorFilter`, configura CORS para que el frontend pueda enviar cookies.
- `LoginController.java`: al autenticar, crea `UsernamePasswordAuthenticationToken` con autoridad base `ROLE_USER` y almacena `usuarioInfo` en la sesión para que el filtro pueda resolverlo.

### 3.6 API REST (`src/main/java/com/lavioleta/desarrollo/violetaserver/controller/AuthorizationController.java`)
- Base path: `/api/v1/security/privilegios`.
- Endpoints implementados:
  - `GET /self` y `GET /` → lista agrupada (`List<ObjetoPrivilegiosDTO>`). Loggea usuario y tamaño.
  - `GET /{objeto}` → lista `List<PrivilegioDTO>` (404 si vacío).
  - `GET /{objeto}/{privilegio}` → `PrivilegeCheckResponse` (`allowed=true/false`).
  - `GET /version/requerimientos` → `VersionRequirementDTO` para clientes legacy.
  - `POST /cache/flush` → invalida todo el cache; protegido con `@PreAuthorize("hasRole('ADMIN') or hasAuthority('SECURITY_CACHE_FLUSH')")`.
- Logging: cada endpoint escribe en `INFO` el usuario, objeto y privilegio para trazabilidad.

### 3.7 Documentación y contrato (`docs/api-docs.json`)
- OpenAPI 3.1 actualizado: describe todos los endpoints anteriores, los DTOs y los esquemas de seguridad (`cookieAuth`, `bearerAuth`).
- Sirve para generar clientes, probar via Swagger UI y confirmar path exactos.

### 3.8 Pruebas backend
- Ejecutar `./gradlew test` para validar repositorio, servicio y controller.
- Para verificar build completo: `./gradlew build` (última ejecución exitosa registrada en el terminal `java`).

## 4. Frontend React + TypeScript (Vite)

### 4.1 Servicios y configuración
- `src/lib/constants.ts`: centraliza `API_BASE_URL` (`VITE_API_BASE_URL`) y la sucursal predeterminada (`DEFAULT_SUCURSAL`, via `VITE_SUCURSAL_DEFAULT`). Expone `buildApiUrl(path)` para componer rutas absolutas y evitar hardcodear host/puerto.
- `src/services/autorizacion/auth.service.ts`: usa `buildApiUrl("/api/v1/auth")`, conserva el hashing SHA-256 y ahora persiste tanto el usuario como el token emitido por `/login`. Todos los requests se hacen con `withCredentials=true` para depender de la cookie de sesión. Métodos expuestos: `login`, `setUsuario`, `clearUsuario`, `obtenerSucursales`, etc.
- `src/services/autorizacion/authorization.service.ts`: cliente Axios apuntando a `buildApiUrl("/api/v1/security/privilegios")`, mantiene `withCredentials=true`, logs (`logRequest`) y sanitización (`encodeSegment`). Se añadió validación estricta del payload (`Array.isArray`) para evitar errores `reduce is not a function` cuando el backend devuelve mensajes en vez del JSON esperado.

### 4.2 AuthContext, guardado de sesión y hook de privilegios
- `src/context/autorizacion/AuthContext.tsx`:
  - Estado: `usuario`, `privilegios`, `loadingPrivilegios`, `privilegiosError`, `privilegiosEndpoint` (derivado de `buildApiUrl("/api/v1/security/privilegios/self")`).
  - React Query sólo intenta cargar privilegios cuando existe `usuario` (evita llamadas anónimas que devolvían HTML/errores y rompían el `reduce`). Se mantiene `placeholderData` para conservar el diccionario tras un reload.
  - Normaliza cada objeto (`trim().toUpperCase()`) y privilegio, loggeando inputs/salidas y rehusando `hasPrivilege` con trazas `console.debug`.
  - Maneja errores diferenciando 401, códigos inesperados y excepciones de red; el mensaje se refleja en la tarjeta de depuración.
  - `setUsuario` sincroniza `AuthService`, borra tokens/caches al hacer logout y hace `invalidateQueries(['privilegios'])` después de login/logout. `refreshPrivilegios` ignora la invocación si no hay sesión.
- `src/hooks/autorizacion/usePrivilege.ts`: hook delgado que devuelve `{ allowed, loading }` y encapsula el acceso al contexto cuando sólo se necesita una bandera aislada.
- `src/hooks/autorizacion/usePrivilegedActions.ts`: resuelve múltiples privilegios de un mismo objeto en una sola lectura de contexto, expone `allowed` e `isAllowed` y reduce renders duplicados.

### 4.3 Módulo de demostración (`src/modules/autorizacion/EjemploAutorizacion.tsx`)
- Objeto fijo `EJEMPLOAUT` con privilegios `ALT|BAJ|MOD|CON` resueltos con `usePrivilegedActions(EJEMPLOAUT, ['ALT','BAJ','MOD','CON'])`.
- Secciones:
  1. **Sección A** – Botones visibles pero deshabilitados si falta el privilegio.
  2. **Sección B** – Solo renderiza botones con privilegios asignados.
  3. **Sección C** – Botones que muestran `Alert` indicando si la acción está permitida.
  4. **Tarjeta de depuración** – Badges con estado (`loading`, usuario, cada privilegio) y un `pre` con el diccionario. Cuando `privilegiosError` existe, muestra un `Alert` con el mensaje y el endpoint consultado.
- Ayuda a validar fin a fin que el backend entrega los privilegios esperados (en especial `CON`, que fue el caso reportado).

- `src/hooks/autorizacion/__tests__/usePrivilege.test.tsx`: monta el hook con un `AuthContext` falso y verifica los casos permitido/denegado.
- `src/modules/autorizacion/__tests__/EjemploAutorizacion.test.tsx`: asegura que los botones visibles corresponden con los privilegios y que el diálogo informa correctamente.
- Ejecutar `npm run test` para correr las suites, `npm run build` para el build de producción (último build exitoso mostrado en terminal `esbuild`).

### 4.4 Autenticación de frontend y rutas protegidas (no contemplado inicialmente)
- `src/features/auth/components/LoginForm.tsx`: dejó de usar `authApi`/Zustand y ahora inyecta `AuthService.login()`. Al autenticar, el backend entrega cookie y token; el formulario invoca `setUsuarioContext` para poblar `AuthContext` y redirigir al dashboard.
- `src/features/auth/pages/LoginPage.tsx`: utiliza `useAuth()` para detectar sesiones ya iniciadas y redirigir automáticamente.
- `src/routes/ProtectedRoute.tsx` + `src/routes/AppRoutes.tsx`: todo el árbol principal (`DashboardLayout`, `/sistema/**`, `/components/**`) está envuelto por un guard que exige `usuario`. Evita que se carguen páginas protegidas sin sesión y previene las llamadas anónimas a `/privilegios/self`.
- `src/layouts/DashboardLayout.tsx`, `src/layouts/components/Topbar.tsx`, `src/pages/Dashboard.tsx`: reemplazan el store legacy por `AuthContext`, muestran datos del usuario autenticado y ofrecen logout consistente (`setUsuario(null)` → clearing de cookies/caches).
- `src/api/authApi.ts`, `src/store/authStore.ts`, `src/hooks/useAuth.ts` (Zustand) fueron retirados, ya que duplicaban estado/token y no soportaban cookies.

> **Anotación retroactiva**: El plan original asumía que el store de autenticación ya estaba integrado con cookies. Al ejecutar el flujo real se detectó que el login guardaba un token en `localStorage` pero *no configuraba* la cookie que Spring Security requiere, permitiendo llegar al módulo de privilegios sin sesión. La actualización anterior alinea login, contexto y rutas con la estrategia basada en cookies descrita en el backend.

### 4.5 Manejo de errores y depuración
- Logs en consola: `AuthorizationService` (TS) imprime cada request/response; `AuthContext` loggea diccionarios y evaluaciones.
- UI muestra `privilegiosError` cuando la API responde 401/500 y expone el endpoint usado (`/api/v1/security/privilegios/self`).
- React Query usa `placeholderData` para evitar quedar con datos “fresh” después de un reload (corrección al bug donde `CON` desaparecía tras recargar).

## 5. Estrategia de invalidación y refresco
- Backend:
  - Llamar `authorizationService.evictUsuario(usuarioObjetivo)` inmediatamente después de modificar sus privilegios (por rol o asignación directa) para que la próxima petición repueble el cache.
  - Usar `POST /api/v1/security/privilegios/cache/flush` (solo administradores) para invalidar todo el cache cuando se hagan cambios masivos.
- Frontend:
  - `AuthContext.refreshPrivilegios()` expone `refetch()` de React Query; puede engancharse a botones “Actualizar privilegios” o a websockets que notifiquen cambios.
  - Tras login/logout se invalida `['privilegios']` automáticamente y se limpia el diccionario si el usuario es `null`.

## 6. Re-implementación en otro backend
1. **Replicar el modelo SQL**: portar `AuthorizationRepository` (o equivalente) respetando la unión de privilegios directos, roles y puestos.
2. **Agregar cache**: usar Caffeine (Java), Redis u otro mecanismo con TTL configurable y funciones `evictUsuario/All`.
3. **Exponer DTOs y endpoints**: clonar la estructura de `AuthorizationController` y documentarla en un OpenAPI (`docs/api-docs.json`).
4. **Integrar con seguridad**: implementar un filtro tipo `AuthorityPopulatorFilter` y un `PermissionEvaluator`/`AuthorizationManager` para que `@PreAuthorize` delegue en el servicio.
5. **Configurar propiedades externas**: definir un prefijo (`security.authorization`) y exponer TTL, tamaños y banderas en los `application-*.properties` del nuevo backend.
6. **Actualizar frontend**: apuntar `AuthorizationService` (TS) al nuevo dominio; si el contrato se mantiene idéntico, `AuthContext` y el módulo de ejemplo funcionarán sin cambios.
7. **Automatizar invalidación**: garantizar que los procesos que modifican privilegios invoquen `evictUsuario` o el endpoint de flush.

## 7. Referencia de archivos modificados
| Archivo | Descripción de cambios principales |
| --- | --- |
| `src/main/java/.../repository/AuthorizationRepository.java` | Nuevas consultas CTE/uniones para resolver privilegios y parámetros de versión. |
| `src/main/java/.../service/AuthorizationService.java` | Capa de negocio con cache Caffeine, helpers `hasPrivilege`, `getAuthoritiesForUsuario`, `evict*` y privilegios de ejemplo. |
| `src/main/java/.../controller/AuthorizationController.java` | REST completo de privilegios, logging detallado y endpoint de flush. |
| `src/main/java/.../security/AuthorityPopulatorFilter.java` | Población de autoridades por petición a partir del servicio. |
| `src/main/java/.../security/DomainPermissionMapper.java` | Convenciones `OBJETO_PRIVILEGIO` para mapear a authorities. |
| `src/main/java/.../security/PrivilegePermissionEvaluator.java` + `config/MethodSecurityConfiguration.java` | Soporte para expresiones `hasPermission` basadas en privilegios. |
| `src/main/java/.../config/SecurityConfig.java` | Registro del filtro de autoridades y habilitación de `@EnableMethodSecurity`. |
| `src/main/java/.../controller/LoginController.java` | Inicialización explícita de la sesión Spring Security para que el filtro pueda leer el usuario. |
| `src/main/resources/application.properties` y `application-dev.properties` | Propiedades `security.authorization.*` (TTL, tamaño, versiones, enforcement). |
| `docs/api-docs.json` | Documentación OpenAPI de todos los endpoints/DTOs de privilegios. |
| `frontend/src/lib/constants.ts` | Normaliza `API_BASE_URL`, `DEFAULT_SUCURSAL` y expone `buildApiUrl`. |
| `frontend/src/services/autorizacion/auth.service.ts` | Cliente `/api/v1/auth` con hashing, manejo de tokens/cookies y helpers `setUsuario/clearUsuario`. |
| `frontend/src/services/autorizacion/authorization.service.ts` | Cliente `/api/v1/security/privilegios`, logs, validación de payload y encoding. |
| `frontend/src/context/autorizacion/AuthContext.tsx` | Estado global de privilegios, gating por sesión, manejo de errores y React Query. |
| `frontend/src/hooks/autorizacion/usePrivilege.ts` | Hook delgado reutilizado por componentes. |
| `frontend/src/hooks/autorizacion/usePrivilegedActions.ts` | Agrupa múltiples privilegios y expone `allowed`/`isAllowed` para reutilización en UI. |
| `frontend/src/modules/autorizacion/EjemploAutorizacion.tsx` | UI demostrativa y tarjeta de depuración con errores/endpoint; consume `usePrivilegedActions` para poblar las tres secciones. |
| `frontend/src/hooks/autorizacion/__tests__/usePrivilege.test.tsx` y `frontend/src/modules/autorizacion/__tests__/EjemploAutorizacion.test.tsx` | Pruebas de privilegios y del módulo de ejemplo. |
| `frontend/src/features/auth/components/LoginForm.tsx` y `frontend/src/features/auth/pages/LoginPage.tsx` | Login basado en cookies que sincroniza `AuthContext`. |
| `frontend/src/routes/ProtectedRoute.tsx`, `src/routes/AppRoutes.tsx`, `src/layouts/**`, `src/pages/Dashboard.tsx` | Rutas protegidas y consumo del contexto (remplazan el store antiguo). |

## 8. Pruebas, build y verificación
- **Backend**: `./gradlew test` → unit/integration tests; `./gradlew build` → build completo (último resultado: éxito en terminal `java`).
- **Frontend**: `npm run test` → suites Vitest; `npm run build` → bundle de producción (último build exitoso en terminal `esbuild`, ~709 kB). Considerar `build.chunkSizeWarningLimit` si se desea eliminar la advertencia de tamaño.
- **Manual QA**: Inicio de sesión → módulo "Ejemplo autorización" → verificación de privilegios (`CON` debe permanecer tras recargar gracias a `placeholderData`). Revisar logs backend (`AuthorizationController`) para confirmar recepción de las llamadas.

---
Mantén este documento actualizado si se agregan nuevos objetos, endpoints o mecanismos de invalidación. Sirve como punto de partida para cualquier equipo que necesite reimplementar o auditar el subsistema de autorización.
