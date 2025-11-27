# 📘 Guía de Tests - Búsqueda de Vendedores

## 📋 Contenido
1. [¿Qué se prueba?](#qué-se-prueba)
2. [Estructura de un Test](#estructura-de-un-test)
3. [Resultados y Reportes](#resultados-y-reportes)
4. [Solución de Problemas Comunes](#solución-de-problemas-comunes)


---

## 🎯 ¿Qué se prueba?

### Archivo de Test
- **Ubicación:** `src/test/java/.../integration/BusquedaVendedoresIntegrationTest.java`
- **Tipo:** Test de integración completo
- **Endpoint:** `POST /api/v1/busqueda/vendedores`

### Categorías de Tests (11 tests total)

| Categoría | Qué verifica | Ejemplo |
|-----------|--------------|---------|
| **Búsquedas** | Búsquedas por nombre, apellido, clave, comisión | NOM, APE, CLA, COMI |
| **Filtros** | Vendedores activos/inactivos | `soloActivos: true/false` |
| **Validaciones** | Rechaza datos inválidos (HTTP 400) | Tipo vacío, valor nulo |
| **Límites** | Respeta parámetro `limite` | Máximo 5 resultados |
| **Edge Cases** | Sin resultados, casos especiales | Lista vacía |

---

## 🔧 Estructura de un Test

### Patrón AAA (Arrange-Act-Assert)
```java
@Test
@DisplayName("Búsqueda por nombre - debe encontrar vendedor")
void busquedaPorNombre_DebeEncontrarVendedor() throws Exception {
    
    // 1. ARRANGE: Preparar datos de entrada
    BusquedaVendedoresRequest request = BusquedaVendedoresRequest.builder()
        .tipoBusqueda("NOM")
        .valor("CARLOS")
        .soloActivos(true)
        .limite(501)
        .build();
    
    // 2. ACT: Ejecutar petición HTTP simulada
    MvcResult result = mockMvc.perform(
        post("/api/v1/busqueda/vendedores")
            .contentType(MediaType.APPLICATION_JSON)
            .content(objectMapper.writeValueAsString(request))
    )
    
    // 3. ASSERT: Verificar respuesta
    .andExpect(status().isOk())  // HTTP 200
    .andExpect(jsonPath("$.success").value(true))
    .andExpect(jsonPath("$.vendedores").isArray())
    .andReturn();
}
```

### Herramientas Clave

**MockMvc:** Simula peticiones HTTP sin servidor real  
**ObjectMapper:** Convierte objetos ↔ JSON  
**JSONPath:** Navega campos del JSON (`$.vendedores[0].nombre`)  
**AssertJ:** Verificaciones legibles (`assertThat(...).isNotEmpty()`)

---

## 📊 Resultados y Reportes


### Ubicación de Archivos
```
📁 build/reports/tests/test/
├── index.html              ← Resumen general (ABRE ESTE)
├── classes/                ← Detalle por clase
└── packages/               ← Detalle por paquete

📁 build/test-results/test/
└── TEST-*.xml              ← Formato para CI/CD
```

### Interpretación Rápida

**Resumen (index.html):**
```
Tests: 11  |  Failures: 8  |  Success: 27%  |  Duration: 8s
```

**Estados:**
- ✅ **PASSED:** Test exitoso
- ❌ **FAILED:** Test falló - revisar detalle
- ⏭️ **IGNORED:** Test deshabilitado

### Error Típico
```
❌ Status expected:<200> but was:<400>

Causa: Base de datos MariaDB en localhost:3308 no disponible
Los tests intentan conectarse pero fallan
```

**Solución:** Iniciar MariaDB o usar H2 en memoria para tests

---

## 🚀 Comandos Rápidos

```bash
# Ejecutar tests
gradle test

# Ver reporte
start build\reports\tests\test\index.html

# Ejecutar solo esta clase
gradle test --tests "BusquedaVendedoresIntegrationTest"

# Limpiar y ejecutar
gradle clean test
```

---

## 📚 Referencias Clave

- **JUnit 5:** Framework de tests
- **MockMvc:** Simula peticiones HTTP
- **JSONPath:** Navega JSON (`$.campo`)
- **AssertJ:** Verificaciones legibles

---

**Última actualización:** 27 de noviembre de 2025  
