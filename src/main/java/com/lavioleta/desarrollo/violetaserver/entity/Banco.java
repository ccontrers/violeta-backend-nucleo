package com.lavioleta.desarrollo.violetaserver.entity;

import jakarta.persistence.*;
import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * Entidad que representa un banco
 */
@Entity
@Table(name = "bancos")
@Data
@NoArgsConstructor
@AllArgsConstructor
public class Banco {
    
    @Id
    @Column(name = "banco", length = 10)
    private String banco;
    
    @Column(name = "nombre", length = 50)
    private String nombre;
    
    @Column(name = "activoapp")
    private Integer activoapp;
}
