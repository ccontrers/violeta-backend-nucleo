#include <vcl.h>
//---------------------------------------------------------------------------
#include "pch.h"

#pragma hdrstop

#include <DateUtils.hpp>
#include "ClassServidorAlmacen.h"
#include "ClassServidorVioleta.h"
#include "ClassBufferRespuestas.h"
#include "FormServidorVioleta.h"
#include "comunes.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

//ID_GRA_MOVALM
void ServidorAlmacen::GrabaMovimientoAlmacen(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  GRABA UN MOVIMIENTO DE ALMACEN
	char *buffer_sql=new char[1024*64*200];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	DatosTabla datosBitacora(mServidorVioleta->Tablas);
	AnsiString tarea, clave, usuario;
	int num_partidas, i, num_partidas_validar=0;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[15000], select;
	TDateTime fecha_mov;
	AnsiString tipo, concepto, articulo, costo, costobase, cantidad;
	AnsiString tipo_bitacora_costos, cancelado;
	int afectar_costo_base=0;
	int error=0;
	TDate fecha=Today();
	TTime hora=Time();
	BufferRespuestas* resp_concepto=NULL;
	BufferRespuestas* resp_parametro=NULL;
	BufferRespuestas* resp_detmov=NULL;
	AnsiString variableaplica,estadoaplica,terminal, fechamodi, horamodi;
	AnsiString fechamodi_mov="", horamodi_mov="";
	AnsiString mensaje;

	AnsiString almaEnt, almaSal;

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

	try{
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave del movimiento
		tarea=mFg.ExtraeStringDeBuffer(&parametros); // Indicador si se va a agregar o modificar.
		usuario=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que esta grabando el movimiento
		variableaplica=mFg.ExtraeStringDeBuffer(&parametros);//campo aplicado
		terminal= mFg.ExtraeStringDeBuffer(&parametros);//campo terminal
		fechamodi=mFg.ExtraeStringDeBuffer(&parametros);//última fecha de modificación del movimiento (consultada por el cliente)
		horamodi=mFg.ExtraeStringDeBuffer(&parametros);//última hora de modificación del moviemiento (consultada por el cliente)
		mensaje=mFg.ExtraeStringDeBuffer(&parametros);

		// Obtiene los datos de la tabla de movimientos de almacén
		datos.AsignaTabla("movalma");
		parametros+=datos.InsCamposDesdeBuffer(parametros);
		// Extrae algunos datos necesarios para calcular otros
		fecha_mov=StrToDate(datos.ObtieneValorCampo("fechamov"));
		tipo=datos.ObtieneValorCampo("tipo");
		concepto=datos.ObtieneValorCampo("concepto");
		estadoaplica=datos.ObtieneValorCampo("aplica");
		almaEnt=datos.ObtieneValorCampo("almaent");
		almaSal=datos.ObtieneValorCampo("almasal");
		/*if(tarea=="M")
			cancelado=datos.ObtieneValorCampo("cancelado");*/
		//para consultar el estado aplica actual o antes del cambio
		AnsiString VariableAplicaActual, fechamodi_movalm;
		VariableAplicaActual="";
		if (tarea=="M") {
			BufferRespuestas* resp_VariableAplicaActual=NULL;
			BufferRespuestas* resp_info_movalma=NULL;
			try {
				instruccion.sprintf("select aplica from movalma where movimiento='%s'", clave);
				if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_VariableAplicaActual)) {
					if (resp_VariableAplicaActual->ObtieneNumRegistros()>0){
						VariableAplicaActual=resp_VariableAplicaActual->ObtieneDato("aplica");
					} else throw (Exception("No se encuentra registro CAMBIOPRECDIFER en tabla movalma"));
				} else throw (Exception("Error al consultar en tabla movalma"));
			} __finally {
				if (resp_VariableAplicaActual!=NULL) delete resp_VariableAplicaActual;
			}

            try {
				instruccion.sprintf("select fechamodi, horamodi from movalma where movimiento='%s'", clave);
				if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_info_movalma)) {
					if (resp_info_movalma->ObtieneNumRegistros()>0){
						fechamodi_mov=resp_info_movalma->ObtieneDato("fechamodi");
						horamodi_mov=resp_info_movalma->ObtieneDato("horamodi");
					}
				} else throw (Exception("Error al consultar la última fecha y hora de modificación"));
			} __finally {
				if (resp_info_movalma!=NULL) delete resp_info_movalma;
			}

		}


		// Verificaciones para no permitir modificaciones cuando no sea permitido.
		if (tarea=="M") {

			if(fechamodi.Trim() !="" && horamodi.Trim() !="" && fechamodi_mov !="" && horamodi_mov !=""){
				if(fechamodi != fechamodi_mov || horamodi != horamodi_mov)
					throw (Exception("No se puede grabar. Este movimiento acaba de ser modificado, vuelva a cargar el movimiento para modificarlo"));
			}

			// Verifica que la fecha del movimiento sea posterior a la fecha de cierre.
			instruccion.sprintf("select @error:=if(m.fechamov<=cast(e.valor as datetime), 1, 0) as error from movalma m left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' where m.movimiento='%s'",FormServidor->ObtieneClaveSucursal(), clave);
			mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 3, error);
		}
		// Verifica que la fecha del movimiento sea posterior a la fecha de cierre.
		instruccion.sprintf("select @error:=if('%s'<=cast(e.valor as datetime), 1, 0) as error from estadosistemaemp e where e.estado='FUCIERRE' AND e.sucursal = '%s' ", mFg.DateToMySqlDate(fecha_mov), FormServidor->ObtieneClaveSucursal());
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 4, error);

		if (concepto=="SMCM" ||
			concepto=="SABA" ||
			concepto=="SSMC" ||
			concepto=="SSLA" ||
			concepto=="SSGU" ||
			concepto=="SSCA" ||
			concepto=="SSSO" ||
			concepto=="SSMI" ||
			concepto=="SSVI" ) {

			if (tarea=="A") {
				// Si se requieren costos promedio al dia pero no hay, entonces no permite grabar
				BufferRespuestas* resp_costosprom=NULL;
				try {
					instruccion.sprintf("SELECT * FROM precalculocostospromedio%s limit 1", FormServidor->ObtieneClaveEmpresa());
					if (!mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_costosprom))
						throw (Exception("Error al consultar la tabla de costos promedio"));
					if (resp_costosprom->ObtieneNumRegistros()==0)
						throw (Exception("Se requiere que la tabla 'precalculocostospromedio' este llena para fijar el costo promedio de la salida"));
				} __finally {
					if (resp_costosprom!=NULL) delete resp_costosprom;
				}

			}

		}

        if(tipo == "T"){
			if(almaEnt == almaSal){
				throw (Exception("No se pueden hacer TRASPASOS de un ALMACEN hacia SI MISMO"));
			}
		}

		// Si es entrada vemos si es de las que modifican costos y precios.
		if (tipo=="E") {
			// Ver si el movimiento afecta costo fijo
			instruccion.sprintf("SELECT costofijo FROM conceptosmovalma \
					WHERE concepto='%s'", concepto);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_concepto);
			if (resp_concepto->ObtieneNumRegistros()>0){
				int costofijo=StrToInt(resp_concepto->ObtieneDato("costofijo"));
				if (costofijo==1) {
					// Ver si esta configurado el servidor para afectar costo base y precios
					instruccion.sprintf("SELECT valor FROM parametrosemp \
							WHERE parametro='MOVSAFECTANCB' AND sucursal = '%s' ",  FormServidor->ObtieneClaveSucursal());
					mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_parametro);
					if (resp_parametro->ObtieneNumRegistros()>0){
						afectar_costo_base=StrToInt(resp_parametro->ObtieneDato("valor"));
					} else throw (Exception("Error al consultar parametro MOVSAFECTANCB en GrabaMovimientoAlmacen"));
				}
			} else throw (Exception("Error al consultar conceptosmovalma en GrabaMovimientoAlmacen"));

		}

		if (error==0) {
			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

			// Caso especial para salidas a martin castrejon desde abastos
			if (concepto=="SMCM" ||
				concepto=="SABA" ||
				concepto=="SSMC" ||
				concepto=="SSLA" ||
				concepto=="SSGU" ||
				concepto=="SSCA" ||
				concepto=="SSSO" ||
				concepto=="SSMI" ||
				concepto=="SSVI" ) {
				// Crea una tabla para almacenar el costo y costobase de forma temporal para en caso de edición recuperalo de aquí
				instruccion="create temporary table tmpcostosoriginal ( \
					articulo varchar(9), costo decimal(16,6), costobase decimal(16,6), PRIMARY KEY (articulo)) Engine = InnoDB";
				instrucciones[num_instrucciones++]=instruccion;

				if (tarea=="M") {
					// Graba el costo y costo base del movimiento como estaba originalmente
					// SOLO SI NO ES CERO
					instruccion.sprintf("insert into tmpcostosoriginal (articulo, costo, costobase) \
						select d.articulo, d.costo, d.costobase from dmovalma d \
						where movimiento='%s' and d.costo<>0", clave);
					instrucciones[num_instrucciones++]=instruccion;
				}
			}

			// Obtiene el folio para el movimiento
			if (tarea=="A") {
				instruccion.sprintf("select @folioaux:=valor from foliosemp where folio='MOVALMA' AND sucursal = '%s' %s ",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliosig=@folioaux+1");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @folioaux=cast(@folioaux as char)");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @folio=concat('%s', lpad(@folioaux,9,'0'))", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("update foliosemp set valor=@foliosig where folio='MOVALMA' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				tipo_bitacora_costos = "EMA";
			} else {
				instruccion.sprintf("set @folio='%s'", clave);
				instrucciones[num_instrucciones++]=instruccion;
				tipo_bitacora_costos = "MEMA";
			}

			// Si se está modificando entonces borra el detalle que ya exista.
			if (tarea=="M") {

					/* Crea tabla temporal para almacenar cantidades de articulos antes de
					/////////////////////////ACTUALIZAR EXISTENCIAS//////////////////////*/
					instruccion="CREATE temporary TABLE tmpcantidad(referencia VARCHAR(11) NOT NULL,\
					producto VARCHAR(8) NOT NULL, present VARCHAR(255) NOT NULL, almacen VARCHAR(4) NOT NULL,  \
					cantidad DECIMAL(12,3) NOT NULL, PRIMARY KEY (referencia, producto, present, almacen))";
					instrucciones[num_instrucciones++]=instruccion;

				if(tipo == "E"){
					instruccion = "INSERT INTO tmpcantidad SELECT d.movimiento, a.producto, a.present, m.almaent, \
					SUM(d.cantidad * a.factor) AS cantidad FROM dmovalma d INNER JOIN movalma m ON   \
					d.movimiento = m.movimiento INNER JOIN articulos a ON a.articulo = d.articulo \
					WHERE d.movimiento = @folio GROUP BY a.producto, a.present";
					instrucciones[num_instrucciones++]=instruccion;

					instruccion = "UPDATE tmpcantidad tmp INNER JOIN existenciasactuales ea ON ea.producto = tmp.producto \
					AND ea.present = tmp.present AND ea.almacen = tmp.almacen SET ea.cantidad = (ea.cantidad  - tmp.cantidad) \
					, ea.entradas = (ea.entradas - tmp.cantidad) ";
					instrucciones[num_instrucciones++]=instruccion;
				}

				if(tipo == "S"){
					instruccion = "INSERT INTO tmpcantidad SELECT d.movimiento, a.producto, a.present, m.almasal, \
					SUM(d.cantidad * a.factor) AS cantidad FROM dmovalma d INNER JOIN movalma m ON   \
					d.movimiento = m.movimiento INNER JOIN articulos a ON a.articulo = d.articulo \
					WHERE d.movimiento = @folio GROUP BY a.producto, a.present";
					instrucciones[num_instrucciones++]=instruccion;

					instruccion = "UPDATE tmpcantidad tmp INNER JOIN existenciasactuales ea ON ea.producto = tmp.producto \
					AND ea.present = tmp.present AND ea.almacen = tmp.almacen SET ea.cantidad = (ea.cantidad  + tmp.cantidad) \
					, ea.salidas = (ea.salidas - tmp.cantidad)";
					instrucciones[num_instrucciones++]=instruccion;

				}

                if(estadoaplica=="1"){
					if(tipo == "T"){
						instruccion="CREATE temporary TABLE tmpcantidad2(referencia VARCHAR(11) NOT NULL,\
						producto VARCHAR(8) NOT NULL, present VARCHAR(255) NOT NULL, almacen VARCHAR(4) NOT NULL,  \
						cantidad DECIMAL(12,3) NOT NULL, PRIMARY KEY (referencia, producto, present, almacen))";
						instrucciones[num_instrucciones++]=instruccion;

						instruccion = "INSERT INTO tmpcantidad SELECT d.movimiento, a.producto, a.present, m.almaent, \
						SUM(d.cantidad * a.factor) AS cantidad FROM dmovalma d INNER JOIN movalma m ON   \
						d.movimiento = m.movimiento INNER JOIN articulos a ON a.articulo = d.articulo \
						WHERE d.movimiento = @folio GROUP BY a.producto, a.present";
						instrucciones[num_instrucciones++]=instruccion;

						instruccion = "INSERT INTO tmpcantidad2 SELECT d.movimiento, a.producto, a.present, m.almasal, \
						SUM(d.cantidad * a.factor) AS cantidad FROM dmovalma d INNER JOIN movalma m ON   \
						d.movimiento = m.movimiento INNER JOIN articulos a ON a.articulo = d.articulo \
						WHERE d.movimiento = @folio GROUP BY a.producto, a.present";
						instrucciones[num_instrucciones++]=instruccion;

						//validacion para saber si antes ya estaba aplicado se debe regresar y volver a aplicar nuevamente el movimiento
						if (VariableAplicaActual=="1") {
							instruccion = "UPDATE tmpcantidad tmp INNER JOIN existenciasactuales ea ON ea.producto = tmp.producto \
							AND ea.present = tmp.present AND ea.almacen = tmp.almacen SET ea.cantidad = (ea.cantidad  - tmp.cantidad) \
							, ea.entradas = (ea.entradas - tmp.cantidad) ";
							instrucciones[num_instrucciones++]=instruccion;

							instruccion = "UPDATE tmpcantidad2 tmp INNER JOIN existenciasactuales ea ON ea.producto = tmp.producto \
							AND ea.present = tmp.present AND ea.almacen = tmp.almacen SET ea.cantidad = (ea.cantidad  + tmp.cantidad) \
							, ea.salidas = (ea.salidas - tmp.cantidad)";
							instrucciones[num_instrucciones++]=instruccion;
						}
					}
				}

				//////////////////////INSERT BITACORA MODIFICACIONES/////////////////////////////
				datosBitacora.AsignaTabla("bitacoramovalmamod");
				datosBitacora.InsCampo("movimiento", "@folio", 1);
				datosBitacora.InsCampo("almaent", almaEnt);
				datosBitacora.InsCampo("almasal", almaSal);
				datosBitacora.InsCampo("usuario", usuario);
				datosBitacora.InsCampo("fecha", mFg.DateToAnsiString(fecha));
				datosBitacora.InsCampo("hora", mFg.TimeToAnsiString(hora));
				datosBitacora.InsCampo("fechamov", mFg.DateToAnsiString(fecha_mov));
				datosBitacora.InsCampo("aplica", VariableAplicaActual);

				instrucciones[num_instrucciones++]=datosBitacora.GenerarSqlInsert();

				instruccion.sprintf("SET @lastid:=LAST_INSERT_ID()");
				instrucciones[num_instrucciones++]=instruccion;

				instruccion.sprintf("SELECT articulo, cantidad, costo, costobase FROM dmovalma WHERE movimiento='%s'",clave);
				mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_detmov);
				if (resp_detmov->ObtieneNumRegistros()>0){
					for (int m = 0; m < resp_detmov->ObtieneNumRegistros() ; m++) {

						//////////////////////INSERT BITACORA MODIFICACIONES/////////////////////////////
						datosBitacora.AsignaTabla("bitacoradmovalmamod");
						datosBitacora.InsCampo("idbitacora", "@lastid", 1);
						datosBitacora.InsCampo("articulo", resp_detmov->ObtieneDato("articulo"));
						datosBitacora.InsCampo("cantidad", resp_detmov->ObtieneDato("cantidad"));
						datosBitacora.InsCampo("costo", resp_detmov->ObtieneDato("costo"));
						datosBitacora.InsCampo("costobase", resp_detmov->ObtieneDato("costobase"));
						instrucciones[num_instrucciones++]=datosBitacora.GenerarSqlInsert();
						//////////////////////FIN INSERT BITACORA MODIFICACIONES/////////////////////////
                        resp_detmov->IrAlSiguienteRegistro();
					}
				}

				//////////////////////FIN INSERT BITACORA MODIFICACIONES/////////////////////////

				//////////////////////FIN DE ACTUALIZAR EXISTENCIAS/////////////////////////////
				instruccion.sprintf("delete from dmovalma where movimiento=@folio");
				instrucciones[num_instrucciones++]=instruccion;

				//meter el valor del campo cancelado en una variable, para almacenarlo en la bitacora movalma
				instruccion.sprintf("select @status:=cancelado from movalma where movimiento=@folio ");
				instrucciones[num_instrucciones++]=instruccion;

				instruccion.sprintf("UPDATE movalma SET auditado= 0 where movimiento=@folio");
				instrucciones[num_instrucciones++]=instruccion;
			}

			// Graba la cabecera en la tabla "movalma"
			if (tarea=="A") {
				datos.InsCampo("movimiento", "@folio", 1);
				datos.InsCampo("usualta", usuario);
				datos.InsCampo("usumodi", usuario);
				datos.InsCampo("fechaalta", mFg.DateToAnsiString(fecha));
				datos.InsCampo("horaalta", mFg.TimeToAnsiString(hora));
				datos.InsCampo("fechamodi", mFg.DateToAnsiString(fecha));
				datos.InsCampo("horamodi", mFg.TimeToAnsiString(hora));
				datos.InsCampo("cancelado", "0");
				instrucciones[num_instrucciones++]=datos.GenerarSqlInsert();

 			//se grabara el movimiento en la bitacora de movimientos almacen
			instruccion.sprintf("INSERT INTO bitacoramovalma (idbitacoramovalma, movimiento, usuario, fecha, hora, tipo_tarea,\
			aplicado_ant,aplicado_act,cancelado) values (NULL,@folio,'%s','%s','%s','%s',%s,%s, 0)",usuario,mFg.DateToMySqlDate(Today()),
			mFg.TimeToMySqlTime(Time()),tarea,estadoaplica,variableaplica,cancelado );
			instrucciones[num_instrucciones++] = instruccion;
			} else {
				datos.InsCampo("usumodi", usuario);
				datos.InsCampo("fechamodi", mFg.DateToAnsiString(fecha));
				datos.InsCampo("horamodi", mFg.TimeToAnsiString(hora));
                datos.ReemplazaCampo("aplica", "aplica", 1);
				instrucciones[num_instrucciones++]=datos.GenerarSqlUpdate("movimiento=@folio");

				//se grabara el movimiento en la bitacora de movimientos almacen
				instruccion.sprintf("INSERT INTO bitacoramovalma (idbitacoramovalma, movimiento, usuario, fecha, hora, tipo_tarea,\
				aplicado_ant,aplicado_act,cancelado) values (NULL,@folio,'%s','%s','%s','%s',%s,%s, @status)",usuario,mFg.DateToMySqlDate(Today()),
				mFg.TimeToMySqlTime(Time()),tarea,estadoaplica,variableaplica);
				instrucciones[num_instrucciones++] = instruccion;
			}

			// Guarda el mensaje
			if (mensaje!=" ") { // Un espacio lo tomamos como nulo.
				instruccion.sprintf("REPLACE INTO movalmamensajes (referencia, mensaje) VALUES (@folio,'%s')", mensaje);
			} else {
				instruccion.sprintf("DELETE FROM movalmamensajes WHERE referencia=@folio");
			}
			instrucciones[num_instrucciones++]=instruccion;

			if (afectar_costo_base==1) {
				instruccion="create temporary table tmpcostos ( \
					articulo varchar(9), producto varchar(8), present varchar(255), \
					costo decimal(16,6), modificar bool, PRIMARY KEY (articulo)) Engine = InnoDB";
				instrucciones[num_instrucciones++]=instruccion;
			}



			BufferRespuestas* resp_compactivo_previo=NULL;
			// Graba las partidas en "dmovalma"
			AnsiString lista_articulos = " ";
			//para validar si de los articulos eliminados existe alguno que no esté activo en base a esta lista completa.
			AnsiString lista_articulos_completa = " ";

			num_partidas=StrToInt(mFg.ExtraeStringDeBuffer(&parametros)); // Obtiene el número de partidas
			for (i=0; i<num_partidas; i++) {
				datos.AsignaTabla("dmovalma");
				parametros+=datos.InsCamposDesdeBuffer(parametros);
				datos.InsCampo("movimiento", "@folio", 1);
				articulo=datos.ObtieneValorCampo("articulo");
				cantidad=datos.ObtieneValorCampo("cantidad");
				costo=datos.ObtieneValorCampo("costo");
				costobase=datos.ObtieneValorCampo("costobase");
				instrucciones[num_instrucciones++]=datos.GenerarSqlInsert();


				if( i != num_partidas - 1)
					lista_articulos_completa = lista_articulos_completa + "'" + articulo + "', ";
				else
					lista_articulos_completa = lista_articulos_completa + "'" + articulo + "'";


				/*Evaluar si algún artículo esta inactivo, solo de los que se modificarán*/
				try{
					if( i != num_partidas - 1){
							if (tarea=="M") {
								//try
								instruccion.sprintf("SELECT movimiento, articulo, cantidad, costo, costobase FROM dmovalma \
										 WHERE movimiento='%s' and articulo='%s' and cantidad<>%s", clave, articulo, cantidad);

								if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_compactivo_previo)) {
									if (resp_compactivo_previo->ObtieneNumRegistros()>0){
											if (num_partidas_validar==0) {
												lista_articulos = lista_articulos + "'" + articulo + "'";
											}else
												lista_articulos = lista_articulos + ", '" + articulo + "'";
											num_partidas_validar=num_partidas_validar+1;
										}
								}
							}else
								lista_articulos = lista_articulos + "'" + articulo + "', ";

					}else{
						if (tarea=="M") {
						instruccion.sprintf("SELECT movimiento, articulo, cantidad, costo, costobase FROM dmovalma \
										 WHERE movimiento='%s' and articulo='%s' and cantidad<>%s", clave, articulo, cantidad);

								if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_compactivo_previo)) {
									if (resp_compactivo_previo->ObtieneNumRegistros()>0){
											if (num_partidas_validar==0) {
												lista_articulos = lista_articulos + "'" + articulo + "'";
											}else
												lista_articulos = lista_articulos + ", '" + articulo + "'";
											num_partidas_validar=num_partidas_validar+1;
										}
								}

						}else
							lista_articulos = lista_articulos + "'" + articulo + "'";
					}
				} __finally {
					if (resp_compactivo_previo!=NULL) delete resp_compactivo_previo;
					resp_compactivo_previo=NULL;

				}



				if (afectar_costo_base==1) {
					instruccion.sprintf("insert into tmpcostos (articulo, costo, modificar) values ('%s', %s, 1)", articulo, costobase);
					instrucciones[num_instrucciones++]=instruccion;
				}

				/////////////////////////ACTUALIZAR EXISTENCIAS//////////////////////
				/**
					para aplicar el movimiento a modificar el almacen (existenciasactuales), es necesario que el registro que se
					creo en movalma sea considerado como aplicado, el cual no solo es un registro, si no que se considerara
					como un movimiento aprobado
				**/

					if(tipo == "E"){
						instruccion.sprintf("UPDATE articulos a INNER JOIN movalma m ON m.movimiento = @folio \
						INNER JOIN existenciasactuales ea ON a.producto = ea.producto AND a.present = ea.present   \
						AND m.almaent = ea.almacen INNER JOIN dmovalma d ON m.movimiento = d.movimiento AND \
						a.articulo = d.articulo	SET ea.cantidad = (ea.cantidad + (d.cantidad * a.factor)) \
						, ea.entradas = (ea.entradas + (d.cantidad * a.factor)) \
						WHERE a.articulo = '%s' AND m.movimiento = @folio",articulo);
						instrucciones[num_instrucciones++]=instruccion;

					}
					if(tipo == "S"){
						instruccion.sprintf("UPDATE articulos a INNER JOIN movalma m ON m.movimiento = @folio \
						INNER JOIN existenciasactuales ea ON a.producto = ea.producto AND a.present = ea.present   \
						AND m.almasal = ea.almacen INNER JOIN dmovalma d ON m.movimiento = d.movimiento AND \
						a.articulo = d.articulo	SET ea.cantidad = (ea.cantidad - (d.cantidad * a.factor)) \
						, ea.salidas = (ea.salidas + (d.cantidad * a.factor)) \
						WHERE a.articulo = '%s' AND m.movimiento = @folio",articulo);
						instrucciones[num_instrucciones++]=instruccion;

					}

                    if(estadoaplica=="1"){

						if(tipo == "T"){
							instruccion.sprintf("UPDATE articulos a INNER JOIN movalma m ON m.movimiento = @folio \
							INNER JOIN existenciasactuales ea ON a.producto = ea.producto AND a.present = ea.present   \
							AND m.almaent = ea.almacen INNER JOIN dmovalma d ON m.movimiento = d.movimiento AND \
							a.articulo = d.articulo	SET ea.cantidad = (ea.cantidad + (d.cantidad * a.factor)) \
							, ea.entradas = (ea.entradas + (d.cantidad * a.factor))  \
							WHERE a.articulo = '%s' AND m.movimiento = @folio",articulo);
							instrucciones[num_instrucciones++]=instruccion;

							instruccion.sprintf("UPDATE articulos a INNER JOIN movalma m ON m.movimiento = @folio \
							INNER JOIN existenciasactuales ea ON a.producto = ea.producto AND a.present = ea.present   \
							AND m.almasal = ea.almacen INNER JOIN dmovalma d ON m.movimiento = d.movimiento AND \
							a.articulo = d.articulo	SET ea.cantidad = (ea.cantidad - (d.cantidad * a.factor)) \
							, ea.salidas = (ea.salidas + (d.cantidad * a.factor)) \
							WHERE a.articulo = '%s' AND m.movimiento = @folio",articulo);
							instrucciones[num_instrucciones++]=instruccion;

						}
					}

				/////////////////////////FIN DE ACTUALIZAR EXISTENCIAS/////////////////////////

			}

			//vsalidación para insertar a validación los articulos que pudiera ser eliminados si es modificación
			try {
				resp_compactivo_previo=NULL;
				if (tarea=="M") {
					instruccion.sprintf("SELECT movimiento, articulo, cantidad, costo, costobase FROM dmovalma \
							 WHERE movimiento='%s' and articulo not in(%s)", clave, lista_articulos_completa);

				if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_compactivo_previo)) {
					if (resp_compactivo_previo->ObtieneNumRegistros()>0){
						for(int c = 0; c < resp_compactivo_previo->ObtieneNumRegistros(); c++)
						{
							if (num_partidas_validar==0) {
								lista_articulos = lista_articulos + "'" + resp_compactivo_previo->ObtieneDato("articulo") + "'";
							}else
								lista_articulos = lista_articulos + ", '" + resp_compactivo_previo->ObtieneDato("articulo") + "'";
							num_partidas_validar=num_partidas_validar+1;
							resp_compactivo_previo->IrAlSiguienteDato();
						}
					}// else throw (Exception("No se encuentra ningún artículo"));  //no existen articulos eliminados
				} else throw (Exception("Error al consultar los artículos del bloque eliminados"));
				}
			} __finally {
				if (resp_compactivo_previo!=NULL) delete resp_compactivo_previo;
			}



			/* VALIDAR SI ALGÚN ARTICULO ESTA INACTIVO*/
			if (lista_articulos!=" ") {

				BufferRespuestas* resp_compactivo=NULL;
				try {

						instruccion.sprintf("SELECT p.nombre, a.present, a.multiplo, a.activo FROM articulos a INNER JOIN productos p \
							ON p.producto = a.producto WHERE a.articulo IN (%s)", lista_articulos);

					if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_compactivo)) {
						if (resp_compactivo->ObtieneNumRegistros()>0){
							for(int c = 0; c < resp_compactivo->ObtieneNumRegistros(); c++)
							{
								if(resp_compactivo->ObtieneDato("activo") == "0")
									throw(Exception("El artículo " + resp_compactivo->ObtieneDato("nombre") + " " + resp_compactivo->ObtieneDato("present") + " " + resp_compactivo->ObtieneDato("multiplo") + " está inactivo."));

								resp_compactivo->IrAlSiguienteDato();

							}

						} else throw (Exception("No se encuentra ningún artículo"));
					} else throw (Exception("Error al consultar los artículos"));
				} __finally {
					if (resp_compactivo!=NULL) delete resp_compactivo;
				}
            }
		/*FIN VALIDAR SI ALGÚN ARTICULO ESTA INACTIVO*/

			/* valida negativo */
			if (cantidad < 0 || costo < 0 || costobase < 0) {
				BufferRespuestas* resp_compactivo=NULL;
				try {
					instruccion.sprintf("SELECT p.nombre, a.present, a.multiplo, a.activo FROM articulos a INNER JOIN productos p \
						ON p.producto = a.producto WHERE a.articulo = '%s'", articulo);

					if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_compactivo)) {
						if (resp_compactivo->ObtieneNumRegistros()>0){
							throw(Exception("El artículo " + resp_compactivo->ObtieneDato("nombre") + " " + resp_compactivo->ObtieneDato("present") + " " + resp_compactivo->ObtieneDato("multiplo") + " tiene cantidad/costo negativos."));
						} else throw (Exception("No se encuentra ningún artículo"));
					} else throw (Exception("Error al consultar los artículos"));
				} __finally {
					if (resp_compactivo!=NULL) delete resp_compactivo;
				}
			}
			/*fin valida negativos*/

			if (concepto=="SMCM" ||
				concepto=="SABA" ||
				concepto=="SSMC" ||
				concepto=="SSLA" ||
				concepto=="SSGU" ||
				concepto=="SSCA" ||
				concepto=="SSSO" ||
				concepto=="SSMI" ||
				concepto=="SSVI" ) {
				if (tarea=="A") {
					// si la tarea es ALTA y el concepto es SMCM
					// grabar costo promedio y costo base de todos los articulos del movimiento.
					instruccion.sprintf("update dmovalma d \
						inner join articulos a on d.articulo=a.articulo \
						inner join presentacionescb pres on a.producto=pres.producto and a.present=pres.present \
						left join precalculocostospromedio%s c on c.producto=a.producto and c.present=a.present \
						set d.costo=ifnull(c.costounit,0)*a.factor, d.costobase=pres.costobase \
						where d.movimiento=@folio and pres.idempresa=%s ", FormServidor->ObtieneClaveEmpresa(),
						FormServidor->ObtieneClaveEmpresa());
					instrucciones[num_instrucciones++]=instruccion;
				} else {
					// si la tarea es modificación y el concepto es SMCM,
					// a) grabar costo promedio y costo base de articulos que  NO estaban grabados.
					instruccion.sprintf("update dmovalma d \
						inner join articulos a on d.articulo=a.articulo \
						inner join presentacionescb pres on a.producto=pres.producto and a.present=pres.present \
						left join precalculocostospromedio%s c on c.producto=a.producto and c.present=a.present \
						set d.costo=ifnull(c.costounit,0)*a.factor, d.costobase=pres.costobase \
						where d.movimiento='%s' and pres.idempresa=%s and d.articulo not in \
						(select articulo from tmpcostosoriginal)", FormServidor->ObtieneClaveEmpresa(), clave, FormServidor->ObtieneClaveEmpresa());
					instrucciones[num_instrucciones++]=instruccion;
					// b) grabar costo promedio y costo base de articulos que SI estaban grabados.
					instruccion.sprintf("update dmovalma d \
						inner join tmpcostosoriginal t on t.articulo=d.articulo \
						set d.costo=t.costo, d.costobase=t.costobase \
						where d.movimiento='%s' ", clave);
					instrucciones[num_instrucciones++]=instruccion;
				}
			}


			if (afectar_costo_base==1) {
				// Agrega la clave de producto y presentacion de los articulos a modificar.
				instruccion.sprintf("update tmpcostos cos, articulos a \
					set cos.producto=a.producto, cos.present=a.present \
					where cos.articulo=a.articulo");
				instrucciones[num_instrucciones++]=instruccion;

				// En la tabla auxiliar de costos establece que precios NO se van a modificar
				// (los que tengan COMPRAS POSTERIORES a la fecha)
				instruccion.sprintf("update compras c, dcompras d, tmpcostos cos \
					set cos.modificar=0 \
					where c.fechacom>'%s' and c.cancelado=0 and \
					d.articulo=cos.articulo and c.referencia=d.referencia",
					mFg.DateToMySqlDate(fecha_mov));
				instrucciones[num_instrucciones++]=instruccion;

				//lina de un ates y despues== antes: and m.aplica=1 (despues de la fecha) || despues: sin aplicar
				// (los que tengan MOVIMIENTOS DE ENTRADA que afectan costo POSTERIORES a la fecha)
				instruccion.sprintf("update movalma m, dmovalma d, conceptosmovalma cm, tmpcostos cos \
					set cos.modificar=0 \
					where  m.fechamov>'%s' AND m.cancelado=0 AND \
					m.movimiento=d.movimiento AND m.tipo='E' AND \
					m.concepto=cm.concepto AND cm.tipomov=0 AND \
					cm.costofijo=1 AND d.articulo=cos.articulo",
					mFg.DateToMySqlDate(fecha_mov));
				instrucciones[num_instrucciones++]=instruccion;

				// Los que estén bloqueados para modificación de precios se marcan para NO modificar sus precios.
				instruccion.sprintf("update tmpcostos cos \
					inner join articulos a ON cos.articulo=a.articulo \
					inner join preciosbloqueados pb ON pb.producto=a.producto and pb.present=a.present and pb.idempresa=%s \
					set cos.modificar=0 \
					where pb.fechaVigencia>='%s' ", FormServidor->ObtieneClaveEmpresa(),
					mFg.DateToMySqlDate(fecha));
				instrucciones[num_instrucciones++]=instruccion;

				//Ingresa los cambios de los costos en la bitacora
				instruccion.sprintf("INSERT INTO bitacoracostos SELECT @folio AS referencia,a.articulo,'%s' AS tipo, \
				 p.costobase,(tc.costo/a.factor) AS costo,CURDATE(),CURTIME(), '%s', p.idempresa \
					FROM presentacionescb p,tmpcostos tc,articulos a  WHERE tc.articulo=a.articulo AND a.present=p.present \
					AND a.producto=p.producto AND tc.costo<>0 AND tc.modificar=1 AND p.idempresa=%s ",
					tipo_bitacora_costos, usuario, FormServidor->ObtieneClaveEmpresa() );
				instrucciones[num_instrucciones++] = instruccion;

				// Cambia el costo unitario en la tabla presentaciones.
				instruccion="update tmpcostos cos, presentacionescb p, articulos a ";
				instruccion+="set p.costobase=cos.costo ";
				instruccion+="where cos.articulo=a.articulo and a.present=p.present \
				 and a.producto=p.producto and cos.costo<>0 and cos.modificar=1 and p.idempresa=" + FormServidor->ObtieneClaveEmpresa()+" ";
				instrucciones[num_instrucciones++]=instruccion;

				// Cambia el costo y los precios en la tabla de precios (globales)
				instruccion.sprintf("select @valor:=valor from parametrosemp where parametro='DIGREDOND' AND sucursal = '%s' ",
				FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;

				// Cambia el costo y los precios en la tabla de precios.
				AnsiString asignacion_precio;
				if (paramcambioprec=="0")
				asignacion_precio.sprintf("prec.precio=ROUND(ROUND(present.costobase*(1+prec.porcutil/100),@valor)*a.factor,@valor), \
									prec.precioproximo=ROUND(ROUND(present.costobase*(1+prec.porcutil/100),@valor)*a.factor,@valor) ");
				else
				asignacion_precio.sprintf("prec.precioproximo=ROUND(ROUND(present.costobase*(1+prec.porcutil/100),@valor)*a.factor,@valor) ");

				instruccion="update presentacionescb present, tmpcostos cos, precios prec, tiposdeprecios tp, articulos a ";
				instruccion+="set prec.costo=present.costobase, ";
				instruccion+=asignacion_precio;
				instruccion+="where cos.producto=a.producto and cos.present=a.present and ";
				instruccion+="cos.costo<>0 and cos.modificar=1 and ";
				instruccion+="prec.articulo=a.articulo and a.present=present.present and a.producto=present.producto ";
				instruccion+=" and present.idempresa="+FormServidor->ObtieneClaveEmpresa();
				instruccion+=" and prec.tipoprec=tp.tipoprec and tp.idempresa="+FormServidor->ObtieneClaveEmpresa()+" ";
				instrucciones[num_instrucciones++]=instruccion;
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
		if (resp_concepto!=NULL) delete resp_concepto;
		if (resp_parametro!=NULL) delete resp_parametro;
		if (resp_detmov!=NULL) delete resp_detmov;
	}
}

