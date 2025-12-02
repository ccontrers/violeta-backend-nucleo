package com.lavioleta.desarrollo.violetaserver.usuarios.service;

import com.lavioleta.desarrollo.violetaserver.usuarios.dto.request.AsignarClaveRequest;
import com.lavioleta.desarrollo.violetaserver.usuarios.dto.request.CambiarClaveRequest;
import com.lavioleta.desarrollo.violetaserver.usuarios.dto.response.ClaveResponse;

/**
 * API del servicio para gestión de claves de usuario.
 * Migración de ServidorAdminSistema::CambiaClave y ServidorAdminSistema::AsignaPassword.
 */
public interface ClaveUsuarioService {

    /**
     * Asigna una clave inicial a un usuario que no tiene password (password IS NULL).
     * Equivalente a ID_ASIG_PASSWORD del legado.
     *
     * @param empleado identificador del usuario (PK usuarios.empleado)
     * @param request  contiene el hash SHA-256 de la nueva clave
     * @return resultado de la operación
     */
    ClaveResponse asignarClave(String empleado, AsignarClaveRequest request);

    /**
     * Cambia la clave de un usuario que ya tiene password.
     * Equivalente a ID_GRA_CAMBIACLAVE del legado.
     *
     * @param empleado identificador del usuario (PK usuarios.empleado)
     * @param request  contiene el hash SHA-256 de la clave actual y la nueva
     * @return resultado de la operación
     */
    ClaveResponse cambiarClave(String empleado, CambiarClaveRequest request);
}
