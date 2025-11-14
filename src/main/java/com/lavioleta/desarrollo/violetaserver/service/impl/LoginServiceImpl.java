package com.lavioleta.desarrollo.violetaserver.service.impl;

import com.lavioleta.desarrollo.violetaserver.config.SecurityProperties;
import com.lavioleta.desarrollo.violetaserver.dto.request.LoginRequest;
import com.lavioleta.desarrollo.violetaserver.dto.response.LoginResponse;
import com.lavioleta.desarrollo.violetaserver.dto.response.LoginResponse.UsuarioInfo;
import com.lavioleta.desarrollo.violetaserver.repository.LoginRepository;
import com.lavioleta.desarrollo.violetaserver.security.JwtTokenProvider;
import com.lavioleta.desarrollo.violetaserver.service.LoginService;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.stereotype.Service;

import java.util.*;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.ConcurrentMap;
import java.util.regex.Pattern;

@Service
public class LoginServiceImpl implements LoginService {
    
    private static final Logger logger = LoggerFactory.getLogger(LoginServiceImpl.class);
    private static final Logger auditLogger = LoggerFactory.getLogger("SECURITY_AUDIT");
    private static final Pattern SHA256_PATTERN = Pattern.compile("^[a-fA-F0-9]{64}$");
    
    private final LoginRepository loginRepository;
    private final SecurityProperties securityProperties;
    private final JwtTokenProvider jwtTokenProvider;
    
    // Cache de intentos fallidos por IP
    private final ConcurrentMap<String, IntentosLogin> intentosFallidos = new ConcurrentHashMap<>();
    
    public LoginServiceImpl(LoginRepository loginRepository, 
                           SecurityProperties securityProperties,
                           @Autowired(required = false) JwtTokenProvider jwtTokenProvider) {
        this.loginRepository = loginRepository;
        this.securityProperties = securityProperties;
        this.jwtTokenProvider = jwtTokenProvider;
        
        logger.info("LoginService inicializado con configuración:");
        logger.info("- Max intentos: {}", securityProperties.getLogin().getMaxAttempts());
        logger.info("- Tiempo bloqueo: {} minutos", securityProperties.getLogin().getLockoutDurationMinutes());
        logger.info("- Rate limiting habilitado: {}", securityProperties.getLogin().isEnableRateLimiting());
        logger.info("- Auditoría habilitada: {}", securityProperties.getLogin().isEnableAuditLog());
    }
    
    @Override
    public LoginResponse autenticar(LoginRequest request, String clientIp) {
        String usuario = request.getUsuario().toUpperCase();
        String passwordHash = request.getPasswordHash();
        
        if (securityProperties.getLogin().isLogFailedAttempts()) {
            logger.debug("Intento de login para usuario: {} desde IP: {}", usuario, clientIp);
        }
        
        // 1. Validaciones de entrada
        if (!validarFormatoPasswordHash(passwordHash)) {
            registrarIntentoFallido(clientIp, usuario, "Formato de password hash inválido");
            return LoginResponse.failure("Formato de credenciales inválido");
        }
        
        // 2. Verificar rate limiting
        if (estaIpBloqueada(clientIp, usuario)) {
            long tiempoRestante = getTiempoRestanteBloqueo(clientIp);
            String mensaje = String.format("Demasiados intentos fallidos. Intente nuevamente en %d minutos.", 
                tiempoRestante / 60000 + 1);
            
            if (securityProperties.getLogin().isLogFailedAttempts()) {
                auditLogger.warn("Intento desde IP bloqueada - Usuario: {}, IP: {}, Tiempo restante: {} ms", 
                    usuario, clientIp, tiempoRestante);
            }
            
            return LoginResponse.failure(mensaje);
        }
        
        // 3. Verificar si usuario existe y está activo
        if (!loginRepository.usuarioExisteYActivo(usuario)) {
            registrarIntentoFallido(clientIp, usuario, "Usuario no existe o está inactivo");
            return LoginResponse.failure("Credenciales inválidas");
        }
        
        // 4. Verificar credenciales usando la sucursal seleccionada por el usuario
        String sucursalSeleccionada = request.getSucursal();
        Optional<UsuarioInfo> usuarioInfo = loginRepository.verificarCredenciales(usuario, passwordHash, sucursalSeleccionada);
        
        if (usuarioInfo.isPresent()) {
            // Login exitoso
            limpiarIntentosFallidos(clientIp);
            
            if (securityProperties.getLogin().isLogSuccessfulLogins()) {
                auditLogger.info("LOGIN EXITOSO - Usuario: {} ({}), IP: {}, Sucursal: {}, Perfil: {}", 
                    usuarioInfo.get().getNombre(), usuario, clientIp, 
                    usuarioInfo.get().getSucursal(), usuarioInfo.get().getPerfil());
            }
            
            return LoginResponse.success(
                usuarioInfo.get(), 
                "Login exitoso. Bienvenido " + usuarioInfo.get().getNombre()
            );
            
        } else {
            // Login fallido
            registrarIntentoFallido(clientIp, usuario, "Credenciales incorrectas");
            return LoginResponse.failure("Credenciales inválidas");
        }
    }
    
