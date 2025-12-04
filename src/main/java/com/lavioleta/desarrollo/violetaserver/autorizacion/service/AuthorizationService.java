package com.lavioleta.desarrollo.violetaserver.autorizacion.service;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.stream.Collectors;

import org.springframework.stereotype.Service;
import org.springframework.util.StringUtils;

import com.github.benmanes.caffeine.cache.Cache;
import com.github.benmanes.caffeine.cache.Caffeine;
import com.lavioleta.desarrollo.violetaserver.autorizacion.config.AuthorizationProperties;
import com.lavioleta.desarrollo.violetaserver.autorizacion.dto.security.ObjetoPrivilegiosDTO;
import com.lavioleta.desarrollo.violetaserver.autorizacion.dto.security.PrivilegioDTO;
import com.lavioleta.desarrollo.violetaserver.autorizacion.dto.security.PrivilegeCheckResponse;
import com.lavioleta.desarrollo.violetaserver.autorizacion.dto.security.VersionRequirementDTO;
import com.lavioleta.desarrollo.violetaserver.autorizacion.repository.AuthorizationRepository;
import com.lavioleta.desarrollo.violetaserver.autorizacion.repository.AuthorizationRepository.PrivilegeRecord;
import com.lavioleta.desarrollo.violetaserver.autorizacion.security.DomainPermissionMapper;

@Service
public class AuthorizationService {

    private final AuthorizationRepository authorizationRepository;
    private final AuthorizationProperties authorizationProperties;
    private final DomainPermissionMapper domainPermissionMapper;
    private final Cache<String, PrivilegeCacheEntry> privilegeCache;

    public AuthorizationService(AuthorizationRepository authorizationRepository,
                                AuthorizationProperties authorizationProperties,
                                DomainPermissionMapper domainPermissionMapper) {
        this.authorizationRepository = authorizationRepository;
        this.authorizationProperties = authorizationProperties;
        this.domainPermissionMapper = domainPermissionMapper;
        this.privilegeCache = Caffeine.newBuilder()
            .expireAfterWrite(authorizationProperties.getCache().getTtl())
            .maximumSize(authorizationProperties.getCache().getMaximumSize())
            .build();
    }

    public List<PrivilegioDTO> getPrivilegios(String usuario, String objeto) {
        validarUsuario(usuario);
        PrivilegeCacheEntry entry = loadEntry(usuario);
        List<PrivilegioDTO> cached = entry.privilegiosPorObjeto().get(objeto);
        if (cached != null) {
            return cached;
        }
        List<PrivilegioDTO> desdeRepo = fetchPrivilegios(usuario, objeto);
        privilegeCache.put(usuario, entry.withObjeto(objeto, desdeRepo));
        return desdeRepo;
    }

    public List<ObjetoPrivilegiosDTO> getPrivilegiosAgrupados(String usuario) {
        validarUsuario(usuario);
        PrivilegeCacheEntry entry = loadEntry(usuario);
        return entry.privilegiosPorObjeto().entrySet().stream()
            .map(e -> ObjetoPrivilegiosDTO.builder()
                .objeto(e.getKey())
                .privilegios(e.getValue().stream().map(PrivilegioDTO::getPrivilegio).toList())
                .build())
            .toList();
    }

    public PrivilegeCheckResponse revisarPrivilegio(String usuario, String objeto, String privilegio) {
        boolean allowed = hasPrivilege(usuario, objeto, privilegio);
        return PrivilegeCheckResponse.builder()
            .objeto(objeto)
            .privilegio(privilegio)
            .allowed(allowed)
            .build();
    }

    public boolean hasPrivilege(String usuario, String objeto, String privilegio) {
        if (!authorizationProperties.isEnforcementEnabled()) {
            return true;
        }
        validarUsuario(usuario);
        if (!StringUtils.hasText(objeto) || !StringUtils.hasText(privilegio)) {
            return false;
        }
        PrivilegeCacheEntry entry = loadEntry(usuario);
        List<PrivilegioDTO> privilegios = entry.privilegiosPorObjeto().get(objeto);
        if (privilegios == null) {
            privilegios = fetchPrivilegios(usuario, objeto);
            entry = entry.withObjeto(objeto, privilegios);
            privilegeCache.put(usuario, entry);
        }
        String normalized = privilegio.toUpperCase();
        return privilegios.stream()
            .map(PrivilegioDTO::getPrivilegio)
            .anyMatch(p -> p.equalsIgnoreCase(normalized));
    }

