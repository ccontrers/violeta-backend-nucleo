# ClassControladorInterfaz

## 1. Resumen
`ControladorInterfaz` centraliza la gestión de múltiples controles visuales heterogéneos (derivados de una familia de componentes prefijados con `VT*`) permitiendo:
- Registro declarativo de controles y su mapeo a campos de datos (servidor / tabla).
- Agrupamiento lógico (hasta 3 grupos por control) para operaciones masivas (habilitar, ocultar, validar, etc.).
- Validación uniforme (longitud mínima, email, teléfono, máscaras específicas).
- Seguimiento de estado (modificado, bloqueado, solo lectura, valor respaldo).
- Carga y extracción de datos desde/hacia `BufferRespuestas` y `DatosTabla`.
- Ciclo completo de edición: inicializar ? modificar ? validar ? respaldar/restaurar ? extraer ? bloquear.

Su implementación actual es extensa (>1400 líneas) y de estilo procedimental, con fuerte duplicación en estructuras `switch` por tipo de control.

## 2. Objetivo Principal
Abstraer operaciones repetitivas sobre un conjunto heterogéneo de controles UI en formularios administrativos del sistema, reduciendo código imperativo disperso y asegurando reglas homogéneas de validación y transferencia de datos.

## 3. Modelo de Datos Interno
### 3.1 `ElementoInterfaz`
Nodo de lista enlazada simple con:
- `mControl` (TObject*): puntero real al componente visual.
- `mTipoComponente` (enum `TiposControles`).
- `mCampo`: nombre lógico del campo (prefijo `#` en Inserta indica "no pertenece a tabla" y se limpia para almacenar).
- `mGrupo1/2/3`: etiquetas de agrupación independientes.
- Estado editable: `mValorInicial`, `mValorRespaldo`.
- Validación: `mLongitudMinima` (sobrecargado; valores negativos representan validaciones especiales), `mValidar` (bool).
- Metadatos: `mNumElemento`, `mPerteneceATabla`.
- Enlace: `mSiguiente`.

### 3.2 Estructura contenedora
`ControladorInterfaz` mantiene solo `mPrimerElemento` y `mUltimoElemento` para inserción cola y recorrido manual. El recuento se almacena en `mNumElementos`.

## 4. Dependencias Clave
- Controles personalizados: `VTEdit`, `VTLabeledEdit`, `VTComboBox`, `VTMaskEdit`, `VTCheckBox`, `VTRadioButton`, `VTRadioGroup`, `VTDateTimePicker`, `VTCheckListBox`, `VTListBox`, `VTStringGrid`, `VTBitBtn`, `VTButton`.
- Infra de datos: `ClassDatosTabla` (carga / extracción), `ClassBufferRespuestas` (llenado desde servidor).
- Utilidades: `ClassFuncionesGenericas` (conversiones, validaciones `ValidaEmail`, `ValidaTelefono`, formato fechas/horas, IntToAnsiString, DateToAnsiString, etc.).
- VCL: `TControl`, `TLabel`, `TPageControl`, `TTabSheet`, `TWinControl`.

## 5. Enumeraciones
- `TiposControles`: discrimina switches operativos (afecta validación, extracción, flags de edición, etc.). Mezcla componentes interactivos y contenedores neutrales.
- `TiposValidaciones`: Emails (`-1`), Teléfono (`-2`). Se codifican reutilizando el campo `mLongitudMinima` (sentinelas negativos) ? decisión que reduce claridad semántica.

## 6. Métodos (Agrupados)
### 6.1 Registro y Ciclo de Vida
- `Inserta(...)`: Valida coherencia (campos vacíos para tipos específicos, teléfono requiere anchos preasignados y tipo `VTMASKEDIT`). Construye nodo y enlaza al final de la lista. Lanza excepciones en errores de contrato.
- `QuitaElementos()`: Libera nodos, reinicia punteros y contador (no afecta los controles reales, de los que no es dueño).
- Destructor: invoca `QuitaElementos()`.

### 6.2 Inicialización y Presentación
- `Inicializa(Grupo, SoloSiPertenecenATabla)`: Recorre lista asignando `mValorInicial` a cada control elegible. Ajusta tipos numéricos (flotante/entero/moneda) y evita disparar eventos (`ActivarOnChange=false` / respaldo handlers click temporalmente).
- `Deshabilita` / `Habilita`: Cambian propiedad `Enabled` (no respetan estado bloqueado, solo setean directamente; el bloqueo se maneja en otra bandera interna de cada VTcontrol).
- `Oculta` / `Muestra`: Cambian `Visible`.
- `AsignaSoloLectura`: Cambia `ReadOnly` cuando el control lo soporta.
- `Bloquea` / `Desbloquea`: Cambian bandera `Bloqueado` (propiedad específica de la familia `VT*`).
- `ForceFocus(control)`: Atraviesa jerarquía de `Parent` y activa la `TabSheet` contenedora antes de hacer `SetFocus`.

