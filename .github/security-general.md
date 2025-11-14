# Seguridad general — reglas obligatorias

## Objetivos
- Prevenir SQL injection; validar y sanitizar entradas.
- Logging y auditoría mínima en eventos críticos.
- Manejo de errores sin filtrar información sensible.

## Recomendaciones concretas
- DTOs con anotaciones de validación (`@NotNull`, `@Size`, `@Pattern`).
- QueryBuilder + `jdbcClient.param(...)` para todas las consultas.
- No exponer stack traces en producción; logs con niveles apropiados.
- Registrar cambios importantes (creación/actualización/eliminación) con usuario/sucursal.

## Autenticación híbrida (cookie + JWT)
- `/api/v1/auth/login` mantiene sesión clásica y devuelve `JSESSIONID`; los clientes deben enviar cookies (`withCredentials=true`).
- `/api/v1/auth/login/jwt` devuelve un `token` JWT (HS256); el cliente debe mandar `Authorization: Bearer <token>` en cada request.
- Ambos esquemas están activos. Endpoints protegidos aceptan cualquiera; documenta ambos (`cookieAuth`, `bearerAuth`) en OpenAPI.
- Scripts útiles:
  - `login_test.ps1 -PasswordPlano` calcula hash, obtiene el token y lo guarda en `$env:AUTH_TOKEN`.
  - `consulta_proveedor.ps1` consume `/api/catalogo/proveedores/{clave}` con ese token.
- El token expira en 4 h (`jwt.expiration`). Automatizaciones deben renovarlo a tiempo.
- En el perfil `test` los filtros se deshabilitan automáticamente; usa `@ActiveProfiles("test")` en pruebas.

## Ejemplo — patrón obligatorio
```java
String sql = queryBuilder.build();
return jdbcClient.sql(sql)
  .param(param1)
  .param(param2)
  .query(this::mapRow)
  .list();
```
---

## Anexo: Testing and Debugging

````markdown
# Testing and Debugging

## Backend
- Enable SQL logs at TRACE as needed
- Use `@Slf4j` and structured logs with context
- Profiles: dev, test, prod

### Unit Testing (Backend)
```java
@ExtendWith(MockitoExtension.class)
class EjemploBusquedaArticulosServiceTest {
  @Mock private EjemploBusquedaArticulosRepository repository;
  @InjectMocks private EjemploBusquedaArticulosServiceImpl service;
  // Arrange/Act/Assert...
}
```

## Frontend
- Use React Testing Library
- Validate rendering after API calls and user interactions

### Unit Testing (Frontend)
```ts
import { render, screen, fireEvent, waitFor } from '@testing-library/react'
// Example: expects result to appear after search
```

## API Testing Workflow
1. Terminal 1: run server via Gradle (don’t touch during tests)
2. Terminal 2: use PowerShell `Invoke-RestMethod` examples to exercise endpoints
3. If connection fails, restart server manually

````

---

## Anexo: PowerShell API Testing Examples

````markdown
# PowerShell API Testing Examples (Windows)

Use `Invoke-RestMethod` instead of curl on Windows.

## Important
- Run API tests in a separate terminal from the server
- Use `;` to chain commands in PowerShell (not `&&`)

## Examples
```powershell
$headers = @{'Content-Type' = 'application/json'}
$body = '{"sucursal":"S1","mostrarExistencias":"SI","codcondicion":"E","filas":"20","condicion":"PAPELERIA"}'
$response = Invoke-RestMethod -Uri 'http://localhost:5986/api/v1/ejemplo/busqueda/articulos' -Method POST -Headers $headers -Body $body
$response | ConvertTo-Json -Depth 10

# Save result
$response | ConvertTo-Json -Depth 10 | Out-File -FilePath "resultado_api.json" -Encoding UTF8

# By name
$body = '{"sucursal":"S1","mostrarExistencias":"SI","codcondicion":"N","filas":"20","condicion":"aceite"}'
$response = Invoke-RestMethod -Uri 'http://localhost:5986/api/v1/ejemplo/busqueda/articulos' -Method POST -Headers $headers -Body $body
$response | ConvertTo-Json -Depth 10

# Catalogs
$body = '{"sucursal":"S1","mostrarExistencias":"NO","codcondicion":"","filas":"20","condicion":""}'
$response = Invoke-RestMethod -Uri 'http://localhost:5986/api/v1/ejemplo/busqueda/articulos' -Method POST -Headers $headers -Body $body
$response | ConvertTo-Json -Depth 10
```
````