package com.lavioleta.desarrollo.violetaserver.repository;

import com.lavioleta.desarrollo.violetaserver.entity.ClienteDetalleEcommerce;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

@Repository
public interface ClienteDetalleEcommerceRepository extends JpaRepository<ClienteDetalleEcommerce, String> {
}
