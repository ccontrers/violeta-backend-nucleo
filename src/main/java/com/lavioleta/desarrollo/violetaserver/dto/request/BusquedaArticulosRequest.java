package com.lavioleta.desarrollo.violetaserver.dto.request;

public class BusquedaArticulosRequest {
    private String sucursal;
    private String mostrarExistencias; // "NO" o "SI"
    private String mostrarInactivos; // "NO" o "SI"
    private String codcondicion; // N, C, M, E, CB, ART, ""
    private String filas; // número de filas/límite
    private String condicion; // valor a buscar (nombre, clave, marca, etc.)
    
    // Constructors
    public BusquedaArticulosRequest() {}
    
    public BusquedaArticulosRequest(String sucursal, String mostrarExistencias, 
                                         String codcondicion, String filas, String condicion) {
        this.sucursal = sucursal;
        this.mostrarExistencias = mostrarExistencias;
        this.codcondicion = codcondicion;
        this.filas = filas;
        this.condicion = condicion;
    }
    
    // Getters and Setters
    public String getSucursal() { return sucursal; }
    public void setSucursal(String sucursal) { this.sucursal = sucursal; }
    
    public String getMostrarExistencias() { return mostrarExistencias; }
    public void setMostrarExistencias(String mostrarExistencias) { this.mostrarExistencias = mostrarExistencias; }
    
    public String getMostrarInactivos() { return mostrarInactivos; }
    public void setMostrarInactivos(String mostrarInactivos) { this.mostrarInactivos = mostrarInactivos; }
    
    public String getCodcondicion() { return codcondicion; }
    public void setCodcondicion(String codcondicion) { this.codcondicion = codcondicion; }
    
    public String getFilas() { return filas; }
    public void setFilas(String filas) { this.filas = filas; }
    
    public String getCondicion() { return condicion; }
    public void setCondicion(String condicion) { this.condicion = condicion; }
    
    // Métodos de compatibilidad para el servicio
    public String getTipoBusqueda() { return codcondicion; }
    public void setTipoBusqueda(String tipoBusqueda) { this.codcondicion = tipoBusqueda; }
    
    public String getDatoBuscado() { return condicion; }
    public void setDatoBuscado(String datoBuscado) { this.condicion = datoBuscado; }
    
    public String getSoloActivos() { 
        // Si mostrarInactivos es "SI", entonces soloActivos debe ser "0" (incluir inactivos)
        // Si mostrarInactivos es "NO" o null, entonces soloActivos es "1" (solo activos)
        return "SI".equals(mostrarInactivos) ? "0" : "1";
    }
    public void setSoloActivos(String soloActivos) { /* compatibility method */ }
    
    @Override
    public String toString() {
        return "EjemploBusquedaArticulosRequest{" +
                "sucursal='" + sucursal + '\'' +
                ", mostrarExistencias='" + mostrarExistencias + '\'' +
                ", mostrarInactivos='" + mostrarInactivos + '\'' +
                ", codcondicion='" + codcondicion + '\'' +
                ", filas='" + filas + '\'' +
                ", condicion='" + condicion + '\'' +
                '}';
    }
}
