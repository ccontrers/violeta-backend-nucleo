package com.lavioleta.desarrollo.violetaserver.repository;

import com.lavioleta.desarrollo.violetaserver.dto.request.ClienteRequest;
import com.lavioleta.desarrollo.violetaserver.dto.request.BajaClienteRequest;
import com.lavioleta.desarrollo.violetaserver.dto.response.ClienteResponse;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.jdbc.core.simple.JdbcClient;
import org.springframework.stereotype.Repository;
import org.springframework.transaction.annotation.Transactional;

import java.math.BigDecimal;
import java.time.LocalDate;
import java.util.*;

/**
 * Repository para operaciones CRUD del catálogo de clientes
 * Migrado de ClassServidorCatalogos.cpp - GrabaCliente/BajaCliente
 */
@Repository
public class CatalogoClientesRepository {
    
    private static final Logger logger = LoggerFactory.getLogger(CatalogoClientesRepository.class);
    private final JdbcClient jdbcClient;
    
    public CatalogoClientesRepository(JdbcClient jdbcClient) {
        this.jdbcClient = jdbcClient;
    }

    /**
     * Normaliza valores de tipo de empresa aceptando 0/1 y F/M durante transición.
     * Devuelve '0' para física, '1' para moral. Cualquier otro retorna tal cual para facilitar debug.
     */
    private String normalizarTipoEmpre(String raw) {
        if (raw == null) return null;
        return switch (raw.toUpperCase()) {
            case "0", "F" -> "0";
            case "1", "M" -> "1";
            default -> raw; // dejar pasar para detectar valores inesperados en logs
        };
    }

    private String nullSiVacio(String v) {
        return (v == null || v.isBlank()) ? null : v;
    }

    private String normalizarMetodoPago(String m) {
        if (m == null || m.isBlank()) return "01";
        return switch (m) {
            case "01","02","03","04","05","06","28","29" -> m; // catálogo típico (puede ampliarse)
            default -> "01"; // fallback seguro
        };
    }
    
    /**
     * Obtiene el ID de empresa asociado a una sucursal
     */
    public Integer obtenerEmpresaPorSucursal(String sucursal) {
        try {
            logger.debug("Buscando empresa para sucursal: {}", sucursal);
            String sql = """
                SELECT idempresa 
                FROM sucursales 
                WHERE sucursal = ? AND activa = 1
                """;
            
            Optional<Integer> resultado = jdbcClient.sql(sql)
                    .param(sucursal)
                    .query((rs, rowNum) -> rs.getInt("idempresa"))
                    .optional();
            
            if (resultado.isPresent()) {
                logger.debug("Empresa encontrada para sucursal {}: {}", sucursal, resultado.get());
                return resultado.get();
            } else {
                logger.warn("No se encontró empresa activa para sucursal {}, usando empresa por defecto: 1", sucursal);
                return 1; // Empresa por defecto si no encuentra
            }
                    
        } catch (Exception e) {
            logger.warn("Error al obtener empresa para sucursal {}: {}", sucursal, e.getMessage());
            return 1; // Empresa por defecto
        }
    }
    
    /**
     * Graba cliente (alta o modificación)
     * Equivalente a ServidorCatalogos::GrabaCliente
     */
    @Transactional(rollbackFor = Exception.class)
    public String grabaCliente(ClienteRequest request, String sucursal) {
        try {
            logger.info("Iniciando grabado de cliente. Operación: {}, Cliente: {}", 
                       request.getOperacion(), request.getCliente());
            
            String codigoCliente;
            
            if ("A".equals(request.getOperacion())) {
                // Alta - generar nuevo código
                codigoCliente = generarCodigoCliente(sucursal);
                insertarCliente(request, codigoCliente, sucursal);
            } else {
                // Modificación - usar código existente
                codigoCliente = request.getCliente();
                actualizarCliente(request, codigoCliente);
            }
            
            // Manejar teléfonos
            if (request.getTelefonos() != null && !request.getTelefonos().isEmpty()) {
                manejarTelefonos(codigoCliente, request.getTelefonos());
            }
            
            // Manejar direcciones de entrega
            if (request.getDireccionesEntrega() != null && !request.getDireccionesEntrega().isEmpty()) {
                manejarDireccionesEntrega(codigoCliente, request.getDireccionesEntrega());
            }
            
            // Manejar datos por empresa
            manejarDatosEmpresa(codigoCliente, request, sucursal);
            
            logger.info("Cliente grabado exitosamente: {}", codigoCliente);
            return codigoCliente;
            
        } catch (Exception e) {
            logger.error("Error al grabar cliente: {}", e.getMessage(), e);
            throw new RuntimeException("Error al grabar cliente: " + e.getMessage(), e);
        }
    }
    
    /**
     * Genera nuevo código de cliente usando folios
     */
    private String generarCodigoCliente(String sucursal) {
        try {
            // Obtener siguiente folio (equivalente a C++)
            String sql = """
                SELECT valor FROM foliosemp 
                WHERE folio = 'CLIENTES' AND sucursal = ? 
                FOR UPDATE
                """;
            
            Integer folioActual = jdbcClient.sql(sql)
                    .param(sucursal)
                    .query(Integer.class)
                    .single();
            
            Integer siguienteFolio = folioActual + 1;
            String codigoCliente = sucursal + String.format("%05d", folioActual);
            
            // Actualizar folio
            String sqlUpdate = """
                UPDATE foliosemp 
                SET valor = ? 
                WHERE folio = 'CLIENTES' AND sucursal = ?
                """;
            
            jdbcClient.sql(sqlUpdate)
                    .param(siguienteFolio)
                    .param(sucursal)
                    .update();
            
            logger.debug("Código de cliente generado: {}", codigoCliente);
            return codigoCliente;
            
        } catch (Exception e) {
            logger.error("Error al generar código de cliente: {}", e.getMessage());
            throw new RuntimeException("Error al generar código de cliente", e);
        }
    }
    
