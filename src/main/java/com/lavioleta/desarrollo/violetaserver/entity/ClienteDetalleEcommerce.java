package com.lavioleta.desarrollo.violetaserver.entity;

import jakarta.persistence.*;

@Entity
@Table(name = "clientesdetalleecommerce")
public class ClienteDetalleEcommerce {
    
    @Id
    @Column(name = "cliente")
    private String cliente;
    
    @Column(name = "marketing")
    private Boolean marketing = false;
    
    @Column(name = "num_pedidos")
    private Integer numPedidos = 0;
    
    @Column(name = "verificaciontel")
    private Boolean verificaciontel = false;
    
    @Column(name = "verificacionemail")
    private Boolean verificacionemail = false;
    
    @Column(name = "telefono", length = 13)
    private String telefono;
    
    @Column(name = "email", length = 50)
    private String email;
    
    @Column(name = "activo")
    private Boolean activo = false;
    
    @Column(name = "usuario", length = 10)
    private String usuario = "";
    
    @Column(name = "PASSWORD", length = 64)
    private String password = "";
    
    @Column(name = "idG", length = 100)
    private String idG;
    
    @Column(name = "idF", length = 100)
    private String idF;
    
    @Column(name = "openpaycli", length = 50)
    private String openpaycli;
    
    // Constructores
    public ClienteDetalleEcommerce() {}
    
    public ClienteDetalleEcommerce(String cliente) {
        this.cliente = cliente;
    }
    
    // Getters y Setters
    public String getCliente() { 
        return cliente; 
    }
    
    public void setCliente(String cliente) { 
        this.cliente = cliente; 
    }
    
    public Boolean getMarketing() { 
        return marketing; 
    }
    
    public void setMarketing(Boolean marketing) { 
        this.marketing = marketing; 
    }
    
    public Integer getNumPedidos() { 
        return numPedidos; 
    }
    
    public void setNumPedidos(Integer numPedidos) { 
        this.numPedidos = numPedidos; 
    }
    
    public Boolean getVerificaciontel() { 
        return verificaciontel; 
    }
    
    public void setVerificaciontel(Boolean verificaciontel) { 
        this.verificaciontel = verificaciontel; 
    }
    
    public Boolean getVerificacionemail() { 
        return verificacionemail; 
    }
    
    public void setVerificacionemail(Boolean verificacionemail) { 
        this.verificacionemail = verificacionemail; 
    }
    
    public String getTelefono() { 
        return telefono; 
    }
    
    public void setTelefono(String telefono) { 
        this.telefono = telefono; 
    }
    
    public String getEmail() { 
        return email; 
    }
    
    public void setEmail(String email) { 
        this.email = email; 
    }
    
    public Boolean getActivo() { 
        return activo; 
    }
    
    public void setActivo(Boolean activo) { 
        this.activo = activo; 
    }
    
    public String getUsuario() { 
        return usuario; 
    }
    
    public void setUsuario(String usuario) { 
        this.usuario = usuario; 
    }
    
    public String getPassword() { 
        return password; 
    }
    
    public void setPassword(String password) { 
        this.password = password; 
    }
    
    public String getIdG() { 
        return idG; 
    }
    
    public void setIdG(String idG) { 
        this.idG = idG; 
    }
    
    public String getIdF() { 
        return idF; 
    }
    
    public void setIdF(String idF) { 
        this.idF = idF; 
    }
    
    public String getOpenpaycli() { 
        return openpaycli; 
    }
    
    public void setOpenpaycli(String openpaycli) { 
        this.openpaycli = openpaycli; 
    }
}
