package com.lavioleta.desarrollo.violetaserver.service;

import com.lavioleta.desarrollo.violetaserver.dto.request.ProveedorRequest;
import com.lavioleta.desarrollo.violetaserver.dto.request.TelefonoProveedorDTO;
import com.lavioleta.desarrollo.violetaserver.dto.request.CondicionComercialProveedorDTO;
import com.lavioleta.desarrollo.violetaserver.dto.request.CuentaRetencionProveedorDTO;
import com.lavioleta.desarrollo.violetaserver.dto.response.ProveedorResponse;
import com.lavioleta.desarrollo.violetaserver.dto.response.TelefonoProveedorResponse;
import com.lavioleta.desarrollo.violetaserver.dto.response.CondicionComercialProveedorResponse;
import com.lavioleta.desarrollo.violetaserver.dto.response.CuentaRetencionProveedorResponse;
import com.lavioleta.desarrollo.violetaserver.entity.Proveedor;
import com.lavioleta.desarrollo.violetaserver.entity.TelefonoProveedor;
import com.lavioleta.desarrollo.violetaserver.entity.CondicionComercialProveedor;
import com.lavioleta.desarrollo.violetaserver.entity.CuentaRetencionProveedor;
import com.lavioleta.desarrollo.violetaserver.entity.FolioEmp;
import com.lavioleta.desarrollo.violetaserver.repository.ProveedorRepository;
import com.lavioleta.desarrollo.violetaserver.repository.TelefonoProveedorRepository;
import com.lavioleta.desarrollo.violetaserver.repository.CondicionComercialProveedorRepository;
import com.lavioleta.desarrollo.violetaserver.repository.CuentaRetencionProveedorRepository;
import com.lavioleta.desarrollo.violetaserver.repository.FolioEmpRepository;

import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.data.domain.Page;
import org.springframework.data.domain.PageImpl;
import org.springframework.data.domain.Pageable;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;
import org.springframework.util.StringUtils;

import java.time.LocalDateTime;
import java.util.List;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * Servicio para el catálogo de proveedores
 * Migrado de ServidorCatalogos.cpp: ConsultaProveedor, GrabaProveedor, BajaProveedor
 */
@Service
@Slf4j
@RequiredArgsConstructor
public class CatalogoProveedoresService {

    private final ProveedorRepository proveedorRepository;
    private final TelefonoProveedorRepository telefonoProveedorRepository;
    private final CondicionComercialProveedorRepository condicionComercialProveedorRepository;
    private final CuentaRetencionProveedorRepository cuentaRetencionProveedorRepository;
    private final FolioEmpRepository folioEmpRepository;
    
    @Value("${violeta.sucursal:S1}")
    private String claveSucursal;


    /**
     * Consulta todos los proveedores activos con paginación
     * Migrado de: ServidorCatalogos::ConsultaProveedor()
     */
    @Transactional(readOnly = true)
    public Page<ProveedorResponse> findAllProveedores(Pageable pageable) {
        log.debug("Consultando todos los proveedores activos - Página: {}, Tamaño: {}", 
                 pageable.getPageNumber(), pageable.getPageSize());
        
        Page<Proveedor> proveedores = proveedorRepository.findByActivoTrueOrderByRazonsocial(pageable);
        
        List<ProveedorResponse> responseList = proveedores.getContent().stream()
                .map(this::convertToResponse)
                .collect(Collectors.toList());
        
        return new PageImpl<>(responseList, pageable, proveedores.getTotalElements());
    }

    /**
     * Busca proveedores por razón social con paginación
     */
    @Transactional(readOnly = true)
    public Page<ProveedorResponse> findByRazonSocial(String razonSocial, Pageable pageable) {
        log.debug("Buscando proveedores por razón social: {}", razonSocial);
        
        if (StringUtils.hasText(razonSocial)) {
            Page<Proveedor> proveedores = proveedorRepository.findByRazonsocialContainingIgnoreCaseAndActivoTrue(razonSocial, pageable);
            
            List<ProveedorResponse> responseList = proveedores.getContent().stream()
                    .map(this::convertToResponse)
                    .collect(Collectors.toList());
            
            return new PageImpl<>(responseList, pageable, proveedores.getTotalElements());
        } else {
            return findAllProveedores(pageable);
        }
    }