### 6.3 Validación y Estados
- `Valida(Grupo, SoloSiPertenecenATabla)`: Recorre controles activos / visibles / marcados para validar. Reglas:
  - Longitud mínima para textos.
  - Email vía `mFg.ValidaEmail`.
  - Teléfono vía `mFg.ValidaTelefono` usando anchos configurados (`mAnchoLada`, `mAnchoNumTel`).
  - `VTMaskEdit::Validar()` para máscaras genéricas.
  - Manejo de primer error: establece `mMsgError`, enfoca control y detiene ciclo.
- `CambiaValidar(Validar, Grupo)`: Marca/desmarca controles de la validación.
- `AsignaAnchoLada` / `AsignaAnchoNumTel`: Parámetros para validación telefónica.
- `AsignaMensajeError` / `ObtieneMensajeError`: Mensajería de error externalizable.

### 6.4 Tracking Modificaciones y Respaldo
- `AsignaEstadoModificado(Modificado, Grupo)`: Marca bandera `Modificado` específica por tipo de control.
- `ObtieneEstadoModificado(Grupo)`: Retorna true si algún control del grupo está modificado (corto-circuito al primero encontrado).
- `ControlModificado(control, tipo)`: Helper para decidir si incluir en extracción (`LlenaDatos`).
- `Respalda(Grupo, SoloSiPertenecenATabla)`: Copia valor actual en `mValorRespaldo` (moneda sin comas, índices numéricos serializados a cadena).
- `Restaura(Grupo, SoloSiPertenecenATabla)`: Reconstruye estado desde respaldo.

### 6.5 Transferencia de Datos
- `CargaDatos(Buffer, Grupo)`: Expectativa: `Buffer->ObtieneNumRegistros()==1`. Para cada campo coincidente asigna valor (con conversión). Si no hay registro: inicializa. Si hay >1: lanza excepción (limitado a un solo registro).
- `LlenaDatos(DatosTabla, Grupo, SoloModificados)`: Inserta pares campo->valor según tipo conversión, respetando el filtro de modificados.

## 7. Flujo de Uso Típico
1. Configurar anchos para teléfono si se usarán validaciones telefónicas.
2. Registrar controles mediante `Inserta` (definiendo grupos y valores iniciales).
3. `Inicializa()` para establecer valores base.
4. (Opcional) `Respalda()` previo a edición.
5. Usuario interactúa; flags de modificación se alteran por lógica interna de cada control.
6. `Valida()` antes de guardar; si falla, se muestra mensaje y enfoca control.
7. `LlenaDatos(..., SoloModificados=true)` para preparar payload de actualización.
8. En caso de cancelar: `Restaura()`.
9. Aplicar `Bloquea()` / `Desbloquea()` según privilegios dinámicos.

## 8. Reglas e Invariantes (Implícitas)
- Cada nodo registrado debe ser consistente con el tipo de control real (`dynamic_cast` asume validez; no hay verificación de punteros nulos).
- Campos con prefijo `#` nunca se consideran pertenecientes a tabla.
- Validaciones especiales se señalan sobrecargando `mLongitudMinima` con sentinelas negativos.
- Para teléfono: se debe haber llamado antes a `AsignaAnchoLada/NumTel` o `Inserta` lanzará excepción si se pide esa validación.

## 9. Riesgos y Debilidades
| Riesgo | Descripción | Impacto | Mitigación |
|--------|-------------|---------|-----------|
| Lista enlazada manual | Recorre mutando `mPrimerElemento` y luego restaura (patrón frágil) | Posibles errores si se interrumpe flujo (excepción intermedia) | Usar iterador local (`for (auto *p=mPrimerElemento; p; p=p->mSiguiente)`) |
| Duplicación masiva | Código repetido en Inicializa, CargaDatos, Respalda, Restaura, etc. | Mantenimiento costoso | Extraer plantilla genérica o tabla de estrategias por tipo |
| Sobrecarga semántica de `mLongitudMinima` | Se usa tanto para longitud como sentinelas de validación especial | Confusión / errores futuros | Introducir campo `mTipoValidacion` separado |
| Falta de RAII | No se usa smart pointers; liberación manual | Fugas si se cambia estructura sin QuitaElementos | Modernizar a `std::unique_ptr` / contenedor STL |
| Ausencia de hilos seguros | Sin sincronización | Corrupción si UI multi-hilo | Restringir a hilo GUI, documentar | 
| Dependencia fuerte de VCL y controles custom | No aislado para test unitarios | Tests complicados | Inyección de interfaz abstracta (adapter pattern) |
| Errores silenciosos de cast | `dynamic_cast` sin comprobación posterior | Accesos inválidos si tipos no coinciden | Afirmaciones o verificación tras cast |
| Escalabilidad | Cada nuevo control exige modificar múltiples switch | Propenso a regresiones | Polimorfismo por interfaz común (Strategy/Visitor) |
| Excepciones de Inserta con mensajes concatenados | Construcción de strings con números sin sanitizar | Mensajes crípticos | Centralizar formateo mensajes |

