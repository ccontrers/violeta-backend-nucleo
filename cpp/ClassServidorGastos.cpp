#include <vcl.h>
//---------------------------------------------------------------------------
#include "pch.h"

#pragma hdrstop

#include <DateUtils.hpp>
#include "ClassServidorGastos.h"
#include "ClassServidorVioleta.h"
#include "ClassBufferRespuestas.h"
#include "violetaS.h"
#include "FormServidorVioleta.h"
#include "comunes.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)

//ID_GRA_GASTO
void ServidorGastos::GrabaGasto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  GRABA GASTO

	char *buffer_sql=new char[3000*1000];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString tarea_gasto, clave_gasto, usuario, terminal, sucursal;
	int num_partidas, i;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[3000];
	double anticipo, valor;
	AnsiString periodic;
	int dias_plazo;
	TDateTime fecha_inic, fecha_venc, fecha_gas;
	int acredito;
	int error=0;
	AnsiString articulo, costo_art;
	TDate fecha=Today();
	TTime hora=Time();
	AnsiString tipobitacoracosto;
	AnsiString mensaje;

	// Consulta de parámetro para asignación de precios
	// * Si es 0 el cambio de precio es inmediato y se guarda en el campo precios.precio
	// * si es 1 entonces el cambio de precios es diferido y se guarda precios.precioproximo para aplicarse posteriormente)
	AnsiString paramcambioprec;
	BufferRespuestas* resp_paramcambioprec=NULL;
	sucursal=FormServidor->ObtieneClaveSucursal();
	try {
		instruccion.sprintf("SELECT valor FROM parametrosglobemp WHERE parametro='CAMBIOPRECDIFER' AND idempresa = %s ", FormServidor->ObtieneClaveEmpresa());
		if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_paramcambioprec)) {
			if (resp_paramcambioprec->ObtieneNumRegistros()>0){
				paramcambioprec=resp_paramcambioprec->ObtieneDato("valor");
			} else throw (Exception("No se encuentra registro CAMBIOPRECDIFER en tabla parametrosglobemp"));
		} else throw (Exception("Error al consultar en tabla parametros"));
	} __finally {
		if (resp_paramcambioprec!=NULL) delete resp_paramcambioprec;
	}

	try {
		clave_gasto=mFg.ExtraeStringDeBuffer(&parametros); // Clave de la gasto.
		tarea_gasto=mFg.ExtraeStringDeBuffer(&parametros); // Indicador si se va a agregar o modificar.
		usuario=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que esta grabando la gasto.
		terminal=mFg.ExtraeStringDeBuffer(&parametros); // Terminal desde la que se está grabando la gasto.
		mensaje=mFg.ExtraeStringDeBuffer(&parametros); // Mensaje de la gasto.


		// Obtiene los datos de la tabla de gastos
		datos.AsignaTabla("gastos");
		parametros+=datos.InsCamposDesdeBuffer(parametros);
		// Extrae los datos que necesitamos para crear las letras y transacciones.
		datos.AsignaValorCampo("referencia", "@folio", 1);
		valor=StrToFloat(datos.ObtieneValorCampo("valor"));
		periodic=datos.ObtieneValorCampo("periodic");
		anticipo=StrToFloat(datos.ObtieneValorCampo("anticipo"));
		acredito=StrToInt(datos.ObtieneValorCampo("acredito"));
		fecha_gas=StrToDate(datos.ObtieneValorCampo("fechagas"));

		// Verificaciones para no permitir modificaciones cuando no sea permitido.
		if (tarea_gasto=="M") {

			// Verifica que la fecha de la factura sea la fecha de gasto previamente registrada.
			instruccion.sprintf("select @error:=if(c.fechagas<=cast(e.valor as datetime) , 1, 0) as error from gastos c left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' where c.referencia='%s' ",FormServidor->ObtieneClaveSucursal(), clave_gasto );
			mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 6, error);

			// Verifica que no tenga pagos (sin tomar en cuenta el anticipo ni los pagos no cobrables)  ---------
			instruccion.sprintf("select @error:=if(COALESCE(sum(t.valor),0)<0, 1, 0) as error from gastos c, transxpaggastos t where c.referencia=t.referencia and c.referencia='%s' and c.cancelado=0 and t.cancelada=0 and t.valor<0 and t.tipo<>'ANTI' group by c.referencia", clave_gasto);
			mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 2, error);

			//verificara si la gasto ya tiene el UUID cargado y si es asi no permite guardar la gasto
			instruccion.sprintf("SELECT @error:=IF(muuid<>'',1,0) AS error FROM gastos c WHERE c.referencia='%s' AND c.cancelado=0 GROUP BY c.referencia", clave_gasto);
			mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 5, error);

			// Verifica que la fecha gasto de la factura sea posterior a la fecha de cierre
			instruccion.sprintf("select @error:=if('%s'<=cast(e.valor as datetime) , 1, 0) as error from gastos c left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' where c.referencia='%s'", mFg.DateToMySqlDate(fecha_gas),FormServidor->ObtieneClaveSucursal(), clave_gasto);
			mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 3, error);

			tipobitacoracosto = "MC";

		} else {
			// Verifica que la fecha de la factura sea posterior a la fecha de cierre.
			instruccion.sprintf("select @error:=if('%s'<=cast(e.valor as datetime), 1, 0) as error from estadosistemaemp e where e.estado='FUCIERRE' AND e.sucursal = '%s' ", mFg.DateToMySqlDate(fecha_gas), FormServidor->ObtieneClaveSucursal());
			mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 4, error);
			tipobitacoracosto = "C";
		}

		if (error==0) {
			if (periodic=="MES") throw(Exception("Todavía no se implementa la periodicidad mensual en gastos"));
			if (periodic=="QUI") throw(Exception("Todavía no se implementa la periodicidad quincenal en gastos"));
			if (periodic=="SEM") throw(Exception("Todavía no se implementa la periodicidad semanal en gastos"));
			dias_plazo=StrToInt(periodic);

			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";


			// Obtiene el folio para la gasto
			if (tarea_gasto=="A") {
				instruccion.sprintf("select @folioaux:=valor from foliosemp where folio='GASTOS' AND sucursal = '%s' %s ",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliosig=@folioaux+1");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @folioaux=cast(@folioaux as char)");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @folio=concat('%s', lpad(@folioaux,9,'0'))", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("update foliosemp set valor=@foliosig where folio='GASTOS' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
			} else {
				instruccion.sprintf("set @folio='%s'", clave_gasto);
				instrucciones[num_instrucciones++]=instruccion;
			}

			// Todo se va a una letra que se salda automáticamente en el caso de las
			// gastos de contado
			if (!acredito) {
				anticipo=valor;
				dias_plazo=0;
			}

			fecha_venc=fecha_gas+dias_plazo;
			fecha_inic=fecha_venc;
			// Graba la cabecera en la tabla "gastos"
			if (tarea_gasto=="A") {
				instruccion.sprintf("select @seccion:=seccion, @depart:=depart from terminales where terminal='%s'", terminal);
				instrucciones[num_instrucciones++]=instruccion;

				datos.InsCampo("usualta", usuario);
				datos.InsCampo("usumodi", usuario);
				datos.InsCampo("fechaalta", mFg.DateToAnsiString(fecha));
				datos.InsCampo("horaalta", mFg.TimeToAnsiString(hora));
				datos.InsCampo("fechamodi", mFg.DateToAnsiString(fecha));
				datos.InsCampo("horamodi", mFg.TimeToAnsiString(hora));
				datos.InsCampo("fechainic", mFg.DateToAnsiString(fecha_inic));
				datos.InsCampo("fechavenc", mFg.DateToAnsiString(fecha_venc));
				datos.InsCampo("cancelado", "0");
				datos.InsCampo("terminal", terminal);
				datos.InsCampo("docseccion", "@seccion",1);
				datos.InsCampo("docdepart", "@depart",1);
				datos.InsCampo("plazo", mFg.IntToAnsiString(dias_plazo));


				instrucciones[num_instrucciones++]=datos.GenerarSqlInsert();

                	// Guarda el mensaje
				if (mensaje!=" ") { // Un espacio lo tomamos como nulo.
					instruccion.sprintf("insert into gastosmensajes (referencia, mensaje) values (@folio,'%s')",
						mensaje);
					instrucciones[num_instrucciones++]=instruccion;
				}

			} else {
				datos.InsCampo("usumodi", usuario);
				datos.InsCampo("fechamodi", mFg.DateToAnsiString(fecha) );
				datos.InsCampo("horamodi", mFg.TimeToAnsiString(hora) );
				datos.InsCampo("fechainic", mFg.DateToAnsiString(fecha_inic));
				datos.InsCampo("fechavenc", mFg.DateToAnsiString(fecha_venc));
				datos.InsCampo("plazo", mFg.IntToAnsiString(dias_plazo));
				instrucciones[num_instrucciones++]=datos.GenerarSqlUpdate("referencia=@folio");

				// Guarda el mensaje
				if (mensaje!=" ") { // Un espacio lo tomamos como nulo.
					instruccion.sprintf("replace into gastosmensajes (referencia, mensaje) values (@folio,'%s')",
						mensaje);
				} else {
					instruccion.sprintf("delete from gastosmensajes where referencia=@folio");
				}
				instrucciones[num_instrucciones++]=instruccion;
			}


			// Graba el cargo por la gasto en transxpaggastos
			if (tarea_gasto=="A") {
					// Obtiene el folio para la NUEVA transaccion
					instruccion.sprintf("select @foliotranaux:=valor from foliosemp where folio='TRANPROV' AND sucursal = '%s' %s ", FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
					instrucciones[num_instrucciones++]=instruccion;
					instruccion.sprintf("set @foliosigtran=@foliotranaux+1");
					instrucciones[num_instrucciones++]=instruccion;
					instruccion.sprintf("set @foliotranaux=cast(@foliotranaux as char)");
					instrucciones[num_instrucciones++]=instruccion;
					instruccion.sprintf("set @foliotran=concat('%s', lpad(@foliotranaux,9,'0'))", FormServidor->ObtieneClaveSucursal());
					instrucciones[num_instrucciones++]=instruccion;
					instruccion.sprintf("update foliosemp set valor=@foliosigtran where folio='TRANPROV' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
					instrucciones[num_instrucciones++]=instruccion;
					instruccion.sprintf("insert into transxpaggastos \
						(tracredito, referencia, concepto, destino, tipo, cancelada, aplicada, valor, fechaalta, horaalta, fechamodi, horamodi, fechaapl, usualta, usumodi) \
						values (@foliotran, @folio, 'C', 'C', 'GAST', 0,1, %12.2f, '%s', '%s', '%s', '%s', '%s', '%s', '%s')",
						valor, mFg.DateToMySqlDate(fecha_gas), mFg.TimeToMySqlTime(hora), mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), mFg.DateToMySqlDate(fecha_gas), usuario, usuario );
					instrucciones[num_instrucciones++]=instruccion;
			} else {
					// Obtiene el folio de la transaccion ya existente
					instruccion.sprintf("select @foliotran:=tracredito from transxpaggastos where referencia=@folio and concepto='C' and destino='C' and tipo='COMP' and cancelada=0");
					instrucciones[num_instrucciones++]=instruccion;
					instruccion.sprintf("update transxpaggastos set valor=%12.2f, fechamodi='%s', horamodi='%s', fechaapl='%s', usumodi='%s', cancelada=0 where tracredito=@foliotran", valor, mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), mFg.DateToMySqlDate(fecha_gas), usuario);
					instrucciones[num_instrucciones++]=instruccion;
			}

			instruccion="create temporary table tmpcostos ( \
				articulo varchar(9), producto varchar(8), present varchar(255), \
				costo decimal(16,6), modificar bool, PRIMARY KEY (articulo)) Engine = InnoDB";
			instrucciones[num_instrucciones++]=instruccion;

			// Si se está modificando entonces borra el detalle y las letras que ya existan.
			if (tarea_gasto=="M") {

				instruccion.sprintf("delete from dgastos where referencia=@folio");
				instrucciones[num_instrucciones++]=instruccion;
			}

			// Graba las partidas en "dgastos" y cambia los costos y los precios de cada artículo
			// involucrado.
			AnsiString lista_articulos = "";
			BufferRespuestas* resp_arteliminado=NULL;
			num_partidas=StrToInt(mFg.ExtraeStringDeBuffer(&parametros)); // Obtiene el número de partidas

			for (i=0; i<num_partidas; i++) {

				datos.AsignaTabla("dgastos");
				parametros+=datos.InsCamposDesdeBuffer(parametros);
				datos.InsCampo("referencia", "@folio", 1);

				instrucciones[num_instrucciones++]=datos.GenerarSqlInsert();

			}

			instrucciones[num_instrucciones++]="COMMIT";
		}

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			instruccion.sprintf("select %d as error, @folio as folio", error);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	} __finally {
		delete buffer_sql;
	}
}

