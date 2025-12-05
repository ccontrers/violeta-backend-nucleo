package com.lavioleta.desarrollo.violetaserver.catalogos.objetossistema.controller;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.doNothing;
import static org.mockito.Mockito.when;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.delete;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.get;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.post;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.put;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.jsonPath;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.status;

import com.fasterxml.jackson.databind.ObjectMapper;
import com.lavioleta.desarrollo.violetaserver.catalogos.objetossistema.dto.request.ObjetoSistemaRequest;
import com.lavioleta.desarrollo.violetaserver.catalogos.objetossistema.dto.response.GrupoObjetoResponse;
import com.lavioleta.desarrollo.violetaserver.catalogos.objetossistema.dto.response.ObjetoSistemaResponse;
import com.lavioleta.desarrollo.violetaserver.catalogos.objetossistema.service.ObjetosSistemaService;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.web.servlet.WebMvcTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.http.MediaType;
import org.springframework.test.web.servlet.MockMvc;

@WebMvcTest(ObjetosSistemaController.class)
class ObjetosSistemaControllerTest {

    @Autowired
    private MockMvc mockMvc;

    @MockBean
    private ObjetosSistemaService service;

    @Autowired
    private ObjectMapper objectMapper;

    @Test
    void getAll_ShouldReturnList() throws Exception {
        ObjetoSistemaResponse response = new ObjetoSistemaResponse();
        response.setObjeto("TEST");
        response.setNombre("Test Object");
        response.setGrupo("GROUP");

        when(service.getAll()).thenReturn(Collections.singletonList(response));

        mockMvc.perform(get("/api/v1/objetos-sistema"))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$[0].objeto").value("TEST"))
                .andExpect(jsonPath("$[0].nombre").value("Test Object"));
    }

    @Test
    void getById_WhenExists_ShouldReturnObject() throws Exception {
        ObjetoSistemaResponse response = new ObjetoSistemaResponse();
        response.setObjeto("TEST");
        response.setNombre("Test Object");

        when(service.getById("TEST")).thenReturn(response);

        mockMvc.perform(get("/api/v1/objetos-sistema/TEST"))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.objeto").value("TEST"));
    }

    @Test
    void getAllGrupos_ShouldReturnList() throws Exception {
        GrupoObjetoResponse response = new GrupoObjetoResponse();
        response.setGrupo("GROUP");
        response.setNombre("Group Name");

        when(service.getAllGrupos()).thenReturn(Collections.singletonList(response));

        mockMvc.perform(get("/api/v1/objetos-sistema/grupos"))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$[0].grupo").value("GROUP"));
    }

    @Test
    void create_WhenValid_ShouldReturnCreated() throws Exception {
        ObjetoSistemaRequest request = new ObjetoSistemaRequest();
        request.setObjeto("NEW");
        request.setNombre("New Object");
        request.setGrupo("GROUP");

        ObjetoSistemaResponse response = new ObjetoSistemaResponse();
        response.setObjeto("NEW");
        response.setNombre("New Object");
        response.setGrupo("GROUP");

        when(service.create(any(ObjetoSistemaRequest.class))).thenReturn(response);

        mockMvc.perform(post("/api/v1/objetos-sistema")
                .contentType(MediaType.APPLICATION_JSON)
                .content(objectMapper.writeValueAsString(request)))
                .andExpect(status().isCreated())
                .andExpect(jsonPath("$.objeto").value("NEW"));
    }

    @Test
    void update_WhenValid_ShouldReturnUpdated() throws Exception {
        ObjetoSistemaRequest request = new ObjetoSistemaRequest();
        request.setObjeto("UPDATED");
        request.setNombre("Updated Object");
        request.setGrupo("GROUP");

        ObjetoSistemaResponse response = new ObjetoSistemaResponse();
        response.setObjeto("UPDATED");
        response.setNombre("Updated Object");
        response.setGrupo("GROUP");

        when(service.update(eq("UPDATED"), any(ObjetoSistemaRequest.class))).thenReturn(response);

        mockMvc.perform(put("/api/v1/objetos-sistema/UPDATED")
                .contentType(MediaType.APPLICATION_JSON)
                .content(objectMapper.writeValueAsString(request)))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.objeto").value("UPDATED"));
    }

    @Test
    void delete_WhenExists_ShouldReturnNoContent() throws Exception {
        doNothing().when(service).delete("TEST");

        mockMvc.perform(delete("/api/v1/objetos-sistema/TEST"))
                .andExpect(status().isNoContent());
    }

    @Test
    void create_WhenInvalid_ShouldReturnBadRequest() throws Exception {
        ObjetoSistemaRequest request = new ObjetoSistemaRequest();
        // Missing required fields

        mockMvc.perform(post("/api/v1/objetos-sistema")
                .contentType(MediaType.APPLICATION_JSON)
                .content(objectMapper.writeValueAsString(request)))
                .andExpect(status().isBadRequest());
    }
}
