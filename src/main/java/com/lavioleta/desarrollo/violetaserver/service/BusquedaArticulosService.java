package com.lavioleta.desarrollo.violetaserver.service;

import com.lavioleta.desarrollo.violetaserver.dto.request.BusquedaArticulosRequest;
import com.lavioleta.desarrollo.violetaserver.dto.response.BusquedaArticulosResponse;

import java.util.Map;

public interface BusquedaArticulosService {
    
    /**
     * Busca artículos según los criterios especificados
     * @param request Parámetros de búsqueda
     * @return Respuesta con resultados de la búsqueda
     */
    BusquedaArticulosResponse buscarArticulos(BusquedaArticulosRequest request);
    
    /**
     * Obtiene detalles completos de un artículo específico
     * @param articulo Código del artículo
     * @param sucursal Código de la sucursal
     * @return Mapa con detalles del artículo incluyendo precios y existencias
     */
    Map<String, Object> obtenerDetallesArticulo(String articulo, String sucursal);
}
