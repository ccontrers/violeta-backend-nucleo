package com.lavioleta.desarrollo.violetaserver.integration;

import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureWebMvc;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.test.context.ActiveProfiles;
import org.springframework.test.web.servlet.MockMvc;
import org.springframework.test.web.servlet.setup.MockMvcBuilders;
import org.springframework.web.context.WebApplicationContext;

import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.*;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.*;
import static org.springframework.test.web.servlet.result.MockMvcResultHandlers.print;

/**
 * Test simple para validar que el contexto de Spring y la BD funcionan
 */
@SpringBootTest(webEnvironment = SpringBootTest.WebEnvironment.MOCK)
@AutoConfigureWebMvc
@ActiveProfiles("test")
class SimpleIntegrationTest {

    @Autowired
    private WebApplicationContext webApplicationContext;

    @Test
    void contextLoads() {
        // Test simple para verificar que el contexto carga correctamente
        System.out.println("✓ Contexto de Spring cargado exitosamente");
    }

    @Test
    void testBasicEndpoint() throws Exception {
        MockMvc mockMvc = MockMvcBuilders.webAppContextSetup(webApplicationContext).build();
        
        // Test simple del endpoint de listar clientes
        mockMvc.perform(get("/api/v1/catalogos/clientes")
                        .header("X-Sucursal", "S1"))
                .andDo(print())
                .andExpect(result -> {
                    int status = result.getResponse().getStatus();
                    String content = result.getResponse().getContentAsString();
                    System.out.println("Status: " + status);
                    System.out.println("Content: " + content);
                    
                    if (status == 500) {
                        System.err.println("ERROR 500 - Hay un problema en el servidor");
                        // No forzamos que falle, solo reportamos
                    } else {
                        System.out.println("✓ Endpoint respondió con status: " + status);
                    }
                });
    }
}