//---------------------------------------------------------------------------
//ID_CANC_MOVALM
void ServidorAlmacen::CancelaMovimientoAlmacen(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  CANCELA UN MOVIMIENTO DE ALMACEN
	char *buffer_sql=new char[1024*32];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instrucciones[50], instruccion;
	int num_instrucciones=0;
	AnsiString clave,aplica,app, usumodi;
	int i;
	int error=0;
	TDate fecha=Today();
	TTime hora=Time();


	try{
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave del movimiento
		aplica=mFg.ExtraeStringDeBuffer(&parametros); // aplicado o no
		usumodi=mFg.ExtraeStringDeBuffer(&parametros);// usuario que modifica

		// Verifica que la fecha del movimiento sea posterior a la fecha de cierre.
		instruccion.sprintf("select @error:=if(m.fechamov<=cast(e.valor as datetime), 1, 0) as error from movalma m left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' where m.movimiento='%s'",FormServidor->ObtieneClaveSucursal(), clave);
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 3, error);

		AnsiString produc2,present2,multiplo2,factor2,select_inactivos,nombre;
		BufferRespuestas* resp_inactivos=NULL;
		try{
			select_inactivos.sprintf("SELECT  @error:=IF(a.activo=0, 1, 0) AS error, \
				a.producto, a.present, a.multiplo, a.factor, pro.nombre \
				FROM movalma m LEFT JOIN dmovalma dm ON dm.movimiento=m.movimiento  \
				LEFT JOIN articulos a ON a.articulo=dm.articulo \
				LEFT JOIN productos pro ON pro.producto=a.producto \
				WHERE m.movimiento='%s' AND a.activo=0",clave );
			if(mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, select_inactivos.c_str(), resp_inactivos)){
				if (resp_inactivos->ObtieneNumRegistros()>0){
					produc2=resp_inactivos->ObtieneDato("producto");
					present2=resp_inactivos->ObtieneDato("present");
					multiplo2=resp_inactivos->ObtieneDato("multiplo");
					factor2=resp_inactivos->ObtieneDato("factor");
					nombre = resp_inactivos->ObtieneDato("nombre");
					error=4;
					throw (Exception("Hay artículos inactivos en el detalle de movimiento de almacen a cancelar.\n Favor de activar el artículo.\n  Artículo:"+nombre+" "+present2+" "+multiplo2));
				}
			}else{
				throw (Exception("Error al consultar los artículos inactivos en cancelar movimientos de almacen"));
            }
	   /*	,a.producto, a.present, a.multiplo, a.factor   */
	   }__finally{
			if (resp_inactivos!=NULL) delete resp_inactivos;
       }

		if (error==0) {
			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

			AnsiString tipo;
			BufferRespuestas* resp_tipo=NULL;
			try {
				instruccion.sprintf("SELECT tipo,aplica FROM movalma WHERE movimiento='%s'",clave);
				if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_tipo)){
						tipo=resp_tipo->ObtieneDato("tipo");
						app=resp_tipo->ObtieneDato("aplica");
				}
				 else throw (Exception("Error al consultar en tabla parametros"));
			} __finally {
				if (resp_tipo!=NULL) delete resp_tipo;
			}

			instruccion.sprintf("SET @folio:='%s'",clave);
			instrucciones[num_instrucciones++]=instruccion;


			//cehcar el estado del moviminto y en base a ello actualizar la tabla
			if (aplica=="1") {
				// Cancela el movimiento
				instruccion.sprintf("update movalma set cancelado=1 , aplica=1, usumodi='%s', fechamodi='%s', horamodi='%s'\
				where movimiento='%s' and cancelado=0",usumodi,mFg.DateToMySqlDate(fecha),mFg.TimeToMySqlTime(hora),clave);
				instrucciones[num_instrucciones++]=instruccion;
			}else{
				// Cancela el movimiento
				instruccion.sprintf("update movalma set cancelado=1, usumodi='%s', fechamodi='%s', horamodi='%s' \
				 where movimiento='%s' and cancelado=0",usumodi,mFg.DateToMySqlDate(fecha),mFg.TimeToMySqlTime(hora),clave);
				instrucciones[num_instrucciones++]=instruccion;
			}


  			//se grabara el movimiento en la bitacora de movimientos almacen
			instruccion.sprintf("INSERT INTO bitacoramovalma (idbitacoramovalma, movimiento, usuario, fecha, hora, tipo_tarea,\
			aplicado_ant,aplicado_act,cancelado) values (NULL,@folio,'%s','%s','%s','C',%s,%s, 1)",usumodi,mFg.DateToMySqlDate(Today()),
			mFg.TimeToMySqlTime(Time()),app,aplica);
			instrucciones[num_instrucciones++] = instruccion;


				/////////////***************** ACTUALIZAR EXISTENCIAS **************//////////////////
					instruccion="CREATE temporary TABLE tmpcantidad(referencia VARCHAR(11) NOT NULL,\
					producto VARCHAR(8) NOT NULL, present VARCHAR(255) NOT NULL, almacen VARCHAR(4) NOT NULL,  \
					cantidad DECIMAL(12,3) NOT NULL, PRIMARY KEY (referencia, producto, present, almacen))";
					instrucciones[num_instrucciones++]=instruccion;

				if(tipo == "E"){
					instruccion = "INSERT INTO tmpcantidad SELECT d.movimiento, a.producto, a.present, m.almaent, \
					SUM(d.cantidad * a.factor) AS cantidad FROM dmovalma d INNER JOIN movalma m ON   \
					d.movimiento = m.movimiento INNER JOIN articulos a ON a.articulo = d.articulo \
					WHERE d.movimiento = @folio GROUP BY a.producto, a.present";
					instrucciones[num_instrucciones++]=instruccion;

					instruccion = "UPDATE tmpcantidad tmp INNER JOIN existenciasactuales ea ON ea.producto = tmp.producto \
					AND ea.present = tmp.present AND ea.almacen = tmp.almacen SET ea.cantidad = (ea.cantidad  - tmp.cantidad) \
					, ea.entradas = (ea.entradas - tmp.cantidad) ";
					instrucciones[num_instrucciones++]=instruccion;
				}

				if(tipo == "S"){
					instruccion = "INSERT INTO tmpcantidad SELECT d.movimiento, a.producto, a.present, m.almasal, \
					SUM(d.cantidad * a.factor) AS cantidad FROM dmovalma d INNER JOIN movalma m ON   \
					d.movimiento = m.movimiento INNER JOIN articulos a ON a.articulo = d.articulo \
					WHERE d.movimiento = @folio GROUP BY a.producto, a.present";
					instrucciones[num_instrucciones++]=instruccion;

					instruccion = "UPDATE tmpcantidad tmp INNER JOIN existenciasactuales ea ON ea.producto = tmp.producto \
					AND ea.present = tmp.present AND ea.almacen = tmp.almacen SET ea.cantidad = (ea.cantidad  + tmp.cantidad) \
					, ea.salidas = (ea.salidas - tmp.cantidad) ";
					instrucciones[num_instrucciones++]=instruccion;

				}
				if(app=="1"){
					if(tipo == "T"){
						instruccion="CREATE temporary TABLE tmpcantidad2(referencia VARCHAR(11) NOT NULL,\
						producto VARCHAR(8) NOT NULL, present VARCHAR(255) NOT NULL, almacen VARCHAR(4) NOT NULL,  \
						cantidad DECIMAL(12,3) NOT NULL, PRIMARY KEY (referencia, producto, present, almacen))";
						instrucciones[num_instrucciones++]=instruccion;

						instruccion = "INSERT INTO tmpcantidad SELECT d.movimiento, a.producto, a.present, m.almaent, \
						SUM(d.cantidad * a.factor) AS cantidad FROM dmovalma d INNER JOIN movalma m ON   \
						d.movimiento = m.movimiento INNER JOIN articulos a ON a.articulo = d.articulo \
						WHERE d.movimiento = @folio GROUP BY a.producto, a.present";
						instrucciones[num_instrucciones++]=instruccion;

						instruccion = "INSERT INTO tmpcantidad2 SELECT d.movimiento, a.producto, a.present, m.almasal, \
						SUM(d.cantidad * a.factor) AS cantidad FROM dmovalma d INNER JOIN movalma m ON   \
						d.movimiento = m.movimiento INNER JOIN articulos a ON a.articulo = d.articulo \
						WHERE d.movimiento = @folio GROUP BY a.producto, a.present";
						instrucciones[num_instrucciones++]=instruccion;

						instruccion = "UPDATE tmpcantidad tmp INNER JOIN existenciasactuales ea ON ea.producto = tmp.producto \
						AND ea.present = tmp.present AND ea.almacen = tmp.almacen SET ea.cantidad = (ea.cantidad  - tmp.cantidad) \
						, ea.entradas = (ea.entradas - tmp.cantidad)";
						instrucciones[num_instrucciones++]=instruccion;

						instruccion = "UPDATE tmpcantidad2 tmp INNER JOIN existenciasactuales ea ON ea.producto = tmp.producto \
						AND ea.present = tmp.present AND ea.almacen = tmp.almacen SET ea.cantidad = (ea.cantidad  + tmp.cantidad) \
						, ea.salidas = (ea.salidas - tmp.cantidad)";
						instrucciones[num_instrucciones++]=instruccion;

					}
				}

				/////////////***************** FIN DE ACTUALIZAR EXISTENCIAS **************//////////////////


			instrucciones[num_instrucciones++]="COMMIT";
		}

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			if(error == 4){
				instruccion.sprintf("select %d as error, '%s' as producto,\
						'%s' as present,'%s' as multiplo,'%s' as factor", error,produc2,present2,multiplo2,factor2);
			}else{
				instruccion.sprintf("select %d as error", error);
			}
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	} __finally {
		delete buffer_sql;
	}
}

