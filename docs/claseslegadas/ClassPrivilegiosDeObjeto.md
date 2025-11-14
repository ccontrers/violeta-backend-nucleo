# ClassPrivilegiosDeObjeto

## Resumen General
`PrivilegiosDeObjeto` administra los privilegios (permisos finos) que un usuario posee sobre un ?objeto? funcional del sistema (módulo, catálogo, proceso). Carga desde el servidor las asignaciones y expone indicadores booleanos agregados (consultar, modificar, altas, bajas, editar) así como operaciones de consulta individual. También valida condiciones de restricción global (parámetro `RESCATART`) y aplica lógica para prohibir altas/modificaciones bajo ciertas combinaciones de bandera externa (`ProhibirAltMod`) y restricción de parámetros.

## Objetivos
1. Centralizar la verificación de permisos por objeto y usuario.
2. Reducir viajes a servidor para privilegios ya consultados (cache en listas internas).
3. Incorporar políticas dinámicas (restricción por parámetro, control de versión mínima) en un punto único.
4. Exponer flags agregados amigables para la interfaz (permitidoConsultar, permitidoEditar, etc.).

## Dependencias
- `ClienteVioleta` / global `gClienteVioleta`: Envío de peticiones (IDs `ID_OBT_ASIGPRIVUSUAROL`, `ID_OBT_ASIGPRIVUSUAROL_ESP`).
- `BufferRespuestas`: Parseo de secuencias devueltas (privilegios, versión, subversión, tiempo límite).
- `FuncionesGenericas`: Mensajes (`AppMessageBox`), formateo.
- `VersionSiiv.h`: Variables globales de versión programa (`gVersionAplicacion`, `gSubversionAplicacion`).
- `FormClienteVioleta.h`: Interacción con temporizador de validación de versión vía cliente (inicio/detención y cierre de aplicación).

## Estructura Interna
| Miembro | Tipo | Descripción |
|---------|------|-------------|
| `mPrivilegios` | `TStringList*` | Claves de privilegio (ej. `CON`, `MOD`, `ALT`, `BAJ`). |
| `mDescripciones` | `TStringList*` | Descripciones legibles paralelas a las claves. |
| `mUsuario` | `AnsiString` | Usuario al que pertenecen los privilegios (sólo para referencia). |
| `mObjeto` | `AnsiString` | Objeto actual consultado. |
| `mEsRestringido` | `bool` | Resultado cacheado de parámetro `RESCATART`. |
| `mNumPrivilegios` | `int` | Conteo de privilegios cargados. |
| `mPermitidoConsultar` | `bool` | Flag agregado para consulta (`CON`). |
| `mPermitidoModificar` | `bool` | Flag agregado para modificación (`MOD`). |
| `mPermitidoAltas` | `bool` | Altas (`ALT`). |
| `mPermitidoBajas` | `bool` | Bajas (`BAJ`). |
| `mPermitidoEditar` | `bool` | (Consultar OR modificar) según lógica actual. |
| `mVersionAplicacion` / `mSubversionAplicacion` | `AnsiString` | Valores locales de versión del cliente. |

## Flujo de Consulta de Privilegios
1. Llamada a `ConsultaPrivilegios(usuario, objeto, ProhibirAltMod)`.
2. Se evalúa `mEsRestringido = esRestringido()` (SELECT a `parametrosemp` para `RESCATART`).
3. Se arma petición `ID_OBT_ASIGPRIVUSUAROL` con usuario y objeto.
4. La respuesta concatena múltiples bloques: (a) privilegios, (b) subversión mínima DB, (c) versión mínima DB, (d) tiempo límite de actualización.
5. Se calculan versiones numéricas (programa vs base) y, si el programa está desactualizado, se evalúa el tiempo restante para forzar cierre.
6. Se limpian listas y se rellenan con privilegios: se activan flags según aparezcan (`CON`, `MOD`, `ALT`, `BAJ`).
7. Si `ProhibirAltMod==true` y `mEsRestringido==true` se fuerzan a falso los flags de modificación/altas/bajas/editar.

