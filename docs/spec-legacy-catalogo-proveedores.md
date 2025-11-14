# spec-legacy-catalogo-proveedores.md

Estado: descripción técnica fiel del comportamiento actual. No incluye sugerencias de mejora ni modernización.

## 1. Introducción técnica general

Propósito del módulo: administrar el catálogo maestro de proveedores (altas, bajas lógicas/físicas, modificaciones y consulta detallada) incluyendo:
- Datos generales de identificación (tipo de empresa, razón social / representante legal, RFC, CURP, dirección, colonia / estado, país, correos, contacto).
- Parámetros comerciales y de crédito (límite, plazo, descuentos, descuentos por pronto pago, apoyos, cuentas bancarias, tipos de cuenta, cuenta principal, referencia de pago fija o dinámica, agrupación de pagos, vigencia de condiciones comerciales, parámetros de pedido automático: mincajas / minpeso / mindinero / días reorden / confianza / cotizable).
- Clasificación operacional (proveedor de gastos vs mercancía, parte relacionada, banderas de cuadrado de totales, emisión de complemento de pago, retenciones, impuestos retenidos, cuentas contables asociadas para retenciones).
- Bitácora de condiciones comerciales históricas (pacto, vigencia, empleado/representante, texto libre condicional, estado: vigente, cancelado, cumplido) y bitácoras de cambios (bitacoraproveedores en otra parte del sistema).

Arquitectura funcional (observada):
Cliente (VCL C++ Builder) -> Formulario `TFormCatProveedores` (contiene controles, validaciones UI, construcción de buffer de parámetros) -> Cliente de transporte (`gClienteVioleta`) empaqueta petición (ID_GRA_PROVEEDOR / ID_BAJ_PROVEEDOR / ID_CON_PROVEEDOR) -> Servidor (`ServidorCatalogos`) interpreta parámetros, arma SQL dinámico y ejecuta en MySQL -> Respuesta serializada (múltiples result sets concatenados) -> Cliente parsea mediante `BufferRespuestas` y llena controles / combos / grids.

Flujo general alta/modificación:
1. Usuario pulsa Agregar o Modificar (navegación en `FrameNav`).
2. UI valida campos (`mControlador.Valida()`, reglas adicionales específicas). 
3. Método cliente `TFormCatProveedores::GrabaProveedor()` serializa: modo (A/M), clave proveedor (si M), plazo, usuario, campos mapeados de la tabla `proveedores`, datos de teléfonos (si cambiaron), bloque de condiciones comerciales (solo si hay cambio), y datos de cuentas de retención (o flags para UPDATE parcial) en el orden definido.
4. Servidor `ServidorCatalogos::GrabaProveedor()` parsea en el mismo orden, crea transacción, genera folio (si alta) con secuencia en `foliosemp` (folio 'PROV'), inserta / actualiza `proveedores`, maneja side-effects (ajuste de plazo en compras existentes), sustituye teléfonos, inserta/actualiza condiciones comerciales, inserta/actualiza `cuentas_proveedor`, COMMIT.
5. Devuelve `select @folio as folio` para reflejar la clave resultante (en alta).
6. Cliente refresca formulario invocando `CargaProveedor()` que ejecuta `ConsultaProveedor` para reconstruir la vista.

## 2. Estructura de la interfaz (Form)

Clase: `TFormCatProveedores` (definida en `FormCatalogoProveedores.h/.cpp`, diseño en `.dfm`).

Contenedores clave:
- `TPageControl *PageControlProveedores` con pestañas (identificadas en dfm): Principales, (Ubicación), Crédito, Condiciones, (Adicional / TabSheet1), Pedidos automáticos, etc. (Los nombres exactos de todas las TabSheet relevantes presentes en header: `TabSheetPrincipales`, `TabSheetProveedoresUbicacion`, `TabSheetlCredito`, `TabSheetlCondiciones`, `TabSheet1`, `TabSheetPedidosAutomaticos`).

