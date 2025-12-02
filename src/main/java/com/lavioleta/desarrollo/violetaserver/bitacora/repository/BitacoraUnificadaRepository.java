package com.lavioleta.desarrollo.violetaserver.bitacora.repository;

import java.sql.Date;
import java.sql.Time;
import java.time.LocalDate;
import java.time.LocalTime;
import java.util.ArrayList;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Set;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.jdbc.core.simple.JdbcClient;
import org.springframework.stereotype.Repository;

import com.lavioleta.desarrollo.violetaserver.bitacora.dto.response.BitacoraUnificadaResponse.BitacoraUsuarioEntry;

@Repository
public class BitacoraUnificadaRepository {

    private static final Logger log = LoggerFactory.getLogger(BitacoraUnificadaRepository.class);

    private static final String BITACORA_SQL = """
(SELECT v.referencia AS referencia, 'VENT' AS tipodocumento, 'ALTA' AS operacion, v.cancelado, v.fechavta AS fechadoc,
        v.usualta AS usuariooper, v.fechaalta AS fechaoper, v.horaalta AS horaoper
 FROM ventas v
 WHERE v.cancelado = 0
   AND v.fechaalta BETWEEN :fechaInicio AND :fechaFin
  AND v.usualta IN (:usuarios))
UNION
(SELECT v.referencia AS referencia, 'VENT' AS tipodocumento, 'MODI' AS operacion, v.cancelado, v.fechavta AS fechadoc,
        v.usumodi AS usuariooper, v.fechamodi AS fechaoper, v.horamodi AS horaoper
 FROM ventas v
 WHERE (v.fechaalta <> v.fechamodi OR v.horaalta <> v.horamodi)
   AND v.cancelado = 0
   AND v.fechamodi BETWEEN :fechaInicio AND :fechaFin
  AND v.usumodi IN (:usuarios))
UNION
(SELECT v.referencia AS referencia, 'VENT' AS tipodocumento, 'CANC' AS operacion, v.cancelado, v.fechavta AS fechadoc,
        v.usumodi AS usuariooper, v.fechamodi AS fechaoper, v.horamodi AS horaoper
 FROM ventas v
 WHERE v.cancelado = 1
   AND v.fechamodi BETWEEN :fechaInicio AND :fechaFin
  AND v.usumodi IN (:usuarios))
UNION
(SELECT c.referencia AS referencia, 'COMP' AS tipodocumento, 'ALTA' AS operacion, c.cancelado, c.fechacom AS fechadoc,
        c.usualta AS usuariooper, c.fechaalta AS fechaoper, c.horaalta AS horaoper
 FROM compras c
 WHERE c.cancelado = 0
   AND c.fechaalta BETWEEN :fechaInicio AND :fechaFin
  AND c.usualta IN (:usuarios))
UNION
(SELECT c.referencia AS referencia, 'COMP' AS tipodocumento, 'MODI' AS operacion, c.cancelado, c.fechacom AS fechadoc,
        c.usumodi AS usuariooper, c.fechamodi AS fechaoper, c.horaalta AS horaoper
 FROM compras c
 WHERE (c.fechaalta <> c.fechamodi OR c.horaalta <> c.horamodi)
   AND c.cancelado = 0
   AND c.fechamodi BETWEEN :fechaInicio AND :fechaFin
  AND c.usumodi IN (:usuarios))
UNION
(SELECT c.referencia AS referencia, 'COMP' AS tipodocumento, 'CANC' AS operacion, c.cancelado, c.fechacom AS fechadoc,
        c.usumodi AS usuariooper, c.fechamodi AS fechaoper, c.horaalta AS horaoper
 FROM compras c
 WHERE c.cancelado = 1
   AND c.fechamodi BETWEEN :fechaInicio AND :fechaFin
  AND c.usumodi IN (:usuarios))
UNION
(SELECT ncc.referencia AS referencia, 'NCREDCLI' AS tipodocumento, 'ALTA' AS operacion, ncc.cancelado, ncc.fechanot AS fechadoc,
        ncc.usualta AS usuariooper, ncc.fechaalta AS fechaoper, ncc.horaalta AS horaoper
 FROM notascredcli ncc
 WHERE ncc.cancelado = 0
   AND ncc.fechaalta BETWEEN :fechaInicio AND :fechaFin
  AND ncc.usualta IN (:usuarios))
UNION
(SELECT ncc.referencia AS referencia, 'NCREDCLI' AS tipodocumento, 'MODI' AS operacion, ncc.cancelado, ncc.fechanot AS fechadoc,
        ncc.usumodi AS usuariooper, ncc.fechamodi AS fechaoper, ncc.horamodi AS horaoper
 FROM notascredcli ncc
 WHERE (ncc.fechaalta <> ncc.fechamodi OR ncc.horaalta <> ncc.horamodi)
   AND ncc.cancelado = 0
   AND ncc.fechamodi BETWEEN :fechaInicio AND :fechaFin
  AND ncc.usumodi IN (:usuarios))
UNION
(SELECT ncc.referencia AS referencia, 'NCREDCLI' AS tipodocumento, 'CANC' AS operacion, ncc.cancelado, ncc.fechanot AS fechadoc,
        ncc.usumodi AS usuariooper, ncc.fechamodi AS fechaoper, ncc.horamodi AS horaoper
 FROM notascredcli ncc
 WHERE ncc.cancelado = 1
   AND ncc.fechamodi BETWEEN :fechaInicio AND :fechaFin
  AND ncc.usumodi IN (:usuarios))
UNION
(SELECT ncp.referencia AS referencia, 'NCREDPROV' AS tipodocumento, 'ALTA' AS operacion, ncp.cancelado, ncp.fechanot AS fechadoc,
        ncp.usualta AS usuariooper, ncp.fechaalta AS fechaoper, ncp.horaalta AS horaoper
 FROM notascredprov ncp
 WHERE ncp.cancelado = 0
   AND ncp.fechaalta BETWEEN :fechaInicio AND :fechaFin
  AND ncp.usualta IN (:usuarios))
UNION
(SELECT ncp.referencia AS referencia, 'NCREDPROV' AS tipodocumento, 'MODI' AS operacion, ncp.cancelado, ncp.fechanot AS fechadoc,
        ncp.usumodi AS usuariooper, ncp.fechamodi AS fechaoper, ncp.horamodi AS horaoper
 FROM notascredcli ncp
 WHERE (ncp.fechaalta <> ncp.fechamodi OR ncp.horaalta <> ncp.horamodi)
   AND ncp.cancelado = 0
   AND ncp.fechamodi BETWEEN :fechaInicio AND :fechaFin
  AND ncp.usumodi IN (:usuarios))
UNION
(SELECT ncp.referencia AS referencia, 'NCREDPROV' AS tipodocumento, 'CANC' AS operacion, ncp.cancelado, ncp.fechanot AS fechadoc,
        ncp.usumodi AS usuariooper, ncp.fechamodi AS fechaoper, ncp.horamodi AS horaoper
 FROM notascredcli ncp
 WHERE ncp.cancelado = 1
   AND ncp.fechamodi BETWEEN :fechaInicio AND :fechaFin
  AND ncp.usumodi IN (:usuarios))
UNION
(SELECT ncc.referencia AS referencia, 'NCARGOCLI' AS tipodocumento, 'ALTA' AS operacion, ncc.cancelado, ncc.fechanot AS fechadoc,
        ncc.usualta AS usuariooper, ncc.fechaalta AS fechaoper, ncc.horaalta AS horaoper
 FROM notascarcli ncc
 WHERE ncc.cancelado = 0
   AND ncc.fechaalta BETWEEN :fechaInicio AND :fechaFin
  AND ncc.usualta IN (:usuarios))
UNION
(SELECT ncc.referencia AS referencia, 'NCARGOCLI' AS tipodocumento, 'MODI' AS operacion, ncc.cancelado, ncc.fechanot AS fechadoc,
        ncc.usumodi AS usuariooper, ncc.fechamodi AS fechaoper, ncc.horamodi AS horaoper
 FROM notascarcli ncc
 WHERE (ncc.fechaalta <> ncc.fechamodi OR ncc.horaalta <> ncc.horamodi)
   AND ncc.cancelado = 0
   AND ncc.fechaalta BETWEEN :fechaInicio AND :fechaFin
  AND ncc.usualta IN (:usuarios))
UNION
(SELECT ncc.referencia AS referencia, 'NCARGOCLI' AS tipodocumento, 'CANC' AS operacion, ncc.cancelado, ncc.fechanot AS fechadoc,
        ncc.usumodi AS usuariooper, ncc.fechamodi AS fechaoper, ncc.horamodi AS horaoper
 FROM notascarcli ncc
 WHERE ncc.cancelado = 1
   AND ncc.fechaalta BETWEEN :fechaInicio AND :fechaFin
  AND ncc.usualta IN (:usuarios))
UNION
(SELECT ncp.referencia AS referencia, 'NCARGOPROV' AS tipodocumento, 'ALTA' AS operacion, ncp.cancelado, ncp.fechanot AS fechadoc,
        ncp.usualta AS usuariooper, ncp.fechaalta AS fechaoper, ncp.horaalta AS horaoper
 FROM notascarprov ncp
 WHERE ncp.cancelado = 0
   AND ncp.fechaalta BETWEEN :fechaInicio AND :fechaFin
  AND ncp.usualta IN (:usuarios))
UNION
(SELECT ncp.referencia AS referencia, 'NCARGOPROV' AS tipodocumento, 'MODI' AS operacion, ncp.cancelado, ncp.fechanot AS fechadoc,
        ncp.usumodi AS usuariooper, ncp.fechamodi AS fechaoper, ncp.horamodi AS horaoper
 FROM notascarprov ncp
 WHERE (ncp.fechaalta <> ncp.fechamodi OR ncp.horaalta <> ncp.horamodi)
   AND ncp.cancelado = 0
   AND ncp.fechaalta BETWEEN :fechaInicio AND :fechaFin
  AND ncp.usumodi IN (:usuarios))
UNION
(SELECT ncp.referencia AS referencia, 'NCARGOPROV' AS tipodocumento, 'CANC' AS operacion, ncp.cancelado, ncp.fechanot AS fechadoc,
        ncp.usumodi AS usuariooper, ncp.fechamodi AS fechaoper, ncp.horamodi AS horaoper
 FROM notascarprov ncp
 WHERE ncp.cancelado = 1
   AND ncp.fechamodi BETWEEN :fechaInicio AND :fechaFin
  AND ncp.usumodi IN (:usuarios))
UNION
(SELECT ma.movimiento AS referencia, 'MOVALMA' AS tipodocumento, 'ENT-ALTA' AS operacion, ma.cancelado, ma.fechamov AS fechadoc,
        ma.usualta AS usuariooper, ma.fechaalta AS fechaoper, ma.horaalta AS horaoper
 FROM movalma ma
 WHERE ma.tipo = 'E'
   AND ma.cancelado = 0
   AND ma.fechaalta BETWEEN :fechaInicio AND :fechaFin
  AND ma.usualta IN (:usuarios))
UNION
(SELECT ma.movimiento AS referencia, 'MOVALMA' AS tipodocumento, 'ENT-MODI' AS operacion, ma.cancelado, ma.fechamov AS fechadoc,
        ma.usumodi AS usuariooper, ma.fechamodi AS fechaoper, ma.horamodi AS horaoper
 FROM movalma ma
 WHERE ma.tipo = 'E'
   AND ma.cancelado = 0
   AND (ma.fechaalta <> ma.fechamodi OR ma.horaalta <> ma.horamodi)
   AND ma.fechamodi BETWEEN :fechaInicio AND :fechaFin
  AND ma.usumodi IN (:usuarios))
UNION
(SELECT ma.movimiento AS referencia, 'MOVALMA' AS tipodocumento, 'ENT-CANC' AS operacion, ma.cancelado, ma.fechamov AS fechadoc,
        ma.usumodi AS usuariooper, ma.fechamodi AS fechaoper, ma.horamodi AS horaoper
 FROM movalma ma
 WHERE ma.tipo = 'E'
   AND ma.cancelado = 1
   AND ma.fechamodi BETWEEN :fechaInicio AND :fechaFin
  AND ma.usumodi IN (:usuarios))
UNION
(SELECT ma.movimiento AS referencia, 'MOVALMA' AS tipodocumento, 'SAL-ALTA' AS operacion, ma.cancelado, ma.fechamov AS fechadoc,
        ma.usualta AS usuariooper, ma.fechaalta AS fechaoper, ma.horaalta AS horaoper
 FROM movalma ma
 WHERE ma.tipo = 'S'
   AND ma.cancelado = 0
   AND ma.fechaalta BETWEEN :fechaInicio AND :fechaFin
  AND ma.usualta IN (:usuarios))
UNION
(SELECT ma.movimiento AS referencia, 'MOVALMA' AS tipodocumento, 'SAL-MODI' AS operacion, ma.cancelado, ma.fechamov AS fechadoc,
        ma.usumodi AS usuariooper, ma.fechamodi AS fechaoper, ma.horamodi AS horaoper
 FROM movalma ma
 WHERE ma.tipo = 'S'
   AND ma.cancelado = 0
   AND (ma.fechaalta <> ma.fechamodi OR ma.horaalta <> ma.horamodi)
   AND ma.fechamodi BETWEEN :fechaInicio AND :fechaFin
  AND ma.usumodi IN (:usuarios))
UNION
(SELECT ma.movimiento AS referencia, 'MOVALMA' AS tipodocumento, 'SAL-CANC' AS operacion, ma.cancelado, ma.fechamov AS fechadoc,
        ma.usumodi AS usuariooper, ma.fechamodi AS fechaoper, ma.horamodi AS horaoper
 FROM movalma ma
 WHERE ma.tipo = 'S'
   AND ma.cancelado = 1
   AND ma.fechamodi BETWEEN :fechaInicio AND :fechaFin
  AND ma.usumodi IN (:usuarios))
UNION
(SELECT ma.movimiento AS referencia, 'MOVALMA' AS tipodocumento, 'TRASP-ALTA' AS operacion, ma.cancelado, ma.fechamov AS fechadoc,
        ma.usualta AS usuariooper, ma.fechaalta AS fechaoper, ma.horaalta AS horaoper
 FROM movalma ma
 WHERE ma.tipo = 'T'
   AND ma.cancelado = 0
   AND ma.fechaalta BETWEEN :fechaInicio AND :fechaFin
  AND ma.usualta IN (:usuarios))
UNION
(SELECT ma.movimiento AS referencia, 'MOVALMA' AS tipodocumento, 'TRASP-MODI' AS operacion, ma.cancelado, ma.fechamov AS fechadoc,
        ma.usumodi AS usuariooper, ma.fechamodi AS fechaoper, ma.horamodi AS horaoper
 FROM movalma ma
 WHERE ma.tipo = 'T'
   AND ma.cancelado = 0
   AND (ma.fechaalta <> ma.fechamodi OR ma.horaalta <> ma.horamodi)
   AND ma.fechamodi BETWEEN :fechaInicio AND :fechaFin
  AND ma.usumodi IN (:usuarios))
UNION
(SELECT ma.movimiento AS referencia, 'MOVALMA' AS tipodocumento, 'TRASP-CANC' AS operacion, ma.cancelado, ma.fechamov AS fechadoc,
        ma.usumodi AS usuariooper, ma.fechamodi AS fechaoper, ma.horamodi AS horaoper
 FROM movalma ma
 WHERE ma.tipo = 'T'
   AND ma.cancelado = 1
   AND ma.fechamodi BETWEEN :fechaInicio AND :fechaFin
  AND ma.usumodi IN (:usuarios))
UNION
(SELECT pc.pago AS referencia, 'PAGOSCLI' AS tipodocumento, 'ALTA' AS operacion, pc.cancelado, pc.fecha AS fechadoc,
        pc.usualta AS usuariooper, pc.fecha AS fechaoper, pc.hora AS horaoper
 FROM pagoscli pc
 WHERE pc.cancelado = 0
   AND pc.fecha BETWEEN :fechaInicio AND :fechaFin
  AND pc.usualta IN (:usuarios))
UNION
(SELECT pc.pago AS referencia, 'PAGOSCLI' AS tipodocumento, 'MODI' AS operacion, pc.cancelado, pc.fecha AS fechadoc,
        pc.usumodi AS usuariooper, pc.fechamodi AS fechaoper, pc.horamodi AS horaoper
 FROM pagoscli pc
 WHERE pc.cancelado = 0
   AND (pc.fecha <> pc.fechamodi AND pc.hora <> pc.horamodi)
   AND pc.fechamodi BETWEEN :fechaInicio AND :fechaFin
  AND pc.usumodi IN (:usuarios))
UNION
(SELECT pc.pago AS referencia, 'PAGOSCLI' AS tipodocumento, 'CANC' AS operacion, pc.cancelado, pc.fecha AS fechadoc,
        pc.usumodi AS usuariooper, pc.fechamodi AS fechaoper, pc.horamodi AS horaoper
 FROM pagoscli pc
 WHERE pc.cancelado = 1
   AND pc.fechamodi BETWEEN :fechaInicio AND :fechaFin
  AND pc.usumodi IN (:usuarios))
UNION
(SELECT pp.pago AS referencia, 'PAGOSPROV' AS tipodocumento, 'ALTA' AS operacion, pp.cancelado, pp.fecha AS fechadoc,
        pp.usualta AS usuariooper, pp.fecha AS fechaoper, pp.hora AS horaoper
 FROM pagosprov pp
 WHERE pp.cancelado = 0
   AND pp.fecha BETWEEN :fechaInicio AND :fechaFin
  AND pp.usualta IN (:usuarios))
UNION
(SELECT pp.pago AS referencia, 'PAGOSPROV' AS tipodocumento, 'MODI' AS operacion, pp.cancelado, pp.fecha AS fechadoc,
        pp.usumodi AS usuariooper, pp.fechamodi AS fechaoper, pp.horamodi AS horaoper
 FROM pagosprov pp
 WHERE pp.cancelado = 0
   AND (pp.fecha <> pp.fechamodi AND pp.hora <> pp.horamodi)
   AND pp.fechamodi BETWEEN :fechaInicio AND :fechaFin
  AND pp.usumodi IN (:usuarios))
UNION
(SELECT pp.pago AS referencia, 'PAGOSPROV' AS tipodocumento, 'CANC' AS operacion, pp.cancelado, pp.fecha AS fechadoc,
        pp.usumodi AS usuariooper, pp.fechamodi AS fechaoper, pp.horamodi AS horaoper
 FROM pagosprov pp
 WHERE pp.cancelado = 1
   AND pp.fechamodi BETWEEN :fechaInicio AND :fechaFin
  AND pp.usumodi IN (:usuarios))
UNION
(SELECT pvta.referencia AS referencia, 'PEDVENTA' AS tipodocumento, 'ALTA' AS operacion, pvta.cancelado, pvta.fechaped AS fechadoc,
        pvta.usumodi AS usuariooper, pvta.fechamodi AS fechaoper, pvta.horamodi AS horaoper
 FROM pedidosventa pvta
 WHERE pvta.cancelado = 0
   AND pvta.fechamodi BETWEEN :fechaInicio AND :fechaFin
  AND pvta.usumodi IN (:usuarios))
UNION
(SELECT pvta.referencia AS referencia, 'PEDVENTA' AS tipodocumento, 'MODI' AS operacion, pvta.cancelado, pvta.fechaped AS fechadoc,
        pvta.usumodi AS usuariooper, pvta.fechamodi AS fechaoper, pvta.horamodi AS horaoper
 FROM pedidosventa pvta
 WHERE (pvta.fechaalta <> pvta.fechamodi OR pvta.horaalta <> pvta.horamodi)
   AND pvta.cancelado = 1
   AND pvta.fechamodi BETWEEN :fechaInicio AND :fechaFin
  AND pvta.usumodi IN (:usuarios))
UNION
(SELECT pvta.referencia AS referencia, 'PEDVENTA' AS tipodocumento, 'CANC' AS operacion, pvta.cancelado, pvta.fechaped AS fechadoc,
        pvta.usumodi AS usuariooper, pvta.fechamodi AS fechaoper, pvta.horamodi AS horaoper
 FROM pedidosventa pvta
 WHERE pvta.cancelado = 1
   AND pvta.fechamodi BETWEEN :fechaInicio AND :fechaFin
  AND pvta.usumodi IN (:usuarios))
UNION
(SELECT pcom.referencia AS referencia, 'PEDCOMPRA' AS tipodocumento, 'ALTA' AS operacion, pcom.cancelado, pcom.fechaped AS fechadoc,
        pcom.usumodi AS usuariooper, pcom.fechamodi AS fechaoper, pcom.horamodi AS horaoper
 FROM pedidos pcom
 WHERE pcom.cancelado = 0
   AND pcom.fechamodi BETWEEN :fechaInicio AND :fechaFin
  AND pcom.usumodi IN (:usuarios))
UNION
(SELECT pcom.referencia AS referencia, 'PEDCOMPRA' AS tipodocumento, 'MODI' AS operacion, pcom.cancelado, pcom.fechaped AS fechadoc,
        pcom.usumodi AS usuariooper, pcom.fechamodi AS fechaoper, pcom.horamodi AS horaoper
 FROM pedidos pcom
 WHERE (pcom.fechaalta <> pcom.fechamodi OR pcom.horaalta <> pcom.horamodi)
   AND pcom.cancelado = 1
   AND pcom.fechamodi BETWEEN :fechaInicio AND :fechaFin
  AND pcom.usumodi IN (:usuarios))
UNION
(SELECT pcom.referencia AS referencia, 'PEDCOMPRA' AS tipodocumento, 'CANC' AS operacion, pcom.cancelado, pcom.fechaped AS fechadoc,
        pcom.usumodi AS usuariooper, pcom.fechamodi AS fechaoper, pcom.horamodi AS horaoper
 FROM pedidos pcom
 WHERE pcom.cancelado = 1
   AND pcom.fechamodi BETWEEN :fechaInicio AND :fechaFin
  AND pcom.usumodi IN (:usuarios))
ORDER BY fechaoper DESC, horaoper DESC, referencia
""";