    /**
     * Busca proveedores por múltiples criterios
     */
    @Transactional(readOnly = true)
    public Page<ProveedorResponse> findByCriteriosMultiples(String razonSocial, String rfc, 
                                                           String estado, String comprador, 
                                                           String tipoProveedor, Pageable pageable) {
        log.debug("Búsqueda múltiple - Razón social: {}, RFC: {}, Estado: {}, Comprador: {}, Tipo: {}", 
                 razonSocial, rfc, estado, comprador, tipoProveedor);
        
        Page<Proveedor> proveedores = proveedorRepository.findByCriteriosMultiples(
                razonSocial, rfc, estado, comprador, tipoProveedor, pageable);
        
        List<ProveedorResponse> responseList = proveedores.getContent().stream()
                .map(this::convertToResponse)
                .collect(Collectors.toList());
        
        return new PageImpl<>(responseList, pageable, proveedores.getTotalElements());
    }

    /**
     * Busca un proveedor por su código
     */
    @Transactional(readOnly = true)
    public ProveedorResponse findByProveedor(String proveedor) {
        log.debug("Consultando proveedor por código: {}", proveedor);
        
        Optional<Proveedor> proveedorOpt = proveedorRepository.findById(proveedor);
        if (proveedorOpt.isPresent() && proveedorOpt.get().getActivo()) {
            return convertToResponse(proveedorOpt.get());
        }
        
        throw new RuntimeException("Proveedor no encontrado o inactivo: " + proveedor);
    }

    /**
     * Graba un proveedor (alta o modificación)
     * Migrado de: ServidorCatalogos::GrabaProveedor()
     */
    @Transactional
    public ProveedorResponse grabaProveedor(ProveedorRequest request, String usuario) {
        log.debug("Grabando proveedor: {}", request.getProveedor());
        
        boolean esAlta = !StringUtils.hasText(request.getProveedor());
        Proveedor proveedor;
        String rfcOriginal = null;
        String curpOriginal = null;
        
        if (esAlta) {
            // Alta de nuevo proveedor
            String nuevoCodigoProveedor = generarCodigoProveedor();
            proveedor = new Proveedor();
            proveedor.setProveedor(nuevoCodigoProveedor);
            proveedor.setActivo(true);
            proveedor.setUsualta(usuario);
            log.info("Alta de nuevo proveedor con código: {}", nuevoCodigoProveedor);
        } else {
            // Modificación de proveedor existente
            Optional<Proveedor> proveedorOpt = proveedorRepository.findById(request.getProveedor());
            if (!proveedorOpt.isPresent()) {
                throw new RuntimeException("Proveedor no encontrado: " + request.getProveedor());
            }
            proveedor = proveedorOpt.get();
            // Guardar RFC y CURP originales para validación
            rfcOriginal = proveedor.getRfc();
            curpOriginal = proveedor.getCurp();
        }
        
        // Validar datos antes de grabar
        validarDatosProveedor(request, proveedor.getProveedor(), rfcOriginal, curpOriginal);
        
        // Mapear datos básicos
        mapearDatosBasicos(request, proveedor);
        
        // Configurar campos de auditoría
        proveedor.setUsumodi(usuario);
        
        // Guardar proveedor principal
        proveedor = proveedorRepository.save(proveedor);
        
        // Procesar teléfonos
        if (request.getTelefonos() != null) {
            procesarTelefonos(proveedor, request.getTelefonos(), usuario);
        }
        
        // TODO: Procesar otras relaciones cuando los DTOs estén disponibles
        // procesarCondicionesComerciales(proveedor, request.getCondicionesComerciales(), usuario);
        // procesarCuentasRetencion(proveedor, request.getCuentasRetencion(), usuario);
        
        log.info("Proveedor grabado exitosamente: {}", proveedor.getProveedor());
        return convertToResponse(proveedor);
    }

