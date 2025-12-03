package com.lavioleta.desarrollo.violetaserver.usuarios.dto.request;

import io.swagger.v3.oas.annotations.media.Schema;
import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.Pattern;
import jakarta.validation.constraints.Size;

/**
 * DTO para cambiar la clave de un usuario que ya tiene password.
 * Equivalente a ServidorAdminSistema::CambiaClave del sistema legado.
 */
@Schema(description = "Petición para cambiar la clave de un usuario existente")
public class CambiarClaveRequest {

    @NotBlank(message = "La clave actual es requerida")
    @Size(min = 64, max = 64, message = "La clave debe ser un hash SHA-256 de 64 caracteres hexadecimales")
    @Pattern(regexp = "^[a-fA-F0-9]{64}$", message = "La clave debe ser un hash SHA-256 válido (64 caracteres hexadecimales)")
    @Schema(description = "Hash SHA-256 de la clave actual (64 caracteres hex)", example = "9f86d081884c7d659a2feaa0c55ad015a3bf4f1b2b0b822cd15d6c15b0f00a08")
    private String claveActual;

    @NotBlank(message = "La nueva clave es requerida")
    @Size(min = 64, max = 64, message = "La clave debe ser un hash SHA-256 de 64 caracteres hexadecimales")
    @Pattern(regexp = "^[a-fA-F0-9]{64}$", message = "La clave debe ser un hash SHA-256 válido (64 caracteres hexadecimales)")
    @Schema(description = "Hash SHA-256 de la nueva clave (64 caracteres hex)", example = "3c5636d872b042199e6a04a0e1e77aa15641244b7c5a826c71da5dce9d765a61")
    private String nuevaClave;

    public CambiarClaveRequest() {
    }

    public CambiarClaveRequest(String claveActual, String nuevaClave) {
        this.claveActual = claveActual;
        this.nuevaClave = nuevaClave;
    }

    public String getClaveActual() {
        return claveActual;
    }

    public void setClaveActual(String claveActual) {
        this.claveActual = claveActual;
    }

    public String getNuevaClave() {
        return nuevaClave;
    }

    public void setNuevaClave(String nuevaClave) {
        this.nuevaClave = nuevaClave;
    }
}
