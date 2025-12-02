package com.lavioleta.desarrollo.violetaserver.usuarios.controller;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.util.StringUtils;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import com.lavioleta.desarrollo.violetaserver.usuarios.dto.request.AsignarClaveRequest;
import com.lavioleta.desarrollo.violetaserver.usuarios.dto.request.CambiarClaveRequest;
import com.lavioleta.desarrollo.violetaserver.usuarios.dto.response.ClaveResponse;
import com.lavioleta.desarrollo.violetaserver.usuarios.service.ClaveUsuarioService;

import io.swagger.v3.oas.annotations.Operation;
import io.swagger.v3.oas.annotations.Parameter;
import io.swagger.v3.oas.annotations.media.Content;
import io.swagger.v3.oas.annotations.media.ExampleObject;
import io.swagger.v3.oas.annotations.media.Schema;
import io.swagger.v3.oas.annotations.responses.ApiResponse;
import io.swagger.v3.oas.annotations.responses.ApiResponses;
import io.swagger.v3.oas.annotations.tags.Tag;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.validation.Valid;

/**
 * Controlador REST para gestión de claves de usuario.
 * Migración de FormSistemaCambiarClave + ServidorAdminSistema::CambiaClave / AsignaPassword.
 */
@RestController
@RequestMapping("/api/v1/usuarios")
@Tag(name = "Gestión de Claves", description = "Endpoints para asignar y cambiar claves de usuario (migrado desde FormSistemaCambiarClave)")
public class ClaveUsuarioController {

    private static final Logger log = LoggerFactory.getLogger(ClaveUsuarioController.class);

    private final ClaveUsuarioService service;

    public ClaveUsuarioController(ClaveUsuarioService service) {
        this.service = service;
    }

    @PostMapping("/{id}/clave")
    @Operation(
        summary = "Asignar clave inicial",
        description = "Asigna una clave inicial a un usuario que no tiene password (password IS NULL). " +
                      "Equivalente a ServidorAdminSistema::AsignaPassword del sistema legado. " +
                      "La clave debe enviarse ya cifrada en SHA-256 (64 caracteres hexadecimales)."
    )
    @ApiResponses({
        @ApiResponse(
            responseCode = "201",
            description = "Clave asignada correctamente",
            content = @Content(
                mediaType = "application/json",
                schema = @Schema(implementation = ClaveResponse.class),
                examples = @ExampleObject(value = "{\"success\":true,\"usuario\":\"CRCP\",\"message\":\"Clave asignada correctamente\"}")
            )
        ),
        @ApiResponse(
            responseCode = "400",
            description = "Petición inválida (validación de formato hash)",
            content = @Content(mediaType = "application/json")
        ),
        @ApiResponse(
            responseCode = "404",
            description = "Usuario no encontrado",
            content = @Content(
                mediaType = "application/json",
                examples = @ExampleObject(value = "{\"success\":false,\"usuario\":null,\"message\":\"Usuario no encontrado\"}")
            )
        ),
        @ApiResponse(
            responseCode = "409",
            description = "Conflicto: el usuario ya tiene clave asignada",
            content = @Content(
                mediaType = "application/json",
                examples = @ExampleObject(value = "{\"success\":false,\"usuario\":null,\"message\":\"El usuario ya tiene una clave asignada. Use el endpoint de cambio de clave.\"}")
            )
        )
    })
    public ResponseEntity<ClaveResponse> asignarClave(
            @Parameter(description = "Identificador del usuario (empleado)", example = "CRCP")
            @PathVariable("id") String empleado,
            @Valid @RequestBody AsignarClaveRequest request,
            HttpServletRequest httpRequest) {
        try {
            log.info("Asignando clave inicial a usuario {} desde IP {}", empleado, obtenerClientIp(httpRequest));
            ClaveResponse response = service.asignarClave(empleado, request);

            HttpStatus status;
            if (response.isSuccess()) {
                status = HttpStatus.CREATED;
            } else if (response.getMessage() != null && response.getMessage().contains("no encontrado")) {
                status = HttpStatus.NOT_FOUND;
            } else if (response.getMessage() != null && response.getMessage().contains("ya tiene")) {
                status = HttpStatus.CONFLICT;
            } else {
                status = HttpStatus.BAD_REQUEST;
            }

            return ResponseEntity.status(status).body(response);
        } catch (Exception ex) {
            log.error("Error inesperado al asignar clave al usuario {}", empleado, ex);
            return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR)
                    .body(ClaveResponse.error("Error interno del servidor"));
        }
    }

    @PutMapping("/{id}/clave")
    @Operation(
        summary = "Cambiar clave existente",
        description = "Cambia la clave de un usuario que ya tiene password. " +
                      "Equivalente a ServidorAdminSistema::CambiaClave del sistema legado. " +
                      "Ambas claves (actual y nueva) deben enviarse ya cifradas en SHA-256 (64 caracteres hexadecimales)."
    )
    @ApiResponses({
        @ApiResponse(
            responseCode = "200",
            description = "Clave cambiada correctamente",
            content = @Content(
                mediaType = "application/json",
                schema = @Schema(implementation = ClaveResponse.class),
                examples = @ExampleObject(value = "{\"success\":true,\"usuario\":\"CRCP\",\"message\":\"Clave cambiada correctamente\"}")
            )
        ),
        @ApiResponse(
            responseCode = "400",
            description = "Petición inválida (validación de formato hash)",
            content = @Content(mediaType = "application/json")
        ),
        @ApiResponse(
            responseCode = "401",
            description = "Clave actual incorrecta",
            content = @Content(
                mediaType = "application/json",
                examples = @ExampleObject(value = "{\"success\":false,\"usuario\":null,\"message\":\"La clave actual no es correcta\"}")
            )
        ),
        @ApiResponse(
            responseCode = "404",
            description = "Usuario no encontrado",
            content = @Content(
                mediaType = "application/json",
                examples = @ExampleObject(value = "{\"success\":false,\"usuario\":null,\"message\":\"Usuario no encontrado\"}")
            )
        ),
        @ApiResponse(
            responseCode = "409",
            description = "Conflicto (reservado para futuras validaciones)",
            content = @Content(mediaType = "application/json")
        )
    })
    public ResponseEntity<ClaveResponse> cambiarClave(
            @Parameter(description = "Identificador del usuario (empleado)", example = "CRCP")
            @PathVariable("id") String empleado,
            @Valid @RequestBody CambiarClaveRequest request,
            HttpServletRequest httpRequest) {
        try {
            log.info("Cambiando clave de usuario {} desde IP {}", empleado, obtenerClientIp(httpRequest));
            ClaveResponse response = service.cambiarClave(empleado, request);

            HttpStatus status;
            if (response.isSuccess()) {
                status = HttpStatus.OK;
            } else if (response.getMessage() != null && response.getMessage().contains("no encontrado")) {
                status = HttpStatus.NOT_FOUND;
            } else if (response.getMessage() != null && response.getMessage().contains("no es correcta")) {
                status = HttpStatus.UNAUTHORIZED;
            } else {
                status = HttpStatus.BAD_REQUEST;
            }

            return ResponseEntity.status(status).body(response);
        } catch (Exception ex) {
            log.error("Error inesperado al cambiar clave del usuario {}", empleado, ex);
            return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR)
                    .body(ClaveResponse.error("Error interno del servidor"));
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
