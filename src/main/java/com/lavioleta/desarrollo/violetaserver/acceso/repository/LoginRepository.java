package com.lavioleta.desarrollo.violetaserver.acceso.repository;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.jdbc.core.simple.JdbcClient;
import org.springframework.stereotype.Repository;

import com.lavioleta.desarrollo.violetaserver.acceso.dto.response.LoginResponse.UsuarioInfo;

import java.util.*;

@Repository
public class LoginRepository {
    
    private static final Logger logger = LoggerFactory.getLogger(LoginRepository.class);
    private final JdbcClient jdbcClient;
    
    public LoginRepository(JdbcClient jdbcClient) {
        this.jdbcClient = jdbcClient;
    }
    
    /**
     * Verifica las credenciales del usuario de forma segura
     * @param usuario nombre de usuario
     * @param passwordHash hash SHA-256 del password
     * @param sucursalSeleccionada código de sucursal seleccionada por el usuario en el login
     * @return Optional con información del usuario si las credenciales son válidas
     */
    public Optional<UsuarioInfo> verificarCredenciales(String usuario, String passwordHash, String sucursalSeleccionada) {
        // Query con JOIN para obtener nombre del empleado y nombre de la sucursal seleccionada
         String sql = """
             SELECT u.empleado, u.password, u.activo,
                 CONCAT(e.nombre, ' ', e.appat, ' ', e.apmat) AS nombre_completo,
                 us.sucursal AS sucursal,
                 s.nombre AS sucursal_nombre,
                 s.idempresa,
                 emp.nombre AS empresa_nombre,
                 'Usuario' AS perfil
             FROM usuarios u
             INNER JOIN empleados e ON u.empleado = e.empleado
             INNER JOIN usuariosucursal us ON us.usuario = u.empleado AND us.sucursal = ?
             INNER JOIN sucursales s ON s.sucursal = us.sucursal
             INNER JOIN empresas emp ON emp.idempresa = s.idempresa
             WHERE u.empleado = ?
             """;
        
        logger.info("Verificando credenciales para usuario: {} en sucursal: {}", usuario, sucursalSeleccionada);
        logger.debug("SQL: {}", sql);
        
        try {
                String usuarioUpper = usuario.toUpperCase();
                return jdbcClient.sql(sql)
                    .param(sucursalSeleccionada)        // Para INNER JOIN usuariosucursal
                    .param(usuarioUpper)                // Usuario
                    .query((rs, rowNum) -> {
                        String storedPasswordHash = rs.getString("password");
                        
                        // Verificación segura de password hash
                        if (storedPasswordHash != null && storedPasswordHash.equalsIgnoreCase(passwordHash)) {
                            // Password correcto, crear UsuarioInfo con datos reales
                            return UsuarioInfo.builder()
                                    .empleado(rs.getString("empleado"))
                                    .nombre(rs.getString("nombre_completo"))
                                    .sucursal(rs.getString("sucursal"))
                                    .sucursalNombre(rs.getString("sucursal_nombre"))
                                    .idempresa(rs.getInt("idempresa"))
                                    .empresaNombre(rs.getString("empresa_nombre"))
                                    .activo(rs.getBoolean("activo"))
                                    .perfil(rs.getString("perfil"))
                                    .build();
                        } else {
                            // Password incorrecto
                            logger.warn("Password incorrecto para usuario: {}", usuario);
                            return null;
                        }
                    })
                    .optional()
                    .filter(usuarioInfo -> usuarioInfo != null); // Filtrar nulls (password incorrecto)
                    
        } catch (Exception e) {
            logger.error("Error al verificar credenciales para usuario {}: {}", usuario, e.getMessage());
            return Optional.empty();
        }
    }
    
    /**
     * Verifica si un usuario existe
     */
    public boolean usuarioExisteYActivo(String usuario) {
        String sql = "SELECT COUNT(*) FROM usuarios u WHERE u.empleado = ?";
        
        try {
            Integer count = jdbcClient.sql(sql)
                    .param(usuario.toUpperCase())
                    .query(Integer.class)
                    .single();
            
            return count != null && count > 0;
        } catch (Exception e) {
            logger.error("Error al verificar existencia de usuario {}: {}", usuario, e.getMessage());
            return false;
        }
    }
    
    /**
     * Obtiene la lista de sucursales disponibles
     * @return lista de mapas con sucursal y nombre
     */
    public List<Map<String, String>> obtenerSucursales() {
        String sql = "SELECT sucursal, nombre FROM sucursales ORDER BY sucursal";
        
        logger.debug("Ejecutando consulta para obtener sucursales: {}", sql);
        
        try {
            List<Map<String, String>> sucursales = jdbcClient.sql(sql)
                    .query((rs, rowNum) -> {
                        Map<String, String> sucursal = new HashMap<>();
                        sucursal.put("sucursal", rs.getString("sucursal"));
                        sucursal.put("nombre", rs.getString("nombre"));
                        return sucursal;
                    })
                    .list();
            
            logger.debug("Se obtuvieron {} sucursales de la base de datos", sucursales.size());
            return sucursales;
            
        } catch (Exception e) {
            logger.error("Error al obtener sucursales: {}", e.getMessage(), e);
            throw e; // Re-lanzar para que el servicio maneje el fallback
        }
    }
    
    /**
     * Registra un intento de login para auditoría
     */
    public void registrarIntentoLogin(String usuario, String ip, boolean exitoso) {
        try {
            // Auditoría deshabilitada temporalmente - tabla no existe
            logger.debug("Auditoría de login para usuario: {}, exitoso: {}", usuario, exitoso);
            /*
            // Si existe tabla de auditoría, registrar el intento
            String sql = """
                INSERT INTO auditoria_login (usuario, ip, exitoso, fecha_hora) 
                VALUES (?, ?, ?, NOW())
                """;
            
            jdbcClient.sql(sql)
                    .param(usuario.toUpperCase())
                    .param(ip)
                    .param(exitoso ? 1 : 0)
                    .update();
            */
                    
        } catch (Exception e) {
            // No fallar si no existe tabla de auditoría
            logger.debug("No se pudo registrar auditoría de login: {}", e.getMessage());
        }
    }
}
