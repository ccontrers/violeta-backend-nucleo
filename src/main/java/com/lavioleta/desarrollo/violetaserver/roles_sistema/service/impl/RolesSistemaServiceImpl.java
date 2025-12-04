package com.lavioleta.desarrollo.violetaserver.roles_sistema.service.impl;

import java.util.List;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Service;

import com.lavioleta.desarrollo.violetaserver.roles_sistema.dto.response.RolComboOptionResponse;
import com.lavioleta.desarrollo.violetaserver.roles_sistema.repository.RolesSistemaRepository;
import com.lavioleta.desarrollo.violetaserver.roles_sistema.service.RolesSistemaService;

/**
 * Implementación del servicio de roles del sistema.
 */
@Service
public class RolesSistemaServiceImpl implements RolesSistemaService {

    private static final Logger log = LoggerFactory.getLogger(RolesSistemaServiceImpl.class);

    private final RolesSistemaRepository repository;

    public RolesSistemaServiceImpl(RolesSistemaRepository repository) {
        this.repository = repository;
    }

    @Override
    public List<RolComboOptionResponse> listarRolesCombo() {
        log.debug("Obteniendo roles para combo box");
        return repository.obtenerRolesParaCombo();
    }
}
