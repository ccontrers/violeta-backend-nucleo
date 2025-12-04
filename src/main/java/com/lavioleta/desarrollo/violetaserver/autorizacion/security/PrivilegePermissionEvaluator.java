package com.lavioleta.desarrollo.violetaserver.autorizacion.security;

import java.io.Serializable;

import org.springframework.security.access.PermissionEvaluator;
import org.springframework.security.core.Authentication;
import org.springframework.stereotype.Component;

import com.lavioleta.desarrollo.violetaserver.autorizacion.service.AuthorizationService;

@Component
public class PrivilegePermissionEvaluator implements PermissionEvaluator {

    private final AuthorizationService authorizationService;

    public PrivilegePermissionEvaluator(AuthorizationService authorizationService) {
        this.authorizationService = authorizationService;
    }

    @Override
    public boolean hasPermission(Authentication authentication, Object targetDomainObject, Object permission) {
        if (authentication == null || targetDomainObject == null || permission == null) {
            return false;
        }
        return authorizationService.hasPrivilege(authentication.getName(),
            targetDomainObject.toString(), permission.toString());
    }

    @Override
    public boolean hasPermission(Authentication authentication, Serializable targetId, String targetType, Object permission) {
        return hasPermission(authentication, targetType, permission);
    }
}
