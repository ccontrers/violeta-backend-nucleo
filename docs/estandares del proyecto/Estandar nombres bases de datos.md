**Estándar de nomenclatura para MariaDB**

La idea es mantener consistencia, claridad semántica y facilidad de
integración entre servicios con orden y legibilidad técnica.

**1. Estructura general de nombres**

**Convención base (snake_case, en español):**

\<dominio\>\_\<subdominio\>\_\<tipo\>

**Ejemplo general:**

facturacion_facturas_db

Donde:

- dominio → nombre del microservicio o módulo del ERP (ventas,
  inventario, rrhh, finanzas, etc.)

- subdominio → parte o componente dentro del dominio (facturas, nómina,
  existencias, etc.)

- tipo → naturaleza del recurso:

  - db→ base de datos principal

  - dw → data warehouse (almacén de datos)

  - test → base de pruebas

**2. Ejemplos por contexto ERP**

  -------------------------------------------------------------------
  **Dominio**      **Nombre sugerido**      **Descripción**
  ---------------- ------------------------ -------------------------
  CRM              crm_nucleo_db            Base principal de CRM

  Contabilidad     contabilidad_nucleo_db   Contabilidad

  Recursos Humanos rrhh_nucleo_db           Nómina y empleados

  Finanzas         finanzas_cuentas_db      Contabilidad general

  Integraciones    integracion_eventos_db   Eventos y registros de
                                            integración

  BI / Data        erp_dw                   Consolidado para análisis
  Warehouse                                 
  -------------------------------------------------------------------

**3. Tablas**

**Convención:**

\<entidad\>\_\<subentidad\>

**Reglas:**

- Siempre en *snake_case*, en singular.

- Sin prefijos de dominio salvo que haya múltiples módulos dentro de la
  mismo BD.

- Nombres descriptivos, evitando abreviaturas confusas.

**Ejemplos:**

cliente

cliente_direccion

factura

factura_detalle

pedido

pedido_detalle

producto

existencia_movimiento

**4. Columnas**

**Convención:**

\<nombre\>\_\<tipo/referencia\>

**Reglas:**

- En *snake_case*.

- Usa id como clave primaria y \<entidad\>\_id como foránea.

- Sufijos estándar:

  - \_id → identificador o clave foránea

  - \_fecha → fechas

  - \_hora → horas

  - \_monto, \_total, \_cantidad, \_flag → valores cuantitativos o
    booleanos

  - \_creado, \_actualizado → para auditoría

**Ejemplos:**

id

cliente_id

creado_fecha

actualizado_fecha

total_monto

activo_flag

**5. Llaves, índices y restricciones**

  -----------------------------------------------------------------
  **Tipo**    **Convención**                **Ejemplo**
  ----------- ----------------------------- -----------------------
  Clave       pk\_\<tabla\>                 pk_factura
  primaria                                  

  Clave       fk\_\<tabla\>\_\<columna\>    fk_factura_cliente_id
  foránea                                   

  Índice      idx\_\<tabla\>\_\<columna\>   idx_cliente_correo

  Único       uk\_\<tabla\>\_\<columna\>    uk_usuario_nombre
  -----------------------------------------------------------------

**6. Vistas, triggers y procedimientos**

  ------------------------------------------------------------------------------------
  **Tipo**              **Convención**                  **Ejemplo**
  --------------------- ------------------------------- ------------------------------
  Vista                 vw\_\<tabla\>\_\<proposito\>    vw_factura_resumen

  Trigger               trg\_\<tabla\>\_\<evento\>      trg_factura_despues_insertar

  Procedimiento         sp\_\<accion\>\_\<objeto\>      sp_actualizar_existencias
  almacenado                                            

  Función               fn\_\<resultado\>\_\<objeto\>   fn_total_factura_monto
  ------------------------------------------------------------------------------------

**7. Ambientes y prefijos por entorno**

Para diferenciar ambientes:

  -----------------------------------------------------
  **Ambiente**         **Prefijo**   **Ejemplo**
  -------------------- ------------- ------------------
  Desarrollo/pruebas   YYYYMMDD \_   20251027_rrhh_db

  Producción           *(sin         rrhh_db
                       sufijo)*      
  -----------------------------------------------------

**8. Buenas prácticas para microservicios**

1.  **Una base de datos por microservicio** (no compartida).

2.  Cada servicio **es dueño de su modelo de datos**.

3.  Evita *joins* entre bases de diferentes servicios.

4.  Para analítica o BI, crea bases derivadas o réplicas:

5.  facturacion_facturas_raw

6.  facturacion_facturas_dw

7.  Evita nombres genéricos o ambiguos (app_db, main_db).

**9. Ejemplo completo ERP**

  -------------------------------------------------------------------------
  **Microservicio**   **Base de datos**        **Tablas principales**
  ------------------- ------------------------ ----------------------------
  Facturación         contabilidad_nucleo_db   Polizas, balances, etc.

  Finanzas            crm_nucleo_db            Administración de relación
                                               con los clientes

  RRHH                rrhh_nucleo_db           empleado, nomina, asistencia

  BI                  erp_dw                   vw_resumen_ventas,
                                               vw_valor_inventario

  Monolito            multiempresa             Ventas, dventas
  -------------------------------------------------------------------------
