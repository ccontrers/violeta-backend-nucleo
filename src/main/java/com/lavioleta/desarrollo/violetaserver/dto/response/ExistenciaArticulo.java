package com.lavioleta.desarrollo.violetaserver.dto.response;

/**
 * DTO para representar la existencia de un artículo
 */
public class ExistenciaArticulo {
    private Double cantidad;
    private Double costo;

    public ExistenciaArticulo() {
    }

    public ExistenciaArticulo(Double cantidad, Double costo) {
        this.cantidad = cantidad;
        this.costo = costo;
    }

    public Double getCantidad() {
        return cantidad;
    }

    public void setCantidad(Double cantidad) {
        this.cantidad = cantidad;
    }

    public Double getCosto() {
        return costo;
    }

    public void setCosto(Double costo) {
        this.costo = costo;
    }

    @Override
    public String toString() {
        return "ExistenciaArticulo{" +
                "cantidad=" + cantidad +
                ", costo=" + costo +
                '}';
    }
}
