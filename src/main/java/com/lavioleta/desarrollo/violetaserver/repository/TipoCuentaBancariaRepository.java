package com.lavioleta.desarrollo.violetaserver.repository;

import com.lavioleta.desarrollo.violetaserver.entity.TipoCuentaBancaria;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Query;
import org.springframework.stereotype.Repository;
import java.util.List;

/**
 * Repositorio para tipos de cuentas bancarias
 */
@Repository
public interface TipoCuentaBancariaRepository extends JpaRepository<TipoCuentaBancaria, String> {
    
    /**
     * Buscar tipos de cuenta activos
     */
    @Query("SELECT t FROM TipoCuentaBancaria t WHERE t.activo = 1 ORDER BY t.descripcion")
    List<TipoCuentaBancaria> findAllActivos();
}
