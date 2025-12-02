# Proceso Legado: Catálogo de Proveedores

## Introducción
El catálogo de proveedores es el módulo legacy que permite registrar, consultar, actualizar y dar de baja proveedores con los que la organización mantiene relaciones comerciales para operaciones de compra de mercancía o gastos. Sirve como repositorio maestro de datos de identificación, contacto, condiciones comerciales, parámetros de crédito, referencias bancarias, banderas fiscales y configuraciones que impactan procesos posteriores (compras, pagos, conciliación, generación de complementos de pago, pedidos automáticos y cálculo de precios).

Está orientado a usuarios administrativos de compras, finanzas / cuentas por pagar y personal de control interno que requiere validar la consistencia de la información cargada. Este documento describe el funcionamiento funcional (no técnico) observado a partir de la interfaz legacy y reglas que emergen del comportamiento del sistema.

## Resumen ejecutivo
- El alta de un proveedor requiere definir su tipo (persona física o moral), datos de identificación (razón social o nombre del representante), dirección fiscal y contactos clave, además de parámetros opcionales de crédito y bancarios.
- El módulo centraliza banderas que afectan cálculos y controles: aplicación de crédito, agrupación de pagos, redondeos para cálculo de pedidos, clasificación de uso (gastos vs mercancía), retenciones fiscales, vigencia y condiciones comerciales.
- Los procesos clave: Alta, Modificación, Baja y Consulta se apoyan en validaciones front-end (formatos, obligatoriedad) y en lógica de negocio de nivel servidor (generación de clave secuencial, preservación de integridad referencial, verificación de saldos antes de inactivar, actualización condicionada de registros relacionados).
- Existen restricciones para evitar duplicidades (RFC) y para asegurar coherencia de parámetros de pedido automático (solo un tipo de mínimo: cajas, peso o dinero). Las condiciones comerciales mantienen un historial consultable.

## Estructura de la Interfaz
La interfaz se organiza en pestañas (TPageControl). Principales agrupaciones funcionales:
1. Principales: Identificación general, clasificación, banderas de estatus, parámetros básicos de crédito, retención y comprador/capturista.
2. Dirección y Contacto: Domicilio fiscal / operativo, correos, teléfonos y contacto comercial.
3. Datos de Crédito y Adicionales: Cuentas bancarias, descuentos especiales, apoyos y reglas de cálculo de pedidos (redondeo a techo, reducción de costo base).
4. Condiciones Comerciales: Captura y mantenimiento de pactos (fecha de pacto, vigencia, involucrados y texto de la condición) con historial y estados (vigente, cancelado, cumplido) e impresión / exportación.
5. Condiciones Gastos: Configuración específica cuando actúa como proveedor de gastos (cuenta contable, tipo de retención, agrupación de pagos, cuentas de retención IVA/ISR).
6. Pedidos Automáticos: Parámetros para generar pedidos automáticos (días reorden, mínimos por cajas, peso o dinero, confianza para pedido, vigencia de días y ajustes bancarios si aplican a procesos financieros relacionados).

## Campos y Validaciones
Se listan por sección destacando propósito, obligatoriedad, validaciones y efectos. Cuando alguna validación no es deducible se marca.

