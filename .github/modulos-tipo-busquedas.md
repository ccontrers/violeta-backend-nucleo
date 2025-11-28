# Módulos tipo Búsquedas# Módulos tipo Búsquedas# Módulos tipo — Búsquedas



Guía para migrar módulos de búsqueda desde el sistema legado C++ Builder a Spring Boot + React, basada en las migraciones exitosas de **Búsqueda de Artículos** y **Búsqueda de Clientes**.



## IntroducciónPatrones y estándares para módulos de búsqueda en VioletaServer.## Arquitectura general



Los **módulos de búsqueda** en el sistema legacy son interfaces especializadas que permiten localizar registros mediante múltiples criterios de filtrado. Se caracterizan por:- Endpoint REST (Controller) → Service → Repository



- **Múltiples tipos de búsqueda** en un solo endpoint (por nombre, código, RFC, etc.)## Módulos Implementados- Respuesta con paginación y filtros.

- **Respuestas con metadatos** (clasificaciones, marcas, totales)

- **Filtros dinámicos** según configuración de empresa- Backend: QueryBuilder para filtros dinámicos.

- **Paginación** para manejar grandes volúmenes de datos

- **Performance crítica** debido al uso intensivo### BusquedaArticulosController