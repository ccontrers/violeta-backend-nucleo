package com.lavioleta.desarrollo.violetaserver.repository;

import com.lavioleta.desarrollo.violetaserver.dto.response.BusquedaArticulosResponse.*;
import com.lavioleta.desarrollo.violetaserver.dto.response.PrecioArticulo;
import com.lavioleta.desarrollo.violetaserver.dto.response.ExistenciaArticulo;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.jdbc.core.simple.JdbcClient;
import org.springframework.stereotype.Repository;

import java.util.List;

@Repository
public class BusquedaArticulosRepository {
    
    private static final Logger logger = LoggerFactory.getLogger(BusquedaArticulosRepository.class);
    private final JdbcClient jdbcClient;
    private static final int NUM_LIMITE_RESULTADOS_BUSQ = 501;
    
    public BusquedaArticulosRepository(JdbcClient jdbcClient) {
        this.jdbcClient = jdbcClient;
    }
    
    public boolean verificarMostrarExistencias(String sucursal) {
        String sql = "SELECT valor FROM parametrosemp WHERE parametro='EXISTENCIASPV' AND sucursal = ?";
        logger.debug("Verificando parámetro de existencias para sucursal: {}", sucursal);
        
        try {
            return jdbcClient.sql(sql)
                    .param(sucursal)
                    .query(String.class)
                    .optional()
                    .map(valor -> !"0".equals(valor))
                    .orElse(false);
        } catch (Exception e) {
            logger.error("Error al verificar parámetro de existencias: {}", e.getMessage());
            return false;
        }
    }
    
    public List<ArticuloResultado> buscarPorNombre(String nombre, String condicionActivos, String tablaExistencias, String condicionExistencias) {
        // Determinar si incluir existencias
        boolean incluirExistencias = !tablaExistencias.isEmpty();
        boolean soloActivos = "a.activo=1 and".equals(condicionActivos);
        
        // Construir query seguro usando QueryBuilder
        QueryBuilder queryBuilder = QueryBuilder
            .select("p.nombre, a.present, a.multiplo, a.factor, p.marca, m.nombre as nommarca, p.producto, a.articulo, a.ean13, a.activo")
            .from("articulos a, productos p, marcas m" + (incluirExistencias ? ", existenciasactuales e, secciones s, almacenes al" : ""))
            .where("CONCAT(p.nombre,'%',a.present) LIKE ?")
            .where("p.producto = a.producto")
            .where("p.marca = m.marca");
        
        // Agregar condición de activos si es necesaria
        queryBuilder.whereActivo(soloActivos);
        
        // Agregar condiciones de existencias si es necesario
        if (incluirExistencias) {
            String sucursal = extractSucursalFromCondition(condicionExistencias);
            queryBuilder
                .where("e.producto = a.producto")
                .where("e.present = a.present")
                .where("s.sucursal = '" + sucursal + "'")
                .where("s.seccion = al.seccion")
                .where("al.almacen = e.almacen")
                .where("e.cantidad > 0");
        }
        
        // Ordenamiento y límite
        String sql = queryBuilder
            .orderBy("p.nombre, a.present, a.factor DESC")
            .limit("?")
            .build();
        
        logger.debug("Ejecutando búsqueda por nombre: {} - SQL: {}", nombre, sql);
        
        return jdbcClient.sql(sql)
                .param("%" + nombre + "%")
                .param(NUM_LIMITE_RESULTADOS_BUSQ)
                .query((rs, rowNum) -> new ArticuloResultado(
                    rs.getString("nombre"),
                    rs.getString("present"),
                    rs.getString("multiplo"),
                    rs.getObject("factor", Double.class),
                    rs.getString("marca"),
                    rs.getString("nommarca"),
                    rs.getString("producto"),
                    rs.getString("articulo"),
                    rs.getString("ean13"),
                    rs.getObject("activo", Integer.class)
                ))
                .list();
    }
    
