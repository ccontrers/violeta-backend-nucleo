package com.lavioleta.desarrollo.violetaserver.entity;

import jakarta.persistence.*;
import java.math.BigDecimal;

@Entity
@Table(name = "datoscredito")
public class DatosCredito {
    
    @Id
    @Column(name = "cliente")
    private String cliente;
    
    // Datos principales del cliente
    @Column(name = "cliautos")
    private Integer cliautos;
    
    @Column(name = "cliingre", precision = 16, scale = 2)
    private BigDecimal cliingre;
    
    @Column(name = "cliegre", precision = 16, scale = 2)
    private BigDecimal cliegre;
    
    @Column(name = "ventapm", precision = 16, scale = 2)
    private BigDecimal ventapm;
    
    // Primera propiedad
    @Column(name = "p1tipo", length = 1)
    private String p1tipo;
    
    @Column(name = "p1valor", precision = 16, scale = 2)
    private BigDecimal p1valor;
    
    @Column(name = "p1hipoteca", precision = 16, scale = 2)
    private BigDecimal p1hipoteca;
    
    @Column(name = "p1hnombre", length = 40)
    private String p1hnombre;
    
    @Column(name = "p1dir", length = 40)
    private String p1dir;
    
    // Segunda propiedad
    @Column(name = "p2tipo", length = 1)
    private String p2tipo;
    
    @Column(name = "p2valor", precision = 16, scale = 2)
    private BigDecimal p2valor;
    
    @Column(name = "p2hipoteca", precision = 16, scale = 2)
    private BigDecimal p2hipoteca;
    
    @Column(name = "p2hnombre", length = 40)
    private String p2hnombre;
    
    @Column(name = "p2dir", length = 40)
    private String p2dir;
    
    // Primer aval
    @Column(name = "a1nombre", length = 40)
    private String a1nombre;
    
    @Column(name = "a1dir", length = 40)
    private String a1dir;
    
    @Column(name = "a1tel", length = 20)
    private String a1tel;
    
    @Column(name = "a1tra", length = 40)
    private String a1tra;
    
    @Column(name = "a1puesto", length = 20)
    private String a1puesto;
    
    @Column(name = "a1antig")
    private Integer a1antig;
    
    @Column(name = "a1teltra", length = 20)
    private String a1teltra;
    
    @Column(name = "a1dirtra", length = 60)
    private String a1dirtra;
    
    @Column(name = "a1casa")
    private Boolean a1casa;
    
    @Column(name = "a1autos")
    private Integer a1autos;
    
    @Column(name = "a1ingre", precision = 16, scale = 2)
    private BigDecimal a1ingre;
    
    @Column(name = "a1egre", precision = 16, scale = 2)
    private BigDecimal a1egre;
    
    // Segundo aval
    @Column(name = "a2nombre", length = 40)
    private String a2nombre;
    
    @Column(name = "a2dir", length = 40)
    private String a2dir;
    
    @Column(name = "a2tel", length = 20)
    private String a2tel;
    
    @Column(name = "a2tra", length = 40)
    private String a2tra;
    
    @Column(name = "a2puesto", length = 20)
    private String a2puesto;
    
    @Column(name = "a2antig")
    private Integer a2antig;
    
    @Column(name = "a2teltra", length = 20)
    private String a2teltra;
    
    @Column(name = "a2dirtra", length = 60)
    private String a2dirtra;
    
    @Column(name = "a2casa")
    private Boolean a2casa;
    
    @Column(name = "a2autos")
    private Integer a2autos;
    
    @Column(name = "a2ingre", precision = 16, scale = 2)
    private BigDecimal a2ingre;
    
    @Column(name = "a2egre", precision = 16, scale = 2)
    private BigDecimal a2egre;
    
    // Referencias familiares
    @Column(name = "rf1nom", length = 40)
    private String rf1nom;
    
    @Column(name = "rf1dir", length = 60)
    private String rf1dir;
    
    @Column(name = "rf1tel", length = 20)
    private String rf1tel;
    
    @Column(name = "rf1par", length = 10)
    private String rf1par;
    
    @Column(name = "rf2nom", length = 40)
    private String rf2nom;
    
