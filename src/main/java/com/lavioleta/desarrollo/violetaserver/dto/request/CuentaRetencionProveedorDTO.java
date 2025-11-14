package com.lavioleta.desarrollo.violetaserver.dto.request;

import jakarta.validation.constraints.*;
import lombok.Data;
import com.fasterxml.jackson.annotation.JsonIgnoreProperties;

/**
 * DTO para cuentas de retención de proveedores
 * Migrado de tabla cuentas_proveedor (relación 1:1)
 */
@Data
@JsonIgnoreProperties(ignoreUnknown = true)
public class CuentaRetencionProveedorDTO {
    
    // Clave del proveedor (FK)
    private String proveedor;
    
    @Size(max = 20, message = "La cuenta de retención de IVA no puede exceder 20 caracteres")
    private String cuenta_iva_ret;
    
    @Size(max = 20, message = "La cuenta de retención de ISR no puede exceder 20 caracteres")
    private String cuenta_isr_ret;
    
    @Size(max = 20, message = "La cuenta de retención de IEPS no puede exceder 20 caracteres")
    private String cuenta_ieps_ret;
    
    @Size(max = 20, message = "La cuenta de retención de IETU no puede exceder 20 caracteres")
    private String cuenta_ietu_ret;
    
    // Indica si se configuraron las cuentas de retención
    private Boolean configurado = false;
    
    /**
     * Validación personalizada: al menos una cuenta debe estar configurada
     * si configurado = true
     */
    @AssertTrue(message = "Al menos una cuenta de retención debe estar configurada")
    private boolean isConfiguracionValida() {
        if (Boolean.TRUE.equals(configurado)) {
            return (cuenta_iva_ret != null && !cuenta_iva_ret.trim().isEmpty()) ||
                   (cuenta_isr_ret != null && !cuenta_isr_ret.trim().isEmpty()) ||
                   (cuenta_ieps_ret != null && !cuenta_ieps_ret.trim().isEmpty()) ||
                   (cuenta_ietu_ret != null && !cuenta_ietu_ret.trim().isEmpty());
        }
        return true;
    }
}