    /**
     * Baja lógica de un proveedor
     * Migrado de: ServidorCatalogos::BajaProveedor()
     */
    @Transactional
    public void bajaProveedor(String codigoProveedor, String usuario) {
        log.debug("Baja de proveedor: {}", codigoProveedor);
        
        Optional<Proveedor> proveedorOpt = proveedorRepository.findById(codigoProveedor);
        if (!proveedorOpt.isPresent()) {
            throw new RuntimeException("Proveedor no encontrado: " + codigoProveedor);
        }
        
        Proveedor proveedor = proveedorOpt.get();
        
        // Verificar si se puede dar de baja (validaciones de negocio)
        validarBajaProveedor(proveedor);
        
        // Baja lógica
        proveedor.setActivo(false);
        proveedor.setUsumodi(usuario);
        
        proveedorRepository.save(proveedor);
        
        log.info("Proveedor dado de baja: {}", codigoProveedor);
    }

    /**
     * Genera un nuevo código de proveedor único
     * Migrado de: ServidorCatalogos::GrabaProveedor() 
     * Patrón: {CLAVE_SUCURSAL}{FOLIO_6_DIGITOS}
     * Ejemplo: S1000848
     */
    private String generarCodigoProveedor() {
        // Obtener y bloquear el folio de la sucursal para evitar duplicados
        FolioEmp folioEmp = folioEmpRepository.findByFolioAndSucursalWithLock("PROV", claveSucursal)
            .orElseThrow(() -> new RuntimeException("No existe folio PROV para la sucursal: " + claveSucursal));
        
        // Obtener el valor actual del folio
        Integer folioActual = folioEmp.getValor();
        
        // Generar código: prefijo sucursal + folio con 6 dígitos (padding con ceros)
        String nuevoCodigo = String.format("%s%06d", claveSucursal, folioActual);
        
        // Incrementar el folio para el siguiente proveedor
        folioEmpRepository.incrementarValor("PROV", claveSucursal);
        
        log.info("Código de proveedor generado: {}", nuevoCodigo);
        return nuevoCodigo;
    }

    /**
     * Valida los datos del proveedor antes de grabarlo
     */
    private void validarDatosProveedor(ProveedorRequest request, String proveedorActual, String rfcOriginal, String curpOriginal) {
        log.debug("Validando proveedor - Actual: {}, RFC: {} (Original: {}), CURP: {} (Original: {})", 
                  proveedorActual, request.getRfc(), rfcOriginal, request.getCurp(), curpOriginal);
        
        // Validar campos obligatorios
        if (!StringUtils.hasText(request.getRazonsocial())) {
            throw new RuntimeException("La razón social es obligatoria");
        }
        
        if (!StringUtils.hasText(request.getRfc())) {
            throw new RuntimeException("El RFC es obligatorio");
        }
        
        // Validar formato de RFC
        if (!isValidRFC(request.getRfc())) {
            throw new RuntimeException("El formato del RFC no es válido");
        }
        
        // Validar unicidad de RFC solo si cambió o es alta
        if (StringUtils.hasText(request.getRfc())) {
            boolean rfcCambio = rfcOriginal == null || !request.getRfc().equalsIgnoreCase(rfcOriginal);
            if (rfcCambio) {
                boolean existeRfc = proveedorRepository.existsByRfcAndProveedorNot(request.getRfc(), proveedorActual);
                log.debug("Validación RFC - Cambió: {}, Existe: {}, RFC buscado: {}, RFC original: {}", 
                          rfcCambio, existeRfc, request.getRfc(), rfcOriginal);
                if (existeRfc) {
                    throw new RuntimeException("Ya existe un proveedor con el RFC: " + request.getRfc());
                }
            } else {
                log.debug("RFC no cambió, se omite validación de duplicados");
            }
        }
        
        // Validar unicidad de CURP solo si cambió o es alta
        if (StringUtils.hasText(request.getCurp())) {
            boolean curpCambio = curpOriginal == null || !request.getCurp().equalsIgnoreCase(curpOriginal);
            if (curpCambio) {
                boolean existeCurp = proveedorRepository.existsByCurpAndProveedorNot(request.getCurp(), proveedorActual);
                log.debug("Validación CURP - Cambió: {}, Existe: {}, CURP buscada: {}, CURP original: {}", 
                          curpCambio, existeCurp, request.getCurp(), curpOriginal);
                if (existeCurp) {
                    throw new RuntimeException("Ya existe un proveedor con la CURP: " + request.getCurp());
                }
            } else {
                log.debug("CURP no cambió, se omite validación de duplicados");
            }
        }
        
        // Validar email si se proporciona
        if (StringUtils.hasText(request.getEmail()) && !isValidEmail(request.getEmail())) {
            throw new RuntimeException("El formato del email no es válido");
        }
    }

