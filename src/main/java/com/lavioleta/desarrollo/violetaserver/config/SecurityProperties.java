package com.lavioleta.desarrollo.violetaserver.config;

import org.springframework.boot.context.properties.ConfigurationProperties;
import org.springframework.stereotype.Component;
import org.springframework.validation.annotation.Validated;

import jakarta.validation.constraints.Min;
import jakarta.validation.constraints.Max;
import jakarta.validation.constraints.NotNull;

/**
 * Configuración de propiedades de seguridad desde application.properties
 */
@Component
@ConfigurationProperties(prefix = "security")
@Validated
public class SecurityProperties {
    
    private Login login = new Login();
    private Session session = new Session();
    
    /**
     * Configuración de login y rate limiting
     */
    public static class Login {
        @Min(value = 1, message = "Debe permitir al menos 1 intento")
        @Max(value = 10, message = "Máximo 10 intentos permitidos")
        private int maxAttempts = 5;
        
        @Min(value = 1, message = "Bloqueo mínimo de 1 minuto")
        @Max(value = 120, message = "Bloqueo máximo de 2 horas")
        private int lockoutDurationMinutes = 15;
        
        @Min(value = 1, message = "Debe permitir al menos 1 intento para admins")
        @Max(value = 20, message = "Máximo 20 intentos para admins")
        private int adminMaxAttempts = 8;
        
        @NotNull
        private boolean enableRateLimiting = true;
        
        @NotNull
        private boolean persistentLockout = true;
        
        @Min(value = 1, message = "Limpieza mínima cada 1 hora")
        @Max(value = 168, message = "Limpieza máxima cada 7 días")
        private int cleanupIntervalHours = 24;
        
        @NotNull
        private boolean enableAuditLog = true;
        
        @NotNull
        private boolean logFailedAttempts = true;
        
        @NotNull
        private boolean logSuccessfulLogins = false;
        
        // Getters y setters
        public int getMaxAttempts() { 
            return maxAttempts; 
        }
        
        public void setMaxAttempts(int maxAttempts) { 
            this.maxAttempts = maxAttempts; 
        }
        
        public int getLockoutDurationMinutes() { 
            return lockoutDurationMinutes; 
        }
        
        public void setLockoutDurationMinutes(int lockoutDurationMinutes) { 
            this.lockoutDurationMinutes = lockoutDurationMinutes; 
        }
        
        public int getAdminMaxAttempts() { 
            return adminMaxAttempts; 
        }
        
        public void setAdminMaxAttempts(int adminMaxAttempts) { 
            this.adminMaxAttempts = adminMaxAttempts; 
        }
        
        public boolean isEnableRateLimiting() { 
            return enableRateLimiting; 
        }
        
        public void setEnableRateLimiting(boolean enableRateLimiting) { 
            this.enableRateLimiting = enableRateLimiting; 
        }
        
        public boolean isPersistentLockout() { 
            return persistentLockout; 
        }
        
        public void setPersistentLockout(boolean persistentLockout) { 
            this.persistentLockout = persistentLockout; 
        }
        
        public int getCleanupIntervalHours() { 
            return cleanupIntervalHours; 
        }
        
        public void setCleanupIntervalHours(int cleanupIntervalHours) { 
            this.cleanupIntervalHours = cleanupIntervalHours; 
        }
        
        public boolean isEnableAuditLog() { 
            return enableAuditLog; 
        }
        
        public void setEnableAuditLog(boolean enableAuditLog) { 
            this.enableAuditLog = enableAuditLog; 
        }
        
        public boolean isLogFailedAttempts() { 
            return logFailedAttempts; 
        }
        
        public void setLogFailedAttempts(boolean logFailedAttempts) { 
            this.logFailedAttempts = logFailedAttempts; 
        }
        
        public boolean isLogSuccessfulLogins() { 
            return logSuccessfulLogins; 
        }
        
        public void setLogSuccessfulLogins(boolean logSuccessfulLogins) { 
            this.logSuccessfulLogins = logSuccessfulLogins; 
        }
        
        /**
         * Obtiene el número máximo de intentos según el perfil del usuario
         */
        public int getMaxAttemptsForProfile(String perfil) {
            if ("ADMIN".equalsIgnoreCase(perfil) || "ADMINISTRADOR".equalsIgnoreCase(perfil)) {
                return adminMaxAttempts;
            }
            return maxAttempts;
        }
        
        /**
         * Obtiene el tiempo de bloqueo en milisegundos
         */
        public long getLockoutDurationMs() {
            return lockoutDurationMinutes * 60L * 1000L;
        }
        
        /**
         * Obtiene el intervalo de limpieza en milisegundos
         */
        public long getCleanupIntervalMs() {
            return cleanupIntervalHours * 60L * 60L * 1000L;
        }
    }
    
    /**
     * Configuración de sesión de usuario
     */
    public static class Session {
        @Min(value = 1, message = "Sesión mínima de 1 hora")
        @Max(value = 24, message = "Sesión máxima de 24 horas")
        private int durationHours = 8;
        
        @NotNull
        private boolean extendOnActivity = true;
        
        @NotNull
        private boolean secureCookies = true;
        
        // Getters y setters
        public int getDurationHours() { 
            return durationHours; 
        }
        
        public void setDurationHours(int durationHours) { 
            this.durationHours = durationHours; 
        }
        
        public boolean isExtendOnActivity() { 
            return extendOnActivity; 
        }
        
        public void setExtendOnActivity(boolean extendOnActivity) { 
            this.extendOnActivity = extendOnActivity; 
        }
        
        public boolean isSecureCookies() { 
            return secureCookies; 
        }
        
        public void setSecureCookies(boolean secureCookies) { 
            this.secureCookies = secureCookies; 
        }
        
        /**
         * Obtiene la duración de la sesión en milisegundos
         */
        public long getDurationMs() {
            return durationHours * 60L * 60L * 1000L;
        }
    }
    
    // Getters principales
    public Login getLogin() { 
        return login; 
    }
    
    public void setLogin(Login login) { 
        this.login = login; 
    }
    
    public Session getSession() { 
        return session; 
    }
    
    public void setSession(Session session) { 
        this.session = session; 
    }
}
