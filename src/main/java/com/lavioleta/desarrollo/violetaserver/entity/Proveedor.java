package com.lavioleta.desarrollo.violetaserver.entity;

import jakarta.persistence.*;
import lombok.Data;
import org.hibernate.annotations.CreationTimestamp;
import org.hibernate.annotations.UpdateTimestamp;

import java.time.LocalDate;
import java.math.BigDecimal;

/**
 * Entidad JPA para la tabla proveedores
 * Migrado de FormCatalogoProveedores.cpp
 */
@Entity
@Table(name = "proveedores")
@Data
public class Proveedor {
    
    @Id
    @Column(name = "proveedor", length = 10)
    private String proveedor;
    
    @Column(name = "razonsocial", length = 60, nullable = false)
    private String razonsocial;
    
    @Column(name = "tipoempre", length = 1)
    private String tipoempre; // 0=Física, 1=Moral
    
    @Column(name = "replegal", length = 60)
    private String replegal;
    
    @Column(name = "titrepleg", length = 10)
    private String titrepleg;
    
    @Column(name = "fechnrep")
    private LocalDate fechnrep;
    
    @Column(name = "rfc", length = 15)
    private String rfc;
    
    @Column(name = "curp", length = 18)
    private String curp;
    
    // === UBICACIÓN ===
    @Column(name = "calle", length = 60)
    private String calle;
    
    @Column(name = "colonia", length = 40)
    private String colonia;
    
    @Column(name = "cvecolonia", length = 10)
    private String cvecolonia;
    
    @Column(name = "cp", length = 10)
    private String cp;
    
    @Column(name = "localidad", length = 40)
    private String localidad;
    
    @Column(name = "estado", length = 4)
    private String estado;
    
    @Column(name = "pais", length = 40)
    private String pais = "MEXICO";
    
    // === CONTACTO ===
    @Column(name = "contacto", length = 60)
    private String contacto;
    
    @Column(name = "emailcto", length = 50)
    private String emailcto;
    
    @Column(name = "fechncon")
    private LocalDate fechncon;
    
    @Column(name = "email", length = 50)
    private String email;
    
    // === CRÉDITO Y COMERCIAL ===
    @Column(name = "limcred", precision = 12, scale = 2)
    private BigDecimal limcred;
    
    @Column(name = "plazo")
    private Integer plazo;
    
    @Column(name = "descuento", precision = 6, scale = 3)
    private BigDecimal descuento;
    
    @Column(name = "descppp", precision = 6, scale = 3)
    private BigDecimal descppp;
    
    // === CUENTAS BANCARIAS ===
    @Column(name = "bancoc1", length = 10)
    private String bancoc1;
    
    @Column(name = "bancoc2", length = 10)
    private String bancoc2;
    
    @Column(name = "bancoc3", length = 10)
    private String bancoc3;
    
    @Column(name = "cuentab1", length = 20)
    private String cuentab1;
    
    @Column(name = "cuentab2", length = 20)
    private String cuentab2;
    
    @Column(name = "cuentab3", length = 20)
    private String cuentab3;
    
    @Column(name = "tipocuenta1", length = 5)
    private String tipocuenta1;
    
    @Column(name = "tipocuenta2", length = 5)
    private String tipocuenta2;
    
    @Column(name = "tipocuenta3", length = 5)
    private String tipocuenta3;
    
    @Column(name = "cuentadefault")
    private Integer cuentadefault;
    
    // === APOYOS ===
    @Column(name = "apoyos", length = 60)
    private String apoyos;
    
    @Column(name = "fechauap")
    private LocalDate fechauap;
    
    // === FLAGS Y CONFIGURACIONES ===
    @Column(name = "credito")
    private Boolean credito = false;
    
    @Column(name = "reduccostobase")
    private Integer reduccostobase = 0;
    
    @Column(name = "porcreduccosto", precision = 6, scale = 3)
    private BigDecimal porcreduccosto = BigDecimal.ZERO;
    
    @Column(name = "esparterelac")
    private Boolean esparterelac = false;
    
    @Column(name = "cuadreestcomp")
    private Boolean cuadreestcomp = true;
    
    @Column(name = "cuadreestncre")
    private Boolean cuadreestncre = true;
    
    @Column(name = "cuadreestpagos")
    private Boolean cuadreestpagos = true;
    
