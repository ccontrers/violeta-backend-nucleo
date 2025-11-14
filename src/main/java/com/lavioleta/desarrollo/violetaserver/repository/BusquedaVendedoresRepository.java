package com.lavioleta.desarrollo.violetaserver.repository;

import com.lavioleta.desarrollo.violetaserver.dto.response.BusquedaVendedoresResponse.VendedorResultado;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.jdbc.core.simple.JdbcClient;
import org.springframework.stereotype.Repository;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.List;

/**
 * Repository para búsqueda de vendedores
 * Migrado desde ServidorBusquedas::BuscaVendedores (ClassServidorBusquedas.cpp líneas 3895-3950)
 * 
 * SQL Legacy:
 * - NOM: WHERE e.nombre LIKE 'valor%' ORDER BY e.nombre
 * - APE: WHERE (e.appat LIKE 'valor%' OR e.apmat LIKE 'valor%') ORDER BY e.appat
 * - COMI: WHERE v.tipocomi = 'valor' ORDER BY v.tipocomi
 * - CLA: WHERE e.empleado LIKE 'valor%' ORDER BY e.empleado
 * 
 * Todas usan: INNER JOIN vendedores v ON e.empleado = v.empleado
 * Filtro activos: AND v.activo=1 (si soloActivos=true) o AND v.activo=0 (si false)
 */
@Repository
public class BusquedaVendedoresRepository {
    
    private static final Logger logger = LoggerFactory.getLogger(BusquedaVendedoresRepository.class);
    
    private final JdbcClient jdbcClient;
    
    public BusquedaVendedoresRepository(JdbcClient jdbcClient) {
        this.jdbcClient = jdbcClient;
    }
    
    /**
     * Busca vendedores por nombre
     * Legacy: SELECT e.empleado, CONCAT(e.nombre,' ',e.appat,' ',e.apmat) nombre,
     *         e.localidad, v.tipocomi
     *         FROM empleados e INNER JOIN vendedores v ON e.empleado=v.empleado
     *         WHERE e.nombre LIKE 'valor%' AND v.activo={0|1}
     *         ORDER BY e.nombre
     *         LIMIT 501
     */
    public List<VendedorResultado> buscarPorNombre(String nombre, boolean soloActivos, int limite) {
        String sql = """
            SELECT e.empleado,
                   CONCAT(e.nombre, ' ', e.appat, ' ', e.apmat) AS nombre,
                   e.localidad,
                   v.tipocomi,
                   v.activo
            FROM empleados e
            INNER JOIN vendedores v ON e.empleado = v.empleado
            WHERE e.nombre LIKE ?
              AND v.activo = ?
            ORDER BY e.nombre
            LIMIT ?
            """;
        
        logger.debug("Ejecutando búsqueda de vendedores por nombre: {} (soloActivos: {})", nombre, soloActivos);
        
        return jdbcClient.sql(sql)
                .param(nombre + "%")
                .param(soloActivos ? 1 : 0)
                .param(limite)
                .query(this::mapToVendedorResultado)
                .list();
    }
    
    /**
     * Busca vendedores por apellidos (paterno o materno)
     * Legacy: WHERE (e.appat LIKE 'valor%' OR e.apmat LIKE 'valor%')
     *         ORDER BY e.appat
     */
    public List<VendedorResultado> buscarPorApellido(String apellido, boolean soloActivos, int limite) {
        String sql = """
            SELECT e.empleado,
                   CONCAT(e.nombre, ' ', e.appat, ' ', e.apmat) AS nombre,
                   e.localidad,
                   v.tipocomi,
                   v.activo
            FROM empleados e
            INNER JOIN vendedores v ON e.empleado = v.empleado
            WHERE (e.appat LIKE ? OR e.apmat LIKE ?)
              AND v.activo = ?
            ORDER BY e.appat
            LIMIT ?
            """;
        
        logger.debug("Ejecutando búsqueda de vendedores por apellido: {} (soloActivos: {})", apellido, soloActivos);
        
        String patronApellido = apellido + "%";
        return jdbcClient.sql(sql)
                .param(patronApellido)
                .param(patronApellido)
                .param(soloActivos ? 1 : 0)
                .param(limite)
                .query(this::mapToVendedorResultado)
                .list();
    }
    
    /**
     * Busca vendedores por tipo de comisión (coincidencia exacta)
     * Legacy: WHERE v.tipocomi = 'valor'
     *         ORDER BY v.tipocomi
     */
    public List<VendedorResultado> buscarPorTipoComision(String tipoComision, boolean soloActivos, int limite) {
        String sql = """
            SELECT e.empleado,
                   CONCAT(e.nombre, ' ', e.appat, ' ', e.apmat) AS nombre,
                   e.localidad,
                   v.tipocomi,
                   v.activo
            FROM empleados e
            INNER JOIN vendedores v ON e.empleado = v.empleado
            WHERE v.tipocomi = ?
              AND v.activo = ?
            ORDER BY v.tipocomi
            LIMIT ?
            """;
        
        logger.debug("Ejecutando búsqueda de vendedores por tipo de comisión: {} (soloActivos: {})", 
                    tipoComision, soloActivos);
        
        return jdbcClient.sql(sql)
                .param(tipoComision)
                .param(soloActivos ? 1 : 0)
                .param(limite)
                .query(this::mapToVendedorResultado)
                .list();
    }
    
    /**
     * Busca vendedores por clave de empleado
     * Legacy: WHERE e.empleado LIKE 'valor%'
     *         ORDER BY e.empleado
     */
    public List<VendedorResultado> buscarPorClave(String clave, boolean soloActivos, int limite) {
        String sql = """
            SELECT e.empleado,
                   CONCAT(e.nombre, ' ', e.appat, ' ', e.apmat) AS nombre,
                   e.localidad,
                   v.tipocomi,
                   v.activo
            FROM empleados e
            INNER JOIN vendedores v ON e.empleado = v.empleado
            WHERE e.empleado LIKE ?
              AND v.activo = ?
            ORDER BY e.empleado
            LIMIT ?
            """;
        
        logger.debug("Ejecutando búsqueda de vendedores por clave: {} (soloActivos: {})", clave, soloActivos);
        
        return jdbcClient.sql(sql)
                .param(clave + "%")
                .param(soloActivos ? 1 : 0)
                .param(limite)
                .query(this::mapToVendedorResultado)
                .list();
    }
    
    /**
     * Mapea ResultSet a VendedorResultado
     * Columnas del legacy:
     * - empleado: PK varchar(10)
     * - nombre: CONCAT de nombre + appat + apmat
     * - localidad: varchar(40)
     * - tipocomi: varchar(2)
     * - activo: tinyint(1) -> Boolean
     */
    private VendedorResultado mapToVendedorResultado(ResultSet rs, int rowNum) throws SQLException {
        return VendedorResultado.builder()
                .empleado(rs.getString("empleado"))
                .nombre(rs.getString("nombre"))
                .localidad(rs.getString("localidad"))
                .tipocomi(rs.getString("tipocomi"))
                .activo(rs.getInt("activo") == 1)
                .build();
    }
}
