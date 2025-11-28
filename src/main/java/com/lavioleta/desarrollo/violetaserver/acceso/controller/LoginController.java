package com.lavioleta.desarrollo.violetaserver.acceso.controller;

import com.lavioleta.desarrollo.violetaserver.acceso.dto.request.LoginRequest;
import com.lavioleta.desarrollo.violetaserver.acceso.dto.response.LoginResponse;
import com.lavioleta.desarrollo.violetaserver.acceso.service.LoginService;

import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpSession;
import jakarta.validation.Valid;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.security.authentication.UsernamePasswordAuthenticationToken;
import org.springframework.security.core.authority.SimpleGrantedAuthority;
import org.springframework.security.core.context.SecurityContext;
import org.springframework.security.core.context.SecurityContextHolder;
import org.springframework.security.web.context.HttpSessionSecurityContextRepository;
import org.springframework.web.bind.annotation.*;
import io.swagger.v3.oas.annotations.Operation;
import io.swagger.v3.oas.annotations.tags.Tag;
import io.swagger.v3.oas.annotations.responses.ApiResponse;
import io.swagger.v3.oas.annotations.responses.ApiResponses;

import java.util.List;

@RestController
@RequestMapping("/api/v1/auth")
@Tag(name = "Autenticación", description = "Endpoints para login y recursos iniciales")
public class LoginController {
    
    private static final Logger logger = LoggerFactory.getLogger(LoginController.class);
    private final LoginService loginService;
    
    public LoginController(LoginService loginService) {
        this.loginService = loginService;
    }
    
