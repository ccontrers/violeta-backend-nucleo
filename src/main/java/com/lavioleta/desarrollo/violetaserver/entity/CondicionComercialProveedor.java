package com.lavioleta.desarrollo.violetaserver.entity;

import jakarta.persistence.*;
import lombok.Data;
import lombok.EqualsAndHashCode;
import lombok.ToString;
import org.hibernate.annotations.CreationTimestamp;

import java.time.LocalDate;
import java.time.LocalDateTime;
import java.math.BigDecimal;

/**
 * Entidad JPA para la tabla condicionescomerprov
 * ID autoincrementable
 */
@Entity
@Table(name = "condicionescomerprov")
@Data
@EqualsAndHashCode(exclude = {"proveedor"})
@ToString(exclude = {"proveedor"})
public class CondicionComercialProveedor {
    
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    @Column(name = "id")
    private Integer id;
    
    @Column(name = "proveedor", length = 10, nullable = false)
    private String proveedorId;
    
    @Column(name = "descuento", precision = 6, scale = 3)
    private BigDecimal descuento;
    
    @Column(name = "numarticulos")
    private Integer numarticulos;
    
    @Column(name = "importemin", precision = 12, scale = 2)
    private BigDecimal importemin;
    
    @Column(name = "importemax", precision = 12, scale = 2)
    private BigDecimal importemax;
    
    @CreationTimestamp
    @Column(name = "fechaalta", nullable = false, updatable = false)
    private LocalDateTime fechaalta;
    
    @Column(name = "usuario", length = 10)
    private String usuario;
    
    @Column(name = "fechainicio")
    private LocalDate fechainicio;
    
    @Column(name = "fechafin")
    private LocalDate fechafin;
    
    @Column(name = "comentarios", length = 100)
    private String comentarios;
    
    @Column(name = "activo")
    private Boolean activo = true;
    
    // Relación con Proveedor
    @ManyToOne(fetch = FetchType.LAZY)
    @JoinColumn(name = "proveedor", insertable = false, updatable = false)
    private Proveedor proveedor;
    
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
     * Valida el rango de importes e fechas antes de persistir
     */
    @PrePersist
    @PreUpdate
    private void validarRangos() {
        // Validar rango de importes
        if (importemin != null && importemax != null && importemin.compareTo(importemax) > 0) {
            throw new IllegalStateException("El importe mínimo debe ser menor o igual al máximo");
        }
        
        // Validar rango de fechas
        if (fechainicio != null && fechafin != null && fechainicio.isAfter(fechafin)) {
            throw new IllegalStateException("La fecha de inicio debe ser menor o igual a la fecha fin");
        }
    }
}