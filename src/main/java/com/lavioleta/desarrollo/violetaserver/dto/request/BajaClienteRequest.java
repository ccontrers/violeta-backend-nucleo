package com.lavioleta.desarrollo.violetaserver.dto.request;

import jakarta.validation.constraints.NotBlank;
import lombok.Data;

/**
 * DTO para operación de baja de cliente
 * Equivalente a ServidorCatalogos::BajaCliente
 */
@Data
public class BajaClienteRequest {
    
    @NotBlank(message = "La clave del cliente es requerida")
    private String cliente;
    
    // Opcional: motivo de la baja para auditoría
    private String motivo;
    
    // Opcional: confirmar eliminación de datos relacionados
    private Boolean confirmarEliminacion = false;
}
