# Especificación Técnica Legada: Módulo de Búsqueda de Proveedores

Este documento describe con precisión el comportamiento actual del módulo de búsqueda de proveedores en el sistema legado (cliente C++ Builder / VCL + servidor propietario). No incluye propuestas de mejora ni diseño futuro; su objetivo es documentar el punto de partida existente.

---
## 1. Alcance
- Componente: Ventana modal "Búsqueda de proveedores".
- Tipo: Consulta y selección (no modifica datos).
- Participantes: Usuario final, Cliente VCL (front desktop), Servidor de búsquedas, Base de datos MySQL.
- Fuentes de datos: Tabla principal `proveedores`.

---
## 2. Arquitectura Actual (Resumen Operativo)
| Capa | Elemento | Descripción |
|------|----------|------------|
| UI (desktop) | `TFormBusqProveedores` (FormBusquedaProveedores.*) | Formulario modal con pestañas y grid de resultados. |
| Lógica cliente | Método `BuscaProveedor(TipoBusqueda)` | Construye buffer de parámetros y envía solicitud al servidor. |
| Servidor | `ServidorBusquedas::BuscaProveedores` | Decodifica parámetros, arma instrucción SQL, ejecuta y empaqueta resultados. |
| Base de datos | Tabla `proveedores` | Fuente de los campos mostrados. |

No intervienen joins adicionales para esta búsqueda (solo tabla `proveedores`).

---
## 3. Interfaz de Usuario (Layout Actual)
| Elemento | Tipo | Identificador (código) | Función |
|----------|------|------------------------|---------|
| Pestañas | `TPageControl` | `tpgBusqueda` | Agrupa criterios mutuamente excluyentes. |
| Tab 1 | `TTabSheet` | `TabSheetRazon` | Búsqueda por razón social. |
| Tab 2 | `TTabSheet` | `TabSheetRfc` | Búsqueda por RFC. |
| Tab 3 | `TTabSheet` | `TabSheetReplegal` | Búsqueda por representante legal. |
| Tab 4 | `TTabSheet` | `TabSheetClave` | Búsqueda por clave interna. |
| Campo texto | `VTLabeledEdit` | `EditRazon`, `EditRfc`, `EditReplegal`, `EditClave` | Captura del criterio principal. |
| Botón buscar | `VTBitBtn` | `ButtonBuscarRazon`, etc. | Dispara la búsqueda según la pestaña activa. |
| Filtros tipo proveedor | `VTCheckBox` (duplicados por tab) | `CheckBoxProvGast*`, `CheckBoxProvMerc*` | Restringen por tipo (gastos/mercancía). |
| Filtro activos | `VTCheckBox` | `CheckBoxMostrarInactivos` | Incluye registros inactivos cuando está marcado. |
| Grid resultados | `VTStringGrid` | `StringGridResultados` | Presenta lista de coincidencias. |
| Acciones | `VTBitBtn` | `ButtonSeleccionar`, `ButtonCancelar` | Devuelve selección / cierra modal. |

El grid se inicializa con 11 columnas visibles (ver sección 7). El título literal: `"Búsqueda de proveedores"`.

---
## 4. Modos de Búsqueda
| Código interno | Significado | Columna evaluada | Ordenación aplicada |
|----------------|------------|------------------|---------------------|
| `RSO` | Razón social | `razonsocial` | `ORDER BY razonsocial` |
| `RFC` | RFC | `rfc` | `ORDER BY rfc, razonsocial` |
| `CLA` | Clave proveedor | `proveedor` | `ORDER BY rfc, razonsocial` |
| `REP` | Representante legal | `replegal` | `ORDER BY razonsocial` |
| vacío | Sin criterio | No genera consulta (no se envía nada) |

La coincidencia es siempre con patrón `%valor%` (comodín en ambos extremos) aplicado a la columna definida por el modo.

---
## 5. Protocolo de Parámetros (Cliente → Servidor)
El cliente construye un buffer lineal (secuencia de segmentos) y el servidor los extrae en orden mediante `mFg.ExtraeStringDeBuffer`.

