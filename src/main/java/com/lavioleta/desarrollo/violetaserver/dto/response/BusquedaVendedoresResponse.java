package com.lavioleta.desarrollo.violetaserver.dto.response;

import lombok.Builder;
import lombok.Data;

import java.util.List;

/**
 * DTO de respuesta para búsqueda de vendedores
 * Migrado desde ServidorBusquedas::BuscaVendedores
 */
@Data
@Builder
public class BusquedaVendedoresResponse {
    
    private boolean success;
    private String message;
    private int totalResultados;
    
    @Builder.Default
    private List<VendedorResultado> vendedores = List.of();
    
    /**
     * DTO que mapea los campos del SELECT legacy:
     * e.empleado,
     * CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS nombre,
     * e.localidad,
     * v.tipocomi
     * 
     * Tablas involucradas:
     * - empleados e (nombre, appat, apmat, localidad)
     * - vendedores v (tipocomi, activo)
     * 
     * JOIN: INNER JOIN vendedores v ON e.empleado = v.empleado
     */
    @Data
    @Builder
    public static class VendedorResultado {
        /**
         * Clave del empleado/vendedor (PK de empleados, FK en vendedores)
         */
        private String empleado;
        
        /**
         * Nombre completo concatenado: nombre + ' ' + appat + ' ' + apmat
         */
        private String nombre;
        
        /**
         * Localidad del empleado
         */
        private String localidad;
        
        /**
         * Tipo de comisión del vendedor (campo de tabla vendedores)
         */
        private String tipocomi;
        
        /**
         * Estado activo del vendedor (no visible en UI legacy pero útil para debugging)
         */
        private Boolean activo;
    }
}
