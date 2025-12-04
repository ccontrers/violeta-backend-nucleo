package com.lavioleta.desarrollo.violetaserver.bitacora_privilegios.controller;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.util.StringUtils;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import com.lavioleta.desarrollo.violetaserver.bitacora_privilegios.dto.request.BitacoraPrivilegiosRequest;
import com.lavioleta.desarrollo.violetaserver.bitacora_privilegios.dto.response.BitacoraPrivilegiosResponse;
import com.lavioleta.desarrollo.violetaserver.bitacora_privilegios.service.BitacoraPrivilegiosService;

import io.swagger.v3.oas.annotations.Operation;
import io.swagger.v3.oas.annotations.responses.ApiResponse;
import io.swagger.v3.oas.annotations.responses.ApiResponses;
import io.swagger.v3.oas.annotations.tags.Tag;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.validation.Valid;

/**
 * Controlador REST para bitácora de modificaciones de privilegios.
 * Migrado desde FormBitacoraModPrivilegios.cpp
 * 
 * Combo boxes utilizados por este módulo:
 * - ComboBoxEmpleado: GET /api/v1/usuarios/combo-box
 * - ComboBoxRol: GET /api/v1/roles/combo-box
 * - ComboBoxTipo y ComboBoxEntidad: valores fijos manejados en frontend
 */
@RestController
@RequestMapping("/api/v1/bitacora-privilegios")
@Tag(name = "Bitácora de Privilegios", description = "Consulta de modificaciones a privilegios de usuarios y roles")
public class BitacoraPrivilegiosController {

    private static final Logger log = LoggerFactory.getLogger(BitacoraPrivilegiosController.class);

    private final BitacoraPrivilegiosService service;

    public BitacoraPrivilegiosController(BitacoraPrivilegiosService service) {
        this.service = service;
    }

    @PostMapping
    @Operation(
            summary = "Consultar bitácora de privilegios",
            description = "Consulta la bitácora de modificaciones de privilegios para usuarios y roles. " +
                    "Migrado desde FormBitacoraModPrivilegios."
    )
    @ApiResponses({
            @ApiResponse(responseCode = "200", description = "Consulta ejecutada correctamente"),
            @ApiResponse(responseCode = "400", description = "Parámetros inválidos (fechas, rango, valores)"),
            @ApiResponse(responseCode = "500", description = "Error interno del servidor")
    })
    public ResponseEntity<BitacoraPrivilegiosResponse> consultar(
            @Valid @RequestBody BitacoraPrivilegiosRequest request,
            HttpServletRequest httpRequest) {
        try {
            log.info("Consultando bitácora de privilegios desde IP {} para rango {} - {}",
                    obtenerClientIp(httpRequest), request.getFechaInicio(), request.getFechaFin());

            BitacoraPrivilegiosResponse response = service.consultar(request);
            HttpStatus status = response.isSuccess() ? HttpStatus.OK : HttpStatus.BAD_REQUEST;
            return ResponseEntity.status(status).body(response);

        } catch (Exception ex) {
            log.error("Error inesperado al consultar la bitácora de privilegios", ex);
            return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR)
                    .body(BitacoraPrivilegiosResponse.error("Error interno del servidor"));
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
