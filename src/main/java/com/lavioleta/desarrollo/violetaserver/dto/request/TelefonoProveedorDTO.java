package com.lavioleta.desarrollo.violetaserver.dto.request;

import jakarta.validation.constraints.*;
import lombok.Data;
import com.fasterxml.jackson.annotation.JsonIgnoreProperties;

/**
 * DTO para teléfonos de proveedores
 * Migrado de tabla telefonosproveedores con PK compuesta
 */
@Data
@JsonIgnoreProperties(ignoreUnknown = true)
public class TelefonoProveedorDTO {
    
    // PK compuesta en BD: proveedor, lada, telefono
    @NotBlank(message = "La lada es requerida")
    @Size(max = 10, message = "La lada no puede exceder 10 caracteres")
    private String lada;
    
    @NotBlank(message = "El teléfono es requerido")
    @Size(max = 20, message = "El teléfono no puede exceder 20 caracteres")
    private String telefono;
    
    @Size(max = 20, message = "La extensión no puede exceder 20 caracteres")
    private String extension;
    
    @Size(max = 100, message = "El tipo de teléfono no puede exceder 100 caracteres")
    private String tipo;
    
    @Size(max = 100, message = "Los comentarios no pueden exceder 100 caracteres")
    private String comentarios;
    
    // Flag para indicar si es el teléfono principal
    private Boolean principal = false;
}