package com.lavioleta.desarrollo.violetaserver.controller;

import com.lavioleta.desarrollo.violetaserver.dto.request.ProveedorRequest;
import com.lavioleta.desarrollo.violetaserver.dto.response.ProveedorResponse;
import com.lavioleta.desarrollo.violetaserver.service.CatalogoProveedoresService;

import io.swagger.v3.oas.annotations.Operation;
import io.swagger.v3.oas.annotations.Parameter;
import io.swagger.v3.oas.annotations.media.Content;
import io.swagger.v3.oas.annotations.media.Schema;
import io.swagger.v3.oas.annotations.responses.ApiResponse;
import io.swagger.v3.oas.annotations.responses.ApiResponses;
import io.swagger.v3.oas.annotations.tags.Tag;
import jakarta.validation.Valid;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.data.domain.Page;
import org.springframework.data.domain.Pageable;
import org.springframework.data.web.PageableDefault;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.validation.annotation.Validated;
import org.springframework.web.bind.annotation.*;

/**
 * Controlador REST para el catálogo de proveedores
 * Migrado de FormCatalogoProveedores.cpp y ServidorCatalogos
 */
@RestController
@RequestMapping("/api/catalogo/proveedores")
@RequiredArgsConstructor
@Validated
@Slf4j
@Tag(name = "Catálogo de Proveedores", description = "Operaciones CRUD para el catálogo de proveedores")
public class CatalogoProveedoresController {
    
    private final CatalogoProveedoresService catalogoProveedoresService;
    
