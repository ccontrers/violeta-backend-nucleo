package com.lavioleta.desarrollo.violetaserver.config;

import com.lavioleta.desarrollo.violetaserver.security.JwtAuthenticationFilter;
import com.lavioleta.desarrollo.violetaserver.security.JwtTokenProvider;
import org.springframework.boot.test.context.TestConfiguration;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.context.annotation.Bean;
import org.springframework.security.config.annotation.web.builders.HttpSecurity;
import org.springframework.security.config.annotation.web.configuration.EnableWebSecurity;
import org.springframework.security.web.SecurityFilterChain;

/**
 * Configuración de seguridad para tests
 * Deshabilita la seguridad para facilitar las pruebas
 */
@TestConfiguration
@EnableWebSecurity
public class TestSecurityConfig {

    /**
     * Mock del filtro de autenticación JWT para evitar validaciones en tests
     */
    @MockBean
    private JwtAuthenticationFilter jwtAuthenticationFilter;

    /**
     * Mock del proveedor de tokens JWT para evitar dependencias en tests
     */
    @MockBean
    private JwtTokenProvider jwtTokenProvider;

    /**
     * Configuración de seguridad que permite todas las peticiones sin autenticación
     */
    @Bean
    public SecurityFilterChain testSecurityFilterChain(HttpSecurity http) throws Exception {
        http
            .csrf(csrf -> csrf.disable())
            .authorizeHttpRequests(auth -> auth
                .anyRequest().permitAll()
            );
        
        return http.build();
    }
}
