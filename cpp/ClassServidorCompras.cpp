#include <vcl.h>
//---------------------------------------------------------------------------
#include "pch.h"

#pragma hdrstop

#include <DateUtils.hpp>
#include "ClassServidorCompras.h"
#include "ClassServidorVioleta.h"
#include "ClassBufferRespuestas.h"
#include "violetaS.h"
#include "FormServidorVioleta.h"
#include "comunes.h"
#include "ClassArregloDetalle.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)
//ID_GRA_COMPRA
void ServidorCompras::GrabaCompra(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    //  GRABA COMPRA
	char *buffer_sql=new char[10000*1000];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString tarea_compra, clave_compra, usuario, terminal, sucursal;
	int num_partidas, i;
	int num_instrucciones=0;
	AnsiString instruccion;
	AnsiString * instrucciones = new AnsiString[10000];
	double anticipo, valor;
	AnsiString periodic;
	int dias_plazo;
	TDateTime fecha_inic, fecha_venc, fecha_com;
	int acredito;
	int error=0;
	AnsiString articulo, costo_art;
	TDate fecha=Today();
	TTime hora=Time();
	AnsiString tipobitacoracosto;
	AnsiString mensaje,esParteRel="";

	// Consulta de parámetro para asignación de precios
	// * Si es 0 el cambio de precio es inmediato y se guarda en el campo precios.precio
	// * si es 1 entonces el cambio de precios es diferido y se guarda precios.precioproximo para aplicarse posteriormente)
	AnsiString paramcambioprec;
	BufferRespuestas* resp_paramcambioprec=NULL;
	sucursal=FormServidor->ObtieneClaveSucursal();
	AnsiString foliofactura, proveedor, muui_compra, fecha_venc_ant, campo_ID,fechaini, horaini, numregdespues,numregantes, fechamodi, horamodi;
	BufferRespuestas* resp_info_compra=NULL;
	BufferRespuestas* resp_info_compra_2=NULL;
	AnsiString fecha_modi_comp, hora_modi_comp;
	AnsiString pedidofacturar;
	AnsiString recepcion, fraccionado;
	AnsiString empresa_sistema = FormServidor->ObtieneClaveEmpresa();


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
		clave_compra=mFg.ExtraeStringDeBuffer(&parametros); // Clave de la compra.
		tarea_compra=mFg.ExtraeStringDeBuffer(&parametros); // Indicador si se va a agregar o modificar.
		usuario=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que esta grabando la compra.
		terminal=mFg.ExtraeStringDeBuffer(&parametros); // Terminal desde la que se está grabando la compra.
		mensaje=mFg.ExtraeStringDeBuffer(&parametros); // Mensaje de la compra.
		fechaini=mFg.ExtraeStringDeBuffer(&parametros); //nuevos parametros de fecha de inicio de captura
		horaini=mFg.ExtraeStringDeBuffer(&parametros);  // nuevo paramaetro para hora de inicio de captura
		numregdespues=mFg.ExtraeStringDeBuffer(&parametros);
		numregantes=mFg.ExtraeStringDeBuffer(&parametros);
		fechamodi=mFg.ExtraeStringDeBuffer(&parametros); //Última fecha de modificación que consultó el cliente
		horamodi=mFg.ExtraeStringDeBuffer(&parametros); //Última hora de modificación que sonsultó el cliente
		pedidofacturar= mFg.ExtraeStringDeBuffer(&parametros); // Pedido que se va a facturar, vacio si no es facturacion de pedido
		recepcion= mFg.ExtraeStringDeBuffer(&parametros); //validar si hay recepcion
		fraccionado=mFg.ExtraeStringDeBuffer(&parametros);   //validar si la compra es fraccionada

		// Obtiene los datos de la tabla de compras
		datos.AsignaTabla("compras");
		parametros+=datos.InsCamposDesdeBuffer(parametros);
		// Extrae los datos que necesitamos para crear las letras y transacciones.
		datos.AsignaValorCampo("referencia", "@folio", 1);
		valor=StrToFloat(datos.ObtieneValorCampo("valor"));
		periodic=datos.ObtieneValorCampo("periodic");
		anticipo=StrToFloat(datos.ObtieneValorCampo("anticipo"));
		acredito=StrToInt(datos.ObtieneValorCampo("acredito"));
		fecha_com=StrToDate(datos.ObtieneValorCampo("fechacom"));
		foliofactura=datos.ObtieneValorCampo("folioprov");
		proveedor=datos.ObtieneValorCampo("proveedor");

		if (tarea_compra=="M") {
			try {
				instruccion.sprintf("SELECT fechavenc FROM compras WHERE referencia='%s'",clave_compra);
				if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_info_compra)) {
					if (resp_info_compra->ObtieneNumRegistros()>0){
						fecha_venc_ant=resp_info_compra->ObtieneDato("fechavenc");
					}
				}

			} __finally {
				if (resp_info_compra!=NULL) delete resp_info_compra;
			}

			try {
				instruccion.sprintf("SELECT fechamodi, horamodi FROM compras WHERE referencia='%s'",clave_compra);
				if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_info_compra_2)) {
					if (resp_info_compra_2->ObtieneNumRegistros()>0){
						fecha_modi_comp=resp_info_compra_2->ObtieneDato("fechamodi");
						hora_modi_comp=resp_info_compra_2->ObtieneDato("horamodi");
					}
				}

			} __finally {
				if (resp_info_compra_2!=NULL) delete resp_info_compra_2;
			}
		}

		//se verificara si el proveedor es parte relacionada
		if(proveedor!=" "){
			AnsiString proveedor_esParteRelac;
			BufferRespuestas* resp_prov=NULL;
			try{
				proveedor_esParteRelac.sprintf("SELECT * FROM proveedores WHERE proveedor='%s' ",proveedor );
				if(mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, proveedor_esParteRelac.c_str(), resp_prov)){
					if (resp_prov->ObtieneNumRegistros()>0){
					 esParteRel= resp_prov->ObtieneDato("esparterelac");

					}
				}else{
					throw (Exception("Error al consultar el proveedor parte relacionada ID_GRA_COMPRA"));
				}
		   }__finally{
				if (resp_prov!=NULL) delete resp_prov;
		   }

		}

		// Verificaciones para no permitir modificaciones cuando no sea permitido.
		if (tarea_compra=="M") {
			if(fechamodi!=fecha_modi_comp || horamodi!=hora_modi_comp)
			  	throw (Exception("No se puede grabar. Esta compra acaba de ser modificada, vuelva a cargar la compra para modificarla"));

			// Verifica que la fecha de la factura sea la fecha de compra previamente registrada.
			instruccion.sprintf("select @error:=if(c.fechacom<=cast(e.valor as datetime) , 1, 0) as error from compras c left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' where c.referencia='%s'",FormServidor->ObtieneClaveSucursal(), clave_compra);
			mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 6, error);

			// Verifica que no tenga pagos (sin tomar en cuenta el anticipo ni los pagos no cobrables)  ---------
			instruccion.sprintf("select @error:=if(COALESCE(sum(t.valor),0)<0, 1, 0) as error from compras c, transxpag t where c.referencia=t.referencia and c.referencia='%s' and c.cancelado=0 and t.cancelada=0 and t.valor<0 and t.tipo<>'ANTI' group by c.referencia", clave_compra);
			mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 2, error);

			//verificara si la compra ya tiene el UUID cargado y si es asi no permite guardar la compra
			instruccion.sprintf("SELECT @error:=IF(muuid<>'',1,0) AS error FROM compras c WHERE c.referencia='%s' AND c.cancelado=0 GROUP BY c.referencia", clave_compra);
			mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 5, error);

			// Verifica que la fecha compra de la factura sea posterior a la fecha de cierre
			instruccion.sprintf("select @error:=if('%s'<=cast(e.valor as datetime) , 1, 0) as error from compras c left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' where c.referencia='%s'", mFg.DateToMySqlDate(fecha_com),FormServidor->ObtieneClaveSucursal(), clave_compra);
			mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 3, error);

			tipobitacoracosto = "MC";

		} else {
			// Verifica que la fecha de la factura sea posterior a la fecha de cierre.
			instruccion.sprintf("select @error:=if('%s'<=cast(e.valor as datetime), 1, 0) as error from estadosistemaemp e where e.estado='FUCIERRE' AND e.sucursal = '%s' ", mFg.DateToMySqlDate(fecha_com), FormServidor->ObtieneClaveSucursal());
			mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 4, error);
			tipobitacoracosto = "C";
		}

		if (error==0) {
			if (periodic=="MES") throw(Exception("Todavía no se implementa la periodicidad mensual en compras"));
			if (periodic=="QUI") throw(Exception("Todavía no se implementa la periodicidad quincenal en compras"));
			if (periodic=="SEM") throw(Exception("Todavía no se implementa la periodicidad semanal en compras"));
			dias_plazo=StrToInt(periodic);

			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

            if (tarea_compra=="M") {
				/* Crea tabla temporal para alamacenar cantidades de articulos antes de */
				instruccion="CREATE temporary TABLE tmpcantidad(referencia VARCHAR(11) NOT NULL,\
				producto VARCHAR(8) NOT NULL, present VARCHAR(255) NOT NULL, almacen VARCHAR(4) NOT NULL,  \
				cantidad DECIMAL(12,3) NOT NULL, PRIMARY KEY (referencia, producto, present, almacen))";
				instrucciones[num_instrucciones++]=instruccion;

				instruccion.sprintf("INSERT INTO tmpcantidad SELECT d.referencia, a.producto, a.present, c.almacen,  \
				SUM(d.cantidad * a.factor) AS cantidad FROM dcompras d INNER JOIN compras c ON    \
				c.referencia = d.referencia INNER JOIN articulos a ON a.articulo = d.articulo WHERE d.referencia = '%s' \
				GROUP BY a.producto, a.present", clave_compra);
				instrucciones[num_instrucciones++]=instruccion;
			}

			// Obtiene el folio para la compra
			if (tarea_compra=="A") {
				instruccion.sprintf("select @folioaux:=valor from foliosemp where folio='COMPRAS' AND sucursal = '%s' %s ",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliosig=@folioaux+1");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @folioaux=cast(@folioaux as char)");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @folio=concat('%s', lpad(@folioaux,9,'0'))", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("update foliosemp set valor=@foliosig where folio='COMPRAS' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				
			} else {
				instruccion.sprintf("set @folio='%s'", clave_compra);
				instrucciones[num_instrucciones++]=instruccion;
			}

			// Todo se va a una letra que se salda automáticamente en el caso de las
			// compras de contado
			if (!acredito) {
				anticipo=valor;
				dias_plazo=0;
			}

			fecha_venc=fecha_com+dias_plazo;
			fecha_inic=fecha_venc;
			// Graba la cabecera en la tabla "compras"
			if (tarea_compra=="A") {
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
				//datos.InsCampo("saldo", "0");
				instrucciones[num_instrucciones++]=datos.GenerarSqlInsert();

               	// Guarda el mensaje
				if (mensaje!=" ") { // Un espacio lo tomamos como nulo.
					instruccion.sprintf("insert into comprasmensajes (referencia, mensaje) values (@folio,'%s')",
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
					instruccion.sprintf("replace into comprasmensajes (referencia, mensaje) values (@folio,'%s')",
						mensaje);
				} else {
					instruccion.sprintf("delete from comprasmensajes where referencia=@folio");
				}
				instrucciones[num_instrucciones++]=instruccion;
			}

			/*  fechaini
				horaini
				numregdespues
				numregantes */

			if (tarea_compra=="A") {
				TTime horainitime = StrToTime(horaini);
				instruccion.sprintf("INSERT INTO bitacoracomprasmoduuid (referencia, foliofactura, proveedor, usuario, fechaalta, horaalta, tipo, fechapagoantes, fechaini, horaini, numregistrosantes, usumodi ,fechamodi ,horamodi ) \
				VALUES (@folio,'%s', '%s','%s','%s', '%s','%s', '%s',  '%s', '%s', %d , '%s', '%s', '%s' )",
				foliofactura, proveedor, usuario, mFg.DateToMySqlDate(fecha_com), mFg.TimeToAnsiString(hora), tarea_compra, mFg.DateToMySqlDate(fecha_venc),mFg.StrToMySqlDate(fechaini), mFg.TimeToMySqlTime(horainitime), StrToInt( numregdespues ) , usuario, mFg.DateToMySqlDate(fecha_com), mFg.TimeToAnsiString(hora)   );
				instrucciones[num_instrucciones++]=instruccion;
			} else {
				instruccion.sprintf("INSERT INTO bitacoracomprasmoduuid (referencia, foliofactura, proveedor, usuario, usumodi, fechamodi, horamodi, tipo, fechapagoantes, fechapagodespues ,numregistrosantes, numregistrosdespues ) \
				VALUES (@folio, '%s', '%s', '%s', '%s', CURDATE(), CURTIME(), '%s', '%s', '%s', '%s', '%s')",
				foliofactura, proveedor, usuario, usuario, tarea_compra,  mFg.StrToMySqlDate(fecha_venc_ant) , mFg.StrToMySqlDate(fecha_venc_ant) , numregantes, numregdespues);
				instrucciones[num_instrucciones++]=instruccion;
            }

			// Graba el cargo por la compra en transxpag
			if (acredito) {
				if (tarea_compra=="A") {
						// Obtiene el folio para la NUEVA transaccion
						instruccion.sprintf("select @foliotranaux:=valor from foliosemp where folio='TRANPROV' AND sucursal = '%s' %s ",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
						instrucciones[num_instrucciones++]=instruccion;
						instruccion.sprintf("set @foliosigtran=@foliotranaux+1");
						instrucciones[num_instrucciones++]=instruccion;
						instruccion.sprintf("set @foliotranaux=cast(@foliotranaux as char)");
						instrucciones[num_instrucciones++]=instruccion;
						instruccion.sprintf("set @foliotran=concat('%s', lpad(@foliotranaux,9,'0'))", FormServidor->ObtieneClaveSucursal());
						instrucciones[num_instrucciones++]=instruccion;
						instruccion.sprintf("update foliosemp set valor=@foliosigtran where folio='TRANPROV' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
						instrucciones[num_instrucciones++]=instruccion;
						instruccion.sprintf("insert into transxpag \
							(tracredito, referencia, concepto, destino, tipo, cancelada, aplicada, valor, fechaalta, horaalta, fechamodi, horamodi, fechaapl, usualta, usumodi) \
							values (@foliotran, @folio, 'C', 'C', 'COMP', 0,1, %12.2f, '%s', '%s', '%s', '%s', '%s', '%s', '%s')",
							valor, mFg.DateToMySqlDate(fecha_com), mFg.TimeToMySqlTime(hora), mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), mFg.DateToMySqlDate(fecha_com), usuario, usuario );
						instrucciones[num_instrucciones++]=instruccion;
				} else {
						// Obtiene el folio de la transaccion ya existente
						instruccion.sprintf("select @foliotran:=tracredito from transxpag where referencia=@folio and concepto='C' and destino='C' and tipo='COMP' and cancelada=0");
						instrucciones[num_instrucciones++]=instruccion;
						instruccion.sprintf("update transxpag set valor=%12.2f, fechamodi='%s', horamodi='%s', fechaapl='%s', usumodi='%s', cancelada=0 where tracredito=@foliotran", valor, mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), mFg.DateToMySqlDate(fecha_com), usuario);
						instrucciones[num_instrucciones++]=instruccion;
				}
			}

			instruccion="create temporary table tmpcostos ( \
				articulo varchar(9), producto varchar(8), present varchar(255), \
				costo decimal(16,6), modificar bool, PRIMARY KEY (articulo)) Engine = InnoDB";
			instrucciones[num_instrucciones++]=instruccion;

			// Si se está modificando entonces borra el detalle y las letras que ya existan.
			if (tarea_compra=="M") {

				instruccion = "UPDATE tmpcantidad tmp INNER JOIN existenciasactuales ea ON ea.producto = tmp.producto \
				AND ea.present = tmp.present AND ea.almacen = tmp.almacen SET ea.cantidad = (ea.cantidad  - tmp.cantidad) \
				, ea.compras = (ea.compras - tmp.cantidad) ";
				instrucciones[num_instrucciones++]=instruccion;

				//Agrega productos de compra a tmpcostos
				instruccion.sprintf("INSERT INTO tmpcostos SELECT a.articulo, a.producto, a.present, d.costoimp,1 AS modificar \
				FROM dcompras d INNER JOIN compras c ON c.referencia = d.referencia \
				INNER JOIN articulos a ON a.articulo = d.articulo WHERE d.referencia = '%s' \
				GROUP BY a.articulo", clave_compra);
				instrucciones[num_instrucciones++]=instruccion;

				instruccion.sprintf("delete from dcompras where referencia=@folio");
				instrucciones[num_instrucciones++]=instruccion;
			}

			// Graba las partidas en "dcompras" y cambia los costos y los precios de cada artículo
			// involucrado.
			AnsiString lista_articulos = "";
			BufferRespuestas* resp_arteliminado=NULL;
			num_partidas=StrToInt(mFg.ExtraeStringDeBuffer(&parametros)); // Obtiene el número de partidas
			for (i=0; i<num_partidas; i++) {

				datos.AsignaTabla("dcompras");
				parametros+=datos.InsCamposDesdeBuffer(parametros);
				datos.InsCampo("referencia", "@folio", 1);
				if(esParteRel!="1")
					datos.InsCampo("id", i);
				/*else
					campo_ID=datos.ObtieneValorCampo("id");*/
				articulo=datos.ObtieneValorCampo("articulo");
				costo_art=datos.ObtieneValorCampo("costoimp");
				instrucciones[num_instrucciones++]=datos.GenerarSqlInsert();

				/* VALIDAR SI ALGÚN ARTICULO ESTA ELIMINADO*/
				try {
					instruccion.sprintf("SELECT articulo FROM articulos WHERE articulo = '%s'",articulo);
					if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_arteliminado)) {
						if (resp_arteliminado->ObtieneNumRegistros()==0)
							throw (Exception("El artículo del renglón #" + IntToStr(i+1) + " fue eliminado. Hay que quitarlo de la compra."));
					} else throw (Exception("Error al consultar los artículos"));
				} __finally {
					if (resp_arteliminado!=NULL) delete resp_arteliminado;
				}
			/*FIN VALIDAR SI ALGÚN ARTICULO ESTA ELIMINADO*/

				/*Evaluar si algún artículo esta inactivo*/
				if( i != num_partidas - 1)
					lista_articulos = lista_articulos + "'" + articulo + "', ";
					else
						lista_articulos = lista_articulos + "'" + articulo + "'";


				if (tarea_compra=="M") {
					instruccion.sprintf("replace into tmpcostos (articulo, costo, modificar) values ('%s', %s, 1)", articulo, costo_art);
					instrucciones[num_instrucciones++]=instruccion;
				}
				else{
					instruccion.sprintf("insert into tmpcostos (articulo, costo, modificar) values ('%s', %s, 1)", articulo, costo_art);
					instrucciones[num_instrucciones++]=instruccion;
				}
				instruccion.sprintf("UPDATE articulos a INNER JOIN compras c ON c.referencia = @folio \
				INNER JOIN existenciasactuales ea ON a.producto = ea.producto AND a.present = ea.present \
				AND c.almacen = ea.almacen INNER JOIN  \
				dcompras d ON c.referencia = d.referencia AND a.articulo = d.articulo \
				SET ea.cantidad = (ea.cantidad + (d.cantidad * a.factor)), ea.compras = (ea.compras + (d.cantidad * a.factor))  \
				WHERE a.articulo = '%s' AND c.referencia = @folio",articulo);
				instrucciones[num_instrucciones++]=instruccion;

				/*	por Alfredo Pedraza
					SE INSERTA EN TABLA DE stock SI ES EL PRIMER REGISTRO DE COMPRA PARA PODER TOMAR UN PUNTO DE REFERENCIA PRIMARIO PARA EL STOCK
					llenar con la primer compra el stock mínimo, reorden y máximo con base a la primer compra.
					25% de lo que se pidió sería el mínimo.		35% reorden.		100% Máximo.		*/
				instruccion.sprintf("INSERT INTO stock(sucursal, producto, present, minimo, maximo, reorden, diasrot) \
				SELECT * FROM (SELECT '%s' as sucursal, producto,  present, d.cantidad * 0.25 * a.factor  as minimo, \
				d.cantidad * a.factor as maximo, d.cantidad * 0.35 * a.factor as reorden, 0 as diasrot \
				from articulos as a inner join  dcompras as d on d.articulo=a.articulo where a.articulo='%s' \
				and d.referencia=@folio) AS aux \
				WHERE NOT EXISTS (SELECT s.sucursal, s.producto, s.present \
				FROM stock as s inner join articulos as a on a.producto=s.producto and a.present=s.present \
				inner join dcompras as d on d.articulo=a.articulo \
				WHERE a.articulo='%s' and d.referencia=@folio and s.sucursal='%s') LIMIT 1  \
				", sucursal, articulo, articulo, sucursal);
				instrucciones[num_instrucciones++]=instruccion;

			}

			/* VALIDAR SI ALGÚN ARTICULO ESTA INACTIVO*/
			   	BufferRespuestas* resp_compactivo=NULL;
				try {
					instruccion.sprintf("SELECT p.nombre, a.present, a.multiplo, a.activo FROM articulos a INNER JOIN productos p \
							ON p.producto = a.producto WHERE a.articulo IN (%s)",lista_articulos);
					if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_compactivo)) {
						if (resp_compactivo->ObtieneNumRegistros()>0){
							for(int c = 0; c < resp_compactivo->ObtieneNumRegistros(); c++)
							{
								if(resp_compactivo->ObtieneDato("activo") == "0")
									throw(Exception("El artículo " + resp_compactivo->ObtieneDato("nombre") + " " + resp_compactivo->ObtieneDato("present") + " " + resp_compactivo->ObtieneDato("multiplo") + " está inactivo. Favor de activarlo antes de grabar la compra"));

								resp_compactivo->IrAlSiguienteDato();

							}

						} else throw (Exception("No se encuentra ningún artículo"));
					} else throw (Exception("Error al consultar los artículos"));
				} __finally {
					if (resp_compactivo!=NULL) delete resp_compactivo;
				}
			/*FIN VALIDAR SI ALGÚN ARTICULO ESTA INACTIVO*/

			// Obtiene en variables de mysql si se va a reducir el costo base o no
			instruccion.sprintf("SELECT @modcosto:=IF((DATEDIFF(CURDATE(), \
			IF(c.fechaalta <= c.fechacom ,c.fechaalta,c.fechacom))) > p.valor,'NO','SI' ) FROM compras c, \
			parametrosglobemp p WHERE c.referencia = @folio  AND p.parametro = 'MAXCOMPRAMOD' AND  p.idempresa = %s ", FormServidor->ObtieneClaveEmpresa());
			instrucciones[num_instrucciones++]= instruccion;

			////////////////////////////////////////////////////////////////////////////////
			// Calcula el precio de los artículos con base en la compra inmediata anterior.
			////////////////////////////////////////////////////////////////////////////////
											
			// Obtiene en variables de mysql si se va a reducir el costo base o no
			instruccion.sprintf("SELECT @reduccostobase1:=p.reduccostobase, @porcreduccosto1:=p.porcreduccosto FROM compras c INNER JOIN proveedores p ON c.proveedor=p.proveedor WHERE c.referencia=@folio;");
			instrucciones[num_instrucciones++]=instruccion;

			// Reduce el costo base para precios con respecto a la configuracion del proveedor
			instruccion.sprintf("update tmpcostos cos \
				set cos.costo=if(@reduccostobase1, cos.costo/(1+@porcreduccosto1/100), cos.costo)");
			instrucciones[num_instrucciones++]=instruccion;

			// Agrega la clave de producto y presentacion de los articulos a modificar.
			instruccion.sprintf("update tmpcostos cos, articulos a \
				set cos.producto=a.producto, cos.present=a.present \
				where cos.articulo=a.articulo");
			instrucciones[num_instrucciones++]=instruccion;

			// En la tabla auxiliar de costos establece que precios NO se van a modificar
			// (los que tengan compras posteriores a la fecha)
			instruccion.sprintf("update compras c, dcompras d, tmpcostos cos set cos.modificar=0 where c.fechacom>'%s' and c.cancelado=0 and d.articulo=cos.articulo and c.referencia=d.referencia", mFg.DateToMySqlDate(fecha_com));
			instrucciones[num_instrucciones++]=instruccion;

			// Los que están bloqueados para modificación de precios se marcan para NO modificar sus precios.
			instruccion.sprintf("update tmpcostos cos \
					inner join articulos a ON cos.articulo=a.articulo \
					inner join preciosbloqueados pb ON pb.producto=a.producto and pb.present=a.present and pb.idempresa=%s \
				set cos.modificar=0 \
				where pb.fechaVigencia>='%s' ",
				FormServidor->ObtieneClaveEmpresa(), mFg.DateToMySqlDate(fecha));
			instrucciones[num_instrucciones++]=instruccion;


			/*
			//Ingresa los cambios de los costos en la bitacora
			instruccion.sprintf("INSERT INTO bitacoracostos SELECT @folio AS referencia,a.articulo, \
			'%s' AS tipo, p.costobase,(tc.costo/a.factor) AS costo,CURDATE(),CURTIME(),'%s' \
			FROM presentaciones p,tmpcostos tc,articulos a  \
			WHERE tc.articulo=a.articulo AND a.present=p.present AND a.producto=p.producto AND tc.costo<>0  \
			AND tc.modificar=1 AND @modcosto = 'SI' ",tipobitacoracosto,usuario );
			instrucciones[num_instrucciones++] = instruccion;

			// Cambia el costo unitario en la tabla presentaciones.
			instruccion="update tmpcostos cos, presentaciones p, articulos a ";
			instruccion+="set p.costobase=cos.costo/a.factor ";
			instruccion+=", p.costoultimo=cos.costo/a.factor ";
			instruccion+="where cos.articulo=a.articulo and a.present=p.present and a.producto=p.producto and cos.costo<>0 \
			and cos.modificar=1 AND @modcosto = 'SI' ";
			instrucciones[num_instrucciones++]=instruccion;    */


            //Ingresa los cambios de los costos en la bitacora
			instruccion.sprintf("INSERT INTO bitacoracostos SELECT @folio AS referencia,a.articulo, \
			'%s' AS tipo, p.costobase,(tc.costo/a.factor) AS costo,CURDATE(),CURTIME(),'%s', p.idempresa \
			FROM presentacionescb p,tmpcostos tc,articulos a  \
			WHERE tc.articulo=a.articulo AND a.present=p.present AND a.producto=p.producto AND tc.costo<>0  \
			AND tc.modificar=1 AND @modcosto = 'SI' AND p.idempresa=%s ",
			tipobitacoracosto,usuario, empresa_sistema);
			instrucciones[num_instrucciones++] = instruccion;

			// Cambia el costo unitario en la tabla presentaciones.
			instruccion="update tmpcostos cos, presentacionescb p, articulos a ";
			instruccion+="set p.costobase=cos.costo/a.factor ";
			instruccion+=", p.costoultimo=cos.costo/a.factor ";
			instruccion+="where cos.articulo=a.articulo and a.present=p.present and a.producto=p.producto and cos.costo<>0 \
			and cos.modificar=1 AND @modcosto = 'SI' AND p.idempresa="+empresa_sistema;
			instrucciones[num_instrucciones++]=instruccion;



			// Cambia el costo y los precios en la tabla de precios (globales)
			instruccion.sprintf("select @valor:=valor from parametrosemp where parametro='DIGREDOND' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
			instrucciones[num_instrucciones++]=instruccion;

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
			instruccion+="tp.tipoprec=prec.tipoprec and prec.articulo=a.articulo and a.present=present.present and a.producto=present.producto \
			AND @modcosto = 'SI' and present.idempresa="+empresa_sistema+" and tp.idempresa="+empresa_sistema ;
			instrucciones[num_instrucciones++]=instruccion;

			// Obtiene la sucursal a la que afecta la compra a nivel inventario
			// (donde entrará fisicamente es donde se va a vender y ahí afectará los precios)
			instruccion.sprintf("select @sucursal:=sec.sucursal from compras c \
				inner join almacenes a ON a.almacen=c.almacen \
				INNER JOIN secciones sec ON a.seccion=sec.seccion \
				where c.referencia=@folio");
			instrucciones[num_instrucciones++]=instruccion;


			// Cambia el costo y los precios en la tabla de PRECIOS LOCALES
			AnsiString asignacion_precio_local;
			asignacion_precio_local.sprintf("prec.precio=ROUND(ROUND(present.costobase*(1+prec.porcutil/100),@valor)*a.factor,@valor) ");
			instruccion.sprintf("update preciolocal prec \
                inner join tiposdeprecios tp ON tp.tipoprec=prec.tipoprec \
				inner join articulos a ON prec.articulo=a.articulo \
				inner join presentacionescb present ON a.present=present.present and a.producto=present.producto \
				inner join tmpcostos cos ON cos.producto=a.producto and cos.present=a.present \
				inner join autorizarprecios aut ON aut.sucursal=@sucursal \
					AND aut.producto=a.producto AND aut.present=a.present \
					AND aut.fechavigencia>='%s' \
				set prec.costo=present.costobase, \
				%s \
				where cos.costo<>0 and cos.modificar=1 and \
				prec.sucursal=@sucursal and @modcosto = 'SI' and present.idempresa=%s and tp.idempresa=%s ",
				mFg.DateToMySqlDate(fecha), asignacion_precio_local, empresa_sistema, empresa_sistema );
			instrucciones[num_instrucciones++]=instruccion;

			////////////////////////////////////////////////////////////////////////////
			//            FIN RECALCULO DE PRECIOS
			////////////////////////////////////////////////////////////////////////////


			// Hace un abono que refleje el anticipo.
			if (acredito) {
				if (anticipo>0.0) {
					if (tarea_compra=="A") {
						// Obtiene el folio para la NUEVA transaccion
						instruccion.sprintf("select @foliotranaux:=valor from foliosemp where folio='TRANPROV' AND sucursal = '%s' %s ",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
						instrucciones[num_instrucciones++]=instruccion;
						instruccion.sprintf("set @foliosigtran=@foliotranaux+1");
						instrucciones[num_instrucciones++]=instruccion;
						instruccion.sprintf("set @foliotranaux=cast(@foliotranaux as char)");
						instrucciones[num_instrucciones++]=instruccion;
						instruccion.sprintf("set @foliotran=concat('%s', lpad(@foliotranaux,9,'0'))", FormServidor->ObtieneClaveSucursal());
						instrucciones[num_instrucciones++]=instruccion;
						instruccion.sprintf("update foliosemp set valor=@foliosigtran where folio='TRANPROV' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
						instrucciones[num_instrucciones++]=instruccion;
						instruccion.sprintf("insert into transxpag \
							(tracredito, referencia, concepto, destino, tipo, cancelada, aplicada, valor, fechaalta, horaalta, fechamodi, horamodi, fechaapl, usualta, usumodi) \
							values (@foliotran, @folio, 'A', 'C', 'ANTI', 0,1, -%12.2f, '%s', '%s', '%s', '%s', '%s', '%s', '%s')",
							anticipo, mFg.DateToMySqlDate(fecha_inic), mFg.TimeToMySqlTime(hora), mFg.DateToMySqlDate(fecha),mFg.TimeToMySqlTime(hora), mFg.DateToMySqlDate(fecha_inic), usuario, usuario );
						instrucciones[num_instrucciones++]=instruccion;
					} else {
						// Obtiene el folio de la transaccion ya existente
						instruccion.sprintf("select @foliotran:=tracredito from transxpag where referencia=@folio and concepto='A' and destino='C' and tipo='ANTI' and cancelada=0");
						instrucciones[num_instrucciones++]=instruccion;
						instruccion.sprintf("update transxpag set valor=-%12.2f, fechamodi='%s', horamodi='%s', fechaapl='%s', usumodi='%s', cancelada=0 where tracredito=@foliotran", anticipo, mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), mFg.DateToMySqlDate(fecha_inic), usuario);
						instrucciones[num_instrucciones++]=instruccion;
					}
				} else {
					if (tarea_compra=="M") {
						instruccion.sprintf("update transxpag set cancelada=1, fechamodi='%s', horamodi='%s', usumodi='%s' where referencia=@folio and concepto='A' and destino='C' and tipo='ANTI' and cancelada=0", mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario);
						instrucciones[num_instrucciones++]=instruccion;
					}
				}
			}

			if (pedidofacturar!=" ") {
				// Si se trata de facturación de un pedido se marca este como facturado y se guarda el folio de la compra
				instruccion.sprintf("update pedidos set facturado=1, compra = @folio where referencia='%s'", pedidofacturar);
				instrucciones[num_instrucciones++]=instruccion;
			}

			if(tarea_compra=="A") {
				if(recepcion != "") {
					AnsiString folio = "";
					AnsiString folio_pedido = "";
					AnsiString condicion_fraccionado = " ";
					int count_recepciones = 0;
					TStringDynArray array_recepcion(SplitString(recepcion, ","));
					count_recepciones = array_recepcion.Length;
					BufferRespuestas* resp_verif_compxped=NULL;
					BufferRespuestas* resp_articulos_fracc=NULL;

					if(fraccionado == "1") {
						condicion_fraccionado = "INNER JOIN recepcionarticulofraccionado raf ON raf.recepcion = r.recepcion AND raf.articulo = dr.articulo AND raf.pedido = p.referencia ";
					}

					for(int f = 0; f < count_recepciones; f++) {
						folio += "'" + array_recepcion[f] + "'";
						if (f != count_recepciones-1) {
							folio = folio + ",";
						}
					}
					try {
						instruccion.sprintf("SELECT p.referencia, p.facturado, p.compra \
						FROM recepciones r \
						INNER JOIN drecepciones dr ON dr.recepcion = r.recepcion \
						INNER JOIN pedrecepcion pr ON pr.recepcion = r.recepcion \
						INNER JOIN pedidos p ON p.referencia = pr.pedido \
						INNER JOIN dpedidos dp ON dp.referencia = p.referencia AND dr.articulo = dp.articulo \
						%s \
						WHERE r.recepcion IN(%s) AND p.facturado = 0 \
						GROUP BY pr.pedido  \
						",
						condicion_fraccionado,
						folio);
						if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_verif_compxped)) {
							for(int c = 0; c < resp_verif_compxped->ObtieneNumRegistros(); c++) {
								resp_verif_compxped->IrAlRegistroNumero(c);

								folio_pedido += "'" + resp_verif_compxped->ObtieneDato("referencia") + "'";
								if (c != resp_verif_compxped->ObtieneNumRegistros()-1) {
									folio_pedido = folio_pedido + ",";
								}

							}

							// Si se trata de facturación de un pedido se marca este como facturado y se guarda el folio de la compra
							instruccion.sprintf("UPDATE pedidos SET facturado=1 WHERE referencia IN (%s) ",
							folio_pedido);
							instrucciones[num_instrucciones++]=instruccion;

							// Si se trata de facturación de un pedido se marca este como facturado y se guarda el folio de la compra
							instruccion.sprintf("UPDATE pedidos SET compra = @folio WHERE referencia IN (%s) LIMIT 1",
							folio_pedido);
							instrucciones[num_instrucciones++]=instruccion;

							// Si se trata de facturación de un pedido se marca este como facturado y se guarda el folio de la compra
							instruccion.sprintf("UPDATE recepciones SET fechamodi = CURDATE(), horamodi = CURTIME() WHERE recepcion IN (%s)",
							folio);
							instrucciones[num_instrucciones++]=instruccion;
						}
					} __finally {
						if (resp_verif_compxped!=NULL) delete resp_verif_compxped;
					}

					try {
						instruccion.sprintf("SELECT pedido, recepcion, articulo \
						FROM recepcionarticulofraccionado \
						WHERE recepcion IN (%s) ",
						folio);
						if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_articulos_fracc)) {
							for(int c = 0; c < resp_articulos_fracc->ObtieneNumRegistros(); c++) {
								resp_articulos_fracc->IrAlRegistroNumero(c);

								AnsiString a, b, c;

								a = resp_articulos_fracc->ObtieneDato("pedido");
								b = resp_articulos_fracc->ObtieneDato("recepcion");
								c = resp_articulos_fracc->ObtieneDato("articulo");

								// Si se trata de facturación de un pedido se marca este como facturado y se guarda el folio de la compra
								instruccion.sprintf("INSERT INTO compraspedidosprov \
								(pedido, recepcion, compra, articulo) \
								VALUES \
								('%s','%s',@folio,'%s') ",
								resp_articulos_fracc->ObtieneDato("pedido"),
								resp_articulos_fracc->ObtieneDato("recepcion"),
								resp_articulos_fracc->ObtieneDato("articulo"));
								instrucciones[num_instrucciones++]=instruccion;

								// Si se trata de facturación de un pedido se marca este como facturado y se guarda el folio de la compra
								instruccion.sprintf("DELETE FROM recepcionarticulofraccionado \
								WHERE articulo = '%s' AND pedido = '%s' AND recepcion = '%s' ",
								resp_articulos_fracc->ObtieneDato("articulo"),
								resp_articulos_fracc->ObtieneDato("pedido"),
								resp_articulos_fracc->ObtieneDato("recepcion"));
								instrucciones[num_instrucciones++]=instruccion;

							}
						}
					} __finally {
						if (resp_articulos_fracc!=NULL) delete resp_articulos_fracc;
					}
				}
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
		delete[] instrucciones;
	}
}

