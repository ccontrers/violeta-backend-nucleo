package com.lavioleta.desarrollo.violetaserver.autorizacion.security;

import org.springframework.stereotype.Component;
import org.springframework.util.StringUtils;

/**
 * Convierte privilegios de dominio (objeto + privilegio) a autoridades estándar.
 */
@Component
public class DomainPermissionMapper {

    private static final String SEPARATOR = "_";

    public String toAuthority(String objeto, String privilegio) {
        if (!StringUtils.hasText(objeto) || !StringUtils.hasText(privilegio)) {
            throw new IllegalArgumentException("Objeto y privilegio son requeridos");
        }
        return objeto.toUpperCase() + SEPARATOR + privilegio.toUpperCase();
    }

    public String extractObjeto(String authority) {
        if (!StringUtils.hasText(authority) || !authority.contains(SEPARATOR)) {
            return authority;
        }
        return authority.substring(0, authority.indexOf(SEPARATOR));
    }

    public String extractPrivilegio(String authority) {
        if (!StringUtils.hasText(authority) || !authority.contains(SEPARATOR)) {
            return authority;
        }
        return authority.substring(authority.indexOf(SEPARATOR) + 1);
    }
}
