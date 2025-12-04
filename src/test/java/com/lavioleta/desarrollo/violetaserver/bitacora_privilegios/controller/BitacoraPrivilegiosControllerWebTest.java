package com.lavioleta.desarrollo.violetaserver.bitacora_privilegios.controller;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.when;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.post;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.jsonPath;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.status;

import java.time.LocalDate;
import java.time.LocalTime;
import java.util.Collections;
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
import com.lavioleta.desarrollo.violetaserver.bitacora_privilegios.dto.request.BitacoraPrivilegiosRequest;
import com.lavioleta.desarrollo.violetaserver.bitacora_privilegios.dto.response.BitacoraPrivilegiosResponse;
import com.lavioleta.desarrollo.violetaserver.bitacora_privilegios.dto.response.BitacoraPrivilegiosResponse.BitacoraPrivilegioEntry;
import com.lavioleta.desarrollo.violetaserver.bitacora_privilegios.service.BitacoraPrivilegiosService;

/**
 * Tests de capa web para BitacoraPrivilegiosController.
 * Valida comportamiento HTTP y validaciones de request.
 */
@WebMvcTest(BitacoraPrivilegiosController.class)
@AutoConfigureMockMvc(addFilters = false)
@ActiveProfiles("test")
@DisplayName("Web Layer: BitacoraPrivilegiosController")
class BitacoraPrivilegiosControllerWebTest {

    private static final String BASE = "/api/v1/bitacora-privilegios";

    @Autowired
    MockMvc mockMvc;

    @Autowired
    ObjectMapper objectMapper;

    @MockBean
    BitacoraPrivilegiosService service;

    private BitacoraPrivilegiosRequest validRequest;
    private BitacoraPrivilegiosResponse successResponse;
    private BitacoraPrivilegioEntry sampleEntry;

    @BeforeEach
    void setUp() {
        validRequest = BitacoraPrivilegiosRequest.builder()
                .fechaInicio(LocalDate.of(2025, 1, 1))
                .fechaFin(LocalDate.of(2025, 1, 31))
                .build();

        sampleEntry = BitacoraPrivilegioEntry.builder()
                .fecha(LocalDate.of(2025, 1, 15))
                .hora(LocalTime.of(10, 30, 0))
                .usuarioOperador("Carlos Contreras Pérez")
                .usuarioModificado("María García López")
                .tipoOperacion("MODIFICACIÓN")
                .entidadInvolucrada("PRIVILEGIO")
                .nombreEntidad("VENTAS_CON")
                .contexto("USUARIOS")
                .build();

        successResponse = BitacoraPrivilegiosResponse.success(
                "Se encontraron 1 registros",
                List.of(sampleEntry));
    }

    @Nested
    @DisplayName("POST /api/v1/bitacora-privilegios")
    class ConsultarBitacora {

        @Test
        @DisplayName("Consulta básica con fechas válidas -> 200")
        void consultaBasicaExitosa() throws Exception {
            when(service.consultar(any(BitacoraPrivilegiosRequest.class))).thenReturn(successResponse);

            mockMvc.perform(post(BASE)
                            .contentType(MediaType.APPLICATION_JSON)
                            .content(objectMapper.writeValueAsString(validRequest)))
                    .andExpect(status().isOk())
                    .andExpect(jsonPath("$.success").value(true))
                    .andExpect(jsonPath("$.eventos").isArray())
                    .andExpect(jsonPath("$.eventos[0].usuarioOperador").value("Carlos Contreras Pérez"));
        }

        @Test
        @DisplayName("Consulta con filtro de usuario CRCP -> 200")
        void consultaConFiltroUsuario() throws Exception {
            BitacoraPrivilegiosRequest requestConUsuario = BitacoraPrivilegiosRequest.builder()
                    .fechaInicio(LocalDate.of(2025, 1, 1))
                    .fechaFin(LocalDate.of(2025, 1, 31))
                    .usuario("CRCP")
                    .build();

            when(service.consultar(any(BitacoraPrivilegiosRequest.class))).thenReturn(successResponse);

            mockMvc.perform(post(BASE)
                            .contentType(MediaType.APPLICATION_JSON)
                            .content(objectMapper.writeValueAsString(requestConUsuario)))
                    .andExpect(status().isOk())
                    .andExpect(jsonPath("$.success").value(true));
        }

