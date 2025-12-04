package com.lavioleta.desarrollo.violetaserver.autorizacion.dto.security;

import java.util.List;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

@Data
@Builder
@NoArgsConstructor
@AllArgsConstructor
public class ObjetoPrivilegiosDTO {
    private String objeto;
    private List<String> privilegios;
}
