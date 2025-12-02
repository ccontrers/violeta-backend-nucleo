package com.lavioleta.desarrollo.violetaserver.sucursales.service;

import java.util.List;

import com.lavioleta.desarrollo.violetaserver.sucursales.dto.response.SucursalComboOptionResponse;

public interface SucursalesService {

    List<SucursalComboOptionResponse> listarSucursalesCombo(Integer idEmpresa);
}