Control binding: en el constructor se invocan numerosas llamadas `mControlador.Inserta(...)` con la firma:
`mControlador.Inserta(Componente, TipoControl, "campo", "tabla", filtro/grupo, ..., valor_default, flags)`.
Esto establece un mapeo bidireccional entre control y campo de la tabla principal (`proveedores`) o tablas relacionadas (`condicionescomerprov`, `cuentas_proveedor`). Ejemplos (extracto literal exacto):
```
mControlador.Inserta(RadioGroupTipoEmpre, VTRADIOGROUP, "tipoempre", "proveedores", "", "", "1");
mControlador.Inserta(DtpFechNrep, VTDATETIMEPICKER, "fechnrep", "proveedores", "", "", "01/01/1950", 2);
mControlador.Inserta(EditRazonSocial, VTLABELEDEDIT, "razonsocial", "proveedores", "", "", "", 2);
mControlador.Inserta(EditRFC, VTLABELEDEDIT, "rfc", "proveedores", "", "", "", 0);
... (más de 70 inserciones incluyendo teléfonos, condicionescomerprov y cuentas_proveedor)
```

Eventos principales (según sección __published__ de `.h`):
- Navegación: `FrameNavButtonAgregarClick`, `FrameNavButtonQuitarClick`, `FrameNavButtonGrabarClick`, `FrameNavButtonCancelarClick`, `FrameNavButtonBuscarClick`.
- Validación dinámica / UX: `RadioGroupTipoEmpreClick`, `CheckBoxCreditoClick`, `EditDescuentoExit`, `EditRepLegalChange`, `EditRazonSocialChange`, `CheckBoxActivoClick`, `CheckBoxRedondearTechoClick`, `EditCPExit`, `ComboBoxTipoCuenta(1..3)Change`, `RadioGroupTipoRefPagProvClick`, etc.
- Condiciones comerciales: `DtpFechaPactoChange`, `DtpFechaVigenciaPactoChange`, `ComboBoxEmpleadoChange`, `MemoContenidoChange`, impresión `VTBitBtnImprimeConComClick`.
- Historial grid: `StringGridHistorialDrawCell`, `StringGridHistorialDblClick`, `StringGridHistorialClick`.

Estructuras/variables internas relevantes (privadas):
- `PrivilegiosDeObjeto mPrivilegios`: determina permisos (modificar, bajas, privilegios específicos: "RED", "ACT", "EST").
- `ControladorInterfaz mControlador`: mecanismo central de binding y validación.
- Varias `AnsiString` para datos contextuales (`mProveedor`, `mRazonSocialProveedor`, dirección, etc.).
- Flags condiciones comerciales: `bool mNoEsMismaCondicion; bool mBanderaCambioCondicion; int mIdCondicion;`.
- Estado cuentas: `AnsiString mClaveProv; bool mProveedorGastos;`.
- Parámetro global RESICO: `AnsiString mParamImpRESICO`.

Flujo cliente (consulta): `CargaProveedor()` construye petición ID_CON_PROVEEDOR, parsea múltiples result sets secuenciales (proveedor, teléfonos, bancos, cuentas retención, condiciones) usando `BufferRespuestas` y rellena controles, además de lógicas condicionales (habilitar/readonly para tipos de cuenta bancarios BBVA, set de radio principal según `cuentadefault`).

Validaciones UI previas a persistencia (en `GrabaProveedor()`):
- `mControlador.Valida()` (reglas implícitas de los bindings, e.g. campos obligatorios).
- RFC dependiendo de tipo de empresa (`mFg.validaRFC(rfccorto,"0"|"1")`).
- Persona física requiere `EditRepLegal` no vacío.
- Proveedor NO de gastos requiere comprador seleccionado; proveedor de gastos requiere cuenta contable de gastos.
- Si `CheckBoxEsResico` activo, impuesto retenido (ISR) debe ser activo (verifica sección 3 de texto del combo: `mFg.ExtraeSeccion(ComboBoxImp->Text,3)`).
- Exclusividad de mínimos: sólo uno distinto de cero entre `mincajas`, `minpeso`, `mindinero` (también reforzado en CHECK de base de datos `caja_peso_dinero`).
- Ajuste de referencia fija: se pone NULL cuando no aplica (`tiporefpago` > 0 o texto vacío).
- Nulificación condicional de campos vacíos (cvecolonia, comprador, capturista, imupuestoret si no RESICO, cuentas de tipo si vacías, etc.).

## 3. Detalle por pestaña (TTabSheet)

Nota: El `.dfm` extenso define layout; se documentan pestañas declaradas en `.h` (solo se incluyen comportamientos funcionales que impactan lógica / queries, no estética).

