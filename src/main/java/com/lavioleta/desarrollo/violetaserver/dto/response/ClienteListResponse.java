package com.lavioleta.desarrollo.violetaserver.dto.response;

import lombok.Data;
import lombok.Builder;
import java.time.LocalDate;
import java.math.BigDecimal;
import java.util.List;

/**
 * DTO para listado y consulta de clientes
 * Versión ligera para listas y búsquedas
 */
@Data
@Builder
public class ClienteListResponse {
    
    private Boolean success;
    private String message;
    private List<ClienteResumen> clientes;
    private Integer totalRegistros;
    private Integer pagina;
    private Integer registrosPorPagina;
    
    @Data
    @Builder
    public static class ClienteResumen {
        private String cliente;
        private String nombre;
        private String appat;
        private String apmat;
        private String nombreCompleto; // Nombre + apellidos formateado
        private String rsocial;
        private String nomnegocio;
        private String rfc;
        private String tipoempre;
        private String tipoempresaNombre; // "Física" o "Moral"
        private Boolean activo;
        private Boolean credito;
        private BigDecimal limcred;
        private String bloqueo;
        private String bloqueoNombre;
        private String sucursal;
        private LocalDate fechaalta;
        private LocalDate fechamodi;
        
        // Datos principales de contacto
        private String telefonoPrincipal;
        private String email;
        private String direccionCompleta; // Dirección formateada
        private String coloniaNombre;
        private String cp;
        
        // Datos de empresa actual
        private String vendedor;
        private String vendedorNombre;
        private String cobrador;
        private String cobradorNombre;
    }
    
    /**
     * Respuesta exitosa
     */
    public static ClienteListResponse success(List<ClienteResumen> clientes, 
                                            Integer totalRegistros,
                                            Integer pagina, 
                                            Integer registrosPorPagina) {
        return ClienteListResponse.builder()
                .success(true)
                .message("Consulta exitosa")
                .clientes(clientes)
                .totalRegistros(totalRegistros)
                .pagina(pagina)
                .registrosPorPagina(registrosPorPagina)
                .build();
    }
    
    /**
     * Respuesta de error
     */
    public static ClienteListResponse error(String message) {
        return ClienteListResponse.builder()
                .success(false)
                .message(message)
                .clientes(List.of())
                .totalRegistros(0)
                .build();
    }
}
