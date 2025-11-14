# C++ original — contexto y reglas de interpretación

Este documento captura el contexto del sistema legacy en C++ Builder para guiar la migración.

## Fuentes principales
- Código: `src/cpp/*.cpp`, `src/cpp/*.h`
- Servidores/servicios: `ClassServidorBusquedas.cpp`, `ClassServidorCatalogos.cpp`

## Reglas de interpretación
- No portar concatenaciones SQL: reemplazar por consultas parametrizadas (QueryBuilder + `jdbcClient.param`).
- Identificar lógica de negocio acoplada a UI y desacoplarla en Services.
- Mapear nombres legacy a nuevos nombres en DTOs/Types (mantener tabla de correspondencias).

## Hallazgos típicos
- SQL embebido en métodos UI.
- Validaciones de entrada en eventos de formulario: mover a DTOs/backend y repetir en frontend.
- Formateos específicos (teléfono, CP, RFC) implementados en UI legacy: replicar como utilidades.

## Referencias
- Ver también: `.github/dfm-original.md`, `.github/sql-original.md`

---

## Playbooks de Migración (desde docs/migration-playbooks.md)

```markdown
# Migration Playbooks

Guides for migrating legacy C++ flows to Spring Boot + React.

## Interface Principles
- Keep UX consistent across search types
- Only vary form fields, data shapes, and specific validations

## Reusable Component Skeleton
Tabs + content panes with consistent actions (Search, Clear), loading indicators

## Avoid Legacy Pitfalls
1. Never build SQL with string concatenation
2. Avoid fragmented form state; use custom hooks
3. Don’t hardcode SQL conditions; encapsulate in builders
4. Sanitize all parameters

## Standard Migration Architecture
- Backend: Controller, Service, Repository (query builder), DTOs
- Frontend: Hook, Component, API service, Types
- Security: Sanitization, validation, audit logging

## Catalogs Migration (Clientes as reference)
- Identify all related tables and NOT NULL/constraints
- Multi-company DTO patterns with explicit defaults
- Repository with necessary JOINs
- @Transactional service for multi-table ops

## Checklists
Backend, Frontend, DB steps to ensure completeness (see full repo rules for details).
```