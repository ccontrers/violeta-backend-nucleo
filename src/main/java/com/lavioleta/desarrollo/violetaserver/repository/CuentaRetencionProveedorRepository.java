package com.lavioleta.desarrollo.violetaserver.repository;

import com.lavioleta.desarrollo.violetaserver.entity.CuentaRetencionProveedor;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Query;
import org.springframework.data.repository.query.Param;
import org.springframework.stereotype.Repository;

import java.util.List;
import java.util.Optional;

/**
 * Repositorio para la entidad CuentaRetencionProveedor
 * Relación 1:1 con Proveedor
 */
@Repository
public interface CuentaRetencionProveedorRepository extends JpaRepository<CuentaRetencionProveedor, String> {
    
    /**
     * Busca cuenta de retención por proveedor
     */
    Optional<CuentaRetencionProveedor> findByProveedorId(String proveedorId);
    
    /**
     * Busca proveedores con cuentas de retención configuradas
     */
    @Query("SELECT c FROM CuentaRetencionProveedor c WHERE c.configurado = true")
    List<CuentaRetencionProveedor> findAllConfiguradas();
    
    /**
     * Busca proveedores con retención de IVA configurada
     */
    @Query("SELECT c FROM CuentaRetencionProveedor c WHERE c.cuenta_iva_ret IS NOT NULL AND c.cuenta_iva_ret <> ''")
    List<CuentaRetencionProveedor> findWithIvaRetencion();
    
    /**
     * Busca proveedores con retención de ISR configurada
     */
    @Query("SELECT c FROM CuentaRetencionProveedor c WHERE c.cuenta_isr_ret IS NOT NULL AND c.cuenta_isr_ret <> ''")
    List<CuentaRetencionProveedor> findWithIsrRetencion();
    
    /**
     * Busca proveedores con retención de IEPS configurada
     */
    @Query("SELECT c FROM CuentaRetencionProveedor c WHERE c.cuenta_ieps_ret IS NOT NULL AND c.cuenta_ieps_ret <> ''")
    List<CuentaRetencionProveedor> findWithIepsRetencion();
    
    /**
     * Busca proveedores con retención de IETU configurada
     */
    @Query("SELECT c FROM CuentaRetencionProveedor c WHERE c.cuenta_ietu_ret IS NOT NULL AND c.cuenta_ietu_ret <> ''")
    List<CuentaRetencionProveedor> findWithIetuRetencion();
    
    /**
     * Busca por cuenta específica de IVA
     */
    @Query("SELECT c FROM CuentaRetencionProveedor c WHERE c.cuenta_iva_ret = :cuentaIvaRet")
    List<CuentaRetencionProveedor> findByCuentaIvaRet(@Param("cuentaIvaRet") String cuentaIvaRet);
    
    /**
     * Busca por cuenta específica de ISR
     */
    @Query("SELECT c FROM CuentaRetencionProveedor c WHERE c.cuenta_isr_ret = :cuentaIsrRet")
    List<CuentaRetencionProveedor> findByCuentaIsrRet(@Param("cuentaIsrRet") String cuentaIsrRet);
    
    /**
     * Verifica si existe una cuenta específica de retención
     */
    @Query("SELECT COUNT(c) > 0 FROM CuentaRetencionProveedor c WHERE " +
           "c.cuenta_iva_ret = :cuenta OR c.cuenta_isr_ret = :cuenta OR " +
           "c.cuenta_ieps_ret = :cuenta OR c.cuenta_ietu_ret = :cuenta")
    boolean existsByCuentaRetencion(@Param("cuenta") String cuenta);
    
    /**
     * Cuenta proveedores con cuentas de retención configuradas
     */
    @Query("SELECT COUNT(c) FROM CuentaRetencionProveedor c WHERE c.configurado = true")
    Long countConfiguradas();
    
    /**
     * Elimina cuenta de retención de un proveedor
     */
    void deleteByProveedorId(String proveedorId);
}