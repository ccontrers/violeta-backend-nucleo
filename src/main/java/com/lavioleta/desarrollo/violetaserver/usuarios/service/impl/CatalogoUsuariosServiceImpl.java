package com.lavioleta.desarrollo.violetaserver.usuarios.service.impl;

import java.util.Collections;
import java.util.List;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;
import org.springframework.util.StringUtils;

import com.lavioleta.desarrollo.violetaserver.usuarios.dto.request.UsuarioRequest;
import com.lavioleta.desarrollo.violetaserver.usuarios.dto.response.EmpleadoOptionResponse;
import com.lavioleta.desarrollo.violetaserver.usuarios.dto.response.UsuarioComboOptionResponse;
import com.lavioleta.desarrollo.violetaserver.usuarios.dto.response.UsuarioListResponse;
import com.lavioleta.desarrollo.violetaserver.usuarios.dto.response.UsuarioResponse;
import com.lavioleta.desarrollo.violetaserver.usuarios.repository.CatalogoUsuariosRepository;
import com.lavioleta.desarrollo.violetaserver.usuarios.service.CatalogoUsuariosService;

@Service
public class CatalogoUsuariosServiceImpl implements CatalogoUsuariosService {

    private static final Logger log = LoggerFactory.getLogger(CatalogoUsuariosServiceImpl.class);
    private static final String DEFAULT_SUCURSAL = "S1";

    private final CatalogoUsuariosRepository repository;

    public CatalogoUsuariosServiceImpl(CatalogoUsuariosRepository repository) {
        this.repository = repository;
    }

    @Override
    public UsuarioListResponse listarUsuarios(boolean soloActivos, String filtro) {
        try {
            List<UsuarioListResponse.UsuarioSummary> usuarios = repository.listarUsuarios(soloActivos, filtro);
            String message = usuarios.isEmpty() ? "No se encontraron usuarios" : "Usuarios recuperados correctamente";
            return UsuarioListResponse.success(message, usuarios);
        } catch (Exception ex) {
            log.error("Error al listar usuarios", ex);
            return UsuarioListResponse.error("Error al listar usuarios: " + ex.getMessage());
        }
    }

    @Override
    public UsuarioResponse obtenerUsuario(String empleado) {
        try {
            return repository.obtenerUsuario(empleado)
                    .map(detail -> UsuarioResponse.ok("Usuario recuperado correctamente", detail))
                    .orElseGet(() -> UsuarioResponse.error("Usuario no encontrado: " + empleado));
        } catch (Exception ex) {
            log.error("Error al obtener usuario {}", empleado, ex);
            return UsuarioResponse.error("Error al obtener usuario: " + ex.getMessage());
        }
    }

    @Override
    @Transactional
    public UsuarioResponse crearUsuario(UsuarioRequest request, String sucursalHeader) {
        String empleado = request.getEmpleado();
        if (!repository.existeEmpleado(empleado)) {
            return UsuarioResponse.error("El empleado " + empleado + " no existe en el catálogo de empleados");
        }
        if (repository.existeUsuario(empleado)) {
            return UsuarioResponse.error("Ya existe un usuario asociado al empleado " + empleado);
        }

        try {
            int rows = repository.insertarUsuario(request, resolveSucursal(sucursalHeader));
            if (rows == 0) {
                return UsuarioResponse.error("No se pudo crear el usuario para " + empleado);
            }
            return repository.obtenerUsuario(empleado)
                    .map(detail -> UsuarioResponse.ok("Usuario creado correctamente", detail))
                    .orElseGet(() -> UsuarioResponse.ok("Usuario creado", null));
        } catch (Exception ex) {
            log.error("Error al crear usuario {}", empleado, ex);
            return UsuarioResponse.error("Error al crear usuario: " + ex.getMessage());
        }
    }

    @Override
    @Transactional
    public UsuarioResponse actualizarUsuario(String empleado, UsuarioRequest request) {
        if (!repository.existeUsuario(empleado)) {
            return UsuarioResponse.error("No existe un usuario registrado para el empleado " + empleado);
        }

        try {
            int rows = repository.actualizarUsuario(empleado, request);
            if (rows == 0) {
                return UsuarioResponse.error("No se realizaron cambios para el usuario " + empleado);
            }
            return repository.obtenerUsuario(empleado)
                    .map(detail -> UsuarioResponse.ok("Usuario actualizado correctamente", detail))
                    .orElseGet(() -> UsuarioResponse.ok("Usuario actualizado", null));
        } catch (Exception ex) {
            log.error("Error al actualizar usuario {}", empleado, ex);
            return UsuarioResponse.error("Error al actualizar usuario: " + ex.getMessage());
        }
    }

    @Override
    @Transactional
    public UsuarioResponse eliminarUsuario(String empleado) {
        if (!repository.existeUsuario(empleado)) {
            return UsuarioResponse.error("No existe un usuario registrado para el empleado " + empleado);
        }

        try {
            int rows = repository.eliminarUsuario(empleado);
            if (rows == 0) {
                return UsuarioResponse.error("El usuario no pudo eliminarse, verifique referencias");
            }
            return UsuarioResponse.ok("Usuario eliminado correctamente", null);
        } catch (Exception ex) {
            log.error("Error al eliminar usuario {}", empleado, ex);
            return UsuarioResponse.error("Error al eliminar usuario: " + ex.getMessage());
        }
    }

    @Override
    public List<EmpleadoOptionResponse> obtenerEmpleadosDisponibles() {
        return repository.listarEmpleadosDisponibles();
    }

    @Override
    public List<UsuarioComboOptionResponse> listarUsuariosCombo(String sucursal) {
        return repository.listarUsuariosCombo(sucursal);
    }

    private String resolveSucursal(String sucursalHeader) {
        return StringUtils.hasText(sucursalHeader) ? sucursalHeader.trim() : DEFAULT_SUCURSAL;
    }
}
