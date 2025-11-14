package com.lavioleta.desarrollo.violetaserver.service.impl;

import com.lavioleta.desarrollo.violetaserver.dto.request.BusquedaArticulosRequest;
import com.lavioleta.desarrollo.violetaserver.dto.response.BusquedaArticulosResponse;
import com.lavioleta.desarrollo.violetaserver.dto.response.BusquedaArticulosResponse.*;
import com.lavioleta.desarrollo.violetaserver.repository.BusquedaArticulosRepository;
import com.lavioleta.desarrollo.violetaserver.service.BusquedaArticulosService;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Service;

import java.util.*;

@Service
public class BusquedaArticulosServiceImpl implements BusquedaArticulosService {
    
    private static final Logger logger = LoggerFactory.getLogger(BusquedaArticulosServiceImpl.class);
    private final BusquedaArticulosRepository repository;
    
    public BusquedaArticulosServiceImpl(BusquedaArticulosRepository repository) {
        this.repository = repository;
    }
    
    @Override
    public BusquedaArticulosResponse buscarArticulos(BusquedaArticulosRequest request) {
        try {
            logger.info("Iniciando búsqueda de artículos. Tipo: {}, Sucursal: {}, Dato: {}", 
                       request.getTipoBusqueda(), request.getSucursal(), request.getDatoBuscado());
            
            // Verificar si mostrar existencias
            boolean mostrarExistencias = verificarMostrarExistencias(request);
            
            // Construir condiciones
            String tablaExistencias = construirTablaExistencias(mostrarExistencias);
            String condicionExistencias = construirCondicionExistencias(mostrarExistencias, request.getSucursal());
            String joinExistencias = construirJoinExistencias(mostrarExistencias, request.getSucursal());
            String condicionActivos = construirCondicionActivos(request.getSoloActivos());
            
            // Ejecutar búsqueda según tipo
            BusquedaArticulosResponse response = ejecutarBusqueda(
                request, condicionActivos, tablaExistencias, condicionExistencias, joinExistencias);
            
            logger.info("Búsqueda completada. Resultados encontrados: {}", 
                       response.getArticulos() != null ? response.getArticulos().size() : 0);
            
            return response;
            
        } catch (Exception e) {
            logger.error("Error en búsqueda de artículos: {}", e.getMessage(), e);
            return new BusquedaArticulosResponse(false, "Error en búsqueda: " + e.getMessage());
        }
    }
    
    private boolean verificarMostrarExistencias(BusquedaArticulosRequest request) {
        if ("NO".equals(request.getMostrarExistencias())) {
            return false;
        }
        return repository.verificarMostrarExistencias(request.getSucursal());
    }
    
    private String construirTablaExistencias(boolean mostrarExistencias) {
        if (!mostrarExistencias) {
            return "";
        }
        return "existenciasactuales e, secciones s, almacenes al";
    }
    
    private String construirCondicionExistencias(boolean mostrarExistencias, String sucursal) {
        if (!mostrarExistencias) {
            return "";
        }
        return String.format("e.producto = a.producto AND e.present = a.present AND s.sucursal = '%s' " +
                           "AND s.seccion = al.seccion AND al.almacen = e.almacen AND e.cantidad > 0", sucursal);
    }
    
    private String construirJoinExistencias(boolean mostrarExistencias, String sucursal) {
        if (!mostrarExistencias) {
            return "";
        }
        return String.format(" INNER JOIN secciones s ON s.sucursal = '%s' INNER JOIN almacenes al " +
                           "ON s.seccion = al.seccion INNER JOIN existenciasactuales e ON e.producto = a.producto AND " +
                           "e.present = a.present AND al.almacen = e.almacen AND e.cantidad > 0", sucursal);
    }
    
    private String construirCondicionActivos(String soloActivos) {
        if ("1".equals(soloActivos)) {
            return "a.activo=1 and";
        }
        return "";
    }
    