    /**
     * Inserta nuevo cliente en tabla clientes
     */
    /**
     * Inserta nuevo cliente con TODOS los campos - CORREGIDO
     */
    private void insertarCliente(ClienteRequest request, String codigoCliente, String sucursal) {
        String sql = """
            INSERT INTO clientes (
                cliente, nombre, appat, apmat, rsocial, nomnegocio, 
                rfc, curp, activo, fechaalta, fechamodi, sucursal,
                titulo, tipoempre, fnaccli, metododef, metodosup,
                digitosdef, digitossup, usocfdi, enviarcfd, valorsup,
                esparterelac, imprsaldos, agruparncre, esAsociado,
                forzarimprimirvertical, credMax, sgerencia, venxvol,
                contacto, contacfnac, calle, numext, numint, referenciadomic,
                colonia, cp, email, email2, medio, credito, limcred, plazo,
                excederlc, bloqueo, numpedidos, giro, canal, regimenfiscal,
                sociedadmercantil, sucremotarelacion, usuremotorelacion,
                ubicaciongis
            ) VALUES (
                ?, ?, ?, ?, ?, ?, ?, ?, ?, NOW(), NOW(), ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?,
                ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, 
                CASE WHEN ? IS NOT NULL AND ? IS NOT NULL THEN POINT(?, ?) ELSE NULL END
            )
            """;
        
        jdbcClient.sql(sql)
                .param(codigoCliente)
                .param(request.getNombre() != null ? request.getNombre() : "")
                .param(request.getAppat() != null ? request.getAppat() : "")
                .param(request.getApmat() != null ? request.getApmat() : "")
                .param(request.getRsocial() != null ? request.getRsocial() : "")
                .param(request.getNomnegocio() != null ? request.getNomnegocio() : "")
                .param(request.getRfc() != null ? request.getRfc() : "")
                .param(request.getCurp() != null ? request.getCurp() : "")
                .param(request.getActivo() != null ? (request.getActivo() ? 1 : 0) : 1)
                .param(sucursal)
                .param(request.getTitulo())
                .param(normalizarTipoEmpre(request.getTipoempre()))
                .param(request.getFnaccli())
                // Normalizar métodos de pago (si vienen valores inválidos caer a '01')
                .param(normalizarMetodoPago(request.getMetododef()))
                .param(normalizarMetodoPago(request.getMetodosup()))
                .param(request.getDigitosdef() != null ? request.getDigitosdef() : "0000")
                .param(request.getDigitossup() != null ? request.getDigitossup() : "0000")
                .param(request.getUsocfdi() != null ? request.getUsocfdi() : "G01")
                .param(request.getEnviarcfd() != null ? (request.getEnviarcfd() ? 1 : 0) : 0)
                .param(request.getValorsup() != null ? request.getValorsup() : BigDecimal.ZERO)
                .param(request.getEsparterelac() != null ? (request.getEsparterelac() ? 1 : 0) : 0)
                .param(request.getImprsaldos() != null ? (request.getImprsaldos() ? 1 : 0) : 1)
                .param(request.getAgruparncre() != null ? (request.getAgruparncre() ? 1 : 0) : 0)
                .param(request.getEsAsociado() != null ? (request.getEsAsociado() ? 1 : 0) : 0)
                .param(request.getForzarimprimirvertical() != null ? request.getForzarimprimirvertical() : 0)
                .param(request.getCredMax() != null ? request.getCredMax() : BigDecimal.ZERO)
                .param(request.getSgerencia() != null ? (request.getSgerencia() ? 1 : 0) : 0)
                .param(request.getVenxvol() != null ? (request.getVenxvol() ? 1 : 0) : 1)
                // CAMPOS FALTANTES QUE CAUSABAN LOS NULLS:
                .param(request.getContacto()) // contacto
                .param(request.getContacfnac()) // contacfnac
                .param(request.getCalle()) // calle
                .param(request.getNumext()) // numext
                .param(request.getNumint()) // numint
                .param(request.getReferenciadomic()) // referenciadomic
                .param(nullSiVacio(request.getColonia())) // colonia
                .param(request.getCp()) // cp
                .param(request.getEmail()) // email
                .param(request.getEmail2()) // email2
                .param(request.getMedio()) // medio
                .param(request.getCredito() != null ? (request.getCredito() ? 1 : 0) : 0) // credito
                .param(request.getLimcred() != null ? request.getLimcred() : BigDecimal.ZERO) // limcred
                .param(request.getPlazo()) // plazo
                .param(request.getExcederlc() != null ? (request.getExcederlc() ? 1 : 0) : 0) // excederlc
                .param(request.getBloqueo() != null ? request.getBloqueo() : "06") // bloqueo
                .param(request.getNumpedidos()) // numpedidos
                .param(nullSiVacio(request.getGiro())) // giro
                .param(nullSiVacio(request.getCanal())) // canal
                .param(nullSiVacio(request.getRegimenfiscal())) // regimenfiscal
                .param(nullSiVacio(request.getSociedadmercantil())) // sociedadmercantil
                .param(request.getSucremotarelacion()) // sucremotarelacion
                .param(request.getUsuremotorelacion()) // usuremotorelacion
                // Coordenadas GIS
                .param(request.getLatitud()) // Para verificar
                .param(request.getLongitud()) // Para verificar
                .param(request.getLatitud()) // Para POINT
                .param(request.getLongitud()) // Para POINT
                .update();
                
        logger.debug("Cliente insertado exitosamente con TODOS los campos: {}", codigoCliente);
    }
    
