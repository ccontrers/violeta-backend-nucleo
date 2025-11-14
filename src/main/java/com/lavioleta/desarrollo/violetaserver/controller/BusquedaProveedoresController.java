package com.lavioleta.desarrollo.violetaserver.controller;

import com.lavioleta.desarrollo.violetaserver.dto.request.BusquedaProveedoresRequest;
import com.lavioleta.desarrollo.violetaserver.dto.response.BusquedaProveedoresResponse;
import com.lavioleta.desarrollo.violetaserver.service.BusquedaProveedoresService;
import jakarta.validation.Valid;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;
import io.swagger.v3.oas.annotations.Operation;
import io.swagger.v3.oas.annotations.tags.Tag;
import io.swagger.v3.oas.annotations.responses.ApiResponse;
import io.swagger.v3.oas.annotations.responses.ApiResponses;

@RestController
@RequestMapping("/api/v1/busqueda")
@Tag(name = "Búsquedas Proveedores", description = "Operaciones de búsqueda de proveedores.")
public class BusquedaProveedoresController {
    
    private static final Logger logger = LoggerFactory.getLogger(BusquedaProveedoresController.class);
    private final BusquedaProveedoresService busquedaProveedoresService;
    
    public BusquedaProveedoresController(BusquedaProveedoresService busquedaProveedoresService) {
        this.busquedaProveedoresService = busquedaProveedoresService;
    }
    
    /**
     * Endpoint para búsqueda de proveedores
     * POST /api/v1/busqueda/proveedores
     * 
     * Implementa la funcionalidad legacy de ServidorBusquedas::BuscaProveedores
     * Soporta 4 tipos de búsqueda: RSO, RFC, CLA, REP
     */
    @PostMapping("/proveedores")
    @Operation(summary = "Buscar proveedores", description = "Búsqueda por códigos RSO, RFC, CLA o REP según request.")
    @ApiResponses({
        @ApiResponse(responseCode = "200", description = "Búsqueda exitosa"),
        @ApiResponse(responseCode = "400", description = "Solicitud inválida"),
        @ApiResponse(responseCode = "500", description = "Error interno")
    })
    public ResponseEntity<BusquedaProveedoresResponse> buscarProveedores(
            @Valid @RequestBody BusquedaProveedoresRequest request) {
        
        logger.info("Recibida solicitud de búsqueda de proveedores - Tipo: {}, Condición: {}", 
                   request.getCodcondicion(), request.getCondicion());
        
        try {
            BusquedaProveedoresResponse response = busquedaProveedoresService.buscarProveedores(request);
            
            if (response.isSuccess()) {
                logger.debug("Búsqueda exitosa - {} proveedores encontrados", response.getTotalResultados());
                return ResponseEntity.ok(response);
            } else {
                logger.warn("Búsqueda falló: {}", response.getMessage());
                return ResponseEntity.badRequest().body(response);
            }
            
        } catch (Exception e) {
            logger.error("Error inesperado en búsqueda de proveedores", e);
            
            BusquedaProveedoresResponse errorResponse = BusquedaProveedoresResponse.builder()
                    .success(false)
                    .message("Error interno del servidor")
                    .totalResultados(0)
                    .build();
                    
            return ResponseEntity.internalServerError().body(errorResponse);
        }
    }
}