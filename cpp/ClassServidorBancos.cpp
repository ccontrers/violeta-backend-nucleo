//---------------------------------------------------------------------------
#include <vcl.h>
#include "pch.h"

#pragma hdrstop

#include <DateUtils.hpp>
#include "ClassServidorBancos.h"
#include "ClassServidorVioleta.h"
#include "ClassBufferRespuestas.h"
#include "violetaS.h"
#include "FormServidorVioleta.h"
#include "comunes.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

// --------------------------------------------------------------------------

/***************************************************************************/
/***************************BANCOS X PAGOS EN EFECTIVO**********************/
/***************************************************************************/

//ID_GRA_BANCOSPAGOSEFE
void ServidorBancos::GrabaBancoPagosefe(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  GRABA BANCO X PAGOS EN EFECTIVO
	char *buffer_sql=new char[1024*10000];
	char *aux_buffer_sql=buffer_sql;
	AnsiString tarea_rel, folio_rel, cancelado;
	AnsiString sucursal, total, fechapagos, fecha_alta, hora_alta, fecha_mod, hora_mod;
	AnsiString usuario_alta,usuario_mod, terminal;
	int num_partidas, i, error;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[50000];
	int num_movs = 0, indice_movs = 0, indice_pagos = 0;
	AnsiString idmovbanco_param;
	AnsiString totalpagos;
	double acumula_totalpagos=0.0;
	AnsiString cadena_movimientos="";
	TDate fecha=Today();
	TTime hora=Time();
	AnsiString valorcomp, movbancarios, mensaje;

	try {
		folio_rel=mFg.ExtraeStringDeBuffer(&parametros); // Folio del movimiento bancario.
		tarea_rel=mFg.ExtraeStringDeBuffer(&parametros); // Indicador si se va a agregar o modificar.
		sucursal=mFg.ExtraeStringDeBuffer(&parametros); //sucursal
		fechapagos=mFg.ExtraeStringDeBuffer(&parametros); //Fecha de ventas
		fecha_alta=mFg.ExtraeStringDeBuffer(&parametros); //Fecha de alta del movimiento
		hora_alta=mFg.ExtraeStringDeBuffer(&parametros); //Hora de alta del movimiento
		fecha_mod=mFg.ExtraeStringDeBuffer(&parametros); //Fecha de modificación del movimiento
		hora_mod=mFg.ExtraeStringDeBuffer(&parametros); //Hora de modificación del movimiento
		usuario_alta=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que esta grabando el movimiento.
		usuario_mod=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que esta modificando el movimiento.
		total=mFg.ExtraeStringDeBuffer(&parametros); //Total
		totalpagos=mFg.ExtraeStringDeBuffer(&parametros); //Total  de las ventas
		cancelado=mFg.ExtraeStringDeBuffer(&parametros); //Si esta cancelada la cuenta
		movbancarios=mFg.ExtraeStringDeBuffer(&parametros); //Movimientos bancarios
		valorcomp=mFg.ExtraeStringDeBuffer(&parametros); //Diferencia entre movimientos y pagos
		num_partidas=StrToInt(mFg.ExtraeStringDeBuffer(&parametros)); //Número de partidas

		if (num_partidas<=0)
			throw (Exception("Debe existir al menos un movimiento bancario en la relación"));

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		// Ingresa el movimiento
		if (tarea_rel=="A") {
			instruccion.sprintf("INSERT INTO bancospagosefedef (sucursal, fechapagos, fechaalta, horaalta, \
				fechamodi, horamodi, usualta, usumodi, total, totalpagos, cancelado )  \
				VALUES ('%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s')",
				sucursal, mFg.StrToMySqlDate(fechapagos), mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora),
				mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario_alta, usuario_mod,
				total, totalpagos, cancelado);
			instrucciones[num_instrucciones++]=instruccion;

			instrucciones[num_instrucciones++]="set @folio:=LAST_INSERT_ID()";
		} else {
			//Se cancelan los movimientos de la relación para luego dar de alta los reemplazos
			instruccion.sprintf("UPDATE bancosmov bm INNER JOIN bancosxpagosefe bv ON \
				bm.idmovbanco = bv.idmovbanco  \
				SET bm.cancelado = '1' WHERE bv.idbancospagosefe = '%s'", folio_rel);
			instrucciones[num_instrucciones++]=instruccion;

			// Se borran las relaciones entre movimientos y pagos en efectivos (para luego agregar las nuevas)
			instruccion.sprintf("delete from bancosxpagosefe where idbancospagosefe='%s'",folio_rel);
			instrucciones[num_instrucciones++]=instruccion;

			// Se actualizan los datos de la relacion (tabla bancospagosefedef)
			instruccion.sprintf("update bancospagosefedef set sucursal = '%s', fechapagos = '%s', \
				fechamodi = '%s',horamodi = '%s', usumodi = '%s', total = '%s' \
				, totalpagos = '%s' where idbancospagosefe = '%s' ",
				sucursal, mFg.StrToMySqlDate(fechapagos),  mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora),
				usuario_mod, total, totalpagos, folio_rel);
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("set @folio:=%s",folio_rel);
			instrucciones[num_instrucciones++]=instruccion;
		}


		// Aplica los nuevos movimientos.
		for (i=0; i<num_partidas; i++) {
			//Ingresa los movimientos asignados a arreglos
			idmovbanco_param =  mFg.ExtraeStringDeBuffer(&parametros);

			// Se inserta las nuevas relaciones entre movimientos y pagos en efectivos
			instruccion.sprintf("INSERT INTO bancosxpagosefe VALUES (@folio,'%s')", idmovbanco_param);
			instrucciones[num_instrucciones++] = instruccion;

			/*Al ingresar los movimientos los aplica poniendo 1 en el campo de aplicado*/
			instruccion.sprintf("update bancosmov set aplicado = '1', cancelado = '0' \
				where idmovbanco = '%s' ", idmovbanco_param);
			instrucciones[num_instrucciones++] = instruccion;

			// Quita los registros de bancosxcob porque se volverán a distribuir.
			instruccion.sprintf("DELETE bxc FROM bancosxcob bxc \
				INNER JOIN bancostransacc bt ON bxc.transacc=bt.transacc \
				WHERE bt.idmovbanco=%s", idmovbanco_param);
			instrucciones[num_instrucciones++] = instruccion;

			// Crea una cadena con la lista de movimientos separados por coma para usarla en un select posterior.
			if (i>0) cadena_movimientos=cadena_movimientos+",";
			cadena_movimientos=cadena_movimientos+idmovbanco_param;

			num_movs++;
		}

		//Busqeda del parametro para establecer limite de diferencia en la relación
		AnsiString tolerdifrel;
		BufferRespuestas* resp_limite=NULL;
		try{
			instruccion = "SELECT valor FROM parambancos WHERE parametro =  'TOLERDIFREL'";
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_limite);
			tolerdifrel =  resp_limite->ObtieneDato("valor");
		}
		__finally {
			if (resp_limite!=NULL) delete resp_limite;
		}


		// ************  INICIO DE DISTRIBUYE los pagos en efectivo en los movimientos
		BufferRespuestas* resp_pagos=NULL;
		BufferRespuestas* resp_bancostransacc=NULL;
		try {

			// Consulta los pagos
			instruccion.sprintf("SELECT txc.tracredito, (txc.valor*-1) as valor \
				FROM transxcob txc \
					INNER JOIN pagoscli p ON txc.pago=p.pago \
					INNER JOIN ventas v ON txc.referencia=v.referencia \
					INNER JOIN terminales t ON t.terminal=v.terminal \
					INNER JOIN secciones s ON t.seccion = s.seccion \
				WHERE p.fecha='%s' AND p.cancelado = 0 AND p.formapag='E' AND txc.cancelada=0 AND txc.aplicada=1 AND s.sucursal = '%s' \
					ORDER BY p.pago asc ",
					mFg.StrToMySqlDate(fechapagos), sucursal );
			if (!mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_pagos))
				throw (Exception("Error al consultar los pagos en efectivo"));
			if (resp_pagos->ObtieneNumRegistros()==0)
				throw (Exception("No se encuentran pagos a créditos en efectivo en la fecha: \n" + fechapagos));

		 /*	double comptotalvta = 0.00, valorcomp;
			//Calculo del total de pagos a crédito en efectivo del día
			if (resp_pagos->ObtieneNumRegistros()>0){
						for(int i=0; i<resp_pagos->ObtieneNumRegistros(); i++) {
							comptotalvta = comptotalvta + mFg.CadenaAFlotante(resp_pagos->ObtieneDato("valor"));
							resp_pagos->IrAlSiguienteRegistro();
						}
			}        */

			double difvalor = abs(mFg.CadenaAFlotante(valorcomp));
			if( mFg.CadenaAFlotante(movbancarios) > (mFg.CadenaAFlotante(totalpagos) + mFg.CadenaAFlotante(tolerdifrel)) ){
					mensaje = "El total de los movimientos es mayor que los pagos en efectivo.\n";
					mensaje += "Total de movimientos: " + mFg.FormateaCantidad(movbancarios) + "\n";
					mensaje += "Total de pagos en efectivo: ";
					mensaje +=  mFg.FormateaCantidad(totalpagos) + "\n";
					mensaje += "Diferencia entre total de movimientos y pagos: " + mFg.FormateaCantidad(valorcomp) + "\n";
					mensaje += "\nNo se permite que los movimientos sean mayores a los pagos en efectivo.";
					throw (Exception(mensaje));
			}

		   //	resp_pagos->IrAlPrimerDato();   */
			//Fin de la comprobación de pagos con movimientos

			// Consulta el detalle de los movimientos
			instruccion.sprintf("SELECT bt.transacc, bt.idmovbanco, bt.total FROM bancosmov bm \
				INNER JOIN bancostransacc bt ON bt.idmovbanco=bm.idmovbanco WHERE bm.idmovbanco IN (%s) ",
				cadena_movimientos );
			if (!mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_bancostransacc))
				throw (Exception("Error al consultar los el detalle de los movimientos de la relación"));
			if (resp_bancostransacc->ObtieneNumRegistros()==0)
				throw (Exception("No se encuentran los movimientos asignados a la relación"));


			AnsiString transaccbanco_actual, tracredito_actual;
			double totalbanco_actual, totalbanco_resto, valorpago_actual, valorpago_considerado, valorpago_previamenteplicado=0;

			// Recorre todos los movimientos para aplicarle los pagos que le quepan
			while ( indice_movs<resp_bancostransacc->ObtieneNumRegistros() && indice_pagos<resp_pagos->ObtieneNumRegistros()) {
				resp_bancostransacc->IrAlRegistroNumero(indice_movs);

				transaccbanco_actual=resp_bancostransacc->ObtieneDato("transacc");
				totalbanco_actual=mFg.CadenaAFlotante(resp_bancostransacc->ObtieneDato("total"));
				totalbanco_resto=totalbanco_actual;

				// Aplica los pagos al movimiento actual
				bool parar_ciclo_pagos=false;
				while ( indice_pagos<resp_pagos->ObtieneNumRegistros() && !parar_ciclo_pagos ) {
					resp_pagos->IrAlRegistroNumero(indice_pagos);

					valorpago_actual=mFg.CadenaAFlotante(resp_pagos->ObtieneDato("valor"))-valorpago_previamenteplicado;
					tracredito_actual=resp_pagos->ObtieneDato("tracredito");

					// Determinar cuanto del pago actual se puede aplicar al movimiento
					if (totalbanco_resto>=valorpago_actual) {
						// Se puede aplicar todo del pago actual
						valorpago_considerado=valorpago_actual;
						indice_pagos++; // Solo cuando se aplica todo el pago pasamos al siguiente
						// Para siguiente pago a aplicar inicia con cero aplicacion
						valorpago_previamenteplicado=0;
					} else {
						// Solo se puede aplicar parte del pago actual.
						valorpago_considerado=totalbanco_resto;
						parar_ciclo_pagos=true;
						// Para siguiente pago a aplicar acumula lo que se pudo aplicar en este.
						valorpago_previamenteplicado= valorpago_previamenteplicado+valorpago_considerado;
					}

					instruccion.sprintf("insert into bancosxcob (transacc, tracredito, valorconsiderado) values \
						('%s', '%s', %s)", transaccbanco_actual, tracredito_actual, mFg.FormateaCantidad(valorpago_considerado,2,false) );
					instrucciones[num_instrucciones++] = instruccion;

					totalbanco_resto=totalbanco_resto-valorpago_considerado;
				}

				indice_movs++;
			}

		} __finally {
			if (resp_pagos!=NULL) delete resp_pagos;
			if (resp_bancostransacc!=NULL) delete resp_bancostransacc;
		}
		// ************  FIN DE DISTRIBUYE los pagos en efectivo en los movimientos

		instrucciones[num_instrucciones++]="COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_REP);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			instruccion.sprintf("select %d as error, @folio as folio, '%s' as  fecha", error ,fechapagos);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Consulta los posibles pagos de clientes relacionados.
			instruccion.sprintf("SELECT bt.idmovbanco, bt.transacc, v.referencia, IFNULL(cfd.seriefolio,'') AS seriefolio,IFNULL(cfd.muuid,'') AS muuid, txc.pago, txc.tracredito, bxc.valorconsiderado AS valor, c.rsocial \
				FROM bancosmov b \
				INNER JOIN bancostransacc bt ON bt.idmovbanco=b.idmovbanco \
				INNER JOIN bancosxcob bxc ON bxc.transacc=bt.transacc \
				INNER JOIN transxcob txc ON txc.tracredito=bxc.tracredito \
				INNER JOIN ventas v ON v.referencia=txc.referencia \
				INNER JOIN clientes c ON v.cliente=c.cliente \
				LEFT JOIN cfd ON cfd.tipocomprobante = 'VENT' AND cfd.referencia = v.referencia AND cfd.estado = 1 \
				WHERE b.idmovbanco in \
					( select idmovbanco from bancosxpagosefe where idbancospagosefe=@folio ) ");
			mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	} __finally {
		if(buffer_sql != NULL) delete buffer_sql;
	}
}

//---------------------------------------------------------------------------
//ID_BUSQ_BANCOSPAGOSEFE
void ServidorBancos::BuscaBancoPagosefe(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// BUSQUEDA DE RELACION DE BANCOS CON PAGOS EN EFECTIVO
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instruccion;
	AnsiString tipo_busqueda;
	AnsiString dato_buscado,dato_buscado2, fecha_ini, fecha_fin;
	AnsiString revision_alma_ent, revision_alma_sal;

	// Extrae el tipo de búsqueda
	tipo_busqueda=mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ);

	if (tipo_busqueda=="MOVID"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);

		instruccion = "SELECT idbancospagosefe,sucursal, DATE_FORMAT(fechapagos,'%d-%m-%Y') AS fecha,";
		instruccion += "total, totalpagos, cancelado FROM bancospagosefedef where idbancospagosefe = '";
		instruccion += dato_buscado;
		instruccion += "'";
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="SUCURSAL"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion = "SELECT idbancospagosefe,sucursal, DATE_FORMAT(fechapagos,'%d-%m-%Y') AS fecha,";
		instruccion += "total, totalpagos, cancelado FROM bancospagosefedef where sucursal = '";
		instruccion += dato_buscado;
		instruccion += "' and fechapagos >= '";
		instruccion += fecha_ini;
		instruccion += "' and fechapagos <= '";
		instruccion += fecha_fin;
		instruccion += "'";
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="FECHA"){
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion = "SELECT idbancospagosefe,sucursal, DATE_FORMAT(fechapagos,'%d-%m-%Y') AS fecha,";
		instruccion += "total, totalpagos, cancelado FROM bancospagosefedef where fechapagos >= '";
		instruccion += fecha_ini;
		instruccion += "' and fechapagos <= '";
		instruccion += fecha_fin;
		instruccion += "'";
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="FECHAALTA"){
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion = "SELECT idbancospagosefe,sucursal, DATE_FORMAT(fechapagos,'%d-%m-%Y') AS fecha,";
		instruccion += "total, totalpagos, cancelado FROM bancospagosefedef where fechaalta >= '";
		instruccion += fecha_ini;
		instruccion += "' and fechaalta <= '";
		instruccion += fecha_fin;
		instruccion += "'";
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}

}

//---------------------------------------------------------------------------
//ID_CON_BANCOSPAGOSEFE
void ServidorBancos::ConsultaBancoPagosefe(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA MOVIMIENTO DE ALMACEN
	AnsiString busqueda;
	AnsiString id, modo;

	id=mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Detalle generales del movimiento
	busqueda = "SELECT idbancospagosefe,sucursal,fechapagos, fechaalta, total, cancelado FROM bancospagosefedef ";
	busqueda += "WHERE idbancospagosefe = '";
	busqueda +=	id ;
	busqueda += "'";
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, busqueda.c_str(), Respuesta->TamBufferResultado);

	// Detalle del movimiento
	busqueda = "SELECT bm.idmovbanco,bc.banco, bc.numerocuenta, ";
	busqueda += "bcm.descripcion AS conceptomov, ";
	busqueda += "bm.total, DATE_FORMAT(bm.fechaaplbanco,'%d-%m-%Y') as fecha ";
	busqueda += "FROM bancosxpagosefe bxv INNER JOIN bancosmov bm ON bm.idmovbanco = bxv.idmovbanco ";
	busqueda += "INNER JOIN bancoscuentas bc ON bc.idnumcuenta = bm.idnumcuenta ";
	busqueda += "INNER JOIN  bancosconceptomov  bcm ON bcm.conceptomov = bm.conceptomov WHERE bxv.idbancospagosefe = '";
	busqueda +=	id ;
	busqueda += "' ORDER BY bm.idmovbanco";
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, busqueda.c_str(), Respuesta->TamBufferResultado);

	// Obtiene todos los pagos del movimiento.
	busqueda.sprintf("SELECT bt.idmovbanco, bt.transacc, v.referencia, IFNULL(cfd.seriefolio,'') AS seriefolio,IFNULL(cfd.muuid,'') AS muuid, txc.pago, txc.tracredito, bxc.valorconsiderado AS valor, c.rsocial \
				FROM bancosmov b \
				INNER JOIN bancostransacc bt ON bt.idmovbanco=b.idmovbanco \
				INNER JOIN bancosxcob bxc ON bxc.transacc=bt.transacc \
				INNER JOIN transxcob txc ON txc.tracredito=bxc.tracredito \
				INNER JOIN ventas v ON v.referencia=txc.referencia \
				INNER JOIN clientes c ON v.cliente=c.cliente \
				LEFT JOIN cfd ON cfd.tipocomprobante = 'VENT' AND cfd.referencia = v.referencia AND cfd.estado = 1 \
				WHERE b.idmovbanco in \
			( select idmovbanco from bancosxpagosefe where idbancospagosefe=%s ) ", id);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, busqueda.c_str(), Respuesta->TamBufferResultado);
}

