package com.lavioleta.desarrollo.violetaserver.dto.response;

import lombok.Data;
import lombok.Builder;
import lombok.AllArgsConstructor;
import lombok.NoArgsConstructor;
import com.fasterxml.jackson.annotation.JsonFormat;
import java.time.LocalDate;
import java.math.BigDecimal;
import java.util.List;

/**
 * DTO de respuesta para operaciones del catálogo de proveedores
 * Migrado de FormCatalogoProveedores.cpp y ServidorCatalogos::ConsultaProveedor
 */
@Data
@Builder
@AllArgsConstructor
@NoArgsConstructor
public class ProveedorResponse {
    
    // Información básica
    private String proveedor;
    private String razonsocial;
    private String tipoempre; // 0=Física, 1=Moral
    private String replegal;
    private String titrepleg;
    
    @JsonFormat(pattern = "yyyy-MM-dd")
    private LocalDate fechnrep;
    
    private String rfc;
    private String curp;
    
    // Ubicación
    private String calle;
    private String colonia;
    private String cvecolonia;
    private String cp;
    private String localidad;
    private String estado;
    private String pais;
    
    // Contacto
    private String contacto;
    private String emailcto;
    
    @JsonFormat(pattern = "yyyy-MM-dd")
    private LocalDate fechncon;
    
    private String email;
    
    // Crédito y comercial
    private BigDecimal limcred;
    private Integer plazo;
    private BigDecimal descuento;
    private BigDecimal descppp;
    
    // Cuentas bancarias
    private String bancoc1;
    private String bancoc2;
    private String bancoc3;
    private String cuentab1;
    private String cuentab2;
    private String cuentab3;
    private String tipocuenta1;
    private String tipocuenta2;
    private String tipocuenta3;
    private Integer cuentadefault;
    
    // Apoyos
    private String apoyos;
    
    @JsonFormat(pattern = "yyyy-MM-dd")
    private LocalDate fechauap;
    
    // Configuración
    private Boolean credito;
    private Integer reduccostobase;
    private BigDecimal porcreduccosto;
    private Boolean esparterelac;
    private Boolean cuadreestcomp;
    private Boolean cuadreestncre;
    private Boolean cuadreestpagos;
    private Boolean cuadreestncar;
    private Boolean activo;
    private Boolean redondeocptecho;
    private Boolean emitencpago;
    
    private String comprador;
    private Integer provgastos;
    private Integer provmercancia;
    private Integer esresico;
    private Integer impuestoret;
    
    private String numcuenta;
    private Integer agrupapagfact;
    private Integer agrupapaggast;
    private Integer tiporefpago;
    private String referenciafija;
    private Integer diasvigencia;
    private String tiporetencion;
    private Integer diasreorden;
    private String capturista;
    
    // Pedidos automáticos
    private Integer mincajas;
    private Double minpeso;
    private Double mindinero;
    private Boolean confianzapedidoautomatico;
    private Boolean ajuste_bancario;
    private Boolean cotizable;
    private String correo_cotizacion;
    
    // Fechas de auditoría
    @JsonFormat(pattern = "yyyy-MM-dd")
    private LocalDate fechaalta;
    
    @JsonFormat(pattern = "yyyy-MM-dd")
    private LocalDate fechacambio;
    
    private String usualta;
    private String usumodi;
    
    // Relaciones
    private List<TelefonoProveedorResponse> telefonos;
    private List<CondicionComercialProveedorResponse> condicionesComerciales;
    private CuentaRetencionProveedorResponse cuentaRetencion;
    
    // Información adicional calculada
    private String estadoDescripcion;
    private String compradorDescripcion;
    private String impuestoDescripcion;
    private String tipoEmpresaDescripcion;
    
    /**
     * Obtiene la descripción del tipo de empresa
     */
    public String getTipoEmpresaDescripcion() {
        if ("0".equals(tipoempre)) {
            return "Persona Física";
        } else if ("1".equals(tipoempre)) {
            return "Persona Moral";
        }
        return "No definido";
    }
    
    /**
     * Obtiene el tipo de proveedor basado en flags
     */
    public String getTipoProveedor() {
        if (Boolean.TRUE.equals(provgastos != null && provgastos == 1)) {
            return "Gastos";
        } else if (Boolean.TRUE.equals(provmercancia != null && provmercancia == 1)) {
            return "Mercancía";
        }
        return "Mixto";
    }
    
    /**
     * Indica si el proveedor tiene configurado pedido automático
     */
    public Boolean tienePedidoAutomatico() {
        return (mincajas != null && mincajas > 0) ||
               (minpeso != null && minpeso > 0) ||
               (mindinero != null && mindinero > 0);
    }
    
    /**
     * Obtiene el criterio del pedido automático
     */
    public String getCriterioPedidoAutomatico() {
        if (mincajas != null && mincajas > 0) {
            return "Mínimo " + mincajas + " cajas";
        } else if (minpeso != null && minpeso > 0) {
            return "Mínimo " + minpeso + " kg";
        } else if (mindinero != null && mindinero > 0) {
            return "Mínimo $" + mindinero;
        }
        return "No configurado";
    }
}