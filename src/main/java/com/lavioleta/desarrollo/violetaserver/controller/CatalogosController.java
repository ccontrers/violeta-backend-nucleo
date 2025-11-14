package com.lavioleta.desarrollo.violetaserver.controller;

import com.lavioleta.desarrollo.violetaserver.service.CatalogosService;
import org.springframework.web.bind.annotation.*;
import java.util.List;
import java.util.Map;
import io.swagger.v3.oas.annotations.Operation;
import io.swagger.v3.oas.annotations.tags.Tag;
import io.swagger.v3.oas.annotations.responses.ApiResponse;
import io.swagger.v3.oas.annotations.responses.ApiResponses;

/**
 * Controlador para catálogos auxiliares
 */
@RestController
@RequestMapping("/api/v1/catalogos")
@Tag(name = "Catálogos", description = "Recursos de catálogos estáticos y auxiliares")
public class CatalogosController {
    
    private final CatalogosService catalogosService;
    
    public CatalogosController(CatalogosService catalogosService) {
        this.catalogosService = catalogosService;
    }
    
    /**
     * Obtener catálogo de colonias
     */
    @GetMapping("/colonias")
    @Operation(summary = "Listar colonias", description = "Obtiene todas las colonias.")
    public Map<String, Object> obtenerColonias() {
        List<Map<String, Object>> colonias = catalogosService.obtenerColonias();
        return Map.of(
            "success", true,
            "message", "Colonias obtenidas exitosamente",
            "colonias", colonias
        );
    }
    
    /**
     * Buscar colonias por nombre
     */
    @GetMapping("/colonias/buscar")
    @Operation(summary = "Buscar colonias", description = "Filtra colonias por nombre (contiene).")
    public Map<String, Object> buscarColonias(@RequestParam String nombre) {
        List<Map<String, Object>> colonias = catalogosService.buscarColoniasPorNombre(nombre);
        return Map.of(
            "success", true,
            "message", "Búsqueda completada",
            "colonias", colonias
        );
    }
    
    /**
     * Obtener catálogo de canales de clientes
     */
    @GetMapping("/canales")
    @Operation(summary = "Canales de clientes", description = "Obtiene catálogo de canales.")
    public Map<String, Object> obtenerCanalesClientes() {
        List<Map<String, Object>> canales = catalogosService.obtenerCanalesClientes();
        return Map.of(
            "success", true,
            "message", "Canales obtenidos exitosamente",
            "canales", canales
        );
    }
    
    /**
     * Obtener catálogo de giros de negocio
     */
    @GetMapping("/giros")
    @Operation(summary = "Giros de negocio", description = "Obtiene giros de negocio válidos.")
    public Map<String, Object> obtenerGirosNegocio() {
        List<Map<String, Object>> giros = catalogosService.obtenerGirosNegocio();
        return Map.of(
            "success", true,
            "message", "Giros obtenidos exitosamente",
            "giros", giros
        );
    }
    
    /**
     * Obtener catálogo de regímenes fiscales
     */
    @GetMapping("/regimenes-fiscales")
    @Operation(summary = "Regímenes fiscales", description = "Obtiene catálogo de regímenes fiscales.")
    public Map<String, Object> obtenerRegimenesFiscales() {
        List<Map<String, Object>> regimenes = catalogosService.obtenerRegimenesFiscales();
        return Map.of(
            "success", true,
            "message", "Regímenes fiscales obtenidos exitosamente",
            "regimenes", regimenes
        );
    }
    
    /**
     * Obtener catálogo de sociedades mercantiles
     */
    @GetMapping("/sociedades-mercantiles")
    @Operation(summary = "Sociedades mercantiles", description = "Obtiene catálogo de sociedades mercantiles.")
    public Map<String, Object> obtenerSociedadesMercantiles() {
        List<Map<String, Object>> sociedades = catalogosService.obtenerSociedadesMercantiles();
        return Map.of(
            "success", true,
            "message", "Sociedades mercantiles obtenidas exitosamente",
            "sociedades", sociedades
        );
    }
    
    
    /**
     * Obtener catálogo de usos CFDI
     */
    @GetMapping("/usos-cfdi")
    @Operation(summary = "Usos CFDI", description = "Obtiene catálogo de usos CFDI.")
    public Map<String, Object> obtenerUsosCfdi() {
        List<Map<String, Object>> usos = catalogosService.obtenerUsosCfdi();
        return Map.of(
            "success", true,
            "message", "Usos CFDI obtenidos exitosamente",
            "usos", usos
        );
    }
    