    /**
     * Autenticación con generación de JWT token
     * Similar a autenticar() pero retorna un token JWT en lugar de crear sesión
     */
    @Override
    public LoginResponse autenticarConJwt(LoginRequest request, String clientIp) {
        // Si JWT no está disponible (ej: en tests), retornar error
        if (jwtTokenProvider == null) {
            logger.warn("JWT - JwtTokenProvider no disponible (probablemente en entorno de test)");
            return LoginResponse.failure("Autenticación JWT no disponible en este entorno");
        }
        
        String usuario = request.getUsuario();
        String sucursal = request.getSucursal();
        String passwordHash = request.getPasswordHash();
        
        logger.debug("Intento de autenticación JWT - Usuario: {}, Sucursal: {}, IP: {}", 
            usuario, sucursal, clientIp);
        
        // Validaciones previas (igual que autenticar)
        if (estaIpBloqueada(clientIp, usuario)) {
            long tiempoRestante = getTiempoRestanteBloqueo(clientIp);
            long minutosRestantes = (tiempoRestante / 60000) + 1;
            
            logger.warn("INTENTO DE LOGIN JWT BLOQUEADO - Usuario: {}, IP: {}, Tiempo restante: {} minutos", 
                usuario, clientIp, minutosRestantes);
            
            return LoginResponse.failure(String.format(
                "Demasiados intentos fallidos. Espere %d minutos antes de volver a intentar.", 
                minutosRestantes
            ));
        }
        
        if (!validarFormatoPasswordHash(passwordHash)) {
            logger.warn("JWT - Formato de password hash inválido para usuario: {}", usuario);
            return LoginResponse.failure("Formato de credenciales inválido");
        }
        
        // Intentar autenticar
        Optional<UsuarioInfo> usuarioInfoOpt = loginRepository.verificarCredenciales(usuario, passwordHash, sucursal);
        
        if (usuarioInfoOpt.isPresent()) {
            // Login exitoso - Generar JWT token
            UsuarioInfo usuarioInfo = usuarioInfoOpt.get();
            limpiarIntentosFallidos(clientIp);
            
            String token = jwtTokenProvider.generateToken(
                usuarioInfo.getEmpleado(),
                usuarioInfo.getNombre(),
                usuarioInfo.getSucursal(),
                usuarioInfo.getIdempresa(),
                usuarioInfo.getPerfil()
            );
            
            loginRepository.registrarIntentoLogin(usuario, clientIp, true);
            
            logger.info("✓ LOGIN JWT EXITOSO - Usuario: {}, Empleado: {}, Sucursal: {}, IP: {}", 
                usuario, usuarioInfo.getEmpleado(), sucursal, clientIp);
            
            auditLogger.info("JWT LOGIN - Usuario: {}, Empleado: {}, Nombre: {}, Sucursal: {}, Empresa: {}, Perfil: {}, IP: {}", 
                usuario, usuarioInfo.getEmpleado(), usuarioInfo.getNombre(), 
                usuarioInfo.getSucursal(), usuarioInfo.getIdempresa(), usuarioInfo.getPerfil(), clientIp);
            
            return LoginResponse.successWithToken(usuarioInfo, "Login exitoso - Token JWT generado", token);
            
        } else {
            // Login fallido
            registrarIntentoFallido(clientIp, usuario, "Credenciales incorrectas (JWT)");
            return LoginResponse.failure("Credenciales inválidas");
        }
    }
    
    @Override
    public boolean validarFormatoPasswordHash(String passwordHash) {
        return passwordHash != null && SHA256_PATTERN.matcher(passwordHash).matches();
    }
    
    @Override
    public List<Map<String, String>> obtenerSucursales() {
        try {
            logger.debug("Obteniendo lista de sucursales desde base de datos");
            
            List<Map<String, String>> sucursales = loginRepository.obtenerSucursales();
            
            logger.debug("Se obtuvieron {} sucursales", sucursales.size());
            return sucursales;
            
        } catch (Exception e) {
            logger.error("Error al obtener sucursales desde base de datos: {}", e.getMessage(), e);
            
            // Fallback con sucursales por defecto
            logger.warn("Usando sucursales por defecto debido a error en base de datos");
            return obtenerSucursalesPorDefecto();
        }
    }
    
