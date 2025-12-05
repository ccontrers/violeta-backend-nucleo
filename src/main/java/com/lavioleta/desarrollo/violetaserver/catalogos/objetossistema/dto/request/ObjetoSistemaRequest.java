package com.lavioleta.desarrollo.violetaserver.catalogos.objetossistema.dto.request;

import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.Size;
import lombok.Data;

@Data
public class ObjetoSistemaRequest {
    @NotBlank(message = "La clave del objeto es obligatoria")
    @Size(max = 10, message = "La clave del objeto no puede exceder 10 caracteres")
    private String objeto;

    @NotBlank(message = "El nombre es obligatorio")
    @Size(max = 40, message = "El nombre no puede exceder 40 caracteres")
    private String nombre;

    @NotBlank(message = "El grupo es obligatorio")
    @Size(max = 10, message = "El grupo no puede exceder 10 caracteres")
    private String grupo;
}
