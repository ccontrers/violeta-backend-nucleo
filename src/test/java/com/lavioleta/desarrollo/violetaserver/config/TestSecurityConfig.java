package com.lavioleta.desarrollo.violetaserver.config;

import com.lavioleta.desarrollo.violetaserver.security.JwtAuthenticationFilter;
import com.lavioleta.desarrollo.violetaserver.security.JwtTokenProvider;
import org.springframework.boot.test.context.TestConfiguration;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Profile;
import org.springframework.security.config.annotation.web.builders.HttpSecurity;
import org.springframework.security.config.annotation.web.configuration.EnableWebSecurity;
import org.springframework.security.config.annotation.web.configurers.AbstractHttpConfigurer;
import org.springframework.security.web.SecurityFilterChain;

/**
 * Configuración de seguridad para tests
 * Deshabilita la autenticación para permitir que los tests unitarios funcionen sin credenciales
 * También provee mocks de los beans de seguridad necesarios
 */
@TestConfiguration
@EnableWebSecurity
@Profile("test")
public class TestSecurityConfig {

    /**
     * Mock del JwtTokenProvider para tests
     * Evita que Spring intente instanciar el bean real que requiere configuración
     */
    @MockBean
    private JwtTokenProvider jwtTokenProvider;

    /**
     * Mock del JwtAuthenticationFilter para tests
     * Evita que el filtro se ejecute durante los tests
     */
    @MockBean
    private JwtAuthenticationFilter jwtAuthenticationFilter;

    @Bean
    public SecurityFilterChain testSecurityFilterChain(HttpSecurity http) throws Exception {
        http
            .csrf(AbstractHttpConfigurer::disable)
            .authorizeHttpRequests(auth -> auth
                .anyRequest().permitAll()  // Permitir todas las peticiones en tests
            );
        
        return http.build();
    }
}
