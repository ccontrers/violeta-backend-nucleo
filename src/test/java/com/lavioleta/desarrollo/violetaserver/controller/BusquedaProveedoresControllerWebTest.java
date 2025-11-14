package com.lavioleta.desarrollo.violetaserver.controller;

import com.fasterxml.jackson.databind.ObjectMapper;
import com.lavioleta.desarrollo.violetaserver.config.TestSecurityConfig;
import com.lavioleta.desarrollo.violetaserver.dto.request.BusquedaProveedoresRequest;
import com.lavioleta.desarrollo.violetaserver.dto.response.BusquedaProveedoresResponse;
import com.lavioleta.desarrollo.violetaserver.service.BusquedaProveedoresService;
import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;
import org.springframework.boot.test.autoconfigure.web.servlet.WebMvcTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.context.annotation.Import;
import org.springframework.http.MediaType;
import org.springframework.test.context.ActiveProfiles;
import org.springframework.test.web.servlet.MockMvc;

import java.util.Arrays;
import java.util.List;

import static org.hamcrest.Matchers.*;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.when;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.get;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.post;
import static org.springframework.test.web.servlet.result.MockMvcResultHandlers.print;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.*;

/**
 * Tests de capa web para BusquedaProveedoresController.
 * Valida endpoints, serialización JSON, manejo de errores HTTP y validaciones.
 */
@WebMvcTest(BusquedaProveedoresController.class)
@Import(TestSecurityConfig.class)
@AutoConfigureMockMvc(addFilters = false)
@ActiveProfiles("test")
@DisplayName("Web Layer: BusquedaProveedoresController")
class BusquedaProveedoresControllerWebTest {

    @Autowired
    private MockMvc mockMvc;

    @Autowired
    private ObjectMapper objectMapper;

    @MockBean
    private BusquedaProveedoresService busquedaProveedoresService;

    @Nested
    @DisplayName("POST /api/v1/busqueda/proveedores - Casos Exitosos")
    class CasosExitosos {

        @Test
        @DisplayName("Debería realizar búsqueda exitosamente")
        void busqueda_deberiaRealizarBusquedaExitosamente() throws Exception {
            // ARRANGE
            BusquedaProveedoresRequest request = crearRequest("RSO", "ACME");
            BusquedaProveedoresResponse response = crearRespuestaExitosa();

            when(busquedaProveedoresService.buscarProveedores(any(BusquedaProveedoresRequest.class)))
                .thenReturn(response);

            // ACT & ASSERT
            mockMvc.perform(post("/api/v1/busqueda/proveedores")
                    .contentType(MediaType.APPLICATION_JSON)
                    .content(objectMapper.writeValueAsString(request)))
                .andDo(print())
                .andExpect(status().isOk())
                .andExpect(content().contentType(MediaType.APPLICATION_JSON))
                .andExpect(jsonPath("$.success", is(true)))
                .andExpect(jsonPath("$.totalResultados", is(2)))
                .andExpect(jsonPath("$.proveedores", hasSize(2)))
                .andExpect(jsonPath("$.proveedores[0].proveedor", is("PROV001")))
                .andExpect(jsonPath("$.proveedores[0].razonsocial", is("ACME Corporation S.A. de C.V.")));
        }

        @Test
        @DisplayName("Debería retornar respuesta sin resultados")
        void busqueda_deberiaRetornarSinResultados() throws Exception {
            // ARRANGE
            BusquedaProveedoresRequest request = crearRequest("RSO", "NoExiste");
            BusquedaProveedoresResponse response = BusquedaProveedoresResponse.builder()
                .success(true)
                .message("No se encontraron proveedores")
                .totalResultados(0)
                .proveedores(List.of())
                .build();

            when(busquedaProveedoresService.buscarProveedores(any(BusquedaProveedoresRequest.class)))
                .thenReturn(response);

            // ACT & ASSERT
            mockMvc.perform(post("/api/v1/busqueda/proveedores")
                    .contentType(MediaType.APPLICATION_JSON)
                    .content(objectMapper.writeValueAsString(request)))
                .andDo(print())
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.success", is(true)))
                .andExpect(jsonPath("$.totalResultados", is(0)))
                .andExpect(jsonPath("$.proveedores", hasSize(0)));
        }
    }

