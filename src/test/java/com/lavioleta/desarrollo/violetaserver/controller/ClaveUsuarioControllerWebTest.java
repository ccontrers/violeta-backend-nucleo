package com.lavioleta.desarrollo.violetaserver.controller;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.when;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.post;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.put;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.jsonPath;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.status;

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
import com.lavioleta.desarrollo.violetaserver.usuarios.controller.ClaveUsuarioController;
import com.lavioleta.desarrollo.violetaserver.usuarios.dto.request.AsignarClaveRequest;
import com.lavioleta.desarrollo.violetaserver.usuarios.dto.request.CambiarClaveRequest;
import com.lavioleta.desarrollo.violetaserver.usuarios.dto.response.ClaveResponse;
import com.lavioleta.desarrollo.violetaserver.usuarios.service.ClaveUsuarioService;

@WebMvcTest(ClaveUsuarioController.class)
@AutoConfigureMockMvc(addFilters = false)
@ActiveProfiles("test")
@DisplayName("Web Layer: ClaveUsuarioController")
class ClaveUsuarioControllerWebTest {

    private static final String BASE = "/api/v1/usuarios";
    private static final String USUARIO_PRUEBA = "CRCP";

    // Hash SHA-256 de ejemplo (64 caracteres hex)
    private static final String HASH_CLAVE_ACTUAL = "9f86d081884c7d659a2feaa0c55ad015a3bf4f1b2b0b822cd15d6c15b0f00a08";
    private static final String HASH_CLAVE_NUEVA = "3c5636d872b042199e6a04a0e1e77aa15641244b7c5a826c71da5dce9d765a61";

    @Autowired
    MockMvc mockMvc;

    @Autowired
    ObjectMapper objectMapper;

    @MockBean
    ClaveUsuarioService service;

    private AsignarClaveRequest asignarRequest;
    private CambiarClaveRequest cambiarRequest;

    @BeforeEach
    void setUp() {
        asignarRequest = new AsignarClaveRequest(HASH_CLAVE_NUEVA);
        cambiarRequest = new CambiarClaveRequest(HASH_CLAVE_ACTUAL, HASH_CLAVE_NUEVA);
    }

    @Nested
    @DisplayName("POST /api/v1/usuarios/{id}/clave - Asignar clave inicial")
    class AsignarClave {

        @Test
        @DisplayName("Asignar clave exitoso -> 201 Created")
        void asignarClaveOk() throws Exception {
            ClaveResponse response = ClaveResponse.exito(USUARIO_PRUEBA, "Clave asignada correctamente");
            when(service.asignarClave(anyString(), any(AsignarClaveRequest.class))).thenReturn(response);

            mockMvc.perform(post(BASE + "/{id}/clave", USUARIO_PRUEBA)
                            .contentType(MediaType.APPLICATION_JSON)
                            .content(objectMapper.writeValueAsString(asignarRequest)))
                    .andExpect(status().isCreated())
                    .andExpect(jsonPath("$.success").value(true))
                    .andExpect(jsonPath("$.usuario").value(USUARIO_PRUEBA))
                    .andExpect(jsonPath("$.message").value("Clave asignada correctamente"));
        }

        @Test
        @DisplayName("Usuario no encontrado -> 404 Not Found")
        void asignarClaveUsuarioNoEncontrado() throws Exception {
            ClaveResponse response = ClaveResponse.error("Usuario no encontrado");
            when(service.asignarClave(anyString(), any(AsignarClaveRequest.class))).thenReturn(response);

            mockMvc.perform(post(BASE + "/{id}/clave", "INEXISTENTE")
                            .contentType(MediaType.APPLICATION_JSON)
                            .content(objectMapper.writeValueAsString(asignarRequest)))
                    .andExpect(status().isNotFound())
                    .andExpect(jsonPath("$.success").value(false))
                    .andExpect(jsonPath("$.message").value("Usuario no encontrado"));
        }