//----------------------------------------------------------------------------
//ID_CANC_BANCOSPAGOSEFE
void ServidorBancos::CancelaBancoPagosefe(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  CANCELA RELACION DE BANCOS CON PAGOS EN EFECTIVO
	char *buffer_sql=new char[1024*32];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instrucciones[30], instruccion;
	int num_instrucciones=0;
	AnsiString id;
	int error=0;
	int i;


	try{
		id=mFg.ExtraeStringDeBuffer(&parametros); // ID de la relación

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		// Cancela la venta
		instruccion.sprintf("update bancospagosefedef set cancelado=1 where idbancospagosefe='%s' and cancelado=0",id);
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("UPDATE bancosmov bm \
			INNER JOIN bancosxpagosefe bv ON bm.idmovbanco = bv.idmovbanco \
			SET bm.cancelado = '1' WHERE bv.idbancospagosefe = '%s'", id);
		instrucciones[num_instrucciones++]=instruccion;

		instrucciones[num_instrucciones++]="COMMIT";

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

//----------------------------------------------------------------------------
//ID_CON_REPPAGOSEFEMOVBANC
void ServidorBancos::RepRelacionPagosefeMovimientosBancarios(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros){
  /*alguna inofrmacion*/

	char *buffer_sql=new char[1024*64*10];
	AnsiString instruccion;


	AnsiString fecha_inicial, fecha_final, sucursal;
	AnsiString condicion_sucursal=" ";


	try{
		fecha_inicial=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_final=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		sucursal=mFg.ExtraeStringDeBuffer(&parametros);



		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_REP);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {

			// Obtiene todos los datos de los proveedores
			/*instruccion.sprintf("SELECT * FROM bancospagosefedef bped \
								WHERE  bped.fechapagos>='%s' AND bped.fechapagos<='%s' \
								%s  and bped.cancelado=0 \
								ORDER BY bped.fechapagos ASC, bped.sucursal ASC ",fecha_inicial,
								fecha_final,condicion_sucursal.c_str() ); */

			instruccion.sprintf("SELECT p.*,SUM(t.valor*-1) AS subtotal, s.sucursal, \
					IFNULL( auxpagos.idbancospagosefe,0) AS idbancospagosefe,        \
					IFNULL(auxpagos.totalpagos,SUM((t.valor*-1))) AS totalpagos,     \
					IFNULL(auxpagos.total,0) AS total                                \
					FROM transxcob t                                                 \
					INNER JOIN  pagoscli p                                           \
					LEFT JOIN ventas v ON v.cancelado=0 AND v.referencia=t.referencia\
					INNER JOIN terminales ter ON v.terminal=ter.terminal             \
					INNER JOIN secciones sec ON ter.seccion=sec.seccion              \
					INNER JOIN  sucursales s ON s.sucursal =sec.sucursal             \
					LEFT JOIN clientes cli ON cli.cliente=v.cliente                  \
					LEFT JOIN cheqxcob chxc ON chxc.pago=p.pago                      \
					LEFT JOIN chequesclientes chc ON chc.chequecli=chxc.chequecli    \
					LEFT JOIN chequesclasif chclasif ON chclasif.clasif=chc.clasif   \
					LEFT JOIN bancos b ON b.banco=chc.banco                          \
					LEFT JOIN cfd ON v.referencia=cfd.referencia AND cfd.tipocomprobante='VENT' \
						LEFT JOIN (SELECT * FROM bancospagosefedef bped                  \
						WHERE  bped.fechapagos>='%s' AND bped.fechapagos<='%s' \
						AND bped.sucursal='%s'  AND bped.cancelado=0                     \
						GROUP BY bped.sucursal, bped.fechapagos                          \
						)AS auxpagos ON auxpagos.sucursal=s.sucursal AND auxpagos.fechapagos=p.fecha \
					WHERE  p.fecha>='%s' AND p.fecha<='%s'AND p.pago=t.pago   \
					AND t.cancelada=0 AND p.cancelado=0 AND p.formapag='E' AND s.sucursal='%s'\
					GROUP BY s.sucursal,p.fecha",fecha_inicial,fecha_final,sucursal,
					fecha_inicial,fecha_final,sucursal);

			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	   }
	}__finally{
		//if (resp_vtamovbancarios!=NULL) delete resp_vtamovbancarios;
		delete buffer_sql;
	}
}

//----------------------------------------------------------------------------
//ID_GRA_MOVBAN
void ServidorBancos::GrabaMovBanco(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  GRABA MOVIMIENTO BANCARIO
	char *buffer_sql=new char[1024*300];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString origen;
	AnsiString tarea_banco, folio_banco, id_num_cuenta, banco, tipo, descripcion, identificador,cancelado, aplicado;
	AnsiString subtotal, iva, total, fechaaplban, fecha_alta, hora_alta, fecha_mod, hora_mod;
	AnsiString usuario_alta,usuario_mod, terminal, sucursal, afectacion,EliminarTransacBanco;
	int num_partidas, i, error;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[300];
    TDate fecha=Today();
	TTime hora=Time();

	try {
		EliminarTransacBanco=mFg.ExtraeStringDeBuffer(&parametros); // Limitar Actualización
		folio_banco=mFg.ExtraeStringDeBuffer(&parametros); // Folio del movimiento bancario.
		tarea_banco=mFg.ExtraeStringDeBuffer(&parametros); // Indicador si se va a agregar o modificar.
		id_num_cuenta=mFg.ExtraeStringDeBuffer(&parametros); //Id numero de cuenta
		tipo=mFg.ExtraeStringDeBuffer(&parametros); //Tipo principal
		afectacion=mFg.ExtraeStringDeBuffer(&parametros); //Concepto
		descripcion=mFg.ExtraeStringDeBuffer(&parametros); //Descripcion principal
		identificador=mFg.ExtraeStringDeBuffer(&parametros); //Identificador principal
		origen=mFg.ExtraeStringDeBuffer(&parametros); // Origen
		cancelado=mFg.ExtraeStringDeBuffer(&parametros); //Si esta cancelada la cuenta
		aplicado=mFg.ExtraeStringDeBuffer(&parametros); //Si esta aplicado el movimiento
		subtotal=mFg.ExtraeStringDeBuffer(&parametros); //Subtotal
		iva=mFg.ExtraeStringDeBuffer(&parametros); //IVA
		total=mFg.ExtraeStringDeBuffer(&parametros); //Total
		fechaaplban=mFg.ExtraeStringDeBuffer(&parametros); //Fecha de aplicacón del banco
		fecha_alta=mFg.ExtraeStringDeBuffer(&parametros); //Fecha de alta del movimiento
		hora_alta=mFg.ExtraeStringDeBuffer(&parametros); //Hora de alta del movimiento
		fecha_mod=mFg.ExtraeStringDeBuffer(&parametros); //Fecha de modificación del movimiento
		hora_mod=mFg.ExtraeStringDeBuffer(&parametros); //Hora de modificación del movimiento
		usuario_alta=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que esta grabando el movimiento.
		usuario_mod=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que esta modificando el movimiento.
		terminal=mFg.ExtraeStringDeBuffer(&parametros); // Terminal desde la que se está grabando la compra.
		sucursal=mFg.ExtraeStringDeBuffer(&parametros); //Sucursal
		num_partidas=StrToInt(mFg.ExtraeStringDeBuffer(&parametros)); //Número de partidas

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

			// Ingresa el movimiento
			if (tarea_banco=="A") {
				instruccion.sprintf("insert into bancosmov \
				(idmovbanco, idnumcuenta, conceptomov, afectacion, descripcion, identificador, cancelado, \
				 aplicado, subtotal, ivabanco,  total, fechaaplbanco, fechaalta, horaalta, fechamodi, \
				 horamodi, usualta, usumodi, terminal, origen, sucursal)  	\
				values (NULL, %s, '%s','%s', '%s', '%s', %s, %s, '%s', '%s', \
				'%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s','%s','%s' )",
				id_num_cuenta, tipo, afectacion, descripcion, identificador, cancelado, aplicado
				,subtotal, iva, total, mFg.StrToMySqlDate(fechaaplban)
				,mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora),mFg.DateToMySqlDate(fecha),
				mFg.TimeToMySqlTime(hora), usuario_alta, usuario_mod, terminal, origen, sucursal);
				instrucciones[num_instrucciones++]=instruccion;

				instrucciones[num_instrucciones++]="set @folio:=LAST_INSERT_ID()";
			} else {
					instruccion.sprintf("update bancosmov set idnumcuenta = %s, conceptomov = '%s', afectacion = '%s', \
					descripcion = '%s', identificador = '%s',cancelado = %s, aplicado = %s, subtotal = '%s', ivabanco = '%s', \
					total = '%s', fechamodi = '%s', horamodi = '%s', \
					usumodi = '%s', terminal = '%s', origen = '%s', sucursal = '%s' \
					where idmovbanco = '%s' ",
					id_num_cuenta, tipo, afectacion, descripcion, identificador, cancelado, aplicado, subtotal, iva, total,
					mFg.DateToMySqlDate(fecha),mFg.TimeToMySqlTime(hora), usuario_mod, terminal,
					origen, sucursal, folio_banco);

					instrucciones[num_instrucciones++]=instruccion;

					instruccion.sprintf("set @folio:=%s",folio_banco);
					instrucciones[num_instrucciones++]=instruccion;

					if(EliminarTransacBanco=="1")
					{
						instruccion.sprintf("delete from bancostransacc where idmovbanco='%s'",folio_banco);
						instrucciones[num_instrucciones++]=instruccion;
					}

				}


			if(EliminarTransacBanco=="1")
			{

				for (i=0; i<num_partidas; i++) {

					datos.AsignaTabla("bancostransacc");
					parametros+=datos.InsCamposDesdeBuffer(parametros);
					datos.InsCampo("idmovbanco", "@folio", 1);
					instrucciones[num_instrucciones++]=datos.GenerarSqlInsert();
				}

			}


			instrucciones[num_instrucciones++]="COMMIT";

			// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			instruccion.sprintf("select %d as error, @folio as folio, '%s' as fecha", error,fechaaplban);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}

	} __finally {
		delete buffer_sql;
	}
}

//---------------------------------------------------------------------------
//ID_BUSQ_MOVBAN
void ServidorBancos::BuscaMovBanco(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// BUSQUEDA DE MOVIMIENTOS BANCARIOS
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instruccion;
	AnsiString tipo_busqueda;
	AnsiString dato_buscado,dato_buscado2, fecha_ini, fecha_fin, origen, aplicados;
	AnsiString revision_alma_ent, revision_alma_sal, condicion_origen = " ", condicion_aplicados;

	// Extrae el tipo de búsqueda
	tipo_busqueda=mFg.ExtraeStringDeBuffer(&parametros);    //Tipo de busqueda
	origen=mFg.ExtraeStringDeBuffer(&parametros);    //Origen
	aplicados=mFg.ExtraeStringDeBuffer(&parametros);    //Si solo los aplicados o solo los no aplicados.
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ);

	if(origen != "")
		condicion_origen.sprintf(" AND bm.origen = '%s' ", origen);

	condicion_aplicados.sprintf(" AND bm.aplicado = %s ", aplicados);

	if (tipo_busqueda=="MOVID"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);

		instruccion.sprintf("SELECT bm.idmovbanco, bc.banco, bc.numerocuenta,bm.conceptomov, \
		IF(bm.afectacion = 'C','CARGO',IF(bm.afectacion = 'A','ABONO','NA')) AS afectacion, \
		IFNULL(bm.identificador,'') as identificador, IFNULL(bm.descripcion,'') as descripcion, bm.subtotal, \
		bm.ivabanco, bm.total, bm.origen, bm.sucursal, bm.cancelado FROM bancosmov bm INNER JOIN bancoscuentas bc \
		ON bm.idnumcuenta = bc.idnumcuenta WHERE bm.idmovbanco = '%s' %s %s ", dato_buscado, condicion_origen, condicion_aplicados);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="IDENTIFICADOR"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("SELECT bm.idmovbanco, bc.banco, bc.numerocuenta,bm.conceptomov, \
		IF(bm.afectacion = 'C','CARGO',IF(bm.afectacion = 'A','ABONO','NA')) AS afectacion, \
		IFNULL(bm.identificador,'') as identificador, IFNULL(bm.descripcion,'') as descripcion, bm.subtotal, \
		bm.ivabanco, bm.total, bm.origen, bm.sucursal, bm.cancelado FROM bancosmov bm INNER JOIN bancoscuentas bc \
		ON bm.idnumcuenta = bc.idnumcuenta WHERE bm.identificador LIKE '%s%%' and bm.fechaaplbanco>='%s' \
		and bm.fechaaplbanco<='%s' %s %s ",
		dato_buscado,fecha_ini, fecha_fin, condicion_origen, condicion_aplicados);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="NUMCTA"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("SELECT bm.idmovbanco, bc.banco, bc.numerocuenta,bm.conceptomov, \
		IF(bm.afectacion = 'C','CARGO',IF(bm.afectacion = 'A','ABONO','NA')) AS afectacion, \
		IFNULL(bm.identificador,'') as identificador, IFNULL(bm.descripcion,'') as descripcion, bm.subtotal, \
		bm.ivabanco, bm.total, bm.origen, bm.sucursal, bm.cancelado FROM bancosmov bm INNER JOIN bancoscuentas bc \
		ON bm.idnumcuenta = bc.idnumcuenta WHERE bm.idnumcuenta = '%s' and bm.fechaaplbanco>='%s' \
		and bm.fechaaplbanco<='%s' %s %s ", dato_buscado, fecha_ini, fecha_fin, condicion_origen, condicion_aplicados);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="TIPO"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("SELECT bm.idmovbanco, bc.banco, bc.numerocuenta,bm.conceptomov, \
		IF(bm.afectacion = 'C','CARGO',IF(bm.afectacion = 'A','ABONO','NA')) AS afectacion, \
		IFNULL(bm.identificador,'') as identificador, IFNULL(bm.descripcion,'') as descripcion, bm.subtotal, \
		 bm.ivabanco, bm.total, bm.origen, bm.sucursal, bm.cancelado FROM bancosmov bm INNER JOIN bancoscuentas bc \
		ON bm.idnumcuenta = bc.idnumcuenta WHERE bm.conceptomov = '%s' and bm.fechaaplbanco>='%s' \
		and bm.fechaaplbanco<='%s' %s %s ", dato_buscado, fecha_ini, fecha_fin, condicion_origen, condicion_aplicados);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="FECHAAPL"){
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("SELECT bm.idmovbanco, bc.banco, bc.numerocuenta,bm.conceptomov, \
		IF(bm.afectacion = 'C','CARGO',IF(bm.afectacion = 'A','ABONO','NA')) AS afectacion, \
		IFNULL(bm.identificador,'') as identificador, IFNULL(bm.descripcion,'') as descripcion, bm.subtotal, \
		bm.ivabanco, bm.total, bm.origen, bm.sucursal, bm.cancelado FROM bancosmov bm INNER JOIN bancoscuentas bc \
		ON bm.idnumcuenta = bc.idnumcuenta WHERE bm.fechaaplbanco>='%s' \
		and bm.fechaaplbanco<='%s' %s %s ", fecha_ini, fecha_fin, condicion_origen, condicion_aplicados);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="FECHALTA"){
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("SELECT bm.idmovbanco, bc.banco, bc.numerocuenta,bm.conceptomov, \
		IF(bm.afectacion = 'C','CARGO',IF(bm.afectacion = 'A','ABONO','NA')) AS afectacion, \
		IFNULL(bm.identificador,'') as identificador, IFNULL(bm.descripcion,'') as descripcion, bm.subtotal, \
		bm.ivabanco, bm.total, bm.origen, bm.sucursal, bm.cancelado FROM bancosmov bm INNER JOIN bancoscuentas bc \
		ON bm.idnumcuenta = bc.idnumcuenta WHERE bm.fechaalta>='%s' \
		and bm.fechaalta<='%s' %s %s ", fecha_ini, fecha_fin, condicion_origen, condicion_aplicados);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="ORIGEN"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("SELECT bm.idmovbanco, bc.banco, bc.numerocuenta,bm.conceptomov, \
		IF(bm.afectacion = 'C','CARGO',IF(bm.afectacion = 'A','ABONO','NA')) AS afectacion, \
		IFNULL(bm.identificador,'') as identificador, IFNULL(bm.descripcion,'') as descripcion, bm.subtotal, \
		bm.ivabanco, bm.total, bm.origen, bm.sucursal, bm.cancelado FROM bancosmov bm INNER JOIN bancoscuentas bc \
		ON bm.idnumcuenta = bc.idnumcuenta WHERE bm.origen = '%s' and bm.fechaaplbanco>='%s' \
		and bm.fechaaplbanco<='%s' %s ", dato_buscado, fecha_ini, fecha_fin, condicion_aplicados);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}


}

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//ID_CON_MOVBAN
void ServidorBancos::ConsultaMovimientoBancos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    // CONSULTA MOVIMIENTO DE ALMACEN
	AnsiString instruccion;
	AnsiString id;

	id=mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los generales (de cabecera) del movimiento
	instruccion.sprintf("SELECT bm.idnumcuenta, bm.idmovbanco, bc.banco, bc.numerocuenta,bm.conceptomov, bm.afectacion, \
	IFNULL(bm.identificador,'') as identificador, IFNULL(bm.descripcion,'') as descripcion, bm.subtotal, \
	bm.ivabanco, bm.total, bm.cancelado, bm.fechaaplbanco, bm.origen, bm.sucursal, bm.fechaalta \
	FROM bancosmov bm INNER JOIN bancoscuentas bc \
	ON bm.idnumcuenta = bc.idnumcuenta WHERE bm.idmovbanco = '%s'", id);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene todo el detalle del movimiento.
	instruccion.sprintf("SELECT bt.tipodet, bt.identificador, \
		bt.subtotal, bt.ivabanco, bt.total, \
		i.tipoimpu,i.porcentaje, bt.cveimp FROM bancostransacc bt \
		INNER JOIN impuestos i ON bt.cveimp = i.impuesto WHERE bt.idmovbanco = '%s'", id );
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Consulta las posibles ventas relacionadas.
	instruccion = "SELECT b.venta, b.valorconsiderado,c.rsocial ";
	instruccion += ",IFNULL(cfd.muuid,'') AS muuid, IFNULL(cfd.seriefolio,'') AS seriefolio, ";
	instruccion += "IFNULL(cfd2.seriefolio,'') AS seriefolio2, IFNULL(cfd2.muuid,'') AS muuid2 ";
	instruccion += "FROM bancosdventas b ";
	instruccion += "INNER JOIN ventas v ON v.referencia=b.venta ";
	instruccion += "INNER JOIN clientes c ON v.cliente=c.cliente ";
	instruccion += "INNER JOIN terminales t ON t.terminal = v.terminal ";
	instruccion += "INNER JOIN secciones s ON t.seccion = s.seccion ";
	instruccion += "LEFT JOIN cfd ON cfd.tipocomprobante = 'VENT' AND cfd.estado = 1 ";
	instruccion += "AND cfd.referencia = b.venta AND v.ticket=0  ";
	instruccion += "LEFT JOIN cfd cfd2 ON cfd2.estado = 1 AND v.ticket=1 ";
	instruccion += "AND DATE_FORMAT(v.fechavta,'%d/%m/%Y') = cfd2.referencia ";
	instruccion += "AND s.sucursal = cfd2.sucursal AND cfd2.tipocomprobante='TICK' ";
	instruccion += "WHERE b.idmovbanco = '";
	instruccion += id;
	instruccion += "' order by b.venta";
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Consulta los posibles pagos de clientes relacionados.
	instruccion.sprintf("SELECT v.referencia, txc.pago, txc.tracredito, bxc.valorconsiderado AS valor, \
		c.rsocial ,c.nomnegocio \
		FROM bancosmov b \
		INNER JOIN bancostransacc bt ON bt.idmovbanco=b.idmovbanco \
		INNER JOIN bancosxcob bxc ON bxc.transacc=bt.transacc \
		INNER JOIN transxcob txc ON txc.tracredito=bxc.tracredito \
		INNER JOIN ventas v ON v.referencia=txc.referencia \
		INNER JOIN clientes c ON v.cliente=c.cliente \
		WHERE b.idmovbanco=%s ", id);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Consulta los posibles pagos de proveedores relacionados.
	instruccion.sprintf("SELECT c.referencia, txp.pago, txp.tracredito, (txp.valor*-1) AS valor, \
		p.razonsocial, p.replegal \
		FROM bancosmov b \
		INNER JOIN bancostransacc bt ON bt.idmovbanco=b.idmovbanco \
		INNER JOIN bancosxpag bxp ON bxp.transacc=bt.transacc \
		INNER JOIN transxpag txp ON txp.tracredito=bxp.tracredito \
		INNER JOIN compras c ON c.referencia=txp.referencia \
		INNER JOIN proveedores p ON p.proveedor=c.proveedor \
		WHERE b.idmovbanco=%s ", id);

	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

}

