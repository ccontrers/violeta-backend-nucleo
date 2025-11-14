package com.lavioleta.desarrollo.violetaserver.repository;

import com.lavioleta.desarrollo.violetaserver.entity.TelefonoProveedor;
import com.lavioleta.desarrollo.violetaserver.entity.TelefonoProveedorId;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Query;
import org.springframework.data.repository.query.Param;
import org.springframework.stereotype.Repository;

import java.util.List;
import java.util.Optional;

/**
 * Repositorio para la entidad TelefonoProveedor
 * Maneja PK compuesta: proveedor, lada, telefono
 */
@Repository
public interface TelefonoProveedorRepository extends JpaRepository<TelefonoProveedor, TelefonoProveedorId> {
    
    /**
     * Busca todos los teléfonos de un proveedor ordenados
     */
    List<TelefonoProveedor> findByProveedorIdOrderByLadaAscTelefonoAsc(String proveedorId);
    
    /**
     * Busca teléfonos por tipo
     */
    List<TelefonoProveedor> findByProveedorIdAndTipoContainingIgnoreCase(String proveedorId, String tipo);
    
    /**
     * Cuenta cuántos teléfonos tiene un proveedor
     */
    @Query("SELECT COUNT(t) FROM TelefonoProveedor t WHERE t.proveedorId = :proveedorId")
    Long countByProveedorId(@Param("proveedorId") String proveedorId);
    
    /**
     * Elimina todos los teléfonos de un proveedor
     */
    void deleteByProveedorId(String proveedorId);
    
    /**
     * Busca teléfonos por número (sin importar el proveedor)
     */
    @Query("SELECT t FROM TelefonoProveedor t WHERE t.lada = :lada AND t.telefono = :telefono")
    List<TelefonoProveedor> findByLadaAndTelefono(@Param("lada") String lada, @Param("telefono") String telefono);
}