    /**
     * Consulta todos los proveedores activos con paginación
     */
    @GetMapping
    @Operation(summary = "Consultar proveedores", 
               description = "Obtiene todos los proveedores activos con paginación")
    @ApiResponses(value = {
        @ApiResponse(responseCode = "200", description = "Proveedores encontrados",
                    content = @Content(schema = @Schema(implementation = Page.class))),
        @ApiResponse(responseCode = "500", description = "Error interno del servidor")
    })
    public ResponseEntity<Page<ProveedorResponse>> consultarProveedores(
            @PageableDefault(size = 20, sort = "razonsocial") Pageable pageable) {
        try {
            log.debug("Consultando proveedores con paginación: {}", pageable);
            Page<ProveedorResponse> proveedores = catalogoProveedoresService.findAllProveedores(pageable);
            return ResponseEntity.ok(proveedores);
        } catch (Exception e) {
            log.error("Error al consultar proveedores", e);
            return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR).build();
        }
    }
    
    /**
     * Consulta proveedor por clave
     */
    @GetMapping("/{proveedor}")
    @Operation(summary = "Consultar proveedor por clave",
               description = "Obtiene un proveedor específico por su clave")
    @ApiResponses(value = {
        @ApiResponse(responseCode = "200", description = "Proveedor encontrado",
                    content = @Content(schema = @Schema(implementation = ProveedorResponse.class))),
        @ApiResponse(responseCode = "404", description = "Proveedor no encontrado"),
        @ApiResponse(responseCode = "500", description = "Error interno del servidor")
    })
    public ResponseEntity<ProveedorResponse> consultarProveedor(
            @Parameter(description = "Clave del proveedor", required = true)
            @PathVariable String proveedor) {
        try {
            log.debug("Consultando proveedor: {}", proveedor);
            ProveedorResponse response = catalogoProveedoresService.findByProveedor(proveedor);
            
            if (response != null) {
                return ResponseEntity.ok(response);
            } else {
                return ResponseEntity.notFound().build();
            }
        } catch (Exception e) {
            log.error("Error al consultar proveedor: {}", proveedor, e);
            return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR).build();
        }
    }
    
    /**
     * Búsqueda de proveedores por razón social
     */
    @GetMapping("/buscar/razon-social")
    @Operation(summary = "Buscar proveedores por razón social",
               description = "Busca proveedores que contengan el texto especificado en la razón social")
    @ApiResponses(value = {
        @ApiResponse(responseCode = "200", description = "Búsqueda realizada exitosamente",
                    content = @Content(schema = @Schema(implementation = Page.class))),
        @ApiResponse(responseCode = "400", description = "Parámetros de búsqueda inválidos"),
        @ApiResponse(responseCode = "500", description = "Error interno del servidor")
    })
    public ResponseEntity<Page<ProveedorResponse>> buscarPorRazonSocial(
            @Parameter(description = "Texto a buscar en la razón social", required = true)
            @RequestParam String razonSocial,
            @PageableDefault(size = 20, sort = "razonsocial") Pageable pageable) {
        try {
            if (razonSocial == null || razonSocial.trim().isEmpty()) {
                return ResponseEntity.badRequest().build();
            }
            
            log.debug("Buscando proveedores por razón social: {}", razonSocial);
            Page<ProveedorResponse> proveedores = catalogoProveedoresService.findByRazonSocial(razonSocial.trim(), pageable);
            return ResponseEntity.ok(proveedores);
        } catch (Exception e) {
            log.error("Error al buscar proveedores por razón social: {}", razonSocial, e);
            return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR).build();
        }
    }
    
    /**
     * Búsqueda avanzada de proveedores por múltiples criterios
     */
    @GetMapping("/buscar/avanzada")
    @Operation(summary = "Búsqueda avanzada de proveedores",
               description = "Busca proveedores por múltiples criterios: razón social, RFC, estado, comprador, tipo")
    @ApiResponses(value = {
        @ApiResponse(responseCode = "200", description = "Búsqueda realizada exitosamente",
                    content = @Content(schema = @Schema(implementation = Page.class))),
        @ApiResponse(responseCode = "500", description = "Error interno del servidor")
    })
    public ResponseEntity<Page<ProveedorResponse>> busquedaAvanzada(
            @Parameter(description = "Texto a buscar en la razón social")
            @RequestParam(required = false) String razonSocial,
            @Parameter(description = "RFC del proveedor")
            @RequestParam(required = false) String rfc,
            @Parameter(description = "Estado del proveedor")
            @RequestParam(required = false) String estado,
            @Parameter(description = "Comprador asignado")
            @RequestParam(required = false) String comprador,
            @Parameter(description = "Tipo de proveedor: GASTOS, MERCANCIA")
            @RequestParam(required = false) String tipoProveedor,
            @PageableDefault(size = 20, sort = "razonsocial") Pageable pageable) {
        try {
            log.debug("Búsqueda avanzada de proveedores con criterios múltiples");
            Page<ProveedorResponse> proveedores = catalogoProveedoresService.findByCriteriosMultiples(
                razonSocial, rfc, estado, comprador, tipoProveedor, pageable);
            return ResponseEntity.ok(proveedores);
        } catch (Exception e) {
            log.error("Error en búsqueda avanzada de proveedores", e);
            return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR).build();
        }
    }
    
    /**
     * Graba proveedor (Alta/Modificación)
     * Migrado de ServidorCatalogos::GrabaProveedor
     */
    @PostMapping
    @Operation(summary = "Grabar proveedor", 
               description = "Crea un nuevo proveedor o modifica uno existente según la operación especificada")
    @ApiResponses(value = {
        @ApiResponse(responseCode = "200", description = "Proveedor grabado exitosamente",
                    content = @Content(schema = @Schema(implementation = ProveedorResponse.class))),
        @ApiResponse(responseCode = "201", description = "Proveedor creado exitosamente",
                    content = @Content(schema = @Schema(implementation = ProveedorResponse.class))),
        @ApiResponse(responseCode = "400", description = "Datos de entrada inválidos"),
        @ApiResponse(responseCode = "409", description = "Conflicto - RFC o CURP duplicados"),
        @ApiResponse(responseCode = "500", description = "Error interno del servidor")
    })
    public ResponseEntity<ProveedorResponse> grabarProveedor(
            @Parameter(description = "Datos del proveedor a grabar", required = true)
            @Valid @RequestBody ProveedorRequest request,
            @Parameter(description = "Usuario que realiza la operación", required = true)
            @RequestHeader(value = "X-Usuario") String usuario) {
        try {
            log.info("Procesando solicitud de grabado de proveedor: operación={}, usuario={}", 
                    request.getOperacion(), usuario);
            
            ProveedorResponse response = catalogoProveedoresService.grabaProveedor(request, usuario);
            
            if ("A".equals(request.getOperacion())) {
                return ResponseEntity.status(HttpStatus.CREATED).body(response);
            } else {
                return ResponseEntity.ok(response);
            }
        } catch (RuntimeException e) {
            log.error("Error de negocio al grabar proveedor: {}", e.getMessage());
            if (e.getMessage().contains("Ya existe")) {
                return ResponseEntity.status(HttpStatus.CONFLICT).build();
            }
            return ResponseEntity.badRequest().build();
        } catch (Exception e) {
            log.error("Error al grabar proveedor", e);
            return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR).build();
        }
    }
    
    /**
     * Baja lógica de proveedor
     * Migrado de ServidorCatalogos::BajaProveedor
     */
    @DeleteMapping("/{proveedor}")
    @Operation(summary = "Dar de baja proveedor",
               description = "Realiza la baja lógica de un proveedor (lo marca como inactivo)")
    @ApiResponses(value = {
        @ApiResponse(responseCode = "200", description = "Proveedor dado de baja exitosamente"),
        @ApiResponse(responseCode = "404", description = "Proveedor no encontrado"),
        @ApiResponse(responseCode = "409", description = "Conflicto - El proveedor no se puede dar de baja"),
        @ApiResponse(responseCode = "500", description = "Error interno del servidor")
    })
    public ResponseEntity<Void> bajaProveedor(
            @Parameter(description = "Clave del proveedor a dar de baja", required = true)
            @PathVariable String proveedor,
            @Parameter(description = "Usuario que realiza la operación")
            @RequestHeader(value = "X-Usuario", defaultValue = "SISTEMA") String usuario) {
        try {
            log.info("Procesando baja de proveedor: {}, usuario: {}", proveedor, usuario);
            catalogoProveedoresService.bajaProveedor(proveedor, usuario);
            return ResponseEntity.ok().build();
        } catch (RuntimeException e) {
            log.error("Error de negocio al dar de baja proveedor {}: {}", proveedor, e.getMessage());
            if (e.getMessage().contains("no encontrado")) {
                return ResponseEntity.notFound().build();
            }
            return ResponseEntity.status(HttpStatus.CONFLICT).build();
        } catch (Exception e) {
            log.error("Error al dar de baja proveedor: {}", proveedor, e);
            return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR).build();
        }
    }
    
    /**
     * Endpoint para pruebas de conectividad
     */
    @GetMapping("/ping")
    @Operation(summary = "Ping", description = "Endpoint para verificar que el servicio está disponible")
    @ApiResponse(responseCode = "200", description = "Servicio disponible")
    public ResponseEntity<String> ping() {
        return ResponseEntity.ok("Catálogo de Proveedores - Servicio disponible");
    }
}