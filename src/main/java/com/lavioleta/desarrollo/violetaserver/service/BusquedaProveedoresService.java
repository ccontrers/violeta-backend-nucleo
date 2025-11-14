package com.lavioleta.desarrollo.violetaserver.service;

import com.lavioleta.desarrollo.violetaserver.dto.request.BusquedaProveedoresRequest;
import com.lavioleta.desarrollo.violetaserver.dto.response.BusquedaProveedoresResponse;

public interface BusquedaProveedoresService {
    
    /**
     * Ejecuta una búsqueda de proveedores según los criterios especificados
     * 
     * @param request Parámetros de búsqueda
     * @return Respuesta con los proveedores encontrados
     */
    BusquedaProveedoresResponse buscarProveedores(BusquedaProveedoresRequest request);
}