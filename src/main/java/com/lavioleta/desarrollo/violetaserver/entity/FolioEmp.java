package com.lavioleta.desarrollo.violetaserver.entity;

import jakarta.persistence.*;
import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.io.Serializable;

/**
 * Entidad para la tabla foliosemp
 * Gestiona los folios consecutivos por tipo y sucursal
 */
@Entity
@Table(name = "foliosemp")
@Data
@NoArgsConstructor
@AllArgsConstructor
@IdClass(FolioEmp.FolioEmpId.class)
public class FolioEmp {

    @Id
    @Column(name = "folio", length = 10, nullable = false)
    private String folio;

    @Id
    @Column(name = "sucursal", length = 2, nullable = false)
    private String sucursal;

    @Column(name = "descripcion", length = 40)
    private String descripcion;

    @Column(name = "valor")
    private Integer valor;

    /**
     * Clase interna para la clave compuesta
     */
    @Data
    @NoArgsConstructor
    @AllArgsConstructor
    public static class FolioEmpId implements Serializable {
        private String folio;
        private String sucursal;
    }
}