        @Test
        @DisplayName("Usuario ya tiene clave -> 409 Conflict")
        void asignarClaveYaTieneClave() throws Exception {
            ClaveResponse response = ClaveResponse.error("El usuario ya tiene una clave asignada. Use el endpoint de cambio de clave.");
            when(service.asignarClave(anyString(), any(AsignarClaveRequest.class))).thenReturn(response);

            mockMvc.perform(post(BASE + "/{id}/clave", USUARIO_PRUEBA)
                            .contentType(MediaType.APPLICATION_JSON)
                            .content(objectMapper.writeValueAsString(asignarRequest)))
                    .andExpect(status().isConflict())
                    .andExpect(jsonPath("$.success").value(false));
        }

        @Test
        @DisplayName("Hash inválido (no 64 chars) -> 400 Bad Request")
        void asignarClaveHashInvalido() throws Exception {
            AsignarClaveRequest invalidRequest = new AsignarClaveRequest("hash_muy_corto");

            mockMvc.perform(post(BASE + "/{id}/clave", USUARIO_PRUEBA)
                            .contentType(MediaType.APPLICATION_JSON)
                            .content(objectMapper.writeValueAsString(invalidRequest)))
                    .andExpect(status().isBadRequest())
                    .andExpect(jsonPath("$.errorCode").value("VALIDATION_ERROR"));
        }

        @Test
        @DisplayName("Clave vacía -> 400 Bad Request")
        void asignarClaveVacia() throws Exception {
            AsignarClaveRequest invalidRequest = new AsignarClaveRequest("");

            mockMvc.perform(post(BASE + "/{id}/clave", USUARIO_PRUEBA)
                            .contentType(MediaType.APPLICATION_JSON)
                            .content(objectMapper.writeValueAsString(invalidRequest)))
                    .andExpect(status().isBadRequest())
                    .andExpect(jsonPath("$.errorCode").value("VALIDATION_ERROR"));
        }

        @Test
        @DisplayName("Excepción inesperada -> 500 Internal Server Error")
        void asignarClaveExcepcion() throws Exception {
            when(service.asignarClave(anyString(), any(AsignarClaveRequest.class)))
                    .thenThrow(new RuntimeException("Error de BD"));

            mockMvc.perform(post(BASE + "/{id}/clave", USUARIO_PRUEBA)
                            .contentType(MediaType.APPLICATION_JSON)
                            .content(objectMapper.writeValueAsString(asignarRequest)))
                    .andExpect(status().isInternalServerError())
                    .andExpect(jsonPath("$.success").value(false))
                    .andExpect(jsonPath("$.message").value("Error interno del servidor"));
        }
    }

    @Nested
    @DisplayName("PUT /api/v1/usuarios/{id}/clave - Cambiar clave existente")
    class CambiarClave {

        @Test
        @DisplayName("Cambiar clave exitoso -> 200 OK")
        void cambiarClaveOk() throws Exception {
            ClaveResponse response = ClaveResponse.exito(USUARIO_PRUEBA, "Clave cambiada correctamente");
            when(service.cambiarClave(anyString(), any(CambiarClaveRequest.class))).thenReturn(response);

            mockMvc.perform(put(BASE + "/{id}/clave", USUARIO_PRUEBA)
                            .contentType(MediaType.APPLICATION_JSON)
                            .content(objectMapper.writeValueAsString(cambiarRequest)))
                    .andExpect(status().isOk())
                    .andExpect(jsonPath("$.success").value(true))
                    .andExpect(jsonPath("$.usuario").value(USUARIO_PRUEBA))
                    .andExpect(jsonPath("$.message").value("Clave cambiada correctamente"));
        }