    /**
     * Endpoint para autenticación de usuarios
     * POST /api/v1/auth/login
     */
    @PostMapping("/login")
    @Operation(summary = "Autenticar usuario", description = "Valida credenciales y devuelve información de sesión / perfil.")
    @ApiResponses({
        @ApiResponse(responseCode = "200", description = "Autenticación exitosa"),
        @ApiResponse(responseCode = "401", description = "Credenciales inválidas"),
        @ApiResponse(responseCode = "500", description = "Error interno")
    })
    public ResponseEntity<LoginResponse> login(
            @Valid @RequestBody LoginRequest request,
            HttpServletRequest httpRequest) {
        
        try {
            // Obtener IP del cliente
            String clientIp = obtenerClientIp(httpRequest);
            logger.debug("Procesando login request para usuario: {} desde IP: {}", request.getUsuario(), clientIp);
            
            // Procesar autenticación
            LoginResponse response = loginService.autenticar(request, clientIp);
            
            // Determinar código de respuesta HTTP
            HttpStatus status = response.isSuccess() ? HttpStatus.OK : HttpStatus.UNAUTHORIZED;

            if (response.isSuccess() && response.getUsuario() != null) {
                inicializarSesion(httpRequest, response.getUsuario());
            } else if (!response.isSuccess()) {
                limpiarSesion(httpRequest);
            }
            
            return ResponseEntity.status(status).body(response);
            
        } catch (Exception e) {
            logger.error("Error inesperado durante login para usuario {}: {}", request.getUsuario(), e.getMessage(), e);
            
            LoginResponse errorResponse = LoginResponse.failure("Error interno del servidor. Intente nuevamente.");
            return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR).body(errorResponse);
        }
    }
    
    /**
     * Endpoint para autenticación con JWT (token-based authentication)
     * POST /api/v1/auth/login/jwt
     * 
     * Similar a /login pero retorna un JWT token en lugar de crear una sesión cookie.
     * El token debe ser enviado en el header "Authorization: Bearer {token}" en llamadas subsecuentes.
     * 
     * Casos de uso: Aplicaciones móviles, APIs externas, scripts automatizados
     */
    @PostMapping("/login/jwt")
    @Operation(
        summary = "Autenticar usuario con JWT", 
        description = "Valida credenciales y retorna un JSON Web Token (JWT) para autenticación posterior. " +
                      "No crea sesión cookie - el token debe enviarse en cada petición vía header 'Authorization: Bearer {token}'."
    )
    @ApiResponses({
        @ApiResponse(responseCode = "200", description = "Autenticación exitosa - Token JWT generado"),
        @ApiResponse(responseCode = "401", description = "Credenciales inválidas"),
        @ApiResponse(responseCode = "500", description = "Error interno")
    })
    public ResponseEntity<LoginResponse> loginJwt(
            @Valid @RequestBody LoginRequest request,
            HttpServletRequest httpRequest) {
        
        try {
            // Obtener IP del cliente
            String clientIp = obtenerClientIp(httpRequest);
            logger.debug("Procesando JWT login request para usuario: {} desde IP: {}", request.getUsuario(), clientIp);
            
            // Procesar autenticación con generación de JWT
            LoginResponse response = loginService.autenticarConJwt(request, clientIp);
            
            // Determinar código de respuesta HTTP
            HttpStatus status = response.isSuccess() ? HttpStatus.OK : HttpStatus.UNAUTHORIZED;
            
            return ResponseEntity.status(status).body(response);
            
        } catch (Exception e) {
            logger.error("Error inesperado durante JWT login para usuario {}: {}", request.getUsuario(), e.getMessage(), e);
            
            LoginResponse errorResponse = LoginResponse.failure("Error interno del servidor. Intente nuevamente.");
            return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR).body(errorResponse);
        }
    }
    
    /**
     * Endpoint para obtener la lista de sucursales disponibles
     * GET /api/v1/auth/sucursales
     */
    @GetMapping("/sucursales")
    @Operation(summary = "Listar sucursales", description = "Obtiene catálogo de sucursales disponibles para autenticación.")
    @ApiResponses({
        @ApiResponse(responseCode = "200", description = "Listado obtenido"),
        @ApiResponse(responseCode = "500", description = "Error interno")
    })
    public ResponseEntity<?> obtenerSucursales() {
        try {
            logger.debug("Solicitando lista de sucursales");
            
            var sucursales = loginService.obtenerSucursales();
            
            return ResponseEntity.ok(sucursales);
            
        } catch (Exception e) {
            logger.error("Error al obtener sucursales: {}", e.getMessage(), e);
            return ResponseEntity.status(HttpStatus.INTERNAL_SERVER_ERROR)
                    .body("Error interno del servidor al obtener sucursales");
        }
    }
    
    /**
     * Endpoint para verificar estado de autenticación (futuro)
     * GET /api/v1/auth/status
     */
    @GetMapping("/status")
    @Operation(summary = "Estado del servicio de autenticación", description = "Devuelve mensaje simple para monitoreo / health check.")
    public ResponseEntity<String> status() {
        return ResponseEntity.ok("Servicio de autenticación disponible");
    }
    
    /**
     * Obtiene la IP real del cliente considerando proxies y load balancers
     */
    private String obtenerClientIp(HttpServletRequest request) {
        String xForwardedFor = request.getHeader("X-Forwarded-For");
        if (xForwardedFor != null && !xForwardedFor.isEmpty()) {
            // Tomar la primera IP en caso de múltiples proxies
            return xForwardedFor.split(",")[0].trim();
        }
        
        String xRealIp = request.getHeader("X-Real-IP");
        if (xRealIp != null && !xRealIp.isEmpty()) {
            return xRealIp;
        }
        
        return request.getRemoteAddr();
    }

    private void inicializarSesion(HttpServletRequest request, LoginResponse.UsuarioInfo usuarioInfo) {
        UsernamePasswordAuthenticationToken authentication = new UsernamePasswordAuthenticationToken(
            usuarioInfo.getEmpleado(),
            null,
            List.of(new SimpleGrantedAuthority("ROLE_USER"))
        );

        authentication.setDetails(usuarioInfo);

        SecurityContext context = SecurityContextHolder.createEmptyContext();
        context.setAuthentication(authentication);
        SecurityContextHolder.setContext(context);

        HttpSession session = request.getSession(true);
        session.setAttribute(HttpSessionSecurityContextRepository.SPRING_SECURITY_CONTEXT_KEY, context);
        session.setAttribute("usuarioInfo", usuarioInfo);
    }

    private void limpiarSesion(HttpServletRequest request) {
        SecurityContextHolder.clearContext();
        HttpSession session = request.getSession(false);
        if (session != null) {
            session.invalidate();
        }
    }
}
