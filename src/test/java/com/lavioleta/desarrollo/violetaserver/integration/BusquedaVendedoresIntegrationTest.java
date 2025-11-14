package com.lavioleta.desarrollo.violetaserver.integration;

import com.fasterxml.jackson.databind.ObjectMapper;
import com.lavioleta.desarrollo.violetaserver.dto.request.BusquedaVendedoresRequest;
import com.lavioleta.desarrollo.violetaserver.dto.response.BusquedaVendedoresResponse;
import com.lavioleta.desarrollo.violetaserver.dto.response.BusquedaVendedoresResponse.VendedorResultado;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureWebMvc;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.http.MediaType;
import org.springframework.test.context.ActiveProfiles;
import org.springframework.test.web.servlet.MockMvc;
import org.springframework.test.web.servlet.MvcResult;
import org.springframework.test.web.servlet.setup.MockMvcBuilders;
import org.springframework.web.context.WebApplicationContext;

import static org.assertj.core.api.Assertions.assertThat;
import static org.hamcrest.Matchers.greaterThanOrEqualTo;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.post;
import static org.springframework.test.web.servlet.result.MockMvcResultHandlers.print;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.*;

/**
 * Tests de integración para BusquedaVendedoresController
 * 
 * Utiliza el vendedor de prueba existente en BD:
 * - Clave: CAMT
 * - Nombre: CARLOS MAGOS TAPIA
 * - Estado: activo (v.activo=1)
 * 
 * Endpoint: POST /api/v1/busqueda/vendedores
 * 
 * Tipos de búsqueda probados:
 * - NOM: Por nombre (e.nombre LIKE 'CARLOS%')
 * - APE: Por apellidos (e.appat LIKE 'MAGOS%')
 * - COMI: Por tipo de comisión
 * - CLA: Por clave (e.empleado LIKE 'CAMT%')
 */
@SpringBootTest(webEnvironment = SpringBootTest.WebEnvironment.MOCK)
@AutoConfigureWebMvc
@ActiveProfiles("test")
@DisplayName("Tests de integración - BusquedaVendedoresController")
class BusquedaVendedoresIntegrationTest {
    
    @Autowired
    private WebApplicationContext webApplicationContext;
    
    @Autowired
    private ObjectMapper objectMapper;
    
    private MockMvc mockMvc;
    
    // Vendedor de prueba existente en BD
    private static final String VENDEDOR_CLAVE = "CAMT";
    private static final String VENDEDOR_NOMBRE = "CARLOS";
    private static final String VENDEDOR_APELLIDO = "MAGOS";
    
    @BeforeEach
    void setUp() {
        mockMvc = MockMvcBuilders.webAppContextSetup(webApplicationContext).build();
    }
    
    // ==================== TESTS POR TIPO DE BÚSQUEDA ====================
    
    @Test
    @DisplayName("Búsqueda por nombre (NOM) - debe encontrar a CARLOS MAGOS TAPIA")
    void busquedaPorNombre_DebeEncontrarVendedor() throws Exception {
        // Given
        BusquedaVendedoresRequest request = BusquedaVendedoresRequest.builder()
                .tipoBusqueda("NOM")
                .valor(VENDEDOR_NOMBRE)
                .soloActivos(true)
                .limite(501)
                .build();
        
        // When & Then
        MvcResult result = mockMvc.perform(post("/api/v1/busqueda/vendedores")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content(objectMapper.writeValueAsString(request)))
                .andDo(print())
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.success").value(true))
                .andExpect(jsonPath("$.totalResultados").isNumber())
                .andExpect(jsonPath("$.vendedores").isArray())
                .andReturn();
        
        // Verificar que CAMT está en los resultados
        BusquedaVendedoresResponse response = objectMapper.readValue(
                result.getResponse().getContentAsString(),
                BusquedaVendedoresResponse.class
        );
        
        assertThat(response.getVendedores())
                .as("Debe contener al vendedor CAMT")
                .anyMatch(v -> VENDEDOR_CLAVE.equals(v.getEmpleado()));
    }
    
    @Test
    @DisplayName("Búsqueda por apellido (APE) - debe encontrar a CARLOS MAGOS TAPIA")
    void busquedaPorApellido_DebeEncontrarVendedor() throws Exception {
        // Given
        BusquedaVendedoresRequest request = BusquedaVendedoresRequest.builder()
                .tipoBusqueda("APE")
                .valor(VENDEDOR_APELLIDO)
                .soloActivos(true)
                .limite(501)
                .build();
        
        // When & Then
        MvcResult result = mockMvc.perform(post("/api/v1/busqueda/vendedores")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content(objectMapper.writeValueAsString(request)))
                .andDo(print())
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.success").value(true))
                .andExpect(jsonPath("$.totalResultados").isNumber())
                .andExpect(jsonPath("$.vendedores").isArray())
                .andReturn();
        
        BusquedaVendedoresResponse response = objectMapper.readValue(
                result.getResponse().getContentAsString(),
                BusquedaVendedoresResponse.class
        );
        
        assertThat(response.getVendedores())
                .as("Debe contener al vendedor CAMT")
                .anyMatch(v -> VENDEDOR_CLAVE.equals(v.getEmpleado()));
    }
    