### 1. Principales
- Código del proveedor: Clave interna generada automáticamente en alta (secuencia basada en sucursal). No editable en modificación.
- Tipo de Empresa (Persona Física / Persona Moral): Controla visibilidad y obligatoriedad de campos de Nombre vs Razón Social.
- Razón Social: Obligatoria para persona moral. Para persona física se rellena automáticamente con el nombre del representante si el campo no está visible.
- Título (Sr., Sra., Ing., etc.): Campo opcional de cortesía asociado al representante legal.
- Nombre del representante legal: Obligatorio para persona física. Validación: no permitir vacío si tipo = física. (Lógica confirmada en validación previa al guardado.)
- Fecha de Nacimiento / Fundación: Fecha asociada al representante (física) o constitución (moral). Valor por defecto legado (p.ej. 01/01/1950 si no se captura). Uso preciso no deducible en cálculos posteriores (NO INFERIBLE).
- RFC: Validación de formato según tipo (física vs moral); si formato incorrecto se exige corrección o dejar vacío.
- CURP: Campo para persona física; formato controlado por máscara (patrón alfanumérico). Validaciones detalladas internas no totalmente visibles (máscara asegura longitud/estructura básica).
- Activo: Bandera de habilitación. Al intentar desactivar se valida que no existan saldos pendientes (se consulta movimientos de compras/pagos); si hay saldo ≠ 0 se rechaza la desactivación.
- Es parte relacionada: Indica relación corporativa/fiscal especial. Sin validación adicional observable.
- Banderas de “Cuadrar” (Compras, Notas Crédito, Notas Cargo, Pagos): Definen si el sistema fuerza coincidencia exacta entre totales de documentos y representaciones (QR/XML). Relevantes para integridad fiscal.
- Emiten complemento de pago: Si activo, se asume que el proveedor genera complementos; influencia exacta en procesos downstream NO INFERIBLE.
- Descuento regular y Descuento por pronto pago (PPP): Númericos (%). Descuento PPP habilitado solo si se activa crédito. Validaciones de rango (0–100%) no visibles; asumir control básico (NO INFERIBLE exacto).
- Crédito Autorizado + Plazo + Límite de Crédito: Solo habilitados si “Crédito” está marcado. Plazo y límite requieren numéricos; si desmarca crédito se deshabilitan y datos podrían ser ignorados o enviados nulos.
- Comprador: Obligatorio si NO es proveedor de gastos; se fuerza selección de un elemento en combo (validación front-end). Si es proveedor de gastos y no se selecciona comprador, se acepta (regla condicionada).
- Capturista: Usuario asociado; opcional. Si no seleccionado puede almacenarse NULL.
- Retención (Impuesto retenido / Es RESICO): Si “Es RESICO” está marcado, debe elegir un impuesto ISR retenido activo; se valida que porcentaje no sea “0”.
- Referencia de Pago (Tipo referencia: Fija, Variable, Folio Proveedor, Razón Social): Define cómo se construye la referencia en procesos de pago. Si se selecciona tipo diferente de “Fija” se ignora valor de “Referencia Fija”.
- Días de vigencia (diasvigencia): N° de días aplicables a alguna condición de uso; impacto exacto NO INFERIBLE.
- Agrupar pagos facturas / Agrupar pagos gastos: Banderas para consolidación de pagos. Efecto técnico no detallado (funcional: reduce número de transacciones de pago emitidas).
- Proveedor Gastos / Proveedor Mercancía: Clasificación operativa; incide en validez de campos (comprador vs cuenta de gastos) y potencial segmentación de reportes.
- Ajuste bancario: Bandera que habilitaría tratamiento especial en conciliaciones (NO INFERIBLE detalles).
- Redondear a techo (cálculo pedido): Controla criterio de redondeo de cálculos de pedido automático.
- Reducción costo base (% reducción): Habilitado solo si privilegio “RED” presente. Si se activa permite aplicar porcentaje para cálculo de precios base (finalidad: negociación comercial). Validación de privilegios: si usuario no tiene privilegio, campos se bloquean.
- Agrupa pagos por facturas / gastos: Alternan estrategia de consolidación.
- Es RESICO: Activa validación de ISR retenido.
- Min campos de pedido automático (mincajas, minpeso, mindinero): Regla: solo uno de los tres puede ser > 0. Validación explícita: si más de uno > 0 se bloquea guardado.

### 2. Dirección y Contacto
- Calle y Número: Texto (máx 60). Opcional pero relevante para documentación fiscal; no se observa validación de obligatoriedad estricta.
- Colonia / Clave Colonia: Doble captura (texto + catálogo). Si no se selecciona del combo, cvecolonia puede quedar NULL.
- Código Postal (CP): Numérico 5 dígitos. Valida si pertenece a lista permitida (para Carta Porte / CP20). Si no válido se muestra alerta visual.
- Municipio (Localidad) / Estado / País: País por defecto MEXICO. Estado se selecciona de catálogo. Estado es clave foránea.
- Correo electrónico proveedor (email): Validación manual post-grabación: se revisan patrones inválidos (ej. combinaciones '@-' o '-@', '-.', '.-'). Si inválido se muestra mensaje pero el guardado ya ocurrió (riesgo descrito en Riesgos).
- Contacto (nombre contacto comercial) + Email contacto: Igual validación de correo; errores notificados tras grabar.
- Teléfonos: Subcomponente que permite introducir tipo, lada, número, extensión. Lista visual; se pueden agregar o remover filas antes de grabar. Validación de formato no detallada (NO INFERIBLE precisa), sí control de máscaras.

