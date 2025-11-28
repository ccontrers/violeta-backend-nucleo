package com.lavioleta.desarrollo.violetaserver.usuarios.controller;

import java.util.List;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.util.StringUtils;
import org.springframework.web.bind.annotation.DeleteMapping;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestHeader;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import com.lavioleta.desarrollo.violetaserver.usuarios.dto.request.UsuarioRequest;
import com.lavioleta.desarrollo.violetaserver.usuarios.dto.response.EmpleadoOptionResponse;
import com.lavioleta.desarrollo.violetaserver.usuarios.dto.response.UsuarioListResponse;
import com.lavioleta.desarrollo.violetaserver.usuarios.dto.response.UsuarioResponse;
import com.lavioleta.desarrollo.violetaserver.usuarios.service.CatalogoUsuariosService;

import io.swagger.v3.oas.annotations.Operation;
import io.swagger.v3.oas.annotations.tags.Tag;
import io.swagger.v3.oas.annotations.responses.ApiResponse;
import io.swagger.v3.oas.annotations.responses.ApiResponses;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.validation.Valid;

@RestController
@RequestMapping("/api/v1/usuarios")
@Tag(name = "Catálogo de Usuarios", description = "Endpoints migrados desde FormCatalogoUsuarios")
public class CatalogoUsuariosController {

    private static final Logger log = LoggerFactory.getLogger(CatalogoUsuariosController.class);

    private final CatalogoUsuariosService service;

    public CatalogoUsuariosController(CatalogoUsuariosService service) {
        this.service = service;
    }

