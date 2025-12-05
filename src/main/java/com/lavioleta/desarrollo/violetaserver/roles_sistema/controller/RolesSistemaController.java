package com.lavioleta.desarrollo.violetaserver.roles_sistema.controller;

import java.util.List;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import com.lavioleta.desarrollo.violetaserver.roles_sistema.dto.response.RolComboOptionResponse;
import com.lavioleta.desarrollo.violetaserver.roles_sistema.service.RolesSistemaService;

import io.swagger.v3.oas.annotations.Operation;
import io.swagger.v3.oas.annotations.responses.ApiResponse;
import io.swagger.v3.oas.annotations.responses.ApiResponses;
import io.swagger.v3.oas.annotations.tags.Tag;

/**
 * Controller REST para roles del sistema.
 */
@RestController
@RequestMapping("/api/v1/roles")
@Tag(name = "Roles del Sistema", description = "Operaciones con roles del sistema")
public class RolesSistemaController {

    private static final Logger log = LoggerFactory.getLogger(RolesSistemaController.class);

    private final RolesSistemaService service;

    public RolesSistemaController(RolesSistemaService service) {
        this.service = service;
    }

    /**
     * Endpoint para obtener la lista de roles para el combo box.
     * Migrado desde FormBitacoraModPrivilegios.cpp::BlanqueaFormulario() - ComboBoxRol
     */
    @GetMapping("/combo-box")
    @Operation(
            summary = "ComboBox de roles",
            description = "Lista roles del sistema para llenar combos generales."
    )
    @ApiResponses({
            @ApiResponse(responseCode = "200", description = "Lista de roles obtenida correctamente"),
            @ApiResponse(responseCode = "500", description = "Error interno del servidor")
    })
    public ResponseEntity<List<RolComboOptionResponse>> comboRoles() {
        log.debug("Obteniendo roles para combo box");
        try {
            List<RolComboOptionResponse> roles = service.listarRolesCombo();
            return ResponseEntity.ok(roles);
        } catch (Exception ex) {
            log.error("Error al obtener roles para combo", ex);
            return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR).build();
        }
    }
}