### 3. Datos de Crédito y Adicionales
- Cuentas Bancarias (1,2,3): Cada una con Tipo de cuenta y Banco asociado. Si el tipo de cuenta es de banco específico (ej. BBVA), el banco se fija y vuelve de solo lectura.
- Cuenta principal: Selección (radio) de cuál de las cuentas registradas se usará como principal (1,2 o 3). Se almacena en campo cuentadefault (1–3). Si no hay cuentas indicadas el valor se fuerza a NULL.
- Apoyos / Fecha Último Apoyo (fechauap): Registro informativo de apoyos recibidos. Sin lógica adicional observable.
- Descuento a costo base (%): Solo editable si privilegio concede descuento; combinado con bandera Redondear techo y reducción costo base.
- Parámetros de Pedido (se completa en pestaña específica, ver sección 6): algunos duplicados conceptuales (redondeo) visibles aquí por usabilidad.

### 4. Condiciones Comerciales
- Fecha de Pacto y Fecha de Vigencia: Fechas del acuerdo comercial. Se validan contra la fecha actual al salir del control (ej. vigencia debe ser >= hoy — lógica aparente con mensaje si no cumple).
- Comprador / Representante: Identificación de actores que pactan. Representante texto libre; comprador desde catálogo de empleados activos. Cambio marca bandera para indicar modificación pendiente.
- Condición Comercial (texto multilinea): Máximo deducible ~500 caracteres. Se evita entrada de comillas simples, dobles y '|' para prevenir problemas en SQL y formato de envío por correo.
- Historial: Grid con columnas (Condición ID, Proveedor, Fecha Pacto, Fecha Vigencia, Empleado, Representante, Condición, Vigente, Cancelado, Cumplido). Cada fila refleja un registro histórico no vigente o estado actual. Iconografía indica estados (1 activo vs no).
- Estados especiales:
  - Vigente: Calculado en backend si fecha_vigencia >= hoy.
  - Cancelado / Cumplido: Cambiables vía menú contextual con verificación de privilegios (CNE para cancelar, CUM para cumplido). Al activar se ocultan opciones para evitar doble acción.
- Impresión / Exportación: Se permite exportar historial a Excel o imprimir condición seleccionada.
- Envío automático por correo: Bandera VTCheckBoxCFDEnvioAuto; si activa requiere correos válidos (proveedor y contacto). Verifica formato antes de permitir marcar.

### 5. Condiciones Gastos
- Cuenta de gastos: Obligatoria si se marca proveedor de gastos; si no se captura, al grabar se valida según reglas de sección Principales.
- Tipo de retención: Selección entre configuraciones (No configurada, Arrendamiento, Honorarios). Interactúa con retenciones aplicables.
- Cuentas de Retención (IVA, ISR): Se consultan y persistidas en tabla cuentas_proveedor; actualizadas en modificaciones por campo individual (pueden ponerse NULL si se limpia).
- Agrupar pagos de gastos: Consolida liquidaciones.

### 6. Pedidos Automáticos
- Días de reorden: Entero (por defecto 7). Habilitados en relación a criterios de inventario / abastecimiento. Activación depende de banderas de confianza.
- Mínimo Cajas / Min Peso (kg) / Min Dinero: Regla exclusiva: solo uno > 0. Validación implementada antes de grabar (mensaje de error si conflicto).
- Confianza para pedidos: Bandera que aprueba su participación en algoritmo automático.
- Días de vigencia (diasvigencia) y otros campos combinan cálculo de caducidad / validez de ofertas o pactos (detalles NO INFERIBLE exacto).
- Ajuste bancario: Marca posibilidad de ajustes en procesos bancarios vinculados a pagos (semántica exacta NO INFERIBLE).