    /**
     * Valida si se puede dar de baja el proveedor
     */
    private void validarBajaProveedor(Proveedor proveedor) {
        // Aquí se pueden agregar validaciones de negocio
        // Por ejemplo: verificar que no tenga movimientos pendientes, facturas sin pagar, etc.
        log.debug("Validando baja de proveedor: {}", proveedor.getProveedor());
    }

    /**
     * Mapea los datos básicos del request al entity
     */
    private void mapearDatosBasicos(ProveedorRequest request, Proveedor proveedor) {
        proveedor.setRazonsocial(request.getRazonsocial());
        proveedor.setRfc(request.getRfc());
        proveedor.setCurp(request.getCurp());
        proveedor.setColonia(request.getColonia());
        proveedor.setEstado(request.getEstado());
        proveedor.setCp(request.getCp());
        proveedor.setPais(request.getPais());
        proveedor.setEmail(request.getEmail());
        proveedor.setComprador(request.getComprador());
        proveedor.setCredito(request.getCredito());
        proveedor.setDescuento(request.getDescuento());
        proveedor.setCotizable(request.getCotizable());
        proveedor.setProvgastos(request.getProvgastos());
        proveedor.setProvmercancia(request.getProvmercancia());
        proveedor.setEsresico(request.getEsresico());
        proveedor.setMincajas(request.getMincajas());
        proveedor.setMinpeso(request.getMinpeso());
        proveedor.setMindinero(request.getMindinero());
        // TODO: Add more fields when DTO is complete
    }

    /**
     * Procesa los teléfonos del proveedor
     * Elimina todos los teléfonos existentes y crea los nuevos
     */
    private void procesarTelefonos(Proveedor proveedor, List<TelefonoProveedorDTO> telefonosRequest, String usuario) {
        log.debug("Procesando teléfonos para proveedor: {}", proveedor.getProveedor());
        
        // Eliminar teléfonos existentes
        telefonoProveedorRepository.deleteByProveedorId(proveedor.getProveedor());
        telefonoProveedorRepository.flush(); // Asegurar que se eliminen antes de insertar
        
        // Crear nuevos teléfonos
        if (telefonosRequest != null && !telefonosRequest.isEmpty()) {
            for (TelefonoProveedorDTO telefonoDTO : telefonosRequest) {
                TelefonoProveedor telefono = new TelefonoProveedor();
                telefono.setProveedorId(proveedor.getProveedor());
                telefono.setLada(telefonoDTO.getLada() != null ? telefonoDTO.getLada() : "");
                telefono.setTelefono(telefonoDTO.getTelefono());
                telefono.setExtension(telefonoDTO.getExtension());
                telefono.setTipo(telefonoDTO.getTipo() != null ? telefonoDTO.getTipo() : "Negocio");
                
                telefonoProveedorRepository.save(telefono);
                log.debug("Teléfono guardado: {} {}", telefono.getLada(), telefono.getTelefono());
            }
        }
        
        log.info("Teléfonos procesados exitosamente para proveedor: {}", proveedor.getProveedor());
    }

