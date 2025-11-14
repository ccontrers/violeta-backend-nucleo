package com.lavioleta.desarrollo.violetaserver.dto.response;

import lombok.Data;
import lombok.Builder;
import lombok.AllArgsConstructor;
import lombok.NoArgsConstructor;
import com.fasterxml.jackson.annotation.JsonFormat;
import java.time.LocalDate;
import java.math.BigDecimal;

/**
 * DTO de respuesta para condiciones comerciales de proveedores
 */
@Data
@Builder
@AllArgsConstructor
@NoArgsConstructor
public class CondicionComercialProveedorResponse {
    
    private Integer id;
    private String proveedor;
    private BigDecimal descuento;
    private Integer numarticulos;
    private BigDecimal importemin;
    private BigDecimal importemax;
    
    @JsonFormat(pattern = "yyyy-MM-dd")
    private LocalDate fechaalta;
    
    private String usuario;
    
    @JsonFormat(pattern = "yyyy-MM-dd")
    private LocalDate fechainicio;
    
    @JsonFormat(pattern = "yyyy-MM-dd")
    private LocalDate fechafin;
    
    private String comentarios;
    private Boolean activo;
    
    /**
     * Indica si la condición está vigente en la fecha actual
     */
    public Boolean esVigente() {
        if (!Boolean.TRUE.equals(activo)) {
            return false;
        }
        
        LocalDate hoy = LocalDate.now();
        
        // Verificar fecha de inicio
        if (fechainicio != null && hoy.isBefore(fechainicio)) {
            return false;
        }
        
        // Verificar fecha de fin
        if (fechafin != null && hoy.isAfter(fechafin)) {
            return false;
        }
        
        return true;
    }
    
    /**
     * Obtiene descripción del rango de importes
     */
    public String getRangoImportes() {
        if (importemin != null && importemax != null) {
            return String.format("$%.2f - $%.2f", importemin, importemax);
        } else if (importemin != null) {
            return String.format("Mínimo $%.2f", importemin);
        } else if (importemax != null) {
            return String.format("Máximo $%.2f", importemax);
        }
        return "Sin límite";
    }
}