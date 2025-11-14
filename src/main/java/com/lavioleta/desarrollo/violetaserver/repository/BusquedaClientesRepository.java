package com.lavioleta.desarrollo.violetaserver.repository;

import com.lavioleta.desarrollo.violetaserver.dto.response.BusquedaClientesResponse.ClienteResultado;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.jdbc.core.simple.JdbcClient;
import org.springframework.stereotype.Repository;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.List;

@Repository
public class BusquedaClientesRepository {
    
    private static final Logger logger = LoggerFactory.getLogger(BusquedaClientesRepository.class);
    private static final int NUM_LIMITE_RESULTADOS_BUSQ = 50;
    
    private final JdbcClient jdbcClient;
    
    public BusquedaClientesRepository(JdbcClient jdbcClient) {
        this.jdbcClient = jdbcClient;
    }
    
    /**
     * Busca clientes por nombre completo (nombre + apellidos)
     */
    public List<ClienteResultado> buscarPorNombre(String nombre, String condicionActivos) {
        String sql = String.format("""
            SELECT cli.cliente, cli.rsocial, cli.nomnegocio, cli.rfc, 
                   CONCAT(COALESCE(rf.regimenfiscal, ''), ' - ', COALESCE(rf.descripcion, '')) AS regimenfiscal,
                   cli.contacto, mun.nombre as municipio, loc.nombre as localidad,
                   cli.calle, col.nombre as colonia, cli.cp,
                   COALESCE(tel.telefono, '') as telefono, cli.email, cli.email2, cli.activo
            FROM clientes cli
            LEFT JOIN colonias col ON cli.colonia = col.colonia
            LEFT JOIN localidades loc ON loc.localidad = col.localidad
            LEFT JOIN municipios mun ON loc.municipio = mun.municipio
            LEFT JOIN cregimenfiscal rf ON cli.regimenfiscal = rf.regimenfiscal
            LEFT JOIN telefonosclientes tel ON cli.cliente = tel.cliente AND tel.tipo = 'PRINCIPAL'
            WHERE %s CONCAT(cli.nombre, ' ', cli.appat, ' ', cli.apmat) LIKE ?
            ORDER BY REPLACE(cli.rsocial, '\"', '') 
            LIMIT ?
            """, condicionActivos);
            
        logger.debug("Ejecutando búsqueda de clientes por nombre: {}", nombre);
        
        return jdbcClient.sql(sql)
                .param("%" + nombre + "%")
                .param(NUM_LIMITE_RESULTADOS_BUSQ)
                .query(this::mapToClienteResultado)
                .list();
    }
    
    /**
     * Busca clientes por apellido paterno
     */
    public List<ClienteResultado> buscarPorApellido(String apellido, String condicionActivos) {
        String sql = String.format("""
            SELECT cli.cliente, cli.rsocial, cli.nomnegocio, cli.rfc, 
                   CONCAT(COALESCE(rf.regimenfiscal, ''), ' - ', COALESCE(rf.descripcion, '')) AS regimenfiscal,
                   cli.contacto, mun.nombre as municipio, loc.nombre as localidad,
                   cli.calle, col.nombre as colonia, cli.cp,
                   COALESCE(tel.telefono, '') as telefono, cli.email, cli.email2, cli.activo
            FROM clientes cli
            LEFT JOIN colonias col ON cli.colonia = col.colonia
            LEFT JOIN localidades loc ON loc.localidad = col.localidad
            LEFT JOIN municipios mun ON loc.municipio = mun.municipio
            LEFT JOIN cregimenfiscal rf ON cli.regimenfiscal = rf.regimenfiscal
            LEFT JOIN telefonosclientes tel ON cli.cliente = tel.cliente AND tel.tipo = 'PRINCIPAL'
            WHERE %s cli.appat LIKE ?
            ORDER BY REPLACE(cli.rsocial, '\"', '') 
            LIMIT ?
            """, condicionActivos);
            
        logger.debug("Ejecutando búsqueda de clientes por apellido: {}", apellido);
        
        return jdbcClient.sql(sql)
                .param("%" + apellido + "%")
                .param(NUM_LIMITE_RESULTADOS_BUSQ)
                .query(this::mapToClienteResultado)
                .list();
    }
    
    /**
     * Busca clientes por RFC
     */
    public List<ClienteResultado> buscarPorRfc(String rfc, String condicionActivos) {
        String sql = String.format("""
            SELECT cli.cliente, cli.rsocial, cli.nomnegocio, cli.rfc, 
                   CONCAT(COALESCE(rf.regimenfiscal, ''), ' - ', COALESCE(rf.descripcion, '')) AS regimenfiscal,
                   cli.contacto, mun.nombre as municipio, loc.nombre as localidad,
                   cli.calle, col.nombre as colonia, cli.cp,
                   COALESCE(tel.telefono, '') as telefono, cli.email, cli.email2, cli.activo
            FROM clientes cli
            LEFT JOIN colonias col ON cli.colonia = col.colonia
            LEFT JOIN localidades loc ON loc.localidad = col.localidad
            LEFT JOIN municipios mun ON loc.municipio = mun.municipio
            LEFT JOIN cregimenfiscal rf ON cli.regimenfiscal = rf.regimenfiscal
            LEFT JOIN telefonosclientes tel ON cli.cliente = tel.cliente AND tel.tipo = 'PRINCIPAL'
            WHERE %s cli.rfc LIKE ?
            ORDER BY cli.rfc, REPLACE(cli.rsocial, '\"', '') 
            LIMIT ?
            """, condicionActivos);
            
        logger.debug("Ejecutando búsqueda de clientes por RFC: {}", rfc);
        
        return jdbcClient.sql(sql)
                .param("%" + rfc + "%")
                .param(NUM_LIMITE_RESULTADOS_BUSQ)
                .query(this::mapToClienteResultado)
                .list();
    }
    
