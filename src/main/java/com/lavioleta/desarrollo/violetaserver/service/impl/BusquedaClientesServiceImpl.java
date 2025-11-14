package com.lavioleta.desarrollo.violetaserver.service.impl;

import com.lavioleta.desarrollo.violetaserver.dto.request.BusquedaClientesRequest;
import com.lavioleta.desarrollo.violetaserver.dto.response.BusquedaClientesResponse;
import com.lavioleta.desarrollo.violetaserver.dto.response.BusquedaClientesResponse.ClienteResultado;
import com.lavioleta.desarrollo.violetaserver.repository.BusquedaClientesRepository;
import com.lavioleta.desarrollo.violetaserver.service.BusquedaClientesService;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Service;

import java.util.Collections;
import java.util.List;

@Service
public class BusquedaClientesServiceImpl implements BusquedaClientesService {
    
    private static final Logger logger = LoggerFactory.getLogger(BusquedaClientesServiceImpl.class);
    
    private final BusquedaClientesRepository repository;
    
    public BusquedaClientesServiceImpl(BusquedaClientesRepository repository) {
        this.repository = repository;
    }
    
    @Override
    public BusquedaClientesResponse buscarClientes(BusquedaClientesRequest request) {
        logger.info("Ejecutando búsqueda de clientes - Tipo: {}, Condición: {}, Mostrar inactivos: {}", 
                   request.getCodcondicion(), request.getCondicion(), request.getMostrarInactivos());
        
        try {
            // Determinar condición para clientes activos
            String condicionActivos = request.getMostrarInactivos() ? "" : "cli.activo = 1 AND ";
            
            List<ClienteResultado> clientes = Collections.emptyList();
            
            // Ejecutar búsqueda según el tipo
            switch (request.getCodcondicion()) {
                case "NOM" -> {
                    if (request.getCondicion() != null && !request.getCondicion().trim().isEmpty()) {
                        clientes = repository.buscarPorNombre(request.getCondicion().trim(), condicionActivos);
                    }
                }
                case "APE" -> {
                    if (request.getCondicion() != null && !request.getCondicion().trim().isEmpty()) {
                        clientes = repository.buscarPorApellido(request.getCondicion().trim(), condicionActivos);
                    }
                }
                case "RFC" -> {
                    if (request.getCondicion() != null && !request.getCondicion().trim().isEmpty()) {
                        clientes = repository.buscarPorRfc(request.getCondicion().trim(), condicionActivos);
                    }
                }
                case "RSO" -> {
                    if (request.getCondicion() != null && !request.getCondicion().trim().isEmpty()) {
                        clientes = repository.buscarPorRazonSocial(request.getCondicion().trim(), condicionActivos);
                    }
                }
                case "NNE" -> {
                    if (request.getCondicion() != null && !request.getCondicion().trim().isEmpty()) {
                        clientes = repository.buscarPorNombreNegocio(request.getCondicion().trim(), condicionActivos);
                    }
                }
                case "CLA" -> {
                    if (request.getCondicion() != null && !request.getCondicion().trim().isEmpty()) {
                        clientes = repository.buscarPorClave(request.getCondicion().trim(), condicionActivos);
                    }
                }
                case "CONT" -> {
                    if (request.getCondicion() != null && !request.getCondicion().trim().isEmpty()) {
                        clientes = repository.buscarPorContacto(request.getCondicion().trim(), condicionActivos);
                    }
                }
                case "EMA" -> {
                    if (request.getCondicion() != null && !request.getCondicion().trim().isEmpty()) {
                        clientes = repository.buscarPorEmail(request.getCondicion().trim(), condicionActivos);
                    }
                }
                case "" -> {
                    // Búsqueda vacía - no devolver nada por defecto
                    logger.debug("Búsqueda vacía - no se ejecuta ninguna consulta");
                }
                default -> {
                    logger.warn("Tipo de búsqueda no reconocido: {}", request.getCodcondicion());
                    return BusquedaClientesResponse.builder()
                            .success(false)
                            .message("Tipo de búsqueda no válido: " + request.getCodcondicion())
                            .totalResultados(0)
                            .clientes(Collections.emptyList())
                            .build();
                }
            }
            
            // Crear respuesta exitosa
            String mensaje = clientes.isEmpty() 
                ? "No se encontraron clientes con los criterios especificados"
                : String.format("Se encontraron %d cliente(s)", clientes.size());
                
            logger.info("Búsqueda completada - {} resultados encontrados", clientes.size());
            
            return BusquedaClientesResponse.builder()
                    .success(true)
                    .message(mensaje)
                    .totalResultados(clientes.size())
                    .clientes(clientes)
                    .build();
                    
        } catch (Exception e) {
            logger.error("Error durante la búsqueda de clientes", e);
            return BusquedaClientesResponse.builder()
                    .success(false)
                    .message("Error interno del servidor: " + e.getMessage())
                    .totalResultados(0)
                    .clientes(Collections.emptyList())
                    .build();
        }
    }
}
