# Estrategia de Pruebas (Testing Overview)

Este documento resume la estrategia actual de pruebas del proyecto `violetaserver` tras la unificación y limpieza de suites legacy.

## ⚠️ **Estado Crítico: Tests Unitarios Faltantes**

**URGENTE:** El proyecto carece de tests unitarios para la lógica de negocio crítica. Esta es una necesidad de **alta prioridad** que debe ser atendida inmediatamente.

### Tests Faltantes Críticos
- **❌ Repository Tests:** Validación de preservación de campos, normalización automática, manejo de FK constraints
- **❌ Service Tests:** Lógica de negocio, validaciones custom, transformaciones de datos
- **❌ DTO Validation Tests:** Verificación de `@AssertTrue` rules, validaciones complejas
- **❌ Utility Tests:** Funciones de normalización, mapeo de datos

### Impacto de la Ausencia de Tests
- **Regresiones no detectadas:** Cambios que rompen funcionalidad existente
- **Refactoring riesgoso:** Sin red de seguridad para cambios de código
- **Debugging complejo:** Errores difíciles de aislar sin tests granulares
- **Documentación viva faltante:** Tests como especificación ejecutable

### Ejemplo de Test Crítico Faltante
```java
@Test
void actualizarCliente_deberiaPreservarCamposNoModificados() {
    // ARRANGE: Cliente existente con digitosdef=""
    ClienteRequest request = new ClienteRequest();
    request.setCliente("S100579");
    request.setNomnegocio("NUEVO NOMBRE");
    // digitosdef NO incluido en request
    
    // ACT: Actualizar solo nombre comercial
    repository.actualizarCliente(request, "S100579");
    
    // ASSERT: digitosdef debe mantener valor original ""
    Cliente resultado = repository.consultarPorCodigo("S100579");
    assertThat(resultado.getDigitosdef()).isEqualTo(""); // NO "0000"
}
```

---

## Capas de Pruebas

1. Pruebas Unitarias (Service / lógica pura)
   - Objetivo: Validar reglas de negocio sin involucrar infraestructura.
   - Ubicación: `src/test/java/.../service/`
   - Herramientas: JUnit 5, Mockito.

2. Pruebas de Capa Web (Controller aislado)
   - Archivo principal: `CatalogoClientesControllerWebTest.java`
   - Anotación: `@WebMvcTest(CatalogoClientesController.class)`
   - Objetivo: Verificar mapeo de endpoints, códigos HTTP, validaciones y traducción de excepciones.
   - Uso de `MockMvc` + `@MockBean` (pendiente migrar a configuración de test cuando se elimine la anotación en futuras versiones de Spring Boot).

3. Pruebas de Integración
   - Ubicación: `src/test/java/.../integration/`
   - Objetivo: Validar flujos end-to-end con base de datos reales (perfil `20250910_multiempresa`).
   - Incluye escenarios con datos reales (ej. cliente `S100579`).

## Manejo de Errores HTTP (Post-Refactor)

| Situación                              | Código | Origen Handler / Lógica                |
|---------------------------------------|--------|----------------------------------------|
| Validación Bean Validation            | 400    | `MethodArgumentNotValidException`      |
| JSON malformado                       | 400    | `HttpMessageNotReadableException`      |
| Media type no soportado               | 415    | `HttpMediaTypeNotSupportedException`   |
| Recurso (ruta) inexistente            | 404    | `NoResourceFound` (Spring)             |
| `success=false` (lógica negocio)      | 400/404| Controlador decide (400 general / 404 consulta) |
| Error inesperado (Runtime)            | 500    | `RuntimeException` handler             |
| Fallback genérico                     | 500    | Handler global                         |

## DTO Unificado de Errores (ApiError)

Todos los errores gestionados por el `GlobalExceptionHandler` ahora retornan un JSON uniforme:

Campos:
- `timestamp` (ISO-8601, `Instant`)
- `status` (int HTTP)
- `error` (razón estándar: "Bad Request", "Not Found", etc.)
- `message` (descripción legible orientada a cliente)
- `path` (URI de la solicitud)
- `method` (HTTP method)
- `errorCode` (código estable interno para lógica de frontend / i18n)
- `fieldErrors` (lista de errores de validación campo a campo, opcional)
- `traceId` (para correlación si se añade un filtro MDC futuro)

Ejemplo (validación):
```json
{
   "timestamp": "2025-09-22T16:41:20.512Z",
   "status": 400,
   "error": "Bad Request",
   "message": "Error de validación",
   "path": "/api/v1/catalogos/clientes",
   "method": "POST",
   "errorCode": "VALIDATION_ERROR",
   "fieldErrors": [
      { "field": "rfc", "rejectedValue": "ABC", "message": "RFC inválido" }
   ],
   "traceId": null
}
```

### Tabla de `errorCode` actuales

| errorCode              | HTTP | Descripción                                      |
|------------------------|------|--------------------------------------------------|
| `VALIDATION_ERROR`     | 400  | Violaciones Bean Validation                      |
| `MALFORMED_JSON`       | 400  | Cuerpo JSON ilegible / parse error               |
| `UNSUPPORTED_MEDIA_TYPE`| 415 | Content-Type no soportado                        |
| `RESOURCE_NOT_FOUND`   | 404  | Ruta inexistente (NoResourceFound)               |
| `RUNTIME_EXCEPTION`    | 500  | Excepción en tiempo de ejecución no controlada   |
| `UNEXPECTED_ERROR`     | 500  | Cualquier excepción no mapeada específica        |

Planeados futuros (negocio):
- `CLIENTE_NO_ENCONTRADO` (404 lógico en lugar de success=false)
- `CLIENTE_DUPLICADO` (409 si se añade unicidad estricta)
- `CREDITO_NO_ENCONTRADO` (404 para módulo crédito)