Orden y significado:
1. `tipo_busqueda` (RSO|RFC|CLA|REP|"")
2. `solo_activos` ("1" = aplicar filtro de activos; "0" = no forzar activos)  
3. `dato_buscado` (solo si `tipo_busqueda` ≠ "") → el servidor lo envuelve en `%` (prefijo y sufijo) 
4. `solo_ProvGastos` ("1" = restringir a `provgastos=1`)  
5. `solo_ProvMercancia` ("1" = restringir a `provmercancia=1`)  

Si `tipo_busqueda` viene vacío, el servidor no ejecuta consulta (bloque condicional final vacío).

### 5.1 Reglas Internas de Flags
| Parámetro | Interpretación en servidor | Condición agregada |
|-----------|----------------------------|--------------------|
| `solo_activos == "1"` | Forzar activos | `AND activo=1` |
| `solo_ProvGastos == "1"` | Filtrar por gastos | `AND provgastos=1` |
| `solo_ProvMercancia == "1"` | Filtrar por mercancía | `AND provmercancia=1` |

Los flags no son mutuamente excluyentes a nivel servidor; la exclusión se implementa en interfaz (checkboxes desmarcan el otro). Si ambos fueran "1" el WHERE contendría ambas condiciones (posible intersección). En la práctica UI evita ese escenario.

---
## 6. Lógica de Construcción SQL (Servidor)
Fragmento relevante (simplificado):
```cpp
if (tipo_busqueda == "RSO") {
  instruccion.sprintf(
    "select proveedor, razonsocial, replegal, rfc, estado, localidad, calle, colonia, redondeocptecho, "
    " if(provgastos=1, 'Si', 'No') as provgastos, if(provmercancia=1, 'Si', 'No') as provmercancia"
    " from proveedores where razonsocial like '%s' %s %s %s order by razonsocial limit %s",
    dato_buscado, condicion_solo_activos, condicion_solo_ProvGastos, condicion_solo_ProvMercancia, NUM_LIMITE_RESULTADOS_BUSQ);
}
```
Variaciones para RFC / CLA / REP reemplazan la columna del `LIKE` y el `ORDER BY`.

### 6.1 Patrón General
```
SELECT proveedor, razonsocial, replegal, rfc, estado, localidad, calle, colonia,
       redondeocptecho,
       IF(provgastos=1,'Si','No') AS provgastos,
       IF(provmercancia=1,'Si','No') AS provmercancia
FROM proveedores
WHERE <columna_modo> LIKE '%<dato_buscado>%'
  [AND activo=1]
  [AND provgastos=1]
  [AND provmercancia=1]
ORDER BY <orden_modo>
LIMIT <NUM_LIMITE_RESULTADOS_BUSQ>
```
`NUM_LIMITE_RESULTADOS_BUSQ` es una constante definida en el servidor (valor no mostrado en el fragmento incluido; limitado para evitar cargas grandes).

### 6.2 Consideraciones Observadas
- Parámetros se interpolan por concatenación (no usa prepared statements).
- Búsqueda con comodines en ambos extremos (%valor%) → no usa índices BTREE eficientemente.

---
## 7. Columnas Mostradas en la UI
Orden declarado en inicialización de grid (mapping directo al SELECT):
1. `proveedor`
2. `razonsocial`
3. `replegal`
4. `rfc`
5. `estado`
6. `localidad`
7. `calle`
8. `colonia`
9. `redondeocptecho`
10. `provgastos` (transformado a 'Si'/'No')
11. `provmercancia` (transformado a 'Si'/'No')

No se muestra `activo`; el filtro actúa antes de poblar la cuadrícula.