    @Column(name = "rf2dir", length = 60)
    private String rf2dir;
    
    @Column(name = "rf2tel", length = 20)
    private String rf2tel;
    
    @Column(name = "rf2par", length = 10)
    private String rf2par;
    
    // Referencias no familiares
    @Column(name = "rnf1nom", length = 40)
    private String rnf1nom;
    
    @Column(name = "rnf1dir", length = 60)
    private String rnf1dir;
    
    @Column(name = "rnf1tel", length = 20)
    private String rnf1tel;
    
    @Column(name = "rnf1rel", length = 10)
    private String rnf1rel;
    
    @Column(name = "rnf2nom", length = 40)
    private String rnf2nom;
    
    @Column(name = "rnf2dir", length = 60)
    private String rnf2dir;
    
    @Column(name = "rnf2tel", length = 20)
    private String rnf2tel;
    
    @Column(name = "rnf2rel", length = 10)
    private String rnf2rel;
    
    // Referencias bancarias
    @Column(name = "rb1banco", length = 40)
    private String rb1banco;
    
    @Column(name = "rb1suc", length = 40)
    private String rb1suc;
    
    @Column(name = "rb1telsuc", length = 20)
    private String rb1telsuc;
    
    @Column(name = "rb1cuenta", length = 20)
    private String rb1cuenta;
    
    @Column(name = "rb1antig")
    private Integer rb1antig;
    
    @Column(name = "rb1limcred", precision = 16, scale = 2)
    private BigDecimal rb1limcred;
    
    @Column(name = "rb1adeudos", precision = 16, scale = 2)
    private BigDecimal rb1adeudos;
    
    @Column(name = "rb2banco", length = 40)
    private String rb2banco;
    
    @Column(name = "rb2suc", length = 40)
    private String rb2suc;
    
    @Column(name = "rb2telsuc", length = 20)
    private String rb2telsuc;
    
    @Column(name = "rb2cuenta", length = 20)
    private String rb2cuenta;
    
    @Column(name = "rb2antig")
    private Integer rb2antig;
    
    @Column(name = "rb2limcred", precision = 16, scale = 2)
    private BigDecimal rb2limcred;
    
    @Column(name = "rb2adeudos", precision = 16, scale = 2)
    private BigDecimal rb2adeudos;
    
    // Referencias comerciales
    @Column(name = "rc1emp", length = 60)
    private String rc1emp;
    
    @Column(name = "rc1contacto", length = 40)
    private String rc1contacto;
    
    @Column(name = "rc1telefono", length = 20)
    private String rc1telefono;
    
    @Column(name = "rc2emp", length = 60)
    private String rc2emp;
    
    @Column(name = "rc2contacto", length = 40)
    private String rc2contacto;
    
    @Column(name = "rc2telefono", length = 20)
    private String rc2telefono;
    
    // Autorización y poder
    @Column(name = "podernom", length = 40)
    private String podernom;
    
    @Column(name = "poderpar", length = 20)
    private String poderpar;
    
    @Column(name = "poderpuest", length = 20)
    private String poderpuest;
    
    @Column(name = "aut1nom", length = 40)
    private String aut1nom;
    
    @Column(name = "aut1puest", length = 20)
    private String aut1puest;
    
    @Column(name = "aut2nom", length = 40)
    private String aut2nom;
    
    @Column(name = "aut2puest", length = 20)
    private String aut2puest;
    
    @Column(name = "aut3nom", length = 40)
    private String aut3nom;
    
    @Column(name = "aut3puest", length = 20)
    private String aut3puest;
    
    // Comentarios
    @Column(name = "comentcr", length = 80)
    private String comentcr;
    
    // Constructores
    public DatosCredito() {}
    
    public DatosCredito(String cliente) {
        this.cliente = cliente;
    }
    
    // Getters y Setters
    public String getCliente() { return cliente; }
    public void setCliente(String cliente) { this.cliente = cliente; }
    
    public Integer getCliautos() { return cliautos; }
    public void setCliautos(Integer cliautos) { this.cliautos = cliautos; }
    
    public BigDecimal getCliingre() { return cliingre; }
    public void setCliingre(BigDecimal cliingre) { this.cliingre = cliingre; }
    