//------------------------------------------------------------------------------
//ID_CANC_COMPRA
void ServidorCompras::CancelaCompra(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  CANCELA COMPRA
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
	AnsiString empresa_sistema = FormServidor->ObtieneClaveEmpresa();

	AnsiString at_tmpultimasfech;

	// Consulta de parámetro para asignación de precios
	// * Si es 0 el cambio de precio es inmediato y se guarda en el campo precios.precio
	// * si es 1 entonces el cambio de precios es diferido y se guarda precios.precioproximo para aplicarse posteriormente)
	AnsiString paramcambioprec;
	BufferRespuestas* resp_paramcambioprec=NULL;
	try {
		instruccion.sprintf("SELECT valor FROM parametrosglobemp WHERE parametro='CAMBIOPRECDIFER' AND idempresa = %s ",FormServidor->ObtieneClaveEmpresa());
		if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_paramcambioprec)) {
			if (resp_paramcambioprec->ObtieneNumRegistros()>0){
				paramcambioprec=resp_paramcambioprec->ObtieneDato("valor");
			} else throw (Exception("No se encuentra registro CAMBIOPRECDIFER en tabla parametrosglobemp"));
		} else throw (Exception("Error al consultar en tabla parametros"));
	} __finally {
		if (resp_paramcambioprec!=NULL) delete resp_paramcambioprec;
	}


	try {
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave de la compra (referencia)
		usuario=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que esta grabando la compra.

		// Verifica que no tenga pagos (sin tomar en cuenta el anticipo ni los cheques no cobrables)  ---------
		instruccion.sprintf("select @error:=if(COALESCE(sum(t.valor),0)<0, 1, 0) as error from compras c, transxpag t where c.referencia=t.referencia and c.referencia='%s' and c.cancelado=0 and t.cancelada=0 and t.valor<0 and t.tipo<>'ANTI' group by c.referencia", clave);
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 2, error);

		// Verifica que la fecha de la factura sea posterior a la fecha de cierre.
		instruccion.sprintf("select @error:=if(c.fechacom<=cast(e.valor as datetime), 1, 0) as error from compras c left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' where c.referencia='%s'",FormServidor->ObtieneClaveSucursal(), clave);
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 3, error);

		//cuando se quiera cancelar una compra y en ella este algun producto inactivo, es necesario evitar
		//este resgistro, esto, con la finalidad de evitar productos incongruentes
		AnsiString produc2,present2,multiplo2,factor2,select_inactivos,nombre;
		BufferRespuestas* resp_inactivos=NULL;
		try{
			select_inactivos.sprintf("SELECT  @error:=IF(a.activo=0, 1, 0) AS error, \
				a.producto, a.present, a.multiplo, a.factor, pro.nombre \
				FROM compras m LEFT JOIN dcompras dm ON dm.referencia=m.referencia \
				LEFT JOIN articulos a ON a.articulo=dm.articulo \
				LEFT JOIN productos pro ON pro.producto=a.producto \
				WHERE m.referencia='%s' AND a.activo=0",clave );
			if(mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, select_inactivos.c_str(), resp_inactivos)){
				if (resp_inactivos->ObtieneNumRegistros()>0){
					produc2=resp_inactivos->ObtieneDato("producto");
					present2=resp_inactivos->ObtieneDato("present");
					multiplo2=resp_inactivos->ObtieneDato("multiplo");
					factor2=resp_inactivos->ObtieneDato("factor");
					nombre = resp_inactivos->ObtieneDato("nombre");
					error=4;
					throw (Exception("Hay artículos inactivos en el detalle de la compra a cancelar.\n Favor de activar el artículo o quitarlo de la lista.\n  artículo:"+nombre+" "+present2+" "+multiplo2));
				}
			}else{
				throw (Exception("Error al consultar los artículos inactivos en cancelar compras"));
			}
	   /*	,a.producto, a.present, a.multiplo, a.factor   */
	   }__finally{
			if (resp_inactivos!=NULL) delete resp_inactivos;
	   }


		if (error==0) {
			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

			// Cancela la compra
			instruccion.sprintf("update compras set cancelado=1, fechamodi='%s', horamodi='%s', usumodi='%s' where referencia='%s' and cancelado=0", mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario, clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Actualizar el pedido que tenga una compra asignada
			instruccion.sprintf("UPDATE pedidos SET facturado=0, compra=NULL WHERE compra='%s' ", clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Actualizar los pedidos que tengan relación con una recepción y una compra
			instruccion.sprintf("UPDATE comprecepcion cr \
			INNER JOIN pedrecepcion pr ON pr.recepcion = cr.recepcion \
			INNER JOIN pedidos p ON p.referencia = pr.pedido \
			SET p.compra = NULL, p.facturado = 0 \
			WHERE cr.compra = '%s' ", clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Cancela el cargo hecho por el total de la compra
			instruccion.sprintf("update transxpag set cancelada=1, fechamodi='%s', horamodi='%s', usumodi='%s' where referencia='%s' and concepto='C' and destino='C' and tipo='COMP' and cancelada=0", mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario, clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Cancela el abono hecho por el ANTICIPO
			instruccion.sprintf("update transxpag set cancelada=1, fechamodi='%s', horamodi='%s', usumodi='%s' where referencia='%s' and concepto='A' and destino='C' and tipo='ANTI' and cancelada=0", mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario, clave);
			instrucciones[num_instrucciones++]=instruccion;

			/* Crea tabla temporal para almacenar cantidades de articulos antes de eliminar */
			instruccion="CREATE temporary TABLE tmpcantidad(referencia VARCHAR(11) NOT NULL,\
			producto VARCHAR(8) NOT NULL, present VARCHAR(255) NOT NULL, almacen VARCHAR(4) NOT NULL,  \
			cantidad DECIMAL(12,3) NOT NULL, PRIMARY KEY (referencia, producto, present, almacen))";
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("INSERT INTO tmpcantidad SELECT d.referencia, a.producto, a.present, c.almacen,  \
			SUM(d.cantidad * a.factor) AS cantidad FROM dcompras d INNER JOIN compras c ON    \
			c.referencia = d.referencia INNER JOIN articulos a ON a.articulo = d.articulo WHERE d.referencia = '%s' \
			GROUP BY a.producto, a.present", clave);
			instrucciones[num_instrucciones++]=instruccion;

			instruccion = "UPDATE tmpcantidad tmp INNER JOIN existenciasactuales ea ON ea.producto = tmp.producto \
			AND ea.present = tmp.present AND ea.almacen = tmp.almacen SET ea.cantidad = (ea.cantidad  - tmp.cantidad) \
			, ea.compras = (ea.compras - tmp.cantidad) ";
            instrucciones[num_instrucciones++]=instruccion;


			// Obtiene en variables de mysql si se va a reducir el costo base o no

			try{
				instruccion.sprintf("SELECT IF((DATEDIFF(CURDATE(),IF(c.fechaalta <= c.fechacom ,c.fechaalta,c.fechacom))) \
				> p.valor,'NO','SI' ) as modcosto FROM compras c, parametrosglobemp p \
				WHERE c.referencia = '%s'  AND p.parametro = 'MAXCOMPRAMOD' AND p.idempresa = %s ", clave, FormServidor->ObtieneClaveEmpresa());
				if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_costo)) {
					if (resp_costo->ObtieneNumRegistros()>0){
						modcosto=resp_costo->ObtieneDato("modcosto");
					} else throw (Exception("No se encuentra registro"));
				} else throw (Exception("Error al consultar en la tabla"));
			} __finally {
				if (resp_costo!=NULL) delete resp_costo;
			}

			if(modcosto == "SI"){
			////////////////////////////////////////////////////////////////////////////////
			// Calcula el precio de los artículos con base en la compra inmediata anterior.
			////////////////////////////////////////////////////////////////////////////////

			instruccion="create temporary table tmpcostos ( \
				articulo varchar(9), producto varchar(8), present varchar(255), \
				costoimp decimal(16,6), modificar bool, fechacom DATE, PRIMARY KEY (articulo)) Engine = InnoDB";
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("create temporary table tmpultimasfech \
				select d.articulo, max(c.fechacom) as fechacom \
					from compras c, dcompras d, dcompras dcanc \
					where c.referencia=d.referencia and dcanc.articulo=d.articulo \
					and dcanc.referencia='%s' and c.cancelado=0 \
					group by d.articulo", clave);
			instrucciones[num_instrucciones++]=instruccion;

			/*Ingresar a nueva tabla los productos y presentacions de los articulos hermanos*/
			instrucciones[num_instrucciones++]="CREATE TEMPORARY TABLE prod_present (producto VARCHAR(8), \
			present VARCHAR(255) ,PRIMARY KEY (producto, present)) ENGINE = INNODB ";

			instruccion.sprintf("REPLACE INTO prod_present SELECT a.producto,a.present FROM articulos a \
			INNER JOIN dcompras d ON a.articulo= d.articulo WHERE d.referencia='%s'",clave);
			instrucciones[num_instrucciones++]=instruccion;

			at_tmpultimasfech = mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);


			instruccion.sprintf("SELECT a.articulo,MAX(c.fechacom) AS fechacom FROM \
			prod_present pp INNER JOIN articulos a ON a.producto = pp.producto AND a.present = pp.present    \
			INNER JOIN dcompras dc ON a.articulo = dc.articulo INNER JOIN compras c ON dc.referencia = c.referencia   \
			INNER JOIN dcompras dcanc ON dcanc.referencia='%s' WHERE a.articulo  NOT IN (SELECT d.articulo   \
			FROM compras c, dcompras d, dcompras dcanc WHERE c.referencia=d.referencia AND dcanc.articulo=d.articulo \
			AND dcanc.referencia='%s' AND c.cancelado=0 GROUP BY d.articulo) AND c.cancelado=0 GROUP BY a.articulo \
			INTO OUTFILE '%s'",
			clave, clave, at_tmpultimasfech);
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf(" LOAD DATA INFILE '%s' INTO TABLE tmpultimasfech ", at_tmpultimasfech);
			instrucciones[num_instrucciones++] = instruccion;



			instruccion.sprintf("create temporary table tmpultimascomp \
				select d.articulo, max(c.referencia) as referencia \
					from compras c, dcompras d, tmpultimasfech u \
					where c.referencia=d.referencia and u.articulo=d.articulo \
					and u.fechacom=c.fechacom and c.cancelado=0 \
					group by d.articulo");
			instrucciones[num_instrucciones++]=instruccion;

			instruccion="CREATE TEMPORARY TABLE  tmpdnotascredprov   \
				SELECT n.compra,n.referencia, d.articulo, SUM(IF(n.tipo<>'0',d.costoimp,0)) AS costoimp   \
				FROM notascredprov n, dnotascredprov d WHERE n.compra IN (SELECT referencia FROM tmpultimascomp) \
				AND n.cancelado=0 AND n.referencia=d.referencia	GROUP BY d.articulo, n.compra ";
				instrucciones[num_instrucciones++]=instruccion;

			instrucciones[num_instrucciones++]="CREATE TEMPORARY TABLE tmpultimasfech2 \
				SELECT * FROM tmpultimasfech ";

			instruccion="INSERT INTO tmpcostos (articulo, costoimp, modificar,fechacom)  \
				SELECT fech.articulo,(d.costoimp - IFNULL(td.costoimp,0)) AS costoimp, \
					1 AS modificar , fech.fechacom \
				FROM tmpultimascomp comp \
					INNER JOIN tmpultimasfech fech ON fech.articulo = comp.articulo \
					INNER JOIN articulos a ON a.articulo = fech.articulo  \
					INNER JOIN dcompras d ON d.referencia = comp.referencia  AND d.articulo = fech.articulo \
					LEFT JOIN tmpdnotascredprov td ON td.articulo = fech.articulo AND td.compra = comp.referencia \
				WHERE fech.fechacom IN \
					(SELECT MAX(fech.fechacom) FROM tmpultimasfech2 fech INNER JOIN articulos \
					a ON a.articulo=fech.articulo GROUP BY a.producto, a.present)";
				instrucciones[num_instrucciones++]=instruccion;

			// Agrega la clave de producto y presentacion de los articulos a modificar.
			instruccion.sprintf("update tmpcostos cos, articulos a \
				set cos.producto=a.producto, cos.present=a.present \
				where cos.articulo=a.articulo");
			instrucciones[num_instrucciones++]=instruccion;

			// En la tabla auxiliar de costos establece que precios NO se van a modificar
			// (los que tengan compras posteriores a la fecha)
			instruccion.sprintf("select @fechacom:=fechacom from compras where referencia='%s'", clave);
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("update compras c, dcompras d, tmpcostos cos set cos.modificar=0 where c.fechacom>@fechacom and c.cancelado=0 and d.articulo=cos.articulo and c.referencia=d.referencia");
			instrucciones[num_instrucciones++]=instruccion;

			// Los que están bloqueados para modificación de precios se marcan para NO modificar sus precios.
			instruccion.sprintf("update tmpcostos cos \
					inner join articulos a ON cos.articulo=a.articulo \
					inner join preciosbloqueados pb ON pb.producto=a.producto and pb.present=a.present and pb.idempresa=%s \
				set cos.modificar=0 \
				where pb.fechaVigencia>='%s' ",
				FormServidor->ObtieneClaveEmpresa(), mFg.DateToMySqlDate(fecha));
			instrucciones[num_instrucciones++]=instruccion;

			//Ingresa los cambios de los costos en la bitacora
			instruccion.sprintf("INSERT INTO bitacoracostos SELECT '%s' AS referencia,a.articulo, \
			'CC' AS tipo, p.costobase,(tc.costoimp/a.factor) AS costo,CURDATE(),CURTIME(),'%s', p.idempresa \
			FROM presentacionescb p, tmpcostos tc, \
			articulos a WHERE tc.articulo=a.articulo AND a.present=p.present AND a.producto=p.producto AND tc.costoimp<>0  \
			AND tc.modificar=1 AND p.idempresa=%s ", clave, usuario, empresa_sistema);
			instrucciones[num_instrucciones++]=instruccion;

			// Cambia el costo unitario en la tabla presentaciones.
			instruccion="update tmpcostos cos, presentacionescb p, articulos a ";
			instruccion+="set p.costobase=cos.costoimp/a.factor ";
			instruccion+=", p.costoultimo=cos.costoimp/a.factor ";
			instruccion+="where cos.articulo=a.articulo and a.present=p.present and a.producto=p.producto and cos.costoimp<>0 \
			and cos.modificar=1 and p.idempresa="+empresa_sistema;
			instrucciones[num_instrucciones++]=instruccion;

			// Cambia el costo y los precios en la tabla de precios (globales).
			instruccion.sprintf("select @valor:=valor from parametrosemp where parametro='DIGREDOND' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
			instrucciones[num_instrucciones++]=instruccion;

			AnsiString asignacion_precio;
			if (paramcambioprec=="0"){
				asignacion_precio.sprintf("prec.precio=ROUND(ROUND(present.costobase*(1+prec.porcutil/100),@valor)*a.factor,@valor), \
									prec.precioproximo=ROUND(ROUND(present.costobase*(1+prec.porcutil/100),@valor)*a.factor,@valor) ");
			}else{
				asignacion_precio.sprintf("prec.precioproximo=ROUND(ROUND(present.costobase*(1+prec.porcutil/100),@valor)*a.factor,@valor) ");
				instruccion="update presentacionescb present, tmpcostos cos, precios prec, tiposdeprecios tp , articulos a ";
				instruccion+="set prec.costo=present.costobase, ";
				instruccion+=asignacion_precio;
				instruccion+="where cos.producto=a.producto and cos.present=a.present and cos.costoimp<>0 and cos.modificar=1 and ";
				instruccion+="prec.articulo=a.articulo and a.present=present.present and a.producto=present.producto ";
                instruccion+="and tp.tipoprec=prec.tipoprec  and present.idempresa="+empresa_sistema+" and tp.idempresa="+empresa_sistema;
				instrucciones[num_instrucciones++]=instruccion;
			}

			// Obtiene la sucursal a la que afecta la compra a nivel inventario
			// (donde entrará fisicamente es donde se va a vender y ahí afectará los precios)
			instruccion.sprintf("select @sucursal:=sec.sucursal from compras c \
				inner join almacenes a ON a.almacen=c.almacen \
				INNER JOIN secciones sec ON a.seccion=sec.seccion \
				where c.referencia='%s'",clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Cambia el costo y los precios en la tabla de PRECIOS LOCALES
			AnsiString asignacion_precio_local;
			asignacion_precio_local.sprintf("prec.precio=ROUND(ROUND(present.costobase*(1+prec.porcutil/100),@valor)*a.factor,@valor) ");
			instruccion.sprintf("update preciolocal prec \
				inner join tiposdeprecios tp ON tp.tipoprec=prec.tipoprec \
				inner join articulos a ON prec.articulo=a.articulo \
				inner join presentacionescb present ON a.present=present.present and a.producto=present.producto \
				inner join tmpcostos cos ON cos.producto=a.producto and cos.present=a.present \
				inner join autorizarprecios aut ON aut.sucursal=@sucursal \
					AND aut.producto=a.producto AND aut.present=a.present \
					AND aut.fechavigencia>='%s' \
				set prec.costo=present.costobase, \
				%s \
				where cos.costoimp<>0 and cos.modificar=1 and \
				prec.sucursal=@sucursal and present.idempresa=%s and tp.idempresa=%s ",
				mFg.DateToMySqlDate(fecha), asignacion_precio_local, empresa_sistema, empresa_sistema);
			instrucciones[num_instrucciones++]=instruccion;

			////////////////////////////////////////////////////////////////////////////
			//            FIN RECALCULO DE PRECIOS
			////////////////////////////////////////////////////////////////////////////
			}

			instrucciones[num_instrucciones++]="COMMIT";
		}
		/*if( error== 4){
			mServidorVioleta->MuestraMensaje("Error de solicitud: Ariculos inactivos en el detalle de compra a cancelar");
		}*/
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
		if(at_tmpultimasfech != "")
			mServidorVioleta->BorraArchivoTemp(at_tmpultimasfech);
	}
}


//------------------------------------------------------------------------------
//ID_CON_COMPRA
void ServidorCompras::ConsultaCompra(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA COMPRAS
	AnsiString instruccion;
	AnsiString clave, menos_devoluciones, esParteRel="";

	clave=mFg.ExtraeStringDeBuffer(&parametros);
	menos_devoluciones=mFg.ExtraeStringDeBuffer(&parametros);


    mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los generales (de cabecera) de la compra
	instruccion.sprintf("SELECT sec.sucursal, c.*, cm.mensaje, imp.porcentaje, c.impuestoret  FROM compras c INNER JOIN terminales ter \
						ON c.terminal=ter.terminal INNER JOIN secciones sec ON sec.seccion=ter.seccion  \
						left join comprasmensajes cm on cm.referencia=c.referencia \
						LEFT JOIN impuestos imp ON imp.impuesto = c.impuestoret \
						where c.referencia='%s'", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

    // Calcula el saldo de la compra (SALDO B)
    instruccion.sprintf("select sum(valor) as saldo from transxpag where referencia='%s' and cancelada=0 group by referencia", clave);
    mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene todos los datos del proveedor de la compra
	instruccion.sprintf("select p.* from proveedores p, compras c where c.referencia='%s' and c.proveedor=p.proveedor", clave);
    mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

    // NULO:  Verifica que la fecha de la factura sea posterior a la fecha de cierre.
	instruccion.sprintf("select if(c.fechacom>=cast(e.valor as datetime), 1, 0) as modificar from compras c left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' where c.referencia='%s'",FormServidor->ObtieneClaveSucursal(), clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	// solo saber si es un proveedor relacional
	//se verificara si el proveedor es parte relacionada
	AnsiString proveedor_esParteRelac;
	BufferRespuestas* resp_prov=NULL;
	try{
		proveedor_esParteRelac.sprintf("select esparterelac from proveedores p, compras c where c.referencia='%s' and c.proveedor=p.proveedor", clave );
		if(mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, proveedor_esParteRelac.c_str(), resp_prov)){
			if (resp_prov->ObtieneNumRegistros()>0){
			 esParteRel= resp_prov->ObtieneDato("esparterelac");

			}
		}else{
			throw (Exception("Error al consultar el proveedor parte relacionada ID_GRA_COMPRA"));
		}
   }__finally{
		if (resp_prov!=NULL) delete resp_prov;
   }



    // Obtiene todo el detalle de la compra con algunos datos extras que necesita el cliente
	if (menos_devoluciones=="0") {
		BufferRespuestas * br_ordenPedidoVenta;
		AnsiString ordenPedidoVenta = "0";
		try{
			mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,
				"SELECT valor FROM parametrosemp p WHERE sucursal = (SELECT valor \
				from parametrosgen where parametro = 'SUCCEDISGLOBAL') AND parametro = 'ORDPEDVENTAF6'",
				br_ordenPedidoVenta);

			ordenPedidoVenta = br_ordenPedidoVenta->ObtieneDato();
		}__finally{
			delete br_ordenPedidoVenta;
		}

        // Obtiene los datos de la compra completa (sin importar las devoluciones)
		instruccion="select d.referencia, d.articulo, d.cantidad, d.costo, d.costoimp, ";
		instruccion+="a.present, p.producto, p.nombre, a.multiplo,";
		instruccion+="d.claveimp1, i1.tipoimpu as tipoimpuesto1, i1.porcentaje as porcentajeimp1, t1.nombre as nomtipoimp1, ";
        instruccion+="d.claveimp2, i2.tipoimpu as tipoimpuesto2, i2.porcentaje as porcentajeimp2, t2.nombre as nomtipoimp2, ";
		instruccion+="d.claveimp3, i3.tipoimpu as tipoimpuesto3, i3.porcentaje as porcentajeimp3, t3.nombre as nomtipoimp3, ";
		instruccion+="d.claveimp4, i4.tipoimpu as tipoimpuesto4, i4.porcentaje as porcentajeimp4, t4.nombre as nomtipoimp4, ";
		instruccion+="(a.factor / pmm.cajalogisticafactor) AS cajasbultos, ";
		instruccion+="d.iepscuota, d.id, ";
		instruccion+="IF(a.factor=1,1,2) ORDEN,p.clasif1  "; //Agrupacion para ordenar por multiplo
		instruccion+="from dcompras d  inner join  articulos a  inner join  productos p ";
		instruccion+="left join impuestos i1 on i1.impuesto=d.claveimp1 ";
		instruccion+="left join impuestos i2 on i2.impuesto=d.claveimp2 ";
        instruccion+="left join impuestos i3 on i3.impuesto=d.claveimp3 ";
        instruccion+="left join impuestos i4 on i4.impuesto=d.claveimp4 ";
        instruccion+="left join tiposdeimpuestos t1 on t1.tipoimpu=i1.tipoimpu ";
        instruccion+="left join tiposdeimpuestos t2 on t2.tipoimpu=i2.tipoimpu ";
        instruccion+="left join tiposdeimpuestos t3 on t3.tipoimpu=i3.tipoimpu ";
		instruccion+="left join tiposdeimpuestos t4 on t4.tipoimpu=i4.tipoimpu ";
		instruccion+="LEFT JOIN presentacionesminmax pmm ON a.producto=pmm.producto AND a.present=pmm.present ";
		instruccion+="where d.referencia='";
		instruccion+=clave;
        instruccion+="' and d.articulo=a.articulo and a.producto=p.producto ";
		instruccion+=" group by p.nombre, a.present, a.multiplo ";
		if(esParteRel=="1" && ordenPedidoVenta=="1")
			instruccion+="ORDER BY a.factor = 1, IF(a.factor = 1, p.clasif1, left(multiplo,3)), p.nombre, a.present, a.multiplo ";
		else
			instruccion+=" order by p.nombre, a.present, a.multiplo ";

		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);



    } else {
        // Suma las notas de crédito y deja el resultado en una tabla temporal,
		// para luego hacerle un left join con las compras.
		instruccion="create temporary table dnotascredprovaux ";
		instruccion+="select d.articulo, sum(if(n.tipo='0',d.cantidad,0)) as cantidad, ";
		instruccion+="sum(if(n.tipo<>'0',d.costo,0)) as costo ";
		instruccion+="from notascredprov n, dnotascredprov d ";
		instruccion+="where n.compra='";
		instruccion+=clave;
		instruccion+="' and n.cancelado=0 and n.referencia=d.referencia ";
		instruccion+="group by d.articulo";
		if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
			throw (Exception("Error en query EjecutaSelectSqlNulo"));

		// Sumatoria de todas las notas de crédito.
		instruccion.sprintf("select ifnull(sum(valor),0) as sumnotas, ifnull(sum(if(notascredprov.tipo=0,1,0)),0) as numdevol,  ifnull(sum(if(notascredprov.tipo=1,1,0)),0) as numboni, ifnull(sum(if(notascredprov.tipo=2,1,0)),0) as numdesc from notascredprov where cancelado=0 and compra='%s'", clave);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

		// Obtiene los datos de lo que resta de las compras
		// menos las notas de credito ya aplicadas.
		instruccion="select dc.referencia, dc.articulo, sum(dc.cantidad-ifnull(dn.cantidad,0)) as cantidad, ";
		instruccion+="sum(dc.costo-ifnull(dn.costo,0)) as costo, ";
		instruccion+="a.present, p.producto, p.nombre, a.multiplo, ";
		instruccion+="dc.claveimp1, i1.tipoimpu as tipoimpuesto1, i1.porcentaje as porcentajeimp1, t1.nombre as nomtipoimp1, ";
		instruccion+="dc.claveimp2, i2.tipoimpu as tipoimpuesto2, i2.porcentaje as porcentajeimp2, t2.nombre as nomtipoimp2, ";
		instruccion+="dc.claveimp3, i3.tipoimpu as tipoimpuesto3, i3.porcentaje as porcentajeimp3, t3.nombre as nomtipoimp3, ";
		instruccion+="dc.claveimp4, i4.tipoimpu as tipoimpuesto4, i4.porcentaje as porcentajeimp4, t4.nombre as nomtipoimp4, ";
		instruccion+="dc.iepscuota, dc.id, ";
		instruccion+="IF(a.factor=1,1,2) ORDEN,p.clasif1  "; //Agrupacion para ordenar por multiplo
		instruccion+="from compras c  inner join  dcompras dc  inner join  articulos a  inner join  productos p ";
		instruccion+="left join impuestos i1 on i1.impuesto=dc.claveimp1 ";
		instruccion+="left join impuestos i2 on i2.impuesto=dc.claveimp2 ";
		instruccion+="left join impuestos i3 on i3.impuesto=dc.claveimp3 ";
		instruccion+="left join impuestos i4 on i4.impuesto=dc.claveimp4 ";
		instruccion+="left join tiposdeimpuestos t1 on t1.tipoimpu=i1.tipoimpu ";
		instruccion+="left join tiposdeimpuestos t2 on t2.tipoimpu=i2.tipoimpu ";
		instruccion+="left join tiposdeimpuestos t3 on t3.tipoimpu=i3.tipoimpu ";
		instruccion+="left join tiposdeimpuestos t4 on t4.tipoimpu=i4.tipoimpu ";
		instruccion+="left join dnotascredprovaux dn on dn.articulo=dc.articulo ";
		instruccion+="where c.referencia='";
		instruccion+=clave;
		instruccion+="' and c.referencia=dc.referencia ";
		instruccion+="and dc.articulo=a.articulo and a.producto=p.producto ";
		instruccion+=" group by p.nombre, a.present, a.multiplo ";
		if(esParteRel=="1")
			instruccion+="ORDER BY dc.id asc, p.nombre, a.present, a.multiplo ";
		else
			instruccion+=" order by p.nombre, a.present, a.multiplo ";


		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
}

//------------------------------------------------------------------------------
//ID_GRA_PEDIDO_PROV
void ServidorCompras::GrabaPedidoProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  GRABA PEDIDO
	char *buffer_sql=new char[10000*1000];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString tarea_pedido, clave_pedido, usuario;
	int num_partidas, i;
	int num_instrucciones=0;
	AnsiString instruccion;
	AnsiString * instrucciones = new AnsiString[10000];
	TDate fecha=Today();
	TDate fechaaduana=Today();
	TTime hora=Time();
	AnsiString impuesto1, articulo;
	double costo, costoimp;
	AnsiString mensaje, lista_articulos, esParteRel="", provRemoto="", diasvigencia="",fechapromesa="", proveedor="";
	AnsiString empresa_sistema = FormServidor->ObtieneClaveEmpresa();
	int paramFechaAduanaAuto = 0, paramDiasRango = 0, paramBloqueaRecepcion = 0;



	try {
		clave_pedido=mFg.ExtraeStringDeBuffer(&parametros); // Clave del pedido.
		tarea_pedido=mFg.ExtraeStringDeBuffer(&parametros); // Indicador si se va a agregar o modificar.
		usuario=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que esta grabando el pedido.
		mensaje=mFg.ExtraeStringDeBuffer(&parametros); // Mensaje que se esta grabando el pedido.
		provRemoto=mFg.ExtraeStringDeBuffer(&parametros); // aca solo se mandara la clave del prov remoto cuando sea venta de abastos
		//fechapromesa=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		// Obtiene los datos de la tabla de pedidos
		datos.AsignaTabla("pedidos");
		parametros+=datos.InsCamposDesdeBuffer(parametros);
		// Extrae los datos que necesitamos para crear las letras y transacciones.
		datos.AsignaValorCampo("referencia", "@folio", 1);

		BufferRespuestas* resp_fechaduauto=NULL;
		try {
			AnsiString consulta;
			consulta.sprintf("SELECT valor FROM parametrosglobemp WHERE parametro = 'CONFECHADU' AND idempresa = %s", empresa_sistema);
			if(mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, consulta.c_str(), resp_fechaduauto)){
				if (resp_fechaduauto->ObtieneNumRegistros()>0){
					paramFechaAduanaAuto = StrToInt(resp_fechaduauto->ObtieneDato("valor"));
				}
			}else{
				throw (Exception("Error al consultar parametro CONFECHADU"));
			}
		} __finally {
			if (resp_fechaduauto!=NULL) delete resp_fechaduauto;
		}

		BufferRespuestas* resp_bloquearecepcion=NULL;
		try {
			AnsiString consulta;
			consulta.sprintf("SELECT valor FROM parametrosemp WHERE parametro = 'BLOQPEDRECEP' AND sucursal = '%s'", FormServidor->ObtieneClaveSucursal());
			if(mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, consulta.c_str(), resp_bloquearecepcion)){
				if (resp_bloquearecepcion->ObtieneNumRegistros()>0){
					paramBloqueaRecepcion = StrToInt(resp_bloquearecepcion->ObtieneDato("valor"));
				}
			}else{
				throw (Exception("Error al consultar parametro CONFECHADU"));
			}
		} __finally {
			if (resp_bloquearecepcion!=NULL) delete resp_bloquearecepcion;
		}

		if (tarea_pedido=="M") {
			if(paramBloqueaRecepcion == 1){
				AnsiString proveedor_esParteRelac;
				BufferRespuestas* resp_estatus=NULL;
				try{
					proveedor_esParteRelac.sprintf("SELECT estatus FROM pedidos WHERE referencia='%s' ", clave_pedido);
					if(mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, proveedor_esParteRelac.c_str(), resp_estatus)){
						if (resp_estatus->ObtieneNumRegistros()>0){
							if(StrToInt(resp_estatus->ObtieneDato("estatus"))>1)
								throw (Exception("El pedido ya no puede ser modificado, ya que está siendo recepcionado actualmente."));
						}
					}
			   }__finally{
					if (resp_estatus!=NULL) delete resp_estatus;
			   }
		   }
	   }

		BufferRespuestas* resp_diasmargen=NULL;
		try {
			AnsiString consulta;
			consulta.sprintf("SELECT valor FROM parametrosemp WHERE parametro = 'RANGOVIGENPED' AND sucursal = '%s'",
			FormServidor->ObtieneClaveSucursal());
			if(mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, consulta.c_str(), resp_diasmargen)){
				if (resp_diasmargen->ObtieneNumRegistros()>0){
					paramDiasRango = StrToInt(resp_diasmargen->ObtieneDato("valor"));
					fechaaduana = fechaaduana+paramDiasRango;
				}
			}else{
				throw (Exception("Error al consultar parametro RANGOVIGENPED"));
			}
		} __finally {
			if (resp_diasmargen!=NULL) delete resp_diasmargen;
		}


		 if(provRemoto!=" "){
			AnsiString proveedor_esParteRelac;
			BufferRespuestas* resp_prov=NULL;
			try{
				proveedor_esParteRelac.sprintf("SELECT * FROM proveedores WHERE proveedor='%s' ",provRemoto );
				if(mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, proveedor_esParteRelac.c_str(), resp_prov)){
					if (resp_prov->ObtieneNumRegistros()>0){
					 esParteRel= resp_prov->ObtieneDato("esparterelac");

					}
				}else{
					throw (Exception("Error al consultar el proveedor parte relacionada ID_GRA_PEDIDO_PROV"));
				}
		   }__finally{
				if (resp_prov!=NULL) delete resp_prov;
		   }

		}

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		// Obtiene el folio para el pedido
		if (tarea_pedido=="A") {
			instruccion.sprintf("select @folioaux:=valor from foliosemp where folio='PEDIDOS' AND sucursal = '%s' %s ", FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @foliosig=@folioaux+1");
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @folioaux=cast(@folioaux as char)");
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @folio=concat('%s', lpad(@folioaux,9,'0'))", FormServidor->ObtieneClaveSucursal());
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("update foliosemp set valor=@foliosig where folio='PEDIDOS' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
			instrucciones[num_instrucciones++]=instruccion;
		} else {
			instruccion.sprintf("set @folio='%s'", clave_pedido);
			instrucciones[num_instrucciones++]=instruccion;
		}

		// Si se está modificando entonces borra el detalle que ya exista.
		if (tarea_pedido=="M") {
			instruccion.sprintf("delete from dpedidos where referencia=@folio");
			instrucciones[num_instrucciones++]=instruccion;
		}

		/*AnsiString instruccion_vigencia;
		BufferRespuestas* resp_vig=NULL;

		try{
			instruccion_vigencia.sprintf("SELECT *\
				FROM proveedores WHERE proveedor='%s' ",provRemoto);
			if(mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion_vigencia.c_str(), resp_vig)){
				if (resp_vig->ObtieneNumRegistros()>0){
					 diasvigencia= resp_vig->ObtieneDato("diasvigencia");
				}
			}else{
				throw (Exception("Error al consultar diasvigencia ID_GRA_PEDIDO_PROV"));
			}
		} __finally{
			if (resp_vig!=NULL) delete resp_vig;
		} */

		// Graba la cabecera en la tabla "pedidos"

		AnsiString instruccion_pivote;
		BufferRespuestas* resp_piv=NULL;

		try{
			instruccion_pivote.sprintf("SELECT pv.proveedor FROM proveedores p \
			LEFT JOIN proveedorpivote pv ON pv.rfc = p.rfc \
			WHERE p.proveedor = '%s' ", datos.ObtieneValorCampo("proveedor"));

			if(mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion_pivote.c_str(), resp_piv)){
				if (resp_piv->ObtieneNumRegistros()>0){
					if(resp_piv->ObtieneDato("proveedor") != NULL){
						proveedor = resp_piv->ObtieneDato("proveedor");
					}
				}
			}else{
				throw (Exception("Error al consultar proveedor pivote"));
			}
		} __finally{
			if (resp_piv!=NULL) delete resp_piv;
		}

		if (tarea_pedido=="A") {
			datos.InsCampo("usualta", usuario);
			datos.InsCampo("usumodi", usuario);
			datos.InsCampo("fechaalta", mFg.DateToAnsiString(fecha));
			datos.InsCampo("horaalta", mFg.TimeToAnsiString(hora));
			datos.InsCampo("fechamodi", mFg.DateToAnsiString(fecha));
			datos.InsCampo("horamodi", mFg.TimeToAnsiString(hora));
			datos.InsCampo("cancelado", "0");

			if(proveedor != ""){
			 datos.ReemplazaCampo("proveedor", proveedor);
			}

			if(paramFechaAduanaAuto==1) {
				datos.InsCampo("fechaaduana", mFg.DateToAnsiString(fechaaduana));
            }

			//datos.InsCampo("fechapromesa",fechapromesa);
			//datos.InsCampo("fechavigencia",mFg.DateToAnsiString(fecha)+diasvigencia);
			instrucciones[num_instrucciones++]=datos.GenerarSqlInsert();

			// Guarda el mensaje
			if (mensaje!=" ") { // Un espacio lo tomamos como nulo.
				instruccion.sprintf("insert into pedidoscomprasmensajes (referencia, mensaje) values (@folio,'%s')",
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
				instruccion.sprintf("replace into pedidoscomprasmensajes (referencia, mensaje) values (@folio,'%s')",
					mensaje);
			} else {
				instruccion.sprintf("delete from pedidoscomprasmensajes where referencia=@folio");
			}
			instrucciones[num_instrucciones++]=instruccion;
		}

		// Graba las partidas en "dpedidos"
		num_partidas=StrToInt(mFg.ExtraeStringDeBuffer(&parametros)); // Obtiene el número de partidas
		BufferRespuestas* resp_arteliminado=NULL;
		for (i=0; i<num_partidas; i++) {
			datos.AsignaTabla("dpedidos");
			parametros+=datos.InsCamposDesdeBuffer(parametros);

			// +++ Inicio de sección para asegurar grabar los impuestos vigentes del artículo en caso
			// de que no se hayan mandado desde el cliente
			impuesto1=datos.ObtieneValorCampo("claveimp1");
			articulo=datos.ObtieneValorCampo("articulo");
			costo=mFg.CadenaAFlotante(datos.ObtieneValorCampo("costo"));
			costoimp=mFg.CadenaAFlotante(datos.ObtieneValorCampo("costoimp"));
			if (impuesto1=="") {
				///
				bool excluirIEPS=false;
				// Si está activo el parámetro de excluir IEPS (en supers normalmente es donde está activo)
				if (FormServidor->ObtieneActivoExcluirIEPS()) {
					BufferRespuestas* resp_paramexcluirIEPS=NULL;
					try {
						// busca si el producto está en la lista de excluir IEPS y el primer impuesto es IEPS, entonces activamos la bandera para ignorar este primer impuesto
						instruccion.sprintf("SELECT i1.tipoimpu \
							FROM excluirproductosiesps e \
							INNER JOIN productos p ON e.producto = p.producto \
							INNER JOIN articulos a ON p.producto = a.producto \
							INNER JOIN impuestos i1 ON i1.impuesto = p.claveimpc1 AND i1.tipoimpu = 'IESPS' \
							WHERE a.articulo = '%s' ", articulo);
						if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_paramexcluirIEPS)) {
							if (resp_paramexcluirIEPS->ObtieneNumRegistros()>0){
								excluirIEPS=true;
							} else {
								excluirIEPS=false;
							}
						} else throw (Exception("Error al consultar en tabla parametros"));
					} __finally {
						if (resp_paramexcluirIEPS!=NULL) delete resp_paramexcluirIEPS;
					}
				}
				if(excluirIEPS){
					instruccion.sprintf("SELECT @imp1:=pro.claveimpv2, @imp2:=pro.claveimpv3, \
						@imp3:=pro.claveimpv4, @imp4:=NULL, \
						@cosbas:=pres.costobase \
						FROM articulos a, productos pro, presentacionescb pres \
						WHERE a.articulo='%s' AND a.producto=pro.producto AND a.present=pres.present \
						AND a.producto=pres.producto AND pres.idempresa=%s ",articulo, empresa_sistema);
					instrucciones[num_instrucciones++]=instruccion;

					// Asigna las variables a cada campo
					datos.ReemplazaCampo("claveimp1", "@imp1",1);
					datos.ReemplazaCampo("claveimp2", "@imp2",1);
					datos.ReemplazaCampo("claveimp3", "@imp3",1);
					datos.ReemplazaCampo("claveimp4", "@imp4",1);
				}else{
                    // Pone las claves de los impuestos en variables mysql.
					instruccion.sprintf("select @imp1:=pro.claveimpv1, @imp2:=pro.claveimpv2, \
							@imp3:=pro.claveimpv3, @imp4:=pro.claveimpv4, \
							@cosbas:=pres.costobase \
							from articulos a, productos pro, presentacionescb pres \
							where a.articulo='%s' and a.producto=pro.producto \
							and a.present=pres.present and a.producto=pres.producto and pres.idempresa=%s ",
							articulo, empresa_sistema);
					instrucciones[num_instrucciones++]=instruccion;

					// Asigna las variables a cada campo
					datos.ReemplazaCampo("claveimp1", "@imp1",1);
					datos.ReemplazaCampo("claveimp2", "@imp2",1);
					datos.ReemplazaCampo("claveimp3", "@imp3",1);
					datos.ReemplazaCampo("claveimp4", "@imp4",1);
				}
				///

				if(costo == 0.00 ){
					datos.ReemplazaCampo("costo", "0");
				}
				else{
					datos.ReemplazaCampo("costo",costo );
				}
				if (costoimp == 0.00) {
					datos.ReemplazaCampo("costoimp", "0");
				} else {
					datos.ReemplazaCampo("costoimp",costoimp);
                }

			}
			// +++ Fin de sección para asegurar grabar impuestos
			/* esta validacion es cuando es un nuevo pedidos/compra donde viene desde abastos o calculador de pedidos*/
			//if(esParteRel!="1")
			if(datos.ObtieneIndiceCampo("id")<0)
				datos.InsCampo("id", i);

			datos.InsCampo("referencia", "@folio", 1);
			instrucciones[num_instrucciones++]=datos.GenerarSqlInsert();

			/* VALIDAR SI ALGÚN ARTICULO ESTA ELIMINADO*/
				try {
					instruccion.sprintf("SELECT articulo FROM articulos WHERE articulo = '%s'",articulo);
					if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_arteliminado)) {
						if (resp_arteliminado->ObtieneNumRegistros()==0)
							throw (Exception("El artículo del renglón #" + IntToStr(i+1) + " fue eliminado. Hay que quitarlo de la compra."));
					} else throw (Exception("Error al consultar los artículos"));
				} __finally {
					if (resp_arteliminado!=NULL) delete resp_arteliminado;
				}
			/*FIN VALIDAR SI ALGÚN ARTICULO ESTA ELIMINADO*/

				/*Evaluar si algún artículo esta inactivo*/
				if( i != num_partidas - 1)
					lista_articulos = lista_articulos + "'" + articulo + "', ";
					else
						lista_articulos = lista_articulos + "'" + articulo + "'";
		}

		/* VALIDAR SI ALGÚN ARTICULO ESTA INACTIVO*/
		BufferRespuestas* resp_compactivo=NULL;
		try {
			instruccion.sprintf("SELECT p.nombre, a.present, a.multiplo, a.activo FROM articulos a INNER JOIN productos p \
					ON p.producto = a.producto WHERE a.articulo IN (%s)",lista_articulos);
			if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_compactivo)) {
				if (resp_compactivo->ObtieneNumRegistros()>0){
					for(int c = 0; c < resp_compactivo->ObtieneNumRegistros(); c++)
					{
						if(resp_compactivo->ObtieneDato("activo") == "0")
							throw(Exception("El artículo " + resp_compactivo->ObtieneDato("nombre") + " " + resp_compactivo->ObtieneDato("present") + " " + resp_compactivo->ObtieneDato("multiplo") + " está inactivo."));

						resp_compactivo->IrAlSiguienteDato();

					}

				} else throw (Exception("No se encuentra ningún artículo"));
			} else throw (Exception("Error al consultar los artículo"));
		} __finally {
			if (resp_compactivo!=NULL) delete resp_compactivo;
		}
		/*FIN VALIDAR SI ALGÚN ARTICULO ESTA INACTIVO*/

		instrucciones[num_instrucciones++]="COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			instruccion.sprintf("select 0 as error, @folio as folio");
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	} __finally {
		delete buffer_sql;
		delete[] instrucciones;
	}
}

