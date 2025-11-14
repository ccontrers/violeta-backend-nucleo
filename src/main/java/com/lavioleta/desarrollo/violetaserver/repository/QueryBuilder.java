package com.lavioleta.desarrollo.violetaserver.repository;

import org.springframework.util.StringUtils;
import java.util.ArrayList;
import java.util.List;

/**
 * Query Builder seguro para evitar inyección SQL
 */
public class QueryBuilder {
    
    private StringBuilder selectClause = new StringBuilder();
    private StringBuilder fromClause = new StringBuilder();
    private List<String> whereConditions = new ArrayList<>();
    private StringBuilder joinClause = new StringBuilder();
    private StringBuilder orderClause = new StringBuilder();
    private String limitClause = "";
    
    public static QueryBuilder select(String columns) {
        QueryBuilder builder = new QueryBuilder();
        builder.selectClause.append("SELECT ").append(columns);
        return builder;
    }
    
    public QueryBuilder from(String table) {
        this.fromClause.append("FROM ").append(table);
        return this;
    }
    
    public QueryBuilder addTable(String table) {
        if (this.fromClause.length() > 0) {
            this.fromClause.append(", ").append(table);
        }
        return this;
    }
    
    public QueryBuilder where(String condition) {
        if (StringUtils.hasText(condition)) {
            this.whereConditions.add(condition);
        }
        return this;
    }
    
    public QueryBuilder whereActivo(boolean soloActivos) {
        if (soloActivos) {
            this.whereConditions.add("a.activo = 1");
        }
        return this;
    }
    
    public QueryBuilder join(String joinClause) {
        if (StringUtils.hasText(joinClause)) {
            this.joinClause.append(" ").append(joinClause);
        }
        return this;
    }
    
    public QueryBuilder orderBy(String orderBy) {
        this.orderClause.append("ORDER BY ").append(orderBy);
        return this;
    }
    
    public QueryBuilder limit(String limit) {
        this.limitClause = "LIMIT " + limit;
        return this;
    }
    
    public String build() {
        StringBuilder sql = new StringBuilder();
        
        // SELECT
        sql.append(selectClause).append(" ");
        
        // FROM
        sql.append(fromClause);
        
        // JOIN
        if (joinClause.length() > 0) {
            sql.append(joinClause);
        }
        
        // WHERE
        if (!whereConditions.isEmpty()) {
            sql.append(" WHERE ");
            sql.append(String.join(" AND ", whereConditions));
        }
        
        // ORDER BY
        if (orderClause.length() > 0) {
            sql.append(" ").append(orderClause);
        }
        
        // LIMIT
        if (StringUtils.hasText(limitClause)) {
            sql.append(" ").append(limitClause);
        }
        
        return sql.toString();
    }
    
    /**
     * Query específico para búsqueda con existencias
     */
    public static class ExistenciasQueryBuilder {
        
        public static String buildSelectClause() {
            return """
                p.nombre, a.present, a.multiplo, a.factor, p.marca, m.nombre as nommarca, 
                p.producto, a.articulo, a.ean13, a.activo
                """;
        }
        
        public static String buildFromClause(boolean incluirExistencias) {
            StringBuilder from = new StringBuilder("articulos a, productos p, marcas m");
            if (incluirExistencias) {
                from.append(", existenciasactuales e, secciones s, almacenes al");
            }
            return from.toString();
        }
        
        public static String buildJoinClause(boolean incluirExistencias, String sucursal) {
            if (!incluirExistencias) {
                return "";
            }
            return String.format("""
                INNER JOIN secciones s ON s.sucursal = '%s' 
                INNER JOIN almacenes al ON s.seccion = al.seccion 
                INNER JOIN existenciasactuales e ON e.producto = a.producto 
                    AND e.present = a.present AND al.almacen = e.almacen AND e.cantidad > 0
                """, sucursal);
        }
        
        public static List<String> buildBaseConditions() {
            List<String> conditions = new ArrayList<>();
            conditions.add("p.producto = a.producto");
            conditions.add("p.marca = m.marca");
            return conditions;
        }
        
        public static List<String> buildExistenciasConditions(boolean incluirExistencias, String sucursal) {
            List<String> conditions = new ArrayList<>();
            if (incluirExistencias) {
                conditions.add("e.producto = a.producto");
                conditions.add("e.present = a.present");
                conditions.add(String.format("s.sucursal = '%s'", sucursal));
                conditions.add("s.seccion = al.seccion");
                conditions.add("al.almacen = e.almacen");
                conditions.add("e.cantidad > 0");
            }
            return conditions;
        }
    }
}
