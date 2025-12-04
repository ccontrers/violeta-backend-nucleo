# Plan backend para replicar el subsistema de autorización

> Usa este plan junto con `spec-autorizacion-privilegios.md`, el contrato filtrado `docs/api-auth-privilegios.json` y el resumen operativo de `README.md`. Los archivos de referencia viven en `autorizacionback/src/main/java/com/lavioleta/desarrollo/violetaserver/**` y pueden copiarse o adaptarse según el nuevo repositorio.

## 1. Objetivo
Recrear únicamente la parte backend del subsistema de privilegios: repositorio JDBC, servicio con cache Caffeine, integración con Spring Security y endpoints REST `/api/v1/security/privilegios/**`, manteniendo el comportamiento descrito en la especificación.

> Estado actual: el módulo vive en `src/main/java/com/lavioleta/desarrollo/violetaserver/autorizacion/**` con controller, service, repository, DTOs, security y config conectados a Spring Security.

## 2. Artefactos de referencia
- Repositorio: `autorizacionback/.../repository/AuthorizationRepository.java`
- Servicio + cache: `autorizacionback/.../service/AuthorizationService.java`
- DTOs: `autorizacionback/.../dto/security/*.java`
- Configuración: `autorizacionback/.../config/AuthorizationProperties.java` + `application*.properties`
- Seguridad: `autorizacionback/.../security/{AuthorityPopulatorFilter,DomainPermissionMapper,PrivilegePermissionEvaluator}.java`
- Method security handler: `autorizacionback/.../config/MethodSecurityConfiguration.java`
- Controller: `autorizacionback/.../controller/AuthorizationController.java`

## 3. Supuestos previos
1. Existe autenticación previa (JWT, sesión o híbrido) que puebla `SecurityContextHolder` con el usuario.
2. Las tablas listadas en la spec §3.1 están disponibles (privilegios directos, roles, puestos, parámetros de versión).
3. El proyecto usa Spring Boot 3, Lombok, `spring-jdbc`, `spring-security`, `jakarta.validation` y Caffeine (`com.github.ben-manes.caffeine`).
4. Hay forma de exponer propiedades externas por perfil (`application.properties`, `application-*.properties`).

