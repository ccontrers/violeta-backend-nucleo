package com.lavioleta.desarrollo.violetaserver.repository;

import com.lavioleta.desarrollo.violetaserver.entity.CondicionComercialProveedor;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Query;
import org.springframework.data.repository.query.Param;
import org.springframework.stereotype.Repository;

import java.time.LocalDate;
import java.util.List;

/**
 * Repositorio para la entidad CondicionComercialProveedor
 */
@Repository
public interface CondicionComercialProveedorRepository extends JpaRepository<CondicionComercialProveedor, Integer> {
    
    /**
     * Busca todas las condiciones comerciales de un proveedor
     */
    List<CondicionComercialProveedor> findByProveedorIdOrderByFechaaltaDesc(String proveedorId);
    
    /**
     * Busca condiciones comerciales activas de un proveedor
     */
    List<CondicionComercialProveedor> findByProveedorIdAndActivoTrueOrderByFechaaltaDesc(String proveedorId);
    
    /**
     * Busca condiciones comerciales vigentes en una fecha específica
     */
    @Query("SELECT c FROM CondicionComercialProveedor c WHERE c.proveedorId = :proveedorId AND c.activo = true AND " +
           "(c.fechainicio IS NULL OR c.fechainicio <= :fecha) AND " +
           "(c.fechafin IS NULL OR c.fechafin >= :fecha) " +
           "ORDER BY c.fechaalta DESC")
    List<CondicionComercialProveedor> findVigentesByProveedorIdAndFecha(
        @Param("proveedorId") String proveedorId,
        @Param("fecha") LocalDate fecha
    );
    
    /**
     * Busca condiciones comerciales vigentes hoy
     */
    @Query("SELECT c FROM CondicionComercialProveedor c WHERE c.proveedorId = :proveedorId AND c.activo = true AND " +
           "(c.fechainicio IS NULL OR c.fechainicio <= CURRENT_DATE) AND " +
           "(c.fechafin IS NULL OR c.fechafin >= CURRENT_DATE) " +
           "ORDER BY c.fechaalta DESC")
    List<CondicionComercialProveedor> findVigentesByProveedorId(@Param("proveedorId") String proveedorId);
    
    /**
     * Busca condiciones por rango de descuento
     */
    @Query("SELECT c FROM CondicionComercialProveedor c WHERE c.proveedorId = :proveedorId AND c.activo = true AND " +
           "c.descuento BETWEEN :descuentoMin AND :descuentoMax " +
           "ORDER BY c.descuento DESC")
    List<CondicionComercialProveedor> findByProveedorIdAndDescuentoBetween(
        @Param("proveedorId") String proveedorId,
        @Param("descuentoMin") java.math.BigDecimal descuentoMin,
        @Param("descuentoMax") java.math.BigDecimal descuentoMax
    );
    
    /**
     * Busca la mejor condición comercial vigente para un proveedor
     * (la de mayor descuento)
     */
    @Query("SELECT c FROM CondicionComercialProveedor c WHERE c.proveedorId = :proveedorId AND c.activo = true AND " +
           "(c.fechainicio IS NULL OR c.fechainicio <= CURRENT_DATE) AND " +
           "(c.fechafin IS NULL OR c.fechafin >= CURRENT_DATE) " +
           "ORDER BY c.descuento DESC LIMIT 1")
    CondicionComercialProveedor findMejorCondicionVigente(@Param("proveedorId") String proveedorId);
    
    /**
     * Cuenta condiciones comerciales activas de un proveedor
     */
    @Query("SELECT COUNT(c) FROM CondicionComercialProveedor c WHERE c.proveedorId = :proveedorId AND c.activo = true")
    Long countActivasByProveedorId(@Param("proveedorId") String proveedorId);
    
    /**
     * Elimina todas las condiciones comerciales de un proveedor
     */
    void deleteByProveedorId(String proveedorId);
    
    /**
     * Busca condiciones comerciales por usuario que las creó
     */
    List<CondicionComercialProveedor> findByUsuarioOrderByFechaaltaDesc(String usuario);
    
    /**
     * Busca condiciones que vencen en los próximos días
     */
    @Query("SELECT c FROM CondicionComercialProveedor c WHERE c.activo = true AND " +
           "c.fechafin IS NOT NULL AND c.fechafin BETWEEN CURRENT_DATE AND :fechaLimite " +
           "ORDER BY c.fechafin ASC")
    List<CondicionComercialProveedor> findProximasAVencer(@Param("fechaLimite") LocalDate fechaLimite);
}