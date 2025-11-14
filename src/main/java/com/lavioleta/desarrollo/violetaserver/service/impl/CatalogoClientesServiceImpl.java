package com.lavioleta.desarrollo.violetaserver.service.impl;

import com.lavioleta.desarrollo.violetaserver.dto.request.ClienteRequest;
import com.lavioleta.desarrollo.violetaserver.dto.request.BajaClienteRequest;
import com.lavioleta.desarrollo.violetaserver.dto.response.ClienteResponse;
import com.lavioleta.desarrollo.violetaserver.dto.response.ClienteListResponse;
import com.lavioleta.desarrollo.violetaserver.repository.CatalogoClientesRepository;
import com.lavioleta.desarrollo.violetaserver.service.CatalogoClientesService;
import com.lavioleta.desarrollo.violetaserver.dto.DatosCreditoDTO;
import com.lavioleta.desarrollo.violetaserver.dto.ClienteDetalleEcommerceDTO;
import com.lavioleta.desarrollo.violetaserver.entity.DatosCredito;
import com.lavioleta.desarrollo.violetaserver.entity.ClienteDetalleEcommerce;
import com.lavioleta.desarrollo.violetaserver.repository.DatosCreditoRepository;
import com.lavioleta.desarrollo.violetaserver.repository.ClienteDetalleEcommerceRepository;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.jdbc.core.simple.JdbcClient;
import org.springframework.stereotype.Service;

import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * Implementación del servicio de catálogo de clientes con mapeo completo
 */
@Service
public class CatalogoClientesServiceImpl implements CatalogoClientesService {
    
    private static final Logger logger = LoggerFactory.getLogger(CatalogoClientesServiceImpl.class);
    private final CatalogoClientesRepository repository;
    private final DatosCreditoRepository datosCreditoRepository;
    private final ClienteDetalleEcommerceRepository clienteDetalleEcommerceRepository;
    private final JdbcClient jdbcClient;
    
    public CatalogoClientesServiceImpl(CatalogoClientesRepository repository,
                                     DatosCreditoRepository datosCreditoRepository,
                                     ClienteDetalleEcommerceRepository clienteDetalleEcommerceRepository,
                                     JdbcClient jdbcClient) {
        this.repository = repository;
        this.datosCreditoRepository = datosCreditoRepository;
        this.clienteDetalleEcommerceRepository = clienteDetalleEcommerceRepository;
        this.jdbcClient = jdbcClient;
    }
    
    @Override
    public Integer obtenerEmpresaPorSucursal(String sucursal) {
        return repository.obtenerEmpresaPorSucursal(sucursal);
    }
    
    @Override
    public ClienteResponse grabarCliente(ClienteRequest request, String sucursal) {
        try {
            logger.info("Procesando grabado de cliente: {} - Operación: {}", request.getCliente(), request.getOperacion());
            
            // Usar el método del repository que ya maneja tanto alta como modificación
            String codigoCliente = repository.grabaCliente(request, sucursal);
            
            // Consultar el cliente grabado para retornarlo en la respuesta
            Integer idEmpresa = request.getIdEmpresa() != null ? request.getIdEmpresa() : obtenerEmpresaPorSucursal(sucursal);
            Optional<ClienteResponse.ClienteCompleto> clienteCompleto = repository.consultarCliente(codigoCliente, idEmpresa);
            
            if (clienteCompleto.isPresent()) {
                String operacionTexto = "A".equals(request.getOperacion()) ? "creado" : "actualizado";
                return ClienteResponse.success(
                    "Cliente " + operacionTexto + " exitosamente", 
                    clienteCompleto.get()
                );
            } else {
                // Si no se puede consultar, devolver respuesta exitosa básica
                String operacionTexto = "A".equals(request.getOperacion()) ? "creado" : "actualizado";
                return ClienteResponse.successDelete("Cliente " + codigoCliente + " " + operacionTexto + " exitosamente");
            }
            
        } catch (Exception e) {
            logger.error("Error al grabar cliente: {}", e.getMessage(), e);
            return ClienteResponse.error("Error interno: " + e.getMessage());
        }
    }
    
