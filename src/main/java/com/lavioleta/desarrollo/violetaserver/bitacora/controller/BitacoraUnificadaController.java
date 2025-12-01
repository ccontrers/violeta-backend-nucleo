package com.lavioleta.desarrollo.violetaserver.bitacora.controller;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.util.StringUtils;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import com.lavioleta.desarrollo.violetaserver.bitacora.dto.request.BitacoraUnificadaRequest;
import com.lavioleta.desarrollo.violetaserver.bitacora.dto.response.BitacoraUnificadaResponse;
import com.lavioleta.desarrollo.violetaserver.bitacora.service.BitacoraUnificadaService;

import io.swagger.v3.oas.annotations.Operation;
import io.swagger.v3.oas.annotations.responses.ApiResponse;
import io.swagger.v3.oas.annotations.responses.ApiResponses;
import io.swagger.v3.oas.annotations.tags.Tag;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.validation.Valid;

@RestController
@RequestMapping("/api/v1/bitacora-unificada")
@Tag(name = "Bitácora de Usuarios", description = "Consulta migrada de ServidorAdminSistema::ConsultaBitacoraUnificada")
public class BitacoraUnificadaController {

    private static final Logger log = LoggerFactory.getLogger(BitacoraUnificadaController.class);

    private final BitacoraUnificadaService service;

    public BitacoraUnificadaController(BitacoraUnificadaService service) {
        this.service = service;
    }

    @PostMapping
    @Operation(summary = "Consultar bitácora unificada", description = "Ejecuta la consulta legacy ConsultaBitacoraUnificada para un rango de fechas, un usuario y opcionalmente una sucursal")
    @ApiResponses({
            @ApiResponse(responseCode = "200", description = "Consulta ejecutada"),
            @ApiResponse(responseCode = "400", description = "Parámetros inválidos"),
            @ApiResponse(responseCode = "500", description = "Error interno")
    })
    public ResponseEntity<BitacoraUnificadaResponse> consultar(
            @Valid @RequestBody BitacoraUnificadaRequest request,
            HttpServletRequest httpRequest) {
        try {
                    log.info("Consultando bitácora unificada desde IP {} para el usuario {} en sucursal {}",
                        obtenerClientIp(httpRequest), request.getUsuario(), request.getSucursal());
            BitacoraUnificadaResponse response = service.consultar(request);
            HttpStatus status = response.isSuccess() ? HttpStatus.OK : HttpStatus.BAD_REQUEST;
            return ResponseEntity.status(status).body(response);
        } catch (Exception ex) {
            log.error("Error inesperado al consultar la bitácora unificada", ex);
            return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR)
                    .body(BitacoraUnificadaResponse.error("Error interno del servidor"));
        }
    }

    private String obtenerClientIp(HttpServletRequest request) {
        String header = request.getHeader("X-Forwarded-For");
        if (StringUtils.hasText(header)) {
            return header.split(",")[0].trim();
        }
        header = request.getHeader("X-Real-IP");
        if (StringUtils.hasText(header)) {
            return header;
        }
        return request.getRemoteAddr();
    }
}