//----------------------------------------------------------------------------
//ID_CANC_MOVBAN
void ServidorBancos::CancelaMov(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  CANCELA VENTA
	char *buffer_sql=new char[1024*32];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instrucciones[30], instruccion;
	int num_instrucciones=0;
	AnsiString id;
	int error=0;
	int i;


	try{
		id=mFg.ExtraeStringDeBuffer(&parametros); // ID del movimiento bancario

		/*instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";*/

		// Cancela la venta
		instruccion.sprintf("update bancosmov set cancelado=1 where idmovbanco='%s' and cancelado=0",id);
		instrucciones[num_instrucciones++]=instruccion;

		//instrucciones[num_instrucciones++]="COMMIT";

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

// ---------------------------------------------------------------------------
// ID_CON_SUCURSAL_MOVBAN
void ServidorBancos::ConsultaSucursalBancos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros) {
	// CONSULTA SUCURSAL
	AnsiString instruccion;
	AnsiString clave_sucursal;
	AnsiString idEmpresa;

	clave_sucursal = mFg.ExtraeStringDeBuffer(&parametros);

	idEmpresa = FormServidor->ObtieneClaveEmpresa();

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos de la sucursal
	instruccion.sprintf("SELECT s.sucursal AS Sucursal, s.numid AS NumId, s.nombre AS Nombre, \
	bc.cuentapagcli, bc.cuentapagprov FROM sucursales s INNER JOIN \
		bancosconfigsuc bc ON s.sucursal=bc.sucursal where s.sucursal='%s'",
		clave_sucursal);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

	// Obtiene todas las sucursales
	instruccion.sprintf("SELECT s.sucursal AS Sucursal, s.numid AS NumId, s.nombre AS Nombre, \
		CONCAT(bct.banco,' | ',bct.numerocuenta) AS cuentapagclidesc, \
		CONCAT(bct2.banco,' | ',bct2.numerocuenta) AS cuentapagprovdesc  \
		FROM sucursales s \
		INNER JOIN bancosconfigsuc bc ON s.sucursal=bc.sucursal \
		LEFT JOIN bancoscuentas bct ON bct.idnumcuenta = bc.cuentapagcli \
		LEFT JOIN bancoscuentas bct2 ON bct2.idnumcuenta = bc.cuentapagprov \
		WHERE s.idempresa = %s \
		ORDER BY s.nombre", idEmpresa);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}

// ---------------------------------------------------------------------------
// ID_GRA_CONFBANC
void ServidorBancos::GrabaConfiguracionBancos(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA SUCURSAL
	GrabaGenerico(Respuesta, MySQL, parametros, "bancosconfigsuc", "sucursal");
}

// ---------------------------------------------------------------------------
// ID_CON_REPAUXBAN
void ServidorBancos::ReporteAuxiliarBancos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros) {
	// CONSULTA SUCURSAL
	AnsiString instruccion;
	AnsiString idcuenta, naturaleza, fecha_ini, fecha_fin, fecha_saldos, sucursal, opcionsucursales = " ";
	AnsiString empresa, opcionempresa = " ", join_sucemp_bancosmov = " ";
	AnsiString fecha_ini_operacion, fecha_fin_operacion, usarfechaoperacion;
	AnsiString opcionCuenta = " ", opcionNaturaleza = " ";
	AnsiString origen, opcionorigenes = " ";
	AnsiString analitico;
	AnsiString joinpagos_ventasref=" ", fechaoperpagos_ventasref=" ", joinventas_ventasref=" ", fechaoperventas_ventasref=" ";
	AnsiString joinpagos_comprasref=" ", fechaoperpagos_comprasref=" ";
	AnsiString fechaoper_varios=" ";
	AnsiString fechaoperajustes=" ";
	AnsiString joinventas_sucursales=" ", joincompras_sucursales=" ";
	AnsiString joinpagoscli_ajustes=" ", joinpagosprov_ajustes=" ";


	idcuenta = mFg.ExtraeStringDeBuffer(&parametros);
	naturaleza = mFg.ExtraeStringDeBuffer(&parametros);
	empresa = mFg.ExtraeStringDeBuffer(&parametros);
	sucursal = mFg.ExtraeStringDeBuffer(&parametros);
	origen = mFg.ExtraeStringDeBuffer(&parametros);
	fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
	fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
	fecha_ini_operacion=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
	fecha_fin_operacion=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
	usarfechaoperacion=mFg.ExtraeStringDeBuffer(&parametros);//1 o 0 para saber si se considerara
	analitico = mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	if (usarfechaoperacion == "1") {

		joinpagos_ventasref.sprintf(" INNER JOIN pagoscli pag ON txc.pago=pag.pago ");
		fechaoperpagos_ventasref.sprintf(" and pag.fecha between '%s' and '%s' ", fecha_ini_operacion, fecha_fin_operacion);

		joinventas_ventasref.sprintf(" INNER JOIN ventas v ON v.referencia=bdv.venta ");
		fechaoperventas_ventasref.sprintf(" and v.fechavta between '%s' and '%s' ", fecha_ini_operacion, fecha_fin_operacion);

		joinpagos_comprasref.sprintf(" INNER JOIN pagosprov pag ON txp.pago=pag.pago ");
		fechaoperpagos_comprasref.sprintf(" and pag.fecha between '%s' and '%s' ", fecha_ini_operacion, fecha_fin_operacion);

		fechaoper_varios.sprintf(" and bm.fechaaplbanco between '%s' and '%s' ", fecha_ini_operacion, fecha_fin_operacion);

		// En los ajustes de pagos de clientes y de proveedores, la fecha de operación es la misma que la
		// fecha del pago.
		fechaoperajustes.sprintf(" and pag.fecha between '%s' and '%s' ", fecha_ini_operacion, fecha_fin_operacion);
		joinpagoscli_ajustes.sprintf(" INNER JOIN pagoscli pag ON bt.pagcliajuste = pag.pago ");
		joinpagosprov_ajustes.sprintf(" INNER JOIN pagosprov pag ON bt.pagprovajuste = pag.pago ");
	}

	if(sucursal != " ") {
		opcionsucursales.sprintf(" AND bm.sucursal = '%s'", sucursal);

		joinventas_sucursales.sprintf(" INNER JOIN ventas v ON v.referencia=txc.referencia \
			INNER JOIN terminales t ON t.terminal = v.terminal \
			INNER JOIN secciones s ON t.seccion = s.seccion  AND s.sucursal = '%s' ", sucursal);

		joincompras_sucursales.sprintf(" INNER JOIN compras c ON c.referencia=txp.referencia \
			INNER JOIN terminales t ON t.terminal = c.terminal \
			INNER JOIN secciones s ON t.seccion = s.seccion  AND s.sucursal = '%s' ", sucursal);

	}

	if(empresa != " "){

		if(sucursal == " "){
			joinventas_sucursales.sprintf(" INNER JOIN ventas v ON v.referencia=txc.referencia \
			INNER JOIN terminales t ON t.terminal = v.terminal \
			INNER JOIN secciones s ON t.seccion = s.seccion \
			INNER JOIN sucursales suc ON suc.sucursal = s.sucursal AND suc.idempresa = %s ", empresa);

            joincompras_sucursales.sprintf(" INNER JOIN compras c ON c.referencia=txp.referencia \
			INNER JOIN terminales t ON t.terminal = c.terminal \
			INNER JOIN secciones s ON t.seccion = s.seccion \
			INNER JOIN sucursales suc ON suc.sucursal = s.sucursal AND suc.idempresa = %s ", empresa);

		}

		join_sucemp_bancosmov.sprintf(" INNER JOIN sucursales suc ON suc.sucursal = bm.sucursal AND suc.idempresa = %s ", empresa);

	}

	if(origen != " ")
		opcionorigenes.sprintf(" AND bm.origen in (%s) ", origen);

	if(idcuenta != " ") {

		instruccion.sprintf("SELECT SUM(ifnull(bt.total,0))+bc.saldoinicial AS saldoinicial \
			FROM bancoscuentas bc \
			LEFT JOIN bancosmov bm ON bm.idnumcuenta = bc.idnumcuenta AND bm.cancelado = 0 AND bm.aplicado = 1 \
			AND bm.fechaaplbanco<'%s' \
			LEFT JOIN bancostransacc bt ON bm.idmovbanco = bt.idmovbanco \
			WHERE bc.idnumcuenta = '%s' ", fecha_ini, idcuenta);
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
			Respuesta->TamBufferResultado);

		instruccion.sprintf("SELECT bm.afectacion,SUM(bt.total) AS total FROM bancosmov bm \
			INNER JOIN bancostransacc bt ON bm.idmovbanco = bt.idmovbanco \
			WHERE bm.idnumcuenta = '%s' AND bm.cancelado = 0 AND bm.aplicado = 1 \
			AND bm.fechaaplbanco between '%s' and '%s' \
			GROUP BY bm.afectacion",idcuenta, fecha_ini, fecha_fin);
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
			Respuesta->TamBufferResultado);

		opcionCuenta.sprintf(" bc.idnumcuenta = '%s' AND ", idcuenta);
	}

	if(naturaleza != " ") {

		// Obtiene el saldo inicial
		instruccion.sprintf("select sum(saldoinicial) as saldoinicial from \
		 (SELECT SUM(ifnull(bt.total,0))+bc.saldoinicial AS saldoinicial \
			FROM bancoscuentas bc \
			LEFT JOIN bancosmov bm ON bm.idnumcuenta = bc.idnumcuenta AND bm.cancelado = 0 AND bm.aplicado = 1 AND bm.fechaaplbanco<'%s' \
			LEFT JOIN bancostransacc bt ON bm.idmovbanco = bt.idmovbanco \
			WHERE bc.naturaleza = '%s' AND bc.idempresa = %s \
			group by bm.idnumcuenta) t ", fecha_ini, naturaleza, empresa);
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
			Respuesta->TamBufferResultado);

		instruccion.sprintf("SELECT bm.afectacion,SUM(bt.total) AS total \
			FROM bancosmov bm \
			INNER JOIN bancostransacc bt ON bm.idmovbanco = bt.idmovbanco \
			INNER JOIN bancoscuentas bc ON bm.idnumcuenta = bc.idnumcuenta \
			WHERE bc.naturaleza = '%s' AND bm.cancelado = 0 AND bm.aplicado = 1 \
			AND bm.fechaaplbanco between '%s' and '%s' \
			AND bc.idempresa = %s \
			GROUP BY bm.afectacion",naturaleza, fecha_ini, fecha_fin, empresa);
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

		opcionNaturaleza.sprintf(" bc.naturaleza = '%s' AND ", naturaleza);
	}

	if(naturaleza == " " && idcuenta == " ") {

		// Obtiene el saldo inicial
		instruccion.sprintf("select sum(saldoinicial) as saldoinicial from \
		 (SELECT SUM(ifnull(bt.total,0))+bc.saldoinicial AS saldoinicial \
			FROM bancoscuentas bc \
			LEFT JOIN bancosmov bm ON bm.idnumcuenta = bc.idnumcuenta AND bm.cancelado = 0 AND bm.aplicado = 1 AND bm.fechaaplbanco<'%s' \
			LEFT JOIN bancostransacc bt ON bm.idmovbanco = bt.idmovbanco \
            WHERE bc.idempresa = %s \
			group by bm.idnumcuenta) t ",fecha_ini, empresa);
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
			Respuesta->TamBufferResultado);


		instruccion.sprintf("SELECT bm.afectacion,SUM(bt.total) AS total \
			FROM bancosmov bm \
			INNER JOIN bancostransacc bt ON bm.idmovbanco = bt.idmovbanco \
			INNER JOIN bancoscuentas bc ON bm.idnumcuenta = bc.idnumcuenta \
			WHERE bm.cancelado = 0 AND bm.aplicado = 1 \
			AND bm.fechaaplbanco between '%s' and '%s' \
            AND bc.idempresa = %s \
			GROUP BY bm.afectacion", fecha_ini, fecha_fin, empresa);
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	}

	// ***********************************************************************
	// INICIO DE SECCION PARA CALCULO DE IMPUESTOS

	instruccion="create temporary table ventasreferenciasaux ( \
				referencia varchar(11), KEY (referencia)) Engine = InnoDB";
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query EjecutaSelectSqlNulo"));

	instruccion.sprintf("create temporary table ventasreferenciasfinalaux like ventasreferenciasaux");
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query EjecutaSelectSqlNulo"));


	// 11111111111111111111111111111   Calculo de impuestos de ventas
	instruccion.sprintf(" \
			insert into ventasreferenciasaux (referencia) \
			(SELECT txc.referencia \
			FROM bancosmov bm \
			  INNER JOIN bancostransacc bt ON bm.idmovbanco = bt.idmovbanco \
			  INNER JOIN bancoscuentas bc ON bc.idnumcuenta = bm.idnumcuenta \
			  INNER JOIN bancosxcob bxc ON bxc.transacc=bt.transacc \
			  INNER JOIN transxcob txc ON txc.tracredito=bxc.tracredito \
			  %s \
			  %s \
			WHERE  %s %s bm.cancelado = 0 AND bm.aplicado = 1 \
			  AND bm.fechaaplbanco BETWEEN '%s' and '%s' %s %s \
			  AND bm.origen in ('PACLI', 'PACLE') \
              AND bc.idempresa = %s \
			) \
			 UNION \
			( \
			SELECT bdv.venta \
			FROM bancosmov bm \
			  INNER JOIN bancostransacc bt ON bm.idmovbanco = bt.idmovbanco \
			  INNER JOIN bancoscuentas bc ON bc.idnumcuenta = bm.idnumcuenta \
			  INNER JOIN bancosxventas bxv ON bxv.idmovbanco=bm.idmovbanco \
			  INNER JOIN bancosdventas bdv ON bdv.idbancosventas=bxv.idbancosventas and bdv.idmovbanco=bm.idmovbanco \
			  %s \
			  %s \
			WHERE  %s %s bm.cancelado = 0 AND bm.aplicado = 1 \
			  AND bm.fechaaplbanco BETWEEN '%s' and '%s' %s %s %s \
			  AND bm.origen IN ('VENCE','VENTC') \
              AND bc.idempresa = %s \
			) ",joinpagos_ventasref,
				joinventas_sucursales,
				opcionNaturaleza, opcionCuenta, fecha_ini, fecha_fin, opcionorigenes, fechaoperpagos_ventasref, empresa,
				joinventas_ventasref, join_sucemp_bancosmov,
				opcionNaturaleza, opcionCuenta, fecha_ini, fecha_fin, opcionsucursales, opcionorigenes, fechaoperventas_ventasref, empresa);
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query EjecutaSelectSqlNulo"));

	// Con todas las ventas de ventasreferenciasaux pero sin repetirse
	instruccion.sprintf("insert into ventasreferenciasfinalaux \
		select DISTINCT * from ventasreferenciasaux");
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query EjecutaSelectSqlNulo"));

	// Crea una tabla para tener las ventas en cuestión
	instruccion.sprintf("create temporary table ventasaux (referencia char(11), \
		totaltodo decimal(16,4), \
		total0 decimal(16,4), total15 decimal(16,4), total16 decimal(16,4), \
		subnograbado decimal(16,4), subtotal15 decimal(16,4), subtotal16 decimal(16,4), subgrabado decimal(16,4), subtotal decimal(16,4), \
		totaliva decimal(16,4), \
		totaliesps decimal(16,4), \
		valor decimal(16,4), \
		porcentaje0 decimal(16,10), \
		porcentaje15 decimal(16,10), \
		porcentaje16 decimal(16,10), \
		porcentajesub0 decimal(16,10), \
		porcentajesub15 decimal(16,10), \
		porcentajesub16 decimal(16,10), \
		porcentajesubgrabado decimal(16,10), \
		porcentajesubtotal decimal(16,10), \
		porcentajeiva decimal(16,10), \
		porcentajeiesps decimal(16,10), \
		PRIMARY KEY (referencia)) Engine = InnoDB");
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query EjecutaSelectSqlNulo"));

	// Se crean dos copias de ventasaux,
	// ya que se requiere usar la tabla dos veces ventasaux en un mismo query y mysql no lo permite.
	instruccion.sprintf("create temporary table ventasaaux like ventasaux");
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query EjecutaSelectSqlNulo"));
	instruccion.sprintf("create temporary table ventasbaux like ventasaux");
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query EjecutaSelectSqlNulo"));

	// Suma las notas de crédito y deja el resultado en una tabla temporal,
	// para luego hacerle un left join con las ventas.
	instruccion.sprintf("create temporary table dnotascredcliaux \
		select v.referencia as venta, d.articulo, sum(if(n.tipo='0',d.cantidad,0)) as cantidad, \
		sum(if(n.tipo<>'0',d.precio,0)) as precio, \
		sum(if(n.tipo<>'0',d.precioimp,0)) as precioimp \
		from notascredcli n, dnotascredcli d, ventas v, ventasreferenciasfinalaux ra \
		where n.venta=v.referencia and n.cancelado=0 and v.cancelado=0 and \
		ra.referencia=v.referencia and \
		n.referencia=d.referencia \
		group by v.referencia, d.articulo");
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query EjecutaSelectSqlNulo"));

	instruccion.sprintf("insert into ventasaux \
			(referencia, totaltodo, total0, total15, total16, \
			subnograbado,subtotal15,subtotal16,subgrabado,subtotal, \
			totaliva, totaliesps) \
	  select v.referencia, \
		sum((dv.precioimp-ifnull(dn.precioimp,0))*(dv.cantidad-ifnull(dn.cantidad,0)) ) as totaltodo, \
		sum(if( \
			(not (ifnull(i1.tipoimpu,'')='IVA' AND ifnull(i1.porcentaje,0)>0) AND \
			not (ifnull(i2.tipoimpu,'')='IVA' AND ifnull(i2.porcentaje,0)>0) AND \
			not (ifnull(i3.tipoimpu,'')='IVA' AND ifnull(i3.porcentaje,0)>0) AND  \
			not (ifnull(i4.tipoimpu,'')='IVA' AND ifnull(i4.porcentaje,0)>0)), \
			(dv.precioimp-ifnull(dn.precioimp,0))*(dv.cantidad-ifnull(dn.cantidad,0)),\
			 0)) as total0, \
		sum(if( \
			((i1.tipoimpu='IVA' and i1.porcentaje=15.00) OR \
			(i2.tipoimpu='IVA' and i2.porcentaje=15.00) OR \
			(i3.tipoimpu='IVA' and i3.porcentaje=15.00) OR \
			(i4.tipoimpu='IVA' and i4.porcentaje=15.00)), \
			(dv.precioimp-ifnull(dn.precioimp,0))*(dv.cantidad-ifnull(dn.cantidad,0)), \
			0)) as total15, \
		sum(if( \
			((i1.tipoimpu='IVA' and i1.porcentaje=16.00) OR \
			(i2.tipoimpu='IVA' and i2.porcentaje=16.00) OR \
			(i3.tipoimpu='IVA' and i3.porcentaje=16.00) OR \
			(i4.tipoimpu='IVA' and i4.porcentaje=16.00)), \
			(dv.precioimp-ifnull(dn.precioimp,0))*(dv.cantidad-ifnull(dn.cantidad,0)), \
			0)) as total16, \
			\
		sum(@subnograbado:=if( \
			(not (ifnull(i1.tipoimpu,'')='IVA' AND ifnull(i1.porcentaje,0)>0) AND \
			not (ifnull(i2.tipoimpu,'')='IVA' AND ifnull(i2.porcentaje,0)>0) AND \
			not (ifnull(i3.tipoimpu,'')='IVA' AND ifnull(i3.porcentaje,0)>0) AND  \
			not (ifnull(i4.tipoimpu,'')='IVA' AND ifnull(i4.porcentaje,0)>0)), \
			(dv.precio-ifnull(dn.precio,0))*(dv.cantidad-ifnull(dn.cantidad,0)),\
			 0)) as subnograbado, \
		sum(@subtotal15:=if( \
			((i1.tipoimpu='IVA' and i1.porcentaje=15.00) OR \
			(i2.tipoimpu='IVA' and i2.porcentaje=15.00) OR \
			(i3.tipoimpu='IVA' and i3.porcentaje=15.00) OR \
			(i4.tipoimpu='IVA' and i4.porcentaje=15.00)), \
			(dv.precio-ifnull(dn.precio,0))*(dv.cantidad-ifnull(dn.cantidad,0)), \
			0)) as subtotal15, \
		sum(@subtotal16:=if( \
			((i1.tipoimpu='IVA' and i1.porcentaje=16.00) OR \
			(i2.tipoimpu='IVA' and i2.porcentaje=16.00) OR \
			(i3.tipoimpu='IVA' and i3.porcentaje=16.00) OR \
			(i4.tipoimpu='IVA' and i4.porcentaje=16.00)), \
			(dv.precio-ifnull(dn.precio,0))*(dv.cantidad-ifnull(dn.cantidad,0)), \
			0)) as subtotal16, \
		sum(@subgrabado:=@subtotal15+@subtotal16) as subgrabado, \
		sum(@subtotal:=@subtotal15+@subtotal16+@subnograbado) as subtotal, \
		sum(if(i1.tipoimpu='IVA', (dv.precio-ifnull(dn.precio,0))*(dv.cantidad-ifnull(dn.cantidad,0))*(i1.porcentaje/100), 0) + \
		if(i2.tipoimpu='IVA', (dv.precio-ifnull(dn.precio,0))*(dv.cantidad-ifnull(dn.cantidad,0))*(1+i1.porcentaje/100)*(i2.porcentaje/100), 0)) as totaliva, \
		sum(if(i1.tipoimpu='IESPS', \
		(dv.precio-ifnull(dn.precio,0))*(dv.cantidad-ifnull(dn.cantidad,0))*(i1.porcentaje/100), \
		 0)) as totaliesps \
		from ventas v  inner join  dventas dv  inner join ventasreferenciasfinalaux ra \
		left join dnotascredcliaux dn on dn.articulo=dv.articulo and dn.venta=v.referencia \
		left join impuestos i1 on i1.impuesto=dv.claveimp1 \
		left join impuestos i2 on i2.impuesto=dv.claveimp2 \
		left join impuestos i3 on i3.impuesto=dv.claveimp3 \
		left join impuestos i4 on i4.impuesto=dv.claveimp4 \
		where v.referencia=dv.referencia and v.referencia=ra.referencia \
		group by v.referencia \
		order by v.referencia");
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query EjecutaSelectSqlNulo"));

	instruccion.sprintf("update ventasaux set valor=totaltodo, \
		porcentaje0=total0/(totaltodo), \
		porcentaje15=total15/(totaltodo), \
		porcentaje16=total16/(totaltodo), \
		porcentajesub0=subnograbado/(totaltodo), \
		porcentajesub15=subtotal15/(totaltodo), \
		porcentajesub16=subtotal16/(totaltodo), \
		porcentajesubgrabado=subgrabado/(totaltodo), \
		porcentajesubtotal=subtotal/(totaltodo), \
		porcentajeiva=totaliva/(totaltodo), \
		porcentajeiesps=totaliesps/(totaltodo) WHERE totaltodo <> 0 ");
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query EjecutaSelectSqlNulo"));

	// Tener las ventas con las proporciones para cada impuesto y los diferentes totales de forma única
	instruccion.sprintf("insert into ventasaaux \
		select * from ventasaux");
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query EjecutaSelectSqlNulo"));

	instruccion.sprintf("insert into ventasbaux \
		select * from ventasaaux");
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query EjecutaSelectSqlNulo"));


	// 2222222222222222222222222   Calculo de impuestos de compras

	instruccion="create temporary table comprasreferenciasfinalaux ( \
				referencia varchar(11), KEY (referencia)) Engine = InnoDB";
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query EjecutaSelectSqlNulo"));

	instruccion.sprintf(" \
			insert into comprasreferenciasfinalaux (referencia) \
			SELECT txp.referencia \
			FROM bancosmov bm \
			  INNER JOIN bancostransacc bt ON bm.idmovbanco = bt.idmovbanco \
			  INNER JOIN bancoscuentas bc ON bc.idnumcuenta = bm.idnumcuenta \
			  INNER JOIN bancosxpag bxp ON bxp.transacc=bt.transacc \
			  INNER JOIN transxpag txp ON txp.tracredito=bxp.tracredito \
			  %s \
			  %s \
			WHERE  %s %s bm.cancelado = 0 AND bm.aplicado = 1 \
			  AND bm.fechaaplbanco BETWEEN '%s' and '%s' %s %s \
			  AND bm.origen in ('PAPRO') \
			  AND bc.idempresa = %s \
			GROUP BY referencia ",
				joinpagos_comprasref,
				joincompras_sucursales,
				opcionNaturaleza, opcionCuenta, fecha_ini, fecha_fin, opcionorigenes, fechaoperpagos_comprasref, empresa);
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query EjecutaSelectSqlNulo"));

	// Crea una tabla para tener las compras en cuestión
	instruccion.sprintf("create temporary table comprasaux (referencia char(11), \
		totaltodo decimal(16,4), \
		total0 decimal(16,4), total15 decimal(16,4), total16 decimal(16,4), \
		subnograbado decimal(16,4), subtotal15 decimal(16,4), subtotal16 decimal(16,4), subgrabado decimal(16,4), subtotal decimal(16,4), \
		totaliva decimal(16,4), \
		totaliesps decimal(16,4), valor decimal(16,4), \
		porcentaje0 decimal(16,10), porcentaje15 decimal(16,10), porcentaje16 decimal(16,10), \
		porcentajesub0 decimal(16,10), \
		porcentajesub15 decimal(16,10), \
		porcentajesub16 decimal(16,10), \
		porcentajesubgrabado decimal(16,10), \
		porcentajesubtotal decimal(16,10), \
		porcentajeiva decimal(16,10), \
		porcentajeiesps decimal(16,10), \
		PRIMARY KEY (referencia)) Engine = InnoDB");
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query EjecutaSelectSqlNulo"));

	// Suma las notas de crédito y deja el resultado en una tabla temporal,
	// para luego hacerle un left join con las compras.
	instruccion.sprintf("create temporary table dnotascredprovaux \
		select c.referencia as compra, d.articulo, sum(if(n.tipo='0',d.cantidad,0)) as cantidad, \
		sum(if(n.tipo<>'0',d.costo,0)) as costo, \
		sum(if(n.tipo<>'0',d.costoimp,0)) as costoimp \
		from notascredprov n, dnotascredprov d, compras c, comprasreferenciasfinalaux ra \
		where n.compra=c.referencia and n.cancelado=0 and c.cancelado=0 and \
		ra.referencia=c.referencia and \
		n.referencia=d.referencia \
		group by c.referencia, d.articulo");
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query EjecutaSelectSqlNulo"));

	instruccion.sprintf("insert into comprasaux \
		(referencia, totaltodo, total0, total15, total16, \
		subnograbado,subtotal15,subtotal16,subgrabado,subtotal, \
		totaliva, totaliesps) \
		select c.referencia, \
		sum((dc.costoimp-ifnull(dn.costoimp,0))*(dc.cantidad-ifnull(dn.cantidad,0)) ) as totaltodo, \
		sum(if(	(not (ifnull(i1.tipoimpu,'')='IVA' AND ifnull(i1.porcentaje,0)>0) AND \
			not (ifnull(i2.tipoimpu,'')='IVA' AND ifnull(i2.porcentaje,0)>0) AND \
			not (ifnull(i3.tipoimpu,'')='IVA' AND ifnull(i3.porcentaje,0)>0) AND  \
			not (ifnull(i4.tipoimpu,'')='IVA' AND ifnull(i4.porcentaje,0)>0) ), \
			(dc.costoimp-ifnull(dn.costoimp,0))*(dc.cantidad-ifnull(dn.cantidad,0)) , \
			 0)) as total0, \
		sum(if( ((i1.tipoimpu='IVA' and i1.porcentaje=15.00) OR \
			(i2.tipoimpu='IVA' and i2.porcentaje=15.00) OR \
			(i3.tipoimpu='IVA' and i3.porcentaje=15.00) OR \
			(i4.tipoimpu='IVA' and i4.porcentaje=15.00)), \
			(dc.costoimp-ifnull(dn.costoimp,0))*(dc.cantidad-ifnull(dn.cantidad,0)), \
			0)) as total15, \
		sum(if(	((i1.tipoimpu='IVA' and i1.porcentaje=16.00) OR \
			(i2.tipoimpu='IVA' and i2.porcentaje=16.00) OR \
			(i3.tipoimpu='IVA' and i3.porcentaje=16.00) OR \
			(i4.tipoimpu='IVA' and i4.porcentaje=16.00)), \
			(dc.costoimp-ifnull(dn.costoimp,0))*(dc.cantidad-ifnull(dn.cantidad,0)), \
			0)) as total16, \
			\
		sum(@subnograbado:=if( (not (ifnull(i1.tipoimpu,'')='IVA' AND ifnull(i1.porcentaje,0)>0) AND \
			not (ifnull(i2.tipoimpu,'')='IVA' AND ifnull(i2.porcentaje,0)>0) AND \
			not (ifnull(i3.tipoimpu,'')='IVA' AND ifnull(i3.porcentaje,0)>0) AND  \
			not (ifnull(i4.tipoimpu,'')='IVA' AND ifnull(i4.porcentaje,0)>0)), \
			(dc.costo-ifnull(dn.costo,0))*(dc.cantidad-ifnull(dn.cantidad,0)),\
			 0)) as subnograbado, \
		sum(@subtotal15:=if( ((i1.tipoimpu='IVA' and i1.porcentaje=15.00) OR \
			(i2.tipoimpu='IVA' and i2.porcentaje=15.00) OR \
			(i3.tipoimpu='IVA' and i3.porcentaje=15.00) OR \
			(i4.tipoimpu='IVA' and i4.porcentaje=15.00)), \
			(dc.costo-ifnull(dn.costo,0))*(dc.cantidad-ifnull(dn.cantidad,0)), \
			0)) as subtotal15, \
		sum(@subtotal16:=if( ((i1.tipoimpu='IVA' and i1.porcentaje=16.00) OR \
			(i2.tipoimpu='IVA' and i2.porcentaje=16.00) OR \
			(i3.tipoimpu='IVA' and i3.porcentaje=16.00) OR \
			(i4.tipoimpu='IVA' and i4.porcentaje=16.00)), \
			(dc.costo-ifnull(dn.costo,0))*(dc.cantidad-ifnull(dn.cantidad,0)), \
			0)) as subtotal16, \
		sum(@subgrabado:=@subtotal15+@subtotal16) as subgrabado, \
		sum(@subtotal:=@subtotal15+@subtotal16+@subnograbado) as subtotal, \
		sum(if(i1.tipoimpu='IVA', (dc.costo-dc.iepscuota-ifnull(dn.costo,0))*(dc.cantidad-ifnull(dn.cantidad,0))*(i1.porcentaje/100), 0) + \
		if(i2.tipoimpu='IVA', ( ((dc.costo-ifnull(dn.costo,0))*(dc.cantidad-ifnull(dn.cantidad,0))) + ((dc.costo-dc.iepscuota-ifnull(dn.costo,0))*(dc.cantidad-ifnull(dn.cantidad,0))*(i1.porcentaje/100))) * (i2.porcentaje/100), 0)) as totaliva, \
		sum(if(i1.tipoimpu='IESPS', (dc.costo-dc.iepscuota-ifnull(dn.costo,0))*(dc.cantidad-ifnull(dn.cantidad,0))*(i1.porcentaje/100), 0)) as totaliesps \
		from compras c  inner join  dcompras dc  inner join  comprasreferenciasfinalaux ra \
		left join dnotascredprovaux dn on dn.articulo=dc.articulo and dn.compra=c.referencia \
		left join impuestos i1 on i1.impuesto=dc.claveimp1 \
		left join impuestos i2 on i2.impuesto=dc.claveimp2 \
		left join impuestos i3 on i3.impuesto=dc.claveimp3 \
		left join impuestos i4 on i4.impuesto=dc.claveimp4 \
		where c.referencia=dc.referencia and c.referencia=ra.referencia \
		group by c.referencia \
		order by c.referencia");
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query EjecutaSelectSqlNulo"));

	instruccion.sprintf("update comprasaux set valor=totaltodo, \
		porcentaje0=total0/(totaltodo), \
		porcentaje15=total15/(totaltodo), \
		porcentaje16=total16/(totaltodo), \
		porcentajesub0=subnograbado/(totaltodo), \
		porcentajesub15=subtotal15/(totaltodo), \
		porcentajesub16=subtotal16/(totaltodo), \
		porcentajesubgrabado=subgrabado/(totaltodo), \
		porcentajesubtotal=subtotal/(totaltodo), \
		porcentajeiva=totaliva/(totaltodo), \
		porcentajeiesps=totaliesps/(totaltodo) WHERE totaltodo <> 0 ");
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query EjecutaSelectSqlNulo"));



	// FIN DE SECCION PARA CALCULO DE IMPUESTOS
	// ***********************************************************************

	if(analitico == "0") {

		//obtiene todos los datos (NO ANALITICO)
		instruccion.sprintf(" \
		SELECT \
		  idmovbanco, transacc, idnumcuenta, numerocuenta, banco, \
		  (CASE WHEN descconceptomov='E' THEN 'Efectivo' WHEN descconceptomov = 'C' THEN 'Cheque' WHEN descconceptomov = 'D' THEN 'Depósito' WHEN descconceptomov = 'T' THEN 'Transferencia bancaria'  WHEN descconceptomov = 'O' THEN 'Otro' END) AS descconceptomov, \
		  descripcion, identificador, fechaaplbanco, tipodet, identdet, afectacion, totalmov, \
		  naturaleza, sucursal, origen, fechaalta, horaalta, \
		  SUM(valordoc) AS valordoc, \
		   SUM(proporcion0) AS proporcion0, \
		   SUM(proporcion15) AS proporcion15, \
		   SUM(proporcion16) AS proporcion16, \
		   SUM(proporcionsub0) AS proporcionsub0, \
		   SUM(proporcionsub15) AS proporcionsub15, \
		   SUM(proporcionsub16) AS proporcionsub16, \
		   SUM(proporcionsubgrabado) AS proporcionsubgrabado, \
		   SUM(proporcionsubtotal) AS proporcionsubtotal, \
		   SUM(proporcioniva) AS proporcioniva, \
		   SUM(proporcioniesps) AS proporcioniesps \
		FROM \
		( \
			(SELECT \
			  bm.idmovbanco, bt.transacc, bc.idnumcuenta, bc.numerocuenta, bc.banco, \
			  bt.tipodet AS descconceptomov, bm.descripcion, bm.identificador, \
			  bm.fechaaplbanco, bt.tipodet, bt.identificador AS identdet, \
			  bm.afectacion, bm.total as totalmov, bn.naturaleza, bm.sucursal, bm.origen, bm.fechaalta, bm.horaalta, \
				bt.total AS valordoc, \
				0.0 AS proporcion0, 0.0 AS proporcion15, bt.total AS proporcion16, 0.0 AS proporcionsub0, 0.0 AS proporcionsub15, bt.subtotal AS proporcionsub16, \
				bt.subtotal AS proporcionsubgrabado, bt.subtotal AS proporcionsubtotal, bt.ivabanco AS proporcioniva, 0.0 AS proporcioniesps \
			FROM \
			  bancosmov bm \
			  INNER JOIN bancostransacc bt ON bm.idmovbanco = bt.idmovbanco and bt.tipodet='A' \
			  INNER JOIN bancoscuentas bc ON bc.idnumcuenta = bm.idnumcuenta \
			  INNER JOIN bancosnaturalezas bn ON bn.naturaleza = bc.naturaleza \
			  INNER JOIN bancosconceptomov bcm ON bm.conceptomov = bcm.conceptomov \
			  %s \
              %s \
			WHERE  %s %s \
			  bm.cancelado = 0 AND bm.aplicado = 1 AND bm.fechaaplbanco BETWEEN '%s' and '%s' %s \
			  %s %s \
			  AND bm.origen in ('PACLI', 'PACLE') \
			  AND bc.idempresa = %s \
			) \
			UNION ALL \
			( \
			SELECT \
			  bm.idmovbanco, bt.transacc, bc.idnumcuenta, bc.numerocuenta, bc.banco, \
			  bt.tipodet AS descconceptomov, bm.descripcion, bm.identificador, \
			  bm.fechaaplbanco, bt.tipodet, bt.identificador AS identdet, \
			  bm.afectacion, bm.total as totalmov, bn.naturaleza, bm.sucursal, bm.origen, bm.fechaalta, bm.horaalta, \
				bt.total AS valordoc, \
				0.0 AS proporcion0, 0.0 AS proporcion15, bt.total AS proporcion16, 0.0 AS proporcionsub0, 0.0 AS proporcionsub15, bt.subtotal AS proporcionsub16, \
				bt.subtotal AS proporcionsubgrabado, bt.subtotal AS proporcionsubtotal, bt.ivabanco AS proporcioniva, 0.0 AS proporcioniesps \
			FROM \
			  bancosmov bm \
			  INNER JOIN bancostransacc bt ON bm.idmovbanco = bt.idmovbanco and bt.tipodet='A' \
			  INNER JOIN bancoscuentas bc ON bc.idnumcuenta = bm.idnumcuenta \
			  INNER JOIN bancosnaturalezas bn ON bn.naturaleza = bc.naturaleza \
			  INNER JOIN bancosconceptomov bcm ON bm.conceptomov = bcm.conceptomov \
			  %s \
              %s \
			WHERE  %s %s \
			  bm.cancelado = 0 AND bm.aplicado = 1 AND bm.fechaaplbanco BETWEEN '%s' and '%s' %s \
			  %s %s \
			  AND bm.origen in ('PAPRO') \
			  AND bc.idempresa = %s \
			) \
			UNION ALL \
			(SELECT \
			  bm.idmovbanco, bt.transacc, bc.idnumcuenta, bc.numerocuenta, bc.banco, \
			  bt.tipodet AS descconceptomov, bm.descripcion, bm.identificador, \
			  bm.fechaaplbanco, bt.tipodet, bt.identificador AS identdet, \
			  bm.afectacion, bm.total as totalmov, bn.naturaleza, bm.sucursal, bm.origen, bm.fechaalta, bm.horaalta, \
			  @valormov:=bxc.valorconsiderado AS valordoc, \
			  (@valormov*va.porcentaje0) as proporcion0, \
			  (@valormov*va.porcentaje15) as proporcion15, \
			  (@valormov*va.porcentaje16) as proporcion16, \
			  (@valormov*va.porcentajesub0) as proporcionsub0, \
			  (@valormov*va.porcentajesub15) as proporcionsub15, \
			  (@valormov*va.porcentajesub16) as proporcionsub16, \
			  (@valormov*va.porcentajesubgrabado) as proporcionsubgrabado, \
			  (@valormov*va.porcentajesubtotal) as proporcionsubtotal, \
			  (@valormov*va.porcentajeiva) as proporcioniva, \
			  (@valormov*va.porcentajeiesps) as proporcioniesps \
			FROM \
			  bancosmov bm \
			  INNER JOIN bancostransacc bt ON bm.idmovbanco = bt.idmovbanco \
			  INNER JOIN bancoscuentas bc ON bc.idnumcuenta = bm.idnumcuenta \
			  INNER JOIN bancosnaturalezas bn ON bn.naturaleza = bc.naturaleza \
			  INNER JOIN bancosconceptomov bcm ON bm.conceptomov = bcm.conceptomov \
			  INNER JOIN bancosxcob bxc ON bxc.transacc=bt.transacc \
			  INNER JOIN transxcob txc ON txc.tracredito=bxc.tracredito \
			  INNER JOIN ventasaaux va ON txc.referencia=va.referencia \
			  %s \
			WHERE  %s %s \
			  bm.cancelado = 0 AND bm.aplicado = 1 AND bm.fechaaplbanco BETWEEN '%s' and '%s' %s %s \
			  AND bm.origen in ('PACLI', 'PACLE') \
			  AND bc.idempresa = %s \
			) \
			UNION ALL \
			( \
			SELECT \
			  bm.idmovbanco, bt.transacc, bc.idnumcuenta, bc.numerocuenta, bc.banco, \
			  bt.tipodet AS descconceptomov, bm.descripcion, bm.identificador, \
			  bm.fechaaplbanco, bt.tipodet, bt.identificador AS identdet, \
			  bm.afectacion, bm.total as totalmov, bn.naturaleza, bm.sucursal, bm.origen, bm.fechaalta, bm.horaalta, \
			  @valormov:=(txp.valor*-1) AS valordoc, \
			  (@valormov*ca.porcentaje0) as proporcion0, \
			  (@valormov*ca.porcentaje15) as proporcion15, \
			  (@valormov*ca.porcentaje16) as proporcion16, \
			  (@valormov*ca.porcentajesub0) as proporcionsub0, \
			  (@valormov*ca.porcentajesub15) as proporcionsub15, \
			  (@valormov*ca.porcentajesub16) as proporcionsub16, \
			  (@valormov*ca.porcentajesubgrabado) as proporcionsubgrabado, \
			  (@valormov*ca.porcentajesubtotal) as proporcionsubtotal, \
			  (@valormov*ca.porcentajeiva) as proporcioniva, \
			  (@valormov*ca.porcentajeiesps) as proporcioniesps \
			FROM \
			  bancosmov bm \
			  INNER JOIN bancostransacc bt ON bm.idmovbanco = bt.idmovbanco \
			  INNER JOIN bancoscuentas bc ON bc.idnumcuenta = bm.idnumcuenta \
			  INNER JOIN bancosnaturalezas bn ON bn.naturaleza = bc.naturaleza \
			  INNER JOIN bancosconceptomov bcm ON bm.conceptomov = bcm.conceptomov \
			  INNER JOIN bancosxpag bxp ON bxp.transacc=bt.transacc \
			  INNER JOIN transxpag txp ON txp.tracredito=bxp.tracredito \
			  %s \
			  INNER JOIN comprasaux ca ON txp.referencia=ca.referencia \
			WHERE  %s %s \
			  bm.cancelado = 0 AND bm.aplicado = 1 AND bm.fechaaplbanco BETWEEN '%s' and '%s' %s %s \
			  AND bm.origen in ('PAPRO') \
			  AND bc.idempresa = %s \
			) \
			UNION ALL \
			( \
			SELECT \
			  bm.idmovbanco, bt.transacc, bc.idnumcuenta, bc.numerocuenta, bc.banco, \
			  bt.tipodet AS descconceptomov, bm.descripcion, bm.identificador, \
			  bm.fechaaplbanco, bt.tipodet, bt.identificador AS identdet, \
			  bm.afectacion, bm.total as totalmov, bn.naturaleza, bm.sucursal, bm.origen, bm.fechaalta, bm.horaalta, \
			  @valormov:=bdv.valorconsiderado AS valordoc, \
			  (@valormov*va.porcentaje0) as proporcion0, \
			  (@valormov*va.porcentaje15) as proporcion15, \
			  (@valormov*va.porcentaje16) as proporcion16, \
			  (@valormov*va.porcentajesub0) as proporcionsub0, \
			  (@valormov*va.porcentajesub15) as proporcionsub15, \
			  (@valormov*va.porcentajesub16) as proporcionsub16, \
			  (@valormov*va.porcentajesubgrabado) as proporcionsubgrabado, \
			  (@valormov*va.porcentajesubtotal) as proporcionsubtotal, \
			  (@valormov*va.porcentajeiva) as proporcioniva, \
			  (@valormov*va.porcentajeiesps) as proporcioniesps \
			FROM \
			  bancosmov bm \
			  INNER JOIN bancostransacc bt ON bm.idmovbanco = bt.idmovbanco \
			  INNER JOIN bancoscuentas bc ON bc.idnumcuenta = bm.idnumcuenta \
			  INNER JOIN bancosnaturalezas bn ON bn.naturaleza = bc.naturaleza \
			  INNER JOIN bancosconceptomov bcm ON bm.conceptomov = bcm.conceptomov \
			  INNER JOIN bancosxventas bxv ON bxv.idmovbanco=bm.idmovbanco \
			  INNER JOIN bancosdventas bdv ON bdv.idbancosventas=bxv.idbancosventas and bdv.idmovbanco=bm.idmovbanco \
			  INNER JOIN ventasbaux va ON bdv.venta=va.referencia \
              %s \
			WHERE  %s %s  \
			  bm.cancelado = 0 AND bm.aplicado = 1 \
			  AND bm.fechaaplbanco BETWEEN '%s' and '%s' %s %s \
			  AND bm.origen IN ('VENCE','VENTC') AND bc.idempresa = %s \
			  GROUP BY bm.idmovbanco, bdv.venta  \
			) \
			UNION ALL \
			( \
			SELECT \
			  bm.idmovbanco, bt.transacc, bc.idnumcuenta, bc.numerocuenta, bc.banco, \
			  bt.tipodet AS descconceptomov, bm.descripcion, bm.identificador, \
			  bm.fechaaplbanco, bt.tipodet, bt.identificador AS identdet, \
			  bm.afectacion, bm.total as totalmov, bn.naturaleza, bm.sucursal, bm.origen, bm.fechaalta, bm.horaalta, \
			  @valormov:=bt.total AS valordoc, \
			  if( ifnull(i1.tipoimpu,'')='IVA' AND ifnull(i1.porcentaje,0)=0.0, bt.total, 0) as proporcion0, \
			  if( ifnull(i1.tipoimpu,'')='IVA' AND ifnull(i1.porcentaje,0)=15.0, bt.total, 0) as proporcion15, \
			  if( ifnull(i1.tipoimpu,'')='IVA' AND ifnull(i1.porcentaje,0)=16.0, bt.total, 0) as proporcion16, \
			  @subnograbado:=if( ifnull(i1.tipoimpu,'')='IVA' AND ifnull(i1.porcentaje,0)=0.0, bt.subtotal, 0) as proporcionsub0, \
			  @subtotal15:=if( ifnull(i1.tipoimpu,'')='IVA' AND ifnull(i1.porcentaje,0)=15.0, bt.subtotal, 0) as proporcionsub15, \
			  @subtotal16:=if( ifnull(i1.tipoimpu,'')='IVA' AND ifnull(i1.porcentaje,0)=16.0, bt.subtotal, 0) as proporcionsub16, \
			  (@subtotal15+@subtotal16) as proporcionsubgrabado, \
			  (@subnograbado+@subtotal15+@subtotal16) as proporcionsubtotal, \
			  if( ifnull(i1.tipoimpu,'')='IVA' AND ifnull(i1.porcentaje,0)>0.0, bt.ivabanco, 0) as proporcioniva, \
			  0.0 as proporcioniesps \
			FROM \
			  bancosmov bm \
			  INNER JOIN bancostransacc bt ON bm.idmovbanco = bt.idmovbanco \
			  INNER JOIN bancoscuentas bc ON bc.idnumcuenta = bm.idnumcuenta \
			  INNER JOIN bancosnaturalezas bn ON bn.naturaleza = bc.naturaleza \
			  INNER JOIN bancosconceptomov bcm ON bm.conceptomov = bcm.conceptomov \
			  left join impuestos i1 on i1.impuesto=bt.cveimp \
			  %s \
			WHERE %s %s \
			  bm.cancelado = 0 AND bm.aplicado = 1 \
			  AND bm.fechaaplbanco BETWEEN '%s' and '%s' %s %s \
			  %s \
			  AND bm.origen NOT IN ('PACLI', 'PACLE', 'PAPRO', 'VENCE','VENTC') \
			  AND bc.idempresa = %s \
			) \
		) t \
		GROUP BY idmovbanco \
		ORDER BY idnumcuenta, fechaaplbanco, fechaalta, horaalta, idmovbanco ",
							joinpagoscli_ajustes, join_sucemp_bancosmov,
							opcionNaturaleza, opcionCuenta, fecha_ini, fecha_fin, opcionorigenes, fechaoperajustes, opcionsucursales,
                            empresa,
							joinpagosprov_ajustes, join_sucemp_bancosmov,
							opcionNaturaleza, opcionCuenta, fecha_ini, fecha_fin, opcionorigenes, fechaoperajustes, opcionsucursales,
							empresa,
							joinpagos_ventasref,
							opcionNaturaleza, opcionCuenta, fecha_ini, fecha_fin, opcionorigenes, fechaoperpagos_ventasref,
							empresa,
							joinpagos_comprasref, opcionNaturaleza, opcionCuenta, fecha_ini, fecha_fin, opcionorigenes, fechaoperpagos_comprasref,
							empresa,
							join_sucemp_bancosmov, opcionNaturaleza, opcionCuenta, fecha_ini, fecha_fin, opcionsucursales, opcionorigenes,
							empresa,
							join_sucemp_bancosmov, opcionNaturaleza, opcionCuenta, fecha_ini, fecha_fin, opcionsucursales, opcionorigenes,
							fechaoper_varios, empresa );
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	} else {

		//obtiene todos los datos (SI ANALITICO)
		instruccion.sprintf(" \
			(SELECT \
			  bm.idmovbanco, bt.transacc, bc.idnumcuenta, bc.numerocuenta, bc.banco, \
			  bcm.descripcion AS descconceptomov, bm.descripcion, bm.identificador, \
			  bm.fechaaplbanco, bt.tipodet, bt.identificador AS identdet, \
			  bm.afectacion, bm.total as totalmov, bn.naturaleza, bm.sucursal, bm.origen, bm.fechaalta, bm.horaalta, \
				'AJUSTE' AS referenciadoc, bt.total AS valordoc, '' AS razonsocialdoc, 'AJUSTE' AS pagodoc, 0.0 AS tracreditodoc, '' AS seriefolio, '' AS muuid, '' AS  muuidorefer, \
				0.0 AS proporcion0, 0.0 AS proporcion15, bt.total AS proporcion16, 0.0 AS proporcionsub0, 0.0 AS proporcionsub15, bt.subtotal AS proporcionsub16, \
				bt.subtotal AS proporcionsubgrabado, bt.subtotal AS proporcionsubtotal, bt.ivabanco AS proporcioniva, 0.0 AS proporcioniesps \
			FROM \
			  bancosmov bm \
			  INNER JOIN bancostransacc bt ON bm.idmovbanco = bt.idmovbanco and bt.tipodet='A' \
			  INNER JOIN bancoscuentas bc ON bc.idnumcuenta = bm.idnumcuenta \
			  INNER JOIN bancosnaturalezas bn ON bn.naturaleza = bc.naturaleza \
			  INNER JOIN bancosconceptomov bcm ON bm.conceptomov = bcm.conceptomov \
			  %s \
              %s \
			WHERE  %s %s \
			  bm.cancelado = 0 AND bm.aplicado = 1 AND bm.fechaaplbanco BETWEEN '%s' and '%s' %s \
			  %s %s \
			  AND bm.origen in ('PACLI', 'PACLE') \
			  AND bc.idempresa = %s \
			) \
			UNION ALL \
			( \
			SELECT \
			  bm.idmovbanco, bt.transacc, bc.idnumcuenta, bc.numerocuenta, bc.banco, \
			  bcm.descripcion AS descconceptomov, bm.descripcion, bm.identificador, \
			  bm.fechaaplbanco, bt.tipodet, bt.identificador AS identdet, \
			  bm.afectacion, bm.total as totalmov, bn.naturaleza, bm.sucursal, bm.origen, bm.fechaalta, bm.horaalta, \
			  'AJUSTE' AS referenciadoc, bt.total AS valordoc, '' AS razonsocialdoc, 'AJUSTE' AS pagodoc, 0.0 AS tracreditodoc, '' AS seriefolio, '' AS muuid, '' AS  muuidorefer, \
				0.0 AS proporcion0, 0.0 AS proporcion15, bt.total AS proporcion16, 0.0 AS proporcionsub0, 0.0 AS proporcionsub15, bt.subtotal AS proporcionsub16, \
				bt.subtotal AS proporcionsubgrabado, bt.subtotal AS proporcionsubtotal, bt.ivabanco AS proporcioniva, 0.0 AS proporcioniesps \
			FROM \
			  bancosmov bm \
			  INNER JOIN bancostransacc bt ON bm.idmovbanco = bt.idmovbanco and bt.tipodet='A' \
			  INNER JOIN bancoscuentas bc ON bc.idnumcuenta = bm.idnumcuenta \
			  INNER JOIN bancosnaturalezas bn ON bn.naturaleza = bc.naturaleza \
			  INNER JOIN bancosconceptomov bcm ON bm.conceptomov = bcm.conceptomov \
			  %s \
              %s \
			WHERE  %s %s \
			  bm.cancelado = 0 AND bm.aplicado = 1 AND bm.fechaaplbanco BETWEEN '%s' and '%s' %s \
			  %s %s \
			  AND bm.origen in ('PAPRO') \
			  AND bc.idempresa = %s \
			) \
			UNION ALL \
			(SELECT \
			  bm.idmovbanco, bt.transacc, bc.idnumcuenta, bc.numerocuenta, bc.banco, \
			  bcm.descripcion AS descconceptomov, bm.descripcion, bm.identificador, \
			  bm.fechaaplbanco, bt.tipodet, bt.identificador AS identdet, \
			  bm.afectacion, bm.total as totalmov, bn.naturaleza, bm.sucursal, bm.origen, bm.fechaalta, bm.horaalta, \
			  v.referencia AS referenciadoc, @valormov:=bxc.valorconsiderado AS valordoc, c.rsocial AS razonsocialdoc, \
			  txc.pago AS pagodoc, txc.tracredito AS tracreditodoc, \
			  cfd.seriefolio, cfd.muuid, \
			  IFNULL(cfd.muuid, v.referencia) AS muuidorefer, \
			  (@valormov*va.porcentaje0) as proporcion0, \
			  (@valormov*va.porcentaje15) as proporcion15, \
			  (@valormov*va.porcentaje16) as proporcion16, \
			  (@valormov*va.porcentajesub0) as proporcionsub0, \
			  (@valormov*va.porcentajesub15) as proporcionsub15, \
			  (@valormov*va.porcentajesub16) as proporcionsub16, \
			  (@valormov*va.porcentajesubgrabado) as proporcionsubgrabado, \
			  (@valormov*va.porcentajesubtotal) as proporcionsubtotal, \
			  (@valormov*va.porcentajeiva) as proporcioniva, \
			  (@valormov*va.porcentajeiesps) as proporcioniesps \
			FROM \
			  bancosmov bm \
			  INNER JOIN bancostransacc bt ON bm.idmovbanco = bt.idmovbanco \
			  INNER JOIN bancoscuentas bc ON bc.idnumcuenta = bm.idnumcuenta \
			  INNER JOIN bancosnaturalezas bn ON bn.naturaleza = bc.naturaleza \
			  INNER JOIN bancosconceptomov bcm ON bm.conceptomov = bcm.conceptomov \
			  INNER JOIN bancosxcob bxc ON bxc.transacc=bt.transacc \
			  INNER JOIN transxcob txc ON txc.tracredito=bxc.tracredito \
			  INNER JOIN ventas v ON v.referencia=txc.referencia \
			  INNER JOIN clientes c ON v.cliente=c.cliente \
			  INNER JOIN ventasaaux va ON v.referencia=va.referencia \
			  %s \
			  LEFT JOIN cfd ON v.referencia = cfd.referencia AND cfd.tipocomprobante = 'VENT' \
			WHERE  %s %s \
			  bm.cancelado = 0 AND bm.aplicado = 1 AND bm.fechaaplbanco BETWEEN '%s' and '%s' %s %s \
			  AND bm.origen in ('PACLI', 'PACLE') \
			  AND bc.idempresa = %s \
			) \
			UNION ALL \
			( \
			SELECT \
			  bm.idmovbanco, bt.transacc, bc.idnumcuenta, bc.numerocuenta, bc.banco, \
			  bcm.descripcion AS descconceptomov, bm.descripcion, bm.identificador, \
			  bm.fechaaplbanco, bt.tipodet, bt.identificador AS identdet, \
			  bm.afectacion, bm.total as totalmov, bn.naturaleza, bm.sucursal, bm.origen, bm.fechaalta, bm.horaalta, \
			  c.referencia AS referenciadoc, @valormov:=(txp.valor*-1) AS valordoc, p.razonsocial AS razonsocialdoc, \
			  txp.pago AS pagodoc, txp.tracredito AS tracreditodoc, \
			  c.folioprov AS seriefolio, c.muuid, \
			  IFNULL(c.muuid, c.referencia) AS muuidorefer, \
			  (@valormov*ca.porcentaje0) as proporcion0, \
			  (@valormov*ca.porcentaje15) as proporcion15, \
			  (@valormov*ca.porcentaje16) as proporcion16, \
			  (@valormov*ca.porcentajesub0) as proporcionsub0, \
			  (@valormov*ca.porcentajesub15) as proporcionsub15, \
			  (@valormov*ca.porcentajesub16) as proporcionsub16, \
			  (@valormov*ca.porcentajesubgrabado) as proporcionsubgrabado, \
			  (@valormov*ca.porcentajesubtotal) as proporcionsubtotal, \
			  (@valormov*ca.porcentajeiva) as proporcioniva, \
			  (@valormov*ca.porcentajeiesps) as proporcioniesps \
			FROM \
			  bancosmov bm \
			  INNER JOIN bancostransacc bt ON bm.idmovbanco = bt.idmovbanco \
			  INNER JOIN bancoscuentas bc ON bc.idnumcuenta = bm.idnumcuenta \
			  INNER JOIN bancosnaturalezas bn ON bn.naturaleza = bc.naturaleza \
			  INNER JOIN bancosconceptomov bcm ON bm.conceptomov = bcm.conceptomov \
			  INNER JOIN bancosxpag bxp ON bxp.transacc=bt.transacc \
			  INNER JOIN transxpag txp ON txp.tracredito=bxp.tracredito \
			  INNER JOIN compras c ON c.referencia=txp.referencia \
			  INNER JOIN proveedores p ON p.proveedor=c.proveedor \
			  %s \
			  INNER JOIN comprasaux ca ON c.referencia=ca.referencia \
			WHERE  %s %s \
			  bm.cancelado = 0 AND bm.aplicado = 1 AND bm.fechaaplbanco BETWEEN '%s' and '%s' %s %s \
			  AND bm.origen in ('PAPRO') \
			  AND bc.idempresa = %s \
			) \
			UNION ALL \
			( \
			SELECT \
			  bm.idmovbanco, 0 as transacc, bc.idnumcuenta, bc.numerocuenta, bc.banco, \
			  bcm.descripcion AS descconceptomov, bm.descripcion, bm.identificador, \
			  bm.fechaaplbanco, '' as tipodet, '' AS identdet, \
			  bm.afectacion, bm.total as totalmov, bn.naturaleza, bm.sucursal, bm.origen, bm.fechaalta, bm.horaalta, \
			  IF(ISNULL(cfd.muuid),'',bdv.venta) AS referenciadoc,  SUM(@valormov:=bdv.valorconsiderado) AS valordoc, c.rsocial AS razonsocialdoc, \
			  '' AS pagodoc, '' AS tracreditodoc, \
			  IFNULL(cfd.seriefolio,cfd2.seriefolio) AS seriefolio, IFNULL(cfd.muuid, cfd2.muuid) AS muuid, \
			  IFNULL(cfd.muuid, IFNULL(cfd2.muuid,v.referencia)) AS muuidorefer, \
			  SUM(@valormov*va.porcentaje0) as proporcion0, \
			  SUM(@valormov*va.porcentaje15) as proporcion15, \
			  SUM(@valormov*va.porcentaje16) as proporcion16, \
			  SUM(@valormov*va.porcentajesub0) as proporcionsub0, \
			  SUM(@valormov*va.porcentajesub15) as proporcionsub15, \
			  SUM(@valormov*va.porcentajesub16) as proporcionsub16, \
			  SUM(@valormov*va.porcentajesubgrabado) as proporcionsubgrabado, \
			  SUM(@valormov*va.porcentajesubtotal) as proporcionsubtotal, \
			  SUM(@valormov*va.porcentajeiva) as proporcioniva, \
			  SUM(@valormov*va.porcentajeiesps) as proporcioniesps \
			FROM \
			  bancosmov bm \
			  INNER JOIN bancoscuentas bc ON bc.idnumcuenta = bm.idnumcuenta \
			  INNER JOIN bancosnaturalezas bn ON bn.naturaleza = bc.naturaleza \
			  INNER JOIN bancosconceptomov bcm ON bm.conceptomov = bcm.conceptomov \
			  INNER JOIN bancosxventas bxv ON bxv.idmovbanco=bm.idmovbanco \
			  INNER JOIN bancosdventas bdv ON bdv.idbancosventas=bxv.idbancosventas and bdv.idmovbanco=bm.idmovbanco \
			  INNER JOIN ventas v ON v.referencia=bdv.venta \
			  INNER JOIN clientes c ON v.cliente=c.cliente \
			  INNER JOIN ventasbaux va ON v.referencia=va.referencia \
			  LEFT JOIN cfd  ON v.referencia=cfd.referencia AND cfd.tipocomprobante='VENT' AND v.ticket=0 \
			  LEFT JOIN cfd AS cfd2  ON v.ticket=1 AND cfd2.estado=1 AND \
				cfd2.tipocomprobante='TICK' AND DATE_FORMAT(v.fechavta,'%%d/%%m/%%Y') = cfd2.referencia AND \
				bm.sucursal=cfd2.sucursal \
			  %s \
			WHERE  %s %s \
			  bm.cancelado = 0 AND bm.aplicado = 1 \
			  AND bm.fechaaplbanco BETWEEN '%s' and '%s' %s %s  \
			  AND bm.origen IN ('VENCE','VENTC') \
			  AND bc.idempresa = %s \
			GROUP BY idmovbanco, muuidorefer \
			) \
			UNION ALL \
			( \
			SELECT \
			  bm.idmovbanco, bt.transacc, bc.idnumcuenta, bc.numerocuenta, bc.banco, \
			  bcm.descripcion AS descconceptomov, bm.descripcion, bm.identificador, \
			  bm.fechaaplbanco, bt.tipodet, bt.identificador AS identdet, \
			  bm.afectacion, bm.total as totalmov, bn.naturaleza, bm.sucursal, bm.origen, bm.fechaalta, bm.horaalta, \
			  '' AS referenciadoc,  @valormov:=bt.total AS valordoc, '' AS razonsocialdoc, \
			  '' AS pagodoc, '' AS tracreditodoc, \
			  '' AS seriefolio, '' AS muuid, \
			  '' AS muuidorefer, \
			  if( ifnull(i1.tipoimpu,'')='IVA' AND ifnull(i1.porcentaje,0)=0.0, bt.total, 0) as proporcion0, \
			  if( ifnull(i1.tipoimpu,'')='IVA' AND ifnull(i1.porcentaje,0)=15.0, bt.total, 0) as proporcion15, \
			  if( ifnull(i1.tipoimpu,'')='IVA' AND ifnull(i1.porcentaje,0)=16.0, bt.total, 0) as proporcion16, \
			  @subnograbado:=if( ifnull(i1.tipoimpu,'')='IVA' AND ifnull(i1.porcentaje,0)=0.0, bt.subtotal, 0) as proporcionsub0, \
			  @subtotal15:=if( ifnull(i1.tipoimpu,'')='IVA' AND ifnull(i1.porcentaje,0)=15.0, bt.subtotal, 0) as proporcionsub15, \
			  @subtotal16:=if( ifnull(i1.tipoimpu,'')='IVA' AND ifnull(i1.porcentaje,0)=16.0, bt.subtotal, 0) as proporcionsub16, \
			  (@subtotal15+@subtotal16) as proporcionsubgrabado, \
			  (@subnograbado+@subtotal15+@subtotal16) as proporcionsubtotal, \
			  if( ifnull(i1.tipoimpu,'')='IVA' AND ifnull(i1.porcentaje,0)>0.0, bt.ivabanco, 0) as proporcioniva, \
			  0.0 as proporcioniesps \
			FROM \
			  bancosmov bm \
			  INNER JOIN bancostransacc bt ON bm.idmovbanco = bt.idmovbanco \
			  INNER JOIN bancoscuentas bc ON bc.idnumcuenta = bm.idnumcuenta \
			  INNER JOIN bancosnaturalezas bn ON bn.naturaleza = bc.naturaleza \
			  INNER JOIN bancosconceptomov bcm ON bm.conceptomov = bcm.conceptomov \
			  left join impuestos i1 on i1.impuesto=bt.cveimp \
			  %s \
			WHERE %s %s \
			  bm.cancelado = 0 AND bm.aplicado = 1 \
			  AND bm.fechaaplbanco BETWEEN '%s' and '%s' %s %s \
			  %s \
			  AND bm.origen NOT IN ('PACLI', 'PACLE', 'PAPRO', 'VENCE','VENTC') \
			  AND bc.idempresa = %s \
			) \
			ORDER BY idnumcuenta, fechaaplbanco, fechaalta, horaalta, idmovbanco, transacc ",
							joinpagoscli_ajustes, join_sucemp_bancosmov,
							opcionNaturaleza, opcionCuenta, fecha_ini, fecha_fin, opcionorigenes, fechaoperajustes, opcionsucursales,
							empresa, //10
							joinpagosprov_ajustes, join_sucemp_bancosmov,
							opcionNaturaleza, opcionCuenta, fecha_ini, fecha_fin, opcionorigenes, fechaoperajustes, opcionsucursales,
							empresa, //20
							joinpagos_ventasref,
							opcionNaturaleza, opcionCuenta, fecha_ini, fecha_fin, opcionorigenes, fechaoperpagos_ventasref,
							empresa, //28
							joinpagos_comprasref,
							opcionNaturaleza, opcionCuenta, fecha_ini, fecha_fin, opcionorigenes, fechaoperpagos_comprasref,
							empresa, //36
							join_sucemp_bancosmov, opcionNaturaleza, opcionCuenta, fecha_ini, fecha_fin, opcionsucursales, opcionorigenes,
							empresa, //44
							join_sucemp_bancosmov, opcionNaturaleza, opcionCuenta, fecha_ini, fecha_fin, opcionsucursales, opcionorigenes,
							fechaoper_varios ,empresa //53
							);
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
}
// ---------------------------------------------------------------------------
// ID_CON_REPSALDOS
void ServidorBancos::ReporteSaldos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros) {

	AnsiString instruccion, instrucciones[10];
	AnsiString fecha_ini;
	TDate fecha=Today();
	char *buffer_sql=new char[1024*64*20];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i;

	fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

	try{
		instruccion.sprintf("CREATE TEMPORARY TABLE existenciasaux ( idnumcuenta INT(11), \
		afectacion VARCHAR(1), cargo DECIMAL(16,2), abono DECIMAL(16,2)) ENGINE = INNODB");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("INSERT INTO existenciasaux SELECT bm.idnumcuenta, bm.afectacion, \
		IF(bm.afectacion = 'C', bt.total,0) AS cargo,IF(bm.afectacion = 'A', bt.total,0) AS abono FROM \
		bancosmov bm INNER JOIN bancostransacc bt ON bm.idmovbanco = bt.idmovbanco \
		WHERE bm.cancelado = 0 AND bm.aplicado = 1 \
		AND bm.fechaaplbanco <= '%s' ",fecha_ini);
		instrucciones[num_instrucciones++]=instruccion;

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
		if(mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)){

			instruccion.sprintf("SELECT bc.numerocuenta AS cuenta, bc.descripcion, bc.saldoinicial, \
			IFNULL(SUM(ea.cargo),0) AS cargo, IFNULL(SUM(ea.abono),0) AS abono FROM \
			bancoscuentas bc LEFT JOIN existenciasaux ea ON bc.idnumcuenta = ea.idnumcuenta GROUP BY \
			ea.idnumcuenta, bc.numerocuenta, bc.banco");
			mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),Respuesta->TamBufferResultado);
		}

	}
	 __finally {
		delete buffer_sql;
	}
}

