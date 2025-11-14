package com.lavioleta.desarrollo.violetaserver.dto.request;

import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.Pattern;
import jakarta.validation.constraints.Size;
import lombok.Builder;
import lombok.Data;

@Data
@Builder
public class BusquedaClientesRequest {
    
    @NotBlank(message = "Tipo de búsqueda es requerido")
    @Pattern(
        regexp = "^(NOM|APE|RFC|RSO|NNE|CLA|CONT|EMA|)$",
        message = "Tipo de búsqueda inválido. Valores permitidos: NOM, APE, RFC, RSO, NNE, CLA, CONT, EMA o vacío"
    )
    private String codcondicion;
    
    @Builder.Default
    private Boolean mostrarInactivos = false;
    
    @Size(max = 100, message = "La condición no puede exceder 100 caracteres")
    private String condicion;
    
    @Builder.Default
    @Pattern(regexp = "^[1-9][0-9]*$", message = "El número de filas debe ser un entero positivo")
    private String filas = "20";
}