//------------------------------------------------------------------------------
//ID_CANC_PEDIDO_PROV
void ServidorCompras::CancelaPedidoProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  CANCELA PEDIDO
	char *buffer_sql=new char[1024*64];
    char *aux_buffer_sql=buffer_sql;
    DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instrucciones[50], instruccion;
	int num_instrucciones=0;
	AnsiString clave, usuario;
    int i,error=0;
    TDate fecha=Today();
	TTime hora=Time();


	try {
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave del pedido (referencia)
		usuario=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que esta cancelando el pedido.


		//cuando se quiera cancelar un pedido a proveedor y en ella este algun producto inactivo, es necesario evitar
		//este resgistro, esto, con la finalidad de evitar productos incongruentes
		BufferRespuestas* buffer_resp_artinactivos=NULL;
		try {
			instruccion.sprintf("SELECT @error:=IF(a.activo=0, 1, 0) AS error,\
						a.producto, a.present, a.multiplo, a.factor,pro.nombre \
						FROM pedidos p \
						LEFT JOIN dpedidos dp ON dp.referencia=p.referencia \
						LEFT JOIN articulos a ON a.articulo=dp.articulo  \
						LEFT JOIN productos pro ON pro.producto=a.producto\
						WHERE p.referencia='%s' AND a.activo=0",clave );
			if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), buffer_resp_artinactivos)) {
				if (buffer_resp_artinactivos->ObtieneNumRegistros()>0){
					error=1;
					throw (Exception("Hay artículos inactivos en el detalle de pedidos a proveedor a cancelar.\n \
									Favor de activar el artículo o quitarlo de la lista."));
				}
			} else throw (Exception("Error al consultar detalle de los articulos inactivos en pedidos a proveedor "));
		} __finally {
			if (buffer_resp_artinactivos!=NULL) delete buffer_resp_artinactivos;
		}



		if (error==0) {

			instruccion.sprintf("update pedidos set cancelado=1, fechamodi='%s', horamodi='%s', usumodi='%s' where referencia='%s' and cancelado=0", mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario, clave);
			instrucciones[num_instrucciones]=instruccion;
			num_instrucciones++;

			//para cuando el pedido cancelado se origino de un pedido automatico
			BufferRespuestas* buffer_resp_pedauto=NULL;
			try {
				instruccion.sprintf("SELECT 1 FROM pedidos ped \
					INNER JOIN pedidos_automaticos_pedidos pap ON ped.referencia = pap.pedido_id \
					WHERE ped.referencia = '%s' ", clave );
				if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), buffer_resp_pedauto)) {
					if (buffer_resp_pedauto->ObtieneNumRegistros()>0){
						instruccion.sprintf("update pedidos_automaticos_pedidos set cancelado=1 where pedido_id = '%s' ", clave);
						instrucciones[num_instrucciones]=instruccion;
						num_instrucciones++;
					}
				}
			} __finally {
				if (buffer_resp_pedauto!=NULL) delete buffer_resp_pedauto;
			}

			BufferRespuestas* buffer_resp_pedvta=NULL;
			try {
				instruccion.sprintf("SELECT * FROM pedvta_pedprov pvp WHERE pvp.referencia_pedprov  = '%s' ", clave );
				if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), buffer_resp_pedvta)) {
					if (buffer_resp_pedvta->ObtieneNumRegistros()>0){
						instruccion.sprintf("DELETE FROM pedvta_pedprov WHERE referencia_pedprov = '%s' ", clave);
						instrucciones[num_instrucciones]=instruccion;
						num_instrucciones++;
					}
				}
			} __finally {
				if (buffer_resp_pedvta!=NULL) delete buffer_resp_pedvta;
			}

		}
		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		if(mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)){
            instruccion.sprintf("select %d as error", error);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	} __finally {
		delete buffer_sql;
	}
}

//------------------------------------------------------------------------------
//ID_CON_PEDIDO_PROV
void ServidorCompras::ConsultaPedidoProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    // CONSULTA PEDIDOS
	AnsiString instruccion;
	AnsiString clave;
	AnsiString inner_claveprov = " ", campo_claveprov = " ";
	BufferRespuestas* resp_pedidoauto = NULL;
	AnsiString select_detalle_facvioleta, joins_adicionales, campos_select;
	AnsiString select_clave_proveedor, clave_proveedor;
	AnsiString select_parametro, valor_parametro;
	BufferRespuestas* respuesta_clave_proveedor = NULL;
	BufferRespuestas* respuesta_parametro = NULL;
	AnsiString esParteRel="";

    clave=mFg.ExtraeStringDeBuffer(&parametros);
    mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los generales (de cabecera) del pedido
	instruccion.sprintf("select p.*, com.folioprov, pcm.mensaje, imp.porcentaje, p.impuestoret FROM pedidos p \
				left join pedidoscomprasmensajes pcm on pcm.referencia=p.referencia \
				LEFT JOIN compras com ON com.referencia=p.compra \
				LEFT JOIN impuestos imp ON imp.impuesto = p.impuestoret \
				where p.referencia='%s'", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene todos los datos del proveedor del pedido
	instruccion.sprintf("select pro.*, imp.porcentaje AS isrret, pro.impuestoret from proveedores pro \
	INNER JOIN pedidos ped ON ped.proveedor=pro.proveedor \
	LEFT JOIN impuestos imp ON imp.impuesto = pro.impuestoret where ped.referencia='%s' and ped.proveedor=pro.proveedor", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Los pedidos siempre permiten modificarse (claro que depende si el usuario puede modificar)
	instruccion.sprintf("select 1 as modificar");
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);


	// solo saber si es un proveedor relacional
	//se verificara si el proveedor es parte relacionada
	AnsiString proveedor_esParteRelac;
	BufferRespuestas* resp_prov=NULL;
	try{
		proveedor_esParteRelac.sprintf("select esparterelac from proveedores p, pedidos pc where pc.referencia='%s' and pc.proveedor=p.proveedor", clave );
		if(mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, proveedor_esParteRelac.c_str(), resp_prov)){
			if (resp_prov->ObtieneNumRegistros()>0){
			 esParteRel= resp_prov->ObtieneDato("esparterelac");

			}
		}else{
			throw (Exception("Error al consultar el proveedor parte relacionada ID_GRA_COMPRA"));
		}
   }__finally{
		if (resp_prov!=NULL) delete resp_prov;
   }

	BufferRespuestas * br_ordenPedidoVenta;
	AnsiString ordenPedidoVenta = "0";
	try{
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,
			"SELECT valor FROM parametrosemp p WHERE sucursal = (SELECT valor \
			from parametrosgen where parametro = 'SUCCEDISGLOBAL') AND parametro = 'ORDPEDVENTAF6'",
			br_ordenPedidoVenta);

		ordenPedidoVenta = br_ordenPedidoVenta->ObtieneDato();
	}__finally{
		delete br_ordenPedidoVenta;
	}


	try{
		select_clave_proveedor.sprintf("SELECT proveedor FROM pedidos WHERE referencia = '%s'", clave);
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, select_clave_proveedor.c_str(), respuesta_clave_proveedor);
		if(respuesta_clave_proveedor->ObtieneNumRegistros() > 0){
			clave_proveedor = respuesta_clave_proveedor->ObtieneDato();
		}
	}__finally{
		if (respuesta_clave_proveedor != NULL){
			delete respuesta_clave_proveedor;
		}
	}

	try{
		select_parametro.sprintf("SELECT valor FROM parametrosemp\
		WHERE parametro = 'IMPPEDPROVREL' AND sucursal = '%s'", FormServidor->ObtieneClaveSucursal());
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, select_parametro.c_str(), respuesta_parametro);
		if(respuesta_parametro->ObtieneNumRegistros() > 0){
			valor_parametro = respuesta_parametro->ObtieneDato();
		}
	}__finally{
		if (respuesta_parametro != NULL){
			delete respuesta_parametro;
		}
	}

	//Para obtener clave prod dependiendo de si es para pedidos o pedidos automaticos
	try{
		instruccion.sprintf("SELECT 1 FROM pedidos ped \
			INNER JOIN pedidos_automaticos_pedidos pap ON ped.referencia = pap.pedido_id WHERE ped.referencia = '%s' ", clave);
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), resp_pedidoauto);

		if(resp_pedidoauto->ObtieneNumRegistros() > 0){
			campo_claveprov = " IFNULL(clvprod.claveproductoproveedor,'') AS claveproductoproveedor, ";
			inner_claveprov = " LEFT JOIN claveproducto_proveedor clvprod ON a.producto = clvprod.producto AND a.present = clvprod.present AND ped.proveedor = clvprod.proveedor ";
		} else {
			campo_claveprov = " IFNULL(ap.claveproductoproveedor,'') AS claveproductoproveedor, ";
			inner_claveprov = " LEFT JOIN articulosped AS ap ON ap.producto=a.producto AND ap.present=a.present AND ap.proveedor=ped.proveedor AND ap.sucursal = sec.sucursal ";
		}
	}__finally{
		if (resp_pedidoauto != NULL){
			delete resp_pedidoauto;
		}
	}

	if (clave_proveedor == "S1001497" && valor_parametro == "1") {
		campos_select = "SELECT d.referencia, d.articulo, dv.cantidad, d.costo, ";

		joins_adicionales = " INNER JOIN pedvta_pedprov pvpp\
		ON d.referencia = pvpp.referencia_pedprov\
		INNER JOIN pedidosventa pv\
		ON pvpp.referencia_pedvta = pv.referencia\
		INNER JOIN dventas dv\
		ON pv.venta = dv.referencia AND d.articulo = dv.articulo ";
	} else {
		campos_select = "select d.referencia, d.articulo, d.cantidad, d.costo, ";
        joins_adicionales = " ";
	}

	// Obtiene todo el detalle del pedido con algunos datos extras que necesita el cliente
	instruccion=campos_select;
	instruccion+="a.present, p.producto, p.nombre, a.multiplo, a.factor, ";
    instruccion+="d.claveimp1, i1.tipoimpu as tipoimpuesto1, i1.porcentaje as porcentajeimp1, t1.nombre as nomtipoimp1, ";
    instruccion+="d.claveimp2, i2.tipoimpu as tipoimpuesto2, i2.porcentaje as porcentajeimp2, t2.nombre as nomtipoimp2, ";
	instruccion+="d.claveimp3, i3.tipoimpu as tipoimpuesto3, i3.porcentaje as porcentajeimp3, t3.nombre as nomtipoimp3, ";
	instruccion+="d.claveimp4, i4.tipoimpu as tipoimpuesto4, i4.porcentaje as porcentajeimp4, t4.nombre as nomtipoimp4, ";
	instruccion+="d.iepscuota, dr.cantidad as rcantidad,";
	instruccion+="(a.factor / pmm.cajalogisticafactor) AS cajasbultos, ";
	instruccion += campo_claveprov;
	instruccion+="IF(a.factor=1,1,2) ORDEN, p.clasif1, d.id, ";
	instruccion+="pr.recepcion as recepcion, ";
	instruccion+="ped.foliofactura AS factura, a.present,";
	instruccion+="CONCAT(emp.nombre,' ',emp.appat,' ',emp.apmat) AS comprador, ";
	instruccion+="SUM(d.cantidad) - SUM(dr.cantidad) AS faltantes, ";
	instruccion+="fechaaduana, ped.almacen ,";
	instruccion+="IFNULL(cfg.permitecompras, 1) permitecompras ";
	instruccion+="from dpedidos d  inner join  articulos a  inner join  productos p ";
	instruccion+="left join impuestos i1 on i1.impuesto=d.claveimp1 ";
	instruccion+="left join impuestos i2 on i2.impuesto=d.claveimp2 ";
	instruccion+="left join impuestos i3 on i3.impuesto=d.claveimp3 ";
	instruccion+="left join impuestos i4 on i4.impuesto=d.claveimp4 ";
	instruccion+="left join tiposdeimpuestos t1 on t1.tipoimpu=i1.tipoimpu ";
	instruccion+="left join tiposdeimpuestos t2 on t2.tipoimpu=i2.tipoimpu ";
	instruccion+="left join tiposdeimpuestos t3 on t3.tipoimpu=i3.tipoimpu ";
	instruccion+="left join tiposdeimpuestos t4 on t4.tipoimpu=i4.tipoimpu ";
	instruccion+="LEFT JOIN pedidos as ped on ped.referencia=d.referencia ";
	instruccion+="LEFT JOIN almacenes alma ON alma.almacen = ped.almacen ";
	instruccion+="LEFT JOIN secciones sec ON sec.seccion = alma.seccion ";
	instruccion+= inner_claveprov;
	instruccion+="LEFT JOIN presentacionesminmax pmm ON a.producto=pmm.producto AND a.present=pmm.present ";
	instruccion+="LEFT JOIN pedrecepcion pr ON d.referencia=pr.pedido ";
	instruccion+="LEFT JOIN drecepciones dr ON dr.recepcion=pr.recepcion ";
	instruccion+="INNER JOIN empleados emp ON emp.empleado=ped.comprador ";
	instruccion+="left JOIN articuloempresacfg cfg ON cfg.articulo = d.articulo AND cfg.idempresa = ";
	instruccion+= AnsiString(FormServidor->ObtieneClaveEmpresa());
	instruccion+= joins_adicionales;
	instruccion+=" where d.referencia='";
	instruccion+=clave;
	instruccion+="' and d.articulo=a.articulo and a.producto=p.producto ";
	instruccion+=" group by p.nombre, a.present, a.multiplo ";
	if(esParteRel=="1" && ordenPedidoVenta=="1")
		instruccion+=" ORDER BY a.factor = 1, IF(a.factor = 1, p.clasif1, left(multiplo,3)), p.nombre, a.present, a.multiplo ";
	else
		instruccion+=" ORDER BY p.nombre, a.present, a.multiplo ";

	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	if (clave_proveedor == "S1001497" && valor_parametro == "1") {
		instruccion.sprintf("SELECT\
		pvpp.referencia_pedprov,\
		dp.articulo,\
		dv.cantidad,\
		dp.costo,\
		dp.costoimp,\
		art.present,\
		art.producto,\
		prod.nombre,\
		art.multiplo,\
		'' AS claveproductoproveedor,\
		(dp.costo * dv.cantidad) AS costotot,\
		(dv.precioimp * dv.cantidad) AS costototimp,\
		dp.id\
		FROM pedvta_pedprov pvpp\
		INNER JOIN pedidosventa pv\
			ON pvpp.referencia_pedvta = pv.referencia\
		INNER JOIN dventas dv\
			ON pv.venta = dv.referencia\
		INNER JOIN dpedidos dp\
			ON pvpp.referencia_pedprov = dp.referencia AND dv.articulo = dp.articulo\
		INNER JOIN articulos art\
			ON dp.articulo = art.articulo\
		INNER JOIN productos prod\
			ON art.producto = prod.producto\
		WHERE pvpp.referencia_pedprov = '%s'\
		ORDER BY prod.nombre, art.present, art.multiplo", clave);
	} else {
		instruccion.sprintf("select d.articulo, d.cantidad, d.costo, d.costoimp, \
		a.present, p.producto, p.nombre, a.multiplo, \
		%s \
		(d.costo * d.cantidad) AS costotot, (d.costoimp * d.cantidad) AS costototimp, \
		d.id \
		FROM dpedidos d  \
		INNER JOIN articulos a  \
		INNER JOIN productos p \
		LEFT JOIN pedidos as ped on ped.referencia = d.referencia \
		LEFT JOIN almacenes alma ON alma.almacen = ped.almacen \
		LEFT JOIN secciones sec ON sec.seccion = alma.seccion \
		%s \
		LEFT JOIN presentacionesminmax pmm ON a.producto = pmm.producto AND a.present = pmm.present \
		INNER JOIN empleados emp ON emp.empleado = ped.comprador \
		LEFT JOIN articuloempresacfg cfg ON cfg.articulo = d.articulo AND cfg.idempresa = %s \
		WHERE d.referencia = '%s' AND d.articulo = a.articulo AND a.producto = p.producto \
		GROUP BY p.nombre, a.present, a.multiplo \
		ORDER BY p.nombre, a.present, a.multiplo",
		campo_claveprov, inner_claveprov, FormServidor->ObtieneClaveEmpresa(), clave);
	}

	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}

//------------------------------------------------------------------------------
//ID_GRA_DEVOL_PROV
void ServidorCompras::GrabaDevolProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    //  GRABA DEVOLUCION
	char *buffer_sql=new char[10000*1000];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString tarea_devol, clave_devol, clave_compra, usuario, terminal;
	int num_partidas, i;
	int num_instrucciones=0;
	AnsiString instruccion, tipo;
	AnsiString * instrucciones = new AnsiString[10000];
	double valor;
	TDateTime fecha_dev;
	int error=0;
	AnsiString articulo, costo_art;
	AnsiString modcosto;
	TDate fecha=Today();
	TTime hora=Time();
	BufferRespuestas* resp_costo=NULL;
	AnsiString tipobitacoracosto;
	BufferRespuestas* resp_verificacion=NULL;
	AnsiString mensaje;
    AnsiString empresa_sistema = FormServidor->ObtieneClaveEmpresa();

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

		datos.AsignaTabla("notascredprov");

		// Obtiene los datos de la tabla de notas de crédito
		datos.AsignaTabla("notascredprov");
		parametros+=datos.InsCamposDesdeBuffer(parametros);

		// Extrae los datos que necesitamos para crear las letras y transacciones.
		datos.AsignaValorCampo("referencia", "@folio", 1);
		valor=StrToFloat(datos.ObtieneValorCampo("valor"));
		fecha_dev=StrToDate(datos.ObtieneValorCampo("fechanot"));
		clave_compra=datos.ObtieneValorCampo("compra");
		tipo =datos.ObtieneValorCampo("tipo");
		// Verificaciones para no permitir modificaciones cuando no sea permitido.
		if (tarea_devol=="M") {

			// Obtiene el folio de la compra correspondiente y el valor de la nota
			instruccion.sprintf("select @compra:=compra, @valor:=valor from notascredprov where referencia='%s'",clave_devol);
			mServidorVioleta->EjecutaSelectSqlNulo(Respuesta,  MySQL, instruccion.c_str());

			// Verifica que no haya notas de crédito posteriores de la misma compra
			instruccion.sprintf("select @error:=if((COALESCE(sum(t.valor),0))<0, 1, 0) as error from notascredprov n, transxpag t where n.compra=@compra and n.referencia>'%s' and n.compra=t.referencia and n.cancelado=0 and t.cancelada=0 and t.tipo='DEVO'", clave_devol);
			mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 2, error);

			// Verifica que la fecha de la nota sea posterior a la fecha de cierre.
			instruccion.sprintf("select @error:=if(n.fechanot<=cast(e.valor as datetime), 1, 0) as error from notascredprov n left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' where n.referencia='%s'",FormServidor->ObtieneClaveSucursal(), clave_devol);
			mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 3, error);

			// Verifica que la notas de crédito tenga asignado un uuid
			instruccion.sprintf("SELECT @error:=IF(muuid<>'',1,0) AS error FROM notascredprov WHERE referencia='%s' AND cancelado=0 GROUP BY referencia", clave_devol);
			mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 6, error);

			tipobitacoracosto = "MNC" ;

			// Verifica que la fecha de la devolución sea posterior a la fecha de cierre.
			instruccion.sprintf("select @error:=if('%s'<=cast(e.valor as datetime), 1, 0) as error from estadosistemaemp e where e.estado='FUCIERRE' AND e.sucursal = '%s'  ", mFg.DateToMySqlDate(fecha_dev), FormServidor->ObtieneClaveSucursal());
			mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 4, error);

		} else {

			// Verifica que la fecha de la devolución sea posterior a la fecha de cierre.
			instruccion.sprintf("select @error:=if('%s'<=cast(e.valor as datetime), 1, 0) as error from estadosistemaemp e where e.estado='FUCIERRE' AND e.sucursal = '%s' ", mFg.DateToMySqlDate(fecha_dev), FormServidor->ObtieneClaveSucursal());
			mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 4, error);

			tipobitacoracosto = "NC";
		}


		if (error==0) {
			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

			// Obtiene el folio para la nota
			if (tarea_devol=="A") {
				instruccion.sprintf("select @folioaux:=valor from foliosemp where folio='NOTASCRPRO' AND sucursal = '%s' %s ",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliosig=@folioaux+1");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @folioaux=cast(@folioaux as char)");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @folio=concat('%s', lpad(@folioaux,9,'0'))", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("update foliosemp set valor=@foliosig where folio='NOTASCRPRO' AND sucursal = '%s' ",FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
			} else {
				instruccion.sprintf("set @folio='%s'", clave_devol);
				instrucciones[num_instrucciones++]=instruccion;
			}

			// Si se está modificando entonces borra el detalle
			if (tarea_devol=="M" && tipo == "0") {
				// Crea tabla temporal para alamacenar cantidades de articulos antes de
				instruccion="CREATE temporary TABLE tmpcantidad(referencia VARCHAR(11) NOT NULL,\
				producto VARCHAR(8) NOT NULL, present VARCHAR(255) NOT NULL, almacen VARCHAR(4) NOT NULL,  \
				cantidad DECIMAL(12,3) NOT NULL, PRIMARY KEY (referencia, producto, present, almacen))";
				instrucciones[num_instrucciones++]=instruccion;

				instruccion.sprintf("INSERT INTO tmpcantidad SELECT d.referencia, a.producto, a.present, c.almacen, \
				SUM(d.cantidad * a.factor) AS cantidad FROM dnotascredprov d INNER JOIN notascredprov n ON   \
				n.referencia = d.referencia INNER JOIN articulos a ON a.articulo = d.articulo   \
				INNER JOIN compras c ON n.compra = c.referencia WHERE \
				d.referencia = '%s' GROUP BY a.producto, a.present", clave_devol);
				instrucciones[num_instrucciones++]=instruccion;

				instruccion = "UPDATE tmpcantidad tmp INNER JOIN existenciasactuales ea ON ea.producto = tmp.producto \
				AND ea.present = tmp.present AND ea.almacen = tmp.almacen SET ea.cantidad = (ea.cantidad  + tmp.cantidad), \
				ea.devcompras = ea.devcompras - tmp.cantidad ";
				instrucciones[num_instrucciones++]=instruccion;
			}

			if (tarea_devol=="M") {
				instruccion.sprintf("delete from dnotascredprov where referencia=@folio");
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
					instruccion.sprintf("insert into notascredprovmensajes (referencia, mensaje) values (@folio,'%s')",
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
					instruccion.sprintf("replace into notascredprovmensajes (referencia, mensaje) values (@folio,'%s')",
						mensaje);
				} else {
					instruccion.sprintf("delete from notascredprovmensajes where referencia=@folio");
				}
				instrucciones[num_instrucciones++]=instruccion;
			}

			instruccion="create temporary table tmparticulosrecalcular (articulo varchar(9), PRIMARY KEY (articulo)) Engine = InnoDB";
			instrucciones[num_instrucciones++]=instruccion;

			instruccion="create temporary table tmpcostos ( \
				articulo varchar(9), producto varchar(8), present varchar(255), \
				costo decimal(16,6), modificar bool, PRIMARY KEY (articulo)) Engine = InnoDB";
			instrucciones[num_instrucciones++]=instruccion;

			// Graba las partidas en "dnotascredprov"
			num_partidas=StrToInt(mFg.ExtraeStringDeBuffer(&parametros)); // Obtiene el número de partidas
			for (i=0; i<num_partidas; i++) {
				datos.AsignaTabla("dnotascredprov");
				parametros+=datos.InsCamposDesdeBuffer(parametros);
				datos.InsCampo("referencia", "@folio", 1);
				articulo=datos.ObtieneValorCampo("articulo");
				costo_art=datos.ObtieneValorCampo("costoimp");
				instrucciones[num_instrucciones++]=datos.GenerarSqlInsert();

				if (tipo == "0") {
					instruccion.sprintf("UPDATE articulos a INNER JOIN dnotascredprov d ON a.articulo = d.articulo \
					INNER JOIN notascredprov n ON n.referencia = d.referencia INNER JOIN compras c ON n.compra = c.referencia \
					INNER JOIN existenciasactuales ea ON a.producto = ea.producto AND a.present = ea.present   \
					AND c.almacen = ea.almacen SET ea.cantidad = (ea.cantidad - (d.cantidad * a.factor)) \
					,ea.devcompras = (ea.devcompras + (d.cantidad * a.factor))  WHERE a.articulo = '%s' AND d.referencia = @folio",articulo);
					instrucciones[num_instrucciones++]=instruccion;
				}

				instruccion.sprintf("insert into tmparticulosrecalcular (articulo) values ('%s')", articulo);
				instrucciones[num_instrucciones++]=instruccion;
			}


			// Obtiene en variables de mysql si se va a reducir el costo base o no
			try{
			instruccion.sprintf("SELECT IF((DATEDIFF(CURDATE(),IF(c.fechaalta <= c.fechacom ,c.fechaalta,c.fechacom))) \
			> p.valor,'NO','SI' ) as modcosto FROM compras c, parametrosglobemp p \
			WHERE c.referencia = '%s'  AND p.parametro = 'MAXCOMPRAMOD' AND p.idempresa = %s ", clave_compra, FormServidor->ObtieneClaveEmpresa());
			if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_costo)) {
			if (resp_costo->ObtieneNumRegistros()>0){
				modcosto=resp_costo->ObtieneDato("modcosto");
			} else throw (Exception("No se encuentra registro"));
			} else throw (Exception("Error al consultar en la tabla"));
			} __finally {
			if (resp_costo!=NULL) delete resp_costo;
			}

		if(tipo != "0"){
			if(modcosto == "SI"){
			// *********** INICIO DE CALCULO DE COSTOS YA INCUYENDO LAS NOTAS DE CREDITO
				// Suma las notas de crédito y deja el resultado en una tabla temporal,
				// para luego hacerle un left join con las compras.
				instruccion="create temporary table tmpdnotascredprov ";
				instruccion+="select d.articulo, ";
				instruccion+="sum(if(n.tipo<>'0',d.costo,0)) as costo, ";
				instruccion+="sum(if(n.tipo<>'0',d.costoimp,0)) as costoimp, ";
				instruccion+="1 as modificar ";
				instruccion+="from notascredprov n, dnotascredprov d, tmparticulosrecalcular art ";
				instruccion+="where n.compra='";
				instruccion+=clave_compra;
				instruccion+="' and n.cancelado=0 and n.referencia=d.referencia and d.articulo=art.articulo ";
				instruccion+="group by d.articulo";
				instrucciones[num_instrucciones++]=instruccion;

				// Obtiene los datos de lo que resta de las compras
				// menos las notas de credito ya aplicadas.
				instruccion="insert into tmpcostos (articulo, costo, modificar) ";
				instruccion+="select dc.articulo, ";
				instruccion+="sum(dc.costoimp-ifnull(dn.costoimp,0)) as costo, ";
				instruccion+="1 as modificar ";
				instruccion+="from compras c  inner join  dcompras dc  inner join  tmparticulosrecalcular art ";
				instruccion+="left join tmpdnotascredprov dn on dn.articulo=dc.articulo ";
				instruccion+="where c.referencia='";
				instruccion+=clave_compra;
				instruccion+="' and c.referencia=dc.referencia ";
				instruccion+="and art.articulo=dc.articulo ";
				instruccion+="group by dc.articulo";
				instrucciones[num_instrucciones++]=instruccion;

				// Agrega la clave de producto y presentacion de los articulos a modificar.
				instruccion.sprintf("update tmpcostos cos, articulos a \
					set cos.producto=a.producto, cos.present=a.present \
					where cos.articulo=a.articulo");
				instrucciones[num_instrucciones++]=instruccion;

			// *********** FIN DE CALCULO DE COSTOS YA INCUYENDO LAS NOTAS DE CREDITO


			// En la tabla auxiliar de costos establece que precios NO se van a modificar
			// a) Los que tengan compras posteriores a la fecha de compra correspondiente a la nota)
			instruccion.sprintf("update compras c, dcompras d, tmpcostos cos set cos.modificar=0 where c.fechacom>'%s' and c.cancelado=0 and d.articulo=cos.articulo and c.referencia=d.referencia", mFg.DateToMySqlDate(fecha_dev));
			instrucciones[num_instrucciones++]=instruccion;
			// b) Tampoco se van a modificar los que tengan costo<=0
			instruccion.sprintf("update tmpcostos cos set cos.modificar=0 where cos.costo<=0");
			instrucciones[num_instrucciones++]=instruccion;

			// Los que están bloqueados para modificación de precios se marcan para NO modificar sus precios.
			instruccion.sprintf("update tmpcostos cos \
					inner join articulos a ON cos.articulo=a.articulo \
					inner join preciosbloqueados pb ON pb.producto=a.producto and pb.present=a.present and pb.idempresa=%s \
				set cos.modificar=0 \
				where pb.fechaVigencia>='%s' ", FormServidor->ObtieneClaveEmpresa(),
				mFg.DateToMySqlDate(fecha));
			instrucciones[num_instrucciones++]=instruccion;

			//Ingresa los cambios de los costos en la bitacora
			instruccion.sprintf("INSERT INTO bitacoracostos SELECT @folio AS referencia,a.articulo, \
			'%s' AS tipo, p.costobase,(tc.costo/a.factor) AS costo,CURDATE(),CURTIME(),'%s', p.idempresa \
			FROM presentacionescb p, tmpcostos tc, articulos a  \
			WHERE tc.articulo=a.articulo AND a.present=p.present AND a.producto=p.producto AND tc.costo<>0     \
			AND tc.modificar=1 AND p.idempresa=%s ",tipobitacoracosto,usuario, empresa_sistema);
			instrucciones[num_instrucciones++]=instruccion;

			// Cambia el costo unitario en la tabla presentaciones.
			instruccion="update tmpcostos cos, presentacionescb p, articulos a ";
			instruccion+="set p.costobase=cos.costo/a.factor ";
			instruccion+=", p.costoultimo=cos.costo/a.factor ";
			instruccion+="where cos.articulo=a.articulo and a.present=p.present and a.producto=p.producto and cos.costo<>0 and cos.modificar=1 ";
			instruccion+=" and p.idempresa=" + empresa_sistema;
			instrucciones[num_instrucciones++]=instruccion;

			// Cambia el costo y los precios en la tabla de precios.
			instruccion.sprintf("select @valor:=valor from parametrosemp where parametro='DIGREDOND' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
			instrucciones[num_instrucciones++]=instruccion;

			AnsiString asignacion_precio;
			if (paramcambioprec=="0")
				asignacion_precio.sprintf("prec.precio=ROUND(ROUND(present.costobase*(1+prec.porcutil/100),@valor)*a.factor,@valor), \
									prec.precioproximo=ROUND(ROUND(present.costobase*(1+prec.porcutil/100),@valor)*a.factor,@valor) ");
			else
				asignacion_precio.sprintf("prec.precioproximo=ROUND(ROUND(present.costobase*(1+prec.porcutil/100),@valor)*a.factor,@valor) ");
			instruccion="update presentacionescb present, tmpcostos cos, precios prec, tiposdeprecios tp, articulos a ";
			instruccion+="set prec.costo=present.costobase, ";
			instruccion+=asignacion_precio;
			instruccion+="where cos.producto=a.producto and cos.present=a.present and cos.costo<>0 and cos.modificar=1 and ";
			instruccion+=" prec.tipoprec=tp.tipoprec and prec.articulo=a.articulo and a.present=present.present and a.producto=present.producto ";
			instruccion+=" and present.idempresa="+empresa_sistema+" and tp.idempresa="+empresa_sistema;
			instrucciones[num_instrucciones++]=instruccion;

			// Obtiene la sucursal a la que afecta la compra a nivel inventario
			// (donde entrará fisicamente es donde se va a vender y ahí afectará los precios)
			instruccion.sprintf("select @sucursal:=sec.sucursal from compras c \
				inner join almacenes a ON a.almacen=c.almacen \
				INNER JOIN secciones sec ON a.seccion=sec.seccion \
				where c.referencia='%s'",clave_compra);
			instrucciones[num_instrucciones++]=instruccion;

			//para consultar el parametro del Digito de Redondeo
			instruccion.sprintf("select @valor:=valor from parametrosemp where parametro='DIGREDOND' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
			instrucciones[num_instrucciones++]=instruccion;

			// Cambia el costo y los precios en la tabla de PRECIOS LOCALES
			AnsiString asignacion_precio_local;
			asignacion_precio_local.sprintf("prec.precio=ROUND(ROUND(present.costobase*(1+prec.porcutil/100),@valor)*a.factor,@valor) ");
			instruccion.sprintf("update preciolocal prec \
				inner join tiposdeprecios tp ON tp.tipoprec=prec.tipoprec \
				inner join articulos a ON prec.articulo=a.articulo \
				inner join presentacionescb present ON a.present=present.present and a.producto=present.producto \
				inner join tmpcostos cos ON cos.producto=a.producto and cos.present=a.present \
				inner join autorizarprecios aut ON aut.sucursal=@sucursal \
					AND aut.producto=a.producto AND aut.present=a.present \
					AND aut.fechavigencia>='%s' \
				set prec.costo=present.costobase, \
				%s \
				where cos.costo<>0 and cos.modificar=1 and \
				prec.sucursal=@sucursal and present.idempresa=%s and tp.idempresa=%s ",
				 mFg.DateToMySqlDate(fecha), asignacion_precio_local, empresa_sistema, empresa_sistema);
			instrucciones[num_instrucciones++]=instruccion;


			////////////////////////////////////////////////////////////////////////////
			//            FIN RECALCULO DE PRECIOS
			////////////////////////////////////////////////////////////////////////////

			}
		}

			// Hace un abono que refleje en proveedores la nota de crédito.
			if (tarea_devol=="A") {
				// Obtiene el folio para la NUEVA transaccion
				instruccion.sprintf("select @foliotranaux:=valor from foliosemp where folio='TRANPROV' AND sucursal = '%s' %s ",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliosigtran=@foliotranaux+1");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliotranaux=cast(@foliotranaux as char)");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliotran=concat('%s', lpad(@foliotranaux,9,'0'))", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("update foliosemp set valor=@foliosigtran where folio='TRANPROV' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("insert into transxpag \
					(tracredito, referencia, notaprov, concepto, destino, tipo, cancelada, aplicada, valor, fechaalta, horaalta, fechamodi, horamodi, fechaapl, usualta, usumodi) \
					values (@foliotran, '%s', @folio, 'A', 'C', 'DEVO', 0,1, -%12.2f, '%s', '%s', '%s', '%s', '%s', '%s', '%s')",
					 clave_compra, valor, mFg.DateToMySqlDate(fecha_dev), mFg.TimeToMySqlTime(hora), mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), mFg.DateToMySqlDate(fecha_dev), usuario, usuario);
				instrucciones[num_instrucciones++]=instruccion;
			} else {
				// Obtiene el folio de la transaccion ya existente
				instruccion.sprintf("select @foliotran:=tracredito from transxpag where referencia='%s' and notaprov=@folio and concepto='A' and destino='C' and tipo='DEVO' and cancelada=0", clave_compra);
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("update transxpag set valor=-%12.2f, fechamodi='%s', horamodi='%s', fechaapl='%s', usumodi='%s' where tracredito=@foliotran", valor, mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), mFg.DateToMySqlDate(fecha_dev), usuario);
				instrucciones[num_instrucciones++]=instruccion;
			}


            	////////////////// INICIO CALCULO DE SALDOS DE COMPRAS

			// Crea una tabla para almacenar los folios de las compras afectadas por el pago
			// para posteriormente recalcular saldos de estas compras.
			instruccion="create temporary table comprasaux (compra char(11), PRIMARY KEY (compra)) Engine = InnoDB";
			instrucciones[num_instrucciones++]=instruccion;

			instruccion="insert into comprasaux (compra) ";
			instruccion+="select t.referencia as compra from transxpag t where t.referencia='";
			instruccion+=clave_compra;
			instruccion+="' and t.cancelada=0 GROUP BY t.referencia";
			instrucciones[num_instrucciones++]=instruccion;

			// Crea una tabla donde se van a poner los saldos de las compras
			// afectadas por la cancelación
			instruccion="create temporary table auxcomprassaldos (compra char(11), saldo decimal(16,2), PRIMARY KEY (compra)) Engine = InnoDB";
			instrucciones[num_instrucciones++]=instruccion;

			// Calcula los saldos de las compras relacionadas con el pago (SALDO B)
			instruccion="insert into auxcomprassaldos (compra, saldo) ";
			instruccion+="select c.referencia as compra, sum(t.valor) as saldo ";
			instruccion+="from compras c, comprasaux caux, transxpag t ";
			instruccion+="where c.referencia=caux.compra and ";
			instruccion+="t.referencia=c.referencia and t.cancelada=0 and c.cancelado=0 ";
			instruccion+="group by c.referencia";
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
					instruccion="select * from auxcomprassaldos where saldo<0";
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
		delete[] instrucciones;
	}
}