// ---------------------------------------------------------------------------

/*******************************************************************************/
/**********************************BANCOS X VENTAS******************************/
/*******************************************************************************/

//ID_GRA_BANCVTA
void ServidorBancos::GrabaBancoVta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  GRABA BANCO X VENTA
	char *buffer_sql=new char[1024*10000];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString origen;
	AnsiString tarea_bancovta, folio_bancovta, banco, tipo, descripcion, identificador,cancelado;
	AnsiString sucursal, termino, total, fechaventas, fecha_alta, hora_alta, fecha_mod, hora_mod;
	AnsiString usuario_alta,usuario_mod, terminal;
	int num_partidas, i, error;
	int num_instrucciones=0, idintruccion = 0, idint = 0;
	AnsiString instruccion, instrucciones[10000];
	AnsiString idmovbancop[100],idmovbanco[100], totalbanco[100],totalbancop[100], folios[100];
	double totalvta = 0.00;
	double comptotalvta = 0.00, total_compara = 0.00;
	double resto, faltante = 0.00, valorcomp, faltanteunico = 0.00;
	AnsiString respadoidmovbanco, mensaje, cadenaID, totalban, folio, respaldofolio, residmovbanco, respfolio;
	AnsiString rventa, totalventas, condicion_id = " ";
	bool valida = true;
	int cont = 0;
	TDate fecha = Today();
	TTime hora = Time();

	try {
		folio_bancovta=mFg.ExtraeStringDeBuffer(&parametros); // Folio del movimiento bancario.
		tarea_bancovta=mFg.ExtraeStringDeBuffer(&parametros); // Indicador si se va a agregar o modificar.
		sucursal=mFg.ExtraeStringDeBuffer(&parametros); //sucursal
		fechaventas=mFg.ExtraeStringDeBuffer(&parametros); //Fecha de ventas
		fecha_alta=mFg.ExtraeStringDeBuffer(&parametros); //Fecha de alta del movimiento
		hora_alta=mFg.ExtraeStringDeBuffer(&parametros); //Hora de alta del movimiento
		fecha_mod=mFg.ExtraeStringDeBuffer(&parametros); //Fecha de modificación del movimiento
		hora_mod=mFg.ExtraeStringDeBuffer(&parametros); //Hora de modificación del movimiento
		usuario_alta=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que esta grabando el movimiento.
		usuario_mod=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que esta modificando el movimiento.
		termino=mFg.ExtraeStringDeBuffer(&parametros); // tipo de venta.
		total=mFg.ExtraeStringDeBuffer(&parametros); //Total
		totalventas=mFg.ExtraeStringDeBuffer(&parametros); //Total  de las ventas
		cancelado=mFg.ExtraeStringDeBuffer(&parametros); //Si esta cancelada la cuenta
		num_partidas=StrToInt(mFg.ExtraeStringDeBuffer(&parametros)); //Número de partidas

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

			// Ingresa el movimiento
			if (tarea_bancovta=="A") {
				instruccion.sprintf("INSERT INTO bancosventasdef (sucursal, fechaventas, fechaalta, horaalta, \
				fechamodi, horamodi, usualta, usumodi, termino, total, totalventas, cancelado )  \
				VALUES ('%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s')",
				sucursal, mFg.StrToMySqlDate(fechaventas), mFg.DateToMySqlDate(fecha),mFg.TimeToMySqlTime(hora)
				,mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario_alta, usuario_mod,
				termino, total, totalventas, cancelado);
				instrucciones[num_instrucciones++]=instruccion;

				instrucciones[num_instrucciones++]="set @folio:=LAST_INSERT_ID()";
			} //Se  modifica el movimiento
			else {
					instruccion.sprintf("delete from bancosdventas where idbancosventas='%s'",folio_bancovta);
					instrucciones[num_instrucciones++]=instruccion;

                    instruccion.sprintf("UPDATE bancosmov bm INNER JOIN bancosxventas bv ON \
					bm.idmovbanco = bv.idmovbanco  \
					SET bm.cancelado = '1' WHERE bv.idbancosventas = '%s'", folio_bancovta);
					instrucciones[num_instrucciones++]=instruccion;

					instruccion.sprintf("delete from bancosxventas where idbancosventas='%s'",folio_bancovta);
					instrucciones[num_instrucciones++]=instruccion;

					instruccion.sprintf("update bancosventasdef set sucursal = '%s', fechaventas = '%s', \
					fechamodi = '%s',horamodi = '%s', usumodi = '%s', termino = '%s', total = '%s' \
					, totalventas = '%s' where idbancosventas = '%s' ",
					sucursal, mFg.StrToMySqlDate(fechaventas),  mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora),
					usuario_mod, termino, total, totalventas, folio_bancovta);

					instrucciones[num_instrucciones++]=instruccion;

					instruccion.sprintf("set @folio:=%s",folio_bancovta);
					instrucciones[num_instrucciones++]=instruccion;

					//Se obtienen todos los id de movimientos bancarios antes de eliminarlos
					BufferRespuestas* resp_n=NULL;
					try{
						instruccion.sprintf("SELECT idmovbanco FROM bancosxventas WHERE idbancosventas = '%s'",folio_bancovta);
						mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_n);

						for(i = 0; i < resp_n->ObtieneNumRegistros(); i++){
							if(i == resp_n->ObtieneNumRegistros() - 1)
								cadenaID += resp_n->ObtieneDato("idmovbanco");
								else
								cadenaID += resp_n->ObtieneDato("idmovbanco") +  ",";

							resp_n->IrAlSiguienteRegistro();
						}

					}
					__finally {
						if (resp_n!=NULL) delete resp_n;
					}

					//Se eliminan los movimientos ingresado y su relacion, para volver a ingresarlos
					instruccion.sprintf("delete from bancosxventas where idbancosventas='%s'",folio_bancovta);
					instrucciones[num_instrucciones++]=instruccion;

					instruccion.sprintf("delete from bancosdventas where idbancosventas='%s'",folio_bancovta);
					instrucciones[num_instrucciones++]=instruccion;

				}

			//Ingresa los movimientos asignados a arreglos
			for (i=0; i<num_partidas; i++) {

				datos.AsignaTabla("bancosxventas");
				parametros+=datos.InsCamposDesdeBuffer(parametros);
				idmovbancop[idintruccion] =  datos.ObtieneValorCampo("idmovbanco");
				totalbancop[idintruccion] =  datos.ObtieneValorCampo("idbancosventas");
				instruccion.sprintf("INSERT INTO bancosxventas VALUES (@folio,'%s')", datos.ObtieneValorCampo("idmovbanco"));
				instrucciones[num_instrucciones++] = instruccion;

				/*Al ingresar los movimientos los aplica poniendo 1 en el campo de aplicado*/
				instruccion.sprintf("update bancosmov set aplicado = '1', cancelado = '0' \
				where idmovbanco = '%s' ", datos.ObtieneValorCampo("idmovbanco"));
				instrucciones[num_instrucciones++] = instruccion;

				idintruccion++;
			}

			//Llena la cadena con los ID
			if (tarea_bancovta=="A"){
				for(i = 0; i< num_partidas; i++){
					if(i == num_partidas - 1)
						cadenaID += idmovbancop[i];
						else
						cadenaID += idmovbancop[i] +  ",";
				}
			}

				instruccion.sprintf("delete from bancosdventas where idmovbanco IN ('%s')", cadenaID);
				instrucciones[num_instrucciones++]=instruccion;

				int idint = 0;
				BufferRespuestas* resp_id=NULL;
				try {
					//Busca los ID de las relaciones anteriores del mismo día
					if (tarea_bancovta=="M")
						condicion_id.sprintf("AND bxv.idmovbanco NOT IN (%s)",cadenaID);

						instruccion.sprintf("SELECT bxv.idmovbanco FROM bancosventasdef bvd INNER JOIN bancosxventas \
						bxv ON bvd.idbancosventas = bxv.idbancosventas WHERE bvd.fechaventas = '%s'  \
						AND bvd.sucursal = '%s' AND bvd.termino = '%s' AND bvd.cancelado = 0 %s \
						ORDER BY bxv.idmovbanco",
						mFg.StrToMySqlDate(fechaventas), sucursal, termino, condicion_id);

					if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_id)) {
						if (resp_id->ObtieneNumRegistros()>0){
							for(int i=1; i<resp_id->ObtieneNumRegistros()+1; i++) {

								//Elimina relación de movimientos bancarios con ventas
								instruccion.sprintf("delete from bancosdventas where idmovbanco = '%s'", resp_id->ObtieneDato("idmovbanco"));
								instrucciones[num_instrucciones++]=instruccion;

								//Calcula el total de los movimientos bancarios seleccionados
								BufferRespuestas* resp_total=NULL;
								try{
									instruccion.sprintf("SELECT total FROM bancosmov WHERE idmovbanco = '%s'",resp_id->ObtieneDato("idmovbanco"));
									mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_total);
									totalban =  resp_total->ObtieneDato("total");
								}
								__finally {
									if (resp_total!=NULL) delete resp_total;
								}

								//Relacion con los folios de los movimientos
								BufferRespuestas* resp_folio=NULL;
								try{
									instruccion.sprintf("SELECT idbancosventas FROM bancosxventas WHERE idmovbanco =  '%s'",resp_id->ObtieneDato("idmovbanco"));
									mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_folio);
									folio =  resp_folio->ObtieneDato("idbancosventas");
								}
								__finally {
									if (resp_folio!=NULL) delete resp_folio;
								}

								//Se llena el arreglo con los movimientos ingresados en los movimientos anteriores del mismo día
								idmovbanco[idint] =  resp_id->ObtieneDato("idmovbanco");
								totalbanco[idint] =  totalban;
								folios[idint] =	folio;
								idint++;
								resp_id->IrAlSiguienteRegistro();
							}

							//Continua llenando el arreglo con los movimientos ingresados en el movimiento actual
							for(i = 0; i < num_partidas; i++ ){
								idmovbanco[idint] = idmovbancop[i];
								totalbanco[idint] =  totalbancop[i];
								folios[idint] =	"N";
								idint++;
							}
						}  else{
							//Se llena el arreglo con los movimientos ingresados en el movimiento actual
								for(i = 0; i < num_partidas; i++ ){
									idmovbanco[idint] = idmovbancop[i];
									totalbanco[idint] =  totalbancop[i];
									folios[idint] =	"N";
									idint++;
								}
						}

					}else throw (Exception("Error al consultar en tabla bancosventasdef"));
				}
				__finally {
				if (resp_id!=NULL) delete resp_id;
				}

			int indice = idint;
			idint = 0;
			//Calcula el total de los movimientos del día
			for(i = 0; i < indice;i++)
			{
				total_compara += totalbanco[i].ToDouble();
			}

			//Bsuqeda del parametro para establecer limite de diferencia en la relación
			AnsiString tolerdifrel;
			BufferRespuestas* resp_limite=NULL;
			try{
				instruccion = "SELECT valor FROM parambancos WHERE parametro =  'TOLERDIFREL'";
				mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_limite);
				tolerdifrel =  resp_limite->ObtieneDato("valor");
			}
			__finally {
				if (resp_limite!=NULL) delete resp_limite;
			}

			BufferRespuestas* resp=NULL;
			try {
				//Se calcula el total de ventas del dia en base a los criterios
				instruccion.sprintf("SELECT v.referencia,SUM(d.valor) as valor, SUM(IFNULL(n.valor,0)) AS valorNC \
					FROM ventas v  \
					INNER JOIN dventasfpago d ON d.referencia = v.referencia \
					INNER JOIN formasdepago f ON f.formapag = d.formapag \
					INNER JOIN terminales t ON t.terminal = v.terminal \
					INNER JOIN secciones s ON t.seccion = s.seccion \
                    LEFT JOIN notascredcli n ON n.venta = v.referencia	AND n.cancelado = 0 \
							WHERE v.fechavta='%s' AND v.cancelado = 0  \
					AND f.termino = '%s' AND s.sucursal = '%s' GROUP BY v.referencia ORDER BY v.referencia",
				mFg.StrToMySqlDate(fechaventas), termino, sucursal);
				if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp)) {
					if (resp->ObtieneNumRegistros()>0){
						for(int i=0; i<resp->ObtieneNumRegistros(); i++) {
							comptotalvta += resp->ObtieneDato("valor").ToDouble() - resp->ObtieneDato("valorNC").ToDouble();
							resp->IrAlSiguienteRegistro();
						}

						//Comparación
						valorcomp = abs(comptotalvta - total_compara);
						if(total_compara > (comptotalvta + mFg.CadenaAFlotante(tolerdifrel)) ){
								AnsiString ctv = mFg.FormateaCantidad(comptotalvta,2,true);
								AnsiString vc = mFg.FormateaCantidad(valorcomp,2,true);
								mensaje = "El total de los movimientos es mayor que las ventas.\n";
								mensaje += "Total de movimientos: " + mFg.FormateaCantidad(total_compara) + "\n";
								mensaje += "Total de ventas con";
									if(termino == "CONEF")
										mensaje += " EFECTIVO";
									if(termino == "TARCR")
										mensaje += " TARJETA DE CRÉDITO";
								mensaje += ": " + ctv + "\n";
								mensaje += "Diferencia entre total de movimientos y ventas: " + vc + "\n";
								mensaje += "\nNo se permite que los movimientos sean mayores a las ventas.";
								throw (Exception(mensaje));
						}

						resp->IrAlPrimerDato();

						//Inicia el ingreso de la relación de movimientos bancarios con ventas
						for(int i=0; i<resp->ObtieneNumRegistros(); i++) {
							if(totalbanco[idint] == ""){
							   i = resp->ObtieneNumRegistros();
							   valida = false;
							}

								if(idmovbanco[idint] != ""){

									if(folios[idint] == "N"){
										double valorConsiderado = resp->ObtieneDato("valor").ToDouble() - resp->ObtieneDato("valorNC").ToDouble();

										instruccion.sprintf("insert into bancosdventas values (@folio, '%s', '%s', '%f')",
										idmovbanco[idint], resp->ObtieneDato("referencia"),
										valorConsiderado);
									}
									else{
										double valorConsiderado = resp->ObtieneDato("valor").ToDouble() - resp->ObtieneDato("valorNC").ToDouble();

										instruccion.sprintf("insert into bancosdventas values ('%s', '%s', '%s', '%f')",
										folios[idint],idmovbanco[idint], resp->ObtieneDato("referencia"),
										valorConsiderado);
									}
								instrucciones[num_instrucciones++] = instruccion;
								}


								 if(valida){
								 totalvta = totalvta + resp->ObtieneDato("valor").ToDouble() - resp->ObtieneDato("valorNC").ToDouble() ;
									while(totalbanco[idint] != "" && totalvta >= totalbanco[idint].ToDouble()){
									   resto = totalvta - totalbanco[idint].ToDouble();
									   if(mFg.EsCero(resto))
											resto = 0;
									   respadoidmovbanco = idmovbanco[idint];
									   respaldofolio = folios[idint];
									   idint++;

									   if(resto > 0 ){

										   if(idmovbanco[idint] == ""){
												residmovbanco = respadoidmovbanco;
												respfolio = respaldofolio;
												cont = 1;
										   }

										   if(cont == 0){

										   if(idmovbanco[idint] == ""){
												residmovbanco = respadoidmovbanco;
												respfolio = respaldofolio;
										   }
										   else{
												residmovbanco = idmovbanco[idint];
												respfolio = folios[idint];
										   }
										   if(respfolio == "N"){
											   instruccion.sprintf("insert into bancosdventas values (@folio, '%s', '%s', '%f')",
											   residmovbanco, resp->ObtieneDato("referencia"), resto );
										   }
										   else{
                                           		instruccion.sprintf("insert into bancosdventas values ('%s', '%s', '%s', '%f')",
											   respfolio, residmovbanco, resp->ObtieneDato("referencia"), resto );
										   }
										   instrucciones[num_instrucciones++] = instruccion;
										   }

										   if(rventa == resp->ObtieneDato("referencia")){
										   faltanteunico = faltanteunico + faltante;
										   faltante = resp->ObtieneDato("valor").ToDouble() - resp->ObtieneDato("valorNC").ToDouble() - faltanteunico - resto /* - faltanterespaldo - faltanteunico*/;
										   }
												else{
													faltante = resp->ObtieneDato("valor").ToDouble() - resp->ObtieneDato("valorNC").ToDouble() - resto /*- faltanterespaldo*/;
													}

											   if(respaldofolio == "N"){
												   instruccion.sprintf("UPDATE bancosdventas SET valorconsiderado = '%f' \
												   WHERE idbancosventas = @folio AND idmovbanco = '%s' AND venta = '%s' ",
												   faltante, respadoidmovbanco ,resp->ObtieneDato("referencia"));
											   }
											   else{
													 instruccion.sprintf("UPDATE bancosdventas SET valorconsiderado = '%f' \
												   WHERE idbancosventas = '%s' AND idmovbanco = '%s' AND venta = '%s' ",
												   faltante, respaldofolio, respadoidmovbanco ,resp->ObtieneDato("referencia"));
											   }
										   instrucciones[num_instrucciones++] = instruccion;

										   if(idmovbanco[idint] == "")
												i = resp->ObtieneNumRegistros();

										   cont = 0;
									   }

									   totalvta =  resto;
									   resto = 0.00;
									   rventa = resp->ObtieneDato("referencia");
									}
									faltanteunico = 0.00;
									faltante = 0.00;
							  	 }

							resp->IrAlSiguienteRegistro();
						}
					} else throw (Exception("No se encuentran ventas en la fecha: \n" + fechaventas));
				} else throw (Exception("Error al consultar en tabla ventas"));
			} __finally {
				if (resp!=NULL) delete resp;
			}

			instrucciones[num_instrucciones++]="COMMIT";

			// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_REP);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			instruccion.sprintf("select %d as error, @folio as folio, '%s' as  fecha", error ,fechaventas);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Obtiene todo el detalle del movimiento.

			instruccion = "SELECT b.idmovbanco,IF(ISNULL(cfd.muuid),'',b.venta) as venta, IFNULL(cfd.muuid,'') AS muuid, \
			IFNULL(cfd.seriefolio,'') AS seriefolio, IFNULL(cfd2.seriefolio,'') AS seriefolio2, \
			IFNULL(cfd2.muuid,v.referencia) AS muuid2, c.rsocial, sum(b.valorconsiderado) as valorconsiderado,  \
			IFNULL(cfd.muuid, IFNULL(cfd2.muuid,v.referencia)) AS muuidorefer, IF(ISNULL(cfd2.muuid),'',b.venta) as venta2 \
				FROM bancosdventas b      \
					 INNER JOIN ventas v ON v.referencia=b.venta \
					 INNER JOIN clientes c ON v.cliente=c.cliente \
					 INNER JOIN terminales t ON t.terminal = v.terminal \
					 INNER JOIN secciones s ON t.seccion = s.seccion  \
					 LEFT JOIN cfd ON cfd.tipocomprobante = 'VENT' AND cfd.estado = 1 \
					 AND cfd.referencia = b.venta AND v.ticket=0  \
					 LEFT JOIN cfd cfd2 ON cfd2.estado = 1 AND v.ticket=1 \
					 AND DATE_FORMAT(v.fechavta,'%d/%m/%Y') = cfd2.referencia \
					 AND s.sucursal = cfd2.sucursal AND cfd2.tipocomprobante='TICK' \
					 WHERE b.idbancosventas =  @folio GROUP BY b.idmovbanco, muuidorefer \
					 order by b.idmovbanco ";
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	} __finally {
		if(buffer_sql != NULL) delete buffer_sql;
	}
}