1. TabSheetPrincipales ("Principales"):
	- Campos de identidad y clasificación: `proveedor`, `tipoempre`, `razonsocial`, `replegal`, `titrepleg`, `fechnrep`, `rfc`, `curp`.
	- Flags operativos: `esparterelac`, `activo`, cuadrar (4 flags), `emitencpago`.
	- Crédito y descuentos: `credito`, `limcred`, `plazo`, `descppp`, `descuento` (habilitados/deshabilitados según checkbox crédito).
	- Bancos y cuentas: `tipocuenta1..3`, `bancoc1..3`, `cuentab1..3`, `cuentadefault`, control de longitud dinámica según tipo de cuenta y bloqueo de banco si tipo predefine banco (caso BBVA).
	- Comprador, capturista, impuesto retenido (`impuestoret`), redondeo (`redondeocptecho`), reducción costo base (`reduccostobase` / `porcreduccosto`).
	- Referencia de pago: `tiporefpago` (radio), `referenciafija` (solo válida si `tiporefpago`==0 y campo lleno, caso contrario se envía NULL).
	- Validaciones y side-effects: cambio tipo de empresa alterna visibilidad de razón social vs representante legal y etiqueta de fecha (nacimiento/fundación).

2. TabSheetProveedoresUbicacion:
	- Dirección: `calle`, `colonia`, `cvecolonia` (combo poblado con `SELECT col.colonia ...`), `cp`, `localidad`, `estado`, `pais` (default MEXICO), `email` (validación simple de secuencias inválidas post-grabación), contacto: `contacto`, `emailcto`, `fechncon` (fecha contacto). Teléfonos se gestionan con sub-frame `TCajaListaTelefonos` que produce filas: (tipo, lada, teléfono, extencionTel).
	- Poblaciones combos se cargan en `Inicializa()` con selects literales (ver sección 4 para SQL).

3. TabSheetlCredito ("Crédito"):
	- Reutiliza controles ya listados en Principales (crédito, descuentos, apoyos, fecha UAP `fechauap`). No genera queries específicas en cliente aparte del binding principal.

4. TabSheetlCondiciones ("Condiciones"):
	- Gestión de condiciones comerciales: `fecha_pacto`, `fecha_vigencia`, `empleado` (combo), `representante`, `condicion_comer` (MemoContenido), historial en `StringGridHistorial` con columnas fijas (Condicion, Proveedor, Fechas, Empleado, Representante, Texto, Vigente, Cancelado, Cumplido).
	- Flags internos `mBanderaCambioCondicion`, `mNoEsMismaCondicion`, `mIdCondicion` controlan si se envía bloque de condiciones en el buffer (valor "1" + campos) o se omite ("0").
	- SQL resultante para listado condiciones proviene de servidor (ver ConsultaProveedor).

5. TabSheet1 (propiedades adicionales):
	- Banderas: `provgastos`, `provmercancia`, `esresico`, `impuestoret`, `numcuenta` (cuando gastos), retención: `tiporetencion`, `diasvigencia`, cuentas para retenciones: `cuenta_retencion_iva`, `cuenta_retencion_isr` (en tabla `cuentas_proveedor`), agrupaciones de pagos: `agrupapagfact`, `agrupapaggast`.

6. TabSheetPedidosAutomaticos:
	- Reorden y mínimos: `diasreorden`, `mincajas`, `minpeso`, `mindinero`, `confianzapedidoautomatico`.
	- Ajustes: `ajuste_bancario`.
	- Comercial adicional: `cotizable`, `correo_cotizacion`.
	- Exclusividad de mínimos validada en UI + constraint DB.

