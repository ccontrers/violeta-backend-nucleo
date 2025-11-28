package com.lavioleta.desarrollo.violetaserver.usuarios.dto.response;

import java.time.LocalDate;
import java.util.Collections;
import java.util.List;

import io.swagger.v3.oas.annotations.media.Schema;
import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

@Data
@Builder
@NoArgsConstructor
@AllArgsConstructor
@Schema(name = "UsuarioListResponse", description = "Listado de usuarios con metadatos")
public class UsuarioListResponse {

    private boolean success;
    private String message;
    private List<UsuarioSummary> usuarios;

    public static UsuarioListResponse success(String message, List<UsuarioSummary> usuarios) {
        return UsuarioListResponse.builder()
                .success(true)
                .message(message)
                .usuarios(usuarios != null ? usuarios : Collections.emptyList())
                .build();
    }

    public static UsuarioListResponse error(String message) {
        return UsuarioListResponse.builder()
                .success(false)
                .message(message)
                .usuarios(Collections.emptyList())
                .build();
    }

    @Data
    @Builder
    @NoArgsConstructor
    @AllArgsConstructor
    @Schema(name = "UsuarioSummary")
    public static class UsuarioSummary {
        private String empleado;
        private String nombre;
        private LocalDate ultimoAcceso;
        private Boolean activo;
    }
}
