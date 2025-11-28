package com.lavioleta.desarrollo.violetaserver.acceso.dto.request;

import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.Pattern;
import jakarta.validation.constraints.Size;
import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

@Data
@Builder
@NoArgsConstructor
@AllArgsConstructor
public class LoginRequest {
    
    @NotBlank(message = "Usuario es requerido")
    @Size(min = 3, max = 50, message = "Usuario debe tener entre 3 y 50 caracteres")
    @Pattern(regexp = "^[A-Za-z0-9_]+$", message = "Usuario solo puede contener letras, números y guiones bajos")
    private String usuario;
    
    @NotBlank(message = "Password es requerido")
    @Size(min = 64, max = 64, message = "Password hash debe tener exactamente 64 caracteres (SHA-256)")
    @Pattern(regexp = "^[a-fA-F0-9]{64}$", message = "Password hash debe ser un valor hexadecimal válido")
    private String passwordHash;
    
    @Builder.Default
    private String sucursal = "S1";
}
