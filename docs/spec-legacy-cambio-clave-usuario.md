# Especificacion tecnica: modulo legado de cambio de clave de usuario

## 1. Introduccion tecnica general
- **Proposito**: permitir que un usuario del sistema legado VCL actualice su clave de acceso o reciba una primera asignacion de clave cuando el registro en `usuarios.password` sigue en blanco (`NULL`). El formulario `cpp/FormSistemaCambiarClave.*` encapsula la interaccion de UI y delega toda la persistencia al servidor `ServidorAdminSistema` vía `ClienteVioleta`.
- **Arquitectura funcional**:
  - **Interfaz**: formulario VCL `TFormSisCambiarClave` con tres `VTLabeledEdit` de captura y dos `VTBitBtn` para confirmar o cancelar.
  - **Capa logica cliente**: metodos `Cambiar` y `AsignarPassword` aplican validaciones locales, derivan los hashes SHA-256 y serializan los parametros hacia el servidor usando `ClienteVioleta`.
  - **Capa servidor**: `ServidorAdminSistema::CambiaClave` y `ServidorAdminSistema::AsignaPassword` arman lotes SQL y los ejecutan sobre MySQL mediante `ServidorVioleta`.
  - **Capa de datos**: tablas `db/usuarios.sql`, `db/usuariosucursal.sql` y `db/sucursales.sql` (ademas de la FK implicita a `db/empleados.sql`) almacenan credenciales y relaciones con sucursales.
- **Flujo de ejecucion**:
```
[Usuario] -> [TFormSisCambiarClave UI] -> [gClienteVioleta peticion ID_GRA_CAMBIACLAVE o ID_ASIG_PASSWORD]
        -> [ServidorAdminSistema::{CambiaClave|AsignaPassword}] -> [MySQL usuarios (+ usuariosucursal/sucursales)]
        -> [BufferRespuestas usuario] -> [Mensajes VCL / cierre del formulario]
```

## 2. Estructura de la interfaz (FormSistemaCambiarClave)
### 2.1 Componentes declarados en `cpp/FormSistemaCambiarClave.dfm`
| Componente | Tipo | Propiedades relevantes | Rol |
|------------|------|------------------------|-----|
| `Label1` | `TLabel` | `WordWrap=True`, texto dinamico segun `CLAVESSEGURAS` | Instrucciones de formato de clave. |
| `EditPasswordAnterior` | `VTLabeledEdit` | `MaxLength=13`, `PasswordChar='*'`, etiqueta "Clave anterior" | Captura de la clave actual. Se oculta cuando se asigna primera clave. |
| `EditPassword` | `VTLabeledEdit` | `MaxLength=13`, etiqueta "Clave de acceso nueva" | Captura de la nueva clave (texto oculto). |
| `EditPasswordRepetida` | `VTLabeledEdit` | `MaxLength=13`, etiqueta "Reescriba la clave de acceso nueva" | Confirmacion de la nueva clave. |
| `ButtonCambiar` | `VTBitBtn` | `Caption="Ca&mbiar"`, `OnClick=ButtonCambiarClick` | Dispara `Cambiar` o `AsignarPassword`. |
| `ButtonCancelar` | `VTBitBtn` | `Caption="C&ancelar"`, `OnClick=ButtonCancelarClick` | Cierra la ventana sin cambios. |

### 2.2 Eventos y funciones asociadas
- `ButtonCambiarClick`: evalua `mAsignaPassword`. Si es `true`, llama `AsignarPassword(mUsuario, EditPassword->Text, EditPasswordRepetida->Text)`; de lo contrario invoca `Cambiar(mUsuario, EditPasswordAnterior->Text, ...)`. Al recibir `true`, ejecuta `Close()`.
- `ButtonCancelarClick`: llama `Close()` sin efectos adicionales.
- `FormShow`: limpia las tres cajas (`Text=""`) y alterna la visibilidad de `EditPasswordAnterior` segun `mAsignaPassword`.
- `FormPaint`: invoca `gClienteVioleta->GradienteForma(this)` para aplicar el gradiente corporativo.