---
## 8. Tabla `proveedores` (Campos Relevantes para el Módulo)
| Columna | Tipo | Uso en módulo | Observaciones |
|---------|------|---------------|---------------|
| proveedor | varchar(11) PK | Clave mostrada y devuelta | Primary Key |
| razonsocial | varchar(60) | Criterio y resultado | Indexada |
| replegal | varchar(60) | Criterio y resultado | Puede ser NULL |
| rfc | varchar(15) | Criterio y resultado | Indexada |
| estado | varchar(4) | Resultado | Código abreviado (FK a `estados`) |
| localidad | varchar(40) | Resultado | Texto libre |
| calle | varchar(60) | Resultado | Texto libre |
| colonia | varchar(40) | Resultado | Texto libre (no normalizado en la consulta) |
| redondeocptecho | tinyint(1) | Resultado | Indicador (1 ó 0) |
| provgastos | tinyint(4) | Filtro / Resultado | 1 = sí |
| provmercancia | tinyint(4) | Filtro / Resultado | 1 = sí |
| activo | tinyint(1) | Filtro (no visible) | 1 = activo |

Claves/Índices usados indirectamente: `KEY razonsocial`, `KEY rfc`, `KEY replegal`, `PRIMARY KEY (proveedor)`.

---
## 9. Flujo de Ejecución (Secuencia)
1. Usuario abre formulario.
2. Selecciona pestaña (modo). Pestaña por defecto: "Por razón social".
3. Ingresa texto en campo principal.
4. (Opcional) Activa uno de los filtros tipo proveedor. UI desmarca el otro si se marca uno.
5. (Opcional) Marca "Mostrar inactivos" (en realidad quita la restricción de `activo=1`).
6. Pulsa botón "Buscar" asociado a la pestaña activa.
7. Cliente invoca `BuscaProveedor(modo)` → prepara buffer de parámetros.
8. Cliente envía petición al servidor (ID interno `ID_BUSQ_PROVEEDOR`).
9. Servidor parsea parámetros en orden; construye y ejecuta SQL.
10. Servidor empaqueta filas en buffer de respuesta.
11. Cliente recibe buffer y rellena `VTStringGrid` fila a fila.
12. Usuario selecciona fila (click) y pulsa "Seleccionar" → devuelve clave al llamante. O pulsa "Cancelar" → descarta.

---
## 10. Validaciones y Comportamiento de Entrada
| Situación | Comportamiento Actual |
|-----------|-----------------------|
| Campo criterio vacío | No envía consulta (se muestra mensaje de requerir texto). |
| Selección de ambos tipos (gastos/mercancía) | UI evita simultaneidad (clic en uno desmarca el otro). |
| Modo vacío | Servidor no ejecuta consulta. |
| Número de resultados excede límite | Resultados se cortan al límite; no hay indicación visual. |

---
## 11. Representación de Resultados (Transformaciones)
| Campo origen | Transformación en SELECT | Valor mostrado |
|--------------|--------------------------|----------------|
| provgastos | `IF(provgastos=1,'Si','No')` | "Si" / "No" |
| provmercancia | `IF(provmercancia=1,'Si','No')` | "Si" / "No" |
| redondeocptecho | Directo (0/1) | 0 / 1 (sin etiqueta) |

No hay formateo adicional (no se convierten estados a nombres completos, no se capitaliza, etc.).

---
## 12. Estructura de Salida (Orden de Campos en Buffer)
El servidor retorna columnas en el orden del SELECT. El cliente asume posiciones fijas al llenar la grilla. No se encontraron alias adicionales excepto los aplicados a `provgastos` y `provmercancia`.

Orden definitivo dentro del buffer por fila:
1. proveedor
2. razonsocial
3. replegal
4. rfc
5. estado
6. localidad
7. calle
8. colonia
9. redondeocptecho
10. provgastos (Si/No)
11. provmercancia (Si/No)

---
## 13. Límites y Constantes Observadas
| Constante | Uso | Valor (si no visible queda "no expuesto" en fragmento) |
|-----------|-----|----------------------------------|
| `NUM_LIMITE_RESULTADOS_BUSQ` | LIMIT en SQL | No expuesto en fragmento analizado |
| `TAM_MAX_BUFFER_RESPUESTA_BUSQ` | Tamaño máximo buffer respuesta | No expuesto en fragmento |

---
## 14. Manejo de Errores (Actual)
- No se registra manejo granular en fragmento. Si la consulta falla, el servidor no arma respuesta útil (comportamiento implícito no detallado).
- Validación principal se realiza en cliente antes de enviar.

