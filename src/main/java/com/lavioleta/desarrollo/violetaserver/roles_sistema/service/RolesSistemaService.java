package com.lavioleta.desarrollo.violetaserver.roles_sistema.service;

import java.util.List;

import com.lavioleta.desarrollo.violetaserver.roles_sistema.dto.response.RolComboOptionResponse;

/**
 * Servicio para operaciones con roles del sistema.
 */
public interface RolesSistemaService {

    /**
     * Obtiene la lista de roles del sistema para el combo box.
     * 
     * @return Lista de roles ordenados por nombre
     */
    List<RolComboOptionResponse> listarRolesCombo();
}