//------------------------------------------------------------------------------
//ID_CANC_GASTO
void ServidorGastos::CancelaGasto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  CANCELA GASTO
	char *buffer_sql=new char[1024*32];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instrucciones[50], instruccion;
	int num_instrucciones=0;
	AnsiString clave, usuario;
	int error=0;
	int i;
	AnsiString modcosto;
	TDate fecha=Today();
	TTime hora=Time();
	BufferRespuestas* resp_costo=NULL;

	// Consulta de parámetro para asignación de precios
	// * Si es 0 el cambio de precio es inmediato y se guarda en el campo precios.precio
	// * si es 1 entonces el cambio de precios es diferido y se guarda precios.precioproximo para aplicarse posteriormente)
	AnsiString paramcambioprec;
	BufferRespuestas* resp_paramcambioprec=NULL;
	try {
		instruccion.sprintf("SELECT valor FROM parametrosglobemp WHERE parametro='CAMBIOPRECDIFER' AND idempresa = %s ", FormServidor->ObtieneClaveEmpresa());
		if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_paramcambioprec)) {
			if (resp_paramcambioprec->ObtieneNumRegistros()>0){
				paramcambioprec=resp_paramcambioprec->ObtieneDato("valor");
			} else throw (Exception("No se encuentra registro CAMBIOPRECDIFER en tabla parametrosglobemp"));
		} else throw (Exception("Error al consultar en tabla parametros"));
	} __finally {
		if (resp_paramcambioprec!=NULL) delete resp_paramcambioprec;
	}


	try {
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave de la gasto (referencia)
		usuario=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que esta grabando la gasto.

		// Verifica que no tenga pagos (sin tomar en cuenta el anticipo ni los cheques no cobrables)  ---------
		instruccion.sprintf("select @error:=if(COALESCE(sum(t.valor),0)<0, 1, 0) as error from gastos g, transxpaggastos t where g.referencia=t.referencia and g.referencia='%s' and g.cancelado=0 and t.cancelada=0 and t.valor<0 and t.tipo<>'ANTI' group by G.referencia", clave);
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 2, error);

		// Verifica que la fecha de la factura sea posterior a la fecha de cierre.
		instruccion.sprintf("select @error:=if(g.fechagas<=cast(e.valor as datetime), 1, 0) as error from gastos g left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' where g.referencia='%s'",FormServidor->ObtieneClaveSucursal(), clave);
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 3, error);

		if (error==0) {
			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

			// Cancela la gasto
			instruccion.sprintf("update gastos set cancelado=1, fechamodi='%s', horamodi='%s', usumodi='%s' where referencia='%s' and cancelado=0", mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario, clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Cancela el cargo hecho por el total de la gasto
			instruccion.sprintf("update transxpaggastos set cancelada=1, fechamodi='%s', horamodi='%s', usumodi='%s' where referencia='%s' and concepto='C' and destino='C' and tipo='GAST' and cancelada=0", mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario, clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Cancela el abono hecho por el ANTICIPO
			instruccion.sprintf("update transxpaggastos set cancelada=1, fechamodi='%s', horamodi='%s', usumodi='%s' where referencia='%s' and concepto='A' and destino='C' and tipo='ANTI' and cancelada=0", mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario, clave);
			instrucciones[num_instrucciones++]=instruccion;

			instrucciones[num_instrucciones++]="COMMIT";
		}

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			instruccion.sprintf("select %d as error", error);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	} __finally {
		delete buffer_sql;
	}
}
//------------------------------------------------------------------------------
//ID_CON_GASTO
void ServidorGastos::ConsultaGasto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA GASTOS
	AnsiString instruccion;
	AnsiString clave, menos_devoluciones,orden;

	clave=mFg.ExtraeStringDeBuffer(&parametros);
	orden=mFg.ExtraeStringDeBuffer(&parametros);


    mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

    // Obtiene todos los generales (de cabecera) de la gasto
	instruccion.sprintf("SELECT c.*, cm.mensaje FROM gastos c INNER JOIN terminales ter \
						ON c.terminal=ter.terminal INNER JOIN secciones sec ON sec.seccion=ter.seccion  \
                        left join gastosmensajes cm on cm.referencia=c.referencia \
						where c.referencia='%s'", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

    // Calcula el saldo de la gasto (SALDO B)
	instruccion.sprintf("select sum(valor) as saldo from transxpaggastos where referencia='%s' and cancelada=0 group by referencia", clave);
    mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene todos los datos del proveedor de la gasto
    instruccion.sprintf("select p.* from proveedores p, gastos c where c.referencia='%s' and c.proveedor=p.proveedor", clave);
    mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

    // NULO:  Verifica que la fecha de la factura sea posterior a la fecha de cierre.
	instruccion.sprintf("select if(c.fechagas>=cast(e.valor as datetime), 1, 0) as modificar from gastos c left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' where c.referencia='%s'",FormServidor->ObtieneClaveSucursal(), clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene los datos de la gasto completa (sin importar las devoluciones)
	instruccion="SELECT d.referencia,d.idorden,d.claveprodserv,d.cantidad,d.unidad, ";
	instruccion+="d.descripcion,d.valorunitario,d.descuento,d.isr_retenido,d.tasa_isr_retenido, ";
	instruccion+="ifnull(d.iva_trasladado,0)+ifnull(d.ieps_trasladado,0) as Imp_Trasladados, ";
	instruccion+="ifnull(d.isr_retenido,0)+ifnull(d.iva_retenido,0)+ifnull(d.ieps_retenido,0) as Imp_Retenidos, ";
	instruccion+="d.iva_retenido,d.tasa_iva_retenido,d.ieps_retenido,d.tasa_ieps_retenido,d.iva_trasladado, ";
	instruccion+="d.tasa_iva_trasladado,d.ieps_trasladado,d.tasa_ieps_trasladado,d.costoimp,d.costo,d.viaembarq ";
	instruccion+="from dgastos d  ";
	instruccion+="where d.referencia='";
	instruccion+=clave;
	instruccion+="'";
	if(orden=="1")
		instruccion+=" ORDER BY d.idorden ASC ";
	else
		instruccion+=" ORDER BY d.idorden DESC ";

	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);




}
//------------------------------------------------------------------------------
//ID_QRY_GASTOS_PROV
void ServidorGastos::ConsultaGastosProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    // CONSULTA LAS COMPRAS DE UN PROVEEDOR
    AnsiString instruccion;
    AnsiString proveedor;

    proveedor=mFg.ExtraeStringDeBuffer(&parametros);

    mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

    // Carga todos los datos del proveedor.
    instruccion.sprintf("select * from proveedores where proveedor='%s'", proveedor);
    mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Crea una tabla donde se van a poner los saldos de las compras del proveedor
	instruccion="create temporary table auxgastosssaldos (gasto char(11), saldo decimal(16,2), PRIMARY KEY (gasto)) Engine = InnoDB";
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query EjecutaSelectSqlNulo"));

    // Calcula los saldos de las compras del proveedor (SALDOB (saldo virtual))
	instruccion.sprintf("insert into auxgastosssaldos (gasto, saldo) \
		select g.referencia as gasto, sum(t.valor) as saldo \
		from gastos g \
		inner join transxpaggastos t ON t.referencia=g.referencia \
		where g.proveedor='%s' and \
		t.cancelada=0 and g.cancelado=0 and g.acredito=1 \
		group by g.referencia",
        proveedor);
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query EjecutaSelectSqlNulo"));

	instruccion.sprintf("select g.referencia as Ref, g.folioprov as Factura, \
	cs.saldo as Saldo, g.muuid as uuid, g.fechaalta, g.fechagas, \
	datediff(curdate(),g.fechavenc) as diasvenc \
	from gastos g \
	INNER JOIN estadosistemaemp e ON e.estado='FUCIERRE' AND e.sucursal = '%s' \
	inner join auxgastosssaldos cs ON g.referencia=cs.gasto \
    INNER JOIN sucursales suc ON g.sucursal = suc.sucursal AND suc.idempresa = %s \
	where cs.saldo > 0 \
	and cancelado=0 and proveedor='%s'",
	FormServidor->ObtieneClaveSucursal(), FormServidor->ObtieneClaveEmpresa(), proveedor);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

}
//------------------------------------------------------------------------------
//ID_GRA_PAGO_GASTO
void ServidorGastos::GrabaPagoGasto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    //  GRABA PAGO DE PROVEEDOR
	char *buffer_sql=new char[1024*64*50];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	int i, num_transacciones;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[5000];
	AnsiString clave_pago, tarea_pago;
	AnsiString proveedor, identificador, forma_pago, valor, ajuste;
	AnsiString num_cheque, banco_cheque, tipo_cheque, fecha_cobro_cheque, usuario;
	AnsiString folio_compra_sistema, valor_tran, folio_tran;
	AnsiString aplicada="1", estado_cheque,banco_cheque2=" " ;
	BufferRespuestas* resp_verificacion=NULL;
	int error;
	TDate fecha=Today();
	TTime hora=Time();
	AnsiString terminal, bancos_movbancos, bancos_NumCta, bancos_fechabancos, esperacomplemento;

	AnsiString valor_banco, valor_tran_banco;
	double valor_banco_double, valor_tran_banco_double, valor_tran_banco_acumulado_double=0, ajuste_double, ajuste_double_restante;

	try {
		// Grabar pago
		clave_pago=mFg.ExtraeStringDeBuffer(&parametros);
		tarea_pago=mFg.ExtraeStringDeBuffer(&parametros);
		proveedor=mFg.ExtraeStringDeBuffer(&parametros);
		identificador=mFg.ExtraeStringDeBuffer(&parametros);
		forma_pago=mFg.ExtraeStringDeBuffer(&parametros);
		valor=mFg.ExtraeStringDeBuffer(&parametros);
		ajuste=mFg.ExtraeStringDeBuffer(&parametros);
		num_cheque=mFg.ExtraeStringDeBuffer(&parametros);
		banco_cheque=mFg.ExtraeStringDeBuffer(&parametros);
		tipo_cheque=mFg.ExtraeStringDeBuffer(&parametros);
		estado_cheque=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_cobro_cheque=mFg.ExtraeStringDeBuffer(&parametros);
		usuario=mFg.ExtraeStringDeBuffer(&parametros);

		terminal=mFg.ExtraeStringDeBuffer(&parametros);
		bancos_movbancos=mFg.ExtraeStringDeBuffer(&parametros);
		bancos_NumCta=mFg.ExtraeStringDeBuffer(&parametros);
		bancos_fechabancos=mFg.ExtraeStringDeBuffer(&parametros);
		esperacomplemento=mFg.ExtraeStringDeBuffer(&parametros);
		// El valor de banco se obtiene sumando al valor del pago el ajuste (puede ser negativo el ajuste)
		ajuste_double=mFg.CadenaAFlotante(ajuste);
		valor_banco_double=mFg.CadenaAFlotante(valor)+ajuste_double;
		valor_banco=mFg.FormateaCantidad(valor_banco_double, 2, false);
		ajuste_double_restante=fabs(ajuste_double);  // Le quita el signo a ajuste_double_restante en caso de ajustes negativos.

		//validacion de espacio vacio en num_cheque
		//se declaro otra valiable para asignar el mismo valor que viene por defaul desde el cleinte en algun otro punto
		//del codigo
		if(banco_cheque==" "){
			banco_cheque2=banco_cheque;
			banco_cheque="NULL";
		}
		else{
			banco_cheque2=banco_cheque;
			banco_cheque.sprintf(" '%s' ",banco_cheque);
		}

		// Consulta parametros de bancos
		AnsiString paramautomat;
		AnsiString idmovbancoant;
		TDate paramfechaini;
		BufferRespuestas* resp_parambancos=NULL;
		AnsiString paramautoconsolidado;
		try {
			instruccion.sprintf("SELECT (SELECT valor FROM parambancos WHERE parametro='AUTOMAT') AS paramautomat, \
								(SELECT valor FROM parambancos WHERE parametro='FECHAINI') AS paramfechaini, \
								(SELECT valor FROM parametrosemp WHERE parametro='AUTOCONSOLPROV' AND sucursal = '%s' ) AS paramautoconsol, \
								(SELECT bt.idmovbanco FROM bancosxpaggasto bp inner join transxpag t on t.tracredito=bp.tracredito inner join bancostransacc bt on bt.transacc=bp.transacc where t.pago='%s' limit 1) as idmovbancoant",FormServidor->ObtieneClaveSucursal(), clave_pago);
			if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_parambancos)) {
				if (resp_parambancos->ObtieneNumRegistros()>0){
					paramautomat=resp_parambancos->ObtieneDato("paramautomat");
					paramfechaini=StrToDate(resp_parambancos->ObtieneDato("paramfechaini"));
					paramautoconsolidado=resp_parambancos->ObtieneDato("paramautoconsol");
					idmovbancoant=resp_parambancos->ObtieneDato("idmovbancoant");
				} else throw (Exception("No se encuentran registros en parambancos"));
			} else throw (Exception("Error al consultar en tabla parambancos"));
		} __finally {
			if (resp_parambancos!=NULL) delete resp_parambancos;
		}

		// Asígna los valores a las banderas de las transacciones dependiendo del
		// tipo de cheque.
		if (forma_pago=="C") {
			if (estado_cheque=="C") aplicada="1";
			else aplicada="0";
		}

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		if (tarea_pago=="A") {

			// Cabecera de bancos
			// Si está activada automatización de bancos y la fecha actual es igual o posterior a la fecha programada de inicio de registro automático
			if (forma_pago!="E" && (paramautomat=="1" && fecha>=paramfechaini)) {

				// Se insertan registros nuevos en bancosmov,
				// ALTA EN BANCOS
				instruccion.sprintf("insert into bancosmov \
					(idmovbanco, idnumcuenta, conceptomov, descripcion, identificador, cancelado, aplicado, \
					subtotal, ivabanco, total, fechaaplbanco, fechaalta, horaalta, fechamodi, horamodi, usualta, \
					usumodi, terminal, origen, sucursal, afectacion) \
					values (NULL, %s, 'D', 'PAGO DE GASTO', '%s', 0, 1, \
					%s, 0.00, %s, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', 'PAPRO', '%s', 'A')",
					bancos_NumCta, identificador,
					valor_banco, valor_banco, mFg.StrToMySqlDate(bancos_fechabancos),
					mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora),
					mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora),
					usuario, usuario, terminal, FormServidor->ObtieneClaveSucursal() );
				instrucciones[num_instrucciones++]=instruccion;

				instruccion.sprintf("set @idmovbanconuevo=LAST_INSERT_ID()");
				instrucciones[num_instrucciones++]=instruccion;

			}

			instruccion.sprintf("select @folioaux:=valor from foliosemp where folio='PAGGAST' AND sucursal = '%s'  %s ", FormServidor->ObtieneClaveSucursal(),  MODO_BLOQUEO_CLAVES_UNICAS);
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @foliosig=@folioaux+1");
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @folioaux=cast(@folioaux as char)");
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @folio=concat('%s', lpad(@folioaux,9,'0'))", FormServidor->ObtieneClaveSucursal());
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("update foliosemp set valor=@foliosig where folio='PAGGAST' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("insert into pagosgastos \
				(pago, ident, proveedor, fecha, hora, fechamodi, horamodi, valor, cancelado, usualta, usumodi, ajuste, formapag,terminal,esperacomplemento) \
				values (@folio, '%s', '%s', '%s', '%s', '%s', '%s', %s, 0, '%s', '%s', %s, '%s', '%s', %s)",
				identificador, proveedor, mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora),
				mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), valor,
				usuario, usuario, ajuste, forma_pago, terminal, esperacomplemento);
			instrucciones[num_instrucciones++]=instruccion;

			if (forma_pago=="C") {
				// Si es cheque entonces crea un registro en la tabla de cheques.
				instruccion.sprintf("select @foliocheqaux:=valor from foliosemp where folio='CHEQGAST' AND sucursal = '%s' %s ",FormServidor->ObtieneClaveSucursal(),  MODO_BLOQUEO_CLAVES_UNICAS);
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliocheqsig=@foliocheqaux+1");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliocheqaux=cast(@foliocheqaux as char)");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliocheq=concat('%s', lpad(@foliocheqaux,9,'0'))", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("update foliosemp set valor=@foliocheqsig where folio='CHEQGAST' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("insert into chequesgastos \
					(chequegasto,folio,banco,fechaalta,fechacob,valor,proveedor,estado,clasif) \
					values ( @foliocheq,'%s', %s, '%s', '%s', %s, '%s', '%s','%s')",
					num_cheque, banco_cheque.c_str(), mFg.DateToMySqlDate(fecha),
					mFg.StrToMySqlDate(fecha_cobro_cheque), valor, proveedor, estado_cheque, tipo_cheque);
				instrucciones[num_instrucciones++]=instruccion;

				// Crea la relación cheque-pago
				instruccion="insert into cheqxgas (chequegasto,pago) values (@foliocheq, @folio)";
				instrucciones[num_instrucciones++]=instruccion;
			}

		} else {
			// MODIFICACION
			instruccion.sprintf("set @folio='%s'", clave_pago);
			instrucciones[num_instrucciones++]=instruccion;

			// Al modificar un pago, en PAGOSPROV solo se actualizan los campos FECHAMODI, HORAMODI y USUMODI
			instruccion.sprintf("update pagosgastos set fechamodi='%s', horamodi='%s', usumodi='%s', ident='%s' where pago=@folio and cancelado=0", mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario, identificador);
			instrucciones[num_instrucciones++]=instruccion;

			// Marca como canceladas todas las transacciones, para
			// posteriormente descancelar solo a las que el usuario haga modificaciones.
			instruccion.sprintf("update transxpaggastos set cancelada=1, fechamodi='%s', horamodi='%s', usumodi='%s' where transxpag.pago=@folio", mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario );
			instrucciones[num_instrucciones++]=instruccion;

			if (forma_pago!="E" && idmovbancoant!="" && idmovbancoant!="0") {
				instruccion.sprintf("set @idmovbanconuevo=%s", idmovbancoant);
				instrucciones[num_instrucciones++]=instruccion;

				// **************************************************************************************
				// El auxiliar de bancos solo se actualiza cuando el pago original ya tenía registro de bancos.
				// Tanto para alta como modificacion de pagos se insertan registros nuevos en bancosmov,
				// ALTA EN BANCOS
				instruccion.sprintf("update bancosmov \
					set idnumcuenta=%s, identificador='%s', \
					subtotal=%s, total=%s, fechaaplbanco='%s', fechamodi='%s', horamodi='%s', usumodi='%s' \
					where idmovbanco=@idmovbanconuevo ",
					bancos_NumCta, identificador,
					valor_banco, valor_banco, mFg.DateToMySqlDate(fecha),
					mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora),
					usuario);
				instrucciones[num_instrucciones++]=instruccion;

				// Borra los registros anteriores de transacciones de bancos correspondientes a la transaccion
				instruccion.sprintf("delete from bancosxpaggasto where transacc in (select transacc from bancostransacc where idmovbanco=@idmovbanconuevo)");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("delete from bancostransacc where idmovbanco=@idmovbanconuevo");
				instrucciones[num_instrucciones++]=instruccion;
			}
		}


		num_transacciones=StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
		for (i=0; i<num_transacciones; i++) {
			folio_tran=mFg.ExtraeStringDeBuffer(&parametros);
			folio_compra_sistema=mFg.ExtraeStringDeBuffer(&parametros);
			valor_tran=mFg.ExtraeStringDeBuffer(&parametros);

			if (folio_tran=="?") {
				// Crea cada transacción
				instruccion.sprintf("select @foliotranaux:=valor from foliosemp where folio='TRANGAST' AND sucursal = '%s' %s ",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliotransig=@foliotranaux+1");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliotranaux=cast(@foliotranaux as char)");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliotran=concat('%s', lpad(@foliotranaux,9,'0'))", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("update foliosemp set valor=@foliotransig where folio='TRANGAST' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("insert into transxpaggastos \
					(tracredito, referencia, pago, concepto, destino, tipo, \
					cancelada, aplicada, valor, fechaalta, horaalta, fechamodi, horamodi, fechaapl, usualta, usumodi) values \
					(@foliotran, '%s', @folio, 'A', 'C', 'PAGO', \
					0, %s, -%s, '%s', '%s', '%s', '%s', '%s', '%s', '%s')",
					folio_compra_sistema, aplicada, valor_tran, mFg.DateToMySqlDate(fecha),
					mFg.TimeToMySqlTime(hora), mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora),
					mFg.DateToMySqlDate(fecha), usuario, usuario);
				instrucciones[num_instrucciones++]=instruccion;

			} else {
				// Aqui no es necesario actualizar la fecha de modificación ni el usuario
				// que modificó debido a que ya se hizo al cancelar todos los abonos que forman parte del pago.
				instruccion.sprintf("update transxpaggastos set cancelada=0 where tracredito='%s'", folio_tran);
				instrucciones[num_instrucciones++]=instruccion;

				instruccion.sprintf("set @foliotran='%s'", folio_tran);
				instrucciones[num_instrucciones++]=instruccion;
			}

			// Si está activada automatización de bancos y la fecha actual es igual o posterior a la fecha programada de inicio de registro automático
			// Los registros de bancostransacc y bancosxpaggasto se borran en las modificaciones, por lo que también aplica el volver a crearlos.
			if ( forma_pago!="E" && ((idmovbancoant!=""  && idmovbancoant!="0") || (paramautomat=="1" && fecha>=paramfechaini)) ) {
				AnsiString identificador_detalle="";
				if (forma_pago=="C") {
					identificador_detalle=num_cheque+"-"+banco_cheque2.c_str();
				} else {
					identificador_detalle=identificador;
				}

				valor_tran_banco_double=mFg.CadenaAFlotante(valor_tran);

				valor_tran_banco=mFg.FormateaCantidad(valor_tran_banco_double, 2, false);

				// PENDIENTE SACAR EL ID DEL IMPUESTO TIPO IVA
				instruccion.sprintf("insert into bancostransacc \
					(transacc, idmovbanco, tipodet, identificador, \
					subtotal, ivabanco, total, cveimp) \
					values (NULL, @idmovbanconuevo, '%s', '%s', \
					%s, 0.00, %s, 7)",
					forma_pago, identificador_detalle,
					valor_tran_banco, valor_tran_banco);
				instrucciones[num_instrucciones++]=instruccion;

				instruccion.sprintf("set @transacc=LAST_INSERT_ID()");
				instrucciones[num_instrucciones++]=instruccion;

				// Inserta relación de movimiento bancario con pago
				instruccion.sprintf("insert into bancosxpaggasto (transacc, tracredito) \
					values (@transacc, @foliotran)"  );
				instrucciones[num_instrucciones++]=instruccion;

			}

		}

		// Aplica el ajuste positivo en bancos como un renglon adicional en bancostransacc de tipo O y identificador AJUSTE
		// y sin registro en bancosxcob
		if (ajuste_double>0 || ajuste_double<0) {
			double ajuste_iva, ajuste_subtotal;
			ajuste_subtotal=ajuste_double/1.16;
			ajuste_iva=ajuste_double-ajuste_subtotal;
			if (forma_pago!="E" && ((idmovbancoant!="" && idmovbancoant!="0") || (paramautomat=="1" && fecha>=paramfechaini))) {
				instruccion.sprintf("insert into bancostransacc \
					(transacc, idmovbanco, tipodet, identificador, \
					subtotal, ivabanco, total, cveimp) \
					values (NULL, @idmovbanconuevo, 'A', 'AJUSTE', \
					%s, %s, %s, 8)",
					mFg.FormateaCantidad(ajuste_subtotal,2,false), mFg.FormateaCantidad(ajuste_iva,2,false), ajuste);
				instrucciones[num_instrucciones++]=instruccion;
			}
		}

		////////////////// INICIO CALCULO DE SALDOS DE COMPRAS

		// Crea una tabla para almacenar los folios de las compras afectadas por el pago
		// para posteriormente recalcular saldos de estas compras.
		instruccion="create temporary table gastoaux (gasto char(11), PRIMARY KEY (gasto)) Engine = InnoDB";
		instrucciones[num_instrucciones++]=instruccion;

		instruccion="insert into gastoaux (gasto) ";
		instruccion+="select t.referencia as gasto from transxpaggastos t where t.pago=@folio and t.cancelada=0 ";
		instrucciones[num_instrucciones++]=instruccion;

		// Crea una tabla donde se van a poner los saldos de las compras
		// afectadas por la cancelación
		instruccion="create temporary table auxgastosaldos (gasto char(11), saldo decimal(16,2), PRIMARY KEY (gasto)) Engine = InnoDB";
		instrucciones[num_instrucciones++]=instruccion;

		// Calcula los saldos de las compras relacionadas con el pago (SALDO B)
		instruccion="insert into auxgastosaldos (gasto, saldo) ";
		instruccion+="select g.referencia as gasto, sum(t.valor) as saldo ";
		instruccion+="from gastos g, gastoaux gaux, transxpaggastos t ";
		instruccion+="where g.referencia=gaux.gasto and ";
		instruccion+="t.referencia=g.referencia and t.cancelada=0 and g.cancelado=0 ";
		instruccion+="group by g.referencia";
		instrucciones[num_instrucciones++]=instruccion;

		////////////////// FIN CALCULO DE SALDOS DE COMPRAS

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			// VERIFICA SI QUEDO ALGUN SALDO NEGATIVO DE LAS FACTURAS EN CUESTION
			//
			instruccion="select * from auxgastosaldos where saldo<0";
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_verificacion);
			if (resp_verificacion->ObtieneNumRegistros()==0) {
				instruccion="COMMIT";
				error=0;
			} else {
				instruccion="ROLLBACK";
				error=1;
			}

			mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
			if (mServidorVioleta->EjecutaAccionSql(Respuesta, MySQL, instruccion.c_str())) {
				instruccion.sprintf("select %d as error, @folio as pago", error);
				mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
				if (error==1) {
					instruccion="select * from auxgastosaldos where saldo<0";
					mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
				}
			}
		}
	} __finally {
		if (resp_verificacion!=NULL) delete resp_verificacion;
		delete buffer_sql;
	}
}
//------------------------------------------------------------------------------
//ID_CON_PAGO_GASTO
void ServidorGastos::ConsultaPagoGasto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA PAGO DE PROVEEDOR
	AnsiString instruccion;
	AnsiString clave;

	clave=mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los generales del pago
	instruccion.sprintf("select * from pagosgastos where pago='%s'", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene todos los datos del cheque en cuestión
	instruccion.sprintf("select chequesgastos.* \
		from chequesgastos, cheqxgas \
		where cheqxgas.pago='%s' and \
		cheqxgas.chequegasto=chequesgastos.chequegasto", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene todas las transacciones correspondientes al pago.
	instruccion.sprintf("select t.tracredito, t.referencia, t.notaprov, t.pago, t.concepto, \
	t.tipo, t.cancelada, (t.valor*-1) as valor, t.fechaalta, t.horaalta, t.fechamodi, t.usualta, t.usumodi, \
	g.folioprov from transxpaggastos t, gastos g \
	where t.referencia=g.referencia and t.pago='%s' and t.cancelada=0", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Verifica que la fecha del pago no sea posterior a la fecha de cierre.
	instruccion.sprintf("select @error:=if((ifnull(chp.fechacob, '1900-01-01')<=cast(e.valor as datetime) \
	and chp.estado=ifnull(chp.estado, 'C')), 1, 0) \
	as error from pagosgastos p inner join cheqxgas chxp  inner join chequesgastos chp \
	left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' where p.pago='%s' \
	and chxp.pago=p.pago and chxp.chequegasto=chp.chequegasto",FormServidor->ObtieneClaveSucursal(), clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// **************************************************************************************
	// Obtiene todos los datos de bancos
	instruccion.sprintf("SELECT bm.idmovbanco, bcue.idnumcuenta, bcue.numerocuenta, bm.fechaaplbanco \
		FROM bancosxpaggasto bpag \
		INNER JOIN bancostransacc bt ON bpag.transacc=bt.transacc \
		INNER JOIN bancosmov bm ON bt.idmovbanco=bm.idmovbanco \
		INNER JOIN bancoscuentas bcue ON bcue.idnumcuenta=bm.idnumcuenta \
		INNER JOIN transxpaggastos t ON t.tracredito=bpag.tracredito \
		WHERE t.pago='%s' and bm.aplicado=1 and bm.cancelado=0 limit 1 ", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

}
//------------------------------------------------------------------------------
//ID_CON_PAGOS_GASTOS_DIA
void ServidorGastos::ConsultaPagosGastoDelDia(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA PAGOS DE PROVEEDOR DEL DIA
	AnsiString instruccion;
	AnsiString fecha, formapag, tipocheque, revision_tipo_cheque, mostrar_todas_las_formas=" ", proveedor;
	AnsiString condicion_proveedor=" ";


	fecha=mFg.ExtraeStringDeBuffer(&parametros);
	formapag=mFg.ExtraeStringDeBuffer(&parametros);
	proveedor=mFg.ExtraeStringDeBuffer(&parametros);


    revision_tipo_cheque=" ";
    if (formapag=="C") {
        mostrar_todas_las_formas.sprintf(" and p.formapag='%s' ",formapag);
        tipocheque=mFg.ExtraeStringDeBuffer(&parametros);
		if (tipocheque!="")
			revision_tipo_cheque.sprintf(" and chp.clasif='%s'",tipocheque);
    }else{
		if(formapag!="")
			mostrar_todas_las_formas.sprintf(" and p.formapag='%s' ",formapag);
	}

	if (proveedor!=" ") {
	  condicion_proveedor.sprintf(" and pr.proveedor='%s' ",proveedor);
	}
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	instruccion.sprintf("SELECT \
	p.ident AS identpago, \
	t.tracredito AS foliotran, \
	g.folioprov AS gastofolprov, \
	if(pr.tipoempre=0, replegal, razonsocial) AS nombreprov, \
	(t.valor*-1) AS valor, \
	(CASE \
	WHEN p.formapag='E' THEN 'EFECTIVO' \
	WHEN p.formapag='C' THEN 'CHEQUE' \
	WHEN p.formapag='D' THEN 'DEPOSITO' \
	WHEN p.formapag='T' THEN 'TRANSFERENCIA BANCARIA' \
	END) AS formapag, \
	chp.folio AS numcheque, \
	b.nombre AS nombanco, \
	chclasif.descripcion AS tipocheque, \
	chp.fechacob, \
	chp.estado AS status, \
	p.muuid AS UUID \
	FROM gastos g \
	INNER JOIN transxpaggastos t ON g.referencia=t.referencia \
	INNER JOIN pagosgastos p ON p.pago=t.pago \
	INNER JOIN proveedores pr ON pr.proveedor=g.proveedor \
	LEFT JOIN cheqxgas chxp ON chxp.pago=p.pago \
	LEFT JOIN chequesgastos chp ON chp.chequegasto=chxp.chequegasto \
	LEFT JOIN chequesclasif chclasif ON chclasif.clasif=chp.clasif \
	LEFT JOIN bancos b ON b.banco=chp.banco \
    INNER JOIN sucursales suc ON suc.sucursal = g.sucursal \
	WHERE p.fecha='%s' \
	AND g.cancelado=0 \
	AND t.cancelada=0 \
	AND p.cancelado=0 \
	%s %s %s \
	AND suc.idempresa = %s ",
	mFg.StrToMySqlDate(fecha), mostrar_todas_las_formas,
	revision_tipo_cheque,condicion_proveedor, FormServidor->ObtieneClaveEmpresa());
    mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

    //Se suman los ajustes
    if (formapag=="C") {
		instruccion.sprintf("select sum(p.ajuste) as ajuste \
		from pagosgastos p, cheqxgas chxp, chequesgastos chp where \
		p.fecha='%s' and chxp.pago=p.pago and chp.chequegasto=chxp.chequegasto \
		and p.cancelado=0 \
		%s %s \
		group by p.fecha",
		mFg.StrToMySqlDate(fecha), mostrar_todas_las_formas,
		revision_tipo_cheque);
    } else {
		instruccion.sprintf("select sum(p.ajuste) as ajuste \
		from pagosgastos p where \
		p.fecha='%s' \
		and p.cancelado=0 \
		%s \
		group by p.fecha",
		mFg.StrToMySqlDate(fecha), mostrar_todas_las_formas);
    }
    mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}
//------------------------------------------------------------------------------
//ID_CANC_PAGO_GASTO
void ServidorGastos::CancelaPagoGasto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  CANCELA PAGO DE PROVEEDOR
	char *buffer_sql=new char[1024*32];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instrucciones[50], instruccion;
	int num_instrucciones=0;
	AnsiString clave, usuario, estado, terminal;
	int i, error=0;
	TDate fecha=Today();
	TTime hora=Time();

	try {
		clave=mFg.ExtraeStringDeBuffer(&parametros);    // Clave del pago a cancelar.
		usuario=mFg.ExtraeStringDeBuffer(&parametros);  // Usuario que está cancelando.
		estado=mFg.ExtraeStringDeBuffer(&parametros);   // Estado con el que quedará el cheque al ser cancelado,
														// en el caso de pagos en efectivo se debe recibir ""
		terminal=mFg.ExtraeStringDeBuffer(&parametros);

		// Verifica que la fecha del pago no sea posterior a la fecha de cierre.
		instruccion.sprintf("select @error:=if((ifnull(chp.fechacob, '01-01-1900')<=cast(e.valor as datetime) \
		and chp.estado=ifnull(chp.estado, 'C')), 1, 0) \
		as error from pagosgastos p  inner join  cheqxgas chxp  inner join  chequesgastos chp \
		left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' where p.pago='%s' \
		and p.pago=chxp.pago and chxp.chequegasto=chp.chequegasto",FormServidor->ObtieneClaveSucursal(), clave); \
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 1, error);

		if (error==0) {
			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

			instruccion.sprintf("update pagosgastos set cancelado=1, fechamodi='%s', horamodi='%s', usumodi='%s' where pago='%s' and cancelado=0", mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario, clave);
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("update transxpaggastos set cancelada=1, aplicada=0, fechamodi='%s', horamodi='%s', usumodi='%s' where pago='%s' and cancelada=0", mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario, clave);
			instrucciones[num_instrucciones++]=instruccion;

			if (estado!="") {
				instruccion.sprintf("update chequesgastos, cheqxgas \
				set chequesgastos.estado='%s' \
				where cheqxgas.pago='%s' and chequesgastos.chequegasto=cheqxgas.chequegasto", estado, clave);
				instrucciones[num_instrucciones++]=instruccion;
			}

			// *********************************************************************************************
			// ------ INICIO Cancela el movimiento o suprime las transacciones parciales segun corresponda.
			//        (EXCEPTO EN PAGOS EN EFECTIVO).
			instruccion.sprintf("SELECT @idmovbancoanterior:=bt.idmovbanco, @ajuste:=p.ajuste \
			FROM bancosxpaggasto bc \
			inner join transxpaggastos t on t.tracredito=bc.tracredito \
			inner join bancostransacc bt on bt.transacc=bc.transacc \
			INNER JOIN pagosgastos p ON p.pago=t.pago \
			where t.pago='%s' AND p.formapag<>'E' LIMIT 1", clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Inserta en una tabla temporal todas las transacciones del movimiento.
			instruccion="create temporary table bancostransacctmp ( \
				idtemp int(11)  AUTO_INCREMENT, \
				transacc int(11), \
				PRIMARY KEY (idtemp) ) Engine = InnoDB";
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("insert into bancostransacctmp (idtemp, transacc) \
				SELECT NULL as idtemp, bt.transacc \
				FROM bancostransacc bt\
				INNER JOIN bancosxpaggasto bxp ON bt.transacc=bxp.transacc\
				INNER JOIN transxpaggastos txc ON bxp.tracredito=txc.tracredito AND txc.pago='%s'\
				WHERE bt.idmovbanco=@idmovbancoanterior", clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Borra las transacciones de bancosxpag correspondientes a las transacciones del movimiento
			instruccion.sprintf("delete from bancosxpaggasto where transacc in (select transacc from bancostransacctmp)");
			instrucciones[num_instrucciones++]=instruccion;

			// Borra las transacciones del movimiento.
			instruccion.sprintf("delete from bancostransacc where transacc in (select transacc from bancostransacctmp)");
			instrucciones[num_instrucciones++]=instruccion;

			// Borra la transacción de ajuste que corresponda al movimiento original por el mismo monto.
			instruccion.sprintf("delete from bancostransacc where idmovbanco=@idmovbancoanterior and tipodet='A' and identificador='AJUSTE' and total=@ajuste limit 1");
			instrucciones[num_instrucciones++]=instruccion;

			// Si el movimiento se queda sin transacciones se marca como cancelado.
			instruccion.sprintf("UPDATE bancosmov SET cancelado=1 \
			WHERE idmovbanco=@idmovbancoanterior AND \
			idmovbanco NOT IN (SELECT bt.idmovbanco FROM bancostransacc bt WHERE bt.idmovbanco=@idmovbancoanterior)");
			instrucciones[num_instrucciones++]=instruccion;

			// Suma los totales del movimientos.
			instruccion.sprintf("SELECT @totalmov:=IFNULL(SUM(total),0) AS total, @subtotalmov:=IFNULL(SUM(subtotal),0) AS subtotal, @ivabancomov:=IFNULL(SUM(ivabanco),0) AS ivabanco FROM bancostransacc WHERE idmovbanco=@idmovbancoanterior");
			instrucciones[num_instrucciones++]=instruccion;

			// Actualiza los totales del movimiento (solo si no está cancelado aun)
			instruccion.sprintf("update bancosmov \
				set \
				subtotal=@subtotalmov, total=@totalmov, ivabanco=@ivabancomov, fechamodi='%s', horamodi='%s', usumodi='%s' \
				where cancelado=0 and idmovbanco=@idmovbancoanterior ",
				mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora),
				usuario);
			instrucciones[num_instrucciones++]=instruccion;

			instrucciones[num_instrucciones++]="COMMIT";
		}


		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			instruccion.sprintf("select %d as error", error);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	} __finally {
		delete buffer_sql;
	}
}
//------------------------------------------------------------------------------
//ID_MODIF_FECHA_PAGO_GASTO
void ServidorGastos::ModificaFechaPagoGasto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  MODIFICA LA FECHA DE UN PAGO DE PROVEEDOR
	char *buffer_sql=new char[1024*32];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instrucciones[50], instruccion;
	int num_instrucciones=0;
	AnsiString clave, usuario, fecha_nueva;
	int i, error=0;
	TDate fecha=Today();
	TTime hora=Time();

	try{
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave del pago a modificar su fecha.
		usuario=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que está modificando la fecha.
		fecha_nueva=mFg.ExtraeStringDeBuffer(&parametros); // Nueva fecha a asignar.

		// Verifica que la fecha del pago sea posterior a la fecha de cierre.
		instruccion.sprintf("select @error:=if(p.fecha<= CAST(e.valor AS DATETIME), 1, 0) \
			as error from pagosgastos p INNER JOIN estadosistemaemp AS e \
			ON e.estado = 'FUCIERRE' AND e.sucursal = '%s' \
			where p.pago='%s' ",FormServidor->ObtieneClaveSucursal(), clave);
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 1, error);

		// Verifica que la fecha nueva sea posterior a la fecha de cierre.
		instruccion.sprintf("select @error:=if('%s'<=cast(e.valor as datetime), 1, 0) as error from estadosistemaemp e where e.estado='FUCIERRE' AND e.sucursal = '%s' ", mFg.DateToMySqlDate(StrToDate(fecha_nueva)), FormServidor->ObtieneClaveSucursal());
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 2, error);

		if (error==0) {
			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

			instruccion.sprintf("update pagosgastos set fecha='%s', fechamodi='%s', horamodi='%s', usumodi='%s' where pago='%s' and cancelado=0", mFg.StrToMySqlDate(fecha_nueva), mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario, clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Marca las transacciones como aplicadas y cambia las fechas.
			instruccion.sprintf("update transxpaggastos set aplicada=1, fechaapl='%s', fechaalta='%s', fechamodi='%s', horamodi='%s', usumodi='%s' where pago='%s' and cancelada=0", mFg.StrToMySqlDate(fecha_nueva) , mFg.StrToMySqlDate(fecha_nueva) ,mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario, clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Si existe cheque lo marca consolidado y modifica sus fechas.
			instruccion.sprintf("update chequesgastos, cheqxgas \
				set chequesgastos.estado='C', chequesgastos.fechacob='%s', chequesgastos.fechaalta='%s' \
				where cheqxgas.pago='%s' and chequesgastos.chequegasto=cheqxgas.chequegasto and chequesgastos.estado<>'X'",  mFg.StrToMySqlDate(fecha_nueva),  mFg.StrToMySqlDate(fecha_nueva), clave);
			instrucciones[num_instrucciones++]=instruccion;

			instrucciones[num_instrucciones++]="COMMIT";
		}

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			instruccion.sprintf("select %d as error", error);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	} __finally {
		delete buffer_sql;
	}
}
//---------------------------------------------------------------------------
// ID_CON_FACT_GASTO
void ServidorGastos::EjecutaConsultaFactGasto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
	AnsiString muuid;

	muuid=mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_REP);
	instruccion.sprintf("SELECT \
		dg.idorden, \
		dg.cantidad, \
		dg.unidad, \
		dg.descripcion, \
		dg.claveprodserv, \
		dg.valorunitario, \
		dg.descuento, \
		dg.costo, \
		(dg.iva_trasladado+dg.ieps_trasladado), \
		(dg.iva_retenido+dg.ieps_retenido), \
		dg.costoimp, \
		dg.isr_retenido, \
		dg.iva_retenido, \
		dg.ieps_retenido, \
		dg.iva_trasladado, \
		dg.ieps_trasladado, \
		dg.tasa_isr_retenido, \
		dg.tasa_iva_retenido, \
		dg.tasa_ieps_retenido, \
		dg.tasa_iva_trasladado, \
		dg.tasa_ieps_trasladado \
	FROM gastos g \
	INNER JOIN dgastos dg ON dg.referencia = g.referencia \
	WHERE g.muuid = '%s' AND g.cancelado=0 ", muuid);

	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	instruccion.sprintf("SELECT imp.porcentaje, g.impuestoret FROM gastos g \
	LEFT JOIN impuestos imp ON imp.impuesto = g.impuestoret \
	WHERE g.muuid = '%s'", muuid);

	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}