Queries generadas por la interfaz (carga de combos) – literales en `Inicializa()` (fragmentos exactos):
```
SELECT estado AS Clave, nombre AS Nombre FROM estados;
SELECT c.empleado, CONCAT(e.nombre, ' ', e.appat, ' ', e.apmat) AS nombre FROM empleados e INNER JOIN compradores c ON e.empleado = c.empleado AND c.activo = 1 WHERE e.activo = 1 ORDER BY e.nombre, e.appat, e.apmat;
SELECT col.colonia AS Clave, loc.nombre AS localidad, col.nombre AS Nombre FROM colonias col INNER JOIN localidades loc ON loc.localidad = col.localidad ORDER BY col.localidad, col.Nombre;
SELECT impuesto, tipoimpu, porcentaje, activo FROM impuestos WHERE tipoimpu = 'ISR';
SELECT numcuenta, nombrecuenta FROM cuentascont WHERE sucursal = '<sucursalActual>';
SELECT cr.numero_cuenta, cc.nombrecuenta, cr.tasa FROM cuentas_retenciones cr INNER JOIN cuentascont cc ON cc.numcuenta = cr.numero_cuenta AND cc.sucursal=cr.sucursal WHERE cr.sucursal = '<sucursalActual>' AND cr.impuesto='IVA';
SELECT cr.numero_cuenta, cc.nombrecuenta, cr.tasa FROM cuentas_retenciones cr INNER JOIN cuentascont cc ON cc.numcuenta = cr.numero_cuenta AND cc.sucursal=cr.sucursal WHERE cr.sucursal = '<sucursalActual>' AND cr.impuesto='ISR';
SELECT clave, descripcion, caracteres, bancorel FROM tiposcuentasbancarias WHERE activo = 1;
SELECT u.empleado, CONCAT(e.nombre, ' ', e.appat, ' ', e.apmat) AS nombre FROM usuarios u INNER JOIN empleados e ON e.empleado=u.empleado WHERE e.activo=1 AND e.puesto='CACO' ORDER BY e.nombre;
```

## 4. Lógica de negocios backend (ServidorCatalogos)

### 4.1 GrabaProveedor
Archivo: `ClassServidorCatalogos.cpp` (líneas ~1475+ en versión analizada). Secuencia exacta (SQL y lógica):

Parámetros en orden recibido (derivado de cliente):
1. `tarea_proveedor` ("A" alta / "M" modificación)
2. `clave_proveedor` (vacío si alta)
3. `plazo` (int)
4. `usuario` (clave usuario)
5. Bloque serializado de campos de `proveedores` (DatosTabla.InsCamposDesdeBuffer) — incluye valores para todos los campos mapeados; posteriormente se fuerza `proveedor` a `@folio`.
6. Indicador teléfonos ("1" si se envían, "0" si no)
7. Si teléfonos = 1: número de teléfonos N seguido de N bloques de campos de `telefonosproveedores` (cada bloque se reescribe `proveedor` a `@folio`).
8. Indicador condiciones comerciales ("1" activa / "0" no).
9. Si condiciones = 1 (modificación): idcondicion, fecha_pacto, fecha_vigencia, empleado, representante, comentarios (texto condición) OR si alta: bloque directo de campos.
10. Para alta: bloque de campos `cuentas_proveedor` (con proveedor=@folio). Para modificación: valores simples (cuenta_iva, cuenta_isr) para construir SQL de update/nulificación.

Construcción:
```
instrucciones[0] = 'SET AUTOCOMMIT=0';
instrucciones[1] = 'START TRANSACTION';
```
Alta (tarea_proveedor == 'A') folio secuencia (SQL literal exacta):
```
select @folioaux:=valor from foliosemp where folio='PROV' AND sucursal = '<Sucursal>' <MODO_BLOQUEO_CLAVES_UNICAS>;
set @foliosig=@folioaux+1;
set @folioaux=cast(@folioaux as char);
set @folio=concat('<Sucursal>', lpad(@folioaux,6,'0'));
update foliosemp set valor=@foliosig where folio='PROV' AND sucursal = '<Sucursal>';
INSERT INTO proveedores (... fechaalta=Today, usualta=usuario ... proveedor=@folio)  -- (insert generado por DatosTabla)
```
Modificación:
```
set @folio='<clave_proveedor>';
UPDATE proveedores SET ... fechamodi=Today, usumodi=usuario WHERE proveedor='<clave_proveedor>';
```
Si cambia el plazo (comparación SELECT plazo): crea tabla temporal y actualiza plazos en `compras`:
```
create temporary table tmpcomprassaldos (compra char(11), saldo decimal(16,2), PRIMARY KEY (compra)) Engine = InnoDB;
insert into tmpcomprassaldos (compra, saldo)
  select c.referencia as compra, sum(t.valor) as saldo
  from compras c, transxpag t
  where c.proveedor='<clave>' and t.referencia=c.referencia and t.cancelada=0 and c.cancelado=0
  group by c.referencia;
UPDATE tmpcomprassaldos au INNER JOIN compras c ON c.referencia = au.compra AND au.saldo > 0
  SET c.plazo = <plazo>, c.periodic = <plazo>, c.fechainic = ADDDATE(c.fechacom, INTERVAL <plazo> DAY), c.fechavenc = ADDDATE(c.fechacom, INTERVAL <plazo> DAY);
```
Teléfonos (si indicador=1): en modificación se ejecuta: `delete from telefonosproveedores where proveedor='<clave>'` seguido de N inserts; en alta solo inserts con proveedor=@folio.