    /**
     * Actualiza cliente existente - SOLO campos que han cambiado
     */
    private void actualizarCliente(ClienteRequest request, String codigoCliente) {
        // Primero consultar el cliente actual para comparar
        String sqlConsulta = """
            SELECT titulo, plazo, numpedidos, digitosdef, digitossup 
            FROM clientes 
            WHERE cliente = ?
            """;
        
        Map<String, Object> clienteActual = jdbcClient.sql(sqlConsulta)
                .param(codigoCliente)
                .query()
                .singleRow();
        
        String sql = """
            UPDATE clientes SET 
                nombre = ?, appat = ?, apmat = ?, fnaccli = ?, titulo = ?, 
                rsocial = ?, nomnegocio = ?, contacto = ?, contacfnac = ?, 
                tipoempre = ?, calle = ?, numext = ?, numint = ?, referenciadomic = ?, 
                colonia = ?, cp = ?, 
                ubicaciongis = CASE WHEN ? IS NOT NULL AND ? IS NOT NULL THEN POINT(?, ?) ELSE ubicaciongis END,
                bloqueo = ?, credito = ?, fechamodi = ?, curp = ?, rfc = ?, 
                excederlc = ?, plazo = ?, limcred = ?, email = ?, email2 = ?, 
                medio = ?, activo = ?, sgerencia = ?, esparterelac = ?, venxvol = ?, 
                esAsociado = ?, giro = ?, canal = ?, regimenfiscal = ?, sociedadmercantil = ?,
                enviarcfd = ?, metododef = ?, metodosup = ?, digitosdef = ?, digitossup = ?, 
                usocfdi = ?, valorsup = ?, agruparncre = ?, forzarimprimirvertical = ?, 
                credMax = ?, sucremotarelacion = ?, usuremotorelacion = ?, imprsaldos = ?, 
                numpedidos = ?
            WHERE cliente = ?
            """;
        
        int updated = jdbcClient.sql(sql)
                .param(request.getNombre())
                .param(request.getAppat())
                .param(request.getApmat())
                .param(request.getFnaccli())
                // Preservar título si no viene en el request
                .param(request.getTitulo() != null ? request.getTitulo() : clienteActual.get("titulo"))
                .param(request.getRsocial())
                .param(request.getNomnegocio())
                .param(request.getContacto())
                .param(request.getContacfnac())
                .param(normalizarTipoEmpre(request.getTipoempre()))
                .param(request.getCalle())
                .param(request.getNumext())
                .param(request.getNumint())
                .param(request.getReferenciadomic())
                .param(nullSiVacio(request.getColonia()))
                .param(request.getCp())
                .param(request.getLatitud()) // Para verificar
                .param(request.getLongitud()) // Para verificar
                .param(request.getLatitud()) // Para POINT
                .param(request.getLongitud()) // Para POINT
                .param(request.getBloqueo())
                .param(request.getCredito())
                .param(LocalDate.now()) // fechamodi
                .param(request.getCurp())
                .param(request.getRfc())
                .param(request.getExcederlc())
                // Preservar plazo si no viene en el request o es 0
                .param(request.getPlazo() != null ? request.getPlazo() : clienteActual.get("plazo"))
                .param(request.getLimcred())
                .param(request.getEmail())
                .param(request.getEmail2())
                .param(request.getMedio())
                .param(request.getActivo())
                .param(request.getSgerencia())
                .param(request.getEsparterelac())
                .param(request.getVenxvol())
                .param(request.getEsAsociado())
                .param(nullSiVacio(request.getGiro()))
                .param(nullSiVacio(request.getCanal()))
                .param(nullSiVacio(request.getRegimenfiscal())) // CAMPO AGREGADO
                .param(nullSiVacio(request.getSociedadmercantil())) // CAMPO AGREGADO
                .param(request.getEnviarcfd())
                // Normalizar métodos de pago (si vienen valores inválidos caer a '01')
                .param(normalizarMetodoPago(request.getMetododef()))
                .param(normalizarMetodoPago(request.getMetodosup()))
                // Preservar valores originales si no se envían desde el frontend
                .param(request.getDigitosdef() != null ? request.getDigitosdef() : clienteActual.get("digitosdef"))
                .param(request.getDigitossup() != null ? request.getDigitossup() : clienteActual.get("digitossup"))
                .param(request.getUsocfdi() != null ? request.getUsocfdi() : "G01")
                .param(request.getValorsup() != null ? request.getValorsup() : BigDecimal.ZERO)
                .param(request.getAgruparncre() != null ? request.getAgruparncre() : false)
                .param(request.getForzarimprimirvertical() != null ? request.getForzarimprimirvertical() : 0)
                .param(request.getCredMax() != null ? request.getCredMax() : BigDecimal.ZERO)
                .param(request.getSucremotarelacion())
                .param(request.getUsuremotorelacion())
                .param(request.getImprsaldos() != null ? request.getImprsaldos() : true)
                // Preservar numpedidos si no viene en el request
                .param(request.getNumpedidos() != null ? request.getNumpedidos() : clienteActual.get("numpedidos"))
                .param(codigoCliente)
                .update();
        
        if (updated == 0) {
            throw new RuntimeException("Cliente no encontrado: " + codigoCliente);
        }
        
        logger.debug("Cliente actualizado preservando campos no modificados: {}", codigoCliente);
    }
    
