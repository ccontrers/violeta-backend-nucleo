# ClassTablasBdCliente

## Resumen General
`TablasBdCliente` es un wrapper mínimo especializado que hereda de `TablasBd` para inicializar y mantener en memoria la estructura (metadatos) de las tablas de la base de datos La Violeta dentro del contexto de un `ClienteVioleta`. Su único rol adicional es vincularse con el cliente y disparar la carga de metadatos al construirse.

## Objetivos
1. Asociar la instancia de `TablasBd` al cliente activo (`ClienteVioleta->Tablas`).
2. Ejecutar inmediatamente la consulta de estructuras (`EjecutaConsultaTablas`) al momento de creación, poblando definiciones de campos/índices en memoria.
3. Simplificar el punto de acceso a metadatos para otras capas (formularios, validaciones, generación dinámica de SQL).

## Dependencias
- `ClienteVioleta`: Invoca `EjecutaConsultaTablas()` y mantiene puntero inverso (`ClienteVioleta->Tablas = this`).
- `TablasBd` (clase base): Provee almacenamiento y lógica para registrar definiciones de tablas/campos (no mostrada aquí, asumida ya documentada o pendiente).

## Atributos
| Miembro | Tipo | Descripción |
|---------|------|-------------|
| `mVioletaCliente` | `ClienteVioleta*` | Referencia al cliente propietario para ejecutar la consulta de tablas y exponer esta instancia. |

(No añade miembros propios más allá del puntero al cliente; toda la estructura reside en la superclase `TablasBd`).

## Constructor
`TablasBdCliente(ClienteVioleta *Cliente)`:
1. Guarda el puntero a `Cliente` en `mVioletaCliente`.
2. Asigna `this` a `Cliente->Tablas` para que el resto del sistema acceda consistentemente.
3. Llama a `Cliente->EjecutaConsultaTablas()` que (según implementación en `ClienteVioleta`) realiza petición al servidor para obtener la descripción de todas las tablas y poblar la instancia base.

## Flujo de Uso Típico
Creación interna en el constructor de `ClienteVioleta`:
```cpp
Tablas = new TablasBdCliente(this);
// Tras esto Tablas-> (desde la clase base) ya contiene metadatos.
```
Acceso posterior:
```cpp
auto campo = gClienteVioleta->Tablas->BuscaCampo("articulos", "descripcion");
```

## Ventajas
- Centraliza la carga en un momento controlado (startup del cliente) evitando latencia dinámica a mitad de operaciones.
- Permite que validaciones de UI o generación de reportes consulten tipos/longitudes sin tocar el servidor cada vez.

## Limitaciones y Riesgos
1. Carga atómica única: No hay mecanismo de refresco si la estructura cambia en el servidor después de iniciar el cliente.
2. Dependencia circular liviana (cliente conoce tablas y tablas conocen cliente); mantener claro que `TablasBdCliente` no debe invocar otras funciones que generen nuevas dependencias cruzadas complicadas.
3. Falta de manejo de errores visible: Si `EjecutaConsultaTablas()` falla, no se muestra aquí estrategia de fallback; conviene validar `true/false` y quizá lanzar excepción si metadatos son críticos.
4. No hay indicadores de versión de esquema; podría almacenarse hash para detectar divergencias.

## Recomendaciones de Mejora
- Añadir método `Refrescar()` que vuelva a ejecutar la consulta de tablas bajo demanda.
- Guardar una marca de tiempo de última carga y versión (si la tabla de metadatos la expone).
- Integrar verificación de integridad: comparar conteo de tablas o checksum con valor esperado.
- Manejar excepción si la carga inicial regresa cero tablas (fail fast).

## Contrato Simplificado
- Precondición: Conexión al servidor válida; el usuario/terminal tiene privilegios de lectura de metadatos.
- Postcondición: `TablasBd` base poblada con definiciones disponibles para consultas locales.

## Resumen
`TablasBdCliente` actúa como adaptador del cliente hacia la estructura de BD, delegando la lógica real a la superclase y simplificando el acceso inmediato a metadatos. Ampliar capacidades de refresco y validación de consistencia fortalecería su rol en escenarios de evolución de esquema.

---
© Documentación técnica generada automáticamente.
