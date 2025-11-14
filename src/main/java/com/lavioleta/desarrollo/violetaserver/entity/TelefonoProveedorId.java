package com.lavioleta.desarrollo.violetaserver.entity;

import lombok.Data;
import lombok.AllArgsConstructor;
import lombok.NoArgsConstructor;
import java.io.Serializable;

/**
 * Clase para PK compuesta de TelefonoProveedor
 * Campos: proveedor, lada, telefono
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
public class TelefonoProveedorId implements Serializable {
    
    private String proveedorId;
    private String lada;
    private String telefono;
}