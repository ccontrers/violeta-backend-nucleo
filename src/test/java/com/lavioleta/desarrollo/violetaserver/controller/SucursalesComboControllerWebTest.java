package com.lavioleta.desarrollo.violetaserver.controller;

import static org.mockito.ArgumentMatchers.any;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.get;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.jsonPath;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.status;

import java.util.List;

import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;
import org.springframework.boot.test.autoconfigure.web.servlet.WebMvcTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.ActiveProfiles;
import org.springframework.test.web.servlet.MockMvc;

import com.lavioleta.desarrollo.violetaserver.sucursales.controller.SucursalesComboController;
import com.lavioleta.desarrollo.violetaserver.sucursales.dto.response.SucursalComboOptionResponse;
import com.lavioleta.desarrollo.violetaserver.sucursales.service.SucursalesService;

@WebMvcTest(SucursalesComboController.class)
@AutoConfigureMockMvc(addFilters = false)
@ActiveProfiles("test")
@DisplayName("Web Layer: SucursalesComboController")
class SucursalesComboControllerWebTest {

    private static final String BASE = "/api/v1/sucursales";

    @Autowired
    MockMvc mockMvc;

    @MockBean
    SucursalesService service;

    @Nested
    @DisplayName("GET /api/v1/sucursales/combo-box")
    class ComboSucursales {

        @Test
        @DisplayName("Combo sucursales -> 200")
        void comboOk() throws Exception {
            List<SucursalComboOptionResponse> combo = List.of(
                    SucursalComboOptionResponse.builder()
                            .sucursal("S1")
                            .nombre("Sucursal 1")
                            .build());
            org.mockito.Mockito.when(service.listarSucursalesCombo(any())).thenReturn(combo);

            mockMvc.perform(get(BASE + "/combo-box")
                    .param("idEmpresa", "10"))
                    .andExpect(status().isOk())
                    .andExpect(jsonPath("$[0].sucursal").value("S1"));
        }

        @Test
        @DisplayName("Combo sucursales excepción -> 500")
        void comboError() throws Exception {
            org.mockito.Mockito.when(service.listarSucursalesCombo(any()))
                    .thenThrow(new RuntimeException("db error"));

            mockMvc.perform(get(BASE + "/combo-box"))
                    .andExpect(status().isInternalServerError());
        }
    }
}