    private BusquedaArticulosResponse ejecutarBusqueda(BusquedaArticulosRequest request,
                                                            String condicionActivos,
                                                            String tablaExistencias,
                                                            String condicionExistencias,
                                                            String joinExistencias) {
        
        BusquedaArticulosResponse response = new BusquedaArticulosResponse(true, "Búsqueda exitosa");
        
        String tipoBusqueda = request.getTipoBusqueda();
        
        if ("N".equals(tipoBusqueda)) {
            // Búsqueda por nombre
            List<ArticuloResultado> articulos = repository.buscarPorNombre(
                request.getDatoBuscado(), condicionActivos, tablaExistencias, condicionExistencias);
            response.setArticulos(articulos);
            response.setTotalResultados(articulos.size());
            
        } else if ("C".equals(tipoBusqueda)) {
            // Búsqueda por clave/producto
            List<ArticuloResultado> articulos = repository.buscarPorClave(
                request.getDatoBuscado(), condicionActivos, tablaExistencias, condicionExistencias);
            response.setArticulos(articulos);
            response.setTotalResultados(articulos.size());
            
        } else if ("M".equals(tipoBusqueda)) {
            // Búsqueda por marca
            List<ArticuloResultado> articulos = repository.buscarPorMarca(
                request.getDatoBuscado(), condicionActivos, tablaExistencias, condicionExistencias);
            response.setArticulos(articulos);
            response.setTotalResultados(articulos.size());
            
        } else if ("E".equals(tipoBusqueda)) {
            // Búsqueda por clasificación
            String datoBuscado = request.getDatoBuscado();
            String codigoClasificacion = datoBuscado;
            
            // Si el dato buscado no es numérico, buscar el código por nombre
            if (!datoBuscado.matches("\\d+")) {
                codigoClasificacion = repository.obtenerCodigoClasificacion(datoBuscado);
                if (codigoClasificacion == null) {
                    logger.warn("No se encontró clasificación con nombre: {}", datoBuscado);
                    response.setArticulos(new ArrayList<>());
                    response.setTotalResultados(0);
                    response.setMessage("No se encontró la clasificación: " + datoBuscado);
                    return response;
                }
                logger.info("Convertido nombre '{}' a código de clasificación: {}", datoBuscado, codigoClasificacion);
            }
            
            List<ArticuloResultado> articulos = repository.buscarPorClasificacion(
                codigoClasificacion, condicionActivos, tablaExistencias, condicionExistencias);
            response.setArticulos(articulos);
            response.setTotalResultados(articulos.size());
            
        } else if ("CB".equals(tipoBusqueda)) {
            // Búsqueda por código de barras
            List<ArticuloResultado> articulos = repository.buscarPorCodigoBarras(
                request.getDatoBuscado(), condicionActivos, joinExistencias);
            response.setArticulos(articulos);
            response.setTotalResultados(articulos.size());
            
        } else if ("ART".equals(tipoBusqueda)) {
            // Búsqueda por artículo específico
            List<ArticuloResultado> articulos = repository.buscarPorArticulo(
                request.getDatoBuscado(), condicionActivos, tablaExistencias, condicionExistencias);
            response.setArticulos(articulos);
            response.setTotalResultados(articulos.size());
            
        } else if ("".equals(tipoBusqueda) || tipoBusqueda == null) {
            // Búsqueda inicial - devolver clasificaciones y marcas
            List<ClasificacionResultado> clasificaciones = repository.obtenerClasificaciones();
            List<MarcaResultado> marcas = repository.obtenerMarcas();
            response.setClasificaciones(clasificaciones);
            response.setMarcas(marcas);
            response.setArticulos(new ArrayList<>());
            response.setTotalResultados(clasificaciones.size() + marcas.size());
        } else {
            // Tipo de búsqueda no reconocido
            response.setSuccess(false);
            response.setMessage("Tipo de búsqueda no válido: " + tipoBusqueda);
            response.setArticulos(new ArrayList<>());
            response.setTotalResultados(0);
        }
        
        return response;
    }
    
    @Override
    public Map<String, Object> obtenerDetallesArticulo(String articulo, String sucursal) {
        logger.info("Obteniendo detalles completos del artículo: {} en sucursal: {}", articulo, sucursal);
        
        Map<String, Object> detalles = new HashMap<>();
        
        try {
            // Obtener precios del artículo
            var precios = repository.obtenerPreciosArticulo(articulo, sucursal);
            logger.debug("Encontrados {} precios para el artículo {}", precios.size(), articulo);
            
            // Obtener existencia del artículo
            var existencia = repository.obtenerExistenciaArticulo(articulo, sucursal);
            logger.debug("Existencia para artículo {}: cantidad={}, costo={}", articulo, existencia.getCantidad(), existencia.getCosto());
            
            // Formatear precios a 2 decimales
            List<Map<String, String>> preciosFormateados = precios.stream()
                    .map(precio -> Map.of(
                        "tipoPrec", precio.getTipoPrec(),
                        "precio", String.format("%.2f", precio.getPrecio() != null ? precio.getPrecio() : 0.0)
                    ))
                    .toList();
            
            detalles.put("articulo", articulo);
            detalles.put("sucursal", sucursal);
            detalles.put("precios", preciosFormateados);
            detalles.put("existencia", Map.of(
                "cantidad", String.format("%.2f", existencia.getCantidad() != null ? existencia.getCantidad() : 0.0),
                "costo", String.format("%.2f", existencia.getCosto() != null ? existencia.getCosto() : 0.0)
            ));
            detalles.put("success", true);
            
        } catch (Exception e) {
            logger.error("Error al obtener detalles del artículo {}: {}", articulo, e.getMessage(), e);
            detalles.put("success", false);
            detalles.put("error", e.getMessage());
        }
        
        return detalles;
    }
}
