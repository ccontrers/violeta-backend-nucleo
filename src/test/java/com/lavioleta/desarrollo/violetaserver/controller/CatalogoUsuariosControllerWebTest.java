package com.lavioleta.desarrollo.violetaserver.controller;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.isNull;
import static org.mockito.Mockito.when;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.delete;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.get;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.post;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.put;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.jsonPath;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.status;

import java.time.LocalDate;
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
import com.lavioleta.desarrollo.violetaserver.config.TestSecurityConfig;
import com.lavioleta.desarrollo.violetaserver.usuarios.controller.CatalogoUsuariosController;
import com.lavioleta.desarrollo.violetaserver.usuarios.dto.request.UsuarioRequest;
import com.lavioleta.desarrollo.violetaserver.usuarios.dto.response.EmpleadoOptionResponse;
import com.lavioleta.desarrollo.violetaserver.usuarios.dto.response.UsuarioComboOptionResponse;
import com.lavioleta.desarrollo.violetaserver.usuarios.dto.response.UsuarioListResponse;
import com.lavioleta.desarrollo.violetaserver.usuarios.dto.response.UsuarioResponse;
import com.lavioleta.desarrollo.violetaserver.usuarios.service.CatalogoUsuariosService;

@WebMvcTest(CatalogoUsuariosController.class)

@AutoConfigureMockMvc(addFilters = false)
@ActiveProfiles("test")
@DisplayName("Web Layer: CatalogoUsuariosController")
class CatalogoUsuariosControllerWebTest {

        private static final String BASE = "/api/v1/usuarios";

        @Autowired
        MockMvc mockMvc;

        @Autowired
        ObjectMapper objectMapper;

        @MockBean
        CatalogoUsuariosService service;

        private UsuarioRequest validRequest;
        private UsuarioResponse successResponse;

        @BeforeEach
        void setUp() {
                validRequest = UsuarioRequest.builder()
                                .empleado("CRCP")
                                .password("secret")
                                .activo(true)
                                .fechaAlta(LocalDate.of(2023, 5, 10))
                                .fechaBaja(LocalDate.of(2099, 1, 1))
                                .usuarioContpaq("CRCP_CONT")
                                .passwordContpaq("cp_secret")
                                .build();

                UsuarioResponse.UsuarioDetail detail = UsuarioResponse.UsuarioDetail.builder()
                                .empleado("CRCP")
                                .nombreCompleto("CRCP Demo")
                                .activo(true)
                                .fechaAlta(LocalDate.of(2023, 5, 10))
                                .build();

                successResponse = UsuarioResponse.ok("OK", detail);
        }

        @Nested
        @DisplayName("GET /api/v1/usuarios")
        class ListarUsuarios {
                @Test
                @DisplayName("Listado exitoso -> 200")
                void listadoOk() throws Exception {
                        UsuarioListResponse response = UsuarioListResponse.builder()
                                        .success(true)
                                        .message("Consulta")
                                        .usuarios(List.of(UsuarioListResponse.UsuarioSummary.builder()
                                                        .empleado("CRCP")
                                                        .nombre("CRCP Demo")
                                                        .activo(true)
                                                        .build()))
                                        .build();
                        when(service.listarUsuarios(anyBoolean(), any())).thenReturn(response);

                        mockMvc.perform(get(BASE))
                                        .andExpect(status().isOk())
                                        .andExpect(jsonPath("$.success").value(true))
                                        .andExpect(jsonPath("$.usuarios[0].empleado").value("CRCP"));
                }

                @Test
                @DisplayName("Listado inválido -> 400")
                void listadoBadRequest() throws Exception {
                        UsuarioListResponse response = UsuarioListResponse.builder()
                                        .success(false)
                                        .message("Error filtros")
                                        .build();
                        when(service.listarUsuarios(anyBoolean(), any())).thenReturn(response);

                        mockMvc.perform(get(BASE))
                                        .andExpect(status().isBadRequest())
                                        .andExpect(jsonPath("$.success").value(false));
                }

                @Test
                @DisplayName("Listado excepcion -> 500")
                void listadoExcepcion() throws Exception {
                        when(service.listarUsuarios(anyBoolean(), any())).thenThrow(new RuntimeException("boom"));

                        mockMvc.perform(get(BASE))
                                        .andExpect(status().isInternalServerError())
                                        .andExpect(jsonPath("$.success").value(false))
                                        .andExpect(jsonPath("$.message").value("Error interno del servidor"));
                }
        }

        @Nested
        @DisplayName("GET /api/v1/usuarios/{empleado}")
        class ObtenerUsuario {
                @Test
                @DisplayName("Usuario encontrado -> 200")
                void obtenerOk() throws Exception {
                        when(service.obtenerUsuario("CRCP")).thenReturn(successResponse);

                        mockMvc.perform(get(BASE + "/{empleado}", "CRCP"))
                                        .andExpect(status().isOk())
                                        .andExpect(jsonPath("$.success").value(true))
                                        .andExpect(jsonPath("$.usuario.empleado").value("CRCP"));
                }