    /**
     * Extrae la sucursal de la condición de existencias de forma segura
     */
    private String extractSucursalFromCondition(String condicionExistencias) {
        if (condicionExistencias == null || condicionExistencias.isEmpty()) {
            return "1"; // Valor por defecto
        }
        // Extracción segura usando regex para sucursal
        java.util.regex.Pattern pattern = java.util.regex.Pattern.compile("s\\.sucursal = '([^']+)'");
        java.util.regex.Matcher matcher = pattern.matcher(condicionExistencias);
        if (matcher.find()) {
            return matcher.group(1);
        }
        return "1"; // Valor por defecto
    }
    
    public List<ArticuloResultado> buscarPorClave(String clave, String condicionActivos, String tablaExistencias, String condicionExistencias) {
        // Determinar si incluir existencias
        boolean incluirExistencias = !tablaExistencias.isEmpty();
        boolean soloActivos = "a.activo=1 and".equals(condicionActivos);
        
        // Construir query seguro usando QueryBuilder
        QueryBuilder queryBuilder = QueryBuilder
            .select("p.nombre, a.present, a.multiplo, a.factor, p.marca, m.nombre as nommarca, p.producto, a.articulo, a.ean13, a.activo")
            .from("articulos a, productos p, marcas m" + (incluirExistencias ? ", existenciasactuales e, secciones s, almacenes al" : ""))
            .where("p.producto LIKE ?")
            .where("p.producto = a.producto")
            .where("p.marca = m.marca");
        
        // Agregar condición de activos si es necesaria
        queryBuilder.whereActivo(soloActivos);
        
        // Agregar condiciones de existencias si es necesario
        if (incluirExistencias) {
            String sucursal = extractSucursalFromCondition(condicionExistencias);
            queryBuilder
                .where("e.producto = a.producto")
                .where("e.present = a.present")
                .where("s.sucursal = '" + sucursal + "'")
                .where("s.seccion = al.seccion")
                .where("al.almacen = e.almacen")
                .where("e.cantidad > 0");
        }
        
        // Ordenamiento y límite
        String sql = queryBuilder
            .orderBy("p.nombre, a.present, a.factor DESC")
            .limit("?")
            .build();
        
        logger.debug("Ejecutando búsqueda por clave: {} - SQL: {}", clave, sql);
        
        return jdbcClient.sql(sql)
                .param(clave + "%")
                .param(NUM_LIMITE_RESULTADOS_BUSQ)
                .query((rs, rowNum) -> new ArticuloResultado(
                    rs.getString("nombre"),
                    rs.getString("present"),
                    rs.getString("multiplo"),
                    rs.getObject("factor", Double.class),
                    rs.getString("marca"),
                    rs.getString("nommarca"),
                    rs.getString("producto"),
                    rs.getString("articulo"),
                    rs.getString("ean13"),
                    rs.getObject("activo", Integer.class)
                ))
                .list();
    }
    
    public List<ArticuloResultado> buscarPorMarca(String marca, String condicionActivos, String tablaExistencias, String condicionExistencias) {
        // Determinar si incluir existencias
        boolean incluirExistencias = !tablaExistencias.isEmpty();
        boolean soloActivos = "a.activo=1 and".equals(condicionActivos);
        
        // Construir query seguro usando QueryBuilder
        QueryBuilder queryBuilder = QueryBuilder
            .select("p.nombre, a.present, a.multiplo, a.factor, p.marca, m.nombre as nommarca, p.producto, a.articulo, a.ean13, a.activo")
            .from("articulos a, productos p, marcas m" + (incluirExistencias ? ", existenciasactuales e, secciones s, almacenes al" : ""))
            .where("p.marca = ?")
            .where("p.producto = a.producto")
            .where("p.marca = m.marca");
        
        // Agregar condición de activos si es necesaria
        queryBuilder.whereActivo(soloActivos);
        
        // Agregar condiciones de existencias si es necesario
        if (incluirExistencias) {
            String sucursal = extractSucursalFromCondition(condicionExistencias);
            queryBuilder
                .where("e.producto = a.producto")
                .where("e.present = a.present")
                .where("s.sucursal = '" + sucursal + "'")
                .where("s.seccion = al.seccion")
                .where("al.almacen = e.almacen")
                .where("e.cantidad > 0");
        }
        
        // Ordenamiento y límite
        String sql = queryBuilder
            .orderBy("p.nombre, a.present, a.factor DESC")
            .limit("?")
            .build();
        
        logger.debug("Ejecutando búsqueda por marca: {} - SQL: {}", marca, sql);
        
        return jdbcClient.sql(sql)
                .param(marca)
                .param(NUM_LIMITE_RESULTADOS_BUSQ)
                .query((rs, rowNum) -> new ArticuloResultado(
                    rs.getString("nombre"),
                    rs.getString("present"),
                    rs.getString("multiplo"),
                    rs.getObject("factor", Double.class),
                    rs.getString("marca"),
                    rs.getString("nommarca"),
                    rs.getString("producto"),
                    rs.getString("articulo"),
                    rs.getString("ean13"),
                    rs.getObject("activo", Integer.class)
                ))
                .list();
    }
    
