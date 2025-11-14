package com.lavioleta.desarrollo.violetaserver.repository;

import com.lavioleta.desarrollo.violetaserver.entity.Banco;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Query;
import org.springframework.stereotype.Repository;
import java.util.List;

/**
 * Repositorio para bancos
 */
@Repository
public interface BancoRepository extends JpaRepository<Banco, String> {
    
    /**
     * Buscar bancos activos ordenados por nombre
     */
    @Query("SELECT b FROM Banco b WHERE b.activoapp = 1 ORDER BY b.nombre, b.banco")
    List<Banco> findAllActivos();
    
    /**
     * Buscar todos los bancos ordenados por nombre (sin filtrar por activoapp)
     */
    @Query("SELECT b FROM Banco b ORDER BY b.nombre, b.banco")
    List<Banco> findAllOrdenados();
}