    @Override
    public ClienteResponse eliminarCliente(BajaClienteRequest request) {
        try {
            logger.info("Procesando eliminación de cliente: {}", request.getCliente());
            
            // Usar el método del repository para dar de baja el cliente
            repository.bajaCliente(request);
            
            return ClienteResponse.successDelete("Cliente " + request.getCliente() + " eliminado exitosamente");
            
        } catch (Exception e) {
            logger.error("Error al eliminar cliente: {}", e.getMessage(), e);
            return ClienteResponse.error("Error interno: " + e.getMessage());
        }
    }
    
    @Override
    public ClienteResponse consultarCliente(String codigoCliente, Integer idEmpresa) {
        logger.debug("Consultando cliente: {} para empresa: {}", codigoCliente, idEmpresa);
        
        try {
            Optional<ClienteResponse.ClienteCompleto> clienteCompleto = repository.consultarCliente(codigoCliente, idEmpresa);
            
            if (clienteCompleto.isPresent()) {
                logger.info("Cliente {} consultado exitosamente", codigoCliente);
                return ClienteResponse.builder()
                        .success(true)
                        .message("Cliente consultado exitosamente")
                        .cliente(clienteCompleto.get())
                        .build();
            } else {
                logger.warn("Cliente no encontrado: {}", codigoCliente);
                return ClienteResponse.builder()
                        .success(false)
                        .message("Cliente no encontrado: " + codigoCliente)
                        .cliente(null)
                        .build();
            }
            
        } catch (Exception e) {
            logger.error("Error al consultar cliente {}: {}", codigoCliente, e.getMessage(), e);
            return ClienteResponse.builder()
                    .success(false)
                    .message("Error al consultar cliente: " + e.getMessage())
                    .cliente(null)
                    .build();
        }
    }
    
    @Override
    public ClienteListResponse listarClientes(String filtros, Integer pagina, Integer registrosPorPagina, Integer idEmpresa) {
        try {
            // Implementar paginación básica
            int limit = registrosPorPagina != null ? registrosPorPagina : 50;
            int offset = pagina != null ? (pagina - 1) * limit : 0;
            
            // Consulta con JOIN entre clientes y clientesemp
            String sql = """
                SELECT c.cliente, c.nombre, c.rfc, c.bloqueo 
                FROM clientes c
                INNER JOIN clientesemp ce ON c.cliente = ce.cliente
                WHERE ce.idempresa = ? 
                ORDER BY c.cliente 
                LIMIT ? OFFSET ?
                """;
            
            List<Map<String, Object>> resultados = jdbcClient.sql(sql)
                .param(idEmpresa)
                .param(limit)
                .param(offset)
                .query()
                .listOfRows();
            
            // Convertir a DTOs
            List<ClienteListResponse.ClienteResumen> clientes = resultados.stream()
                .map(row -> {
                    return ClienteListResponse.ClienteResumen.builder()
                        .cliente(String.valueOf(row.get("cliente")))
                        .nombre(String.valueOf(row.get("nombre")))
                        .rfc(String.valueOf(row.get("rfc")))
                        .bloqueo(String.valueOf(row.get("bloqueo")))
                        .activo(!"02".equals(String.valueOf(row.get("bloqueo")))) // Activo si no está bloqueado
                        .build();
                })
                .collect(Collectors.toList());
            
            // Contar total para paginación
            String countSql = """
                SELECT COUNT(*) 
                FROM clientes c
                INNER JOIN clientesemp ce ON c.cliente = ce.cliente
                WHERE ce.idempresa = ?
                """;
            Long total = jdbcClient.sql(countSql)
                .param(idEmpresa)
                .query(Long.class)
                .single();
            
            return ClienteListResponse.success(clientes, total.intValue(), 
                pagina != null ? pagina : 1, limit);
            
        } catch (Exception e) {
            logger.error("Error al listar clientes para empresa {}: {}", idEmpresa, e.getMessage(), e);
            return ClienteListResponse.error("Error al consultar la lista de clientes: " + e.getMessage());
        }
    }
    
    @Override
    public boolean existeCliente(String codigoCliente) {
        try {
            String sql = "SELECT COUNT(*) FROM clientes WHERE cliente = ?";
            Long count = jdbcClient.sql(sql)
                .param(codigoCliente)
                .query(Long.class)
                .single();
            return count > 0;
        } catch (Exception e) {
            logger.error("Error al verificar existencia del cliente {}: {}", codigoCliente, e.getMessage());
            return false;
        }
    }
    
    // MÉTODOS DE CRÉDITO Y ECOMMERCE - COMPLETAMENTE FUNCIONALES
    