### Campos Observados con Uso Dudoso
- Fecha de Nacimiento / Fundación y Fecha de Nacimiento de Contacto: Persisten con valores por defecto lejanos (1900/1950) cuando no se capturan — su explotación real no se evidencia.
- Reducción de costo base + Descuento a costo base + Redondear a techo: combinaciones de lógica de pricing no plenamente visibles (riesgo de ambigüedad para usuarios).

## Procesos

### Proceso de Alta
1. Usuario ingresa al formulario y selecciona “Agregar”. El sistema limpia controles y habilita campos editables.
2. Selecciona Tipo de Empresa.
3. Captura datos de identificación: Razón Social (moral) o Nombre / Representante (física). RFC y CURP (si aplica).
4. Registra dirección y contacto (calle, colonia, estado, correos, teléfonos).
5. Define banderas operativas (activo, cuadrar documentos, emiten complemento, clasificación gastos/mercancía, parte relacionada, confianza pedidos, etc.).
6. (Opcional) Configura crédito (marca checkbox) y entonces captura Plazo, Límite, Descuentos.
7. (Opcional) Define cuentas bancarias y elige principal; asigna retenciones si aplica (RESICO / tipo retención).
8. (Opcional) Introduce parámetros de pedidos automáticos (solo un mínimo > 0) y vigencia.
9. (Opcional) Captura Condición Comercial inicial (texto, fechas, comprador, representante).
10. Guarda. El backend genera clave secuencial (prefijo sucursal + número correlativo). Inserta registros telefónicos y condición comercial si marcada como modificada.
11. Muestra mensaje de éxito y habilita bitácora si procede.

Criterio de finalización: Clave generada y visible, controles pasan a modo consulta/modificación y se permite impresión / bitácora.

### Proceso de Modificación
1. Buscar proveedor (uso de formulario de búsqueda con posibilidad de incluir inactivos).
2. Al cargar, el sistema evalúa privilegios para bloquear/permitir edición (bajas, descuentos, estados, banderas).
3. Usuario cambia campos deseados; modificaciones en condición comercial marcan bandera interna para envío parcial al backend.
4. Validaciones: formato RFC/curp, exclusividad mínimos pedidos, comprador vs gastos, correos básicos, ISR retenido si RESICO.
5. Guardar actualiza solamente campos cambiados; condiciones comerciales se insertan o actualizan según cambios detectados.
6. Histórico se refresca; si se cambió plazo de crédito, se ajustan fechas de vencimiento de compras con saldo (regla inferida por lógica de servidor).

Criterio de finalización: Cambios persistidos y reflejados en controles; estado de modificado restablecido a limpio.

### Proceso de Baja
1. Usuario con privilegio de baja intenta desactivar o eliminar.
2. Sistema verifica saldos pendientes (compras / pagos). Si saldo ≠ 0, cancela operación (mensaje explicativo).
3. Si no hay impedimentos: marca inactivo (baja lógica). Eliminación física total no se observa en interfaz (nota: función backend permite delete completo, pero proceso funcional recomendado es inactivar).
4. Se actualizan banderas y se restringen acciones posteriores.

Criterio de finalización: Proveedor marcado inactivo y no aparece en búsquedas por defecto de “solo activos”.

### Proceso de Consulta
1. Acceso a búsqueda, selección de proveedor.
2. Carga datos generales, teléfonos, cuentas, condiciones comerciales y estados (vigente / cancelado / cumplido).
3. Usuario puede navegar pestañas, imprimir condición, exportar historial, revisar bitácora (si disponible).

Criterio de finalización: Información visualizada sin cambios y posibilidad de salir sin afectar datos.

## Relaciones de Datos
Conceptualmente:
- Proveedor (tabla principal) se relaciona con: Estados (catálogo geográfico), Colonias (para localizar y normalizar direcciones), Empleados (como comprador), Usuarios (capturista, usu alta/modificación), Impuestos (retención ISR), Cuentas de retención (tabla cuentas_proveedor), Teléfonos (telefonosproveedores), Condiciones comerciales (condicionescomerprov).
- Condiciones Comerciales: Historial con fechas vigencia y estado (activo/cancelado/cumplido). Cada registro conserva texto libre y actores (empleado y representante).
- Cuentas Proveedor: Retenciones específicas de IVA / ISR complementarias a imputaciones fiscales.
- Teléfonos: Lista dependiente a nivel hijo 1:N.
- Parámetros de Pedido Automático: Integrados en la propia entidad proveedor para cálculo en módulos de abastecimiento.
- Bitácora (bitacoraproveedores): Registra movimientos de alta / modificación (inserción post-grabación). Provee trazabilidad.

