# CORS Configuration Guide

Este documento resume la configuración actual de CORS en el proyecto `violetaserver` y detalla los pasos a seguir para ajustarla cuando el backend se despliegue en diferentes entornos.

## 1. Configuración vigente (modo desarrollo)

- Archivo principal: `src/main/java/com/lavioleta/desarrollo/violetaserver/config/CorsConfig.java`.
- Objetivo: permitir que la SPA en desarrollo (`http://localhost:8080`) consuma los endpoints REST expuestos por el backend (`http://localhost:5986`).
- Características:
  - Origen permitido: `http://localhost:8080`.
  - Métodos permitidos: `GET`, `POST`, `PUT`, `DELETE`, `OPTIONS`.
  - Headers permitidos: cualquier encabezado (`*`).
  - Credenciales habilitadas: `true` (permite Authorization headers, cookies, etc.).
  - Caché de preflight: `3600` segundos.

> **Nota**: Cuando Spring Security está activo, la configuración efectiva se declara en `src/main/java/com/lavioleta/desarrollo/violetaserver/config/SecurityConfig.java`. Allí también se añadió `http://localhost:8080` a la lista de `allowedOrigins`.

## 2. Pasos para despliegue en producción

### 2.1. Servidor WAN con IP pública

1. Identifica la URL completa de la aplicación web (por ejemplo `http://203.0.113.45:8080`).
2. En `SecurityConfig.java` (y, opcionalmente, en `CorsConfig.java`) reemplaza el origen de desarrollo por la URL pública:
   ```java
   configuration.setAllowedOrigins(List.of(
       "http://203.0.113.45:8080"
   ));
   ```
3. Si la aplicación web se sirve desde otro puerto o ruta, asegúrate de reflejarlo con precisión (ej. `http://203.0.113.45/app`).
4. Recompila y despliega el backend.

### 2.2. Servidor en la nube con dominio propio

1. Obtén el dominio asignado (por ejemplo `https://app.midominio.com`).
2. Actualiza las listas de orígenes permitidos con ese dominio:
   ```java
   configuration.setAllowedOrigins(List.of(
       "https://app.midominio.com"
   ));
   ```
3. Si la aplicación web utiliza subdominios múltiples, considera habilitar varios orígenes explícitos (`List.of(...)`) o evaluar una política dinámica.
4. Vuelve a construir y desplegar el backend.

## 3. Buenas prácticas adicionales

- **Propiedades por entorno**: utiliza variables de entorno o archivos `application-*.properties` para definir los orígenes sin modificar código fuente. Ejemplo:
  ```properties
  cors.allowed-origins=https://app.midominio.com
  ```
- **Perfiles Spring**: crea un perfil `prod` con un `CorsConfigurationSource` que lea estas propiedades, manteniendo el perfil `dev` con `localhost`.
- **Seguridad**: evita el uso de comodines (`*`) en producción. Especifica siempre los orígenes válidos.
- **Monitoreo**: registra los intentos de acceso bloqueados por CORS para detectar clientes mal configurados.

---

Con estos ajustes el backend quedará preparado para consumir solicitudes desde los estándares de desarrollo y producción, reduciendo problemas de política de mismo origen al exponer los endpoints REST en entornos WAN o dominios propios.
