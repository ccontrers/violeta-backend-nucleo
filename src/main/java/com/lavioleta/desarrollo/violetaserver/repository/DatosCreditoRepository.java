package com.lavioleta.desarrollo.violetaserver.repository;

import com.lavioleta.desarrollo.violetaserver.entity.DatosCredito;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

@Repository
public interface DatosCreditoRepository extends JpaRepository<DatosCredito, String> {
}
