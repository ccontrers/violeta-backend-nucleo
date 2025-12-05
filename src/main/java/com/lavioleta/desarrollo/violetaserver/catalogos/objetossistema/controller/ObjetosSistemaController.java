package com.lavioleta.desarrollo.violetaserver.catalogos.objetossistema.controller;

import com.lavioleta.desarrollo.violetaserver.catalogos.objetossistema.dto.request.ObjetoSistemaRequest;
import com.lavioleta.desarrollo.violetaserver.catalogos.objetossistema.dto.response.GrupoObjetoResponse;
import com.lavioleta.desarrollo.violetaserver.catalogos.objetossistema.dto.response.ObjetoSistemaResponse;
import com.lavioleta.desarrollo.violetaserver.catalogos.objetossistema.service.ObjetosSistemaService;
import io.swagger.v3.oas.annotations.Operation;
import io.swagger.v3.oas.annotations.tags.Tag;
import jakarta.validation.Valid;
import java.util.List;
import lombok.RequiredArgsConstructor;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.DeleteMapping;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

@RestController
@RequestMapping("/api/v1/objetos-sistema")
@RequiredArgsConstructor
@Tag(name = "Catálogo de Objetos del Sistema", description = "API para la administración de objetos del sistema y sus privilegios")
public class ObjetosSistemaController {

    private final ObjetosSistemaService service;

    @GetMapping
    @Operation(summary = "Listar todos los objetos del sistema")
    public ResponseEntity<List<ObjetoSistemaResponse>> getAll() {
        return ResponseEntity.ok(service.getAll());
    }

    @GetMapping("/{objeto}")
    @Operation(summary = "Obtener un objeto del sistema por su clave")
    public ResponseEntity<ObjetoSistemaResponse> getById(@PathVariable String objeto) {
        return ResponseEntity.ok(service.getById(objeto));
    }

    @GetMapping("/grupos")
    @Operation(summary = "Listar todos los grupos de objetos")
    public ResponseEntity<List<GrupoObjetoResponse>> getAllGrupos() {
        return ResponseEntity.ok(service.getAllGrupos());
    }

    @PostMapping
    @Operation(summary = "Crear un nuevo objeto del sistema")
    public ResponseEntity<ObjetoSistemaResponse> create(@Valid @RequestBody ObjetoSistemaRequest request) {
        return new ResponseEntity<>(service.create(request), HttpStatus.CREATED);
    }

    @PutMapping("/{objeto}")
    @Operation(summary = "Actualizar un objeto del sistema existente")
    public ResponseEntity<ObjetoSistemaResponse> update(@PathVariable String objeto, @Valid @RequestBody ObjetoSistemaRequest request) {
        return ResponseEntity.ok(service.update(objeto, request));
    }

    @DeleteMapping("/{objeto}")
    @Operation(summary = "Eliminar un objeto del sistema")
    public ResponseEntity<Void> delete(@PathVariable String objeto) {
        service.delete(objeto);
        return ResponseEntity.noContent().build();
    }
}
