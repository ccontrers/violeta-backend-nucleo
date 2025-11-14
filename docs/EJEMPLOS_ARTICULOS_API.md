# Ejemplos Prácticos - API de Búsqueda de Artículos

## Configuración Inicial

### 1. Iniciar la aplicación
```bash
cd c:\Github\violetaserver
.\gradlew bootRun
```

La aplicación estará disponible en: `http://localhost:5986`

### 2. Verificar que la aplicación esté funcionando
```bash
curl -X GET http://localhost:5986/actuator/health
```

## Ejemplos de Uso

### Ejemplo 1: Búsqueda por Nombre (Más común en TPV)

**Escenario**: Cliente en punto de venta busca productos que contengan "aceite"

**Request**:
```bash
curl -X POST http://localhost:5986/api/v1/ejemplo/busqueda/articulos \
  -H "Content-Type: application/json" \
  -d '{
    "sucursal": "1",
    "mostrarExistencias": "SI", 
    "codcondicion": "N",
    "filas": "10",
    "condicion": "aceite"
  }'
```

**PowerShell**:
```powershell
$body = @{
    sucursal = "1"
    mostrarExistencias = "SI"
    codcondicion = "N" 
    filas = "10"
    condicion = "aceite"
} | ConvertTo-Json

Invoke-RestMethod -Uri "http://localhost:5986/api/v1/ejemplo/busqueda/articulos" -Method POST -Body $body -ContentType "application/json"
```

**Response esperado**:
```json
{
    "success": true,
    "message": "Búsqueda completada exitosamente",
    "totalResultados": 4,
    "articulos": [
        {
            "nombre": "Aceite Capullo 1L",
            "presentacion": "1 LT",
            "existencia": 150,
            "precio": 55.90,
            "marca": "CAPULLO",
            "nombreMarca": "Capullo",
            "producto": "ACE001",
            "articulo": "ACE001", 
            "codigoBarras": "7501234567890",
            "sucursal": 1
        },
        {
            "nombre": "Aceite Capullo 500ml",
            "presentacion": "500 ML",
            "existencia": 200,
            "precio": 31.50,
            "marca": "CAPULLO",
            "nombreMarca": "Capullo",
            "producto": "ACE002",
            "articulo": "ACE002",
            "codigoBarras": "7501234567891", 
            "sucursal": 1
        }
    ],
    "clasificaciones": [],
    "marcas": []
}
```

### Ejemplo 2: Búsqueda por Código de Barras

**Escenario**: Escaneo de código de barras en TPV

```bash
curl -X POST http://localhost:5986/api/v1/ejemplo/busqueda/articulos \
  -H "Content-Type: application/json" \
  -d '{
    "sucursal": "1",
    "mostrarExistencias": "SI",
    "codcondicion": "CB", 
    "filas": "1",
    "condicion": "7501234567890"
  }'
```

### Ejemplo 3: Obtener Catálogos (Clasificaciones y Marcas)

**Escenario**: Cargar filtros para interfaz de búsqueda

```bash
curl -X POST http://localhost:5986/api/v1/ejemplo/busqueda/articulos \
  -H "Content-Type: application/json" \
  -d '{
    "sucursal": "1",
    "mostrarExistencias": "NO",
    "codcondicion": "",
    "filas": "0", 
    "condicion": ""
  }'
```

**Response esperado**:
```json
{
    "success": true,
    "message": "Búsqueda completada exitosamente",
    "totalResultados": 8,
    "articulos": [],
    "clasificaciones": [
        {"codigo": "ALIMENTOS", "nombre": "Alimentos y Bebidas"},
        {"codigo": "LIMPIEZA", "nombre": "Productos de Limpieza"},
        {"codigo": "LACTEOS", "nombre": "Lácteos"},
        {"codigo": "ACEITES", "nombre": "Aceites y Grasas"}
    ],
    "marcas": [
        {"codigo": "CAPULLO", "nombre": "Capullo"},
        {"codigo": "PATRONA", "nombre": "La Patrona"},
        {"codigo": "CRISTAL", "nombre": "Cristal"},
        {"codigo": "NATURA", "nombre": "Natura"}
    ]
}
```

### Ejemplo 4: Búsqueda por Marca