    @Column(name = "cuadreestncar")
    private Boolean cuadreestncar = true;
    
    @Column(name = "activo")
    private Boolean activo = true;
    
    @Column(name = "redondeocptecho")
    private Boolean redondeocptecho = true;
    
    @Column(name = "emitencpago")
    private Boolean emitencpago = true;
    
    @Column(name = "comprador", length = 10)
    private String comprador;
    
    @Column(name = "provgastos")
    private Integer provgastos = 0;
    
    @Column(name = "provmercancia")
    private Integer provmercancia = 1;
    
    @Column(name = "esresico")
    private Integer esresico = 0;
    
    @Column(name = "impuestoret")
    private Integer impuestoret;
    
    @Column(name = "numcuenta", length = 30)
    private String numcuenta;
    
    @Column(name = "agrupapagfact")
    private Integer agrupapagfact = 0;
    
    @Column(name = "agrupapaggast")
    private Integer agrupapaggast = 0;
    
    @Column(name = "tiporefpago")
    private Integer tiporefpago;
    
    @Column(name = "referenciafija", length = 18)
    private String referenciafija;
    
    @Column(name = "diasvigencia")
    private Integer diasvigencia;
    
    @Column(name = "tiporetencion", length = 15)
    private String tiporetencion = "No configurada";
    
    @Column(name = "diasreorden")
    private Integer diasreorden = 7;
    
    @Column(name = "capturista", length = 10)
    private String capturista;
    
    // === PEDIDOS AUTOMÁTICOS ===
    // Constraint: CHECK ((mincajas > 0 AND minpeso = 0 AND mindinero = 0) OR (mincajas = 0 AND minpeso > 0 AND mindinero = 0) OR (mincajas = 0 AND minpeso = 0 AND mindinero > 0) OR (mincajas = 0 AND minpeso = 0 AND mindinero = 0))
    @Column(name = "mincajas")
    private Integer mincajas = 0;
    
    @Column(name = "minpeso")
    private Double minpeso = 0.0;
    
    @Column(name = "mindinero")
    private Double mindinero = 0.0;
    
    @Column(name = "confianzapedidoautomatico")
    private Boolean confianzapedidoautomatico = false;
    
    @Column(name = "ajuste_bancario")
    private Boolean ajuste_bancario = false;
    
    @Column(name = "cotizable")
    private Boolean cotizable = false;
    
    @Column(name = "correo_cotizacion", length = 100)
    private String correo_cotizacion;
    
    // === AUDITORÍA ===
    @CreationTimestamp
    @Column(name = "fechaalta", nullable = false, updatable = false)
    private LocalDate fechaalta;
    
    @UpdateTimestamp
    @Column(name = "fechamodi")
    private LocalDate fechacambio;
    
    @Column(name = "usualta", length = 10, updatable = false)
    private String usualta;
    
    @Column(name = "usumodi", length = 10)
    private String usumodi;
    
    // === RELACIONES ===
    // TODO: Habilitar cuando las entidades relacionadas estén correctamente mapeadas
    /*
    @OneToMany(mappedBy = "proveedor", cascade = CascadeType.ALL, fetch = FetchType.LAZY, orphanRemoval = true)
    private List<TelefonoProveedor> telefonos;
    
    @OneToMany(mappedBy = "proveedor", cascade = CascadeType.ALL, fetch = FetchType.LAZY, orphanRemoval = true)
    private List<CondicionComercialProveedor> condicionesComerciales;
    
    @OneToOne(mappedBy = "proveedor", cascade = CascadeType.ALL, fetch = FetchType.LAZY, orphanRemoval = true)
    private CuentaRetencionProveedor cuentaRetencion;
    */
    
    // === MÉTODOS DE NEGOCIO ===
    
    /**
     * Valida que solo un mínimo esté configurado para pedidos automáticos
     */
    @PrePersist
    @PreUpdate
    private void validarMinimos() {
        int contadorMinimos = 0;
        if (mincajas != null && mincajas > 0) contadorMinimos++;
        if (minpeso != null && minpeso > 0) contadorMinimos++;
        if (mindinero != null && mindinero > 0) contadorMinimos++;
        
        if (contadorMinimos > 1) {
            throw new IllegalStateException("Solo uno de los mínimos (cajas, peso o dinero) puede ser mayor a 0");
        }
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