    /**
     * Maneja operaciones CRUD en teléfonos del cliente
     */
    private void manejarTelefonos(String codigoCliente, List<ClienteRequest.TelefonoRequest> telefonos) {
        for (ClienteRequest.TelefonoRequest telefono : telefonos) {
            switch (telefono.getOperacion()) {
                case "A" -> insertarTelefono(codigoCliente, telefono);
                case "M" -> actualizarTelefono(codigoCliente, telefono);
                case "E" -> eliminarTelefono(codigoCliente, telefono);
            }
        }
        logger.debug("Teléfonos procesados para cliente: {}", codigoCliente);
    }
    
    private void insertarTelefono(String codigoCliente, ClienteRequest.TelefonoRequest telefono) {
        String sql = """
            INSERT INTO telefonosclientes (cliente, lada, telefono, tipo, extencionTel)
            VALUES (?, ?, ?, ?, ?)
            ON DUPLICATE KEY UPDATE
            tipo = VALUES(tipo), 
            extencionTel = VALUES(extencionTel)
            """;
        
        int rowsAffected = jdbcClient.sql(sql)
                .param(codigoCliente)
                .param(telefono.getLada())
                .param(telefono.getTelefono())
                .param(telefono.getTipo())
                .param(telefono.getExtencionTel())
                .update();
        
        if (rowsAffected == 1) {
            logger.debug("Teléfono insertado: {}-{}-{}", codigoCliente, telefono.getLada(), telefono.getTelefono());
        } else if (rowsAffected == 2) {
            logger.debug("Teléfono actualizado (duplicado): {}-{}-{}", codigoCliente, telefono.getLada(), telefono.getTelefono());
        }
    }
    
    private void actualizarTelefono(String codigoCliente, ClienteRequest.TelefonoRequest telefono) {
        String sql = """
            UPDATE telefonosclientes 
            SET tipo = ?, extencionTel = ?
            WHERE cliente = ? AND lada = ? AND telefono = ?
            """;
        
        jdbcClient.sql(sql)
                .param(telefono.getTipo())
                .param(telefono.getExtencionTel())
                .param(codigoCliente)
                .param(telefono.getLada())
                .param(telefono.getTelefono())
                .update();
    }
    
    private void eliminarTelefono(String codigoCliente, ClienteRequest.TelefonoRequest telefono) {
        String sql = """
            DELETE FROM telefonosclientes 
            WHERE cliente = ? AND lada = ? AND telefono = ?
            """;
        
        jdbcClient.sql(sql)
                .param(codigoCliente)
                .param(telefono.getLada())
                .param(telefono.getTelefono())
                .update();
    }
    
    /**
     * Modelo delta de direcciones:
     *  - 'A': insertar nueva (ignora id entrante y genera uno)
     *  - 'M': actualizar (si viene id válido) – actualmente no usado por UI pero soportado
     *  - 'E': eliminar (si viene id válido)
     *  Las direcciones existentes NO enviadas permanecen intactas.
     */
    private void manejarDireccionesEntrega(String codigoCliente, List<ClienteRequest.DireccionEntregaRequest> direcciones) {
        int ins=0, upd=0, del=0, skip=0;
        for (ClienteRequest.DireccionEntregaRequest dirReq : direcciones) {
            String op = dirReq.getOperacion();
            Integer id = dirReq.getIddireccion();
            switch (op) {
                case "A" -> { insertarDireccionEntrega(codigoCliente, dirReq); ins++; }
                case "M" -> {
                    if (id != null && id > 0) { actualizarDireccionEntrega(codigoCliente, dirReq); upd++; }
                    else { skip++; logger.debug("[DIR] Skip M sin id"); }
                }
                case "E" -> {
                    if (id != null && id > 0) { eliminarDireccionEntrega(codigoCliente, dirReq); del++; }
                    else { skip++; logger.debug("[DIR] Skip E sin id"); }
                }
                default -> { skip++; logger.warn("[DIR] Op desconocida {}", op); }
            }
        }
        logger.debug("[DIR] Delta aplicado cliente {} -> inserts={}, updates={}, deletes={}, skips={}", codigoCliente, ins, upd, del, skip);
    }

    private List<Integer> consultarIdsDireccionesEntrega(String codigoCliente) {
        String sql = "SELECT iddireccion FROM direccionesentregaclientes WHERE cliente = ? ORDER BY iddireccion";
        return jdbcClient.sql(sql)
                .param(codigoCliente)
                .query(Integer.class)
                .list();
    }
    
    private void insertarDireccionEntrega(String codigoCliente, ClienteRequest.DireccionEntregaRequest direccion) {
        // Siempre generar un nuevo id para evitar reutilizar IDs que el frontend envía accidentalmente
        Integer nuevoId = obtenerSiguienteIdDireccion(codigoCliente);
        logger.debug("Insertar dirección NUEVA para cliente {} -> asignado id {} (id recibido: {})", codigoCliente, nuevoId, direccion.getIddireccion());

        String sql = """
            INSERT INTO direccionesentregaclientes (
                cliente, iddireccion, dafault, calle, numext, numint,
                referenciadomic, colonia, cp, fechaalta, fechamodi
            ) VALUES (
                ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?
            )
            """;

        jdbcClient.sql(sql)
                .param(codigoCliente)
                .param(nuevoId)
                .param(direccion.getDafault())
                .param(direccion.getCalle())
                .param(direccion.getNumext())
                .param(direccion.getNumint())
                .param(direccion.getReferenciadomic())
                .param(direccion.getColonia())
                .param(direccion.getCp())
                .param(LocalDate.now())
                .param(LocalDate.now())
                .update();

        logger.debug("Dirección de entrega insertada OK: {}-{}", codigoCliente, nuevoId);
    }
    
