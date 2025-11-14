package com.lavioleta.desarrollo.violetaserver.config;

import org.springframework.boot.context.properties.ConfigurationProperties;
import org.springframework.boot.context.properties.EnableConfigurationProperties;
import org.springframework.stereotype.Component;

/**
 * Propiedades de configuración de la aplicación
 */
@Component
@ConfigurationProperties(prefix = "app")
@EnableConfigurationProperties
public class AppProperties {
    
    private final Sucursal sucursal = new Sucursal();
    
    public Sucursal getSucursal() {
        return sucursal;
    }
    
    public static class Sucursal {
        /**
         * Sucursal activa por defecto para el sistema
         */
        private String activa = "S1";
        
        public String getActiva() {
            return activa;
        }
        
        public void setActiva(String activa) {
            this.activa = activa;
        }
    }
}
