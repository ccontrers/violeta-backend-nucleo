package com.lavioleta.desarrollo.violetaserver.usuarios.repository;

import java.sql.Date;
import java.time.LocalDate;
import java.util.ArrayList;
import java.util.List;
import java.util.Optional;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.jdbc.core.simple.JdbcClient;
import org.springframework.stereotype.Repository;
import org.springframework.util.StringUtils;

import com.lavioleta.desarrollo.violetaserver.usuarios.dto.request.UsuarioRequest;
import com.lavioleta.desarrollo.violetaserver.usuarios.dto.response.EmpleadoOptionResponse;
import com.lavioleta.desarrollo.violetaserver.usuarios.dto.response.UsuarioComboOptionResponse;
import com.lavioleta.desarrollo.violetaserver.usuarios.dto.response.UsuarioListResponse;
import com.lavioleta.desarrollo.violetaserver.usuarios.dto.response.UsuarioResponse;

@Repository
public class CatalogoUsuariosRepository {

    private static final Logger log = LoggerFactory.getLogger(CatalogoUsuariosRepository.class);

    private final JdbcClient jdbcClient;

    public CatalogoUsuariosRepository(JdbcClient jdbcClient) {
        this.jdbcClient = jdbcClient;
    }

    public List<UsuarioListResponse.UsuarioSummary> listarUsuarios(boolean soloActivos, String filtro) {
        StringBuilder sql = new StringBuilder()
                .append("SELECT u.empleado, ")
                .append("       CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS nombreCompleto, ")
                .append("       (SELECT b.fecha FROM bitacorausuario b WHERE b.usuario = u.empleado ORDER BY b.fecha DESC LIMIT 1) AS ultimoAcceso, ")
                .append("       u.activo ")
                .append("FROM usuarios u ")
                .append("JOIN empleados e ON e.empleado = u.empleado ")
                .append("WHERE 1=1 ");

        List<Object> params = new ArrayList<>();
        if (soloActivos) {
            sql.append("AND u.activo = 1 ");
        }
        if (StringUtils.hasText(filtro)) {
            String like = "%" + filtro.trim() + "%";
            sql.append("AND (u.empleado LIKE ? OR e.nombre LIKE ? OR e.appat LIKE ? OR e.apmat LIKE ?) ");
            params.add(like);
            params.add(like);
            params.add(like);
            params.add(like);
        }
        sql.append("ORDER BY nombreCompleto ASC");

        var statement = jdbcClient.sql(sql.toString());
        for (Object param : params) {
            statement = statement.param(param);
        }

        return statement.query((rs, rowNum) -> UsuarioListResponse.UsuarioSummary.builder()
                        .empleado(rs.getString("empleado"))
                        .nombre(rs.getString("nombreCompleto"))
                        .ultimoAcceso(getLocalDate(rs.getDate("ultimoAcceso")))
                        .activo(rs.getBoolean("activo"))
                        .build())
                .list();
    }

    public Optional<UsuarioResponse.UsuarioDetail> obtenerUsuario(String empleado) {
        String sql = """
                SELECT u.empleado, u.password, u.activo, u.fechaalta, u.fechabaja,
                       u.usuariocontpaq, u.passwordcontpaq,
                       CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS nombreCompleto,
                       (SELECT b.fecha FROM bitacorausuario b WHERE b.usuario = u.empleado ORDER BY b.fecha DESC LIMIT 1) AS ultimoAcceso
                FROM usuarios u
                JOIN empleados e ON e.empleado = u.empleado
                WHERE u.empleado = ?
                """;

        return jdbcClient.sql(sql)
                .param(empleado)
                .query((rs, rowNum) -> UsuarioResponse.UsuarioDetail.builder()
                        .empleado(rs.getString("empleado"))
                        .nombreCompleto(rs.getString("nombreCompleto"))
                        .activo(rs.getBoolean("activo"))
                        .fechaAlta(getLocalDate(rs.getDate("fechaalta")))
                        .fechaBaja(getLocalDate(rs.getDate("fechabaja")))
                        .ultimoAcceso(getLocalDate(rs.getDate("ultimoAcceso")))
                        .password(rs.getString("password"))
                        .usuarioContpaq(rs.getString("usuariocontpaq"))
                        .passwordContpaq(rs.getString("passwordcontpaq"))
                        .build())
                .optional();
    }

    public List<EmpleadoOptionResponse> listarEmpleadosDisponibles() {
        String sql = """
                SELECT e.empleado,
                       CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS nombreCompleto,
                       CASE WHEN u.empleado IS NULL THEN 0 ELSE 1 END AS tieneUsuario
                FROM empleados e
                LEFT JOIN usuarios u ON u.empleado = e.empleado
                WHERE e.activo = 1
                ORDER BY nombreCompleto
                """;

        return jdbcClient.sql(sql)
                .query((rs, rowNum) -> EmpleadoOptionResponse.builder()
                        .empleado(rs.getString("empleado"))
                        .nombreCompleto(rs.getString("nombreCompleto"))
                        .tieneUsuario(rs.getInt("tieneUsuario") == 1)
                        .build())
                .list();
    }