//---------------------------------------------------------------------------
//ID_BUSQ_BANCVTA
void ServidorBancos::BuscaBancoVta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// BUSQUEDA DE MOVIMIENTOS BANCARIOS
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instruccion;
	AnsiString tipo_busqueda;
	AnsiString dato_buscado,dato_buscado2, fecha_ini, fecha_fin;
	AnsiString revision_alma_ent, revision_alma_sal;

	// Extrae el tipo de búsqueda
	tipo_busqueda=mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ);

	if (tipo_busqueda=="MOVID"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);

		instruccion = "SELECT idbancosventas,sucursal, DATE_FORMAT(fechaventas,'%d-%m-%Y') AS fecha,";
		instruccion += "termino, total, totalventas, cancelado FROM bancosventasdef where idbancosventas = '";
		instruccion += dato_buscado;
		instruccion += "'";
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="SUCURSAL"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion = "SELECT idbancosventas,sucursal, DATE_FORMAT(fechaventas,'%d-%m-%Y') AS fecha,";
		instruccion += "termino, total, totalventas, cancelado FROM bancosventasdef where sucursal = '";
		instruccion += dato_buscado;
		instruccion += "' and fechaventas >= '";
		instruccion += fecha_ini;
		instruccion += "' and fechaventas <= '";
		instruccion += fecha_fin;
		instruccion += "'";
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="TIPO"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion = "SELECT idbancosventas,sucursal, DATE_FORMAT(fechaventas,'%d-%m-%Y') AS fecha,";
		instruccion += "termino, total, totalventas, cancelado FROM bancosventasdef where termino = '";
		instruccion += dato_buscado;
		instruccion += "' and fechaventas >= '";
		instruccion += fecha_ini;
		instruccion += "' and fechaventas <= '";
		instruccion += fecha_fin;
		instruccion += "'";
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="FECHA"){
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion = "SELECT idbancosventas,sucursal, DATE_FORMAT(fechaventas,'%d-%m-%Y') AS fecha,";
		instruccion += "termino, total, totalventas, cancelado FROM bancosventasdef where fechaventas >= '";
		instruccion += fecha_ini;
		instruccion += "' and fechaventas <= '";
		instruccion += fecha_fin;
		instruccion += "'";
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="FECHAALTA"){
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion = "SELECT idbancosventas,sucursal, DATE_FORMAT(fechaventas,'%d-%m-%Y') AS fecha,";
		instruccion += "termino, total, totalventas, cancelado FROM bancosventasdef where fechaalta >= '";
		instruccion += fecha_ini;
		instruccion += "' and fechaalta <= '";
		instruccion += fecha_fin;
		instruccion += "'";
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}

}

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//ID_CON_BANCVTA
void ServidorBancos::ConsultaBancoVta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    // CONSULTA MOVIMIENTO DE ALMACEN
	AnsiString busqueda;
	AnsiString id, modo;

	id=mFg.ExtraeStringDeBuffer(&parametros);
	modo=mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Detalle generales del movimiento
	busqueda = "SELECT idbancosventas,sucursal,fechaventas, fechaalta, termino, total, cancelado FROM bancosventasdef ";
	busqueda += "WHERE idbancosventas = '";
	busqueda +=	id ;
	busqueda += "'";
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, busqueda.c_str(), Respuesta->TamBufferResultado);

	// Detalle del movimiento
	busqueda = "SELECT bm.idmovbanco,bc.banco, bc.numerocuenta,";
	busqueda += "bcm.descripcion AS conceptomov,";
	busqueda += "bm.total, DATE_FORMAT(bm.fechaaplbanco,'%d-%m-%Y') as fecha";
	busqueda += " FROM bancosxventas bxv INNER JOIN bancosmov bm ON bm.idmovbanco = bxv.idmovbanco ";
	busqueda += "INNER JOIN bancoscuentas bc ON bc.idnumcuenta = bm.idnumcuenta ";
	busqueda += "INNER JOIN  bancosconceptomov  bcm ON bcm.conceptomov = bm.conceptomov WHERE bxv.idbancosventas = '";
	busqueda +=	id ;
	busqueda += "' ORDER BY bm.idmovbanco";
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, busqueda.c_str(), Respuesta->TamBufferResultado);

	// Obtiene todas las ventas del movimiento.
	if(modo == "BXV")
		busqueda = "SELECT b.idmovbanco, IF(ISNULL(cfd.muuid),'',b.venta) as venta, IF(ISNULL(cfd2.muuid),'',b.venta) as venta2, IFNULL(cfd.muuid,'') AS muuid, ";
		else
			busqueda = "SELECT b.idmovbanco, b.venta as venta, IFNULL(cfd.muuid,'') AS muuid, ";
	busqueda +=	"IFNULL(cfd.seriefolio,'') AS seriefolio, IFNULL(cfd2.seriefolio,'') AS seriefolio2, ";

	if(modo == "BXV")
		busqueda += "IFNULL(cfd2.muuid,'') AS muuid2, c.rsocial, sum(b.valorconsiderado) as valorconsiderado, ";
		else
			busqueda += "IFNULL(cfd2.muuid,'') AS muuid2, c.rsocial, b.valorconsiderado, ";

	busqueda += "IFNULL(cfd.muuid, IFNULL(cfd2.muuid,v.referencia)) AS muuidorefer ";
	busqueda += "FROM bancosdventas b ";
	busqueda += "INNER JOIN ventas v ON v.referencia=b.venta ";
	busqueda += "INNER JOIN clientes c ON v.cliente=c.cliente ";
	busqueda += "INNER JOIN terminales t ON t.terminal = v.terminal ";
	busqueda += "INNER JOIN secciones s ON t.seccion = s.seccion ";
	busqueda += "LEFT JOIN cfd ON cfd.tipocomprobante = 'VENT' AND cfd.estado = 1 ";
	busqueda += "AND cfd.referencia = b.venta ";
	busqueda += "LEFT JOIN cfd cfd2 ON  cfd2.estado = 1 AND v.ticket=1 ";
	busqueda += "AND DATE_FORMAT(v.fechavta,'%d/%m/%Y') = cfd2.referencia AND cfd2.tipocomprobante='TICK' ";
	busqueda += "AND s.sucursal = cfd2.sucursal ";
	busqueda += "WHERE b.idbancosventas = '";
	busqueda +=	id ;
	busqueda += "' ";

	if(modo == "BXV")
		busqueda +="GROUP BY idmovbanco, muuidorefer ";

	busqueda +="ORDER BY b.idmovbanco ";

	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, busqueda.c_str(), Respuesta->TamBufferResultado);
}