    /**
     * Busca clientes por razón social
     */
    public List<ClienteResultado> buscarPorRazonSocial(String razonSocial, String condicionActivos) {
        String sql = String.format("""
            SELECT cli.cliente, cli.rsocial, cli.nomnegocio, cli.rfc, 
                   CONCAT(COALESCE(rf.regimenfiscal, ''), ' - ', COALESCE(rf.descripcion, '')) AS regimenfiscal,
                   cli.contacto, mun.nombre as municipio, loc.nombre as localidad,
                   cli.calle, col.nombre as colonia, cli.cp,
                   COALESCE(tel.telefono, '') as telefono, cli.email, cli.email2, cli.activo
            FROM clientes cli
            LEFT JOIN colonias col ON cli.colonia = col.colonia
            LEFT JOIN localidades loc ON loc.localidad = col.localidad
            LEFT JOIN municipios mun ON loc.municipio = mun.municipio
            LEFT JOIN cregimenfiscal rf ON cli.regimenfiscal = rf.regimenfiscal
            LEFT JOIN telefonosclientes tel ON cli.cliente = tel.cliente AND tel.tipo = 'PRINCIPAL'
            WHERE %s cli.rsocial LIKE ?
            ORDER BY REPLACE(cli.rsocial, '\"', '') 
            LIMIT ?
            """, condicionActivos);
            
        logger.debug("Ejecutando búsqueda de clientes por razón social: {}", razonSocial);
        
        return jdbcClient.sql(sql)
                .param("%" + razonSocial + "%")
                .param(NUM_LIMITE_RESULTADOS_BUSQ)
                .query(this::mapToClienteResultado)
                .list();
    }
    
    /**
     * Busca clientes por nombre de negocio
     */
    public List<ClienteResultado> buscarPorNombreNegocio(String nombreNegocio, String condicionActivos) {
        String sql = String.format("""
            SELECT cli.cliente, cli.rsocial, cli.nomnegocio, cli.rfc, 
                   CONCAT(COALESCE(rf.regimenfiscal, ''), ' - ', COALESCE(rf.descripcion, '')) AS regimenfiscal,
                   cli.contacto, mun.nombre as municipio, loc.nombre as localidad,
                   cli.calle, col.nombre as colonia, cli.cp,
                   COALESCE(tel.telefono, '') as telefono, cli.email, cli.email2, cli.activo
            FROM clientes cli
            LEFT JOIN colonias col ON cli.colonia = col.colonia
            LEFT JOIN localidades loc ON loc.localidad = col.localidad
            LEFT JOIN municipios mun ON loc.municipio = mun.municipio
            LEFT JOIN cregimenfiscal rf ON cli.regimenfiscal = rf.regimenfiscal
            LEFT JOIN telefonosclientes tel ON cli.cliente = tel.cliente AND tel.tipo = 'PRINCIPAL'
            WHERE %s cli.nomnegocio LIKE ?
            ORDER BY cli.nomnegocio, REPLACE(cli.rsocial, '\"', '') 
            LIMIT ?
            """, condicionActivos);
            
        logger.debug("Ejecutando búsqueda de clientes por nombre de negocio: {}", nombreNegocio);
        
        return jdbcClient.sql(sql)
                .param("%" + nombreNegocio + "%")
                .param(NUM_LIMITE_RESULTADOS_BUSQ)
                .query(this::mapToClienteResultado)
                .list();
    }
    
    /**
     * Busca clientes por clave de cliente
     */
    public List<ClienteResultado> buscarPorClave(String clave, String condicionActivos) {
        String sql = String.format("""
            SELECT cli.cliente, cli.rsocial, cli.nomnegocio, cli.rfc, 
                   CONCAT(COALESCE(rf.regimenfiscal, ''), ' - ', COALESCE(rf.descripcion, '')) AS regimenfiscal,
                   cli.contacto, mun.nombre as municipio, loc.nombre as localidad,
                   cli.calle, col.nombre as colonia, cli.cp,
                   COALESCE(tel.telefono, '') as telefono, cli.email, cli.email2, cli.activo
            FROM clientes cli
            LEFT JOIN colonias col ON cli.colonia = col.colonia
            LEFT JOIN localidades loc ON loc.localidad = col.localidad
            LEFT JOIN municipios mun ON loc.municipio = mun.municipio
            LEFT JOIN cregimenfiscal rf ON cli.regimenfiscal = rf.regimenfiscal
            LEFT JOIN telefonosclientes tel ON cli.cliente = tel.cliente AND tel.tipo = 'PRINCIPAL'
            WHERE %s cli.cliente LIKE ?
            ORDER BY cli.cliente, REPLACE(cli.rsocial, '\"', '') 
            LIMIT ?
            """, condicionActivos);
            
        logger.debug("Ejecutando búsqueda de clientes por clave: {}", clave);
        
        return jdbcClient.sql(sql)
                .param("%" + clave + "%")
                .param(NUM_LIMITE_RESULTADOS_BUSQ)
                .query(this::mapToClienteResultado)
                .list();
    }
    
