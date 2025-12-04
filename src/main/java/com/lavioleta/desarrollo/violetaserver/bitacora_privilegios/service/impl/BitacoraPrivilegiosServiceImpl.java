package com.lavioleta.desarrollo.violetaserver.bitacora_privilegios.service.impl;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Service;
import org.springframework.util.StringUtils;

import com.lavioleta.desarrollo.violetaserver.bitacora_privilegios.dto.request.BitacoraPrivilegiosRequest;
import com.lavioleta.desarrollo.violetaserver.bitacora_privilegios.dto.response.BitacoraPrivilegiosResponse;
import com.lavioleta.desarrollo.violetaserver.bitacora_privilegios.repository.BitacoraPrivilegiosRepository;
import com.lavioleta.desarrollo.violetaserver.bitacora_privilegios.service.BitacoraPrivilegiosService;

/**
 * Implementación del servicio de bitácora de modificaciones de privilegios.
 * Migrado desde FormBitacoraModPrivilegios.cpp
 * 
 * Nota: La validación de privilegios (BITMPUR) no se incluye en esta fase
 * según indicación del documento de migración.
 */
@Service
public class BitacoraPrivilegiosServiceImpl implements BitacoraPrivilegiosService {

    private static final Logger log = LoggerFactory.getLogger(BitacoraPrivilegiosServiceImpl.class);

    private final BitacoraPrivilegiosRepository repository;

    public BitacoraPrivilegiosServiceImpl(BitacoraPrivilegiosRepository repository) {
        this.repository = repository;
    }

    @Override
    public BitacoraPrivilegiosResponse consultar(BitacoraPrivilegiosRequest request) {
        log.info("Consultando bitácora de privilegios: {} - {}", 
                request.getFechaInicio(), request.getFechaFin());

        try {
            var eventos = repository.consultarBitacora(
                    request.getFechaInicio(),
                    request.getFechaFin(),
                    sanitize(request.getUsuario()),
                    sanitize(request.getRol()),
                    sanitize(request.getTipoContexto()),
                    sanitize(request.getEntidadInvolucrada()));

            String message = eventos.isEmpty()
                    ? "No se encontraron registros para los criterios indicados"
                    : String.format("Se encontraron %d registros", eventos.size());

            log.debug("Consulta de bitácora completada: {} eventos", eventos.size());
            return BitacoraPrivilegiosResponse.success(message, eventos);

        } catch (Exception ex) {
            log.error("Error al consultar la bitácora de privilegios", ex);
            return BitacoraPrivilegiosResponse.error("No fue posible consultar la bitácora");
        }
    }

    private String sanitize(String value) {
        if (!StringUtils.hasText(value)) {
            return null;
        }
        return value.trim();
    }
}
