package com.lavioleta.desarrollo.violetaserver.catalogos.objetossistema.service;

import com.lavioleta.desarrollo.violetaserver.catalogos.objetossistema.dto.request.ObjetoSistemaRequest;
import com.lavioleta.desarrollo.violetaserver.catalogos.objetossistema.dto.response.GrupoObjetoResponse;
import com.lavioleta.desarrollo.violetaserver.catalogos.objetossistema.dto.response.ObjetoSistemaResponse;
import java.util.List;

public interface ObjetosSistemaService {
    List<ObjetoSistemaResponse> getAll();
    ObjetoSistemaResponse getById(String objeto);
    List<GrupoObjetoResponse> getAllGrupos();
    ObjetoSistemaResponse create(ObjetoSistemaRequest request);
    ObjetoSistemaResponse update(String objeto, ObjetoSistemaRequest request);
    void delete(String objeto);
}
