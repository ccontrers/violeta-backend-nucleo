package com.lavioleta.desarrollo.violetaserver.controller;

import static org.mockito.ArgumentMatchers.any;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.post;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.jsonPath;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.status;

import java.time.LocalDate;
import java.time.LocalTime;
import java.util.List;

import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;
import org.springframework.boot.test.autoconfigure.web.servlet.WebMvcTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.http.MediaType;
import org.springframework.test.context.ActiveProfiles;
import org.springframework.test.web.servlet.MockMvc;

import com.fasterxml.jackson.databind.ObjectMapper;
import com.lavioleta.desarrollo.violetaserver.bitacora.controller.BitacoraUnificadaController;
import com.lavioleta.desarrollo.violetaserver.bitacora.dto.request.BitacoraUnificadaRequest;
import com.lavioleta.desarrollo.violetaserver.bitacora.dto.response.BitacoraUnificadaResponse;
import com.lavioleta.desarrollo.violetaserver.bitacora.dto.response.BitacoraUnificadaResponse.BitacoraUsuarioEntry;
import com.lavioleta.desarrollo.violetaserver.bitacora.service.BitacoraUnificadaService;

@WebMvcTest(BitacoraUnificadaController.class)
@AutoConfigureMockMvc(addFilters = false)
@ActiveProfiles("test")
@DisplayName("Web Layer: BitacoraUnificadaController")
class BitacoraUnificadaControllerWebTest {

    private static final String BASE = "/api/v1/bitacora-unificada";

    @Autowired
    MockMvc mockMvc;

    @Autowired
    ObjectMapper objectMapper;

    @MockBean
    BitacoraUnificadaService service;

    private BitacoraUnificadaRequest validRequest;

    @BeforeEach
    void setUp() {
        validRequest = BitacoraUnificadaRequest.builder()
                .fechaInicio(LocalDate.of(2025, 1, 1))
                .fechaFin(LocalDate.of(2025, 1, 31))
                .usuario("USR001")
            .sucursal("S1")
                .build();
    }

    @Nested
    @DisplayName("POST /api/v1/bitacora-unificada")
    class ConsultarBitacora {

        @Test
        @DisplayName("Consulta con resultados -> 200")
        void consultaConResultados() throws Exception {
            BitacoraUsuarioEntry entry = BitacoraUsuarioEntry.builder()
                    .referencia("VTA0001")
                    .tipoDocumento("VENT")
                    .operacion("ALTA")
                    .cancelado(false)
                    .fechaDocumento(LocalDate.of(2025, 1, 5))
                    .usuario("USR001")
                    .fechaOperacion(LocalDate.of(2025, 1, 5))
                    .horaOperacion(LocalTime.of(10, 15, 30))
                    .build();
            BitacoraUnificadaResponse response = BitacoraUnificadaResponse.success("Consulta", List.of(entry));

            org.mockito.Mockito.when(service.consultar(any(BitacoraUnificadaRequest.class))).thenReturn(response);

            mockMvc.perform(post(BASE)
                            .contentType(MediaType.APPLICATION_JSON)
                            .content(objectMapper.writeValueAsString(validRequest)))
                    .andExpect(status().isOk())
                    .andExpect(jsonPath("$.success").value(true))
                    .andExpect(jsonPath("$.eventos[0].referencia").value("VTA0001"));
        }

        @Test
        @DisplayName("Consulta sin coincidencias -> 200")
        void consultaSinResultados() throws Exception {
            BitacoraUnificadaResponse response = BitacoraUnificadaResponse.success("Sin registros", List.of());
            org.mockito.Mockito.when(service.consultar(any(BitacoraUnificadaRequest.class))).thenReturn(response);

                BitacoraUnificadaRequest requestSinSucursal = BitacoraUnificadaRequest.builder()
                    .fechaInicio(LocalDate.of(2025, 1, 1))
                    .fechaFin(LocalDate.of(2025, 1, 31))
                    .usuario("USR001")
                    .build();

                mockMvc.perform(post(BASE)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content(objectMapper.writeValueAsString(requestSinSucursal)))
                    .andExpect(status().isOk())
                    .andExpect(jsonPath("$.success").value(true))
                    .andExpect(jsonPath("$.eventos.length()").value(0));
        }

        @Test
        @DisplayName("Validación de parámetros obligatorios -> 400")
        void consultaParametrosInvalidos() throws Exception {
            BitacoraUnificadaRequest invalid = BitacoraUnificadaRequest.builder()
                    .fechaInicio(LocalDate.of(2025, 1, 1))
                    .build();

            mockMvc.perform(post(BASE)
                            .contentType(MediaType.APPLICATION_JSON)
                            .content(objectMapper.writeValueAsString(invalid)))
                    .andExpect(status().isBadRequest())
                    .andExpect(jsonPath("$.errorCode").value("VALIDATION_ERROR"));
        }
    }
}