## Lógica de Restricción
- Parámtero `RESCATART = '1'` ? `mEsRestringido=true`.
- Con `mEsRestringido` y `ProhibirAltMod` ambos verdaderos se inhiben `MOD/ALT/BAJ` y `mPermitidoEditar`.
- Si `RESCATART` es falso o la bandera de prohibición no se envía, los privilegios retornan a comportamiento normal.

## Control de Versiones
Durante `ConsultaPrivilegios` se validan versiones:
- Si versión del programa (`gVersionAplicacion` + subversión /1000) < versión mínima de BD, se advierte y se inicia temporizador.
- Al agotarse el tiempo límite (`TiempoValidacionLimite`) se cierra la aplicación (`Application->Terminate()`).
Esto introduce acoplamiento entre control de versión y flujo de privilegios (podría separarse).

## Principales Métodos
| Método | Descripción |
|--------|-------------|
| `ConsultaPrivilegios(usuario, objeto, ProhibirAltMod=false)` | Carga privilegios y aplica restricciones; devuelve true si encontró al menos uno. |
| `TieneAlgunPrivilegio()` | Retorna si hay al menos un privilegio cargado. |
| `ObtieneNumPrivilegios()` | Devuelve conteo actual. |
| `TienePrivilegio(clave)` | Búsqueda exacta (case sensitive) en lista. |
| `ObtieneNombrePrivilegio(clave)` | Retorna descripción asociada o lanza excepción si no existe. |
| `TienePrivilegioEspecifico(objeto, privilegio)` | Petición puntual (ID distinto) sin alterar cache principal. |
| `ObtieneClaveObjeto()` | Devuelve `mObjeto` actual. |
| `esRestringido()` (privado) | Consultar parámetro `RESCATART`. |

## Ejemplo de Uso
```cpp
PrivilegiosDeObjeto priv;
if (priv.ConsultaPrivilegios(usuario, "CAT_ARTICULOS", true)) {
    if (!priv.mPermitidoAltas) {
        MostrarAviso("No puede dar de alta en este catálogo");
    }
    if (priv.TienePrivilegio("CON")) {
        CargarListado();
    }
}
```

## Riesgos y Observaciones
1. Acoplamiento excesivo: versión del sistema y control de cierre se manejan aquí (violando SRP). 
2. Dependencia global `gClienteVioleta` dificulta test unitarios aislados.
3. Lógica de parseo de respuesta asume orden rígido de bloques (fragilidad si cambia contrato del servidor).
4. `TienePrivilegioEspecifico` genera nueva llamada cada vez (falta cache selectiva; riesgo de latencia si se abusa en la UI).
5. Excepciones genéricas en obtención de nombre de privilegio; podría retornar opcional para no romper flujo de UI.
6. Sin sincronización para acceso concurrente (en la práctica usualmente se usa en hilo de UI). 
7. Posible mezcla de concepto: `mPermitidoEditar` se define como `(Consultar OR Modificar)` pero se anula en un bloque donde `Consultar` puede seguir true; semántica confusa.
8. Mensajes UI desde clase de dominio (mezcla lógica + presentación).

## Recomendaciones de Mejora
- Extraer control de versiones a un servicio separado (e.g. `ValidadorVersionSistema`).
- Introducir struct `PrivilegiosAggregados` con flags, separando datos de lógica de adquisición.
- Cachear resultado de `TienePrivilegioEspecifico` con clave (usuario,objeto,privilegio) con TTL.
- Centralizar construcción/desserialización de paquetes servidor en una capa de transporte común.
- Reemplazar `TStringList` por contenedor hash para búsquedas O(1) y claridad semántica.
- Evitar interacción directa con UI (inyectar callback de notificación o retornar códigos).

## Contrato Simplificado
- Precondición: Conexión de cliente operativa; IDs de solicitud válidos.
- Postcondición (éxito): Listas internas contienen privilegios y flags reflejan capacidades finales tras restricciones.
- Postcondición (falla): Excepción en consultas SQL o privilegio inexistente al pedir nombre; flags permanecen en valores por defecto (false).

## Resumen
`PrivilegiosDeObjeto` concentra obtención y evaluación de permisos de usuario por objeto y añade políticas de restricción y validación de versión. Funciona, pero mezcla múltiples responsabilidades (permisos, actualización forzada, UI) que conviene segmentar para mayor mantenibilidad y testabilidad.

---
© Documentación técnica generada automáticamente.