                @Test
                @DisplayName("Usuario no encontrado -> 404")
                void obtener404() throws Exception {
                        UsuarioResponse notFound = UsuarioResponse.error("Usuario no encontrado");
                        when(service.obtenerUsuario("CRPX")).thenReturn(notFound);

                        mockMvc.perform(get(BASE + "/{empleado}", "CRPX"))
                                        .andExpect(status().isNotFound())
                                        .andExpect(jsonPath("$.success").value(false));
                }

                @Test
                @DisplayName("Excepción en consulta -> 500")
                void obtenerExcepcion() throws Exception {
                        when(service.obtenerUsuario("CRCP")).thenThrow(new RuntimeException("db down"));

                        mockMvc.perform(get(BASE + "/{empleado}", "CRCP"))
                                        .andExpect(status().isInternalServerError())
                                        .andExpect(jsonPath("$.success").value(false))
                                        .andExpect(jsonPath("$.message").value("Error interno del servidor"));
                }
        }

        @Nested
        @DisplayName("POST /api/v1/usuarios")
        class CrearUsuario {
                @Test
                @DisplayName("Crear usuario -> 201")
                void crearOk() throws Exception {
                        when(service.crearUsuario(any(UsuarioRequest.class), anyString())).thenReturn(successResponse);

                        mockMvc.perform(post(BASE)
                                        .contentType(MediaType.APPLICATION_JSON)
                                        .header("X-Sucursal", "S1")
                                        .content(objectMapper.writeValueAsString(validRequest)))
                                        .andExpect(status().isCreated())
                                        .andExpect(jsonPath("$.success").value(true));
                }

                @Test
                @DisplayName("Validación Bean -> 400")
                void crearValidacion() throws Exception {
                        UsuarioRequest invalid = UsuarioRequest.builder().empleado("").build();

                        mockMvc.perform(post(BASE)
                                        .contentType(MediaType.APPLICATION_JSON)
                                        .content(objectMapper.writeValueAsString(invalid)))
                                        .andExpect(status().isBadRequest())
                                        .andExpect(jsonPath("$.errorCode").value("VALIDATION_ERROR"));
                }

                @Test
                @DisplayName("Servicio responde error -> 400")
                void crearRespuestaNegativa() throws Exception {
                        when(service.crearUsuario(any(UsuarioRequest.class), anyString()))
                                        .thenReturn(UsuarioResponse.error("Duplicado"));

                        mockMvc.perform(post(BASE)
                                        .contentType(MediaType.APPLICATION_JSON)
                                        .content(objectMapper.writeValueAsString(validRequest)))
                                        .andExpect(status().isBadRequest())
                                        .andExpect(jsonPath("$.success").value(false));
                }

                @Test
                @DisplayName("Excepción inesperada -> 500")
                void crearExcepcion() throws Exception {
                        when(service.crearUsuario(any(UsuarioRequest.class), anyString()))
                                        .thenThrow(new RuntimeException("deadlock"));

                        mockMvc.perform(post(BASE)
                                        .contentType(MediaType.APPLICATION_JSON)
                                        .content(objectMapper.writeValueAsString(validRequest)))
                                        .andExpect(status().isInternalServerError())
                                        .andExpect(jsonPath("$.message").value("Error interno del servidor"));
                }
        }

        @Nested
        @DisplayName("PUT /api/v1/usuarios/{empleado}")
        class ActualizarUsuario {
                @Test
                @DisplayName("Actualizar OK -> 200")
                void actualizarOk() throws Exception {
                        when(service.actualizarUsuario(anyString(), any(UsuarioRequest.class)))
                                        .thenReturn(successResponse);

                        mockMvc.perform(put(BASE + "/{empleado}", "CRCP")
                                        .contentType(MediaType.APPLICATION_JSON)
                                        .content(objectMapper.writeValueAsString(validRequest)))
                                        .andExpect(status().isOk())
                                        .andExpect(jsonPath("$.success").value(true));
                }

                @Test
                @DisplayName("Actualizar service error -> 400")
                void actualizarBadRequest() throws Exception {
                        when(service.actualizarUsuario(anyString(), any(UsuarioRequest.class)))
                                        .thenReturn(UsuarioResponse.error("No existe"));

                        mockMvc.perform(put(BASE + "/{empleado}", "CRCP")
                                        .contentType(MediaType.APPLICATION_JSON)
                                        .content(objectMapper.writeValueAsString(validRequest)))
                                        .andExpect(status().isBadRequest())
                                        .andExpect(jsonPath("$.success").value(false));
                }