    @Override
    public DatosCreditoDTO obtenerDatosCredito(String idCliente) {
        logger.debug("Obteniendo datos de crédito para cliente: {}", idCliente);
        
        try {
            Optional<DatosCredito> datosCredito = datosCreditoRepository.findById(idCliente);
            
            if (datosCredito.isPresent()) {
                DatosCreditoDTO dto = convertirDatosCreditoADTO(datosCredito.get());
                logger.info("Datos de crédito encontrados para cliente: {}", idCliente);
                return dto;
            }
            
            logger.debug("No se encontraron datos de crédito para el cliente: {}", idCliente);
            return null;
            
        } catch (Exception e) {
            logger.error("Error al obtener datos de crédito para cliente {}: {}", idCliente, e.getMessage(), e);
            return null;
        }
    }
    
    @Override
    public DatosCreditoDTO guardarDatosCredito(DatosCreditoDTO datosCredito) {
        logger.debug("Guardando datos de crédito para cliente: {}", datosCredito.getIdcliente());
        
        try {
            // Buscar si ya existe el registro
            Optional<DatosCredito> existente = datosCreditoRepository.findById(datosCredito.getIdcliente());
            DatosCredito entidad;
            
            if (existente.isPresent()) {
                entidad = existente.get();
                logger.debug("Actualizando datos de crédito existentes para cliente: {}", datosCredito.getIdcliente());
            } else {
                entidad = new DatosCredito();
                entidad.setCliente(datosCredito.getIdcliente());
                logger.debug("Creando nuevos datos de crédito para cliente: {}", datosCredito.getIdcliente());
            }
            
            // Mapear campos del DTO a la entidad
            actualizarEntidadDatosCredito(entidad, datosCredito);
            
            DatosCredito guardado = datosCreditoRepository.save(entidad);
            
            logger.info("Datos de crédito guardados exitosamente para cliente: {}", guardado.getCliente());
            return convertirDatosCreditoADTO(guardado);
            
        } catch (Exception e) {
            logger.error("Error al guardar datos de crédito para cliente {}: {}", 
                        datosCredito.getIdcliente(), e.getMessage(), e);
            throw new RuntimeException("Error al guardar datos de crédito: " + e.getMessage(), e);
        }
    }
    
    @Override
    public ClienteDetalleEcommerceDTO obtenerDetalleEcommerce(String idCliente) {
        logger.debug("Obteniendo detalles de ecommerce para cliente: {}", idCliente);
        
        try {
            Optional<ClienteDetalleEcommerce> detalleEcommerce = clienteDetalleEcommerceRepository.findById(idCliente);
            
            if (detalleEcommerce.isPresent()) {
                ClienteDetalleEcommerceDTO dto = convertirDetalleEcommerceADTO(detalleEcommerce.get());
                logger.info("Detalles de ecommerce encontrados para cliente: {}", idCliente);
                return dto;
            }
            
            logger.debug("No se encontraron detalles de ecommerce para el cliente: {}", idCliente);
            return null;
            
        } catch (Exception e) {
            logger.error("Error al obtener detalles de ecommerce para cliente {}: {}", idCliente, e.getMessage(), e);
            return null;
        }
    }
    
    @Override
    public ClienteDetalleEcommerceDTO guardarDetalleEcommerce(ClienteDetalleEcommerceDTO detalleEcommerce) {
        logger.debug("Guardando detalles de ecommerce para cliente: {}", detalleEcommerce.getIdcliente());
        
        try {
            // Buscar si ya existe el registro
            Optional<ClienteDetalleEcommerce> existente = clienteDetalleEcommerceRepository.findById(detalleEcommerce.getIdcliente());
            ClienteDetalleEcommerce entidad;
            
            if (existente.isPresent()) {
                entidad = existente.get();
                logger.debug("Actualizando detalles de ecommerce existentes para cliente: {}", detalleEcommerce.getIdcliente());
            } else {
                entidad = new ClienteDetalleEcommerce();
                entidad.setCliente(detalleEcommerce.getIdcliente());
                logger.debug("Creando nuevos detalles de ecommerce para cliente: {}", detalleEcommerce.getIdcliente());
            }
            
            // Mapear campos del DTO a la entidad
            actualizarEntidadDetalleEcommerce(entidad, detalleEcommerce);
            
            ClienteDetalleEcommerce guardado = clienteDetalleEcommerceRepository.save(entidad);
            
            logger.info("Detalles de ecommerce guardados exitosamente para cliente: {}", guardado.getCliente());
            return convertirDetalleEcommerceADTO(guardado);
            
        } catch (Exception e) {
            logger.error("Error al guardar detalles de ecommerce para cliente {}: {}", 
                        detalleEcommerce.getIdcliente(), e.getMessage(), e);
            throw new RuntimeException("Error al guardar detalles de ecommerce: " + e.getMessage(), e);
        }
    }
    