    public List<ArticuloResultado> buscarPorClasificacion(String clasif, String condicionActivos, String tablaExistencias, String condicionExistencias) {
        // Determinar si incluir existencias
        boolean incluirExistencias = !tablaExistencias.isEmpty();
        boolean soloActivos = "a.activo=1 and".equals(condicionActivos);
        
        // Construir query seguro usando QueryBuilder
        QueryBuilder queryBuilder = QueryBuilder
            .select("p.nombre, a.present, a.multiplo, a.factor, p.marca, m.nombre as nommarca, p.producto, a.articulo, a.ean13, a.activo")
            .from("articulos a, productos p, marcas m" + (incluirExistencias ? ", existenciasactuales e, secciones s, almacenes al" : ""))
            .where("p.clasif1 = ?")
            .where("p.producto = a.producto")
            .where("p.marca = m.marca");
        
        // Agregar condición de activos si es necesaria
        queryBuilder.whereActivo(soloActivos);
        
        // Agregar condiciones de existencias si es necesario
        if (incluirExistencias) {
            String sucursal = extractSucursalFromCondition(condicionExistencias);
            queryBuilder
                .where("e.producto = a.producto")
                .where("e.present = a.present")
                .where("s.sucursal = '" + sucursal + "'")
                .where("s.seccion = al.seccion")
                .where("al.almacen = e.almacen")
                .where("e.cantidad > 0");
        }
        
        // Ordenamiento y límite
        String sql = queryBuilder
            .orderBy("p.nombre, a.present, a.factor DESC")
            .limit("?")
            .build();
        
        logger.debug("Ejecutando búsqueda por clasificación: {} - SQL: {}", clasif, sql);
        
        return jdbcClient.sql(sql)
                .param(clasif)
                .param(NUM_LIMITE_RESULTADOS_BUSQ)
                .query((rs, rowNum) -> new ArticuloResultado(
                    rs.getString("nombre"),
                    rs.getString("present"),
                    rs.getString("multiplo"),
                    parseDoubleSafely(rs.getObject("factor")),
                    rs.getString("marca"),
                    rs.getString("nommarca"),
                    rs.getString("producto"),
                    rs.getString("articulo"),
                    rs.getString("ean13"),
                    parseIntSafely(rs.getObject("activo"))
                ))
                .list();
    }
    
