package com.lavioleta.desarrollo.violetaserver.dto.request;

import jakarta.validation.constraints.*;
import jakarta.validation.Valid;
import jakarta.validation.constraints.AssertTrue;
import lombok.Data;
import java.time.LocalDate;
import java.math.BigDecimal;
import java.util.List;

/**
 * DTO para operaciones de catálogo de clientes
 * Migrado de FormCatalogoClientes.cpp
 * Maneja múltiples direcciones, teléfonos y datos por empresa
 */
@Data
public class ClienteRequest {
    
    // Operación: A=Alta, M=Modificación
    @NotBlank(message = "La operación es requerida")
    @Pattern(regexp = "^[AM]$", message = "Operación debe ser A (Alta) o M (Modificación)")
    private String operacion;
    
    // Clave del cliente (para modificación, se genera automático en alta)
    private String cliente;
    
    // ID de empresa actual para datos específicos por empresa
    @NotNull(message = "El ID de empresa es requerido")
    private Integer idEmpresa;
    
    // === DATOS PRINCIPALES (tabla clientes) ===
    // Para persona física (tipoempre=0) el nombre es obligatorio; para moral (tipoempre=1) se valida condicionalmente
    @Size(max = 60, message = "El nombre no puede exceder 60 caracteres")
    private String nombre;
    
    @Size(max = 40, message = "El apellido paterno no puede exceder 40 caracteres")
    private String appat;
    
    @Size(max = 40, message = "El apellido materno no puede exceder 40 caracteres")
    private String apmat;
    
    @Size(max = 10, message = "El título no puede exceder 10 caracteres")
    private String titulo;
    
    private LocalDate fnaccli; // Fecha nacimiento cliente
    
    // Tipo empresa: 0=Física, 1=Moral (compat: también acepta F/M durante transición)
    @Pattern(regexp = "^[01FM]$", message = "Tipo empresa debe ser 0/ F (Física) o 1/ M (Moral)")
    private String tipoempre;
    
    @Size(max = 255, message = "La razón social no puede exceder 255 caracteres")
    private String rsocial;
    
    @Size(max = 60, message = "El nombre comercial no puede exceder 60 caracteres")
    private String nomnegocio;
    
    @Size(max = 15, message = "El RFC no puede exceder 15 caracteres")
    private String rfc;
    
    @Size(max = 18, message = "El CURP no puede exceder 18 caracteres")
    private String curp;
    
    // Status
    private Boolean activo = true;
    private Boolean sgerencia = false; // Supervisado gerencia
    private Boolean esparterelac = false; // Parte relacionada
    private Boolean venxvol = true; // Ventas por volumen
    private Boolean esAsociado = false;
    
    // Catálogos relacionados
    private String giro; // Giro del negocio
    private String canal; // Canal de cliente
    private String regimenfiscal; // Régimen fiscal
    private String sociedadmercantil; // Sociedad mercantil
    
    // === UBICACIÓN FISCAL (tabla clientes) ===
    @Size(max = 60, message = "La calle no puede exceder 60 caracteres")
    private String calle;
    
    @Size(max = 10, message = "El número exterior no puede exceder 10 caracteres")
    private String numext;
    
    @Size(max = 10, message = "El número interior no puede exceder 10 caracteres")
    private String numint;
    
    @Size(max = 60, message = "La referencia no puede exceder 60 caracteres")
    private String referenciadomic;
    
    @Size(max = 5, message = "El código postal no puede exceder 5 caracteres")
    private String cp;
    
    private String colonia;
    
    // Coordenadas GIS
    private Double latitud;
    private Double longitud;
    
    // === CONTACTO (tabla clientes) ===
    @Size(max = 40, message = "El contacto no puede exceder 40 caracteres")
    private String contacto;
    
    private LocalDate contacfnac; // Fecha nacimiento contacto
    
    @Email(message = "El email debe tener formato válido")
    @Size(max = 50, message = "El email no puede exceder 50 caracteres")
    private String email;
    
    @Email(message = "El email2 debe tener formato válido")
    @Size(max = 50, message = "El email2 no puede exceder 50 caracteres")
    private String email2;
    
    private String medio; // Cómo se enteró de servicios
    
    // === CRÉDITO (tabla clientes) ===
    private Boolean credito = false;
    
    @DecimalMin(value = "0", message = "El límite de crédito debe ser mayor o igual a 0")
    private BigDecimal limcred = BigDecimal.ZERO;
    
    @Min(value = 0, message = "El plazo debe ser mayor o igual a 0")
    private Integer plazo;
    
    private Boolean excederlc = false; // Exceder límite crédito
    private String bloqueo = "06"; // Status del cliente
    private Boolean imprsaldos = true; // Imprimir saldos en factura
    private String numpedidos; // Número de pedidos permitidos
    
    // private String comentcr; // Comentario crédito - Campo no existe en la tabla actual
    
    // === CFDI (tabla clientes) ===
    private Boolean enviarcfd = false; // Envío automático CFD
    
    @NotBlank(message = "El método de pago por defecto es requerido")
    @Size(max = 2, message = "El método de pago debe tener máximo 2 caracteres")
    private String metododef = "01"; // Método de pago por defecto - Efectivo
    