//----------------------------------------------------------------------------
//ID_CANC_BANCVTA
void ServidorBancos::CancelaBancoVta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  CANCELA VENTA
	char *buffer_sql=new char[1024*32];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instrucciones[30], instruccion;
	int num_instrucciones=0;
	AnsiString id;
	int error=0;
	int i;


	try{
		id=mFg.ExtraeStringDeBuffer(&parametros); // ID del movimiento bancario

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		// Cancela la venta
		instruccion.sprintf("update bancosventasdef set cancelado=1 where idbancosventas='%s' and cancelado=0",id);
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("UPDATE bancosmov bm INNER JOIN bancosxventas bv ON bm.idmovbanco = bv.idmovbanco  \
			SET bm.cancelado = '1' WHERE bv.idbancosventas = '%s'", id);
		instrucciones[num_instrucciones++]=instruccion;

		instrucciones[num_instrucciones++]="COMMIT";

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

// ---------------------------------------------------------------------------
/*GENERICOS*/
// ---------------------------------------------------------------------------
//
void ServidorBancos::GrabaGenerico(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros, AnsiString Tabla,
	AnsiString CampoClave) {
	// GRABA GENERICO
	char *buffer_sql = new char[1024 * 5];
	char *aux_buffer_sql = buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instrucciones[5];
	int num_instrucciones = 0;
	AnsiString clave;
	AnsiString tarea;
	AnsiString clave_formateada, ventasxvolumen;
	char tipo;
	int i;

	try {
		tarea = mFg.ExtraeStringDeBuffer(&parametros);
		// Indicador si se va a agregar o modificar
		clave = mFg.ExtraeStringDeBuffer(&parametros); // Clave

		datos.AsignaTabla(Tabla.c_str());
		parametros += datos.InsCamposDesdeBuffer(parametros);
		if (tarea == "A")
			instrucciones[num_instrucciones] = datos.GenerarSqlInsert();
		else {
			tipo = datos.ObtieneTipoCampo(CampoClave);
			if (tipo == 'I' || tipo == 'N' || tipo == 'F')
				clave_formateada = clave;
			else
				clave_formateada = "'" + clave + "'";
			instrucciones[num_instrucciones] = datos.GenerarSqlUpdate
				(CampoClave + "=" + clave_formateada);
		}
		num_instrucciones++;

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql = mFg.AgregaStringABuffer
			(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i = 0; i < num_instrucciones; i++)
			aux_buffer_sql = mFg.AgregaStringABuffer(instrucciones[i],
			aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta,
			TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,
			buffer_sql);
	}
	__finally {
		delete buffer_sql;
	}
}
//----------------------------------------------------------------------------
//ID_CON_REPVTAMOVBANC
void ServidorBancos::RepRelacionVentaMovimientosBancarios(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros){

	char *buffer_sql=new char[1024*64*10];
	AnsiString instruccion;

	AnsiString fecha_inicial, fecha_final, sucursal, conceptomov;
	AnsiString activo;

	AnsiString condicion_sucursal=" ";
	AnsiString condicion_sucursalvta=" ";
	AnsiString condicion_conceptomov=" ";
	AnsiString condicion_conmovventa=" ";

	try{
		fecha_inicial=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_final=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		sucursal=mFg.ExtraeStringDeBuffer(&parametros);
		conceptomov=mFg.ExtraeStringDeBuffer(&parametros);

		if (sucursal!=" "){
			condicion_sucursal.sprintf(" and bvd.sucursal='%s' ", sucursal);
			condicion_sucursalvta.sprintf(" and v.sucursal='%s' ", sucursal);
		}


		if (conceptomov!=" "){
			condicion_conceptomov.sprintf("and bvd.termino='%s' ", conceptomov);
			condicion_conmovventa.sprintf("and f.termino='%s' ", conceptomov);
		}
		 else {
			condicion_conceptomov.sprintf(" and bvd.termino IN('conef','tarcr')");
			condicion_conmovventa.sprintf("and f.termino IN('conef','tarcr')");
		 }

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_REP);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {

			// Obtiene todos los datos de los proveedores
			/*instruccion.sprintf("SELECT * FROM bancosventasdef bvd \
								WHERE  bvd.fechaventas>='%s' AND bvd.fechaventas<='%s' \
								%s %s and bvd.cancelado=0 \
								ORDER BY bvd.fechaventas ASC, bvd.sucursal ASC, bvd.termino ASC ",fecha_inicial,
								fecha_final,condicion_sucursal.c_str(),condicion_conceptomov.c_str() ); */

			  instruccion.sprintf("SELECT s.sucursal,v.fechavta as fechaventas, f.termino,SUM(d.valor) AS totventa,  \
				IFNULL( auxbvd.idbancosventas,0) AS idbancosventas,   \
				IFNULL(auxbvd.totalventas,SUM(d.valor)) AS totalventas, \
				IFNULL(auxbvd.total,0) AS total, IFNULL(SUM(nc.valor),0) AS totalnotascred \
				FROM ventas v    \
				INNER JOIN dventasfpago d ON d.referencia = v.referencia \
				INNER JOIN formasdepago f ON f.formapag = d.formapag \
				INNER JOIN terminales t ON v.terminal=t.terminal  \
				INNER JOIN secciones sec ON t.seccion=sec.seccion   \
				INNER JOIN  sucursales s ON s.sucursal =sec.sucursal   \
				LEFT JOIN notascredcli nc ON nc.venta = v.referencia AND nc.cancelado = 0 \
				LEFT JOIN (SELECT bvd.idbancosventas,bvd.sucursal, bvd.fechaventas, \
					bvd.termino,bvd.totalventas,bvd.total  \
					FROM bancosventasdef bvd  \
					INNER JOIN bancosdventas bdv ON bvd.idbancosventas=bdv.idbancosventas   \
					WHERE  bvd.cancelado=0 AND bvd.sucursal= '%s'  AND bvd.fechaventas>='%s' AND bvd.fechaventas <='%s'  %s  \
					GROUP BY bvd.sucursal,bvd.fechaventas, bvd.termino \
				) AS auxbvd ON auxbvd.sucursal= s.sucursal AND auxbvd.fechaventas=  v.fechavta AND auxbvd.termino= f.termino \
				WHERE v.fechavta>='%s' AND v.fechavta <='%s' AND v.cancelado=0 %s AND s.sucursal='%s'  \
				GROUP BY sucursal,v.fechavta, f.termino",sucursal,fecha_inicial,fecha_final,
				 condicion_conceptomov,fecha_inicial,fecha_final,condicion_conmovventa,sucursal);

			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	   }
	}__finally{
		//if (resp_vtamovbancarios!=NULL) delete resp_vtamovbancarios;
		delete buffer_sql;
	}
}
//----------------------------------------------------------------------------


