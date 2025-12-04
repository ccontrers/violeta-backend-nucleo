package com.lavioleta.desarrollo.violetaserver.roles_sistema.dto.response;

import io.swagger.v3.oas.annotations.media.Schema;
import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * DTO para elementos del combo box de roles del sistema.
 * Corresponde a la consulta legacy:
 * SELECT claverol, nombre FROM rolessistema ORDER BY nombre
 */
@Data
@Builder
@NoArgsConstructor
@AllArgsConstructor
@Schema(name = "RolComboOptionResponse", description = "Elemento para ComboBox de roles del sistema")
public class RolComboOptionResponse {

    @Schema(description = "Clave única del rol", example = "ADMIN")
    private String clave;

    @Schema(description = "Nombre descriptivo del rol", example = "Administrador del Sistema")
    private String nombre;
}