## 10. Oportunidades de Refactor
1. Reemplazar lista enlazada por `std::vector<std::unique_ptr<ElementoInterfaz>>` ? iteración sencilla y memoria contigua.
2. Separar responsabilidades en mini-manejadores por tipo (`IControlAdapter` con métodos: setValorInicial, getValor, setReadOnly, setBloqueado, valida, respalda, restaura, etc.).
3. Introducir `enum class TipoValidacion { Ninguna, Longitud, Email, Telefono, Mascara }` y campos explícitos (por ej. `minLongitud`).
4. Fusionar procedimientos duplicados mediante función genérica `AplicarAGrupo(Grupo, lambda)`.
5. Externalizar formato/parseo monetario a helper específico (evitar StringReplace repetido).
6. Añadir logging estructurado para validaciones fallidas (contexto: campo, valor, regla). 
7. Documentar claramente contrato de propiedad: no destruye controles asociados.
8. Introducir `try/catch` en loops que alteran `mPrimerElemento` para garantizar restauración en excepción.
9. Optimizar `LlenaDatos`: construir lista previa de elementos modificados para evitar segunda evaluación.
10. Exponer iterador const para inspección externa sin mutar estado interno.

## 11. Contrato Propuesto
- Pre: Todos los controles registrados con `Inserta` existen durante la vida del `ControladorInterfaz` (no se destruyen fuera sin desregistrar).
- Pre: Para validación telefónica se establecen anchos antes de registrar controles con esa regla.
- Post `Inserta`: `mNumElementos` incrementa y `mUltimoElemento` apunta al nuevo nodo.
- Post `Valida`: Devuelve true si no se estableció mensaje de error; si false, `ObtieneMensajeError()` describe la causa y el foco se mueve al control infractor.
- Post `CargaDatos(Buffer, G)`: Si `Buffer` tiene 1 registro, los controles del grupo se sincronizan con él; si 0, se re-inicializan; >1 lanza excepción.

## 12. Ejemplo Simplificado
```cpp
ControladorInterfaz ci;
ci.AsignaAnchoLada(3);
ci.AsignaAnchoNumTel(7);
ci.Inserta(txtNombre, VTEDIT, "nombre", "GENERAL", "", "", "", 3);
ci.Inserta(txtCorreo, VTEDIT, "email", "GENERAL", "", "", "", VALIDAR_EMAIL);
ci.Inserta(mskTelefono, VTMASKEDIT, "telefono", "GENERAL", "", "", "", VALIDAR_TELEFONO);
ci.Inicializa();
// Usuario edita...
if (!ci.Valida("GENERAL")) {
    ShowMessage(ci.ObtieneMensajeError());
    return;
}
DatosTabla datos; datos.AsignaTabla("clientes");
ci.LlenaDatos(&datos, "GENERAL", true); // Solo campos modificados
```

## 13. Interacción con Otros Módulos
- `BufferRespuestas`: fuente de datos unitaria para carga de formulario.
- `DatosTabla`: destino para operaciones CRUD (alta / actualización). El controlador no envía directamente al servidor; solo llena el contenedor.
- `FuncionesGenericas`: validaciones y conversiones; dependencia transversal fuerte.
- Posible coordinación con un módulo de privilegios para invocar `Bloquea` / `Desbloquea` según permisos (ver `ClassPrivilegiosDeObjeto`).

## 14. Observaciones de Calidad
- Cohesión media: combina validación, mapeo de datos, estado visual y control de interacción.
- Acoplamiento alto a jerarquía concreta de componentes; difícil migrar a otro framework.
- Falta de pruebas y separación de preocupaciones hace que cada cambio sea de riesgo.
- Uso de reinterpretación de semántica de `LongitudMinima` es un code smell.

## 15. Métricas Aproximadas (del archivo fuente)
- >1400 líneas: tamaño que justifica segmentación (una clase por responsabilidad o patrón Strategy por tipo de control).
- 10+ bloques `switch` repetidos sobre el mismo enum.

## 16. Prioridad de Refactor (Orden Sugerido)
1. Extraer adaptadores por tipo de control (elimina duplicación).
2. Separar validación en servicio aparte (inyectable / testeable).
3. Reemplazar lista enlazada por contenedor STL ? simplifica loops & excepciones.
4. Introducir metadatos semánticos separados (validación vs longitud).
5. Desacoplar de VCL mediante interfaces cuando se busque test unitario.

## 17. Resumen Ejecutivo
`ControladorInterfaz` aporta una capa unificada sobre un conjunto amplio de controles personalizados, resolviendo de manera pragmática la edición y validación de formularios. No obstante, su implementación monolítica y repetitiva dificulta extensibilidad y prueba. Se recomienda un refactor progresivo hacia un modelo orientado a adaptadores y servicios de validación desacoplados para mejorar mantenibilidad y reducir riesgo.

---
Última revisión: 2025-09-22