//---------------------------------------------------------------------------
//ID_CON_MOVALM
void ServidorAlmacen::ConsultaMovimientoAlmacen(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    // CONSULTA MOVIMIENTO DE ALMACEN
    AnsiString instruccion;
	AnsiString clave, ordenamiento;
	AnsiString order_by;
	AnsiString almamov = " ", almaent = " ", tipo = " ";
	BufferRespuestas* resp_almacen=NULL;

	clave=mFg.ExtraeStringDeBuffer(&parametros);
	ordenamiento=mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	if (ordenamiento!="0") {
		order_by="order by SUBSTRING(TRIM(a.multiplO),1,4) desc, p.nombre asc, a.present asc";
	} else {
		order_by="order by p.nombre, a.present, a.factor desc";
    }

	// Obtiene todos los generales (de cabecera) del movimiento
    instruccion.sprintf("select m.*, \
        if(m.tipo='T', 'TRASPASO', if(m.tipo='E','ENTRADA','SALIDA') ) as nomtipo, \
        c.descripcion as nomconcepto, \
        concat(e.nombre, ' ', e.appat, ' ',e.apmat) as nomchofer, \
		ae.nombre as nomalmaent, \
		asal.nombre as nomalmasal, \
		p.razonsocial as nomproveedor, \
		c.costofijo, c.tipomov, \
		mvm.mensaje  \
		from movalma m \
        left join conceptosmovalma c on m.concepto=c.concepto \
        left join empleados e on m.chofer=e.empleado \
		left join almacenes ae on m.almaent=ae.almacen \
		left join almacenes asal on m.almasal=asal.almacen \
		left join proveedores p on m.proveedor=p.proveedor \
		LEFT JOIN movalmamensajes mvm ON mvm.referencia = m.movimiento \
        where m.movimiento='%s'", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	   try{
			if(mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_almacen)){
				if (resp_almacen->ObtieneNumRegistros()>0){

					tipo=resp_almacen->ObtieneDato("tipo");

					if(tipo == "T"){
						almamov=resp_almacen->ObtieneDato("almasal");
						almaent=resp_almacen->ObtieneDato("almaent");
					}else if(tipo == "E"){
						almamov=resp_almacen->ObtieneDato("almaent");
						almaent=resp_almacen->ObtieneDato("almaent");
					}else if(tipo == "S"){
						almamov=resp_almacen->ObtieneDato("almasal");
						almaent=" ";
					}
				}
			}else{
				throw (Exception("error movimientos de almacen"));
			}
	   }__finally{
			if (resp_almacen!=NULL){
				delete resp_almacen;
			}

	   }

	// MODIFICAR:  Verifica que la fecha de la factura sea posterior a la fecha de cierre.
	instruccion.sprintf("select if(m.fechamov>cast(e.valor as datetime), 1, 0) as modificar \
						 from movalma m left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' \
						 where m.movimiento='%s'",FormServidor->ObtieneClaveSucursal(), clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene todo el detalle del movimiento.
	instruccion.sprintf("SELECT a.activo, d.cantidad, a.multiplo, p.nombre, a.present, \
		a.producto, a.articulo, d.costo, \
		IFNULL(cp.costounit,0) AS costopromunit, a.factor, \
		IFNULL(cp.costounit,0)*a.factor AS costoprom, \
		d.costobase as costobase, a.factor, a.peso,(a.peso * d.cantidad) as totpeso \
		FROM movalma m \
		INNER JOIN dmovalma d ON m.movimiento=d.movimiento \
		INNER JOIN articulos a ON d.articulo=a.articulo \
		INNER JOIN productos p ON a.producto=p.producto \
		INNER JOIN presentaciones pre ON a.producto=pre.producto and a.present=pre.present \
		LEFT JOIN precalculocostospromedio%s cp ON a.producto=cp.producto AND a.present=cp.present \
		WHERE m.movimiento='%s' %s ", FormServidor->ObtieneClaveEmpresa(), clave, order_by );

	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	instruccion.sprintf("SELECT s.sucursal as sucursalAlt  \
							FROM almacenes al    \
							INNER JOIN terminales t ON t.seccion=al.seccion  \
							INNER JOIN secciones s ON s.seccion=al.seccion    \
							WHERE al.almacen = '%s' GROUP BY al.almacen ", almamov);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	instruccion.sprintf("SELECT s.sucursal as sucursalEnt FROM almacenes al  \
							INNER JOIN terminales t ON 	t.seccion=al.seccion \
							INNER JOIN secciones s ON s.seccion=al.seccion \
							WHERE al.almacen = '%s' GROUP BY al.almacen  ", almaent);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	instruccion.sprintf("SELECT s.sucursal, al.almacen \
							FROM almacenes al \
							INNER JOIN secciones s ON s.seccion=al.seccion  \
							INNER JOIN sucursales sx ON sx.sucursal=s.sucursal \
							WHERE sx.salidaotrasucursal=1 and al.almacen in ('%s') ", almamov);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	instruccion.sprintf("SELECT mensaje FROM movalmamensajes WHERE referencia = '%s' ", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}

//---------------------------------------------------------------------------
//ID_GRA_INVENTARIO
void ServidorAlmacen::GrabaInventario(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  GRABA UN INVENTARIO
	char *buffer_sql=new char[1024*64];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString tarea, clave, usuario;
	int num_partidas, i;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[100];
	TDateTime fecha_inv;
	TDate fecha=Today();
	TTime hora=Time();
	AnsiString descripcion, almacen, tipo, cerrado, terminal;

	try{
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave del inventario
		tarea=mFg.ExtraeStringDeBuffer(&parametros); // Indicador si se va a agregar o modificar.
		usuario=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que esta grabando
		descripcion=mFg.ExtraeStringDeBuffer(&parametros); // descripción
		fecha_inv=StrToDate(mFg.ExtraeStringDeBuffer(&parametros)); // Fecha
		almacen=mFg.ExtraeStringDeBuffer(&parametros); // Almacen
		tipo=mFg.ExtraeStringDeBuffer(&parametros); // Tipo
		cerrado=mFg.ExtraeStringDeBuffer(&parametros); // Cerrado
		terminal=mFg.ExtraeStringDeBuffer(&parametros);

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		// Obtiene el folio para el movimiento
		if (tarea=="A") {
			instruccion.sprintf("select @folioaux:=valor from foliosemp where folio='INVENTARIO' AND sucursal = '%s' %s ",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @foliosig=@folioaux+1");
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @folioaux=cast(@folioaux as char)");
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @folio=concat('%s', lpad(@folioaux,9,'0'))", FormServidor->ObtieneClaveSucursal());
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("update foliosemp set valor=@foliosig where folio='INVENTARIO' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
			instrucciones[num_instrucciones++]=instruccion;
		} else {
			instruccion.sprintf("set @folio='%s'", clave);
			instrucciones[num_instrucciones++]=instruccion;
		}

		// Graba la cabecera en la tabla "inventarios"
		datos.AsignaTabla("inventarios");
		datos.InsCampo("almacen", almacen);
		datos.InsCampo("descripcion", descripcion);
		datos.InsCampo("tipo", tipo);
		datos.InsCampo("fechainv", mFg.DateToAnsiString(fecha_inv));
		datos.InsCampo("cerrado", cerrado);
		if (tarea=="A") {
			datos.InsCampo("inventario", "@folio", 1);
			datos.InsCampo("usualta", usuario);
			datos.InsCampo("usumodi", usuario);
			datos.InsCampo("fechaalta", mFg.DateToAnsiString(fecha));
			datos.InsCampo("horaalta", mFg.TimeToAnsiString(hora));
			datos.InsCampo("fechamodi", mFg.DateToAnsiString(fecha));
			datos.InsCampo("horamodi", mFg.TimeToAnsiString(hora));
			datos.InsCampo("terminalalta", terminal);
			instrucciones[num_instrucciones++]=datos.GenerarSqlInsert();
		} else {
			datos.InsCampo("usumodi", usuario);
			datos.InsCampo("fechamodi", mFg.DateToAnsiString(fecha));
			datos.InsCampo("horamodi", mFg.TimeToAnsiString(hora));
			datos.InsCampo("terminalmodi", terminal);
			instrucciones[num_instrucciones++]=datos.GenerarSqlUpdate("inventario=@folio");
		}

		instrucciones[num_instrucciones++]="COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			instruccion.sprintf("select @folio as folio");
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	} __finally {
		delete buffer_sql;
	}
}


//---------------------------------------------------------------------------
//ID_BAJ_INVENTARIO
void ServidorAlmacen::BorraInventario(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  CANCELA UN INVENTARIO
	char *buffer_sql=new char[1024*32];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instrucciones[50], instruccion;
	int num_instrucciones=0;
	AnsiString clave;
	int i;

	try{
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave del inventario

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		instruccion.sprintf("delete from dinventarios \
			where inventario='%s'", clave);
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("delete from lotesinv where inventario='%s'", clave);
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("delete from inventarios where inventario='%s'", clave);
		instrucciones[num_instrucciones++]=instruccion;

		instrucciones[num_instrucciones++]="COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);
	} __finally {
		delete buffer_sql;
	}
}

//---------------------------------------------------------------------------
//ID_CON_INVENTARIO
void ServidorAlmacen::ConsultaInventario(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA INVENTARIO
	AnsiString instruccion;
	AnsiString clave;

	clave=mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los generales (de cabecera) del inventario
	instruccion.sprintf("select * from inventarios where inventario='%s'", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	instruccion="create temporary table cantidadesaux (lote char(11), cantidad decimal(12,3), PRIMARY KEY principal (lote)) Engine = InnoDB";
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query EjecutaSelectSqlNulo"));

	instruccion.sprintf("insert into cantidadesaux (lote, cantidad) \
		SELECT t.lote, SUM(cantidad) AS cantidad FROM (( \
		select l.lote, sum(d.cantidad) as cantidad \
		from lotesinv l, dinventarios d \
		where l.inventario='%s' and l.inventario=d.inventario and \
		l.lote=d.lote \
		group by l.lote) \
		UNION ALL (select l.lote, sum(d.cantidad) as cantidad \
		from lotesinv l, dinventariosinex d \
		where l.inventario='%s' and l.inventario=d.inventario and \
		l.lote=d.lote \
		group by l.lote)) t GROUP BY t.lote", clave, clave);
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query EjecutaSelectSqlNulo"));

	// Obtiene todos los lotes del inventario.
	instruccion.sprintf("select l.lote, c.cantidad, concat(e.nombre,' ',e.appat,' ',e.apmat) AS nombre, \
		l.usumodi as usuario \
		from lotesinv l inner join empleados e \
		left join cantidadesaux c on c.lote=l.lote \
		where l.inventario='%s' and l.usumodi=e.empleado \
		order by l.lote, l.fechaalta, l.horaalta", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

}

//---------------------------------------------------------------------------
//ID_GRA_LOTE_INVENTARIO
void ServidorAlmacen::GrabaLotInventario(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  GRABA UN LOTE DE INVENTARIO
	char *buffer_sql=new char[1024*64];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString tarea, clave, inventario,  usuario, capmovil;
	int num_partidas, i;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[100];
	TDate fecha=Today();
	TTime hora=Time();

	try{
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave del lote del inventario
		inventario=mFg.ExtraeStringDeBuffer(&parametros); // Clave del inventario al que pertenece el lote
		tarea=mFg.ExtraeStringDeBuffer(&parametros); // Indicador si se va a agregar o modificar.
		usuario=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que esta grabando
		capmovil=mFg.ExtraeStringDeBuffer(&parametros); // Capturado por dispositivo movil

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		// Graba la cabecera en la tabla "lotesinv"
		datos.AsignaTabla("lotesinv");
		if (tarea=="A") {
			datos.InsCampo("lote", clave);
			datos.InsCampo("inventario", inventario);
			datos.InsCampo("usualta", usuario);
			datos.InsCampo("usumodi", usuario);
			datos.InsCampo("fechaalta", mFg.DateToAnsiString(fecha));
			datos.InsCampo("horaalta", mFg.TimeToAnsiString(hora));
			datos.InsCampo("fechamodi", mFg.DateToAnsiString(fecha));
			datos.InsCampo("horamodi", mFg.TimeToAnsiString(hora));
			datos.InsCampo("capmovil", capmovil); // Solo en las altas se graba este campo
			instrucciones[num_instrucciones++]=datos.GenerarSqlInsert();
		} else {
			datos.InsCampo("usumodi", usuario);
			datos.InsCampo("fechamodi", mFg.DateToAnsiString(fecha));
			datos.InsCampo("horamodi", mFg.TimeToAnsiString(hora));
			instrucciones[num_instrucciones++]=datos.GenerarSqlUpdate("inventario='"+inventario+"' and lote='"+clave+"'");
		}

		instrucciones[num_instrucciones++]="COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);
	} __finally {
		delete buffer_sql;
	}

}

//------------------------------------------------------------------------------
//ID_GRA_LOTEINV_MOVIL
void ServidorAlmacen::GrabaLoteinvMovil(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  GRABA LOTE DE INVENTARIO CAPTURADO EN DISPOSITIVO MOVIL
	char *buffer_sql=new char[1024*64*10];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString lote, inventario, usuario;
	int num_partidas, i;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[1000];
	TDate fecha=Today();
	TTime hora=Time();
	TDate fecha_cierre_c, fecha_inventario_c;

	try{
		lote=mFg.ExtraeStringDeBuffer(&parametros);
		inventario=mFg.ExtraeStringDeBuffer(&parametros);
		usuario=mFg.ExtraeStringDeBuffer(&parametros);

		AnsiString cerrado = "0", error = "0";
		BufferRespuestas* resp_valida=NULL;
		try {
			instruccion.sprintf("SELECT cerrado FROM inventarios WHERE inventario='%s'",inventario);
			if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_valida)){
					cerrado=resp_valida->ObtieneDato("cerrado");
			}
		} __finally {
			if (resp_valida!=NULL) delete resp_valida;
		}

		if(cerrado == "1")
			error = "5";

		AnsiString fecha_cierre = "1900-01-01", fecha_inventario = "1900-01-01";
		BufferRespuestas* resp_fecha=NULL;
		try {
			instruccion.sprintf("SELECT es.valor, i.fechaalta FROM estadosistemaemp es, inventarios i WHERE es.estado='FUCIERRE' \
			AND  i.inventario = '%s' AND es.sucursal = '%s' ",inventario, FormServidor->ObtieneClaveSucursal());
			if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_fecha)){
					fecha_cierre=resp_fecha->ObtieneDato("valor").c_str();
					fecha_inventario=resp_fecha->ObtieneDato("fechaalta").c_str();
			}
		} __finally {
			if (resp_fecha!=NULL) delete resp_fecha;
		}

		fecha_cierre_c = mFg.MySqlDateToTDate(fecha_cierre.c_str());

		fecha_inventario = mFg.StrToMySqlDate(fecha_inventario);
		fecha_inventario_c = mFg.MySqlDateToTDate(fecha_inventario.c_str());

		if(fecha_inventario_c <= fecha_cierre_c)
			error = "7";


		/*INICIO NUEVA VALIDACION DE EXISTENCIA DE ERROR*/
		AnsiString auxLote=" ";
		BufferRespuestas* resp_exist=NULL;
		try {
			instruccion.sprintf("SELECT * FROM lotesinv WHERE lote='%s' and inventario ='%s' ",lote,inventario);
			if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_exist)){
					auxLote=resp_exist->ObtieneDato("lote").c_str();
					//fecha_inventario=resp_fecha->ObtieneDato("fechaalta").c_str();
			}
		} __finally {
			if (resp_exist!=NULL) delete resp_exist;
		}
		if(auxLote == lote)
            error = "9";

		/*FIN DE VALIDACION*/


		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		if(error == "0"){

			// Graba la cabecera en la tabla "lotesinv"
			datos.AsignaTabla("lotesinv");
			datos.InsCampo("lote", lote);
			datos.InsCampo("inventario", inventario);
			datos.InsCampo("usualta", usuario);
			datos.InsCampo("usumodi", usuario);
			datos.InsCampo("fechaalta", mFg.DateToAnsiString(fecha));
			datos.InsCampo("horaalta", mFg.TimeToAnsiString(hora));
			datos.InsCampo("fechamodi", mFg.DateToAnsiString(fecha));
			datos.InsCampo("horamodi", mFg.TimeToAnsiString(hora));
			datos.InsCampo("capmovil", "1");
			instrucciones[num_instrucciones++]=datos.GenerarSqlInsert();


			// Graba las partidas en "dinventarios"
			num_partidas=StrToInt(mFg.ExtraeStringDeBuffer(&parametros)); // Obtiene el número de partidas
			for (i=0; i<num_partidas; i++) {
				datos.AsignaTabla("dinventarios");
				parametros+=datos.InsCamposDesdeBuffer(parametros);
				instrucciones[num_instrucciones++]=datos.GenerarSqlInsert();
			}


			// Graba las partidas en "dinventarios"
			num_partidas=StrToInt(mFg.ExtraeStringDeBuffer(&parametros)); // Obtiene el número de partidas
			for (i=0; i<num_partidas; i++) {
				datos.AsignaTabla("dinventariosinex");
				parametros+=datos.InsCamposDesdeBuffer(parametros);
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
			instruccion.sprintf("select %s as error",error);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}

	} __finally {
		delete buffer_sql;
	}
}

//------------------------------------------------------------------------------
//ID_GRA_RECEPCIONES_MOVIL
void ServidorAlmacen::GrabaRecepcionesMovil(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  GRABA RECEPCION CAPTURADA EN DISPOSITIVO MOVIL
	char *buffer_sql=new char[1024*64*10];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString recepcion, proveedor, auxFHA;
	TDateTime fechahoraalta;
	int num_partidas, i;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[1000];

	try{
		recepcion=mFg.ExtraeStringDeBuffer(&parametros);
		proveedor=mFg.ExtraeStringDeBuffer(&parametros);
		auxFHA=mFg.ExtraeStringDeBuffer(&parametros);

		fechahoraalta = StrToDateTime(auxFHA);

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		// Graba la cabecera en la tabla "recepciones"
		datos.AsignaTabla("recepciones");
		datos.InsCampo("recepcion", NULL);
		datos.InsCampo("proveedor", proveedor);
		datos.InsCampo("fechaalta", FormatDateTime("dd/mm/yyyy", fechahoraalta));
		datos.InsCampo("horaalta", FormatDateTime("tt", fechahoraalta));

		instrucciones[num_instrucciones++]=datos.GenerarSqlInsert();

		instrucciones[num_instrucciones++]="SET @AUXILIAR = LAST_INSERT_ID()";

		// Graba las partidas en "drecepciones"
		num_partidas=StrToInt(mFg.ExtraeStringDeBuffer(&parametros)); // Obtiene el número de partidas
		for (i=0; i<num_partidas; i++) {
			datos.AsignaTabla("drecepciones");
			parametros+=datos.InsCamposDesdeBuffer(parametros);
			//modificar el indice
			datos.ReemplazaCampo("recepcion", "@AUXILIAR",1);
			instrucciones[num_instrucciones++]=datos.GenerarSqlInsert();
		}

		instrucciones[num_instrucciones++]="COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			instruccion.sprintf("select 0 as error");
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	} __finally {
		delete buffer_sql;
	}
}
//------------------------------------------------------------------------------
//ID_GRA_RECEP
void ServidorAlmacen::GrabaRecepcionEditada(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  GRABA UNA RECEPCION
	char *buffer_sql=new char[1024*64*10];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString clave, factura;
	int num_partidas, i;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[1000];

	try{
		clave=mFg.ExtraeStringDeBuffer(&parametros);
		factura=mFg.ExtraeStringDeBuffer(&parametros);

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		instruccion.sprintf("set @recep='%s'", clave);
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("set @fact='%s'", factura);
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("delete from drecepciones where recepcion=@recep and factura=@fact");
		if (factura!="") {

		}
		instrucciones[num_instrucciones++]=instruccion;

		// Graba las partidas en "dkits"
		num_partidas=StrToInt(mFg.ExtraeStringDeBuffer(&parametros)); // Obtiene el número de partidas
		for (i=0; i<num_partidas; i++) {
			datos.AsignaTabla("drecepciones");
			parametros+=datos.InsCamposDesdeBuffer(parametros);
			datos.InsCampo("recepcion", "@recep", 1);
			datos.InsCampo("factura", "@fact", 1);
			instrucciones[num_instrucciones++]=datos.GenerarSqlInsert();
		}

		instrucciones[num_instrucciones++]="COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			instruccion.sprintf("select @recep as recep");
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	} __finally {
		delete buffer_sql;
	}
}
//---------------------------------------------------------------------------
//ID_BAJ_LOTE_INVENTARIO
void ServidorAlmacen::BorraLotInventario(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  BORRA EL LOTE DE UN INVENTARIO
	char *buffer_sql=new char[1024*32];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instrucciones[50], instruccion;
	int num_instrucciones=0;
	AnsiString clave, inventario;
	int i;

	try{
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave del lote
		inventario=mFg.ExtraeStringDeBuffer(&parametros); // Clave del inventario al que pertenece el lote

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		instruccion.sprintf("delete from dinventarios \
			where lote='%s' and inventario='%s'", clave, inventario);
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("delete from lotesinv where lote='%s' and inventario='%s'", clave, inventario);
		instrucciones[num_instrucciones++]=instruccion;


		instrucciones[num_instrucciones++]="COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);
	} __finally {
		delete buffer_sql;
	}
}

//---------------------------------------------------------------------------
//ID_CON_LOTE_INVENTARIO
void ServidorAlmacen::ConsultaLotInventario(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    // CONSULTA INVENTARIO
    AnsiString instruccion;
    AnsiString lote, inventario;

    lote=mFg.ExtraeStringDeBuffer(&parametros);
    inventario=mFg.ExtraeStringDeBuffer(&parametros);
    mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

    // Obtiene todos los generales (de cabecera) del lote
	instruccion.sprintf("select l.*, concat(e.nombre, ' ', e.appat, ' ', e.apmat) as nombreusumodi \
		from lotesinv l, empleados e where l.lote='%s' and l.inventario='%s' and l.usumodi=e.empleado ", lote, inventario);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene todos los articulos del lote.
	instruccion.sprintf("SELECT t.cantidad, t.multiplo, t.nombre, t.present, t.producto, t.articulo, \
	t.observaciones, t.activo, t.tipo FROM ((SELECT d.cantidad, \
	a.multiplo, \
	p.nombre, \
	a.present, \
	a.producto, \
	a.articulo, \
	d.observaciones, \
	d.id, \
	a.activo, \
	1 AS tipo \
	FROM lotesinv l \
	INNER JOIN dinventarios d ON l.lote=d.lote AND l.inventario=d.inventario \
	INNER JOIN articulos a ON d.articulo=a.articulo \
	INNER JOIN productos p ON a.producto=p.producto \
	WHERE l.lote='%s' \
	AND l.inventario='%s' \
	ORDER BY d.id \
	)UNION ALL( \
	SELECT d.cantidad, \
	'-' AS multiplo, \
	d.codigobarras AS nombre, \
	'-' AS present, \
	'-' AS producto, \
	'-' AS articulo, \
	d.observaciones, \
	d.id, \
	0 AS activo, \
	2 AS tipo \
	FROM lotesinv l \
	LEFT JOIN dinventariosinex d ON d.lote=l.lote AND d.inventario=l.inventario \
	WHERE l.lote='%s' \
	AND l.inventario='%s' \
    AND d.cantidad > 0 \
	ORDER BY d.id )) t ORDER BY t.id ASC", lote, inventario, lote, inventario);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}

//---------------------------------------------------------------------------
//ID_GRA_ART_INVENTARIO
void ServidorAlmacen::GrabaArtInventario(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  GRABA UN ARTICULO DE INVENTARIO
	char *buffer_sql=new char[1024*64];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString articulo, inventario, lote, usuario, cantidad;
	int num_partidas, i;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[100];
	TDate fecha=Today();
	TTime hora=Time();

	try{
		articulo=mFg.ExtraeStringDeBuffer(&parametros); // Clave del artículo
		lote=mFg.ExtraeStringDeBuffer(&parametros); // Clave del lote
		inventario=mFg.ExtraeStringDeBuffer(&parametros); // Clave del inventario
		cantidad=mFg.ExtraeStringDeBuffer(&parametros); // Cantidad del artículo
		usuario=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que esta grabando

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		// Graba la modificación en lotesinv
		datos.AsignaTabla("lotesinv");
		datos.InsCampo("usumodi", usuario);
		datos.InsCampo("fechamodi", mFg.DateToAnsiString(fecha));
		datos.InsCampo("horamodi", mFg.TimeToAnsiString(hora));
		instrucciones[num_instrucciones++]=datos.GenerarSqlUpdate("inventario='"+inventario+"' and lote='"+lote+"'");

		// Agregar el artículo
		instruccion.sprintf("replace into dinventarios (inventario, lote, articulo, cantidad) \
			values ('%s', '%s', '%s', %s)", inventario, lote, articulo, cantidad);
		instrucciones[num_instrucciones++]=instruccion;

		instrucciones[num_instrucciones++]="COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);
	} __finally {
		delete buffer_sql;
	}
}

//---------------------------------------------------------------------------
//ID_BAJ_ART_INVENTARIO
void ServidorAlmacen::BorraArtInventario(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  BORRA EL ARTICULO DE UN INVENTARIO
	char *buffer_sql=new char[1024*32];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instrucciones[50], instruccion;
	int num_instrucciones=0;
	AnsiString articulo, inventario, lote, usuario;
	TDate fecha=Today();
	TTime hora=Time();
	int i;

	try{
		articulo=mFg.ExtraeStringDeBuffer(&parametros); // Clave del artículo
		lote=mFg.ExtraeStringDeBuffer(&parametros); // Clave del lote
		inventario=mFg.ExtraeStringDeBuffer(&parametros); // Clave del inventario
		usuario=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que esta grabando

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		// Graba la modificación en lotesinv
		datos.AsignaTabla("lotesinv");
		datos.InsCampo("usumodi", usuario);
		datos.InsCampo("fechamodi", mFg.DateToAnsiString(fecha));
		datos.InsCampo("horamodi", mFg.TimeToAnsiString(hora));
		instrucciones[num_instrucciones++]=datos.GenerarSqlUpdate("inventario='"+inventario+"' and lote='"+lote+"'");

		instruccion.sprintf("delete from dinventarios  \
			where inventario='%s' and lote='%s' and articulo='%s'", inventario, lote, articulo);
		instrucciones[num_instrucciones++]=instruccion;

		instrucciones[num_instrucciones++]="COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);
	} __finally {
		delete buffer_sql;
	}
}

//---------------------------------------------------------------------------
//ID_CON_ART_INVENTARIO
void ServidorAlmacen::ConsultaArtInventario(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    // CONSULTA INVENTARIO
    AnsiString instruccion;
    AnsiString articulo, inventario, lote;

    articulo=mFg.ExtraeStringDeBuffer(&parametros); // Clave del artículo
	lote=mFg.ExtraeStringDeBuffer(&parametros); // Clave del lote
	inventario=mFg.ExtraeStringDeBuffer(&parametros); // Clave del inventario
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene el artículo
	instruccion.sprintf("select * from dinventarios d where d.inventario='%s' and d.lote='%s' and d.articulo='%s'", inventario, lote, articulo);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}

//---------------------------------------------------------------------------
//ID_GRA_EMBARQUE
void ServidorAlmacen::GrabaEmbarque(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros){

	//  GRABA UN EMBARQUE
	char *buffer_sql=new char[1024*64*10];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString tarea, clave, chofer, viaembarque, pedido, surtidor, division,mListaManio,clave_suc;
	AnsiString terminal, usuario;
	AnsiString remolque;
	int num_pedidos, i;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[200];
	TDate fecha=Today();
	AnsiString select, aux_embarque, aux_cancelado;
	BufferRespuestas* resp_embarque=NULL;
	bool bandera_grabar;
	int error=0;
	TStringList* mListadomaniobristas = new TStringList();
	int reqCartaPorte2 = 0;
    AnsiString archivo_temp1="", archivo_temp2="", archivo_temp3="", archivo_temp4="";

	try{
		clave_suc=mFg.ExtraeStringDeBuffer(&parametros); // Clave de la sucursal
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave del lote del inventario
		tarea=mFg.ExtraeStringDeBuffer(&parametros); // Indicador si se va a agregar o modificar.
		chofer=mFg.ExtraeStringDeBuffer(&parametros); // Chofer que realizara la entrega.
		surtidor=mFg.ExtraeStringDeBuffer(&parametros); // Via de embarque Asignada
		remolque=mFg.ExtraeStringDeBuffer(&parametros); //remolque
		viaembarque=mFg.ExtraeStringDeBuffer(&parametros); // Via de embarque Asignada
		division=mFg.ExtraeStringDeBuffer(&parametros); // División del embarque
		terminal=mFg.ExtraeStringDeBuffer(&parametros); // terminal que dio de alta el embarque
		usuario=mFg.ExtraeStringDeBuffer(&parametros); // usuario que dio de alta el embarque o que lo editó
		reqCartaPorte2=StrToInt(mFg.ExtraeStringDeBuffer(&parametros)); // División del embarque

		// Verifica que el embarque no este liberado de lo contrario no se podra guardar el embarque---------
		instruccion.sprintf("select @error:=if(em.etapa=9, 1, 0) as error from embarques em where em.embarque='%s'", clave);
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 1, error);

		// Verifica que el embarque no este cancelado de lo contrario no se podra guardar el embarque---------
		instruccion.sprintf("select @error:=if(em.cancelado=1, 1, 0) as error from embarques em where em.embarque='%s'", clave);
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 2, error);

		// Verifica que la vía de embarque no esté en reparación ---------
		instruccion.sprintf("select @error:=if(v.estado='R', 1, 0) as error, v.estado, v.viaembarq \
			from viasembarque v \
			where v.viaembarq='%s' "
			, viaembarque);
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 3, error);

		if (error==0) {
			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

			if (tarea=="A")  {
				instruccion.sprintf("set @folioUUID:=UUID()");
				instrucciones[num_instrucciones++]=instruccion;
				// Obtiene el folio para el movimiento
				instruccion.sprintf("select @folioaux:=valor from foliosemp where folio='ASIGEMB' AND sucursal = '%s' %s ",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliosig=@folioaux+1");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @folioaux=cast(@folioaux as char)");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @folio=concat('%s', lpad(@folioaux,9,'0'))", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("update foliosemp set valor=@foliosig where folio='ASIGEMB' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;

                // folio surtido
				instruccion.sprintf("select @folioaux_surtido:=valor from foliosemp where folio='SURTPEDI' and sucursal= '%s' %s ",
				FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
				instrucciones[num_instrucciones++]=instruccion;

				instruccion.sprintf("set @foliosig_surtido:=(@folioaux_surtido+1) ");
				instrucciones[num_instrucciones++]=instruccion;

				instruccion.sprintf("set @folioauxcast:=cast(@folioaux_surtido as char) ");
				instrucciones[num_instrucciones++]=instruccion;

				instruccion.sprintf("set @folio_surtido:=concat('%s', lpad(@folioauxcast, 9,'0')) ",
				FormServidor->ObtieneClaveSucursal() );
				instrucciones[num_instrucciones++]=instruccion;

				instruccion.sprintf("update foliosemp set valor=@foliosig_surtido where folio='SURTPEDI' AND sucursal = '%s' ",
				FormServidor->ObtieneClaveSucursal() );
				instrucciones[num_instrucciones++]=instruccion;


			} else {
					// Modificación
					instruccion.sprintf("set @folio='%s'", clave);
					instrucciones[num_instrucciones++]=instruccion;

					//se borra el embarque en los pedidos que se habian asignado con anterioridad
					instruccion.sprintf("update pedidosventa set embarque=NULL, embarquediv=1 where embarque=@folio and embarquediv=%s", division);
						instrucciones[num_instrucciones++]=instruccion;

					//Variable temporal, la cual hace referencia a el estado previo de la via de embarque
					instruccion.sprintf("SELECT  @stx:= vi.estado FROM viasembarque vi INNER JOIN embarques em ON em.viaembarq=vi.viaembarq WHERE em.embarque=@folio");
						instrucciones[num_instrucciones++]=instruccion;

					// Asignar el estado anterior de la via de embarque a la nueva via de embarque
					instruccion.sprintf("UPDATE viasembarque  SET  estado =  @stx WHERE  viaembarq='%s'",viaembarque);
						instrucciones[num_instrucciones++]=instruccion;

					//se libera la via de embarque en caso de que se haya cambiado.
					instruccion.sprintf("update viasembarque v, embarques em set v.estado=if(v.estado='R','R','D') where em.embarque=@folio and em.viaembarq=v.viaembarq and v.viaembarq<>'%s'",viaembarque);
						instrucciones[num_instrucciones++]=instruccion;
					}


			// Graba la cabecera en la tabla "embarques"
			datos.AsignaTabla("embarques");
			if (tarea=="A") {
				datos.InsCampo("embarque", "@folio",1);
				datos.InsCampo("viaembarq", viaembarque);
				datos.InsCampo("chofer", chofer);
					datos.InsCampo("surtidor", surtidor);
				datos.InsCampo("fechasalid", mFg.DateToAnsiString(fecha));
				datos.InsCampo("horasalid", mFg.TimeToMySqlTime(Time()) );
				datos.InsCampo("idweb", "@folioUUID",1);
				datos.InsCampo("fechainicarga", mFg.DateToAnsiString(fecha));
				datos.InsCampo("horainicarga", mFg.TimeToMySqlTime(Time()));
				datos.InsCampo("sucursal", clave_suc);
				if (remolque=="") {
					datos.InsCampo("idremolque", "NULL");
				} else {
					datos.InsCampo("idremolque", remolque);
				}
				datos.InsCampo("concartaporte", reqCartaPorte2);
				instrucciones[num_instrucciones++]=datos.GenerarSqlInsert();

				instruccion.sprintf("INSERT INTO surtidopedidos \
				(surtido, embarque, terminal, fecha_alta, hora_alta, usualta, cancelado) \
				 VALUES (@folio_surtido, @folio, '%s', '%s', '%s', '%s', 0 ) ",
				 terminal, mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(Time()), usuario);
				instrucciones[num_instrucciones++]=instruccion;

			} else {
				datos.InsCampo("viaembarq", viaembarque);
				datos.InsCampo("chofer", chofer);
				datos.InsCampo("surtidor", surtidor);
				if (remolque=="") {
					datos.InsCampo("idremolque", "NULL");
				} else {
					datos.InsCampo("idremolque", remolque);
				}
				datos.InsCampo("concartaporte", reqCartaPorte2);
				instrucciones[num_instrucciones++]=datos.GenerarSqlUpdate("embarque=@folio");

				instruccion="SELECT @surtido:=s.surtido FROM surtidopedidos s WHERE s.embarque=@folio ";
				instrucciones[num_instrucciones++]=instruccion;

				instruccion.sprintf("UPDATE surtidopedidos s SET s.fechamodi=CURDATE(), s.horamodi=CURTIME(), usumod='%s' \
					WHERE s.surtido=@surtido ", usuario);
				instrucciones[num_instrucciones++]=instruccion;
			}

			instruccion.sprintf(" UPDATE surtidores s \
			LEFT JOIN parametrosglobemp pg ON pg.parametro = 'ALMACENGLOB' AND pg.idempresa = %s \
			SET s.almacen_asignado = pg.valor \
			WHERE s.empleado = '%s' AND s.almacen_asignado IS NULL ", FormServidor->ObtieneClaveEmpresa(), surtidor);
			instrucciones[num_instrucciones++]=instruccion;

			// Graba la asignacion del embarque en los pedidos
			num_pedidos=StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
			for (i=0; i<num_pedidos; i++) {
				pedido=mFg.ExtraeStringDeBuffer(&parametros);
				// Asigna el embarque a el pedido
					instruccion.sprintf("update pedidosventa set embarque=@folio, embarquediv=%s where referencia='%s'", division, pedido);
				instrucciones[num_instrucciones++]=instruccion;
			}


			// Crear tabla para nuevo calculo de almacenes surtidos
			instruccion.sprintf("CREATE TEMPORARY TABLE tmp_almacenes_surtidos \
				(surtido VARCHAR(11), clasificacion VARCHAR(11) ) Engine = InnoDB");
			instrucciones[num_instrucciones++]=instruccion;

			// Crear tabla donde estarán convertidos los productos
			instruccion.sprintf("CREATE TEMPORARY TABLE tmp_producto_convertido \
				(referencia VARCHAR(11), articulo VARCHAR(9), \
				producto varchar(8), present varchar(255), \
				multiplo varchar(13), cantidad decimal(12,3), factor DECIMAL(10,3), INDEX(articulo)) Engine = InnoDB");
			instrucciones[num_instrucciones++]=instruccion;

			archivo_temp1=mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);
			instruccion.sprintf("SELECT p.referencia, ar.articulo, ar.producto, ar.present, ar.multiplo, \
				SUM(dp.cantidad) AS cantidad, ar.factor \
				FROM pedidosventa p \
				INNER JOIN embarques e ON e.embarque=p.embarque \
				INNER JOIN dpedidosventa dp ON p.referencia=dp.referencia \
				INNER JOIN articulos ar ON ar.articulo = dp.articulo \
				INNER JOIN presentacionesminmax pmm ON pmm.producto = ar.producto AND pmm.present = ar.present \
				WHERE p.embarque=@folio AND ar.multiplo = pmm.maxmult \
				GROUP BY referencia, ar.producto, ar.present INTO OUTFILE '%s' ", archivo_temp1);
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE tmp_producto_convertido ",archivo_temp1);
			instrucciones[num_instrucciones++]=instruccion;

			archivo_temp2=mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);
			instruccion.sprintf("SELECT p.referencia, art.articulo, art.producto, art.present, pmm.maxmult, \
				TRUNCATE((dp.cantidad*ar.factor)/pmm.maxfactor,0) AS cantidad, pmm.maxfactor \
				FROM pedidosventa p \
				INNER JOIN embarques e ON e.embarque=p.embarque \
				INNER JOIN dpedidosventa dp ON p.referencia=dp.referencia \
				INNER JOIN articulos ar ON ar.articulo = dp.articulo \
				INNER JOIN presentacionesminmax pmm ON pmm.producto = ar.producto AND pmm.present = ar.present \
				INNER JOIN articulos art ON art.producto = pmm.producto AND art.present = pmm.present AND art.multiplo = pmm.maxmult \
				WHERE p.embarque=@folio AND ar.multiplo <> pmm.maxmult  \
				GROUP BY referencia, ar.producto, ar.present INTO OUTFILE '%s' ", archivo_temp2);
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE tmp_producto_convertido ",archivo_temp2);
			instrucciones[num_instrucciones++]=instruccion;

			archivo_temp3=mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);
			instruccion.sprintf(" SELECT p.referencia, ar.articulo, ar.producto, ar.present, ar.multiplo, \
				TRUNCATE((MOD(SUM(dp.cantidad)*ar.factor, pmm.maxfactor)/ar.factor),5), ar.factor \
				FROM pedidosventa p \
				INNER JOIN embarques e ON e.embarque=p.embarque  \
				INNER JOIN dpedidosventa dp ON p.referencia=dp.referencia \
				INNER JOIN articulos ar ON ar.articulo = dp.articulo \
				INNER JOIN presentacionesminmax pmm ON pmm.producto = ar.producto AND pmm.present = ar.present \
				WHERE p.embarque=@folio AND ar.multiplo <> pmm.maxmult \
				GROUP BY referencia, ar.articulo INTO OUTFILE '%s' ", archivo_temp3);
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE tmp_producto_convertido ", archivo_temp3);
			instrucciones[num_instrucciones++]=instruccion;

			archivo_temp4=mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);
            //Guardar las clasificaciones de los almacenes de los pedidos
			instruccion.sprintf(" SELECT @surtido:=sp.surtido, ac.clasificacion \
				FROM embarques e \
				INNER JOIN pedidosventa pv ON pv.embarque = e.embarque \
				INNER JOIN tmp_producto_convertido dp ON dp.referencia=pv.referencia \
				INNER JOIN surtidopedidos sp ON sp.embarque = pv.embarque \
				INNER JOIN terminales ter ON ter.terminal = pv.terminal \
				INNER JOIN articuloxseccion axs ON axs.articulo = dp.articulo AND axs.seccion = ter.seccion \
				INNER JOIN almacenes_clasificados ac ON ac.almacen = axs.almacen \
				WHERE e.embarque = @folio AND dp.cantidad <> 0 \
				GROUP BY ac.clasificacion  INTO OUTFILE '%s' ", archivo_temp4);
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE tmp_almacenes_surtidos ", archivo_temp4);
			instrucciones[num_instrucciones++]=instruccion;


			instruccion = "INSERT IGNORE INTO almacenes_surtidos (surtido, clasificacion) \
				SELECT surtido, clasificacion \
				FROM tmp_almacenes_surtidos tas";
			instrucciones[num_instrucciones++]=instruccion;


			if(tarea=="M"){

				//Si se quitan pedidos se eliminan registros
				instruccion = "DELETE als \
					FROM almacenes_surtidos als \
					LEFT JOIN tmp_almacenes_surtidos tas ON tas.surtido=als.surtido AND tas.clasificacion=als.clasificacion \
					WHERE als.surtido=@surtido AND tas.surtido IS NULL ";
				instrucciones[num_instrucciones++]=instruccion;

			}

            // INICIO listado de maniobristas
            //TFrmPolizasContpaq *FrmPolizasContpaq; lin 2485
			mListaManio=mFg.ExtraeStringDeBuffer(&parametros); //lista de maniobristas
			int contListMan=0;
			AnsiString paramLista="";
			AnsiString unicoLista="";
			AnsiString listaParamManiobristas="";

			if(mListaManio!=""){
				mListadomaniobristas->Delimiter = '|';
				mListadomaniobristas->StrictDelimiter = true;
				mListadomaniobristas->DelimitedText = mListaManio;
                //se
				if(tarea=="M"){
					instruccion.sprintf("delete from dembarquemaniobrista where embarque=@folio");
					instrucciones[num_instrucciones++]=instruccion;
				}


				for (int x=0; x<mListadomaniobristas->Count-1; x++) {
                   unicoLista="";
					if (mListadomaniobristas->Strings[x]!=L"") {

						unicoLista=mListadomaniobristas->Strings[x];

						instruccion.sprintf("INSERT INTO dembarquemaniobrista (embarque,maniobrista,id) values (@folio,'%s','%d')",unicoLista,x);
						instrucciones[num_instrucciones++]=instruccion;

					}
				}
			}else{
				//en caso de que sea modificacion y en algun momento se decida quitar la lista de maniobristas
				//la lista se recibe vacia, y no se modificaria o seguiran los maniobristas asignados a ese embarque
				//entonces se valdará que cuando se reciba vacia y sea modificacion y existan maniobristas
				// se borren los maniobristas de de ese embarque
                //el codigo no estoy segur@ que sea aplicado..... porque faltaria validar
			   /*	if(tarea=="M"){
					instruccion.sprintf("DELETE FROM dembarquemaniobrista WHERE EXISTS( SELECT * FROM dembarquemaniobrista WHERE embarque=@folio)");
					instrucciones[num_instrucciones++]=instruccion;
				}  */
            }

			//FIN listado de maniobristas

			instrucciones[num_instrucciones++]="COMMIT";
		}

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);

		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
            if (archivo_temp1!="")
				mServidorVioleta->BorraArchivoTemp(archivo_temp1);
			if (archivo_temp2!="")
				mServidorVioleta->BorraArchivoTemp(archivo_temp2);
			if (archivo_temp3!="")
				mServidorVioleta->BorraArchivoTemp(archivo_temp3);
			if (archivo_temp4!="")
				mServidorVioleta->BorraArchivoTemp(archivo_temp4);

			//instruccion="select @folio as folio, @error as error";
			instruccion.sprintf("select %d as error, @folio as folio", error);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	} __finally {
		if (resp_embarque!=NULL) delete resp_embarque;
		delete buffer_sql;

	}
}

