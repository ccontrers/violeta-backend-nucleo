package com.lavioleta.desarrollo.violetaserver.service;

import com.lavioleta.desarrollo.violetaserver.dto.request.ClienteRequest;
import com.lavioleta.desarrollo.violetaserver.dto.request.BajaClienteRequest;
import com.lavioleta.desarrollo.violetaserver.dto.response.ClienteResponse;
import com.lavioleta.desarrollo.violetaserver.dto.response.ClienteListResponse;
import com.lavioleta.desarrollo.violetaserver.dto.DatosCreditoDTO;
import com.lavioleta.desarrollo.violetaserver.dto.ClienteDetalleEcommerceDTO;

/**
 * Interface del servicio de catálogo de clientes
 */
public interface CatalogoClientesService {
    
    /**
     * Obtiene el ID de empresa asociado a una sucursal
     * @param sucursal código de sucursal
     * @return ID de empresa o 1 por defecto
     */
    Integer obtenerEmpresaPorSucursal(String sucursal);
    
    /**
     * Graba cliente (alta o modificación)
     * @param request datos del cliente
     * @param sucursal sucursal actual
     * @return respuesta con cliente grabado
     */
    ClienteResponse grabarCliente(ClienteRequest request, String sucursal);
    
    /**
     * Elimina cliente y todos sus datos relacionados
     * @param request datos para eliminación
     * @return respuesta de eliminación
     */
    ClienteResponse eliminarCliente(BajaClienteRequest request);
    
    /**
     * Consulta un cliente específico
     * @param codigoCliente código del cliente
     * @param idEmpresa ID de empresa para datos específicos
     * @return respuesta con cliente completo
     */
    ClienteResponse consultarCliente(String codigoCliente, Integer idEmpresa);
    
    /**
     * Lista clientes con filtros
     * @param filtros criterios de búsqueda
     * @param pagina número de página
     * @param registrosPorPagina cantidad por página
     * @param idEmpresa ID de empresa para datos específicos
     * @return lista de clientes
     */
    ClienteListResponse listarClientes(String filtros, Integer pagina, Integer registrosPorPagina, Integer idEmpresa);
    
    /**
     * Verifica si un cliente existe
     * @param codigoCliente código del cliente
     * @return true si existe
     */
    boolean existeCliente(String codigoCliente);
    
    /**
     * Obtiene datos de crédito de un cliente
     * @param idCliente ID del cliente
     * @return datos de crédito o null si no existen
     */
    DatosCreditoDTO obtenerDatosCredito(String idCliente);
    
    /**
     * Guarda o actualiza datos de crédito de un cliente
     * @param datosCredito datos de crédito
     * @return datos guardados
     */
    DatosCreditoDTO guardarDatosCredito(DatosCreditoDTO datosCredito);
    
    /**
     * Obtiene detalles de ecommerce de un cliente
     * @param idCliente ID del cliente
     * @return detalles de ecommerce o null si no existen
     */
    ClienteDetalleEcommerceDTO obtenerDetalleEcommerce(String idCliente);
    
    /**
     * Guarda o actualiza detalles de ecommerce de un cliente
     * @param detalleEcommerce detalles de ecommerce
     * @return detalles guardados
     */
    ClienteDetalleEcommerceDTO guardarDetalleEcommerce(ClienteDetalleEcommerceDTO detalleEcommerce);
}
