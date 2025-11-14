package com.lavioleta.desarrollo.violetaserver.dto.response;

import lombok.Builder;
import lombok.Data;

import java.util.List;

@Data
@Builder
public class BusquedaClientesResponse {
    
    private boolean success;
    private String message;
    private int totalResultados;
    
    @Builder.Default
    private List<ClienteResultado> clientes = List.of();
    
    @Data
    @Builder
    public static class ClienteResultado {
        private String cliente;        // Clave del cliente
        private String rsocial;        // Razón social
        private String nomnegocio;     // Nombre del negocio
        private String rfc;            // RFC
        private String regimenfiscal;  // Régimen fiscal con descripción
        private String contacto;       // Nombre del contacto
        private String municipio;      // Nombre del municipio
        private String localidad;      // Nombre de la localidad
        private String calle;          // Calle
        private String colonia;        // Nombre de la colonia
        private String cp;             // Código postal
        
        // Campos adicionales para información completa
        private String telefono;       // Teléfono principal
        private String email;          // Email principal
        private String email2;         // Email secundario
        private boolean activo;        // Si el cliente está activo
    }
}
