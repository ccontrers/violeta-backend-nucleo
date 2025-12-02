package com.lavioleta.desarrollo.violetaserver.usuarios.repository;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.jdbc.core.simple.JdbcClient;
import org.springframework.stereotype.Repository;

/**
 * Repositorio para operaciones de clave de usuario.
 * Migración de las consultas SQL de ServidorAdminSistema::CambiaClave y ServidorAdminSistema::AsignaPassword.
 */
@Repository
public class ClaveUsuarioRepository {

    private static final Logger log = LoggerFactory.getLogger(ClaveUsuarioRepository.class);

    private final JdbcClient jdbcClient;

    public ClaveUsuarioRepository(JdbcClient jdbcClient) {
        this.jdbcClient = jdbcClient;
    }

    /**
     * Verifica si el usuario existe en la tabla usuarios.
     *
     * @param empleado identificador del usuario
     * @return true si existe
     */
    public boolean existeUsuario(String empleado) {
        String sql = "SELECT COUNT(1) FROM usuarios WHERE empleado = ?";
        Integer count = jdbcClient.sql(sql)
                .param(empleado)
                .query(Integer.class)
                .single();
        return count != null && count > 0;
    }

    /**
     * Verifica si el usuario ya tiene una clave asignada (password NOT NULL).
     *
     * @param empleado identificador del usuario
     * @return true si tiene clave
     */
    public boolean tieneClaveAsignada(String empleado) {
        String sql = "SELECT COUNT(1) FROM usuarios WHERE empleado = ? AND password IS NOT NULL";
        Integer count = jdbcClient.sql(sql)
                .param(empleado)
                .query(Integer.class)
                .single();
        return count != null && count > 0;
    }

    /**
     * Verifica que la clave actual del usuario coincida con la proporcionada.
     * Equivalente al primer SELECT del legado:
     * SELECT @idusuario:=empleado FROM usuarios WHERE empleado='X' AND password='Y'
     *
     * @param empleado     identificador del usuario
     * @param claveActual  hash SHA-256 de la clave actual
     * @return true si coincide
     */
    public boolean verificarClaveActual(String empleado, String claveActual) {
        String sql = "SELECT COUNT(1) FROM usuarios WHERE empleado = ? AND password = ?";
        Integer count = jdbcClient.sql(sql)
                .param(empleado)
                .param(claveActual)
                .query(Integer.class)
                .single();
        return count != null && count > 0;
    }

    /**
     * Asigna clave inicial a un usuario que no tiene password (password IS NULL).
     * Equivalente al UPDATE del legado:
     * UPDATE usuarios SET password='X' WHERE empleado='Y' AND password IS NULL
     *
     * @param empleado   identificador del usuario
     * @param nuevaClave hash SHA-256 de la nueva clave
     * @return número de filas afectadas (1 si éxito, 0 si no)
     */
    public int asignarClaveInicial(String empleado, String nuevaClave) {
        String sql = "UPDATE usuarios SET password = ? WHERE empleado = ? AND password IS NULL";
        log.debug("Ejecutando asignación de clave inicial para usuario {}", empleado);
        return jdbcClient.sql(sql)
                .param(nuevaClave)
                .param(empleado)
                .update();
    }

    /**
     * Cambia la clave de un usuario verificando la clave actual.
     * Equivalente al UPDATE del legado:
     * UPDATE usuarios SET password='nueva' WHERE empleado='X' AND password='actual'
     *
     * @param empleado     identificador del usuario
     * @param claveActual  hash SHA-256 de la clave actual
     * @param nuevaClave   hash SHA-256 de la nueva clave
     * @return número de filas afectadas (1 si éxito, 0 si no)
     */
    public int cambiarClave(String empleado, String claveActual, String nuevaClave) {
        String sql = "UPDATE usuarios SET password = ? WHERE empleado = ? AND password = ?";
        log.debug("Ejecutando cambio de clave para usuario {}", empleado);
        return jdbcClient.sql(sql)
                .param(nuevaClave)
                .param(empleado)
                .param(claveActual)
                .update();
    }
}