        @Test
        @DisplayName("Usuario no encontrado -> 404 Not Found")
        void cambiarClaveUsuarioNoEncontrado() throws Exception {
            ClaveResponse response = ClaveResponse.error("Usuario no encontrado");
            when(service.cambiarClave(anyString(), any(CambiarClaveRequest.class))).thenReturn(response);

            mockMvc.perform(put(BASE + "/{id}/clave", "INEXISTENTE")
                            .contentType(MediaType.APPLICATION_JSON)
                            .content(objectMapper.writeValueAsString(cambiarRequest)))
                    .andExpect(status().isNotFound())
                    .andExpect(jsonPath("$.success").value(false))
                    .andExpect(jsonPath("$.message").value("Usuario no encontrado"));
        }

        @Test
        @DisplayName("Clave actual incorrecta -> 401 Unauthorized")
        void cambiarClaveIncorrecta() throws Exception {
            ClaveResponse response = ClaveResponse.error("La clave actual no es correcta");
            when(service.cambiarClave(anyString(), any(CambiarClaveRequest.class))).thenReturn(response);

            mockMvc.perform(put(BASE + "/{id}/clave", USUARIO_PRUEBA)
                            .contentType(MediaType.APPLICATION_JSON)
                            .content(objectMapper.writeValueAsString(cambiarRequest)))
                    .andExpect(status().isUnauthorized())
                    .andExpect(jsonPath("$.success").value(false))
                    .andExpect(jsonPath("$.message").value("La clave actual no es correcta"));
        }

        @Test
        @DisplayName("Hash inválido (caracteres no hex) -> 400 Bad Request")
        void cambiarClaveHashInvalido() throws Exception {
            CambiarClaveRequest invalidRequest = new CambiarClaveRequest(
                    "zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz", // caracteres no hex
                    HASH_CLAVE_NUEVA
            );

            mockMvc.perform(put(BASE + "/{id}/clave", USUARIO_PRUEBA)
                            .contentType(MediaType.APPLICATION_JSON)
                            .content(objectMapper.writeValueAsString(invalidRequest)))
                    .andExpect(status().isBadRequest())
                    .andExpect(jsonPath("$.errorCode").value("VALIDATION_ERROR"));
        }

        @Test
        @DisplayName("Clave actual vacía -> 400 Bad Request")
        void cambiarClaveActualVacia() throws Exception {
            CambiarClaveRequest invalidRequest = new CambiarClaveRequest("", HASH_CLAVE_NUEVA);

            mockMvc.perform(put(BASE + "/{id}/clave", USUARIO_PRUEBA)
                            .contentType(MediaType.APPLICATION_JSON)
                            .content(objectMapper.writeValueAsString(invalidRequest)))
                    .andExpect(status().isBadRequest())
                    .andExpect(jsonPath("$.errorCode").value("VALIDATION_ERROR"));
        }

        @Test
        @DisplayName("Clave nueva vacía -> 400 Bad Request")
        void cambiarClaveNuevaVacia() throws Exception {
            CambiarClaveRequest invalidRequest = new CambiarClaveRequest(HASH_CLAVE_ACTUAL, "");

            mockMvc.perform(put(BASE + "/{id}/clave", USUARIO_PRUEBA)
                            .contentType(MediaType.APPLICATION_JSON)
                            .content(objectMapper.writeValueAsString(invalidRequest)))
                    .andExpect(status().isBadRequest())
                    .andExpect(jsonPath("$.errorCode").value("VALIDATION_ERROR"));
        }

        @Test
        @DisplayName("Excepción inesperada -> 500 Internal Server Error")
        void cambiarClaveExcepcion() throws Exception {
            when(service.cambiarClave(anyString(), any(CambiarClaveRequest.class)))
                    .thenThrow(new RuntimeException("Error de BD"));

            mockMvc.perform(put(BASE + "/{id}/clave", USUARIO_PRUEBA)
                            .contentType(MediaType.APPLICATION_JSON)
                            .content(objectMapper.writeValueAsString(cambiarRequest)))
                    .andExpect(status().isInternalServerError())
                    .andExpect(jsonPath("$.success").value(false))
                    .andExpect(jsonPath("$.message").value("Error interno del servidor"));
        }
    }
}
