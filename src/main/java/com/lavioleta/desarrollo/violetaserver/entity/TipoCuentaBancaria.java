package com.lavioleta.desarrollo.violetaserver.entity;

import jakarta.persistence.*;
import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * Entidad que representa un tipo de cuenta bancaria
 */
@Entity
@Table(name = "tiposcuentasbancarias")
@Data
@NoArgsConstructor
@AllArgsConstructor
public class TipoCuentaBancaria {
    
    @Id
    @Column(name = "clave", length = 10)
    private String clave;
    
    @Column(name = "descripcion", length = 50)
    private String descripcion;
    
    @Column(name = "activo")
    private Integer activo;
    
    @Column(name = "caracteres")
    private Integer caracteres;
    
    @Column(name = "bancorel", length = 10)
    private String bancorel;
}
