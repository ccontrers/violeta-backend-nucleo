package com.lavioleta.desarrollo.violetaserver.service;

import com.lavioleta.desarrollo.violetaserver.dto.request.BusquedaClientesRequest;
import com.lavioleta.desarrollo.violetaserver.dto.response.BusquedaClientesResponse;

public interface BusquedaClientesService {
    
    /**
     * Ejecuta una búsqueda de clientes según los criterios especificados
     * 
     * @param request Parámetros de búsqueda
     * @return Respuesta con los clientes encontrados
     */
    BusquedaClientesResponse buscarClientes(BusquedaClientesRequest request);
}