Condiciones comerciales (si indicador=1):
- Alta: bloque DatosTabla (proveedor=@folio) insert directo en `condicionescomerprov`.
- Modificación con idcondicion=0 (nuevo registro):
```
SELECT @folio:=MAX(idcondicion)+1  FROM condicionescomerprov;
insert into condicionescomerprov (proveedor,fecha_alta,fecha_update,fecha_pacto,fecha_vigencia,empleado,representante,condicion_comer)
 values ('<clave>', CURDATE(),CURDATE(),'<fecha_pacto>','<fecha_vigencia>','<empleado>','<representante>','<comentarios>');
```
- Modificación con idcondicion>0 (update):
```
update condicionescomerprov set fecha_update=CURDATE(),fecha_pacto='<fecha_pacto>',fecha_vigencia='<fecha_vigencia>',empleado='<empleado>',representante='<representante>',condicion_comer='<comentarios>' where idcondicion=<idcondicion>;
```

Cuentas de retención:
- Alta: insert a `cuentas_proveedor` (proveedor=@folio) con campos presentes o NULL.
- Modificación: dos updates condicionales (SET cuenta_retencion_iva=... / NULL, cuenta_retencion_isr=... / NULL) según parámetros no vacíos.

Fin transacción: `COMMIT`. Devuelve `select @folio as folio` para que el cliente recupere la clave (en modificación @folio ya se definió a la clave existente).

### 4.2 BajaProveedor
Recibe: clave_proveedor.
Secuencia fija (num_instrucciones = 6):
```
SET AUTOCOMMIT=0;
START TRANSACTION;
delete from telefonosproveedores where proveedor='<clave>';
delete from condicionescomerprov where proveedor='<clave>';
delete from proveedores where proveedor='<clave>';
COMMIT;
```
No hay soft-delete: eliminación física de proveedor y dependencias directas.

### 4.3 ConsultaProveedor
Recibe: clave_proveedor; construye buffer de múltiples result sets en este orden (SQL literal):
1. Datos proveedor (con join a impuestos para porcentaje ISR retenido):
```
select p.*, imp.porcentaje AS isrret from proveedores p left join impuestos imp ON imp.impuesto = p.impuestoret where p.proveedor='<clave>';
```
2. Teléfonos:
```
select tipo, lada, telefono,extencionTel from telefonosproveedores where proveedor='<clave>';
```
3. Bancos catálogo completo:
```
select banco, nombre from bancos order by nombre, banco;
```
4. Cuentas de retención (IVA / ISR) asociadas:
```
SELECT cp.cuenta_retencion_iva, cp.cuenta_retencion_isr FROM cuentas_proveedor cp WHERE cp.proveedor='<clave>';
```
5. Condiciones comerciales ordenadas (vigencia descendente por fecha_pacto, id):
```
SELECT ccp.idcondicion,ccp.proveedor,ccp.fecha_pacto,ccp.fecha_vigencia,
		 CONCAT(em.nombre, ' ', em.appat, ' ', em.apmat) AS empleado,
		 ccp.representante, ccp.condicion_comer , IF(ccp.fecha_vigencia < CURDATE(),1, 0)AS activo,
		 ccp.cancelado,ccp.cumplido
FROM condicionescomerprov ccp
LEFT JOIN empleados em  ON ccp.empleado=em.empleado
where proveedor='<clave>' order by ccp.fecha_pacto DESC, idcondicion DESC;
```