    private Integer obtenerSiguienteIdDireccion(String codigoCliente) {
        String sql = """
            SELECT COALESCE(MAX(iddireccion), 0) + 1 
            FROM direccionesentregaclientes 
            WHERE cliente = ?
            """;
        
        return jdbcClient.sql(sql)
                .param(codigoCliente)
                .query(Integer.class)
                .single();
    }
    
    private void actualizarDireccionEntrega(String codigoCliente, ClienteRequest.DireccionEntregaRequest direccion) {
        String sql = """
            UPDATE direccionesentregaclientes SET 
                dafault = ?, calle = ?, numext = ?, numint = ?, referenciadomic = ?, 
                colonia = ?, cp = ?, fechamodi = ?
            WHERE cliente = ? AND iddireccion = ?
            """;
        
        jdbcClient.sql(sql)
                .param(direccion.getDafault())
                .param(direccion.getCalle())
                .param(direccion.getNumext())
                .param(direccion.getNumint())
                .param(direccion.getReferenciadomic())
                .param(direccion.getColonia())
                .param(direccion.getCp())
                .param(LocalDate.now()) // fechamodi
                .param(codigoCliente)
                .param(direccion.getIddireccion())
                .update();
        
        logger.debug("Dirección de entrega actualizada: {}-{}", codigoCliente, direccion.getIddireccion());
    }
    
    private void eliminarDireccionEntrega(String codigoCliente, ClienteRequest.DireccionEntregaRequest direccion) {
        String sql = """
            DELETE FROM direccionesentregaclientes 
            WHERE cliente = ? AND iddireccion = ?
            """;
        
        int deleted = jdbcClient.sql(sql)
                .param(codigoCliente)
                .param(direccion.getIddireccion())
                .update();
        
        logger.debug("Dirección de entrega eliminada: {}-{} (rows affected: {})", codigoCliente, direccion.getIddireccion(), deleted);
    }
    
    /**
     * Maneja datos específicos por empresa
     */
    private void manejarDatosEmpresa(String codigoCliente, ClienteRequest request, String sucursal) {
        // TEMPORAL: Por ahora solo insertar para la empresa actual para debug
        if ("A".equals(request.getOperacion())) {
            // Alta: insertar solo para la empresa actual
            insertarDatosEmpresa(codigoCliente, request.getIdEmpresa(),
                               request.getVendedor() != null ? request.getVendedor() : "ADVV",
                               request.getCobrador() != null ? request.getCobrador() : "ADRIA",
                               request.getTipoprec() != null ? request.getTipoprec() : "01",
                               request.getTipoprecmin() != null ? request.getTipoprecmin() : "01");
        } else {
            // Modificación: solo actualizar la empresa actual
            upsertDatosEmpresa(codigoCliente, request.getIdEmpresa(), 
                              request.getVendedor(), request.getCobrador(), 
                              request.getTipoprec(), request.getTipoprecmin());
            
            // Datos de otras empresas si se especifican
            if (request.getDatosEmpresas() != null) {
                for (ClienteRequest.DatosEmpresaRequest datos : request.getDatosEmpresas()) {
                    if (datos.getActualizar()) {
                        upsertDatosEmpresa(codigoCliente, datos.getIdempresa(),
                                         datos.getVendedor(), datos.getCobrador(),
                                         datos.getTipoprec(), datos.getTipoprecmin());
                    }
                }
            }
        }
        
        logger.debug("Datos de empresa procesados para cliente: {}", codigoCliente);
    }
    
    private void insertarDatosEmpresaConDefaults(String codigoCliente, Integer idEmpresa) {
        // Obtener valores por defecto como hace el C++
        String sqlDefaults = """
            SELECT 
                COALESCE((SELECT valor FROM parametrosglobemp WHERE parametro='TIPOPREDF' AND idempresa=?), '01') as tipoprec,
                COALESCE((SELECT valor FROM parametrosglobemp WHERE parametro='DEFAVEND' AND idempresa=?), '01') as vendedor,
                COALESCE((SELECT valor FROM parametrosglobemp WHERE parametro='DEFACOBR' AND idempresa=?), '01') as cobrador
            """;
            
        var defaults = jdbcClient.sql(sqlDefaults)
                .param(idEmpresa)
                .param(idEmpresa)
                .param(idEmpresa)
                .query((rs, rowNum) -> Map.of(
                    "tipoprec", rs.getString("tipoprec"),
                    "vendedor", rs.getString("vendedor"),
                    "cobrador", rs.getString("cobrador")
                ))
                .single();
        
        insertarDatosEmpresa(codigoCliente, idEmpresa,
                           defaults.get("vendedor"),
                           defaults.get("cobrador"),
                           defaults.get("tipoprec"),
                           defaults.get("tipoprec")); // tipoprecmin = tipoprec por defecto
    }
    
    private void insertarDatosEmpresa(String codigoCliente, Integer idEmpresa,
                                    String vendedor, String cobrador,
                                    String tipoprec, String tipoprecmin) {
        String sqlInsert = """
            INSERT INTO clientesemp (cliente, idempresa, vendedor, cobrador, tipoprec, tipoprecmin)
            VALUES (?, ?, ?, ?, ?, ?)
            """;
        
        jdbcClient.sql(sqlInsert)
                .param(codigoCliente)
                .param(idEmpresa)
                .param(vendedor)
                .param(cobrador)
                .param(tipoprec)
                .param(tipoprecmin)
                .update();
                
        logger.debug("Datos insertados en clientesemp para empresa: {}", idEmpresa);
    }
    
