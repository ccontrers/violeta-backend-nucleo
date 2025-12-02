package com.lavioleta.desarrollo.violetaserver.usuarios.dto.response;

import io.swagger.v3.oas.annotations.media.Schema;

/**
 * DTO de respuesta para operaciones de clave (asignar / cambiar).
 * Replica la respuesta del legado que devuelve { "usuario": "XXX" } en caso de éxito.
 */
@Schema(description = "Respuesta de operación de clave de usuario")
public class ClaveResponse {

    @Schema(description = "Indica si la operación fue exitosa")
    private boolean success;

    @Schema(description = "Identificador del usuario afectado", example = "CRCP")
    private String usuario;

    @Schema(description = "Mensaje descriptivo del resultado")
    private String message;

    public ClaveResponse() {
    }

    private ClaveResponse(boolean success, String usuario, String message) {
        this.success = success;
        this.usuario = usuario;
        this.message = message;
    }

    public static ClaveResponse exito(String usuario, String message) {
        return new ClaveResponse(true, usuario, message);
    }

    public static ClaveResponse error(String message) {
        return new ClaveResponse(false, null, message);
    }

    public boolean isSuccess() {
        return success;
    }

    public void setSuccess(boolean success) {
        this.success = success;
    }

    public String getUsuario() {
        return usuario;
    }

    public void setUsuario(String usuario) {
        this.usuario = usuario;
    }

    public String getMessage() {
        return message;
    }

    public void setMessage(String message) {
        this.message = message;
    }
}
