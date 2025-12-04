# Guía de uso del API de autorización

Esta guía resume cómo interactuar con los servicios REST ubicados en `/api/v1/security/privilegios/**`. El módulo expone los privilegios disponibles para el usuario autenticado, permite verificarlos en tiempo real y ofrece mecanismos administrativos para invalidar la caché de privilegios.

## Requisitos previos
- Autenticación válida (JWT o sesión). Todos los endpoints requieren un `Authorization: Bearer <token>` o una sesión activa, excepto donde se indique lo contrario.
- El usuario debe existir en la base de datos y tener privilegios cargados según la lógica de asignación (directos, roles, puestos).
- Para las operaciones de invalidación de caché se exige `ROLE_ADMIN` o la autoridad `SECURITY_CACHE_FLUSH`.

## Resumen de endpoints
| Método | Ruta | Descripción | Autorización |
| --- | --- | --- | --- |
| GET | `/api/v1/security/privilegios` | Lista todos los objetos y privilegios del usuario actual. | Usuario autenticado |
| GET | `/api/v1/security/privilegios/self` | Alias del anterior, útil para clientes que prefieren un sufijo explícito. | Usuario autenticado |
| GET | `/api/v1/security/privilegios/{objeto}` | Detalle de privilegios para el objeto indicado. | Usuario autenticado |
| GET | `/api/v1/security/privilegios/{objeto}/{privilegio}` | Verificación puntual (true/false). | Usuario autenticado |
| GET | `/api/v1/security/privilegios/version/requerimientos` | Versión mínima requerida por el backend. | Usuario autenticado |
| POST | `/api/v1/security/privilegios/cache/flush` | Invalidación global de caché. | `ROLE_ADMIN` o `SECURITY_CACHE_FLUSH` |
| DELETE | `/api/v1/security/privilegios/cache/{usuario}` | Invalidación de caché para un usuario específico. | `ROLE_ADMIN` o `SECURITY_CACHE_FLUSH` |

## Payloads y respuestas
### Objeto `ObjetoPrivilegiosDTO`
```json
{
  "objeto": "VENTAS",
  "privilegios": ["CON", "ALT"]
}
```

### Objeto `PrivilegioDTO`
```json
{
  "objeto": "VENTAS",
  "privilegio": "CON",
  "descripcion": "Consulta de ventas"
}
```

### Objeto `PrivilegeCheckResponse`
```json
{
  "objeto": "VENTAS",
  "privilegio": "CON",
  "allowed": true
}
```

### Objeto `VersionRequirementDTO`
```json
{
  "versionMinima": "1.0",
  "subversionMinima": "0",
  "tiempoValidezMinutos": 15
}
```

## Ejemplos
Los ejemplos utilizan `curl` con un token ficticio `token-demo`. Reemplaza con el valor real.

### 1. Listar privilegios agrupados
```bash
curl -H "Authorization: Bearer token-demo" \
     http://localhost:6820/api/v1/security/privilegios
```
Respuesta:
```json
[
  {
    "objeto": "VENTAS",
    "privilegios": ["CON", "ALT", "CAN"]
  },
  {
    "objeto": "COMPRAS",
    "privilegios": ["CON"]
  }
]
```

### 2. Listar privilegios de un objeto
```bash
curl -H "Authorization: Bearer token-demo" \
     http://localhost:6820/api/v1/security/privilegios/VENTAS
```
Respuesta 200:
```json
[
  {"objeto": "VENTAS", "privilegio": "CON", "descripcion": "Consulta ventas"},
  {"objeto": "VENTAS", "privilegio": "ALT", "descripcion": "Alta de ventas"}
]
```
Respuesta 404 (sin privilegios): cuerpo vacío.

### 3. Verificar un privilegio puntual
```bash
curl -H "Authorization: Bearer token-demo" \
     http://localhost:6820/api/v1/security/privilegios/VENTAS/ALT
```
Respuesta:
```json
{"objeto": "VENTAS", "privilegio": "ALT", "allowed": true}
```

### 4. Obtener requisitos de versión
```bash
curl -H "Authorization: Bearer token-demo" \
     http://localhost:6820/api/v1/security/privilegios/version/requerimientos
```
Respuesta:
```json
{"versionMinima": "1.0", "subversionMinima": "0", "tiempoValidezMinutos": 15}
```

### 5. Invalidar caché global (administrativo)
```bash
curl -X POST \
     -H "Authorization: Bearer token-admin" \
     http://localhost:6820/api/v1/security/privilegios/cache/flush
```
Respuesta: `202 Accepted`

### 6. Invalidar caché para un usuario
```bash
curl -X DELETE \
     -H "Authorization: Bearer token-admin" \
     http://localhost:6820/api/v1/security/privilegios/cache/CRCP
```
Respuesta: `204 No Content`

## Buenas prácticas
- Consumir `GET /privilegios` al iniciar sesión en el frontend para precargar menús y permisos.
- Usar `GET /{objeto}/{privilegio}` cuando se requiera validar acciones específicas sin transferir toda la lista.
- Después de modificar privilegios en la base de datos, invocar `DELETE /cache/{usuario}` para que el cambio sea efectivo de inmediato.
- Registrar cada consumo administrativo; estos endpoints impactan a todos los nodos de la aplicación mediante la invalidez del caché.
