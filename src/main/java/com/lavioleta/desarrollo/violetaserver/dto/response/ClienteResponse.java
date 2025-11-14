package com.lavioleta.desarrollo.violetaserver.dto.response;

import lombok.Data;
import lombok.Builder;
import java.time.LocalDate;
import java.math.BigDecimal;
import java.util.List;

/**
 * DTO de respuesta para operaciones de catálogo de clientes
 * Incluye toda la información del cliente con sus relaciones
 */
@Data
@Builder
public class ClienteResponse {
    
    private Boolean success;
    private String message;
    private ClienteCompleto cliente;
    
    @Data
    @Builder
    public static class ClienteCompleto {
        
        // === DATOS PRINCIPALES (tabla clientes) ===
        private String cliente;
        private String nombre;
        private String appat;
        private String apmat;
        private String sucursal;
        private LocalDate fnaccli;
        private String titulo;
        private String rsocial;
        private String nomnegocio;
        private String contacto;
        private LocalDate contacfnac;
        private String tipoempre;
        private String rfc;
        private String curp;
        
        // Status
        private Boolean activo;
        private Boolean sgerencia;
        private Boolean esparterelac;
        private Boolean venxvol;
        private Boolean esAsociado;
        
        // Catálogos
        private String giro;
        private String giroNombre; // Nombre descriptivo
        private String canal;
        private String canalNombre; // Nombre descriptivo
        private String regimenfiscal; // CAMPO AGREGADO
        private String regimenfiscalNombre; // Nombre descriptivo  
        private String sociedadmercantil; // CAMPO AGREGADO
        private String sociedadmercantilNombre; // Nombre descriptivo
        
        // === UBICACIÓN FISCAL ===
        private String calle;
        private String numext;
        private String numint;
        private String referenciadomic;
        private String colonia;
        private String coloniaNombre; // Nombre descriptivo
        private String cp;
        private Double latitud; // CAMPO AGREGADO
        private Double longitud; // CAMPO AGREGADO
        
        // === CONTACTO ===
        private String email;
        private String email2;
        private String medio;
        private String medioNombre; // Nombre descriptivo
        
        // === CRÉDITO ===
        private Boolean credito;
        private BigDecimal limcred;
        private Integer plazo;
        private Boolean excederlc;
        private String bloqueo;
        private String bloqueoNombre; // Nombre descriptivo
        private Boolean imprsaldos;
        private String numpedidos;
        private String comentcr;
        
        // === CFDI ===
        private Boolean enviarcfd;
        private String metododef;
        private String metodosup; // CAMPO AGREGADO
        private String digitosdef;
        private String digitossup; // CAMPO AGREGADO
        private String usocfdi;
        private BigDecimal valorsup; // CAMPO AGREGADO
        private BigDecimal credMax; // CAMPO AGREGADO
        private Boolean agruparncre; // CAMPO AGREGADO
        
        // === CONFIGURACIÓN ===
        private String forzarimprimirvertical;
        private String sucremotarelacion;
        private String usuremotorelacion;
        
        // === FECHAS DE AUDITORÍA ===
        private LocalDate fechaalta;
        private LocalDate fechamodi;
        private LocalDate fechabloq;
        private LocalDate fechauven;
        
        // === DATOS RELACIONADOS ===
        private List<TelefonoCliente> telefonos;
        private List<DireccionEntrega> direccionesEntrega;
        private List<DatosEmpresa> datosEmpresas;
        private List<EmpresaDisponible> empresasDisponibles;
    }
    
    @Data
    @Builder
    public static class TelefonoCliente {
        private String tipo;
        private String tipoNombre; // Nombre descriptivo del tipo
        private String lada;
        private String telefono;
        private String extencionTel;
    }
    
    @Data
    @Builder
    public static class DireccionEntrega {
        private Integer iddireccion;
        private Boolean dafault;
        private String calle;
        private String numext;
        private String numint;
        private String referenciadomic;
        private String colonia;
        private String coloniaNombre; // Nombre descriptivo
        private String cp;
        private Double latitud;
        private Double longitud;
        private LocalDate fechaalta;
        private LocalDate fechamodi;
    }
    
    @Data
    @Builder
    public static class DatosEmpresa {
        private Integer idempresa;
        private String nombreEmpresa;
        private String claveEmpresa;
        private String vendedor;
        private String vendedorNombre; // Nombre del vendedor
        private String cobrador;
        private String cobradorNombre; // Nombre del cobrador
        private String tipoprec;
        private String tipoprecNombre; // Nombre del tipo de precio
        private String tipoprecmin;
        private String tipoprecminNombre; // Nombre del tipo de precio mínimo
    }
    
    @Data
    @Builder
    public static class EmpresaDisponible {
        private Integer idempresa;
        private String clave;
        private String nombre;
        private String sucprincipal;
        private Boolean essuper;
        private Boolean tieneConfiguracion; // Si ya tiene datos en clientesemp
    }
    
    // === RESPUESTAS PARA OPERACIONES CRUD ===
    
    /**
     * Respuesta exitosa para operación de grabado
     */
    public static ClienteResponse success(String message, ClienteCompleto cliente) {
        return ClienteResponse.builder()
                .success(true)
                .message(message)
                .cliente(cliente)
                .build();
    }
    
    /**
     * Respuesta exitosa para operación de eliminación
     */
    public static ClienteResponse successDelete(String message) {
        return ClienteResponse.builder()
                .success(true)
                .message(message)
                .build();
    }
    
    /**
     * Respuesta de error
     */
    public static ClienteResponse error(String message) {
        return ClienteResponse.builder()
                .success(false)
                .message(message)
                .build();
    }
}
