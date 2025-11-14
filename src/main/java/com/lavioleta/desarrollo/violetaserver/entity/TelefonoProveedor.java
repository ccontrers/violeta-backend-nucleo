package com.lavioleta.desarrollo.violetaserver.entity;

import jakarta.persistence.*;
import lombok.Data;
import lombok.EqualsAndHashCode;
import lombok.ToString;

/**
 * Entidad JPA para la tabla telefonosproveedores
 * PK compuesta: proveedor, lada, telefono
 */
@Entity
@Table(name = "telefonosproveedores")
@Data
@EqualsAndHashCode(exclude = {"proveedor"})
@ToString(exclude = {"proveedor"})
@IdClass(TelefonoProveedorId.class)
public class TelefonoProveedor {
    
    @Id
    @Column(name = "proveedor", length = 11)
    private String proveedorId;
    
    @Id
    @Column(name = "lada", length = 8)
    private String lada;
    
    @Id
    @Column(name = "telefono", length = 15)
    private String telefono;
    
    @Column(name = "extencionTel", length = 5)
    private String extension;
    
    @Column(name = "tipo", length = 10)
    private String tipo;
    
    // Relación con Proveedor
    @ManyToOne(fetch = FetchType.LAZY)
    @JoinColumn(name = "proveedor", insertable = false, updatable = false)
    private Proveedor proveedor;
    
    /**
     * Obtiene el teléfono completo formateado
     */
    public String getTelefonoCompleto() {
        StringBuilder sb = new StringBuilder();
        if (lada != null && !lada.isEmpty()) {
            sb.append("(").append(lada).append(") ");
        }
        if (telefono != null) {
            sb.append(telefono);
        }
        if (extension != null && !extension.isEmpty()) {
            sb.append(" ext. ").append(extension);
        }
        return sb.toString();
    }
}