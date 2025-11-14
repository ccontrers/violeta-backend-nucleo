# API de Búsqueda de Artículos

## Descripción

Esta API fue convertida desde código C++ (`ClassServidorBusquedas.cpp`) a Spring Boot usando Spring JDBC Client. Proporciona funcionalidades avanzadas de búsqueda de artículos con múltiples criterios.

## Arquitectura

### Capas implementadas:
- **Controller**: `EjemploBusquedaArticulosController` - Manejo de requests HTTP
- **Service**: `EjemploBusquedaArticulosService` - Lógica de negocio
- **Repository**: `EjemploBusquedaArticulosRepository` - Acceso a datos con Spring JDBC Client
- **DTOs**: Request/Response objects para transferencia de datos

### Tecnologías:
- Spring Boot 3.5.4
- Spring JDBC Client (moderna alternativa a JdbcTemplate)
- MariaDB 11.2.6
- Java 21

## Endpoint Principal

**URL**: `POST /api/v1/ejemplo/busqueda/articulos`

## Estructura de Request

```java
public class EjemploBusquedaArticulosRequest {
    private String sucursal;           // Código de sucursal
    private String mostrarExistencias; // "SI" o "NO"
    private String codcondicion;       // Tipo de búsqueda
    private String filas;              // Límite de resultados
    private String condicion;          // Término de búsqueda
}
```

## Tipos de Búsqueda (codcondicion)

| Código | Descripción | Ejemplo de condicion |
|--------|-------------|---------------------|
| `N` | Por nombre | "aceite", "leche" |
| `C` | Por código de artículo | "ACE001", "LAC001" |
| `M` | Por marca | "CAPULLO", "NATURA" |
| `E` | Por clasificación | "ACEITES", "LACTEOS" |
| `CB` | Por código de barras | "7501234567890" |
| `ART` | Por artículo específico | "ACE001" |
| `""` | Obtener catalogos | n/a |

## Estructura de Response

```java
public class EjemploBusquedaArticulosResponse {
    private boolean success;
    private String message;
    private int totalResultados;
    private List<ArticuloResultado> articulos;
    private List<ClasificacionResultado> clasificaciones;
    private List<MarcaResultado> marcas;
}
```

### ArticuloResultado
```java
public class ArticuloResultado {
    private String nombre;         // Nombre del artículo
    private String presentacion;   // Presentación (ej: "1 LT")
    private int existencia;        // Existencia actual
    private double precio;         // Precio de venta
    private String marca;          // Código de marca
    private String nombreMarca;    // Nombre de la marca
    private String producto;       // Código de producto
    private String articulo;       // Código de artículo
    private String codigoBarras;   // Código de barras
    private int sucursal;          // Sucursal
}
```

### ClasificacionResultado
```java
public class ClasificacionResultado {
    private String codigo;  // Código de clasificación
    private String nombre;  // Nombre de clasificación
}
```

### MarcaResultado
```java
public class MarcaResultado {
    private String codigo;  // Código de marca
    private String nombre;  // Nombre de marca
}
```

## Lógica de Negocio

### Flujo de ejecución:

1. **Validación de request**
   - Verifica parámetros obligatorios
   - Valida tipos de búsqueda

2. **Verificación de parámetros de sucursal**
   - Consulta tabla `parametrosemp` para configuración de existencias

3. **Ejecución según tipo de búsqueda**:
   - **Sin tipo**: Retorna clasificaciones y marcas disponibles
   - **Con tipo**: Ejecuta búsqueda específica en tabla `articulos`

4. **Construcción de response**
   - Mapea resultados a DTOs
   - Calcula totales
   - Maneja errores

### Queries principales:

```sql
-- Verificar parámetros de sucursal
SELECT valor FROM parametrosemp 
WHERE parametro='EXISTENCIASPV' AND sucursal = ?

-- Búsqueda por nombre
SELECT a.nombre, a.descripcion, e.existencia, e.precio, 
       a.marca, m.nombre as nombreMarca, a.articulo, 
       a.codigobarras
FROM articulos a
LEFT JOIN marcas m ON a.marca = m.marca
LEFT JOIN existencias e ON a.articulo = e.articulo AND e.sucursal = ?
WHERE a.activo = true AND UPPER(a.nombre) LIKE UPPER(?)
ORDER BY a.nombre
LIMIT ?

-- Obtener clasificaciones
SELECT clasif1, nombre FROM clasificacion1 ORDER BY nombre

-- Obtener marcas
SELECT marca, nombre FROM marcas ORDER BY nombre
```

