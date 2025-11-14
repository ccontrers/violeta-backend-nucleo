package com.lavioleta.desarrollo.violetaserver.dto.response;

/**
 * DTO para representar un precio de artículo
 */
public class PrecioArticulo {
    private String tipoPrec;
    private Double precio;

    public PrecioArticulo() {
    }

    public PrecioArticulo(String tipoPrec, Double precio) {
        this.tipoPrec = tipoPrec;
        this.precio = precio;
    }

    public String getTipoPrec() {
        return tipoPrec;
    }

    public void setTipoPrec(String tipoPrec) {
        this.tipoPrec = tipoPrec;
    }

    public Double getPrecio() {
        return precio;
    }

    public void setPrecio(Double precio) {
        this.precio = precio;
    }

    @Override
    public String toString() {
        return "PrecioArticulo{" +
                "tipoPrec='" + tipoPrec + '\'' +
                ", precio=" + precio +
                '}';
    }
}