//------------------------------------------------------------------------------
//ID_GRA_DEVOL_GASTO
void ServidorGastos::GrabaDevolGasto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    //  GRABA DEVOLUCION
	char *buffer_sql=new char[3000*1000];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString tarea_devol, clave_devol, clave_compra, usuario, terminal;
	int num_partidas, i;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[3000], tipo;
	double valor;
	TDateTime fecha_dev;
	int error=0;
	AnsiString articulo, costo_art;
	TDate fecha=Today();
	TTime hora=Time();
	AnsiString tipobitacoracosto;
	BufferRespuestas* resp_verificacion=NULL;
	AnsiString mensaje;

	// Consulta de parámetro para asignación de precios
	// * Si es 0 el cambio de precio es inmediato y se guarda en el campo precios.precio
	// * si es 1 entonces el cambio de precios es diferido y se guarda precios.precioproximo para aplicarse posteriormente)
	AnsiString paramcambioprec;
	BufferRespuestas* resp_paramcambioprec=NULL;
	try {
		instruccion.sprintf("SELECT valor FROM parametrosglobemp WHERE parametro='CAMBIOPRECDIFER' AND idempresa = %s ", FormServidor->ObtieneClaveEmpresa());
		if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_paramcambioprec)) {
			if (resp_paramcambioprec->ObtieneNumRegistros()>0){
				paramcambioprec=resp_paramcambioprec->ObtieneDato("valor");
			} else throw (Exception("No se encuentra registro CAMBIOPRECDIFER en tabla parametrosglobemp"));
		} else throw (Exception("Error al consultar en tabla parametros"));
	} __finally {
		if (resp_paramcambioprec!=NULL) delete resp_paramcambioprec;
	}

	try {
		clave_devol=mFg.ExtraeStringDeBuffer(&parametros); // Clave de la devolucion.
		tarea_devol=mFg.ExtraeStringDeBuffer(&parametros); // Indicador si se va a agregar o modificar.
		usuario=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que esta grabando la devolucion.
		terminal=mFg.ExtraeStringDeBuffer(&parametros); // Terminal que esta grabando la devolucion.
        mensaje=mFg.ExtraeStringDeBuffer(&parametros); // Mensaje de la compra.

		datos.AsignaTabla("notascredgasto");

		// Obtiene los datos de la tabla de notas de crédito
		datos.AsignaTabla("notascredgasto");
		parametros+=datos.InsCamposDesdeBuffer(parametros);

		// Extrae los datos que necesitamos para crear las letras y transacciones.
		datos.AsignaValorCampo("referencia", "@folio", 1);
		valor=StrToFloat(datos.ObtieneValorCampo("valor"));
		fecha_dev=StrToDate(datos.ObtieneValorCampo("fechanot"));
		clave_compra=datos.ObtieneValorCampo("gasto");
		tipo =datos.ObtieneValorCampo("tipo");
		// Verificaciones para no permitir modificaciones cuando no sea permitido.
		if (tarea_devol=="M") {

			// Obtiene el folio de la compra correspondiente y el valor de la nota
			instruccion.sprintf("select @gasto:=gasto, @valor:=valor from notascredgasto where referencia='%s'",clave_devol);
			mServidorVioleta->EjecutaSelectSqlNulo(Respuesta,  MySQL, instruccion.c_str());

			// Verifica que no haya notas de crédito posteriores de la misma compra
			instruccion.sprintf("select @error:=if((COALESCE(sum(t.valor),0))<0, 1, 0) as error from notascredgasto n, transxpag t where n.gasto=@gasto and n.referencia>'%s' and n.gasto=t.referencia and n.cancelado=0 and t.cancelada=0 and t.tipo='DEVO'", clave_devol);
			mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 2, error);

			// Verifica que la fecha de la nota sea posterior a la fecha de cierre.
			instruccion.sprintf("select @error:=if(n.fechanot<=cast(e.valor as datetime), 1, 0) as error from notascredgasto n left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' where n.referencia='%s'",FormServidor->ObtieneClaveSucursal(), clave_devol);
			mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 3, error);

			// Verifica que la notas de crédito tenga asignado un uuid
			instruccion.sprintf("SELECT @error:=IF(muuid<>'',1,0) AS error FROM notascredgasto WHERE referencia='%s' AND cancelado=0 GROUP BY referencia", clave_devol);
			mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 6, error);

			tipobitacoracosto = "MNC" ;

		} else {

			// Verifica que la fecha de la devolución sea posterior a la fecha de cierre.
			instruccion.sprintf("select @error:=if('%s'<=cast(e.valor as datetime), 1, 0) as error from estadosistemaemp e where e.estado='FUCIERRE' AND e.sucursal = '%s'  ", mFg.DateToMySqlDate(fecha_dev), FormServidor->ObtieneClaveSucursal());
			mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 4, error);

			tipobitacoracosto = "NC";
		}


		if (error==0) {
			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

			// Obtiene el folio para la nota
			if (tarea_devol=="A") {
				instruccion.sprintf("select @folioaux:=valor from foliosemp where folio='NOTASCRGAS' AND sucursal = '%s' %s ",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliosig=@folioaux+1");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @folioaux=cast(@folioaux as char)");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @folio=concat('%s', lpad(@folioaux,9,'0'))", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("update foliosemp set valor=@foliosig where folio='NOTASCRGAS' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
			} else {
				instruccion.sprintf("set @folio='%s'", clave_devol);
				instrucciones[num_instrucciones++]=instruccion;
			}

			if (tarea_devol=="M") {
				instruccion.sprintf("delete from dnotascredgasto where referencia=@folio");
				instrucciones[num_instrucciones++]=instruccion;
			}

			// Graba la cabecera en la tabla "notascredprov"
			if (tarea_devol=="A") {
				instruccion.sprintf("select @seccion:=seccion, @depart:=depart from terminales where terminal='%s'", terminal);
				instrucciones[num_instrucciones++]=instruccion;

				datos.InsCampo("terminal", terminal);
				datos.InsCampo("docseccion", "@seccion",1);
				datos.InsCampo("docdepart", "@depart",1);
				datos.InsCampo("usualta", usuario);
				datos.InsCampo("usumodi", usuario);
				datos.InsCampo("fechaalta", mFg.DateToAnsiString(fecha));
				datos.InsCampo("horaalta", mFg.TimeToAnsiString(hora));
				datos.InsCampo("fechamodi", mFg.DateToAnsiString(fecha));
				datos.InsCampo("horamodi", mFg.TimeToAnsiString(hora));
				datos.InsCampo("cancelado", "0");
				instrucciones[num_instrucciones++]=datos.GenerarSqlInsert();

				// Guarda el mensaje
				if (mensaje!=" ") { // Un espacio lo tomamos como nulo.
					instruccion.sprintf("insert into notascredgastosmensajes (referencia, mensaje) values (@folio,'%s')",
						mensaje);
					instrucciones[num_instrucciones++]=instruccion;
				}

			} else {
				datos.InsCampo("usumodi", usuario);
				datos.InsCampo("fechamodi", mFg.DateToAnsiString(fecha));
				datos.InsCampo("horamodi", mFg.TimeToAnsiString(hora));
				instrucciones[num_instrucciones++]=datos.GenerarSqlUpdate("referencia=@folio");

                // Guarda el mensaje
				if (mensaje!=" ") { // Un espacio lo tomamos como nulo.
					instruccion.sprintf("replace into notascredgastosmensajes (referencia, mensaje) values (@folio,'%s')",
						mensaje);
				} else {
					instruccion.sprintf("delete from notascredgastosmensajes where referencia=@folio");
				}
				instrucciones[num_instrucciones++]=instruccion;
			}

			instruccion="create temporary table tmpcostos ( \
				articulo varchar(9), producto varchar(8), present varchar(255), \
				costo decimal(16,6), modificar bool, PRIMARY KEY (articulo)) Engine = InnoDB";
			instrucciones[num_instrucciones++]=instruccion;

			// Graba las partidas en "dnotascredgasto"
			num_partidas=StrToInt(mFg.ExtraeStringDeBuffer(&parametros)); // Obtiene el número de partidas
			for (i=0; i<num_partidas; i++) {
				datos.AsignaTabla("dnotascredgasto");
				parametros+=datos.InsCamposDesdeBuffer(parametros);
				datos.InsCampo("referencia", "@folio", 1);
				articulo=datos.ObtieneValorCampo("claveprodserv");
				costo_art=datos.ObtieneValorCampo("costoimp");
				instrucciones[num_instrucciones++]=datos.GenerarSqlInsert();
			}

			// Hace un abono que refleje en proveedores la nota de crédito.
			if (tarea_devol=="A") {
				// Obtiene el folio para la NUEVA transaccion
				instruccion.sprintf("select @foliotranaux:=valor from foliosemp where folio='TRANGAST' AND sucursal = '%s' %s ",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliosigtran=@foliotranaux+1");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliotranaux=cast(@foliotranaux as char)");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliotran=concat('%s', lpad(@foliotranaux,9,'0'))", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("update foliosemp set valor=@foliosigtran where folio='TRANGAST' AND sucursal = '%s' ",FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("insert into transxpaggastos \
					(tracredito, referencia, notaprov, concepto, destino, tipo, cancelada, aplicada, valor, fechaalta, horaalta, fechamodi, horamodi, fechaapl, usualta, usumodi) \
					values (@foliotran, '%s', @folio, 'A', 'C', 'DEVO', 0,1, -%12.2f, '%s', '%s', '%s', '%s', '%s', '%s', '%s')",
					 clave_compra, valor, mFg.DateToMySqlDate(fecha_dev), mFg.TimeToMySqlTime(hora), mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), mFg.DateToMySqlDate(fecha_dev), usuario, usuario);
				instrucciones[num_instrucciones++]=instruccion;
			} else {
				// Obtiene el folio de la transaccion ya existente
				instruccion.sprintf("select @foliotran:=tracredito from transxpaggastos where referencia='%s' and notaprov=@folio and concepto='A' and destino='C' and tipo='DEVO' and cancelada=0", clave_compra);
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("update transxpaggastos set valor=-%12.2f, fechamodi='%s', horamodi='%s', fechaapl='%s', usumodi='%s' where tracredito=@foliotran", valor, mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), mFg.DateToMySqlDate(fecha_dev), usuario);
				instrucciones[num_instrucciones++]=instruccion;
			}

			////////////////// INICIO CALCULO DE SALDOS DE COMPRAS

			// Crea una tabla para almacenar los folios de las compras afectadas por el pago
			// para posteriormente recalcular saldos de estas compras.
			instruccion="create temporary table gastosaux (gasto char(11), PRIMARY KEY (gasto)) Engine = InnoDB";
			instrucciones[num_instrucciones++]=instruccion;

			instruccion="insert into gastosaux (gasto) ";
			instruccion+="select t.referencia as gasto from transxpaggastos t where t.referencia='";
			instruccion+=clave_compra;
			instruccion+="' and t.cancelada=0 GROUP BY t.referencia";
			instrucciones[num_instrucciones++]=instruccion;

			// Crea una tabla donde se van a poner los saldos de las compras
			// afectadas por la cancelación
			instruccion="create temporary table auxgastossaldos (gasto char(11), saldo decimal(16,2), PRIMARY KEY (gasto)) Engine = InnoDB";
			instrucciones[num_instrucciones++]=instruccion;

			// Calcula los saldos de las compras relacionadas con el pago (SALDO B)
			instruccion="insert into auxgastossaldos (gasto, saldo) ";
			instruccion+="select g.referencia as gasto, sum(t.valor) as saldo ";
			instruccion+="from gastos g, gastosaux caux, transxpaggastos t ";
			instruccion+="where g.referencia=caux.gasto and ";
			instruccion+="t.referencia=g.referencia and t.cancelada=0 and g.cancelado=0 ";
			instruccion+="group by g.referencia";
			instrucciones[num_instrucciones++]=instruccion;

			////////////////// FIN CALCULO DE SALDOS DE COMPRAS
		}


		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {

            // VERIFICA SI QUEDO ALGUN SALDO NEGATIVO DE LAS FACTURAS EN CUESTION
			//
			try{
				//que esta parate solo se realice cuando no hay error en alguna validacion anterior
				if(error==0){
					instruccion="select * from auxgastossaldos where saldo<0";
					mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_verificacion);
					if (resp_verificacion->ObtieneNumRegistros()==0) {
						instruccion="COMMIT";
						error=0;
					} else {
						instruccion="ROLLBACK";
						error=5;
					}
				}else {
					instruccion="ROLLBACK";
				}
			}
			__finally{
				if(resp_verificacion!=NULL) delete resp_verificacion;
            }

			mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

			if (mServidorVioleta->EjecutaAccionSql(Respuesta, MySQL, instruccion.c_str())) {
                instruccion.sprintf("select %d as error, @folio as folio", error);
				mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
			}

		}
	} __finally {
		delete buffer_sql;
	}
}
//------------------------------------------------------------------------------
//ID_CON_DEVOL_GASTO
void ServidorGastos::ConsultaDevolGastos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA NOTAS DE CREDITO
	AnsiString instruccion;
	AnsiString clave,orden;

	clave=mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
	orden=mFg.ExtraeStringDeBuffer(&parametros);

	// Obtiene todos los generales (de cabecera) de la nota de credito.
	instruccion.sprintf("select ncp.*, @foliogasto:=ncp.gasto, ncpm.mensaje, i.porcentaje from notascredgasto ncp \
	left join notascredgastosmensajes ncpm ON ncpm.referencia = ncp.referencia \
	left join impuestos i ON i.impuesto = ncp.impuestoret where ncp.referencia='%s'", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene los datos del gasto respectivo.
	instruccion.sprintf("select g.* from gastos g where g.referencia=@foliogasto");
    mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Calcula el saldo del gasto (SALDO B)
	instruccion.sprintf("select sum(if(cancelada=0,valor,0)) as saldo from transxpaggastos where referencia=@foliogasto and cancelada=0 group by referencia");
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

    // Obtiene todos los datos del proveedor de la nota de credito.
	instruccion.sprintf("select p.* from proveedores p, gastos g where g.referencia=@foliogasto and g.proveedor=p.proveedor");
    mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

    // NULO:  Verifica que la fecha de la factura sea posterior a la fecha de cierre.
	instruccion.sprintf("select if(n.fechanot<=cast(e.valor as datetime), 1, 0) as modificar from notascredgasto n left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' where n.referencia='%s'",FormServidor->ObtieneClaveSucursal(), clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Suma las notas de crédito y deja el resultado en una tabla temporal,
	// para luego hacerle un left join con los gastos.
	instruccion="create temporary table  dnotascredprovaux ";
	instruccion+="select d.claveprodserv, sum(if(n.tipo='0',d.cantidad,0)) as cantidad, ";
	instruccion+="sum(if(n.tipo<>'0',d.costo,0)) as costo ";
	instruccion+="from notascredgasto n, dnotascredgasto d ";
	instruccion+="where n.gasto=@foliogasto";
	instruccion+=" and n.cancelado=0 and n.referencia=d.referencia ";
	instruccion+="group by d.claveprodserv";
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query EjecutaSelectSqlNulo"));

	// Sumatoria de todas las notas de crédito.
	instruccion.sprintf("select ifnull(sum(valor),0) as sumnotas from notascredgasto where cancelado=0 and gasto=@foliogasto");
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene los datos de la factura de gasto
	instruccion="SELECT ";
	instruccion+="dg.idorden, ";
	instruccion+="dg.cantidad, ";
	instruccion+="dg.unidad, ";
	instruccion+="dg.descripcion, ";
	instruccion+="dg.claveprodserv, ";
	instruccion+="dg.valorunitario, ";
	instruccion+="dg.descuento, ";
	instruccion+="dg.costo, ";
	instruccion+="(dg.iva_trasladado+dg.ieps_trasladado), ";
	instruccion+="(dg.iva_retenido+dg.ieps_retenido), ";
	instruccion+="dg.costoimp, ";
	instruccion+="dg.isr_retenido, ";
	instruccion+="dg.iva_retenido, ";
	instruccion+="dg.ieps_retenido, ";
	instruccion+="dg.iva_trasladado, ";
	instruccion+="dg.ieps_trasladado, ";
	instruccion+="dg.tasa_isr_retenido, ";
	instruccion+="dg.tasa_iva_retenido, ";
	instruccion+="dg.tasa_ieps_retenido, ";
	instruccion+="dg.tasa_iva_trasladado, ";
	instruccion+="dg.tasa_ieps_trasladado ";
	instruccion+="FROM gastos g ";
	instruccion+="INNER JOIN dgastos dg ON dg.referencia = g.referencia ";
	instruccion+="WHERE g.referencia=@foliogasto ";

	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene los datos de la factura de gasto
	instruccion="SELECT ";
	instruccion+="dn.idorden, ";
	instruccion+="dn.cantidad, ";
	instruccion+="dn.unidad, ";
	instruccion+="dn.descripcion, ";
	instruccion+="dn.claveprodserv, ";
	instruccion+="dn.valorunitario, ";
	instruccion+="dn.descuento, ";
	instruccion+="dn.costo, ";
	instruccion+="(dn.iva_trasladado+dn.ieps_trasladado), ";
	instruccion+="(dn.iva_retenido+dn.ieps_retenido), ";
	instruccion+="dn.costoimp, ";
	instruccion+="dn.isr_retenido, ";
	instruccion+="dn.iva_retenido, ";
	instruccion+="dn.ieps_retenido, ";
	instruccion+="dn.iva_trasladado, ";
	instruccion+="dn.ieps_trasladado, ";
	instruccion+="dn.tasa_isr_retenido, ";
	instruccion+="dn.tasa_iva_retenido, ";
	instruccion+="dn.tasa_ieps_retenido, ";
	instruccion+="dn.tasa_iva_trasladado, ";
	instruccion+="dn.tasa_ieps_trasladado ";
	instruccion+="FROM notascredgasto n ";
	instruccion+="INNER JOIN dnotascredgasto dn ON dn.referencia = n.referencia ";

	AnsiString cond_ncg = " ";
	cond_ncg.sprintf("WHERE n.gasto=@foliogasto AND n.referencia='%s' ", clave);
	instruccion+= cond_ncg;

	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}
//------------------------------------------------------------------------------
//ID_CANC_DEVOL_GASTO
void ServidorGastos::CancelaDevolGasto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  CANCELA DEVOLUCION
	char *buffer_sql=new char[1024*64];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instrucciones[70], instruccion;
	int num_instrucciones=0;
	AnsiString clave, usuario;
	AnsiString modcosto, tipo;
	int i, error=0;
	TDate fecha=Today();
	TTime hora=Time();
	BufferRespuestas* resp_costo=NULL;
	AnsiString producto, present, multiplo, nombre ;

	try {
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave de la devolución
		usuario=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que esta cancelando la devolución.

		// Obtiene el folio de la compra correspondiente y el valor de la nota
		instruccion.sprintf("select @gasto:=gasto, @valor:=valor from notascredgasto where referencia='%s'",clave);
		mServidorVioleta->EjecutaSelectSqlNulo(Respuesta,  MySQL, instruccion.c_str());

		// Verifica que no haya notas de crédito posteriores de la misma compra
		instruccion.sprintf("select @error:=if((COALESCE(sum(t.valor),0))<0, 1, 0) as error from notascredgasto n, transxpaggastos t where n.gasto=@gasto and n.referencia>'%s' and n.gasto=t.referencia and n.cancelado=0 and t.cancelada=0 and t.tipo='DEVO'", clave);
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 2, error);

		// Verifica que la fecha de la nota sea posterior a la fecha de cierre.
		instruccion.sprintf("select @error:=if(n.fechanot<=cast(e.valor as datetime), 1, 0) as error from notascredgasto n left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' where n.referencia='%s'",FormServidor->ObtieneClaveSucursal(), clave);
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 3, error);

		if (error==0) {
			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

			instruccion.sprintf("update notascredgasto set cancelado=1, usumodi='%s', fechamodi='%s', horamodi='%s' where referencia='%s' and cancelado=0", usuario, mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Cancela el abono correspondiente de la devolución
			instruccion.sprintf("select @foliotran:=tracredito, @gasto:=referencia from transxpaggastos where notaprov='%s' and concepto='A' and destino='C' and tipo='DEVO' and cancelada=0", clave);
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("update transxpaggastos set cancelada=1, fechamodi='%s', horamodi='%s', usumodi='%s' where tracredito=@foliotran", mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario );
			instrucciones[num_instrucciones++]=instruccion;

			instrucciones[num_instrucciones++]="COMMIT";
		}

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			instruccion.sprintf("select %d as error", error);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	} __finally {
		delete buffer_sql;
	}
}
//---------------------------------------------------------------------------
// ID_EJE_REPGASTOS_X_FACTURA
void ServidorGastos::EjecutaRepGastosXFactura(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString Instruccion;
	AnsiString gasto_ref, fecha_inicial, fecha_final, empresa, sucursal, canceladas, tipo_canceladas, terminospago, proveedor, notascred;
	AnsiString fecha_nota_cr, tipoGasto, solicitadopor, autorizadopor, foliofactprov, parterelacion, numcuenta;

	AnsiString fecha_nota=" ";
    AnsiString foliosistema_pago=" ";

	AnsiString condicion_gasto=" ", condicion_empresa=" ", condicion_sucursal=" ", condicion_canceladas=" ", condicion_terminospago=" ";
	AnsiString condicion_proveedor=" ", condicion_tipoGasto=" ", condicion_solicitadopor=" ";
	AnsiString condicion_autorizadopor=" ", condicion_foliofactprov=" ", condicion_parterelacion=" ";
	AnsiString condicion_numcuenta=" ";
    AnsiString condicion_foliosistema_pago;

	gasto_ref = mFg.ExtraeStringDeBuffer(&parametros);
	fecha_inicial=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
	fecha_final	 =mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
	empresa	 =mFg.ExtraeStringDeBuffer(&parametros);
	sucursal	 =mFg.ExtraeStringDeBuffer(&parametros);
	canceladas	 =mFg.ExtraeStringDeBuffer(&parametros);
	tipo_canceladas=mFg.ExtraeStringDeBuffer(&parametros);
	terminospago =mFg.ExtraeStringDeBuffer(&parametros);
	proveedor	 =mFg.ExtraeStringDeBuffer(&parametros);
	notascred	 =mFg.ExtraeStringDeBuffer(&parametros);
	fecha_nota_cr=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
	tipoGasto	 =mFg.ExtraeStringDeBuffer(&parametros);
	solicitadopor=mFg.ExtraeStringDeBuffer(&parametros);
	autorizadopor=mFg.ExtraeStringDeBuffer(&parametros);
	foliofactprov=mFg.ExtraeStringDeBuffer(&parametros);
	parterelacion=mFg.ExtraeStringDeBuffer(&parametros);
	numcuenta	 =mFg.ExtraeStringDeBuffer(&parametros);
	foliosistema_pago = mFg.ExtraeStringDeBuffer(&parametros);

	if (canceladas!=" ") {
		condicion_canceladas.sprintf(" AND g.cancelado=%s ", canceladas);
		if(canceladas=="1"){
			if(tipo_canceladas=="0"){
				fecha_nota.sprintf(" AND g.fechamodi>='%s' and g.fechamodi<='%s' ", fecha_inicial, fecha_final);
			}else{
				fecha_nota.sprintf(" AND g.fechagas>='%s' and g.fechagas<='%s' ", fecha_inicial, fecha_final);
			}
		} else {
			fecha_nota.sprintf(" AND g.fechagas>='%s' and g.fechagas<='%s' ", fecha_inicial, fecha_final);
		}
	}else{
		fecha_nota.sprintf(" AND g.fechagas>='%s' and g.fechagas<='%s' ", fecha_inicial, fecha_final);
	}
	if(gasto_ref!=" "){
		condicion_gasto.sprintf(" AND g.referencia='%s' ", gasto_ref);
	}
    if(empresa != " "){
		condicion_empresa.sprintf(" AND suc.idempresa = %s ", empresa);
	}
	if (sucursal!=" ") {
		condicion_sucursal.sprintf(" AND g.sucursal='%s' ", sucursal);
	}
	if (terminospago!=" ") {
		condicion_terminospago.sprintf(" AND g.acredito=%s ", terminospago);
	}
	if (proveedor!=" ") {
		condicion_proveedor.sprintf(" AND g.proveedor='%s' ", proveedor);
	}
	if (tipoGasto!=" ") {
		condicion_tipoGasto.sprintf(" AND g.tipogasto=%s ", tipoGasto);
	}
	if (solicitadopor!=" ") {
		condicion_solicitadopor.sprintf(" AND g.solicita='%s' ", solicitadopor); //
	}
	if (autorizadopor!=" ") {
		condicion_autorizadopor.sprintf(" AND g.autoriza='%s' ", autorizadopor);   //
	}
	if (foliofactprov!=" ") {
		condicion_foliofactprov.sprintf(" AND g.folioprov='%s' ", foliofactprov);  //
	}
	if (parterelacion!=" ") {
		condicion_parterelacion.sprintf(" AND pro.esparterelac=%s ", parterelacion);
	}
	if (numcuenta!=" ") {
		condicion_numcuenta.sprintf(" AND g.numcuenta='%s' ", numcuenta);
	}
	if (foliosistema_pago!=" ") {
		condicion_foliosistema_pago.sprintf(" AND g.referencia='%s' ", foliosistema_pago);
	}

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_REP_GRANDE);

	if (notascred=="1") {
		Instruccion.sprintf("SELECT \
			t.referencia, \
			t.sucursal, \
			t.muuid, \
			t.folioprov, \
			t.fechagas AS fecha, \
			t.proveedor, \
			t.razonsocial, \
			SUM(t.total) AS total, \
			SUM(t.subtotal) AS subtotal, \
			SUM(t.iva_trasladado) AS iva_trasladado, \
			SUM(t.ieps_trasladado) AS ieps_trasladado, \
			SUM(t.iva_retenido) AS iva_retenido, \
			SUM(t.isr_retenido) AS isr_retenido, \
	   		SUM(t.ISR_resico) AS ISR_resico \
		FROM ( \
				 ( \
					SELECT g.referencia, \
						g.sucursal, \
						g.folioprov, \
						g.muuid, \
						g.fechagas, \
						pro.proveedor, \
						pro.razonsocial, \
						SUM(dg.costoimp)-(g.descuento) AS total, \
						SUM(dg.costo)-(g.descuento) AS subtotal, \
						SUM(dg.iva_trasladado) AS iva_trasladado, \
						SUM(dg.ieps_trasladado) AS ieps_trasladado, \
						SUM(dg.iva_retenido) AS iva_retenido, \
						SUM(dg.isr_retenido) AS isr_retenido, \
						(SUM(dg.costo)*i.porcentaje/100) AS ISR_resico, \
						suc.numid \
					FROM gastos g \
					INNER JOIN dgastos dg ON g.referencia=dg.referencia \
					INNER JOIN proveedores pro ON g.proveedor=pro.proveedor \
					INNER JOIN terminales ter ON ter.terminal=g.terminal \
					INNER JOIN secciones sec ON ter.seccion=sec.seccion \
					INNER JOIN sucursales suc ON sec.sucursal=suc.sucursal \
					LEFT JOIN impuestos i on i.impuesto = g.impuestoret \
					WHERE 1 %s \
						%s \
						%s \
						%s \
						%s \
						%s \
						%s \
						%s \
						%s \
						%s \
						%s \
						%s \
						%s \
						%s \
					GROUP BY g.referencia \
					ORDER BY g.referencia \
				 ) \
				 UNION ALL \
				 ( \
					SELECT \
						g.referencia, \
						g.sucursal, \
						g.folioprov, \
						g.muuid, \
						g.fechagas, \
						pro.proveedor, \
						pro.razonsocial, \
						(SUM(d.costoimp)-(n.descuento))*-1 AS total, \
						sum(if(n.tipo<>'0',d.costo,0))*-1 AS subtotal, \
						SUM(d.iva_trasladado)*-1 AS iva_trasladado, \
						SUM(d.ieps_trasladado)*-1 AS ieps_trasladado, \
						SUM(d.iva_retenido)*-1 AS iva_retenido, \
						SUM(d.isr_retenido)*-1 AS isr_retenido, \
						(SUM(d.costo)*i.porcentaje/100)*-1 AS ISR_resico, \
						suc.numid \
					FROM notascredgasto n \
					INNER JOIN dnotascredgasto d ON n.referencia=d.referencia \
					INNER JOIN gastos g ON n.gasto=g.referencia \
					INNER JOIN proveedores pro ON g.proveedor=pro.proveedor \
					INNER JOIN terminales ter ON ter.terminal=g.terminal \
					INNER JOIN secciones sec ON ter.seccion=sec.seccion \
					INNER JOIN sucursales suc ON sec.sucursal=suc.sucursal \
					LEFT JOIN impuestos i on i.impuesto = n.impuestoret \
					WHERE 1 %s \
						%s \
						%s \
						%s \
						%s \
						%s \
						%s \
						%s \
						%s \
						%s \
						%s \
						%s \
						%s \
						AND n.cancelado=0 \
					GROUP BY g.referencia \
				 ) \
				  ) t GROUP BY t.referencia ORDER BY t.numid, t.referencia",
				  fecha_nota,
				  condicion_empresa,
				  condicion_sucursal,
				  condicion_canceladas,
				  condicion_terminospago,
				  condicion_proveedor,
				  condicion_tipoGasto,
				  condicion_solicitadopor,
				  condicion_autorizadopor,
				  condicion_foliofactprov,
				  condicion_parterelacion,
				  condicion_numcuenta,
				  fecha_nota,
				  condicion_gasto,

				  condicion_empresa,
				  condicion_sucursal,
				  condicion_canceladas,
				  condicion_terminospago,
				  condicion_proveedor,
				  condicion_tipoGasto,
				  condicion_solicitadopor,
				  condicion_autorizadopor,
				  condicion_foliofactprov,
				  condicion_parterelacion,
                  condicion_numcuenta,
				  fecha_nota,
				  condicion_gasto
				  );
	} else {
		Instruccion.sprintf(" SELECT g.referencia, \
			g.sucursal, \
			g.muuid, \
			g.folioprov, \
			g.fechagas AS fecha, \
			pro.proveedor, \
			pro.razonsocial, \
			SUM(dg.costoimp)-(g.descuento) AS total, \
			SUM(dg.costo) AS subtotal, \
            g.descuento, \
			SUM(dg.iva_trasladado) AS iva_trasladado, \
			SUM(dg.ieps_trasladado) AS ieps_trasladado, \
			SUM(dg.iva_retenido) AS iva_retenido, \
			SUM(dg.isr_retenido) AS isr_retenido, \
			SUM(dg.costo)*i.porcentaje/100 AS ISR_resico \
		FROM gastos g \
		INNER JOIN dgastos dg ON g.referencia=dg.referencia \
		INNER JOIN proveedores pro ON g.proveedor=pro.proveedor \
		INNER JOIN terminales ter ON ter.terminal=g.terminal \
		INNER JOIN secciones sec ON ter.seccion=sec.seccion \
		INNER JOIN sucursales suc ON sec.sucursal=suc.sucursal \
		LEFT JOIN impuestos i on i.impuesto = g.impuestoret \
		WHERE 1 %s \
			%s \
			%s \
			%s \
			%s \
			%s \
			%s \
			%s \
			%s \
			%s \
			%s \
			%s \
			%s \
		GROUP BY g.referencia \
		ORDER BY g.referencia",
	  fecha_nota,
	  condicion_empresa,
	  condicion_sucursal,
	  condicion_canceladas,
	  condicion_terminospago,
	  condicion_proveedor,
	  condicion_tipoGasto,
	  condicion_solicitadopor,
	  condicion_autorizadopor,
	  condicion_foliofactprov,
	  condicion_parterelacion,
	  condicion_numcuenta,
	  condicion_gasto,
	  condicion_foliosistema_pago
	  );
	}

	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), Respuesta->TamBufferResultado);
}
//---------------------------------------------------------------------------
// ID_EJE_REPGASTOS_X_PROVEEDOR
void ServidorGastos::EjecutaRepGastosXProveedor(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    AnsiString instruccion;

    AnsiString fecha_inicial, fecha_final, sucursal, acredito, tipogasto, solicitadopor, autorizadopor;
	AnsiString incluir_devoluciones, fecha_limite_devoluciones, limite_compras, proveedor, factproveedor;
	AnsiString parterelacion;

    AnsiString condicion_sucursal=" ", condicion_acredito=" ", limite_having=" ";
    AnsiString condicion_tipogasto=" ", condicion_solicitadopor=" ", condicion_autorizadopor=" ";
	AnsiString condicion_proveedor=" ", condicion_factproveedor=" ", limite_having_nc=" ", condicion_parterelacion=" ";

    fecha_inicial=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
    fecha_final=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
    sucursal=mFg.ExtraeStringDeBuffer(&parametros);
    acredito=mFg.ExtraeStringDeBuffer(&parametros);
    tipogasto=mFg.ExtraeStringDeBuffer(&parametros);
    solicitadopor=mFg.ExtraeStringDeBuffer(&parametros);
    autorizadopor=mFg.ExtraeStringDeBuffer(&parametros);
    incluir_devoluciones=mFg.ExtraeStringDeBuffer(&parametros);
    fecha_limite_devoluciones=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
    limite_compras=mFg.ExtraeStringDeBuffer(&parametros);
    proveedor=mFg.ExtraeStringDeBuffer(&parametros);
    factproveedor=mFg.ExtraeStringDeBuffer(&parametros);
	parterelacion=mFg.ExtraeStringDeBuffer(&parametros);

    if (sucursal!=" ") {
		condicion_sucursal.sprintf(" AND sec.sucursal='%s' ", sucursal);
    }

    if(limite_compras!="0.00"){
        limite_having.sprintf(" group by prov.proveedor HAVING (total)>=%s order by prov.razonsocial ", limite_compras);
        limite_having_nc.sprintf(" group by t.proveedor HAVING (total)>=%s order by t.nomproveedor ", limite_compras);
    } else {
        limite_having.sprintf(" group by prov.proveedor order by prov.razonsocial ");
        limite_having_nc.sprintf(" group by t.proveedor order by t.nomproveedor ");
    }

    if (acredito!=" ") {
        condicion_acredito.sprintf(" AND g.acredito=%s ", acredito);
    }
    if (tipogasto!=" ") {
        condicion_tipogasto.sprintf(" AND g.tipogasto='%s' ", tipogasto);
    }
    if (solicitadopor!=" ") {
        condicion_solicitadopor.sprintf(" AND g.solicita='%s' ", solicitadopor);
    }
    if (autorizadopor!=" ") {
        condicion_autorizadopor.sprintf(" AND g.autoriza='%s' ", autorizadopor);
    }
    if (proveedor!=" ") {
        condicion_proveedor.sprintf(" AND g.proveedor='%s' ", proveedor);
    }
    if (factproveedor!=" ") {
        condicion_factproveedor.sprintf(" AND g.folioprov='%s' ", factproveedor);
    }
	if (parterelacion!=" ") {
		condicion_parterelacion.sprintf(" AND prov.esparterelac='%s' ", parterelacion);
	}

    mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_REP);

    if (incluir_devoluciones=="1") {
        instruccion.sprintf("SELECT \
            t.proveedor, \
            t.nomproveedor, \
            t.rfc, \
            t.curp, \
            t.calle, \
            t.nomcolonia, \
            t.nommunicipio, \
            t.nomestado, \
            t.pais, \
            t.cp, \
            t.tel, \
            SUM(t.total) as total, \
            SUM(t.subtotal) as subtotal, \
			SUM(t.iva_trasladado) as iva_trasladado, \
			SUM(t.ieps_trasladado) as ieps_trasladado, \
			SUM(t.iva_retenido) as iva_retenido, \
			SUM(t.isr_retenido) as isr_retenido, \
			SUM(t.ISR_Resico) as ISR_Resico \
		FROM (( \
            SELECT \
                prov.proveedor, \
                prov.razonsocial AS nomproveedor, \
                prov.rfc, \
                prov.curp, \
                prov.calle, \
                prov.colonia AS nomcolonia, \
                prov.localidad AS nommunicipio, \
                prov.estado AS nomestado, \
                prov.pais, \
                prov.cp, \
				telprov.tel, \
				(SUM(d.costoimp)-n.descuento)*-1 AS total, \
				SUM(d.costo)*-1 AS subtotal, \
				SUM(d.iva_trasladado)*-1 AS iva_trasladado, \
				SUM(d.ieps_trasladado)*-1 AS ieps_trasladado, \
				SUM(d.iva_retenido)*-1 AS iva_retenido, \
				SUM(d.isr_retenido)*-1 AS isr_retenido, \
				(SUM(d.costo*i.porcentaje)/100)*-1 AS ISR_Resico \
            FROM notascredgasto n \
            INNER JOIN dnotascredgasto d ON n.referencia=d.referencia \
            INNER JOIN gastos g ON n.gasto=g.referencia \
            INNER JOIN proveedores prov ON g.proveedor=prov.proveedor \
            INNER JOIN terminales ter ON ter.terminal=g.terminal \
            INNER JOIN secciones sec ON ter.seccion=sec.seccion \
            LEFT JOIN \
            (SELECT (count(prov.proveedor)) AS numtels, \
                    concat(tel.lada,' ',tel.telefono) AS tel, \
                    prov.proveedor \
             FROM proveedores prov, \
                  telefonosproveedores tel \
             WHERE prov.proveedor=tel.proveedor \
             GROUP BY prov.proveedor) telprov ON telprov.proveedor=prov.proveedor \
             LEFT JOIN impuestos i on i.impuesto = n.impuestoret \
            WHERE g.fechagas>='%s' \
            AND g.fechagas<='%s'  \
            %s \
            %s \
            %s \
            %s \
            %s \
            %s \
            %s \
            %s \
            AND g.cancelado=0 \
			AND n.cancelado=0 \
			AND n.fechanot<='%s' \
            group by prov.proveedor order by prov.razonsocial \
        ) UNION ALL ( \
            SELECT \
                prov.proveedor, \
                prov.razonsocial AS nomproveedor, \
				prov.rfc, \
                prov.curp, \
                prov.calle, \
                prov.colonia AS nomcolonia, \
                prov.localidad AS nommunicipio, \
                prov.estado AS nomestado, \
                prov.pais, \
                prov.cp, \
                telprov.tel, \
				SUM(dg.costoimp)-(g.descuento) AS total, \
                SUM(dg.costo) AS subtotal, \
				SUM(dg.iva_trasladado) AS iva_trasladado, \
				SUM(dg.ieps_trasladado) AS ieps_trasladado, \
				SUM(dg.iva_retenido) AS iva_retenido, \
				SUM(dg.isr_retenido) AS isr_retenido, \
				(SUM(dg.costo*i.porcentaje)/100) AS ISR_Resico \
			FROM gastos g \
            INNER JOIN dgastos dg ON g.referencia=dg.referencia \
            INNER JOIN proveedores prov ON g.proveedor=prov.proveedor \
            INNER JOIN terminales ter ON ter.terminal=g.terminal \
            INNER JOIN secciones sec ON ter.seccion=sec.seccion \
            LEFT JOIN \
            (SELECT (count(prov.proveedor)) AS numtels, \
                    concat(tel.lada,' ',tel.telefono) AS tel, \
                    prov.proveedor \
             FROM proveedores prov, \
                  telefonosproveedores tel \
             WHERE prov.proveedor=tel.proveedor \
			 GROUP BY prov.proveedor) telprov ON telprov.proveedor=prov.proveedor \
             LEFT JOIN impuestos i on i.impuesto = g.impuestoret \
            WHERE g.fechagas>='%s' \
            AND g.fechagas<='%s'  \
            %s \
            %s \
            %s \
            %s \
            %s \
            %s \
            %s \
			%s \
            AND g.cancelado=0 \
            group by prov.proveedor order by prov.razonsocial \
        ) ) t %s ",
        fecha_inicial,
        fecha_final,
        condicion_sucursal,
        condicion_acredito,
        condicion_tipogasto,
        condicion_solicitadopor,
        condicion_autorizadopor,
        condicion_proveedor,
		condicion_factproveedor,
		condicion_parterelacion,
		fecha_limite_devoluciones,

        fecha_inicial,
        fecha_final,
        condicion_sucursal,
        condicion_acredito,
        condicion_tipogasto,
        condicion_solicitadopor,
        condicion_autorizadopor,
        condicion_proveedor,
        condicion_factproveedor,
		condicion_parterelacion,

        limite_having_nc);
    } else {
        instruccion.sprintf("SELECT \
            prov.proveedor, \
            prov.razonsocial AS nomproveedor, \
            prov.rfc, \
            prov.curp, \
            prov.calle, \
            prov.colonia AS nomcolonia, \
            prov.localidad AS nommunicipio, \
            prov.estado AS nomestado, \
            prov.pais, \
			prov.cp, \
            telprov.tel, \
			SUM(dg.costoimp)-g.descuento AS total, \
			SUM(dg.costo) AS subtotal, \
			SUM(dg.iva_trasladado) AS iva_trasladado, \
			SUM(dg.ieps_trasladado) AS ieps_trasladado, \
			SUM(dg.iva_retenido) AS iva_retenido, \
			SUM(dg.isr_retenido) AS isr_retenido, \
			(SUM(dg.costo*i.porcentaje)/100) AS ISR_Resico \
        FROM gastos g \
        INNER JOIN dgastos dg ON g.referencia=dg.referencia \
        INNER JOIN proveedores prov ON g.proveedor=prov.proveedor \
        INNER JOIN terminales ter ON ter.terminal=g.terminal \
        INNER JOIN secciones sec ON ter.seccion=sec.seccion \
        LEFT JOIN \
        (SELECT (count(prov.proveedor)) AS numtels, \
                concat(tel.lada,' ',tel.telefono) AS tel, \
                prov.proveedor \
         FROM proveedores prov, \
              telefonosproveedores tel \
         WHERE prov.proveedor=tel.proveedor \
         GROUP BY prov.proveedor) telprov ON telprov.proveedor=prov.proveedor \
		LEFT JOIN impuestos i on i.impuesto = g.impuestoret \
        WHERE g.fechagas>='%s' \
        AND g.fechagas<='%s'  \
        %s \
        %s \
        %s \
        %s \
        %s \
        %s \
		%s \
		%s \
        AND g.cancelado=0 \
		%s ",
		fecha_inicial,
        fecha_final,
        condicion_sucursal,
        condicion_acredito,
        condicion_tipogasto,
        condicion_solicitadopor,
        condicion_autorizadopor,
        condicion_proveedor,
        condicion_factproveedor,
		condicion_parterelacion,
        limite_having);
    }

    mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}
//------------------------------------------------------------------------------
