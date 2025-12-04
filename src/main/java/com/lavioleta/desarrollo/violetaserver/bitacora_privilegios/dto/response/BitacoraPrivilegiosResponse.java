package com.lavioleta.desarrollo.violetaserver.bitacora_privilegios.dto.response;

import java.time.LocalDate;
import java.time.LocalTime;
import java.util.Collections;
import java.util.List;

import io.swagger.v3.oas.annotations.media.Schema;
import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * DTO de respuesta para la consulta de bitácora de modificaciones de privilegios.
 * Migrado desde FormBitacoraModPrivilegios.cpp::MostrarBitacora()
 * 
 * Columnas del grid legacy:
 * - Fecha, Hora, Usuario (operador), Usuario modificado, Rol modificado, 
 *   Operación (tipo_mod), Entidad involucrada (entidad_mod), Nombre de la entidad
 */
@Data
@Builder
@NoArgsConstructor
@AllArgsConstructor
@Schema(name = "BitacoraPrivilegiosResponse", description = "Resultado de la consulta de bitácora de modificaciones de privilegios")
public class BitacoraPrivilegiosResponse {

    @Schema(description = "Indica si la operación fue exitosa")
    private boolean success;

    @Schema(description = "Mensaje descriptivo del resultado")
    private String message;

    @Schema(description = "Lista de eventos de modificación de privilegios")
    private List<BitacoraPrivilegioEntry> eventos;

    public static BitacoraPrivilegiosResponse success(String message, List<BitacoraPrivilegioEntry> eventos) {
        return BitacoraPrivilegiosResponse.builder()
                .success(true)
                .message(message)
                .eventos(eventos != null ? eventos : Collections.emptyList())
                .build();
    }

    public static BitacoraPrivilegiosResponse error(String message) {
        return BitacoraPrivilegiosResponse.builder()
                .success(false)
                .message(message)
                .eventos(Collections.emptyList())
                .build();
    }

    /**
     * Representa una entrada individual de la bitácora de privilegios.
     * Corresponde a las columnas del StringGridReporte legacy.
     */
    @Data
    @Builder
    @NoArgsConstructor
    @AllArgsConstructor
    @Schema(name = "BitacoraPrivilegioEntry", description = "Evento registrado en la bitácora de modificaciones de privilegios")
    public static class BitacoraPrivilegioEntry {

        @Schema(description = "Fecha del evento", example = "2025-01-15")
        private LocalDate fecha;

        @Schema(description = "Hora del evento", example = "10:32:11")
        private LocalTime hora;

        @Schema(description = "Nombre completo del usuario operador que realizó la modificación", example = "Juan Pérez López")
        private String usuarioOperador;

        @Schema(description = "Nombre completo del usuario modificado (solo para contexto USUARIOS)", example = "María García Sánchez")
        private String usuarioModificado;

        @Schema(description = "Clave del rol modificado (solo para contexto ROLES)", example = "ADMIN")
        private String rolModificado;

        @Schema(description = "Tipo de operación realizada (ALTA, BAJA, MODIFICACIÓN)", example = "MODIFICACIÓN")
        private String tipoOperacion;

        @Schema(description = "Tipo de entidad afectada", example = "PRIVILEGIO", allowableValues = {"GRUPO", "OBJETO", "PRIVILEGIO", "SUCURSAL", "ROL", "USUARIO"})
        private String entidadInvolucrada;

        @Schema(description = "Identificador o nombre de la entidad afectada", example = "BITMPUR_CON")
        private String nombreEntidad;

        @Schema(description = "Contexto de la modificación: USUARIOS o ROLES", example = "USUARIOS")
        private String contexto;
    }
}