## 4. Roadmap resumido
| Fase | Objetivo | Referencias |
| --- | --- | --- |
| 1. Datos | Replicar SQL UNION + DTOs | Repository + DTOs |
| 2. Servicio | Cacheo, helpers `hasPrivilege`, autoridades | AuthorizationService, DomainPermissionMapper |
| 3. Seguridad | Filtro `AuthorityPopulatorFilter`, `PrivilegePermissionEvaluator`, method security | security/*.java, MethodSecurityConfiguration |
| 4. API | Endpoints REST + invalidación cache | AuthorizationController |
| 5. Config & QA | Propiedades, perfiles, pruebas | application*.properties, README.md, `./gradlew test` |

## 5. Checklist detallado
### 5.1 Acceso a datos y DTOs
1. Copia `AuthorizationRepository.java` y ajusta el `JdbcClient`/datasource al nuevo contexto.
2. Conserva la plantilla SQL (`PRIVILEGE_SQL_TEMPLATE`) con CTEs para privilegios directos, roles y puestos.
3. Verifica que `findVersionRequirements()` lea los parámetros correctos o adapta los nombres según tu esquema.
4. Replica los DTOs (`PrivilegioDTO`, `ObjetoPrivilegiosDTO`, `PrivilegeCheckResponse`, `VersionRequirementDTO`) para mantener el contrato con el frontend y las pruebas.

### 5.2 Servicio y cache
1. Porta `AuthorizationService` tal cual y registra el bean en tu contexto Spring.
2. Asegúrate de tener la dependencia de Caffeine y revisa que los valores `security.authorization.cache.*` existan en `AuthorizationProperties`.
3. Conserva `ensureExamplePrivilegios` si deseas mantener `EJEMPLOAUT`; puedes eliminarlo si no es necesario, pero actualiza el spec.
4. Exponer métodos públicos: `getPrivilegios`, `getPrivilegiosAgrupados`, `hasPrivilege`, `getAuthoritiesForUsuario`, `revisarPrivilegio`, `evictUsuario`, `evictAll`, `getVersionRequirements`.

### 5.3 Integración con Spring Security
1. Registra `DomainPermissionMapper` como componente para estandarizar el formato `OBJETO_PRIVILEGIO`.
2. Añade `AuthorityPopulatorFilter` al `SecurityFilterChain` **después** de `UsernamePasswordAuthenticationFilter` para que cada request authenticated reciba todas las autoridades dinámicas.
3. Declara `PrivilegePermissionEvaluator` como bean y vincúlalo con `MethodSecurityConfiguration` para permitir expresiones `@PreAuthorize("hasPermission('OBJ','PRIV')")`.
4. Si usas JWT y sesiones, recuerda conservar los pasos del spec (§3.5) para poblar el contexto durante el login.

### 5.4 API REST
1. Expone `AuthorizationController` bajo `/api/v1/security/privilegios` y mantén sincronizado `docs/api-auth-privilegios.json`.
2. Endpoints obligatorios:
   - `GET /self` y `GET /` → `List<ObjetoPrivilegiosDTO>`.
   - `GET /{objeto}` → `List<PrivilegioDTO>` (404 si vacío).
   - `GET /{objeto}/{privilegio}` → `PrivilegeCheckResponse`.
   - `GET /version/requerimientos` → `VersionRequirementDTO`.
   - `POST /cache/flush` → invalida cache global; protege con `@PreAuthorize("hasRole('ADMIN') or hasAuthority('SECURITY_CACHE_FLUSH')")`.
   - `DELETE /cache/{usuario}` → invalida cache individual para procesos administrativos.
3. Mantén el logging (`log.info`) para usuario/objeto/privilegio y la validación de `Principal`.

### 5.5 Configuración externa y perfiles
1. Copia `AuthorizationProperties` y registra `@ConfigurationProperties(prefix = "security.authorization")`.
2. Replica las claves en `autorizacionback/src/main/resources/application.properties` y ajusta los valores por perfil (ver `application-dev.properties`).
3. Si tu proyecto usa otro sistema de configuración (K8s, Consul), documenta la equivalencia de estas claves.

### 5.6 Operación e invalidación
1. Expón utilidades administrativas que llamen `AuthorizationService.evictUsuario` al modificar privilegios; documenta el flujo.
2. Documenta `POST /api/v1/security/privilegios/cache/flush` para invalidaciones masivas.
3. Define métricas/logs para medir tasas de cache hit (puede instrumentarse alrededor del `privilegeCache`).

### 5.7 QA y verificación
1. Ejecuta `./gradlew test` y agrega pruebas unitarias para `AuthorizationService` (cache, enforcement flag) y `AuthorizationController` (MVC tests) si tu proyecto aún no las tiene.
2. Verifica manualmente el flujo:
   - Login → request autenticada
   - `GET /api/v1/security/privilegios/self`
   - Endpoint protegido con `@PreAuthorize("hasAuthority('OBJ_PRIV')")`
3. Si existe frontend, confirma que consume los mismos DTOs; de lo contrario, provee clientes o SDKs.

## 6. Checklist de cierre
- [ ] SQL replicado y probado (consulta completa de privilegios y parámetros de versión).
- [ ] Servicio y cache funcionando con propiedades externas.
- [ ] Filtro de autoridades y `PrivilegePermissionEvaluator` registrados en `SecurityConfig`/`MethodSecurityConfiguration`.
- [ ] Endpoints REST operativos y documentados (`docs/api-auth-privilegios.json` + README).
- [ ] Procedimientos de invalidación (`evictUsuario`, `cache/flush`) documentados y probados.
- [ ] Pruebas automatizadas + verificación manual ejecutadas sin errores.

> Una vez completado este checklist, actualiza `docs/spec-autorizacion-privilegios.md` con cualquier ajuste local y referencia la ubicación final de los artefactos en tu repositorio.