    @GetMapping("/tipos-precios")
    @Operation(summary = "Tipos de precios", description = "Obtiene lista general de tipos de precios.")
    public Map<String, Object> obtenerTiposPrecios() {
        List<Map<String, Object>> tipos = catalogosService.obtenerTiposPrecios();
        return Map.of(
            "success", true,
            "message", "Tipos de precios obtenidos exitosamente",
            "tipos", tipos
        );
    }
    
    @GetMapping("/tipos-precios/empresa/{idEmpresa}")
    @Operation(summary = "Tipos de precios por empresa", description = "Obtiene tipos de precios asociados a una empresa.")
    public Map<String, Object> obtenerTiposPreciosPorEmpresa(@PathVariable Integer idEmpresa) {
        List<Map<String, Object>> tipos = catalogosService.obtenerTiposPreciosPorEmpresa(idEmpresa);
        return Map.of(
            "success", true,
            "message", "Tipos de precios obtenidos exitosamente para la empresa",
            "tipos", tipos
        );
    }
    
    /**
     * Obtener catálogo de tipos de cuentas bancarias
     */
    @GetMapping("/tipos-cuentas-bancarias")
    @Operation(summary = "Tipos de cuentas bancarias", description = "Obtiene catálogo de tipos de cuentas bancarias activas.")
    public Map<String, Object> obtenerTiposCuentasBancarias() {
        List<Map<String, Object>> tipos = catalogosService.obtenerTiposCuentasBancarias();
        return Map.of(
            "success", true,
            "message", "Tipos de cuentas bancarias obtenidos exitosamente",
            "tipos", tipos
        );
    }
    
    /**
     * Obtener catálogo de bancos
     */
    @GetMapping("/bancos")
    @Operation(summary = "Bancos", description = "Obtiene catálogo de bancos activos.")
    public Map<String, Object> obtenerBancos() {
        List<Map<String, Object>> bancos = catalogosService.obtenerBancos();
        return Map.of(
            "success", true,
            "message", "Bancos obtenidos exitosamente",
            "bancos", bancos
        );
    }
    
    /*
    // Comentado temporalmente - tablas no disponibles
    
    @GetMapping("/categorias-clientes")
    public Map<String, Object> obtenerCategoriasClientes() {
        List<Map<String, Object>> categorias = catalogosService.obtenerCategoriasClientes();
        return Map.of(
            "success", true,
            "message", "Categorías de clientes obtenidas exitosamente",
            "categorias", categorias
        );
    }
    
    @GetMapping("/tipos-clientes")
    public Map<String, Object> obtenerTiposClientes() {
        List<Map<String, Object>> tipos = catalogosService.obtenerTiposClientes();
        return Map.of(
            "success", true,
            "message", "Tipos de clientes obtenidos exitosamente",
            "tipos", tipos
        );
    }
    
    @GetMapping("/clasificaciones-clientes")
    public Map<String, Object> obtenerClasificacionesClientes() {
        List<Map<String, Object>> clasificaciones = catalogosService.obtenerClasificacionesClientes();
        return Map.of(
            "success", true,
            "message", "Clasificaciones de clientes obtenidas exitosamente",
            "clasificaciones", clasificaciones
        );
    }
    
    @GetMapping("/subclasificaciones-clientes")
    public Map<String, Object> obtenerSubclasificacionesClientes() {
        List<Map<String, Object>> subclasificaciones = catalogosService.obtenerSubclasificacionesClientes();
        return Map.of(
            "success", true,
            "message", "Subclasificaciones de clientes obtenidas exitosamente",
            "subclasificaciones", subclasificaciones
        );
    }
    
    @GetMapping("/nacionalidades")
    public Map<String, Object> obtenerNacionalidades() {
        List<Map<String, Object>> nacionalidades = catalogosService.obtenerNacionalidades();
        return Map.of(
            "success", true,
            "message", "Nacionalidades obtenidas exitosamente",
            "nacionalidades", nacionalidades
        );
    }
    
    @GetMapping("/regimenes-matrimoniales")
    public Map<String, Object> obtenerRegimenesMatrimoniales() {
        List<Map<String, Object>> regimenes = catalogosService.obtenerRegimenesMatrimoniales();
        return Map.of(
            "success", true,
            "message", "Regímenes matrimoniales obtenidos exitosamente",
            "regimenes", regimenes
        );
    }
    */
}