    // TODO: Implement these methods when DTOs and entities are properly configured
    /*
    private void procesarCondicionesComerciales(Proveedor proveedor, List<CondicionComercialProveedorDTO> condicionesRequest, String usuario) {
        // Implementation pending DTO completion
    }

    private void procesarCuentasRetencion(Proveedor proveedor, List<CuentaRetencionProveedorDTO> cuentasRequest, String usuario) {
        // Implementation pending DTO completion
    }
    */

    /**
     * Convierte un entity Proveedor a ProveedorResponse
     */
    private ProveedorResponse convertToResponse(Proveedor proveedor) {
        ProveedorResponse response = new ProveedorResponse();
        
        // Datos básicos disponibles
        response.setProveedor(proveedor.getProveedor());
        response.setRazonsocial(proveedor.getRazonsocial());
        response.setTipoempre(proveedor.getTipoempre());
        response.setReplegal(proveedor.getReplegal());
        response.setTitrepleg(proveedor.getTitrepleg());
        response.setFechnrep(proveedor.getFechnrep());
        response.setRfc(proveedor.getRfc());
        response.setCurp(proveedor.getCurp());
        response.setCalle(proveedor.getCalle());
        response.setColonia(proveedor.getColonia());
        response.setCvecolonia(proveedor.getCvecolonia());
        response.setEstado(proveedor.getEstado());
        response.setLocalidad(proveedor.getLocalidad());
        response.setCp(proveedor.getCp());
        response.setPais(proveedor.getPais());
        response.setContacto(proveedor.getContacto());
        response.setEmailcto(proveedor.getEmailcto());
        response.setFechncon(proveedor.getFechncon());
        response.setEmail(proveedor.getEmail());
        response.setComprador(proveedor.getComprador());
        response.setCredito(proveedor.getCredito());
        response.setLimcred(proveedor.getLimcred());
        response.setPlazo(proveedor.getPlazo());
        response.setDescuento(proveedor.getDescuento());
        response.setDescppp(proveedor.getDescppp());
        response.setBancoc1(proveedor.getBancoc1());
        response.setBancoc2(proveedor.getBancoc2());
        response.setBancoc3(proveedor.getBancoc3());
        response.setCuentab1(proveedor.getCuentab1());
        response.setCuentab2(proveedor.getCuentab2());
        response.setCuentab3(proveedor.getCuentab3());
        response.setTipocuenta1(proveedor.getTipocuenta1());
        response.setTipocuenta2(proveedor.getTipocuenta2());
        response.setTipocuenta3(proveedor.getTipocuenta3());
        response.setCuentadefault(proveedor.getCuentadefault());
        response.setApoyos(proveedor.getApoyos());
        response.setFechauap(proveedor.getFechauap());
        response.setReduccostobase(proveedor.getReduccostobase());
        response.setPorcreduccosto(proveedor.getPorcreduccosto());
        response.setEsparterelac(proveedor.getEsparterelac());
        response.setCuadreestcomp(proveedor.getCuadreestcomp());
        response.setCuadreestncre(proveedor.getCuadreestncre());
        response.setCuadreestpagos(proveedor.getCuadreestpagos());
        response.setCuadreestncar(proveedor.getCuadreestncar());
        response.setRedondeocptecho(proveedor.getRedondeocptecho());
        response.setEmitencpago(proveedor.getEmitencpago());
        response.setAgrupapagfact(proveedor.getAgrupapagfact());
        response.setAgrupapaggast(proveedor.getAgrupapaggast());
        response.setTiporefpago(proveedor.getTiporefpago());
        response.setReferenciafija(proveedor.getReferenciafija());
        response.setDiasvigencia(proveedor.getDiasvigencia());
        response.setTiporetencion(proveedor.getTiporetencion());
        response.setImpuestoret(proveedor.getImpuestoret());
        response.setNumcuenta(proveedor.getNumcuenta());
        response.setDiasreorden(proveedor.getDiasreorden());
        response.setCapturista(proveedor.getCapturista());
        response.setConfianzapedidoautomatico(proveedor.getConfianzapedidoautomatico());
        response.setAjuste_bancario(proveedor.getAjuste_bancario());
        response.setCotizable(proveedor.getCotizable());
        response.setCorreo_cotizacion(proveedor.getCorreo_cotizacion());
        response.setProvgastos(proveedor.getProvgastos());
        response.setProvmercancia(proveedor.getProvmercancia());
        response.setEsresico(proveedor.getEsresico());
        response.setMincajas(proveedor.getMincajas());
        response.setMinpeso(proveedor.getMinpeso());
        response.setMindinero(proveedor.getMindinero());
        response.setActivo(proveedor.getActivo());
        response.setFechaalta(proveedor.getFechaalta());
        response.setUsualta(proveedor.getUsualta());
        response.setFechacambio(proveedor.getFechacambio());
        response.setUsumodi(proveedor.getUsumodi());
        
        // Cargar teléfonos
        response.setTelefonos(cargarTelefonos(proveedor.getProveedor()));
        
        // TODO: Cargar otras relaciones cuando entities y responses estén completos
        // response.setCondicionesComerciales(cargarCondicionesComerciales(proveedor.getProveedor()));
        // response.setCuentasRetencion(cargarCuentasRetencion(proveedor.getProveedor()));
        
        return response;
    }