    private void upsertDatosEmpresa(String codigoCliente, Integer idEmpresa, 
                                   String vendedor, String cobrador, 
                                   String tipoprec, String tipoprecmin) {
        // Verificar si existe
        String sqlCheck = """
            SELECT COUNT(*) FROM clientesemp 
            WHERE cliente = ? AND idempresa = ?
            """;
        
        Integer count = jdbcClient.sql(sqlCheck)
                .param(codigoCliente)
                .param(idEmpresa)
                .query(Integer.class)
                .single();
        
        if (count > 0) {
            // Actualizar
            String sqlUpdate = """
                UPDATE clientesemp SET 
                    vendedor = ?, cobrador = ?, tipoprec = ?, tipoprecmin = ?
                WHERE cliente = ? AND idempresa = ?
                """;
            
            jdbcClient.sql(sqlUpdate)
                    .param(nullSiVacio(vendedor))
                    .param(nullSiVacio(cobrador))
                    .param(nullSiVacio(tipoprec))
                    .param(nullSiVacio(tipoprecmin))
                    .param(codigoCliente)
                    .param(idEmpresa)
                    .update();
        } else {
            // Insertar
            String sqlInsert = """
                INSERT INTO clientesemp (cliente, idempresa, vendedor, cobrador, tipoprec, tipoprecmin)
                VALUES (?, ?, ?, ?, ?, ?)
                """;
            
            jdbcClient.sql(sqlInsert)
                    .param(codigoCliente)
                    .param(idEmpresa)
                    .param(nullSiVacio(vendedor))
                    .param(nullSiVacio(cobrador))
                    .param(nullSiVacio(tipoprec))
                    .param(nullSiVacio(tipoprecmin))
                    .update();
        }
    }
    
    /**
     * Baja cliente y todos sus datos relacionados
     * Equivalente a ServidorCatalogos::BajaCliente
     */
    @Transactional(rollbackFor = Exception.class)
    public void bajaCliente(BajaClienteRequest request) {
        try {
            logger.info("Iniciando baja de cliente: {}", request.getCliente());
            
            // Verificar que el cliente existe
            if (!existeCliente(request.getCliente())) {
                throw new RuntimeException("Cliente no encontrado: " + request.getCliente());
            }
            
            // Eliminar en orden debido a foreign keys (mismo orden que C++)
            eliminarTelefonosCliente(request.getCliente());
            eliminarDireccionesEntregaCliente(request.getCliente());
            eliminarDatosEmpresaCliente(request.getCliente());
            eliminarCliente(request.getCliente());
            
            logger.info("Cliente eliminado exitosamente: {}", request.getCliente());
            
        } catch (Exception e) {
            logger.error("Error al eliminar cliente {}: {}", request.getCliente(), e.getMessage(), e);
            throw new RuntimeException("Error al eliminar cliente: " + e.getMessage(), e);
        }
    }
    
    private boolean existeCliente(String codigoCliente) {
        String sql = "SELECT COUNT(*) FROM clientes WHERE cliente = ?";
        Integer count = jdbcClient.sql(sql)
                .param(codigoCliente)
                .query(Integer.class)
                .single();
        return count > 0;
    }
    
    private void eliminarTelefonosCliente(String codigoCliente) {
        String sql = "DELETE FROM telefonosclientes WHERE cliente = ?";
        int deleted = jdbcClient.sql(sql)
                .param(codigoCliente)
                .update();
        logger.debug("Eliminados {} teléfonos del cliente: {}", deleted, codigoCliente);
    }
    
    private void eliminarDireccionesEntregaCliente(String codigoCliente) {
        String sql = "DELETE FROM direccionesentregaclientes WHERE cliente = ?";
        int deleted = jdbcClient.sql(sql)
                .param(codigoCliente)
                .update();
        logger.debug("Eliminadas {} direcciones de entrega del cliente: {}", deleted, codigoCliente);
    }
    
    private void eliminarDatosEmpresaCliente(String codigoCliente) {
        String sql = "DELETE FROM clientesemp WHERE cliente = ?";
        int deleted = jdbcClient.sql(sql)
                .param(codigoCliente)
                .update();
        logger.debug("Eliminados {} registros de clientesemp del cliente: {}", deleted, codigoCliente);
    }
    
    private void eliminarCliente(String codigoCliente) {
        String sql = "DELETE FROM clientes WHERE cliente = ?";
        int deleted = jdbcClient.sql(sql)
                .param(codigoCliente)
                .update();
        
        if (deleted == 0) {
            throw new RuntimeException("No se pudo eliminar el cliente: " + codigoCliente);
        }
        
        logger.debug("Cliente eliminado de tabla clientes: {}", codigoCliente);
    }
    
