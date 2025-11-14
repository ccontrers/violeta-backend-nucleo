package com.lavioleta.desarrollo.violetaserver.entity;

import jakarta.persistence.*;
import lombok.Data;
import lombok.EqualsAndHashCode;
import lombok.ToString;

/**
 * Entidad JPA para la tabla cuentas_proveedor
 * Relación 1:1 con proveedores
 */
@Entity
@Table(name = "cuentas_proveedor")
@Data
@EqualsAndHashCode(exclude = {"proveedor"})
@ToString(exclude = {"proveedor"})
public class CuentaRetencionProveedor {
    
    @Id
    @Column(name = "proveedor", length = 10, nullable = false)
    private String proveedorId;
    
    @Column(name = "cuenta_iva_ret", length = 20)
    private String cuenta_iva_ret;
    
    @Column(name = "cuenta_isr_ret", length = 20)
    private String cuenta_isr_ret;
    
    @Column(name = "cuenta_ieps_ret", length = 20)
    private String cuenta_ieps_ret;
    
    @Column(name = "cuenta_ietu_ret", length = 20)
    private String cuenta_ietu_ret;
    
    @Column(name = "configurado")
    private Boolean configurado = false;
    
    // Relación 1:1 con Proveedor
    @OneToOne(fetch = FetchType.LAZY)
    @JoinColumn(name = "proveedor", insertable = false, updatable = false)
    private Proveedor proveedor;
    
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
    
    /**
     * Valida que si está configurado, al menos una cuenta esté definida
     */
    @PrePersist
    @PreUpdate
    private void validarConfiguracion() {
        if (Boolean.TRUE.equals(configurado) && !tieneRetenciones()) {
            throw new IllegalStateException("Al menos una cuenta de retención debe estar configurada");
        }
        
        // Auto-configurar el flag si hay cuentas definidas
        if (tieneRetenciones()) {
            configurado = true;
        }
    }
}