package com.lavioleta.desarrollo.violetaserver.roles_sistema.repository;

import java.util.List;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.jdbc.core.simple.JdbcClient;
import org.springframework.stereotype.Repository;

import com.lavioleta.desarrollo.violetaserver.roles_sistema.dto.response.RolComboOptionResponse;

/**
 * Repository para consultas de roles del sistema.
 * Tabla: rolessistema
 */
@Repository
public class RolesSistemaRepository {

    private static final Logger log = LoggerFactory.getLogger(RolesSistemaRepository.class);

    /**
     * Consulta SQL para obtener roles del sistema para ComboBox.
     * Migrado desde FormBitacoraModPrivilegios.cpp::BlanqueaFormulario() - ComboBoxRol
     */
    private static final String ROLES_COMBO_SQL = """
            SELECT claverol, nombre
            FROM rolessistema
            ORDER BY nombre
            """;

    private final JdbcClient jdbcClient;

    public RolesSistemaRepository(JdbcClient jdbcClient) {
        this.jdbcClient = jdbcClient;
    }

    /**
     * Obtiene la lista de roles del sistema para el combo box.
     * 
     * @return Lista de roles ordenados por nombre
     */
    public List<RolComboOptionResponse> obtenerRolesParaCombo() {
        log.debug("Consultando roles para combo box");
        return jdbcClient.sql(ROLES_COMBO_SQL)
                .query((rs, rowNum) -> RolComboOptionResponse.builder()
                        .clave(rs.getString("claverol"))
                        .nombre(rs.getString("nombre"))
                        .build())
                .list();
    }
}
