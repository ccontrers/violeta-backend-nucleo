package com.lavioleta.desarrollo.violetaserver.config;

import com.lavioleta.desarrollo.violetaserver.security.JwtAuthenticationFilter;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.context.annotation.Profile;
import org.springframework.security.config.annotation.web.builders.HttpSecurity;
import org.springframework.security.config.annotation.web.configuration.EnableWebSecurity;
import org.springframework.security.config.http.SessionCreationPolicy;
import org.springframework.security.web.SecurityFilterChain;
import org.springframework.security.web.authentication.UsernamePasswordAuthenticationFilter;
import org.springframework.web.cors.CorsConfiguration;
import org.springframework.web.cors.CorsConfigurationSource;
import org.springframework.web.cors.UrlBasedCorsConfigurationSource;

import java.util.List;

/**
 * Configuración de Spring Security para la aplicación
 * Soporta autenticación híbrida: JWT (token) + Sesión HTTP (cookie)
 * 
 * Solo se activa cuando NO estamos en perfil "test" (para no interferir con tests unitarios)
 */
@Configuration
@EnableWebSecurity
@Profile("!test")
public class SecurityConfig {

    @Autowired
    private JwtAuthenticationFilter jwtAuthenticationFilter;

    @Bean
    public SecurityFilterChain filterChain(HttpSecurity http) throws Exception {
        http
            // Deshabilitar CSRF (necesario para APIs REST)
            .csrf(csrf -> csrf.disable())
            
            // Configurar CORS
            .cors(cors -> cors.configurationSource(corsConfigurationSource()))
            
            // Configuración de sesiones: IF_REQUIRED permite tanto JWT como sesiones
            .sessionManagement(session -> session
                .sessionCreationPolicy(SessionCreationPolicy.IF_REQUIRED)
                .maximumSessions(5) // Múltiples sesiones permitidas
                .maxSessionsPreventsLogin(false)
            )
            
            // Autorización de endpoints
            .authorizeHttpRequests(auth -> auth
                // Endpoints públicos
                .requestMatchers(
                    "/api/v1/auth/**",           // Login y logout
                    "/swagger-ui/**",            // Swagger UI
                    "/v3/api-docs/**",           // OpenAPI docs
                    "/actuator/health"           // Health check
                ).permitAll()
                
                // Todos los demás requieren autenticación
                .anyRequest().authenticated()
            )
            
            // Agregar filtro JWT ANTES del filtro de sesiones
            // Esto permite que primero intente JWT, y si no hay token, usa sesiones
            .addFilterBefore(jwtAuthenticationFilter, UsernamePasswordAuthenticationFilter.class);

        return http.build();
    }

    @Bean
    public CorsConfigurationSource corsConfigurationSource() {
        CorsConfiguration configuration = new CorsConfiguration();
        configuration.setAllowedOriginPatterns(List.of(
            "http://localhost:3120",        // React dev anterior
            "http://localhost:6820",        // Backend
            "https://*.lovable.app"  // Frontend desplegado (subdominios dinámicos)
        ));
        configuration.setAllowedMethods(List.of("GET", "POST", "PUT", "DELETE", "OPTIONS"));
        configuration.setAllowedHeaders(List.of("*"));
        configuration.setAllowCredentials(true); // Importante para cookies
        configuration.setExposedHeaders(List.of("Authorization")); // Exponer header con JWT

        UrlBasedCorsConfigurationSource source = new UrlBasedCorsConfigurationSource();
        source.registerCorsConfiguration("/**", configuration);
        return source;
    }
}
