package com.lavioleta.desarrollo.violetaserver.dto;

public class ClienteDetalleEcommerceDTO {
    private String idcliente;
    private Boolean marketing;
    private Boolean verificaciontel;
    private Boolean verificacionemail;
    private Boolean activo;
    private String observaciones;
    
    // Constructor por defecto
    public ClienteDetalleEcommerceDTO() {}
    
    // Constructor con parámetros principales
    public ClienteDetalleEcommerceDTO(String idcliente, Boolean marketing, 
                                     Boolean verificaciontel, Boolean verificacionemail, 
                                     Boolean activo) {
        this.idcliente = idcliente;
        this.marketing = marketing;
        this.verificaciontel = verificaciontel;
        this.verificacionemail = verificacionemail;
        this.activo = activo;
    }
    
    // Getters y Setters
    public String getIdcliente() { 
        return idcliente; 
    }
    
    public void setIdcliente(String idcliente) { 
        this.idcliente = idcliente; 
    }
    
    public Boolean getMarketing() { 
        return marketing; 
    }
    
    public void setMarketing(Boolean marketing) { 
        this.marketing = marketing; 
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
    
    public Boolean getActivo() { 
        return activo; 
    }
    
    public void setActivo(Boolean activo) { 
        this.activo = activo; 
    }
    
    public String getObservaciones() { 
        return observaciones; 
    }
    
    public void setObservaciones(String observaciones) { 
        this.observaciones = observaciones; 
    }
}