//----------------------------------------------------------------------------
//ID_CANC_EMBARQUE
void ServidorAlmacen::CancelaEmbarque(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  CANCELA VENTA
	char *buffer_sql=new char[1024*32];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instrucciones[50], instruccion;
	int num_instrucciones=0;
	AnsiString clave;
	int error=0;
	int i;
	TDate fecha=Today();
	TTime hora=Time();


	try{
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave del embarque (referencia)

		// Verifica que el embarque no este liberado de lo contrario no se podra cancelar el embarque---------
		instruccion.sprintf("select @error:=if(em.etapa=9, 1, 0) as error from embarques em where em.embarque='%s'", clave);
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 1, error);

		// Verifica que el embarque no tenga facturas asignadas, de lo contrario no se podra cancelar el embarque---------
		instruccion.sprintf("select @error:=1 as error from ventas v where v.embarque='%s' and v.cancelado=0 limit 1", clave);
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 2, error);

		if (error==0) {
			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

			// Cancela y marca como libre el embarque además Libera la via de embarque (excepto si está en reparación.
			instruccion.sprintf("update embarques e, viasembarque v \
				set e.cancelado=1, e.etapa=9, v.estado=if(v.estado='R','R','D'), e.fecharecep='%s', e.horarecep='%s' \
				where e.embarque='%s' and v.viaembarq=e.viaembarq ",mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), clave);
			instrucciones[num_instrucciones++]=instruccion;

			// elimina la asigancion del embarque en los pedidos
			instruccion.sprintf("update pedidosventa set embarque=NULL where embarque='%s' ", clave);
			instrucciones[num_instrucciones++]=instruccion;

			// cancela el registro de surtido que pertence al embarque
			instruccion.sprintf("update surtidopedidos set cancelado=1 where embarque='%s' ", clave);
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

// ID_QRY_FRAME_ART
void ServidorAlmacen::ConsultaFrameArticulo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA LOS DATOS DE UN ARTICULO UTILES PARA UN FRAME
	AnsiString instruccion;
	AnsiString articulo, codigobarras, producto, present, multiplo, tipo, sucursal_consulta, sucursal_existencias;
	AnsiString solo_activos, condicion_solo_activos=" ", condicion_solo_activosB=" ", condicion_sucursal=" ";
	AnsiString almacen_aux, condicion_almacen, cad_conjunto_almacenes=" ";
	AnsiString considerar_verventmayoreo, condicion_considerar_verventmayoreo=" ";
	int calcular_existencias;
	int i;
	TDate fecha=Today();
	AnsiString archivo_temp1,archivo_temp2;
	AnsiString usuario, terminal, mensaje, lProducto, lPresent, lMultiplo, IP, modo_calcular_existencias;
	BufferRespuestas* resp_almacenes=NULL;
	AnsiString empresa, permiso_accion, condicion_accion = " ";
	AnsiString almacen_existencias;
	AnsiString cad_conjunto_alma_ex;
	BufferRespuestas* resp_almacenes_ex = NULL;

	try{
		sucursal_consulta=mFg.ExtraeStringDeBuffer(&parametros);
		articulo=mFg.ExtraeStringDeBuffer(&parametros);
		solo_activos=mFg.ExtraeStringDeBuffer(&parametros);
		codigobarras=mFg.ExtraeStringDeBuffer(&parametros);
		producto=mFg.ExtraeStringDeBuffer(&parametros);
		present=mFg.ExtraeStringDeBuffer(&parametros);
		multiplo=mFg.ExtraeStringDeBuffer(&parametros);
		tipo=mFg.ExtraeStringDeBuffer(&parametros);
		calcular_existencias=StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
		sucursal_existencias=mFg.ExtraeStringDeBuffer(&parametros);
		considerar_verventmayoreo=mFg.ExtraeStringDeBuffer(&parametros);
		usuario=mFg.ExtraeStringDeBuffer(&parametros);
		terminal=mFg.ExtraeStringDeBuffer(&parametros);
		lProducto=mFg.ExtraeStringDeBuffer(&parametros);
		lPresent=mFg.ExtraeStringDeBuffer(&parametros);
		lMultiplo=mFg.ExtraeStringDeBuffer(&parametros);
		IP = mFg.ExtraeStringDeBuffer(&parametros);
		modo_calcular_existencias = mFg.ExtraeStringDeBuffer(&parametros);
		empresa = mFg.ExtraeStringDeBuffer(&parametros);
		permiso_accion = mFg.ExtraeStringDeBuffer(&parametros);

		mensaje = "-- sucursal_consulta:" + sucursal_consulta +
			" articulo: " + articulo +
			" solo_activos: " + solo_activos +
			" codigobarras: " + codigobarras +
			" producto: " + producto +
			" present: " + present +
			" multiplo: " + multiplo +
			" tipo: " + tipo +
			" calcular_existencias: " + calcular_existencias +
			" sucursal_existencias: " + sucursal_existencias +
			" considerar_verventmayoreo: " + considerar_verventmayoreo +
			" usuario: " + usuario +
			" terminal: " + terminal +
			" lProducto: " + lProducto +
			" lPresent: " + lPresent +
			" lMultiplo: " + lMultiplo +
			" IP: " + IP +
			" modo_calcular_existencias: " + modo_calcular_existencias +
			" empresa: " + empresa ;


		mServidorVioleta->MuestraMensaje(mensaje);
		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

		if (considerar_verventmayoreo=="1") {
			condicion_considerar_verventmayoreo=" and t.verventmayoreo=1 ";
		}

		if (solo_activos=="1") {
			condicion_solo_activos=" a.activo=1 and ";
			condicion_solo_activosB=" and a.activo=1 ";
		}

		if (sucursal_existencias!=" ") {
				instruccion.sprintf("SELECT a.almacen FROM almacenes a \
					INNER JOIN secciones s ON a.seccion=s.seccion \
					WHERE s.sucursal='%s'", sucursal_existencias);
				mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_almacenes);
				for(i=0; i<resp_almacenes->ObtieneNumRegistros(); i++){
					resp_almacenes->IrAlRegistroNumero(i);
					almacen_aux=resp_almacenes->ObtieneDato("almacen");

					cad_conjunto_almacenes+="'";
					cad_conjunto_almacenes+=almacen_aux;
					cad_conjunto_almacenes+="'";
					if (i<resp_almacenes->ObtieneNumRegistros()-1) // A todos menos al ultimo impuesto el signo +.
						cad_conjunto_almacenes+=",";
				}
		}

		if(permiso_accion== "1"){
			condicion_accion=" WHERE mul.permiso = 1 ";
			if(tipo== "0")
				permiso_accion = "permitecompras";
			else if (tipo == "1")
				permiso_accion = "permiteventas";
			else if (tipo == "5")
				permiso_accion = "permitemovalma";
			else
                permiso_accion = "0";
		}


		instruccion.sprintf("SELECT a.almacen FROM almacenes a \
			INNER JOIN secciones s ON a.seccion=s.seccion \
			WHERE s.sucursal='%s'", sucursal_consulta);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_almacenes_ex);
		for(i=0; i<resp_almacenes_ex->ObtieneNumRegistros(); i++){
			resp_almacenes_ex->IrAlRegistroNumero(i);
			almacen_existencias=resp_almacenes_ex->ObtieneDato("almacen");

			cad_conjunto_alma_ex+="'";
			cad_conjunto_alma_ex+=almacen_existencias;
			cad_conjunto_alma_ex+="'";
			if (i<resp_almacenes_ex->ObtieneNumRegistros()-1) // A todos menos al ultimo impuesto el signo +.
				cad_conjunto_alma_ex+=",";
		}

		if (articulo==" ") {
			if (producto==" ") {
				if (codigobarras==" ")
					throw Exception("No deben estar vacios los parámetros de busqueda del artículo");

				if(permiso_accion=="0")	
					// Busca el artículo con el código de barras dado.
					instruccion.sprintf("select * from \
						((select a.*, @articulo:=a.articulo, @producto:=a.producto, \
						@present:=a.present, @multiplo:=a.multiplo, (ex.cantidad/factor) AS existencias \
						from articulos a \
						INNER JOIN existenciasactuales ex ON ex.producto = a.producto AND ex.present = a.present AND ex.almacen IN (%s) \
						where %s a.ean13='%s') \
						  union \
						(select a.*, @articulo:=a.articulo, @producto:=a.producto, \
						@present:=a.present, @multiplo:=a.multiplo, (ex.cantidad/factor) AS existencias \
						from articulos a, codigosbarras cb, existenciasactuales ex \
						where %s a.articulo=cb.articulo and cb.codigobarras='%s' \
						AND ex.producto = a.producto AND ex.present = a.present AND ex.almacen IN (%s))) \
						 u limit 1 ",
						 cad_conjunto_alma_ex, condicion_solo_activos, codigobarras,
						 condicion_solo_activos, codigobarras, cad_conjunto_alma_ex);
				else
					instruccion.sprintf("SELECT res.*, @articulo:=res.articulo, @producto:=res.producto, @present:=res.present, @multiplo:=res.multiplo FROM(\
						select * from \
							(\
								(\
									select a.*, (ex.cantidad/factor) AS existencias\
									from articulos a \
									INNER JOIN existenciasactuales ex ON ex.producto = a.producto AND ex.present = a.present AND ex.almacen IN (%s) \
									where %s a.ean13='%s'\
								) \
									union \
								(\
									select a.*, (ex.cantidad/factor) AS existencias\
									from articulos a, codigosbarras cb, existenciasactuales ex \
									where %s a.articulo=cb.articulo and cb.codigobarras='%s' \
									AND ex.producto = a.producto AND ex.present = a.present AND ex.almacen IN (%s)\
								)\
							) u limit 1) res\
						INNER JOIN empresas e \
						LEFT JOIN articuloempresacfg cfg ON cfg.articulo=res.articulo AND e.idempresa = cfg.idempresa\
						WHERE e.idempresa=%s\
						AND (cfg.%s = 1 OR isnull(cfg.%s))",
						cad_conjunto_alma_ex, condicion_solo_activos, codigobarras,
						condicion_solo_activos, codigobarras, cad_conjunto_alma_ex, empresa, permiso_accion, permiso_accion);
			} else {
				if (present==" ") {
					// Si no hay presentacion, entonces toma cualquier articulo de ese producto, verificando si contiene el permiso necesario:
					if(permiso_accion == "0")
						instruccion.sprintf("select a.*, @articulo:=a.articulo, @producto:=a.producto, @present:=a.present, @multiplo:=a.multiplo, TRUNCATE((ex.cantidad/factor),3) AS existencias \
						from articulos a \
						INNER JOIN existenciasactuales ex ON ex.producto = a.producto AND ex.present = a.present AND ex.almacen IN (%s) \
						where %s a.producto='%s' order by a.present, a.factor desc limit 1", cad_conjunto_alma_ex, condicion_solo_activos, producto);
					else
						instruccion.sprintf("SELECT mul.*, @articulo:=articulo, @producto:=producto , @present:=present , @multiplo:=multiplo , existencias, permiso FROM \
						( \
							select a.*, TRUNCATE((ex.cantidad/factor),0) AS existencias, ifnull(cfg.%s,1) AS permiso \
							from articulos a \
							INNER JOIN existenciasactuales ex ON ex.producto = a.producto AND ex.present = a.present AND ex.almacen IN (%s) \
							INNER JOIN empresas e \
							LEFT JOIN articuloempresacfg cfg ON a.articulo = cfg.articulo AND e.idempresa = cfg.idempresa \
							WHERE %s \
							a.producto='%s' \
							AND e.idempresa = %s \
							ORDER BY a.present, a.factor DESC \
						) mul %s LIMIT 1", permiso_accion, cad_conjunto_alma_ex, condicion_solo_activos, producto, empresa, condicion_accion);
				} else {
					if (multiplo==" ") {
						if(permiso_accion=="0"){
							// Si no hay multiplo entonces toma cualquier articulo de ese producto y presentacion
							instruccion.sprintf("select a.*, @articulo:=a.articulo, @producto:=a.producto, @present:=a.present, @multiplo:=a.multiplo, TRUNCATE((ex.cantidad/factor),3) AS existencias \
							from articulos a \
							INNER JOIN existenciasactuales ex ON ex.producto = a.producto AND ex.present = a.present AND ex.almacen IN (%s) where %s a.producto='%s' and a.present='%s' GROUP BY a.articulo order by a.factor desc limit 1", cad_conjunto_alma_ex, condicion_solo_activos, producto, present);
						} else {
							// Si no hay multiplo entonces toma cualquier articulo de ese producto y presentacion
							instruccion.sprintf("SELECT mul.*,  @articulo:=mul.articulo, @producto:=mul.producto, @present:=mul.present, @multiplo:=mul.multiplo FROM( \
								select a.*, TRUNCATE((ex.cantidad/factor),0) AS existencias, ifnull(cfg.%s, 1) AS permiso \
								from articulos a \
								INNER JOIN existenciasactuales ex ON ex.producto = a.producto AND ex.present = a.present AND ex.almacen IN (%s) \
								INNER JOIN empresas e \
								LEFT JOIN articuloempresacfg cfg ON a.articulo = cfg.articulo AND e.idempresa = cfg.idempresa \
								where %s a.producto='%s' and a.present='%s' AND e.idempresa = '%s' GROUP BY a.articulo order by a.factor desc \
							) mul %s limit 1",
							permiso_accion, cad_conjunto_alma_ex, condicion_solo_activos, producto, present, empresa, condicion_accion);
						}
					} else {
						instruccion.sprintf("select a.*, @articulo:=a.articulo, @producto:=a.producto, @present:=a.present, @multiplo:=a.multiplo, TRUNCATE((ex.cantidad/factor),3) AS existencias \
						from articulos a \
						INNER JOIN existenciasactuales ex ON ex.producto = a.producto AND ex.present = a.present AND ex.almacen IN (%s) where %s a.producto='%s' and a.present='%s' and a.multiplo='%s' limit 1", cad_conjunto_alma_ex, condicion_solo_activos, producto, present, multiplo);
					}
				}
			}
		} else {
			// Busca el artìculo
			instruccion.sprintf("select a.*, @articulo:=a.articulo, @producto:=a.producto, @present:=a.present, @multiplo:=a.multiplo, TRUNCATE((ex.cantidad/factor),3) AS existencias \
			from articulos a INNER JOIN existenciasactuales ex ON ex.producto = a.producto AND ex.present = a.present AND ex.almacen IN (%s) where %s a.articulo='%s' limit 1", cad_conjunto_alma_ex, condicion_solo_activos, articulo);
		}
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

		// Datos del producto
		instruccion.sprintf("select * from productos where @producto=producto");
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

		// Datos de la presentacion
		instruccion.sprintf("select ps.*, pcb.costobase, pcb.costoultimo from presentaciones ps \
			INNER JOIN presentacionescb pcb ON pcb.producto = ps.producto AND pcb.present = ps.present  \
			WHERE @producto=ps.producto and @present=ps.present and pcb.idempresa=%s ", empresa);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

        if(permiso_accion !="0"){
			// Todas las presentaciones que estén activas y permitan realizar una ACCION en la empresa correspondiente
			instruccion.sprintf("SELECT res.present FROM \
				( \
					select DISTINCT p.present, IFNULL(cfg.%s, 1) AS permiso \
					FROM presentaciones p \
					INNER JOIN articulos a ON a.producto=p.producto AND a.present=p.present \
					LEFT JOIN articuloempresacfg cfg ON a.articulo = cfg.articulo AND cfg.idempresa = %s \
					WHERE p.producto=@producto %s \
					HAVING permiso = 1 \
				) res",permiso_accion, empresa, condicion_solo_activosB );
		}
		else {
			// Todas las presentaciones del producto.
			instruccion.sprintf("select distinct p.present from presentaciones p, articulos a where p.producto=@producto and a.producto=p.producto and a.present=p.present %s ", condicion_solo_activosB);
		}
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

		if(permiso_accion !="0"){
			// Todos los multiplos de la presentacion que estén activos y permitan realizar una ACCION en la empresa correspondiente
			instruccion.sprintf("SELECT DISTINCT multiplo FROM( \
				SELECT ifnull(cfg.%s, 1) AS permiso, e.idempresa, a.multiplo \
				FROM articulos a \
				INNER JOIN empresas e \
				LEFT JOIN articuloempresacfg cfg ON a.articulo=cfg.articulo AND e.idempresa = cfg.idempresa \
				WHERE a.producto=@producto\
				AND a.present = @present\
				AND e.idempresa=%s %s\
			) as mul %s ", permiso_accion, empresa, condicion_solo_activosB,  condicion_accion);
		}
		else {
			// Todos los multiplos de la presentacion.
			instruccion.sprintf("select distinct a.multiplo from articulos a where a.producto=@producto and a.present=@present %s ", condicion_solo_activosB);
		}
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);


		// Todos los precios del articulo.
		instruccion.sprintf(" select p.tipoprec, t.descripcion, \
						ROUND(IF( ISNULL(aut.fechaVigencia) OR ISNULL(pl.precio), p.precio, pl.precio ),IFNULL(pd.valor,6)) AS precio, \
						 t.verventmayoreo \
							from tiposdeprecios t \
							  INNER JOIN precios p \
							  INNER JOIN articulos a ON p.articulo=a.articulo \
							  LEFT JOIN preciolocal pl \
								ON pl.articulo = a.articulo \
								AND pl.tipoprec = t.tipoprec \
								AND pl.sucursal='%s' \
							  LEFT JOIN autorizarprecios aut \
								ON aut.sucursal = '%s' \
								AND aut.producto = a.producto \
								AND aut.present = a.present \
								AND aut.fechavigencia >= '%s' \
							LEFT JOIN parametrosemp AS pd ON pd.parametro='DIGREDOND' AND pd.sucursal = '%s' \
						   where p.tipoprec=t.tipoprec and p.articulo=@articulo and t.idempresa=%s %s \
						   order by p.precio desc, p.tipoprec desc",
						   sucursal_consulta,
						   sucursal_consulta,
						   mFg.DateToMySqlDate(fecha),
						   sucursal_consulta, empresa,
						   condicion_considerar_verventmayoreo );
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

		bool ExcluirIEPSproducto=false;

		// Si está activo el parámetro de excluir IEPS (en supers normalmente es donde está activo)
		if ( FormServidor->ObtieneActivoExcluirIEPS()) {
			BufferRespuestas* resp_paramexcluirIEPS=NULL;
			try {
				// busca si el producto está en la lista de excluir IEPS y el primer impuesto es IEPS, entonces activamos la bandera para ignorar este primer impuesto
				instruccion.sprintf("SELECT i1.tipoimpu FROM excluirproductosiesps e "
					"INNER JOIN productos p ON e.producto=p.producto "
					"INNER join impuestos i1 on i1.impuesto=p.claveimpc1 AND i1.tipoimpu='IESPS' "
					"WHERE e.producto=@producto ");
				if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_paramexcluirIEPS)) {
					if (resp_paramexcluirIEPS->ObtieneNumRegistros()>0){
						ExcluirIEPSproducto=true;
					} else {
						ExcluirIEPSproducto=false;
					}
				} else throw (Exception("Error al consultar en tabla parametros"));
			} __finally {
				if (resp_paramexcluirIEPS!=NULL) delete resp_paramexcluirIEPS;
			}
		}



		// Los impuestos y costos del artìculo.
		if (tipo=="0") {
			// COMPRAS
			if (ExcluirIEPSproducto) {
				mServidorVioleta->MuestraMensaje("ID_QRY_FRAME_ART para compras EXCLUYENDO IEPS");

				//se obtienen los codigos de los impuestos de compra
				instruccion.sprintf("select "
					"p.claveimpc2 AS claveimpc1, i1.tipoimpu as tipoimpuesto1, i1.porcentaje as porcentajeimp1, t1.nombre as nomtipoimp1, "
					"p.claveimpc3 AS claveimpc2, i2.tipoimpu as tipoimpuesto2, i2.porcentaje as porcentajeimp2, t2.nombre as nomtipoimp2, "
					"p.claveimpc4 AS claveimpc3, i3.tipoimpu as tipoimpuesto3, i3.porcentaje as porcentajeimp3, t3.nombre as nomtipoimp3, "
					"0 AS claveimpc4, i4.tipoimpu AS tipoimpuesto4, i4.porcentaje AS porcentajeimp4, t4.nombre AS nomtipoimp4, "
					"pre.iepscuota "
					"from productos p "
					"inner join presentaciones pre on pre.producto=@producto and pre.present=@present "
					"left join impuestos i1 on i1.impuesto=p.claveimpc2 "
					"left join impuestos i2 on i2.impuesto=p.claveimpc3 "
					"left join impuestos i3 on i3.impuesto=p.claveimpc4 "
					"LEFT JOIN impuestos i4 ON i4.impuesto=0 "
					"left join tiposdeimpuestos t1 on t1.tipoimpu=i1.tipoimpu "
					"left join tiposdeimpuestos t2 on t2.tipoimpu=i2.tipoimpu "
					"left join tiposdeimpuestos t3 on t3.tipoimpu=i3.tipoimpu "
					"LEFT JOIN tiposdeimpuestos t4 ON t4.tipoimpu=i4.tipoimpu "
					"where p.producto=@producto");
			} else {
				mServidorVioleta->MuestraMensaje("ID_QRY_FRAME_ART para compras sin excluir IEPS");

				//se obtienen los codigos de los impuestos de compra
				instruccion.sprintf("select \
					p.claveimpc1, i1.tipoimpu as tipoimpuesto1, i1.porcentaje as porcentajeimp1, t1.nombre as nomtipoimp1, \
					p.claveimpc2, i2.tipoimpu as tipoimpuesto2, i2.porcentaje as porcentajeimp2, t2.nombre as nomtipoimp2, \
					p.claveimpc3, i3.tipoimpu as tipoimpuesto3, i3.porcentaje as porcentajeimp3, t3.nombre as nomtipoimp3, \
					p.claveimpc4, i4.tipoimpu as tipoimpuesto4, i4.porcentaje as porcentajeimp4, t4.nombre as nomtipoimp4, \
					pre.iepscuota \
					from productos p \
					inner join presentaciones pre on pre.producto=@producto and pre.present=@present \
					left join impuestos i1 on i1.impuesto=p.claveimpc1 \
					left join impuestos i2 on i2.impuesto=p.claveimpc2 \
					left join impuestos i3 on i3.impuesto=p.claveimpc3 \
					left join impuestos i4 on i4.impuesto=p.claveimpc4 \
					left join tiposdeimpuestos t1 on t1.tipoimpu=i1.tipoimpu \
					left join tiposdeimpuestos t2 on t2.tipoimpu=i2.tipoimpu \
					left join tiposdeimpuestos t3 on t3.tipoimpu=i3.tipoimpu \
					left join tiposdeimpuestos t4 on t4.tipoimpu=i4.tipoimpu \
					where p.producto=@producto");
			}
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
		if (tipo=="1") {
			// VENTAS
			if (ExcluirIEPSproducto) {
				mServidorVioleta->MuestraMensaje("ID_QRY_FRAME_ART para ventas EXCLUYENDO IEPS");
				//se obtienen los codigos de los impuestos de venta
				instruccion.sprintf("select "
					"p.claveimpv2 AS claveimpv1, i1.tipoimpu as tipoimpuesto1, i1.porcentaje as porcentajeimp1, t1.nombre as nomtipoimp1, "
					"p.claveimpv3 AS claveimpv2, i2.tipoimpu as tipoimpuesto2, i2.porcentaje as porcentajeimp2, t2.nombre as nomtipoimp2, "
					"p.claveimpv4 AS claveimpv3, i3.tipoimpu as tipoimpuesto3, i3.porcentaje as porcentajeimp3, t3.nombre as nomtipoimp3, "
					"0 AS claveimpv4, i4.tipoimpu AS tipoimpuesto4, i4.porcentaje AS porcentajeimp4, t4.nombre AS nomtipoimp4,  "
					"0.0 as iepscuota "
					"from productos p "
					"left join impuestos i1 on i1.impuesto=p.claveimpv2 "
					"left join impuestos i2 on i2.impuesto=p.claveimpv3 "
					"left join impuestos i3 on i3.impuesto=p.claveimpv4 "
					"LEFT JOIN impuestos i4 ON i4.impuesto=0 "
					"left join tiposdeimpuestos t1 on t1.tipoimpu=i1.tipoimpu "
					"left join tiposdeimpuestos t2 on t2.tipoimpu=i2.tipoimpu "
					"left join tiposdeimpuestos t3 on t3.tipoimpu=i3.tipoimpu "
					"LEFT JOIN tiposdeimpuestos t4 ON t4.tipoimpu=i4.tipoimpu "
					"where p.producto=@producto");
			} else {
				mServidorVioleta->MuestraMensaje("ID_QRY_FRAME_ART para ventas sin excluir IEPS");
				//se obtienen los codigos de los impuestos de venta
				instruccion.sprintf("select \
					p.claveimpv1, i1.tipoimpu as tipoimpuesto1, i1.porcentaje as porcentajeimp1, t1.nombre as nomtipoimp1, \
					p.claveimpv2, i2.tipoimpu as tipoimpuesto2, i2.porcentaje as porcentajeimp2, t2.nombre as nomtipoimp2, \
					p.claveimpv3, i3.tipoimpu as tipoimpuesto3, i3.porcentaje as porcentajeimp3, t3.nombre as nomtipoimp3, \
					p.claveimpv4, i4.tipoimpu as tipoimpuesto4, i4.porcentaje as porcentajeimp4, t4.nombre as nomtipoimp4, \
					0.0 as iepscuota \
					from productos p \
					left join impuestos i1 on i1.impuesto=p.claveimpv1 \
					left join impuestos i2 on i2.impuesto=p.claveimpv2 \
					left join impuestos i3 on i3.impuesto=p.claveimpv3 \
					left join impuestos i4 on i4.impuesto=p.claveimpv4 \
					left join tiposdeimpuestos t1 on t1.tipoimpu=i1.tipoimpu \
					left join tiposdeimpuestos t2 on t2.tipoimpu=i2.tipoimpu \
					left join tiposdeimpuestos t3 on t3.tipoimpu=i3.tipoimpu \
					left join tiposdeimpuestos t4 on t4.tipoimpu=i4.tipoimpu \
					where p.producto=@producto");
			}
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}

		// CALCULA LAS EXISTENCIAS
		if (calcular_existencias)
		{
			if(modo_calcular_existencias == "MODN"){
                throw Exception("Configuración de modo de calculo para revisión de existencias está incorrecto MODN");
			}
			else
				if(modo_calcular_existencias == "MODEA"){

					//AQUI EMPEZAMOS
					// Crea una tabla donde se pondrán las existencias que se vayan encontrando.
					instruccion="create temporary table articulosaux (articulo varchar(9) NOT NULL, producto varchar(8) NOT NULL, present \
						varchar(255) NOT NULL, multiplo varchar(10) NOT NULL, factor DECIMAL(10,3) NULL, activo tinyint(1) unsigned NOT NULL) ENGINE = MEMORY";
					if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
						throw (Exception("Error en query EjecutaSelectSqlNulo"));

					// Para forzar que al menos los productos activos aparescan aunque no tengan movimientos.
					instruccion.sprintf("insert into articulosaux (articulo, producto, present, multiplo, factor, activo) \
						select articulo, producto, present, multiplo, factor, activo from articulos \
						where producto=@producto and present=@present");
					if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
						throw (Exception("Error en query EjecutaSelectSqlNulo"));

					if (cad_conjunto_almacenes!=" ") {
						condicion_almacen.sprintf(" and almacen in (%s) ", cad_conjunto_almacenes);
					} else condicion_almacen=" ";
					//Total de existencias por multiplo
					instruccion.sprintf("SELECT SUM(cantidad) as cantidad FROM existenciasactuales WHERE producto = \
					@producto AND present = @present %s",condicion_almacen);
					mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

					// Posibles multiplos del producto-presentacion
					instruccion.sprintf("select multiplo, factor from articulosaux a where a.producto=@producto and a.present=@present \
										%s order by a.factor desc",condicion_solo_activosB);
					mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

				}
		}
	} __finally {
		if (resp_almacenes!=NULL) delete resp_almacenes;
		if (resp_almacenes_ex!=NULL) delete resp_almacenes_ex;
	}
}