### Ventajas
- Contrato estable para frontend (puede mapear `errorCode` a mensajes i18n).
- Tests más expresivos: se valida semántica (`$.errorCode`).
- Facilidad de agregar metadatos (`traceId`, `docsLink`).
- Minimiza dependencia de mensajes literales (evita fragilidad por redacción).

### Extensión rápida
Para agregar un nuevo código:
1. Definir constante semántica (`errorCode`).
2. Añadir handler específico o lógica en servicio que lance excepción custom.
3. Ajustar test con `jsonPath("$.errorCode").value("NUEVO_CODIGO")`.
4. Documentar en esta tabla.

### Mejora futura sugerida
Implementar un filtro (`OncePerRequestFilter`) que genere un UUID y lo ponga en `MDC` como `traceId` para poblar ese campo y correlacionar logs.

## Convenciones en Tests Web

- Agrupación por dominio mediante `@Nested`.
- Nombres descriptivos: `Crear cliente datos inválidos -> 400`.
- Primero se prueba el camino feliz (200) y luego variantes de error.
- Se evita mezclar pruebas de múltiples capas en un mismo archivo.
- Se mockea `obtenerEmpresaPorSucursal` solo una vez en `@BeforeEach`.

## Eliminación de Tests Legacy

Los archivos antiguos:
- `CatalogoClientesControllerTest.java`
- `CatalogoClientesControllerTestFixed.java`

Fueron vaciados y ya no aportan valor (su historia queda en git). Se recomienda borrarlos físicamente en una próxima limpieza si la herramienta lo permite.

## Comandos Útiles

Ejecutar toda la suite:
```powershell
./gradlew test --no-daemon
```

Forzar limpieza de resultados previos:
```powershell
./gradlew cleanTest test --no-daemon
```

Ver reporte HTML:
```
build/reports/tests/test/index.html
```

## Próximos Pasos Recomendados

### **PRIORIDAD 1: Implementar Tests Unitarios Faltantes (URGENTE)**

#### **A. Repository Tests**
```java
// src/test/java/.../repository/CatalogoClientesRepositoryTest.java
@DataJpaTest
@AutoConfigureTestDatabase(replace = AutoConfigureTestDatabase.Replace.NONE)
class CatalogoClientesRepositoryTest {
    
    @Test
    void actualizarCliente_deberiaPreservarDigitosDef_cuandoNoSeEnvian() {
        // Test preservación de campos críticos
    }
    
    @Test
    void insertarCliente_deberiaAsignarCodigoAutomatico() {
        // Test generación automática de códigos
    }
    
    @Test
    void upsertDatosEmpresa_deberiaConvertirVaciosANull_paraFKs() {
        // Test manejo FK constraints
    }
}
```

#### **B. DTO Validation Tests**
```java
// src/test/java/.../dto/ClienteRequestValidationTest.java
class ClienteRequestValidationTest {
    
    @Test
    void personaMoral_sinRazonSocial_deberiaFallar() {
        // Test @AssertTrue isNombreORazonValido()
    }
    
    @Test
    void metodoPagoInvalido_deberiaFallar() {
        // Test normalización métodos pago
    }
}
```

#### **C. Service Tests**
```java
// src/test/java/.../service/CatalogoClientesServiceTest.java
@ExtendWith(MockitoExtension.class)
class CatalogoClientesServiceTest {
    
    @Test
    void grabarCliente_deberiaValidarRFCDuplicado() {
        // Test lógica de negocio duplicados
    }
}
```

#### **Cronograma Sugerido:**
- **Semana 1:** Repository tests (preservación campos, FK constraints)
- **Semana 2:** DTO validation tests (reglas negocio complejas)
- **Semana 3:** Service tests (validaciones duplicados, transacciones)

### **PRIORIDAD 2: Expansión Tests Existentes**

1. Tests de paginación (parámetros fuera de rango -> 400).
2. Validaciones adicionales de longitud/formato (RFC, razón social) + asserts sobre mensajes.
3. Migrar de `@MockBean` a configuración dedicada de beans de prueba si Spring Boot elimina soporte.
4. Cobertura de negociación de contenido (`Accept` inválido -> 406 si se habilita).
5. Incorporar nuevos `errorCode` de negocio conforme se formalicen reglas.

## Buenas Prácticas Adoptadas
- Separación clara: web vs integración.
- Uso de datos válidos para llaves foráneas en requests simulados.
- Semántica HTTP consistente y documentada.
- Tests con nombres que describen intención y resultado esperado.

## Checklist Rápido al Agregar un Nuevo Endpoint
- [ ] ¿Tiene test camino feliz (200)?
- [ ] ¿Cubre validaciones (400)?
- [ ] ¿Cubre recurso no encontrado (404) si aplica?
- [ ] ¿Cubre error inesperado (500)?
- [ ] ¿Se documentó semántica HTTP específica si difiere del patrón?

## **Checklist para Nueva Funcionalidad (OBLIGATORIO)**
- [ ] **¿Tiene tests unitarios de repository?** (lógica datos, preservación campos)
- [ ] **¿Tiene tests de validación DTO?** (reglas negocio, @AssertTrue)  
- [ ] **¿Tiene tests de service?** (lógica negocio, transacciones)
- [ ] **¿Tiene tests E2E de integración?** (flujo completo)
- [ ] **¿Tests cubren casos edge?** (FK constraints, datos inválidos)

---
Si necesitas que automatice la creación del DTO `ApiError` y ajuste del `GlobalExceptionHandler`, indícalo y preparo el cambio junto con nuevos asserts.
