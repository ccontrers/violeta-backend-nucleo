package com.lavioleta.desarrollo.violetaserver.usuarios.dto.response;

import io.swagger.v3.oas.annotations.media.Schema;
import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

@Data
@Builder
@NoArgsConstructor
@AllArgsConstructor
@Schema(name = "UsuarioComboOptionResponse", description = "Elemento para ComboBox general de usuarios")
public class UsuarioComboOptionResponse {
    private String empleado;
    private String nombreCompleto;
    private String sucursal;
}