//---------------------------------------------------------------------------
//ID_GRA_TRANSFORMACIONALM
void ServidorAlmacen::GrabaTrasformacionAlmacen(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  GRABA UNA TRANSFORMACION DE ALMACEN
	char *buffer_sql=new char[1024*64*10];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString tarea, clave, usuario;
	int num_partidas, i;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[2000];
	AnsiString articulo, almacen, foliofisic, terminal;
	TDateTime fecha_trans;
	int error=0;
	TDate fecha=Today();
	TTime hora=Time();

	try{
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave de la transformacion
		tarea=mFg.ExtraeStringDeBuffer(&parametros); // Indicador si se va a agregar o modificar.
		usuario=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que esta grabando
		terminal=mFg.ExtraeStringDeBuffer(&parametros); // terminal donde graba

		// Obtiene los datos de la tabla de movimientos de almacén
		datos.AsignaTabla("transformacion");
		parametros+=datos.InsCamposDesdeBuffer(parametros);
		// Extrae algunos datos necesarios para calcular otros
		fecha_trans=StrToDate(datos.ObtieneValorCampo("fechatrans"));
		almacen=datos.ObtieneValorCampo("almacen");
		foliofisic=datos.ObtieneValorCampo("foliofisic");

		// Verificaciones para no permitir modificaciones cuando no sea permitido.
		if (tarea=="M") {
			// Verifica que la fecha de la transformacion sea posterior a la fecha de cierre.
			instruccion.sprintf("select @error:=if(t.fechatrans<=cast(e.valor as datetime), 1, 0) as error from transformacion t left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' where t.transfor='%s'",FormServidor->ObtieneClaveSucursal(), clave);
			mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 3, error);
		}
		// Verifica que la fecha de la transformación sea posterior a la fecha de cierre.
		instruccion.sprintf("select @error:=if('%s'<=cast(e.valor as datetime), 1, 0) as error from estadosistemaemp e where e.estado='FUCIERRE' AND e.sucursal = '%s' ", mFg.DateToMySqlDate(fecha_trans), FormServidor->ObtieneClaveSucursal());
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 4, error);

        AnsiString produc2,present2,multiplo2,factor2,select_inactivos,nombre;
		BufferRespuestas* resp_inactivos=NULL;
		try{
			select_inactivos.sprintf("SELECT  @error:=IF(a.activo=0, 1, 0) AS error, \
				a.producto, a.present, a.multiplo, a.factor, pro.nombre \
				FROM movalma m LEFT JOIN dmovalma dm ON dm.movimiento=m.movimiento  \
				LEFT JOIN articulos a ON a.articulo=dm.articulo \
				LEFT JOIN productos pro ON pro.producto=a.producto \
				WHERE m.transfor='%s' AND a.activo=0",clave );
			if(mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, select_inactivos.c_str(), resp_inactivos)){
				if (resp_inactivos->ObtieneNumRegistros()>0){
					produc2=resp_inactivos->ObtieneDato("producto");
					present2=resp_inactivos->ObtieneDato("present");
					multiplo2=resp_inactivos->ObtieneDato("multiplo");
					factor2=resp_inactivos->ObtieneDato("factor");
					nombre = resp_inactivos->ObtieneDato("nombre");
					error=5;
					throw (Exception("Hay artículos inactivos en el detalle de la transformación a modificar.\n Favor de activar el artículo.\n  Artículo:"+nombre+" "+present2+" "+multiplo2));
				}
			}else{
				throw (Exception("Error al consultar los artículos inactivos en modificar movimientos de almacen"));
			}
	   /*	,a.producto, a.present, a.multiplo, a.factor   */
	   }__finally{
			if (resp_inactivos!=NULL) delete resp_inactivos;
	   }

		if (error==0) {
			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

			// Obtiene los folio para el la transformación y los movimientos necesarios
			if (tarea=="A") {
				// Obtiene la clave para la transformación
				instruccion.sprintf("select @folioaux:=valor from foliosemp where folio='TRANSFALMA' AND sucursal = '%s' %s ",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliosig=@folioaux+1");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @folioaux=cast(@folioaux as char)");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @folio=concat('%s', lpad(@folioaux,9,'0'))", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("update foliosemp set valor=@foliosig where folio='TRANSFALMA' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;

				// Obtiene la clave para la salida
				instruccion.sprintf("select @folioaux:=valor from foliosemp where folio='MOVALMA' AND sucursal = '%s' %s ",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliosig=@folioaux+1");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @folioaux=cast(@folioaux as char)");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliomovsal=concat('%s', lpad(@folioaux,9,'0'))", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("update foliosemp set valor=@foliosig where folio='MOVALMA' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;

				// Obtiene la clave para la entrada
				instruccion.sprintf("select @folioaux:=valor from foliosemp where folio='MOVALMA' AND sucursal = '%s' %s ",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliosig=@folioaux+1");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @folioaux=cast(@folioaux as char)");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliomovent=concat('%s', lpad(@folioaux,9,'0'))", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("update foliosemp set valor=@foliosig where folio='MOVALMA' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;

			} else {
				instruccion.sprintf("set @folio='%s'", clave);
				instrucciones[num_instrucciones++]=instruccion;

				instruccion.sprintf("select @foliomovsal:=movimiento from movalma where transfor='%s' and tipo='S'", clave);
				instrucciones[num_instrucciones++]=instruccion;


				instruccion.sprintf("select @foliomovent:=movimiento from movalma where transfor='%s' and tipo='E'", clave);
				instrucciones[num_instrucciones++]=instruccion;


			}



			// Si se está modificando entonces borra el detalle que ya exista.
			if (tarea=="M") {

				/* Crea tabla temporal para almacenar cantidades de articulos antes de
				/////////////////////////ACTUALIZAR EXISTENCIAS//////////////////////*/
				instruccion="CREATE temporary TABLE tmpcantidadm(referencia VARCHAR(11) NOT NULL,\
				producto VARCHAR(8) NOT NULL, present VARCHAR(255) NOT NULL, almacen VARCHAR(4) NOT NULL,  \
				cantidad DECIMAL(12,3) NOT NULL, PRIMARY KEY (referencia, producto, present, almacen))";
				instrucciones[num_instrucciones++]=instruccion;

				instruccion="CREATE temporary TABLE tmpcantidadm2(referencia VARCHAR(11) NOT NULL,\
				producto VARCHAR(8) NOT NULL, present VARCHAR(255) NOT NULL, almacen VARCHAR(4) NOT NULL,  \
				cantidad DECIMAL(12,3) NOT NULL, PRIMARY KEY (referencia, producto, present, almacen))";
				instrucciones[num_instrucciones++]=instruccion;

				instruccion = "INSERT INTO tmpcantidadm SELECT d.movimiento, a.producto, a.present, m.almasal, \
				SUM(d.cantidad * a.factor) AS cantidad FROM dmovalma d INNER JOIN movalma m ON   \
				d.movimiento = m.movimiento INNER JOIN articulos a ON a.articulo = d.articulo \
				WHERE d.movimiento = @foliomovsal GROUP BY a.producto, a.present";
				instrucciones[num_instrucciones++]=instruccion;

				instruccion = "UPDATE tmpcantidadm tmp INNER JOIN existenciasactuales ea ON ea.producto = tmp.producto \
				AND ea.present = tmp.present AND ea.almacen = tmp.almacen SET ea.cantidad = (ea.cantidad  + tmp.cantidad) \
				, ea.salidas = (ea.salidas - tmp.cantidad)";
				instrucciones[num_instrucciones++]=instruccion;

				instruccion = "INSERT INTO tmpcantidadm2 SELECT d.movimiento, a.producto, a.present, m.almaent, \
				SUM(d.cantidad * a.factor) AS cantidad FROM dmovalma d INNER JOIN movalma m ON   \
				d.movimiento = m.movimiento INNER JOIN articulos a ON a.articulo = d.articulo \
				WHERE d.movimiento = @foliomovent GROUP BY a.producto, a.present";
				instrucciones[num_instrucciones++]=instruccion;

				instruccion = "UPDATE tmpcantidadm2 tmp INNER JOIN existenciasactuales ea ON ea.producto = tmp.producto \
				AND ea.present = tmp.present AND ea.almacen = tmp.almacen SET ea.cantidad = (ea.cantidad  - tmp.cantidad) \
				, ea.entradas = (ea.entradas - tmp.cantidad) ";
				instrucciones[num_instrucciones++]=instruccion;

				instruccion.sprintf("delete from dmovalma where movimiento=@foliomovsal or movimiento=@foliomovent");
				instrucciones[num_instrucciones++]=instruccion;

				instruccion.sprintf("UPDATE transformacion SET auditado = 0 where transfor='%s' ", clave);
				instrucciones[num_instrucciones++]=instruccion;
			}

			// Graba la cabecera en la tabla "transformacion"
			if (tarea=="A") {
				datos.InsCampo("transfor", "@folio", 1);
				datos.InsCampo("usualta", usuario);
				datos.InsCampo("usumodi", usuario);
				datos.InsCampo("fechaalta", mFg.DateToAnsiString(fecha));
				datos.InsCampo("horaalta", mFg.TimeToAnsiString(hora));
				datos.InsCampo("fechamodi", mFg.DateToAnsiString(fecha));
				datos.InsCampo("horamodi", mFg.TimeToAnsiString(hora));
				datos.InsCampo("cancelado", "0");
				instrucciones[num_instrucciones++]=datos.GenerarSqlInsert();
			} else {
				datos.InsCampo("usumodi", usuario);
				datos.InsCampo("fechamodi", mFg.DateToAnsiString(fecha));
				datos.InsCampo("horamodi", mFg.TimeToAnsiString(hora));
				instrucciones[num_instrucciones++]=datos.GenerarSqlUpdate("transfor=@folio");
			}


			// Graba la cabecera en la tabla "movalma" para la SALIDA
			datos.AsignaTabla("movalma");
			datos.InsCampo("tipo", "S");
			datos.InsCampo("almasal", almacen);
			datos.InsCampo("concepto", "TRSA");
			datos.InsCampo("folioenvio", foliofisic);
			datos.InsCampo("fechamov", mFg.DateToAnsiString(fecha_trans));
			datos.InsCampo("terminal", terminal);
			if (tarea=="A") {
				datos.InsCampo("movimiento", "@foliomovsal", 1);
				datos.InsCampo("transfor", "@folio", 1);
				datos.InsCampo("usualta", usuario);
				datos.InsCampo("usumodi", usuario);
				datos.InsCampo("fechaalta", mFg.DateToAnsiString(fecha));
				datos.InsCampo("horaalta", mFg.TimeToAnsiString(hora));
				datos.InsCampo("fechamodi", mFg.DateToAnsiString(fecha));
				datos.InsCampo("horamodi", mFg.TimeToAnsiString(hora));
				datos.InsCampo("cancelado", "0");
				instrucciones[num_instrucciones++]=datos.GenerarSqlInsert();
			} else {
				datos.InsCampo("usumodi", usuario);
				datos.InsCampo("fechamodi", mFg.DateToAnsiString(fecha));
				datos.InsCampo("horamodi", mFg.TimeToAnsiString(hora));
				instrucciones[num_instrucciones++]=datos.GenerarSqlUpdate("movimiento=@foliomovsal");
			}

			// Graba la cabecera en la tabla "movalma" para la ENTRADA
			datos.AsignaTabla("movalma");
			datos.InsCampo("tipo", "E");
			datos.InsCampo("almaent", almacen);
			datos.InsCampo("concepto", "TREN");
			datos.InsCampo("folioenvio", foliofisic);
			datos.InsCampo("fechamov", mFg.DateToAnsiString(fecha_trans));
            datos.InsCampo("terminal", terminal);
			if (tarea=="A") {
				datos.InsCampo("movimiento", "@foliomovent", 1);
				datos.InsCampo("transfor", "@folio", 1);
				datos.InsCampo("usualta", usuario);
				datos.InsCampo("usumodi", usuario);
				datos.InsCampo("fechaalta", mFg.DateToAnsiString(fecha));
				datos.InsCampo("horaalta", mFg.TimeToAnsiString(hora));
				datos.InsCampo("fechamodi", mFg.DateToAnsiString(fecha));
				datos.InsCampo("horamodi", mFg.TimeToAnsiString(hora));
				datos.InsCampo("cancelado", "0");
				instrucciones[num_instrucciones++]=datos.GenerarSqlInsert();
			} else {
				datos.InsCampo("usumodi", usuario);
				datos.InsCampo("fechamodi", mFg.DateToAnsiString(fecha));
				datos.InsCampo("horamodi", mFg.TimeToAnsiString(hora));
				instrucciones[num_instrucciones++]=datos.GenerarSqlUpdate("movimiento=@foliomovent");
			}



			// Graba las partidas en "dmovalma" para SALIDAS
			num_partidas=StrToInt(mFg.ExtraeStringDeBuffer(&parametros)); // Obtiene el número de partidas
			for (i=0; i<num_partidas; i++) {
				datos.AsignaTabla("dmovalma");
				parametros+=datos.InsCamposDesdeBuffer(parametros);
				datos.InsCampo("movimiento", "@foliomovsal", 1);
				instrucciones[num_instrucciones++]=datos.GenerarSqlInsert();
			}

			//se actualiza existenciasactuales desde subconsulta agrupada por  producto, presentación  y almacen: para SALIDAS
			instruccion.sprintf("UPDATE existenciasactuales ea, \
			(SELECT  sum(d.cantidad * a.factor) as sumCantidad, a.producto , a.present, m.almasal \
			FROM  movalma m \
			INNER JOIN dmovalma d ON m.movimiento = d.movimiento \
			INNER JOIN articulos a on a.articulo = d.articulo \
			WHERE m.movimiento = @foliomovsal \
			GROUP BY a.producto, a.present, m.almasal) as tgs \
			SET ea.cantidad = ea.cantidad- tgs.sumCantidad , ea.salidas =ea.salidas+ tgs.sumCantidad \
			WHERE ea.producto=tgs.producto AND ea.present=tgs.present AND ea.almacen=tgs.almasal");
			instrucciones[num_instrucciones++]=instruccion;

			// Graba las partidas en "dmovalma" para ENTRADAS
			num_partidas=StrToInt(mFg.ExtraeStringDeBuffer(&parametros)); // Obtiene el número de partidas
			for (i=0; i<num_partidas; i++) {
				datos.AsignaTabla("dmovalma");
				parametros+=datos.InsCamposDesdeBuffer(parametros);
				datos.InsCampo("movimiento", "@foliomovent", 1);
				instrucciones[num_instrucciones++]=datos.GenerarSqlInsert();
			}

			//se actualiza existenciasactuales desde subconsulta agrupada por  producto, presentación  y almacen: para ENTRADAS
			instruccion.sprintf("UPDATE existenciasactuales ea, \
			(SELECT  sum(d.cantidad * a.factor) as sumCantidad, a.producto , a.present, m.almaent \
			FROM  movalma m \
			INNER JOIN dmovalma d ON m.movimiento = d.movimiento \
			INNER JOIN articulos a on a.articulo = d.articulo \
			WHERE m.movimiento = @foliomovent \
			group by a.producto, a.present, m.almaent) as tgent \
			set ea.cantidad = ea.cantidad + tgent.sumCantidad , ea.entradas =ea.entradas+tgent.sumCantidad \
			where ea.producto=tgent.producto and ea.present=tgent.present and ea.almacen=tgent.almaent");
			instrucciones[num_instrucciones++]=instruccion;


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

//---------------------------------------------------------------------------
//ID_CANC_TRANSFORMACIONALM
void ServidorAlmacen::CancelaTrasformacionAlmacen(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  CANCELA UNA TRANSFORMACION DE PRODUCTOS
	char *buffer_sql=new char[1024*32];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instrucciones[50], instruccion;
	int num_instrucciones=0;
	AnsiString clave;
	int i;
	int error=0;

	try{
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave de la transformación

		// Verifica que la fecha de la transformación sea posterior a la fecha de cierre.
		instruccion.sprintf("select @error:=if(t.fechatrans<=cast(e.valor as datetime), 1, 0) as error from transformacion t left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' where t.transfor='%s'",FormServidor->ObtieneClaveSucursal(), clave);
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 3, error);

		AnsiString produc2,present2,multiplo2,factor2,select_inactivos,nombre;
		BufferRespuestas* resp_inactivos=NULL;
		try{
			select_inactivos.sprintf("SELECT  @error:=IF(a.activo=0, 1, 0) AS error, \
				a.producto, a.present, a.multiplo, a.factor, pro.nombre \
				FROM movalma m LEFT JOIN dmovalma dm ON dm.movimiento=m.movimiento  \
				LEFT JOIN articulos a ON a.articulo=dm.articulo \
				LEFT JOIN productos pro ON pro.producto=a.producto \
				WHERE m.transfor='%s' AND a.activo=0",clave );
			if(mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, select_inactivos.c_str(), resp_inactivos)){
				if (resp_inactivos->ObtieneNumRegistros()>0){
					produc2=resp_inactivos->ObtieneDato("producto");
					present2=resp_inactivos->ObtieneDato("present");
					multiplo2=resp_inactivos->ObtieneDato("multiplo");
					factor2=resp_inactivos->ObtieneDato("factor");
					nombre = resp_inactivos->ObtieneDato("nombre");
					error=4;
					throw (Exception("Hay artículos inactivos en el detalle de la tranformación a cancelar.\n Favor de activar el artículo.\n  Artículo:"+nombre+" "+present2+" "+multiplo2));
				}
			}else{
				throw (Exception("Error al consultar los artículos inactivos en cancelar movimientos de almacen"));
			}
	   /*	,a.producto, a.present, a.multiplo, a.factor   */
	   }__finally{
			if (resp_inactivos!=NULL) delete resp_inactivos;
	   }

		if (error==0) {
			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

            instruccion.sprintf("set @folio='%s'", clave);
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("select @foliomovsal:=movimiento from movalma where transfor='%s' and tipo='S'", clave);
			instrucciones[num_instrucciones++]=instruccion;


			instruccion.sprintf("select @foliomovent:=movimiento from movalma where transfor='%s' and tipo='E'", clave);
			instrucciones[num_instrucciones++]=instruccion;

			/* Crea tabla temporal para almacenar cantidades de articulos antes de
			/////////////////////////ACTUALIZAR EXISTENCIAS//////////////////////*/
			instruccion="CREATE temporary TABLE tmpcantidadm(referencia VARCHAR(11) NOT NULL,\
			producto VARCHAR(8) NOT NULL, present VARCHAR(255) NOT NULL, almacen VARCHAR(4) NOT NULL,  \
			cantidad DECIMAL(12,3) NOT NULL, PRIMARY KEY (referencia, producto, present, almacen))";
			instrucciones[num_instrucciones++]=instruccion;

			instruccion="CREATE temporary TABLE tmpcantidadm2(referencia VARCHAR(11) NOT NULL,\
			producto VARCHAR(8) NOT NULL, present VARCHAR(255) NOT NULL, almacen VARCHAR(4) NOT NULL,  \
			cantidad DECIMAL(12,3) NOT NULL, PRIMARY KEY (referencia, producto, present, almacen))";
			instrucciones[num_instrucciones++]=instruccion;

			instruccion = "INSERT INTO tmpcantidadm SELECT d.movimiento, a.producto, a.present, m.almasal, \
			SUM(d.cantidad * a.factor) AS cantidad FROM dmovalma d INNER JOIN movalma m ON   \
			d.movimiento = m.movimiento INNER JOIN articulos a ON a.articulo = d.articulo \
			WHERE d.movimiento = @foliomovsal GROUP BY a.producto, a.present";
			instrucciones[num_instrucciones++]=instruccion;

			instruccion = "UPDATE tmpcantidadm tmp INNER JOIN existenciasactuales ea ON ea.producto = tmp.producto \
			AND ea.present = tmp.present AND ea.almacen = tmp.almacen SET ea.cantidad = (ea.cantidad  + tmp.cantidad) \
			, ea.salidas = (ea.salidas - tmp.cantidad)";
			instrucciones[num_instrucciones++]=instruccion;

			instruccion = "INSERT INTO tmpcantidadm2 SELECT d.movimiento, a.producto, a.present, m.almaent, \
			SUM(d.cantidad * a.factor) AS cantidad FROM dmovalma d INNER JOIN movalma m ON   \
			d.movimiento = m.movimiento INNER JOIN articulos a ON a.articulo = d.articulo \
			WHERE d.movimiento = @foliomovent GROUP BY a.producto, a.present";
			instrucciones[num_instrucciones++]=instruccion;

			instruccion = "UPDATE tmpcantidadm2 tmp INNER JOIN existenciasactuales ea ON ea.producto = tmp.producto \
			AND ea.present = tmp.present AND ea.almacen = tmp.almacen SET ea.cantidad = (ea.cantidad  - tmp.cantidad) \
			, ea.entradas = (ea.entradas - tmp.cantidad) ";
			instrucciones[num_instrucciones++]=instruccion;

			// Cancela la transformación
			instruccion.sprintf("update transformacion set cancelado=1 where transfor='%s' and cancelado=0", clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Cancela todos los movimientos que hacen referencia a la transformación
			instruccion.sprintf("update movalma set cancelado=1 where transfor='%s' and cancelado=0", clave);
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
//ID_CON_TRANSFORMACIONALM
void ServidorAlmacen::ConsultaTrasformacionAlmacen(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA UNA TRANSFORMACION DE PRODUCTO
	AnsiString instruccion;
	AnsiString clave;

	clave=mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los generales (de cabecera) de la transformacion
	instruccion.sprintf("SELECT t.*,al.nombre FROM transformacion t, almacenes al WHERE  t.almacen=al.almacen AND transfor='%s'", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

    // MODIFICAR:  Verifica que la fecha de la factura sea posterior a la fecha de cierre.
	instruccion.sprintf("select if(t.fechatrans>=cast(e.valor as datetime), 1, 0) as modificar from transformacion t left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' where t.transfor='%s'",FormServidor->ObtieneClaveSucursal(), clave);
    mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

    // Obtiene todo el detalle de las salidas
    instruccion.sprintf("select d.cantidad, a.multiplo, p.nombre, a.present, \
		a.producto, a.articulo \
        from movalma m, dmovalma d, articulos a, productos p \
        where m.transfor='%s' and m.tipo='S' and m.movimiento=d.movimiento and \
        d.articulo=a.articulo and a.producto=p.producto", clave);
    mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene todo el detalle de las entradas
	instruccion.sprintf("select d.cantidad, a.multiplo, p.nombre, a.present, \
        a.producto, a.articulo, TRUNCATE(d.costo,4) as costo, TRUNCATE(d.cantidad*d.costo,4) AS subtotal  \
        from movalma m, dmovalma d, articulos a, productos p \
        where m.transfor='%s' and m.tipo='E' and m.movimiento=d.movimiento and \
        d.articulo=a.articulo and a.producto=p.producto", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

}
//---------------------------------------------------------------------------
//ID_AJU_MOVALM
void ServidorAlmacen::GrabaAjustesMovimientoAlmacen(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    //  GRABA UN MOVIMIENTO DE ALMACEN
	char *buffer_sql=new char[1024*64*50];
	char *aux_buffer_sql=buffer_sql;
    DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString tarea, clave, usuario;
	int salidas, entradas;
	int error=0;
	int num_partidas, i;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[10000];
	TDateTime fecha_mov;
	TDate fecha=Today();
	TTime hora=Time();

	try{
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave del movimiento
		tarea=mFg.ExtraeStringDeBuffer(&parametros); // Indicador si se va a agregar o modificar.
		usuario=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que esta grabando el movimiento
		salidas=StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
		entradas=StrToInt(mFg.ExtraeStringDeBuffer(&parametros));

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		if (salidas!=0) {
			// Obtiene los datos de la tabla de movimientos de almacén
			datos.AsignaTabla("movalma");
			parametros+=datos.InsCamposDesdeBuffer(parametros);
			// Extrae algunos datos necesarios para calcular otros
			fecha_mov=StrToDate(datos.ObtieneValorCampo("fechamov"));
			// Verifica que la fecha del movimiento sea posterior a la fecha de cierre.
			instruccion.sprintf("select @error:=if('%s'<=cast(e.valor as datetime), 1, 0) as error from estadosistemaemp e where e.estado='FUCIERRE' AND e.sucursal = '%s' ", mFg.DateToMySqlDate(fecha_mov), FormServidor->ObtieneClaveSucursal());
			mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 4, error);
			if (error==0) {
				// Obtiene el folio para el movimiento
				instruccion.sprintf("select @folioaux1:=valor from foliosemp where folio='MOVALMA' AND sucursal = '%s' %s ",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliosig1=@folioaux1+1");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @folioaux1=cast(@folioaux1 as char)");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @folio1=concat('%s', lpad(@folioaux1,9,'0'))", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("update foliosemp set valor=@foliosig1 where folio='MOVALMA' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;

				// Graba la cabecera en la tabla "movalma"
				datos.InsCampo("movimiento", "@folio1", 1);
				datos.InsCampo("usualta", usuario);
				datos.InsCampo("usumodi", usuario);
				datos.InsCampo("fechaalta", mFg.DateToAnsiString(fecha));
				datos.InsCampo("horaalta", mFg.TimeToAnsiString(hora));
				datos.InsCampo("fechamodi", mFg.DateToAnsiString(fecha));
				datos.InsCampo("horamodi", mFg.TimeToAnsiString(hora));
				datos.InsCampo("cancelado", "0");
				instrucciones[num_instrucciones++]=datos.GenerarSqlInsert();

				// Graba las partidas en "dmovalma"
				num_partidas=salidas;
				for (i=0; i<num_partidas; i++) {
					datos.AsignaTabla("dmovalma");
					parametros+=datos.InsCamposDesdeBuffer(parametros);
					datos.InsCampo("movimiento", "@folio1", 1);
					instrucciones[num_instrucciones++]=datos.GenerarSqlInsert();
				}

                //se actualiza existenciasactuales desde subconsulta agrupada por  producto, presentación  y almacen: para SALIDAS
				instruccion.sprintf("UPDATE existenciasactuales ea, \
				(SELECT  sum(d.cantidad * a.factor) as sumCantidad, a.producto , a.present, m.almasal \
				FROM  movalma m \
				INNER JOIN dmovalma d ON m.movimiento = d.movimiento \
				INNER JOIN articulos a on a.articulo = d.articulo \
				WHERE m.movimiento = @folio1 \
				GROUP BY a.producto, a.present, m.almasal) as tgs \
				SET ea.cantidad = ea.cantidad- tgs.sumCantidad , ea.salidas =ea.salidas+ tgs.sumCantidad \
				WHERE ea.producto=tgs.producto AND ea.present=tgs.present AND ea.almacen=tgs.almasal");
				instrucciones[num_instrucciones++]=instruccion;

			}
		}

		if (entradas!=0 && error==0) {
			// Obtiene los datos de la tabla de movimientos de almacén
			datos.AsignaTabla("movalma");
			parametros+=datos.InsCamposDesdeBuffer(parametros);
			// Extrae algunos datos necesarios para calcular otros
			fecha_mov=StrToDate(datos.ObtieneValorCampo("fechamov"));
			// Verifica que la fecha del movimiento sea posterior a la fecha de cierre.
			instruccion.sprintf("select @error:=if('%s'<=cast(e.valor as datetime), 1, 0) as error from estadosistemaemp e where e.estado='FUCIERRE' AND e.sucursal = '%s' ", mFg.DateToMySqlDate(fecha_mov), FormServidor->ObtieneClaveSucursal());
			mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 4, error);
			if (error==0) {
				// Obtiene el folio para el movimiento
				instruccion.sprintf("select @folioaux2:=valor from foliosemp where folio='MOVALMA' AND sucursal = '%s' %s ",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliosig2=@folioaux2+1");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @folioaux2=cast(@folioaux2 as char)");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @folio2=concat('%s', lpad(@folioaux2,9,'0'))", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("update foliosemp set valor=@foliosig2 where folio='MOVALMA' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;

				// Graba la cabecera en la tabla "movalma"
				datos.InsCampo("movimiento", "@folio2", 1);
				datos.InsCampo("usualta", usuario);
				datos.InsCampo("usumodi", usuario);
				datos.InsCampo("fechaalta", mFg.DateToAnsiString(fecha));
				datos.InsCampo("horaalta", mFg.TimeToAnsiString(hora));
				datos.InsCampo("fechamodi", mFg.DateToAnsiString(fecha));
				datos.InsCampo("horamodi", mFg.TimeToAnsiString(hora));
				datos.InsCampo("cancelado", "0");
				instrucciones[num_instrucciones++]=datos.GenerarSqlInsert();

				// Graba las partidas en "dmovalma"
				num_partidas=entradas;
				for (i=0; i<num_partidas; i++) {
					datos.AsignaTabla("dmovalma");
					parametros+=datos.InsCamposDesdeBuffer(parametros);
					datos.InsCampo("movimiento", "@folio2", 1);
					instrucciones[num_instrucciones++]=datos.GenerarSqlInsert();
				}

                //se actualiza existenciasactuales desde subconsulta agrupada por  producto, presentación  y almacen: para ENTRADAS
				instruccion.sprintf("UPDATE existenciasactuales ea, \
				(SELECT  sum(d.cantidad * a.factor) as sumCantidad, a.producto , a.present, m.almaent \
				FROM  movalma m \
				INNER JOIN dmovalma d ON m.movimiento = d.movimiento \
				INNER JOIN articulos a on a.articulo = d.articulo \
				WHERE m.movimiento = @folio2 \
				group by a.producto, a.present, m.almaent) as tgent \
				set ea.cantidad = ea.cantidad + tgent.sumCantidad , ea.entradas =ea.entradas+tgent.sumCantidad \
				where ea.producto=tgent.producto and ea.present=tgent.present and ea.almacen=tgent.almaent");
				instrucciones[num_instrucciones++]=instruccion;

			}
		}
		instrucciones[num_instrucciones++]="COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			instruccion.sprintf("select %d as error, @folio1 as folio1, @folio2 as folio2", error);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	} __finally {
		delete buffer_sql;
	}
}

//---------------------------------------------------------------------------
// ID_RESUMEN_CARGA
void ServidorAlmacen::ResumenCargaEmbarque(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*64*20];
	char *aux_buffer_sql=buffer_sql;
	int i, num_partidas;
	double cantidad, cantidad_mod, cantidad_final, cantidad_art, cantidad_act;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[1000];
	AnsiString embarque,sucursal;
	AnsiString almacen, condicion_almacen=" ",tiporeporte,tipoorden, division;
	AnsiString almacen_aux, cad_conjunto_almacenes=" ",  condicion_almacen_x_seccion=" ", condicion_division=" ";
	AnsiString condicion_solo_con_existencias=" ";
    AnsiString condicion_auxpos_almacen = " ";
	TDate fecha=Today();
	TTime hora=Time();
	AnsiString archivo_temp1="", archivo_temp2="", archivo_temp3="", archivo_temp4="", archivo_temp5="";
	AnsiString archivo_temp6="", archivo_temp7="", archivo_temp8="";
	BufferRespuestas* resp_almacenes=NULL;
	AnsiString modo_calcular_existencias;
	AnsiString ImprimeSoloConExistencias, usuario;
	bool EliminarSinExistencias=false;
	AnsiString mParametroPosArt = "0";
    AnsiString ubicacion_articulo = " ", join_auxubiart = " ";

	AnsiString idtag, condicion_join_tag=" ", condicion_retorna_tag=" ", ordenUbicacionResumidoExistencias = " ";

	try{
		embarque=mFg.ExtraeStringDeBuffer(&parametros);
		almacen=mFg.ExtraeStringDeBuffer(&parametros);
		tiporeporte=mFg.ExtraeStringDeBuffer(&parametros);
		tipoorden=mFg.ExtraeStringDeBuffer(&parametros);
		division=mFg.ExtraeStringDeBuffer(&parametros);
		ImprimeSoloConExistencias=mFg.ExtraeStringDeBuffer(&parametros);
		usuario=mFg.ExtraeStringDeBuffer(&parametros);
		idtag=mFg.ExtraeStringDeBuffer(&parametros);
		ordenUbicacionResumidoExistencias=mFg.ExtraeStringDeBuffer(&parametros);

		//nueva consulta de un parametro del método para revisar las existencias con base a la tabla de existencias actuales.
		BufferRespuestas* resp_parametros=NULL;
		try{
			instruccion.sprintf("SELECT * FROM parametrosemp WHERE parametro = 'CALCEXISTRCAR' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_parametros);
			if (resp_parametros->ObtieneNumRegistros()>0){
				modo_calcular_existencias=resp_parametros->ObtieneDato("valor");
			}
		}__finally{
			if (resp_parametros!=NULL) delete resp_parametros;
		}
		//nueva consulta de un parametro del método para revisar las existencias con base a la tabla de existencias actuales.
		BufferRespuestas* resp_parametro_embarque=NULL;
		try{

			//consulta parametro para asignar la forma de mostrar las ubicaciones en el resumen de carga resumido con existencias.
			instruccion.sprintf("SELECT * FROM parametrosemp WHERE parametro = 'ACTUBIART' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_parametro_embarque);
			if (resp_parametros->ObtieneNumRegistros()>0){
				mParametroPosArt=resp_parametro_embarque->ObtieneDato("valor");
			}
		}__finally{
			if (resp_parametro_embarque!=NULL) delete resp_parametro_embarque;
		}
		//consulta parametro para asignar la forma de mostrar las ubicaciones en el resumen de carga resumido con existencias.
		if(almacen!=" "){
			condicion_almacen_x_seccion.sprintf("and axs.almacen='%s'",almacen);
            condicion_auxpos_almacen.sprintf("and axp.almacen='%s'", almacen);
		}

		if (division!=" ") {
			condicion_division.sprintf(" and p.embarquediv=%s ", division);
		}

		if(ImprimeSoloConExistencias=="1"){
			condicion_solo_con_existencias = " and af.cantidad>0 ";
			EliminarSinExistencias=true;
		}

		if (mParametroPosArt == "0") {
			ubicacion_articulo = ", axs.almacen AS posart ";
		} else {
			ubicacion_articulo = ", IF(ISNULL(axp.almacen), axs.almacen,\
			CONCAT_WS('-',LEFT(axp.almacen, 3),axp.nombrecalle,axp.numfrente,axp.nivel)) AS posart ";
			join_auxubiart = " INNER JOIN auxposart axp ON ar.producto = axp.producto AND ar.present = axp.present ";
        }


		if (tiporeporte=="1" || tiporeporte=="2" ) { // CALCULA EXISTENCIAS (EN TIPO DE REPORTE "RESUMIDO CON EXISTENCIAS")
									//para vgalidaciones de exixtencias se aplica para el segundo reporte

				// Por sucursal
				sucursal=FormServidor->ObtieneClaveSucursal();
				instruccion.sprintf("SELECT a.almacen FROM almacenes a \
					INNER JOIN secciones s ON a.seccion=s.seccion \
					WHERE s.sucursal='%s'", sucursal);
				mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_almacenes);
				for(i=0; i<resp_almacenes->ObtieneNumRegistros(); i++){
					resp_almacenes->IrAlRegistroNumero(i);
					almacen_aux=resp_almacenes->ObtieneDato("almacen");

					cad_conjunto_almacenes+="'";
					cad_conjunto_almacenes+=almacen_aux;
					cad_conjunto_almacenes+="'";
					if (i<resp_almacenes->ObtieneNumRegistros()-1) // A todos menos al ultimo impuesto el signo +.
						cad_conjunto_almacenes+=",";
				}


			instruccion="SET SESSION sql_log_bin = 0";
			instrucciones[num_instrucciones++]=instruccion;

			// Crea una tabla temporal para agrupar los productos por producto-present-almacen
			instruccion="create temporary table auxagrupados ( \
				producto varchar(8), present varchar(255), almacen char(4), \
				INDEX(producto, present)) Engine = InnoDB";
			instrucciones[num_instrucciones++]=instruccion;

			// Crea una tabla temporal para agrupar producto presentacion
			instruccion="create temporary table auxagrupadospp ( \
				producto varchar(8), present varchar(255), \
				INDEX(producto, present)) Engine = InnoDB";
			instrucciones[num_instrucciones++]=instruccion;

			// Crea una tabla donde se pondrán las existencias que se vayan encontrando.
			instruccion="create temporary table existenciasaux ( \
				producto varchar(8), present varchar(255), almacen char(4), \
				tipo char(2), cantidad decimal(12,3), INDEX(producto, present, almacen)) Engine = InnoDB";
			instrucciones[num_instrucciones++]=instruccion;

			// Crea una tabla donde se pondrán las existencias sumadas
			instruccion="create temporary table auxexistsumadas ( \
				producto varchar(8), present varchar(255), \
				cantidad decimal(12,3), cantvpprom decimal(12,3), \
				INDEX(producto, present)) Engine = InnoDB";
			instrucciones[num_instrucciones++]=instruccion;

			// Crea una tabla donde se pondrán las existencias finales
			instruccion="create temporary table auxexistfinales ( \
				producto varchar(8), present varchar(255), \
				cantidad decimal(12,3), \
				maxfactor decimal(10,3), multmax varchar(10), \
				minfactor decimal(10,3), multmin varchar(10), \
				INDEX(producto, present)) Engine = InnoDB";
			instrucciones[num_instrucciones++]=instruccion;

			// Crear tabla donde estarán convertidos los productos
			instruccion.sprintf("CREATE TEMPORARY TABLE auxproductoconvertido \
			(referencia VARCHAR(11), articulo VARCHAR(9), \
			producto varchar(8), present varchar(255), \
			multiplo varchar(13), cantidad decimal(12,3), factor DECIMAL(10,3), INDEX(articulo)) Engine = InnoDB");
			instrucciones[num_instrucciones++]=instruccion;

			// Cuando es resumido.

			// Agrupa por producto-presentacion-almacén
			if (cad_conjunto_almacenes!=" ") {
				condicion_almacen.sprintf(" and alm.almacen in (%s) ", cad_conjunto_almacenes);
			} else condicion_almacen=" ";
			instruccion.sprintf("insert into auxagrupados (producto, present, almacen) \
				SELECT ar.producto, ar.present, alm.almacen \
				FROM pedidosventa p, embarques e, dpedidosventa dp, \
					articuloxseccion axs, terminales ter, articulos ar, almacenes alm \
				WHERE e.embarque=p.embarque AND p.referencia=dp.referencia AND ar.articulo=dp.articulo AND \
					ter.terminal=p.terminal AND axs.articulo=dp.articulo AND  axs.seccion=ter.seccion \
					AND p.embarque='%s' %s \
					%s \
				GROUP BY ar.producto, ar.present, alm.almacen",embarque, condicion_division, condicion_almacen);
			instrucciones[num_instrucciones++]=instruccion;

			// Agrupa por producto-presentacion-cantidad
			instruccion.sprintf("insert into auxagrupadospp (producto, present) \
				select aux.producto, aux.present \
				from auxagrupados aux \
				group by aux.producto, aux.present");
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("set @fechacorte='1900-01-01'");
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("set @fechacorte='1900-01-01'");
			instrucciones[num_instrucciones++]=instruccion;

			// Calcula base (0) de los artículos en cuestión la existencia al momento del corte previo.
			// La utilidad de esto es simplemente para tomar en cuenta todos los articulos aunque no
			// tengan ningun movimiento
			instruccion.sprintf("insert into existenciasaux (producto, present, almacen, tipo, cantidad) \
				select aux.producto, aux.present, aux.almacen as almacen, 'BA' as tipo, 0 as cantidad \
				from auxagrupados aux");
			instrucciones[num_instrucciones++]=instruccion;

			if (modo_calcular_existencias=="MODN") {
                throw Exception("Configuración de modo de calculo para revisión de existencias está incorrecto MODN");
			} else {
				if(modo_calcular_existencias == "MODEA") {
					// Agrega las existencias
					archivo_temp1=mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);
					instruccion.sprintf("SELECT e.producto, e.present, aux.almacen, 'EX' as tipo, SUM(e.cantidad) AS cantidad \
						FROM auxagrupados aux \
						INNER JOIN existenciasactuales e ON aux.producto=e.producto AND e.present=aux.present AND e.almacen=aux.almacen \
						GROUP BY aux.producto, aux.present, aux.almacen INTO OUTFILE '%s'", archivo_temp1);
					instrucciones[num_instrucciones++]=instruccion;

					instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE existenciasaux ",archivo_temp1);
					instrucciones[num_instrucciones++]=instruccion;
				} else
					throw Exception("Configuración de modo de calculo para revisión de existencias está incorrecto");
			}

			// Suma los movimientos para obtener las existencias
			instruccion.sprintf("insert into auxexistsumadas (producto, present, cantidad) \
					select e.producto, e.present, sum(e.cantidad) as cantidad \
					from existenciasaux e \
					group by e.producto, e.present");
			instrucciones[num_instrucciones++]=instruccion;

			// Obtiene los factores minimo y máximo
			instruccion.sprintf("insert into auxexistfinales (producto, present, cantidad, maxfactor, minfactor) \
				select a.producto, a.present, e.cantidad, max(a.factor) as maxfactor, min(a.factor) as minfactor \
				from auxexistsumadas e, articulos a \
				where e.producto=a.producto and e.present=a.present and a.activo=1 \
				group by e.producto, e.present");
			instrucciones[num_instrucciones++]=instruccion;

			// Obtiene los múltiplos maximos
			instruccion.sprintf("update auxexistfinales e, articulos a \
				set e.multmax=a.multiplo where e.producto=a.producto and \
				e.present=a.present and e.maxfactor=a.factor and a.activo=1 ");
			instrucciones[num_instrucciones++]=instruccion;

			// Obtiene los múltiplos minimos
			instruccion.sprintf("update auxexistfinales e, articulos a \
				set e.multmin=a.multiplo where e.producto=a.producto and \
				e.present=a.present and e.minfactor=a.factor and a.activo=1 ");
			instrucciones[num_instrucciones++]=instruccion;

			// Se guardan los productos que son multiplo maximo
			archivo_temp6=mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);
			instruccion.sprintf(" SELECT \
				p.referencia, \
				ar.articulo, \
				ar.producto, \
				ar.present, \
				ar.multiplo, \
				sum(dp.cantidad) AS cantidad, \
				ar.factor \
				FROM pedidosventa p \
				INNER JOIN embarques e ON e.embarque=p.embarque \
				INNER JOIN dpedidosventa dp ON p.referencia=dp.referencia \
				INNER JOIN articulos ar ON ar.articulo = dp.articulo \
				INNER JOIN presentacionesminmax pmm ON pmm.producto = ar.producto AND pmm.present = ar.present \
				WHERE p.embarque='%s' AND ar.multiplo = pmm.maxmult \
				GROUP BY \
				referencia, \
				ar.producto, \
				ar.present INTO OUTFILE '%s'", embarque, archivo_temp6);
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE auxproductoconvertido ",archivo_temp6);
			instrucciones[num_instrucciones++]=instruccion;

			// Se guardan los productos que se convierten a multiplo maximo
			archivo_temp7=mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);
			instruccion.sprintf("SELECT \
				p.referencia, \
				art.articulo, \
				art.producto, \
				art.present, \
				pmm.maxmult, \
				TRUNCATE((dp.cantidad*ar.factor)/pmm.maxfactor,0) AS cantidad, \
				pmm.maxfactor \
				FROM pedidosventa p \
				INNER JOIN embarques e ON e.embarque=p.embarque \
				INNER JOIN dpedidosventa dp ON p.referencia=dp.referencia \
				INNER JOIN articulos ar ON ar.articulo = dp.articulo \
				INNER JOIN presentacionesminmax pmm ON pmm.producto = ar.producto AND pmm.present = ar.present \
				INNER JOIN articulos art ON art.producto = pmm.producto AND art.present = pmm.present AND art.multiplo = pmm.maxmult \
				WHERE p.embarque='%s' AND ar.multiplo<>pmm.maxmult \
				GROUP BY referencia, ar.articulo \
				INTO OUTFILE '%s'", embarque, archivo_temp7);
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE auxproductoconvertido ",archivo_temp7);
			instrucciones[num_instrucciones++]=instruccion;

			// Se guardan los productos que se convierten a multiplo maximo
			archivo_temp8=mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);
			instruccion.sprintf("SELECT \
				p.referencia, \
				ar.articulo, \
				ar.producto, \
				ar.present, \
				ar.multiplo, \
				TRUNCATE((MOD(SUM(dp.cantidad)*ar.factor, pmm.maxfactor)/ar.factor),5), \
				ar.factor \
				FROM pedidosventa p \
				INNER JOIN embarques e ON e.embarque=p.embarque \
				INNER JOIN dpedidosventa dp ON p.referencia=dp.referencia \
				INNER JOIN articulos ar ON ar.articulo = dp.articulo \
				INNER JOIN presentacionesminmax pmm ON pmm.producto = ar.producto AND pmm.present = ar.present \
				WHERE p.embarque='%s' \
				AND ar.multiplo!=pmm.maxmult \
				GROUP BY \
				referencia, \
         		ar.articulo INTO OUTFILE '%s'", embarque, archivo_temp8);
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE auxproductoconvertido ",archivo_temp8);
			instrucciones[num_instrucciones++]=instruccion;

            if (EliminarSinExistencias &&(tiporeporte=="1" || tiporeporte=="2" )) {
			//se crea tabla temporal para respaldar articulos de todo el embarque antes de eliminar los que no tienen existencias
				instruccion="CREATE TEMPORARY TABLE auxdpedidosventa ( \
				referencia VARCHAR(11) NOT NULL DEFAULT '', \
				articulo VARCHAR(9) NOT NULL DEFAULT '', \
				cantidad DECIMAL(12,3) NULL DEFAULT NULL, \
				claveimp1 INT(2) NULL DEFAULT NULL, \
				claveimp2 INT(2) NULL DEFAULT NULL, \
				claveimp3 INT(2) NULL DEFAULT NULL, \
				claveimp4 INT(2) NULL DEFAULT NULL, \
				costobase DECIMAL(16,6) NULL DEFAULT NULL, \
				porcdesc DECIMAL(16,2) NULL DEFAULT NULL, \
				precio DECIMAL(16,6) NULL DEFAULT NULL, \
				precioimp DECIMAL(16,6) NULL DEFAULT NULL, \
				id INT(4) NULL DEFAULT '0', \
				PRIMARY KEY (referencia, articulo), \
				INDEX articulo (articulo)) ENGINE=InnoDB ";
				instrucciones[num_instrucciones++]=instruccion;

				instruccion.sprintf("insert into auxdpedidosventa (referencia,articulo,cantidad,claveimp1,claveimp2,claveimp3, \
					claveimp4,costobase,porcdesc,precio,precioimp,id) \
					select dp.referencia, dp.articulo, dp.cantidad, dp.claveimp1, dp.claveimp2, dp.claveimp3, \
					dp.claveimp4, dp.costobase, dp.porcdesc, dp.precio, dp.precioimp, dp.id \
					from  pedidosventa as p inner join \
					dpedidosventa as dp on dp.referencia=p.referencia \
					where p.embarque='%s' and p.facturado=0 %s", embarque, condicion_division);
				instrucciones[num_instrucciones++]=instruccion;

				//se eliminan todos los registros de pedidos sin facturar
				instruccion.sprintf("delete dp.* \
					from dpedidosventa as dp  inner join pedidosventa as p	on dp.referencia=p.referencia \
					where p.embarque='%s' and p.facturado=0 %s ", embarque, condicion_division);
				instrucciones[num_instrucciones++]=instruccion;

				//se insertan los que tiene existencias  para contemplar condición de imprimir solo con existencias
				instruccion.sprintf("insert into dpedidosventa (referencia,articulo,cantidad,claveimp1,claveimp2,claveimp3, \
				claveimp4,costobase,porcdesc,precio,precioimp,id) \
				select dp.referencia, dp.articulo, dp.cantidad, dp.claveimp1, dp.claveimp2, dp.claveimp3, dp.claveimp4, \
				dp.costobase, dp.porcdesc, dp.precio, dp.precioimp, dp.id \
				FROM pedidosventa p, embarques e, auxdpedidosventa dp,productos pr, presentaciones pre, \
				articuloxseccion axs, terminales ter, \
				clientes cl, articulos ar \
				LEFT JOIN auxexistfinales af ON af.producto=ar.producto AND af.present=ar.present \
				WHERE p.cliente=cl.cliente and e.embarque=p.embarque and p.referencia=dp.referencia and ar.articulo=dp.articulo \
				and ter.terminal=p.terminal and axs.articulo=dp.articulo \
				and axs.seccion=ter.seccion and ar.present=pre.present and pre.producto=pr.producto and ar.producto=pr.producto \
				and p.embarque='%s' and p.facturado=0 \
				and af.cantidad>0 %s", embarque, condicion_division);
				instrucciones[num_instrucciones++]=instruccion;

				//en tabla de respaldo para bitacora se insertan los que no tienen existencias
				instruccion.sprintf("insert into dpedidosventasinexistencia (embarque,referencia,articulo,cantidad,claveimp1,claveimp2, \
				claveimp3,claveimp4,costobase,porcdesc,precio,precioimp,id,usuario,fechamodi,horamodi) \
				select '%s' as embarque, dp.referencia, dp.articulo, dp.cantidad, dp.claveimp1, dp.claveimp2, \
				dp.claveimp3, dp.claveimp4, dp.costobase, dp.porcdesc, dp.precio, \
				dp.precioimp, dp.id ,'%s' as usuario, CURDATE() as fechamodi, CURTIME() as horamodi \
				FROM pedidosventa p, embarques e, auxdpedidosventa dp,productos pr, presentaciones pre, \
				articuloxseccion axs, terminales ter, \
				clientes cl, articulos ar \
				LEFT JOIN auxexistfinales af ON af.producto=ar.producto AND af.present=ar.present \
				WHERE p.cliente=cl.cliente and e.embarque=p.embarque and p.referencia=dp.referencia \
				and ar.articulo=dp.articulo and ter.terminal=p.terminal and axs.articulo=dp.articulo \
				and axs.seccion=ter.seccion and ar.present=pre.present and pre.producto=pr.producto \
				and ar.producto=pr.producto and p.embarque='%s' and p.facturado=0 \
				and af.cantidad<=0 %s", embarque, usuario, embarque, condicion_division);
				instrucciones[num_instrucciones++]=instruccion;

				//se actualiza el total de pedido en base a los articulos que se tienen con existencias
				instruccion.sprintf("UPDATE pedidosventa AS p, \
						 (SELECT dp.referencia, sum(dp.precioimp*dp.cantidad) as svalor \
						 FROM pedidosventa as p \
							inner join dpedidosventa as dp on dp.referencia=p.referencia and p.embarque='%s' and p.facturado=0 \
						GROUP BY dp.referencia) t \
					SET p.valor=t.svalor, fechamodi='%s', horamodi='%s', usumodi='%s' \
					WHERE p.referencia = t.referencia and p.embarque='%s'  and p.facturado=0 and p.valor<>t.svalor %s"
					,embarque, mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario, embarque, condicion_division);
				instrucciones[num_instrucciones++]=instruccion;

			}

            //se obtiene la ubicacion de los articulos del embarque
				instruccion.sprintf("CREATE TEMPORARY TABLE auxposart ( \
									SELECT \
									art.producto, \
									art.present, \
									acl.almacen, \
									acl.nombrecalle, \
									afr.numfrente, \
                                    apos.nivel \
									FROM pedidosventa p \
									INNER JOIN dpedidosventa dpv ON p.referencia = dpv.referencia \
									INNER JOIN articulos art ON dpv.articulo = art.articulo \
									LEFT JOIN almacenposarticulos aposart ON aposart.producto = art.producto AND aposart.present = art.present \
									LEFT JOIN almacenposiciones apos ON aposart.idalmposicion = apos.idalmposicion \
									LEFT JOIN almacenfrentes afr ON apos.idalmfrente = afr.idalmfrente \
									LEFT JOIN almacencalles acl ON afr.idalmcalle = acl.idalmcalle \
									WHERE p.embarque = '%s' %s \
									GROUP BY art.producto, art.present )", embarque, condicion_division);
                instrucciones[num_instrucciones++]=instruccion;

			instruccion="SET SESSION sql_log_bin = 1";
			instrucciones[num_instrucciones++]=instruccion;
		}

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		condicion_retorna_tag.sprintf(" ,( CASE WHEN ( a.idarticulotag IS NOT NULL ) THEN 1 ELSE 0 END ) AS tag");
		condicion_join_tag.sprintf(" LEFT JOIN articulostagsasignados a ON a.idarticulotag = %s AND a.producto = ar.producto AND a.present = ar.present ",idtag);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			if (archivo_temp1!="")
				mServidorVioleta->BorraArchivoTemp(archivo_temp1);
			if (archivo_temp2!="")
				mServidorVioleta->BorraArchivoTemp(archivo_temp2);
			if (archivo_temp3!="")
				mServidorVioleta->BorraArchivoTemp(archivo_temp3);
			if (archivo_temp4!="")
				mServidorVioleta->BorraArchivoTemp(archivo_temp4);
			if (archivo_temp5!="")
				mServidorVioleta->BorraArchivoTemp(archivo_temp5);
			if (archivo_temp6!="")
				mServidorVioleta->BorraArchivoTemp(archivo_temp6);
			if (archivo_temp7!="")
				mServidorVioleta->BorraArchivoTemp(archivo_temp7);
			if (archivo_temp8!="")
				mServidorVioleta->BorraArchivoTemp(archivo_temp8);

			if(tiporeporte==2){

				// DETALLADO
				if(tipoorden=="0"){
					instruccion.sprintf("select ar.articulo, p.referencia, cl.rsocial, pr.producto, sum(dp.cantidad) as cantidad, \
										concat(af.multmax,'-',TRUNCATE(af.maxfactor,0)) as multmaxf, \
										concat(af.multmin,'-',TRUNCATE(af.minfactor,0)) as multminf, \
										concat(left(dp.multiplo,4),'-',TRUNCATE(dp.factor,0)) as mulfactor, pr.nombre as nomproducto,\
										pre.present, axs.almacen, RIGHT(ar.ean13, 5) AS cod_barr %s \
										FROM pedidosventa p, embarques e, auxproductoconvertido dp, \
										productos pr, presentaciones pre, articuloxseccion axs, terminales ter, \
										clientes cl, articulos ar %s \
										LEFT JOIN auxexistfinales af ON af.producto=ar.producto AND af.present=ar.present \
										WHERE p.cliente=cl.cliente and e.embarque=p.embarque and \
										p.referencia=dp.referencia and ar.articulo=dp.articulo and ter.terminal=p.terminal and axs.articulo=dp.articulo and \
										axs.seccion=ter.seccion \
										and ar.present=pre.present and pre.producto=pr.producto and ar.producto=pr.producto AND dp.cantidad != 0 and \
										p.embarque='%s' %s %s %s \
										GROUP BY pr.nombre asc, pre.present, ar.articulo, referencia with rollup",
										condicion_retorna_tag, condicion_join_tag, embarque, condicion_division, condicion_almacen_x_seccion,condicion_solo_con_existencias);
				}

				if(tipoorden=="1") {
					// Nota: En la sig. instrucción tuve que poner concat(ar.multiplo) en el group by ya que si solo ponía ar.multiplo
					// no funcionaba como debería (me parece un bug de mariadb con WITH ROOLUP, quizas algo por las optimizaciones con índices)
					instruccion.sprintf("select ar.articulo, p.referencia, cl.rsocial, pr.producto, sum(dp.cantidad) as cantidad, \
										concat(af.multmax,'-',TRUNCATE(af.maxfactor,0)) as multmaxf, \
										concat(af.multmin,'-',TRUNCATE(af.minfactor,0)) as multminf, \
										concat(left(dp.multiplo,4),'-',TRUNCATE(dp.factor,0)) as mulfactor, pr.nombre as nomproducto, pre.present, axs.almacen, \
										IF(ar.factor=1,1,2) ORDEN,pr.clasif1, RIGHT(ar.ean13, 5) AS cod_barr %s %s\
										FROM pedidosventa p, embarques e, auxproductoconvertido dp, \
										productos pr, presentaciones pre, articuloxseccion axs, terminales ter, \
										clientes cl, articulos ar %s %s\
										LEFT JOIN auxexistfinales af ON af.producto=ar.producto AND af.present=ar.present \
										WHERE p.cliente=cl.cliente and e.embarque=p.embarque and \
										p.referencia=dp.referencia and ar.articulo=dp.articulo and ter.terminal=p.terminal and axs.articulo=dp.articulo and \
										axs.seccion=ter.seccion \
										and ar.present=pre.present and pre.producto=pr.producto and ar.producto=pr.producto AND dp.cantidad != 0 \
										and p.embarque='%s' %s %s %s\
										GROUP BY ORDEN DESC, IF(ORDEN='1',pr.clasif1,left(ar.multiplo,3)), pr.nombre ASC, pre.present, ar.articulo, referencia WITH ROLLUP ",
										condicion_retorna_tag, ubicacion_articulo, condicion_join_tag, join_auxubiart, embarque, condicion_division, condicion_almacen_x_seccion,condicion_solo_con_existencias);
                }
			}

			else {
				if(tiporeporte==1){
					// RESUMIDO CON EXISTENCIAS
					/*  calculo con el factor del producto,
						se cambio, por el factor maximo
						truncate(sum(dp.cantidad*ar.factor)/ar.factor,0) as pedmax, \
						truncate(mod(sum(dp.cantidad*ar.factor),ar.factor),0) as pedmin, \
						truncate((ifnull(af.cantidad,0))/ar.factor,0) as existmax, \
						truncate(mod((ifnull(af.cantidad,0)),ar.factor),0) as existmin, \
					*/


					if(tipoorden=="0")
						instruccion.sprintf("select pr.nombre as NomProducto, ar.present, \
								concat(af.multmax,'-',TRUNCATE(af.maxfactor, 0)) as multmaxf, \
								concat(af.multmin,'-',TRUNCATE(af.minfactor, 0)) as multminf, \
								truncate(sum(dp.cantidad*ar.factor)/af.maxfactor,0) as pedmax, \
								truncate(mod(sum(dp.cantidad*ar.factor),af.maxfactor),0) as pedmin, \
								truncate((ifnull(af.cantidad,0))/af.maxfactor,0) as existmax, \
								truncate(mod((ifnull(af.cantidad,0)),af.maxfactor),0) as existmin, \
								sum(dp.cantidad*ar.factor) as pedunid, \
								(ifnull(af.cantidad,0)) as existunid %s \
								FROM pedidosventa p \
									inner join embarques e ON e.embarque=p.embarque \
									inner join auxproductoconvertido dp ON p.referencia=dp.referencia \
									inner join articulos ar ON ar.articulo=dp.articulo \
									%s \
									inner join productos pr ON ar.producto=pr.producto \
									inner join terminales ter ON ter.terminal=p.terminal \
									inner join articuloxseccion axs ON axs.seccion=ter.seccion and axs.articulo=dp.articulo \
									inner join clientes cl ON p.cliente=cl.cliente \
									left join auxexistfinales af on af.producto=ar.producto and af.present=ar.present \
								WHERE p.embarque='%s' AND dp.cantidad != 0 %s %s %s \
								GROUP BY ar.producto, ar.present \
								ORDER BY pr.nombre asc, ar.present "
								,condicion_retorna_tag, condicion_join_tag, embarque, condicion_division, condicion_almacen_x_seccion,condicion_solo_con_existencias);

					if(tipoorden=="1"){
						AnsiString ordenResumidoExistencias = " pr.nombre asc, ar.present ";
						if(ordenUbicacionResumidoExistencias == "1")
							ordenResumidoExistencias = " IFNULL(axp.almacen, axs.almacen), axp.nombrecalle, axp.numfrente, axp.nivel ";

						instruccion.sprintf("select pr.nombre as NomProducto, ar.present, \
								concat(af.multmax,'-',TRUNCATE(af.maxfactor, 0)) as multmaxf, \
								concat(af.multmin,'-',TRUNCATE(af.minfactor, 0)) as multminf, \
								truncate(sum(dp.cantidad*ar.factor)/af.maxfactor,0) as pedmax, \
								truncate(mod(sum(dp.cantidad*ar.factor),af.maxfactor),0) as pedmin, \
								truncate((ifnull(af.cantidad,0))/af.maxfactor,0) as existmax, \
								truncate(mod((ifnull(af.cantidad,0)),af.maxfactor),0) as existmin, \
								sum(dp.cantidad*ar.factor) as pedunid, \
								(ifnull(af.cantidad,0)) as existunid %s,  \
								IF(ar.factor=1,1,2) ORDEN, \
								pr.clasif1, \
								SUBSTRING(ar.ean13, -4) AS codigo_barras \
								%s \
								FROM pedidosventa p \
									inner join embarques e ON e.embarque=p.embarque \
									inner join auxproductoconvertido dp ON p.referencia=dp.referencia \
									inner join articulos ar ON ar.articulo=dp.articulo \
									%s \
									inner join productos pr ON ar.producto=pr.producto \
									inner join terminales ter ON ter.terminal=p.terminal \
									inner join articuloxseccion axs ON axs.seccion=ter.seccion and axs.articulo=dp.articulo \
									inner join clientes cl ON p.cliente=cl.cliente \
									left join auxexistfinales af on af.producto=ar.producto and af.present=ar.present \
									%s \
								WHERE p.embarque='%s' AND dp.cantidad != 0 %s %s %s \
								GROUP BY pr.producto, ar.present \
								ORDER BY %s "
								,condicion_retorna_tag, ubicacion_articulo, condicion_join_tag, join_auxubiart, embarque,
								condicion_division, condicion_almacen_x_seccion, condicion_solo_con_existencias, ordenResumidoExistencias);
					}
				}

				else {
					// RESUMIDO SIN EXISTENCIAS
					if(tipoorden=="0")
						instruccion.sprintf("select ' ' as referencia, ' ' as rsocial, pr.producto, sum(dp.cantidad) as cantidad, \
								concat(left(ar.multiplo,4),'-', TRUNCATE(ar.factor,0)) as mulfactor, pr.nombre as NomProducto, pre.present, axs.almacen %s \
								FROM pedidosventa p, embarques e, dpedidosventa dp, \
								productos pr, presentaciones pre, articuloxseccion axs, terminales ter, \
								articulos ar %s, clientes cl \
								WHERE p.cliente=cl.cliente and e.embarque=p.embarque and \
								p.referencia=dp.referencia and ar.articulo=dp.articulo and ter.terminal=p.terminal and axs.articulo=dp.articulo and \
								axs.seccion=ter.seccion \
								and ar.present=pre.present and pre.producto=pr.producto and ar.producto=pr.producto and p.embarque='%s' %s %s \
								GROUP BY pr.nombre asc, pre.present, ar.articulo \
								ORDER BY pr.nombre asc, pre.present, ar.articulo "
								,condicion_retorna_tag, condicion_join_tag, embarque, condicion_division, condicion_almacen_x_seccion);

					if(tipoorden=="1")
						instruccion.sprintf("select ' ' as referencia, ' ' as rsocial, pr.producto, sum(dp.cantidad) as cantidad, \
								concat(left(ar.multiplo,4),'-',TRUNCATE(ar.factor,0)) as mulfactor, pr.nombre as NomProducto, pre.present, axs.almacen %s \
								,IF(ar.factor=1,1,2) ORDEN,pr.clasif1 \
								FROM pedidosventa p, embarques e, dpedidosventa dp, \
								productos pr, presentaciones pre, articuloxseccion axs, terminales ter, \
								articulos ar %s, clientes cl \
								WHERE p.cliente=cl.cliente and e.embarque=p.embarque and \
								p.referencia=dp.referencia and ar.articulo=dp.articulo and ter.terminal=p.terminal and axs.articulo=dp.articulo and \
								axs.seccion=ter.seccion \
								and ar.present=pre.present and pre.producto=pr.producto and ar.producto=pr.producto and p.embarque='%s' %s %s \
								GROUP BY ORDEN DESC,ar.multiplo,pr.nombre asc, pre.present,ar.articulo \
								ORDER BY ORDEN DESC,IF(ORDEN='1',pr.clasif1,left(ar.multiplo,3)),pr.nombre ASC, pre.present,ar.multiplo "
								,condicion_retorna_tag, condicion_join_tag, embarque, condicion_division, condicion_almacen_x_seccion);
				}

			}



			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	} __finally {
		if(resp_almacenes!=NULL) delete resp_almacenes;
		delete buffer_sql;
	}
}
//---------------------------------------------------------------------------
//ID_MOV_SURTIDOR
void ServidorAlmacen::GrabaMovimientoSurtidor(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
 //nueva funcion

	char *buffer_sql=new char[1024*32];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instrucciones[50], instruccion;
	int num_instrucciones=0;
	AnsiString embarque,usuariomod,surtante=" ",surdesp=" ",evento,sucursal,terminal,formulario;
    int error=0,i;


	try{
		embarque=mFg.ExtraeStringDeBuffer(&parametros);
		usuariomod=mFg.ExtraeStringDeBuffer(&parametros);
		surtante=mFg.ExtraeStringDeBuffer(&parametros);
		surdesp=mFg.ExtraeStringDeBuffer(&parametros);
		evento=mFg.ExtraeStringDeBuffer(&parametros);
		sucursal=mFg.ExtraeStringDeBuffer(&parametros);
		terminal=mFg.ExtraeStringDeBuffer(&parametros);
		formulario=mFg.ExtraeStringDeBuffer(&parametros);

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		//si la tarea es alta, esta será un resgistro con surtidor anterior vacio
		//luego se tiene que registrar en la bitacora del movimiento realizado del surtidor
		if(evento == "A"){
			instruccion.sprintf("insert into bitacorasurtidor (embarque, usumodi, fechamodi,horamodi,surt_antes,surt_desp,evento,sucursal,terminal,formulario)\
			values ('%s','%s', CURDATE(), CURTIME(),'','%s','%s','%s','%s','%s')"
			,embarque,usuariomod,surdesp,evento,sucursal,terminal,formulario);
			instrucciones[num_instrucciones++]=instruccion;

			//enembarque se modificará al surtidor...
			instruccion.sprintf("update embarques set surtidor='%s' where embarque='%s'", surdesp,embarque);
			instrucciones[num_instrucciones++]=instruccion;

		}else{
			instruccion.sprintf("insert into bitacorasurtidor (embarque, usumodi, fechamodi,horamodi,surt_antes,surt_desp,evento,sucursal,terminal,formulario)\
			values ('%s','%s', CURDATE(), CURTIME(),'%s','%s','%s','%s','%s','%s')"
			,embarque,usuariomod,surtante,surdesp,evento,sucursal,terminal,formulario);
			instrucciones[num_instrucciones++]=instruccion;

			//SE MARCA LA VIA DE EMBARQUE COMO NO DISPONIBLE
			instruccion.sprintf("update embarques set surtidor='%s' where embarque='%s'", surdesp,embarque);
			instrucciones[num_instrucciones++]=instruccion;

		}

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


	}__finally{

		if(buffer_sql!=NULL) delete  buffer_sql;
    }
}
//---------------------------------------------------------------------------
 //ID_GRA_ERRORES_EMBARQUES
