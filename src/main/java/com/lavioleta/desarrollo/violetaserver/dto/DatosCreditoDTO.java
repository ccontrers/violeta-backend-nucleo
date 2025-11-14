package com.lavioleta.desarrollo.violetaserver.dto;

import com.fasterxml.jackson.annotation.JsonFormat;
import java.time.LocalDate;
import java.math.BigDecimal;

public class DatosCreditoDTO {
    private String idcliente;
    
    // Datos de la solicitud
    @JsonFormat(pattern = "yyyy-MM-dd")
    private LocalDate fechasolicitud;
    private BigDecimal montosol;
    private Integer plazosol;
    private BigDecimal ventasprom;
    private Integer numautos;
    private BigDecimal ingresos;
    private BigDecimal egresos;
    private BigDecimal patrimonio;
    
    // Primera propiedad
    private String p1tipo;
    private BigDecimal p1valor;
    private String p1dir;
    private BigDecimal p1vhipot;
    private String p1acreedor;
    
    // Segunda propiedad
    private String p2tipo;
    private BigDecimal p2valor;
    private String p2dir;
    private BigDecimal p2vhipot;
    private String p2acreedor;
    
    // Primer aval
    private String a1nombre;
    private String a1tel;
    private String a1dir;
    
    // Segundo aval
    private String a2nombre;
    private String a2tel;
    private String a2dir;
    
    // Primera referencia familiar
    private String rf1nom;
    private String rf1tel;
    private String rf1parent;
    private String rf1ocup;
    
    // Segunda referencia familiar
    private String rf2nom;
    private String rf2tel;
    private String rf2parent;
    private String rf2ocup;
    
    // Primera referencia bancaria
    private String rb1banco;
    private String rb1sucursal;
    private String rb1cuenta;
    private String rb1tel;
    
    // Segunda referencia bancaria
    private String rb2banco;
    private String rb2sucursal;
    private String rb2cuenta;
    private String rb2tel;
    
    // Primera referencia comercial
    private String rc1empresa;
    private String rc1contacto;
    private String rc1tel;
    private BigDecimal rc1limite;
    
    // Segunda referencia comercial
    private String rc2empresa;
    private String rc2contacto;
    private String rc2tel;
    private BigDecimal rc2limite;
    
    // Información laboral
    private String empresa;
    private String puesto;
    private Integer antiguedad;
    
    // Observaciones
    private String observaciones;
    
    // Constructor por defecto
    public DatosCreditoDTO() {}
    
    // Constructor completo
    public DatosCreditoDTO(String idcliente, LocalDate fechasolicitud, BigDecimal montosol, 
                          Integer plazosol, BigDecimal ventasprom, Integer numautos, 
                          BigDecimal ingresos, BigDecimal egresos, BigDecimal patrimonio) {
        this.idcliente = idcliente;
        this.fechasolicitud = fechasolicitud;
        this.montosol = montosol;
        this.plazosol = plazosol;
        this.ventasprom = ventasprom;
        this.numautos = numautos;
        this.ingresos = ingresos;
        this.egresos = egresos;
        this.patrimonio = patrimonio;
    }
    
    // Getters y Setters
    public String getIdcliente() { return idcliente; }
    public void setIdcliente(String idcliente) { this.idcliente = idcliente; }
    
    public LocalDate getFechasolicitud() { return fechasolicitud; }
    public void setFechasolicitud(LocalDate fechasolicitud) { this.fechasolicitud = fechasolicitud; }
    
    public BigDecimal getMontosol() { return montosol; }
    public void setMontosol(BigDecimal montosol) { this.montosol = montosol; }
    
    public Integer getPlazosol() { return plazosol; }
    public void setPlazosol(Integer plazosol) { this.plazosol = plazosol; }
    
    public BigDecimal getVentasprom() { return ventasprom; }
    public void setVentasprom(BigDecimal ventasprom) { this.ventasprom = ventasprom; }
    
    public Integer getNumautos() { return numautos; }
    public void setNumautos(Integer numautos) { this.numautos = numautos; }
    
    public BigDecimal getIngresos() { return ingresos; }
    public void setIngresos(BigDecimal ingresos) { this.ingresos = ingresos; }
    
    public BigDecimal getEgresos() { return egresos; }
    public void setEgresos(BigDecimal egresos) { this.egresos = egresos; }
    
    public BigDecimal getPatrimonio() { return patrimonio; }
    public void setPatrimonio(BigDecimal patrimonio) { this.patrimonio = patrimonio; }
    
    // Propiedades
    public String getP1tipo() { return p1tipo; }
    public void setP1tipo(String p1tipo) { this.p1tipo = p1tipo; }
    
    public BigDecimal getP1valor() { return p1valor; }
    public void setP1valor(BigDecimal p1valor) { this.p1valor = p1valor; }
    
    public String getP1dir() { return p1dir; }
    public void setP1dir(String p1dir) { this.p1dir = p1dir; }
    
    public BigDecimal getP1vhipot() { return p1vhipot; }
    public void setP1vhipot(BigDecimal p1vhipot) { this.p1vhipot = p1vhipot; }
    
    public String getP1acreedor() { return p1acreedor; }
    public void setP1acreedor(String p1acreedor) { this.p1acreedor = p1acreedor; }
    
    public String getP2tipo() { return p2tipo; }
    public void setP2tipo(String p2tipo) { this.p2tipo = p2tipo; }
    
    public BigDecimal getP2valor() { return p2valor; }
    public void setP2valor(BigDecimal p2valor) { this.p2valor = p2valor; }
    
