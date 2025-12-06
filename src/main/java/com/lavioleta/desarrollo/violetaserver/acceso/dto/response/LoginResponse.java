package com.lavioleta.desarrollo.violetaserver.acceso.dto.response;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

@Data
@Builder
@NoArgsConstructor
@AllArgsConstructor
public class LoginResponse {
    
    private boolean success;
    private String message;
    private UsuarioInfo usuario;
    private String token; // Para futuras implementaciones de JWT
    
    @Data
    @Builder
    @NoArgsConstructor
    @AllArgsConstructor
    public static class UsuarioInfo {
        private String empleado;
        private String nombre;
        private String sucursal;
        private String sucursalNombre;
        private Integer idempresa;
        private String empresaNombre;
        private boolean activo;
        private String perfil;
    }
    
    // Métodos helper para respuestas comunes
    public static LoginResponse success(UsuarioInfo usuario, String message) {
        return LoginResponse.builder()
                .success(true)
                .message(message)
                .usuario(usuario)
                .token(null) // Sin token para login con sesión
                .build();
    }
    
    public static LoginResponse successWithToken(UsuarioInfo usuario, String message, String token) {
        return LoginResponse.builder()
                .success(true)
                .message(message)
                .usuario(usuario)
                .token(token) // Con token para APIs/mobile
                .build();
    }
    
    public static LoginResponse failure(String message) {
        return LoginResponse.builder()
                .success(false)
                .message(message)
                .build();
    }
}
