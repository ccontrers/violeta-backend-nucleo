package com.lavioleta.desarrollo.violetaserver.service;

import java.util.List;
import java.util.Map;

/**
 * Servicio para catálogos auxiliares
 */
public interface CatalogosService {
    
    /**
     * Obtener todas las colonias
     */
    List<Map<String, Object>> obtenerColonias();
    
    /**
     * Buscar colonias por nombre
     */
    List<Map<String, Object>> buscarColoniasPorNombre(String nombre);
    
    /**
     * Obtener catálogo de canales de clientes
     */
    List<Map<String, Object>> obtenerCanalesClientes();
    
    /**
     * Obtener catálogo de giros de negocio
     */
    List<Map<String, Object>> obtenerGirosNegocio();
    
    /**
     * Obtener catálogo de regímenes fiscales
     */
    List<Map<String, Object>> obtenerRegimenesFiscales();
    
    /**
     * Obtener catálogo de sociedades mercantiles
     */
    List<Map<String, Object>> obtenerSociedadesMercantiles();
    
    /**
     * Obtener catálogo de formas de pago
     */
    
    /**
     * Obtener catálogo de usos CFDI
     */
    List<Map<String, Object>> obtenerUsosCfdi();
    
    /**
     * Obtener catálogo de tipos de precios
     */
    List<Map<String, Object>> obtenerTiposPrecios();
    
    /**
     * Obtener catálogo de tipos de precios filtrado por empresa
     */
    List<Map<String, Object>> obtenerTiposPreciosPorEmpresa(Integer idEmpresa);
    
    /**
     * Obtener catálogo de tipos de cuentas bancarias activas
     */
    List<Map<String, Object>> obtenerTiposCuentasBancarias();
    
    /**
     * Obtener catálogo de bancos activos
     */
    List<Map<String, Object>> obtenerBancos();
    
    /*
    // Comentado temporalmente - tablas no disponibles
    List<Map<String, Object>> obtenerCategoriasClientes();
    List<Map<String, Object>> obtenerTiposClientes();
    List<Map<String, Object>> obtenerClasificacionesClientes();
    List<Map<String, Object>> obtenerSubclasificacionesClientes();
    List<Map<String, Object>> obtenerNacionalidades();
    List<Map<String, Object>> obtenerRegimenesMatrimoniales();
    */
}
