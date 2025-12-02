package com.lavioleta.desarrollo.violetaserver.bitacora.dto.request;

import java.time.LocalDate;

import io.swagger.v3.oas.annotations.media.Schema;
import jakarta.validation.constraints.AssertTrue;
import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.NotNull;
import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

@Data
@Builder
@NoArgsConstructor
@AllArgsConstructor
@Schema(name = "BitacoraUnificadaRequest", description = "Filtros para consultar la bitácora unificada de usuarios")
public class BitacoraUnificadaRequest {

    @NotNull(message = "fechaInicio es obligatorio")
    @Schema(description = "Fecha inicial del rango (incluyente)", example = "2025-01-01", requiredMode = Schema.RequiredMode.REQUIRED)
    private LocalDate fechaInicio;

    @NotNull(message = "fechaFin es obligatorio")
    @Schema(description = "Fecha final del rango (incluyente)", example = "2025-01-31", requiredMode = Schema.RequiredMode.REQUIRED)
    private LocalDate fechaFin;

    @NotBlank(message = "Debe enviar la clave de usuario")
    @Schema(description = "Clave de empleado a consultar", example = "USR001", requiredMode = Schema.RequiredMode.REQUIRED)
    private String usuario;

    @Schema(description = "Sucursal a la que pertenece la actividad del usuario", example = "S1", requiredMode = Schema.RequiredMode.NOT_REQUIRED)
    private String sucursal;

    @AssertTrue(message = "fechaFin debe ser mayor o igual a fechaInicio")
    public boolean isRangoFechasValido() {
        if (fechaInicio == null || fechaFin == null) {
            return true;
        }
        return !fechaFin.isBefore(fechaInicio);
    }
}
