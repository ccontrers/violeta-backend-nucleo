package com.lavioleta.desarrollo.violetaserver.autorizacion.controller;

import java.security.Principal;
import java.util.List;

import org.springframework.http.ResponseEntity;
import org.springframework.security.access.prepost.PreAuthorize;
import org.springframework.util.StringUtils;
import org.springframework.web.bind.annotation.DeleteMapping;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import com.lavioleta.desarrollo.violetaserver.autorizacion.dto.security.ObjetoPrivilegiosDTO;
import com.lavioleta.desarrollo.violetaserver.autorizacion.dto.security.PrivilegioDTO;
import com.lavioleta.desarrollo.violetaserver.autorizacion.dto.security.PrivilegeCheckResponse;
import com.lavioleta.desarrollo.violetaserver.autorizacion.dto.security.VersionRequirementDTO;
import com.lavioleta.desarrollo.violetaserver.autorizacion.service.AuthorizationService;

import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;

@RestController
@RequestMapping("/api/v1/security/privilegios")
@RequiredArgsConstructor
@Slf4j
public class AuthorizationController {

    private final AuthorizationService authorizationService;

    @GetMapping
    public ResponseEntity<List<ObjetoPrivilegiosDTO>> listar(Principal principal) {
        String usuario = resolveUsuario(principal);
        log.info("[AuthorizationController] Listando privilegios agrupados para {}", usuario);
        return ResponseEntity.ok(authorizationService.getPrivilegiosAgrupados(usuario));
    }

    @GetMapping("/self")
    public ResponseEntity<List<ObjetoPrivilegiosDTO>> listarSelf(Principal principal) {
        return listar(principal);
    }

    @GetMapping("/{objeto}")
    public ResponseEntity<List<PrivilegioDTO>> listarPorObjeto(@PathVariable String objeto, Principal principal) {
        String usuario = resolveUsuario(principal);
        log.info("[AuthorizationController] Listando privilegios para usuario={} objeto={}", usuario, objeto);
        List<PrivilegioDTO> privilegios = authorizationService.getPrivilegios(usuario, objeto);
        if (privilegios.isEmpty()) {
            return ResponseEntity.notFound().build();
        }
        return ResponseEntity.ok(privilegios);
    }

    @GetMapping("/{objeto}/{privilegio}")
    public ResponseEntity<PrivilegeCheckResponse> revisar(@PathVariable String objeto,
            @PathVariable String privilegio, Principal principal) {
        String usuario = resolveUsuario(principal);
        log.info("[AuthorizationController] Revisión de privilegio usuario={} objeto={} privilegio={}", usuario, objeto, privilegio);
        return ResponseEntity.ok(authorizationService.revisarPrivilegio(usuario, objeto, privilegio));
    }

    @GetMapping("/version/requerimientos")
    public ResponseEntity<VersionRequirementDTO> version() {
        return ResponseEntity.ok(authorizationService.getVersionRequirements());
    }

    @PostMapping("/cache/flush")
    @PreAuthorize("hasRole('ADMIN') or hasAuthority('SECURITY_CACHE_FLUSH')")
    public ResponseEntity<Void> flushCache() {
        log.info("Cache de privilegios invalidada manualmente");
        authorizationService.evictAll();
        return ResponseEntity.accepted().build();
    }

    @DeleteMapping("/cache/{usuario}")
    @PreAuthorize("hasRole('ADMIN') or hasAuthority('SECURITY_CACHE_FLUSH')")
    public ResponseEntity<Void> flushCacheUsuario(@PathVariable String usuario) {
        log.info("Cache de privilegios invalidada para usuario={}", usuario);
        authorizationService.evictUsuario(usuario);
        return ResponseEntity.noContent().build();
    }

    @DeleteMapping("/cache/self")
    public ResponseEntity<Void> flushCachePropio(Principal principal) {
        String usuario = resolveUsuario(principal);
        log.info("Cache de privilegios invalidada para usuario autenticado={}", usuario);
        authorizationService.evictUsuario(usuario);
        return ResponseEntity.noContent().build();
    }

    private String resolveUsuario(Principal principal) {
        if (principal == null || !StringUtils.hasText(principal.getName())) {
            throw new IllegalStateException("No se pudo resolver el usuario autenticado");
        }
        return principal.getName();
    }
}