### 2.3 Estado interno del formulario (`FormSistemaCambiarClave.h`)
| Variable | Tipo | Uso actual |
|----------|------|------------|
| `FuncionesGenericas mFg` | Helper sin estado | Validaciones (`ValidaPassword`), mensajes (`AppMessageBox`), manejo de buffers (`ExtraeStringDeBuffer`). |
| `AnsiString mUsuario` | Identificador de empleado (PK `usuarios.empleado`). Setters `EspecificarUsuario`/`AsignarUsuario`. |
| `bool mForzarClaveSegura` | Se carga en el constructor evaluando `gClienteVioleta->Param->Valor("CLAVESSEGURAS")`. Controla mensaje de `Label1` y reglas de longitud/tipo. |
| `bool mAsignaPassword` | Indica si la interfaz fue abierta para asignacion inicial (sin validar clave anterior). Configurable vía `RequiereAsignarPassword(bool)`. |

### 2.4 Otras dependencias cliente
- `Color` de la forma se toma de `gClienteVioleta->Param->Valor("COLORVENTANAS")` (cadena decimal convertida a `TColor`).
- `SHAGenerator(AnsiString Password)` es un wrapper sobre `System::Hash::THashSHA2::GetHashString`, que produce 64 caracteres hexadecimales (SHA-256).

## 3. Logica de negocio
### 3.1 Flujo `Cambiar`
1. **Validacion local**:
   - Si `mForzarClaveSegura` es `true`, exige `Password.Length() > 7` y `mFg.ValidaPassword(Password)` (requiere al menos una mayuscula, una minuscula, un numero, y caracteres alfanumericos). Caso contrario solo verifica longitud > 3.
   - Verifica coincidencia entre `Password` y `PasswordRepetida`.
   - Mensajes de error se muestran con `mFg.AppMessageBox` y se reposiciona el foco (`SetFocus`) en el campo correspondiente.
2. **Hash SHA-256**: `SHAGenerator` aplica SHA-256 a la clave nueva y a `PasswordAnterior`, lo que asegura que el backend nunca recibe texto plano.
3. **Construccion de peticion** (orden exacto de parametros en el buffer de `ClienteVioleta`):
   1. `Usuario` (`mUsuario`).
   2. `PasswordAnterior` (hash SHA-256 en hex).
   3. `Password` (nuevo hash SHA-256).
4. **Envio**: se inicializa la peticion con `ID_GRA_CAMBIACLAVE`, se agregan los `AnsiString` y se llama `gClienteVioleta->EjecutaPeticionActual(resultado_peticion)`.
5. **Procesamiento de respuesta**:
   - Se descarta el entero inicial (tamaño) y el indicador de error usando `mFg.ExtraeStringDeBuffer`.
   - `BufferRespuestas resp_clave(resultado_select)` interpreta la seccion de registros.
   - Se lee el campo `usuario`; si coincide con `mUsuario`, la operacion se considera exitosa y se muestra "Clave asignada correctamente".
   - Cualquier mismatch se trata como error de autenticacion y se retorna `false`.

### 3.2 Flujo `AsignarPassword`
- Aplica el mismo bloque de validacion (llegando al `flag`); solo difiere en que no solicita ni envia `PasswordAnterior` y utiliza el identificador de peticion `ID_ASIG_PASSWORD`.
- Parametros enviados:
  1. `Usuario` (`mUsuario`).
  2. `Password` (hash SHA-256).
- Se interpreta la respuesta exactamente igual que en `Cambiar`.
- En caso de exito y `mAsignaPassword==true`, el formulario se cierra y se notifica "Clave asignada correctamente".

### 3.3 Interaccion con objetos soporte
- `ClienteVioleta` (documentado en `docs/claseslegadas/ClassClienteVioleta.md`): gestiona slots, serializa los parametros y entrega un `char*` con la respuesta del servidor. Cualquier fallo de transporte se expone con `gClienteVioleta->ObtieneErrorMsg()`.
- `BufferRespuestas` (ver `docs/claseslegadas/ClassBufferRespuestas.md`): parsea el bloque devuelto por MySQL y expone los campos por nombre; en este flujo solo se consume `usuario`.