    public Set<String> getAuthoritiesForUsuario(String usuario) {
        validarUsuario(usuario);
        PrivilegeCacheEntry entry = loadEntry(usuario);
        return entry.privilegiosPorObjeto().entrySet().stream()
            .flatMap(e -> e.getValue().stream()
                .map(priv -> domainPermissionMapper.toAuthority(e.getKey(), priv.getPrivilegio())))
            .collect(Collectors.toSet());
    }

    public VersionRequirementDTO getVersionRequirements() {
        Optional<VersionRequirementDTO> fromDb = authorizationRepository.findVersionRequirements();
        return fromDb.orElse(VersionRequirementDTO.builder()
            .versionMinima(authorizationProperties.getVersion().getMin())
            .subversionMinima(authorizationProperties.getVersion().getSub())
            .tiempoValidezMinutos(authorizationProperties.getVersion().getGraceMinutes())
            .build());
    }

    public void evictUsuario(String usuario) {
        privilegeCache.invalidate(usuario);
    }

    public void evictAll() {
        privilegeCache.invalidateAll();
    }

    private PrivilegeCacheEntry loadEntry(String usuario) {
        return privilegeCache.get(usuario, this::loadAllPrivilegios);
    }

    private PrivilegeCacheEntry loadAllPrivilegios(String usuario) {
        List<PrivilegioDTO> privilegios = authorizationRepository.findAllByUsuario(usuario)
            .stream()
            .map(this::mapToDto)
            .toList();
        Map<String, List<PrivilegioDTO>> agrupados = privilegios.stream()
            .collect(Collectors.groupingBy(PrivilegioDTO::getObjeto, ConcurrentHashMap::new, Collectors.toCollection(ArrayList::new)));
        return new PrivilegeCacheEntry(agrupados);
    }

    private List<PrivilegioDTO> fetchPrivilegios(String usuario, String objeto) {
        List<PrivilegeRecord> records = authorizationRepository.findAllByUsuarioAndObjeto(usuario, objeto);
        if (records.isEmpty()) {
            return Collections.emptyList();
        }
        return records.stream().map(this::mapToDto).toList();
    }

    private void validarUsuario(String usuario) {
        if (!StringUtils.hasText(usuario)) {
            throw new IllegalArgumentException("Usuario requerido para consultar privilegios");
        }
    }

    private PrivilegioDTO mapToDto(PrivilegeRecord record) {
        return PrivilegioDTO.builder()
            .objeto(record.objeto())
            .privilegio(record.privilegio())
            .descripcion(record.descripcion())
            .build();
    }

    private static class PrivilegeCacheEntry {
        private final Map<String, List<PrivilegioDTO>> privilegiosPorObjeto;

        private PrivilegeCacheEntry(Map<String, List<PrivilegioDTO>> privilegiosPorObjeto) {
            Map<String, List<PrivilegioDTO>> sanitized = new HashMap<>();
            privilegiosPorObjeto.forEach((objeto, lista) ->
                sanitized.put(objeto, Collections.unmodifiableList(new ArrayList<>(lista))));
            this.privilegiosPorObjeto = Collections.unmodifiableMap(sanitized);
        }

        Map<String, List<PrivilegioDTO>> privilegiosPorObjeto() {
            return privilegiosPorObjeto;
        }

        PrivilegeCacheEntry withObjeto(String objeto, List<PrivilegioDTO> privilegios) {
            Map<String, List<PrivilegioDTO>> copy = new HashMap<>(privilegiosPorObjeto);
            copy.put(objeto, Collections.unmodifiableList(new ArrayList<>(privilegios)));
            return new PrivilegeCacheEntry(copy);
        }
    }
}