### 4.4 Mapeo resultados -> cliente
El cliente consume cada result set secuencialmente usando un puntero `resultado_select` desplazado por `resp.ObtieneTamRespuesta()`. Para el primer set (`resp_proveedor`) llama `mControlador.CargaDatos(&resp_proveedor, "proveedores")` llenando controles mapeados. Teléfonos llenan grid y combos. Bancos poblan combos de cuentas si se requiere (el código exhibe llenado multiplicado de combos en CargaProveedor). Cuentas de retención se asignan directamente a `ComboBoxCuentaIVA->Clave` y `ComboBoxCuentaISR->Clave`. Condiciones comerciales se trasladan a `StringGridHistorial` fila a fila con las columnas definidas.

## 5. Estructura de datos y relaciones

Tablas principales (archivos en `db/`):

### 5.1 proveedores.sql
Campos (extracto literal de definición – tipos y defaults):
```
proveedor varchar(11) PK, razonsocial varchar(60) NOT NULL, tipoempre varchar(1), replegal varchar(60), titrepleg varchar(10), fechnrep date NOT NULL default '0000-00-00',
rfc varchar(15), curp varchar(18), calle varchar(60), colonia varchar(40), cvecolonia varchar(10), cp varchar(10), localidad varchar(40), estado varchar(4), pais varchar(40),
fechaalta date NOT NULL default '2000-01-01', contacto varchar(60), emailcto varchar(50), fechncon date NOT NULL default '0000-00-00', email varchar(50), limcred decimal(16,2),
plazo int(4), descuento decimal(5,2), descppp decimal(5,2), bancoc1 varchar(10), bancoc2 varchar(10), bancoc3 varchar(10), cuentab1 varchar(20), cuentab2 varchar(20), cuentab3 varchar(20),
tipocuenta1 varchar(5), tipocuenta2 varchar(5), tipocuenta3 varchar(5), cuentadefault int(3), apoyos varchar(60), fechauap date NOT NULL default '0000-00-00', credito tinyint(1),
reduccostobase tinyint(4) NOT NULL default 0, porcreduccosto decimal(5,2) NOT NULL default 0.00, esparterelac tinyint(1) NOT NULL default 0,
cuadreestcomp tinyint(1) NOT NULL default 1, cuadreestncre tinyint(1) NOT NULL default 1, cuadreestpagos tinyint(1) NOT NULL default 1, cuadreestncar tinyint(1) NOT NULL default 1,
activo tinyint(1) NOT NULL default 1, redondeocptecho tinyint(1) default 1, emitencpago tinyint(1) NOT NULL default 1, comprador varchar(10), provgastos tinyint(4) default 0,
provmercancia tinyint(4) default 1, esresico tinyint(4) default 0, impuestoret int(11), numcuenta varchar(30), fechamodi date NOT NULL default '2000-01-01', usumodi varchar(10), usualta varchar(10),
agrupapagfact tinyint(4) default 0, agrupapaggast tinyint(4) default 0, tiporefpago tinyint(4), referenciafija varchar(18), diasvigencia int(10), tiporetencion varchar(15) default 'No configurada',
diasreorden int(5) NOT NULL default 7, capturista varchar(10), mincajas int(4) NOT NULL default 0, minpeso double NOT NULL default 0, mindinero double NOT NULL default 0,
confianzapedidoautomatico tinyint(1) NOT NULL default 0, ajuste_bancario tinyint(1) NOT NULL default 0, cotizable tinyint(1) NOT NULL default 0, correo_cotizacion varchar(100),
PRIMARY KEY (proveedor), CHECK constraint caja_peso_dinero (exclusividad de mínimos), múltiples índices secundarios.
```
Fks:
```
estado -> estados.estado
cvecolonia -> colonias.colonia
impuestoret -> impuestos.impuesto
comprador -> compradores.empleado
capturista / usumodi / usualta -> usuarios.empleado
```

### 5.2 telefonosproveedores.sql
Clave compuesta (proveedor,lada,telefono). Campos: `tipo`, `extencionTel`. FK proveedor->proveedores.proveedor.

### 5.3 condicionescomerprov.sql
Autonumerado `idcondicion` PK. Campos de fechas (alta/update/pacto/vigencia), `empleado`, `representante`, `condicion_comer`, banderas `cancelado`, `cumplido`. FK proveedor.

### 5.4 cuentas_proveedor.sql
PK proveedor. Campos: `cuenta_retencion_iva`, `cuenta_retencion_isr` (FK a `cuentascont.numcuenta`). FK proveedor->proveedores.

