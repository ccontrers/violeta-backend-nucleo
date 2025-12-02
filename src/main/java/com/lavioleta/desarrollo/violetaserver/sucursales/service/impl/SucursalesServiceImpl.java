package com.lavioleta.desarrollo.violetaserver.sucursales.service.impl;

import java.util.List;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Service;

import com.lavioleta.desarrollo.violetaserver.sucursales.dto.response.SucursalComboOptionResponse;
import com.lavioleta.desarrollo.violetaserver.sucursales.repository.SucursalesRepository;
import com.lavioleta.desarrollo.violetaserver.sucursales.service.SucursalesService;

@Service
public class SucursalesServiceImpl implements SucursalesService {

    private static final Logger log = LoggerFactory.getLogger(SucursalesServiceImpl.class);

    private final SucursalesRepository repository;

    public SucursalesServiceImpl(SucursalesRepository repository) {
        this.repository = repository;
    }

    @Override
    public List<SucursalComboOptionResponse> listarSucursalesCombo(Integer idEmpresa) {
        log.debug("Listando sucursales para combo (idEmpresa={})", idEmpresa);
        return repository.listarSucursalesCombo(idEmpresa);
    }
}
