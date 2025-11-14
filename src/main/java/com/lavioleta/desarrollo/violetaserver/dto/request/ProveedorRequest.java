package com.lavioleta.desarrollo.violetaserver.dto.request;

import jakarta.validation.constraints.*;
import jakarta.validation.Valid;
import lombok.Data;
import java.time.LocalDate;
import java.math.BigDecimal;
import java.util.List;

/**
 * DTO para operaciones de catálogo de proveedores
 * Migrado de FormCatalogoProveedores.cpp
 * Maneja teléfonos, condiciones comerciales y cuentas de retención
 */
@Data
public class ProveedorRequest {
    
    // Operación: A=Alta, M=Modificación
    @NotBlank(message = "La operación es requerida")
    @Pattern(regexp = "^[AM]$", message = "Operación debe ser A (Alta) o M (Modificación)")
    private String operacion;
    
    // Clave del proveedor (para modificación, se genera automático en alta)
    private String proveedor;
    
    // === DATOS PRINCIPALES (tabla proveedores) ===
    @NotBlank(message = "La razón social es requerida")
    @Size(max = 60, message = "La razón social no puede exceder 60 caracteres")
    private String razonsocial;
    
    // Tipo empresa: 0=Física, 1=Moral
    @Pattern(regexp = "^[01]$", message = "Tipo empresa debe ser 0 (Física) o 1 (Moral)")
    private String tipoempre;
    
    @Size(max = 60, message = "El representante legal no puede exceder 60 caracteres")
    private String replegal;
    
    @Size(max = 10, message = "El título del representante no puede exceder 10 caracteres")
    private String titrepleg;
    
    private LocalDate fechnrep; // Fecha nacimiento/fundación representante
    
    @Size(max = 15, message = "El RFC no puede exceder 15 caracteres")
    private String rfc;
    
    @Size(max = 18, message = "El CURP no puede exceder 18 caracteres")
    private String curp;
    
    // === UBICACIÓN (tabla proveedores) ===
    @Size(max = 60, message = "La calle no puede exceder 60 caracteres")
    private String calle;
    
    @Size(max = 40, message = "La colonia no puede exceder 40 caracteres")
    private String colonia;
    
    @Size(max = 10, message = "La clave de colonia no puede exceder 10 caracteres")
    private String cvecolonia;
    
    @Size(max = 10, message = "El código postal no puede exceder 10 caracteres")
    private String cp;
    
    @Size(max = 40, message = "La localidad no puede exceder 40 caracteres")
    private String localidad;
    
    @Size(max = 4, message = "El estado no puede exceder 4 caracteres")
    private String estado;
    
    @Size(max = 40, message = "El país no puede exceder 40 caracteres")
    private String pais = "MEXICO";
    
    // === CONTACTO (tabla proveedores) ===
    @Size(max = 60, message = "El contacto no puede exceder 60 caracteres")
    private String contacto;
    
    @Size(max = 50, message = "El email del contacto no puede exceder 50 caracteres")
    @Email(message = "El email del contacto debe tener formato válido")
    private String emailcto;
    
    private LocalDate fechncon; // Fecha nacimiento contacto
    
    @Size(max = 50, message = "El email no puede exceder 50 caracteres")
    @Email(message = "El email debe tener formato válido")
    private String email;
    
    // === CRÉDITO Y COMERCIAL ===
    @DecimalMin(value = "0.0", message = "El límite de crédito debe ser mayor o igual a 0")
    private BigDecimal limcred;
    
    @Min(value = 0, message = "El plazo debe ser mayor o igual a 0")
    private Integer plazo;
    
    @DecimalMin(value = "0.0", message = "El descuento debe ser mayor o igual a 0")
    @DecimalMax(value = "100.0", message = "El descuento no puede ser mayor a 100")
    private BigDecimal descuento;
    
    @DecimalMin(value = "0.0", message = "El descuento por pronto pago debe ser mayor o igual a 0")
    @DecimalMax(value = "100.0", message = "El descuento por pronto pago no puede ser mayor a 100")
    private BigDecimal descppp;
    
    // === CUENTAS BANCARIAS ===
    @Size(max = 10, message = "El banco C1 no puede exceder 10 caracteres")
    private String bancoc1;
    
    @Size(max = 10, message = "El banco C2 no puede exceder 10 caracteres")
    private String bancoc2;
    
    @Size(max = 10, message = "El banco C3 no puede exceder 10 caracteres")
    private String bancoc3;
    
    @Size(max = 20, message = "La cuenta B1 no puede exceder 20 caracteres")
    private String cuentab1;
    
    @Size(max = 20, message = "La cuenta B2 no puede exceder 20 caracteres")
    private String cuentab2;
    
    @Size(max = 20, message = "La cuenta B3 no puede exceder 20 caracteres")
    private String cuentab3;
    
    @Size(max = 5, message = "El tipo de cuenta 1 no puede exceder 5 caracteres")
    private String tipocuenta1;
    
    @Size(max = 5, message = "El tipo de cuenta 2 no puede exceder 5 caracteres")
    private String tipocuenta2;
    
    @Size(max = 5, message = "El tipo de cuenta 3 no puede exceder 5 caracteres")
    private String tipocuenta3;
    