  private static final String USUARIOS_HOMONIMOS_SQL = """
SELECT e2.empleado
FROM empleados e
INNER JOIN empleados e2
    ON e.nombre = e2.nombre
     AND e.appat = e2.appat
     AND e.apmat = e2.apmat
WHERE e.empleado = :usuario
""";

    private final JdbcClient jdbcClient;

    public BitacoraUnificadaRepository(JdbcClient jdbcClient) {
        this.jdbcClient = jdbcClient;
    }

    public List<BitacoraUsuarioEntry> consultarBitacoraUsuarios(LocalDate fechaInicio, LocalDate fechaFin, List<String> usuarios) {
    List<String> usuariosExpandidos = expandirUsuariosPorNombre(usuarios);
    if (usuariosExpandidos.isEmpty()) {
            log.debug("Consulta de bitácora unificada omitida por lista de usuarios vacía");
            return List.of();
        }

        return jdbcClient.sql(BITACORA_SQL)
                .param("fechaInicio", Date.valueOf(fechaInicio))
                .param("fechaFin", Date.valueOf(fechaFin))
        .param("usuarios", usuariosExpandidos)
                .query((rs, rowNum) -> {
                    Integer cancelado = rs.getObject("cancelado", Integer.class);
                    return BitacoraUsuarioEntry.builder()
                            .referencia(rs.getString("referencia"))
                            .tipoDocumento(rs.getString("tipodocumento"))
                            .operacion(rs.getString("operacion"))
                            .cancelado(cancelado != null && cancelado.intValue() != 0)
                            .fechaDocumento(toLocalDate(rs.getDate("fechadoc")))
                            .usuario(rs.getString("usuariooper"))
                            .fechaOperacion(toLocalDate(rs.getDate("fechaoper")))
                            .horaOperacion(toLocalTime(rs.getTime("horaoper")))
                            .build();
                })
                .list();
    }

          private List<String> expandirUsuariosPorNombre(List<String> usuariosBase) {
            if (usuariosBase == null || usuariosBase.isEmpty()) {
              return List.of();
            }

            Set<String> acumulado = new LinkedHashSet<>();

            for (String usuario : usuariosBase) {
              if (usuario == null || usuario.isBlank()) {
                continue;
              }

              String usuarioNormalizado = usuario.trim();
              List<String> relacionados = jdbcClient.sql(USUARIOS_HOMONIMOS_SQL)
                  .param("usuario", usuarioNormalizado)
                  .query(String.class)
                  .list();

              if (relacionados.isEmpty()) {
                acumulado.add(usuarioNormalizado);
                continue;
              }

              relacionados.stream()
                  .filter(item -> item != null && !item.isBlank())
                  .map(String::trim)
                  .forEach(acumulado::add);
            }

            return acumulado.isEmpty() ? List.of() : new ArrayList<>(acumulado);
          }

    private static LocalDate toLocalDate(Date date) {
        return date != null ? date.toLocalDate() : null;
    }

    private static LocalTime toLocalTime(Time time) {
        return time != null ? time.toLocalTime() : null;
    }
}