    public boolean existeUsuario(String empleado) {
        Integer count = jdbcClient.sql("SELECT COUNT(1) FROM usuarios WHERE empleado = ?")
                .param(empleado)
                .query(Integer.class)
                .single();
        return count != null && count > 0;
    }

    public boolean existeEmpleado(String empleado) {
        Integer count = jdbcClient.sql("SELECT COUNT(1) FROM empleados WHERE empleado = ?")
                .param(empleado)
                .query(Integer.class)
                .single();
        return count != null && count > 0;
    }

    public List<UsuarioComboOptionResponse> listarUsuariosCombo(String sucursal) {
        StringBuilder sql = new StringBuilder()
                .append("SELECT u.empleado, ")
                .append("       TRIM(CONCAT_WS(' ', e.nombre, e.appat, e.apmat)) AS nombreCompleto, ")
                .append("       e.sucursal AS sucursal ")
                .append("FROM usuarios u ")
                .append("JOIN empleados e ON e.empleado = u.empleado ");

        List<Object> params = new ArrayList<>();
        if (StringUtils.hasText(sucursal)) {
            sql.append("WHERE e.sucursal = ? ");
            params.add(sucursal.trim());
        }

        sql.append("ORDER BY e.nombre, e.appat, e.apmat");

        var statement = jdbcClient.sql(sql.toString());
        for (Object param : params) {
            statement = statement.param(param);
        }

        return statement
                .query((rs, rowNum) -> UsuarioComboOptionResponse.builder()
                        .empleado(rs.getString("empleado"))
                        .nombreCompleto(rs.getString("nombreCompleto"))
                        .sucursal(rs.getString("sucursal"))
                        .build())
                .list();
    }

    public int insertarUsuario(UsuarioRequest request, String sucursal) {
        String sql = """
                INSERT INTO usuarios (empleado, password, activo, fechaalta, fechabaja, usuariocontpaq, passwordcontpaq)
                VALUES (?, ?, ?, ?, ?, ?, ?)
                """;

        int rows = jdbcClient.sql(sql)
            .param(request.getEmpleado())
            .param(optionalString(request.getPassword()))
                .param(request.getActivo() != null && !request.getActivo() ? 0 : 1)
                .param(toSqlDate(defaultFechaAlta(request)))
                .param(toSqlDate(defaultFechaBaja(request)))
            .param(optionalString(request.getUsuarioContpaq()))
            .param(optionalString(request.getPasswordContpaq()))
                .update();

        if (rows > 0) {
            jdbcClient.sql("INSERT IGNORE INTO usuariosucursal(usuario, sucursal) VALUES (?, ?)")
                    .param(request.getEmpleado())
                    .param(sucursal)
                    .update();
        }

        return rows;
    }

    public int actualizarUsuario(String empleado, UsuarioRequest request) {
        String sql = """
                UPDATE usuarios
                SET password = ?,
                    activo = ?,
                    fechaalta = ?,
                    fechabaja = ?,
                    usuariocontpaq = ?,
                    passwordcontpaq = ?
                WHERE empleado = ?
                """;

        return jdbcClient.sql(sql)
            .param(optionalString(request.getPassword()))
                .param(request.getActivo() != null && !request.getActivo() ? 0 : 1)
                .param(toSqlDate(defaultFechaAlta(request)))
                .param(toSqlDate(defaultFechaBaja(request)))
            .param(optionalString(request.getUsuarioContpaq()))
            .param(optionalString(request.getPasswordContpaq()))
                .param(empleado)
                .update();
    }

    public int eliminarUsuario(String empleado) {
        jdbcClient.sql("DELETE FROM usuariosucursal WHERE usuario = ?")
                .param(empleado)
                .update();
        return jdbcClient.sql("DELETE FROM usuarios WHERE empleado = ?")
                .param(empleado)
                .update();
    }

    private static LocalDate getLocalDate(Date sqlDate) {
        return sqlDate != null ? sqlDate.toLocalDate() : null;
    }

    private static Date toSqlDate(LocalDate date) {
        return date != null ? Date.valueOf(date) : null;
    }

    private static String optionalString(String value) {
        return (value == null || value.isBlank()) ? null : value.trim();
    }

    private static LocalDate defaultFechaAlta(UsuarioRequest request) {
        return request.getFechaAlta() != null ? request.getFechaAlta() : LocalDate.now();
    }

    private static LocalDate defaultFechaBaja(UsuarioRequest request) {
        return request.getFechaBaja() != null ? request.getFechaBaja() : LocalDate.of(2099, 1, 1);
    }
}