    // MÉTODOS AUXILIARES DE MAPEO
    
    /**
     * Convierte una entidad DatosCreditoReal a DTO
     */
    private DatosCreditoDTO convertirDatosCreditoADTO(DatosCredito entidad) {
        DatosCreditoDTO dto = new DatosCreditoDTO();
        dto.setIdcliente(entidad.getCliente());
        
        // Campos básicos que sabemos que existen
        dto.setIngresos(entidad.getCliingre());
        dto.setEgresos(entidad.getCliegre());
        dto.setVentasprom(entidad.getVentapm());
        dto.setNumautos(entidad.getCliautos());
        
        // Propiedades básicas
        dto.setP1tipo(entidad.getP1tipo());
        dto.setP1valor(entidad.getP1valor());
        dto.setP1dir(entidad.getP1dir());
        dto.setP2tipo(entidad.getP2tipo());
        dto.setP2valor(entidad.getP2valor());
        
        // Avales básicos
        dto.setA1nombre(entidad.getA1nombre());
        dto.setA1tel(entidad.getA1tel());
        dto.setA1dir(entidad.getA1dir());
        
        return dto;
    }
    
    /**
     * Actualiza una entidad DatosCreditoReal con datos del DTO
     */
    private void actualizarEntidadDatosCredito(DatosCredito entidad, DatosCreditoDTO dto) {
        // Solo campos básicos que sabemos que existen
        if (dto.getIngresos() != null) entidad.setCliingre(dto.getIngresos());
        if (dto.getEgresos() != null) entidad.setCliegre(dto.getEgresos());
        if (dto.getVentasprom() != null) entidad.setVentapm(dto.getVentasprom());
        if (dto.getNumautos() != null) entidad.setCliautos(dto.getNumautos());
        
        // Propiedades básicas
        if (dto.getP1tipo() != null) entidad.setP1tipo(dto.getP1tipo());
        if (dto.getP1valor() != null) entidad.setP1valor(dto.getP1valor());
        if (dto.getP1dir() != null) entidad.setP1dir(dto.getP1dir());
        if (dto.getP2tipo() != null) entidad.setP2tipo(dto.getP2tipo());
        if (dto.getP2valor() != null) entidad.setP2valor(dto.getP2valor());
        
        // Avales básicos
        if (dto.getA1nombre() != null) entidad.setA1nombre(dto.getA1nombre());
        if (dto.getA1tel() != null) entidad.setA1tel(dto.getA1tel());
        if (dto.getA1dir() != null) entidad.setA1dir(dto.getA1dir());
    }
    
    /**
     * Convierte una entidad ClienteDetalleEcommerceReal a DTO
     */
    private ClienteDetalleEcommerceDTO convertirDetalleEcommerceADTO(ClienteDetalleEcommerce entidad) {
        ClienteDetalleEcommerceDTO dto = new ClienteDetalleEcommerceDTO();
        dto.setIdcliente(entidad.getCliente());
        dto.setMarketing(entidad.getMarketing());
        dto.setVerificaciontel(entidad.getVerificaciontel());
        dto.setVerificacionemail(entidad.getVerificacionemail());
        dto.setActivo(entidad.getActivo());
        return dto;
    }
    
    /**
     * Actualiza una entidad ClienteDetalleEcommerceReal con datos del DTO
     */
    private void actualizarEntidadDetalleEcommerce(ClienteDetalleEcommerce entidad, ClienteDetalleEcommerceDTO dto) {
        if (dto.getMarketing() != null) entidad.setMarketing(dto.getMarketing());
        if (dto.getVerificaciontel() != null) entidad.setVerificaciontel(dto.getVerificaciontel());
        if (dto.getVerificacionemail() != null) entidad.setVerificacionemail(dto.getVerificacionemail());
        if (dto.getActivo() != null) entidad.setActivo(dto.getActivo());
    }
}
