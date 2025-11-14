package com.lavioleta.desarrollo.violetaserver.integration;

import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.test.context.ActiveProfiles;
import org.springframework.test.context.TestPropertySource;

/**
 * Test simple de carga de contexto de Spring
 */
@SpringBootTest(webEnvironment = SpringBootTest.WebEnvironment.MOCK)
@ActiveProfiles("test")
@TestPropertySource(properties = {
    "spring.datasource.url=jdbc:h2:mem:testdb",
    "spring.datasource.driver-class-name=org.h2.Driver",
    "spring.datasource.username=sa", 
    "spring.datasource.password=",
    "spring.jpa.database-platform=org.hibernate.dialect.H2Dialect",
    "spring.jpa.hibernate.ddl-auto=create-drop",
    "spring.h2.console.enabled=true",
    "spring.jpa.show-sql=false",
    "logging.level.org.hibernate.SQL=WARN"
})
@DisplayName("Tests de carga de contexto")
class ContextLoadIntegrationTest {

    @Test
    @DisplayName("El contexto de Spring debe cargar correctamente")
    void contextLoads() {
        // Este test pasa si el contexto de Spring se carga sin errores
        // Es útil para verificar configuración básica
    }
}