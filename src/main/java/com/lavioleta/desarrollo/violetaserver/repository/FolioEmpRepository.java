package com.lavioleta.desarrollo.violetaserver.repository;

import com.lavioleta.desarrollo.violetaserver.entity.FolioEmp;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Lock;
import org.springframework.data.jpa.repository.Modifying;
import org.springframework.data.jpa.repository.Query;
import org.springframework.data.repository.query.Param;
import org.springframework.stereotype.Repository;

import jakarta.persistence.LockModeType;
import java.util.Optional;

/**
 * Repositorio para la entidad FolioEmp
 * Gestiona los folios consecutivos por tipo y sucursal
 */
@Repository
public interface FolioEmpRepository extends JpaRepository<FolioEmp, FolioEmp.FolioEmpId> {

    /**
     * Obtiene y bloquea el folio para una sucursal específica
     * Implementa el patrón de bloqueo pesimista para evitar duplicados
     * 
     * @param folio Tipo de folio (ej: "PROV" para proveedores)
     * @param sucursal Código de sucursal
     * @return Optional con el FolioEmp si existe
     */
    @Lock(LockModeType.PESSIMISTIC_WRITE)
    @Query("SELECT f FROM FolioEmp f WHERE f.folio = :folio AND f.sucursal = :sucursal")
    Optional<FolioEmp> findByFolioAndSucursalWithLock(@Param("folio") String folio, 
                                                        @Param("sucursal") String sucursal);

    /**
     * Incrementa el valor del folio
     * 
     * @param folio Tipo de folio
     * @param sucursal Código de sucursal
     * @return Número de registros afectados
     */
    @Modifying
    @Query("UPDATE FolioEmp f SET f.valor = f.valor + 1 WHERE f.folio = :folio AND f.sucursal = :sucursal")
    int incrementarValor(@Param("folio") String folio, @Param("sucursal") String sucursal);
}
