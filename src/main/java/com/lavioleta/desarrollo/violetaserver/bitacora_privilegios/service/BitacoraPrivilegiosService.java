package com.lavioleta.desarrollo.violetaserver.bitacora_privilegios.service;

import com.lavioleta.desarrollo.violetaserver.bitacora_privilegios.dto.request.BitacoraPrivilegiosRequest;
import com.lavioleta.desarrollo.violetaserver.bitacora_privilegios.dto.response.BitacoraPrivilegiosResponse;

/**
 * Servicio para consultar la bitácora de modificaciones de privilegios.
 * Migrado desde FormBitacoraModPrivilegios.cpp
 */
public interface BitacoraPrivilegiosService {

    /**
     * Consulta la bitácora de modificaciones de privilegios según los criterios especificados.
     * 
     * @param request Criterios de búsqueda (fechas obligatorias, filtros opcionales)
     * @return Respuesta con los eventos encontrados
     */
    BitacoraPrivilegiosResponse consultar(BitacoraPrivilegiosRequest request);
}
