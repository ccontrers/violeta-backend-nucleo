# Uso de Axios en el Frontend

Este documento describe cómo se utiliza **Axios** dentro del frontend del proyecto, los patrones aplicados, manejo de errores, estructura propuesta para mejorar mantenibilidad y ejemplos prácticos.

## 1. ¿Qué es Axios y por qué se usa aquí?
Axios es una librería HTTP basada en promesas que simplifica las peticiones a APIs REST. En este proyecto se utiliza para:
- Estandarizar cabeceras, `baseURL` y `timeout`.
- Disponer de interceptores para manejo consistente de errores.
- Tipar respuestas con TypeScript (usando generics: `apiClient.post<T>()`).
- Identificar errores de red vs. errores HTTP (`axios.isAxiosError`).

## 2. Dónde se usa actualmente
| Archivo | Propósito | Base URL | Patrón |
|---------|-----------|---------|--------|
| `src/services/auth.service.ts` | Login, estado, sucursales | `/api/v1/auth` | Instancia dedicada (`axios.create`) |
| `src/services/api.service.ts` | Búsqueda de artículos y catálogos | `http://localhost:5986/api/v1/ejemplo/busqueda` | Instancia con interceptor de errores |
| `src/services/catalogo-clientes.service.ts` | Altas, modificaciones y consulta de clientes | `http://localhost:5986/api/v1/catalogos` | Instancia con interceptor de errores |
| `src/services/clientes.api.service.ts` | Búsqueda de clientes (otra ruta) | `/api/v1/busqueda` | Uso directo de `axios.post` sin instancia |

## 3. Patrones actuales
### 3.1 Creación de instancias
Se crean clientes dedicados con `axios.create({...})` para agrupar configuración.
```ts
const apiClient = axios.create({
  baseURL: '/api/v1/auth',
  timeout: 10000,
  headers: { 'Content-Type': 'application/json' }
});
```

### 3.2 Interceptores de respuesta
Se interceptan errores y se transforman en mensajes de negocio:
```ts
apiClient.interceptors.response.use(
  (response) => response,
  (error) => {
    if (error.response?.status === 404) throw new Error('Servicio no disponible');
    if (error.response?.status >= 500) throw new Error('Error interno del servidor');
    if (error.code === 'ECONNABORTED') throw new Error('La solicitud ha excedido el tiempo límite');
    throw error;
  }
);
```

### 3.3 Métodos semánticos de servicio
Cada clase expone métodos con significado de dominio (e.g. `buscarPorNombre`, `login`, `crearCliente`). Esto desacopla componentes UI de rutas HTTP exactas.

### 3.4 Manejo de errores tipado
```ts
catch (error) {
  if (axios.isAxiosError(error)) {
    if (error.response?.data) return error.response.data; // backend devolvió cuerpo
    throw new Error(`Error de conexión: ${error.message}`);
  }
  throw new Error('Error inesperado');
}
```

## 4. Inconsistencias detectadas
- Mezcla de URLs absolutas (`http://localhost:5986/...`) y relativas (`/api/v1/...`). Esto dificulta despliegues detrás de un proxy o dominios diferentes.
- Un archivo (`clientes.api.service.ts`) no usa instancia configurada (sin timeout ni interceptores comunes).
- Repetición de lógica de interceptores entre artículos y catálogo de clientes.
- No se centraliza la lectura de host/base desde variables de entorno (`import.meta.env`).

