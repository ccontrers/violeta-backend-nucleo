package com.lavioleta.desarrollo.violetaserver.controller;

import com.lavioleta.desarrollo.violetaserver.config.AppProperties;
import com.lavioleta.desarrollo.violetaserver.dto.request.BusquedaArticulosRequest;
import com.lavioleta.desarrollo.violetaserver.dto.response.BusquedaArticulosResponse;
import com.lavioleta.desarrollo.violetaserver.service.BusquedaArticulosService;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.Map;

import io.swagger.v3.oas.annotations.Operation;
import io.swagger.v3.oas.annotations.tags.Tag;
import io.swagger.v3.oas.annotations.responses.ApiResponse;
import io.swagger.v3.oas.annotations.responses.ApiResponses;

@RestController
@RequestMapping("/api/v1/ejemplo/busqueda")
@Tag(name = "Búsquedas Artículos", description = "Operaciones de búsqueda y metadatos de artículos")
public class BusquedaArticulosController {
    
    private static final Logger logger = LoggerFactory.getLogger(BusquedaArticulosController.class);
    private final BusquedaArticulosService busquedaService;
    private final AppProperties appProperties;
    
    public BusquedaArticulosController(BusquedaArticulosService busquedaService,
                                              AppProperties appProperties) {
        this.busquedaService = busquedaService;
        this.appProperties = appProperties;
    }
    
    @PostMapping("/articulos")
    @Operation(summary = "Buscar artículos", description = "Permite buscar artículos por distintos criterios (tipo, sucursal, dato ingresado).")
    @ApiResponses({
        @ApiResponse(responseCode = "200", description = "Búsqueda exitosa / resultados (success=true)"),
        @ApiResponse(responseCode = "400", description = "Validación fallida (sucursal faltante u otros)"),
        @ApiResponse(responseCode = "500", description = "Error interno del servidor")
    })
    public ResponseEntity<BusquedaArticulosResponse> buscarArticulos(
            @RequestBody BusquedaArticulosRequest request) {
        
        logger.info("Recibida solicitud de búsqueda de artículos: tipo={}, sucursal={}, dato={}", 
                   request.getTipoBusqueda(), request.getSucursal(), request.getDatoBuscado());
        
        try {
            // Validaciones básicas
            if (request.getSucursal() == null || request.getSucursal().trim().isEmpty()) {
                return ResponseEntity.badRequest().body(
                    new BusquedaArticulosResponse(false, "La sucursal es requerida"));
            }
            
            BusquedaArticulosResponse response = busquedaService.buscarArticulos(request);
            
            if (response.isSuccess()) {
                return ResponseEntity.ok(response);
            } else {
                return ResponseEntity.badRequest().body(response);
            }
            
        } catch (Exception e) {
            logger.error("Error en endpoint de búsqueda de artículos: {}", e.getMessage(), e);
            
            BusquedaArticulosResponse errorResponse = new BusquedaArticulosResponse(
                false, "Error interno del servidor: " + e.getMessage());
            
            return ResponseEntity.internalServerError().body(errorResponse);
        }
    }
    
    @GetMapping("/ping")
    @Operation(summary = "Ping del servicio de búsqueda de artículos", description = "Endpoint simple para monitoreo / health check.")
    public ResponseEntity<String> ping() {
        return ResponseEntity.ok("Servicio de búsqueda de artículos funcionando correctamente");
    }
    
    @GetMapping("/articulos/tipos")
    @Operation(summary = "Tipos de búsqueda disponibles", description = "Lista códigos de tipos de búsqueda soportados.")
    public ResponseEntity<String[]> obtenerTiposBusqueda() {
        String[] tipos = {"N", "C", "M", "E", "CB", "ART", ""};
        return ResponseEntity.ok(tipos);
    }
    
    @GetMapping("/configuracion/sucursal")
    @Operation(summary = "Sucursal activa por defecto", description = "Devuelve la sucursal configurada por defecto en el sistema.")
    public ResponseEntity<Map<String, String>> obtenerSucursalActiva() {
        logger.info("Solicitando configuración de sucursal activa: {}", appProperties.getSucursal().getActiva());
        return ResponseEntity.ok(Map.of(
            "sucursalActiva", appProperties.getSucursal().getActiva(),
            "descripcion", "Sucursal configurada por defecto en el sistema"
        ));
    }
    
    @GetMapping("/articulos/{articulo}/detalles")
    @Operation(summary = "Detalles de un artículo", description = "Obtiene información detallada de un artículo para una sucursal específica (o la por defecto).")
    public ResponseEntity<?> obtenerDetallesArticulo(
            @PathVariable String articulo,
            @RequestParam(required = false) String sucursal) {
        
        // Usar sucursal por defecto si no se especifica
        String sucursalActiva = sucursal != null ? sucursal : appProperties.getSucursal().getActiva();
        
        logger.info("Recibida solicitud de detalles para artículo: {} en sucursal: {} (configurada: {})", 
                   articulo, sucursalActiva, appProperties.getSucursal().getActiva());
        
        try {
            var detalles = busquedaService.obtenerDetallesArticulo(articulo, sucursalActiva);
            return ResponseEntity.ok(detalles);
        } catch (Exception e) {
            logger.error("Error al obtener detalles del artículo {}: {}", articulo, e.getMessage(), e);
            return ResponseEntity.internalServerError()
                    .body(Map.of("error", "Error al obtener detalles: " + e.getMessage()));
        }
    }
}