    @GetMapping
    @Operation(summary = "Listar usuarios", description = "Equivalente a ConsultaUsuario (grid principal)")
    @ApiResponses({
            @ApiResponse(responseCode = "200", description = "Consulta ejecutada"),
            @ApiResponse(responseCode = "400", description = "Parámetros inválidos"),
            @ApiResponse(responseCode = "500", description = "Error interno")
    })
    public ResponseEntity<UsuarioListResponse> listar(
            @RequestParam(value = "soloActivos", defaultValue = "true") boolean soloActivos,
            @RequestParam(value = "filtro", required = false) String filtro,
            HttpServletRequest request) {
        try {
            log.debug("Listando usuarios desde IP {} - soloActivos={}, filtro={}",
                    obtenerClientIp(request), soloActivos, filtro);
            UsuarioListResponse response = service.listarUsuarios(soloActivos, filtro);
            HttpStatus status = response.isSuccess() ? HttpStatus.OK : HttpStatus.BAD_REQUEST;
            return ResponseEntity.status(status).body(response);
        } catch (Exception ex) {
            log.error("Error inesperado al listar usuarios", ex);
            return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR)
                    .body(UsuarioListResponse.error("Error interno del servidor"));
        }
    }

    @GetMapping("/{empleado}")
    @Operation(summary = "Obtener usuario", description = "Consulta detallada de un usuario")
    @ApiResponses({
            @ApiResponse(responseCode = "200", description = "Usuario encontrado"),
            @ApiResponse(responseCode = "404", description = "Usuario no encontrado")
    })
    public ResponseEntity<UsuarioResponse> obtener(
            @PathVariable String empleado,
            HttpServletRequest request) {
        try {
            log.debug("Consultando usuario {} desde IP {}", empleado, obtenerClientIp(request));
            UsuarioResponse response = service.obtenerUsuario(empleado);
            HttpStatus status = response.isSuccess() ? HttpStatus.OK : HttpStatus.NOT_FOUND;
            return ResponseEntity.status(status).body(response);
        } catch (Exception ex) {
            log.error("Error inesperado al consultar usuario {}", empleado, ex);
            return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR)
                    .body(UsuarioResponse.error("Error interno del servidor"));
        }
    }

    @PostMapping
    @Operation(summary = "Crear usuario", description = "Alta de usuario equivalente a GrabaUsuarios (A)")
        @ApiResponses({
            @ApiResponse(responseCode = "201", description = "Usuario creado"),
            @ApiResponse(responseCode = "400", description = "Petición inválida"),
            @ApiResponse(responseCode = "500", description = "Error interno")
        })
    public ResponseEntity<UsuarioResponse> crear(
            @Valid @RequestBody UsuarioRequest request,
            @RequestHeader(value = "X-Sucursal", defaultValue = "S1") String sucursal,
            HttpServletRequest httpRequest) {
        try {
            log.info("Creando usuario {} desde IP {} en sucursal {}",
                request.getEmpleado(), obtenerClientIp(httpRequest), sucursal);
            UsuarioResponse response = service.crearUsuario(request, sucursal);
            HttpStatus status = response.isSuccess() ? HttpStatus.CREATED : HttpStatus.BAD_REQUEST;
            return ResponseEntity.status(status).body(response);
        } catch (Exception ex) {
            log.error("Error inesperado al crear usuario {}", request.getEmpleado(), ex);
            return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR)
                .body(UsuarioResponse.error("Error interno del servidor"));
        }
    }

    @PutMapping("/{empleado}")
    @Operation(summary = "Actualizar usuario", description = "Modificación equivalente a GrabaUsuarios (M)")
        @ApiResponses({
            @ApiResponse(responseCode = "200", description = "Usuario actualizado"),
            @ApiResponse(responseCode = "400", description = "Petición inválida"),
            @ApiResponse(responseCode = "500", description = "Error interno")
        })
    public ResponseEntity<UsuarioResponse> actualizar(
            @PathVariable String empleado,
            @Valid @RequestBody UsuarioRequest request,
            HttpServletRequest httpRequest) {
        try {
            if (!empleado.equalsIgnoreCase(request.getEmpleado())) {
                request.setEmpleado(empleado);
            }
            log.info("Actualizando usuario {} desde IP {}", empleado, obtenerClientIp(httpRequest));
            UsuarioResponse response = service.actualizarUsuario(empleado, request);
            HttpStatus status = response.isSuccess() ? HttpStatus.OK : HttpStatus.BAD_REQUEST;
            return ResponseEntity.status(status).body(response);
        } catch (Exception ex) {
            log.error("Error inesperado al actualizar usuario {}", empleado, ex);
            return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR)
                    .body(UsuarioResponse.error("Error interno del servidor"));
        }
    }

    @DeleteMapping("/{empleado}")
    @Operation(summary = "Eliminar usuario", description = "Equivalente a BajaUsuario")
    @ApiResponses({
            @ApiResponse(responseCode = "200", description = "Usuario eliminado"),
            @ApiResponse(responseCode = "404", description = "Usuario no encontrado"),
            @ApiResponse(responseCode = "409", description = "Restricción de referencia")
    })
    public ResponseEntity<UsuarioResponse> eliminar(
            @PathVariable String empleado,
            HttpServletRequest httpRequest) {
        try {
            log.info("Eliminando usuario {} desde IP {}", empleado, obtenerClientIp(httpRequest));
            UsuarioResponse response = service.eliminarUsuario(empleado);
            HttpStatus status;
            if (response.isSuccess()) {
                status = HttpStatus.OK;
            } else if (response.getMessage() != null && response.getMessage().contains("No existe")) {
                status = HttpStatus.NOT_FOUND;
            } else {
                status = HttpStatus.CONFLICT;
            }
            return ResponseEntity.status(status).body(response);
        } catch (Exception ex) {
            log.error("Error inesperado al eliminar usuario {}", empleado, ex);
            return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR)
                    .body(UsuarioResponse.error("Error interno del servidor"));
        }
    }

    @GetMapping("/empleados-disponibles")
    @Operation(summary = "Listar empleados disponibles", description = "Tercer result set legacy para llenar combos")
    @ApiResponses({
            @ApiResponse(responseCode = "200", description = "Consulta ejecutada"),
            @ApiResponse(responseCode = "500", description = "Error interno")
    })
    public ResponseEntity<List<EmpleadoOptionResponse>> empleadosDisponibles(HttpServletRequest request) {
        try {
            log.debug("Consultando empleados disponibles desde IP {}", obtenerClientIp(request));
            List<EmpleadoOptionResponse> options = service.obtenerEmpleadosDisponibles();
            return ResponseEntity.ok(options);
        } catch (Exception ex) {
            log.error("Error inesperado al consultar empleados disponibles", ex);
            return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR).build();
        }
    }

    private String obtenerClientIp(HttpServletRequest request) {
        String header = request.getHeader("X-Forwarded-For");
        if (StringUtils.hasText(header)) {
            return header.split(",")[0].trim();
        }
        header = request.getHeader("X-Real-IP");
        if (StringUtils.hasText(header)) {
            return header;
        }
        return request.getRemoteAddr();
    }
}
