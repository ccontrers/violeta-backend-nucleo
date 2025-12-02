package com.lavioleta.desarrollo.violetaserver.bitacora.service;

import com.lavioleta.desarrollo.violetaserver.bitacora.dto.request.BitacoraUnificadaRequest;
import com.lavioleta.desarrollo.violetaserver.bitacora.dto.response.BitacoraUnificadaResponse;

/**
 * Servicio para consultar la bitácora unificada de actividades de usuarios.
 */
public interface BitacoraUnificadaService {

    BitacoraUnificadaResponse consultar(BitacoraUnificadaRequest request);
}