void ServidorAlmacen::GrabaErrorEmbarque(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros){

	char *buffer_sql=new char[1024*32];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instrucciones[15], instruccion;
	int num_instrucciones=0;
	AnsiString embarque=" ",usualta=" ",area=" ",empleado=" ",criticidad=" ";
	AnsiString motivo=" ",formulario=" ",ref_notacred=" ",ref_venta=" ",articulo=" ",sucursal=" ";
	AnsiString campo_articulo=" ",  values_articulo=" ";
	AnsiString campo_motivo=" ",  values_motivo=" ";
	AnsiString cantidad=" ",campo_cantidad=" ",values_cantidad=" ";
	int i;
	int error=0;


	try{
		usualta=mFg.ExtraeStringDeBuffer(&parametros);
		embarque=mFg.ExtraeStringDeBuffer(&parametros);
		area=mFg.ExtraeStringDeBuffer(&parametros);
		empleado=mFg.ExtraeStringDeBuffer(&parametros);
		motivo=mFg.ExtraeStringDeBuffer(&parametros);
		criticidad=mFg.ExtraeStringDeBuffer(&parametros);
		formulario=mFg.ExtraeStringDeBuffer(&parametros);
		ref_notacred=mFg.ExtraeStringDeBuffer(&parametros);
		ref_venta=mFg.ExtraeStringDeBuffer(&parametros);
		articulo=mFg.ExtraeStringDeBuffer(&parametros);
		sucursal=mFg.ExtraeStringDeBuffer(&parametros);
		cantidad=mFg.ExtraeStringDeBuffer(&parametros);

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		//en esta parte se tiene que validar de que formulario viene
		//porque si se inserta un espacio vacio, marcara error por las llaves foraneas
		//no aceptan el "", tiene que ir nulo para que no cause problemas.
		/* instruccion.sprintf("insert into erroresembarques \
			(usualta, fechaalta, horaalta,embarque,area,empleado,criticidad,error,formulario,referencia_nc,referencia_vta,articulo,sucursal)\
			values ('%s',CURDATE(), CURTIME(),'%s','%s','%s','%s','%s','%s','%s','%s','%s','%s')",usualta,embarque,area,
			empleado,criticidad,motivo,formulario,ref_notacred.c_str(),ref_venta.c_str(),articulo.c_str(),sucursal.c_str());
			*/
		if(articulo.IsEmpty()){
			campo_articulo.sprintf(" ");
			values_articulo.sprintf(" ");
		}else{
			campo_articulo.sprintf(", articulo");
			values_articulo.sprintf(",'%s'",articulo);
		}
		if(motivo.IsEmpty()){
			campo_motivo.sprintf(" ");
			values_motivo.sprintf(" ");
		}else{
			campo_motivo.sprintf(", error ");
			values_motivo.sprintf(", '%s' ",motivo );
		}
		if(cantidad.IsEmpty()){
			campo_cantidad.sprintf(" ");
			values_cantidad.sprintf(" ");

		}else{
			campo_cantidad.sprintf(",cantidad ");
			values_cantidad.sprintf(", %d ", StrToInt(cantidad) );
		}


		// Verifica que exista surtidor en embarque, si no es el caso, se actualice, siempre y cuando
		//desde aqui sea un surtidor
		if(area == "S"){
			instruccion.sprintf("SELECT @surtidor:=IF((surtidor<>''), 0, 1) AS error  FROM embarques WHERE embarque='%s'", embarque);
			mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 1, error);
		}

		if(formulario== "E"){
			instruccion.sprintf("insert into erroresembarques \
			(usualta, fechaalta, horaalta,embarque,area,empleado,criticidad %s,formulario %s ,sucursal %s )\
			values ('%s',CURDATE(), CURTIME(),'%s','%s','%s','%s' %s ,'%s' %s,'%s' %s )",campo_motivo,campo_articulo,campo_cantidad,
			usualta,embarque,area,empleado,criticidad,values_motivo,formulario,values_articulo,sucursal,values_cantidad );
			instrucciones[num_instrucciones++]=instruccion;

		}else if(formulario== "NC"){
			instruccion.sprintf("insert into erroresembarques \
			(usualta, fechaalta, horaalta,embarque,area,empleado,criticidad %s,formulario,referencia_nc %s,sucursal %s )\
			values ('%s',CURDATE(), CURTIME(),'%s', '%s' ,'%s','%s' %s, '%s','%s' %s ,'%s' %s )", campo_motivo,campo_articulo, campo_cantidad,
			usualta,embarque,area,empleado,criticidad,values_motivo,formulario,ref_notacred,values_articulo,sucursal,values_cantidad);
			instrucciones[num_instrucciones++]=instruccion;

		}else if(formulario== "VE"){
			instruccion.sprintf("insert into erroresembarques  \
			(usualta, fechaalta, horaalta,embarque,area,empleado,criticidad %s ,formulario,referencia_vta %s,sucursal %s )\
			values ('%s',CURDATE(), CURTIME(),'%s','%s','%s','%s' %s ,'%s','%s' %s,'%s' %s )",campo_motivo,campo_articulo,campo_cantidad,
			usualta,embarque,area,empleado,criticidad,values_motivo,formulario,ref_venta,values_articulo,sucursal,values_cantidad);
			instrucciones[num_instrucciones++]=instruccion;
		}
		//en caso de que se quiera guardar alguen error en un embarque anterior y no tenga surtidor asignado
        //ademas de que en dar de alta un error se especifique el area de surtidor,se forzara la actualizacion de surtidor en embarque
		if(error== 1 && area == "S"){
			instruccion.sprintf("update embarques set surtidor='%s' WHERE embarque='%s'",empleado, embarque);
			instrucciones[num_instrucciones++]=instruccion;
		}

		instrucciones[num_instrucciones++]="COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		//mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);

		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			instruccion.sprintf("select %d as error", error);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}

	}__finally{

		if(buffer_sql!=NULL) delete  buffer_sql;
    }

}
//---------------------------------------------------------------------------
 //ID_CON_ERRORES_EMBARQUES
