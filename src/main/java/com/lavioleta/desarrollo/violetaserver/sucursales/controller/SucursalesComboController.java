package com.lavioleta.desarrollo.violetaserver.sucursales.controller;

import java.util.List;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.util.StringUtils;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import com.lavioleta.desarrollo.violetaserver.sucursales.dto.response.SucursalComboOptionResponse;
import com.lavioleta.desarrollo.violetaserver.sucursales.service.SucursalesService;

import io.swagger.v3.oas.annotations.Operation;
import io.swagger.v3.oas.annotations.responses.ApiResponse;
import io.swagger.v3.oas.annotations.responses.ApiResponses;
import io.swagger.v3.oas.annotations.tags.Tag;
import jakarta.servlet.http.HttpServletRequest;

@RestController
@RequestMapping("/api/v1/sucursales")
@Tag(name = "Sucursales", description = "Consultas auxiliares para sucursales")
public class SucursalesComboController {

    private static final Logger log = LoggerFactory.getLogger(SucursalesComboController.class);

    private final SucursalesService service;

    public SucursalesComboController(SucursalesService service) {
        this.service = service;
    }

    @GetMapping("/combo-box")
    @Operation(summary = "ComboBox de sucursales", description = "Lista sucursales para combos, con filtro opcional por empresa.")
    @ApiResponses({
            @ApiResponse(responseCode = "200", description = "Consulta ejecutada"),
            @ApiResponse(responseCode = "500", description = "Error interno")
    })
    public ResponseEntity<List<SucursalComboOptionResponse>> comboSucursales(
            @RequestParam(value = "idEmpresa", required = false) Integer idEmpresa,
            HttpServletRequest request) {
        try {
            log.debug("Consultando combo de sucursales desde IP {} (idEmpresa={})",
                    obtenerClientIp(request), idEmpresa);
            List<SucursalComboOptionResponse> items = service.listarSucursalesCombo(idEmpresa);
            return ResponseEntity.ok(items);
        } catch (Exception ex) {
            log.error("Error inesperado al consultar combo de sucursales", ex);
            return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR).build();
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
