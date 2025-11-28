package com.lavioleta.desarrollo.violetaserver.acceso.service;

import com.lavioleta.desarrollo.violetaserver.acceso.dto.request.LoginRequest;
import com.lavioleta.desarrollo.violetaserver.acceso.dto.response.LoginResponse;

import java.util.List;
import java.util.Map;

public interface LoginService {
    
    /**
     * Autentica un usuario con sus credenciales
     * @param request datos de login (usuario y password hash)
     * @param clientIp IP del cliente para auditoría
     * @return respuesta con resultado de autenticación
     */
    LoginResponse autenticar(LoginRequest request, String clientIp);
    
    /**
     * Autentica un usuario y genera un token JWT
     * @param request datos de login (usuario y password hash)
     * @param clientIp IP del cliente para auditoría
     * @return respuesta con resultado de autenticación y token JWT
     */
    LoginResponse autenticarConJwt(LoginRequest request, String clientIp);
    
    /**
     * Valida el formato del password hash
     * @param passwordHash hash a validar
     * @return true si el formato es válido
     */
    boolean validarFormatoPasswordHash(String passwordHash);
    
    /**
     * Obtiene la lista de sucursales disponibles
     * @return lista de mapas con sucursal y nombre
     */
    List<Map<String, String>> obtenerSucursales();
}