    public List<ArticuloResultado> buscarPorCodigoBarras(String codigo, String condicionActivos, String joinExistencias) {
        // Determinar si incluir existencias
        boolean incluirExistencias = !joinExistencias.trim().isEmpty();
        boolean soloActivos = "a.activo=1 and".equals(condicionActivos);
        
        // Para código de barras, usamos una estructura diferente con JOINs explícitos
        StringBuilder queryBuilder = new StringBuilder();
        queryBuilder.append("SELECT p.nombre, CONCAT(a.present,' ',IFNULL(cb.descripcion,'')) as present, ");
        queryBuilder.append("a.multiplo, a.factor, p.marca, m.nombre AS nommarca, p.producto, ");
        queryBuilder.append("a.articulo, a.ean13, a.activo ");
        queryBuilder.append("FROM articulos a ");
        queryBuilder.append("INNER JOIN productos p ON p.producto = a.producto ");
        queryBuilder.append("INNER JOIN marcas m ON p.marca = m.marca ");
        queryBuilder.append("LEFT JOIN codigosbarras cb ON cb.articulo = a.articulo");
        
        // Agregar JOIN de existencias si es necesario
        if (incluirExistencias) {
            queryBuilder.append(joinExistencias);
        }
        
        queryBuilder.append(" WHERE (a.ean13 = ? OR cb.codigobarras = ?) ");
        
        // Agregar condición de activos si es necesaria
        if (soloActivos) {
            queryBuilder.append("AND a.activo = 1 ");
        }
        
        queryBuilder.append("ORDER BY p.producto, a.present, a.factor DESC ");
        queryBuilder.append("LIMIT ?");
        
        String sql = queryBuilder.toString();
        
        logger.debug("Ejecutando búsqueda por código de barras: {} - SQL: {}", codigo, sql);
        
        return jdbcClient.sql(sql)
                .param(codigo)
                .param(codigo)
                .param(NUM_LIMITE_RESULTADOS_BUSQ)
                .query((rs, rowNum) -> new ArticuloResultado(
                    rs.getString("nombre"),
                    rs.getString("present"),
                    rs.getString("multiplo"),
                    rs.getObject("factor", Double.class),
                    rs.getString("marca"),
                    rs.getString("nommarca"),
                    rs.getString("producto"),
                    rs.getString("articulo"),
                    rs.getString("ean13"),
                    rs.getObject("activo", Integer.class)
                ))
                .list();
    }
    
    public List<ArticuloResultado> buscarPorArticulo(String articulo, String condicionActivos, String tablaExistencias, String condicionExistencias) {
        // Determinar si incluir existencias
        boolean incluirExistencias = !tablaExistencias.isEmpty();
        boolean soloActivos = "a.activo=1 and".equals(condicionActivos);
        
        // Construir query seguro usando QueryBuilder
        QueryBuilder queryBuilder = QueryBuilder
            .select("p.nombre, a.present, a.multiplo, a.factor, p.marca, m.nombre as nommarca, p.producto, a.articulo, a.ean13, a.activo")
            .from("articulos a, productos p, marcas m" + (incluirExistencias ? ", existenciasactuales e, secciones s, almacenes al" : ""))
            .where("a.articulo = ?")
            .where("p.producto = a.producto")
            .where("p.marca = m.marca");
        
        // Agregar condición de activos si es necesaria
        queryBuilder.whereActivo(soloActivos);
        
        // Agregar condiciones de existencias si es necesario
        if (incluirExistencias) {
            String sucursal = extractSucursalFromCondition(condicionExistencias);
            queryBuilder
                .where("e.producto = a.producto")
                .where("e.present = a.present")
                .where("s.sucursal = '" + sucursal + "'")
                .where("s.seccion = al.seccion")
                .where("al.almacen = e.almacen")
                .where("e.cantidad > 0");
        }
        
        // Ordenamiento y límite
        String sql = queryBuilder
            .orderBy("p.nombre, a.present, a.factor DESC")
            .limit("?")
            .build();
        
        logger.debug("Ejecutando búsqueda por artículo: {} - SQL: {}", articulo, sql);
        
        return jdbcClient.sql(sql)
                .param(articulo)
                .param(NUM_LIMITE_RESULTADOS_BUSQ)
                .query((rs, rowNum) -> new ArticuloResultado(
                    rs.getString("nombre"),
                    rs.getString("present"),
                    rs.getString("multiplo"),
                    rs.getObject("factor", Double.class),
                    rs.getString("marca"),
                    rs.getString("nommarca"),
                    rs.getString("producto"),
                    rs.getString("articulo"),
                    rs.getString("ean13"),
                    rs.getObject("activo", Integer.class)
                ))
                .list();
    }
    