    /**
     * Sucursales por defecto en caso de error en base de datos
     */
    private List<Map<String, String>> obtenerSucursalesPorDefecto() {
        return List.of(
            Map.of("sucursal", "001", "nombre", "SUCURSAL PRINCIPAL"),
            Map.of("sucursal", "002", "nombre", "SUCURSAL CENTRO"),
            Map.of("sucursal", "003", "nombre", "SUCURSAL NORTE")
        );
    }
    
    /**
     * Verifica si una IP está bloqueada por rate limiting
     */
    private boolean estaIpBloqueada(String ip, String usuario) {
        if (!securityProperties.getLogin().isEnableRateLimiting()) {
            return false;
        }
        
        IntentosLogin intentos = intentosFallidos.get(ip);
        if (intentos == null) {
            return false;
        }
        
        // Verificar si ha pasado el tiempo de bloqueo
        long tiempoTranscurrido = System.currentTimeMillis() - intentos.ultimoIntento;
        if (tiempoTranscurrido > securityProperties.getLogin().getLockoutDurationMs()) {
            intentosFallidos.remove(ip);
            return false;
        }
        
        // Determinar límite según perfil de usuario
        int limiteIntentos = securityProperties.getLogin().getMaxAttempts();
        
        return intentos.contador >= limiteIntentos;
    }
    
    /**
     * Obtiene el tiempo restante de bloqueo en milisegundos
     */
    private long getTiempoRestanteBloqueo(String ip) {
        IntentosLogin intentos = intentosFallidos.get(ip);
        if (intentos == null) {
            return 0;
        }
        
        long tiempoTranscurrido = System.currentTimeMillis() - intentos.ultimoIntento;
        return Math.max(0, securityProperties.getLogin().getLockoutDurationMs() - tiempoTranscurrido);
    }
    
    /**
     * Registra un intento fallido y aplica rate limiting
     */
    private void registrarIntentoFallido(String ip, String usuario, String razon) {
        // Registrar en auditoría siempre
        loginRepository.registrarIntentoLogin(usuario, ip, false);
        
        if (!securityProperties.getLogin().isEnableRateLimiting()) {
            return;
        }
        
        IntentosLogin intentos = intentosFallidos.computeIfAbsent(ip, k -> new IntentosLogin());
        intentos.contador++;
        intentos.ultimoIntento = System.currentTimeMillis();
        intentos.ultimoUsuario = usuario;
        
        if (securityProperties.getLogin().isLogFailedAttempts()) {
            auditLogger.warn("INTENTO FALLIDO #{} - Usuario: {}, IP: {}, Razón: {}", 
                intentos.contador, usuario, ip, razon);
        }
        
        // Alerta si se alcanza el límite
        if (intentos.contador >= securityProperties.getLogin().getMaxAttempts()) {
            auditLogger.error("🚨 IP BLOQUEADA por {} minutos - IP: {}, Usuario: {}, Total intentos: {}", 
                securityProperties.getLogin().getLockoutDurationMinutes(), ip, usuario, intentos.contador);
        }
    }
    
    /**
     * Limpia los intentos fallidos tras login exitoso
     */
    private void limpiarIntentosFallidos(String ip) {
        IntentosLogin intentos = intentosFallidos.remove(ip);
        if (intentos != null) {
            logger.debug("Limpiados {} intentos fallidos para IP: {}", intentos.contador, ip);
        }
    }
    
    /**
     * Limpieza automática de intentos fallidos antiguos
     * Se ejecuta cada hora para liberar memoria
     */
    @Scheduled(fixedRate = 3600000) // 1 hora en milisegundos
    public void limpiarIntentosAntiguos() {
        if (!securityProperties.getLogin().isEnableRateLimiting()) {
            return;
        }
        
        long tiempoLimite = System.currentTimeMillis() - securityProperties.getLogin().getLockoutDurationMs();
        AtomicInteger eliminados = new AtomicInteger(0);
        
        intentosFallidos.entrySet().removeIf(entry -> {
            if (entry.getValue().ultimoIntento < tiempoLimite) {
                eliminados.incrementAndGet();
                return true;
            }
            return false;
        });
        
        if (eliminados.get() > 0) {
            logger.debug("Limpieza automática: eliminados {} registros de intentos fallidos antiguos", eliminados.get());
        }
    }
    
    /**
     * Información de intentos de login por IP
     */
    private static class IntentosLogin {
        int contador = 0;
        long ultimoIntento = 0;
        String ultimoUsuario = "";
        
        @Override
        public String toString() {
            return String.format("IntentosLogin{contador=%d, ultimoIntento=%d, ultimoUsuario='%s'}", 
                contador, ultimoIntento, ultimoUsuario);
        }
    }
}
