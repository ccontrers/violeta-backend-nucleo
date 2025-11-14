package com.lavioleta.desarrollo.violetaserver.dto.response;

import lombok.Data;
import lombok.Builder;
import lombok.AllArgsConstructor;
import lombok.NoArgsConstructor;

/**
 * DTO de respuesta para teléfonos de proveedores
 */
@Data
@Builder
@AllArgsConstructor
@NoArgsConstructor
public class TelefonoProveedorResponse {
    
    private String proveedor;
    private String lada;
    private String telefono;
    private String extension;
    private String tipo;
    private String comentarios;
    private Boolean principal;
    
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