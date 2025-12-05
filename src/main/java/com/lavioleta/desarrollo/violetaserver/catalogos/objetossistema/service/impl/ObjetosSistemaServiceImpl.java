package com.lavioleta.desarrollo.violetaserver.catalogos.objetossistema.service.impl;

import com.lavioleta.desarrollo.violetaserver.catalogos.objetossistema.dto.request.ObjetoSistemaRequest;
import com.lavioleta.desarrollo.violetaserver.catalogos.objetossistema.dto.response.GrupoObjetoResponse;
import com.lavioleta.desarrollo.violetaserver.catalogos.objetossistema.dto.response.ObjetoSistemaResponse;
import com.lavioleta.desarrollo.violetaserver.catalogos.objetossistema.dto.response.PrivilegioResponse;
import com.lavioleta.desarrollo.violetaserver.catalogos.objetossistema.entity.GrupoObjeto;
import com.lavioleta.desarrollo.violetaserver.catalogos.objetossistema.entity.ObjetoSistema;
import com.lavioleta.desarrollo.violetaserver.catalogos.objetossistema.entity.Privilegio;
import com.lavioleta.desarrollo.violetaserver.catalogos.objetossistema.repository.ObjetosSistemaRepository;
import com.lavioleta.desarrollo.violetaserver.catalogos.objetossistema.service.ObjetosSistemaService;
import java.util.List;
import java.util.stream.Collectors;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

@Service
@RequiredArgsConstructor
@Slf4j
public class ObjetosSistemaServiceImpl implements ObjetosSistemaService {

    private final ObjetosSistemaRepository repository;

    @Override
    public List<ObjetoSistemaResponse> getAll() {
        return repository.findAll().stream()
                .map(this::mapToResponse)
                .collect(Collectors.toList());
    }

    @Override
    public ObjetoSistemaResponse getById(String objeto) {
        ObjetoSistema entity = repository.findById(objeto)
                .orElseThrow(() -> new RuntimeException("Objeto del sistema no encontrado: " + objeto));
        
        ObjetoSistemaResponse response = mapToResponse(entity);
        response.setPrivilegios(repository.findPrivilegiosByObjeto(objeto).stream()
                .map(this::mapPrivilegioToResponse)
                .collect(Collectors.toList()));
        return response;
    }

    @Override
    public List<GrupoObjetoResponse> getAllGrupos() {
        return repository.findAllGrupos().stream()
                .map(this::mapGrupoToResponse)
                .collect(Collectors.toList());
    }

    @Override
    @Transactional
    public ObjetoSistemaResponse create(ObjetoSistemaRequest request) {
        if (repository.existsById(request.getObjeto())) {
            throw new IllegalArgumentException("El objeto del sistema ya existe: " + request.getObjeto());
        }
        validateGrupo(request.getGrupo());

        ObjetoSistema entity = new ObjetoSistema();
        entity.setObjeto(request.getObjeto());
        entity.setNombre(request.getNombre());
        entity.setGrupo(request.getGrupo());

        repository.save(entity);
        createDefaultPrivilegios(request.getObjeto());

        return getById(request.getObjeto());
    }

    @Override
    @Transactional
    public ObjetoSistemaResponse update(String objeto, ObjetoSistemaRequest request) {
        if (!repository.existsById(objeto)) {
            throw new RuntimeException("Objeto del sistema no encontrado: " + objeto);
        }
        validateGrupo(request.getGrupo());

        ObjetoSistema entity = new ObjetoSistema();
        entity.setObjeto(objeto); // Ensure we update the correct ID
        entity.setNombre(request.getNombre());
        entity.setGrupo(request.getGrupo());

        repository.update(entity);

        return getById(objeto);
    }

    @Override
    @Transactional
    public void delete(String objeto) {
        if (!repository.existsById(objeto)) {
            throw new RuntimeException("Objeto del sistema no encontrado: " + objeto);
        }
        repository.deleteAsignacionPrivilegios(objeto);
        repository.deletePrivilegios(objeto);
        repository.deleteObjeto(objeto);
    }

    private void validateGrupo(String grupo) {
        if (!repository.existsGrupoById(grupo)) {
            throw new IllegalArgumentException("El grupo especificado no existe: " + grupo);
        }
    }

    private void createDefaultPrivilegios(String objeto) {
        createPrivilegio(objeto, "CON", "CONSULTAS");
        createPrivilegio(objeto, "MOD", "MODIFICACIONES");
        createPrivilegio(objeto, "ALT", "ALTAS");
        createPrivilegio(objeto, "BAJ", "BAJAS");
    }

    private void createPrivilegio(String objeto, String privilegioCode, String descripcion) {
        Privilegio priv = new Privilegio();
        priv.setObjeto(objeto);
        priv.setPrivilegio(privilegioCode);
        priv.setDescripcion(descripcion);
        repository.savePrivilegio(priv);
    }

    private ObjetoSistemaResponse mapToResponse(ObjetoSistema entity) {
        ObjetoSistemaResponse response = new ObjetoSistemaResponse();
        response.setObjeto(entity.getObjeto());
        response.setNombre(entity.getNombre());
        response.setGrupo(entity.getGrupo());
        return response;
    }

    private PrivilegioResponse mapPrivilegioToResponse(Privilegio entity) {
        PrivilegioResponse response = new PrivilegioResponse();
        response.setPrivilegio(entity.getPrivilegio());
        response.setDescripcion(entity.getDescripcion());
        return response;
    }

    private GrupoObjetoResponse mapGrupoToResponse(GrupoObjeto entity) {
        GrupoObjetoResponse response = new GrupoObjetoResponse();
        response.setGrupo(entity.getGrupo());
        response.setNombre(entity.getNombre());
        return response;
    }
}
