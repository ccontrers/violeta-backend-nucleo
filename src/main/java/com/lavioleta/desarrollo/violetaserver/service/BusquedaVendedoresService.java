package com.lavioleta.desarrollo.violetaserver.service;

import com.lavioleta.desarrollo.violetaserver.dto.request.BusquedaVendedoresRequest;
import com.lavioleta.desarrollo.violetaserver.dto.response.BusquedaVendedoresResponse;

/**
 * Servicio de búsqueda de vendedores
 * Migrado desde ServidorBusquedas::BuscaVendedores
 */
public interface BusquedaVendedoresService {
    
    /**
     * Ejecuta una búsqueda de vendedores según los criterios especificados
     * 
     * Tipos soportados:
     * - NOM: Búsqueda por nombre
     * - APE: Búsqueda por apellidos
     * - COMI: Búsqueda por tipo de comisión
     * - CLA: Búsqueda por clave de empleado
     * 
     * @param request Parámetros de búsqueda (tipo, valor, soloActivos, limite)
     * @return Respuesta con los vendedores encontrados
     */
    BusquedaVendedoresResponse buscarVendedores(BusquedaVendedoresRequest request);
}
