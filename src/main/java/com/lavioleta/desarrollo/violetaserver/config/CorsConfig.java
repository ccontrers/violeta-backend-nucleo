package com.lavioleta.desarrollo.violetaserver.config;

import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.web.cors.CorsConfiguration;
import org.springframework.web.cors.UrlBasedCorsConfigurationSource;
import org.springframework.web.filter.CorsFilter;

import java.util.Arrays;

/**
 * Configuración de CORS para permitir solicitudes desde el frontend en desarrollo.
 * 
 * Esta configuración permite que aplicaciones web corriendo en localhost:8080
 * puedan consumir los endpoints REST de este servidor.
 */
@Configuration
public class CorsConfig {

    @Bean
    public CorsFilter corsFilter() {
        CorsConfiguration config = new CorsConfiguration();
        
        // Permite solicitudes desde los frontends en desarrollo
        config.setAllowedOrigins(Arrays.asList(
            "http://localhost:8080", // Frontend legacy/dev
            "http://localhost:3000", // Frontend React previo
            "http://localhost:5173", // Shell module federation
            "http://localhost:5174", // Remote principal
            "http://localhost:5175"  // Remote secundario
        ));
        
        // Permite todos los métodos HTTP
        config.setAllowedMethods(Arrays.asList("GET", "POST", "PUT", "DELETE", "OPTIONS"));
        
        // Permite todos los headers
        config.setAllowedHeaders(Arrays.asList("*"));
        
        // Permite credenciales (cookies, authorization headers, etc.)
        config.setAllowCredentials(true);
        
        // Tiempo que el navegador puede cachear la respuesta preflight (en segundos)
        config.setMaxAge(3600L);
        
        UrlBasedCorsConfigurationSource source = new UrlBasedCorsConfigurationSource();
        
        // Aplica la configuración a todos los endpoints
        source.registerCorsConfiguration("/**", config);
        
        return new CorsFilter(source);
    }
}