                @Test
                @DisplayName("Actualizar excepción -> 500")
                void actualizarExcepcion() throws Exception {
                        when(service.actualizarUsuario(anyString(), any(UsuarioRequest.class)))
                                        .thenThrow(new RuntimeException("timeout"));

                        mockMvc.perform(put(BASE + "/{empleado}", "CRCP")
                                        .contentType(MediaType.APPLICATION_JSON)
                                        .content(objectMapper.writeValueAsString(validRequest)))
                                        .andExpect(status().isInternalServerError())
                                        .andExpect(jsonPath("$.message").value("Error interno del servidor"));
                }
        }

        @Nested
        @DisplayName("DELETE /api/v1/usuarios/{empleado}")
        class EliminarUsuario {
                @Test
                @DisplayName("Eliminar OK -> 200")
                void eliminarOk() throws Exception {
                        when(service.eliminarUsuario("CRCP")).thenReturn(UsuarioResponse.ok("Borrado", null));

                        mockMvc.perform(delete(BASE + "/{empleado}", "CRCP"))
                                        .andExpect(status().isOk())
                                        .andExpect(jsonPath("$.success").value(true));
                }

                @Test
                @DisplayName("Eliminar no existe -> 404")
                void eliminarNoExiste() throws Exception {
                        UsuarioResponse nf = UsuarioResponse.error("No existe usuario");
                        when(service.eliminarUsuario("XXXX")).thenReturn(nf);

                        mockMvc.perform(delete(BASE + "/{empleado}", "XXXX"))
                                        .andExpect(status().isNotFound())
                                        .andExpect(jsonPath("$.success").value(false));
                }

                @Test
                @DisplayName("Eliminar restricción -> 409")
                void eliminarRestriccion() throws Exception {
                        UsuarioResponse conflict = UsuarioResponse.error("Restricción de integridad");
                        when(service.eliminarUsuario("CRCP")).thenReturn(conflict);

                        mockMvc.perform(delete(BASE + "/{empleado}", "CRCP"))
                                        .andExpect(status().isConflict())
                                        .andExpect(jsonPath("$.success").value(false));
                }

                @Test
                @DisplayName("Eliminar excepción -> 500")
                void eliminarExcepcion() throws Exception {
                        when(service.eliminarUsuario("CRCP")).thenThrow(new RuntimeException("fk"));

                        mockMvc.perform(delete(BASE + "/{empleado}", "CRCP"))
                                        .andExpect(status().isInternalServerError())
                                        .andExpect(jsonPath("$.message").value("Error interno del servidor"));
                }
        }

        @Nested
        @DisplayName("GET /api/v1/usuarios/empleados-disponibles")
        class EmpleadosDisponibles {
                @Test
                @DisplayName("Listado de empleados -> 200")
                void empleadosOk() throws Exception {
                        List<EmpleadoOptionResponse> options = List.of(
                                        EmpleadoOptionResponse.builder().empleado("EMP1").nombreCompleto("Nombre 1")
                                                        .tieneUsuario(false).build());
                        when(service.obtenerEmpleadosDisponibles()).thenReturn(options);

                        mockMvc.perform(get(BASE + "/empleados-disponibles"))
                                        .andExpect(status().isOk())
                                        .andExpect(jsonPath("$[0].empleado").value("EMP1"));
                }

                @Test
                @DisplayName("Empleados excepción -> 500")
                void empleadosExcepcion() throws Exception {
                        when(service.obtenerEmpleadosDisponibles()).thenThrow(new RuntimeException("err"));

                        mockMvc.perform(get(BASE + "/empleados-disponibles"))
                                        .andExpect(status().isInternalServerError());
                }
        }

        @Nested
        @DisplayName("GET /api/v1/usuarios/combo-box")
        class ComboUsuarios {
                @Test
                @DisplayName("Combo usuarios -> 200")
                void comboOk() throws Exception {
                        List<UsuarioComboOptionResponse> combo = List.of(
                                        UsuarioComboOptionResponse.builder()
                                                        .empleado("EMP1")
                                                        .nombreCompleto("Nombre 1")
                                                        .sucursal("S1")
                                                        .build());
                        when(service.listarUsuariosCombo(anyString())).thenReturn(combo);

                        mockMvc.perform(get(BASE + "/combo-box")
                                        .param("sucursal", "S1"))
                                        .andExpect(status().isOk())
                                        .andExpect(jsonPath("$[0].empleado").value("EMP1"));
                }

                @Test
                @DisplayName("Combo usuarios excepción -> 500")
                void comboError() throws Exception {
                        when(service.listarUsuariosCombo(isNull())).thenThrow(new RuntimeException("boom"));

                        mockMvc.perform(get(BASE + "/combo-box"))
                                        .andExpect(status().isInternalServerError());
                }
        }
}