    public BigDecimal getCliegre() { return cliegre; }
    public void setCliegre(BigDecimal cliegre) { this.cliegre = cliegre; }
    
    public BigDecimal getVentapm() { return ventapm; }
    public void setVentapm(BigDecimal ventapm) { this.ventapm = ventapm; }
    
    public String getP1tipo() { return p1tipo; }
    public void setP1tipo(String p1tipo) { this.p1tipo = p1tipo; }
    
    public BigDecimal getP1valor() { return p1valor; }
    public void setP1valor(BigDecimal p1valor) { this.p1valor = p1valor; }
    
    public BigDecimal getP1hipoteca() { return p1hipoteca; }
    public void setP1hipoteca(BigDecimal p1hipoteca) { this.p1hipoteca = p1hipoteca; }
    
    public String getP1hnombre() { return p1hnombre; }
    public void setP1hnombre(String p1hnombre) { this.p1hnombre = p1hnombre; }
    
    public String getP1dir() { return p1dir; }
    public void setP1dir(String p1dir) { this.p1dir = p1dir; }
    
    public String getP2tipo() { return p2tipo; }
    public void setP2tipo(String p2tipo) { this.p2tipo = p2tipo; }
    
    public BigDecimal getP2valor() { return p2valor; }
    public void setP2valor(BigDecimal p2valor) { this.p2valor = p2valor; }
    
    public BigDecimal getP2hipoteca() { return p2hipoteca; }
    public void setP2hipoteca(BigDecimal p2hipoteca) { this.p2hipoteca = p2hipoteca; }
    
    public String getP2hnombre() { return p2hnombre; }
    public void setP2hnombre(String p2hnombre) { this.p2hnombre = p2hnombre; }
    
    public String getP2dir() { return p2dir; }
    public void setP2dir(String p2dir) { this.p2dir = p2dir; }
    
    // Getters y setters para primer aval
    public String getA1nombre() { return a1nombre; }
    public void setA1nombre(String a1nombre) { this.a1nombre = a1nombre; }
    
    public String getA1dir() { return a1dir; }
    public void setA1dir(String a1dir) { this.a1dir = a1dir; }
    
    public String getA1tel() { return a1tel; }
    public void setA1tel(String a1tel) { this.a1tel = a1tel; }
    
    public String getA1tra() { return a1tra; }
    public void setA1tra(String a1tra) { this.a1tra = a1tra; }
    
    public String getA1puesto() { return a1puesto; }
    public void setA1puesto(String a1puesto) { this.a1puesto = a1puesto; }
    
    public Integer getA1antig() { return a1antig; }
    public void setA1antig(Integer a1antig) { this.a1antig = a1antig; }
    
    public String getA1teltra() { return a1teltra; }
    public void setA1teltra(String a1teltra) { this.a1teltra = a1teltra; }
    
    public String getA1dirtra() { return a1dirtra; }
    public void setA1dirtra(String a1dirtra) { this.a1dirtra = a1dirtra; }
    
    public Boolean getA1casa() { return a1casa; }
    public void setA1casa(Boolean a1casa) { this.a1casa = a1casa; }
    
    public Integer getA1autos() { return a1autos; }
    public void setA1autos(Integer a1autos) { this.a1autos = a1autos; }
    
    public BigDecimal getA1ingre() { return a1ingre; }
    public void setA1ingre(BigDecimal a1ingre) { this.a1ingre = a1ingre; }
    
    public BigDecimal getA1egre() { return a1egre; }
    public void setA1egre(BigDecimal a1egre) { this.a1egre = a1egre; }
    
    // Getters y setters para segundo aval (similar estructura)
    public String getA2nombre() { return a2nombre; }
    public void setA2nombre(String a2nombre) { this.a2nombre = a2nombre; }
    
    public String getA2dir() { return a2dir; }
    public void setA2dir(String a2dir) { this.a2dir = a2dir; }
    
    public String getA2tel() { return a2tel; }
    public void setA2tel(String a2tel) { this.a2tel = a2tel; }
    
    public String getA2tra() { return a2tra; }
    public void setA2tra(String a2tra) { this.a2tra = a2tra; }
    
