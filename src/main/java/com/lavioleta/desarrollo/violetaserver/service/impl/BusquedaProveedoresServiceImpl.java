package com.lavioleta.desarrollo.violetaserver.service.impl;

import com.lavioleta.desarrollo.violetaserver.dto.request.BusquedaProveedoresRequest;
import com.lavioleta.desarrollo.violetaserver.dto.response.BusquedaProveedoresResponse;
import com.lavioleta.desarrollo.violetaserver.dto.response.BusquedaProveedoresResponse.ProveedorResultado;
import com.lavioleta.desarrollo.violetaserver.repository.BusquedaProveedoresRepository;
import com.lavioleta.desarrollo.violetaserver.service.BusquedaProveedoresService;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Service;

import java.util.Collections;
import java.util.List;

@Service
public class BusquedaProveedoresServiceImpl implements BusquedaProveedoresService {
    
    private static final Logger logger = LoggerFactory.getLogger(BusquedaProveedoresServiceImpl.class);
    
    private final BusquedaProveedoresRepository repository;
    
    public BusquedaProveedoresServiceImpl(BusquedaProveedoresRepository repository) {
        this.repository = repository;
    }
    
    @Override
    public BusquedaProveedoresResponse buscarProveedores(BusquedaProveedoresRequest request) {
        logger.info("Ejecutando búsqueda de proveedores - Tipo: {}, Condición: {}, Mostrar inactivos: {}, Solo gastos: {}, Solo mercancía: {}", 
                   request.getCodcondicion(), request.getCondicion(), request.getMostrarInactivos(), 
                   request.getSoloProveedorGastos(), request.getSoloProveedorMercancia());
        
        try {
            // Validar condiciones mutuamente excluyentes (replicar lógica UI legacy)
            if (request.getSoloProveedorGastos() && request.getSoloProveedorMercancia()) {
                return BusquedaProveedoresResponse.builder()
                        .success(false)
                        .message("No se puede filtrar por gastos y mercancía simultáneamente")
                        .totalResultados(0)
                        .proveedores(Collections.emptyList())
                        .build();
            }
            
            // Construir condiciones SQL legacy style
            String condicionActivos = request.getMostrarInactivos() ? "" : "activo = 1 AND ";
            String condicionProvGastos = request.getSoloProveedorGastos() ? "AND provgastos = 1 " : "";
            String condicionProvMercancia = request.getSoloProveedorMercancia() ? "AND provmercancia = 1 " : "";
            
            int limite = Integer.parseInt(request.getFilas());
            List<ProveedorResultado> proveedores = Collections.emptyList();
            
            // Ejecutar búsqueda según el tipo (switch replicando lógica legacy)
            switch (request.getCodcondicion()) {
                case "RSO" -> {
                    if (request.getCondicion() != null && !request.getCondicion().trim().isEmpty()) {
                        proveedores = repository.buscarPorRazonSocial(
                            request.getCondicion().trim(), 
                            condicionActivos, 
                            condicionProvGastos, 
                            condicionProvMercancia, 
                            limite);
                    }
                }
                case "RFC" -> {
                    if (request.getCondicion() != null && !request.getCondicion().trim().isEmpty()) {
                        proveedores = repository.buscarPorRfc(
                            request.getCondicion().trim(), 
                            condicionActivos, 
                            condicionProvGastos, 
                            condicionProvMercancia, 
                            limite);
                    }
                }
                case "CLA" -> {
                    if (request.getCondicion() != null && !request.getCondicion().trim().isEmpty()) {
                        proveedores = repository.buscarPorClave(
                            request.getCondicion().trim(), 
                            condicionActivos, 
                            condicionProvGastos, 
                            condicionProvMercancia, 
                            limite);
                    }
                }
                case "REP" -> {
                    if (request.getCondicion() != null && !request.getCondicion().trim().isEmpty()) {
                        proveedores = repository.buscarPorRepresentanteLegal(
                            request.getCondicion().trim(), 
                            condicionActivos, 
                            condicionProvGastos, 
                            condicionProvMercancia, 
                            limite);
                    }
                }
                case "" -> {
                    // Búsqueda vacía - no devolver nada por defecto (replicando legacy)
                    logger.debug("Búsqueda vacía - no se ejecuta ninguna consulta");
                }
                default -> {
                    logger.warn("Tipo de búsqueda no reconocido: {}", request.getCodcondicion());
                    return BusquedaProveedoresResponse.builder()
                            .success(false)
                            .message("Tipo de búsqueda no válido: " + request.getCodcondicion())
                            .totalResultados(0)
                            .proveedores(Collections.emptyList())
                            .build();
                }
            }
            
            // Crear respuesta exitosa
            String mensaje = proveedores.isEmpty() 
                ? "No se encontraron proveedores con los criterios especificados"
                : String.format("Se encontraron %d proveedor(es)", proveedores.size());
                
            logger.info("Búsqueda completada - {} resultados encontrados", proveedores.size());
            
            return BusquedaProveedoresResponse.builder()
                    .success(true)
                    .message(mensaje)
                    .totalResultados(proveedores.size())
                    .proveedores(proveedores)
                    .build();
                    
        } catch (Exception e) {
            logger.error("Error durante la búsqueda de proveedores", e);
            return BusquedaProveedoresResponse.builder()
                    .success(false)
                    .message("Error interno del servidor: " + e.getMessage())
                    .totalResultados(0)
                    .proveedores(Collections.emptyList())
                    .build();
        }
    }
}