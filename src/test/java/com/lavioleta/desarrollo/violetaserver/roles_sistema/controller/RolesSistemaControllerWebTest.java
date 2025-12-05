package com.lavioleta.desarrollo.violetaserver.roles_sistema.controller;

import static org.mockito.Mockito.when;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.get;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.jsonPath;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.status;

import java.util.Collections;
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

import com.lavioleta.desarrollo.violetaserver.roles_sistema.dto.response.RolComboOptionResponse;
import com.lavioleta.desarrollo.violetaserver.roles_sistema.service.RolesSistemaService;

/**
 * Tests de capa web para RolesSistemaController.
 * Valida comportamiento del endpoint para combo box de roles.
 */
@WebMvcTest(RolesSistemaController.class)
@AutoConfigureMockMvc(addFilters = false)
@ActiveProfiles("test")
@DisplayName("Web Layer: RolesSistemaController")
class RolesSistemaControllerWebTest {

    private static final String BASE = "/api/v1/roles-sistema";

    @Autowired
    MockMvc mockMvc;

    @MockBean
    RolesSistemaService service;

    @Nested
    @DisplayName("GET /api/v1/roles-sistema/combo-box")
    class ObtenerRolesParaCombo {

        @Test
        @DisplayName("Listado de roles exitoso -> 200")
        void listadoRolesExitoso() throws Exception {
            List<RolComboOptionResponse> roles = List.of(
                    RolComboOptionResponse.builder().clave("ADMIN").nombre("Administrador").build(),
                    RolComboOptionResponse.builder().clave("VENTAS").nombre("Vendedor").build(),
                    RolComboOptionResponse.builder().clave("COMPRAS").nombre("Comprador").build()
            );

            when(service.listarRolesCombo()).thenReturn(roles);

            mockMvc.perform(get(BASE + "/combo-box"))
                    .andExpect(status().isOk())
                    .andExpect(jsonPath("$").isArray())
                    .andExpect(jsonPath("$.length()").value(3))
                    .andExpect(jsonPath("$[0].clave").value("ADMIN"))
                    .andExpect(jsonPath("$[0].nombre").value("Administrador"))
                    .andExpect(jsonPath("$[1].clave").value("VENTAS"))
                    .andExpect(jsonPath("$[1].nombre").value("Vendedor"));
        }

        @Test
        @DisplayName("Sin roles en el sistema -> 200 con lista vacía")
        void sinRoles() throws Exception {
            when(service.listarRolesCombo()).thenReturn(Collections.emptyList());

            mockMvc.perform(get(BASE + "/combo-box"))
                    .andExpect(status().isOk())
                    .andExpect(jsonPath("$").isArray())
                    .andExpect(jsonPath("$").isEmpty());
        }

        @Test
        @DisplayName("Un solo rol -> 200 con un elemento")
        void unSoloRol() throws Exception {
            List<RolComboOptionResponse> roles = List.of(
                    RolComboOptionResponse.builder().clave("SUPERUSR").nombre("Super Usuario").build()
            );

            when(service.listarRolesCombo()).thenReturn(roles);

            mockMvc.perform(get(BASE + "/combo-box"))
                    .andExpect(status().isOk())
                    .andExpect(jsonPath("$").isArray())
                    .andExpect(jsonPath("$.length()").value(1))
                    .andExpect(jsonPath("$[0].clave").value("SUPERUSR"))
                    .andExpect(jsonPath("$[0].nombre").value("Super Usuario"));
        }
    }
}