    public String getA2puesto() { return a2puesto; }
    public void setA2puesto(String a2puesto) { this.a2puesto = a2puesto; }
    
    public Integer getA2antig() { return a2antig; }
    public void setA2antig(Integer a2antig) { this.a2antig = a2antig; }
    
    public String getA2teltra() { return a2teltra; }
    public void setA2teltra(String a2teltra) { this.a2teltra = a2teltra; }
    
    public String getA2dirtra() { return a2dirtra; }
    public void setA2dirtra(String a2dirtra) { this.a2dirtra = a2dirtra; }
    
    public Boolean getA2casa() { return a2casa; }
    public void setA2casa(Boolean a2casa) { this.a2casa = a2casa; }
    
    public Integer getA2autos() { return a2autos; }
    public void setA2autos(Integer a2autos) { this.a2autos = a2autos; }
    
    public BigDecimal getA2ingre() { return a2ingre; }
    public void setA2ingre(BigDecimal a2ingre) { this.a2ingre = a2ingre; }
    
    public BigDecimal getA2egre() { return a2egre; }
    public void setA2egre(BigDecimal a2egre) { this.a2egre = a2egre; }
    
    // Getters y setters para referencias familiares
    public String getRf1nom() { return rf1nom; }
    public void setRf1nom(String rf1nom) { this.rf1nom = rf1nom; }
    
    public String getRf1dir() { return rf1dir; }
    public void setRf1dir(String rf1dir) { this.rf1dir = rf1dir; }
    
    public String getRf1tel() { return rf1tel; }
    public void setRf1tel(String rf1tel) { this.rf1tel = rf1tel; }
    
    public String getRf1par() { return rf1par; }
    public void setRf1par(String rf1par) { this.rf1par = rf1par; }
    
    public String getRf2nom() { return rf2nom; }
    public void setRf2nom(String rf2nom) { this.rf2nom = rf2nom; }
    
    public String getRf2dir() { return rf2dir; }
    public void setRf2dir(String rf2dir) { this.rf2dir = rf2dir; }
    
    public String getRf2tel() { return rf2tel; }
    public void setRf2tel(String rf2tel) { this.rf2tel = rf2tel; }
    
    public String getRf2par() { return rf2par; }
    public void setRf2par(String rf2par) { this.rf2par = rf2par; }
    
    // Getters y setters para referencias no familiares
    public String getRnf1nom() { return rnf1nom; }
    public void setRnf1nom(String rnf1nom) { this.rnf1nom = rnf1nom; }
    
    public String getRnf1dir() { return rnf1dir; }
    public void setRnf1dir(String rnf1dir) { this.rnf1dir = rnf1dir; }
    
    public String getRnf1tel() { return rnf1tel; }
    public void setRnf1tel(String rnf1tel) { this.rnf1tel = rnf1tel; }
    
    public String getRnf1rel() { return rnf1rel; }
    public void setRnf1rel(String rnf1rel) { this.rnf1rel = rnf1rel; }
    
    public String getRnf2nom() { return rnf2nom; }
    public void setRnf2nom(String rnf2nom) { this.rnf2nom = rnf2nom; }
    
    public String getRnf2dir() { return rnf2dir; }
    public void setRnf2dir(String rnf2dir) { this.rnf2dir = rnf2dir; }
    
    public String getRnf2tel() { return rnf2tel; }
    public void setRnf2tel(String rnf2tel) { this.rnf2tel = rnf2tel; }
    
    public String getRnf2rel() { return rnf2rel; }
    public void setRnf2rel(String rnf2rel) { this.rnf2rel = rnf2rel; }
    
    // Getters y setters para referencias bancarias
    public String getRb1banco() { return rb1banco; }
    public void setRb1banco(String rb1banco) { this.rb1banco = rb1banco; }
    
    public String getRb1suc() { return rb1suc; }
    public void setRb1suc(String rb1suc) { this.rb1suc = rb1suc; }
    
    public String getRb1telsuc() { return rb1telsuc; }
    public void setRb1telsuc(String rb1telsuc) { this.rb1telsuc = rb1telsuc; }
    
