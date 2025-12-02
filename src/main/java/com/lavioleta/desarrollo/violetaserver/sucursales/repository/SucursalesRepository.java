package com.lavioleta.desarrollo.violetaserver.sucursales.repository;

import java.util.List;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.jdbc.core.simple.JdbcClient;
import org.springframework.stereotype.Repository;

import com.lavioleta.desarrollo.violetaserver.sucursales.dto.response.SucursalComboOptionResponse;

@Repository
public class SucursalesRepository {

    private static final Logger log = LoggerFactory.getLogger(SucursalesRepository.class);

    private final JdbcClient jdbcClient;

    public SucursalesRepository(JdbcClient jdbcClient) {
        this.jdbcClient = jdbcClient;
    }

    public List<SucursalComboOptionResponse> listarSucursalesCombo(Integer idEmpresa) {
        String sql = """
                SELECT s.sucursal, s.nombre
                FROM sucursales s
                %s
                ORDER BY s.nombre
                """;

        boolean filtrarEmpresa = idEmpresa != null;
        var statement = jdbcClient.sql(sql.formatted(filtrarEmpresa ? "WHERE s.idempresa = :idEmpresa" : ""));
        if (filtrarEmpresa) {
            statement = statement.param("idEmpresa", idEmpresa);
        }

        return statement
                .query((rs, rowNum) -> SucursalComboOptionResponse.builder()
                        .sucursal(rs.getString("sucursal"))
                        .nombre(rs.getString("nombre"))
                        .build())
                .list();
    }
}
