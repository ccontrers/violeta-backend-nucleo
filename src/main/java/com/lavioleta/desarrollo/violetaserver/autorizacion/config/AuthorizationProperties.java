package com.lavioleta.desarrollo.violetaserver.autorizacion.config;

import java.time.Duration;

import org.springframework.boot.context.properties.ConfigurationProperties;
import org.springframework.stereotype.Component;
import org.springframework.validation.annotation.Validated;

import jakarta.validation.constraints.Min;
import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.NotNull;

/**
 * Propiedades externas del subsistema de autorización.
 */
@Component
@ConfigurationProperties(prefix = "security.authorization")
@Validated
public class AuthorizationProperties {

    private final Cache cache = new Cache();
    private final Version version = new Version();

    /**
     * Permite desactivar temporalmente la verificación de privilegios.
     */
    private boolean enforcementEnabled = true;

    public Cache getCache() {
        return cache;
    }

    public Version getVersion() {
        return version;
    }

    public boolean isEnforcementEnabled() {
        return enforcementEnabled;
    }

    public void setEnforcementEnabled(boolean enforcementEnabled) {
        this.enforcementEnabled = enforcementEnabled;
    }

    public static class Cache {
        @NotNull
        private Duration ttl = Duration.ofMinutes(5);

        @Min(10)
        private long maximumSize = 500;

        public Duration getTtl() {
            return ttl;
        }

        public void setTtl(Duration ttl) {
            this.ttl = ttl;
        }

        public long getMaximumSize() {
            return maximumSize;
        }

        public void setMaximumSize(long maximumSize) {
            this.maximumSize = maximumSize;
        }
    }

    public static class Version {
        @NotBlank
        private String min = "1.0";

        @NotBlank
        private String sub = "0";

        @Min(0)
        private int graceMinutes = 0;

        public String getMin() {
            return min;
        }

        public void setMin(String min) {
            this.min = min;
        }

        public String getSub() {
            return sub;
        }

        public void setSub(String sub) {
            this.sub = sub;
        }

        public int getGraceMinutes() {
            return graceMinutes;
        }

        public void setGraceMinutes(int graceMinutes) {
            this.graceMinutes = graceMinutes;
        }
    }
}
