package com.lavioleta.desarrollo.violetaserver.dto.response;

import java.util.List;

public class BusquedaArticulosResponse {
    private boolean success;
    private String message;
    private List<ArticuloResultado> articulos;
    private List<ClasificacionResultado> clasificaciones;
    private List<MarcaResultado> marcas;
    private int totalResultados;
    
    // Constructors
    public BusquedaArticulosResponse() {}
    
    public BusquedaArticulosResponse(boolean success, String message) {
        this.success = success;
        this.message = message;
    }
    
    // Inner classes para diferentes tipos de resultados
    public static class ArticuloResultado {
        private String nombre;
        private String present;
        private String multiplo;
        private Double factor;
        private String marca;
        private String nomMarca;
        private String producto;
        private String articulo;
        private String ean13;
        private Integer activo;
        
        // Constructors
        public ArticuloResultado() {}
        
        public ArticuloResultado(String nombre, String present, String multiplo, Double factor,
                               String marca, String nomMarca, String producto, String articulo,
                               String ean13, Integer activo) {
            this.nombre = nombre;
            this.present = present;
            this.multiplo = multiplo;
            this.factor = factor;
            this.marca = marca;
            this.nomMarca = nomMarca;
            this.producto = producto;
            this.articulo = articulo;
            this.ean13 = ean13;
            this.activo = activo;
        }
        
        // Getters and Setters
        public String getNombre() { return nombre; }
        public void setNombre(String nombre) { this.nombre = nombre; }
        
        public String getPresent() { return present; }
        public void setPresent(String present) { this.present = present; }
        
        public String getMultiplo() { return multiplo; }
        public void setMultiplo(String multiplo) { this.multiplo = multiplo; }
        
        public Double getFactor() { return factor; }
        public void setFactor(Double factor) { this.factor = factor; }
        
        public String getMarca() { return marca; }
        public void setMarca(String marca) { this.marca = marca; }
        
        public String getNomMarca() { return nomMarca; }
        public void setNomMarca(String nomMarca) { this.nomMarca = nomMarca; }
        
        public String getProducto() { return producto; }
        public void setProducto(String producto) { this.producto = producto; }
        
        public String getArticulo() { return articulo; }
        public void setArticulo(String articulo) { this.articulo = articulo; }
        
        public String getEan13() { return ean13; }
        public void setEan13(String ean13) { this.ean13 = ean13; }
        
        public Integer getActivo() { return activo; }
        public void setActivo(Integer activo) { this.activo = activo; }
    }
    
    public static class ClasificacionResultado {
        private String clasif1;
        private String nombre;
        
        public ClasificacionResultado() {}
        
        public ClasificacionResultado(String clasif1, String nombre) {
            this.clasif1 = clasif1;
            this.nombre = nombre;
        }
        
        public String getClasif1() { return clasif1; }
        public void setClasif1(String clasif1) { this.clasif1 = clasif1; }
        
        public String getNombre() { return nombre; }
        public void setNombre(String nombre) { this.nombre = nombre; }
    }
    
    public static class MarcaResultado {
        private String marca;
        private String nombre;
        
        public MarcaResultado() {}
        
        public MarcaResultado(String marca, String nombre) {
            this.marca = marca;
            this.nombre = nombre;
        }
        
        public String getMarca() { return marca; }
        public void setMarca(String marca) { this.marca = marca; }
        
        public String getNombre() { return nombre; }
        public void setNombre(String nombre) { this.nombre = nombre; }
    }
    
    // Getters and Setters
    public boolean isSuccess() { return success; }
    public void setSuccess(boolean success) { this.success = success; }
    
    public String getMessage() { return message; }
    public void setMessage(String message) { this.message = message; }
    
    public List<ArticuloResultado> getArticulos() { return articulos; }
    public void setArticulos(List<ArticuloResultado> articulos) { this.articulos = articulos; }
    
    public List<ClasificacionResultado> getClasificaciones() { return clasificaciones; }
    public void setClasificaciones(List<ClasificacionResultado> clasificaciones) { this.clasificaciones = clasificaciones; }
    
    public List<MarcaResultado> getMarcas() { return marcas; }
    public void setMarcas(List<MarcaResultado> marcas) { this.marcas = marcas; }
    
    public int getTotalResultados() { return totalResultados; }
    public void setTotalResultados(int totalResultados) { this.totalResultados = totalResultados; }
}
