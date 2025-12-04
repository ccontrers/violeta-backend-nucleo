package com.lavioleta.desarrollo.violetaserver.autorizacion.repository;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.EnumMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

import org.springframework.jdbc.core.RowMapper;
import org.springframework.jdbc.core.simple.JdbcClient;
import org.springframework.stereotype.Repository;

import com.lavioleta.desarrollo.violetaserver.autorizacion.dto.security.VersionRequirementDTO;

import lombok.RequiredArgsConstructor;

@Repository
@RequiredArgsConstructor
public class AuthorizationRepository {

    private static final String PRIVILEGE_SQL_TEMPLATE = """
        WITH privilegios_directos AS (
            SELECT a.usuario, a.objeto, a.privilegio
            FROM asignacionprivilegios a
            JOIN usuarios u ON u.empleado = a.usuario AND u.activo = 1
        ), privilegios_roles AS (
            SELECT u.empleado AS usuario, ar.objeto, ar.privilegio
            FROM usuarios u
            JOIN usuariorol ur ON ur.usuario = u.empleado
            JOIN asignacionprivrol ar ON ar.rol = ur.rol
        ), privilegios_puesto AS (
            SELECT u.empleado AS usuario, ar.objeto, ar.privilegio
            FROM usuarios u
            JOIN empleados e ON e.empleado = u.empleado
            JOIN rolesxpuesto rxp ON rxp.puesto = e.puesto
            JOIN asignacionprivrol ar ON ar.rol = rxp.rol
        )
        SELECT DISTINCT p.usuario, p.objeto, p.privilegio, pr.descripcion
        FROM (
            SELECT * FROM privilegios_directos
            UNION ALL SELECT * FROM privilegios_roles
            UNION ALL SELECT * FROM privilegios_puesto
        ) p
        JOIN privilegios pr ON pr.objeto = p.objeto AND pr.privilegio = p.privilegio
        WHERE p.usuario = :usuario
        %s
        ORDER BY p.objeto, p.privilegio
        """;

    private static final String VERSION_SQL = """
        SELECT parametro, valor
        FROM parametrosemp
        WHERE parametro IN ('VERSIONMINIMA', 'SUBVERSIONMIN', 'TIEMVALIDVERS')
        """;

    private final JdbcClient jdbcClient;

    public List<PrivilegeRecord> findAllByUsuario(String usuario) {
        return queryPrivileges(usuario, Optional.empty());
    }

    public List<PrivilegeRecord> findAllByUsuarioAndObjeto(String usuario, String objeto) {
        return queryPrivileges(usuario, Optional.ofNullable(objeto));
    }

    public Optional<VersionRequirementDTO> findVersionRequirements() {
        List<VersionRow> rows = jdbcClient.sql(VERSION_SQL)
            .query(this::mapVersionRow)
            .list();

        if (rows.isEmpty()) {
            return Optional.empty();
        }

        Map<VersionParam, String> parametros = rows.stream()
            .collect(() -> new EnumMap<>(VersionParam.class),
                (mapa, row) -> mapa.put(row.param(), row.valor()),
                EnumMap::putAll);

        String min = parametros.getOrDefault(VersionParam.VERSIONMINIMA, "1.0");
        String sub = parametros.getOrDefault(VersionParam.SUBVERSIONMIN, "0");
        int grace = parseIntSafe(parametros.getOrDefault(VersionParam.TIEMVALIDVERS, "0"));

        return Optional.of(VersionRequirementDTO.builder()
            .versionMinima(min)
            .subversionMinima(sub)
            .tiempoValidezMinutos(grace)
            .build());
    }

    private List<PrivilegeRecord> queryPrivileges(String usuario, Optional<String> objeto) {
        String extraClause = objeto.map(o -> " AND p.objeto = :objeto").orElse("");
        return jdbcClient.sql(PRIVILEGE_SQL_TEMPLATE.formatted(extraClause))
            .param("usuario", usuario)
            .param("objeto", objeto.orElse(null))
            .query(new PrivilegeRowMapper())
            .list();
    }

    private VersionRow mapVersionRow(ResultSet rs, int rowNum) throws SQLException {
        String parametro = rs.getString("parametro").toUpperCase();
        String valor = rs.getString("valor");
        return new VersionRow(VersionParam.valueOf(parametro), valor);
    }

    private int parseIntSafe(String raw) {
        try {
            return Integer.parseInt(raw);
        } catch (NumberFormatException ex) {
            return 0;
        }
    }

    public record PrivilegeRecord(String usuario, String objeto, String privilegio, String descripcion) {}

    private record VersionRow(VersionParam param, String valor) {}

    private enum VersionParam {
        VERSIONMINIMA,
        SUBVERSIONMIN,
        TIEMVALIDVERS
    }

    private static class PrivilegeRowMapper implements RowMapper<PrivilegeRecord> {
        @Override
        public PrivilegeRecord mapRow(ResultSet rs, int rowNum) throws SQLException {
            return new PrivilegeRecord(
                rs.getString("usuario"),
                rs.getString("objeto"),
                rs.getString("privilegio"),
                rs.getString("descripcion")
            );
        }
    }
}
