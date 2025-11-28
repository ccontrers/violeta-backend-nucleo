package com.lavioleta.desarrollo.violetaserver.usuarios.service;

import java.util.List;

import com.lavioleta.desarrollo.violetaserver.usuarios.dto.request.UsuarioRequest;
import com.lavioleta.desarrollo.violetaserver.usuarios.dto.response.EmpleadoOptionResponse;
import com.lavioleta.desarrollo.violetaserver.usuarios.dto.response.UsuarioListResponse;
import com.lavioleta.desarrollo.violetaserver.usuarios.dto.response.UsuarioResponse;

/**
 * API del servicio para el catálogo de usuarios legado (Consulta/Graba/Baja).
 */
public interface CatalogoUsuariosService {

    UsuarioListResponse listarUsuarios(boolean soloActivos, String filtro);

    UsuarioResponse obtenerUsuario(String empleado);

    UsuarioResponse crearUsuario(UsuarioRequest request, String sucursalHeader);

    UsuarioResponse actualizarUsuario(String empleado, UsuarioRequest request);

    UsuarioResponse eliminarUsuario(String empleado);

    List<EmpleadoOptionResponse> obtenerEmpleadosDisponibles();
}
