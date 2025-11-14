package com.lavioleta.desarrollo.violetaserver.dto.request;

import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.Pattern;
import jakarta.validation.constraints.Size;
import lombok.Builder;
import lombok.Data;

@Data
@Builder
public class BusquedaProveedoresRequest {
    
    /**
     * Tipo de búsqueda basado en el legacy:
     * RSO - Razón social
     * RFC - RFC 
     * CLA - Clave (proveedor)
     * REP - Representante legal
     * "" - Sin criterio (búsqueda vacía)
     */
    @NotBlank(message = "Tipo de búsqueda es requerido")
    @Pattern(
        regexp = "^(RSO|RFC|CLA|REP|)$",
        message = "Tipo de búsqueda inválido. Valores permitidos: RSO, RFC, CLA, REP o vacío"
    )
    private String codcondicion;
    
    /**
     * Incluir proveedores inactivos
     * Legacy: solo_activos invertido (false = incluir inactivos)
     */
    @Builder.Default
    private Boolean mostrarInactivos = false;
    
    /**
     * Texto de búsqueda - se aplicará con comodines %valor%
     */
    @Size(max = 100, message = "La condición no puede exceder 100 caracteres")
    private String condicion;
    
    /**
     * Filtrar solo proveedores de gastos
     * Legacy: solo_ProvGastos ("1" = forzar provgastos=1)
     */
    @Builder.Default
    private Boolean soloProveedorGastos = false;
    
    /**
     * Filtrar solo proveedores de mercancía
     * Legacy: solo_ProvMercancia ("1" = forzar provmercancia=1)  
     */
    @Builder.Default
    private Boolean soloProveedorMercancia = false;
    
    /**
     * Límite de resultados
     */
    @Builder.Default
    @Pattern(regexp = "^[1-9][0-9]*$", message = "El número de filas debe ser un entero positivo")
    private String filas = "50";
}