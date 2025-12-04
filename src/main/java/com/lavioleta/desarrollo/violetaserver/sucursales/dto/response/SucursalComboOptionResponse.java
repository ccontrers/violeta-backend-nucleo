package com.lavioleta.desarrollo.violetaserver.sucursales.dto.response;

import io.swagger.v3.oas.annotations.media.Schema;
import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

@Data
@Builder
@NoArgsConstructor
@AllArgsConstructor
@Schema(name = "SucursalComboOptionResponse", description = "Elemento para ComboBox general de sucursales")
public class SucursalComboOptionResponse {
    private String sucursal;
    private String nombre;
}