---
## 15. Comportamiento de los Filtros Tipo Proveedor en Cliente
El código UI sincroniza checkboxes: evento `OnClick` de cada checkbox de tipo invoca lógica que desmarca el otro y fuerza un nuevo estado interno antes de la búsqueda. Esto garantiza que nunca se envían ambos flags como "1" en condiciones normales de uso.

---
## 16. Consideraciones de Desempeño Existentes
| Aspecto | Observación |
|---------|------------|
| Índices | LIKE con `%` al inicio y fin impide aprovechamiento de índices simples. |
| Volumen | Limitado artificialmente por `LIMIT` constante. |
| Red de envío | Formato propietario de buffer (no JSON). |
| Selección de columnas | Solo campos necesarios (eficiente en ancho). |

---
## 17. Dependencias (Resumen)
| Elemento | Función |
|----------|---------|
| `FormBusquedaProveedores.cpp/.h/.dfm` | Interfaz y lógica cliente. |
| `ClassServidorBusquedas.cpp` | Implementación de `BuscaProveedores`. |
| Tabla `proveedores` | Datos base. |
| Funciones utilitarias (`mFg.ExtraeStringDeBuffer`) | Decodificación de parámetros. |

---
## 18. Pseudocódigo del Flujo (Servidor)
```pseudo
function BuscaProveedores(paramsBuffer):
  tipo = nextToken(paramsBuffer)
  soloActivos = nextToken(paramsBuffer)
  initResponseBuffer()
  if tipo != "":
    dato = "%" + nextToken(paramsBuffer) + "%"
  provGastosFlag = nextToken(paramsBuffer)
  provMercFlag = nextToken(paramsBuffer)

  condiciones = []
  if soloActivos == "1": condiciones += ["activo=1"]
  if provGastosFlag == "1": condiciones += ["provgastos=1"]
  if provMercFlag == "1": condiciones += ["provmercancia=1"]

  switch(tipo):
    case "RSO": whereCampo = "razonsocial"
    case "RFC": whereCampo = "rfc"
    case "CLA": whereCampo = "proveedor"
    case "REP": whereCampo = "replegal"
    case "": return (sin envío)

  sql = "SELECT proveedor, razonsocial, replegal, rfc, estado, localidad, calle, colonia, redondeocptecho, " +
        "IF(provgastos=1,'Si','No') AS provgastos, IF(provmercancia=1,'Si','No') AS provmercancia " +
        "FROM proveedores WHERE " + whereCampo + " LIKE '" + dato + "'"
  for c in condiciones: sql += " AND " + c
  sql += ordenPor(tipo) + " LIMIT " + LIMITE
  ejecutar(sql)
  serializarFilasEnBuffer()
```

`ordenPor` retorna la cláusula ORDER BY según tabla de la sección 4.

---
## 19. Estado de Selección en Cliente
- Al hacer clic en una fila del grid, se registra el índice actual.
- Botón "Seleccionar" lee esa fila y devuelve la clave a la ventana invocadora.
- Si se cierra o cancela, no se transmite valor.

---
## 20. Limitaciones Funcionales Observadas (Descripción)
| Área | Descripción factual |
|------|---------------------|
| Paginación | No existe; solo se muestra hasta el límite fijado. |
| Orden dinámico | No hay reordenamiento interactivo. |
| Búsqueda vacía | Se impide ejecutar (mensaje previo). |
| Visibilidad de estado | Campo `activo` no se muestra. |

---
## 21. Resumen del Comportamiento Actual
El módulo realiza búsquedas sobre la tabla `proveedores` aplicando un único criterio textual por modo, con filtros opcionales para actividad y tipo (gastos / mercancía). Los resultados se devuelven en un buffer con un conjunto fijo de 11 columnas que se muestran en un grid sin capacidades de orden ni paginación en el cliente. El servidor compone consultas SQL mediante concatenación y devuelve cadenas literal para indicadores booleanos específicos.

---
Documento cerrado a la situación actual: no incluye rediseños ni mejoras propuestas.
