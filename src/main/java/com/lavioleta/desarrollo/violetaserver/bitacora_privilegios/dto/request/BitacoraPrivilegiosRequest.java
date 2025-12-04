package com.lavioleta.desarrollo.violetaserver.bitacora_privilegios.dto.request;

import java.time.LocalDate;

import io.swagger.v3.oas.annotations.media.Schema;
import jakarta.validation.constraints.AssertTrue;
import jakarta.validation.constraints.NotNull;
import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * DTO de solicitud para consultar la bitácora de modificaciones de privilegios.
 * Migrado desde FormBitacoraModPrivilegios.cpp::MostrarBitacora()
 * 
 * Filtros obligatorios: fechaInicio, fechaFin (rango máximo 2 años)
 * Filtros opcionales: usuario, rol, tipoContexto, entidadInvolucrada
 */
@Data
@Builder
@NoArgsConstructor
@AllArgsConstructor
@Schema(name = "BitacoraPrivilegiosRequest", description = "Filtros para consultar la bitácora de modificaciones de privilegios")
public class BitacoraPrivilegiosRequest {

    @NotNull(message = "fechaInicio es obligatorio")
    @Schema(description = "Fecha inicial del rango (incluyente)", example = "2025-01-01", requiredMode = Schema.RequiredMode.REQUIRED)
    private LocalDate fechaInicio;

    @NotNull(message = "fechaFin es obligatorio")
    @Schema(description = "Fecha final del rango (incluyente)", example = "2025-01-31", requiredMode = Schema.RequiredMode.REQUIRED)
    private LocalDate fechaFin;

    @Schema(description = "Clave del empleado para filtrar (operador o usuario modificado)", example = "CRCP", requiredMode = Schema.RequiredMode.NOT_REQUIRED)
    private String usuario;

    @Schema(description = "Clave del rol modificado para filtrar", example = "ADMIN", requiredMode = Schema.RequiredMode.NOT_REQUIRED)
    private String rol;

    @Schema(description = "Tipo de contexto: USUARIOS o ROLES", example = "USUARIOS", allowableValues = {"USUARIOS", "ROLES"}, requiredMode = Schema.RequiredMode.NOT_REQUIRED)
    private String tipoContexto;

    @Schema(description = "Entidad involucrada en la modificación", example = "PRIVILEGIO", allowableValues = {"GRUPO", "OBJETO", "PRIVILEGIO", "SUCURSAL", "ROL", "USUARIO"}, requiredMode = Schema.RequiredMode.NOT_REQUIRED)
    private String entidadInvolucrada;

    /**
     * Valida que fechaFin >= fechaInicio.
     * Corresponde a la validación en ButtonMuestraReporteClick del legacy.
     */
    @AssertTrue(message = "fechaFin debe ser mayor o igual a fechaInicio")
    public boolean isRangoFechasValido() {
        if (fechaInicio == null || fechaFin == null) {
            return true;
        }
        return !fechaFin.isBefore(fechaInicio);
    }

    /**
     * Valida que el rango no exceda 2 años.
     * Corresponde a la validación IncYear(FechaFin, -2) del legacy.
     */
    @AssertTrue(message = "El rango de fechas no puede ser mayor a 2 años")
    public boolean isRangoMaximoDosAnios() {
        if (fechaInicio == null || fechaFin == null) {
            return true;
        }
        LocalDate limiteInferior = fechaFin.minusYears(2);
        return !fechaInicio.isBefore(limiteInferior);
    }

    /**
     * Valida que tipoContexto sea un valor válido si se proporciona.
     */
    @AssertTrue(message = "tipoContexto debe ser USUARIOS o ROLES")
    public boolean isTipoContextoValido() {
        if (tipoContexto == null || tipoContexto.isBlank()) {
            return true;
        }
        return "USUARIOS".equalsIgnoreCase(tipoContexto) || "ROLES".equalsIgnoreCase(tipoContexto);
    }

    /**
     * Valida que entidadInvolucrada sea un valor válido si se proporciona.
     */
    @AssertTrue(message = "entidadInvolucrada debe ser GRUPO, OBJETO, PRIVILEGIO, SUCURSAL, ROL o USUARIO")
    public boolean isEntidadInvolucradaValida() {
        if (entidadInvolucrada == null || entidadInvolucrada.isBlank()) {
            return true;
        }
        return "GRUPO".equalsIgnoreCase(entidadInvolucrada)
                || "OBJETO".equalsIgnoreCase(entidadInvolucrada)
                || "PRIVILEGIO".equalsIgnoreCase(entidadInvolucrada)
                || "SUCURSAL".equalsIgnoreCase(entidadInvolucrada)
                || "ROL".equalsIgnoreCase(entidadInvolucrada)
                || "USUARIO".equalsIgnoreCase(entidadInvolucrada);
    }
}
