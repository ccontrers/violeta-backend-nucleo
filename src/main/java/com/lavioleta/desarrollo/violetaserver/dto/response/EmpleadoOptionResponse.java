package com.lavioleta.desarrollo.violetaserver.dto.response;

import io.swagger.v3.oas.annotations.media.Schema;
import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

@Data
@Builder
@NoArgsConstructor
@AllArgsConstructor
@Schema(name = "EmpleadoOptionResponse", description = "Elemento para combos de selección de empleados")
public class EmpleadoOptionResponse {
    private String empleado;
    private String nombreCompleto;
    private boolean tieneUsuario;
}
