package com.lavioleta.desarrollo.violetaserver.autorizacion.security;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.boot.autoconfigure.condition.ConditionalOnBean;
import org.springframework.security.authentication.UsernamePasswordAuthenticationToken;
import org.springframework.security.core.Authentication;
import org.springframework.security.core.GrantedAuthority;
import org.springframework.security.core.context.SecurityContextHolder;
import org.springframework.stereotype.Component;
import org.springframework.web.filter.OncePerRequestFilter;

import com.lavioleta.desarrollo.violetaserver.autorizacion.service.AuthorizationService;

import jakarta.servlet.FilterChain;
import jakarta.servlet.ServletException;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;

/**
 * Agrega dinámicamente las autoridades de privilegios por request.
 */
@Component
@ConditionalOnBean(AuthorizationService.class)
public class AuthorityPopulatorFilter extends OncePerRequestFilter {

    private static final Logger log = LoggerFactory.getLogger(AuthorityPopulatorFilter.class);

    private final AuthorizationService authorizationService;

    public AuthorityPopulatorFilter(AuthorizationService authorizationService) {
        this.authorizationService = authorizationService;
    }

    @Override
    protected void doFilterInternal(HttpServletRequest request, HttpServletResponse response, FilterChain filterChain)
            throws ServletException, IOException {
        Authentication authentication = SecurityContextHolder.getContext().getAuthentication();
        if (authentication != null && authentication.isAuthenticated() && authentication.getPrincipal() != null) {
            String usuario = authentication.getName();
            augmentAuthorities(authentication, usuario);
        }
        filterChain.doFilter(request, response);
    }

    private void augmentAuthorities(Authentication authentication, String usuario) {
        try {
            Collection<? extends GrantedAuthority> actuales = authentication.getAuthorities();
            Set<String> existentes = new HashSet<>();
            for (GrantedAuthority authority : actuales) {
                existentes.add(authority.getAuthority());
            }

            Set<String> nuevos = new HashSet<>(authorizationService.getAuthoritiesForUsuario(usuario));
            nuevos.removeAll(existentes);

            if (nuevos.isEmpty()) {
                return;
            }

            List<GrantedAuthority> merged = new ArrayList<>(actuales);
            for (String authority : nuevos) {
                merged.add(() -> authority);
            }

            UsernamePasswordAuthenticationToken enriquecido = new UsernamePasswordAuthenticationToken(
                authentication.getPrincipal(),
                authentication.getCredentials(),
                merged
            );
            enriquecido.setDetails(authentication.getDetails());
            SecurityContextHolder.getContext().setAuthentication(enriquecido);
        } catch (Exception ex) {
            log.warn("No se pudieron poblar los privilegios para {}: {}", usuario, ex.getMessage());
        }
    }
}