### 3.4 Servidor `ServidorAdminSistema::CambiaClave` (`cpp/ClassServidorAdminSistema.cpp`, lineas 1606-1646)
1. **Deserializacion**: `mFg.ExtraeStringDeBuffer` obtiene `usuario`, `clave` (actual) y `nueva_clave` siguiendo el mismo orden enviado por el cliente.
2. **Preparacion de batch SQL**:
   - `select @idusuario:=empleado from usuarios where empleado='{usuario}' and password='{clave}'`.
   - `update usuarios set password='{nueva_clave}' where empleado='{usuario}' and password='{clave}'`.
   Las cadenas se escriben directamente, por lo que los valores ya deben venir filtrados/hasheados.
3. **Ejecucion**: el batch se manda a `mServidorVioleta->EjecutaBufferAccionesSql`. Si ambas instrucciones ejecutan sin error, se lanza un `SELECT @idusuario as usuario` para armar el `BufferRespuestas` enviado al cliente.
4. **Condicion de exito**: el cliente solo considera valido cuando `@idusuario` coincide con el identificador enviado.

### 3.5 Servidor `ServidorAdminSistema::AsignaPassword` (`cpp/ClassServidorAdminSistema.cpp`, lineas 10096-10125)
1. **Parametros**: `usuario`, `clave` (nueva, ya cifrada).
2. **Batch SQL**:
   - `select @idusuario:=empleado from usuarios where empleado='{usuario}' AND password IS NULL`.
   - `update usuarios set password='{clave}' where empleado='{usuario}' AND password IS NULL`.
   Esto evita reescrituras accidentales cuando el usuario ya cuenta con una clave.
3. **Respuesta**: mismo `SELECT @idusuario as usuario` final.

### 3.6 Dependencias y funciones auxiliares
- `FuncionesGenericas::ValidaPassword`: valida estructura (longitud 8-13, alfanumerico, mayuscula, minuscula, numero) cuando `CLAVESSEGURAS=1`.
- `FuncionesGenericas::AppMessageBox`: encapsula `Application->MessageBox` para mensajes de error/confirmacion.
- `System::Hash::THashSHA2::GetHashString`: devuelve el digest SHA-256 en hex (ejemplo: `THashSHA2::GetHashString("ClaveAnterior1")` -> `397EB3C1E7915CF2F5E373ED6E2BF8C7CB3ADEA447C1F4D0554F90AB0C9CBABD`).

## 4. Estructura de datos y relaciones
### 4.1 `db/usuarios.sql`
| Campo | Tipo | Restricciones |
|-------|------|---------------|
| `empleado` | `varchar(10)` | PK. FK a `empleados.empleado`. |
| `password` | `varchar(64)` | SHA-256 hex (64 chars). Puede ser `NULL` para usuarios por activar. |
| `activo` | `tinyint(1)` | Bandera de habilitacion. |
| `fechaalta` / `fechabaja` | `date` | Alta/baja administrativa. |
| `usuariocontpaq` / `passwordcontpaq` | `varchar(64)` | Credenciales cifradas via `AES_DECRYPT` cuando aplica. |

### 4.2 `db/usuariosucursal.sql`
| Campo | Tipo | Restricciones |
|-------|------|---------------|
| `usuario` | `varchar(10)` | FK a `usuarios.empleado`. |
| `sucursal` | `varchar(2)` | FK a `sucursales.sucursal`. |
| PK compuesta (`usuario`,`sucursal`). |

### 4.3 `db/sucursales.sql`
| Campo | Tipo | Descripcion |
|-------|------|-------------|
| `sucursal` | `varchar(2)` | PK de sucursal fisica. |
| `numid` | `int(2)` | Codigo interno unico. |
| `nombre` | `varchar(40)` | Nombre comercial mostrado en UI. |
| `activa`, `venxvol`, `salidaotrasucursal`, `vtasecom`, `defaultecom`, `pickup` | Banderas de configuracion. |
| Datos de contacto (`calle`, `colonia`, `cp`, `localidad`, `telefono1-4`, `email`). |
| `idempresa` | FK a `empresas.idempresa`. |
| `diasdistribucion` | Programacion logistica. |

