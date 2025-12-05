package com.lavioleta.desarrollo.violetaserver.bitacora_privilegios.repository;

import java.sql.Date;
import java.sql.Time;
import java.time.LocalDate;
import java.time.LocalTime;
import java.util.List;
import java.util.StringJoiner;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.jdbc.core.simple.JdbcClient;
import org.springframework.stereotype.Repository;

import com.lavioleta.desarrollo.violetaserver.bitacora_privilegios.dto.response.BitacoraPrivilegiosResponse.BitacoraPrivilegioEntry;

/**
 * Repository para consulta de bitácora de modificaciones de privilegios.
 * Migrado desde FormBitacoraModPrivilegios.cpp::MostrarBitacora()
 * 
 * Combina datos de:
 * - bitacoramodprivusu: modificaciones sobre privilegios de usuarios
 * - bitacoramodprivrol: modificaciones sobre privilegios de roles
 * 
 * La consulta utiliza UNION ALL para combinar ambas tablas.
 */
@Repository
public class BitacoraPrivilegiosRepository {

    private static final Logger log = LoggerFactory.getLogger(BitacoraPrivilegiosRepository.class);

    /**
     * Consulta base que combina bitácoras de usuarios y roles.
     * Los filtros de entidad se aplican dentro del UNION para optimizar.
     * Los filtros de usuario, rol y tipo se aplican al resultado combinado.
     */
    private static final String BITACORA_BASE_SQL = """
            SELECT
                usurol.fecha,
                usurol.hora,
                CONCAT(e.nombre, ' ', e.appat, ' ', e.apmat) AS usuario_operador,
                CONCAT(emod.nombre, ' ', emod.appat, ' ', emod.apmat) AS usuario_modificado,
                bpr.rol_mod,
                usurol.tipo_mod,
                usurol.entidad_mod,
                usurol.entidad_nombre,
                usurol.contexto
            FROM (
                SELECT
                    idbitacprivusu AS ID,
                    fecha,
                    hora,
                    usuario,
                    usuario_mod,
                    tipo_mod,
                    entidad_mod,
                    entidad_nombre,
                    'USUARIOS' AS contexto
                FROM bitacoramodprivusu
                WHERE fecha BETWEEN :fechaInicio AND :fechaFin
                %s
                UNION ALL
                SELECT
                    idbitacprivrol AS ID,
                    fecha,
                    hora,
                    usuario,
                    NULL AS usuario_mod,
                    tipo_mod,
                    entidad_mod,
                    entidad_nombre,
                    'ROLES' AS contexto
                FROM bitacoramodprivrol
                WHERE fecha BETWEEN :fechaInicio AND :fechaFin
                %s
            ) usurol
            LEFT JOIN bitacoramodprivusu bpu
                   ON bpu.idbitacprivusu = usurol.ID AND usurol.contexto = 'USUARIOS'
            LEFT JOIN bitacoramodprivrol bpr
                   ON bpr.idbitacprivrol = usurol.ID AND usurol.contexto = 'ROLES'
            LEFT JOIN empleados e
                   ON e.empleado = usurol.usuario
            LEFT JOIN empleados emod
                   ON emod.empleado = usurol.usuario_mod
            WHERE 1=1
            %s
            ORDER BY usurol.fecha DESC, usurol.hora DESC
            """;

    private final JdbcClient jdbcClient;

    public BitacoraPrivilegiosRepository(JdbcClient jdbcClient) {
        this.jdbcClient = jdbcClient;
    }

    /**
     * Consulta la bitácora de modificaciones de privilegios con filtros opcionales.
     *
     * @param fechaInicio Fecha inicial del rango (requerida)
     * @param fechaFin Fecha final del rango (requerida)
     * @param usuario Clave del empleado para filtrar (opcional)
     * @param rol Clave del rol para filtrar (opcional)
     * @param tipoContexto Contexto: USUARIOS o ROLES (opcional)
     * @param entidadInvolucrada Tipo de entidad: GRUPO, OBJETO, PRIVILEGIO, SUCURSAL, ROL, USUARIO (opcional)
     * @return Lista de eventos de bitácora
     */
    public List<BitacoraPrivilegioEntry> consultarBitacora(
            LocalDate fechaInicio,
            LocalDate fechaFin,
            String usuario,
            String rol,
            String tipoContexto,
            String entidadInvolucrada) {

        log.debug("Consultando bitácora de privilegios: fechas={} a {}, usuario={}, rol={}, tipo={}, entidad={}",
                fechaInicio, fechaFin, usuario, rol, tipoContexto, entidadInvolucrada);

        // Construir condición de entidad para aplicar dentro del UNION
        String condicionEntidadUsuarios = buildCondicionEntidad(entidadInvolucrada);
        String condicionEntidadRoles = buildCondicionEntidad(entidadInvolucrada);

        // Construir condiciones externas (post-UNION)
        StringJoiner condicionesExternas = new StringJoiner(" ");
        if (isNotBlank(usuario)) {
            // Filtrar donde el empleado actuó como operador O fue el usuario modificado
            condicionesExternas.add("AND (usurol.usuario = :usuario OR bpu.usuario_mod = :usuario)");
        }
        if (isNotBlank(rol)) {
            condicionesExternas.add("AND bpr.rol_mod = :rol");
        }
        if (isNotBlank(tipoContexto)) {
            condicionesExternas.add("AND usurol.contexto = :tipoContexto");
        }

        String sqlFinal = String.format(BITACORA_BASE_SQL,
                condicionEntidadUsuarios,
                condicionEntidadRoles,
                condicionesExternas.toString());

        log.trace("SQL generado: {}", sqlFinal);

        var query = jdbcClient.sql(sqlFinal)
                .param("fechaInicio", Date.valueOf(fechaInicio))
                .param("fechaFin", Date.valueOf(fechaFin));

        if (isNotBlank(usuario)) {
            query = query.param("usuario", usuario.trim());
        }
        if (isNotBlank(rol)) {
            query = query.param("rol", rol.trim());
        }
        if (isNotBlank(tipoContexto)) {
            query = query.param("tipoContexto", tipoContexto.trim().toUpperCase());
        }
        if (isNotBlank(entidadInvolucrada)) {
            query = query.param("entidadInvolucrada", entidadInvolucrada.trim().toUpperCase());
        }

        return query.query((rs, rowNum) -> BitacoraPrivilegioEntry.builder()
                        .fecha(toLocalDate(rs.getDate("fecha")))
                        .hora(toLocalTime(rs.getTime("hora")))
                        .usuarioOperador(rs.getString("usuario_operador"))
                        .usuarioModificado(rs.getString("usuario_modificado"))
                        .rolModificado(rs.getString("rol_mod"))
                        .tipoOperacion(rs.getString("tipo_mod"))
                        .entidadInvolucrada(rs.getString("entidad_mod"))
                        .nombreEntidad(rs.getString("entidad_nombre"))
                        .contexto(rs.getString("contexto"))
                        .build())
                .list();
    }

    /**
     * Construye la condición de filtro por entidad para aplicar dentro del UNION.
     * Esto optimiza la consulta filtrando antes de combinar los resultados.
     */
    private String buildCondicionEntidad(String entidadInvolucrada) {
        if (isNotBlank(entidadInvolucrada)) {
            return "AND entidad_mod = :entidadInvolucrada";
        }
        return "";
    }

    private static boolean isNotBlank(String value) {
        return value != null && !value.isBlank();
    }

    private static LocalDate toLocalDate(Date date) {
        return date != null ? date.toLocalDate() : null;
    }

    private static LocalTime toLocalTime(Time time) {
        return time != null ? time.toLocalTime() : null;
    }
}
