package com.lavioleta.desarrollo.violetaserver.repository;

import com.lavioleta.desarrollo.violetaserver.entity.Proveedor;
import org.springframework.data.domain.Page;
import org.springframework.data.domain.Pageable;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Query;
import org.springframework.data.repository.query.Param;
import org.springframework.stereotype.Repository;

import java.util.List;
import java.util.Optional;

/**
 * Repositorio para la entidad Proveedor
 * Migrado de ServidorCatalogos::ConsultaProveedor, GrabaProveedor, BajaProveedor
 */
@Repository
public interface ProveedorRepository extends JpaRepository<Proveedor, String> {
    
    /**
     * Busca proveedores por razón social (like ignoreCase)
     */
    List<Proveedor> findByRazonsocialContainingIgnoreCaseAndActivoTrue(String razonsocial);
    
    /**
     * Busca proveedores por razón social con paginación
     */
    Page<Proveedor> findByRazonsocialContainingIgnoreCaseAndActivoTrue(String razonsocial, Pageable pageable);
    
    /**
     * Busca proveedores por RFC
     */
    Optional<Proveedor> findByRfcAndActivoTrue(String rfc);
    
    /**
     * Busca proveedores por CURP
     */
    Optional<Proveedor> findByCurpAndActivoTrue(String curp);
    
    /**
     * Busca proveedores activos
     */
    List<Proveedor> findByActivoTrueOrderByRazonsocial();
    
    /**
     * Busca proveedores activos con paginación
     */
    Page<Proveedor> findByActivoTrueOrderByRazonsocial(Pageable pageable);
    
    /**
     * Busca proveedores por comprador
     */
    List<Proveedor> findByCompradorAndActivoTrueOrderByRazonsocial(String comprador);
    
    /**
     * Busca proveedores por estado
     */
    List<Proveedor> findByEstadoAndActivoTrueOrderByRazonsocial(String estado);
    
    /**
     * Busca proveedores de gastos activos
     */
    @Query("SELECT p FROM Proveedor p WHERE p.provgastos = 1 AND p.activo = true ORDER BY p.razonsocial")
    List<Proveedor> findProveedoresGastosActivos();
    
    /**
     * Busca proveedores de mercancía activos
     */
    @Query("SELECT p FROM Proveedor p WHERE p.provmercancia = 1 AND p.activo = true ORDER BY p.razonsocial")
    List<Proveedor> findProveedoresMercanciaActivos();
    
    /**
     * Busca proveedores RESICO activos
     */
    @Query("SELECT p FROM Proveedor p WHERE p.esresico = 1 AND p.activo = true ORDER BY p.razonsocial")
    List<Proveedor> findProveedoresResicoActivos();
    
    /**
     * Busca proveedores con crédito activo
     */
    @Query("SELECT p FROM Proveedor p WHERE p.credito = true AND p.activo = true ORDER BY p.razonsocial")
    List<Proveedor> findProveedoresConCredito();
    
    /**
     * Busca proveedores con pedido automatico configurado
     */
    @Query("SELECT p FROM Proveedor p WHERE (p.mincajas > 0 OR p.minpeso > 0 OR p.mindinero > 0) AND p.activo = true ORDER BY p.razonsocial")
    List<Proveedor> findProveedoresConPedidoAutomatico();
    
    /**
     * Busca proveedores cotizables
     */
    @Query("SELECT p FROM Proveedor p WHERE p.cotizable = true AND p.activo = true ORDER BY p.razonsocial")
    List<Proveedor> findProveedoresCotizables();
    
    /**
     * Verifica si existe un proveedor con el RFC (excluyendo el proveedor actual)
     */
    @Query("SELECT COUNT(p) > 0 FROM Proveedor p WHERE p.rfc = :rfc AND (:proveedorActual IS NULL OR p.proveedor <> :proveedorActual) AND p.activo = true")
    boolean existsByRfcAndProveedorNot(@Param("rfc") String rfc, @Param("proveedorActual") String proveedorActual);
    
    /**
     * Verifica si existe un proveedor con la CURP (excluyendo el proveedor actual)
     */
    @Query("SELECT COUNT(p) > 0 FROM Proveedor p WHERE p.curp = :curp AND (:proveedorActual IS NULL OR p.proveedor <> :proveedorActual) AND p.activo = true")
    boolean existsByCurpAndProveedorNot(@Param("curp") String curp, @Param("proveedorActual") String proveedorActual);
    
    /**
     * Búsqueda general combinada por múltiples criterios
     */
    @Query("SELECT p FROM Proveedor p WHERE " +
           "(:razonSocial IS NULL OR UPPER(p.razonsocial) LIKE UPPER(CONCAT('%', :razonSocial, '%'))) AND " +
           "(:rfc IS NULL OR UPPER(p.rfc) LIKE UPPER(CONCAT('%', :rfc, '%'))) AND " +
           "(:estado IS NULL OR p.estado = :estado) AND " +
           "(:comprador IS NULL OR p.comprador = :comprador) AND " +
           "(:tipoProveedor IS NULL OR " +
           "   (:tipoProveedor = 'GASTOS' AND p.provgastos = 1) OR " +
           "   (:tipoProveedor = 'MERCANCIA' AND p.provmercancia = 1)) AND " +
           "p.activo = true " +
           "ORDER BY p.razonsocial")
    Page<Proveedor> findByCriteriosMultiples(
        @Param("razonSocial") String razonSocial,
        @Param("rfc") String rfc,
        @Param("estado") String estado,
        @Param("comprador") String comprador,
        @Param("tipoProveedor") String tipoProveedor,
        Pageable pageable
    );
    
    /**
     * Obtiene el siguiente número de proveedor para alta automática
     */
    @Query(value = "SELECT COALESCE(MAX(CAST(p.proveedor AS UNSIGNED)), 0) + 1 FROM proveedores p WHERE p.proveedor REGEXP '^[0-9]+$'", nativeQuery = true)
    Integer getNextProveedorNumber();
}