//------------------------------------------------------------------------------
//ID_CANC_DEVOL_PROV
void ServidorCompras::CancelaDevolProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  CANCELA DEVOLUCION
	char *buffer_sql=new char[1024*64];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instrucciones[70], instruccion;
	int num_instrucciones=0;
	AnsiString clave, usuario;
	AnsiString tipo;
	int i, error=0;
	TDate fecha=Today();
	TTime hora=Time();
	AnsiString producto, present, multiplo, nombre ;

	// Consulta de parámetro para asignación de precios
	// * Si es 0 el cambio de precio es inmediato y se guarda en el campo precios.precio
	// * si es 1 entonces el cambio de precios es diferido y se guarda precios.precioproximo para aplicarse posteriormente)
	AnsiString paramcambioprec;
	BufferRespuestas* resp_paramcambioprec=NULL;
	try {
		instruccion.sprintf( "SELECT valor FROM parametrosglobemp WHERE parametro='CAMBIOPRECDIFER' AND idempresa = %s ", FormServidor->ObtieneClaveEmpresa());
		if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_paramcambioprec)) {
			if (resp_paramcambioprec->ObtieneNumRegistros()>0){
				paramcambioprec=resp_paramcambioprec->ObtieneDato("valor");
			} else throw (Exception("No se encuentra registro CAMBIOPRECDIFER en tabla parametrosglobemp"));
		} else throw (Exception("Error al consultar en tabla parametros"));
	} __finally {
		if (resp_paramcambioprec!=NULL) delete resp_paramcambioprec;
	}

	try {
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave de la devolución
		usuario=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que esta cancelando la devolución.
		tipo=mFg.ExtraeStringDeBuffer(&parametros); //Tipo de nota de credito

		// Obtiene el folio de la compra correspondiente y el valor de la nota
		instruccion.sprintf("select @compra:=compra, @valor:=valor from notascredprov where referencia='%s'",clave);
		mServidorVioleta->EjecutaSelectSqlNulo(Respuesta,  MySQL, instruccion.c_str());

		// Verifica que no haya notas de crédito posteriores de la misma compra
		instruccion.sprintf("select @error:=if((COALESCE(sum(t.valor),0))<0, 1, 0) as error from notascredprov n, transxpag t where n.compra=@compra and n.referencia>'%s' and n.compra=t.referencia and n.cancelado=0 and t.cancelada=0 and t.tipo='DEVO'", clave);
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 2, error);

		// Verifica que la fecha de la nota sea posterior a la fecha de cierre.
		instruccion.sprintf("select @error:=if(n.fechanot<=cast(e.valor as datetime), 1, 0) as error from notascredprov n left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' where n.referencia='%s'",FormServidor->ObtieneClaveSucursal(), clave);
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 3, error);

		//cuando se quiera cancelar un pedido a proveedor y en ella este algun producto inactivo, es necesario evitar
		//este resgistro, esto, con la finalidad de evitar productos incongruentes
		BufferRespuestas* buffer_resp_artinactivos=NULL;
		try {
			instruccion.sprintf("SELECT @error:=IF(a.activo=0, 1, 0) AS error, a.producto, a.present, a.multiplo, a.factor,pro.nombre, n.tipo  \
							FROM notascredprov n \
							LEFT JOIN dnotascredprov dnp ON dnp.referencia=n.referencia \
							LEFT JOIN articulos a ON a.articulo=dnp.articulo  \
							LEFT JOIN productos pro ON pro.producto=a.producto \
							WHERE n.referencia='%s' AND a.activo=0",clave );
			if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), buffer_resp_artinactivos)) {
				if (buffer_resp_artinactivos->ObtieneNumRegistros()>0){
					producto = buffer_resp_artinactivos->ObtieneDato("producto");
					present  = buffer_resp_artinactivos->ObtieneDato("present");
					multiplo  = buffer_resp_artinactivos->ObtieneDato("multiplo");
					nombre  = buffer_resp_artinactivos->ObtieneDato("nombre");
					error=4;
					throw (Exception("Hay artículos inactivos en el detalle de nota crédito proveedores a cancelar.\n Favor de activar el artículo o quitarlo de la lista.\n  artículo:"+nombre+" "+present+" "+multiplo));
				}
			} else throw (Exception("Error al consultar detalle de los articulos inactivos en nota de crédito proveedores "));
		} __finally {
			if (buffer_resp_artinactivos!=NULL) delete buffer_resp_artinactivos;
		}



		if (error==0) {
			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

			instruccion.sprintf("update notascredprov set cancelado=1, usumodi='%s', fechamodi='%s', horamodi='%s' where referencia='%s' and cancelado=0", usuario, mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Cancela el abono correspondiente de la devolución
			instruccion.sprintf("select @foliotran:=tracredito, @compra:=referencia from transxpag where notaprov='%s' and concepto='A' and destino='C' and tipo='DEVO' and cancelada=0", clave);
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("update transxpag set cancelada=1, fechamodi='%s', horamodi='%s', usumodi='%s' where tracredito=@foliotran", mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario );
			instrucciones[num_instrucciones++]=instruccion;

			///////////***************ACTUALIZAR EXISTENCIAS***********************////////////
			instruccion="CREATE temporary TABLE tmpcantidad(referencia VARCHAR(11) NOT NULL,\
			producto VARCHAR(8) NOT NULL, present VARCHAR(255) NOT NULL, almacen VARCHAR(4) NOT NULL,  \
			cantidad DECIMAL(12,3) NOT NULL, PRIMARY KEY (referencia, producto, present, almacen))";
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("INSERT INTO tmpcantidad SELECT d.referencia, a.producto, a.present, c.almacen, \
			SUM(d.cantidad * a.factor) AS cantidad FROM dnotascredprov d INNER JOIN notascredprov n ON  \
			n.referencia = d.referencia AND n.tipo = 0 INNER JOIN articulos a ON a.articulo = d.articulo   \
			INNER JOIN compras c ON n.compra = c.referencia WHERE d.referencia = '%s' \
			GROUP BY a.producto, a.present",clave);
			instrucciones[num_instrucciones++]=instruccion;

			instruccion = "UPDATE tmpcantidad tmp INNER JOIN existenciasactuales ea ON ea.producto = tmp.producto \
			AND ea.present = tmp.present AND ea.almacen = tmp.almacen SET ea.cantidad = (ea.cantidad  + tmp.cantidad) \
			, ea.devcompras = ea.devcompras - (tmp.cantidad)";
			instrucciones[num_instrucciones++]=instruccion;
			/////////////**************FIN DE ACTUALIZAR EXISTENCIAS******************/////////////////


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
//ID_CON_DEVOL_PROV
void ServidorCompras::ConsultaDevolProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA NOTAS DE CREDITO
	AnsiString instruccion;
	AnsiString clave,orden;

	clave=mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
	orden=mFg.ExtraeStringDeBuffer(&parametros);

	// Obtiene todos los generales (de cabecera) de la nota de credito.
	instruccion.sprintf("select ncp.*, @foliocompra:=ncp.compra, ncpm.mensaje, ncp.impuestoret, imp.porcentaje from notascredprov ncp \
	left join notascredprovmensajes ncpm ON ncpm.referencia = ncp.referencia \
	left join impuestos imp on imp.impuesto = ncp.impuestoret  where ncp.referencia='%s'", clave);
    mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene los datos de la compra respectiva.
    instruccion.sprintf("select c.* from compras c where c.referencia=@foliocompra");
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

    // Calcula el saldo de la compra (SALDO B)
	instruccion.sprintf("select sum(if(cancelada=0,valor,0)) as saldo from transxpag where referencia=@foliocompra and cancelada=0 group by referencia");
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

    // Obtiene todos los datos del proveedor de la nota de credito.
    instruccion.sprintf("select p.* from proveedores p, compras c where c.referencia=@foliocompra and c.proveedor=p.proveedor");
    mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

    // NULO:  Verifica que la fecha de la factura sea posterior a la fecha de cierre.
	instruccion.sprintf("select if(n.fechanot<=cast(e.valor as datetime), 1, 0) as modificar from notascredprov n left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' where n.referencia='%s'",FormServidor->ObtieneClaveSucursal(), clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Suma las notas de crédito y deja el resultado en una tabla temporal,
	// para luego hacerle un left join con las compras.
	instruccion="create temporary table  dnotascredprovaux ";
	instruccion+="select d.articulo, sum(if(n.tipo='0',d.cantidad,0)) as cantidad, ";
	instruccion+="sum(if(n.tipo<>'0',d.costo,0)) as costo ";
	instruccion+="from notascredprov n, dnotascredprov d ";
	instruccion+="where n.compra=@foliocompra";
	instruccion+=" and n.cancelado=0 and n.referencia=d.referencia ";
	instruccion+="group by d.articulo";
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query EjecutaSelectSqlNulo"));

	// Sumatoria de todas las notas de crédito.
	instruccion.sprintf("select ifnull(sum(valor),0) as sumnotas from notascredprov where cancelado=0 and compra=@foliocompra");
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene los datos de lo que resta de las compras
	// menos las notas de credito ya aplicadas.
	instruccion="select dc.referencia, dc.articulo, sum(dc.cantidad-ifnull(dn.cantidad,0)) as cantidad, ";
	instruccion+="sum(dc.costo-ifnull(dn.costo,0)) as costo, ";
	instruccion+="a.present, p.producto, p.nombre, a.multiplo, ";
	instruccion+="dc.claveimp1, i1.tipoimpu as tipoimpuesto1, i1.porcentaje as porcentajeimp1, t1.nombre as nomtipoimp1, ";
	instruccion+="dc.claveimp2, i2.tipoimpu as tipoimpuesto2, i2.porcentaje as porcentajeimp2, t2.nombre as nomtipoimp2, ";
	instruccion+="dc.claveimp3, i3.tipoimpu as tipoimpuesto3, i3.porcentaje as porcentajeimp3, t3.nombre as nomtipoimp3, ";
	instruccion+="dc.claveimp4, i4.tipoimpu as tipoimpuesto4, i4.porcentaje as porcentajeimp4, t4.nombre as nomtipoimp4, ";
	instruccion+="dc.iepscuota ";
	if(orden=="1")
			instruccion+=" ,IF(a.factor=1,1,2) ORDEN,p.clasif1  "; //Agrupacion para ordenar por multiplo

	instruccion+="from compras c  inner join  dcompras dc  inner join  articulos a  inner join  productos p ";
	instruccion+="left join impuestos i1 on i1.impuesto=dc.claveimp1 ";
	instruccion+="left join impuestos i2 on i2.impuesto=dc.claveimp2 ";
	instruccion+="left join impuestos i3 on i3.impuesto=dc.claveimp3 ";
	instruccion+="left join impuestos i4 on i4.impuesto=dc.claveimp4 ";
	instruccion+="left join tiposdeimpuestos t1 on t1.tipoimpu=i1.tipoimpu ";
	instruccion+="left join tiposdeimpuestos t2 on t2.tipoimpu=i2.tipoimpu ";
	instruccion+="left join tiposdeimpuestos t3 on t3.tipoimpu=i3.tipoimpu ";
	instruccion+="left join tiposdeimpuestos t4 on t4.tipoimpu=i4.tipoimpu ";
	instruccion+="left join dnotascredprovaux dn on dn.articulo=dc.articulo ";
	instruccion+="where c.referencia=@foliocompra";
	instruccion+=" and c.referencia=dc.referencia ";
	instruccion+="and dc.articulo=a.articulo and a.producto=p.producto ";
	instruccion+="group by dc.articulo";
	if(orden=="1")
			instruccion+=" ORDER BY ORDEN DESC,IF(ORDEN='1',p.clasif1,left(multiplo,3)),p.nombre, a.present, a.multiplo ";
	else
			instruccion+=" order by p.nombre, a.present, a.multiplo ";

	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene todo el detalle de la nota con algunos datos extras que necesita el cliente
	instruccion="select dn.referencia, dn.articulo, dn.cantidad, dn.costo, ";
	instruccion+="a.present, p.producto, p.nombre, a.multiplo, ";
	instruccion+="dc.claveimp1, i1.tipoimpu as tipoimpuesto1, i1.porcentaje as porcentajeimp1, t1.nombre as nomtipoimp1, ";
	instruccion+="dc.claveimp2, i2.tipoimpu as tipoimpuesto2, i2.porcentaje as porcentajeimp2, t2.nombre as nomtipoimp2, ";
	instruccion+="dc.claveimp3, i3.tipoimpu as tipoimpuesto3, i3.porcentaje as porcentajeimp3, t3.nombre as nomtipoimp3, ";
	instruccion+="dc.claveimp4, i4.tipoimpu as tipoimpuesto4, i4.porcentaje as porcentajeimp4, t4.nombre as nomtipoimp4, ";
	instruccion+="dc.iepscuota ";
	if(orden=="1")
			instruccion+=" ,IF(a.factor=1,1,2) ORDEN,p.clasif1  "; //Agrupacion para ordenar por multiplo
	instruccion+="from notascredprov np  inner join  dnotascredprov dn  inner join  dcompras dc  inner join  articulos a  inner join  productos p ";
	instruccion+="left join impuestos i1 on i1.impuesto=dc.claveimp1 ";
	instruccion+="left join impuestos i2 on i2.impuesto=dc.claveimp2 ";
	instruccion+="left join impuestos i3 on i3.impuesto=dc.claveimp3 ";
	instruccion+="left join impuestos i4 on i4.impuesto=dc.claveimp4 ";
	instruccion+="left join tiposdeimpuestos t1 on t1.tipoimpu=i1.tipoimpu ";
	instruccion+="left join tiposdeimpuestos t2 on t2.tipoimpu=i2.tipoimpu ";
	instruccion+="left join tiposdeimpuestos t3 on t3.tipoimpu=i3.tipoimpu ";
	instruccion+="left join tiposdeimpuestos t4 on t4.tipoimpu=i4.tipoimpu ";
	instruccion+="where np.referencia='";
	instruccion+=clave;
	instruccion+="' and np.compra=dc.referencia and np.referencia=dn.referencia ";
	instruccion+=" and dc.articulo=dn.articulo and dn.articulo=a.articulo ";
	instruccion+=" and a.producto=p.producto and a.producto=p.producto";
	if(orden=="1")
			instruccion+=" ORDER BY ORDEN DESC,IF(ORDEN='1',p.clasif1,left(multiplo,3)),p.nombre, a.present, a.multiplo ";
	else
			instruccion+=" order by p.nombre, a.present, a.multiplo ";

	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}

//------------------------------------------------------------------------------
//ID_GRA_PAGO_PROV
void ServidorCompras::GrabaPagoProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  GRABA PAGO DE PROVEEDOR
	char *buffer_sql=new char[1024*64*50];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	int i, num_transacciones;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[5000];
	AnsiString clave_pago, tarea_pago;
	AnsiString proveedor, identificador, forma_pago, valor, ajuste, ajuste_bancario;
	AnsiString num_cheque, banco_cheque, tipo_cheque, fecha_cobro_cheque, usuario;
	AnsiString folio_compra_sistema, valor_tran, folio_tran;
	AnsiString aplicada="1", estado_cheque,banco_cheque2=" " ;
	BufferRespuestas* resp_verificacion=NULL;
	int error;
	TDate fecha=Today();
	TTime hora=Time();
	AnsiString terminal, bancos_movbancos, bancos_NumCta, bancos_fechabancos, esperacomplemento;

	AnsiString valor_banco, valor_tran_banco;
	double valor_banco_double, valor_tran_banco_double, valor_tran_banco_acumulado_double=0,
	ajuste_double, ajuste_double_restante, ajuste_bancario_double;

	try {
		// Grabar pago
		clave_pago=mFg.ExtraeStringDeBuffer(&parametros);
		tarea_pago=mFg.ExtraeStringDeBuffer(&parametros);
		proveedor=mFg.ExtraeStringDeBuffer(&parametros);
		identificador=mFg.ExtraeStringDeBuffer(&parametros);
		forma_pago=mFg.ExtraeStringDeBuffer(&parametros);
		valor=mFg.ExtraeStringDeBuffer(&parametros);
		ajuste=mFg.ExtraeStringDeBuffer(&parametros);
		ajuste_bancario=mFg.ExtraeStringDeBuffer(&parametros);
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
		ajuste_bancario_double=mFg.CadenaAFlotante(ajuste_bancario);
		ajuste_double=mFg.CadenaAFlotante(ajuste);
		valor_banco_double=mFg.CadenaAFlotante(valor)+ ajuste_double + ajuste_bancario_double;
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
								(SELECT valor FROM parametrosemp WHERE parametro='AUTOCONSOLPROV' AND sucursal = '%s') AS paramautoconsol, \
								(SELECT bt.idmovbanco FROM bancosxpag bp inner join transxpag t on t.tracredito=bp.tracredito inner join bancostransacc bt on bt.transacc=bp.transacc where t.pago='%s' limit 1) as idmovbancoant",FormServidor->ObtieneClaveSucursal(), clave_pago);
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

		// Asigna los valores a las banderas de las transacciones dependiendo del
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
				/*instruccion.sprintf("SELECT @sucursaltran:=sec.sucursal from transxpag t \
						inner join ventas v on t.referencia=v.referencia \
						inner join terminales ter on v.terminal=ter.terminal \
						inner join secciones sec on sec.seccion=ter.seccion \
					where t.tracredito=@foliotran " );
				instrucciones[num_instrucciones++]=instruccion;*/

				// Se insertan registros nuevos en bancosmov,
				// ALTA EN BANCOS
				instruccion.sprintf("insert into bancosmov \
					(idmovbanco, idnumcuenta, conceptomov, descripcion, identificador, cancelado, aplicado, \
					subtotal, ivabanco, total, fechaaplbanco, fechaalta, horaalta, fechamodi, horamodi, usualta, usumodi, terminal, origen, sucursal, afectacion) \
					values (NULL, %s, 'D', 'PAGO A PROVEEDOR', '%s', 0, 1, \
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

			instruccion.sprintf("select @folioaux:=valor from foliosemp where folio='PAGPROV' AND sucursal = '%s' %s ",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @foliosig=@folioaux+1");
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @folioaux=cast(@folioaux as char)");
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @folio=concat('%s', lpad(@folioaux,9,'0'))", FormServidor->ObtieneClaveSucursal());
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("update foliosemp set valor=@foliosig where folio='PAGPROV' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("insert into pagosprov \
				(pago, ident, proveedor, fecha, hora, fechamodi, horamodi, valor, cancelado, usualta, usumodi, ajuste, ajuste_bancario, formapag, terminal, esperacomplemento) \
				values (@folio, '%s', '%s', '%s', '%s', '%s', '%s', %s, 0, '%s', '%s', %s, %s, '%s', '%s', %s)",
				identificador, proveedor, mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora),
				mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), valor,
				usuario, usuario, ajuste, ajuste_bancario, forma_pago, terminal, esperacomplemento);
			instrucciones[num_instrucciones++]=instruccion;

			if (forma_pago=="C") {
				// Si es cheque entonces crea un registro en la tabla de cheques.
				instruccion.sprintf("select @foliocheqaux:=valor from foliosemp where folio='CHEQPROV' AND sucursal = '%s' %s ",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliocheqsig=@foliocheqaux+1");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliocheqaux=cast(@foliocheqaux as char)");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliocheq=concat('%s', lpad(@foliocheqaux,9,'0'))", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("update foliosemp set valor=@foliocheqsig where folio='CHEQPROV' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("insert into chequesproveedores \
					(chequeprov,folio,banco,fechaalta,fechacob,valor,proveedor,estado,clasif) \
					values ( @foliocheq,'%s', %s, '%s', '%s', %s, '%s', '%s','%s')",
					num_cheque, banco_cheque.c_str(), mFg.DateToMySqlDate(fecha),
					mFg.StrToMySqlDate(fecha_cobro_cheque), valor, proveedor, estado_cheque, tipo_cheque);
				instrucciones[num_instrucciones++]=instruccion;

				// Crea la relación cheque-pago
				instruccion="insert into cheqxpag (chequeprov,pago) values (@foliocheq, @folio)";
				instrucciones[num_instrucciones++]=instruccion;
			}

		} else {
			// MODIFICACION
			instruccion.sprintf("set @folio='%s'", clave_pago);
			instrucciones[num_instrucciones++]=instruccion;

			// Al modificar un pago, en PAGOSPROV solo se actualizan los campos FECHAMODI, HORAMODI y USUMODI
			instruccion.sprintf("update pagosprov set fechamodi='%s', horamodi='%s', usumodi='%s', ident='%s' where pago=@folio and cancelado=0", mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario, identificador);
			instrucciones[num_instrucciones++]=instruccion;

			// Marca como canceladas todas las transacciones, para
			// posteriormente descancelar solo a las que el usuario haga modificaciones.
			instruccion.sprintf("update transxpag set cancelada=1, fechamodi='%s', horamodi='%s', usumodi='%s' where transxpag.pago=@folio", mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario );
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
				instruccion.sprintf("delete from bancosxpag where transacc in (select transacc from bancostransacc where idmovbanco=@idmovbanconuevo)");
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
				instruccion.sprintf("select @foliotranaux:=valor from foliosemp where folio='TRANPROV' AND sucursal = '%s' %s ",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliotransig=@foliotranaux+1");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliotranaux=cast(@foliotranaux as char)");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliotran=concat('%s', lpad(@foliotranaux,9,'0'))", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("update foliosemp set valor=@foliotransig where folio='TRANPROV' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("insert into transxpag \
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
				instruccion.sprintf("update transxpag set cancelada=0 where tracredito='%s'", folio_tran);
				instrucciones[num_instrucciones++]=instruccion;

				instruccion.sprintf("set @foliotran='%s'", folio_tran);
				instrucciones[num_instrucciones++]=instruccion;
			}

			// Si está activada automatización de bancos y la fecha actual es igual o posterior a la fecha programada de inicio de registro automático
			// Los registros de bancostransacc y bancosxpag se borran en las modificaciones, por lo que también aplica el volver a crearlos.
			if ( forma_pago!="E" && ((idmovbancoant!=""  && idmovbancoant!="0") || (paramautomat=="1" && fecha>=paramfechaini)) ) {
				AnsiString identificador_detalle="";
				if (forma_pago=="C") {
					identificador_detalle=num_cheque+"-"+banco_cheque2.c_str();
				} else {
					identificador_detalle=identificador;
				}

				valor_tran_banco_double=mFg.CadenaAFlotante(valor_tran);
				/*if (ajuste_double<0) {

					// Si hay ajuste negativo resta lo que se va a depositar a las transacciones del movimiento bancario.
					if (valor_tran_banco_double>=ajuste_double_restante) {
						// Si la transaccion actual puede absorver todo el ajuste restante.
						valor_tran_banco_double=valor_tran_banco_double-ajuste_double_restante;
						ajuste_double_restante=0.0f;
					} else {
						if (ajuste_double_restante>0.0f) {
							ajuste_double_restante=ajuste_double_restante-valor_tran_banco_double;
							valor_tran_banco_double=0.0f;
						}
					}
				}*/
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
				instruccion.sprintf("insert into bancosxpag (transacc, tracredito) \
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

		if (ajuste_bancario_double != 0){
			double ajuste_iva, ajuste_subtotal;
			ajuste_subtotal = ajuste_bancario_double/1.16;
			ajuste_iva = ajuste_bancario_double - ajuste_subtotal;

			if (forma_pago!="E" && ((idmovbancoant!="" && idmovbancoant!="0") || (paramautomat=="1" && fecha>=paramfechaini))) {
				instruccion.sprintf("insert into bancostransacc \
					(transacc, idmovbanco, tipodet, identificador, \
					subtotal, ivabanco, total, cveimp) \
					values (NULL, @idmovbanconuevo, 'A', 'AJUSTE_BANCARIO', \
					%s, %s, %s, 8)",
					mFg.FormateaCantidad(ajuste_subtotal,2,false), mFg.FormateaCantidad(ajuste_iva,2,false), ajuste_bancario);
				instrucciones[num_instrucciones++]=instruccion;
			}
		}

		////////////////// INICIO CALCULO DE SALDOS DE COMPRAS

		// Crea una tabla para almacenar los folios de las compras afectadas por el pago
		// para posteriormente recalcular saldos de estas compras.
		instruccion="create temporary table comprasaux (compra char(11), PRIMARY KEY (compra)) Engine = InnoDB";
		instrucciones[num_instrucciones++]=instruccion;

		instruccion="insert into comprasaux (compra) ";
		instruccion+="select t.referencia as compra from transxpag t where t.pago=@folio and t.cancelada=0 ";
		instrucciones[num_instrucciones++]=instruccion;

		// Crea una tabla donde se van a poner los saldos de las compras
		// afectadas por la cancelación
		instruccion="create temporary table auxcomprassaldos (compra char(11), saldo decimal(16,2), PRIMARY KEY (compra)) Engine = InnoDB";
		instrucciones[num_instrucciones++]=instruccion;

		// Calcula los saldos de las compras relacionadas con el pago (SALDO B)
		instruccion="insert into auxcomprassaldos (compra, saldo) ";
		instruccion+="select c.referencia as compra, sum(t.valor) as saldo ";
		instruccion+="from compras c, comprasaux caux, transxpag t ";
		instruccion+="where c.referencia=caux.compra and ";
		instruccion+="t.referencia=c.referencia and t.cancelada=0 and c.cancelado=0 ";
		instruccion+="group by c.referencia";
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
			instruccion="select * from auxcomprassaldos where saldo<0";
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
					instruccion="select * from auxcomprassaldos where saldo<0";
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
//ID_CON_PAGO_PROV
void ServidorCompras::ConsultaPagoProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA PAGO DE PROVEEDOR
	AnsiString instruccion;
	AnsiString clave;

	clave=mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los generales del pago
	instruccion.sprintf("select * from pagosprov where pago='%s'", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene todos los datos del cheque en cuestión
	instruccion.sprintf("select chequesproveedores.* \
		from chequesproveedores, cheqxpag \
		where cheqxpag.pago='%s' and \
		cheqxpag.chequeprov=chequesproveedores.chequeprov", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene todas las transacciones correspondientes al pago.
	instruccion.sprintf("select c.muuid, t.tracredito, t.referencia, t.notaprov, t.pago, t.concepto, \
		t.tipo, t.cancelada, (t.valor*-1) as valor, t.fechaalta, t.horaalta, t.fechamodi, t.usualta, t.usumodi, \
		c.folioprov from transxpag t, compras c \
		where t.referencia=c.referencia and t.pago='%s' and t.cancelada=0", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Verifica que la fecha del pago no sea posterior a la fecha de cierre.
	instruccion.sprintf("select @error:=if((ifnull(chp.fechacob, '1900-01-01')<=cast(e.valor as datetime) \
		and chp.estado=ifnull(chp.estado, 'C')), 1, 0) \
		as error from pagosprov p  inner join  cheqxpag chxp  inner join  chequesproveedores chp \
		left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' where p.pago='%s' \
		and chxp.pago=p.pago and chxp.chequeprov=chp.chequeprov",FormServidor->ObtieneClaveSucursal(), clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);


	// **************************************************************************************
	// Obtiene todos los datos de bancos
	instruccion.sprintf("SELECT bm.idmovbanco, bcue.idnumcuenta, bcue.numerocuenta, bm.fechaaplbanco \
		FROM bancosxpag bpag \
		INNER JOIN bancostransacc bt ON bpag.transacc=bt.transacc \
		INNER JOIN bancosmov bm ON bt.idmovbanco=bm.idmovbanco \
		INNER JOIN bancoscuentas bcue ON bcue.idnumcuenta=bm.idnumcuenta \
		INNER JOIN transxpag t ON t.tracredito=bpag.tracredito \
		WHERE t.pago='%s' and bm.aplicado=1 and bm.cancelado=0 limit 1 ", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

}

//------------------------------------------------------------------------------
//ID_CANC_PAGO_PROV
void ServidorCompras::CancelaPagoProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
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
			as error from pagosprov p  inner join  cheqxpag chxp  inner join  chequesproveedores chp \
			left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' where p.pago='%s' \
			and p.pago=chxp.pago and chxp.chequeprov=chp.chequeprov",FormServidor->ObtieneClaveSucursal(), clave);
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 1, error);

		if (error==0) {
			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

			instruccion.sprintf("update pagosprov set cancelado=1, fechamodi='%s', horamodi='%s', usumodi='%s' where pago='%s' and cancelado=0", mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario, clave);
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("update transxpag set cancelada=1, aplicada=0, fechamodi='%s', horamodi='%s', usumodi='%s' where pago='%s' and cancelada=0", mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario, clave);
			instrucciones[num_instrucciones++]=instruccion;

			if (estado!="") {
				instruccion.sprintf("update chequesproveedores, cheqxpag \
					set chequesproveedores.estado='%s' \
					where cheqxpag.pago='%s' and chequesproveedores.chequeprov=cheqxpag.chequeprov", estado, clave);
				instrucciones[num_instrucciones++]=instruccion;
			}

			// *********************************************************************************************
			// ------ INICIO Cancela el movimiento o suprime las transacciones parciales segun corresponda.
			//        (EXCEPTO EN PAGOS EN EFECTIVO).
			instruccion.sprintf("SELECT @idmovbancoanterior:=bt.idmovbanco, @ajuste:=p.ajuste, \
				@ajuste_bancario:=p.ajuste_bancario \
				FROM bancosxpag bc \
				inner join transxpag t on t.tracredito=bc.tracredito \
				inner join bancostransacc bt on bt.transacc=bc.transacc \
				INNER JOIN pagosprov p ON p.pago=t.pago \
				where t.pago='%s' AND p.formapag<>'E' limit 1", clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Inserta en una tabla temporal todas las transacciones del movimiento.
			instruccion="create temporary table bancostransacctmp ( \
				idtemp int(11)  AUTO_INCREMENT, \
				transacc int(11), \
				PRIMARY KEY (idtemp) ) Engine = InnoDB";
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("insert into bancostransacctmp (idtemp, transacc) \
				SELECT NULL as idtemp, bt.transacc \
				  FROM bancostransacc bt \
				  INNER JOIN bancosxpag bxp ON bt.transacc=bxp.transacc \
				  INNER JOIN transxpag txc ON bxp.tracredito=txc.tracredito AND txc.pago='%s' \
				  WHERE bt.idmovbanco=@idmovbancoanterior", clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Borra las transacciones de bancosxpag correspondientes a las transacciones del movimiento
			instruccion.sprintf("delete from bancosxpag where transacc in (select transacc from bancostransacctmp)");
			instrucciones[num_instrucciones++]=instruccion;

			// Borra las transacciones del movimiento.
			instruccion.sprintf("delete from bancostransacc where transacc in (select transacc from bancostransacctmp)");
			instrucciones[num_instrucciones++]=instruccion;

			// Borra la transacción de ajuste que corresponda al movimiento original por el mismo monto.
			instruccion.sprintf("delete from bancostransacc where idmovbanco=@idmovbancoanterior and tipodet='A' and identificador='AJUSTE' and total=@ajuste limit 1");
			instrucciones[num_instrucciones++]=instruccion;

            // Borra la transaccion de ajuste bancario que corresponda al movimiento original por el mismo monto.
			instruccion.sprintf("delete from bancostransacc where idmovbanco=@idmovbancoanterior and tipodet='A' and identificador='AJUSTE_BANCARIO' and total=@ajuste_bancario limit 1");
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

			// ------ FIN Cancela el movimiento o suprime las transacciones parciales segun corresponda.
			// *********************************************************************************************


			/*
			// ------ INICIO Sección que inserta el movimiento bancario contrario al del pago original.

			instruccion.sprintf("SELECT @idmovbancoanterior:=bt.idmovbanco FROM bancosxpag bc \
				inner join transxpag t on t.tracredito=bc.tracredito \
				inner join bancostransacc bt on bt.transacc=bc.transacc \
				where t.pago='%s' limit 1", clave);
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("INSERT INTO bancosmov \
				(idmovbanco, idnumcuenta, conceptomov, descripcion, identificador, cancelado, aplicado, subtotal, \
					ivabanco, total, fechaaplbanco, fechaalta, horaalta, fechamodi, horamodi, usualta, usumodi, terminal, origen, sucursal, afectacion) \
				SELECT NULL, idnumcuenta, conceptomov, 'CANCELA PAGO DE PROVEEDOR' as descripcion, identificador, cancelado, aplicado, subtotal, \
					ivabanco, total, fechaaplbanco, fechaalta, horaalta, '%s' as fechamodi, '%s' as horamodi, usualta, '%s' as usumodi, '%s' as terminal, 'CANPP', \
					'%s' as sucursal, 'C' as afectacion \
					FROM bancosmov where idmovbanco=@idmovbancoanterior",
					mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario, terminal, FormServidor->ObtieneClaveSucursal() );
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("set @idmovbanconuevo=LAST_INSERT_ID()");
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("INSERT INTO bancostransacc \
					(transacc, idmovbanco, tipodet, identificador, \
					subtotal, ivabanco, total, cveimp) \
				SELECT NULL, @idmovbanconuevo, tipodet, identificador, \
					subtotal, ivabanco, total, cveimp \
					FROM bancostransacc where idmovbanco=@idmovbancoanterior" );
			instrucciones[num_instrucciones++]=instruccion;

			instruccion="create temporary table transxpagtmp ( \
				idtemp int(11)  AUTO_INCREMENT, \
				tracredito varchar(11) not null, \
				PRIMARY KEY (idtemp) ) Engine = InnoDB";
			instrucciones[num_instrucciones++]=instruccion;

			instruccion="insert into transxpagtmp (idtemp, tracredito) \
				SELECT NULL as idtemp, bxp.tracredito \
				  FROM bancostransacc bt \
				  INNER JOIN bancosxpag bxp ON bt.transacc=bxp.transacc \
				  WHERE bt.idmovbanco=@idmovbancoanterior";
			instrucciones[num_instrucciones++]=instruccion;

			instruccion="create temporary table bancostransacctmp ( \
				idtemp int(11)  AUTO_INCREMENT, \
				transacc int(11), \
				PRIMARY KEY (idtemp) ) Engine = InnoDB";
			instrucciones[num_instrucciones++]=instruccion;

			instruccion="insert into bancostransacctmp (idtemp, transacc) \
				SELECT NULL as idtemp, bt.transacc \
				  FROM bancostransacc bt \
				  WHERE bt.idmovbanco=@idmovbanconuevo";
			instrucciones[num_instrucciones++]=instruccion;

			instruccion="INSERT INTO bancosxpag \
					(transacc, tracredito) \
				SELECT b.transacc, t.tracredito \
					FROM transxpagtmp t \
					inner join bancostransacctmp b on b.idtemp=t.idtemp";
			instrucciones[num_instrucciones++]=instruccion;
			// ------ FIN Sección que inserta el movimiento bancario contrario al del pago original.
			*/

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
//ID_MODIF_FECHA_PAGO_PROV
void ServidorCompras::ModificaFechaPagoProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
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
			as error from pagosprov p INNER JOIN estadosistemaemp AS e \
			ON e.estado = 'FUCIERRE' AND e.sucursal = '%s' \
			where p.pago='%s' ",FormServidor->ObtieneClaveSucursal(), clave);
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 1, error);

		// Verifica que la fecha nueva sea posterior a la fecha de cierre.
		instruccion.sprintf("select @error:=if('%s'<=cast(e.valor as datetime), 1, 0) as error from estadosistemaemp e where e.estado='FUCIERRE' AND e.sucursal = '%s' ", mFg.DateToMySqlDate(StrToDate(fecha_nueva)), FormServidor->ObtieneClaveSucursal());
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 2, error);

		//Verifica que la fecha nueva no sea anterior a alguna fecha de las compras que contiene el pago
		instruccion.sprintf("SELECT \
		@ERROR:= IF(SUM( (MONTH('%s') < MONTH(comp.fechacom) AND YEAR('%s') = YEAR(comp.fechacom)) OR \
		(YEAR('%s') < YEAR(comp.fechacom)) ) > 0, 1, 0) as error\
		FROM pagosprov pgp\
		INNER JOIN transxpag txp ON txp.pago = pgp.pago\
		INNER JOIN compras comp ON txp.referencia = comp.referencia\
		WHERE pgp.pago='%s'",
		mFg.DateToMySqlDate(StrToDate(fecha_nueva)),
		mFg.DateToMySqlDate(StrToDate(fecha_nueva)),
		mFg.DateToMySqlDate(StrToDate(fecha_nueva)), clave);

        mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 3, error);

		if (error==0) {
			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

			instruccion.sprintf("update pagosprov set fecha='%s', fechamodi='%s', horamodi='%s', usumodi='%s' where pago='%s' and cancelado=0", mFg.StrToMySqlDate(fecha_nueva), mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario, clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Marca las transacciones como aplicadas y cambia las fechas.
			instruccion.sprintf("update transxpag set aplicada=1, fechaapl='%s', fechaalta='%s', fechamodi='%s', horamodi='%s', usumodi='%s' where pago='%s' and cancelada=0", mFg.StrToMySqlDate(fecha_nueva) , mFg.StrToMySqlDate(fecha_nueva) ,mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario, clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Si existe cheque lo marca consolidado y modifica sus fechas.
			instruccion.sprintf("update chequesproveedores, cheqxpag \
				set chequesproveedores.estado='C', chequesproveedores.fechacob='%s', chequesproveedores.fechaalta='%s' \
				where cheqxpag.pago='%s' and chequesproveedores.chequeprov=cheqxpag.chequeprov and chequesproveedores.estado<>'X'",  mFg.StrToMySqlDate(fecha_nueva),  mFg.StrToMySqlDate(fecha_nueva), clave);
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
//ID_CON_PAGOS_PROV_DIA
void ServidorCompras::ConsultaPagosProvDelDia(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
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

    instruccion.sprintf("select p.ident as identpago, t.tracredito as foliotran, c.folioprov as compfolprov, \
    if(pr.tipoempre=0, replegal, razonsocial) as nombreprov, \
    (t.valor*-1) as valor, (case when p.formapag='E' then 'EFECTIVO' when p.formapag='C' then 'CHEQUE' \
    when p.formapag='D' then 'DEPOSITO' when p.formapag='T' then 'TRANSFERENCIA BANCARIA' end) as formapag, \
    chp.folio as numcheque, b.nombre as nombanco, \
    chclasif.descripcion as tipocheque, \
    chp.fechacob, chp.estado as status, p.muuid as uuid  \
	from compras c  inner join  transxpag t  inner join  pagosprov p  \
	  inner join  proveedores pr on  pr.proveedor=c.proveedor \
      left join cheqxpag chxp on chxp.pago=p.pago \
      left join chequesproveedores chp on chp.chequeprov=chxp.chequeprov \
      left join chequesclasif chclasif on chclasif.clasif=chp.clasif \
	  left join bancos b on b.banco=chp.banco      \
	  INNER JOIN terminales term ON term.terminal = p.terminal \
	  INNER JOIN secciones sec ON sec.seccion = term.seccion \
	  INNER JOIN sucursales suc ON suc.sucursal = sec.sucursal \
	  where p.fecha='%s' and p.pago=t.pago and c.cancelado=0 and  \
      c.referencia=t.referencia and t.cancelada=0 and p.cancelado=0 \
	  %s %s %s AND suc.idempresa = %s ",
	  mFg.StrToMySqlDate(fecha), mostrar_todas_las_formas,
	  revision_tipo_cheque,condicion_proveedor, FormServidor->ObtieneClaveEmpresa());
    mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

    //Se suman los ajustes
    if (formapag=="C") {
        instruccion.sprintf("select sum(p.ajuste) as ajuste \
            from pagosprov p, cheqxpag chxp, chequesproveedores chp where \
              p.fecha='%s' and chxp.pago=p.pago and chp.chequeprov=chxp.chequeprov  \
              and p.cancelado=0 \
              %s %s \
              group by p.fecha",
			  mFg.StrToMySqlDate(fecha), mostrar_todas_las_formas,
              revision_tipo_cheque);
    } else {
        instruccion.sprintf("select sum(p.ajuste) as ajuste \
            from pagosprov p where \
              p.fecha='%s' \
              and p.cancelado=0 \
              %s \
              group by p.fecha",
			  mFg.StrToMySqlDate(fecha), mostrar_todas_las_formas);
    }
    mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}
//------------------------------------------------------------------------------
//ID_CON_AUX_PROV
void ServidorCompras::ConsultaAuxiliarProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    // CONSULTA AUXILIAR DE PROVEEDORES
    AnsiString instruccion, instruccion_revisar_saldo, instruccion_transacciones,instruccion_numReferencia;
	AnsiString proveedor, fecha_inicial_compra, fecha_final_compra,sucursal,condicion_sucursal=" ";
    AnsiString mostrar_todas_transacciones;
	AnsiString mostrar_todo;
	AnsiString num_Referencia;
	AnsiString mEstadoActivaOCancelada;
	AnsiString empresa;
	AnsiString condicion_empresa=" ";

	proveedor=mFg.ExtraeStringDeBuffer(&parametros);
	fecha_inicial_compra=mFg.ExtraeStringDeBuffer(&parametros);
	fecha_final_compra=mFg.ExtraeStringDeBuffer(&parametros);
	mostrar_todas_transacciones=mFg.ExtraeStringDeBuffer(&parametros);
	mostrar_todo=mFg.ExtraeStringDeBuffer(&parametros);
	sucursal=mFg.ExtraeStringDeBuffer(&parametros);
	num_Referencia=mFg.ExtraeStringDeBuffer(&parametros);


	/*
		parámetro empleado para poder consultar las transacciones de algún cliente,
		consultando el estado final de 'Cancelado'
		Cancelado = 1
		Sin cancelar = 0     /// Original
	*/
	mEstadoActivaOCancelada=mFg.ExtraeStringDeBuffer(&parametros);
	empresa=mFg.ExtraeStringDeBuffer(&parametros);


	if(sucursal != " ")
	   condicion_sucursal.sprintf(" sec.sucursal='%s' AND ", sucursal);
	else
	   condicion_sucursal=" ";


	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);


	if (mostrar_todas_transacciones=="0")
		instruccion_transacciones.sprintf(" t.fechaalta>='%s' AND t.fechaalta<='%s' AND ",mFg.StrToMySqlDate(fecha_inicial_compra), mFg.StrToMySqlDate(fecha_final_compra));
	else
		instruccion_transacciones=" ";


	if(num_Referencia!=" ")
	   instruccion_numReferencia.sprintf(" c.referencia = '%s' AND  ",num_Referencia);
	else
	   instruccion_numReferencia=" ";


	if (mostrar_todo=="0")
		instruccion_revisar_saldo=" cs.saldor>0 AND ";
	else
		instruccion_revisar_saldo=" ";

    if(empresa != " "){
		condicion_empresa.sprintf(" suc.idempresa = %s AND ", empresa);
	}

	// Crea una tabla donde se van a poner los saldos de las compras
	// afectadas por la cancelación
	instruccion="create temporary table auxcomprassaldos (compra char(11), saldor decimal(16,2), chcnc decimal(16,2), PRIMARY KEY (compra)) Engine = InnoDB";
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query EjecutaSelectSqlNulo"));

	// Calcula los saldos de las compras del proveedor
	instruccion.sprintf("insert into auxcomprassaldos (compra, saldor, chcnc) \
		select c.referencia as compra, \
		sum(if(t.aplicada=1, t.valor, 0)) as saldor, \
		sum(if(t.aplicada=0, t.valor, 0)) as chcnc \
		from compras c, transxpag t \
		where c.proveedor='%s' and \
		t.referencia=c.referencia and t.cancelada=0 and c.cancelado=%s and c.acredito=1 \
		group by c.referencia",
		proveedor,mEstadoActivaOCancelada);
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query EjecutaSelectSqlNulo"));

	instruccion.sprintf("select t.fechaapl as fechatran, t.tipo, t.tracredito, \
	c.referencia, if(t.tipo='COMP',1,2) as escompra, n.folioprov as notfolioprov, ifnull(nc.folioprov,nc.referencia) as notcfolioprov, \
	p.ident as identpago, c.folioprov as compfolioprov, (case when p.formapag='E' then 'EFECTIVO' when p.formapag='C' then 'CHEQUE' \
	when p.formapag='D' then 'DEPOSITO' when p.formapag='T' then 'TRANSFERENCIA BANCARIA' end) as formapag, \
	chp.fechacob as fechacheque,  chp.estado as statuscheque, chp.clasif as clasifcheque, \
	t.valor, c.fechavenc, t.horaalta as horatran, cs.saldor, (cs.chcnc*-1) as chcnc, c.comprador, \
	n.tipo as tiponotacred, \
	c.acredito as terminos, sec.sucursal, n.referencia AS notacredref, n.tipo AS tiponotacred, p.pago AS refpago \
	from compras c  inner join  auxcomprassaldos cs  inner join  transxpag t \
	  inner join terminales ter on ter.terminal=c.terminal \
	  inner join secciones sec on ter.seccion=sec.seccion \
	  INNER JOIN sucursales suc ON suc.sucursal = sec.sucursal \
	  left join pagosprov p on p.pago=t.pago \
	  left join cheqxpag chxp on chxp.pago=p.pago \
	  left join chequesproveedores chp on chp.chequeprov=chxp.chequeprov \
	  left join notascredprov n on n.referencia=t.notaprov \
	  left join notascarprov nc on nc.referencia=t.notacar \
	where %s %s %s %s %s \
	c.cancelado=%s and c.fechacom>='%s' and c.fechacom<='%s' and \
	c.proveedor='%s' and c.referencia=cs.compra and \
	c.referencia=t.referencia and t.cancelada=0  \
	order by t.referencia, escompra, t.fechaalta,t.tracredito",
	instruccion_revisar_saldo, instruccion_transacciones, instruccion_numReferencia, condicion_sucursal, condicion_empresa,
	mEstadoActivaOCancelada,mFg.StrToMySqlDate(fecha_inicial_compra), mFg.StrToMySqlDate(fecha_final_compra),
	proveedor);
    mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}
//------------------------------------------------------------------------------
//ID_CONSOLIDA_CHEQ_PROV
void ServidorCompras::ConsolidaChequesProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    //  CONSOLIDA UNA SERIE DE CHEQUES DE PROVEEDOR
    char *buffer_sql=new char[1024*100];
    char *aux_buffer_sql=buffer_sql;
    AnsiString instrucciones[1000];
    AnsiString instruccion;
    int num_instrucciones=0;
    AnsiString cheque;
    int num_cheques;
    int i;
    AnsiString FechaSigAplicacion;
    AnsiString StatusCheque;
	AnsiString Usuario;

	try {
		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		num_cheques=StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
		FechaSigAplicacion=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		Usuario=mFg.ExtraeStringDeBuffer(&parametros);

		for (i=0; i<num_cheques; i++) {
			cheque=mFg.ExtraeStringDeBuffer(&parametros); // Clave del cheque
			instruccion.sprintf("update chequesproveedores chp set chp.estado='C' \
				where chp.chequeprov='%s' and chp.estado='P'", cheque);
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("update chequesproveedores chp, pagosprov p, cheqxpag chxp, transxpag t \
				set t.aplicada=1, t.fechaapl='%s', t.horamodi='%s', t.fechamodi='%s', t.usumodi='%s' \
				where chp.chequeprov=chxp.chequeprov and chxp.pago=p.pago and \
				p.pago=t.pago and chp.chequeprov='%s' and t.cancelada=0",
				mFg.DateToMySqlDate(mFg.MySqlDateToTDate(FechaSigAplicacion.c_str())-1)
				,mFg.TimeToMySqlTime(Time()),mFg.DateToMySqlDate(Date()),Usuario,cheque);
			instrucciones[num_instrucciones++]=instruccion;
		}

		instruccion.sprintf("update estadosistemaemp ssis set ssis.valor='%s' \
			where ssis.estado='FACHEQPRO' AND ssis.sucursal = '%s' ", FechaSigAplicacion, FormServidor->ObtieneClaveSucursal());
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
//------------------------------------------------------------------------------
//ID_CON_CHEQXFECH_PROV
void ServidorCompras::ConsultaChequesxfechaProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    // CONSULTA CHEQUES DEL PROVEEDOR DE UN DIA X
    AnsiString instruccion;
    AnsiString fecha_inicio, fecha_fin, tipocheque, revision_tipo_cheque, banco_cheque, revision_banco_cheque;

    fecha_inicio=mFg.ExtraeStringDeBuffer(&parametros);
    fecha_fin=mFg.ExtraeStringDeBuffer(&parametros);
    banco_cheque=mFg.ExtraeStringDeBuffer(&parametros);
    tipocheque=mFg.ExtraeStringDeBuffer(&parametros);

    revision_tipo_cheque=" ";
	revision_banco_cheque=" ";
    if (tipocheque!="")
        revision_tipo_cheque.sprintf(" and chp.clasif='%s'",tipocheque);
    if (banco_cheque!="")
        revision_banco_cheque.sprintf(" and chp.banco='%s'",banco_cheque);

    mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

    instruccion.sprintf("select chp.chequeprov, c.folioprov as compfolprov, \
    if(pr.tipoempre=0, replegal, razonsocial) as nombreprov, \
    (t.valor*-1) as valor, chp.folio as numcheque, b.banco as cvebanco, b.nombre as nombanco, \
    chclasif.clasif as clasif, chclasif.descripcion as tipocheque, chp.fechacob, chp.estado as status \
    from compras c, transxpag t, pagosprov p, proveedores pr, \
        cheqxpag chxp, chequesproveedores chp, \
        chequesclasif chclasif, bancos b \
      where \
      pr.proveedor=c.proveedor and \
      chp.fechacob>='%s' and chp.fechacob<='%s' \
	  and chxp.pago=p.pago and chp.chequeprov=chxp.chequeprov and \
      p.pago=t.pago and c.cancelado=0 and  \
      chclasif.clasif=chp.clasif and b.banco=chp.banco and \
      c.referencia=t.referencia and t.cancelada=0 and p.cancelado=0 \
      %s %s",
	  mFg.StrToMySqlDate(fecha_inicio),mFg.StrToMySqlDate(fecha_fin),
      revision_tipo_cheque, revision_banco_cheque);
    mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

    //Se suman los ajustes
    instruccion.sprintf("select sum(p.ajuste) as ajuste \
        from pagosprov p, cheqxpag chxp, chequesproveedores chp \
          where \
          chp.fechacob>='%s' and chp.fechacob<='%s' and chxp.pago=p.pago and chp.chequeprov=chxp.chequeprov  \
          and p.cancelado=0 \
          %s \
          group by p.cancelado",
		  mFg.StrToMySqlDate(fecha_inicio),mFg.StrToMySqlDate(fecha_fin),
          revision_tipo_cheque);
    mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}
//------------------------------------------------------------------------------
//ID_MOD_CHEQ_PROV
void ServidorCompras::ModificaChequeProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  MODIFICA UN CHEQUE DE PROVEEDOR
    char *buffer_sql=new char[1024*10];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instrucciones[100];
	AnsiString instruccion;
	int num_instrucciones=0;
	AnsiString cheque,folio, banco, fechacob, estado, clasif, usuario, aplicada;
	int num_cheques;
	int i;
	TDate fecha=Today();
	TTime hora=Time();

	try {
		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		cheque=mFg.ExtraeStringDeBuffer(&parametros);
		folio=mFg.ExtraeStringDeBuffer(&parametros);
		banco=mFg.ExtraeStringDeBuffer(&parametros);
		fechacob=mFg.ExtraeStringDeBuffer(&parametros);
		estado=mFg.ExtraeStringDeBuffer(&parametros);
		clasif=mFg.ExtraeStringDeBuffer(&parametros);
		usuario=mFg.ExtraeStringDeBuffer(&parametros);


		instruccion.sprintf("update chequesproveedores chp set chp.estado=if(chp.estado='C','C','%s'), \
			chp.folio='%s', chp.banco='%s', chp.fechacob='%s', \
			chp.clasif='%s' \
			where chp.chequeprov='%s'",
			estado, folio, banco, mFg.StrToMySqlDate(fechacob), clasif, cheque);
		instrucciones[num_instrucciones++]=instruccion;

		if (estado=="C")
			aplicada="1";
		else
			aplicada="0";

		instruccion.sprintf("update transxpag t, pagosprov p, cheqxpag chxp, \
			chequesproveedores chp \
			set t.aplicada=%s, t.fechamodi='%s', t.horamodi='%s', t.usumodi='%s' \
			where t.pago=p.pago and t.cancelada=0 and p.pago=chxp.pago and chxp.chequeprov=chp.chequeprov",
			aplicada, mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario);

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
//ID_QRY_COMPRAS_PROV
void ServidorCompras::ConsultaComprasProveedor(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    // CONSULTA LAS COMPRAS DE UN PROVEEDOR
    AnsiString instruccion;
	AnsiString proveedor, referencia;
	AnsiString condicion_corte=" ";
	AnsiString condicion_referencia=" ";
	bool considerar_corte;

	proveedor=mFg.ExtraeStringDeBuffer(&parametros);
	considerar_corte=StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
	referencia=mFg.ExtraeStringDeBuffer(&parametros);

    if(considerar_corte){
		condicion_corte=" and c.fechacom > cast(e.valor as date) ";
	}

	if(referencia!=" "){
		condicion_referencia.sprintf(" AND c.referencia = '%s' ", referencia);
	}

    mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

    // Carga todos los datos del proveedor.
    instruccion.sprintf("select * from proveedores where proveedor='%s'", proveedor);
    mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Crea una tabla donde se van a poner los saldos de las compras del proveedor
	instruccion="create temporary table auxcomprassaldos (compra char(11), saldo decimal(16,2), PRIMARY KEY (compra)) Engine = InnoDB";
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query EjecutaSelectSqlNulo"));

    // Calcula los saldos de las compras del proveedor (SALDOB (saldo virtual))
	instruccion.sprintf("insert into auxcomprassaldos (compra, saldo) \
		select c.referencia as compra, sum(t.valor) as saldo \
        from compras c, transxpag t \
        where c.proveedor='%s' and \
        t.referencia=c.referencia and t.cancelada=0 and c.cancelado=0 and c.acredito=1 \
        group by c.referencia",
        proveedor);
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query EjecutaSelectSqlNulo"));

	instruccion.sprintf("select c.referencia as Ref, c.folioprov as Factura, cs.saldo as Saldo,c.muuid as uuid, c.fechaalta, c.fechacom, datediff(curdate(),c.fechavenc) as diasvenc \
		from compras c, estadosistemaemp e, auxcomprassaldos cs, terminales ter, secciones sec, sucursales suc \
		where c.referencia=cs.compra and e.estado='FUCIERRE' and e.sucursal = '%s' and \
		cs.saldo > 0 %s %s \
		and cancelado=0 and proveedor='%s' AND ter.terminal = c.terminal \
		AND sec.seccion = ter.seccion AND suc.sucursal = sec.sucursal AND suc.idempresa = %s",FormServidor->ObtieneClaveSucursal(),
        condicion_corte, condicion_referencia, proveedor, FormServidor->ObtieneClaveEmpresa());
    mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}

//------------------------------------------------------------------------------
//ID_CON_CHEQXFECH_PROV_NC
void ServidorCompras::ConsultaChequesxfechaProvNC(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    // CONSULTA CHEQUES DEL PROVEEDOR DE UN DIA X
    AnsiString instruccion;
    AnsiString fecha_inicio, fecha_fin;
    AnsiString proveedor;

    fecha_inicio=mFg.ExtraeStringDeBuffer(&parametros);
    fecha_fin=mFg.ExtraeStringDeBuffer(&parametros);
    proveedor=mFg.ExtraeStringDeBuffer(&parametros);

    mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

    instruccion.sprintf("select chp.chequeprov, c.folioprov as compfolprov, \
    (t.valor*-1) as valor, chp.folio as numcheque, b.banco as cvebanco, b.nombre as nombanco, \
    chp.fechacob, chp.estado as status \
    from compras c, transxpag t, pagosprov p, \
		cheqxpag chxp, chequesproveedores chp, \
        bancos b \
      where \
      chp.clasif='NOR' \
      and c.proveedor='%s' and \
      chp.fechacob>='%s' and chp.fechacob<='%s' \
	  and chxp.pago=p.pago and chp.chequeprov=chxp.chequeprov and \
      p.pago=t.pago and c.cancelado=0 and  \
      b.banco=chp.banco and \
      c.referencia=t.referencia and t.cancelada=0 and p.cancelado=0 and t.aplicada=1",
      proveedor,mFg.StrToMySqlDate(fecha_inicio),mFg.StrToMySqlDate(fecha_fin));
    mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

}

//------------------------------------------------------------------------------
//ID_GRA_CARGO_PROV
void ServidorCompras::GrabaCargoProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    //  GRABA UN CARGO A PROVEEDOR
    //
	char *buffer_sql=new char[1024*64*10];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);

	int num_partidas, i;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[1000];

	int error=0;
	TDate fecha=Today();
	TTime hora=Time();
	char *resultado;
	AnsiString tarea, clave, usuario, terminal;
	AnsiString compra, tipo, folioprov;
	TDateTime fecha_not;
	double valor_cargo;

	try {
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave del cargo.
		tarea=mFg.ExtraeStringDeBuffer(&parametros); // Indicador si se va a agregar o modificar.
		usuario=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que esta grabando el cargo.
		terminal=mFg.ExtraeStringDeBuffer(&parametros); // Terminal de donde se manda grabar.
		compra=mFg.ExtraeStringDeBuffer(&parametros); // Clave de la compra que se va a afectar
		tipo=mFg.ExtraeStringDeBuffer(&parametros); // Tipo del cargo.
		folioprov=mFg.ExtraeStringDeBuffer(&parametros); // Folio físico del cargo.
		fecha_not=StrToDate(mFg.ExtraeStringDeBuffer(&parametros)); // Fecha de la nota.
		valor_cargo=StrToFloat(mFg.ExtraeStringDeBuffer(&parametros)); // Valor del cargo que cobra el banco.

		// Verificaciones para no permitir modificaciones cuando no sea permitido.
		if (tarea=="M") {
			// Verifica que la fecha del cargo (la grabada) sea posterior a la fecha de cierre.
			instruccion.sprintf("select @error:=if(n.fechanot<=cast(e.valor as datetime), 1, 0) as error from notascarprov n left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' where n.referencia='%s'",FormServidor->ObtieneClaveSucursal(), clave);
			mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 3, error);

            // Verifica que tiene un UUID asignado.
			instruccion.sprintf("SELECT @error:=IF(muuid<>'',1,0) AS error FROM notascarprov WHERE referencia='%s' AND cancelado=0 GROUP BY referencia ", clave);
			mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 7, error);
		}
		// Verifica que la fecha del cargo (la que se le va a grabar) sea posterior a la fecha de cierre.
		instruccion.sprintf("select @error:=if('%s'<=cast(e.valor as datetime), 1, 0) as error from estadosistemaemp e where e.estado='FUCIERRE' AND e.sucursal = '%s' ", mFg.DateToMySqlDate(fecha_not), FormServidor->ObtieneClaveSucursal());
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 4, error);


		if (error==0) {
			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

			// Obtiene el folio para el cargo
			if (tarea=="A") {
				instruccion.sprintf("select @folioaux:=valor from foliosemp where folio='NOTASCAPRO' AND sucursal = '%s' %s ",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliosig=@folioaux+1");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @folioaux=cast(@folioaux as char)");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @folio=concat('%s', lpad(@folioaux,9,'0'))", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("update foliosemp set valor=@foliosig where folio='NOTASCAPRO' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
			} else {
				instruccion.sprintf("set @folio='%s'", clave);
				instrucciones[num_instrucciones++]=instruccion;
			}

			// Graba la cabecera en la tabla "notascarprov"
			datos.AsignaTabla("notascarprov");
			datos.InsCampo("fechamodi", mFg.DateToAnsiString(fecha));
			datos.InsCampo("horamodi", mFg.TimeToAnsiString(hora));
			datos.InsCampo("usumodi", usuario);
			datos.InsCampo("fechanot", mFg.DateToAnsiString(fecha_not));
			datos.InsCampo("folioprov", folioprov);
			datos.InsCampo("valor", mFg.CadenaAFlotante(valor_cargo));
			if (tarea=="A") {
				instruccion.sprintf("select @seccion:=seccion, @depart:=depart from terminales where terminal='%s'", terminal);
				instrucciones[num_instrucciones++]=instruccion;

				instruccion.sprintf("select @proveedor:=proveedor from compras where referencia='%s'", compra);
				instrucciones[num_instrucciones++]=instruccion;

				instruccion.sprintf("select @impuesto:=valor from parametrosemp where parametro='IMPCARGOS' AND sucursal = '%s'", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;

				datos.InsCampo("cveimp", "@impuesto", 1);
				datos.InsCampo("referencia", "@folio", 1);
				datos.InsCampo("usualta", usuario);
				datos.InsCampo("fechaalta", mFg.DateToAnsiString(fecha));
				datos.InsCampo("horaalta", mFg.TimeToAnsiString(hora));
				datos.InsCampo("cancelado", "0");
				datos.InsCampo("tipo", "O");
				datos.InsCampo("terminal", terminal);
				datos.InsCampo("docseccion", "@seccion",1);
				datos.InsCampo("docdepart", "@depart",1);
				datos.InsCampo("proveedor", "@proveedor", 1);
				instrucciones[num_instrucciones++]=datos.GenerarSqlInsert();
			} else {
				instrucciones[num_instrucciones++]=datos.GenerarSqlUpdate("referencia=@folio");
			}


			// Hace la transacción del CARGO
			if (tarea=="A") {
				// Obtiene el folio para la NUEVA transaccion
				instruccion.sprintf("select @foliotranaux:=valor from foliosemp where folio='TRANPROV' AND sucursal = '%s' %s ",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliosigtran=@foliotranaux+1");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliotranaux=cast(@foliotranaux as char)");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliotran=concat('%s', lpad(@foliotranaux,9,'0'))", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("update foliosemp set valor=@foliosigtran where folio='TRANPROV' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("insert into transxpag \
					(tracredito, referencia, notacar, concepto, destino, tipo, cancelada, aplicada, valor, fechaalta, horaalta, fechamodi, horamodi, fechaapl, usualta, usumodi) \
					values (@foliotran, '%s', @folio, 'C', 'C', '%s', 0,1, %12.2f, '%s', '%s', '%s', '%s', '%s', '%s', '%s')",
					 compra, tipo, valor_cargo, mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), mFg.DateToMySqlDate(fecha_not), usuario, usuario);
				instrucciones[num_instrucciones++]=instruccion;
			} else {
				// Actualiza solo el valor, la fecha de la nota, la fecha y hora de modificación y el usuario que modificó.
				instruccion.sprintf("update transxpag set valor=%12.2f, fechamodi='%s', horamodi='%s', fechaapl='%s', usumodi='%s' where notacar='%s' and cancelada=0", valor_cargo, mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), mFg.DateToMySqlDate(fecha_not), usuario, clave);
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
	}
}

//------------------------------------------------------------------------------
//ID_CANC_CARGO_PROV
void ServidorCompras::CancelaCargoProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  CANCELA CARGO DE PROVEEDOR POR REBOTE
	//
	char *buffer_sql=new char[1024*64];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instrucciones[50], instruccion;
	int num_instrucciones=0;
	AnsiString clave, usuario;
	int i, error=0;
	TDate fecha=Today();
	TTime hora=Time();
	BufferRespuestas *resp_verificacion=NULL;

	try {
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave de la devolución
		usuario=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que esta cancelando la devolución.

		// Verifica que la fecha de la nota sea posterior a la fecha de cierre.
		instruccion.sprintf("select @error:=if(n.fechanot<=cast(e.valor as datetime), 1, 0) as error from notascarprov n left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' where n.referencia='%s'",FormServidor->ObtieneClaveSucursal(), clave);
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 3, error);

		if (error==0) {
			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

			// Obtiene el folio de la compra a la que se aplicó el cargo
			instruccion.sprintf("select @compra:=t.referencia from transxpag t where t.notacar='%s' and t.cancelada=0", clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Cancela el registro en notascarprov
			instruccion.sprintf("update notascarprov set cancelado=1, usumodi='%s', fechamodi='%s', horamodi='%s' where referencia='%s' and cancelado=0", usuario, mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Cancela el cargo
			instruccion.sprintf("update transxpag set cancelada=1, fechamodi='%s', horamodi='%s', usumodi='%s' where notacar='%s' and cancelada=0",  mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario, clave);
			instrucciones[num_instrucciones++]=instruccion;

			////////////////// INICIO CALCULO DE SALDOS DE COMPRAS

			// Crea una tabla para almacenar los folios de las compras afectadas por el pago
			// para posteriormente recalcular saldos de estas compras.
			instruccion="create temporary table comprasaux (compra char(11), PRIMARY KEY (compra)) Engine = InnoDB";
			instrucciones[num_instrucciones++]=instruccion;

			instruccion="insert into comprasaux (compra) values (@compra)";
			instrucciones[num_instrucciones++]=instruccion;

			// Crea una tabla donde se van a poner los saldos de las compras
			// afectadas por la cancelación
			instruccion="create temporary table auxcomprassaldos (compra char(11), saldo decimal(16,2), PRIMARY KEY (compra)) Engine = InnoDB";
			instrucciones[num_instrucciones++]=instruccion;

			// Calcula los saldos de las compras relacionadas con el pago (SALDO B)
			instruccion="insert into auxcomprassaldos (compra, saldo) ";
			instruccion+="select c.referencia as compra, sum(t.valor) as saldo ";
			instruccion+="from compras c, comprasaux caux, transxpag t ";
			instruccion+="where c.referencia=caux.compra and ";
			instruccion+="t.referencia=c.referencia and t.cancelada=0 and c.cancelado=0 ";
			instruccion+="group by c.referencia";
			instrucciones[num_instrucciones++]=instruccion;

			////////////////// FIN CALCULO DE SALDOS DE COMPRAS

		}

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			// VERIFICA SI QUEDO ALGUN SALDO NEGATIVO DE LAS FACTURAS EN CUESTION
			//
			instruccion="select * from auxcomprassaldos where saldo<0";
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
				instruccion.sprintf("select %d as error", error);
				mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
				if (error==1) {
					instruccion="select * from auxcomprassaldos where saldo<0";
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
//ID_CON_CARGO_PROV
void ServidorCompras::ConsultaCargoProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    // CONSULTA CARGO DE PROVEEDOR
    AnsiString instruccion;
    AnsiString clave;

    clave=mFg.ExtraeStringDeBuffer(&parametros);
    mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

    // Obtiene todos los generales (de cabecera) de la nota de cargo
    instruccion.sprintf("select n.*, i.porcentaje as porcimpu  from notascarprov n, impuestos i where referencia='%s' and n.cveimp=i.impuesto", clave);
    mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

    // Obtiene lo datos de cargo.
    instruccion.sprintf("select @compra:=t.referencia, t.* from transxpag t where t.notacar='%s' and \
        t.concepto='C' and t.destino='C' \
        group by t.referencia", clave);
    mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

    // NULO:  Verifica que la fecha de la nota de cargo sea posterior a la fecha de cierre.
	instruccion.sprintf("select if(n.fechanot<=cast(e.valor as datetime), 1, 0) as \
	modificar from notascarprov n left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' where n.referencia='%s'",FormServidor->ObtieneClaveSucursal(), clave);
    mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

    // Obtiene lo datos de la compra.
    instruccion.sprintf("select * from compras where referencia=@compra");
    mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene todos los datos del proveedor de la nota de cargo.
    instruccion.sprintf("select p.* from proveedores p, notascarprov nc where nc.referencia='%s' and nc.proveedor=p.proveedor", clave);
    mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}


//------------------------------------------------------------------------------
//ID_GRA_CARGO_REBOTE_PROV
void ServidorCompras::GrabaCargoReboteProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    //  GRABA UN CARGO A PROVEEDOR POR UN REBOTE DE CHEQUE
    //
    // Cuando se hace un cargo por rebote, se hace un cargo que anule cada una
    // de las transacciones que pertencen al pago, pero no se cancela ni el pago
    // ni el cheque, solamente al cheque se le marca con estatus (R).
	char *buffer_sql=new char[1024*64*10];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);

    int num_partidas, i;
    int num_instrucciones=0;
    AnsiString instruccion, instrucciones[1000];

    int error=0;
    TDate fecha=Today();
    TTime hora=Time();
    char *resultado;
    BufferRespuestas *resp_lista=NULL;
    AnsiString tarea, clave, usuario, terminal;
    TDateTime fecha_not;
    AnsiString cheque, folioprov, transac_detalle, compra_detalle;
    AnsiString compra_cargo_banco, transac_cargo_banco;
    double valor_cargo_banco, valor_detalle;

	try {
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave del cargo
		tarea=mFg.ExtraeStringDeBuffer(&parametros); // Indicador si se va a agregar o modificar.
		usuario=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que esta grabando el cargo.
		terminal=mFg.ExtraeStringDeBuffer(&parametros); // Terminal de donde se manda grabar.
		cheque=mFg.ExtraeStringDeBuffer(&parametros); // Clave del cheque que se va a rebotar.
		folioprov=mFg.ExtraeStringDeBuffer(&parametros); // Folio físico del cargo.
		fecha_not=StrToDate(mFg.ExtraeStringDeBuffer(&parametros)); // Fecha de la nota.
		valor_cargo_banco=StrToFloat(mFg.ExtraeStringDeBuffer(&parametros)); // Valor del cargo que cobra el banco.

		// Verificaciones para no permitir modificaciones cuando no sea permitido.
		if (tarea=="M") {
			// Verifica que la fecha del cargo (la grabada) sea posterior a la fecha de cierre.
			instruccion.sprintf("select @error:=if(n.fechanot<=cast(e.valor as datetime), 1, 0) as error from notascarprov n left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' where n.referencia='%s'",FormServidor->ObtieneClaveSucursal(), clave);
			mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 3, error);

			// Verifica el cargo tenga un  UUID asignado
			instruccion.sprintf("SELECT @error:=IF(muuid<>'',1,0) AS error FROM notascarprov WHERE referencia='%s' AND cancelado=0 GROUP BY referencia", clave);
			mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 6, error);

		} else {
			// Verifica que el cheque está vigente (no tenga los siguientes estatus "N", "X", "R")
			instruccion.sprintf("select @error:=if(ch.estado='X' or ch.estado='X' or ch.estado='X', \
					1, 0) as error from chequesproveedores ch where ch.chequeprov='%s'", cheque);
			mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 5, error);
		}
		// Verifica que la fecha del cargo (la que se le va a grabar) sea posterior a la fecha de cierre.
		instruccion.sprintf("select @error:=if('%s'<=cast(e.valor as datetime), 1, 0) as error from estadosistemaemp e where e.estado='FUCIERRE' AND e.sucursal = '%s' ", mFg.DateToMySqlDate(fecha_not), FormServidor->ObtieneClaveSucursal());
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 4, error);


		if (error==0) {
			// Asigna a variales de MySql el folio del pago al que corresponde
			// el cheque, el proveedor y el valor total del cargo.
			instruccion.sprintf("select @pago:=chxp.pago, @proveedor:=p.proveedor, @valorcargo:=(ch.valor+%12.2f) \
				from cheqxpag chxp, chequesproveedores ch, pagosprov p \
				where chxp.chequeprov='%s' and chxp.chequeprov=ch.chequeprov \
				and chxp.pago=p.pago and p.cancelado=0",
				valor_cargo_banco, cheque);
			mServidorVioleta->EjecutaSelectSqlNulo(Respuesta,  MySQL, instruccion.c_str());
			if (tarea=="A") {
				instruccion.sprintf("select t.tracredito, t.referencia as compra, t.valor as valor \
					from pagosprov p, transxpag t \
					where p.pago=@pago and t.pago=p.pago \
					and p.cancelado=0 and t.cancelada=0 \
					order by t.tracredito");
				mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_lista);
			}

			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

			// Obtiene el folio para el cargo
			if (tarea=="A") {
				instruccion.sprintf("select @folioaux:=valor from foliosemp where folio='NOTASCAPRO' AND sucursal = '%s' %s ",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliosig=@folioaux+1");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @folioaux=cast(@folioaux as char)");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @folio=concat('%s', lpad(@folioaux,9,'0'))", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("update foliosemp set valor=@foliosig where folio='NOTASCAPRO' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
			} else {
				instruccion.sprintf("set @folio='%s'", clave);
				instrucciones[num_instrucciones++]=instruccion;
			}

			// Graba la cabecera en la tabla "notascarprov"
			datos.AsignaTabla("notascarprov");
			datos.InsCampo("fechamodi", mFg.DateToAnsiString(fecha));
			datos.InsCampo("horamodi", mFg.TimeToAnsiString(hora));
			datos.InsCampo("usumodi", usuario);
			datos.InsCampo("fechanot", mFg.DateToAnsiString(fecha_not));
			datos.InsCampo("folioprov", folioprov);
			datos.InsCampo("valor", "@valorcargo", 1);
			if (tarea=="A") {
				instruccion.sprintf("select @seccion:=seccion, @depart:=depart from terminales where terminal='%s'", terminal);
				instrucciones[num_instrucciones++]=instruccion;

				instruccion.sprintf("select @impuesto:=valor from parametrosemp where parametro='IMPCARGOS' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;

				datos.InsCampo("cveimp", "@impuesto", 1);
				datos.InsCampo("referencia", "@folio", 1);
				datos.InsCampo("usualta", usuario);
				datos.InsCampo("fechaalta", mFg.DateToAnsiString(fecha));
				datos.InsCampo("horaalta", mFg.TimeToAnsiString(hora));
				datos.InsCampo("cancelado", "0");
				datos.InsCampo("tipo", "R");
				datos.InsCampo("terminal", terminal);
				datos.InsCampo("docseccion", "@seccion",1);
				datos.InsCampo("docdepart", "@depart",1);
				datos.InsCampo("pago", "@pago", 1);
				datos.InsCampo("proveedor", "@proveedor", 1);
				instrucciones[num_instrucciones++]=datos.GenerarSqlInsert();
			} else {
				instrucciones[num_instrucciones++]=datos.GenerarSqlUpdate("referencia=@folio");
			}


			// Obtiene en un buffer todos los datos necesarias para hacer un cargo
			// por cada uno de los abonos que se hicieron en el pago, para que se
			// dicho pago se anule.
			if (tarea=="A") {
				if (resp_lista!=NULL) {
					resp_lista->IrAlPrimerDato();
					for (i=0; i<resp_lista->ObtieneNumRegistros(); i++) {
						transac_detalle=resp_lista->ObtieneDato("tracredito");
						compra_detalle=resp_lista->ObtieneDato("compra");
						valor_detalle=StrToFloat(resp_lista->ObtieneDato("valor"))*-1;
						resp_lista->IrAlSiguienteRegistro();

						// Obtenemos la primer compra que es a la que se le
						// va a hacer el cargo cobrado por el banco por el rebote.
						if (i==0) {
							compra_cargo_banco=compra_detalle;
							transac_cargo_banco=transac_detalle;
						}

						// Obtiene el folio para la NUEVA transaccion
						instruccion.sprintf("select @foliotranaux:=valor from foliosemp where folio='TRANPROV' AND sucursal = '%s' %s ",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
						instrucciones[num_instrucciones++]=instruccion;
						instruccion.sprintf("set @foliosigtran=@foliotranaux+1");
						instrucciones[num_instrucciones++]=instruccion;
						instruccion.sprintf("set @foliotranaux=cast(@foliotranaux as char)");
						instrucciones[num_instrucciones++]=instruccion;
						instruccion.sprintf("set @foliotran=concat('%s', lpad(@foliotranaux,9,'0'))", FormServidor->ObtieneClaveSucursal());
						instrucciones[num_instrucciones++]=instruccion;
						instruccion.sprintf("update foliosemp set valor=@foliosigtran where folio='TRANPROV' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
						instrucciones[num_instrucciones++]=instruccion;
						instruccion.sprintf("insert into transxpag \
							(tracredito, referencia, notacar, concepto, destino, tipo, cancelada, aplicada, valor, fechaalta, horaalta, fechamodi, horamodi, fechaapl, usualta, usumodi) \
							values (@foliotran, '%s', @folio, 'C', 'C', 'CHDE', 0,1, %12.2f, '%s', '%s', '%s', '%s', '%s', '%s', '%s')",
							 compra_detalle, valor_detalle, mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), mFg.DateToMySqlDate(fecha_not), usuario, usuario);
						instrucciones[num_instrucciones++]=instruccion;
					}
				}
			}

			// Hace un abono que refleje en proveedores la nota de cargo del banco
			if (tarea=="A") {
				// Obtiene el folio para la NUEVA transaccion
				instruccion.sprintf("select @foliotranaux:=valor from foliosemp where folio='TRANPROV' AND sucursal = '%s' %s ",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliosigtran=@foliotranaux+1");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliotranaux=cast(@foliotranaux as char)");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliotran=concat('%s', lpad(@foliotranaux,9,'0'))", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("update foliosemp set valor=@foliosigtran where folio='TRANPROV' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("insert into transxpag \
					(tracredito, referencia, notacar, concepto, destino, tipo, cancelada, aplicada, valor, fechaalta, horaalta, fechamodi, horamodi, fechaapl, usualta, usumodi) \
					values (@foliotran, '%s', @folio, 'C', 'C', 'NCAR', 0,1, %12.2f, '%s', '%s', '%s', '%s', '%s', '%s', '%s')",
					 compra_cargo_banco, valor_cargo_banco, mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), mFg.DateToMySqlDate(fecha_not), usuario, usuario);
				instrucciones[num_instrucciones++]=instruccion;
			} else {
				// Actualiza solo el valor, la fecha de la nota, la fecha y hora de modificación y el usuario que modificó.
				instruccion.sprintf("update transxpag set valor=%12.2f, fechamodi='%s', horamodi='%s', fechaapl='%s', usumodi='%s' where tracredito='%s'", valor_cargo_banco, mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), mFg.DateToMySqlDate(fecha_not), usuario, transac_cargo_banco);
				instrucciones[num_instrucciones++]=instruccion;
			}

			// Marca el pago como cancelado
			instruccion.sprintf("update pagosprov set cancelado=1, fechamodi='%s', horamodi='%s', usumodi='%s' where pago=@pago and cancelado=0", mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario);
			instrucciones[num_instrucciones++]=instruccion;

			// Marca el cheque como rebotado
			instruccion.sprintf("update chequesproveedores, cheqxpag \
				set chequesproveedores.estado='R' \
				where cheqxpag.pago=@pago and chequesproveedores.chequeprov=cheqxpag.chequeprov");
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
		if (resp_lista!=NULL) delete resp_lista;
		delete buffer_sql;
	}
}

//------------------------------------------------------------------------------
//ID_CANC_CARGO_REBOTE_PROV
void ServidorCompras::CancelaCargoReboteProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  CANCELA CARGO POR REBOTE
	//
	char *buffer_sql=new char[1024*64];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instrucciones[50], instruccion;
	int num_instrucciones=0;
	AnsiString clave, usuario;
	int i, error=0;
	TDate fecha=Today();
    TTime hora=Time();
	BufferRespuestas *resp_verificacion=NULL;

	try {
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave de la devolución
		usuario=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que esta cancelando la devolución.

		// Verifica que la fecha de la nota sea posterior a la fecha de cierre.
		instruccion.sprintf("select @error:=if(n.fechanot<=cast(e.valor as datetime), 1, 0) as error from notascarprov n left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' where n.referencia='%s'",FormServidor->ObtieneClaveSucursal(), clave);
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 3, error);

		if (error==0) {
			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

			// Obtiene el folio del pago al que se aplicó el cargo por rebote.
			instruccion.sprintf("select @foliopago:=pago from notascarprov where referencia='%s' and cancelado=0", clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Cancela el registro en notascarprov
			instruccion.sprintf("update notascarprov set cancelado=1, usumodi='%s', fechamodi='%s', horamodi='%s' where referencia='%s' and cancelado=0", usuario, mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Cancela el cargo hecho por el banco
			instruccion.sprintf("update transxpag set cancelada=1, fechamodi='%s', horamodi='%s', usumodi='%s' where notacar='%s' and concepto='C' and destino='C' and tipo='NCAR' and cancelada=0",  mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario, clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Cancela todos los cargos hechos
			instruccion.sprintf("update transxpag set cancelada=1, fechamodi='%s', horamodi='%s', usumodi='%s' where notacar='%s' and concepto='C' and destino='C' and tipo='CHDE' and cancelada=0",  mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario, clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Marca el pago como NO cancelado
			instruccion.sprintf("update pagosprov set cancelado=0, fechamodi='%s', horamodi='%s', usumodi='%s' where pago=@foliopago and cancelado=1", mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario);
			instrucciones[num_instrucciones++]=instruccion;

			// Marca el cheque como COBRADO
			instruccion.sprintf("update chequesproveedores, cheqxpag \
				set chequesproveedores.estado='C' \
				where cheqxpag.pago=@foliopago and chequesproveedores.chequeprov=cheqxpag.chequeprov");
			instrucciones[num_instrucciones++]=instruccion;

			////////////////// INICIO CALCULO DE SALDOS DE COMPRAS

			// Crea una tabla para almacenar los folios de las compras afectadas por el pago
			// para posteriormente recalcular saldos de estas compras.
			instruccion="create temporary table comprasaux (compra char(11), PRIMARY KEY (compra)) Engine = InnoDB";
			instrucciones[num_instrucciones++]=instruccion;

			instruccion="insert into comprasaux (compra) ";
			instruccion+="select t.referencia as compra from transxpag t where t.pago=@foliopago and t.cancelada=0 ";
			instrucciones[num_instrucciones++]=instruccion;

			// Crea una tabla donde se van a poner los saldos de las compras
			// afectadas por la cancelación
			instruccion="create temporary table auxcomprassaldos (compra char(11), saldo decimal(16,2), PRIMARY KEY (compra)) Engine = InnoDB";
			instrucciones[num_instrucciones++]=instruccion;

			// Calcula los saldos de las compras relacionadas con el pago (SALDO B)
			instruccion="insert into auxcomprassaldos (compra, saldo) ";
			instruccion+="select c.referencia as compra, sum(t.valor) as saldo ";
			instruccion+="from compras c, comprasaux caux, transxpag t ";
			instruccion+="where c.referencia=caux.compra and ";
			instruccion+="t.referencia=c.referencia and t.cancelada=0 and c.cancelado=0 ";
			instruccion+="group by c.referencia";
			instrucciones[num_instrucciones++]=instruccion;

			////////////////// FIN CALCULO DE SALDOS DE COMPRAS

		}

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			// VERIFICA SI QUEDO ALGUN SALDO NEGATIVO DE LAS FACTURAS EN CUESTION
			//
			instruccion="select * from auxcomprassaldos where saldo<0";
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
				instruccion.sprintf("select %d as error", error);
				mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
				if (error==1) {
					instruccion="select * from auxcomprassaldos where saldo<0";
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
//ID_CON_CARGO_REBOTE_PROV
void ServidorCompras::ConsultaCargoReboteProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    // CONSULTA CARGO POR REBOTE DEL PROVEEDOR
    AnsiString instruccion;
    AnsiString clave;

    clave=mFg.ExtraeStringDeBuffer(&parametros);
    mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

    // Obtiene todos los generales (de cabecera) de la nota de cargo
    instruccion.sprintf("select n.*, i.porcentaje as porcimpu  from notascarprov n, impuestos i where referencia='%s' and n.cveimp=i.impuesto", clave);
    mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

    // Obtiene lo datos de cargo cobrado por el banco.
    instruccion.sprintf("select * from transxpag where notacar='%s' and \
        concepto='C' and destino='C' and tipo='NCAR' \
        group by referencia", clave);
    mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

    // NULO:  Verifica que la fecha de la nota de cargo sea posterior a la fecha de cierre.
    instruccion.sprintf("select if(n.fechanot<=cast(e.valor as datetime), 1, 0) as \
	modificar from notascarprov n left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' where n.referencia='%s'",FormServidor->ObtieneClaveSucursal(), clave);
    mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

    // Obtiene las transacciones que eran afectadas por el cheque devuelto
    instruccion.sprintf("select chp.folio, c.folioprov, (t.valor*-1) as valor \
		from transxpag t, cheqxpag chxp, pagosprov p, chequesproveedores chp, notascarprov n, compras c \
        where n.referencia='%s' and \
        n.pago=p.pago and \
		p.pago=t.pago and \
        t.pago=chxp.pago and \
        t.referencia=c.referencia and \
        chxp.chequeprov=chp.chequeprov and \
        t.cancelada=0 order by t.tracredito", clave);
    mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

    // Obtiene los datos del cheque devuelto
    instruccion.sprintf("select chp.chequeprov, chp.folio, b.nombre, chp.fechacob \
        from transxpag t, cheqxpag chxp, pagosprov p, chequesproveedores chp, notascarprov n, compras c, bancos b \
        where n.referencia='%s' and \
        n.pago=p.pago and \
        p.pago=t.pago and \
        t.pago=chxp.pago and \
        t.referencia=c.referencia and \
        chxp.chequeprov=chp.chequeprov and \
        chp.banco=b.banco and \
        t.cancelada=0 order by t.tracredito", clave);
    mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);


    // Obtiene todos los datos del proveedor de la nota de cargo.
    instruccion.sprintf("select p.* from proveedores p, notascarprov nc where nc.referencia='%s' and nc.proveedor=p.proveedor", clave);
    mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}
//------------------------------------------------------------------------------
//ID_CON_CHEQUE_PROV_IMPRESION
void ServidorCompras::ConsultaPagoProvImprimirCheque(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros){

    char *buffer_sql=new char[1024*64*10];
    AnsiString tarea, clave, ident_cola, terminal;

    AnsiString instruccion, instrucciones[1000];
    char *resultado;
	BufferRespuestas* resp_facturas=NULL;
    BufferRespuestas* resp_vcheque=NULL;
    AnsiString facturas, factura, forma;

    double valor_cheque;
    AnsiString ValorCheque=" ", ValorChequeProtegido=" ", ValorChequeLetra=" ";

	try {
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave del pago
		ident_cola=mFg.ExtraeStringDeBuffer(&parametros); // Clave identificadora en la cola de impresión (si es vacia entonces es que es de impresión directa)
		terminal=mFg.ExtraeStringDeBuffer(&parametros); // Terminal desde la que se imprime.
		forma=mFg.ExtraeStringDeBuffer(&parametros); // Tipo de forma a usar

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {

			// Obtiene todas las facturas correspondientes al pago.
			instruccion.sprintf("select c.folioprov from transxpag t, compras c \
				where t.referencia=c.referencia and t.pago='%s' and t.cancelada=0", clave);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_facturas);

			facturas="";
			for(int i=0; i<resp_facturas->ObtieneNumRegistros(); i++){
					resp_facturas->IrAlRegistroNumero(i);
					if(i>0){
						facturas+=", ";
					}
					factura=resp_facturas->ObtieneDato("folioprov");
					facturas+=factura;
			}
			facturas+=" ";

			// Obtiene todos los generales del pago
			instruccion.sprintf("select prov.razonsocial as rsocialprov, chpr.fechacob as fechacheque, \
				chpr.folio as numcheque, '%s' as facturasafec \
				from pagosprov pp, proveedores prov, chequesproveedores chpr, cheqxpag chp \
				where pp.pago='%s' and pp.proveedor=prov.proveedor and chp.pago=pp.pago and \
				chp.chequeprov=chpr.chequeprov", facturas, clave);
			resultado=Respuesta->ObtieneDirLibreBuffer();
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
			BufferRespuestas resp_venta(resultado);

			// Obtiene todo el detalle en este caso por tratarse de un cheque no hay detalle
			// pero es obligado seleccionar algo para que el procedimiento fucnione
			instruccion="select p.pago ";
			instruccion+="from pagosprov p ";
			instruccion+="where p.pago='";
			instruccion+=clave;
			instruccion+="'";

			resultado=Respuesta->ObtieneDirLibreBuffer();
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
			BufferRespuestas resp_detalle(resultado);


			// Obtiene el total del cheque y crea el enunciado de cantidad
			// Obtiene todos los generales del pago
			instruccion.sprintf("select chpr.valor as valor \
				from pagosprov pp, chequesproveedores chpr, cheqxpag chp \
				where pp.pago='%s' and chp.pago=pp.pago and \
				chp.chequeprov=chpr.chequeprov", clave);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_vcheque);

			valor_cheque=StrToFloat(resp_vcheque->ObtieneDato("valor"));

			instruccion="select '";
			instruccion+=mFg.FormateaCantidad(valor_cheque,2,true);
			instruccion+="' as vcheque, '";
			instruccion+=mFg.ProtegeCadenaAsteriscos(mFg.GeneraEnunciadoDeCantidad(valor_cheque),130,0);
			instruccion+="' as vchequeletra, '";
			instruccion+=mFg.ProtegeCadenaAsteriscos(mFg.FormateaCantidad(valor_cheque,2,true),25,1);
			instruccion+="' as vchequepro ";
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Obtiene todos los datos de la forma
			instruccion.sprintf("select * from formas where forma='%s'", forma);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Obtiene todo el detalle de la forma
			instruccion.sprintf("select * from dformas where forma='%s'", forma);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	} __finally {
		if (resp_facturas!=NULL) delete resp_facturas;
		if (resp_vcheque!=NULL) delete resp_vcheque;
		delete buffer_sql;
	}
}
//------------------------------------------------------------------------------
//ID_CON_BLOQUEADOS_COMPRA
void ServidorCompras::ConsultaBloqueadosCompra(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros){

	char *buffer_sql=new char[1024*64*20];
	char *aux_buffer_sql=buffer_sql;
	int i, num_articulos;
	double cantidad;
	AnsiString instruccion="";
	AnsiString articulo,articulos[1000];
	AnsiString producto,present,proveedor;
	AnsiString productos="";

	AnsiString archivo_temp1, archivo_temp2, archivo_temp3, archivo_temp4, archivo_temp5;

	try{
		// Recibe los articulos
		num_articulos=StrToInt(mFg.ExtraeStringDeBuffer(&parametros)); // Obtiene el número de articulos
		proveedor=mFg.ExtraeStringDeBuffer(&parametros); // Obtiene proveedor bloqueado


		instruccion.sprintf("SELECT * FROM prodbloqueadocompra WHERE (proveedor='%s' OR proveedor='TODOS') AND (producto,present) IN ( ('','') ",proveedor);

		for (i=0; i<num_articulos; i++) {
			producto=mFg.ExtraeStringDeBuffer(&parametros);
			present=mFg.ExtraeStringDeBuffer(&parametros);
			productos.sprintf(",('%s','%s')", producto,present);
			instruccion+=productos;
		}
		instruccion+=");";

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}__finally{
		delete buffer_sql;
	}

}
//------------------------------------------------------------------------------
//ID_CON_COM_IMPRESION
void ServidorCompras::ConsultaComprasParaImprimir(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{

	char *buffer_sql=new char[1024*64*10];
	char *aux_buffer_sql=buffer_sql;
	AnsiString clave, terminal;
	int i;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[1000];
	char *resultado;
	double totalpeso, totalvol, totalcajasbultos, peso, volumen, cajasbultos;
	double totalcantidad, cantidad;
	AnsiString orden;

	try{
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave del pedido
		terminal=mFg.ExtraeStringDeBuffer(&parametros); // Terminal desde la que se imprime.
		orden=mFg.ExtraeStringDeBuffer(&parametros); //tipo de orden, default|por multiplo agarra el numero de la terminal

		// Obtiene el precio público
		instruccion.sprintf("select 0 AS error");
		instrucciones[num_instrucciones++]=instruccion;

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {

			// Obtiene todos los generales (de cabecera) de la compra
			instruccion.sprintf("SELECT suc.nombre, c.*, cm.mensaje, CONCAT(emp.nombre, ' ', emp.appat, ' ', emp.apmat) AS nomcomprador \
			FROM compras c \
			INNER JOIN terminales ter ON c.terminal=ter.terminal \
			INNER JOIN secciones sec ON sec.seccion=ter.seccion  \
			INNER JOIN sucursales suc ON suc.sucursal=sec.sucursal \
			INNER JOIN empleados emp ON emp.empleado=c.comprador \
			left join comprasmensajes cm on cm.referencia=c.referencia \
			where c.referencia='%s'", clave);

			resultado=Respuesta->ObtieneDirLibreBuffer();
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
			BufferRespuestas resp_venta(resultado);

			// Obtiene todos los datos del proveedor de la compra
			instruccion.sprintf("select p.* from proveedores p, compras c where c.referencia='%s' and c.proveedor=p.proveedor", clave);

			resultado=Respuesta->ObtieneDirLibreBuffer();
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
			BufferRespuestas resp_proveedor(resultado);

			// Obtiene los datos de la compra completa (sin importar las devoluciones)
			instruccion="select d.referencia, d.articulo, d.cantidad, d.costo, d.costoimp, (d.costo*d.cantidad) as costoTot, ";
			instruccion+="a.present, p.producto, p.nombre, a.multiplo,";
			instruccion+="concat(left(a.multiplo,3),'-',a.factor) as multfactor, ";
			instruccion+="a.factor, a.volumen, a.peso, ";
			instruccion+="(a.factor / pmm.cajalogisticafactor) AS cajasbultos, ";
			instruccion+="d.iepscuota ";
			if(orden=="1")
				instruccion+=",IF(a.factor=1,1,2) ORDEN,p.clasif1  ";
			instruccion+="from dcompras d  inner join  articulos a  inner join  productos p ";
			instruccion+="LEFT JOIN presentacionesminmax pmm ON a.producto=pmm.producto AND a.present=pmm.present ";
			instruccion+="where d.referencia='";
			instruccion+=clave;
			instruccion+="' and d.articulo=a.articulo and a.producto=p.producto ";
			instruccion+=" group by p.nombre, a.present, a.multiplo ";
			if(orden=="1")
				instruccion+="ORDER BY ORDEN DESC,IF(ORDEN='1',p.clasif1,left(multiplo,3)),p.nombre, a.present, a.multiplo ";
			else
				instruccion+=" order by p.nombre, a.present, a.multiplo ";

			resultado=Respuesta->ObtieneDirLibreBuffer();
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
			BufferRespuestas resp_detalle(resultado);

			// *********************************************************
			// Calcula total de articulos, total de peso y total de volumen.
			totalcantidad=0;
			totalpeso=0;
			totalvol=0;
			totalcajasbultos=0;
			for(i=0; i<resp_detalle.ObtieneNumRegistros(); i++){
					resp_detalle.IrAlRegistroNumero(i);
					cantidad=mFg.CadenaAFlotante(resp_detalle.ObtieneDato("cantidad"));
					peso=StrToFloat(resp_detalle.ObtieneDato("peso"));
					volumen=StrToFloat(resp_detalle.ObtieneDato("volumen"));
					cajasbultos=StrToFloat(resp_detalle.ObtieneDato("cajasbultos"));

					totalcantidad+=cantidad;
					totalpeso+=cantidad*peso;
					totalvol+=cantidad*volumen;
					totalcajasbultos+= cantidad*cajasbultos;
			}
			// Totales de la venta
			instruccion="select ";
			instruccion+=mFg.FormateaCantidad(totalcantidad,3,false);
			instruccion+=" as comtotalcant, ";
			instruccion+=mFg.FormateaCantidad(totalpeso,2,false);
			instruccion+=" as comtotalpeso, ";
			instruccion+=mFg.FormateaCantidad(totalvol,2,false);
			instruccion+=" as comtotalvol, ";
			instruccion+=mFg.FormateaCantidad(totalcajasbultos,2,false);
			instruccion+=" as comtotalcajas ";
			// Ejecuta la instrucción
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

		}
	} __finally {
		delete buffer_sql;
	}
}

//------------------------------------------------------------------------------
//ID_CON_COMPED_IMPRESION
void ServidorCompras::ConsultaPedidosParaImprimir(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{

	char *buffer_sql=new char[1024*64*10];
	char *aux_buffer_sql=buffer_sql;
	AnsiString clave;
	int i;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[1000];
	char *resultado;
	double totalpeso, totalvol, totalcajasbultos, peso, volumen, cajasbultos;
	double totalcantidad, cantidad;
	AnsiString orden;

    AnsiString inner_claveprov = " ", campo_claveprov = " ";
	BufferRespuestas* resp_pedidoauto = NULL;

	try{
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave del pedido
		orden=mFg.ExtraeStringDeBuffer(&parametros); //tipo de orden, default|por multiplo agarra el numero de la terminal

		// Obtiene el precio público
		instruccion.sprintf("select 0 AS error");
		instrucciones[num_instrucciones++]=instruccion;

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			// Obtiene todos los generales (de cabecera) del pedido
			instruccion.sprintf("select p.*, pcm.mensaje, suc.nombre, CONCAT(emp.nombre, ' ', emp.appat, ' ', emp.apmat) AS nomcomprador \
			from pedidos p \
			INNER JOIN almacenes alm ON alm.almacen = p.almacen \
			INNER JOIN secciones s ON s.seccion = alm.seccion \
			INNER JOIN sucursales suc ON suc.sucursal = s.sucursal \
			INNER JOIN empleados emp ON emp.empleado=p.comprador \
			left join pedidoscomprasmensajes pcm on pcm.referencia=p.referencia where p.referencia='%s'", clave);

			resultado=Respuesta->ObtieneDirLibreBuffer();
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
			BufferRespuestas resp_venta(resultado);

			// Obtiene todos los datos del proveedor del pedido
			instruccion.sprintf("select pro.* from proveedores pro, pedidos ped where ped.referencia='%s' and ped.proveedor=pro.proveedor", clave);

			resultado=Respuesta->ObtieneDirLibreBuffer();
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
			BufferRespuestas resp_proveedor(resultado);

            //Para obtener clave prod dependiendo de si es para pedidos o pedidos automaticos
			try{
				instruccion.sprintf("SELECT 1 FROM pedidos ped \
					INNER JOIN pedidos_automaticos_pedidos pap ON ped.referencia = pap.pedido_id WHERE ped.referencia = '%s' ", clave);
				mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), resp_pedidoauto);

				if(resp_pedidoauto->ObtieneNumRegistros() > 0){
					campo_claveprov = " clvprod.claveproductoproveedor ";
					inner_claveprov = " LEFT JOIN claveproducto_proveedor clvprod ON a.producto = clvprod.producto AND a.present = clvprod.present AND ped.proveedor = clvprod.proveedor ";
				} else {
					campo_claveprov = " artp.claveproductoproveedor ";
					inner_claveprov = " LEFT JOIN articulosped artp ON artp.producto = a.producto AND artp.present = a.present AND artp.sucursal = sec.sucursal AND artp.proveedor = ped.proveedor  ";
				}
			}__finally{
				if (resp_pedidoauto != NULL){
					delete resp_pedidoauto;
				}
			}

			// Obtiene todo el detalle del pedido con algunos datos extras que necesita el cliente
			instruccion="select d.referencia, d.articulo, d.cantidad, d.costoimp AS costo, (d.costoimp*d.cantidad) as costoTot, ";
			instruccion+="a.present, p.producto, p.nombre, a.multiplo, a.factor, ";
			instruccion+="concat(left(a.multiplo,3),'-',a.factor) as multfactor, ";
			instruccion+="d.iepscuota, ";
			instruccion+="a.factor, a.volumen, a.peso, ";
			instruccion+="(a.factor / pmm.cajalogisticafactor) AS cajasbultos, ";
			instruccion+=campo_claveprov;
			instruccion+="from dpedidos d  inner join  articulos a  inner join  productos p ";
			instruccion+="LEFT JOIN pedidos as ped on ped.referencia=d.referencia ";
			instruccion+="LEFT JOIN presentacionesminmax pmm ON a.producto=pmm.producto AND a.present=pmm.present ";
			instruccion+="LEFT JOIN almacenes alma ON alma.almacen = ped.almacen ";
			instruccion+="LEFT JOIN secciones sec ON sec.seccion = alma.seccion ";
			instruccion+=inner_claveprov;
			instruccion+="where d.referencia='";
			instruccion+=clave;
			instruccion+="' and d.articulo=a.articulo and a.producto=p.producto ";
			instruccion+=" group by p.nombre, a.present, a.multiplo ";
			instruccion+=" order by p.nombre, a.present, a.multiplo ";

			resultado=Respuesta->ObtieneDirLibreBuffer();
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
			BufferRespuestas resp_detalle(resultado);

			// *********************************************************
			// Calcula total de articulos, total de peso y total de volumen.
			totalcantidad=0;
			totalpeso=0;
			totalvol=0;
			totalcajasbultos=0;
			for(i=0; i<resp_detalle.ObtieneNumRegistros(); i++){
					resp_detalle.IrAlRegistroNumero(i);
					cantidad=mFg.CadenaAFlotante(resp_detalle.ObtieneDato("cantidad"));
					peso=StrToFloat(resp_detalle.ObtieneDato("peso"));
					volumen=StrToFloat(resp_detalle.ObtieneDato("volumen"));
					cajasbultos=StrToFloat(resp_detalle.ObtieneDato("cajasbultos"));

					totalcantidad+=cantidad;
					totalpeso+=cantidad*peso;
					totalvol+=cantidad*volumen;
					totalcajasbultos+= cantidad*cajasbultos;
			}
			// Totales de la venta
			instruccion="select ";
			instruccion+=mFg.FormateaCantidad(totalcantidad,3,false);
			instruccion+=" as comtotalcant, ";
			instruccion+=mFg.FormateaCantidad(totalpeso,2,false);
			instruccion+=" as comtotalpeso, ";
			instruccion+=mFg.FormateaCantidad(totalvol,2,false);
			instruccion+=" as comtotalvol, ";
			instruccion+=mFg.FormateaCantidad(totalcajasbultos,2,false);
			instruccion+=" as comtotalcajas ";
			// Ejecuta la instrucción
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

		}
	} __finally {
		delete buffer_sql;
	}
	}

//------------------------------------------------------------------------------
//ID_EDITA_ALMACEN_ENTRADA_COMPRAS
	void ServidorCompras::ModificaAlmacenEntradaCompras(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
	{     //ModificaFechaPagoProv
			char *buffer_sql=new char[1024*32];
			char *aux_buffer_sql=buffer_sql;
			DatosTabla datos(mServidorVioleta->Tablas);
			AnsiString instrucciones[50], instruccion;
			int num_instrucciones=0;
			AnsiString Referencia, almacen, usuario;
			int i, error=0;
			// Fecha y hora actual
			TDate fecha=Today();
			TTime hora=Time();
			try{
				Referencia=mFg.ExtraeStringDeBuffer(&parametros); // Referendia de la venta
				almacen=mFg.ExtraeStringDeBuffer(&parametros); // Nueva fecha a asignar.
				usuario=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que está modificando la fecha.
				// Verifica que la fecha del pago sea posterior a la fecha de cierre.
				instruccion.sprintf("SELECT  @error:=if(CAST(CONCAT(c.fechamodi,' ',c.horamodi)AS DATETIME)>= CAST(e.valor AS DATETIME), 0, 1) AS error \
					from compras c INNER JOIN estadosistemaemp AS e                                          \
					ON e.estado = 'FUCIERRE' AND e.sucursal = '%s'                                                                      \
					where c.referencia='%s' ",FormServidor->ObtieneClaveSucursal(), Referencia);
				mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 1, error);

				if (error==0) {
					instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
					instrucciones[num_instrucciones++]="START TRANSACTION";
					//Resta las existencias del alamacen actual
					instruccion.sprintf("UPDATE compras c \
						INNER JOIN dcompras d ON c.referencia = d.referencia \
						inner join articulos a on a.articulo = d.articulo \
						INNER JOIN existenciasactuales ea ON a.producto = ea.producto AND a.present = ea.present AND ea.almacen = c.almacen \
						SET ea.cantidad = (ea.cantidad - (d.cantidad * a.factor)), ea.compras = (ea.compras - (d.cantidad * a.factor)) \
						WHERE c.referencia = '%s' ", Referencia);
					instrucciones[num_instrucciones++]=instruccion;

					// Realiza el cambio del almacen al almacen a la compra
					instruccion.sprintf("update compras set almacen='%s',usumodi='%s', fechamodi = '%s',\
						horamodi = '%s'  where referencia='%s'", almacen,usuario,mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora),Referencia);
					instrucciones[num_instrucciones++]=instruccion;

					//Suma las existencias para el nuevo almacen
					instruccion.sprintf("UPDATE compras c \
						INNER JOIN dcompras d ON c.referencia = d.referencia \
						inner join articulos a on a.articulo = d.articulo \
						INNER JOIN existenciasactuales ea ON a.producto = ea.producto AND a.present = ea.present AND ea.almacen=c.almacen \
						SET ea.cantidad = (ea.cantidad + (d.cantidad * a.factor)), ea.compras = (ea.compras + (d.cantidad * a.factor)) \
						WHERE c.referencia = '%s'", Referencia);
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
//ID_CON_RECEPCION
void ServidorCompras::EjecutaRepRecepcion(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{

    char *buffer_sql = new char[1024 * 64];
	char *aux_buffer_sql = buffer_sql;
	int num_instrucciones = 0;
	AnsiString instruccion,instrucciones[100];
	AnsiString clave;

    clave=mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
	try{

		instruccion.sprintf("create temporary table if NOT EXISTS aux (articulo VARCHAR(11),\
			costo DECIMAL(16,6))");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("INSERT INTO aux SELECT dc.articulo, dc.costo\
			FROM dcompras dc INNER JOIN articulos art ON art.articulo=dc.articulo\
			GROUP BY dc.articulo;");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("SELECT\
			r.recepcion,\
			pro.nombre,\
			art.present,\
			art.multiplo,\
			dr.cantidad,\
			dc.costo,\
			c.referencia\
			FROM drecepciones dr\
			INNER JOIN recepciones r ON dr.recepcion = r.recepcion\
			INNER JOIN articulos art ON dr.articulo = art.articulo\
			INNER JOIN productos pro ON pro.producto = art.producto\
			INNER JOIN pedrecepcion ped ON ped.recepcion = r.recepcion\
			LEFT JOIN comprecepcion cr ON cr.recepcion = r.recepcion\
			LEFT JOIN compras c ON c.referencia = cr.compra\
			LEFT JOIN dcompras dc ON dc.referencia=c.referencia\
			WHERE r.recepcion = '%s'",clave);
		instrucciones[num_instrucciones++]=instruccion;

		aux_buffer_sql = mFg.AgregaStringABuffer
				(mFg.IntToAnsiString(num_instrucciones),aux_buffer_sql);

		for(int i=0;i<num_instrucciones;i++){
			aux_buffer_sql = mFg.AgregaStringABuffer(instrucciones[i],
								aux_buffer_sql);
		}

		if(mServidorVioleta->EjecutaBufferAccionesSql
			(Respuesta,MySQL,buffer_sql)){
			mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,
				instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	} __finally{
        delete buffer_sql;
    }
}
//------------------------------------------------------------------------------
//ID_REC_PEDIDO_PROV
void ServidorCompras::ConsultaRecepcionProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql = new char[1024 * 64];
	char *aux_buffer_sql = buffer_sql;
	int num_instrucciones = 0;
	AnsiString instruccion, consulta, instrucciones[100];
	AnsiString claveRecepcion;
	AnsiString esFraccionado;
	AnsiString sucursal;
	AnsiString condicion_fraccionado=" ";
	AnsiString archivo1, archivo2, archivo3, archivo4, archivo5, archivo6;
	AnsiString archivo7, archivo8, archivo9, archivo10, archivo11, archivo12;
	bool esParteRel = false;
	BufferRespuestas* resp_proveedor = NULL;

	claveRecepcion = mFg.ExtraeStringDeBuffer(&parametros);
	esFraccionado  = mFg.ExtraeStringDeBuffer(&parametros);

	sucursal = FormServidor->ObtieneClaveSucursal();

	try{
		instruccion.sprintf("SELECT \
			prov.esparterelac \
		FROM recepciones r \
		INNER JOIN proveedores prov ON prov.proveedor = r.proveedor \
		WHERE r.recepcion IN (%s) ", claveRecepcion);
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), resp_proveedor);
		if(resp_proveedor->ObtieneNumRegistros() > 0) {
			resp_proveedor->IrAlRegistroNumero(0);
			if(StrToInt(resp_proveedor->ObtieneDato("esparterelac")) == 1){
				esParteRel = true;
			}
		}

	}__finally{
		if (resp_proveedor != NULL){
			delete resp_proveedor;
		}
	}

	try {

		instruccion.sprintf("CREATE OR REPLACE TEMPORARY TABLE auxrecepcionbase ( \
			recepcion VARCHAR(11) NOT NULL, \
			articulo varchar(9) NOT NULL DEFAULT '0', \
			producto VARCHAR (8) NULL DEFAULT NULL, \
			present VARCHAR (255) NULL DEFAULT NULL, \
			multiplo VARCHAR (10) NULL DEFAULT NULL, \
			factor DECIMAL(10, 3) NULL DEFAULT 0, \
			cantidad DECIMAL(12, 3) DEFAULT NULL, \
			INDEX recepcion (recepcion), \
			INDEX articulo (articulo), \
			INDEX producto (producto), \
			INDEX present (present), \
			INDEX multiplo (multiplo) \
		) ENGINE=InnoDB DEFAULT CHARSET=LATIN1");
		instrucciones[num_instrucciones++] = instruccion;

		archivo1 = mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);

		instruccion.sprintf("CREATE OR REPLACE TEMPORARY TABLE auxtotxproducto ( \
			articulo varchar(9) NOT NULL DEFAULT '0', \
			producto VARCHAR (8) NULL DEFAULT NULL, \
			present VARCHAR (255) NULL DEFAULT NULL, \
			multiplo VARCHAR (10) NULL DEFAULT NULL, \
			cantidadmin DECIMAL(12, 3) DEFAULT NULL, \
			INDEX articulo (articulo), \
			INDEX producto (producto), \
			INDEX present (present), \
			INDEX multiplo (multiplo) \
		) ENGINE=InnoDB DEFAULT CHARSET=LATIN1");
		instrucciones[num_instrucciones++] = instruccion;

		archivo2 = mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);

		instruccion.sprintf("CREATE OR REPLACE TEMPORARY TABLE auxmultxproducto ( \
			articulo varchar(9) NOT NULL DEFAULT '0', \
			producto VARCHAR (8) NULL DEFAULT NULL, \
			present VARCHAR (255) NULL DEFAULT NULL, \
			multiplo VARCHAR (10) NULL DEFAULT NULL, \
			factor DECIMAL(10, 3) NULL DEFAULT 0, \
			INDEX articulo (articulo), \
			INDEX producto (producto), \
			INDEX present (present), \
			INDEX multiplo (multiplo) \
		) ENGINE=InnoDB DEFAULT CHARSET=LATIN1");
		instrucciones[num_instrucciones++] = instruccion;

		archivo3 = mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);

		instruccion.sprintf("CREATE OR REPLACE TEMPORARY TABLE auxconversionproducto ( \
			articulo varchar(9) NOT NULL DEFAULT '0', \
			producto VARCHAR (8) NULL DEFAULT NULL, \
			present VARCHAR (255) NULL DEFAULT NULL, \
			multiplo VARCHAR (10) NULL DEFAULT NULL, \
			factor DECIMAL(10, 3) NULL DEFAULT 0, \
			cantmult DECIMAL(12, 3) DEFAULT NULL, \
			cantresta DECIMAL(12, 3) DEFAULT NULL, \
			INDEX articulo (articulo), \
			INDEX producto (producto), \
			INDEX present (present), \
			INDEX multiplo (multiplo) \
		) ENGINE=InnoDB DEFAULT CHARSET=LATIN1");
		instrucciones[num_instrucciones++] = instruccion;

		archivo4 = mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);

		instruccion.sprintf("CREATE OR REPLACE TEMPORARY TABLE auxremanente  ( \
			producto VARCHAR (8) NULL DEFAULT NULL, \
			present VARCHAR (255) NULL DEFAULT NULL, \
			remanente DECIMAL(12,3) NULL DEFAULT NULL, \
			INDEX producto (producto), \
			INDEX present (present) \
		) ENGINE=InnoDB DEFAULT CHARSET=LATIN1");
		instrucciones[num_instrucciones++] = instruccion;

		archivo5 = mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);

		instruccion.sprintf("CREATE OR REPLACE TEMPORARY TABLE auxconversionfinal  ( \
			articulo varchar(9) NOT NULL DEFAULT '0', \
			producto VARCHAR (8) NULL DEFAULT NULL, \
			present VARCHAR (255) NULL DEFAULT NULL, \
			multiplo VARCHAR (10) NULL DEFAULT NULL, \
			factor DECIMAL(10, 3) NULL DEFAULT 0, \
			cantmult DECIMAL(12, 3) DEFAULT NULL, \
			cantresta DECIMAL(12, 3) DEFAULT NULL, \
			INDEX articulo (articulo), \
			INDEX producto (producto), \
			INDEX present (present), \
			INDEX multiplo (multiplo) \
		) ENGINE=InnoDB DEFAULT CHARSET=LATIN1");
		instrucciones[num_instrucciones++] = instruccion;

		archivo6 = mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);

		instruccion.sprintf("CREATE OR REPLACE TEMPORARY TABLE auximpuestos  ( \
			claveimp INT (2) NOT NULL, \
			tipoimpu VARCHAR (10) NOT NULL, \
			porcentaje DECIMAL (5, 2) NOT NULL, \
			nombre VARCHAR (40) NOT NULL, \
			INDEX claveimp (claveimp), \
			INDEX tipoimpu (tipoimpu) \
		) ENGINE=InnoDB DEFAULT CHARSET=LATIN1");
		instrucciones[num_instrucciones++] = instruccion;

		archivo7 = mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);

		instruccion.sprintf("CREATE OR REPLACE TEMPORARY TABLE auximpuestosprod  ( \
			producto VARCHAR (8) NOT NULL, \
			nombre VARCHAR (60) NOT NULL, \
			claveimp1 INT(2) NOT NULL, \
			tipoimp1 VARCHAR(10) NOT NULL, \
			porcentaje1 DECIMAL (5, 2) NOT NULL, \
			nombreimp1 VARCHAR (40) NOT NULL, \
			claveimp2 INT(2) NULL DEFAULT NULL, \
			tipoimp2 VARCHAR(10) NULL DEFAULT NULL, \
			porcentaje2 DECIMAL (5, 2) NULL DEFAULT NULL, \
			nombreimp2 VARCHAR (40) NULL DEFAULT NULL, \
			claveimp3 INT(2) NULL DEFAULT NULL, \
			tipoimp3 VARCHAR(10) NULL DEFAULT NULL, \
			porcentaje3 DECIMAL (5, 2) NULL DEFAULT NULL, \
			nombreimp3 VARCHAR (40) NULL DEFAULT NULL, \
			claveimp4 INT(2) NULL DEFAULT NULL, \
			tipoimp4 VARCHAR(10) NULL DEFAULT NULL, \
			porcentaje4 DECIMAL (5, 2) NULL DEFAULT NULL, \
			nombreimp4 VARCHAR (40) NULL DEFAULT NULL, \
			INDEX producto (producto), \
			INDEX tipoimp1 (tipoimp1), \
			INDEX tipoimp2 (tipoimp2), \
			INDEX tipoimp3 (tipoimp3), \
			INDEX tipoimp4 (tipoimp4) \
		) ENGINE=InnoDB DEFAULT CHARSET=LATIN1");
		instrucciones[num_instrucciones++] = instruccion;

		archivo8 = mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);

		instruccion.sprintf("CREATE OR REPLACE TEMPORARY TABLE auximpuestosped  ( \
			producto VARCHAR (8) NOT NULL, \
			present VARCHAR (255) NOT NULL, \
			claveimp1 INT(2) NOT NULL, \
			tipoimp1 VARCHAR(10) NOT NULL, \
			porcentaje1 DECIMAL (5, 2) NOT NULL, \
			nombreimp1 VARCHAR (40) NOT NULL, \
			claveimp2 INT(2) NULL DEFAULT NULL, \
			tipoimp2 VARCHAR(10) NULL DEFAULT NULL, \
			porcentaje2 DECIMAL (5, 2) NULL DEFAULT NULL, \
			nombreimp2 VARCHAR (40) NULL DEFAULT NULL, \
			claveimp3 INT(2) NULL DEFAULT NULL, \
			tipoimp3 VARCHAR(10) NULL DEFAULT NULL, \
			porcentaje3 DECIMAL (5, 2) NULL DEFAULT NULL, \
			nombreimp3 VARCHAR (40) NULL DEFAULT NULL, \
			claveimp4 INT(2) NULL DEFAULT NULL, \
			tipoimp4 VARCHAR(10) NULL DEFAULT NULL, \
			porcentaje4 DECIMAL (5, 2) NULL DEFAULT NULL, \
			nombreimp4 VARCHAR (40) NULL DEFAULT NULL, \
			costo DECIMAL (16, 6) NULL DEFAULT NULL, \
			id INT(4) NULL DEFAULT NULL, \
			INDEX producto (producto), \
			INDEX present (present), \
			INDEX tipoimp1 (tipoimp1), \
			INDEX tipoimp2 (tipoimp2), \
			INDEX tipoimp3 (tipoimp3), \
			INDEX tipoimp4 (tipoimp4) \
		) ENGINE=InnoDB DEFAULT CHARSET=LATIN1");
		instrucciones[num_instrucciones++] = instruccion;

		archivo9 = mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);

		instruccion.sprintf("CREATE OR REPLACE TEMPORARY TABLE auximpuestoscomp  ( \
			producto VARCHAR (8) NOT NULL, \
			present VARCHAR (255) NULL DEFAULT NULL, \
			proveedor VARCHAR(11) NULL DEFAULT NULL, \
			costo DECIMAL (16, 6) NULL DEFAULT NULL, \
			costoimp DECIMAL (16, 6) NULL DEFAULT NULL, \
			claveimp1 INT(2) NOT NULL, \
			tipoimp1 VARCHAR(10) NOT NULL, \
			porcentaje1 DECIMAL (5, 2) NOT NULL, \
			nombreimp1 VARCHAR (40) NOT NULL, \
			claveimp2 INT(2) NULL DEFAULT NULL, \
			tipoimp2 VARCHAR(10) NULL DEFAULT NULL, \
			porcentaje2 DECIMAL (5, 2) NULL DEFAULT NULL, \
			nombreimp2 VARCHAR (40) NULL DEFAULT NULL, \
			claveimp3 INT(2) NULL DEFAULT NULL, \
			tipoimp3 VARCHAR(10) NULL DEFAULT NULL, \
			porcentaje3 DECIMAL (5, 2) NULL DEFAULT NULL, \
			nombreimp3 VARCHAR (40) NULL DEFAULT NULL, \
			claveimp4 INT(2) NULL DEFAULT NULL, \
			tipoimp4 VARCHAR(10) NULL DEFAULT NULL, \
			porcentaje4 DECIMAL (5, 2) NULL DEFAULT NULL, \
			nombreimp4 VARCHAR (40) NULL DEFAULT NULL, \
			INDEX producto (producto), \
			INDEX present (present), \
			INDEX tipoimp1 (tipoimp1), \
			INDEX tipoimp2 (tipoimp2), \
			INDEX tipoimp3 (tipoimp3), \
			INDEX tipoimp4 (tipoimp4) \
		) ENGINE=InnoDB DEFAULT CHARSET=LATIN1");
		instrucciones[num_instrucciones++] = instruccion;

		archivo10 = mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);

		instruccion.sprintf("SELECT \
			dr.recepcion, \
			a.articulo, \
			a.producto, \
			a.present, \
			a.multiplo, \
			a.factor, \
			artf.cantidad \
		FROM drecepciones dr \
		INNER JOIN articulos a ON a.articulo = dr.articulo \
		INNER JOIN recepcionarticulofraccionado artf ON artf.articulo = dr.articulo AND artf.recepcion = dr.recepcion \
		WHERE dr.recepcion IN (%s) \
			INTO OUTFILE '%s'", claveRecepcion, archivo1);
		instrucciones[num_instrucciones++] = instruccion;

		archivo11 = mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);

		instruccion.sprintf("LOAD DATA INFILE '%s' IGNORE INTO table auxrecepcionbase", archivo1);
		instrucciones[num_instrucciones++] = instruccion;

		archivo12 = mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);

		instruccion.sprintf("SELECT \
			articulo, \
			producto, \
			present, \
			multiplo, \
			(cantidad * FACTOR) AS total_unidades \
		FROM auxrecepcionbase \
			INTO OUTFILE '%s'", archivo2);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("LOAD DATA INFILE '%s' IGNORE INTO table auxtotxproducto", archivo2);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("SELECT producto, present, cantidadmin \
		FROM auxtotxproducto \
			INTO OUTFILE '%s'", archivo3);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("LOAD DATA INFILE '%s' IGNORE INTO table auxremanente", archivo3);
		instrucciones[num_instrucciones++] = instruccion;

		if(esParteRel){
			instruccion.sprintf("SELECT \
				a.articulo, \
				a.producto, \
				a.present, \
				a.multiplo, \
				a.factor \
			FROM pedrecepcion pr \
			INNER JOIN pedidos p ON p.referencia = pr.pedido \
			INNER JOIN pedvta_pedprov pp ON pp.referencia_pedprov = p.referencia \
			INNER JOIN pedidosventa pv ON pv.referencia = pp.referencia_pedvta \
			INNER JOIN dventas dv ON dv.referencia = pv.venta \
			INNER JOIN articulos a ON a.articulo = dv.articulo \
			WHERE pr.recepcion IN (%s) \
				INTO OUTFILE '%s'", claveRecepcion, archivo4);
			instrucciones[num_instrucciones++] = instruccion;
		}else{
			instruccion.sprintf("SELECT \
				a.articulo, \
				a.producto, \
				a.present, \
				a.multiplo, \
				a.factor \
			FROM pedrecepcion pr \
			INNER JOIN dpedidos dp ON dp.referencia = pr.pedido \
			INNER JOIN articulos a ON a.articulo = dp.articulo \
			WHERE pr.recepcion IN (%s) \
				INTO OUTFILE '%s'", claveRecepcion, archivo4);
			instrucciones[num_instrucciones++] = instruccion;
		}

		instruccion.sprintf("LOAD DATA INFILE '%s' IGNORE INTO table auxmultxproducto", archivo4);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("SELECT \
			auxmp.articulo, \
			auxmp.producto, \
			auxmp.present, \
			auxmp.multiplo, \
			auxmp.factor, \
			FLOOR(r.remanente  / auxmp.factor) AS cantmult, \
			MOD(r.remanente , auxmp.factor) AS cantresta \
		FROM auxmultxproducto auxmp \
		INNER JOIN auxremanente r ON auxmp.producto = r.producto AND auxmp.present = r.present \
		ORDER BY producto, present \
			INTO OUTFILE '%s'", archivo5);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("LOAD DATA INFILE '%s' IGNORE INTO table auxconversionproducto", archivo5);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("UPDATE auxremanente r \
		INNER JOIN auxmultxproducto m ON r.producto = m.producto AND r.present = m.present \
		INNER JOIN presentacionesminmax pmm ON pmm.producto = m.producto AND pmm.present = m.present \
		SET r.remanente = MOD(r.remanente, m.factor) \
		WHERE m.factor = pmm.maxfactor");
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("SELECT \
		  auxmp.articulo, \
		  auxmp.producto, \
		  auxmp.present, \
		  auxmp.multiplo, \
		  auxmp.factor, \
		  FLOOR(r.remanente / auxmp.factor) AS cantmult, \
		  MOD(r.remanente, auxmp.factor) AS cantresta \
		FROM auxmultxproducto auxmp \
		INNER JOIN auxremanente r ON auxmp.producto = r.producto AND auxmp.present = r.present \
		WHERE auxmp.factor = 1 \
			INTO OUTFILE '%s'", archivo6);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("LOAD DATA INFILE '%s' IGNORE INTO table auxconversionproducto", archivo6);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("SELECT articulo, producto, present, multiplo, FACTOR, cantmult, cantresta \
		FROM auxconversionproducto HAVING cantmult > 0 \
			INTO OUTFILE '%s'", archivo7);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("LOAD DATA INFILE '%s' IGNORE INTO table auxconversionfinal", archivo7);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("SELECT \
			pr.recepcion, \
			pr.pedido, \
			dp.articulo, \
			a.producto, \
			a.present, \
			a.multiplo, \
			dp.claveimp1, \
			dp.claveimp2, \
			dp.claveimp3, \
			dp.claveimp4, \
			dp.costo, \
			dp.iepscuota, \
			dp.id \
		FROM pedrecepcion pr \
		INNER JOIN pedidos p ON p.referencia = pr.pedido \
		INNER JOIN dpedidos dp ON dp.referencia = p.referencia \
		INNER JOIN articulos a ON a.articulo = dp.articulo \
		WHERE pr.recepcion IN(%s) \
			INTO OUTFILE '%s'", claveRecepcion, archivo8);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("LOAD DATA INFILE '%s' IGNORE INTO table auxconversionfinal", archivo8);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("SELECT \
			i.impuesto, \
			i.tipoimpu, \
			i.porcentaje, \
			ti.nombre \
		FROM impuestos i \
		INNER JOIN tiposdeimpuestos ti ON ti.tipoimpu = i.tipoimpu \
			INTO OUTFILE '%s'", archivo9);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("LOAD DATA INFILE '%s' IGNORE INTO table auximpuestos", archivo9);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("SELECT \
			p.producto, \
			p.nombre, \
			i1.claveimp, \
			i1.tipoimpu AS tipoimp1, \
			i1.porcentaje AS porcentaje1, \
			i1.nombre AS nombreimp1, \
			i2.claveimp, \
			i2.tipoimpu AS tipoimp2, \
			i2.porcentaje AS porcentaje2, \
			i2.nombre AS nombreimp2, \
			i3.claveimp, \
			i3.tipoimpu AS tipoimp3, \
			i3.porcentaje AS porcentaje3, \
			i3.nombre AS nombreimp3, \
			i4.claveimp, \
			i4.tipoimpu AS tipoimp4, \
			i4.porcentaje AS porcentaje4, \
			i4.nombre AS nombreimp4 \
		FROM productos p \
		INNER JOIN auximpuestos i1 ON i1.claveimp = p.claveimpc1 \
		LEFT JOIN auximpuestos i2 ON i2.claveimp = p.claveimpc2 \
		LEFT JOIN auximpuestos i3 ON i3.claveimp = p.claveimpc3 \
		LEFT JOIN auximpuestos i4 ON i4.claveimp = p.claveimpc4 \
			INTO OUTFILE '%s'", archivo10);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("LOAD DATA INFILE '%s' IGNORE INTO table auximpuestosprod", archivo10);
		instrucciones[num_instrucciones++] = instruccion;

		if(esParteRel){
			instruccion.sprintf("SELECT \
				a.producto, \
				a.present, \
				i1.claveimp, \
				i1.tipoimpu  AS tipoimp1, \
				i1.porcentaje AS porcentaje1, \
				i1.nombre    AS nombreimp1, \
				i2.claveimp, \
				i2.tipoimpu  AS tipoimp2, \
				i2.porcentaje AS porcentaje2, \
				i2.nombre    AS nombreimp2, \
				i3.claveimp, \
				i3.tipoimpu  AS tipoimp3, \
				i3.porcentaje AS porcentaje3, \
				i3.nombre    AS nombreimp3, \
				i4.claveimp, \
				i4.tipoimpu  AS tipoimp4, \
				i4.porcentaje AS porcentaje4, \
				i4.nombre    AS nombreimp4, \
				(dv.precio / a.factor) AS costo, \
				dv.id \
			FROM pedrecepcion pr \
			INNER JOIN pedidos p ON p.referencia = pr.pedido \
			INNER JOIN pedvta_pedprov pp ON pp.referencia_pedprov = p.referencia \
			INNER JOIN pedidosventa pv ON pv.referencia = pp.referencia_pedvta \
			INNER JOIN dventas dv ON dv.referencia = pv.venta \
			INNER JOIN articulos a ON a.articulo = dv.articulo \
			INNER JOIN presentacionesminmax pmm ON pmm.producto = a.producto AND pmm.present = a.present \
			INNER JOIN auximpuestos i1 ON i1.claveimp = dv.claveimp1 \
			LEFT JOIN auximpuestos i2 ON i2.claveimp = dv.claveimp2 \
			LEFT JOIN auximpuestos i3 ON i3.claveimp = dv.claveimp3 \
			LEFT JOIN auximpuestos i4 ON i4.claveimp = dv.claveimp4 \
			WHERE pr.recepcion IN(%s) \
			GROUP BY a.producto, a.present \
				INTO OUTFILE '%s'", claveRecepcion, archivo11);
			instrucciones[num_instrucciones++] = instruccion;
		} else {
			instruccion.sprintf("SELECT \
				a.producto, \
				a.present, \
				i1.claveimp, \
				i1.tipoimpu  AS tipoimp1, \
				i1.porcentaje AS porcentaje1, \
				i1.nombre    AS nombreimp1, \
				i2.claveimp, \
				i2.tipoimpu  AS tipoimp2, \
				i2.porcentaje AS porcentaje2, \
				i2.nombre    AS nombreimp2, \
				i3.claveimp, \
				i3.tipoimpu  AS tipoimp3, \
				i3.porcentaje AS porcentaje3, \
				i3.nombre    AS nombreimp3, \
				i4.claveimp, \
				i4.tipoimpu  AS tipoimp4, \
				i4.porcentaje AS porcentaje4, \
				i4.nombre    AS nombreimp4, \
				(dp.costo / a.factor) AS costo, \
				dp.id \
			FROM pedrecepcion pr \
			INNER JOIN pedidos p ON p.referencia = pr.pedido \
			INNER JOIN dpedidos dp ON dp.referencia = p.referencia \
			INNER JOIN articulos a ON a.articulo = dp.articulo \
			INNER JOIN presentacionesminmax pmm ON pmm.producto = a.producto AND pmm.present = a.present \
			INNER JOIN auximpuestos i1 ON i1.claveimp = dp.claveimp1 \
			LEFT JOIN auximpuestos i2 ON i2.claveimp = dp.claveimp2 \
			LEFT JOIN auximpuestos i3 ON i3.claveimp = dp.claveimp3 \
			LEFT JOIN auximpuestos i4 ON i4.claveimp = dp.claveimp4 \
			WHERE pr.recepcion IN(%s) \
			GROUP BY a.producto, a.present \
				INTO OUTFILE '%s'", claveRecepcion, archivo11);
			instrucciones[num_instrucciones++] = instruccion;
		}

		instruccion.sprintf("LOAD DATA INFILE '%s' IGNORE INTO table auximpuestosped", archivo11);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("SELECT \
			a.producto, \
			a.present, \
			c.proveedor, \
			dc.costo, \
			dc.costoimp, \
			i1.claveimp, \
			i1.tipoimpu  AS tipoimp1, \
			i1.porcentaje AS porcentaje1, \
			i1.nombre    AS nombreimp1, \
			i2.claveimp, \
			i2.tipoimpu  AS tipoimp2, \
			i2.porcentaje AS porcentaje2, \
			i2.nombre    AS nombreimp2, \
			i3.claveimp, \
			i3.tipoimpu  AS tipoimp3, \
			i3.porcentaje AS porcentaje3, \
			i3.nombre    AS nombreimp3, \
			i4.claveimp, \
			i4.tipoimpu  AS tipoimp4, \
			i4.porcentaje AS porcentaje4, \
			i4.nombre    AS nombreimp4 \
		FROM drecepciones dr \
		INNER JOIN dcompras dc ON dc.articulo = dr.articulo \
		INNER JOIN articulos a ON a.articulo = dc.articulo \
		INNER JOIN compras c ON c.referencia = dc.referencia \
		INNER JOIN terminales t ON t.terminal = c.terminal \
		INNER JOIN secciones sec ON sec.seccion = t.seccion \
		INNER JOIN auximpuestos i1 ON i1.claveimp = dc.claveimp1 \
		LEFT JOIN auximpuestos i2 ON i2.claveimp = dc.claveimp2 \
		LEFT JOIN auximpuestos i3 ON i3.claveimp = dc.claveimp3 \
		LEFT JOIN auximpuestos i4 ON i4.claveimp = dc.claveimp4 \
		WHERE dr.recepcion IN(%s) AND c.cancelado = 0 AND sec.sucursal = '%s' \
		ORDER BY c.fechacom DESC \
			INTO OUTFILE '%s'", claveRecepcion, sucursal, archivo12);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("LOAD DATA INFILE '%s' IGNORE INTO table auximpuestoscomp", archivo12);
		instrucciones[num_instrucciones++] = instruccion;

		aux_buffer_sql = mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (int i = 0; i < num_instrucciones; i++)
			aux_buffer_sql = mFg.AgregaStringABuffer(instrucciones[i],aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {

			// Obtiene todos los generales (de cabecera) del pedido
			instruccion.sprintf("SELECT \
				r.*,com.folioprov, pr.pedido, p.*, com.comprador, com.fechacom, \
				sec.sucursal, imp.porcentaje, '' AS refpagprov \
				from recepciones r \
				INNER JOIN pedrecepcion pr ON pr.recepcion=r.recepcion \
				INNER JOIN pedidos p ON p.referencia=pr.pedido \
				INNER JOIN almacenes alma ON alma.almacen = p.almacen \
				INNER JOIN secciones sec ON sec.seccion = alma.seccion \
				LEFT JOIN comprecepcion cr ON cr.recepcion=r.recepcion \
				LEFT JOIN compras com ON com.referencia=cr.compra AND com.cancelado = 0 \
				LEFT JOIN impuestos imp ON imp.impuesto = p.impuestoret \
				WHERE r.recepcion IN(%s) \
				LIMIT 1 ", claveRecepcion);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Obtiene todos los datos del proveedor de la recepcion
			instruccion.sprintf("SELECT \
				pro.*, \
				imp.porcentaje AS isrret, \
				pro.impuestoret \
			FROM proveedores pro \
			INNER JOIN recepciones r ON r.proveedor = pro.proveedor \
			LEFT JOIN impuestos imp ON imp.impuesto = pro.impuestoret \
			WHERE r.recepcion IN(%s) and r.proveedor=pro.proveedor\
			GROUP BY pro.proveedor", claveRecepcion);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Obtiene todo el detalle del pedido con algunos datos extras que necesita el cliente
			instruccion.sprintf("SELECT \
				auxcf.articulo, \
				auxcf.cantmult AS cantidad, \
				IFNULL(auxip.costo*auxcf.factor, auxic.costo) AS costo, \
				auxcf.present, \
				auxcf.producto, \
				prod.nombre, \
				auxcf.multiplo, \
				IFNULL(auxip.claveimp1, IFNULL(auxic.claveimp1, prod.claveimp1)) AS claveimp1, \
				IFNULL(auxip.tipoimp1, IFNULL(auxic.tipoimp1, prod.tipoimp1)) AS tipoimpuesto1, \
				IFNULL(auxip.porcentaje1, IFNULL(auxic.porcentaje1, prod.porcentaje1)) AS porcentajeimp1, \
				IFNULL(auxip.nombreimp1, IFNULL(auxic.nombreimp1, prod.nombreimp1)) AS nomtipoimp1, \
				IFNULL(auxip.claveimp2, IFNULL(auxic.claveimp2, prod.claveimp2)) AS claveimp2, \
				IFNULL(auxip.tipoimp2, IFNULL(auxic.tipoimp2, prod.tipoimp2)) AS tipoimpuesto2, \
				IFNULL(auxip.porcentaje2, IFNULL(auxic.porcentaje2, prod.porcentaje2)) AS porcentajeimp2, \
				IFNULL(auxip.nombreimp2, IFNULL(auxic.nombreimp2, prod.nombreimp2)) AS nomtipoimp2, \
				IFNULL(auxip.claveimp3, IFNULL(auxic.claveimp3, prod.claveimp3)) AS claveimp3, \
				IFNULL(auxip.tipoimp3, IFNULL(auxic.tipoimp3, prod.tipoimp3)) AS tipoimpuesto3, \
				IFNULL(auxip.porcentaje3, IFNULL(auxic.porcentaje3, prod.porcentaje3)) AS porcentajeimp3, \
				IFNULL(auxip.nombreimp3, IFNULL(auxic.nombreimp3, prod.nombreimp3)) AS nomtipoimp3, \
				IFNULL(auxip.claveimp4, IFNULL(auxic.claveimp4, prod.claveimp4)) AS claveimp4, \
				IFNULL(auxip.tipoimp4, IFNULL(auxic.tipoimp4, prod.tipoimp4)) AS tipoimpuesto4, \
				IFNULL(auxip.porcentaje4, IFNULL(auxic.porcentaje4, prod.porcentaje4)) AS porcentajeimp4, \
				IFNULL(auxip.nombreimp4, IFNULL(auxic.nombreimp4, prod.nombreimp4)) AS nomtipoimp4, \
				0 AS iepscuota, \
				IFNULL(auxip.id, 0) AS id, \
				(auxcf.factor / pmm.cajalogisticafactor) AS mult, \
				(a.factor / pmm.cajalogisticafactor) AS cajasbultos \
			FROM recepciones r \
			INNER JOIN drecepciones dr ON dr.recepcion = r.recepcion \
			INNER JOIN articulos a ON a.articulo = dr.articulo \
			INNER JOIN auxconversionfinal auxcf ON auxcf.producto = a.producto AND auxcf.present = a.present \
			INNER JOIN auximpuestosprod prod ON prod.producto = a.producto \
			LEFT JOIN auximpuestosped auxip ON auxip.producto = a.producto AND auxip.present = a.present \
			LEFT JOIN auximpuestoscomp auxic ON auxic.producto = a.producto AND auxic.present = a.present \
			LEFT JOIN presentacionesminmax pmm ON pmm.producto = a.producto AND pmm.present = a.present \
			WHERE dr.recepcion IN (%s) \
			GROUP BY auxcf.producto, auxcf.present, auxcf.multiplo \
			ORDER BY prod.nombre, auxcf.present, auxcf.multiplo", claveRecepcion);

			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Obtiene mensaje del pedido
			instruccion.sprintf("SELECT GROUP_CONCAT(IF(cm.mensaje = '(null)', NULL, cm.mensaje)) AS mensaje \
			FROM recepciones r \
			LEFT JOIN pedrecepcion pr ON pr.recepcion = r.recepcion \
			LEFT JOIN pedidoscomprasmensajes cm ON cm.referencia = pr.pedido \
			WHERE r.recepcion IN(%s)", claveRecepcion);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}

	} __finally {
		if(archivo1 != ""){
			mServidorVioleta->BorraArchivoTemp(archivo1);
		}
		if(archivo2 != ""){
			mServidorVioleta->BorraArchivoTemp(archivo2);
		}
		if(archivo3 != ""){
			mServidorVioleta->BorraArchivoTemp(archivo3);
		}
		if(archivo4 != ""){
			mServidorVioleta->BorraArchivoTemp(archivo4);
		}
		if(archivo5 != ""){
			mServidorVioleta->BorraArchivoTemp(archivo5);
		}
		if(archivo6 != ""){
			mServidorVioleta->BorraArchivoTemp(archivo6);
		}
		if(archivo7 != ""){
			mServidorVioleta->BorraArchivoTemp(archivo7);
		}
		if(archivo8 != ""){
			mServidorVioleta->BorraArchivoTemp(archivo8);
		}
		if(archivo9 != ""){
			mServidorVioleta->BorraArchivoTemp(archivo9);
		}
		if(archivo10 != ""){
			mServidorVioleta->BorraArchivoTemp(archivo10);
		}
		if(archivo11 != ""){
			mServidorVioleta->BorraArchivoTemp(archivo11);
		}
		if(archivo12 != ""){
			mServidorVioleta->BorraArchivoTemp(archivo12);
		}
		delete buffer_sql;
	}

}
//------------------------------------------------------------------------------
//ID_AUD_RECEPCION
void ServidorCompras::ConsultaInfRecepcion(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql = new char[1024 * 64];
	char *aux_buffer_sql = buffer_sql;
	int num_instrucciones = 0;
	AnsiString instruccion, consulta, instrucciones[1000];

	AnsiString condicion_pedido = " ";
	AnsiString claveRecepcion, pedido;

	AnsiString archivo1, archivo2, archivo3;

	try{
		claveRecepcion = mFg.ExtraeStringDeBuffer(&parametros);
		pedido = mFg.ExtraeStringDeBuffer(&parametros);

		if(pedido != " ") {
			condicion_pedido.sprintf("AND pr.pedido = '%s'", pedido);
		}

		instruccion.sprintf("CREATE OR REPLACE TEMPORARY TABLE auxproductototal ( \
			recepcion varchar(11) NOT NULL, \
			articulo varchar(9) NOT NULL DEFAULT '0', \
			nombreprod VARCHAR (60) NULL DEFAULT NULL, \
			producto VARCHAR (8) NULL DEFAULT NULL, \
			present VARCHAR (255) NULL DEFAULT NULL, \
			multped VARCHAR (10) NULL DEFAULT NULL, \
			multrec VARCHAR (10) NULL DEFAULT NULL, \
			factorped DECIMAL(10, 3) NULL DEFAULT NULL, \
			factorrec DECIMAL(10, 3) NULL DEFAULT NULL, \
			cantrecibida decimal(12, 3) DEFAULT NULL, \
			cantpedida decimal(12, 3) DEFAULT NULL, \
			cantdevuelta decimal(12, 3) DEFAULT NULL, \
			cantsobrante decimal(12, 3) DEFAULT NULL, \
			INDEX recepcion (recepcion), \
			INDEX articulo (articulo), \
			INDEX producto (producto), \
			INDEX present (present), \
			INDEX multped (multped), \
			INDEX multrec (multrec) \
		) ENGINE = InnoDB DEFAULT CHARSET = LATIN1");
		instrucciones[num_instrucciones++] = instruccion;

		archivo1 = mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);

		instruccion.sprintf("CREATE OR REPLACE TEMPORARY TABLE auxproductoconvertido ( \
			articulo varchar(9) NOT NULL DEFAULT '0', \
			nombreprod VARCHAR (60) NULL DEFAULT NULL, \
			producto VARCHAR (8) NULL DEFAULT NULL, \
			present VARCHAR (255) NULL DEFAULT NULL, \
			multped VARCHAR (10) NULL DEFAULT NULL, \
			multrec VARCHAR (10) NULL DEFAULT NULL, \
			factorped DECIMAL(10, 3) NULL DEFAULT NULL, \
			factorrec DECIMAL(10, 3) NULL DEFAULT NULL, \
			cantrecibida decimal(12, 3) DEFAULT NULL, \
			cantpedida decimal(12, 3) DEFAULT NULL, \
			cantdevuelta decimal(12, 3) DEFAULT NULL, \
			incidencias TINYINT(1) NOT NULL DEFAULT 0, \
            multdif VARCHAR (10) NULL DEFAULT NULL, \
			diferencias decimal(12, 3) DEFAULT NULL, \
			cantsobrante decimal(12, 3) DEFAULT NULL, \
			INDEX articulo (articulo), \
			INDEX producto (producto), \
			INDEX present (present), \
			INDEX multped (multped), \
			INDEX multrec (multrec) \
		) ENGINE = InnoDB DEFAULT CHARSET = LATIN1");
		instrucciones[num_instrucciones++] = instruccion;

		archivo2 = mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);

		instruccion.sprintf("SELECT \
			r.recepcion, \
			IFNULL(pr.articulo, a.articulo) AS articulo, \
			prod.nombre, \
			IFNULL(pr.producto, a.producto) AS producto, \
			IFNULL(pr.present, a.present) AS present, \
			IFNULL(pr.multiplo, a.multiplo) AS 'multiplo pedido', \
			a.multiplo AS 'multiplo recibido', \
			IFNULL(pr.factor, a.factor) AS 'factor pedido', \
			a.factor AS 'factor recibido', \
			dr.cantidad AS 'cantidad recibida', \
			IFNULL(pr.cantidad, 0) AS 'cantidad pedida', \
			COALESCE(drd.cantidad, 0) AS devoluciones, \
			MOD((dr.cantidad * a.factor), IFNULL(pr.factor, a.factor)) AS sobrante \
		FROM recepciones r \
		INNER JOIN drecepciones dr ON dr.recepcion = r.recepcion \
		INNER JOIN articulos a ON a.articulo = dr.articulo \
		INNER JOIN productos prod ON prod.producto = a.producto \
		LEFT JOIN ( \
			SELECT pr.recepcion, a.articulo, a.producto, a.present, \
				a.multiplo, a.factor, dp.cantidad, p.referencia AS pedido \
			FROM pedidos p \
			INNER JOIN pedrecepcion pr ON pr.pedido = p.referencia \
			INNER JOIN dpedidos dp ON pr.pedido = dp.referencia \
			INNER JOIN articulos a ON a.articulo = dp.articulo \
			WHERE pr.recepcion IN (%s) \
		) pr ON pr.recepcion = r.recepcion AND a.producto = pr.producto AND a.present = pr.present \
		LEFT JOIN recepcionesdevol rdv ON rdv.recepcion = r.recepcion \
		LEFT JOIN drecepciondevol drd ON drd.devolucion = rdv.devolucion AND drd.articulo = a.articulo \
		WHERE r.recepcion IN (%s) %s \
		GROUP BY a.producto, a.present, a.multiplo \
			INTO OUTFILE '%s'", claveRecepcion, claveRecepcion, condicion_pedido, archivo1);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("LOAD DATA INFILE '%s' IGNORE INTO table auxproductototal", archivo1);
		instrucciones[num_instrucciones++] = instruccion;


		archivo3 = mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);

		instruccion.sprintf("SELECT pr.recepcion, a.articulo, prod.nombre, a.producto, a.present, a.multiplo AS 'multiplo pedido', \
			a.multiplo AS 'multiplo recibido', a.factor AS 'factor pedido', a.factor AS 'factor recibido', \
			0 AS 'cantidad recibida', IFNULL(dp.cantidad, 0) AS 'cantidad pedida', \
			0 AS devoluciones, 0 AS sobrante \
			FROM pedidos p \
			INNER JOIN pedrecepcion pr ON pr.pedido = p.referencia \
			INNER JOIN dpedidos dp ON pr.pedido = dp.referencia \
			INNER JOIN articulos a ON a.articulo = dp.articulo \
			INNER JOIN productos prod ON prod.producto = a.producto \
			WHERE pr.recepcion IN (%s) AND \
			 NOT EXISTS (SELECT 1 FROM auxproductototal apt WHERE apt.articulo = a.articulo) %s \
			GROUP BY a.articulo  \
			INTO OUTFILE '%s' ", claveRecepcion, condicion_pedido, archivo3);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("LOAD DATA INFILE '%s' IGNORE INTO table auxproductototal", archivo3);
		instrucciones[num_instrucciones++] = instruccion;


		instruccion.sprintf("SELECT \
			apt.articulo, \
			apt.nombreprod, \
			apt.producto, \
			apt.present, \
			apt.multped, \
			apt.multrec, \
			apt.factorped, \
			apt.factorrec, \
			apt.cantrecibida, \
			apt.cantpedida, \
			apt.cantdevuelta, \
			if((apt.cantrecibida*apt.factorrec) != (apt.cantpedida*apt.factorped), 1, 0) AS incidencias, \
			if(apt.factorped < apt.factorrec, apt.multped, apt.multrec) AS multdif, \
			((apt.cantpedida*apt.factorped) - (apt.cantrecibida*apt.factorrec)) AS diferencias, \
			apt.cantsobrante \
		FROM auxproductototal apt \
		ORDER BY apt.nombreprod ASC, apt.present ASC \
			INTO OUTFILE '%s'", archivo2);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("LOAD DATA INFILE '%s' IGNORE INTO table auxproductoconvertido", archivo2);
		instrucciones[num_instrucciones++] = instruccion;

		aux_buffer_sql = mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (int i = 0; i < num_instrucciones; i++)
			aux_buffer_sql = mFg.AgregaStringABuffer(instrucciones[i],aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {

			// Obtiene todos los datos del proveedor de la recepcion
			instruccion.sprintf("SELECT rec.recepcion, rec.fechamodi, rec.horamodi, rec.fecharep, pro.*, rec.fraccionado \
			FROM recepciones rec \
			INNER JOIN proveedores pro ON pro.proveedor = rec.proveedor \
			WHERE rec.recepcion IN(%s)", claveRecepcion);
			mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Obtiene todos los datos del pedido en base a la recepción
			instruccion.sprintf("SELECT ped.referencia, ped.fechaped \
				FROM recepciones rec \
				INNER JOIN pedrecepcion p_r ON p_r.recepcion = rec.recepcion \
				INNER JOIN pedidos ped ON ped.referencia = p_r.pedido \
				WHERE rec.recepcion IN(%s) \
				GROUP BY ped.referencia ", claveRecepcion);
			mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Obtiene referencia de compras
			instruccion.sprintf("SELECT \
				cr.compra, \
				c.folioprov, \
				CONCAT(emp.nombre,' ',emp.appat,' ',emp.apmat) AS comprador \
			FROM recepciones r \
			LEFT JOIN comprecepcion cr ON cr.recepcion = r.recepcion \
			LEFT JOIN pedrecepcion pr ON pr.recepcion = r.recepcion \
			LEFT JOIN compras c ON c.referencia = cr.compra \
			LEFT JOIN empleados emp ON emp.empleado = c.comprador \
			WHERE r.recepcion IN(%s) AND c.cancelado = 0 \
			GROUP BY cr.compra ", claveRecepcion);
			mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Obtiene detalle de recepciones
			instruccion.sprintf("SELECT \
				articulo, \
				nombreprod, \
				present, \
				CONCAT(multped,'X',factorped) AS multped, \
				CONCAT(multrec,'X',factorrec) AS multrec, \
				cantpedida, \
				cantrecibida, \
				cantdevuelta, \
				incidencias, \
				multdif, \
				diferencias \
			FROM auxproductoconvertido \
			ORDER BY nombreprod ASC, present ASC");
			mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}

	}__finally{
		if(archivo1 != ""){
			mServidorVioleta->BorraArchivoTemp(archivo1);
		}
		if(archivo2 != ""){
			mServidorVioleta->BorraArchivoTemp(archivo2);
		}

		if(archivo3 != ""){
			mServidorVioleta->BorraArchivoTemp(archivo3);
		}
	}
}
//------------------------------------------------------------------------------
//ID_REG_AGENDA
void ServidorCompras::registraAgendaRecepcion(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*64*10];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instruccion,referencia,fecha,hora;

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	referencia=mFg.ExtraeStringDeBuffer(&parametros);
	fecha=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
	hora=mFg.ExtraeStringDeBuffer(&parametros);

	AnsiString aux = fecha+" "+hora;

	instruccion.sprintf("update pedidos set fechaagenda = '%s' where referencia = '%s'",
		aux,referencia);
	mServidorVioleta->EjecutaSelectSqlNulo(Respuesta,  MySQL, instruccion.c_str());
}
//------------------------------------------------------------------------------
//ID_MOD_AGENDA
void ServidorCompras::modificaAgendaRecepcion(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*64*10];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instruccion,referencia,fecha,hora;

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	referencia=mFg.ExtraeStringDeBuffer(&parametros);
	fecha=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
	hora=mFg.ExtraeStringDeBuffer(&parametros);

	AnsiString aux = fecha+" "+hora;

	instruccion.sprintf("update pedidos set fechaagenda = '%s' where referencia = '%s'",
		aux,referencia);
	mServidorVioleta->EjecutaSelectSqlNulo(Respuesta,  MySQL, instruccion.c_str());
}
//------------------------------------------------------------------------------
//ID_REG_ADUANA
void ServidorCompras::registraAduana(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
		char *buffer_sql=new char[1024*64*10];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instruccion,referencia,fecha,hora;

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	referencia=mFg.ExtraeStringDeBuffer(&parametros);

    AnsiString aux = fecha+" "+hora;

	instruccion.sprintf("update pedidos set fechaaduana = CURDATE() where referencia = '%s'",
		referencia);
	mServidorVioleta->EjecutaSelectSqlNulo(Respuesta,  MySQL, instruccion.c_str());

	instruccion.sprintf("select fechaaduana from pedidos where referencia = '%s'",referencia);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

}
//------------------------------------------------------------------------------
//ID_DETALLE_RECEPCION
void ServidorCompras::EjecutaDetalleRecepcion(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql = new char[1024 * 64];
	char *aux_buffer_sql = buffer_sql;
	int num_instrucciones = 0;
	AnsiString instruccion, consulta, instrucciones[100];
	AnsiString claveRecepcion, producto;
    AnsiString condicion_producto=" ";
	AnsiString archivo1, archivo2, archivo3;

	claveRecepcion = mFg.ExtraeStringDeBuffer(&parametros);
	producto = mFg.ExtraeStringDeBuffer(&parametros);

	if(producto!=" ") {
		condicion_producto.sprintf(" AND (auxrtot.nombreprod LIKE '%%%s%%' OR auxrtot.producto = '%s')", producto, producto);
    }

	try{

		instruccion.sprintf("CREATE OR REPLACE TEMPORARY TABLE auxrecepciontot ( \
			articulo varchar(9) NOT NULL DEFAULT '0', \
			siguiente_articulo varchar(9) NULL DEFAULT NULL, \
			nombreprod VARCHAR (60) NULL DEFAULT NULL, \
			producto VARCHAR (8) NULL DEFAULT NULL, \
			present VARCHAR (255) NULL DEFAULT NULL, \
			multiplo VARCHAR (10) NULL DEFAULT NULL, \
			factor DECIMAL(10, 3) NULL DEFAULT NULL, \
			cantpedida decimal(12, 3) DEFAULT NULL, \
			cantrecibida decimal(12, 3) DEFAULT NULL, \
			cantcomprada decimal(12, 3) DEFAULT NULL, \
			pedido VARCHAR (11) NULL DEFAULT NULL, \
			recepcion VARCHAR (11) NULL DEFAULT NULL, \
			INDEX articulo (articulo), \
			INDEX producto (producto), \
			INDEX present (present), \
			INDEX multiplo (multiplo), \
			INDEX pedido (pedido), \
			INDEX recepcion (recepcion) \
		) ENGINE=InnoDB DEFAULT CHARSET=LATIN1");
		instrucciones[num_instrucciones++] = instruccion;

		archivo1 = mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);

		instruccion.sprintf("SELECT \
			a.articulo, \
			LEAD(a.articulo) OVER (ORDER BY prod.nombre, a.present, a.multiplo, pr.pedido) AS 'siguiente articulo', \
			prod.nombre, \
			a.producto, \
			a.present, \
			a.multiplo, \
			a.factor, \
			IFNULL((pr.cantidad * pr.factor), (dr.cantidad * a.factor)) AS 'cantidad pedida', \
			dr.cantidad AS 'cantidad recibida', \
			IF(auxac.multiplo_comprado = a.multiplo, IFNULL(auxac.cantidad_comprada / a.factor , 0), IFNULL(auxac.cantidad_comprada , 0)) AS 'cantidad comprada', \
			pr.pedido AS pedido, \
			r.recepcion AS recepcion \
		FROM recepciones r \
			INNER JOIN drecepciones dr ON dr.recepcion = r.recepcion \
			INNER JOIN articulos a ON a.articulo = dr.articulo \
			INNER JOIN productos prod ON prod.producto = a.producto \
			LEFT JOIN ( \
				SELECT \
					a.articulo, a.producto, a.present, a.multiplo, a.factor, dp.cantidad, pr.recepcion, pr.pedido \
				FROM pedidos p \
				INNER JOIN pedrecepcion pr ON pr.pedido = p.referencia \
				INNER JOIN dpedidos dp ON pr.pedido = dp.referencia \
				INNER JOIN articulos a ON a.articulo = dp.articulo \
				WHERE pr.recepcion = '%s' \
			) pr ON pr.recepcion = r.recepcion AND pr.producto = a.producto AND pr.present = a.present \
			LEFT JOIN ( \
				SELECT \
					a.producto, \
					a.present, \
					a.multiplo AS multiplo_comprado, \
					a.factor AS factor_comprado, \
					SUM(dc.cantidad * a.factor) AS cantidad_comprada \
				FROM compras c \
				INNER JOIN dcompras dc ON dc.referencia = c.referencia \
				INNER JOIN articulos a ON a.articulo = dc.articulo \
				INNER JOIN ( \
					SELECT \
						referencia AS compra, \
						recepcion \
					FROM compraspedidosprov cpp \
					INNER JOIN compras c ON c.referencia = cpp.compra \
					WHERE recepcion = '%s' AND c.cancelado = 0 \
					GROUP BY c.referencia \
				) cr ON cr.compra = c.referencia \
				WHERE c.cancelado = 0 AND cr.recepcion = '%s' \
				GROUP BY a.producto, a.present \
			) auxac ON auxac.producto = a.producto AND auxac.present = a.present \
		WHERE r.recepcion = '%s' \
		GROUP BY pr.pedido, dr.articulo \
		ORDER BY prod.nombre, a.present \
			INTO OUTFILE '%s'", claveRecepcion, claveRecepcion, claveRecepcion, claveRecepcion, archivo1);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("LOAD DATA INFILE '%s' IGNORE INTO table auxrecepciontot", archivo1);
		instrucciones[num_instrucciones++] = instruccion;

		aux_buffer_sql = mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (int i = 0; i < num_instrucciones; i++)
			aux_buffer_sql = mFg.AgregaStringABuffer(instrucciones[i],aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {

			// Obtiene detalle de recepciones
			instruccion.sprintf("SELECT fechamodi, horamodi FROM recepciones \
			WHERE recepcion = '%s' ",
			claveRecepcion );
			mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Obtiene detalle de recepciones
			instruccion.sprintf("SELECT \
				auxrtot.articulo, \
				auxrtot.siguiente_articulo, \
				auxrtot.nombreprod AS nombre,  \
				auxrtot.producto, \
				auxrtot.present, \
				auxrtot.multiplo, \
                auxrtot.factor, \
				auxrtot.cantpedida AS cantidad_pedida, \
				auxrtot.cantrecibida AS cantidad_recibida, \
				auxrtot.cantcomprada AS cantidad_comprada, \
				IF((SUM(IFNULL(auxrtot.cantrecibida,0)) - SUM(IFNULL(auxrtot.cantcomprada,0))) > SUM(IFNULL(auxrtot.cantpedida,0)),SUM(IFNULL(auxrtot.cantrecibida,0)),(SUM(IFNULL(auxrtot.cantrecibida,0)) - SUM(IFNULL(auxrtot.cantcomprada,0)))) AS cantidad, \
				IF(artf.articulo IS NULL, 0 , 1) AS seleccionado, \
				auxrtot.pedido, \
				auxrtot.recepcion \
			FROM auxrecepciontot auxrtot \
			LEFT JOIN recepcionarticulofraccionado artf ON artf.recepcion = auxrtot.recepcion AND artf.pedido = auxrtot.pedido AND artf.articulo = auxrtot.articulo \
			WHERE 1 %s \
			GROUP BY auxrtot.articulo, auxrtot.pedido \
			ORDER BY auxrtot.nombreprod, auxrtot.present, auxrtot.multiplo, auxrtot.pedido", condicion_producto);
			mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

        }

    }__finally{
		if(archivo1 != ""){
			mServidorVioleta->BorraArchivoTemp(archivo1);
		}
		if(archivo2 != ""){
			mServidorVioleta->BorraArchivoTemp(archivo2);
		}
		if(archivo3 != ""){
			mServidorVioleta->BorraArchivoTemp(archivo3);
		}
        delete buffer_sql;
	}

}
//------------------------------------------------------------------------------
//ID_CON_RECEPCION_GEN
void ServidorCompras::ConsultaRecepcionGenerada(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql = new char[1024 * 64];
	char *aux_buffer_sql = buffer_sql;
	int num_instrucciones = 0;
	AnsiString instruccion, consulta, instrucciones[1000];

	AnsiString claveRecepcion;

	AnsiString archivo1, archivo2, archivo3;

	try{
		claveRecepcion = mFg.ExtraeStringDeBuffer(&parametros);

		instruccion.sprintf("CREATE OR REPLACE TEMPORARY TABLE auxproductototal ( \
			recepcion varchar(11) NOT NULL, \
			articulo varchar(9) NOT NULL DEFAULT '0', \
			nombreprod VARCHAR (60) NULL DEFAULT NULL, \
			producto VARCHAR (8) NULL DEFAULT NULL, \
			present VARCHAR (255) NULL DEFAULT NULL, \
			multped VARCHAR (10) NULL DEFAULT NULL, \
			multrec VARCHAR (10) NULL DEFAULT NULL, \
			factorped DECIMAL(10, 3) NULL DEFAULT NULL, \
			factorrec DECIMAL(10, 3) NULL DEFAULT NULL, \
			cantrecibida decimal(12, 3) DEFAULT NULL, \
			cantpedida decimal(12, 3) DEFAULT NULL, \
			cantdevuelta decimal(12, 3) DEFAULT NULL, \
			cantsobrante decimal(12, 3) DEFAULT NULL, \
			INDEX recepcion (recepcion), \
			INDEX articulo (articulo), \
			INDEX producto (producto), \
			INDEX present (present), \
			INDEX multped (multped), \
			INDEX multrec (multrec) \
		) ENGINE = InnoDB DEFAULT CHARSET = LATIN1");
		instrucciones[num_instrucciones++] = instruccion;

		archivo1 = mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);

		instruccion.sprintf("CREATE OR REPLACE TEMPORARY TABLE auxproductoconvertido ( \
			articulo varchar(9) NOT NULL DEFAULT '0', \
			nombreprod VARCHAR (60) NULL DEFAULT NULL, \
			producto VARCHAR (8) NULL DEFAULT NULL, \
			present VARCHAR (255) NULL DEFAULT NULL, \
			multped VARCHAR (10) NULL DEFAULT NULL, \
			multrec VARCHAR (10) NULL DEFAULT NULL, \
			factorped DECIMAL(10, 3) NULL DEFAULT NULL, \
			factorrec DECIMAL(10, 3) NULL DEFAULT NULL, \
			cantrecibida decimal(12, 3) DEFAULT NULL, \
			cantpedida decimal(12, 3) DEFAULT NULL, \
			cantdevuelta decimal(12, 3) DEFAULT NULL, \
			incidencias TINYINT(1) NOT NULL DEFAULT 0, \
			diferencias decimal(12, 3) DEFAULT NULL, \
			cantsobrante decimal(12, 3) DEFAULT NULL, \
			INDEX articulo (articulo), \
			INDEX producto (producto), \
			INDEX present (present), \
			INDEX multped (multped), \
			INDEX multrec (multrec) \
		) ENGINE = InnoDB DEFAULT CHARSET = LATIN1");
		instrucciones[num_instrucciones++] = instruccion;

		archivo2 = mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);

		instruccion.sprintf("SELECT \
			r.recepcion, \
			IFNULL(pr.articulo, a.articulo) AS articulo, \
			prod.nombre, \
			IFNULL(pr.producto, a.producto) AS producto, \
			IFNULL(pr.present, a.present) AS present, \
			IFNULL(pr.multiplo, a.multiplo) AS 'multiplo pedido', \
			a.multiplo AS 'multiplo recibido', \
			IFNULL(pr.factor, a.factor) AS 'factor pedido', \
			a.factor AS 'factor recibido', \
			dr.cantidad AS 'cantidad recibida', \
			IFNULL(pr.cantidad, dr.cantidad) AS 'cantidad pedida', \
			COALESCE(rd.cantidad, 0) AS devoluciones, \
			MOD((dr.cantidad * a.factor), IFNULL(pr.factor, a.factor)) AS sobrante \
		FROM recepciones r \
		INNER JOIN drecepciones dr ON dr.recepcion = r.recepcion \
		INNER JOIN articulos a ON a.articulo = dr.articulo \
		INNER JOIN productos prod ON prod.producto = a.producto \
		LEFT JOIN ( \
			SELECT pr.recepcion, a.articulo, a.producto, a.present, \
				a.multiplo, a.factor, dp.cantidad, p.referencia AS pedido \
			FROM pedidos p \
			INNER JOIN pedrecepcion pr ON pr.pedido = p.referencia \
			INNER JOIN dpedidos dp ON pr.pedido = dp.referencia \
			INNER JOIN articulos a ON a.articulo = dp.articulo \
			WHERE pr.recepcion IN (%s) \
		) pr ON pr.recepcion = r.recepcion AND a.producto = pr.producto AND a.present = pr.present \
		LEFT JOIN ( \
			SELECT r.recepcion, dr.cantidad \
			FROM recepciones r \
			INNER JOIN drecepciones dr ON dr.recepcion = r.recepcion \
			INNER JOIN recepcionesdevol rd ON rd.recepcion = r.recepcion \
			INNER JOIN drecepciondevol drd ON drd.devolucion = rd.devolucion \
			WHERE r.recepcion IN (%s) \
		) rd ON rd.recepcion = r.recepcion \
		WHERE r.recepcion IN (%s) \
		GROUP BY a.producto, a.present, a.multiplo \
			INTO OUTFILE '%s'", claveRecepcion, claveRecepcion, claveRecepcion, archivo1);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("LOAD DATA INFILE '%s' IGNORE INTO table auxproductototal", archivo1);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("SELECT \
			apt.articulo, \
			apt.nombreprod, \
			apt.producto, \
			apt.present, \
			apt.multped, \
			apt.multrec, \
			apt.factorped, \
			apt.factorrec, \
			apt.cantrecibida, \
			apt.cantpedida, \
			apt.cantdevuelta, \
			if((apt.cantrecibida*apt.factorrec) != (apt.cantpedida*apt.factorped), 1, 0) AS incidencias, \
			((apt.cantpedida*apt.factorped) - (apt.cantrecibida*apt.factorrec)) AS diferencias, \
			apt.cantsobrante \
		FROM auxproductototal apt \
		ORDER BY apt.nombreprod ASC, apt.present ASC \
			INTO OUTFILE '%s'", archivo2);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("LOAD DATA INFILE '%s' IGNORE INTO table auxproductoconvertido", archivo2);
		instrucciones[num_instrucciones++] = instruccion;

		aux_buffer_sql = mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (int i = 0; i < num_instrucciones; i++)
			aux_buffer_sql = mFg.AgregaStringABuffer(instrucciones[i],aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {

			// Obtiene todos los datos del proveedor del pedido
			instruccion.sprintf("SELECT GROUP_CONCAT(rec.recepcion) AS recepcion, \
			rec.fechamodi, rec.horamodi, rec.fecharep, rec.fraccionado, pro.* \
			FROM recepciones rec \
			INNER JOIN proveedores pro ON pro.proveedor = rec.proveedor \
			WHERE rec.recepcion IN(%s)", claveRecepcion);
			mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Obtiene todos los datos del pedido en base a la recepción
			instruccion.sprintf("SELECT ped.referencia, ped.fechaped \
				FROM recepciones rec \
				INNER JOIN pedrecepcion p_r ON p_r.recepcion = rec.recepcion \
				INNER JOIN pedidos ped ON ped.referencia = p_r.pedido \
				WHERE rec.recepcion IN(%s) \
				GROUP BY ped.referencia ", claveRecepcion);
			mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Obtiene referencia de compras
			instruccion.sprintf("SELECT \
				cr.compra, \
				c.folioprov, \
				CONCAT(emp.nombre,' ',emp.appat,' ',emp.apmat) AS comprador \
			FROM recepciones r \
			LEFT JOIN comprecepcion cr ON cr.recepcion = r.recepcion \
			LEFT JOIN pedrecepcion pr ON pr.recepcion = r.recepcion \
			LEFT JOIN compras c ON c.referencia = cr.compra \
			LEFT JOIN empleados emp ON emp.empleado = c.comprador \
			WHERE r.recepcion IN(%s) AND c.cancelado = 0 \
			GROUP BY cr.compra ", claveRecepcion);
			mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Obtiene detalle de recepciones
            instruccion.sprintf("SELECT \
                articulo, \
                nombreprod, \
                present, \
                CONCAT(multped,'X',factorped) AS multped, \
                CONCAT(multrec,'X',factorrec) AS multrec, \
                cantpedida, \
                cantrecibida, \
                cantdevuelta, \
                incidencias, \
                diferencias \
            FROM auxproductoconvertido \
            ORDER BY nombreprod ASC, present ASC");
            mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}

	}__finally{
		if(archivo1 != ""){
			mServidorVioleta->BorraArchivoTemp(archivo1);
		}
		if(archivo2 != ""){
			mServidorVioleta->BorraArchivoTemp(archivo2);
		}
		if(archivo3 != ""){
			mServidorVioleta->BorraArchivoTemp(archivo3);
		}
	}
}
//------------------------------------------------------------------------------
//ID_FRACC_RECEPCION
void ServidorCompras::EjecutaFraccionaRecepcionxPedido(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql = new char[1024 * 64];
	char *aux_buffer_sql = buffer_sql;
	int num_instrucciones = 0;
	AnsiString instruccion, consulta, instrucciones[500];
	AnsiString recepcion, pedido, articulo;
	double cantidad;
	int numElemts;
	numElemts = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));

	try{

		instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++] = "START TRANSACTION";

		for (int i = 0; i < numElemts; i++) {

			recepcion = mFg.ExtraeStringDeBuffer(&parametros);
			pedido 	  = mFg.ExtraeStringDeBuffer(&parametros);
			articulo  = mFg.ExtraeStringDeBuffer(&parametros);
			cantidad  = mFg.CadenaAFlotante(mFg.ExtraeStringDeBuffer(&parametros));

			instruccion.sprintf("INSERT IGNORE INTO recepcionarticulofraccionado \
			(articulo, pedido, recepcion, cantidad) \
			VALUES \
			('%s', '%s', '%s', %f)", articulo, pedido, recepcion, cantidad);
			instrucciones[num_instrucciones++] = instruccion;
		}

		instrucciones[num_instrucciones++] = "COMMIT";

		aux_buffer_sql = mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (int i = 0; i < num_instrucciones; i++)
			aux_buffer_sql = mFg.AgregaStringABuffer(instrucciones[i],aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
		mServidorVioleta->EjecutaBufferAccionesSql (Respuesta,MySQL,buffer_sql);

    }__finally{
        delete buffer_sql;
	}

}
//------------------------------------------------------------------------------
//ID_BUSQ_PEDIDO_RECEPCIONAR
void ServidorCompras::EjecutaBusqPedidoRecepcionar(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
	AnsiString pedido;

	pedido = mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	instruccion.sprintf("SELECT \
        rp.recepcion, \
		p.referencia, \
		prov.razonsocial, \
		IF(rp.recepcion IS NULL, 0, 1) AS tienerecepcion \
	FROM pedidos p \
	INNER JOIN proveedores prov ON prov.proveedor = p.proveedor \
	LEFT JOIN pedrecepcion rp ON rp.pedido = p.referencia \
	WHERE p.referencia = '%s' \
	GROUP BY p.referencia", pedido);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	instruccion.sprintf("SELECT \
		a.articulo, \
		prod.nombre, \
		a.present, \
		a.multiplo, \
		dr.cantidad, \
        drd.cantidad AS cantidaddevol \
	FROM pedidos p \
	INNER JOIN pedrecepcion pr ON pr.pedido = p.referencia \
	INNER JOIN drecepciones dr ON dr.recepcion = pr.recepcion \
	INNER JOIN articulos a ON a.articulo = dr.articulo \
	INNER JOIN productos prod ON prod.producto = a.producto \
	LEFT JOIN recepcionesdevol rd ON rd.recepcion = pr.recepcion \
	LEFT JOIN drecepciondevol drd ON drd.devolucion = rd.devolucion AND dr.articulo = drd.articulo \
	WHERE p.referencia = '%s'", pedido);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}
