package com.lavioleta.desarrollo.violetaserver.service.impl;

import com.lavioleta.desarrollo.violetaserver.service.CatalogosService;
import org.springframework.jdbc.core.simple.JdbcClient;
import org.springframework.stereotype.Service;
import java.util.List;
import java.util.Map;

/**
 * Implementación del servicio de catálogos auxiliares
 */
@Service
public class CatalogosServiceImpl implements CatalogosService {
    
    private final JdbcClient jdbcClient;
    
    public CatalogosServiceImpl(JdbcClient jdbcClient) {
        this.jdbcClient = jdbcClient;
    }
    
    @Override
    public List<Map<String, Object>> obtenerColonias() {
        String sql = """
            SELECT colonia, nombre, localidad, sector 
            FROM colonias 
            ORDER BY nombre 
            LIMIT 500
            """;
        
        return jdbcClient.sql(sql)
                .query()
                .listOfRows();
    }
    
    @Override
    public List<Map<String, Object>> buscarColoniasPorNombre(String nombre) {
        String sql = """
            SELECT colonia, nombre, localidad, sector 
            FROM colonias 
            WHERE nombre LIKE ? 
            ORDER BY nombre 
            LIMIT 100
            """;
        
        return jdbcClient.sql(sql)
                .param("%" + nombre + "%")
                .query()
                .listOfRows();
    }
    
    @Override
    public List<Map<String, Object>> obtenerCanalesClientes() {
        String sql = """
            SELECT canal, descripcion 
            FROM canalesclientes 
            ORDER BY descripcion
            """;
        
        return jdbcClient.sql(sql)
                .query()
                .listOfRows();
    }
    
    @Override
    public List<Map<String, Object>> obtenerGirosNegocio() {
        String sql = """
            SELECT giro, nombre 
            FROM gironegocio 
            ORDER BY nombre
            """;
        
        return jdbcClient.sql(sql)
                .query()
                .listOfRows();
    }
    
    @Override
    public List<Map<String, Object>> obtenerRegimenesFiscales() {
        String sql = """
            SELECT regimenfiscal, descripcion, esfisica, esmoral, activo 
            FROM cregimenfiscal 
            WHERE activo = 1 
            ORDER BY descripcion
            """;
        
        return jdbcClient.sql(sql)
                .query()
                .listOfRows();
    }
    
    @Override
    public List<Map<String, Object>> obtenerSociedadesMercantiles() {
        String sql = """
            SELECT id, nombre, descripcion 
            FROM catsociedadesmercantiles 
            WHERE activo = 1 
            ORDER BY descripcion
            """;
        
        return jdbcClient.sql(sql)
                .query()
                .listOfRows();
    }
    
    
    @Override
    public List<Map<String, Object>> obtenerUsosCfdi() {
        String sql = """
            SELECT usocfdi, descripcion, pfisica, pmoral 
            FROM cusocfdi 
            ORDER BY descripcion
            """;
        
        return jdbcClient.sql(sql)
                .query()
                .listOfRows();
    }
    
    @Override
    public List<Map<String, Object>> obtenerTiposPrecios() {
        String sql = """
            SELECT tipoprec, descripcion 
            FROM tiposdeprecios 
            ORDER BY tipoprec ASC
            """;
        
        return jdbcClient.sql(sql)
                .query()
                .listOfRows();
    }
    
    @Override
    public List<Map<String, Object>> obtenerTiposPreciosPorEmpresa(Integer idEmpresa) {
        String sql = """
            SELECT tipoprec, descripcion 
            FROM tiposdeprecios 
            WHERE idempresa = ? OR idempresa IS NULL
            ORDER BY tipoprec ASC
            """;
        
        return jdbcClient.sql(sql)
                .param(idEmpresa)
                .query()
                .listOfRows();
    }
    
    @Override
    public List<Map<String, Object>> obtenerTiposCuentasBancarias() {
        String sql = """
            SELECT clave, descripcion, caracteres, bancorel 
            FROM tiposcuentasbancarias 
            WHERE activo = 1
            ORDER BY descripcion
            """;
        
        return jdbcClient.sql(sql)
                .query()
                .listOfRows();
    }
    
    @Override
    public List<Map<String, Object>> obtenerBancos() {
        String sql = """
            SELECT banco, nombre 
            FROM bancos 
            ORDER BY nombre, banco
            """;
        
        return jdbcClient.sql(sql)
                .query()
                .listOfRows();
    }
    
    /*
    // Comentado temporalmente - tablas no disponibles
    
    @Override
    public List<Map<String, Object>> obtenerCategoriasClientes() {
        String sql = """
            SELECT categoria, descripcion 
            FROM ccategoriaclientes 
            WHERE activo = 1
            ORDER BY descripcion
            """;
        
        return jdbcClient.sql(sql)
                .query()
                .listOfRows();
    }
    
    @Override
    public List<Map<String, Object>> obtenerTiposClientes() {
        String sql = """
            SELECT tipocli, descripcion 
            FROM ctiposclientes 
            WHERE activo = 1
            ORDER BY descripcion
            """;
        
        return jdbcClient.sql(sql)
                .query()
                .listOfRows();
    }
    
    @Override
    public List<Map<String, Object>> obtenerClasificacionesClientes() {
        String sql = """
            SELECT clasificacion, descripcion 
            FROM cclasificacionclientes 
            WHERE activo = 1
            ORDER BY descripcion
            """;
        
        return jdbcClient.sql(sql)
                .query()
                .listOfRows();
    }
    
    @Override
    public List<Map<String, Object>> obtenerSubclasificacionesClientes() {
        String sql = """
            SELECT subclasificacion, descripcion 
            FROM csubclasificacionclientes 
            WHERE activo = 1
            ORDER BY descripcion
            """;
        
        return jdbcClient.sql(sql)
                .query()
                .listOfRows();
    }
    
    @Override
    public List<Map<String, Object>> obtenerNacionalidades() {
        String sql = """
            SELECT nacionalidad, descripcion 
            FROM cnacionalidades 
            WHERE activo = 1
            ORDER BY descripcion
            """;
        
        return jdbcClient.sql(sql)
                .query()
                .listOfRows();
    }
    
    @Override
    public List<Map<String, Object>> obtenerNacionalidades() {
        String sql = """
            SELECT nacionalidad, descripcion 
            FROM cnacionalidades 
            WHERE activo = 1
            ORDER BY descripcion
            """;
        
        return jdbcClient.sql(sql)
                .query()
                .listOfRows();
    }
    
    /*
    @Override
    public List<Map<String, Object>> obtenerRegimenesMatrimoniales() {
        String sql = """
            SELECT regimenmatrimonial, descripcion 
            FROM cregimenmatrimonial 
            WHERE activo = 1
            ORDER BY descripcion
            """;
        
        return jdbcClient.sql(sql)
                .query()
                .listOfRows();
    }
    */
}