    /**
     * Consulta un cliente completo con todas sus relaciones
     */
    public Optional<ClienteResponse.ClienteCompleto> consultarCliente(String codigoCliente, Integer idEmpresa) {
        try {
            logger.debug("Consultando cliente: {} para empresa: {}", codigoCliente, idEmpresa);
            
            // Consulta principal del cliente - CORREGIDA con TODOS los campos
            String sql = """
                SELECT c.*, co.nombre as colonia_nombre,
                       g.nombre as giro_nombre, can.descripcion as canal_nombre,
                       rf.descripcion as regimenfiscal_nombre,
                       sm.descripcion as sociedadmercantil_nombre
                FROM clientes c
                LEFT JOIN colonias co ON c.colonia = co.colonia
                LEFT JOIN gironegocio g ON c.giro = g.giro
                LEFT JOIN canalesclientes can ON c.canal = can.canal
                LEFT JOIN cregimenfiscal rf ON c.regimenfiscal = rf.regimenfiscal
                LEFT JOIN catsociedadesmercantiles sm ON c.sociedadmercantil = sm.id
                WHERE c.cliente = ?
                """;
            
            Optional<ClienteResponse.ClienteCompleto> cliente = jdbcClient.sql(sql)
                    .param(codigoCliente)
                    .query((rs, rowNum) -> {
                        return ClienteResponse.ClienteCompleto.builder()
                                .cliente(rs.getString("cliente"))
                                .nombre(rs.getString("nombre"))
                                .appat(rs.getString("appat"))
                                .apmat(rs.getString("apmat"))
                                .sucursal(rs.getString("sucursal"))
                                .fnaccli(rs.getObject("fnaccli", LocalDate.class))
                                .titulo(rs.getString("titulo"))
                                .rsocial(rs.getString("rsocial"))
                                .nomnegocio(rs.getString("nomnegocio"))
                                .contacto(rs.getString("contacto"))
                                .contacfnac(rs.getObject("contacfnac", LocalDate.class))
                                .tipoempre(rs.getString("tipoempre"))
                                .rfc(rs.getString("rfc"))
                                .curp(rs.getString("curp"))
                                .activo(rs.getBoolean("activo"))
                                .sgerencia(rs.getBoolean("sgerencia"))
                                .esparterelac(rs.getBoolean("esparterelac"))
                                .venxvol(rs.getBoolean("venxvol"))
                                .esAsociado(rs.getBoolean("esAsociado"))
                                .giro(rs.getString("giro"))
                                .giroNombre(rs.getString("giro_nombre"))
                                .canal(rs.getString("canal"))
                                .canalNombre(rs.getString("canal_nombre"))
                                .calle(rs.getString("calle"))
                                .numext(rs.getString("numext"))
                                .numint(rs.getString("numint"))
                                .referenciadomic(rs.getString("referenciadomic"))
                                .colonia(rs.getString("colonia"))
                                .coloniaNombre(rs.getString("colonia_nombre"))
                                .cp(rs.getString("cp"))
                                // Coordenadas GIS - extraer de POINT
                                .latitud(extraerLatitudDePoint(rs.getString("ubicaciongis")))
                                .longitud(extraerLongitudDePoint(rs.getString("ubicaciongis")))
                                .email(rs.getString("email"))
                                .email2(rs.getString("email2"))
                                .medio(rs.getString("medio"))
                                .credito(rs.getBoolean("credito"))
                                .limcred(rs.getBigDecimal("limcred"))
                                .plazo(rs.getInt("plazo"))
                                .excederlc(rs.getBoolean("excederlc"))
                                .bloqueo(rs.getString("bloqueo"))
                                .imprsaldos(rs.getBoolean("imprsaldos"))
                                .numpedidos(rs.getString("numpedidos"))
                                // .comentcr() - Campo no existe en la tabla actual
                                .enviarcfd(rs.getBoolean("enviarcfd"))
                                .metododef(rs.getString("metododef"))
                                .digitosdef(rs.getString("digitosdef"))
                                .usocfdi(rs.getString("usocfdi"))
                                .forzarimprimirvertical(rs.getString("forzarimprimirvertical"))
                                .sucremotarelacion(rs.getString("sucremotarelacion"))
                                .usuremotorelacion(rs.getString("usuremotorelacion"))
                                .fechaalta(rs.getObject("fechaalta", LocalDate.class))
                                .fechamodi(rs.getObject("fechamodi", LocalDate.class))
                                .fechabloq(rs.getObject("fechabloq", LocalDate.class))
                                .fechauven(rs.getObject("fechauven", LocalDate.class))
                                // CAMPOS AGREGADOS QUE FALTABAN:
                                .regimenfiscal(rs.getString("regimenfiscal"))
                                .regimenfiscalNombre(rs.getString("regimenfiscal_nombre"))
                                .sociedadmercantil(rs.getString("sociedadmercantil"))
                                .sociedadmercantilNombre(rs.getString("sociedadmercantil_nombre"))
                                .valorsup(rs.getBigDecimal("valorsup"))
                                .metodosup(rs.getString("metodosup"))
                                .digitossup(rs.getString("digitossup"))
                                .credMax(rs.getBigDecimal("credMax"))
                                .agruparncre(rs.getBoolean("agruparncre"))
                                .build();
                    })
                    .optional();
            
            if (cliente.isPresent()) {
                ClienteResponse.ClienteCompleto clienteCompleto = cliente.get();
                
                // Cargar teléfonos
                clienteCompleto.setTelefonos(consultarTelefonos(codigoCliente));
                
                // Cargar direcciones de entrega
                clienteCompleto.setDireccionesEntrega(consultarDireccionesEntrega(codigoCliente));
                
                // Cargar datos de empresas
                clienteCompleto.setDatosEmpresas(consultarDatosEmpresas(codigoCliente));
                
                // Cargar empresas disponibles
                clienteCompleto.setEmpresasDisponibles(consultarEmpresasDisponibles());
                
                return Optional.of(clienteCompleto);
            }
            
            return Optional.empty();
            
        } catch (Exception e) {
            logger.error("Error al consultar cliente {}: {}", codigoCliente, e.getMessage(), e);
            throw new RuntimeException("Error al consultar cliente", e);
        }
    }
    
