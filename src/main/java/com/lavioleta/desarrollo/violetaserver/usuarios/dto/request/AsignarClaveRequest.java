package com.lavioleta.desarrollo.violetaserver.usuarios.dto.request;

import io.swagger.v3.oas.annotations.media.Schema;
import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.Pattern;
import jakarta.validation.constraints.Size;

/**
 * DTO para asignar clave inicial a un usuario que no tiene password (password IS NULL).
 * Equivalente a ServidorAdminSistema::AsignaPassword del sistema legado.
 */
@Schema(description = "Petición para asignar clave inicial a un usuario sin contraseña")
public class AsignarClaveRequest {

    @NotBlank(message = "La nueva clave es requerida")
    @Size(min = 64, max = 64, message = "La clave debe ser un hash SHA-256 de 64 caracteres hexadecimales")
    @Pattern(regexp = "^[a-fA-F0-9]{64}$", message = "La clave debe ser un hash SHA-256 válido (64 caracteres hexadecimales)")
    @Schema(description = "Hash SHA-256 de la nueva clave (64 caracteres hex)", example = "9f86d081884c7d659a2feaa0c55ad015a3bf4f1b2b0b822cd15d6c15b0f00a08")
    private String nuevaClave;

    public AsignarClaveRequest() {
    }

    public AsignarClaveRequest(String nuevaClave) {
        this.nuevaClave = nuevaClave;
    }

    public String getNuevaClave() {
        return nuevaClave;
    }

    public void setNuevaClave(String nuevaClave) {
        this.nuevaClave = nuevaClave;
    }
}
