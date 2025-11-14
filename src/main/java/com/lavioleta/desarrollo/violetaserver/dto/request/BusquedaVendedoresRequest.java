package com.lavioleta.desarrollo.violetaserver.dto.request;

import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.Pattern;
import jakarta.validation.constraints.Size;
import lombok.Builder;
import lombok.Data;

/**
 * DTO para solicitud de búsqueda de vendedores
 * Migrado desde FormBusquedaVendedores.cpp y ServidorBusquedas::BuscaVendedores
 * 
 * Tipos de búsqueda soportados:
 * - NOM: Por nombre (e.nombre LIKE 'valor%')
 * - APE: Por apellidos (e.appat LIKE 'valor%' OR e.apmat LIKE 'valor%')
 * - COMI: Por tipo de comisión exacto (v.tipocomi = 'valor')
 * - CLA: Por clave de empleado (e.empleado LIKE 'valor%')
 */
@Data
@Builder
public class BusquedaVendedoresRequest {
    
    /**
     * Tipo de búsqueda basado en el legacy:
     * NOM - Nombre del vendedor
     * APE - Apellidos (paterno o materno)
     * COMI - Tipo de comisión
     * CLA - Clave del vendedor (empleado)
     */
    @NotBlank(message = "Tipo de búsqueda es requerido")
    @Pattern(
        regexp = "^(NOM|APE|COMI|CLA)$",
        message = "Tipo de búsqueda inválido. Valores permitidos: NOM, APE, COMI, CLA"
    )
    private String tipoBusqueda;
    
    /**
     * Incluir solo vendedores activos
     * Legacy: solo_activos = "1" -> v.activo=1
     * Legacy: solo_activos != "1" -> v.activo=0
     * 
     * Por defecto true para buscar solo activos
     */
    @Builder.Default
    private Boolean soloActivos = true;
    
    /**
     * Texto de búsqueda
     * - Para NOM, APE, CLA: se aplicará como prefijo (valor%)
     * - Para COMI: búsqueda exacta (= 'valor')
     */
    @NotBlank(message = "Valor de búsqueda es requerido")
    @Size(max = 50, message = "El valor de búsqueda no puede exceder 50 caracteres")
    private String valor;
    
    /**
     * Límite de resultados (legacy: NUM_LIMITE_RESULTADOS_BUSQ = 501)
     */
    @Builder.Default
    private Integer limite = 501;
}
