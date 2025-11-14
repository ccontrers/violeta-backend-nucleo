package com.lavioleta.desarrollo.violetaserver.dto.request;

import jakarta.validation.constraints.*;
import lombok.Data;
import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import java.time.LocalDate;
import java.math.BigDecimal;

/**
 * DTO para condiciones comerciales de proveedores
 * Migrado de tabla condicionescomerprov
 */
@Data
@JsonIgnoreProperties(ignoreUnknown = true)
public class CondicionComercialProveedorDTO {
    
    // ID autoincrementable en BD
    private Integer id;
    
    @DecimalMin(value = "0.0", message = "El descuento debe ser mayor o igual a 0")
    @DecimalMax(value = "100.0", message = "El descuento no puede ser mayor a 100")
    private BigDecimal descuento;
    
    @Min(value = 0, message = "El número de artículos debe ser mayor o igual a 0")
    private Integer numarticulos;
    
    @DecimalMin(value = "0.0", message = "El importe mínimo debe ser mayor o igual a 0")
    private BigDecimal importemin;
    
    @DecimalMin(value = "0.0", message = "El importe máximo debe ser mayor o igual a 0")
    private BigDecimal importemax;
    
    private LocalDate fechaalta;
    
    @Size(max = 10, message = "El usuario no puede exceder 10 caracteres")
    private String usuario;
    
    private LocalDate fechainicio;
    private LocalDate fechafin;
    
    @Size(max = 100, message = "Los comentarios no pueden exceder 100 caracteres")
    private String comentarios;
    
    private Boolean activo = true;
    
    /**
     * Validación personalizada: si se especifica rango de importes,
     * el mínimo debe ser menor al máximo
     */
    @AssertTrue(message = "El importe mínimo debe ser menor al máximo")
    private boolean isRangoImportesValido() {
        if (importemin != null && importemax != null) {
            return importemin.compareTo(importemax) <= 0;
        }
        return true;
    }
    
    /**
     * Validación personalizada: si se especifica rango de fechas,
     * la fecha inicio debe ser menor a la fecha fin
     */
    @AssertTrue(message = "La fecha de inicio debe ser menor a la fecha fin")
    private boolean isRangoFechasValido() {
        if (fechainicio != null && fechafin != null) {
            return fechainicio.isBefore(fechafin) || fechainicio.isEqual(fechafin);
        }
        return true;
    }
}