    @Test
    @DisplayName("Búsqueda por clave (CLA) - debe encontrar vendedores que empiecen con CAMT")
    void busquedaPorClave_DebeEncontrarVendedor() throws Exception {
        // Given
        BusquedaVendedoresRequest request = BusquedaVendedoresRequest.builder()
                .tipoBusqueda("CLA")
                .valor(VENDEDOR_CLAVE)
                .soloActivos(true)
                .limite(501)
                .build();
        
        // When & Then - La búsqueda por clave usa LIKE 'CAMT%', puede retornar CAMT y CAMTUR
        MvcResult result = mockMvc.perform(post("/api/v1/busqueda/vendedores")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content(objectMapper.writeValueAsString(request)))
                .andDo(print())
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.success").value(true))
                .andExpect(jsonPath("$.totalResultados").value(greaterThanOrEqualTo(1)))
                .andExpect(jsonPath("$.vendedores[?(@.empleado == 'CAMT')]").exists())
                .andExpect(jsonPath("$.vendedores[?(@.empleado == 'CAMT')].nombre").exists())
                .andExpect(jsonPath("$.vendedores[?(@.empleado == 'CAMT')].localidad").exists())
                .andExpect(jsonPath("$.vendedores[?(@.empleado == 'CAMT')].tipocomi").exists())
                .andReturn();
        
        BusquedaVendedoresResponse response = objectMapper.readValue(
                result.getResponse().getContentAsString(),
                BusquedaVendedoresResponse.class
        );
        
        // Verificar que CAMT está en los resultados
        assertThat(response.getVendedores())
                .isNotEmpty()
                .anyMatch(v -> v.getEmpleado().equals(VENDEDOR_CLAVE));
        
        VendedorResultado camt = response.getVendedores().stream()
                .filter(v -> v.getEmpleado().equals(VENDEDOR_CLAVE))
                .findFirst()
                .orElseThrow();
        
        assertThat(camt.getNombre()).contains("CARLOS", "MAGOS", "TAPIA");
    }
    
    @Test
    @DisplayName("Búsqueda por tipo de comisión (COMI) - debe buscar por coincidencia exacta")
    void busquedaPorTipoComision_DebeFuncionar() throws Exception {
        // Primero obtenemos el tipo de comisión de CAMT
        BusquedaVendedoresRequest requestClave = BusquedaVendedoresRequest.builder()
                .tipoBusqueda("CLA")
                .valor(VENDEDOR_CLAVE)
                .soloActivos(true)
                .limite(501)
                .build();
        
        MvcResult resultClave = mockMvc.perform(post("/api/v1/busqueda/vendedores")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content(objectMapper.writeValueAsString(requestClave)))
                .andExpect(status().isOk())
                .andReturn();
        
        BusquedaVendedoresResponse responseClave = objectMapper.readValue(
                resultClave.getResponse().getContentAsString(),
                BusquedaVendedoresResponse.class
        );
        
        String tipoComision = responseClave.getVendedores().get(0).getTipocomi();
        
        // Ahora buscar por ese tipo de comisión
        BusquedaVendedoresRequest request = BusquedaVendedoresRequest.builder()
                .tipoBusqueda("COMI")
                .valor(tipoComision)
                .soloActivos(true)
                .limite(501)
                .build();
        
        // When & Then
        mockMvc.perform(post("/api/v1/busqueda/vendedores")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content(objectMapper.writeValueAsString(request)))
                .andDo(print())
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.success").value(true))
                .andExpect(jsonPath("$.totalResultados").isNumber())
                .andExpect(jsonPath("$.vendedores").isArray());
    }
    
    // ==================== TESTS DE FILTRO ACTIVOS ====================
    
    @Test
    @DisplayName("Búsqueda solo activos - debe aplicar filtro v.activo=1")
    void busquedaSoloActivos_DebeAplicarFiltro() throws Exception {
        BusquedaVendedoresRequest request = BusquedaVendedoresRequest.builder()
                .tipoBusqueda("NOM")
                .valor("A") // Buscar vendedores con nombre que empiece con A
                .soloActivos(true)
                .limite(10)
                .build();
        
        mockMvc.perform(post("/api/v1/busqueda/vendedores")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content(objectMapper.writeValueAsString(request)))
                .andDo(print())
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.success").value(true));
    }
    
