package com.lavioleta.desarrollo.violetaserver.controller;

import com.lavioleta.desarrollo.violetaserver.dto.request.BusquedaVendedoresRequest;
import com.lavioleta.desarrollo.violetaserver.dto.response.BusquedaVendedoresResponse;
import com.lavioleta.desarrollo.violetaserver.service.BusquedaVendedoresService;
import io.swagger.v3.oas.annotations.Operation;
import io.swagger.v3.oas.annotations.responses.ApiResponse;
import io.swagger.v3.oas.annotations.responses.ApiResponses;
import io.swagger.v3.oas.annotations.tags.Tag;
import jakarta.validation.Valid;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

/**
 * Controller REST para búsqueda de vendedores
 * Migrado desde FormBusquedaVendedores.cpp y ServidorBusquedas::BuscaVendedores
 * 
 * Endpoint: POST /api/v1/busqueda/vendedores
 * 
 * Implementa la funcionalidad legacy con 4 tipos de búsqueda:
 * - NOM: Por nombre del vendedor (e.nombre LIKE 'valor%')
 * - APE: Por apellidos (e.appat LIKE 'valor%' OR e.apmat LIKE 'valor%')
 * - COMI: Por tipo de comisión (v.tipocomi = 'valor')
 * - CLA: Por clave de empleado (e.empleado LIKE 'valor%')
 * 
 * Petición ID legacy: ID_BUSQ_VENDEDOR
 */
@RestController
@RequestMapping("/api/v1/busqueda")
@Tag(name = "Búsquedas Vendedores", description = "Operaciones de búsqueda de vendedores.")
public class BusquedaVendedoresController {
    
    private static final Logger logger = LoggerFactory.getLogger(BusquedaVendedoresController.class);
    private final BusquedaVendedoresService busquedaVendedoresService;
    
    public BusquedaVendedoresController(BusquedaVendedoresService busquedaVendedoresService) {
        this.busquedaVendedoresService = busquedaVendedoresService;
    }
    
    /**
     * Endpoint para búsqueda de vendedores
     * POST /api/v1/busqueda/vendedores
     * 
     * Implementa la funcionalidad legacy de ServidorBusquedas::BuscaVendedores
     * Soporta 4 tipos de búsqueda: NOM, APE, COMI, CLA
     * 
     * Parámetros legacy (buffer):
     * 1. tipo_busqueda: "NOM" | "APE" | "COMI" | "CLA"
     * 2. solo_activos: "1" (activos) | "0" (inactivos)
     * 3. dato_buscado: texto a buscar
     * 
     * SQL legacy siempre usa: LIMIT NUM_LIMITE_RESULTADOS_BUSQ (501)
     */
    @PostMapping("/vendedores")
    @Operation(
        summary = "Buscar vendedores",
        description = """
            Búsqueda de vendedores por diferentes criterios:
            - NOM: Nombre del vendedor (búsqueda por prefijo)
            - APE: Apellidos paterno o materno (búsqueda por prefijo)
            - COMI: Tipo de comisión (búsqueda exacta)
            - CLA: Clave de empleado (búsqueda por prefijo)
            
            Retorna empleado, nombre completo, localidad y tipo de comisión.
            Filtro opcional por estado activo/inactivo.
            """
    )
    @ApiResponses({
        @ApiResponse(responseCode = "200", description = "Búsqueda exitosa"),
        @ApiResponse(responseCode = "400", description = "Solicitud inválida"),
        @ApiResponse(responseCode = "500", description = "Error interno")
    })
    public ResponseEntity<BusquedaVendedoresResponse> buscarVendedores(
            @Valid @RequestBody BusquedaVendedoresRequest request) {
        
        logger.info("Recibida solicitud de búsqueda de vendedores - Tipo: {}, Valor: {}", 
                   request.getTipoBusqueda(), request.getValor());
        
        try {
            BusquedaVendedoresResponse response = busquedaVendedoresService.buscarVendedores(request);
            
            if (response.isSuccess()) {
                logger.debug("Búsqueda exitosa - {} vendedores encontrados", response.getTotalResultados());
                return ResponseEntity.ok(response);
            } else {
                logger.warn("Búsqueda falló: {}", response.getMessage());
                return ResponseEntity.badRequest().body(response);
            }
            
        } catch (Exception e) {
            logger.error("Error inesperado en búsqueda de vendedores", e);
            
            BusquedaVendedoresResponse errorResponse = BusquedaVendedoresResponse.builder()
                    .success(false)
                    .message("Error interno del servidor")
                    .totalResultados(0)
                    .build();
                    
            return ResponseEntity.internalServerError().body(errorResponse);
        }
    }
}
