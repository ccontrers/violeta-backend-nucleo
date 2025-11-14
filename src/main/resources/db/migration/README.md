# ⚠️ NO MIGRACIONES FLYWAY

Este directorio existe para mantener la estructura del proyecto, pero **NO se deben crear archivos de migración Flyway**.

## Política del Proyecto

- ❌ **NO crear archivos V###__*.sql**
- ❌ **NO usar migraciones Flyway**
- ✅ **La base de datos ya tiene la estructura completa**
- ✅ **Usar solo consultas SELECT en development**

## Configuración Requerida

```properties
spring.jpa.hibernate.ddl-auto=none
spring.flyway.enabled=false
```

## Motivo

La base de datos `20250730_multiempresa` en MariaDB ya contiene:
- Todas las tablas necesarias
- Datos de producción
- Estructura completa y funcional

**Cualquier cambio de esquema debe ser coordinado con el administrador de BD.**

---

**Nota**: Este README sirve como recordatorio de la política establecida en `.copilot-rules`