    @Nested
    @DisplayName("POST /api/v1/busqueda/proveedores - Validaciones")
    class Validaciones {

        @Test
        @DisplayName("Debería fallar con codcondicion inválido")
        void validation_deberiaFallarConCodcondicionInvalido() throws Exception {
            // ARRANGE
            BusquedaProveedoresRequest request = crearRequest("INVALIDO", "ACME");

            // ACT & ASSERT
            mockMvc.perform(post("/api/v1/busqueda/proveedores")
                    .contentType(MediaType.APPLICATION_JSON)
                    .content(objectMapper.writeValueAsString(request)))
                .andDo(print())
                .andExpect(status().isBadRequest());
        }

        @Test
        @DisplayName("Debería aceptar request válido mínimo")
        void validation_deberiaAceptarRequestMinimo() throws Exception {
            // ARRANGE
            String minimalJson = """
                {
                    "codcondicion": "RSO",
                    "condicion": "ACME"
                }
                """;

            BusquedaProveedoresResponse response = BusquedaProveedoresResponse.builder()
                .success(true)
                .message("Búsqueda completada")
                .totalResultados(0)
                .proveedores(List.of())
                .build();

            when(busquedaProveedoresService.buscarProveedores(any(BusquedaProveedoresRequest.class)))
                .thenReturn(response);

            // ACT & ASSERT
            mockMvc.perform(post("/api/v1/busqueda/proveedores")
                    .contentType(MediaType.APPLICATION_JSON)
                    .content(minimalJson))
                .andDo(print())
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.success", is(true)));
        }
    }

    @Nested
    @DisplayName("POST /api/v1/busqueda/proveedores - Manejo de Errores")
    class ManejoErrores {

        @Test
        @DisplayName("Debería manejar JSON malformado")
        void error_deberiaManejarJsonMalformado() throws Exception {
            // ARRANGE
            String malformedJson = """
                {
                    "codcondicion": "RSO"
                    "condicion": "ACME"
                }
                """;

            // ACT & ASSERT
            mockMvc.perform(post("/api/v1/busqueda/proveedores")
                    .contentType(MediaType.APPLICATION_JSON)
                    .content(malformedJson))
                .andDo(print())
                .andExpect(status().isBadRequest());
        }

        @Test
        @DisplayName("Debería manejar método HTTP incorrecto")
        void error_deberiaManejarMetodoIncorrecto() throws Exception {
            // ACT & ASSERT
            mockMvc.perform(get("/api/v1/busqueda/proveedores"))
                .andDo(print())
                .andExpect(status().isMethodNotAllowed());
        }
    }

    // Métodos auxiliares
    private BusquedaProveedoresRequest crearRequest(String codcondicion, String condicion) {
        return BusquedaProveedoresRequest.builder()
            .codcondicion(codcondicion)
            .condicion(condicion)
            .build();
    }

    private BusquedaProveedoresResponse crearRespuestaExitosa() {
        return BusquedaProveedoresResponse.builder()
            .success(true)
            .message("Búsqueda completada exitosamente")
            .totalResultados(2)
            .proveedores(Arrays.asList(
                crearProveedorResultado("PROV001", "ACME Corporation S.A. de C.V.", "ACM123456789"),
                crearProveedorResultado("PROV002", "Distribuidora Beta S.A.", "BET987654321")
            ))
            .build();
    }

    private BusquedaProveedoresResponse.ProveedorResultado crearProveedorResultado(
            String proveedor, String razonSocial, String rfc) {
        return BusquedaProveedoresResponse.ProveedorResultado.builder()
            .proveedor(proveedor)
            .razonsocial(razonSocial)
            .rfc(rfc)
            .estado("CDMX")
            .localidad("Ciudad de México")
            .activo(true)
            .provgastos(true)
            .provmercancia(false)
            .build();
    }
}