void ServidorAlmacen::ConsultaErrorEmbarque(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros){

    // CONSULTA UNA TRANSFORMACION DE PRODUCTO
	AnsiString instruccion,condicion="",campo_foliocfd="",condicion_foliocfd="";
	AnsiString referencia,formulario;

	referencia=mFg.ExtraeStringDeBuffer(&parametros);
	formulario=mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	if( formulario=="E" ){
		condicion.sprintf("  ee.embarque = '%s' and cancelado=0 ", referencia);
	}else if(formulario=="NC"){
		condicion.sprintf("  ee.referencia_nc = '%s' and cancelado=0 ", referencia);
	} else if(formulario=="VTA"){
		//campo_foliocfd.sprintf(", v.foliofisic");
		condicion.sprintf("  ee.referencia_vta = '%s' and cancelado=0 ", referencia);
		//condicion_foliocfd.sprintf("left join ventas v on v.referencia=ee.referencia_vta ");
	}

	// Obtiene todos los generales (de cabecera) de la transformacion
	instruccion.sprintf("select ee.*  from erroresembarques  ee  where  %s ", condicion);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

}
//---------------------------------------------------------------------------
 //ID_CANC_ERRORES_EMBARQUES
void ServidorAlmacen::CancelarErrorEmbarque(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros){


	char *buffer_sql=new char[1024*5];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instruccion, instrucciones[5];
	int num_instrucciones=0;
	int i;
	AnsiString formulario, referencia,usuario,condicion="";

	try{
		referencia=mFg.ExtraeStringDeBuffer(&parametros);
		formulario=mFg.ExtraeStringDeBuffer(&parametros);
		usuario=mFg.ExtraeStringDeBuffer(&parametros);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

		if( formulario=="E" ){
			condicion.sprintf("  embarque = '%s'  ", referencia);
		}else if(formulario=="NC"){
			condicion.sprintf("  referencia_nc = '%s' ", referencia);
		} else if(formulario=="VTA"){
			condicion.sprintf("  referencia_vta = '%s' ", referencia);
		}

		// Se asigna una variable el identificador del usurio
		instruccion.sprintf("select @idusuario:=empleado from usuarios where empleado='%s'", usuario);
		instrucciones[num_instrucciones++]=instruccion;

		// Si la clave es correcta para el usuario dado, entonces cambia dicha clave por la nueva
		instruccion.sprintf("update erroresembarques set cancelado=1 where %s",condicion);
		instrucciones[num_instrucciones++]=instruccion;

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			instruccion.sprintf("select @idusuario as usuario");
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	} __finally {
		delete buffer_sql;
	}

}
//---------------------------------------------------------------------------
 //ID_CON_REP_ERRORESEMBARQUES
void ServidorAlmacen::ReporteErrorEmbarque(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros){

	// CONSULTA UNA TRANSFORMACION DE PRODUCTO
	AnsiString instruccion;
	AnsiString formulario, referencia,usuario,condicion_vta="", condicion_nc="";
	AnsiString fecha_ini, fecha_fin,area,sucursal;
	AnsiString campo_area=" ", condicion_area=" ";
	AnsiString surtidor,condicion_surtidor="";

	fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
	fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
	referencia=mFg.ExtraeStringDeBuffer(&parametros);
	area=mFg.ExtraeStringDeBuffer(&parametros);
	sucursal=mFg.ExtraeStringDeBuffer(&parametros);
	surtidor=mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	if(area.IsEmpty()){
		campo_area.sprintf(", ee.area ");
		condicion_area.sprintf(" ");

	}else{
		campo_area.sprintf(" ");
		condicion_area.sprintf("AND ee.area='%s' ", area);
	}

	if(surtidor.IsEmpty()){
		condicion_surtidor.sprintf(" ");
	}else{
		condicion_surtidor.sprintf("AND em.empleado='%s' ",surtidor);
    }

	// Obtiene todo el detalle de las entradas
	instruccion.sprintf("SELECT ee.*, CONCAT(em.nombre,' ',em.appat,' ',em.apmat) AS nombre,  \
			IF(ee.formulario='E',ee.embarque,\
			IF(ee.formulario='NC',ee.referencia_nc,IF(ee.formulario='VE',ee.referencia_vta,ee.embarque))) AS referencia,\
			e.fechasalid as fechaembarque, a.producto,a.present,a.multiplo, p.nombre as nomarticulo\
			%s \
			FROM  erroresembarques ee \
			INNER JOIN empleados em ON em.empleado=ee.empleado \
			INNER JOIN  embarques e  ON e.embarque=ee.embarque \
			INNER JOIN articulos a ON ee.articulo= a.articulo  \
INNER JOIN productos p ON a.producto=p.producto \
			WHERE ee.cancelado=0 AND ee.fechaalta BETWEEN '%s' AND '%s' \
			%s  %s ORDER BY ee.empleado, ee.fechaalta ASC, ee.horaalta ASC",campo_area, fecha_ini,fecha_fin, condicion_area,condicion_surtidor);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene los totales por area seleccionada
	instruccion.sprintf("SELECT ee.empleado, CONCAT(em.nombre,' ',em.appat,' ',em.apmat) AS nombre, \
			sum( IF(ee.criticidad='A', 1,0) ) AS erroralto,  \
			sum( IF(ee.criticidad='M', 1,0) ) AS errormedio, \
			sum( IF(ee.criticidad='B', 1,0) ) AS errorbajo,  \
			count(ee.iderror) AS numerrores     \
			FROM  erroresembarques ee    \
			INNER JOIN empleados em ON em.empleado=ee.empleado \
			INNER JOIN  embarques e  ON e.embarque=ee.embarque  \
			WHERE ee.cancelado=0 AND ee.fechaalta BETWEEN '%s' AND '%s' \
			%s  %s GROUP BY ee.empleado\
			ORDER BY ee.fechaalta ASC, ee.horaalta ASC", fecha_ini,fecha_fin, condicion_area,condicion_surtidor);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

}
//---------------------------------------------------------------------------
// ID_GRA_ALMACENFRENTE
void ServidorAlmacen::GrabaAlmacenDistribucionFrente(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// GRABA FRENTE
	char *buffer_sql = new char[1024 * 200];
	char *aux_buffer_sql = buffer_sql;
	AnsiString instrucciones[1050];
	AnsiString instruccion;
	int num_instrucciones = 0;
	AnsiString tarea, numfrente, idalmcalle, numniveles, numfondos, tamfrente, tamalto, tamfondo, distprevio, distfrentecalle;
	int numniveles_int, numfondos_int;
	int i;

	try {
		tarea = mFg.ExtraeStringDeBuffer(&parametros); // Indicador si se va a agregar o modificar
		numfrente = mFg.ExtraeStringDeBuffer(&parametros);
		idalmcalle = mFg.ExtraeStringDeBuffer(&parametros);
		numniveles = mFg.ExtraeStringDeBuffer(&parametros);
		numfondos = mFg.ExtraeStringDeBuffer(&parametros);
		tamfrente = mFg.ExtraeStringDeBuffer(&parametros);
		tamalto = mFg.ExtraeStringDeBuffer(&parametros);
		tamfondo = mFg.ExtraeStringDeBuffer(&parametros);
		distprevio = mFg.ExtraeStringDeBuffer(&parametros);
		distfrentecalle = mFg.ExtraeStringDeBuffer(&parametros);

		numniveles_int=StrToInt(numniveles);
		numfondos_int=StrToInt(numfondos);


		instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++] = "START TRANSACTION";

		if (tarea == "A" || tarea == "I") { // ALTA

			if (tarea == "I") { // Si es alta con insertar entonces reasigna la numeración para dar espacio al numero que se va a insertar.
				// Actualiza los datos del frente.
				instruccion.sprintf("UPDATE almacenfrentes \
								 set numfrente=numfrente+1 \
							   where idalmcalle=%s AND numfrente>=%s \
							   ORDER BY numfrente desc",
							idalmcalle, numfrente);
				instrucciones[num_instrucciones++]=instruccion;
			}

			instruccion.sprintf("INSERT INTO almacenfrentes \
					   (numfrente, idalmcalle, \
						numniveles, numfondos, \
						tamfrente, tamalto, tamfondo, \
						distprevio, distfrentecalle) \
						   values (%s, %s, %s, %s, %s, %s, %s, %s, %s) ",
					numfrente, idalmcalle, numniveles, numfondos, tamfrente, tamalto, tamfondo, distprevio, distfrentecalle );
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("set @idalmfrente=LAST_INSERT_ID()");
			instrucciones[num_instrucciones++]=instruccion;

			// Inserta todas las posiciones del frente.
			for (int n=1; n<=numniveles_int; n++ ) {
				for (int f=1; f<=numfondos_int; f++ ) {
					instruccion.sprintf("insert into almacenposiciones (idalmfrente, nivel, fondo) values(@idalmfrente, %d, %d)", n,f);
					instrucciones[num_instrucciones++] = instruccion;
				}
			}
		}
		else {	// MODIFICACION

			// Consulta los niveles y fondos que hay antes de la modificación.
			int numniveles_antes, numfondos_antes;
			BufferRespuestas* resp_frente=NULL;
			try {
				instruccion.sprintf("select @idalmfrente:=idalmfrente, numniveles, numfondos from almacenfrentes where numfrente=%s and idalmcalle=%s", numfrente, idalmcalle);
				if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_frente)) {
					if (resp_frente->ObtieneNumRegistros()>0){
						numniveles_antes=StrToInt(resp_frente->ObtieneDato("numniveles"));
						numfondos_antes=StrToInt(resp_frente->ObtieneDato("numfondos"));
					} else throw (Exception("No se encuentra registro del frente en tabla almacenfrentes"));
				} else throw (Exception("Error al consultar en tabla almacenfrentes"));
			} __finally {
				if (resp_frente!=NULL) delete resp_frente;
			}

			// Actualiza los datos del frente.
			instruccion.sprintf("UPDATE almacenfrentes \
						   set numniveles=%s, numfondos=%s, \
								tamfrente=%s, tamalto=%s, tamfondo=%s, \
								distprevio=%s, distfrentecalle=%s \
						   where idalmfrente=@idalmfrente ",
						numniveles, numfondos, tamfrente, tamalto, tamfondo, distprevio, distfrentecalle);
			instrucciones[num_instrucciones++]=instruccion;

			// Borra las familias asignadas a las posiciones del frente que se borraran
			instruccion.sprintf("DELETE apf from almacenposfamilias apf	\
				INNER JOIN almacenposiciones ap ON apf.idalmposicion=apf.idalmposicion AND apf.idalmposicion=ap.idalmposicion \
				where ap.idalmfrente=@idalmfrente and (ap.nivel>%d or ap.fondo>%d)", numniveles_int, numfondos_int);
			instrucciones[num_instrucciones++] = instruccion;

			// Borra todas las posiciones del frente que ya no deben existir.
			instruccion.sprintf("delete from almacenposiciones where idalmfrente=@idalmfrente and (nivel>%d or fondo>%d) ", numniveles_int, numfondos_int);
			instrucciones[num_instrucciones++] = instruccion;

			// Inserta todas las posiciones del frente que aun no existen.
			for (int n=1; n<=numniveles_int; n++ ) {
				for (int f=1; f<=numfondos_int; f++ ) {
					if (n>numniveles_antes || f>numfondos_antes) {
						instruccion.sprintf("insert into almacenposiciones (idalmfrente, nivel, fondo) values(@idalmfrente, %d, %d)", n,f);
						instrucciones[num_instrucciones++] = instruccion;
					}
				}
			}

		}

		instrucciones[num_instrucciones++] = "COMMIT";

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

