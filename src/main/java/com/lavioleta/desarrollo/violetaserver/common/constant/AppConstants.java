package com.lavioleta.desarrollo.violetaserver.common.constant;

public final class AppConstants {
    
    private AppConstants() {
        // Utility class
    }
    
    // API versioning
    public static final String API_V1 = "/api/v1";
    
    // Endpoints
    public static final String SUCURSALES_ENDPOINT = API_V1 + "/sucursales";
    
    // Messages
    public static final String SUCCESS_MESSAGE = "Operación exitosa";
    public static final String ERROR_MESSAGE = "Error en la operación";
}
