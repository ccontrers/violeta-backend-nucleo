package com.lavioleta.desarrollo.violetaserver.config;

import io.swagger.v3.oas.annotations.OpenAPIDefinition;
import io.swagger.v3.oas.annotations.enums.SecuritySchemeIn;
import io.swagger.v3.oas.annotations.enums.SecuritySchemeType;
import io.swagger.v3.oas.annotations.info.Contact;
import io.swagger.v3.oas.annotations.info.Info;
import io.swagger.v3.oas.annotations.info.License;
import io.swagger.v3.oas.annotations.security.SecurityRequirement;
import io.swagger.v3.oas.annotations.security.SecurityScheme;
import io.swagger.v3.oas.annotations.servers.Server;
import org.springframework.context.annotation.Configuration;

/**
 * Configuración de OpenAPI / Swagger para la documentación de los endpoints REST.
 *
 * Accesos por defecto después de levantar la aplicación:
 * - JSON OpenAPI: /v3/api-docs
 * - Swagger UI: /swagger-ui.html o /swagger-ui/index.html
 * 
 * Autenticación - Soporta DOS métodos:
 * 
 * 1. COOKIE/SESSION (cookieAuth) - Para aplicaciones web:
 *    - Ejecuta POST /api/v1/auth/login
 *    - La sesión se crea automáticamente (cookie JSESSIONID)
 *    - Los requests subsecuentes envían la cookie automáticamente
 *    - No requiere configurar "Authorize" en Swagger
 * 
 * 2. JWT TOKEN (bearerAuth) - Para APIs/Mobile/Scripts:
 *    - Ejecuta POST /api/v1/auth/login/jwt
 *    - Recibes un token JWT en el campo "token" del response
 *    - Copia el token y haz clic en "Authorize" en Swagger UI
 *    - Pega el token en el campo bearerAuth
 *    - Los requests incluirán header: Authorization: Bearer {token}
 */
@Configuration
@OpenAPIDefinition(
        info = @Info(
                title = "Violeta API",
                version = "v1",
                description = """
                        Documentación de endpoints del backend Violeta
                        
                        **Autenticación - Dos métodos disponibles:**
                        
                        **1. Cookie/Session (Para aplicaciones web):**
                        - Ejecuta POST /api/v1/auth/login con tus credenciales
                        - La sesión se crea automáticamente (cookie JSESSIONID)
                        - Los siguientes requests usan la sesión automáticamente
                        - No necesitas configurar nada en "Authorize"
                        
                        **2. JWT Token (Para APIs, móviles, scripts):**
                        - Ejecuta POST /api/v1/auth/login/jwt con tus credenciales
                        - Recibes un token JWT en el campo "token" del response
                        - Haz clic en "Authorize" arriba a la derecha
                        - Pega el token en "bearerAuth" (sin agregar "Bearer ")
                        - El token expira en 4 horas
                        
                        **¿Cuál usar?**
                        - Aplicación web React → Cookie/Session (endpoint /login)
                        - App móvil/API/Scripts → JWT (endpoint /login/jwt)
                        """,
                contact = @Contact(name = "Equipo Desarrollo", email = "dev@lavioleta.com"),
                license = @License(name = "Propietario", url = "https://lavioleta.com")
        ),
        servers = {
                @Server(url = "http://localhost:5986", description = "Desarrollo Local")
        },
        security = {
                @SecurityRequirement(name = "cookieAuth"),
                @SecurityRequirement(name = "bearerAuth")
        }
)
@SecurityScheme(
        name = "cookieAuth",
        type = SecuritySchemeType.APIKEY,
        in = SecuritySchemeIn.COOKIE,
        paramName = "JSESSIONID",
        description = """
                **Autenticación basada en sesión HTTP (para aplicaciones web)**
                
                1. Ejecuta POST /api/v1/auth/login con tus credenciales
                2. La sesión se crea automáticamente (cookie JSESSIONID)
                3. Las cookies se envían automáticamente en los siguientes requests
                
                Ideal para: Aplicación web React, navegadores
                """
)
@SecurityScheme(
        name = "bearerAuth",
        type = SecuritySchemeType.HTTP,
        scheme = "bearer",
        bearerFormat = "JWT",
        description = """
                **Autenticación basada en JWT (para APIs, móviles, scripts)**
                
                1. Ejecuta POST /api/v1/auth/login/jwt con tus credenciales
                2. Copia el token del campo "token" en el response
                3. Haz clic en "Authorize" arriba a la derecha
                4. Pega el token en este campo (sin agregar "Bearer ")
                5. Usa el token en tus requests: Authorization: Bearer {token}
                
                El token expira en 4 horas.
                
                Ideal para: Apps móviles, APIs externas, scripts de automatización
                """
)
public class OpenApiConfig {
    // Si se requiere personalización adicional, se pueden definir beans aquí.
}
