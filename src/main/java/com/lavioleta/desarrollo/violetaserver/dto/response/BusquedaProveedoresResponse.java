package com.lavioleta.desarrollo.violetaserver.dto.response;

import lombok.Builder;
import lombok.Data;

import java.util.List;

@Data
@Builder
public class BusquedaProveedoresResponse {
    
    private boolean success;
    private String message;
    private int totalResultados;
    
    @Builder.Default
    private List<ProveedorResultado> proveedores = List.of();
    
    /**
     * DTO que mapea directamente los campos del SELECT legacy:
     * proveedor, razonsocial, replegal, rfc, estado, localidad, calle, colonia,
     * redondeocptecho, provgastos ('Si'/'No'), provmercancia ('Si'/'No')
     */
    @Data
    @Builder
    public static class ProveedorResultado {
        private String proveedor;           // Clave del proveedor (PK)
        private String razonsocial;         // Razón social
        private String replegal;            // Representante legal (puede ser null)
        private String rfc;                 // RFC
        private String estado;              // Código del estado (FK a estados)
        private String localidad;           // Localidad
        private String calle;               // Calle
        private String colonia;             // Colonia
        private boolean redondeocptecho;    // Redondeo (legacy: tinyint 0/1)
        private boolean provgastos;         // Proveedor de gastos (legacy: 'Si'/'No' -> boolean)
        private boolean provmercancia;      // Proveedor de mercancía (legacy: 'Si'/'No' -> boolean)
        private boolean activo;             // Estado activo (no visible en UI legacy pero útil para transparencia)
        
        // Campos adicionales para información completa (opcional)
        private String contacto;            // Contacto principal
        private String email;               // Email principal
    }
}