        @Test
        @DisplayName("Consulta con filtro de rol -> 200")
        void consultaConFiltroRol() throws Exception {
            BitacoraPrivilegioEntry entryRol = BitacoraPrivilegioEntry.builder()
                    .fecha(LocalDate.of(2025, 1, 10))
                    .hora(LocalTime.of(14, 0, 0))
                    .usuarioOperador("Admin Sistema")
                    .rolModificado("VENTAS")
                    .tipoOperacion("ALTA")
                    .entidadInvolucrada("PRIVILEGIO")
                    .nombreEntidad("COMPRAS_MOD")
                    .contexto("ROLES")
                    .build();

            BitacoraPrivilegiosRequest requestConRol = BitacoraPrivilegiosRequest.builder()
                    .fechaInicio(LocalDate.of(2025, 1, 1))
                    .fechaFin(LocalDate.of(2025, 1, 31))
                    .rol("VENTAS")
                    .build();

            when(service.consultar(any(BitacoraPrivilegiosRequest.class)))
                    .thenReturn(BitacoraPrivilegiosResponse.success("1 registro", List.of(entryRol)));

            mockMvc.perform(post(BASE)
                            .contentType(MediaType.APPLICATION_JSON)
                            .content(objectMapper.writeValueAsString(requestConRol)))
                    .andExpect(status().isOk())
                    .andExpect(jsonPath("$.eventos[0].rolModificado").value("VENTAS"))
                    .andExpect(jsonPath("$.eventos[0].contexto").value("ROLES"));
        }

        @Test
        @DisplayName("Consulta con tipoContexto USUARIOS -> 200")
        void consultaConTipoContextoUsuarios() throws Exception {
            BitacoraPrivilegiosRequest requestTipo = BitacoraPrivilegiosRequest.builder()
                    .fechaInicio(LocalDate.of(2025, 1, 1))
                    .fechaFin(LocalDate.of(2025, 1, 31))
                    .tipoContexto("USUARIOS")
                    .build();

            when(service.consultar(any(BitacoraPrivilegiosRequest.class))).thenReturn(successResponse);

            mockMvc.perform(post(BASE)
                            .contentType(MediaType.APPLICATION_JSON)
                            .content(objectMapper.writeValueAsString(requestTipo)))
                    .andExpect(status().isOk());
        }

        @Test
        @DisplayName("Consulta con entidadInvolucrada PRIVILEGIO -> 200")
        void consultaConEntidadPrivilegio() throws Exception {
            BitacoraPrivilegiosRequest requestEntidad = BitacoraPrivilegiosRequest.builder()
                    .fechaInicio(LocalDate.of(2025, 1, 1))
                    .fechaFin(LocalDate.of(2025, 1, 31))
                    .entidadInvolucrada("PRIVILEGIO")
                    .build();

            when(service.consultar(any(BitacoraPrivilegiosRequest.class))).thenReturn(successResponse);

            mockMvc.perform(post(BASE)
                            .contentType(MediaType.APPLICATION_JSON)
                            .content(objectMapper.writeValueAsString(requestEntidad)))
                    .andExpect(status().isOk());
        }

        @Test
        @DisplayName("Consulta sin resultados -> 200 con lista vacía")
        void consultaSinResultados() throws Exception {
            when(service.consultar(any(BitacoraPrivilegiosRequest.class)))
                    .thenReturn(BitacoraPrivilegiosResponse.success(
                            "No se encontraron registros", Collections.emptyList()));

            mockMvc.perform(post(BASE)
                            .contentType(MediaType.APPLICATION_JSON)
                            .content(objectMapper.writeValueAsString(validRequest)))
                    .andExpect(status().isOk())
                    .andExpect(jsonPath("$.success").value(true))
                    .andExpect(jsonPath("$.eventos").isArray())
                    .andExpect(jsonPath("$.eventos").isEmpty());
        }

