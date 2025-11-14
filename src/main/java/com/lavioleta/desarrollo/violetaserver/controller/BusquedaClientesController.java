package com.lavioleta.desarrollo.violetaserver.controller;

import com.lavioleta.desarrollo.violetaserver.dto.request.BusquedaClientesRequest;
import com.lavioleta.desarrollo.violetaserver.dto.response.BusquedaClientesResponse;
import com.lavioleta.desarrollo.violetaserver.service.BusquedaClientesService;
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
@Tag(name = "Búsquedas Clientes", description = "Operaciones de búsqueda de clientes por criterios.")
public class BusquedaClientesController {
    
    private static final Logger logger = LoggerFactory.getLogger(BusquedaClientesController.class);
    private final BusquedaClientesService busquedaClientesService;
    
    public BusquedaClientesController(BusquedaClientesService busquedaClientesService) {
        this.busquedaClientesService = busquedaClientesService;
    }
    
    /**
     * Endpoint para búsqueda de clientes
     * POST /api/v1/busqueda/clientes
     */
    @PostMapping("/clientes")
    @Operation(summary = "Buscar clientes", description = "Realiza búsqueda de clientes según tipo/condición enviada en el request.")
    @ApiResponses({
        @ApiResponse(responseCode = "200", description = "Búsqueda exitosa"),
        @ApiResponse(responseCode = "400", description = "Solicitud inválida / validación"),
        @ApiResponse(responseCode = "500", description = "Error interno")
    })
    public ResponseEntity<BusquedaClientesResponse> buscarClientes(
            @Valid @RequestBody BusquedaClientesRequest request) {
        
        logger.info("Recibida solicitud de búsqueda de clientes - Tipo: {}, Condición: {}", 
                   request.getCodcondicion(), request.getCondicion());
        
        try {
            BusquedaClientesResponse response = busquedaClientesService.buscarClientes(request);
            
            if (response.isSuccess()) {
                logger.debug("Búsqueda exitosa - {} clientes encontrados", response.getTotalResultados());
                return ResponseEntity.ok(response);
            } else {
                logger.warn("Búsqueda falló: {}", response.getMessage());
                return ResponseEntity.badRequest().body(response);
            }
            
        } catch (Exception e) {
            logger.error("Error inesperado en búsqueda de clientes", e);
            
            BusquedaClientesResponse errorResponse = BusquedaClientesResponse.builder()
                    .success(false)
                    .message("Error interno del servidor")
                    .totalResultados(0)
                    .build();
                    
            return ResponseEntity.internalServerError().body(errorResponse);
        }
    }
}