    @NotBlank(message = "El método de pago para ventas superiores es requerido")
    @Size(max = 2, message = "El método de pago superior debe tener máximo 2 caracteres")
    private String metodosup = "01"; // Método de pago para ventas superiores - Efectivo
    
    // Campos de dígitos ahora opcionales (frontend dejó de enviarlos). Se mantienen por compatibilidad.
    @Size(max = 4, message = "Los dígitos por defecto deben tener máximo 4 caracteres")
    private String digitosdef; // Dígitos por defecto - sin valor por defecto para preservar originales

    @Size(max = 4, message = "Los dígitos superiores deben tener máximo 4 caracteres")
    private String digitossup; // Dígitos para ventas superiores - sin valor por defecto para preservar originales
    
    @NotBlank(message = "El uso CFDI es requerido")
    @Size(max = 4, message = "El uso CFDI debe tener máximo 4 caracteres")
    private String usocfdi = "G01"; // Uso CFDI por defecto
    
    private BigDecimal valorsup = BigDecimal.ZERO; // Valor superior para cambio de método de pago
    
    // === CAMPOS BOOLEAN NOT NULL (ya definidos arriba, solo agregamos los faltantes) ===
    private Boolean agruparncre = false; // Agrupar notas de crédito
    private Integer forzarimprimirvertical = 0; // Formato impresión (0=No, 1=Sí)
    
    // === CAMPOS DECIMAL NOT NULL ===
    private BigDecimal credMax = BigDecimal.ZERO; // Crédito máximo
    
    // === CONFIGURACIÓN (tabla clientes) ===
    private String sucremotarelacion; // Sucursal remota relacionada
    private String usuremotorelacion; // Usuario remoto relacionado
    
    // === DATOS POR EMPRESA (tabla clientesemp) ===
    private String vendedor;
    private String cobrador;
    private String tipoprec; // Tipo de precio
    private String tipoprecmin; // Tipo de precio mínimo
    
    // === TELÉFONOS (tabla telefonosclientes) ===
    @Valid
    private List<TelefonoRequest> telefonos;
    
    // === DIRECCIONES DE ENTREGA (tabla direccionesentregaclientes) ===
    @Valid
    private List<DireccionEntregaRequest> direccionesEntrega;
    
    // === DATOS DE TODAS LAS EMPRESAS ===
    // Para mostrar/editar datos de otras empresas
    @Valid
    private List<DatosEmpresaRequest> datosEmpresas;

    // === VALIDACIONES CONDICIONALES ===
    @AssertTrue(message = "Debe capturar nombre si es persona física (0/F) o razón social si es persona moral (1/M)")
    public boolean isNombreORazonValido() {
        if (tipoempre == null) return true; // Ya hay validación de patrón arriba
        if ("0".equals(tipoempre) || "F".equalsIgnoreCase(tipoempre)) {
            return nombre != null && !nombre.isBlank();
        } else if ("1".equals(tipoempre) || "M".equalsIgnoreCase(tipoempre)) {
            return rsocial != null && !rsocial.isBlank();
        }
        return true;
    }
    
    @Data
    public static class TelefonoRequest {
        @NotBlank(message = "El tipo de teléfono es requerido")
        @Size(max = 10, message = "El tipo no puede exceder 10 caracteres")
        private String tipo;
        
        @Size(max = 8, message = "La lada no puede exceder 8 caracteres")
        private String lada;
        
        @NotBlank(message = "El número de teléfono es requerido")
        @Size(max = 15, message = "El número no puede exceder 15 caracteres")
        private String telefono;
        
        @Size(max = 5, message = "La extensión no puede exceder 5 caracteres")
        private String extencionTel;
        
        // Indicador para operaciones (A=Agregar, M=Modificar, E=Eliminar)
        private String operacion = "A";
    }
    
    @Data
    public static class DireccionEntregaRequest {
        // ID de dirección (1, 2, 3, etc.)
        private Integer iddireccion;
        
        private Boolean dafault = false; // Dirección por defecto
        
        @Size(max = 60, message = "La calle no puede exceder 60 caracteres")
        private String calle;
        
        @Size(max = 10, message = "El número exterior no puede exceder 10 caracteres")
        private String numext;
        
        @Size(max = 10, message = "El número interior no puede exceder 10 caracteres")
        private String numint;
        
        @Size(max = 60, message = "La referencia no puede exceder 60 caracteres")
        private String referenciadomic;
        
        private String colonia;
        
        @Size(max = 5, message = "El código postal no puede exceder 5 caracteres")
        private String cp;
        
        // Coordenadas GIS
        private Double latitud;
        private Double longitud;
        
        // Indicador para operaciones (A=Agregar, M=Modificar, E=Eliminar)
        private String operacion = "A";
    }
    
    @Data
    public static class DatosEmpresaRequest {
        @NotNull(message = "El ID de empresa es requerido")
        private Integer idempresa;
        
        private String nombreEmpresa; // Solo para mostrar
        
        private String vendedor;
        private String cobrador;
        private String tipoprec;
        private String tipoprecmin;
        
        // Indicador si estos datos deben actualizarse
        private Boolean actualizar = false;
    }
}
