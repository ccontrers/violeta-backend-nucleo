package com.lavioleta.desarrollo.violetaserver.repository;

import com.lavioleta.desarrollo.violetaserver.dto.response.BusquedaProveedoresResponse.ProveedorResultado;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.jdbc.core.simple.JdbcClient;
import org.springframework.stereotype.Repository;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.List;

@Repository
public class BusquedaProveedoresRepository {
    
    private static final Logger logger = LoggerFactory.getLogger(BusquedaProveedoresRepository.class);
    private static final int NUM_LIMITE_RESULTADOS_BUSQ = 50;
    
    private final JdbcClient jdbcClient;
    
    public BusquedaProveedoresRepository(JdbcClient jdbcClient) {
        this.jdbcClient = jdbcClient;
    }
    
    /**
     * Busca proveedores por razón social
     * Legacy: RSO -> razonsocial LIKE '%valor%' ORDER BY razonsocial
     */
    public List<ProveedorResultado> buscarPorRazonSocial(String razonSocial, String condicionActivos, 
                                                         String condicionProvGastos, String condicionProvMercancia, 
                                                         int limite) {
        String sql = String.format("""
            SELECT proveedor, razonsocial, replegal, rfc, estado, localidad, calle, colonia,
                   redondeocptecho, 
                   IF(provgastos=1, 'Si', 'No') as provgastos, 
                   IF(provmercancia=1, 'Si', 'No') as provmercancia,
                   activo, contacto, email
            FROM proveedores
            WHERE %s razonsocial LIKE ? %s %s
            ORDER BY razonsocial
            LIMIT ?
            """, condicionActivos, condicionProvGastos, condicionProvMercancia);
            
        logger.debug("Ejecutando búsqueda de proveedores por razón social: {}", razonSocial);
        
        return jdbcClient.sql(sql)
                .param("%" + razonSocial + "%")
                .param(limite)
                .query(this::mapToProveedorResultado)
                .list();
    }
    
    /**
     * Busca proveedores por RFC
     * Legacy: RFC -> rfc LIKE '%valor%' ORDER BY rfc, razonsocial
     */
    public List<ProveedorResultado> buscarPorRfc(String rfc, String condicionActivos, 
                                                 String condicionProvGastos, String condicionProvMercancia, 
                                                 int limite) {
        String sql = String.format("""
            SELECT proveedor, razonsocial, replegal, rfc, estado, localidad, calle, colonia,
                   redondeocptecho, 
                   IF(provgastos=1, 'Si', 'No') as provgastos, 
                   IF(provmercancia=1, 'Si', 'No') as provmercancia,
                   activo, contacto, email
            FROM proveedores
            WHERE %s rfc LIKE ? %s %s
            ORDER BY rfc, razonsocial
            LIMIT ?
            """, condicionActivos, condicionProvGastos, condicionProvMercancia);
            
        logger.debug("Ejecutando búsqueda de proveedores por RFC: {}", rfc);
        
        return jdbcClient.sql(sql)
                .param("%" + rfc + "%")
                .param(limite)
                .query(this::mapToProveedorResultado)
                .list();
    }
    
    /**
     * Busca proveedores por clave (proveedor)
     * Legacy: CLA -> proveedor LIKE '%valor%' ORDER BY rfc, razonsocial
     */
    public List<ProveedorResultado> buscarPorClave(String clave, String condicionActivos, 
                                                   String condicionProvGastos, String condicionProvMercancia, 
                                                   int limite) {
        String sql = String.format("""
            SELECT proveedor, razonsocial, replegal, rfc, estado, localidad, calle, colonia,
                   redondeocptecho, 
                   IF(provgastos=1, 'Si', 'No') as provgastos, 
                   IF(provmercancia=1, 'Si', 'No') as provmercancia,
                   activo, contacto, email
            FROM proveedores
            WHERE %s proveedor LIKE ? %s %s
            ORDER BY rfc, razonsocial
            LIMIT ?
            """, condicionActivos, condicionProvGastos, condicionProvMercancia);
            
        logger.debug("Ejecutando búsqueda de proveedores por clave: {}", clave);
        
        return jdbcClient.sql(sql)
                .param("%" + clave + "%")
                .param(limite)
                .query(this::mapToProveedorResultado)
                .list();
    }
    
    /**
     * Busca proveedores por representante legal
     * Legacy: REP -> replegal LIKE '%valor%' ORDER BY razonsocial
     */
    public List<ProveedorResultado> buscarPorRepresentanteLegal(String replegal, String condicionActivos, 
                                                                String condicionProvGastos, String condicionProvMercancia, 
                                                                int limite) {
        String sql = String.format("""
            SELECT proveedor, razonsocial, replegal, rfc, estado, localidad, calle, colonia,
                   redondeocptecho, 
                   IF(provgastos=1, 'Si', 'No') as provgastos, 
                   IF(provmercancia=1, 'Si', 'No') as provmercancia,
                   activo, contacto, email
            FROM proveedores
            WHERE %s replegal LIKE ? %s %s
            ORDER BY razonsocial
            LIMIT ?
            """, condicionActivos, condicionProvGastos, condicionProvMercancia);
            
        logger.debug("Ejecutando búsqueda de proveedores por representante legal: {}", replegal);
        
        return jdbcClient.sql(sql)
                .param("%" + replegal + "%")
                .param(limite)
                .query(this::mapToProveedorResultado)
                .list();
    }
    
    /**
     * Mapea ResultSet a ProveedorResultado
     * Convierte los valores 'Si'/'No' legacy a boolean
     */
    private ProveedorResultado mapToProveedorResultado(ResultSet rs, int rowNum) throws SQLException {
        return ProveedorResultado.builder()
                .proveedor(rs.getString("proveedor"))
                .razonsocial(rs.getString("razonsocial"))
                .replegal(rs.getString("replegal"))
                .rfc(rs.getString("rfc"))
                .estado(rs.getString("estado"))
                .localidad(rs.getString("localidad"))
                .calle(rs.getString("calle"))
                .colonia(rs.getString("colonia"))
                .redondeocptecho(rs.getInt("redondeocptecho") == 1)
                .provgastos("Si".equals(rs.getString("provgastos")))
                .provmercancia("Si".equals(rs.getString("provmercancia")))
                .activo(rs.getBoolean("activo"))
                .contacto(rs.getString("contacto"))
                .email(rs.getString("email"))
                .build();
    }
}