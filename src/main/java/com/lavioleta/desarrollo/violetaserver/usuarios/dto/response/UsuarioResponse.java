package com.lavioleta.desarrollo.violetaserver.usuarios.dto.response;

import java.time.LocalDate;

import io.swagger.v3.oas.annotations.media.Schema;
import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

@Data
@Builder
@NoArgsConstructor
@AllArgsConstructor
@Schema(name = "UsuarioResponse", description = "Respuesta estándar para operaciones de catálogo de usuarios")
public class UsuarioResponse {

    private boolean success;
    private String message;
    private UsuarioDetail usuario;

    public static UsuarioResponse ok(String message, UsuarioDetail detail) {
        return UsuarioResponse.builder()
                .success(true)
                .message(message)
                .usuario(detail)
                .build();
    }

    public static UsuarioResponse error(String message) {
        return UsuarioResponse.builder()
                .success(false)
                .message(message)
                .build();
    }

    @Data
    @Builder
    @NoArgsConstructor
    @AllArgsConstructor
    @Schema(name = "UsuarioDetail")
    public static class UsuarioDetail {
        private String empleado;
        private String nombreCompleto;
        private Boolean activo;
        private LocalDate fechaAlta;
        private LocalDate fechaBaja;
        private LocalDate ultimoAcceso;
        private String password;
        private String usuarioContpaq;
        private String passwordContpaq;
    }
}