Relaciones cardinalidad:
- proveedor 1:N telefonosproveedores
- proveedor 1:N condicionescomerprov
- proveedor 1:1 cuentas_proveedor
- proveedor N:1 estados / colonias / impuestos / compradores / usuarios (capturista / usualta / usumodi)

Integridad reforzada en cliente mediante nulificación condicional para evitar violaciones (ej. comprador nulo si provgastos=1).

## 6. Ejemplos de ejecución

### 6.1 Alta de proveedor (A)
Entrada (parámetros serializados conceptual):
```
"A" | "" | "30" | "USR001" | <buffer campos proveedores (sin proveedor explícito)> | "1" | "2" | <tel1 campos> | <tel2 campos> | "1" | 0 | 2024-10-01 | 2025-09-30 | EMP123 | REP NOMBRE | TEXTO COND | <bloque cuentas_proveedor>
```
SQL generada (orden principal):
```
SET AUTOCOMMIT=0;
START TRANSACTION;
select @folioaux:=valor from foliosemp where folio='PROV' AND sucursal = 'SUC001' <bloqueo>;
set @foliosig=@folioaux+1;
set @folioaux=cast(@folioaux as char);
set @folio=concat('SUC001', lpad(@folioaux,6,'0'));
update foliosemp set valor=@foliosig where folio='PROV' AND sucursal = 'SUC001';
INSERT INTO proveedores (... proveedor=@folio, fechaalta=CURDATE(), usualta='USR001', plazo=30, ...);
INSERT telefonosproveedores (proveedor=@folio, ... ) x2;
INSERT condicionescomerprov (proveedor=@folio, fecha_alta=CURDATE(), fecha_update=CURDATE(), fecha_pacto='2024-10-01', fecha_vigencia='2025-09-30', empleado='EMP123', representante='REP NOMBRE', condicion_comer='TEXTO COND');
INSERT cuentas_proveedor (proveedor=@folio, cuenta_retencion_iva=..., cuenta_retencion_isr=...);
COMMIT;
select @folio as folio;
```
Resultado (primer result set de respuesta al recargar): registro completo de `proveedores` + porcentaje ISR (join impuestos); siguientes sets: teléfonos, bancos catálogo, cuentas retención, condiciones comerciales.

### 6.2 Modificación de proveedor (M) cambiando plazo y teléfono único
Entrada conceptual:
```
"M" | "SUC001000123" | "45" | "USR002" | <buffer campos proveedores con modificaciones> | "1" | "1" | <tel único> | "0" | <cuentaIVA> | <cuentaISR>
```
SQL relevante adicional aparte del UPDATE principal:
```
set @folio='SUC001000123';
UPDATE proveedores SET ..., fechamodi=CURDATE(), usumodi='USR002', plazo=45 WHERE proveedor='SUC001000123';
create temporary table tmpcomprassaldos (...);
insert into tmpcomprassaldos ... (selecciona saldos vigentes);
UPDATE tmpcomprassaldos au INNER JOIN compras c ... SET c.plazo=45, c.periodic=45, c.fechainic=ADDDATE(c.fechacom, INTERVAL 45 DAY), c.fechavenc=...;
delete from telefonosproveedores where proveedor='SUC001000123';
INSERT telefonosproveedores (proveedor=@folio, ...);
update cuentas_proveedor set cuenta_retencion_iva='<cuentaIVA>' ...;
update cuentas_proveedor set cuenta_retencion_isr='<cuentaISR>' ...;
COMMIT;
select @folio as folio;
```

### 6.3 Consulta de proveedor existente
Entrada: clave_proveedor => "SUC001000123".
Secuencia result sets (exactas): ver sección 4.3 (cinco selects). Cliente reconstruye controles y grid historial.

### 6.4 Baja de proveedor
Entrada: clave_proveedor => "SUC001000123".
SQL transaccional:
```
SET AUTOCOMMIT=0;
START TRANSACTION;
delete from telefonosproveedores where proveedor='SUC001000123';
delete from condicionescomerprov where proveedor='SUC001000123';
delete from proveedores where proveedor='SUC001000123';
COMMIT;
```
Resultado: sin result sets de datos adicionales (sólo estado de ejecución). Claves referenciadas en otras tablas pueden impedir la baja (control UI muestra error si MySQL lanza `ER_ROW_IS_REFERENCED`).

---

Fin del documento.