    /**
     * Busca clientes por contacto
     */
    public List<ClienteResultado> buscarPorContacto(String contacto, String condicionActivos) {
        String sql = String.format("""
            SELECT cli.cliente, cli.rsocial, cli.nomnegocio, cli.rfc, 
                   CONCAT(COALESCE(rf.regimenfiscal, ''), ' - ', COALESCE(rf.descripcion, '')) AS regimenfiscal,
                   cli.contacto, mun.nombre as municipio, loc.nombre as localidad,
                   cli.calle, col.nombre as colonia, cli.cp,
                   COALESCE(tel.telefono, '') as telefono, cli.email, cli.email2, cli.activo
            FROM clientes cli
            LEFT JOIN colonias col ON cli.colonia = col.colonia
            LEFT JOIN localidades loc ON loc.localidad = col.localidad
            LEFT JOIN municipios mun ON loc.municipio = mun.municipio
            LEFT JOIN cregimenfiscal rf ON cli.regimenfiscal = rf.regimenfiscal
            LEFT JOIN telefonosclientes tel ON cli.cliente = tel.cliente AND tel.tipo = 'PRINCIPAL'
            WHERE %s cli.contacto LIKE ?
            ORDER BY REPLACE(cli.rsocial, '\"', '') 
            LIMIT ?
            """, condicionActivos);
            
        logger.debug("Ejecutando búsqueda de clientes por contacto: {}", contacto);
        
        return jdbcClient.sql(sql)
                .param("%" + contacto + "%")
                .param(NUM_LIMITE_RESULTADOS_BUSQ)
                .query(this::mapToClienteResultado)
                .list();
    }
    
    /**
     * Busca clientes por email
     */
    public List<ClienteResultado> buscarPorEmail(String email, String condicionActivos) {
        String sql = String.format("""
            SELECT cli.cliente, cli.rsocial, cli.nomnegocio, cli.rfc, 
                   CONCAT(COALESCE(rf.regimenfiscal, ''), ' - ', COALESCE(rf.descripcion, '')) AS regimenfiscal,
                   cli.contacto, mun.nombre as municipio, loc.nombre as localidad,
                   cli.calle, col.nombre as colonia, cli.cp,
                   COALESCE(tel.telefono, '') as telefono, cli.email, cli.email2, cli.activo
            FROM clientes cli
            LEFT JOIN colonias col ON cli.colonia = col.colonia
            LEFT JOIN localidades loc ON loc.localidad = col.localidad
            LEFT JOIN municipios mun ON loc.municipio = mun.municipio
            LEFT JOIN cregimenfiscal rf ON cli.regimenfiscal = rf.regimenfiscal
            LEFT JOIN telefonosclientes tel ON cli.cliente = tel.cliente AND tel.tipo = 'PRINCIPAL'
            WHERE %s cli.email LIKE ? OR cli.email2 LIKE ?
            ORDER BY REPLACE(cli.rsocial, '\"', '') 
            LIMIT ?
            """, condicionActivos);
            
        logger.debug("Ejecutando búsqueda de clientes por email: {}", email);
        
        return jdbcClient.sql(sql)
                .param("%" + email + "%")
                .param("%" + email + "%")
                .param(NUM_LIMITE_RESULTADOS_BUSQ)
                .query(this::mapToClienteResultado)
                .list();
    }
    
    /**
     * Mapea un ResultSet a ClienteResultado
     */
    private ClienteResultado mapToClienteResultado(ResultSet rs, int rowNum) throws SQLException {
        return ClienteResultado.builder()
                .cliente(rs.getString("cliente"))
                .rsocial(rs.getString("rsocial"))
                .nomnegocio(rs.getString("nomnegocio"))
                .rfc(rs.getString("rfc"))
                .regimenfiscal(rs.getString("regimenfiscal"))
                .contacto(rs.getString("contacto"))
                .municipio(rs.getString("municipio"))
                .localidad(rs.getString("localidad"))
                .calle(rs.getString("calle"))
                .colonia(rs.getString("colonia"))
                .cp(rs.getString("cp"))
                .telefono(rs.getString("telefono"))
                .email(rs.getString("email"))
                .email2(rs.getString("email2"))
                .activo(rs.getBoolean("activo"))
                .build();
    }
}