    @Min(value = 1, message = "La cuenta por defecto debe ser 1, 2 o 3")
    @Max(value = 3, message = "La cuenta por defecto debe ser 1, 2 o 3")
    private Integer cuentadefault;
    
    // === APOYOS ===
    @Size(max = 60, message = "Los apoyos no pueden exceder 60 caracteres")
    private String apoyos;
    
    private LocalDate fechauap; // Fecha último apoyo
    
    // === FLAGS Y CONFIGURACIONES ===
    private Boolean credito = false;
    
    @Min(value = 0, message = "La reducción de costo base debe ser mayor o igual a 0")
    private Integer reduccostobase = 0;
    
    @DecimalMin(value = "0.0", message = "El porcentaje de reducción de costo debe ser mayor o igual a 0")
    @DecimalMax(value = "100.0", message = "El porcentaje de reducción de costo no puede ser mayor a 100")
    private BigDecimal porcreduccosto = BigDecimal.ZERO;
    
    private Boolean esparterelac = false;
    private Boolean cuadreestcomp = true;
    private Boolean cuadreestncre = true;
    private Boolean cuadreestpagos = true;
    private Boolean cuadreestncar = true;
    private Boolean activo = true;
    private Boolean redondeocptecho = true;
    private Boolean emitencpago = true;
    
    @Size(max = 10, message = "El comprador no puede exceder 10 caracteres")
    private String comprador;
    
    private Integer provgastos = 0;
    private Integer provmercancia = 1;
    private Integer esresico = 0;
    
    private Integer impuestoret;
    
    @Size(max = 30, message = "El número de cuenta no puede exceder 30 caracteres")
    private String numcuenta;
    
    private Integer agrupapagfact = 0;
    private Integer agrupapaggast = 0;
    
    private Integer tiporefpago;
    
    @Size(max = 18, message = "La referencia fija no puede exceder 18 caracteres")
    private String referenciafija;
    
    private Integer diasvigencia;
    
    @Size(max = 15, message = "El tipo de retención no puede exceder 15 caracteres")
    private String tiporetencion = "No configurada";
    
    @Min(value = 1, message = "Los días de reorden deben ser mayor a 0")
    private Integer diasreorden = 7;
    
    @Size(max = 10, message = "El capturista no puede exceder 10 caracteres")
    private String capturista;
    
    // === PEDIDOS AUTOMÁTICOS (solo uno puede ser > 0) ===
    @Min(value = 0, message = "Las mínimas cajas deben ser mayor o igual a 0")
    private Integer mincajas = 0;
    
    @Min(value = 0, message = "El mínimo peso debe ser mayor o igual a 0")
    private Double minpeso = 0.0;
    
    @Min(value = 0, message = "El mínimo dinero debe ser mayor o igual a 0")
    private Double mindinero = 0.0;
    
    private Boolean confianzapedidoautomatico = false;
    private Boolean ajuste_bancario = false;
    private Boolean cotizable = false;
    
    @Size(max = 100, message = "El correo de cotización no puede exceder 100 caracteres")
    @Email(message = "El correo de cotización debe tener formato válido")
    private String correo_cotizacion;
    
    // === TELÉFONOS ===
    @Valid
    private List<TelefonoProveedorDTO> telefonos;
    
    // === CONDICIONES COMERCIALES ===
    @Valid
    private CondicionComercialProveedorDTO condicionComercial;
    
    // === CUENTAS DE RETENCIÓN ===
    @Valid
    private CuentaRetencionProveedorDTO cuentaRetencion;
    
    /**
     * Validación personalizada: solo un mínimo puede ser mayor a 0
     * (mincajas, minpeso o mindinero)
     */
    @AssertTrue(message = "Solo uno de los mínimos (cajas, peso o dinero) puede ser mayor a 0")
    private boolean isMinimosValidos() {
        int contadorMinimos = 0;
        if (mincajas != null && mincajas > 0) contadorMinimos++;
        if (minpeso != null && minpeso > 0) contadorMinimos++;
        if (mindinero != null && mindinero > 0) contadorMinimos++;
        return contadorMinimos <= 1;
    }
    
    /**
     * Validación personalizada: si es persona física requiere representante legal
     */
    @AssertTrue(message = "Para persona física el representante legal es requerido")
    private boolean isRepresentanteLegalValido() {
        if ("0".equals(tipoempre)) { // Persona física
            return replegal != null && !replegal.trim().isEmpty();
        }
        return true;
    }
    
    /**
     * Validación personalizada: si es RESICO requiere impuesto retenido
     */
    @AssertTrue(message = "Para RESICO se requiere impuesto retenido")
    private boolean isResicoValido() {
        if (esresico != null && esresico == 1) {
            return impuestoret != null;
        }
        return true;
    }
    
    /**
     * Validación personalizada: proveedor de gastos requiere número de cuenta,
     * proveedor de mercancía requiere comprador
     */
    @AssertTrue(message = "Proveedor de gastos requiere número de cuenta, proveedor de mercancía requiere comprador")
    private boolean isClasificacionValida() {
        if (provgastos != null && provgastos == 1) {
            return numcuenta != null && !numcuenta.trim().isEmpty();
        } else {
            return comprador != null && !comprador.trim().isEmpty();
        }
    }
}