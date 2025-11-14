# ClassEnlaceInterfaz

## 1. Resumen
`EnlaceInterfaz` actúa como puente entre la capa de datos accesible vía `ClienteVioleta` (consultas SQL tipo SELECT) y controles visuales VCL (listas, combos, grids, edits). Ejecuta una consulta, construye un `BufferRespuestas` con el resultado y delega la proyección a métodos de llenado específicos del buffer (`LlenaListBox`, `LlenaCheckListBox`, etc.). Ofrece también una variante para poblar un `VTComboBox` solo si una clave buscada no está cargada.

## 2. Objetivo
Reducir repetición de código en formularios que requieren:
- Ejecutar SELECT.
- Convertir resultado textual en estructura interpretable.
- Trasladar filas/columnas a componentes visuales.

## 3. Alcance / No Alcance
Alcance:
- Capa de conveniencia para poblar controles desde SQL SELECT simples.
- Gestión mínima de memoria del buffer crudo (`char*`).

No Alcance:
- Manejo de errores enriquecido (solo booleano de retorno, no expone mensaje ni código; la documentación menciona mError* que aquí no existen).
- Paginación, filtrado incremental, caching.
- Actualización bidireccional (solo lectura hacia UI).

## 4. Dependencias
- `ClienteVioleta`: método `EjecutaSqlSelect(select, char*&)` que retorna buffer plano.
- `BufferRespuestas`: parseo y métodos de llenado: `LlenaListBox`, `LlenaCheckListBox`, `LlenaEdit`, `LlenaStringGrid`, `LlenaComboBox`.
- VCL: `TListBox`, `TCheckListBox`, `TEdit`, `TStringGrid`, `TComboBox`.
- Control personalizado: `VTComboBox` (propiedad `Clave`).

## 5. Métodos
### 5.1 Llenado Directo
Cada método sigue el mismo patrón:
1. Declara `char *resultado_select = NULL`.
2. Llama `mClienteVioleta->EjecutaSqlSelect(select, resultado_select)` ? bool éxito.
3. Construye `BufferRespuestas respuestas(resultado_select);`.
4. Invoca método de llenado.
5. Libera `resultado_select` con `delete` (asume asignación con `new char[]`).
6. Devuelve bool de la ejecución del SELECT.

Métodos:
- `LlenaListBox(select, TListBox*)`
- `LlenaCheckListBox(select, TCheckListBox*)`
- `LlenaEdit(select, TEdit*)` (llenado fila 0, campo 0)
- `LlenaStringGrid(select, TStringGrid*, bool nomCampos, bool numRegs)`
- `LlenaComboBox(select, TComboBox*)`

### 5.2 Llenado Condicional
- `LlenaVTComboBoxSiEsNecesario(select, VTComboBox*, Clave)`:
  - Si `Clave` no vacía: asigna `ComboBox->Clave`.
  - Si tras asignación sigue vacía, ejecuta llenado y reintenta seleccionar la clave.
  - Retorna true salvo que el SELECT falle.

## 6. Flujo de Uso Típico
```cpp
EnlaceInterfaz enlace(&gClienteVioleta);
enlace.LlenaComboBox("select id,nombre from proveedores order by nombre", cboProveedores);
// Forzar selección específica llenando sólo si necesario
enlace.LlenaVTComboBoxSiEsNecesario("select id,nombre from monedas", cboMoneda, "USD");
```

## 7. Riesgos y Observaciones
| Riesgo | Descripción | Impacto | Mitigación |
|--------|-------------|---------|-----------|
| Gestión de memoria manual | Uso de `delete resultado_select` (no `delete[]`) sin confirmación de asignación | Fuga o undefined behavior si no coincide método de asignación | Asegurar contrato (usar `std::unique_ptr<char[]>`) |
| Sin propagación de error detallado | Solo bool; comentarios sugieren mErrorNo/mErrorLinea/mErrorMsg inexistentes | Difícil diagnóstico en UI | Extender API para exponer diagnóstico | 
| Repetición de patrón | Código idéntico en 5 métodos | Mantenimiento repetitivo | Extraer función auxiliar genérica templada | 
| Ausencia de validación de puntero | No se verifica que control sea no nulo | Crash potencial | `if (!ListBox) return false;` |
| Dependencia fuerte de formato de buffer | Acoplamiento a `BufferRespuestas` | Cambios en formato rompen API | Definir interfaz estable de parseo | 

## 8. Mejoras Propuestas
1. Función interna templada: `template<typename F> bool EjecutaYLlena(const AnsiString& sql, F llenador)` para eliminar duplicación.
2. Cambiar retorno a estructura resultado: `{ bool ok; int errorNo; AnsiString mensaje; }`.
3. Uso de RAII para buffer: wrapper que en destructor haga `delete[]` correcto.
4. Sobre carga que acepte parámetros preparados (evitar concatenación directa en capa superior, reduce riesgo SQL injection si adoptan prepared statements en backend).
5. Instrumentar logging (SQL ejecutada + tiempo ejecución). 
6. Validar tamaño de resultset antes de construir controles (optimizar UI bajo grandes volúmenes).

## 9. Contrato Propuesto
- Pre: `mClienteVioleta` ha sido inicializado y autenticado.
- Post (éxito): Control destino queda con datos representando el resultset completo.
- Post (falla): Control queda en estado indefinido (no se limpia); se sugiere limpieza explícita previa.

## 10. Ejemplo de Refactor (Borrador Conceptual)
```cpp
bool EnlaceInterfaz::LlenaGenerico(const AnsiString& sql, std::function<void(BufferRespuestas&)> fn) {
    std::unique_ptr<char[]> raw;
    if (!mClienteVioleta->EjecutaSqlSelect(sql, raw)) return false;
    BufferRespuestas buf(raw.get());
    fn(buf);
    return true;
}
```

## 11. Interacción con Otros Módulos
- Usa `ClienteVioleta` solo para SELECT; complementa a `ControladorInterfaz` (que trata edición) al facilitar poblar combos/listas (catálogos) y grids (consultas).
- `BufferRespuestas` centraliza formato de respuesta; cualquier extensión (paginación) debe iniciarse allí y reflejarse aquí.

## 12. Limitaciones Actuales
- No hay cancelación / asincronía (bloquea UI hasta terminar).
- No considera internacionalización ni formato numérico (delegado a capa inferior).
- Sin caching: repetidos SELECT incurren en costo completo.

## 13. Resumen Ejecutivo
`EnlaceInterfaz` es un adaptador ligero orientado a rellenar controles VCL desde consultas SQL, apoyándose en `BufferRespuestas`. Funciona pero es mínimo y repetitivo. Su principal evolución debería dirigirse a: eliminar duplicación, mejorar robustez de memoria/errores y preparar el terreno para ejecuciones asíncronas o paginadas.

---
Última revisión: 2025-09-22