    public String getRb1cuenta() { return rb1cuenta; }
    public void setRb1cuenta(String rb1cuenta) { this.rb1cuenta = rb1cuenta; }
    
    public Integer getRb1antig() { return rb1antig; }
    public void setRb1antig(Integer rb1antig) { this.rb1antig = rb1antig; }
    
    public BigDecimal getRb1limcred() { return rb1limcred; }
    public void setRb1limcred(BigDecimal rb1limcred) { this.rb1limcred = rb1limcred; }
    
    public BigDecimal getRb1adeudos() { return rb1adeudos; }
    public void setRb1adeudos(BigDecimal rb1adeudos) { this.rb1adeudos = rb1adeudos; }
    
    public String getRb2banco() { return rb2banco; }
    public void setRb2banco(String rb2banco) { this.rb2banco = rb2banco; }
    
    public String getRb2suc() { return rb2suc; }
    public void setRb2suc(String rb2suc) { this.rb2suc = rb2suc; }
    
    public String getRb2telsuc() { return rb2telsuc; }
    public void setRb2telsuc(String rb2telsuc) { this.rb2telsuc = rb2telsuc; }
    
    public String getRb2cuenta() { return rb2cuenta; }
    public void setRb2cuenta(String rb2cuenta) { this.rb2cuenta = rb2cuenta; }
    
    public Integer getRb2antig() { return rb2antig; }
    public void setRb2antig(Integer rb2antig) { this.rb2antig = rb2antig; }
    
    public BigDecimal getRb2limcred() { return rb2limcred; }
    public void setRb2limcred(BigDecimal rb2limcred) { this.rb2limcred = rb2limcred; }
    
    public BigDecimal getRb2adeudos() { return rb2adeudos; }
    public void setRb2adeudos(BigDecimal rb2adeudos) { this.rb2adeudos = rb2adeudos; }
    
    // Getters y setters para referencias comerciales
    public String getRc1emp() { return rc1emp; }
    public void setRc1emp(String rc1emp) { this.rc1emp = rc1emp; }
    
    public String getRc1contacto() { return rc1contacto; }
    public void setRc1contacto(String rc1contacto) { this.rc1contacto = rc1contacto; }
    
    public String getRc1telefono() { return rc1telefono; }
    public void setRc1telefono(String rc1telefono) { this.rc1telefono = rc1telefono; }
    
    public String getRc2emp() { return rc2emp; }
    public void setRc2emp(String rc2emp) { this.rc2emp = rc2emp; }
    
    public String getRc2contacto() { return rc2contacto; }
    public void setRc2contacto(String rc2contacto) { this.rc2contacto = rc2contacto; }
    
    public String getRc2telefono() { return rc2telefono; }
    public void setRc2telefono(String rc2telefono) { this.rc2telefono = rc2telefono; }
    
    // Getters y setters para autorización
    public String getPodernom() { return podernom; }
    public void setPodernom(String podernom) { this.podernom = podernom; }
    
    public String getPoderpar() { return poderpar; }
    public void setPoderpar(String poderpar) { this.poderpar = poderpar; }
    
    public String getPoderpuest() { return poderpuest; }
    public void setPoderpuest(String poderpuest) { this.poderpuest = poderpuest; }
    
    public String getAut1nom() { return aut1nom; }
    public void setAut1nom(String aut1nom) { this.aut1nom = aut1nom; }
    
    public String getAut1puest() { return aut1puest; }
    public void setAut1puest(String aut1puest) { this.aut1puest = aut1puest; }
    
    public String getAut2nom() { return aut2nom; }
    public void setAut2nom(String aut2nom) { this.aut2nom = aut2nom; }
    
    public String getAut2puest() { return aut2puest; }
    public void setAut2puest(String aut2puest) { this.aut2puest = aut2puest; }
    
    public String getAut3nom() { return aut3nom; }
    public void setAut3nom(String aut3nom) { this.aut3nom = aut3nom; }
    
    public String getAut3puest() { return aut3puest; }
    public void setAut3puest(String aut3puest) { this.aut3puest = aut3puest; }
    
    public String getComentcr() { return comentcr; }
    public void setComentcr(String comentcr) { this.comentcr = comentcr; }
}
