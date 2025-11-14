package com.lavioleta.desarrollo.violetaserver.controller;

import com.lavioleta.desarrollo.violetaserver.dto.request.ClienteRequest;
import com.lavioleta.desarrollo.violetaserver.dto.request.BajaClienteRequest;
import com.lavioleta.desarrollo.violetaserver.dto.response.ClienteResponse;
import com.lavioleta.desarrollo.violetaserver.dto.response.ClienteListResponse;
import com.lavioleta.desarrollo.violetaserver.service.CatalogoClientesService;
import com.lavioleta.desarrollo.violetaserver.dto.DatosCreditoDTO;
import com.lavioleta.desarrollo.violetaserver.dto.ClienteDetalleEcommerceDTO;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.validation.Valid;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;
import io.swagger.v3.oas.annotations.Operation;
import io.swagger.v3.oas.annotations.tags.Tag;
import io.swagger.v3.oas.annotations.responses.ApiResponse;
import io.swagger.v3.oas.annotations.responses.ApiResponses;

/**
 * Controller REST para catálogo de clientes
 * Endpoints equivalentes a FormCatalogoClientes.cpp
 * IDs de operación: ID_GRA_CLIENTE, ID_BAJ_CLIENTE
 */
@RestController
@RequestMapping("/api/v1/catalogos/clientes")
@Tag(name = "Catálogo Clientes", description = "Gestión CRUD y datos asociados de clientes")
public class CatalogoClientesController {
    
    private static final Logger logger = LoggerFactory.getLogger(CatalogoClientesController.class);
    private final CatalogoClientesService clientesService;
    
    public CatalogoClientesController(CatalogoClientesService clientesService) { this.clientesService = clientesService; }
    
