package com.lavioleta.desarrollo.violetaserver.usuarios.service.impl;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import com.lavioleta.desarrollo.violetaserver.usuarios.dto.request.AsignarClaveRequest;
import com.lavioleta.desarrollo.violetaserver.usuarios.dto.request.CambiarClaveRequest;
import com.lavioleta.desarrollo.violetaserver.usuarios.dto.response.ClaveResponse;
import com.lavioleta.desarrollo.violetaserver.usuarios.repository.ClaveUsuarioRepository;
import com.lavioleta.desarrollo.violetaserver.usuarios.service.ClaveUsuarioService;

/**
 * Implementación del servicio de gestión de claves de usuario.
 * Réplica de la lógica de ServidorAdminSistema::CambiaClave y ServidorAdminSistema::AsignaPassword.
 */
@Service
public class ClaveUsuarioServiceImpl implements ClaveUsuarioService {

    private static final Logger log = LoggerFactory.getLogger(ClaveUsuarioServiceImpl.class);

    private final ClaveUsuarioRepository repository;

    public ClaveUsuarioServiceImpl(ClaveUsuarioRepository repository) {
        this.repository = repository;
    }

    @Override
    @Transactional
    public ClaveResponse asignarClave(String empleado, AsignarClaveRequest request) {
        log.debug("Asignando clave inicial al usuario {}", empleado);

        // Verificar que el usuario existe
        if (!repository.existeUsuario(empleado)) {
            log.warn("Usuario {} no encontrado para asignar clave", empleado);
            return ClaveResponse.error("Usuario no encontrado");
        }

        // Verificar que el usuario NO tiene clave (password IS NULL)
        if (repository.tieneClaveAsignada(empleado)) {
            log.warn("Usuario {} ya tiene clave asignada, no se puede usar asignar clave inicial", empleado);
            return ClaveResponse.error("El usuario ya tiene una clave asignada. Use el endpoint de cambio de clave.");
        }

        // Ejecutar la asignación (equivalente al UPDATE del legado)
        int filasAfectadas = repository.asignarClaveInicial(empleado, request.getNuevaClave());

        if (filasAfectadas > 0) {
            log.info("Clave inicial asignada correctamente al usuario {}", empleado);
            return ClaveResponse.exito(empleado, "Clave asignada correctamente");
        } else {
            log.error("No se pudo asignar clave inicial al usuario {}", empleado);
            return ClaveResponse.error("No se logró asignar la clave");
        }
    }

    @Override
    @Transactional
    public ClaveResponse cambiarClave(String empleado, CambiarClaveRequest request) {
        log.debug("Cambiando clave del usuario {}", empleado);

        // Verificar que el usuario existe
        if (!repository.existeUsuario(empleado)) {
            log.warn("Usuario {} no encontrado para cambio de clave", empleado);
            return ClaveResponse.error("Usuario no encontrado");
        }

        // Verificar que la clave actual coincide (equivalente al SELECT del legado)
        if (!repository.verificarClaveActual(empleado, request.getClaveActual())) {
            log.warn("Clave actual incorrecta para usuario {}", empleado);
            return ClaveResponse.error("La clave actual no es correcta");
        }

        // Ejecutar el cambio de clave (equivalente al UPDATE del legado)
        int filasAfectadas = repository.cambiarClave(empleado, request.getClaveActual(), request.getNuevaClave());

        if (filasAfectadas > 0) {
            log.info("Clave cambiada correctamente para usuario {}", empleado);
            return ClaveResponse.exito(empleado, "Clave cambiada correctamente");
        } else {
            log.error("No se logró cambiar la clave del usuario {}", empleado);
            return ClaveResponse.error("No se logró cambiar la clave. Verifique su clave anterior y reintente.");
        }
    }
}