    private List<ClienteResponse.TelefonoCliente> consultarTelefonos(String codigoCliente) {
        String sql = """
            SELECT t.tipo, t.lada, t.telefono, t.extencionTel,
                   t.tipo as tipo_nombre
            FROM telefonosclientes t
            WHERE t.cliente = ?
            ORDER BY t.tipo, t.lada, t.telefono
            """;
        
    List<ClienteResponse.TelefonoCliente> lista = jdbcClient.sql(sql)
        .param(codigoCliente)
        .query((rs, rowNum) -> {
            String tipo = rs.getString("tipo");
            String lada = rs.getString("lada");
            String tel = rs.getString("telefono");
            String ext = rs.getString("extencionTel");
            if (logger.isDebugEnabled()) {
            logger.debug("Telefono consultado cliente={} tipo={} lada={} tel={} extencionTel='{}' (len={})", codigoCliente, tipo, lada, tel, ext, (ext==null?"null":ext.length()));
            }
            return ClienteResponse.TelefonoCliente.builder()
                .tipo(tipo)
                .tipoNombre(rs.getString("tipo_nombre"))
                .lada(lada)
                .telefono(tel)
                .extencionTel(ext)
                .build();
        })
        .list();
    return lista;
    }
    
    private List<ClienteResponse.DireccionEntrega> consultarDireccionesEntrega(String codigoCliente) {
        String sql = """
            SELECT d.*, c.nombre as colonia_nombre
            FROM direccionesentregaclientes d
            LEFT JOIN colonias c ON d.colonia = c.colonia
            WHERE d.cliente = ?
            ORDER BY d.iddireccion
            """;
        
        return jdbcClient.sql(sql)
                .param(codigoCliente)
                .query((rs, rowNum) -> ClienteResponse.DireccionEntrega.builder()
                        .iddireccion(rs.getInt("iddireccion"))
                        .dafault(rs.getBoolean("dafault"))
                        .calle(rs.getString("calle"))
                        .numext(rs.getString("numext"))
                        .numint(rs.getString("numint"))
                        .referenciadomic(rs.getString("referenciadomic"))
                        .colonia(rs.getString("colonia"))
                        .coloniaNombre(rs.getString("colonia_nombre"))
                        .cp(rs.getString("cp"))
                        .fechaalta(rs.getObject("fechaalta", LocalDate.class))
                        .fechamodi(rs.getObject("fechamodi", LocalDate.class))
                        .build())
                .list();
    }
    
    private List<ClienteResponse.DatosEmpresa> consultarDatosEmpresas(String codigoCliente) {
        String sql = """
            SELECT ce.*, e.nombre as nombre_empresa, e.clave as clave_empresa,
                   CONCAT(ev.nombre, ' ', ev.appat, ' ', ev.apmat) as vendedor_nombre,
                   CONCAT(ec.nombre, ' ', ec.appat, ' ', ec.apmat) as cobrador_nombre
            FROM clientesemp ce
            INNER JOIN empresas e ON ce.idempresa = e.idempresa
            LEFT JOIN vendedores v ON ce.vendedor = v.empleado
            LEFT JOIN empleados ev ON v.empleado = ev.empleado
            LEFT JOIN cobradores co ON ce.cobrador = co.empleado
            LEFT JOIN empleados ec ON co.empleado = ec.empleado
            WHERE ce.cliente = ?
            ORDER BY e.nombre
            """;
        
        return jdbcClient.sql(sql)
                .param(codigoCliente)
                .query((rs, rowNum) -> ClienteResponse.DatosEmpresa.builder()
                        .idempresa(rs.getInt("idempresa"))
                        .nombreEmpresa(rs.getString("nombre_empresa"))
                        .claveEmpresa(rs.getString("clave_empresa"))
                        .vendedor(rs.getString("vendedor"))
                        .vendedorNombre(rs.getString("vendedor_nombre"))
                        .cobrador(rs.getString("cobrador"))
                        .cobradorNombre(rs.getString("cobrador_nombre"))
                        .tipoprec(rs.getString("tipoprec"))
                        .tipoprecmin(rs.getString("tipoprecmin"))
                        .build())
                .list();
    }
    
    private List<ClienteResponse.EmpresaDisponible> consultarEmpresasDisponibles() {
        String sql = """
            SELECT idempresa, clave, nombre, sucprincipal, essuper
            FROM empresas
            ORDER BY nombre
            """;
        
        return jdbcClient.sql(sql)
                .query((rs, rowNum) -> ClienteResponse.EmpresaDisponible.builder()
                        .idempresa(rs.getInt("idempresa"))
                        .clave(rs.getString("clave"))
                        .nombre(rs.getString("nombre"))
                        .sucprincipal(rs.getString("sucprincipal"))
                        .essuper(rs.getBoolean("essuper"))
                        .tieneConfiguracion(false) // Se actualiza después si es necesario
                        .build())
                .list();
    }
    
    /**
     * Extrae la latitud de un campo POINT de MySQL
     */
    private Double extraerLatitudDePoint(String pointStr) {
        if (pointStr == null || pointStr.isEmpty()) {
            return null;
        }
        try {
            // Formato: POINT(longitud latitud)
            String coordenadas = pointStr.replace("POINT(", "").replace(")", "");
            String[] partes = coordenadas.split(" ");
            if (partes.length >= 2) {
                return Double.parseDouble(partes[1]); // latitud es la segunda parte
            }
        } catch (Exception e) {
            logger.warn("Error al extraer latitud de POINT: {}", pointStr);
        }
        return null;
    }
    
    /**
     * Extrae la longitud de un campo POINT de MySQL
     */
    private Double extraerLongitudDePoint(String pointStr) {
        if (pointStr == null || pointStr.isEmpty()) {
            return null;
        }
        try {
            // Formato: POINT(longitud latitud)
            String coordenadas = pointStr.replace("POINT(", "").replace(")", "");
            String[] partes = coordenadas.split(" ");
            if (partes.length >= 2) {
                return Double.parseDouble(partes[0]); // longitud es la primera parte
            }
        } catch (Exception e) {
            logger.warn("Error al extraer longitud de POINT: {}", pointStr);
        }
        return null;
    }
}