## 5. Recomendación de estructura unificada
Crear un módulo central `src/services/httpClient.ts`:
```ts
// src/services/httpClient.ts
import axios from 'axios';

const BASE_URL = import.meta.env.VITE_API_BASE_URL ?? 'http://localhost:5986';

export const http = axios.create({
  baseURL: BASE_URL,
  timeout: 10000,
  headers: { 'Content-Type': 'application/json' }
});

http.interceptors.response.use(
  r => r,
  error => {
    if (error.response?.status === 404) error.message = 'Recurso no encontrado';
    else if (error.response?.status >= 500) error.message = 'Error interno del servidor';
    else if (error.code === 'ECONNABORTED') error.message = 'Tiempo de espera agotado';
    return Promise.reject(error);
  }
);
```
Luego, cada servicio sólo añade su sub-ruta:
```ts
// auth.service.ts (refactorizado)
import { http } from './httpClient';
const auth = http; // o http.extend si se implementa wrapper

export class AuthService {
  static async login(usuario: string, password: string, sucursal = 'S1') {
    const passwordHash = await this.generateSHA256Hash(password);
    const { data } = await auth.post('/api/v1/auth/login', { usuario: usuario.toUpperCase(), passwordHash, sucursal });
    return data;
  }
}
```

## 6. Patrón de respuesta sugerido
Estandarizar en el backend y frontend el contrato:
```ts
interface ApiResult<T> {
  success: boolean;
  message?: string;
  data?: T;
  errors?: Record<string, string[]>;
}
```
Manejando en interceptor:
```ts
http.interceptors.response.use(r => r, err => {
  // Mapear a ApiResult si backend devuelve otra forma
  if (!err.response) {
    err.normalized = <ApiResult<null>>{ success: false, message: 'Sin conexión al servidor' };
  } else {
    err.normalized = err.response.data; // suponer compatibilidad
  }
  return Promise.reject(err);
});
```
Uso en servicio:
```ts
try {
  const { data } = await http.post<ApiResult<LoginResponse>>('/api/v1/auth/login', body);
  return data;
} catch (e: any) {
  return e.normalized ?? { success: false, message: e.message };
}
```

## 7. Cancelación de peticiones (opcional)
Útil para búsquedas mientras el usuario escribe:
```ts
const controller = new AbortController();
const promise = http.post('/api/v1/busqueda/clientes', payload, { signal: controller.signal });
// Si el usuario cambia input:
controller.abort();
```

## 8. Ejemplo completo refactor de búsqueda de artículos
Actual:
```ts
const response = await apiClient.post<BusquedaArticulosResponse>('/articulos', request);
return response.data;
```
Refactor propuesto:
```ts
import { http } from './httpClient';

export class ArticulosService {
  static async buscarArticulos(req: BusquedaArticulosRequest) {
    const { data } = await http.post<BusquedaArticulosResponse>('/api/v1/ejemplo/busqueda/articulos', req);
    return data;
  }
}
```

## 9. Buenas prácticas aplicables
- Definir tipos compartidos en `src/types/` para todas las respuestas.
- Mantener solo un lugar para configurar timeouts y headers.
- Usar variables de entorno (`.env`) para dominios / puertos.
- Loggear sólo en modo desarrollo (`if (import.meta.env.DEV) console.debug(...)`).
- Implementar reintentos (si se requiere) con un wrapper o librería como `axios-retry` (no añadir hasta necesitarlo).

## 10. Posibles mejoras futuras
| Mejora | Beneficio | Complejidad |
|--------|-----------|-------------|
| Unificar http client | Menos duplicación | Baja |
| Manejo de auth token (interceptor request) | Seguridad / escalabilidad | Media |
| Cache de catálogos (in-memory o SWR) | Menos latencia / menos tráfico | Media |
| Circuit breaker / fallback | Resiliencia | Alta |
| Telemetría (tiempos por endpoint) | Observabilidad | Media |

## 11. Checklist para nuevas llamadas API
1. ¿Existe ya un método semántico? Si sí, reutilizar.
2. Agregar tipo de request y response en `src/types`.
3. Usar el cliente centralizado (`http`).
4. Manejar estado de carga y errores en el componente.
5. Evitar lógica de negocio en componentes (mantenerla en servicios).

## 12. Resumen
Axios actúa como la capa de transporte unificada para comunicar el frontend con los servicios backend. Actualmente cumple su función, pero puede optimizarse centralizando configuración, normalizando respuestas y eliminando duplicaciones para facilitar mantenibilidad y escalabilidad.

---
¿Deseas que prepare el refactor automático creando `httpClient.ts` y actualizando los servicios? Puedo hacerlo en un siguiente paso.
