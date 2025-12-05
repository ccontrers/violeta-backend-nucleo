package com.lavioleta.desarrollo.violetaserver.catalogos.objetossistema.dto.response;

import java.util.List;
import lombok.Data;

@Data
public class ObjetoSistemaResponse {
    private String objeto;
    private String nombre;
    private String grupo;
    private List<PrivilegioResponse> privilegios;
}