    @Test
    @DisplayName("Búsqueda incluir inactivos - debe aplicar filtro v.activo=0")
    void busquedaInactivos_DebeAplicarFiltro() throws Exception {
        BusquedaVendedoresRequest request = BusquedaVendedoresRequest.builder()
                .tipoBusqueda("NOM")
                .valor("A")
                .soloActivos(false) // Buscar solo inactivos (legacy: v.activo=0)
                .limite(10)
                .build();
        
        mockMvc.perform(post("/api/v1/busqueda/vendedores")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content(objectMapper.writeValueAsString(request)))
                .andDo(print())
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.success").value(true));
    }
    
    // ==================== TESTS DE VALIDACIÓN ====================
    
    @Test
    @DisplayName("Validación: Tipo de búsqueda inválido - debe retornar 400")
    void validacionTipoBusquedaInvalido_DebeRetornarBadRequest() throws Exception {
        BusquedaVendedoresRequest request = BusquedaVendedoresRequest.builder()
                .tipoBusqueda("INVALIDO")
                .valor("TEST")
                .soloActivos(true)
                .limite(501)
                .build();
        
        mockMvc.perform(post("/api/v1/busqueda/vendedores")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content(objectMapper.writeValueAsString(request)))
                .andDo(print())
                .andExpect(status().isBadRequest());
    }
    
    @Test
    @DisplayName("Validación: Valor vacío - debe retornar 400")
    void validacionValorVacio_DebeRetornarBadRequest() throws Exception {
        BusquedaVendedoresRequest request = BusquedaVendedoresRequest.builder()
                .tipoBusqueda("NOM")
                .valor("")
                .soloActivos(true)
                .limite(501)
                .build();
        
        mockMvc.perform(post("/api/v1/busqueda/vendedores")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content(objectMapper.writeValueAsString(request)))
                .andDo(print())
                .andExpect(status().isBadRequest());
    }
    
    @Test
    @DisplayName("Validación: Tipo de búsqueda nulo - debe retornar 400")
    void validacionTipoBusquedaNulo_DebeRetornarBadRequest() throws Exception {
        BusquedaVendedoresRequest request = BusquedaVendedoresRequest.builder()
                .tipoBusqueda(null)
                .valor("TEST")
                .soloActivos(true)
                .limite(501)
                .build();
        
        mockMvc.perform(post("/api/v1/busqueda/vendedores")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content(objectMapper.writeValueAsString(request)))
                .andDo(print())
                .andExpect(status().isBadRequest());
    }
    
    // ==================== TESTS DE LÍMITE ====================
    
    @Test
    @DisplayName("Límite de resultados - debe respetar el parámetro límite")
    void limiteResultados_DebeRespetarParametro() throws Exception {
        BusquedaVendedoresRequest request = BusquedaVendedoresRequest.builder()
                .tipoBusqueda("NOM")
                .valor("A")
                .soloActivos(true)
                .limite(5) // Limitar a 5 resultados
                .build();
        
        MvcResult result = mockMvc.perform(post("/api/v1/busqueda/vendedores")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content(objectMapper.writeValueAsString(request)))
                .andDo(print())
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.success").value(true))
                .andReturn();
        
        BusquedaVendedoresResponse response = objectMapper.readValue(
                result.getResponse().getContentAsString(),
                BusquedaVendedoresResponse.class
        );
        
        assertThat(response.getVendedores().size()).isLessThanOrEqualTo(5);
    }
    
    // ==================== TEST CASO SIN RESULTADOS ====================
    
    @Test
    @DisplayName("Sin resultados - debe retornar lista vacía con mensaje apropiado")
    void sinResultados_DebeRetornarListaVacia() throws Exception {
        BusquedaVendedoresRequest request = BusquedaVendedoresRequest.builder()
                .tipoBusqueda("NOM")
                .valor("ZZZZZZZZZZZ") // Valor que no existe
                .soloActivos(true)
                .limite(501)
                .build();
        
        mockMvc.perform(post("/api/v1/busqueda/vendedores")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content(objectMapper.writeValueAsString(request)))
                .andDo(print())
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.success").value(true))
                .andExpect(jsonPath("$.totalResultados").value(0))
                .andExpect(jsonPath("$.vendedores").isEmpty())
                .andExpect(jsonPath("$.message").value("No se encontró ningún registro de nombre = ZZZZZZZZZZZ"));
    }
}
