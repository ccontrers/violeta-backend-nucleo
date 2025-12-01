package com.lavioleta.desarrollo.violetaserver.bitacora.service.impl;

import java.util.List;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Service;
import org.springframework.util.StringUtils;

import com.lavioleta.desarrollo.violetaserver.bitacora.dto.request.BitacoraUnificadaRequest;
import com.lavioleta.desarrollo.violetaserver.bitacora.dto.response.BitacoraUnificadaResponse;
import com.lavioleta.desarrollo.violetaserver.bitacora.repository.BitacoraUnificadaRepository;
import com.lavioleta.desarrollo.violetaserver.bitacora.service.BitacoraUnificadaService;

@Service
public class BitacoraUnificadaServiceImpl implements BitacoraUnificadaService {

    private static final Logger log = LoggerFactory.getLogger(BitacoraUnificadaServiceImpl.class);

    private final BitacoraUnificadaRepository repository;

    public BitacoraUnificadaServiceImpl(BitacoraUnificadaRepository repository) {
        this.repository = repository;
    }

    @Override
    public BitacoraUnificadaResponse consultar(BitacoraUnificadaRequest request) {
        String usuarioFiltrado = sanitizeUsuario(request.getUsuario());
        if (!StringUtils.hasText(usuarioFiltrado)) {
            return BitacoraUnificadaResponse.error("Debe proporcionar un usuario válido");
        }

        try {
            var eventos = repository.consultarBitacoraUsuarios(request.getFechaInicio(), request.getFechaFin(), List.of(usuarioFiltrado));
            String message = eventos.isEmpty()
                    ? "No se encontraron registros para los criterios indicados"
                    : "Consulta generada correctamente";
            return BitacoraUnificadaResponse.success(message, eventos);
        } catch (Exception ex) {
            log.error("Error al consultar la bitácora unificada para el usuario {}", usuarioFiltrado, ex);
            return BitacoraUnificadaResponse.error("No fue posible consultar la bitácora");
        }
    }

    private String sanitizeUsuario(String usuario) {
        if (!StringUtils.hasText(usuario)) {
            return null;
        }
        return usuario.trim();
    }
}