    /**
     * Carga los teléfonos de un proveedor
     */
    private List<TelefonoProveedorResponse> cargarTelefonos(String proveedorId) {
        List<TelefonoProveedor> telefonos = telefonoProveedorRepository.findByProveedorIdOrderByLadaAscTelefonoAsc(proveedorId);
        
        return telefonos.stream()
            .map(this::convertToTelefonoResponse)
            .collect(Collectors.toList());
    }
    
    /**
     * Convierte un TelefonoProveedor entity a TelefonoProveedorResponse
     */
    private TelefonoProveedorResponse convertToTelefonoResponse(TelefonoProveedor telefono) {
        TelefonoProveedorResponse response = new TelefonoProveedorResponse();
        response.setProveedor(telefono.getProveedorId());
        response.setLada(telefono.getLada());
        response.setTelefono(telefono.getTelefono());
        response.setExtension(telefono.getExtension());
        response.setTipo(telefono.getTipo());
        // Campos que no existen en la BD pero se esperan en el response con valores por defecto
        response.setComentarios("");
        response.setPrincipal(false);
        return response;
    }

    // TODO: Implement these methods when DTOs and entities are properly configured
    /*
    private List<CondicionComercialProveedorResponse> cargarCondicionesComerciales(String proveedor) {
        // Implementation pending
        return Collections.emptyList();
    }

    private List<CuentaRetencionProveedorResponse> cargarCuentasRetencion(String proveedor) {
        // Implementation pending
        return Collections.emptyList();
    }
    */

    /**
     * Valida formato de RFC
     */
    private boolean isValidRFC(String rfc) {
        if (!StringUtils.hasText(rfc)) return false;
        // Patrón básico para RFC (puede ser más específico según necesidades)
        return rfc.matches("^[A-ZÑ&]{3,4}[0-9]{2}[0-1][0-9][0-3][0-9][A-Z0-9]{2}[0-9A]$");
    }

    /**
     * Valida formato de email
     */
    private boolean isValidEmail(String email) {
        if (!StringUtils.hasText(email)) return false;
        return email.matches("^[A-Za-z0-9+_.-]+@([A-Za-z0-9.-]+\\.[A-Za-z]{2,})$");
    }
}