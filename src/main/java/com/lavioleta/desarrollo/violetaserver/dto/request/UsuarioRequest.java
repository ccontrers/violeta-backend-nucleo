package com.lavioleta.desarrollo.violetaserver.dto.request;

import java.time.LocalDate;

import io.swagger.v3.oas.annotations.media.Schema;
import jakarta.validation.constraints.NotBlank;
import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * DTO para operaciones de alta / modificación de usuarios (equivalente a GrabaUsuarios).
 */
@Data
@Builder
@NoArgsConstructor
@AllArgsConstructor
@Schema(name = "UsuarioRequest", description = "Datos requeridos para crear o actualizar un usuario legado")
public class UsuarioRequest {

    @Schema(description = "Clave del empleado al que se asociará el usuario", example = "CRCP", requiredMode = Schema.RequiredMode.REQUIRED)
    @NotBlank(message = "El campo empleado es obligatorio")
    private String empleado;

    @Schema(description = "Contraseña del sistema legado (se almacena tal cual en la tabla usuarios)")
    private String password;

    @Schema(description = "Indica si el usuario está activo", example = "true")
    private Boolean activo;

    @Schema(description = "Fecha de alta en formato ISO (se usa la fecha actual si no se envía)", example = "2024-01-15")
    private LocalDate fechaAlta;

    @Schema(description = "Fecha de baja en formato ISO (por defecto 2099-01-01)", example = "2099-01-01")
    private LocalDate fechaBaja;

    @Schema(description = "Usuario asociado al sistema Contpaq", example = "CRCP_CONT")
    private String usuarioContpaq;

    @Schema(description = "Contraseña para el sistema Contpaq")
    private String passwordContpaq;
}