### 4.4 Relacion con la interfaz
- `mUsuario` representa `usuarios.empleado`. El formulario no permite modificarlo; viene prefijado por la capa que abre la ventana (típicamente el proceso de login o un formulario de administracion).
- El exito de la operacion depende exclusivamente de la coincidencia de `usuario` en la respuesta; `BufferRespuestas` no expone sucursales ni otros campos, pero la relacion con `usuariosucursal` asegura que el usuario pertenezca a la sucursal activa (la validacion ocurre antes, al abrir el formulario).

## 5. Consideraciones de seguridad y ejemplos
### 5.1 Validaciones locales
- Reglas "clave segura" controladas por `CLAVESSEGURAS`:
  - `1`: `ValidaPassword` exige 8-13 caracteres, al menos 1 mayuscula, 1 minuscula y 1 digito. Mensaje detallado en `Label1`.
  - `0`: se permite longitud de 4 a 13 caracteres (solo validacion de tamaño).
- Antes de enviar los datos, ambas funciones comparan `Password` vs `PasswordRepetida` para prevenir discrepancias.

### 5.2 Cifrado y transporte
- Tanto la clave anterior como la nueva se convierten a SHA-256 en el cliente. MySQL almacena exactamente el digest recibido.
- `ID_GRA_CAMBIACLAVE` y `ID_ASIG_PASSWORD` viajan sobre el canal activo de `ClienteVioleta` (TCP propietario, HTTP/REST o CORBA). No hay doble cifrado; la proteccion depende del canal configurado.
- Las llamadas al servidor no concatenan comillas escapadas; por lo tanto la entrada debe llegar ya sanitizada (la UI restringe a caracteres alfanumericos y la longitud fija evita inyecciones).

### 5.3 Manejo de resultados
- El servidor siempre responde con un dataset de un solo registro que contiene el campo `usuario`. `BufferRespuestas` mantiene el formato tabular interno; el formulario no lo muestra en UI, solo compara el valor.
- Representacion equivalente en JSON para una operacion exitosa:
```json
{
  "usuario": "MX00123"
}
```
- Cuando el hash anterior no coincide, `@idusuario` se queda `NULL`. En ese caso el dataset llega vacio y el cliente muestra: `"No se logro cambiar la clave,\n Verifique su clave anterior y reintente."`.

### 5.4 Ejemplos de consultas SQL
- **Cambio de clave (usuario `MX00123`)**:
```sql
select @idusuario:=empleado from usuarios where empleado='MX00123' and password='9F86D081884C7D659A2FEAA0C55AD015A3BF4F1B2B0B822CD15D6C15B0F00A08';
update usuarios set password='3C5636D872B042199E6A04A0E1E77AA15641244B7C5A826C71DA5DCE9D765A61' where empleado='MX00123' and password='9F86D081884C7D659A2FEAA0C55AD015A3BF4F1B2B0B822CD15D6C15B0F00A08';
select @idusuario as usuario;
```
- **Asignacion inicial (clave nula)**:
```sql
select @idusuario:=empleado from usuarios where empleado='MX00123' AND password IS NULL;
update usuarios set password='3C5636D872B042199E6A04A0E1E77AA15641244B7C5A826C71DA5DCE9D765A61' where empleado='MX00123' AND password IS NULL;
select @idusuario as usuario;
```

### 5.5 Registro presentado al usuario
- La interfaz no muestra grids; la evidencia de exito es el mensaje modal. Aun asi, para trazabilidad se puede derivar un recordset logico:
| Campo | Valor | Fuente |
|-------|-------|--------|
| `usuario` | `MX00123` | `SELECT @idusuario as usuario` |

Este mismo registro puede serializarse a JSON (ver seccion 5.3) si se requiere replicar el resultado en integraciones externas.

---
Este documento describe el estado actual del modulo legado. No se incluyen propuestas de mejora ni cambios futuros.
