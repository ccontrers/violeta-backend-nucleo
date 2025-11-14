package com.lavioleta.desarrollo.violetaserver.dto.response;

import lombok.Data;
import lombok.Builder;
import lombok.AllArgsConstructor;
import lombok.NoArgsConstructor;

/**
 * DTO de respuesta para cuentas de retención de proveedores
 */
@Data
@Builder
@AllArgsConstructor
@NoArgsConstructor
public class CuentaRetencionProveedorResponse {
    
    private String proveedor;
    private String cuenta_iva_ret;
    private String cuenta_isr_ret;
    private String cuenta_ieps_ret;
    private String cuenta_ietu_ret;
    private Boolean configurado;
    
    /**
     * Obtiene el número de cuentas configuradas
     */
    public Integer getCuentasConfiguradas() {
        int count = 0;
        if (cuenta_iva_ret != null && !cuenta_iva_ret.trim().isEmpty()) count++;
        if (cuenta_isr_ret != null && !cuenta_isr_ret.trim().isEmpty()) count++;
        if (cuenta_ieps_ret != null && !cuenta_ieps_ret.trim().isEmpty()) count++;
        if (cuenta_ietu_ret != null && !cuenta_ietu_ret.trim().isEmpty()) count++;
        return count;
    }
    
    /**
     * Indica si tiene al menos una cuenta configurada
     */
    public Boolean tieneRetenciones() {
        return getCuentasConfiguradas() > 0;
    }
}