    public String getP2dir() { return p2dir; }
    public void setP2dir(String p2dir) { this.p2dir = p2dir; }
    
    public BigDecimal getP2vhipot() { return p2vhipot; }
    public void setP2vhipot(BigDecimal p2vhipot) { this.p2vhipot = p2vhipot; }
    
    public String getP2acreedor() { return p2acreedor; }
    public void setP2acreedor(String p2acreedor) { this.p2acreedor = p2acreedor; }
    
    // Avales
    public String getA1nombre() { return a1nombre; }
    public void setA1nombre(String a1nombre) { this.a1nombre = a1nombre; }
    
    public String getA1tel() { return a1tel; }
    public void setA1tel(String a1tel) { this.a1tel = a1tel; }
    
    public String getA1dir() { return a1dir; }
    public void setA1dir(String a1dir) { this.a1dir = a1dir; }
    
    public String getA2nombre() { return a2nombre; }
    public void setA2nombre(String a2nombre) { this.a2nombre = a2nombre; }
    
    public String getA2tel() { return a2tel; }
    public void setA2tel(String a2tel) { this.a2tel = a2tel; }
    
    public String getA2dir() { return a2dir; }
    public void setA2dir(String a2dir) { this.a2dir = a2dir; }
    
    // Referencias familiares
    public String getRf1nom() { return rf1nom; }
    public void setRf1nom(String rf1nom) { this.rf1nom = rf1nom; }
    
    public String getRf1tel() { return rf1tel; }
    public void setRf1tel(String rf1tel) { this.rf1tel = rf1tel; }
    
    public String getRf1parent() { return rf1parent; }
    public void setRf1parent(String rf1parent) { this.rf1parent = rf1parent; }
    
    public String getRf1ocup() { return rf1ocup; }
    public void setRf1ocup(String rf1ocup) { this.rf1ocup = rf1ocup; }
    
    public String getRf2nom() { return rf2nom; }
    public void setRf2nom(String rf2nom) { this.rf2nom = rf2nom; }
    
    public String getRf2tel() { return rf2tel; }
    public void setRf2tel(String rf2tel) { this.rf2tel = rf2tel; }
    
    public String getRf2parent() { return rf2parent; }
    public void setRf2parent(String rf2parent) { this.rf2parent = rf2parent; }
    
    public String getRf2ocup() { return rf2ocup; }
    public void setRf2ocup(String rf2ocup) { this.rf2ocup = rf2ocup; }
    
    // Referencias bancarias
    public String getRb1banco() { return rb1banco; }
    public void setRb1banco(String rb1banco) { this.rb1banco = rb1banco; }
    
    public String getRb1sucursal() { return rb1sucursal; }
    public void setRb1sucursal(String rb1sucursal) { this.rb1sucursal = rb1sucursal; }
    
    public String getRb1cuenta() { return rb1cuenta; }
    public void setRb1cuenta(String rb1cuenta) { this.rb1cuenta = rb1cuenta; }
    
    public String getRb1tel() { return rb1tel; }
    public void setRb1tel(String rb1tel) { this.rb1tel = rb1tel; }
    
    public String getRb2banco() { return rb2banco; }
    public void setRb2banco(String rb2banco) { this.rb2banco = rb2banco; }
    
    public String getRb2sucursal() { return rb2sucursal; }
    public void setRb2sucursal(String rb2sucursal) { this.rb2sucursal = rb2sucursal; }
    
    public String getRb2cuenta() { return rb2cuenta; }
    public void setRb2cuenta(String rb2cuenta) { this.rb2cuenta = rb2cuenta; }
    
    public String getRb2tel() { return rb2tel; }
    public void setRb2tel(String rb2tel) { this.rb2tel = rb2tel; }
    
    // Referencias comerciales
    public String getRc1empresa() { return rc1empresa; }
    public void setRc1empresa(String rc1empresa) { this.rc1empresa = rc1empresa; }
    
    public String getRc1contacto() { return rc1contacto; }
    public void setRc1contacto(String rc1contacto) { this.rc1contacto = rc1contacto; }
    
    public String getRc1tel() { return rc1tel; }
    public void setRc1tel(String rc1tel) { this.rc1tel = rc1tel; }
    
    public BigDecimal getRc1limite() { return rc1limite; }
    public void setRc1limite(BigDecimal rc1limite) { this.rc1limite = rc1limite; }
    
    public String getRc2empresa() { return rc2empresa; }
    public void setRc2empresa(String rc2empresa) { this.rc2empresa = rc2empresa; }
    
    public String getRc2contacto() { return rc2contacto; }
    public void setRc2contacto(String rc2contacto) { this.rc2contacto = rc2contacto; }
    
    public String getRc2tel() { return rc2tel; }
    public void setRc2tel(String rc2tel) { this.rc2tel = rc2tel; }
    
    public BigDecimal getRc2limite() { return rc2limite; }
    public void setRc2limite(BigDecimal rc2limite) { this.rc2limite = rc2limite; }
    
    // Información laboral
    public String getEmpresa() { return empresa; }
    public void setEmpresa(String empresa) { this.empresa = empresa; }
    
    public String getPuesto() { return puesto; }
    public void setPuesto(String puesto) { this.puesto = puesto; }
    
    public Integer getAntiguedad() { return antiguedad; }
    public void setAntiguedad(Integer antiguedad) { this.antiguedad = antiguedad; }
    
    public String getObservaciones() { return observaciones; }
    public void setObservaciones(String observaciones) { this.observaciones = observaciones; }
}