    public String obtenerCodigoClasificacion(String nombreClasificacion) {
        String sql = "SELECT clasif1 FROM clasificacion1 WHERE UPPER(nombre) = UPPER(?)";
        logger.debug("Buscando código de clasificación para: {}", nombreClasificacion);
        
        try {
            return jdbcClient.sql(sql)
                    .param(nombreClasificacion)
                    .query(String.class)
                    .single();
        } catch (Exception e) {
            logger.warn("No se encontró clasificación con nombre: {}", nombreClasificacion);
            return null;
        }
    }
    
    public List<ClasificacionResultado> obtenerClasificaciones() {
        String sql = "SELECT clasif1, nombre FROM clasificacion1 ORDER BY nombre";
        logger.debug("Obteniendo clasificaciones");
        
        return jdbcClient.sql(sql)
                .query((rs, rowNum) -> new ClasificacionResultado(
                    rs.getString("clasif1"),
                    rs.getString("nombre")
                ))
                .list();
    }
    
    public List<MarcaResultado> obtenerMarcas() {
        String sql = "SELECT marca, nombre FROM marcas ORDER BY nombre";
        logger.debug("Obteniendo marcas");
        
        return jdbcClient.sql(sql)
                .query((rs, rowNum) -> new MarcaResultado(
                    rs.getString("marca"),
                    rs.getString("nombre")
                ))
                .list();
    }
    
    private Integer parseIntSafely(Object value) {
        if (value == null) return null;
        if (value instanceof Integer) return (Integer) value;
        if (value instanceof Number) return ((Number) value).intValue();
        try {
            return Integer.parseInt(value.toString());
        } catch (NumberFormatException e) {
            logger.warn("No se pudo convertir a Integer: {}", value);
            return 1; // valor por defecto
        }
    }
    
    private Double parseDoubleSafely(Object value) {
        if (value == null) return null;
        if (value instanceof Double) return (Double) value;
        if (value instanceof Number) return ((Number) value).doubleValue();
        try {
            return Double.parseDouble(value.toString());
        } catch (NumberFormatException e) {
            logger.warn("No se pudo convertir a Double: {}", value);
            return 1.0; // valor por defecto
        }
    }
    
    /**
     * Obtener precios de un artículo específico
     */
    public List<PrecioArticulo> obtenerPreciosArticulo(String articulo, String sucursal) {
        String sql = """
            SELECT p.tipoprec, p.precio 
            FROM precios p  
            INNER JOIN sucursales s ON s.sucursal = ? 
            INNER JOIN tiposdeprecios tp ON tp.idempresa = s.idempresa AND p.tipoprec = tp.tipoprec
            WHERE p.articulo = ?
            """;
        
        logger.debug("Obteniendo precios para artículo: {} en sucursal: {}", articulo, sucursal);
        
        return jdbcClient.sql(sql)
                .param(sucursal)
                .param(articulo)
                .query((rs, rowNum) -> new PrecioArticulo(
                    rs.getString("tipoprec"),
                    parseDoubleSafely(rs.getObject("precio"))
                ))
                .list();
    }
    
    /**
     * Obtener existencia de un artículo específico
     */
    public ExistenciaArticulo obtenerExistenciaArticulo(String articulo, String sucursal) {
        String sql = """
            SELECT e.cantidad, e.costo
            FROM existencias e
            WHERE e.articulo = ? AND e.sucursal = ?
            """;
        
        logger.debug("Obteniendo existencia para artículo: {} en sucursal: {}", articulo, sucursal);
        
        try {
            return jdbcClient.sql(sql)
                    .param(articulo)
                    .param(sucursal)
                    .query((rs, rowNum) -> new ExistenciaArticulo(
                        parseDoubleSafely(rs.getObject("cantidad")),
                        parseDoubleSafely(rs.getObject("costo"))
                    ))
                    .optional()
                    .orElse(new ExistenciaArticulo(0.0, 0.0));
        } catch (Exception e) {
            logger.warn("Error al obtener existencia para artículo {}: {}", articulo, e.getMessage());
            return new ExistenciaArticulo(0.0, 0.0);
        }
    }
}