//------------------------------------------------------------------------------
//ID_GUARDA_RECEPCION
void ServidorCompras::EjecutaGuardaRecepcion(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{

}
//------------------------------------------------------------------------------
//ID_CON_INF_RECEP
void ServidorCompras::ConsultaInformacionRecepciones(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*64];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instruccion;
	AnsiString recepcion;

	recepcion=mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	instruccion.sprintf("SELECT \
		recepcion, \
		r.terminal, \
		fechamodi, \
		CONCAT(empa.nombre,' ',empa.appat,' ',empa.apmat) AS usualta, \
		horaini, \
		CONCAT(empr.nombre,' ',empr.appat,' ',empr.apmat) AS recepcionista, \
		fraccionado, \
		cancelado, \
		horamodi, \
		CONCAT(empm.nombre,' ',empm.appat,' ',empm.apmat) AS usumod, \
		horafin \
	FROM recepciones r \
	INNER JOIN empleados empa ON empa.empleado = r.usualta \
	INNER JOIN empleados empm ON empm.empleado = r.usumod \
	INNER JOIN empleados empr ON empr.empleado = r.recepcionista \
	WHERE r.recepcion IN (%s)",
	recepcion);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	instruccion.sprintf("SELECT \
		c.referencia, \
		CONCAT(empc.nombre,' ',empc.appat,' ',empc.apmat) AS comprador, \
		CONCAT(empa.nombre,' ',empa.appat,' ',empa.apmat) AS usualta, \
		CONCAT(empm.nombre,' ',empm.appat,' ',empm.apmat) AS usumodi, \
		c.terminal, \
		c.fechaalta, \
		c.horaalta, \
		c.fechamodi, \
		c.horamodi \
	FROM recepciones r \
	INNER JOIN comprecepcion cr ON cr.recepcion = r.recepcion \
	INNER JOIN compras c ON c.referencia = cr.compra \
	INNER JOIN empleados empc ON empc.empleado = c.comprador \
	INNER JOIN empleados empa ON empa.empleado = c.usualta \
	INNER JOIN empleados empm ON empm.empleado = c.usumodi \
	WHERE c.cancelado = 1 AND r.recepcion IN (%s)",
	recepcion);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}
//------------------------------------------------------------------------------