**Escenario**: Ver todos los productos de una marca específica

```bash
curl -X POST http://localhost:5986/api/v1/ejemplo/busqueda/articulos \
  -H "Content-Type: application/json" \
  -d '{
    "sucursal": "1",
    "mostrarExistencias": "SI",
    "codcondicion": "M",
    "filas": "20",
    "condicion": "CAPULLO"
  }'
```

### Ejemplo 5: Búsqueda por Clasificación

**Escenario**: Ver todos los productos de una clasificación

```bash
curl -X POST http://localhost:5986/api/v1/ejemplo/busqueda/articulos \
  -H "Content-Type: application/json" \
  -d '{
    "sucursal": "1", 
    "mostrarExistencias": "NO",
    "codcondicion": "E",
    "filas": "15",
    "condicion": "ACEITES"
  }'
```

### Ejemplo 6: Búsqueda por Código de Artículo

**Escenario**: Búsqueda directa por código interno

```bash
curl -X POST http://localhost:5986/api/v1/ejemplo/busqueda/articulos \
  -H "Content-Type: application/json" \
  -d '{
    "sucursal": "1",
    "mostrarExistencias": "SI", 
    "codcondicion": "C",
    "filas": "5",
    "condicion": "ACE001"
  }'
```

## Casos de Error

### Error 1: Tipo de búsqueda inválido

**Request con error**:
```bash
curl -X POST http://localhost:5986/api/v1/ejemplo/busqueda/articulos \
  -H "Content-Type: application/json" \
  -d '{
    "sucursal": "1",
    "mostrarExistencias": "NO",
    "codcondicion": "INVALID",
    "filas": "10", 
    "condicion": "test"
  }'
```

**Response de error**:
```json
{
    "success": false,
    "message": "Tipo de búsqueda no válido: INVALID",
    "totalResultados": 0,
    "articulos": [],
    "clasificaciones": [],
    "marcas": []
}
```

### Error 2: Problema de base de datos

Si las tablas no existen, verás un error 500 con mensaje descriptivo.

## Integración con Frontend

### JavaScript/jQuery
```javascript
async function buscarArticulos(termino) {
    const request = {
        sucursal: "1",
        mostrarExistencias: "SI",
        codcondicion: "N", 
        filas: "10",
        condicion: termino
    };
    
    try {
        const response = await fetch('/api/v1/ejemplo/busqueda/articulos', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify(request)
        });
        
        const data = await response.json();
        
        if (data.success) {
            mostrarResultados(data.articulos);
        } else {
            mostrarError(data.message);
        }
    } catch (error) {
        console.error('Error en búsqueda:', error);
    }
}
```

### React Hook
```javascript
import { useState } from 'react';

const useBusquedaArticulos = () => {
    const [loading, setLoading] = useState(false);
    const [results, setResults] = useState([]);
    const [error, setError] = useState(null);
    
    const buscar = async (params) => {
        setLoading(true);
        setError(null);
        
        try {
            const response = await fetch('/api/v1/ejemplo/busqueda/articulos', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(params)
            });
            
            const data = await response.json();
            
            if (data.success) {
                setResults(data.articulos);
            } else {
                setError(data.message);
            }
        } catch (err) {
            setError('Error de conexión');
        } finally {
            setLoading(false);
        }
    };
    
    return { buscar, loading, results, error };
};
```

## Performance y Optimización

### Tips para mejor rendimiento:

1. **Usar límites apropiados**:
   ```json
   {
     "filas": "10"  // No más de 50 en TPV
   }
   ```

2. **Términos de búsqueda específicos**:
   ```json
   {
     "condicion": "aceite capullo"  // Más específico que solo "a"
   }
   ```

3. **Usar códigos cuando sea posible**:
   ```json
   {
     "codcondicion": "CB",  // Más rápido que búsqueda por nombre
     "condicion": "7501234567890"
   }
   ```

## Troubleshooting

### Problema: "Connection refused"
**Solución**: Verificar que MariaDB esté ejecutándose en puerto 3308

### Problema: Response vacío
**Solución**: Verificar que existan datos en las tablas, especialmente `articulos` y `existencias`

### Problema: Caracteres especiales
**Solución**: Asegurar encoding UTF-8 en request headers