        @Test
        @DisplayName("Sin fechaInicio -> 400")
        void sinFechaInicio() throws Exception {
            BitacoraPrivilegiosRequest requestInvalido = BitacoraPrivilegiosRequest.builder()
                    .fechaFin(LocalDate.of(2025, 1, 31))
                    .build();

            mockMvc.perform(post(BASE)
                            .contentType(MediaType.APPLICATION_JSON)
                            .content(objectMapper.writeValueAsString(requestInvalido)))
                    .andExpect(status().isBadRequest());
        }

        @Test
        @DisplayName("Sin fechaFin -> 400")
        void sinFechaFin() throws Exception {
            BitacoraPrivilegiosRequest requestInvalido = BitacoraPrivilegiosRequest.builder()
                    .fechaInicio(LocalDate.of(2025, 1, 1))
                    .build();

            mockMvc.perform(post(BASE)
                            .contentType(MediaType.APPLICATION_JSON)
                            .content(objectMapper.writeValueAsString(requestInvalido)))
                    .andExpect(status().isBadRequest());
        }

        @Test
        @DisplayName("Rango invertido (fechaFin < fechaInicio) -> 400")
        void rangoInvertido() throws Exception {
            BitacoraPrivilegiosRequest requestInvalido = BitacoraPrivilegiosRequest.builder()
                    .fechaInicio(LocalDate.of(2025, 1, 31))
                    .fechaFin(LocalDate.of(2025, 1, 1))
                    .build();

            mockMvc.perform(post(BASE)
                            .contentType(MediaType.APPLICATION_JSON)
                            .content(objectMapper.writeValueAsString(requestInvalido)))
                    .andExpect(status().isBadRequest());
        }

        @Test
        @DisplayName("Rango mayor a 2 años -> 400")
        void rangoMayorDosAnios() throws Exception {
            BitacoraPrivilegiosRequest requestInvalido = BitacoraPrivilegiosRequest.builder()
                    .fechaInicio(LocalDate.of(2022, 1, 1))
                    .fechaFin(LocalDate.of(2025, 1, 31))
                    .build();

            mockMvc.perform(post(BASE)
                            .contentType(MediaType.APPLICATION_JSON)
                            .content(objectMapper.writeValueAsString(requestInvalido)))
                    .andExpect(status().isBadRequest());
        }

        @Test
        @DisplayName("tipoContexto inválido -> 400")
        void tipoContextoInvalido() throws Exception {
            BitacoraPrivilegiosRequest requestInvalido = BitacoraPrivilegiosRequest.builder()
                    .fechaInicio(LocalDate.of(2025, 1, 1))
                    .fechaFin(LocalDate.of(2025, 1, 31))
                    .tipoContexto("INVALIDO")
                    .build();

            mockMvc.perform(post(BASE)
                            .contentType(MediaType.APPLICATION_JSON)
                            .content(objectMapper.writeValueAsString(requestInvalido)))
                    .andExpect(status().isBadRequest());
        }

        @Test
        @DisplayName("entidadInvolucrada inválida -> 400")
        void entidadInvolucradaInvalida() throws Exception {
            BitacoraPrivilegiosRequest requestInvalido = BitacoraPrivilegiosRequest.builder()
                    .fechaInicio(LocalDate.of(2025, 1, 1))
                    .fechaFin(LocalDate.of(2025, 1, 31))
                    .entidadInvolucrada("INVALIDA")
                    .build();

            mockMvc.perform(post(BASE)
                            .contentType(MediaType.APPLICATION_JSON)
                            .content(objectMapper.writeValueAsString(requestInvalido)))
                    .andExpect(status().isBadRequest());
        }
    }
}