## Reglas de Negocio Destacadas
- Exclusividad de mínimos pedidos automáticos: solo uno de mincajas, minpeso, mindinero puede ser > 0.
- Validación de RFC depende del tipo de empresa (física vs moral).
- Proveedor de gastos sin comprador requiere cuenta de gastos; proveedor no de gastos requiere comprador.
- No se puede desactivar un proveedor con saldo pendiente.
- Tipo de cuenta bancaria “BBVA” fuerza banco asociado y bloquea edición manual del combo.
- Referencia fija solo se utiliza si tipo de referencia = Fija.
- Si persona física: razón social se rellena con nombre del representante si campo oculto.
- RESICO requiere impuesto de ISR retenido activo distinto de cero.
- Actualización de plazo de crédito impacta recomputación de fechas de vencimiento de compras con saldo.
- Control de privilegios habilita/deshabilita campos (descuento costo base, activar/inactivar, cuadrar documentos, cancelar/cumplir condición comercial).

## Riesgos y Controles Recomendados
| Riesgo | Impacto | Control Recomendado |
|--------|---------|---------------------|
| Correos inválidos detectados solo post-grabación | Inconsistencias en comunicaciones automáticas | Validar antes de enviar al backend y bloquear grabado si formato erróneo |
| Valores por defecto de fechas (1900/1950) sin contexto | Reportes inexactos | Marcar campos como obligatorios o permitir NULL real y documentar uso |
| Múltiples banderas de cuadrado mal comprendidas | Configuración errónea fiscal | Incluir ayuda contextual / tooltip documentado |
| Reducción costo base y descuentos combinados sin clara prioridad | Cálculos de precio inconsistentes | Definir orden de aplicación y registrar en documentación técnica complementaria |
| Cambios de plazo de crédito re-calculan vencimientos sin confirmación | Impacto en cuentas por pagar | Solicitar confirmación emergente al cambiar plazo |
| Eliminación física potencial (backend) vs baja lógica en interfaz | Pérdida de histórico | Restringir delete físico a mantenimiento controlado y auditar |
| Falta de validación de rango (%) en descuentos | Posible entrada absurda (ej. 999) | Añadir validación explícita 0–100 |
| Dependencia de privilegios para campos críticos (descuento costo base) | Riesgo uso indebido si privilegios mal asignados | Revisión periódica de asignaciones de privilegio |

## Glosario
- Proveedor de Gastos: Proveedor usado para facturación de servicios/gastos operativos no inventariables.
- Proveedor de Mercancía: Proveedor que suministra productos físicos para inventario / venta.
- RESICO: Régimen Simplificado de Confianza (fiscal). Requiere retención ISR específica.
- Complemento de Pago: Documento fiscal que complementa una factura pagada en parcialidades o posterior a su emisión.
- Condición Comercial: Conjunto de términos pactados (descuentos, soporte, plazos, compromisos) vigentes hasta su fecha de vigencia.
- Bitácora de Proveedores: Registro histórico de altas y modificaciones para auditoría.
- Pedido Automático: Generación sugerida de órdenes según parámetros mínimos y días de reorden.

## Información Incompleta
- Uso exacto de campos de vigencia (diasvigencia) en cálculos posteriores: NO INFERIBLE.
- Significado operativo de algunas banderas (emitencpago, ajuste_bancario) en subsistemas posteriores: NO INFERIBLE.
- Rangos máximos definitivos de porcentajes de descuento y reducción costo base: NO INFERIBLE.
- Reglas de combinación entre descuento regular, PPP y reducción costo base: NO INFERIBLE.

## Nota de confidencialidad
"Este documento describe un sistema legado y se entrega exclusivamente para fines de análisis funcional y documentación interna. No debe difundirse externamente sin autorización."