//---------------------------------------------------------------------------
// ID_BAJ_ALMACENFRENTE
void ServidorAlmacen::BajaAlmacenDistribucionFrente(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// BORRA FRENTE
	char *buffer_sql = new char[1024 * 50];
	char *aux_buffer_sql = buffer_sql;
	AnsiString instrucciones[50];
	AnsiString instruccion;
	int num_instrucciones = 0;
	AnsiString numfrente, idalmcalle;
	int i;

	try {
		numfrente = mFg.ExtraeStringDeBuffer(&parametros);
		idalmcalle = mFg.ExtraeStringDeBuffer(&parametros);

		instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++] = "START TRANSACTION";

		instruccion.sprintf("select @idalmfrente:=idalmfrente from almacenfrentes where numfrente=%s and idalmcalle=%s", numfrente, idalmcalle);
		instrucciones[num_instrucciones++]=instruccion;

		// Borra las familias asignadas a las posiciones del frente.
		instruccion.sprintf("DELETE apf from almacenposfamilias apf	\
			INNER JOIN almacenposiciones ap ON apf.idalmposicion=apf.idalmposicion AND apf.idalmposicion=ap.idalmposicion \
			where ap.idalmfrente=@idalmfrente");
		instrucciones[num_instrucciones++] = instruccion;


		// Borra todas las posiciones del frente
		instruccion.sprintf("delete from almacenposiciones where idalmfrente=@idalmfrente");
		instrucciones[num_instrucciones++] = instruccion;

		// Borra el frente
		instruccion.sprintf("delete from almacenfrentes where idalmfrente=@idalmfrente");
		instrucciones[num_instrucciones++] = instruccion;

		instrucciones[num_instrucciones++] = "COMMIT";

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


//---------------------------------------------------------------------------
// ID_GRA_ALMACENFAMILIA
void ServidorAlmacen::GrabaAlmacenDistribucionFamilia(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{


	// GRABA ALMACEN-FAMILIA
	char *buffer_sql = new char[1024 * 50];
	char *aux_buffer_sql = buffer_sql;
	AnsiString instrucciones[50];
	AnsiString instruccion;
	int num_instrucciones = 0;
	AnsiString idalmposicion, familia;
	int i;

	try {
		idalmposicion = mFg.ExtraeStringDeBuffer(&parametros);
		familia = mFg.ExtraeStringDeBuffer(&parametros);

		instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++] = "START TRANSACTION";

		instruccion.sprintf("INSERT INTO almacenposfamilias \
				   (idalmposicion, clasif2) \
					   values (%s, '%s') ",
				idalmposicion, familia);
		instrucciones[num_instrucciones++]=instruccion;


		instrucciones[num_instrucciones++] = "COMMIT";

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


//---------------------------------------------------------------------------
// ID_BAJ_ALMACENFAMILIA
void ServidorAlmacen::BajaAlmacenDistribucionFamilia(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// BORRA ALMACEN-FAMILIA
	char *buffer_sql = new char[1024 * 50];
	char *aux_buffer_sql = buffer_sql;
	AnsiString instrucciones[50];
	AnsiString instruccion;
	int num_instrucciones = 0;
	AnsiString idalmposicion, familia;
	int i;

	try {
		idalmposicion = mFg.ExtraeStringDeBuffer(&parametros);
		familia = mFg.ExtraeStringDeBuffer(&parametros);

		instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++] = "START TRANSACTION";

		// Borra
		instruccion.sprintf("delete from almacenposfamilias where idalmposicion=%s and clasif2='%s'", idalmposicion, familia);
		instrucciones[num_instrucciones++] = instruccion;

		instrucciones[num_instrucciones++] = "COMMIT";

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
//---------------------------------------------------------------------------
//ID_MOD_ESTADO_UBICACIONES
void ServidorAlmacen::ModificarEstadoUbicaciones(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
 //nueva funcion

	char *buffer_sql=new char[1024*16];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instrucciones[8], instruccion;
	int num_instrucciones=0;
	AnsiString estado,frentes;
    int error=0,i;


	try{
		estado=mFg.ExtraeStringDeBuffer(&parametros);
		frentes=mFg.ExtraeStringDeBuffer(&parametros);

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		instruccion.sprintf("UPDATE almacenposiciones SET utilizable=%s WHERE idalmposicion IN (%s)", estado,frentes);
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


	}__finally{

		if(buffer_sql!=NULL) delete  buffer_sql;
    }
}
//---------------------------------------------------------------------------
//ID_ACTUALIZAR_ALMACENCONFIGSUC
void ServidorAlmacen::ModificarUbicacionesEspeciales(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*16];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instrucciones[8], instruccion;
	int num_instrucciones=0;
	AnsiString TipoActualizacion,ID,Sucursal;
    int error=0,i;


	try{

		Sucursal=mFg.ExtraeStringDeBuffer(&parametros);
		TipoActualizacion=mFg.ExtraeStringDeBuffer(&parametros);
		ID=mFg.ExtraeStringDeBuffer(&parametros);

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		if(TipoActualizacion=="CrossDocking")
		{
			instruccion.sprintf("UPDATE almacenconfigsuc set idposcross=%s WHERE sucursal='%s'",ID,Sucursal);
			instrucciones[num_instrucciones++]=instruccion;
		}

		if(TipoActualizacion=="PosicionCarga")
		{
			instruccion.sprintf("UPDATE almacenconfigsuc set idposcarga=%s WHERE sucursal='%s'",ID,Sucursal);
			instrucciones[num_instrucciones++]=instruccion;
		}

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


	}__finally{

		if(buffer_sql!=NULL) delete  buffer_sql;
    }
}
//---------------------------------------------------------------------------
// ID_MOD_LOTE_INVENTARIO
void ServidorAlmacen::ModLoteInventario(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{

	char *buffer_sql = new char[1024 * 50];
	char *aux_buffer_sql = buffer_sql;
	AnsiString instrucciones[50];
	AnsiString instruccion;
	int num_instrucciones = 0;
	AnsiString loteAnterior, loteNuevo,inventario;
	int i, error=0;
	TDate fecha=Today(),fechaInventario;
	TTime hora=Time();
	//2643
	try {
		loteAnterior = mFg.ExtraeStringDeBuffer(&parametros);
		loteNuevo = mFg.ExtraeStringDeBuffer(&parametros);
		inventario = mFg.ExtraeStringDeBuffer(&parametros);

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";


        //validacion de la fecha de creacion del inventario
		instruccion.sprintf("select @error:=if('%s'!=li.fechaalta, 1, 0) as error \
				from lotesinv li inner join dinventarios di on li.lote=di.lote \
				where li.lote='%s' and li.inventario='%s'", mFg.DateToMySqlDate(fecha),loteAnterior,inventario );
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 4, error);


		//si el invetario es del mismo dia, procede a actualizar el registro
		if(error==0){

			//instruccion solo de prueba, para no empezar a modificar los registros
			/*instruccion.sprintf(" select * from lotesinv li \
				inner join dinventarios di on li.lote=di.lote \
				where li.lote='%s' and li.inventario='%s' ",loteAnterior,inventario );
			instrucciones[num_instrucciones++]=instruccion; */

			// la tabla padre
			instruccion.sprintf(" update lotesinv li  set li.lote='%s' \
				where li.lote='%s' and li.inventario='%s' ",loteNuevo,loteAnterior,inventario );
			instrucciones[num_instrucciones++]=instruccion;

			//la tabla hija
			instruccion.sprintf(" update dinventarios di  set di.lote='%s' \
				where di.lote='%s' and di.inventario='%s' ",loteNuevo,loteAnterior,inventario );
			instrucciones[num_instrucciones++]=instruccion;


		}

       instrucciones[num_instrucciones++]="COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			instruccion.sprintf("select %d as error ", error);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}


	}
		__finally {
		delete buffer_sql;
	}
}
//---------------------------------------------------------------------------
// ID_MOVER_LOTE_DE_INVENTARIO
void ServidorAlmacen::MoverLoteInventario(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{

	char *buffer_sql = new char[1024 * 50];
	char *aux_buffer_sql = buffer_sql;
	AnsiString instrucciones[50];
	AnsiString instruccion;
	int num_instrucciones = 0;
	AnsiString lote, InventarioNuevo,inventario;
	int i, error=0;
	TDate fecha=Today(),fechaInventario;
	TTime hora=Time();
	//2643
	try {
		lote = mFg.ExtraeStringDeBuffer(&parametros);
		InventarioNuevo = mFg.ExtraeStringDeBuffer(&parametros);
		inventario = mFg.ExtraeStringDeBuffer(&parametros);

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";


		//validacion de la fecha de creacion del inventario
		instruccion.sprintf("select @error:=if('%s'!=li.fechaalta, 1, 0) as error \
				from lotesinv li inner join dinventarios di on li.lote=di.lote \
				where li.lote='%s' and li.inventario='%s'", mFg.DateToMySqlDate(fecha),lote,inventario );
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 4, error);

		//validacion de existencia del inventarionuevo al que se cambiara el lote
		/*instruccion.sprintf("select @error:=if('%s'!=i.inventario, 1, 0) as error, @fechaInventario:=i.fechainv \
				FROM inventarios i , lotesinv li \
				where  i.inventario= li.inventario",InventarioNuevo, mFg.DateToMySqlDate(fecha), mFg.DateToMySqlDate(fecha));*/
		instruccion.sprintf("select \
		@error:=if(EXISTS(SELECT @fechaInv:=i.fechainv FROM inventarios i WHERE i.inventario='%s'),0 ,1) as error",InventarioNuevo);
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 3, error);


		//validacion de que ambos inventarios sean de la misma fecha
		instruccion.sprintf("select @error:=if(@fechaInv!=i.inventario, 1, 0) as error \
				FROM inventarios i \
				where i.inventario='%s' ", inventario);
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 2, error);

		//si el invetario es del mismo dia, procede a actualizar el registro
		if(error==0){

			//instruccion solo de prueba, para no empezar a modificar los registros
			/*instruccion.sprintf(" SELECT i.fechainv, i.inventario, i.tipo, i.almacen, i.descripcion \
								FROM inventarios i \
								WHERE i.inventario='%s' ",inventario );
			instrucciones[num_instrucciones++]=instruccion;*/

			// la tabla padre
			instruccion.sprintf(" UPDATE inventarios i ,lotesinv li \
			SET li.inventario='%s' \
			 WHERE  i.inventario=li.inventario AND i.inventario='%s' and li.lote='%s' ",InventarioNuevo,inventario,lote );
			instrucciones[num_instrucciones++]=instruccion;

			/*
				UPDATE lotesinv SET fechaalta='2019-11-26', fechamodi='2019-11-26' WHERE inventario='VI000000009';

				UPDATE inventarios SET fechaalta='2019-11-26', fechamodi='2019-11-26', fechainv='2019-11-26' WHERE inventario='VI000000009';

				SELECT i.fechainv, i.inventario, i.tipo, i.almacen, i.descripcion FROM inventarios i WHERE i.inventario='S1000000135' ;
			*/


		}

	   instrucciones[num_instrucciones++]="COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			instruccion.sprintf("select %d as error ", error);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}


	}
		__finally {
		delete buffer_sql;
	}
}
//---------------------------------------------------------------------------
// ID_EJE_AUDITARMOVTRANSF
void ServidorAlmacen::AuditarMovTranfs(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString fecha_inicial, fecha_final, movimiento, tipo, estatus, sucursal, concepto;
	AnsiString condicion_movi=" ",condicion_tipo=" ", condicion_sucursal=" ", condicion_fechas=" ", condicion_estatus=" ", condicion_concepto=" ";
	AnsiString instruccion;

	fecha_inicial =mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
	fecha_final	  =mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
	movimiento	  =mFg.ExtraeStringDeBuffer(&parametros);
	tipo		  =mFg.ExtraeStringDeBuffer(&parametros);
	estatus		  =mFg.ExtraeStringDeBuffer(&parametros);
	sucursal	  =mFg.ExtraeStringDeBuffer(&parametros);
	concepto	  =mFg.ExtraeStringDeBuffer(&parametros);

	if (movimiento!=" ") {
		condicion_movi.sprintf(" AND t.movimiento = '%s' ", movimiento);
	}

	if (tipo!=" ") {
		condicion_tipo.sprintf(" AND t.tipo = '%s' ", tipo);
	}

	if (estatus!=" ") {
		condicion_estatus.sprintf(" AND t.auditado = '%s' ", estatus);
	}

	if (sucursal!=" ") {
		condicion_sucursal.sprintf(" AND (t.sucent = '%s' OR t.sucsal = '%s') ", sucursal, sucursal);
	}

	if (concepto!=" ") {
		condicion_concepto.sprintf(" AND t.concepto = '%s' ", concepto);
	}

	condicion_fechas.sprintf(" AND (t.fecha BETWEEN '%s' AND '%s' OR t.fechamodi BETWEEN '%s' AND '%s') ",
	fecha_inicial, fecha_final, fecha_inicial, fecha_final);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_REP);
	// Resultado final
	instruccion.sprintf("SELECT                                              \
	t.movimiento,                                                            \
	t.concepto,                                                              \
	t.descripcion,                                                           \
	t.folioenvio,                                                            \
	t.fecha,                                                                 \
	t.fechaalta,                                                             \
	t.horaalta,                                                              \
	t.fechamodi,                                                             \
	t.horamodi,                                                              \
	t.almaent,                                                               \
	t.almasal,                                                               \
	t.sucent,                                                                \
	t.sucsal,                                                                \
	IF(t.auditado='0', 'NO', 'Si') AS auditado,                               \
	t.usualta,                                                               \
	t.usumodi,                                                               \
	t.tipo,                                                                  \
	t.auditadopor,                                                            \
	t.fechaaudi,                                                    		\
	t.horaaudi                                                        		\
	FROM (                                                                   \
			  (                                                              \
				SELECT                                                       \
				m.movimiento,                                                \
				m.concepto,                                                  \
				cm.descripcion,                                              \
				m.folioenvio,                                                \
				m.fechamov AS fecha,                                         \
				m.fechaalta AS fechaalta,                                    \
				m.horaalta AS horaalta,                                      \
				m.fechamodi AS fechamodi,                                    \
				m.horamodi AS horamodi,                                      \
				m.almaent AS almaent,                                        \
				m.almasal AS almasal,                                        \
				IFNULL(sece.sucursal,'') AS sucent,                          \
				IFNULL(secs.sucursal,'') AS sucsal,                          \
				m.auditado,                                                   \													  \
				CONCAT(em.nombre, ' ', em.appat, ' ', em.apmat) AS usualta,  \
				CONCAT(emp.nombre, ' ', emp.appat, ' ', emp.apmat) AS usumodi,\
				'M' AS tipo,                                                  \
				m.auditadopor,                                                \
				m.fechaaudi,                                                 \
				m.horaaudi                                                   \
				FROM movalma m                                               \
				INNER JOIN empleados em ON em.empleado = m.usualta           \
				INNER JOIN empleados emp ON emp.empleado = m.usumodi         \
				INNER JOIN conceptosmovalma AS cm ON cm.concepto = m.concepto\
				LEFT JOIN almacenes ae ON m.almaent = ae.almacen             \
				LEFT JOIN secciones sece ON ae.seccion = sece.seccion        \
				LEFT JOIN almacenes asal ON m.almasal = asal.almacen         \
				LEFT JOIN secciones secs ON asal.seccion = secs.seccion      \
				WHERE                                                        \
				m.cancelado = 0                                              \
				AND m.transfor = ''                                          \
			  )                                                              \
			  UNION ALL                                                      \
			  (                                                              \
			  SELECT                                                         \
				t.transfor,                                                  \
				m.concepto,                                                  \
				cm.descripcion,                                              \
				m.folioenvio,                                                \
				t.fechatrans AS fecha,                                       \
				t.fechaalta AS fechaalta,                                    \
				t.horaalta AS horaalta,                                      \
				t.fechamodi AS fechamodi,                                    \
				t.horamodi AS horamodi,                                      \
				IFNULL(m.almaent,'') AS almaent,                             \
				IFNULL(m.almasal,'') AS almasal,                             \
				IFNULL(sece.sucursal,'') AS sucent,                          \
				IFNULL(secs.sucursal,'') AS sucsal,                          \
				t.auditado,                                                   \                          \
				CONCAT(em.nombre, ' ', em.appat, ' ', em.apmat) AS usualta,  \
				CONCAT(emp.nombre, ' ', emp.appat, ' ', emp.apmat) AS usumodi,\
				'T' AS tipo,                                                 \
				m.auditadopor,                                               \
				m.fechaaudi,                                                 \
				m.horaaudi                                                   \
				FROM transformacion t                                        \
				INNER JOIN movalma m ON t.transfor = m.transfor              \
				INNER JOIN empleados em ON em.empleado = t.usualta           \
				INNER JOIN empleados emp ON emp.empleado = t.usumodi         \
				INNER JOIN conceptosmovalma AS cm ON cm.concepto = m.concepto\
				LEFT JOIN almacenes ae ON m.almaent = ae.almacen             \
				LEFT JOIN secciones sece ON ae.seccion = sece.seccion        \
				LEFT JOIN almacenes asal ON m.almasal = asal.almacen         \
				LEFT JOIN secciones secs ON asal.seccion = secs.seccion      \
				WHERE                                                        \
				t.cancelado = 0                                              \
				AND m.transfor != ''                                         \
				GROUP BY t.transfor                                          \
			  )                                                              \
		  ) t                                                                \
	WHERE 1 %s %s %s %s %s %s \
	ORDER BY t.fecha DESC, t.horamodi DESC ",
	condicion_fechas,
	condicion_movi,
	condicion_tipo,
	condicion_sucursal,
	condicion_estatus,
	condicion_concepto
	);
	if(!mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado)) {
		throw (Exception("Error al consultar en tabla almacenfrentes"));
	}
}
//---------------------------------------------------------------------------
//ID_UPDT_AUDITADO
void ServidorAlmacen::UpdateAuditado(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*30*10];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i;

	AnsiString Instruccion, instrucciones[10000];
	AnsiString movimientos, transformaciones, auditado, usuario;

	movimientos = mFg.ExtraeStringDeBuffer(&parametros);
	transformaciones = mFg.ExtraeStringDeBuffer(&parametros);
	auditado = mFg.ExtraeStringDeBuffer(&parametros);
	usuario = mFg.ExtraeStringDeBuffer(&parametros);

	//iniciar la transaccion
	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	try{

		if (movimientos.Length()>0) {

			Instruccion.sprintf("UPDATE movalma \
			SET auditado = %s, \
			auditadopor = '%s', \
			fechaaudi = CURDATE(), \
			horaaudi = CURTIME() \
			WHERE movimiento IN (%s) ",
			auditado,usuario,movimientos );
			instrucciones[num_instrucciones++]=Instruccion;
		}


		if (transformaciones.Length()>0) {

			Instruccion.sprintf("UPDATE transformacion \
			SET auditado = %s                        \
			WHERE transfor IN (%s) ",
			auditado,transformaciones );
			instrucciones[num_instrucciones++]=Instruccion;

			Instruccion.sprintf("UPDATE movalma \
			SET auditado = %s,\
			auditadopor = '%s', \
			fechaaudi = CURDATE(), \
			horaaudi = CURTIME() \
			WHERE movimiento IN (%s) ",
			auditado,usuario,transformaciones );
			instrucciones[num_instrucciones++]=Instruccion;
		}

		instrucciones[num_instrucciones++]="COMMIT";

		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
		aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);
	} __finally {
		if(buffer_sql != NULL) delete buffer_sql;
	}
}
//---------------------------------------------------------------------------
//ID_EJE_BITA_MODMOVALMA
void ServidorAlmacen::EjecutaRepBitacoraMovAlma(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
	AnsiString movimiento;
	char *resultado;

	movimiento = mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene la información de la cabecera del movimiento
	instruccion.sprintf("SELECT \
		m.movimiento, \
		IF(m.tipo='T','Traspaso',IF(m.tipo='S','Salida','Entrada')) AS tipo, \
		m.concepto, \
		m.folioenvio, \
		m.fechamov, \
		CONCAT(e.nombre, ' ', e.appat, ' ', e.apmat) AS chofer, \
		pro.razonsocial, \
		alme.nombre AS almaentrada, \
		alms.nombre AS almasalida, \
		IF(m.auditado=1,'AUDITADO','') AS auditado \
	FROM movalma m \
	LEFT JOIN empleados e ON e.empleado = m.chofer \
	LEFT JOIN proveedores pro ON pro.proveedor = m.proveedor \
	LEFT JOIN almacenes alme ON alme.almacen = m.almaent \
	LEFT JOIN almacenes alms ON alms.almacen = m.almasal \
	WHERE m.movimiento='%s'", movimiento);

	resultado=Respuesta->ObtieneDirLibreBuffer();
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	BufferRespuestas resp_movinfo(resultado);

	// Obtiene todos los datos del proveedor del pedido
	instruccion.sprintf("SELECT \
		a.articulo, \
		a.producto, \
		pr.nombre, \
		a.present, \
		dm.cantidad, \
		dm.costo, \
		dm.costobase \
	FROM movalma m \
	INNER JOIN dmovalma dm ON dm.movimiento = m.movimiento \
	INNER JOIN articulos a ON a.articulo = dm.articulo \
	INNER JOIN productos pr ON pr.producto = a.producto \
	WHERE m.movimiento = '%s' \
	ORDER BY pr.nombre, a.present, a.factor DESC", movimiento);

	resultado=Respuesta->ObtieneDirLibreBuffer();
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	BufferRespuestas resp_detmovinfo(resultado);

	// Obtiene todos los datos del proveedor del pedido
	instruccion.sprintf("SELECT \
        bm.idbitacora, \
		bm.movimiento, \
		bm.fecha, \
		bm.hora \
	FROM bitacoramovalmamod bm \
	WHERE bm.movimiento = '%s' \
	ORDER BY bm.idbitacora DESC, bm.fecha DESC, bm.hora DESC ", movimiento);

	resultado=Respuesta->ObtieneDirLibreBuffer();
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	BufferRespuestas resp_bitamov(resultado);

}
//---------------------------------------------------------------------------
//ID_EJE_BITADET_MODMOVALMA
void ServidorAlmacen::EjecutaRepBitacoraDetMovAlma(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
	AnsiString idBitacora, movimiento;
	char *resultado;

	idBitacora = mFg.ExtraeStringDeBuffer(&parametros);
	movimiento = mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene la información de la cabecera del movimiento
	instruccion.sprintf("SELECT \
		bm.movimiento, \
		IF(m.tipo='T','Traspaso',IF(m.tipo='S','Salida','Entrada')) AS tipo, \
		m.concepto, \
		m.folioenvio, \
		bm.fechamov, \
		CONCAT(e.nombre, ' ', e.appat, ' ', e.apmat) AS chofer, \
		pro.razonsocial, \
		alme.nombre AS almaentrada, \
		alms.nombre AS almasalida, \
		IF(m.auditado=1,'AUDITADO','') AS auditado \
		,IF(bm.aplica IS NULL ,CONCAT(IF( m.aplica=1,'SI*','NO*')) ,IF( bm.aplica=1,'SI','NO')) AS aplicado \
	FROM bitacoramovalmamod bm \
	INNER JOIN movalma m ON m.movimiento = bm.movimiento \
	LEFT JOIN empleados e ON e.empleado = m.chofer \
	LEFT JOIN proveedores pro ON pro.proveedor = m.proveedor \
	LEFT JOIN almacenes alme ON alme.almacen = bm.almaent \
	LEFT JOIN almacenes alms ON alms.almacen = bm.almasal \
	WHERE bm.idbitacora = %s", idBitacora);

	resultado=Respuesta->ObtieneDirLibreBuffer();
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	BufferRespuestas resp_movinfo(resultado);

	// Obtiene todos los datos del proveedor del pedido
	/*instruccion.sprintf("SELECT \
		a.articulo, \
		a.producto, \
		pr.nombre, \
		a.present, \
		bdm.cantidad, \
		bdm.costo, \
		bdm.costobase \
	FROM bitacoramovalmamod bm \
	INNER JOIN bitacoradmovalmamod bdm ON bdm.idbitacora = bm.idbitacora \
	INNER JOIN articulos a ON a.articulo = bdm.articulo \
	INNER JOIN productos pr ON pr.producto = a.producto \
	WHERE bm.idbitacora = %s \
	ORDER BY pr.nombre, a.present, a.factor DESC", idBitacora);*/

	instruccion.sprintf("SELECT t.articulo, t.producto, t.nombre, t.present, \
	SUM(t.cantidad) AS cantidad, SUM(t.cantidaddif) AS cantidaddif, \
	SUM(t.costo) AS costo, SUM(t.costobase) AS costobase \
	FROM (( \
	SELECT \
		a.articulo, \
		a.producto, \
		pr.nombre, \
		a.present, \
		bdm.cantidad, \
		0 AS cantidaddif, \
		bdm.costo, \
		0 AS costodif, \
		bdm.costobase, \
		0 AS costobasedif, \
		a.factor \
	FROM bitacoramovalmamod bm \
	INNER JOIN bitacoradmovalmamod bdm ON bdm.idbitacora = bm.idbitacora \
	INNER JOIN articulos a ON a.articulo = bdm.articulo \
	INNER JOIN productos pr ON pr.producto = a.producto \
	WHERE bm.idbitacora = %s \
	ORDER BY pr.nombre, a.present, a.factor DESC \
	)UNION ALL( \
	SELECT \
		a.articulo, \
		a.producto, \
		pr.nombre, \
		a.present, \
		0 AS cantidad, \
		(dm.cantidad) AS cantidaddif, \
		0 AS costo, \
		(dm.costo) AS costodif, \
		0 AS costobase, \
		(dm.costobase) AS costobasedif, \
		a.factor \
	FROM movalma bm \
	INNER JOIN dmovalma dm ON dm.movimiento = bm.movimiento \
	INNER JOIN articulos a ON a.articulo = dm.articulo \
	INNER JOIN productos pr ON pr.producto = a.producto \
	WHERE bm.movimiento = '%s' \
	ORDER BY pr.nombre, a.present, a.factor DESC \
	)) t GROUP BY t.articulo ORDER BY t.nombre, t.present, t.factor DESC", idBitacora, movimiento);

	resultado=Respuesta->ObtieneDirLibreBuffer();
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	BufferRespuestas resp_bitadetmov(resultado);

}
// ---------------------------------------------------------------------------
// ID_APLICA_MOVALMA
void ServidorAlmacen::EjecutaAplicaMovAlma(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros){

		char *buffer_sql=new char[1024*64*200];
		char *aux_buffer_sql=buffer_sql;

		BufferRespuestas* resp_detmov=NULL;
		BufferRespuestas* resp_variableaplica=NULL;

		int num_instrucciones=0;
		int i;
		int num_partidas;
		int error = 0;

		AnsiString instruccion, instrucciones[15000];
		AnsiString clave, usuario, tipo, articulo;
		AnsiString variableaplica, variableaplicaactual;

	try{
        clave = mFg.ExtraeStringDeBuffer(&parametros);
		usuario = mFg.ExtraeStringDeBuffer(&parametros);
		variableaplica = mFg.ExtraeStringDeBuffer(&parametros);
		tipo = mFg.ExtraeStringDeBuffer(&parametros);

		try {
			instruccion.sprintf("SELECT aplica from movalma WHERE movimiento='%s' ", clave);

			if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_variableaplica)) {
				if (resp_variableaplica->ObtieneNumRegistros()>0){
					variableaplicaactual = resp_variableaplica->ObtieneDato("aplica");
				}else {
					throw (Exception("No se encuentra registro en tabla movalma"));
				}

			} else{
					throw (Exception("Error al consultar en tabla movalma"));
			}

			if(variableaplicaactual == 1){
                   throw (Exception("El traspaso ya fue Aplicado"));
			}


		} __finally {
			if (resp_variableaplica!=NULL) delete resp_variableaplica;
		}

		instruccion.sprintf("select @error:=if(m.fechamov<=cast(e.valor as datetime), 1, 0) as error from movalma m left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' where m.movimiento='%s'",FormServidor->ObtieneClaveSucursal(), clave);
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 3, error);

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		instruccion.sprintf("set @folio='%s'", clave);
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("select @status:=cancelado from movalma where movimiento=@folio ");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("UPDATE movalma SET    \
									aplica=1,         \
									usumodi='%s',    \
									fechamodi=CURDATE(), \
									horamodi=CURTIME()    \
									where movimiento=@folio",
									usuario);
		instrucciones[num_instrucciones++]=instruccion;


		instruccion.sprintf("INSERT INTO bitacoramovalma                       \
								(idbitacoramovalma, movimiento, usuario, fecha, hora, \
								tipo_tarea, aplicado_ant, aplicado_act, cancelado)  \
								VALUES (NULL, @folio, '%s',CURDATE(), CURTIME(), 'M' ,%s ,%s , @status)",
								usuario, variableaplicaactual, variableaplica);
		instrucciones[num_instrucciones++] = instruccion;

		   if(variableaplicaactual=="0"){

			 if(tipo == "T"){

				instruccion="CREATE TEMPORARY TABLE tmpcantidad(referencia VARCHAR(11) NOT NULL,\
								producto VARCHAR(8) NOT NULL, present VARCHAR(255) NOT NULL, almacen VARCHAR(4) NOT NULL,  \
								cantidad DECIMAL(12,3) NOT NULL, PRIMARY KEY (referencia, producto, present, almacen))";
				instrucciones[num_instrucciones++]=instruccion;

				instruccion="CREATE TEMPORARY TABLE tmpcantidad2(referencia VARCHAR(11) NOT NULL,\
								producto VARCHAR(8) NOT NULL, present VARCHAR(255) NOT NULL, almacen VARCHAR(4) NOT NULL,  \
								cantidad DECIMAL(12,3) NOT NULL, PRIMARY KEY (referencia, producto, present, almacen))";
				instrucciones[num_instrucciones++]=instruccion;

				instruccion = "INSERT INTO tmpcantidad SELECT d.movimiento, a.producto, a.present, m.almaent, \
								SUM(d.cantidad * a.factor) AS cantidad FROM dmovalma d INNER JOIN movalma m ON   \
								d.movimiento = m.movimiento INNER JOIN articulos a ON a.articulo = d.articulo \
								WHERE d.movimiento = @folio GROUP BY a.producto, a.present";
				instrucciones[num_instrucciones++]=instruccion;

				instruccion = "INSERT INTO tmpcantidad2 SELECT d.movimiento, a.producto, a.present, m.almasal, \
								SUM(d.cantidad * a.factor) AS cantidad FROM dmovalma d INNER JOIN movalma m ON   \
								d.movimiento = m.movimiento INNER JOIN articulos a ON a.articulo = d.articulo \
								WHERE d.movimiento = @folio GROUP BY a.producto, a.present";
				instrucciones[num_instrucciones++]=instruccion;


				if(variableaplicaactual=="1"){
					instruccion = "UPDATE tmpcantidad tmp INNER JOIN existenciasactuales ea ON ea.producto = tmp.producto \
										AND ea.present = tmp.present AND ea.almacen = tmp.almacen SET ea.cantidad = (ea.cantidad  - tmp.cantidad) \
										, ea.entradas = (ea.entradas - tmp.cantidad) ";
					instrucciones[num_instrucciones++]=instruccion;

					instruccion = "UPDATE tmpcantidad2 tmp INNER JOIN existenciasactuales ea ON ea.producto = tmp.producto \
										AND ea.present = tmp.present AND ea.almacen = tmp.almacen SET ea.cantidad = (ea.cantidad  + tmp.cantidad) \
										, ea.salidas = (ea.salidas - tmp.cantidad)";
					instrucciones[num_instrucciones++]=instruccion;
				}

			 }
		   }

		   instruccion.sprintf("SELECT articulo, cantidad, costo, costobase FROM dmovalma WHERE movimiento='%s'",clave);
		   mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_detmov);

		   if (resp_detmov->ObtieneNumRegistros()>0){
				for (int m = 0; m < resp_detmov->ObtieneNumRegistros() ; m++) {

					articulo = resp_detmov->ObtieneDato("articulo");

					if(variableaplicaactual == "0"){
						if(tipo == "T"){
							instruccion.sprintf("UPDATE articulos a INNER JOIN movalma m ON m.movimiento = @folio \
								INNER JOIN existenciasactuales ea ON a.producto = ea.producto AND a.present = ea.present   \
								AND m.almaent = ea.almacen INNER JOIN dmovalma d ON m.movimiento = d.movimiento AND \
								a.articulo = d.articulo	SET ea.cantidad = (ea.cantidad + (d.cantidad * a.factor)) \
								, ea.entradas = (ea.entradas + (d.cantidad * a.factor))  \
								WHERE a.articulo = '%s' AND m.movimiento = @folio ", articulo);
							instrucciones[num_instrucciones++]=instruccion;

							instruccion.sprintf("UPDATE articulos a INNER JOIN movalma m ON m.movimiento = @folio \
								INNER JOIN existenciasactuales ea ON a.producto = ea.producto AND a.present = ea.present   \
								AND m.almasal = ea.almacen INNER JOIN dmovalma d ON m.movimiento = d.movimiento AND \
								a.articulo = d.articulo	SET ea.cantidad = (ea.cantidad - (d.cantidad * a.factor)) \
								, ea.salidas = (ea.salidas + (d.cantidad * a.factor)) \
								WHERE a.articulo = '%s' AND m.movimiento = @folio ", articulo);
							instrucciones[num_instrucciones++]=instruccion;
						}
					}

					resp_detmov->IrAlSiguienteRegistro();
				}
		   }

		   instrucciones[num_instrucciones++]="COMMIT";

		   aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);

		   for (i=0; i<num_instrucciones; i++){
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);
		   }

            mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);

            if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
				instruccion.sprintf("select %d as error, @folio as folio", error);
				mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
			}

	} __finally {
		delete buffer_sql;
		if (resp_detmov!=NULL){
		   delete resp_detmov;
		}

	}
}
// ---------------------------------------------------------------------------
// ID_GRA_ALMACENPRODUCTO
void ServidorAlmacen::GrabaAlmacenDistribucionProductos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{


	// GRABA ALMACEN-FAMILIA
	char *buffer_sql = new char[1024 * 50];
	char *aux_buffer_sql = buffer_sql;
	AnsiString instrucciones[50];
	AnsiString instruccion;
	int num_instrucciones = 0;
	AnsiString idalmposicion, articulo, present;
	int i;

	try {
		idalmposicion = mFg.ExtraeStringDeBuffer(&parametros);
		articulo = mFg.ExtraeStringDeBuffer(&parametros);
		present = mFg.ExtraeStringDeBuffer(&parametros);

		instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++] = "START TRANSACTION";

		instruccion.sprintf("INSERT INTO almacenposarticulos \
				   (idalmposicion, producto, present) \
					   values (%s, '%s', '%s') ",
				idalmposicion, articulo, present);
		instrucciones[num_instrucciones++]=instruccion;


		instrucciones[num_instrucciones++] = "COMMIT";

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
//---------------------------------------------------------------------------
// ID_BAJ_ALMACENPRODUCTO
void ServidorAlmacen::BajaAlmacenDistribucionProductos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// BORRA ALMACEN-FAMILIA
	char *buffer_sql = new char[1024 * 50];
	char *aux_buffer_sql = buffer_sql;
	AnsiString instrucciones[50];
	AnsiString instruccion;
	int num_instrucciones = 0;
	AnsiString idalmposicion, articulo, present;
	int i;

	try {
		idalmposicion = mFg.ExtraeStringDeBuffer(&parametros);
		articulo = mFg.ExtraeStringDeBuffer(&parametros);
		present =  mFg.ExtraeStringDeBuffer(&parametros);

		instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++] = "START TRANSACTION";

		// Borra
		instruccion.sprintf("delete from almacenposarticulos where\
			idalmposicion=%s and producto='%s' and present = '%s'",
			idalmposicion, articulo, present);
		instrucciones[num_instrucciones++] = instruccion;

		instrucciones[num_instrucciones++] = "COMMIT";

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
//---------------------------------------------------------------------------
// ID_MOD_NOMBRECALLE
void ServidorAlmacen::EditaNombreCalle(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    char *buffer_sql = new char[1024 * 50];
	char *aux_buffer_sql = buffer_sql;
	AnsiString instrucciones[50];
	AnsiString instruccion;
	int num_instrucciones = 0;
	AnsiString viejo, nuevo, almacen;
    int i;

	try {
		viejo = mFg.ExtraeStringDeBuffer(&parametros);
		nuevo = mFg.ExtraeStringDeBuffer(&parametros);
		almacen = mFg.ExtraeStringDeBuffer(&parametros);

		instruccion.sprintf("UPDATE almacencalles SET nombrecalle = '%s' WHERE nombrecalle = '%s' AND almacen = '%s'", nuevo, viejo, almacen);
		instrucciones[num_instrucciones++] = instruccion;

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

// ID_GRA_PRODUCTOFAMILIA
void ServidorAlmacen::GrabaAlmacenDistribucionProductosFamilia(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// GRABA PRODUCTO-FAMILIA
	char *buffer_sql = new char[1024 * 50];
	char *aux_buffer_sql = buffer_sql;
	AnsiString instrucciones[50];
	AnsiString instruccion;
	int num_instrucciones = 0;
	AnsiString idalmposicion, clasif1, clasif2, clasif3, sucursal;
	int i;

	try {
		idalmposicion = mFg.ExtraeStringDeBuffer(&parametros);
		clasif1 = mFg.ExtraeStringDeBuffer(&parametros);
		clasif2 = mFg.ExtraeStringDeBuffer(&parametros);
		clasif3 = mFg.ExtraeStringDeBuffer(&parametros);
		sucursal = mFg.ExtraeStringDeBuffer(&parametros);

		instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++] = "START TRANSACTION";

		instruccion.sprintf("INSERT INTO almacenposarticulos      \
							SELECT NULL, '%s' AS idalmposicion,  pro.producto, pre.present  \
							FROM productos pro                                            \
							INNER JOIN presentaciones pre ON pre.producto = pro.producto  \
							INNER JOIN clasificacion1 cla1 ON pro.clasif1 = cla1.clasif1  \
							INNER JOIN clasificacion2 cla2 ON pro.clasif2 = cla2.clasif2  \
							INNER JOIN clasificacion3 cla3 ON pro.clasif3 = cla3.clasif3 \
							INNER JOIN articulosxsuc ats ON ats.producto = pro.producto AND ats.present = pre.present   \
							WHERE cla1.clasif1 = %s AND cla2.clasif2 = %s AND cla3.clasif3 = %s AND ats.sucursal = '%s' ",
											idalmposicion, clasif1, clasif2, clasif3, sucursal);
		instrucciones[num_instrucciones++]=instruccion;


		instrucciones[num_instrucciones++] = "COMMIT";

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
