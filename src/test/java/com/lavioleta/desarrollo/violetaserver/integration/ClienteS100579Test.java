package com.lavioleta.desarrollo.violetaserver.integration;

import com.fasterxml.jackson.databind.ObjectMapper;
import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureWebMvc;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.test.context.ActiveProfiles;
import org.springframework.test.web.servlet.MockMvc;
import org.springframework.test.web.servlet.setup.MockMvcBuilders;
import org.springframework.transaction.annotation.Transactional;
import org.springframework.web.context.WebApplicationContext;

import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.*;
import static org.springframework.test.web.servlet.result.MockMvcResultHandlers.print;

/**
 * Test específico para debuggear el cliente S100579
 */
@SpringBootTest(webEnvironment = SpringBootTest.WebEnvironment.MOCK)
@AutoConfigureWebMvc
@ActiveProfiles("test")
class ClienteS100579Test {

    @Autowired
    private WebApplicationContext webApplicationContext;

    @Autowired
    private ObjectMapper objectMapper;

    private static final String CLIENTE_PRUEBA = "S100579";
    private static final String SUCURSAL_PRUEBA = "S1";

    @Test
    @Transactional
    void testConsultarClienteS100579() throws Exception {
        MockMvc mockMvc = MockMvcBuilders.webAppContextSetup(webApplicationContext).build();
        
        System.out.println("=== PROBANDO CLIENTE S100579 ===");
        
        // Intentar consultar el cliente específico
        mockMvc.perform(get("/api/v1/catalogos/clientes/{codigoCliente}", CLIENTE_PRUEBA)
                        .header("X-Sucursal", SUCURSAL_PRUEBA))
                .andDo(print())
                .andExpect(result -> {
                    int status = result.getResponse().getStatus();
                    String content = result.getResponse().getContentAsString();
                    
                    System.out.println("=== RESULTADO CONSULTA CLIENTE S100579 ===");
                    System.out.println("Status: " + status);
                    System.out.println("Content: " + content);
                    System.out.println("=== FIN RESULTADO ===");
                    
                    if (status == 200) {
                        System.out.println("✓ Cliente S100579 encontrado exitosamente");
                    } else if (status == 404) {
                        System.out.println("⚠ Cliente S100579 no encontrado (404)");
                    } else if (status == 500) {
                        System.err.println("❌ ERROR 500 al consultar cliente S100579");
                        System.err.println("Respuesta: " + content);
                    } else {
                        System.out.println("? Status inesperado: " + status);
                    }
                });
    }

    @Test
    @Transactional
    void testListarClientes() throws Exception {
        MockMvc mockMvc = MockMvcBuilders.webAppContextSetup(webApplicationContext).build();
        
        System.out.println("=== PROBANDO LISTAR CLIENTES ===");
        
        // Intentar listar clientes con paginación
        mockMvc.perform(get("/api/v1/catalogos/clientes")
                        .param("pagina", "1")
                        .param("registrosPorPagina", "5")
                        .header("X-Sucursal", SUCURSAL_PRUEBA))
                .andDo(print())
                .andExpect(result -> {
                    int status = result.getResponse().getStatus();
                    String content = result.getResponse().getContentAsString();
                    
                    System.out.println("=== RESULTADO LISTAR CLIENTES ===");
                    System.out.println("Status: " + status);
                    System.out.println("Content length: " + content.length());
                    if (content.length() < 1000) {
                        System.out.println("Content: " + content);
                    } else {
                        System.out.println("Content (first 500 chars): " + content.substring(0, 500) + "...");
                    }
                    System.out.println("=== FIN RESULTADO ===");
                    
                    if (status == 200) {
                        System.out.println("✓ Lista de clientes obtenida exitosamente");
                    } else {
                        System.err.println("❌ Error al listar clientes: " + status);
                    }
                });
    }

    @Test
    @Transactional 
    void testCheckClienteExistence() throws Exception {
        MockMvc mockMvc = MockMvcBuilders.webAppContextSetup(webApplicationContext).build();
        
        System.out.println("=== PROBANDO EXISTENCIA DE CLIENTE S100579 ===");
        
        // Verificar existencia del cliente
        mockMvc.perform(get("/api/v1/catalogos/clientes/status")
                        .param("cliente", CLIENTE_PRUEBA))
                .andDo(print())
                .andExpect(result -> {
                    int status = result.getResponse().getStatus();
                    String content = result.getResponse().getContentAsString();
                    
                    System.out.println("=== RESULTADO CHECK EXISTENCIA ===");
                    System.out.println("Status: " + status);
                    System.out.println("Content: " + content);
                    System.out.println("=== FIN RESULTADO ===");
                    
                    if (status == 200) {
                        System.out.println("✓ Check de existencia exitoso");
                    } else {
                        System.err.println("❌ Error en check de existencia: " + status);
                    }
                });
    }
}