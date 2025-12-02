package com.lavioleta.desarrollo.violetaserver.bitacora.dto.response;

import java.time.LocalDate;
import java.time.LocalTime;
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
@Schema(name = "BitacoraUnificadaResponse", description = "Resultado de la consulta de bitácora unificada de usuarios")
public class BitacoraUnificadaResponse {

    private boolean success;
    private String message;
    private List<BitacoraUsuarioEntry> eventos;

    public static BitacoraUnificadaResponse success(String message, List<BitacoraUsuarioEntry> eventos) {
        return BitacoraUnificadaResponse.builder()
                .success(true)
                .message(message)
                .eventos(eventos != null ? eventos : Collections.emptyList())
                .build();
    }

    public static BitacoraUnificadaResponse error(String message) {
        return BitacoraUnificadaResponse.builder()
                .success(false)
                .message(message)
                .eventos(Collections.emptyList())
                .build();
    }

    @Data
    @Builder
    @NoArgsConstructor
    @AllArgsConstructor
    @Schema(name = "BitacoraUsuarioEntry", description = "Evento registrado en la bitácora unificada")
    public static class BitacoraUsuarioEntry {
        private String referencia;
        private String tipoDocumento;
        private String operacion;
        private boolean cancelado;
        private LocalDate fechaDocumento;
        private String usuario;
        private LocalDate fechaOperacion;
        private LocalTime horaOperacion;
    }
}
