package com.lavioleta.desarrollo.violetaserver.service.impl;

import com.lavioleta.desarrollo.violetaserver.dto.request.BusquedaVendedoresRequest;
import com.lavioleta.desarrollo.violetaserver.dto.response.BusquedaVendedoresResponse;
import com.lavioleta.desarrollo.violetaserver.dto.response.BusquedaVendedoresResponse.VendedorResultado;
import com.lavioleta.desarrollo.violetaserver.repository.BusquedaVendedoresRepository;
import com.lavioleta.desarrollo.violetaserver.service.BusquedaVendedoresService;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Service;

import java.util.Collections;
import java.util.List;

/**
 * Implementación del servicio de búsqueda de vendedores
 * Migrado desde ServidorBusquedas::BuscaVendedores (ClassServidorBusquedas.cpp)
 * 
 * Lógica legacy:
 * - Extrae 3 parámetros: tipo_busqueda, solo_activos, dato_buscado
 * - Construye condición activos: "1" -> v.activo=1, otro -> v.activo=0
 * - Ejecuta SELECT según tipo con LIMIT NUM_LIMITE_RESULTADOS_BUSQ (501)
 */
@Service
public class BusquedaVendedoresServiceImpl implements BusquedaVendedoresService {
    
    private static final Logger logger = LoggerFactory.getLogger(BusquedaVendedoresServiceImpl.class);
    
    private final BusquedaVendedoresRepository repository;
    
    public BusquedaVendedoresServiceImpl(BusquedaVendedoresRepository repository) {
        this.repository = repository;
    }
    
    @Override
    public BusquedaVendedoresResponse buscarVendedores(BusquedaVendedoresRequest request) {
        logger.info("Ejecutando búsqueda de vendedores - Tipo: {}, Valor: {}, SoloActivos: {}", 
                   request.getTipoBusqueda(), request.getValor(), request.getSoloActivos());
        
        try {
            List<VendedorResultado> vendedores;
            
            // Ejecutar búsqueda según el tipo (replicando switch legacy)
            vendedores = switch (request.getTipoBusqueda()) {
                case "NOM" -> repository.buscarPorNombre(
                    request.getValor().trim(),
                    request.getSoloActivos(),
                    request.getLimite()
                );
                case "APE" -> repository.buscarPorApellido(
                    request.getValor().trim(),
                    request.getSoloActivos(),
                    request.getLimite()
                );
                case "COMI" -> repository.buscarPorTipoComision(
                    request.getValor().trim(),
                    request.getSoloActivos(),
                    request.getLimite()
                );
                case "CLA" -> repository.buscarPorClave(
                    request.getValor().trim(),
                    request.getSoloActivos(),
                    request.getLimite()
                );
                default -> {
                    logger.warn("Tipo de búsqueda no reconocido: {}", request.getTipoBusqueda());
                    yield Collections.emptyList();
                }
            };
            
            // Construir respuesta (replicando formato legacy)
            String mensaje = vendedores.isEmpty()
                ? String.format("No se encontró ningún registro de %s", obtenerDescripcionTipoBusqueda(request))
                : String.format("Se encontraron %d vendedor(es)", vendedores.size());
            
            logger.info("Búsqueda completada - {} resultados encontrados", vendedores.size());
            
            return BusquedaVendedoresResponse.builder()
                    .success(true)
                    .message(mensaje)
                    .totalResultados(vendedores.size())
                    .vendedores(vendedores)
                    .build();
                    
        } catch (Exception e) {
            logger.error("Error durante la búsqueda de vendedores", e);
            return BusquedaVendedoresResponse.builder()
                    .success(false)
                    .message("Error interno del servidor: " + e.getMessage())
                    .totalResultados(0)
                    .vendedores(Collections.emptyList())
                    .build();
        }
    }
    
    /**
     * Obtiene descripción del tipo de búsqueda para mensajes de error
     * Replica mensajes legacy del formulario C++
     */
    private String obtenerDescripcionTipoBusqueda(BusquedaVendedoresRequest request) {
        return switch (request.getTipoBusqueda()) {
            case "NOM" -> "nombre = " + request.getValor();
            case "APE" -> "apellidos = " + request.getValor();
            case "COMI" -> "comisión = " + request.getValor();
            case "CLA" -> "clave = " + request.getValor();
            default -> "criterio desconocido";
        };
    }
}
