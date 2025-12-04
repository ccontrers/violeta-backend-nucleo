package com.lavioleta.desarrollo.violetaserver.autorizacion.dto.security;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

@Data
@Builder
@NoArgsConstructor
@AllArgsConstructor
public class PrivilegeCheckResponse {
    private String objeto;
    private String privilegio;
    private boolean allowed;
}