## Casos de Uso

### 1. Búsqueda de productos para punto de venta
```json
{
    "sucursal": "1",
    "mostrarExistencias": "SI",
    "codcondicion": "N",
    "filas": "10",
    "condicion": "aceite"
}
```

### 2. Búsqueda por código de barras en TPV
```json
{
    "sucursal": "1", 
    "mostrarExistencias": "SI",
    "codcondicion": "CB",
    "filas": "1",
    "condicion": "7501234567890"
}
```

### 3. Cargar catálogos para filtros
```json
{
    "sucursal": "1",
    "mostrarExistencias": "NO", 
    "codcondicion": "",
    "filas": "0",
    "condicion": ""
}
```

## Códigos de Error

| Código HTTP | Descripción | Ejemplo |
|-------------|-------------|---------|
| 200 | Éxito | Búsqueda completada |
| 400 | Bad Request | Tipo de búsqueda no válido |
| 500 | Error interno | Error de base de datos |

## Base de Datos

### Tablas requeridas:
- `parametrosemp` - Parámetros por sucursal
- `articulos` - Catálogo de artículos
- `marcas` - Catálogo de marcas
- `clasificacion1` - Clasificaciones de productos
- `existencias` - Inventario por sucursal

### Script de creación:
Ejecutar: `c:\Github\violetaserver\src\main\resources\db\migration\V001__create_example_tables.sql`

## Testing

### Tests implementados:
- `EjemploBusquedaArticulosRepositoryTest` - Tests de repositorio
- `EjemploBusquedaArticulosServiceTest` - Tests de service con mocks
- `EjemploBusquedaArticulosControllerTest` - Tests de controller
- `EjemploBusquedaArticulosIntegrationTest` - Tests de integración

### Ejecutar tests:
```bash
.\gradlew test --tests "*EjemploBusquedaArticulos*"
```

## Configuración

### application.properties
```properties
# Base de datos
spring.datasource.url=jdbc:mariadb://localhost:3308/catunifabastos
spring.datasource.username=violetausr
spring.datasource.password=1212
spring.datasource.driver-class-name=org.mariadb.jdbc.Driver

# JPA/Hibernate
spring.jpa.hibernate.ddl-auto=none
spring.jpa.show-sql=false
spring.jpa.properties.hibernate.format_sql=true

# Logging
logging.level.org.springframework.jdbc=DEBUG
logging.level.com.lavioleta.desarrollo.violetaserver=INFO
```

## Notas de Migración C++ → Spring Boot

### Cambios principales:
1. **Strings C++** → **Strings Java**: Manejo de encoding UTF-8
2. **Punteros C++** → **Referencias Java**: Gestión automática de memoria
3. **SQL nativo** → **Spring JDBC Client**: Mapeo automático de resultados
4. **Estructuras C++** → **DTOs Java**: Serialización JSON automática
5. **Manejo de errores C++** → **Exception handling Spring**: Logging estructurado

### Beneficios obtenidos:
- ✅ Arquitectura en capas bien definida
- ✅ Inyección de dependencias
- ✅ Manejo automático de transacciones
- ✅ Serialización JSON automática
- ✅ Testing automatizado
- ✅ Logging estructurado
- ✅ Documentación de API
- ✅ Manejo centralizado de excepciones

## Monitoring y Logs

### Logs importantes:
```
INFO  - EjemploBusquedaArticulosController : Recibida solicitud de búsqueda
INFO  - EjemploBusquedaArticulosServiceImpl : Iniciando búsqueda de artículos
DEBUG - JdbcTemplate : Executing prepared SQL query
ERROR - EjemploBusquedaArticulosRepository : Error al verificar parámetro
```

### Métricas disponibles:
- Tiempo de respuesta por tipo de búsqueda
- Número de resultados retornados
- Errores por sucursal
- Uso de cache de clasificaciones/marcas