    /**
     * Graba cliente (alta o modificación)
     * POST /api/v1/catalogos/clientes
     * Equivalente a ID_GRA_CLIENTE
     */
    @PostMapping
    @Operation(summary = "Crear/Actualizar cliente", description = "Alta o modificación de cliente (ID_GRA_CLIENTE)")
    @ApiResponses({
        @ApiResponse(responseCode = "200", description = "Operación exitosa"),
        @ApiResponse(responseCode = "400", description = "Validación o datos incorrectos"),
        @ApiResponse(responseCode = "500", description = "Error interno")
    })
    public ResponseEntity<ClienteResponse> grabarCliente(
            @Valid @RequestBody ClienteRequest request,
            @RequestHeader(value = "X-Sucursal", defaultValue = "S1") String sucursal,
            HttpServletRequest httpRequest) {
        
        try {
            String clientIp = obtenerClientIp(httpRequest);
            logger.info("Procesando grabado de cliente desde IP: {} - Operación: {}, Cliente: {}", 
                       clientIp, request.getOperacion(), request.getCliente());
            
            ClienteResponse response = clientesService.grabarCliente(request, sucursal);
            
            HttpStatus status = response.getSuccess() ? HttpStatus.OK : HttpStatus.BAD_REQUEST;
            return ResponseEntity.status(status).body(response);
            
        } catch (Exception e) {
            logger.error("Error inesperado al grabar cliente: {}", e.getMessage(), e);
            ClienteResponse errorResponse = ClienteResponse.error("Error interno del servidor");
            return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR).body(errorResponse);
        }
    }
    
    /**
     * Elimina cliente
     * DELETE /api/v1/catalogos/clientes/{codigoCliente}
     * Equivalente a ID_BAJ_CLIENTE
     */
    @DeleteMapping("/{codigoCliente}")
    @Operation(summary = "Eliminar cliente", description = "Elimina un cliente por código (ID_BAJ_CLIENTE)")
    @ApiResponses({
        @ApiResponse(responseCode = "200", description = "Cliente eliminado"),
        @ApiResponse(responseCode = "400", description = "Petición inválida"),
        @ApiResponse(responseCode = "404", description = "Cliente no encontrado"),
        @ApiResponse(responseCode = "500", description = "Error interno")
    })
    public ResponseEntity<ClienteResponse> eliminarCliente(
            @PathVariable String codigoCliente,
            @RequestBody(required = false) BajaClienteRequest request,
            HttpServletRequest httpRequest) {
        
        try {
            String clientIp = obtenerClientIp(httpRequest);
            logger.info("Procesando eliminación de cliente desde IP: {} - Cliente: {}", 
                       clientIp, codigoCliente);
            
            // Si no se envía request body, crear uno básico
            if (request == null) {
                request = new BajaClienteRequest();
                request.setCliente(codigoCliente);
            } else {
                request.setCliente(codigoCliente); // Asegurar que coincida con el path
            }
            
            ClienteResponse response = clientesService.eliminarCliente(request);
            
            HttpStatus status = response.getSuccess() ? HttpStatus.OK : HttpStatus.BAD_REQUEST;
            return ResponseEntity.status(status).body(response);
            
        } catch (Exception e) {
            logger.error("Error inesperado al eliminar cliente {}: {}", codigoCliente, e.getMessage(), e);
            ClienteResponse errorResponse = ClienteResponse.error("Error interno del servidor");
            return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR).body(errorResponse);
        }
    }
    
    /**
     * Consulta un cliente específico
     * GET /api/v1/catalogos/clientes/{codigoCliente}
     */
    @GetMapping("/{codigoCliente}")
    @Operation(summary = "Consultar cliente", description = "Obtiene datos de un cliente existente")
    @ApiResponses({
        @ApiResponse(responseCode = "200", description = "Cliente encontrado"),
        @ApiResponse(responseCode = "404", description = "No existe"),
        @ApiResponse(responseCode = "500", description = "Error interno")
    })
    public ResponseEntity<ClienteResponse> consultarCliente(
            @PathVariable String codigoCliente,
            @RequestHeader(value = "X-Sucursal", defaultValue = "S1") String sucursal,
            HttpServletRequest httpRequest) {
        
        try {
            String clientIp = obtenerClientIp(httpRequest);
            logger.debug("Consultando cliente desde IP: {} - Cliente: {}, Sucursal: {}", 
                        clientIp, codigoCliente, sucursal);
            
            // Obtener empresa de la sucursal automáticamente
            Integer idEmpresa = clientesService.obtenerEmpresaPorSucursal(sucursal);
            
            ClienteResponse response = clientesService.consultarCliente(codigoCliente, idEmpresa);
            
            HttpStatus status = response.getSuccess() ? HttpStatus.OK : HttpStatus.NOT_FOUND;
            return ResponseEntity.status(status).body(response);
            
        } catch (Exception e) {
            logger.error("Error inesperado al consultar cliente {}: {}", codigoCliente, e.getMessage(), e);
            ClienteResponse errorResponse = ClienteResponse.error("Error interno del servidor");
            return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR).body(errorResponse);
        }
    }
    
    /**
     * Lista clientes con filtros
     * GET /api/v1/catalogos/clientes
     */
    @GetMapping
    @Operation(summary = "Listar clientes", description = "Listar clientes con paginación y filtros")
    public ResponseEntity<ClienteListResponse> listarClientes(
            @RequestParam(value = "filtros", required = false) String filtros,
            @RequestParam(value = "pagina", defaultValue = "1") Integer pagina,
            @RequestParam(value = "registrosPorPagina", defaultValue = "20") Integer registrosPorPagina,
            @RequestHeader(value = "X-Sucursal", defaultValue = "S1") String sucursal,
            HttpServletRequest httpRequest) {
        
        try {
            String clientIp = obtenerClientIp(httpRequest);
            logger.debug("Listando clientes desde IP: {} - Filtros: {}, Página: {}, Sucursal: {}", 
                        clientIp, filtros, pagina, sucursal);
            
            // Obtener empresa de la sucursal automáticamente
            Integer idEmpresa = clientesService.obtenerEmpresaPorSucursal(sucursal);
            
            ClienteListResponse response = clientesService.listarClientes(
                filtros, pagina, registrosPorPagina, idEmpresa);
            
            HttpStatus status = response.getSuccess() ? HttpStatus.OK : HttpStatus.BAD_REQUEST;
            return ResponseEntity.status(status).body(response);
            
        } catch (Exception e) {
            logger.error("Error inesperado al listar clientes: {}", e.getMessage(), e);
            ClienteListResponse errorResponse = ClienteListResponse.error("Error interno del servidor");
            return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR).body(errorResponse);
        }
    }
    
    /**
     * Verifica si un cliente existe
     * HEAD /api/v1/catalogos/clientes/{codigoCliente}
     */
    @RequestMapping(value = "/{codigoCliente}", method = RequestMethod.HEAD)
    @Operation(summary = "Verificar existencia cliente", description = "HEAD para validar existencia de un cliente")
    public ResponseEntity<Void> verificarExistenciaCliente(
            @PathVariable String codigoCliente,
            HttpServletRequest httpRequest) {
        
        try {
            String clientIp = obtenerClientIp(httpRequest);
            logger.debug("Verificando existencia de cliente desde IP: {} - Cliente: {}", 
                        clientIp, codigoCliente);
            
            boolean existe = clientesService.existeCliente(codigoCliente);
            
            return ResponseEntity.status(existe ? HttpStatus.OK : HttpStatus.NOT_FOUND).build();
            
        } catch (Exception e) {
            logger.error("Error al verificar existencia del cliente {}: {}", codigoCliente, e.getMessage(), e);
            return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR).build();
        }
    }
    
    /**
     * Endpoint de estado del servicio
     * GET /api/v1/catalogos/clientes/status
     */
    @GetMapping("/status")
    @Operation(summary = "Estado servicio clientes", description = "Health endpoint del módulo")
    public ResponseEntity<String> status() {
        return ResponseEntity.ok("Servicio de catálogo de clientes disponible");
    }
    
    /**
     * Obtiene la IP real del cliente considerando proxies y load balancers
     */
    private String obtenerClientIp(HttpServletRequest request) {
        String xForwardedFor = request.getHeader("X-Forwarded-For");
        if (xForwardedFor != null && !xForwardedFor.isEmpty()) {
            return xForwardedFor.split(",")[0].trim();
        }
        
        String xRealIp = request.getHeader("X-Real-IP");
        if (xRealIp != null && !xRealIp.isEmpty()) {
            return xRealIp;
        }
        
        return request.getRemoteAddr();
    }
    
    /**
     * Obtiene datos de crédito de un cliente
     * GET /api/v1/catalogos/clientes/{idCliente}/credito
     */
    @GetMapping("/{idCliente}/credito")
    @Operation(summary = "Obtener datos crédito", description = "Consulta datos de crédito de un cliente")
    public ResponseEntity<DatosCreditoDTO> obtenerDatosCredito(
            @PathVariable String idCliente,
            HttpServletRequest httpRequest) {
        
        try {
            String clientIp = obtenerClientIp(httpRequest);
            logger.info("Consultando datos de crédito desde IP: {} para cliente: {}", clientIp, idCliente);
            
            DatosCreditoDTO datosCredito = clientesService.obtenerDatosCredito(idCliente);
            
            if (datosCredito != null) {
                logger.info("Datos de crédito encontrados para cliente: {}", idCliente);
                return ResponseEntity.ok(datosCredito);
            } else {
                logger.info("No se encontraron datos de crédito para cliente: {}", idCliente);
                return ResponseEntity.notFound().build();
            }
            
        } catch (Exception e) {
            logger.error("Error al consultar datos de crédito para cliente {}: {}", idCliente, e.getMessage(), e);
            return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR).build();
        }
    }
    
    /**
     * Guarda o actualiza datos de crédito de un cliente
     * POST /api/v1/catalogos/clientes/{idCliente}/credito
     */
    @PostMapping("/{idCliente}/credito")
    @Operation(summary = "Guardar datos crédito", description = "Crea o actualiza datos de crédito del cliente")
    public ResponseEntity<DatosCreditoDTO> guardarDatosCredito(
            @PathVariable String idCliente,
            @Valid @RequestBody DatosCreditoDTO datosCredito,
            HttpServletRequest httpRequest) {
        
        try {
            String clientIp = obtenerClientIp(httpRequest);
            logger.info("Guardando datos de crédito desde IP: {} para cliente: {}", clientIp, idCliente);
            
            // Asegurar que el ID del cliente coincida
            datosCredito.setIdcliente(idCliente);
            
            DatosCreditoDTO resultado = clientesService.guardarDatosCredito(datosCredito);
            
            logger.info("Datos de crédito guardados exitosamente para cliente: {}", idCliente);
            return ResponseEntity.ok(resultado);
            
        } catch (Exception e) {
            logger.error("Error al guardar datos de crédito para cliente {}: {}", idCliente, e.getMessage(), e);
            return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR).build();
        }
    }
    
    /**
     * Obtiene detalles de ecommerce de un cliente
     * GET /api/v1/catalogos/clientes/{idCliente}/ecommerce
     */
    @GetMapping("/{idCliente}/ecommerce")
    @Operation(summary = "Obtener datos ecommerce", description = "Consulta detalles de ecommerce del cliente")
    public ResponseEntity<ClienteDetalleEcommerceDTO> obtenerDetalleEcommerce(
            @PathVariable String idCliente,
            HttpServletRequest httpRequest) {
        
        try {
            String clientIp = obtenerClientIp(httpRequest);
            logger.info("Consultando detalles de ecommerce desde IP: {} para cliente: {}", clientIp, idCliente);
            
            ClienteDetalleEcommerceDTO detalleEcommerce = clientesService.obtenerDetalleEcommerce(idCliente);
            
            if (detalleEcommerce != null) {
                logger.info("Detalles de ecommerce encontrados para cliente: {}", idCliente);
                return ResponseEntity.ok(detalleEcommerce);
            } else {
                logger.info("No se encontraron detalles de ecommerce para cliente: {}", idCliente);
                return ResponseEntity.notFound().build();
            }
            
        } catch (Exception e) {
            logger.error("Error al consultar detalles de ecommerce para cliente {}: {}", idCliente, e.getMessage(), e);
            return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR).build();
        }
    }
    
    /**
     * Guarda o actualiza detalles de ecommerce de un cliente
     * POST /api/v1/catalogos/clientes/{idCliente}/ecommerce
     */
    @PostMapping("/{idCliente}/ecommerce")
    @Operation(summary = "Guardar datos ecommerce", description = "Crea o actualiza detalles de ecommerce")
    public ResponseEntity<ClienteDetalleEcommerceDTO> guardarDetalleEcommerce(
            @PathVariable String idCliente,
            @Valid @RequestBody ClienteDetalleEcommerceDTO detalleEcommerce,
            HttpServletRequest httpRequest) {
        
        try {
            String clientIp = obtenerClientIp(httpRequest);
            logger.info("Guardando detalles de ecommerce desde IP: {} para cliente: {}", clientIp, idCliente);
            
            // Asegurar que el ID del cliente coincida
            detalleEcommerce.setIdcliente(idCliente);
            
            ClienteDetalleEcommerceDTO resultado = clientesService.guardarDetalleEcommerce(detalleEcommerce);
            
            logger.info("Detalles de ecommerce guardados exitosamente para cliente: {}", idCliente);
            return ResponseEntity.ok(resultado);
            
        } catch (Exception e) {
            logger.error("Error al guardar detalles de ecommerce para cliente {}: {}", idCliente, e.getMessage(), e);
            return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR).build();
        }
    }
}
