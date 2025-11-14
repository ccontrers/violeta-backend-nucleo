#include <vcl.h>

//---------------------------------------------------------------------------

#include "pch.h"

#pragma hdrstop

#include "ClassServidorVentas.h"
#include "ClassServidorVioleta.h"
#include "ClassBufferRespuestas.h"
#include "violetaS.h"
#include <DateUtils.hpp>
#include "FormServidorVioleta.h"
#include "ClassArregloDetalle.h"
#include "comunes.h"
#include "ClassComprobanteFiscalDigital.h"
#include "ClassFuncionesOpenssl.h"
#include "Timbrar.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#include "RegExpr.hpp"

//---------------------------------------------------------------------------
// ID_REV_VENTA_EXISTENCIAS
void ServidorVentas::RevisaExistenciasVenta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[25000*1000];
	char *aux_buffer_sql=buffer_sql;
	int i, num_partidas;
	double cantidad, cantidad_mod, cantidad_final, cantidad_art, cantidad_act;
	int num_instrucciones=0;
	AnsiString instruccion;
	AnsiString *instrucciones = new AnsiString[25000];
	AnsiString tarea, articulo_aux, sucursal, tipo_mov;
	AnsiString almacen, condicion_almacen=" ";
	AnsiString almacen_aux, cad_conjunto_almacenes=" ";
	TDate fecha=Today();
	AnsiString archivo_temp1, archivo_temp2, archivo_temp3, archivo_temp4, archivo_temp5;
	BufferRespuestas* resp_almacenes=NULL;
	AnsiString modo_calcular_existencias;
	AnsiString having,envio_datos;

	try{
		sucursal=mFg.ExtraeStringDeBuffer(&parametros);
		almacen=mFg.ExtraeStringDeBuffer(&parametros);
		tarea=mFg.ExtraeStringDeBuffer(&parametros); // Indicador si se va a agregar o modificar.
		tipo_mov=mFg.ExtraeStringDeBuffer(&parametros); // Ver si se realizara desde movimientos de almacen


		envio_datos = mFg.ExtraeStringDeBuffer(&parametros);// todos los articulos existencia remota


		//nueva consulta de un parametro del método para revisar las existencias con base a la tabla de existencias actuales.
		BufferRespuestas* resp_parametros=NULL;
		try{
			instruccion.sprintf("SELECT * FROM parametrosemp WHERE parametro = 'CALCEXISTRV' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_parametros);
			if (resp_parametros->ObtieneNumRegistros()>0){
				modo_calcular_existencias=resp_parametros->ObtieneDato("valor");
			}
		}__finally{
			if (resp_parametros!=NULL) delete resp_parametros;
		}

		if(almacen!=" "){
			// Se le da prioridad al parámetro de almacén.

			// Todos los almacenes que pertenecen a la misma sucursal que el almacén indicado.
			instruccion.sprintf("SELECT a.almacen FROM almacenes a \
				INNER JOIN secciones s ON a.seccion=s.seccion \
				WHERE s.sucursal IN (SELECT s.sucursal FROM almacenes al \
				INNER JOIN secciones s ON s.seccion=al.seccion \
				WHERE almacen='%s')", almacen);

		} else {
			if (sucursal!=" ") {
					// Todos los almacenes que pertenecen a la sucursal dada.
					instruccion.sprintf("SELECT a.almacen FROM almacenes a \
						INNER JOIN secciones s ON a.seccion=s.seccion \
						WHERE s.sucursal='%s'", sucursal);

			} else
				throw Exception("Debe indicarse una sucursal o un almacén (RevisaExistenciasVenta)");

		}

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

		// Crea una tabla temporal para recibir los artículos que se van a vender.
		instruccion="create temporary table auxarticulos ( \
			articulo varchar(9), producto varchar(8), present varchar(255), \
			cantidad decimal(12,3), factor decimal(10,3), \
			INDEX(articulo)) Engine = InnoDB";
		instrucciones[num_instrucciones++]=instruccion;

		// Crea una tabla temporal para agrupar los productos por producto-present-almacen
		instruccion="create temporary table auxagrupados ( \
			producto varchar(8), present varchar(255), almacen char(4), \
			INDEX(producto, present)) Engine = InnoDB";
		instrucciones[num_instrucciones++]=instruccion;

		// Crea una tabla temporal para agrupar producto presentacion
		instruccion="create temporary table auxagrupadospp ( \
			producto varchar(8), present varchar(255), \
			cantidad decimal(12,3), INDEX(producto, present)) Engine = InnoDB";
		instrucciones[num_instrucciones++]=instruccion;

		// Crea una tabla donde se pondrán las existencias que se vayan encontrando.
		instruccion="create temporary table existenciasaux ( \
			producto varchar(8), present varchar(255), almacen char(4), \
			tipo char(2), cantidad decimal(12,3), INDEX(producto, present, almacen)) Engine = InnoDB";
		instrucciones[num_instrucciones++]=instruccion;

		// Recibe los articulos y su respectiva cantidad que se va a vender.
		num_partidas=StrToInt(mFg.ExtraeStringDeBuffer(&parametros)); // Obtiene el número de partidas
		for (i=0; i<num_partidas; i++) {
			articulo_aux=mFg.ExtraeStringDeBuffer(&parametros);
			cantidad=mFg.CadenaAFlotante(mFg.ExtraeStringDeBuffer(&parametros));
			cantidad_mod=mFg.CadenaAFlotante(mFg.ExtraeStringDeBuffer(&parametros));

			cantidad_final = cantidad_mod - cantidad;

			if(tipo_mov == "MOVALM"){
				if(tarea == "M"){
					if(cantidad_final < 0){
						cantidad_art = fabs(cantidad_final);
						cantidad_act = 0;
					}
						else{
							cantidad_art = 0;
							cantidad_act = cantidad_final;
						}

					instruccion.sprintf("INSERT INTO existenciasaux (producto, present, almacen, tipo, cantidad) \
					SELECT a.producto,a.present,'%s', 'DM', (%s * a.factor) as cantidad FROM articulos a WHERE a.articulo = '%s'",
					almacen, AnsiString(FloatToStr(cantidad_act)), articulo_aux);
					instrucciones[num_instrucciones++]=instruccion;
				}
					else
						cantidad_art = cantidad;
			}
			else
				cantidad_art = cantidad;

			instruccion.sprintf("insert into auxarticulos ( \
				articulo, cantidad) values ('%s',%s)", articulo_aux, AnsiString(FloatToStr(cantidad_art)));
			instrucciones[num_instrucciones++]=instruccion;

		}

		// Agrega el factor , producto y presentacion a cada artículo
		instruccion.sprintf("update auxarticulos aux, articulos a \
			set aux.factor=a.factor, aux.producto=a.producto, \
				aux.present=a.present  \
			where aux.articulo=a.articulo ");
		instrucciones[num_instrucciones++]=instruccion;

		// Agrupa por producto-presentacion-almacén
		if (cad_conjunto_almacenes!=" ") {
			condicion_almacen.sprintf("where alm.almacen in (%s) ", cad_conjunto_almacenes);
		} else condicion_almacen=" ";
		instruccion.sprintf("insert into auxagrupados (producto, present, almacen) \
			select aux.producto, aux.present, alm.almacen \
			from auxarticulos aux, almacenes alm \
			%s \
			group by aux.producto, aux.present, alm.almacen",condicion_almacen);
		instrucciones[num_instrucciones++]=instruccion;

		// Agrupa por producto-presentacion-cantidad
		instruccion.sprintf("insert into auxagrupadospp (producto, present, cantidad) \
			select aux.producto, aux.present, sum(aux.factor*aux.cantidad) as cantidad \
			from auxarticulos aux \
			group by aux.producto, aux.present");
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
		//cuando se envie desde el calculador de pedidos ocupo que se envien todos los articulos
        //con la finalidad de extraer la cantidad actual de cada articulo
		if(envio_datos=="1"){
			having.sprintf(" ");
		}else{
			having.sprintf("having cantexist<cantidped");
		}

		instruccion="SET SESSION sql_log_bin = 1";
		instrucciones[num_instrucciones++]=instruccion;

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			 /*  ******* cod antes
			 from existenciasaux e, auxagrupados aux, productos p, auxagrupadospp auxpp \

			 , pmm.maxmult='CAJA'
			 canttot<ventas
			 cantexistmay<cantexistunid
			 */
			// Desglose de movimientos por almacén.
			instruccion.sprintf("SELECT p.nombre, e.present, TRUNCATE(SUM(e.cantidad/pmm.maxfactor),0) AS cantexistmay,  \
				MOD(SUM(e.cantidad), pmm.maxfactor) AS cantexistunid , \
				TRUNCATE(auxpp.cantidad/pmm.maxfactor,0) AS cantmayor, \
				MOD(auxpp.cantidad, pmm.maxfactor) AS cantunidad, \
				CONCAT(pmm.maxmult,'X',pmm.maxfactor), pmm.minmult, \
				sum(e.cantidad) as cantexist, auxpp.cantidad as cantidped, amax.articulo \
				FROM existenciasaux e \
				inner join  auxagrupados aux \
				inner join productos p \
				inner join auxagrupadospp auxpp \
				inner join presentacionesminmax pmm \
				inner join articulos amax \
				where e.producto=aux.producto and e.present=aux.present and \
				e.producto=auxpp.producto and e.present=auxpp.present and \
				p.producto=aux.producto and e.almacen=aux.almacen \
				and pmm.producto=aux.producto AND pmm.present=aux.present \
				AND pmm.producto=auxpp.producto AND pmm.present=auxpp.present \
				and amax.producto=aux.producto AND amax.present=aux.present \
				AND amax.producto=auxpp.producto AND amax.present=auxpp.present and amax.multiplo=pmm.maxmult \
				group by aux.producto, aux.present \
				%s ",having);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	} __finally {
		if(resp_almacenes!=NULL) delete resp_almacenes;
		delete buffer_sql;
		delete[] instrucciones;
		mServidorVioleta->BorraArchivoTemp(archivo_temp1);

	}
}

//---------------------------------------------------------------------------
//ID_GRA_CARTAPORTE
void ServidorVentas::GrabaCartaPorte(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  GRABA
	char *buffer_sql=new char[1024*64*10];
	char *aux_buffer_sql=buffer_sql;
	char *buffer_cancela=new char[1024*64*10];
	DatosTabla datos(mServidorVioleta->Tablas);
	DatosTabla datos_detalle(mServidorVioleta->Tablas);
	AnsiString clave_venta, usuario, terminal, mensaje;
	int num_partidas, i;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[2000];
	double anticipo;
	AnsiString Aux_valor;
	TDateTime fecha_inic, fecha_venc, fecha_vta;
	int error=0;
	AnsiString articulo;
	TDate fecha=Today();
	TTime hora=Time();
	BufferRespuestas* resp_fecha_vta=NULL;
	AnsiString cliente, proveedor;


	try {
		clave_venta=mFg.ExtraeStringDeBuffer(&parametros); // Clave de la venta.
		usuario=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que esta grabando la venta.
		terminal=mFg.ExtraeStringDeBuffer(&parametros); // Terminal desde la que se está grabando la venta.
		mensaje=mFg.ExtraeStringDeBuffer(&parametros); // Mensaje

		// Obtiene los datos de la tabla de ventas
		datos.AsignaTabla("cartasporte");
		parametros+=datos.InsCamposDesdeBuffer(parametros);

		// Extrae los datos que necesitamos para crear las letras y transacciones.
		Aux_valor=datos.ObtieneValorCampo("valor");
		cliente=datos.ObtieneValorCampo("clientedest");
		proveedor=datos.ObtieneValorCampo("proveedororig");

		fecha_vta=fecha;

		if (error==0) {
			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

			instruccion.sprintf("select @seccion:=seccion, @depart:=depart, @asigfolvta:=asigfolvta, @anchofolvta:=anchofolvta from terminales where terminal='%s'", terminal);
			instrucciones[num_instrucciones++]=instruccion;

			// Folio de sistema.
			instruccion.sprintf("select @folioaux:=valor from foliosemp where folio='CARTASP' AND sucursal = '%s' %s",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @foliosig=@folioaux+1");
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @folioaux=cast(@folioaux as char)");
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @folio=lpad(@folioaux,9,'0')");
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("update foliosemp set valor=@foliosig where folio='CARTASP' AND sucursal = '%s' ",FormServidor->ObtieneClaveSucursal());
			instrucciones[num_instrucciones++]=instruccion;


			// Graba la cabecera en la tabla "cartasporte"
			datos.InsCampo("referencia", "@folio",1);
			datos.InsCampo("usualta", usuario);
			datos.InsCampo("usumodi", usuario);
			datos.InsCampo("fechacp", mFg.DateToAnsiString(fecha));
			datos.InsCampo("horacp", mFg.TimeToAnsiString(hora));
			datos.InsCampo("fechaalta", mFg.DateToAnsiString(fecha));
			datos.InsCampo("horaalta", mFg.TimeToAnsiString(hora));
			datos.InsCampo("fechamodi", mFg.DateToAnsiString(fecha));
			datos.InsCampo("horamodi", mFg.TimeToAnsiString(hora));
			datos.InsCampo("cancelado", "0");
			datos.InsCampo("terminal", terminal);
			instrucciones[num_instrucciones++]=datos.GenerarSqlInsert();

			// Guarda el mensaje
			if (mensaje!=" ") { // Un espacio lo tomamos como nulo.
				instruccion.sprintf("insert into cartaspmensajes (referencia, mensaje) values (@folio,'%s')",
					mensaje);
				instrucciones[num_instrucciones++]=instruccion;
			}

			// Graba las partidas en "dcartaspart"
			num_partidas=StrToInt(mFg.ExtraeStringDeBuffer(&parametros)); // Obtiene el número de partidas
			for (i=0; i<num_partidas; i++) {
				datos_detalle.AsignaTabla("dcartaspart");
				parametros+=datos_detalle.InsCamposDesdeBuffer(parametros);
				datos_detalle.InsCampo("referencia", "@folio", 1);
				datos_detalle.InsCampo("id", mFg.IntToAnsiString(i));

				// Inserta en dventa
				instrucciones[num_instrucciones++]=datos_detalle.GenerarSqlInsert();
			}

			// Graba las partidas en "dcartaspconcep"
			num_partidas=StrToInt(mFg.ExtraeStringDeBuffer(&parametros)); // Obtiene el número de partidas
			for (i=0; i<num_partidas; i++) {
				datos_detalle.AsignaTabla("dcartaspconcep");
				parametros+=datos_detalle.InsCamposDesdeBuffer(parametros);
				datos_detalle.InsCampo("referencia", "@folio", 1);

				// Inserta en dventa
				instrucciones[num_instrucciones++]=datos_detalle.GenerarSqlInsert();
			}



		}

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			if (error==0) {
				////////////////////////////////////////////////////////////////
				// Crea el CFD (si la configuracion así lo indica) y hace commit
				ComprobanteFiscalDigital *cfd=NULL;
				try {
					cfd=new ComprobanteFiscalDigital(mServidorVioleta);
					if (cfd->EmitirCFDI40(Respuesta, MySQL, "CART")) {
						cfd->CreaCFDICartaPorte33(Respuesta, MySQL);
					} else {
						throw Exception(L"La versión del CFDI tipo CART ya no es soportada");
					}

				} __finally {
					if(cfd!=NULL) delete cfd;
				}
				///////////////////////////////////////////////////////////////*/
				instruccion.sprintf("select %d as error, v.referencia as folio, v.fechacp, v.horacp from cartasporte v where v.referencia=@folio", error);
			} else {
				instruccion.sprintf("select %d as error", error);
			}
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	} __finally {
		if (resp_fecha_vta!=NULL) delete resp_fecha_vta;
		delete buffer_sql;
		delete buffer_cancela;
	}
}


//----------------------------------------------------------------------------
//ID_CANC_CARTAPORTE
void ServidorVentas::CancelaCartaPorte(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  CANCELA CARTA PORTE
	char *buffer_sql=new char[1024*32];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instrucciones[50], instruccion;
	int num_instrucciones=0;
	AnsiString clave, usuario;
	int error=0;
	int i;
	TDate fecha=Today();
	TTime hora=Time();

	try{
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave de la venta (referencia)
		usuario=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que esta cancelando la venta.

		// Verifica que la fecha de la carta porte sea posterior a la fecha de cierre.
		instruccion.sprintf("select @error:=if(cp.fechacp<=cast(e.valor as datetime), 1, 0) as error from cartasporte cp left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' where cp.referencia='%s'",FormServidor->ObtieneClaveSucursal(), clave);
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 3, error);

		if (error==0) {
			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

			/*****************************/
			/*	Se prepara para cancelar */
			/*****************************/
			TMemoryStream *msPFX;
			AnsiString instruccion;
			UnicodeString timbreUsr, timbrePass, timbreUrl, respuestaCancelacion, rfcReceptor, total;
			UnicodeString errores_timbrado;
			UnicodeString rfcEmisor, passCerPFX, uuid;
			AnsiString empresa, certificadoB64 = "";
			FuncionesOpenssl *mFuncOpenssl;
			unsigned char *cer;
			bool timbrado_prueba;
			int xmllongitud;

			BufferRespuestas* resp_config_cfd = NULL;
			BufferRespuestas* resp_cfd = NULL;

			msPFX = new TMemoryStream();
			CoInitialize(NULL);
			Timbrar *tfd = NULL;
			mFuncOpenssl = new FuncionesOpenssl();

			try {
				instruccion.sprintf(
					"SELECT cfd.empresatrans, cfd.cfduuid, cfd.monto, REPLACE(REPLACE(clipag.rfc,' ',''),'-','') AS rfcreceptor \
					 FROM cfdcartasporte cfd \
					 inner join cartasporte c on cfd.referencia=c.referencia \
					 inner join clientes clipag on c.clientepaga=clipag.cliente \
						WHERE cfd.referencia='%s' ", clave
				);
				if (!mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_cfd))
					throw Exception("No se pudieron consultar los parámetros de cfdi");
				if (resp_cfd->ObtieneNumRegistros()==0)
					throw Exception("No se encontraron parámetros de cfdi");
				empresa = resp_cfd->ObtieneDato("empresatrans");
				uuid = resp_cfd->ObtieneDato("cfduuid");
				total = resp_cfd->ObtieneDato("monto");
				rfcReceptor = resp_cfd->ObtieneDato("rfcreceptor");

				//Se consultan los datos para cancelar la carta porte
				instruccion.sprintf(
					"SELECT \
						p.certtimb, p.passcerttimb, p.fechaexpicert, p.fechavenccert, REPLACE(REPLACE(p.rfcemisor,' ',''),'-','') as rfcemisor, \
						p.urltimb, p.usuariotimb, p.passwordtimb, p.timbradoprueba \
					 FROM paramcartasp p \
						WHERE p.empresatrans='%s' ", empresa
				);

				if (!mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_config_cfd))
					throw Exception("No se pudieron consultar los parámetros de facturación");
				if (resp_config_cfd->ObtieneNumRegistros()==0)
					throw Exception("No se encontraron parámetros de facturación");
				certificadoB64 = resp_config_cfd->ObtieneDato("certtimb");
				if (certificadoB64=="")
					throw Exception("No existe un certificado .pfx cargado, debe cargarla en parametros de cartas porte");
				passCerPFX = resp_config_cfd->ObtieneDato("passcerttimb");
				rfcEmisor = resp_config_cfd->ObtieneDato("rfcemisor");

				//Se obtiene el certificado y se coloca en un stream
				cer = mFuncOpenssl->FromBase64((unsigned char *)certificadoB64.data(), &xmllongitud);
				msPFX->WriteBuffer((unsigned char *)cer, xmllongitud-1);

				//Datos para conectarse al PAC
				timbreUrl = resp_config_cfd->ObtieneDato("urltimb");
				timbreUsr = resp_config_cfd->ObtieneDato("usuariotimb");
				timbrePass = resp_config_cfd->ObtieneDato("passwordtimb");
				if (resp_config_cfd->ObtieneDato("timbradoprueba")=="1") {
					timbrado_prueba=true;
					#ifndef _DEBUG
					throw Exception(L"EN VERSION RELEASE NO ESTA PERMITIDO TIMBRAR EN MODO DE PRUEBA");
					#endif
				} else	timbrado_prueba=false;

				try {
					tfd=new Timbrar(timbrado_prueba);
					tfd->timbreUrl = timbreUrl;
					tfd->timbreUsr = timbreUsr;
					tfd->timbrePass = timbrePass;
					tfd->timbreTimeOut = 30;
					respuestaCancelacion = tfd->cancelaCFDIEdicom(rfcEmisor, rfcReceptor, total, uuid, msPFX, passCerPFX, "","");
					errores_timbrado=tfd->ObtieneErrores();
				} __finally {
					if (tfd!=NULL) delete tfd;
				}

				if(respuestaCancelacion!=uuid)
					errores_timbrado += L"El uuid retornado no coincide o esta vacio.";
				if (errores_timbrado!="")
					throw Exception(L"ERRORES AL CANCELAR : "+errores_timbrado);

			} __finally {
				if (resp_config_cfd!=NULL)	delete resp_config_cfd;
				if (resp_cfd!=NULL)	delete resp_cfd;
			}
			/*****************************/
			/*	          FIN            */
			/*****************************/

			// Cancela la venta
			instruccion.sprintf("update cartasporte set cancelado=1, fechamodi='%s', horamodi='%s', usumodi='%s' where referencia='%s' and cancelado=0", mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario, clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Cancela el CFD
			instruccion.sprintf("update cfdcartasporte set estado=0, fechacancela='%s %s' where referencia='%s' and tipocomprobante='CARTAP'", mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), clave);
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
//ID_CON_CARTAPORTE
void ServidorVentas::ConsultaCartaPorte(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA CARTA PORTE
	AnsiString instruccion;
	AnsiString clave;

	clave=mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los generales (de cabecera) de la carta porte
	instruccion.sprintf("select cp.*, \
		cfdcartasporte.cfdcartap, cfdcartasporte.cfduuid, cfdcartasporte.xmlgenerado, cfdcartasporte.cadenaoriginal, cfdcartasporte.version, \
		vm.mensaje, \
		col.nombre as nomcolonia, loc.nombre as nomlocalidad, \
		emp1.nombre as nomcajero, \
		concat(emp3.nombre,' ',emp3.appat) as nomusualta, \
		concat(emp4.nombre,' ',emp4.appat) as nomchofer \
		from cartasporte cp \
		left join cfdcartasporte on cp.referencia=cfdcartasporte.referencia and cfdcartasporte.tipocomprobante='CARTAP' \
		LEFT JOIN  clientes c ON cp.clientedest=c.cliente \
		LEFT JOIN  proveedores pro ON cp.proveedororig=pro.proveedor \
		left join colonias col on c.colonia=col.colonia \
		left join localidades loc on col.localidad=loc.localidad \
		left join empleados emp1 on cp.usumodi=emp1.empleado \
		left join embarques emb on emb.embarque=cp.embarque \
		left join empleados emp3 on cp.usualta=emp3.empleado \
		left join empleados emp4 on emb.chofer=emp4.empleado \
		left join cartaspmensajes vm on vm.referencia=cp.referencia \
		where cp.referencia='%s'", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);


	// Obtiene todos los datos del cliente de la carta porte
	instruccion.sprintf("select cli.* from clientes cli, cartasporte cp where cp.referencia='%s' and cp.clientedest=cli.cliente", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene todos los datos del proveedor de la carta porte
	instruccion.sprintf("select cli.* from proveedores cli, cartasporte cp where cp.referencia='%s' and cp.proveedororig=cli.proveedor", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Verifica que la fecha de la carta sea posterior a la fecha de cierre.
	instruccion.sprintf("select if(cp.fechacp>=cast(e.valor as datetime), 1, 0) as modificar from cartasporte cp left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' where cp.referencia='%s'",FormServidor->ObtieneClaveSucursal(), clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene todo el detalle de la venta con algunos datos extras que necesita el cliente

	// Obtiene los datos de articulos incluidos en la carta porte
	instruccion="select d.referencia, d.articulo, d.cantidad, ";
	instruccion+="d.peso, d.volumen, ";
	instruccion+="a.multiplo, p.nombre, a.producto,a.present, a.multiplo ";
	instruccion+=" from dcartaspart d ";
	instruccion+=" INNER JOIN articulos  a ON d.articulo=a.articulo ";
	instruccion+=" INNER JOIN productos p ON p.producto=a.producto ";
	instruccion+="where d.referencia='";
	instruccion+=clave;
	instruccion+="' order by d.id asc ";
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene los datos conceptos que se declaran en la carta porte
	instruccion="select d.cantidad, ";
	instruccion+="d.peso, d.descripcion ";
	instruccion+=" from dcartaspconcep d ";
	instruccion+="where d.referencia='";
	instruccion+=clave;
	instruccion+="' order by idconcepto ";
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

}

//----------------------------------------------------------------------------
//ID_GRA_VENTA
void ServidorVentas::GrabaVenta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  GRABA VENTA
	char *buffer_sql=new char[10000*1000];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	DatosTabla datos_detalle(mServidorVioleta->Tablas);
	AnsiString tarea_venta, clave_venta, tipo_captura, usuario, terminal, kit, mensaje,almacen_global;
	int cantkits;
	double cantidad;
	int num_partidas, i;
	int num_instrucciones=0;
	AnsiString instruccion;
	AnsiString * instrucciones = new AnsiString[10000];
	double anticipo, valor;
	AnsiString Aux_valor;
	AnsiString periodic;
	int dias_plazo;
	TDateTime fecha_inic, fecha_venc, fecha_vta;
	int acredito;
	int error=0;
	AnsiString articulo, articuloact;
	TDate fecha=Today();
	TTime hora=Time();
	BufferRespuestas* resp_fecha_vta=NULL;
	BufferRespuestas* resp_fecha_vta_canc=NULL;
	AnsiString foliofisico;
	AnsiString cliente;
	AnsiString ticket;
	bool calcular_foliofisico=0, calcular_folioticket=0;
	AnsiString tipo_original;
	AnsiString referencia, facturar, cancelar, tipoft;
	AnsiString cancelar_ticket, crear_notacred;
	AnsiString mParamNuevoMetodo, mParamNuevaClave, mParamNuevosDigitos, mParamNuevoUsoCfdi;
	AnsiString errorMensaje = "prueba", cancelado;
	double total_valida = 0.00, diferencia;
	AnsiString cancelado_nc = "", refnc = "";
	double diferenciaPrecioAnt, diferenciaPrecioAct, precioValida, diferenciacompant;
	AnsiString tipoPrecio;
	double mParamPrecioMin;
	AnsiString nombre, present, prod;
	AnsiString mDIRcalle,mDIRnumext,mDIRnumint,mDIRcolonia,mDIRcp,mDIRubicacionX,mDIRubicacionY,mDIRreferencia_domicilio;
	AnsiString mPedidoAFacturar, mUuidRelacionado, mCampoFiscalRel, refventa, mDIRSector;

	AnsiString transaccion, refCorte;
	AnsiString origen_venta, formas_pago, cantidades_pagada, porcentajes;

	//VariablesKits
	int mCantidadKits,mCantArtKits;
	AnsiString mVentaKit,mFolioKit,mDeslosadoKit,mCantidadKit,mModificadoKit;
	AnsiString mArticuloKit,mAntArtKit,mPrecioArtKit,mTipoPreArtKit;

	AnsiString IDTrnPinpad;
	AnsiString tipoPinpad;
	try {

		mPedidoAFacturar=mFg.ExtraeStringDeBuffer(&parametros); // En caso de ser un pedido, se procede a marcarlo como facturado
		clave_venta=mFg.ExtraeStringDeBuffer(&parametros); // Clave de la venta.
		tarea_venta=mFg.ExtraeStringDeBuffer(&parametros); // Indicador si se va a agregar o modificar.

		// Verificaciones para no permitir modificaciones cuando no sea permitido.
		if (tarea_venta=="M")
			throw Exception("Con la nueva facturación electrónica, ya no se deben modificar las ventas, mejor cancele y vuelva a crear.");

		/////////////////////////////////////////////////////////////////////////////////////////////////////
		tipo_captura=mFg.ExtraeStringDeBuffer(&parametros); // '1'=Mayoreo, '2'=Menudeo //////NO USADO///////
		/////////////////////////////////////////////////////////////////////////////////////////////////////
		tipo_original=mFg.ExtraeStringDeBuffer(&parametros); // '-1'=Nueva,  '0'=Factura, '1'=Ticket
		usuario=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que esta grabando la venta.
		terminal=mFg.ExtraeStringDeBuffer(&parametros); // Terminal desde la que se está grabando la venta.
		kit=mFg.ExtraeStringDeBuffer(&parametros); // Kit que se vende.
		cantkits=StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
		mensaje=mFg.ExtraeStringDeBuffer(&parametros); // Mensaje
		almacen_global=mFg.ExtraeStringDeBuffer(&parametros); // Almacen del que saldra toda la venta
		mParamNuevoMetodo=mFg.ExtraeStringDeBuffer(&parametros); //  BANDERA PARA APLICAR CAMBIO EN EL METODO DE PAGO
		mParamNuevaClave=mFg.ExtraeStringDeBuffer(&parametros);   // NUEVA CLAVE DE METODO
		mParamNuevosDigitos=mFg.ExtraeStringDeBuffer(&parametros); //DIGITOS
		mParamNuevoUsoCfdi=mFg.ExtraeStringDeBuffer(&parametros); //USOCFDI
		//mParamPrecioMin=mFg.CadenaAFlotante(mFg.ExtraeStringDeBuffer(&parametros)); //valor de precio minimo
		/*NUEVOS PARAMETROS DE DIRECCION ENTREGA*/
		mDIRcalle=mFg.ExtraeStringDeBuffer(&parametros); // calle
		mDIRnumext=mFg.ExtraeStringDeBuffer(&parametros); //  num exterior
		mDIRnumint=mFg.ExtraeStringDeBuffer(&parametros); // num interior
		mDIRcolonia=mFg.ExtraeStringDeBuffer(&parametros); // colonia
		mDIRcp=mFg.ExtraeStringDeBuffer(&parametros); // codigo postal
		mDIRubicacionX=mFg.ExtraeStringDeBuffer(&parametros); //  ubicacionX== Latitud
		mDIRubicacionY=mFg.ExtraeStringDeBuffer(&parametros); //  ubicacionY== Altitud
		mDIRreferencia_domicilio=mFg.ExtraeStringDeBuffer(&parametros);   // referencia
		/*FIN DE PARAMETROS DIRECCION DE ENTREGA*/
		/*PARAMETROS BILLETO*/
		transaccion=mFg.ExtraeStringDeBuffer(&parametros);   // Transaccion
		/*FIN PARAMETROS BILLETO*/
		/*PARAMETROS ORIGEN DE LA VENTA*/
		origen_venta=mFg.ExtraeStringDeBuffer(&parametros); // Origen de la venta
		/*FIN PARAMETROS ORIGEN DE LA VENTA*/

		mUuidRelacionado=mFg.ExtraeStringDeBuffer(&parametros);   // muuid de venta sustituta
		mCampoFiscalRel=mFg.ExtraeStringDeBuffer(&parametros);   // folio fiscal de venta sustituta


		// Obtiene los datos de la tabla de ventas
		datos.AsignaTabla("ventas");
		parametros+=datos.InsCamposDesdeBuffer(parametros);

		// Extrae los datos que necesitamos para crear las letras y transacciones.
		valor=StrToFloat(datos.ObtieneValorCampo("valor"));
		Aux_valor=datos.ObtieneValorCampo("valor");
		periodic=datos.ObtieneValorCampo("periodic");
		anticipo=StrToFloat(datos.ObtieneValorCampo("anticipo"));
		acredito=StrToInt(datos.ObtieneValorCampo("acredito"));
		foliofisico=datos.ObtieneValorCampo("foliofisic", false);
		cliente=datos.ObtieneValorCampo("cliente");
		ticket=datos.ObtieneValorCampo("ticket");
		refCorte=datos.ObtieneValorCampo("corte");

		fecha_vta=fecha;


		//nueva consulta de un parametro de precio minimo
		BufferRespuestas* resp_preciomin=NULL;
		//se creara un bufer para obtener el valor del parametro de precio minimo y posteriormete
		//liberar la memoria ocupada de esta consulta
		try{
			instruccion.sprintf("SELECT * FROM parametrosemp WHERE parametro = 'PRECIOMINIMVTA' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_preciomin);
			if (resp_preciomin->ObtieneNumRegistros()>0){
				mParamPrecioMin=mFg.CadenaAFlotante(resp_preciomin->ObtieneDato("valor")); //valor de precio minimo
			}
		}__finally{
			if (resp_preciomin!=NULL) delete resp_preciomin;
		}



		if (tarea_venta=="M") {

			// Verifica que no tenga pagos (sin tomar en cuenta el anticipo ni los pagos no cobrables)  ---------
			instruccion.sprintf("select @error:=if(COALESCE(sum(t.valor),0)<0, 1, 0) as error from ventas v, transxcob t where v.referencia=t.referencia and v.referencia='%s' and v.cancelado=0 and t.cancelada=0 and t.valor<0 and v.acredito=1 group by v.referencia", clave_venta);
			mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 2, error);

			// Verifica que la fecha de la factura sea posterior a la fecha de cierre.
			instruccion.sprintf("select @error:=if(v.fechavta<=cast(e.valor as datetime), 1, 0) as error from ventas v left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' where v.referencia='%s'",FormServidor->ObtieneClaveSucursal(), clave_venta);
			mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 3, error);

		} else {

			// Verifica credito del cliente para autorizacion de la venta
			if(periodic!="0"){
				//verificar tambien termino de pago, si es a contado no debe marcar error
				instruccion.sprintf("select @error:=if((%s > (c.limcred-sum(if(t.aplicada=1, t.valor, 0)))) \
					and (c.excederlc = 0),1,0) as error \
					from transxcob t, ventas v, clientes c \
					where v.cliente='%s' and v.cliente=c.cliente and v.cancelado=0 and t.referencia=v.referencia and \
					t.fechaapl<='%s' and t.cancelada=0 group by v.cliente",
				Aux_valor,cliente, mFg.DateToMySqlDate(fecha_vta));
				mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 5, error);
			}
		}

		if (error==0) {

			if (periodic=="MES") throw(Exception("Todavía no se implementa la periodicidad mensual en ventas"));
			if (periodic=="QUI") throw(Exception("Todavía no se implementa la periodicidad quincenal en ventas"));
			if (periodic=="SEM") throw(Exception("Todavía no se implementa la periodicidad semanal en ventas"));
			dias_plazo=StrToInt(periodic);

			// Asigna u obtiene la fecha de venta
			if (tarea_venta=="M") {
				instruccion.sprintf("select fechavta from ventas where referencia='%s'", clave_venta);
				mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_fecha_vta);
				fecha_vta=StrToDate(resp_fecha_vta->ObtieneDato("fechavta"));
			}

			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

			instruccion.sprintf("select @seccion:=seccion, @depart:=depart, @asigfolvta:=asigfolvta, @anchofolvta:=anchofolvta from terminales where terminal='%s'", terminal);
			instrucciones[num_instrucciones++]=instruccion;

			// Obtiene el folio para la venta
			if (tarea_venta=="A") {

				if (ticket=='0') {
					calcular_foliofisico=1;
				} else {
					calcular_folioticket=1;
				}

				// Folio de sistema.
				instruccion.sprintf("select @folioaux:=valor from foliosemp where folio='VENTAS' AND sucursal = '%s' %s",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliosig=@folioaux+1");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @folioaux=cast(@folioaux as char)");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @folio=concat('%s', lpad(@folioaux,9,'0'))", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("update foliosemp set valor=@foliosig where folio='VENTAS' AND sucursal = '%s' ",FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				//quitar despues de pruebas
				instruccion.sprintf(" select @folio");
				instrucciones[num_instrucciones++]=instruccion;

			} else {

				// MODIFICACION
				instruccion.sprintf("set @folio='%s'", clave_venta);
				instrucciones[num_instrucciones++]=instruccion;

				// Casos en asignación de folios.
				// a) Sigue siendo una factura
				// b) Sigue siendo un ticket
				// c) Era un ticket que se convirtió en factura

				// c) Si es una modificación, y es un ticket que se cambio a factura entonces
				// se debe calcular el folio de las facturas
				if (tipo_original=='1' && ticket=='0') {
					calcular_foliofisico=1;
				}
			}

			if (calcular_foliofisico) {
				// Folio físico.
				instruccion.sprintf("select @foliofisicaux:=folvta, @serieaux:=serievta from terminales where terminal='%s'", terminal);
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliofisicsig=@foliofisicaux+1");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliofisic=if(@asigfolvta=0, '%s', if(@asigfolvta=1, concat(@serieaux, lpad(@foliofisicaux,@anchofolvta,'0')), '') )", foliofisico);
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("update terminales t set t.folvta=@foliofisicsig where t.terminal='%s' and  @asigfolvta=1", terminal);
				instrucciones[num_instrucciones++]=instruccion;
			}

			if (calcular_folioticket) {
				// Folio ticket
				instruccion.sprintf("select @folioticketaux:=foltickets from terminales where terminal='%s'", terminal);
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @folioticketsig=@folioticketaux+1");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @folioticket=concat('%s-', @folioticketaux)", terminal);
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("update terminales t set t.foltickets=@folioticketsig where t.terminal='%s'", terminal);
				instrucciones[num_instrucciones++]=instruccion;
			}

			// Si se está modificando entonces borra el detalle y las letras que ya existan.
			if (tarea_venta=="M") {
				instruccion.sprintf("delete from dventas where referencia=@folio");
				instrucciones[num_instrucciones++]=instruccion;
			}

			// Todo se va a una letra que se salda automáticamente en el caso de las
			// ventas de contado
			if (!acredito) {
				anticipo=valor;
				dias_plazo=0;
			}

			fecha_venc=fecha_vta+dias_plazo;
			fecha_inic=fecha_venc;
			// Graba la cabecera en la tabla "ventas"
			if (tarea_venta=="A") {
				datos.InsCampo("referencia", "@folio",1);
				if (calcular_foliofisico)
					datos.AsignaValorCampo("foliofisic", "@foliofisic",1);
				if (calcular_folioticket)
					datos.InsCampo("folioticket", "@folioticket",1);

				datos.InsCampo("usualta", usuario);
				datos.InsCampo("usumodi", usuario);
				datos.InsCampo("fechavta", mFg.DateToAnsiString(fecha));
				datos.InsCampo("horavta", mFg.TimeToAnsiString(hora));
				datos.InsCampo("fechaalta", mFg.DateToAnsiString(fecha));
				datos.InsCampo("horaalta", mFg.TimeToAnsiString(hora));
				datos.InsCampo("fechamodi", mFg.DateToAnsiString(fecha));
				datos.InsCampo("horamodi", mFg.TimeToAnsiString(hora));
				datos.InsCampo("fechainic", mFg.DateToAnsiString(fecha_inic));
				datos.InsCampo("fechavenc", mFg.DateToAnsiString(fecha_venc));
				datos.InsCampo("cancelado", "0");
				datos.InsCampo("terminal", terminal);
				datos.InsCampo("kit", kit);
				datos.InsCampo("cantkits", mFg.IntToAnsiString(cantkits));
				datos.InsCampo("plazo", mFg.IntToAnsiString(dias_plazo));
				datos.InsCampo("ubicacion","COBRANZA");
				datos.InsCampo("tipoorigen",origen_venta);
				if(almacen_global!=" ")
					datos.InsCampo("almacen",almacen_global);
				instrucciones[num_instrucciones++]=datos.GenerarSqlInsert();

				// Guarda el mensaje
				if (mensaje!=" ") { // Un espacio lo tomamos como nulo.
					instruccion.sprintf("insert into ventasmensajes (referencia, mensaje) values (@folio,'%s')",
						mensaje);
					instrucciones[num_instrucciones++]=instruccion;
				}

				if (transaccion!=" ") {
					instruccion.sprintf(" INSERT INTO billetopagos \
					(transaccion, venta, usuario, fechaalta, horaalta) \
					VALUES \
					('%s', @folio, '%s', CURDATE(), CURTIME())",
					transaccion,
					usuario);
					instrucciones[num_instrucciones++]=instruccion;
				}

			} else {
				if (calcular_folioticket)
					datos.AsignaValorCampo("foliofisic", "@foliofisic",1);
				datos.InsCampo("kit", kit);
				datos.InsCampo("cantkits", mFg.IntToAnsiString(cantkits));
				datos.InsCampo("usumodi", usuario);
				datos.InsCampo("fechamodi", mFg.DateToAnsiString(fecha) );
				datos.InsCampo("horamodi", mFg.TimeToAnsiString(hora) );
				datos.InsCampo("plazo", mFg.IntToAnsiString(dias_plazo));
				if(almacen_global!=" ")
					datos.InsCampo("almacen",almacen_global);
				instrucciones[num_instrucciones++]=datos.GenerarSqlUpdate("referencia=@folio");

				// Guarda el mensaje
				if (mensaje!=" ") { // Un espacio lo tomamos como nulo.
					instruccion.sprintf("replace into ventasmensajes (referencia, mensaje) values (@folio,'%s')",
						mensaje);
				} else {
					instruccion.sprintf("delete from ventasmensajes where referencia=@folio");
				}
				instrucciones[num_instrucciones++]=instruccion;
			}




			// Graba el cargo por la venta en transxcob
			if (acredito) { // Ahora solo guarda el cargo tipo VENT para las ventas de crédito (antes también se hacia para las de contado)
				if (tarea_venta=="A") {
					// Obtiene el folio para la NUEVA transaccion
					instruccion.sprintf("select @foliotranaux:=valor from foliosemp where folio='TRANCLI' AND sucursal = '%s' %s ",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
					instrucciones[num_instrucciones++]=instruccion;
					instruccion.sprintf("set @foliosigtran=@foliotranaux+1");
					instrucciones[num_instrucciones++]=instruccion;
					instruccion.sprintf("set @foliotranaux=cast(@foliotranaux as char)");
					instrucciones[num_instrucciones++]=instruccion;
					instruccion.sprintf("set @foliotran=concat('%s', lpad(@foliotranaux,9,'0'))", FormServidor->ObtieneClaveSucursal());
					instrucciones[num_instrucciones++]=instruccion;
					instruccion.sprintf("update foliosemp set valor=@foliosigtran where folio='TRANCLI' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
					instrucciones[num_instrucciones++]=instruccion;
					instruccion.sprintf("insert into transxcob \
						(tracredito, referencia, concepto, destino, tipo, cancelada, aplicada, valor, fechaalta, horaalta, fechamodi, horamodi, fechaapl, usualta, usumodi) \
						values (@foliotran, @folio, 'C', 'C', 'VENT', 0,1, %12.2f, '%s', '%s', '%s', '%s', '%s', '%s', '%s')",
						valor, mFg.DateToMySqlDate(fecha_vta), mFg.TimeToMySqlTime(hora), mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), mFg.DateToMySqlDate(fecha_vta), usuario, usuario );
					instrucciones[num_instrucciones++]=instruccion;
					instruccion.sprintf("insert into ventasconsaldo\
						(referencia,saldo) values (@folio, %12.2f)",valor);
					instrucciones[num_instrucciones++]=instruccion;


				} else {
					// Obtiene el folio de la transaccion ya existente
					instruccion.sprintf("select @foliotran:=tracredito from transxcob where referencia=@folio and concepto='C' and destino='C' and tipo='VENT' and cancelada=0");
					instrucciones[num_instrucciones++]=instruccion;
					instruccion.sprintf("update transxcob set valor=%12.2f, fechamodi='%s', horamodi='%s', fechaapl='%s', usumodi='%s', cancelada=0 where tracredito=@foliotran", valor, mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), mFg.DateToMySqlDate(fecha_vta), usuario);
					instrucciones[num_instrucciones++]=instruccion;
                    instruccion.sprintf("update ventasconsaldo set=%12.2f where referencia=@folio");
					instrucciones[num_instrucciones++]=instruccion;
				}
			}

			// Graba las partidas en "dventas"
			num_partidas=StrToInt(mFg.ExtraeStringDeBuffer(&parametros)); // Obtiene el número de partidas
			for (i=0; i<num_partidas; i++) {
				datos_detalle.AsignaTabla("dventas");
				parametros+=datos_detalle.InsCamposDesdeBuffer(parametros);
				datos_detalle.InsCampo("referencia", "@folio", 1);

				// Asigna el almacén para el artículo
				if(almacen_global==" ") {
					articulo=datos_detalle.ObtieneValorCampo("articulo");
					cantidad=StrToFloat(datos_detalle.ObtieneValorCampo("cantidad"));
					//instruccion.sprintf("select @almacen:=almacen from articuloxseccion where articulo='%s' and seccion=@seccion", articulo);
					instruccion.sprintf("select @almacen:=MAX(almacen) from articuloxseccion where articulo='%s' and seccion=@seccion", articulo);
					instrucciones[num_instrucciones++]=instruccion;
					datos_detalle.InsCampo("almacen", "@almacen", 1);
					instruccion = "set @almacensal:=@almacen";
					instrucciones[num_instrucciones++]=instruccion;
				} else {
					datos_detalle.InsCampo("almacen", almacen_global);
					instruccion.sprintf("set @almacensal:='%s'",almacen_global);
					instrucciones[num_instrucciones++]=instruccion;
				}

				datos_detalle.InsCampo("id", mFg.IntToAnsiString(i));

				//aun se permite guardar cantidades en cero, asi que evitar esta concurrencia
				double cantidadsprod = datos_detalle.ObtieneValorCampo("cantidad").ToDouble();
				//valida que el parámetro PAGOACTCRED este activo para validación.
				double preciodet = datos_detalle.ObtieneValorCampo("precioimp").ToDouble();
				//aqui validaremos que el precioimp sea mayor la precio minimo, si no es mayor
				//no se permitira guardar
				if ((preciodet < mParamPrecioMin) || mFg.EsCero(cantidadsprod)) {
					//se creara un bufer para obtener los datos del articulo que causa problemas y posteriormete
					//liberar la memoria ocupada de esta consulta
					int msj=0;
					 if(mFg.EsCero(cantidadsprod))
						msj = 2;
					 else
						msj = 1;
					BufferRespuestas* resp_articulo=NULL;
					try{
						instruccion.sprintf("SELECT a.articulo,pre.present,pro.producto, pro.nombre \
						FROM articulos a \
						INNER JOIN presentaciones pre ON a.producto=pre.producto AND a.present=pre.present\
						INNER JOIN productos pro ON pre.producto=pro.producto WHERE a.articulo='%s'", articulo);
						mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_articulo);
						if (resp_articulo->ObtieneNumRegistros()>0){
							nombre= resp_articulo->ObtieneDato("nombre");
							present= resp_articulo->ObtieneDato("present");
							prod=  resp_articulo->ObtieneDato("producto");
						}
					}__finally{
						if (resp_articulo!=NULL) delete resp_articulo;
					}
					//manejar el mensaje de error dependiendo de precio minimo o cantidad 0
					if(msj == 2)
					   throw(Exception("La cantidad del producto: \n"+nombre+" "+present+"\n es cero. Favor de cambiar la cantidad."));
					else
					   throw(Exception("El precio del producto: \n"+nombre+" "+present+"\n es menor del precio mínimo permitido("+FloatToStr(mParamPrecioMin)+")"));
				}


				BufferRespuestas* resp_precios=NULL;
				try{
					instruccion.sprintf("SELECT p.tipoprec, t.descripcion, p.precio AS precio    \
									FROM tiposdeprecios t   \
									INNER JOIN precios p    \
									INNER JOIN articulos a ON p.articulo=a.articulo      \
							   WHERE p.tipoprec=t.tipoprec AND p.articulo='%s' AND t.idempresa=%s     \
							   AND (t.listamovil + t.verventmayoreo + t.verprecdif) > 0 \
									ORDER BY p.tipoprec", articulo, FormServidor->ObtieneClaveEmpresa());
					mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_precios);
					if (resp_precios->ObtieneNumRegistros()>0){

						for(int p = 0; p < resp_precios->ObtieneNumRegistros();p++){

							if(p == 0){
								tipoPrecio = resp_precios->ObtieneDato("tipoprec");
								diferenciaPrecioAnt = 0.00;
								diferenciacompant = 0.00;
							}

							precioValida = resp_precios->ObtieneDato("precio").ToDouble();
							diferenciaPrecioAct = abs(precioValida - preciodet );

							if(p == 0 || diferenciaPrecioAct < diferenciaPrecioAnt) {
								tipoPrecio =  resp_precios->ObtieneDato("tipoprec");
								diferenciaPrecioAnt = diferenciaPrecioAct;
							}

							resp_precios->IrAlSiguienteRegistro();
						}

					}
					else{
						BufferRespuestas* resp_tipoprecios=NULL;
						try {
							instruccion.sprintf("SELECT tipoprec \
													FROM tiposdeprecios t   \
													WHERE t.tipoprec=99 AND t.idempresa=%s"
												, FormServidor->ObtieneClaveEmpresa());
							mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_tipoprecios);
							if (resp_tipoprecios->ObtieneNumRegistros()>0){
								tipoPrecio = "99";
							}else{
								throw(Exception("El articulo: \n"+articulo+" "+present+"\n no cuenta con un tipo de precio.\n Consultar con compras"));
							}

						}
						__finally {
							if (resp_tipoprecios!=NULL) delete resp_tipoprecios;
						}

					}
				}
				__finally {
					if (resp_precios!=NULL) delete resp_precios;
				}

				datos_detalle.InsCampo("tipoprec", tipoPrecio);
				// Inserta en dventa
				instrucciones[num_instrucciones++]=datos_detalle.GenerarSqlInsert();

				total_valida+=datos_detalle.ObtieneValorCampo("precioimp").ToDouble()
							  * datos_detalle.ObtieneValorCampo("cantidad").ToDouble();
				//Actualiza existencias
				articuloact = datos_detalle.ObtieneValorCampo("articulo");
				instruccion.sprintf("UPDATE articulos a INNER JOIN existenciasactuales ea \
				ON a.producto = ea.producto AND a.present = ea.present \
				AND ea.almacen = @almacensal INNER JOIN  \
				dventas d ON d.referencia = @folio AND a.articulo = d.articulo \
				SET ea.cantidad = (ea.cantidad - (d.cantidad * a.factor)) \
				, ea.ventas = (ea.ventas +  (d.cantidad * a.factor)) \
				WHERE a.articulo = '%s' AND d.referencia = @folio", articuloact);
				instrucciones[num_instrucciones++]= instruccion;
			}
			/*
			  -----------------------------
			* despues de ingresado cabecera de ventas y detalle de la venta
			* DIRECCION DE ENTREGA
			  -----------------------------
			*/
			if (tarea_venta=="A") {

				// en caso de que estos campo esten vacios(calle,numExt, colonia, cp) no guarde el regristro en
				// la tabla de ventadirent

				//if(!mDIRcalle.Trim().IsEmpty() || !mDIRnumext.Trim().IsEmpty() || !mDIRcolonia.Trim().IsEmpty() ){
				instruccion.sprintf("set @ubicacionent:=POINT(%s, %s) ", mDIRubicacionX,mDIRubicacionY);
				instrucciones[num_instrucciones++] = instruccion;


				instruccion.sprintf(" insert into ventadirent \
						(id_ventaent,referencia, tipo, cliente,calle ,numext ,numint , colonia,cp ,ubicaciongis ,referenciadom,fechaalta,fechamodi) \
						values \
						(NULL,@folio, 'Venta','%s','%s','%s','%s','%s','%s', @ubicacionent,'%s', CURDATE(),CURDATE())",
				cliente.c_str(),mDIRcalle.c_str(),mDIRnumext.c_str(),
				mDIRnumint.c_str(),mDIRcolonia.c_str() ,mDIRcp.c_str(),
				mDIRreferencia_domicilio.c_str());
				instrucciones[num_instrucciones++] = instruccion;
				//}

				if(mPedidoAFacturar!="0")  // // En caso de obtener el pedido actualizamos tabla de "pedidosdirent"
				{
					instruccion.sprintf(" update pedidosdirent set \
					calle='%s', numext='%s', numint='%s', colonia='%s', \
					cp='%s' ,ubicaciongis=@ubicacionent ,referenciadom='%s', fechamodi=CURDATE() \
					where  referencia='%s' AND cliente='%s' ",
					mDIRcalle.c_str(),mDIRnumext.c_str(),mDIRnumint.c_str(),mDIRcolonia.c_str() ,
					mDIRcp.c_str(),mDIRreferencia_domicilio.c_str(),
					mPedidoAFacturar,cliente.c_str());
					instrucciones[num_instrucciones++] = instruccion;
				}

			}

			/*
			  * fin de direccion de entrega
			  -----------------------------------
			*/

			diferencia = abs(total_valida - valor);

			if(diferencia > 0.80 ){
				throw (Exception("La diferencia entre el total y el detalle de los productos excede la diferencia permitida."));
			}

			int mes_modif;
			// Modifica el precalculo de ventas mensuales
			if (tarea_venta=="A") {

				if (mFg.IntToAnsiString(YearOf(fecha_vta))==mFg.IntToAnsiString(YearOf(fecha)))
					mes_modif=MonthOf(fecha_vta)+24;
				else
					mes_modif=MonthOf(fecha_vta)+12;

				instruccion.sprintf( \
					"UPDATE ventasxmes vm \
							INNER JOIN ( \
								SELECT dv.almacen, a.producto, a.present, SUM(dv.cantidad * a.factor) AS cantidad \
								FROM dventas dv INNER JOIN articulos a ON a.articulo=dv.articulo \
								WHERE dv.referencia = @folio GROUP BY dv.almacen, a.producto, a.present \
							) vent ON vm.almacen=vent.almacen AND vm.producto=vent.producto AND vm.present=vent.present \
						SET vm.cant%s = vm.cant%s + vent.cantidad, vm.ventas30 = vm.ventas30 + vent.cantidad, \
						vm.ventas60 = vm.ventas60 + vent.cantidad, vm.ventas90 = vm.ventas90 + vent.cantidad, \
						vm.ventas180 = vm.ventas180 + vent.cantidad, vm.ventascorte = vm.ventascorte + vent.cantidad ",
					mFg.IntToAnsiString(mes_modif), mFg.IntToAnsiString(mes_modif));
				instrucciones[num_instrucciones++]=instruccion;
			}


			tipoft = "PCAN";
			facturar=mFg.ExtraeStringDeBuffer(&parametros);  //mFacturarTicket
			cancelar=mFg.ExtraeStringDeBuffer(&parametros); // mCancelarTicket
			refventa=mFg.ExtraeStringDeBuffer(&parametros); // Referencia de venta
			cancelar_ticket=mFg.ExtraeStringDeBuffer(&parametros); //Cancelar ticket
			crear_notacred=mFg.ExtraeStringDeBuffer(&parametros); //Crear nota de credito
			formas_pago=mFg.ExtraeStringDeBuffer(&parametros); // Formas de pago
			cantidades_pagada=mFg.ExtraeStringDeBuffer(&parametros); // Cantidades pagadas
			porcentajes=mFg.ExtraeStringDeBuffer(&parametros); // Porcentajes de pagos
			IDTrnPinpad=mFg.ExtraeStringDeBuffer(&parametros);

			tipoPinpad = mFg.ExtraeStringDeBuffer(&parametros);

			if (tarea_venta=="A") {
				TStringDynArray fpagos(SplitString(formas_pago, ","));
				TStringDynArray cantidades(SplitString(cantidades_pagada, ","));
				TStringDynArray porcentaje(SplitString(porcentajes, ","));
				TStringDynArray ArrayIdTrn(SplitString(IDTrnPinpad, ","));
				double totalVenta = 0;

				for (int f = 0; f < fpagos.Length; f++) {

					if (ArrayIdTrn[f]!="0"){
						if(tipoPinpad == "1"){
							instruccion.sprintf("INSERT INTO dventasfpago (referencia, formapag, valor, porcentaje, referencia_fin) \
							 VALUES (@folio,'%s','%s','%s','%s')", AnsiString(fpagos[f]), AnsiString(cantidades[f]), AnsiString(porcentaje[f]), AnsiString(ArrayIdTrn[f]));
							instrucciones[num_instrucciones++]=instruccion;
							totalVenta += StrToFloat(cantidades[f]);
						}else{
                            instruccion.sprintf("INSERT INTO dventasfpago (referencia, formapag, valor, porcentaje, trn_id) \
							 VALUES (@folio,'%s','%s','%s','%s')", AnsiString(fpagos[f]), AnsiString(cantidades[f]), AnsiString(porcentaje[f]), AnsiString(ArrayIdTrn[f]));
							instrucciones[num_instrucciones++]=instruccion;
							totalVenta += StrToFloat(cantidades[f]);
						}
					}else{
						instruccion.sprintf("INSERT INTO dventasfpago (referencia, formapag, valor, porcentaje) \
							 VALUES (@folio,'%s','%s','%s')", AnsiString(fpagos[f]), AnsiString(cantidades[f]), AnsiString(porcentaje[f]));
						instrucciones[num_instrucciones++]=instruccion;
						totalVenta += StrToFloat(cantidades[f]);

						if(fpagos[f]=="FTARCR" || fpagos[f]=="FTARDE"){
							instruccion.sprintf("INSERT INTO bitacoraventasxtpv \
								(referencia, formapag, valor, porcentaje, usuario, terminal, fecha, hora) \
								 VALUES \
								 (@folio, '%s', '%s', '%s', '%s', '%s', CURDATE(), CURTIME())",
							 AnsiString(fpagos[f]), AnsiString(cantidades[f]), AnsiString(porcentaje[f]), usuario, terminal);
							instrucciones[num_instrucciones++]=instruccion;
						}
					}

					
				}

				if (!mFg.SonIguales(totalVenta, valor, 0.1)) {
					throw(Exception("Falló la validación de suma de formas de pago. " + FloatToStr(totalVenta) + " : " + FloatToStr(valor)));
				}
			}


			if(facturar == "SI")
			{
				instruccion.sprintf("INSERT INTO bitacoraFactTick (referencia, tipo, referenciafact, fecha, hora) VALUES ('%s','TICK',@folio,DATE(NOW()),TIME(NOW()));",clave_venta );
				instrucciones[num_instrucciones++]=instruccion;
			}

			if (cancelar_ticket == "1" && crear_notacred == "1") {
				throw (Exception("No se puede cancelar ticket y crear nota de credito al mismo tiempo"));
			}
			/*****************************************************************************/
			//******************************CANCELAR TICKET****************************///
			/*****************************************************************************/
			if (cancelar_ticket == "1" ) {
				int idCancel = 1;
				BufferRespuestas* resp=NULL;
				try{
					instruccion.sprintf("SELECT cancelado FROM ventas  where referencia='%s'",clave_venta);
					mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp);
					if (resp->ObtieneNumRegistros()>0)
						cancelado =  resp->ObtieneDato("cancelado");

						else
							cancelado =  "0";
				}
				__finally {
					if (resp!=NULL) delete resp;
				}

				if (cancelado == "1") {
					throw (Exception("No se pueden facturar tickets que ya están cancelados"));
				}

				instruccion.sprintf("INSERT INTO bitacoraFactTick (referencia, tipo, referenciafact, fecha, hora) VALUES ('%s','PCAN',@folio,DATE(NOW()),TIME(NOW()))",clave_venta);
				instrucciones[num_instrucciones++]=instruccion;

				// Quita de la cola de impresion
				instruccion.sprintf("delete from colaimpresion where foliodoc='%s' and tipo=0", clave_venta);
				instrucciones[num_instrucciones++]=instruccion;

				// Quita de la cola de impresion de surtidos
				instruccion.sprintf("delete from colaimpsurtido where referencia='%s'", clave_venta);
				instrucciones[num_instrucciones++]=instruccion;
				// Cancela la venta
				instruccion.sprintf("update ventas set cancelado=1, fechamodi='%s', horamodi='%s', usumodi='%s', cortecancel='%s', terminalcancel='%s' where referencia='%s' and cancelado=0", mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario, refCorte, terminal, clave_venta);
				instrucciones[num_instrucciones++]=instruccion;

				if (facturar=="SI") {
					if (formas_pago=="FCREDI") {
						// Cancela el cargo hecho por el total de la venta
						instruccion.sprintf("INSERT INTO ventascanceladas (idcancel, referencia, mensaje) VALUES (6, '%s', 'FACTURADA A CREDITO')", clave_venta);
						instrucciones[num_instrucciones++]=instruccion;
					} else {
						// Cancela el cargo hecho por el total de la venta
						instruccion.sprintf("INSERT INTO ventascanceladas (idcancel, referencia, mensaje) VALUES (1, '%s', 'FACTURADA')", clave_venta);
						instrucciones[num_instrucciones++]=instruccion;
					}
				}

				// Crea tabla temporal para almacenar cantidades de articulos antes de cancelar
				instruccion="CREATE temporary TABLE tmpcantidad(referencia VARCHAR(11) NOT NULL,\
				producto VARCHAR(8) NOT NULL, present VARCHAR(255) NOT NULL, almacen VARCHAR(4) NOT NULL,  \
				cantidad DECIMAL(12,3) NOT NULL, PRIMARY KEY (referencia, producto, present, almacen))";
				instrucciones[num_instrucciones++]=instruccion;

				instruccion.sprintf("INSERT INTO tmpcantidad SELECT d.referencia, a.producto, a.present, d.almacen,  \
				SUM(d.cantidad * a.factor) AS cantidad FROM dventas d INNER JOIN \
				articulos a ON a.articulo = d.articulo WHERE d.referencia = '%s' \
				GROUP BY a.producto, a.present, d.almacen", clave_venta);
				instrucciones[num_instrucciones++]=instruccion;

				instruccion = "UPDATE tmpcantidad tmp INNER JOIN existenciasactuales ea ON ea.producto = tmp.producto \
				AND ea.present = tmp.present AND ea.almacen = tmp.almacen SET ea.cantidad = (ea.cantidad  + tmp.cantidad) \
				, ea.ventas = (ea.ventas - tmp.cantidad)";
				instrucciones[num_instrucciones++]=instruccion;

				instruccion.sprintf("select @fechacorte:=ifnull(max(p.fecha),'1900-01-01') as fcorte \
					 from puntoscorte p where p.fecha<='%s'", mFg.DateToMySqlDate(fecha));
				instrucciones[num_instrucciones++]=instruccion;

				// Modifica el precalcuol de ventas mensuales
				int mes_modif_canc;
				TDate fecha_vta_canc;
				instruccion.sprintf("select @fechavtacanc:=fechavta AS asigfvtacanc, fechavta from ventas where referencia='%s' ", clave_venta);
				mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_fecha_vta_canc);
				fecha_vta_canc=StrToDate(resp_fecha_vta_canc->ObtieneDato("fechavta"));

				if (YearOf(fecha_vta_canc)>=YearOf(fecha)-1) {
					// Modifica el precalculo de ventas mensuales
					//Las cancelaciones de ticket siempre son de la misma fecha
					if (mFg.IntToAnsiString(YearOf(fecha_vta_canc))==mFg.IntToAnsiString(YearOf(fecha)))
						mes_modif_canc=MonthOf(fecha_vta_canc)+24;
					else
						mes_modif_canc=MonthOf(fecha_vta_canc)+12;

					instruccion.sprintf( \
						"UPDATE ventasxmes vm \
								INNER JOIN ( \
									SELECT dv.almacen, a.producto, a.present, SUM(dv.cantidad * a.factor) AS cantidad \
									FROM dventas dv INNER JOIN articulos a ON a.articulo=dv.articulo \
									WHERE dv.referencia = '%s' GROUP BY dv.almacen, a.producto, a.present \
								) vent ON vm.almacen=vent.almacen AND vm.producto=vent.producto AND vm.present=vent.present \
							SET vm.cant%s = vm.cant%s - vent.cantidad, \
								vm.ventas30 = vm.ventas30 - vent.cantidad, \
								vm.ventas60 = vm.ventas60 - vent.cantidad, \
								vm.ventas90 = vm.ventas90 - vent.cantidad, \
								vm.ventas180 = vm.ventas180 - vent.cantidad, \
								vm.ventascorte = IF(@fechavtacanc>@fechacorte AND @fechavtacanc<=CURDATE(), vm.ventascorte - vent.cantidad, vm.ventascorte) ",
						clave_venta, mFg.IntToAnsiString(mes_modif_canc), mFg.IntToAnsiString(mes_modif_canc));
					instrucciones[num_instrucciones++]=instruccion;
				}

			}
			/*****************************************************************************/
			//******************************FIN CANCELAR TICKET****************************///
			/*****************************************************************************/

			//--------------------------------
			//Ventas de Kits
			//--------------------------------
            mCantidadKits=StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
			// Si se está modificando entonces borra los kits asosiados.
			if (tarea_venta=="M" && mCantidadKits>0) {
				instruccion.sprintf("delete from ventaskits where venta=@folio");
				instrucciones[num_instrucciones++]=instruccion;
			}
			for (int i = 0; i < mCantidadKits; i++) {
				datos_detalle.AsignaTabla("ventaskits");
				parametros+=datos_detalle.InsCamposDesdeBuffer(parametros);
				datos_detalle.InsCampo("venta", "@folio",1);
				instrucciones[num_instrucciones++]=datos_detalle.GenerarSqlInsert();
			}
				//Detalle de Ventas de Kits
			//---------------------------------
			mCantArtKits=StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
			// Si se está modificando entonces borra los articulos de kits asosiados.
			if (tarea_venta=="M" && mCantArtKits>0) {
				instruccion.sprintf("delete from dventaskits where venta=@folio");
				instrucciones[num_instrucciones++]=instruccion;
			}
			for (int i = 0; i <mCantArtKits; i++) {
				datos_detalle.AsignaTabla("dventaskits");
				parametros+=datos_detalle.InsCamposDesdeBuffer(parametros);
				datos_detalle.InsCampo("venta", "@folio",1);
				instrucciones[num_instrucciones++]=datos_detalle.GenerarSqlInsert();
			}
			//----------------------------------
				//Fin de Ventas y detalle de ventas de Kits
			//----------------------------------

			//Asocia el pedido a la venta
			if(mPedidoAFacturar!="0" )
			{
				instruccion.sprintf("update pedidosventa set venta=@folio, facturado=1 where referencia='%s'",mPedidoAFacturar);
				instrucciones[num_instrucciones++]=instruccion;
			}

		}

		if (crear_notacred == "1" ) {
			BufferRespuestas* resp_ref=NULL;
			try{
				instruccion.sprintf("  SELECT nc.referencia AS refnotcred, nc.cancelado AS cancelado \
				FROM ventas v INNER JOIN terminales t INNER JOIN empleados ec INNER JOIN empleados ev \
				INNER JOIN clientes c   LEFT JOIN cfd ON v.referencia=cfd.referencia AND \
				cfd.tipocomprobante='VENT' LEFT JOIN notascredcli AS nc ON  v.referencia = nc.venta  \
				LEFT JOIN clientesemp cemp ON c.cliente=cemp.cliente \
				WHERE v.referencia='%s' AND v.cliente=c.cliente AND v.terminal=t.terminal AND \
				v.usumodi=ec.empleado AND cemp.vendedor=ev.empleado AND nc.cancelado = '0' AND cemp.idempresa=%s ",clave_venta,FormServidor->ObtieneClaveEmpresa());
				mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_ref);
				if (resp_ref->ObtieneNumRegistros()>0){
					 refnc =  resp_ref->ObtieneDato("refnotcred");
				}

			}
			__finally {
				if (resp_ref!=NULL) delete resp_ref;
			}

			if (refnc != "") {
				throw (Exception("No se pueden facturar tickets que tienen asignada una nota de crédito"));
			}
		}


		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			if (error==0) {

				// Crea el CFD (si la configuracion así lo indica) y hace commit
				ComprobanteFiscalDigital *cfd=NULL;
				AnsiString errorB = "";
				AnsiString  nuevoCampFiscal="";
				try {
						cfd=new ComprobanteFiscalDigital(mServidorVioleta);
						cfd->AsignaValores(mParamNuevoMetodo, mParamNuevaClave, mParamNuevosDigitos.c_str(), mParamNuevoUsoCfdi.c_str(), mUuidRelacionado.c_str(), mCampoFiscalRel);


						if (cfd->EmitirCFDI40(Respuesta, MySQL, "VENT")) {
							cfd->CreaCFDIVenta40(Respuesta, MySQL, mUuidRelacionado);
						} else {
							cfd->CreaCFDIVenta33(Respuesta, MySQL, mUuidRelacionado);
						}

						if (crear_notacred == "1" ) {
								char *buffer_nota_cred=new char[1024*64*10];
								char *aux_buffer_nota_cred=buffer_nota_cred;
							try{
								try{

									aux_buffer_nota_cred=mFg.AgregaStringABuffer("", aux_buffer_nota_cred);
									aux_buffer_nota_cred=mFg.AgregaStringABuffer("", aux_buffer_nota_cred);
									aux_buffer_nota_cred=mFg.AgregaStringABuffer("A", aux_buffer_nota_cred);
									aux_buffer_nota_cred=mFg.AgregaStringABuffer(clave_venta, aux_buffer_nota_cred);
									aux_buffer_nota_cred=mFg.AgregaStringABuffer(usuario, aux_buffer_nota_cred);
									aux_buffer_nota_cred=mFg.AgregaStringABuffer(terminal, aux_buffer_nota_cred);

									//Parametros para el CFD
									aux_buffer_nota_cred=mFg.AgregaStringABuffer(mParamNuevoMetodo, aux_buffer_nota_cred);
									aux_buffer_nota_cred=mFg.AgregaStringABuffer(mParamNuevaClave, aux_buffer_nota_cred);
									aux_buffer_nota_cred=mFg.AgregaStringABuffer(mParamNuevosDigitos.c_str(), aux_buffer_nota_cred);
									aux_buffer_nota_cred=mFg.AgregaStringABuffer(refCorte, aux_buffer_nota_cred);

									aux_buffer_nota_cred=mFg.AgregaStringABuffer(num_partidas, aux_buffer_nota_cred);

									GrabaDevolCli(Respuesta, MySQL, buffer_nota_cred);


									instruccion.sprintf("INSERT INTO bitacoraFactTick (referencia, tipo, referenciafact, fecha, hora) VALUES ('%s','NOTA',@folio,DATE(NOW()),TIME(NOW()))",
									clave_venta);
									if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
										throw (Exception("Error en query EjecutaSelectSqlNulo"));

									instruccion="COMMIT";
									if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
										throw Exception("No se pudo hacer commit");
								} catch(Exception &e) {
										if (e.Message=="1" || e.Message=="2" || e.Message=="3" || e.Message=="4")
												errorB = "1" + e.Message;
												else{
													errorB = "20";
													errorMensaje= e.Message;
												}
								}
							}
							__finally {
								delete buffer_nota_cred;
							}

						}



				} __finally {
					if(cfd!=NULL) delete cfd;
				}

				if(errorB != "")
					error = StrToInt(errorB);

				instruccion.sprintf("select %d as error, '%s' as errorMensaje ,@folionc as folionotacred, v.referencia as folio, v.foliofisic, v.folioticket, v.fechavta, v.horavta from ventas v where v.referencia=@folio", error, errorMensaje);
			} else {
				instruccion.sprintf("select %d as error, '%s' as errorMensaje, '' as folionotacred", error, errorMensaje);
			}
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	} __finally {
		if (resp_fecha_vta!=NULL) delete resp_fecha_vta;
		if (resp_fecha_vta_canc!=NULL) delete resp_fecha_vta_canc;
		delete buffer_sql;
		delete[] instrucciones;
	}
}


//----------------------------------------------------------------------------
//ID_CANC_VENTA
void ServidorVentas::CancelaVenta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  CANCELA VENTA
	char *buffer_sql=new char[1024*32];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instrucciones[50], instruccion;
	int num_instrucciones=0;
	AnsiString clave, usuario, cancela, pac;
	AnsiString pagoactcred, ventacfdi33 = "";
	int error=0;
	int i;
	TDate fecha=Today();
	TTime hora=Time();
	BufferRespuestas* resp_fecha_vta=NULL;

	AnsiString pagoConBilleto;
	AnsiString corteActual;
	AnsiString cancelado, razonCanc, terminalCanc;

	AnsiString motivoCancCFDI, UUIDActivo;


	try{
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave de la venta (referencia)
		usuario=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que esta cancelando la venta.

		cancela=mFg.ExtraeStringDeBuffer(&parametros); // PASAS "0" siempre, aunque ya no se usa

		pac=mFg.ExtraeStringDeBuffer(&parametros); //este servira para FORZAR CANCELACION
		pagoConBilleto=mFg.ExtraeStringDeBuffer(&parametros); //Modificar la transación
		corteActual=mFg.ExtraeStringDeBuffer(&parametros); //CorteActual
		cancelado=mFg.ExtraeStringDeBuffer(&parametros); //cancelado
		razonCanc=mFg.ExtraeStringDeBuffer(&parametros); //razon cancelado
		terminalCanc=mFg.ExtraeStringDeBuffer(&parametros); //terminal donde se cancelo
		motivoCancCFDI=mFg.ExtraeStringDeBuffer(&parametros); // motivo de la cancelacion del CFDI
		UUIDActivo=mFg.ExtraeStringDeBuffer(&parametros); // UUID de la cancelacion del CFDI

		//valida que el parámetro PAGOACTCRED este activo para validación.
		BufferRespuestas* resp_pagoact=NULL;
		try{
			instruccion.sprintf("SELECT valor FROM parametrosemp WHERE parametro = 'PAGOACTCRED' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_pagoact);
			if (resp_pagoact->ObtieneNumRegistros()>0)
				pagoactcred =  resp_pagoact->ObtieneDato("valor");
		}
		__finally {
			if (resp_pagoact!=NULL) delete resp_pagoact;
		}

		BufferRespuestas* resp_ventacfdi33=NULL;
		try{
			instruccion.sprintf("SELECT referencia FROM cfd WHERE referencia = '%s' AND (version = '3.3' OR version = '4.0')",clave);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_ventacfdi33);
			if (resp_ventacfdi33->ObtieneNumRegistros()>0)
				ventacfdi33 =  resp_ventacfdi33->ObtieneDato("referencia");
		}
		__finally {
			if (resp_ventacfdi33!=NULL) delete resp_ventacfdi33;
		}

		// Verifica que no tenga pagos (sin tomar en cuenta el anticipo ni los cheques no cobrables)  ---------
		instruccion.sprintf("select @error:=if(COALESCE(sum(t.valor),0)<0, 1, 0) as error from ventas v, transxcob t where v.referencia=t.referencia and v.referencia='%s' and v.cancelado=0 and t.cancelada=0 and t.valor<0 group by v.referencia", clave);
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 2, error);

		// Verifica que la fecha de la factura sea posterior a la fecha de cierre.
		instruccion.sprintf("select @error:=if(v.fechavta<=cast(e.valor as datetime), 1, 0) as error from ventas v left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' where v.referencia='%s'",FormServidor->ObtieneClaveSucursal(), clave);
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 3, error);

		if(pagoactcred == 1 && ventacfdi33 != "" && pac!="1"){
			AnsiString consulta;
			consulta.sprintf("CREATE TEMPORARY TABLE pagosref SELECT t.pago FROM ventas v INNER JOIN transxcob t \
					ON v.referencia=t.referencia WHERE v.referencia='%s' AND v.cancelado=0                       \
					AND v.acredito=1 AND t.tipo='PAGO' ", clave);
			mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, consulta.c_str());

			// Verifica que no se tengan pagos a la factura para poder cancelar.
			instruccion.sprintf("SELECT @error:=IF(COALESCE(SUM(t.valor),0)<0 AND c.muuid IS NOT NULL, 1, 0) AS error \
				FROM ventas v INNER JOIN transxcob t ON v.referencia=t.referencia 		\
				LEFT JOIN cfd c ON c.referencia = t.pago AND c.tipocomprobante = 'PAGO' AND c.muuid IS NOT NULL        \
				INNER JOIN pagosref pr ON c.referencia = pr.pago WHERE v.referencia='%s'    \
				AND v.cancelado=0 AND v.acredito=1 AND t.tipo='PAGO'", clave);
		  /*	instruccion.sprintf("SELECT @error:=IF(COALESCE(SUM(t.valor),0)<0 AND c.muuid IS NOT NULL, 1, 0) AS error \
					FROM ventas v INNER JOIN transxcob t ON v.referencia=t.referencia LEFT JOIN cfd c ON c.referencia = t.pago       \
					WHERE v.referencia='%s' AND v.cancelado=0 AND v.acredito=1 AND t.tipo='PAGO'  GROUP BY v.referencia", clave); */
			mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 4, error);
		}

        //Verifica que si la compra no se encuentra cancelada
	   instruccion.sprintf("SELECT @error:=if(v.cancelado=1, 1, 0) AS error FROM ventas v WHERE v.referencia='%s';", clave);
	   mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 8, error);

		//validacion de productos inactivos
		AnsiString produc2,present2,multiplo2,factor2,select_inactivos,nombre;
		BufferRespuestas* resp_inactivos=NULL;
		try{
			select_inactivos.sprintf("SELECT  @error:=IF(a.activo=0, 1, 0) AS error,\
						a.producto, a.present, a.multiplo, a.factor,pro.nombre FROM ventas v \
						LEFT JOIN dventas dv ON dv.referencia=v.referencia \
						LEFT JOIN articulos a ON a.articulo=dv.articulo \
						LEFT JOIN productos pro ON pro.producto=a.producto \
						WHERE v.referencia='%s' AND a.activo=0", clave);
			if(mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, select_inactivos.c_str(), resp_inactivos)){
				if (resp_inactivos->ObtieneNumRegistros()>0){
					produc2=resp_inactivos->ObtieneDato("producto");
					present2=resp_inactivos->ObtieneDato("present");
					multiplo2=resp_inactivos->ObtieneDato("multiplo");
					factor2=resp_inactivos->ObtieneDato("factor");
					nombre = resp_inactivos->ObtieneDato("nombre");
					error=6;
					throw (Exception("Hay artículos inactivos en el detalle de venta a cancelar.\n Para poder cancelar debe activar el artículo.\n  Artículo:"+nombre+" "+present2+" "+multiplo2));
				}
			}else{
				throw (Exception("Error al consultar los artículos inactivos en cancelar ventas"));
			}
	   /*	,a.producto, a.present, a.multiplo, a.factor   */
	   }__finally{
			if (resp_inactivos!=NULL) delete resp_inactivos;
	   }
	   // Validacion de notas de cargo
	   AnsiString valor_nota, select_notascargo;
	   BufferRespuestas* resp_notascargo=NULL;
		try{
			select_notascargo.sprintf("SELECT vent.referencia AS referencia_venta, \
										tsxc.tracredito AS referencia_transcredito, \
										ncc.referencia AS referencia_notacargo \
										FROM ventas vent \
										INNER JOIN transxcob tsxc ON vent.referencia = tsxc.referencia \
										INNER JOIN notascarcli ncc ON tsxc.notacar = ncc.referencia \
										WHERE vent.referencia = '%s' \
										AND tsxc.cancelada = 0 \
										AND ncc.cancelado = 0 ", clave);
			if(mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, select_notascargo.c_str(), resp_notascargo)){
				if (resp_notascargo->ObtieneNumRegistros()>0){
					//produc2=resp_inactivos->ObtieneDato("producto");
					error=7;
					throw (Exception("Existen notas de cargo activas en esta venta.\nPara poder cancelar debe eliminar las notas de cargo."));
				}
			}else{
				throw (Exception("Error al consultar las notas de credito en cancelar ventas"));
			}
	   }__finally{
			if (resp_notascargo!=NULL) delete resp_notascargo;
	   }

	   // Validacion de notas de credito clientes
	   AnsiString select_notascredcli;
	   BufferRespuestas* resp_notascredcli=NULL;
		try{
			select_notascredcli.sprintf("SELECT v.referencia,nc.referencia\
				FROM ventas v\
				INNER JOIN notascredcli nc ON nc.venta=v.referencia\
				WHERE v.referencia = '%s'\
				AND nc.cancelado = 0", clave);
			if(mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, select_notascredcli.c_str(), resp_notascredcli)){
				if (resp_notascredcli->ObtieneNumRegistros()>0){
					error=7;
					throw (Exception("Existen notas de crédito activas en esta venta.\nPara poder cancelar debe eliminar las notas de crédito."));
				}
			}else{
				throw (Exception("Error al consultar las notas de credito en cancelar ventas"));
			}
	   }__finally{
			if (resp_notascredcli!=NULL) delete resp_notascargo;
	   }

		if (error==0) {
			if(pac=="0"){
			//con el formulario de forzar la cancelación, esta variable de PAC, tendra el valor==1
			//lo que significa que se ha cancelado directamente en el portal del SAT, solo
			//hace falta cancelarlo a nivel sistema.
			//por ello, se saltara llas siguentes lineas, porque ya ha sido cancelado
				// Cancela el CFDI con el PAC (si no se trata de un CFDI simplemente no se hace nada con ningún PAC)

				// SOLO CUANDO NO ESTA EN DEPURACION
				#ifndef _DEBUG
				ComprobanteFiscalDigital *cfd=NULL;
				try {
					cfd=new ComprobanteFiscalDigital(mServidorVioleta);
					cfd->cancelarCFDI(Respuesta, MySQL, "VENT",clave,"",motivoCancCFDI,UUIDActivo);
				} __finally {
					if(cfd!=NULL) delete cfd;
				}
 				#endif
			}

			// Si se canceló correctamente con el PAQ (o sí no fue necesario por ser CFD) entonces lo cancela en la base de datos.
			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

			// Quita de la cola de impresion
			instruccion.sprintf("delete from colaimpresion where foliodoc='%s' and tipo=0", clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Quita de la cola de impresion de surtidos
			instruccion.sprintf("delete from colaimpsurtido where referencia='%s'", clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Cancela la venta
			instruccion.sprintf("update ventas set cancelado=1, fechamodi='%s', horamodi='%s', usumodi='%s', cortecancel='%s', terminalcancel='%s' where referencia='%s' and cancelado=0 ", mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario, corteActual, terminalCanc, clave);
			instrucciones[num_instrucciones++]=instruccion;

			if (motivoCancCFDI!="01") {
				// Cancela el CFD
				instruccion.sprintf("update cfd set estado=0, fechacancelamin=curdate(), fechacancela='%s %s', motivocanc='%s' where referencia='%s' and tipocomprobante='VENT'", mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), motivoCancCFDI, clave);
				instrucciones[num_instrucciones++]=instruccion;
			} else {
				// Cancela el CFD
				instruccion.sprintf("update cfd set estado=0, fechacancelamin=curdate(), fechacancela='%s %s', motivocanc='%s', uuidrelcanc='%s' where referencia='%s' and tipocomprobante='VENT'", mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), motivoCancCFDI, UUIDActivo, clave);
				instrucciones[num_instrucciones++]=instruccion;
            }

			// Cancela el cargo hecho por el total de la venta
			instruccion.sprintf("update transxcob set cancelada=1, fechamodi='%s', horamodi='%s', usumodi='%s' where referencia='%s' and concepto='C' and destino='C' and tipo='VENT' and cancelada=0", mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario, clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Se elimina la venta en ventas con saldo
			instruccion.sprintf("update ventasconsaldo set saldo = 0 where referencia='%s'",clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Cancela el cargo hecho por el total de la venta
			instruccion.sprintf("INSERT INTO ventascanceladas (idcancel, referencia, mensaje) VALUES (%s, '%s', '%s')", cancelado, clave, razonCanc);
			instrucciones[num_instrucciones++]=instruccion;

			//Se calcula la última fecha de corte
			instruccion.sprintf("select @fechacorte:=ifnull(max(p.fecha),'1900-01-01') as fcorte \
				 from puntoscorte p where p.fecha<='%s'", mFg.DateToMySqlDate(fecha));
			instrucciones[num_instrucciones++]=instruccion;

			//Se calcula la última fecha de corte para restar o no de las ventas acumuladas
			instruccion.sprintf("SELECT @fechacortevtasacum:=valor FROM estadosistemaemp WHERE estado = 'FUCIERREVTAS' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
			instrucciones[num_instrucciones++]= instruccion;

			/* Crea tabla temporal para almacenar cantidades de articulos antes de cancelar*/
			instruccion="CREATE temporary TABLE tmpcantidad(referencia VARCHAR(11) NOT NULL,\
			producto VARCHAR(8) NOT NULL, present VARCHAR(255) NOT NULL, almacen VARCHAR(4) NOT NULL,  \
			cantidad DECIMAL(12,3) NOT NULL, articulo VARCHAR(9) NOT NULL, factor DECIMAL(10,3) NOT NULL,     \
			 PRIMARY KEY (referencia, producto, present, almacen))";
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("INSERT INTO tmpcantidad SELECT d.referencia, a.producto, a.present, d.almacen,  \
			SUM(d.cantidad * a.factor) AS cantidad, d.articulo, a.factor FROM dventas d INNER JOIN \
			articulos a ON a.articulo = d.articulo WHERE d.referencia = '%s' \
			GROUP BY a.producto, a.present, d.almacen", clave);
			instrucciones[num_instrucciones++]=instruccion;

			instruccion = "UPDATE tmpcantidad tmp INNER JOIN existenciasactuales ea ON ea.producto = tmp.producto \
			AND ea.present = tmp.present AND ea.almacen = tmp.almacen SET ea.cantidad = (ea.cantidad  + tmp.cantidad) \
			, ea.ventas = (ea.ventas - tmp.cantidad)";
			instrucciones[num_instrucciones++]=instruccion;

			if (pagoConBilleto=="1") {
				// Se actualiza el registro de la tabla billetopagos
				instruccion.sprintf("UPDATE billetopagos SET cancelado = 1, fechacancel=CURDATE(), horacancel=CURTIME() WHERE venta='%s' ", clave);
				instrucciones[num_instrucciones++]=instruccion;
			}

			// Modifica el precalculo de ventas mensuales
			int mes_modif;
			TDate fecha_vta;
			instruccion.sprintf("select @fechavta:=fechavta AS asigfvta, fechavta from ventas where referencia='%s' ", clave);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_fecha_vta);
			fecha_vta=StrToDate(resp_fecha_vta->ObtieneDato("fechavta"));

			if (YearOf(fecha_vta)>=YearOf(fecha)-1) {
				// Solo afecta a ventas de los ultimos dos años
				if (mFg.IntToAnsiString(YearOf(fecha_vta))==mFg.IntToAnsiString(YearOf(fecha)))
					mes_modif=MonthOf(fecha_vta)+24;
				else
					mes_modif=MonthOf(fecha_vta)+12;

				instruccion.sprintf( \
					"UPDATE ventasxmes vm \
							INNER JOIN ( \
								SELECT dv.almacen, a.producto, a.present, SUM(dv.cantidad * a.factor) AS cantidad \
								FROM dventas dv INNER JOIN articulos a ON a.articulo=dv.articulo \
								WHERE dv.referencia = '%s' GROUP BY dv.almacen, a.producto, a.present \
							) vent ON vm.almacen=vent.almacen AND vm.producto=vent.producto AND vm.present=vent.present \
						SET vm.cant%s = vm.cant%s - vent.cantidad, \
							vm.ventas30 = IF(@fechavta>=DATE_SUB(CURDATE(), INTERVAL 30 DAY) AND @fechavta<=CURDATE(),vm.ventas30 - vent.cantidad, vm.ventas30), \
							vm.ventas60 = IF(@fechavta>=DATE_SUB(CURDATE(), INTERVAL 60 DAY) AND @fechavta<=CURDATE(),vm.ventas60 - vent.cantidad, vm.ventas60), \
							vm.ventas90 = IF(@fechavta>=DATE_SUB(CURDATE(), INTERVAL 90 DAY) AND @fechavta<=CURDATE(),vm.ventas90 - vent.cantidad, vm.ventas90), \
							vm.ventas180 = IF(@fechavta>=DATE_SUB(CURDATE(), INTERVAL 180 DAY) AND @fechavta<=CURDATE(),vm.ventas180 - vent.cantidad, vm.ventas180), \
							vm.ventascorte = IF(@fechavta>@fechacorte AND @fechavta<=CURDATE(), vm.ventascorte - vent.cantidad, vm.ventascorte) ",
					clave, mFg.IntToAnsiString(mes_modif), mFg.IntToAnsiString(mes_modif));
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
			if(error == 6){
				instruccion.sprintf("select %d as error, '%s' as producto,\
						'%s' as present,'%s' as multiplo,'%s' as factor", error,produc2,present2,multiplo2,factor2);
			}else{
				instruccion.sprintf("select %d as error", error);
			}
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	} __finally {
		delete buffer_sql;
		if (resp_fecha_vta!=NULL) delete resp_fecha_vta;
	}
}


//----------------------------------------------------------------------------
//ID_DES_CANCELACION_VENTA
void ServidorVentas::DeshaceCancelacionVenta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  DESHACE LA CANCELACION DE UNA VENTA
	char *buffer_sql=new char[1024*32];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instrucciones[50], instruccion;
	int num_instrucciones=0;
	AnsiString clave, usuario;
	int error=0;
	int i;
	TDate fecha=Today();
	TTime hora=Time();
	BufferRespuestas* resp_fecha_vta=NULL;


	try{
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave de la venta (referencia)
		usuario=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que esta cancelando la venta.

		// Verifica que la venta este cancelada
		instruccion.sprintf("select @error:=if(v.cancelado=0, 1, 0) as error from ventas v left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' where v.referencia='%s'",FormServidor->ObtieneClaveSucursal(), clave);
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 1, error);

		// Verifica que la fecha de la factura sea posterior a la fecha de cierre.
		instruccion.sprintf("select @error:=if(v.fechavta<=cast(e.valor as datetime), 1, 0) as error from ventas v left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' where v.referencia='%s'",FormServidor->ObtieneClaveSucursal(), clave);
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 2, error);

		if (error==0) {
			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

			// DesCancela la venta
			instruccion.sprintf("update ventas set cancelado=0, fechamodi='%s', horamodi='%s', usumodi='%s' where referencia='%s' and cancelado=1", mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario, clave);
			instrucciones[num_instrucciones++]=instruccion;

			// DesCancela el CFD (si hay)
			instruccion.sprintf("update cfd set estado=1, fechacancela=NULL where referencia='%s' and tipocomprobante='VENT'", mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), clave);
			instrucciones[num_instrucciones++]=instruccion;

			// DesCancela el cargo hecho por el total de la venta
			instruccion.sprintf("update transxcob set cancelada=0, fechamodi='%s', horamodi='%s', usumodi='%s' where referencia='%s' and concepto='C' and destino='C' and tipo='VENT' and cancelada=1", mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario, clave);
			instrucciones[num_instrucciones++]=instruccion;

			//vuelve a poner el saldo en ventasconsaldo
			instruccion.sprintf("update ventasconsaldo set saldo = \
				(select valor from transxcob where referencia = '%s'\
				and tipo = 'VENT') where referencia = '%s'",clave,clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Elimnaa el motivo de la cancelación anterior
			instruccion.sprintf("DELETE FROM ventascanceladas WHERE referencia ='%s'", clave);
			instrucciones[num_instrucciones++]=instruccion;

			//Se calcula la última fecha de corte para restar o no de las ventas acumuladas
			instruccion.sprintf("SELECT @fechacortevtasacum:=valor FROM estadosistemaemp WHERE estado = 'FUCIERREVTAS' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
			instrucciones[num_instrucciones++]=instruccion;

			/* Crea tabla temporal para almacenar cantidades de articulos antes de cancelar*/
			instruccion="CREATE temporary TABLE tmpcantidad(referencia VARCHAR(11) NOT NULL,\
			producto VARCHAR(8) NOT NULL, present VARCHAR(255) NOT NULL, almacen VARCHAR(4) NOT NULL,  \
			cantidad DECIMAL(12,3) NOT NULL, articulo VARCHAR(9) NOT NULL, factor DECIMAL(10,3) NOT NULL,     \
			 PRIMARY KEY (referencia, producto, present, almacen))";
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("INSERT INTO tmpcantidad SELECT d.referencia, a.producto, a.present, d.almacen,  \
			SUM(d.cantidad * a.factor) AS cantidad, d.articulo, a.factor FROM dventas d INNER JOIN \
			articulos a ON a.articulo = d.articulo WHERE d.referencia = '%s' \
			GROUP BY a.producto, a.present, d.almacen", clave);
			instrucciones[num_instrucciones++]=instruccion;

			instruccion = "UPDATE tmpcantidad tmp INNER JOIN existenciasactuales ea ON ea.producto = tmp.producto \
			AND ea.present = tmp.present AND ea.almacen = tmp.almacen SET ea.cantidad = (ea.cantidad  - tmp.cantidad) \
			, ea.ventas = (ea.ventas + tmp.cantidad)";
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("select @fechacorte:=ifnull(max(p.fecha),'1900-01-01') as fcorte \
				 from puntoscorte p where p.fecha<='%s'", mFg.DateToMySqlDate(fecha));
			instrucciones[num_instrucciones++]=instruccion;

			// Modifica el precalculo de ventas mensuales
			int mes_modif;
			TDate fecha_vta;
			instruccion.sprintf("select @fechavta:=fechavta AS asigfvta, fechavta from ventas where referencia='%s' ", clave);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_fecha_vta);
			fecha_vta=StrToDate(resp_fecha_vta->ObtieneDato("fechavta"));

			if (YearOf(fecha_vta)>=YearOf(fecha)-1) {
				// Solo afecta a ventas de los ultimos dos años
				if (mFg.IntToAnsiString(YearOf(fecha_vta))==mFg.IntToAnsiString(YearOf(fecha)))
					mes_modif=MonthOf(fecha_vta)+24;
				else
					mes_modif=MonthOf(fecha_vta)+12;

				instruccion.sprintf( \
					"UPDATE ventasxmes vm \
							INNER JOIN ( \
								SELECT dv.almacen, a.producto, a.present, SUM(dv.cantidad * a.factor) AS cantidad \
								FROM dventas dv INNER JOIN articulos a ON a.articulo=dv.articulo \
								WHERE dv.referencia = '%s' GROUP BY dv.almacen, a.producto, a.present \
							) vent ON vm.almacen=vent.almacen AND vm.producto=vent.producto AND vm.present=vent.present \
						SET vm.cant%s = vm.cant%s + vent.cantidad, \
							vm.ventas30 = IF(@fechavta>=DATE_SUB(CURDATE(), INTERVAL 30 DAY) AND @fechavta<=CURDATE(),vm.ventas30 + vent.cantidad, vm.ventas30), \
							vm.ventas60 = IF(@fechavta>=DATE_SUB(CURDATE(), INTERVAL 60 DAY) AND @fechavta<=CURDATE(),vm.ventas60 + vent.cantidad, vm.ventas60), \
                            vm.ventas90 = IF(@fechavta>=DATE_SUB(CURDATE(), INTERVAL 90 DAY) AND @fechavta<=CURDATE(),vm.ventas90 + vent.cantidad, vm.ventas90), \
							vm.ventas180 = IF(@fechavta>=DATE_SUB(CURDATE(), INTERVAL 180 DAY) AND @fechavta<=CURDATE(),vm.ventas180 + vent.cantidad, vm.ventas180), \
							vm.ventascorte = IF(@fechavta>@fechacorte AND @fechavta<=CURDATE(), vm.ventascorte + vent.cantidad, vm.ventascorte) ",
					clave, mFg.IntToAnsiString(mes_modif), mFg.IntToAnsiString(mes_modif));
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
			instruccion.sprintf("select %d as error", error);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	} __finally {
		delete buffer_sql;
		if (resp_fecha_vta!=NULL) delete resp_fecha_vta;
	}
}




//------------------------------------------------------------------------------
//ID_CON_VENTA
void ServidorVentas::ConsultaVenta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA VENTAS
	AnsiString instruccion;
	AnsiString clave, menos_devoluciones;
	AnsiString ordenMultiplo;
	AnsiString cargaexistencias;
	AnsiString campo_cargaexistencias=" ";
	AnsiString innerjoin_cargaexistencias=" ";
	AnsiString almacen_existencias;
	AnsiString cad_conjunto_alma_ex;
	BufferRespuestas* resp_almacenes_ex = NULL;



	clave=mFg.ExtraeStringDeBuffer(&parametros);
	menos_devoluciones=mFg.ExtraeStringDeBuffer(&parametros);
	ordenMultiplo=mFg.ExtraeStringDeBuffer(&parametros);
	cargaexistencias=mFg.ExtraeStringDeBuffer(&parametros);

	if (cargaexistencias=="1") {
		instruccion.sprintf("SELECT a.almacen FROM almacenes a \
			INNER JOIN secciones s ON a.seccion=s.seccion \
			WHERE s.sucursal='%s'", FormServidor->ObtieneClaveSucursal());
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_almacenes_ex);
		for(int i=0; i<resp_almacenes_ex->ObtieneNumRegistros(); i++){
			resp_almacenes_ex->IrAlRegistroNumero(i);
			almacen_existencias=resp_almacenes_ex->ObtieneDato("almacen");

			cad_conjunto_alma_ex+="'";
			cad_conjunto_alma_ex+=almacen_existencias;
			cad_conjunto_alma_ex+="'";
			if (i<resp_almacenes_ex->ObtieneNumRegistros()-1) // A todos menos al ultimo impuesto el signo +.
				cad_conjunto_alma_ex+=",";
		}
		delete resp_almacenes_ex;

		innerjoin_cargaexistencias.sprintf(" INNER JOIN existenciasactuales ex ON ex.producto = a.producto AND ex.present = a.present AND ex.almacen IN (%s) ", cad_conjunto_alma_ex);
		campo_cargaexistencias=", TRUNCATE((ex.cantidad/factor),0) AS existencias ";
	}

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los generales (de cabecera) de la venta
	instruccion.sprintf("select v.*, v.ticket as tipodoc, \
		cfd.folio as cfdfolio, cfd.serie as cfdserie, cfdxml.xmlgenerado as cfdxml, cfdxml.cadenaoriginal as cfdcadenaoriginal, \
		cfd.version, cfd.muuid, cfd.pactimbrador, cfd.metodopago, cfd.digitos, \
		vm.mensaje, \
		col.nombre as nomcolonia, loc.nombre as nomlocalidad, \
		ter.nombre as nomterminal, emp1.nombre as nomcajero, v.comisionada as comisionada, \
		emp2.nombre as nomvendedor, \
		concat(emp3.nombre,' ',emp3.appat) as nomusualta, \
		concat(emp4.nombre,' ',emp4.appat) as nomchofer, \
		ifnull(kt.nombre,'') as nombrekit, IFNULL(kt.desglosado,1) as desglosadokit, \
		cfd.formapago33, cfd.usocfdi, \
		ifnull(vde.calle,' ') as dircalle, dvfp2.termino, GROUP_CONCAT(fp.descripcion) AS descripcion, GROUP_CONCAT(fp.formapag) AS formaspago, \
		GROUP_CONCAT(dvfp.valor) AS cantidadpagos, GROUP_CONCAT(dvfp.porcentaje) AS porcentajespagos, GROUP_CONCAT(IFNULL(dvfp.trn_id,0)) AS idtrn, \
		ifnull(vde.numext, ' ') as dirnumex, \
		ifnull(vde.numint,' ') as dirnumin, ifnull(co.colonia,' ') as dircol, \
		ifnull(vde.cp,' ') as dircp, ifnull(vde.referenciadom,' ') as dirreferencia, \
		ifnull(co.nombre,' ') as dirnomcolonia, emb.concartaporte, IFNULL(co.sector,' ') as dirsect \
		from ventas v  inner join  clientes c \
		left join cfd on v.referencia=cfd.referencia and cfd.tipocomprobante='VENT' \
		left join cfdxml on cfd.compfiscal=cfdxml.compfiscal \
		left join colonias col on c.colonia=col.colonia \
		left join localidades loc on col.localidad=loc.localidad \
		left join terminales ter on v.terminal=ter.terminal \
		left join empleados emp1 on v.usumodi=emp1.empleado \
		left join empleados emp2 on v.vendedor=emp2.empleado \
		left join embarques emb on emb.embarque=v.embarque \
		left join empleados emp3 on v.usualta=emp3.empleado \
		left join empleados emp4 on emb.chofer=emp4.empleado \
		left join kits kt on v.kit=kt.kit \
		left join ventasmensajes vm on vm.referencia=v.referencia \
		LEFT join ventadirent vde on vde.referencia=v.referencia and vde.cliente=v.cliente \
		LEFT JOIN colonias co ON vde.colonia=co.colonia \
		LEFT JOIN dventasfpago dvfp ON dvfp.referencia=v.referencia \
		LEFT JOIN formasdepago fp ON fp.formapag=dvfp.formapag \
		LEFT JOIN ( \
				SELECT fp2.termino, dvfp3.referencia \
				FROM dventasfpago dvfp3 \
				INNER JOIN formasdepago fp2 ON fp2.formapag=dvfp3.formapag \
				WHERE dvfp3.referencia='%s' \
				AND dvfp3.valor = (select max(dvfp4.valor) from dventasfpago dvfp4 where dvfp4.referencia = '%s') LIMIT 1 \
			 ) dvfp2 ON dvfp2.referencia = v.referencia \
		where v.referencia='%s' and v.cliente=c.cliente ", clave, clave, clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Calcula el saldo de la venda (SALDO B)
	instruccion.sprintf("select sum(valor) as saldo from transxcob where referencia='%s' and cancelada=0 group by referencia", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene todos los datos del cliente de la venta
	instruccion.sprintf("select cli.* from clientes cli, ventas v where v.referencia='%s' and v.cliente=cli.cliente", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Verifica que la fecha de la factura sea posterior a la fecha de cierre.
	instruccion.sprintf("select if(v.fechavta>=cast(e.valor as datetime), 1, 0) as modificar from ventas v left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' where v.referencia='%s'",FormServidor->ObtieneClaveSucursal(), clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene todo el detalle de la venta con algunos datos extras que necesita el cliente
	if (menos_devoluciones=="0") {

		// Obtiene los datos de la venta completa (sin importar las devoluciones)
		instruccion="select d.referencia, d.articulo, d.cantidad, d.costobase, ";
		instruccion+="d.porcdesc, d.tipoprec, d.precio, d.precioimp, d.porccomi, ";
		instruccion+="a.present, p.producto, p.nombre, a.multiplo, a.factor, ";
		instruccion+="d.claveimp1, i1.tipoimpu as tipoimpuesto1, i1.porcentaje as porcentajeimp1, t1.nombre as nomtipoimp1, ";
		instruccion+="d.claveimp2, i2.tipoimpu as tipoimpuesto2, i2.porcentaje as porcentajeimp2, t2.nombre as nomtipoimp2, ";
		instruccion+="d.claveimp3, i3.tipoimpu as tipoimpuesto3, i3.porcentaje as porcentajeimp3, t3.nombre as nomtipoimp3, ";
		instruccion+="d.claveimp4, i4.tipoimpu as tipoimpuesto4, i4.porcentaje as porcentajeimp4, t4.nombre as nomtipoimp4, ";
		instruccion+="(a.factor / pmm.cajalogisticafactor) AS cajasbultos ";
		if(ordenMultiplo=="1")
			instruccion+=",IF(a.factor=1,1,2) ORDEN,p.clasif1  "; //Agrupacion para ordenar por multiplo
		instruccion+=",IFNULL(dvk.kit,'') as kit, d.id ";
		instruccion+=campo_cargaexistencias;
		instruccion+="from dventas d  inner join  articulos a  inner join  productos p ";
		instruccion+=innerjoin_cargaexistencias;
		instruccion+="left join impuestos i1 on i1.impuesto=d.claveimp1 ";
		instruccion+="left join impuestos i2 on i2.impuesto=d.claveimp2 ";
		instruccion+="left join impuestos i3 on i3.impuesto=d.claveimp3 ";
		instruccion+="left join impuestos i4 on i4.impuesto=d.claveimp4 ";
        instruccion+="left join tiposdeimpuestos t1 on t1.tipoimpu=i1.tipoimpu ";
        instruccion+="left join tiposdeimpuestos t2 on t2.tipoimpu=i2.tipoimpu ";
        instruccion+="left join tiposdeimpuestos t3 on t3.tipoimpu=i3.tipoimpu ";
		instruccion+="left join tiposdeimpuestos t4 on t4.tipoimpu=i4.tipoimpu ";
		instruccion+="LEFT JOIN presentacionesminmax pmm ON a.producto=pmm.producto AND a.present=pmm.present ";
		instruccion+="left join dventaskits dvk on dvk.venta=d.referencia and dvk.articulo=d.articulo ";
		instruccion+="where d.referencia='";
        instruccion+=clave;
		instruccion+="' and d.articulo=a.articulo and a.producto=p.producto ";
		instruccion+=" group by p.nombre, a.present, a.multiplo ";
		if(ordenMultiplo=="1")
			instruccion+="ORDER BY ORDEN DESC,IF(ORDEN='1',p.clasif1,left(multiplo,3)),p.nombre, a.present, a.multiplo ";
		else
			instruccion+=" order by d.id, p.nombre, a.present, a.multiplo ";

		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    } else {
        // Suma las notas de crédito y deja el resultado en una tabla temporal,
		// para luego hacerle un left join con las ventas.
		instruccion="create temporary table dnotascredcliaux ";
		instruccion+="select d.articulo, sum(if(n.tipo='0',d.cantidad,0)) as cantidad, ";
        instruccion+="sum(if(n.tipo<>'0',d.precio,0)) as precio, ";
		instruccion+="sum(if(n.tipo<>'0',d.precioimp,0)) as precioimp ";
        instruccion+="from notascredcli n, dnotascredcli d ";
		instruccion+="where n.venta='";
		instruccion+=clave;
        instruccion+="' and n.cancelado=0 and n.referencia=d.referencia ";
        instruccion+="group by d.articulo";
		if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
			throw (Exception("Error en query EjecutaSelectSqlNulo"));

        // Sumatoria de todas las notas de crédito.
		instruccion.sprintf("select ifnull(sum(valor),0) as sumnotas, ifnull(sum(if(notascredcli.tipo=0,1,0)),0) as numdevol,  ifnull(sum(if(notascredcli.tipo=1,1,0)),0) as numboni, ifnull(sum(if(notascredcli.tipo=2,1,0)),0) as numdesc from notascredcli where cancelado=0 and venta='%s'", clave);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

		// Obtiene los datos de lo que resta de las ventas
        // menos las notas de credito ya aplicadas.
		instruccion="select dv.referencia, dv.articulo, sum(dv.cantidad-ifnull(dn.cantidad,0)) as cantidad, dv.costobase, ";
		instruccion+="dv.porcdesc, dv.tipoprec, ";
        instruccion+="sum(dv.precio-ifnull(dn.precio,0)) as precio, ";
		instruccion+="sum(dv.precioimp-ifnull(dn.precioimp,0)) as precioimp, ";
        instruccion+="a.present, p.producto, p.nombre, a.multiplo, a.factor, ";
		instruccion+="dv.claveimp1, i1.tipoimpu as tipoimpuesto1, i1.porcentaje as porcentajeimp1, t1.nombre as nomtipoimp1, ";
        instruccion+="dv.claveimp2, i2.tipoimpu as tipoimpuesto2, i2.porcentaje as porcentajeimp2, t2.nombre as nomtipoimp2, ";
		instruccion+="dv.claveimp3, i3.tipoimpu as tipoimpuesto3, i3.porcentaje as porcentajeimp3, t3.nombre as nomtipoimp3, ";
		instruccion+="dv.claveimp4, i4.tipoimpu as tipoimpuesto4, i4.porcentaje as porcentajeimp4, t4.nombre as nomtipoimp4 ";
		if(ordenMultiplo=="1")
			instruccion+=",IF(a.factor=1,1,2) ORDEN,p.clasif1  "; //Agrupacion para ordenar por multiplo
		instruccion+=",IFNULL(dvk.kit,'') as kit ";
		instruccion+="from ventas v  inner join  dventas dv  inner join  articulos a  inner join  productos p ";
        instruccion+="left join impuestos i1 on i1.impuesto=dv.claveimp1 ";
        instruccion+="left join impuestos i2 on i2.impuesto=dv.claveimp2 ";
        instruccion+="left join impuestos i3 on i3.impuesto=dv.claveimp3 ";
		instruccion+="left join impuestos i4 on i4.impuesto=dv.claveimp4 ";
		instruccion+="left join tiposdeimpuestos t1 on t1.tipoimpu=i1.tipoimpu ";
        instruccion+="left join tiposdeimpuestos t2 on t2.tipoimpu=i2.tipoimpu ";
        instruccion+="left join tiposdeimpuestos t3 on t3.tipoimpu=i3.tipoimpu ";
        instruccion+="left join tiposdeimpuestos t4 on t4.tipoimpu=i4.tipoimpu ";
		instruccion+="left join dnotascredcliaux dn on dn.articulo=dv.articulo ";
		instruccion+="left join dventaskits dvk on dvk.venta=dv.referencia and dvk.articulo=dv.articulo ";
		instruccion+="where v.referencia='";
		instruccion+=clave;
		instruccion+="' and v.referencia=dv.referencia ";
		instruccion+="and dv.articulo=a.articulo and a.producto=p.producto ";
		instruccion+=" group by p.nombre, a.present, a.multiplo ";
		if(ordenMultiplo=="1")
			instruccion+="ORDER BY ORDEN DESC,IF(ORDEN='1',p.clasif1,left(a.multiplo,3)),p.nombre, a.present, a.multiplo  ";
		else
			instruccion+=" order by p.nombre, a.present, a.multiplo ";

        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}

	//validar que tenga una direccion de entrega
	instruccion.sprintf("SELECT * from ventas v where v.referencia='%s' \
	and v.referencia not in (select referencia from ventadirent)", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene la informacion de la terminal de la  transaccion por la pinpad
	instruccion.sprintf("SELECT \
	dtrnvnta.mer_legend1 AS nomnegocio, \
	dtrnvnta.mer_legend2 AS dirnegocio, \
	dtrnvnta.mer_legend3 AS ciudnegocio, \
	dtrnvnta.trn_external_mer_id AS negocio, \
	dtrnvnta.trn_external_ter_id AS termnegocio, \
	dtrnvnta.trn_host_date AS fecha \
	FROM ventas v \
	INNER JOIN dventasfpago dvfp ON dvfp.referencia = v.referencia \
	INNER JOIN dettrnxventa dtrnvnta ON dvfp.trn_id = dtrnvnta.trn_id \
	WHERE v.referencia = '%s' \
	GROUP BY dtrnvnta.mer_legend1  ",
	clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene el detalle de las  transaccions por la pinpad
	instruccion.sprintf("SELECT \
	lpad(dtrnvnta.trn_id,9,'0') AS id, \
	dtrnvnta.trn_auth_code AS auth_code, \
	dtrnvnta.trn_amount AS monto, \
	dtrnvnta.trn_host_hour AS hora, \
	IF(dtrnvnta.trn_qty_pay=1, 'COMPRA NORMAL', CONCAT(dtrnvnta.trn_qty_pay, ' MESES SIN INTERESES')) AS tipocompra \
	FROM ventas v \
	INNER JOIN dventasfpago dvfp ON dvfp.referencia = v.referencia \
	INNER JOIN dettrnxventa dtrnvnta ON dvfp.trn_id = dtrnvnta.trn_id \
	WHERE v.referencia = '%s' ",
	clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

    //Obtiene si exiten  kits en la venta
	instruccion.sprintf("select vk.*,k.nombre from ventaskits vk inner join kits k on vk.kit=k.kit \
			where vk.venta='%s' order by vk.kit", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	//Obtiene las dispocisión de efectivo
	instruccion.sprintf("SELECT \
		c.clave, \
		(m.cantretirado)*-1 AS valor \
	FROM movsefectivocaja m \
	INNER JOIN catalogomovscaja c ON c.clave = m.conceptomovs \
	WHERE m.venta = '%s'", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

}

//----------------------------------------------------------------------------
//ID_CANC_CFD
void ServidorVentas::CancelaCfd(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  CANCELA CFD
	// Da de baja un CFD (que no tenga una referencia relacionada, por ejemplo factura global de tickets)
	char *buffer_sql=new char[1024*32];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instrucciones[50], instruccion;
	int num_instrucciones=0;
	AnsiString clave, usuario;
	int error=0;
	int i;
	TDate fecha=Today();
	TTime hora=Time();
	BufferRespuestas* resp_cfd = NULL;
	AnsiString folioerror = " ";
	AnsiString forzarcancelar;

	try{
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave del CFD
		usuario=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que esta cancelando la venta.
		forzarcancelar= mFg.ExtraeStringDeBuffer(&parametros);//parametro de validacion para forzar o no la cancelacion en el CFD

		// Verifica que exista el cfd, que no este cancelado y que no sea ni VENT, ni NCRE ni NCAR.
		instruccion.sprintf("select @error:=if(COALESCE(count(*),0)=0, 1, 0) as error from cfd where compfiscal=%s and estado=1 and tipocomprobante not in ('VENT','NCAR','NCRED')", clave);
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 2, error);

		// Verifica que la fecha de la factura sea posterior a la fecha de cierre.
		instruccion.sprintf("select @error:=if(v.fechavta<=cast(e.valor as datetime), 1, 0) as error from ventas v left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' where v.referencia='%s'",FormServidor->ObtieneClaveSucursal(), clave);
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 3, error);

		// Verifica si hay una nota de crédito activa relacionada con la factura global, entonces no se permite cancelarla.
		// (para cancelar la factura global se debe antes cancelar la nota relacionada).
		instruccion="SELECT @error := IF(COALESCE(COUNT(*), 0) = 0, 0, 1) AS error, @folioerror := cfdnotrel.referencia ";
		instruccion+="FROM cfd c ";
		instruccion+=" INNER JOIN cfd cfdnotrel ON c.muuid=cfdnotrel.cfdirelacionado AND cfdnotrel.estado=1 AND ";
		instruccion+="	cfdnotrel.sucursal=c.sucursal AND cfdnotrel.tipocomprobante='NCRE' ";
		instruccion+="WHERE  c.compfiscal = '";
		instruccion+=clave;
		instruccion+="' AND c.tipocomprobante='TICK' AND (c.version='3.3' or c.version='4.0')";
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 4, error);

		// Busca el comprobante
		instruccion.sprintf("SELECT c.sucursal, c.referencia FROM cfd c \
				WHERE c.compfiscal=%s and c.tipocomprobante='TICK' ", clave );
		if (!mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_cfd))
			throw Exception("No se pudo consultar el CFD o CFDI global");
		if (resp_cfd->ObtieneNumRegistros()==0)
			throw Exception("No se encontro el CFD o CFDI global");

		if (error==0) {
			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

			if (forzarcancelar=="1"){
				// Cancela el CFDI con el PAC (si no se trata de un CFDI simplemente no se hace nada con ningún PAC)
				// SOLO CUANDO NO ESTA EN DEPURACION
				#ifndef _DEBUG
				ComprobanteFiscalDigital *cfd=NULL;
				try {
					cfd=new ComprobanteFiscalDigital(mServidorVioleta);
					cfd->cancelarCFDI(Respuesta, MySQL, "TICK",resp_cfd->ObtieneDato("referencia"), resp_cfd->ObtieneDato("sucursal"),"04","" );
				} __finally {
					if(cfd!=NULL) delete cfd;
				}
				#endif
			}//fin de forzar cancelar ticket global
			// Cancela el CFD
			instruccion.sprintf("update cfd set estado=0, fechacancelamin=curdate(), fechacancela='%s %s' where compfiscal=%s", mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), clave);
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("INSERT INTO bitacoracancforzadas VALUES ( NULL, '%s', '%s', '%s', '%s')", usuario, clave, mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora));
			instrucciones[num_instrucciones++]=instruccion;

			instrucciones[num_instrucciones++]="COMMIT";
		}

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			instruccion.sprintf("select %d as error, @folioerror  as folioerror", error);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	} __finally {
		delete buffer_sql;
		if (resp_cfd!=NULL)	delete resp_cfd;
	}
}

//---------------------------------------------------------------------------
//ID_GRA_KIT
void ServidorVentas::GrabaKit(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  GRABA UN KIT
	char *buffer_sql=new char[1024*64*10];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString tarea, clave, usuario, nombre, desglosado, codigobarras,activo;
	int num_partidas, i;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[1000];
	AnsiString articulo, cantidad;
	TDate fecha=Today();
	AnsiString precioCom;


	try{
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave del kit.
		tarea=mFg.ExtraeStringDeBuffer(&parametros); // Indicador si se va a agregar o modificar.
		usuario=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que esta grabando.
		nombre=mFg.ExtraeStringDeBuffer(&parametros); // Nombre del Kit.
		desglosado=mFg.ExtraeStringDeBuffer(&parametros);  // Se imprime o no desglosado el kit
		codigobarras=mFg.ExtraeStringDeBuffer(&parametros); // Código de barras del kit.
		activo=mFg.ExtraeStringDeBuffer(&parametros); // Activado ó no el kit
		precioCom=mFg.ExtraeStringDeBuffer(&parametros); // precio comercial del kit

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		// Obtiene el folio para el KIT y para su código de barras.
		if (tarea=="A") {
			// Obtiene folio para el KIT
			instruccion.sprintf("select @folioaux:=valor from foliosemp where folio='KITS' AND sucursal = '%s' %s ",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @foliosig=@folioaux+1");
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @folioaux=cast(@folioaux as char)");
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @folio=concat('%s', lpad(@folioaux,4,'0'))", FormServidor->ObtieneClaveSucursal());
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("update foliosemp set valor=@foliosig where folio='KITS' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
			instrucciones[num_instrucciones++]=instruccion;

			// Obtiene folio para el Código de Barras del KIT
			instruccion.sprintf("select @folioaux:=valor from foliosemp where folio='KITSCB' AND sucursal = '%s' %s ",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @foliosig=@folioaux+1");
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @folioaux=cast(@folioaux as char)");
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @foliocb=concat('%s', lpad(@folioaux,9,'0'))", FormServidor->ObtieneNumIdSucursal());
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("update foliosemp set valor=@foliosig where folio='KITSCB' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
			instrucciones[num_instrucciones++]=instruccion;
		} else {
			instruccion.sprintf("set @folio='%s'", clave);
			instrucciones[num_instrucciones++]=instruccion;
		}

		// Si se está modificando entonces borra el detalle que ya exista.
		if (tarea=="M") {
			instruccion.sprintf("delete from dkits where kit=@folio");
			instrucciones[num_instrucciones++]=instruccion;
		}

		// Graba la cabecera en la tabla "kits"
		datos.AsignaTabla("kits");
		datos.InsCampo("nombre", nombre);
		datos.InsCampo("desglosado", desglosado);
		datos.InsCampo("usumodi", usuario);
		datos.InsCampo("fechamodi", mFg.DateToAnsiString(fecha));
		datos.InsCampo("activo", activo);
		datos.InsCampo("preciocomkit",precioCom);
		if (tarea=="A") {
			datos.InsCampo("kit", "@folio", 1);
			datos.InsCampo("ean13", "@foliocb", 1);
			datos.InsCampo("usualta", usuario);
			datos.InsCampo("fechaalta", mFg.DateToAnsiString(fecha));
			datos.InsCampo("cancelado", "0");
			instrucciones[num_instrucciones++]=datos.GenerarSqlInsert();
		} else {
			instrucciones[num_instrucciones++]=datos.GenerarSqlUpdate("kit=@folio");
		}

		// Graba las partidas en "dkits"
		num_partidas=StrToInt(mFg.ExtraeStringDeBuffer(&parametros)); // Obtiene el número de partidas
		for (i=0; i<num_partidas; i++) {
			datos.AsignaTabla("dkits");
			parametros+=datos.InsCamposDesdeBuffer(parametros);
			datos.InsCampo("kit", "@folio", 1);
			instrucciones[num_instrucciones++]=datos.GenerarSqlInsert();
		}

		//Inserción en la bitacora de kits
		instruccion.sprintf("INSERT INTO bitacorakits(kit,nombre,usuario,tipo,activo,desglosado,cancelado) \
							VALUES (@folio,'%s','%s','%s',%s,%s,(select cancelado from kits where kit=@folio))"
							,nombre,usuario,tarea,activo,desglosado);
		instrucciones[num_instrucciones++]=instruccion;

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
//ID_CANC_KIT
void ServidorVentas::CancelaKit(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  CANCELA UN KIT
	char *buffer_sql=new char[1024*32];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instrucciones[50], instruccion;
	int num_instrucciones=0;
	AnsiString clave,usuario;
	int i;

	try{
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave del movimiento
		usuario=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que cancela kit

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

        // Crea variable de cancelación
		instruccion.sprintf("set @kit='%s'", clave);
		instrucciones[num_instrucciones++]=instruccion;

		// Cancela el kit
		instruccion.sprintf("update kits set cancelado=1 where kit=@kit and cancelado=0");
		instrucciones[num_instrucciones++]=instruccion;

		//Inserción en la bitacora de kits
		instruccion.sprintf("INSERT INTO bitacorakits(kit,nombre,usuario,tipo,activo,desglosado,cancelado) \
							VALUES (@kit, \
							(select nombre from kits where kit=@kit),'%s','C', \
							(select activo from kits where kit=@kit), \
							(select desglosado from kits where kit=@kit),1)"
							,usuario);
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
//ID_CON_KIT
void ServidorVentas::ConsultaKit(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA DE UN KIT
	AnsiString instruccion;
	AnsiString clave;

	clave=mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los generales (de cabecera) del kit
	instruccion.sprintf("select * from kits where kit='%s'", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene todo el detalle del kit.
	instruccion.sprintf("select d.cantidad, a.multiplo, p.nombre, a.present, \
		a.producto, a.articulo, d.tipoprec, format(d.precioart,5) as precioart, \
		format(d.preciocomart,5) as preciocomart \
		from kits k, dkits d, articulos a, productos p, precios, tiposdeprecios tp \
		where k.kit='%s' and k.kit=d.kit and \
		d.articulo=a.articulo and a.producto=p.producto and \
		precios.articulo=a.articulo and precios.tipoprec=d.tipoprec and \
		precios.tipoprec=tp.tipoprec and tp.idempresa=%s \
		order by p.nombre asc",
		clave, FormServidor->ObtieneClaveEmpresa());
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}

//------------------------------------------------------------------------------
//ID_GRA_PEDIDO_CLI
void ServidorVentas::GrabaPedidoCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  GRABA PEDIDO CLIENTE
	char *buffer_sql=new char[10000*1000];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString tarea, clave, usuario;
	int num_partidas, i;
	int num_instrucciones=0;
	AnsiString instruccion;
	AnsiString * instrucciones = new AnsiString[10000];
	double valor;
	AnsiString periodic, terminal, kit, mensaje,almacen_global,mensaje_articulo;
	int cantkits;
	int dias_plazo;
	int acredito;
	TDate fecha=Today();
	TTime hora=Time();
	AnsiString tipo_original, cotizacion;
	AnsiString impuesto1, articulo;
	AnsiString usar_preciocliente, cliente;

	AnsiString usocfdi, formapago;

	double mParamPrecioMin,Gprecio;
	AnsiString nombre, present, prod;
	AnsiString mDIRcalle,mDIRnumext,mDIRnumint,mDIRcolonia,mDIRcp,mDIRubicacionX,mDIRubicacionY,mDIRreferencia_domicilio;

	AnsiString origen_pedido;

	int mCantidadKits,mCantArtKits;
	AnsiString msjerror="", errorfinal="";
	AnsiString cadenalimpia="";


	try{
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave del pedido
		tarea=mFg.ExtraeStringDeBuffer(&parametros); // Indicador si se va a agregar o modificar.
		tipo_original=mFg.ExtraeStringDeBuffer(&parametros); // '0'=Pedido, '1'=Cotización
		usuario=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que esta grabando.
		terminal=mFg.ExtraeStringDeBuffer(&parametros); // Terminal donde se realizo el pedido.
		kit=mFg.ExtraeStringDeBuffer(&parametros); // Kit que se pide.
		cantkits=StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
		if (kit==" ") { // Un espacio lo tomamos con kit nulo.
			kit="";
			cantkits=0;
		}
		mensaje=mFg.ExtraeStringDeBuffer(&parametros); // mensaje.
		almacen_global=mFg.ExtraeStringDeBuffer(&parametros); // Almacen del que saldra toda la venta
		usar_preciocliente=mFg.ExtraeStringDeBuffer(&parametros); // Indica si usar o no el precio del cliente para precioimp


		//nuevos parametros para la version 3.3 o 4.0
		formapago = mFg.ExtraeStringDeBuffer(&parametros);
		usocfdi = mFg.ExtraeStringDeBuffer(&parametros);

		/*NUEVOS PARAMETROS DE DIRECCION ENTREGA*/
		mDIRcalle=mFg.ExtraeStringDeBuffer(&parametros); // calle
		mDIRnumext=mFg.ExtraeStringDeBuffer(&parametros); //  num exterior
		mDIRnumint=mFg.ExtraeStringDeBuffer(&parametros); // num interior
		mDIRcolonia=mFg.ExtraeStringDeBuffer(&parametros); // colonia
		mDIRcp=mFg.ExtraeStringDeBuffer(&parametros); // codigo postal
		mDIRubicacionX=mFg.ExtraeStringDeBuffer(&parametros); //  ubicacionX== Latitud
		mDIRubicacionY=mFg.ExtraeStringDeBuffer(&parametros); //  ubicacionY== Altitud
		mDIRreferencia_domicilio=mFg.ExtraeStringDeBuffer(&parametros);   // referencia

		/*FIN DE PARAMETROS DIRECCION DE ENTREGA*/
		origen_pedido=mFg.ExtraeStringDeBuffer(&parametros);   // origen del pedido


		// Obtiene los datos de la tabla de pedidos
		datos.AsignaTabla("pedidosventa");
		parametros+=datos.InsCamposDesdeBuffer(parametros);

		// Extrae los datos que necesitamos para crear el pedido
		periodic=datos.ObtieneValorCampo("periodic");
		acredito=StrToInt(datos.ObtieneValorCampo("acredito"));
		cotizacion=datos.ObtieneValorCampo("cotizacion");
		cliente=datos.ObtieneValorCampo("cliente");

		if (periodic=="MES") throw(Exception("Todavía no se implementa la periodicidad mensual en ventas"));
		if (periodic=="QUI") throw(Exception("Todavía no se implementa la periodicidad quincenal en ventas"));
		if (periodic=="SEM") throw(Exception("Todavía no se implementa la periodicidad semanal en ventas"));
		dias_plazo=StrToInt(periodic);


		//nueva consulta de un parametro de precio minimo
		BufferRespuestas* resp_preciomin=NULL;
		//se creara un bufer para obtener el valor del parametro de precio minimo y posteriormete
		//liberar la memoria ocupada de esta consulta
		try{
			instruccion.sprintf("SELECT * FROM parametrosemp WHERE parametro = 'PRECIOMINIMVTA' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_preciomin);
			if (resp_preciomin->ObtieneNumRegistros()>0){
				mParamPrecioMin=mFg.CadenaAFlotante(resp_preciomin->ObtieneDato("valor")); //valor de precio minimo
			}
		}__finally{
			if (resp_preciomin!=NULL) delete resp_preciomin;
		}


		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		// Obtiene el folio para el pedido
		if (tarea=="A") {
			instruccion.sprintf("select @folioaux:=valor from foliosemp where folio='PEDICLI' AND sucursal = '%s' %s ",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @foliosig=@folioaux+1");
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @folioaux=cast(@folioaux as char)");
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @folio=concat('%s', lpad(@folioaux,9,'0'))", FormServidor->ObtieneClaveSucursal());
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("update foliosemp set valor=@foliosig where folio='PEDICLI' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
			instrucciones[num_instrucciones++]=instruccion;
		} else {
			instruccion.sprintf("set @folio='%s'", clave);
			instrucciones[num_instrucciones++]=instruccion;
		}

		//quitar despues de pruebas
		instruccion.sprintf(" select @folio");
		instrucciones[num_instrucciones++]=instruccion;

		// Si se está modificando entonces borra el detalle y las letras que ya existan.
		if (tarea=="M") {
			instruccion.sprintf("delete from dpedidosventa where referencia=@folio");
			instrucciones[num_instrucciones++]=instruccion;
		}

		// Todo se va a una letra que se salda automáticamente en el caso de las
		// ventas de contado
		if (!acredito) {
			dias_plazo=0;
		}

		// Graba la cabecera en la tabla "pedidosventa"
		if (tarea=="A") {
			datos.InsCampo("referencia", "@folio",1);
			datos.InsCampo("usualta", usuario);
			datos.InsCampo("usumodi", usuario);
			datos.InsCampo("fechaalta", mFg.DateToAnsiString(fecha));
			datos.InsCampo("horaalta", mFg.TimeToAnsiString(hora));
			datos.InsCampo("fechamodi", mFg.DateToAnsiString(fecha));
			datos.InsCampo("horamodi", mFg.TimeToAnsiString(hora));
			datos.InsCampo("fechaped", mFg.DateToAnsiString(fecha));
			datos.InsCampo("fechasurt", mFg.DateToAnsiString(fecha));
			datos.InsCampo("facturado", "0");
			datos.InsCampo("cancelado", "0");
			datos.InsCampo("terminal", terminal);
			datos.InsCampo("kit", kit);
			datos.InsCampo("cantkits", mFg.IntToAnsiString(cantkits));
			datos.InsCampo("plazo", mFg.IntToAnsiString(dias_plazo));
			datos.InsCampo("tipoorigen",origen_pedido);
            //datos.InsCampo("diasvigencia","0");
			if(almacen_global!=" ")
				datos.InsCampo("almacen",almacen_global);
			if(usocfdi!=" ")
				datos.InsCampo("usocfdi33",usocfdi);
			 if(formapago!=" ")
				datos.InsCampo("formapago33",formapago);
			instrucciones[num_instrucciones++]=datos.GenerarSqlInsert();

			// Guarda el mensaje
			if (mensaje!=" ") { // Un espacio lo tomamos como nulo.
				instruccion.sprintf("insert into pedidosmensajes (referencia, mensaje) values (@folio,'%s')",
					mensaje);
				instrucciones[num_instrucciones++]=instruccion;
			}
		} else {
			// Si el tipo era originalmente cotización y se esta grabando como pedido
			// entonces la fecha del pedido se asigna a la fecha actual.
			if (tipo_original=="1" && cotizacion=="0") {
				datos.InsCampo("fechaped", mFg.DateToAnsiString(fecha));
			}
			datos.InsCampo("kit", kit);
			datos.InsCampo("cantkits", mFg.IntToAnsiString(cantkits));
			datos.InsCampo("usumodi", usuario);
			datos.InsCampo("fechamodi", mFg.DateToAnsiString(fecha) );
			datos.InsCampo("horamodi", mFg.TimeToAnsiString(hora) );
			datos.InsCampo("plazo", mFg.IntToAnsiString(dias_plazo));
			if(almacen_global!=" ")
					datos.InsCampo("almacen",almacen_global);
			if(usocfdi!=" ")
				datos.InsCampo("usocfdi33",usocfdi);
			 if(formapago!=" ")
				datos.InsCampo("formapago33",formapago);
			instrucciones[num_instrucciones++]=datos.GenerarSqlUpdate("referencia=@folio");

			// Guarda el mensaje
			if (mensaje!=" ") { // Un espacio lo tomamos como nulo.
				instruccion.sprintf("replace into pedidosmensajes (referencia, mensaje) values (@folio,'%s')",
					mensaje);
			} else {
				instruccion.sprintf("delete from pedidosmensajes where referencia=@folio");
			}
			instrucciones[num_instrucciones++]=instruccion;
		}


		// Graba las partidas en "dpedidosventa"
		int val = 0, id_respuesta;
		num_partidas=StrToInt(mFg.ExtraeStringDeBuffer(&parametros)); // Obtiene el número de partidas
		for (i=0; i<num_partidas; i++) {
			val = i + 1;
			datos.AsignaTabla("dpedidosventa");
			parametros+=datos.InsCamposDesdeBuffer(parametros);

			AnsiString paramevaluaart;
			BufferRespuestas* resp_paramevaluaart=NULL;
			try {
				try {
					instruccion.sprintf("SELECT a.activo,a.producto, a.present, a.multiplo, pro.nombre  FROM articulos a \
					INNER JOIN productos pro ON a.producto=pro.producto WHERE a.articulo='%s'",
					datos.ObtieneValorCampo("articulo"));
					if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_paramevaluaart)) {
						if (resp_paramevaluaart->ObtieneNumRegistros()>0){
							 nombre = resp_paramevaluaart->ObtieneDato("nombre");
							 present = resp_paramevaluaart->ObtieneDato("present");
							 prod  = resp_paramevaluaart->ObtieneDato("producto");

							if(resp_paramevaluaart->ObtieneDato("activo") == 0){
								mensaje_articulo = "No se puede agregar el articulo '" + resp_paramevaluaart->ObtieneDato("producto") +
								" - " + resp_paramevaluaart->ObtieneDato("present") + " - "
								+ resp_paramevaluaart->ObtieneDato("multiplo") + "' por que esta inactivo" ;

								throw (Exception(mensaje_articulo));
							}
						} else throw (Exception("No se encuentra el artículo del renglón #" + IntToStr(val) +
								" con precio: " + datos.ObtieneValorCampo("precioimp") + " y cantidad: "
								+ datos.ObtieneValorCampo("cantidad") + ", probablemente fue eliminado y hay que eliminarlo del pedido para poder grabarlo."));
					} else throw (Exception("Error al consultar el artículo"));
				}
				catch(Exception &e) {

					AnsiString err=e.Message;
					int longitud_anterior;
					do {
						longitud_anterior=err.Length();
						cadenalimpia=StringReplace(err,"|"," ",TReplaceFlags()<<rfReplaceAll);
						cadenalimpia=StringReplace(err,"'"," ",TReplaceFlags()<<rfReplaceAll);
					} while(longitud_anterior!=err.Length());


					//mServidorVioleta->MuestraMensajeError( " Error catch ID_GRA_PEDIDO_CLI "+e.Message);
					//throw (Exception(e.Message));

				}
			} __finally {
				if (resp_paramevaluaart!=NULL) delete resp_paramevaluaart;
				if(cadenalimpia!=""){
                    AnsiString paramevaluaart;
					BufferRespuestas* resp_msjerror=NULL;
					msjerror.sprintf("select '%s' as mensaje ", cadenalimpia);
					try{
						if(mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, msjerror.c_str(), resp_msjerror) ){
							if (resp_msjerror->ObtieneNumRegistros()>0){
								errorfinal = resp_msjerror->ObtieneDato("mensaje");
							}
						}
					}__finally{
						if (resp_msjerror!=NULL) delete resp_msjerror;
                    }
				}
			}
			//aun se permite guardar cantidades en cero, asi que evitar esta concurrencia
			double cantidadsprod = datos.ObtieneValorCampo("cantidad").ToDouble();

			//valida que el parámetro PAGOACTCRED este activo para validación.
			double preciodet = datos.ObtieneValorCampo("precioimp").ToDouble();

			//aca se verificara que el precio sea 0,
			//porque puede venir de diferentes formularios y en algunos en ellos se llenacon 0
			//entonces al no conocer la procedencia, se validara que sea mayor a cero y posteriomente que sea
			//mayor que el preciominimo permitido
			if (preciodet !=0 ){
				//aqui validaremos que el precioimp sea mayor la precio minimo, si no es mayor
				//no se permitira guardar
				if (preciodet < mParamPrecioMin) {
					throw(Exception("El precio del prducto: \n"+nombre+" "+present+" \n es menor del precio mínimo permitido("+FloatToStr(mParamPrecioMin)+")"));
				}
			}
			if(mFg.EsCero(cantidadsprod)){
				throw(Exception("La cantidad del prducto: \n"+nombre+" "+present+" \n no debe ser cero. Favor de cambiar la cantidad"));
			}
			// +++ Inicio de sección para asegurar grabar los impuestos vigentes del artículo en caso
			// de que no se hayan mandado desde el cliente
			impuesto1=datos.ObtieneValorCampo("claveimp1");
			articulo=datos.ObtieneValorCampo("articulo");
			if (impuesto1=="") {
				////
				bool excluirIEPS=false;
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
							from articulos a, productos pro, presentacionescb pres \
							where a.articulo='%s' and a.producto=pro.producto \
							and a.present=pres.present and a.producto=pres.producto and pres.idempresa=%s ",
							articulo, FormServidor->ObtieneClaveEmpresa() );
						instrucciones[num_instrucciones++]=instruccion;
						// Asigna las variables a cada campo
						datos.ReemplazaCampo("claveimp1", "@imp1",1);
						datos.ReemplazaCampo("claveimp2", "@imp2",1);
						datos.ReemplazaCampo("claveimp3", "@imp3",1);
						datos.ReemplazaCampo("claveimp4", "@imp4",1);
						datos.ReemplazaCampo("costobase", "@cosbas",1);
				}else{
					// Pone las claves de los impuestos en variables mysql.
					instruccion.sprintf("select @imp1:=pro.claveimpv1, @imp2:=pro.claveimpv2, \
						@imp3:=pro.claveimpv3, @imp4:=pro.claveimpv4, \
						@cosbas:=pres.costobase \
						from articulos a, productos pro, presentacionescb pres \
						where a.articulo='%s' and a.producto=pro.producto \
						and a.present=pres.present and a.producto=pres.producto and pres.idempresa=%s ",
						articulo, FormServidor->ObtieneClaveEmpresa() );
					instrucciones[num_instrucciones++]=instruccion;
					// Asigna las variables a cada campo
					datos.ReemplazaCampo("claveimp1", "@imp1",1);
					datos.ReemplazaCampo("claveimp2", "@imp2",1);
					datos.ReemplazaCampo("claveimp3", "@imp3",1);
					datos.ReemplazaCampo("claveimp4", "@imp4",1);
					datos.ReemplazaCampo("costobase", "@cosbas",1);
				}
				////
			}
			// +++ Fin de sección para asegurar grabar impuestos

			// Cuando se indica que se debe usar el precio asignado al cliente
			if (usar_preciocliente=="1") {
				// Pone las el precio en variables de mysql
				instruccion.sprintf("SELECT @precioart:=pre.precio FROM clientes c \
					INNER JOIN clientesemp cet ON cet.cliente=c.cliente AND cet.idempresa=%s \
					INNER JOIN precios pre ON pre.tipoprec=cet.tipoprec \
					INNER JOIN tiposdeprecios tp ON tp.tipoprec=pre.tipoprec AND tp.idempresa=%s \
					INNER JOIN articulos a ON pre.articulo=a.articulo \
					WHERE c.cliente='%s' AND a.articulo='%s' ",
					FormServidor->ObtieneClaveEmpresa(), FormServidor->ObtieneClaveEmpresa(), cliente, articulo );
				instrucciones[num_instrucciones++]=instruccion;
				datos.ReemplazaCampo("precioimp", "@precioart",1);
			}

			datos.InsCampo("referencia", "@folio", 1);
			datos.InsCampo("id", mFg.IntToAnsiString(i));
			instrucciones[num_instrucciones++]=datos.GenerarSqlInsert();
		}

		/*
			se actualiza el campo de precio para validar que no se quede en NULL
		*/
		instruccion.sprintf(" update dpedidosventa as dp \
				left join impuestos i1 on i1.impuesto=dp.claveimp1 \
				left join impuestos i2 on i2.impuesto=dp.claveimp2 \
				set dp.precio=(dp.precioimp/if(dp.claveimp2>0, 1+i2.porcentaje/100,1))/if(dp.claveimp1>0, (1+i1.porcentaje/100),1) \
				where  precio IS NULL and dp.referencia=@folio");
				instrucciones[num_instrucciones++] = instruccion;

		/*
		  -----------------------------
		* despues de ingresado cabecera de ventas y detalle de la venta
		* DIRECCION DE ENTREGA
		  -----------------------------
		  1ra fase ... tarea="A" -> solo cuando sea un nuevo registro
		*/
		if (tarea=="A") {
			// en caso de que estos campo esten vacios(calle,numExt, colonia, cp) no guarde el regristro en
			// la tabla de pedidosdirent


			//forzar a que cuando venga vacio/nulo se agregue 0
			/*if(mDIRubicacionX.IsEmpty())
			  mDIRubicacionX=0;
			if(mDIRubicacionY.IsEmpty())
			  mDIRubicacionY=0;*/

			instruccion.sprintf("set @ubicacionent:=POINT(%s, %s) ", mDIRubicacionX,mDIRubicacionY);
			instrucciones[num_instrucciones++] = instruccion;

			instruccion.sprintf(" insert into pedidosdirent \
					(id_pedirent,referencia, tipo, cliente,calle ,numext ,numint , colonia,cp ,ubicaciongis ,referenciadom,fechaalta,fechamodi) \
					values \
					(NULL,@folio, 'Pedido','%s','%s','%s','%s','%s','%s', @ubicacionent,'%s', CURDATE(),CURDATE())",
			cliente.c_str(),mDIRcalle.c_str(),mDIRnumext.c_str(),
			mDIRnumint.c_str(),mDIRcolonia.c_str() ,mDIRcp.c_str(),
			mDIRreferencia_domicilio.c_str());
			instrucciones[num_instrucciones++] = instruccion;

		}
		if (tarea=="M") {
			instruccion.sprintf("set @ubicacionent:=POINT(%s, %s) ", mDIRubicacionX,mDIRubicacionY);
			instrucciones[num_instrucciones++] = instruccion;

			instruccion.sprintf(" update pedidosdirent set \
					calle='%s', numext='%s', numint='%s', colonia='%s', \
					cp='%s' ,ubicaciongis=@ubicacionent ,referenciadom='%s', fechamodi=CURDATE() \
					where  referencia=@folio AND cliente='%s' ",
					mDIRcalle.c_str(),mDIRnumext.c_str(),mDIRnumint.c_str(),mDIRcolonia.c_str() ,
					mDIRcp.c_str(),mDIRreferencia_domicilio.c_str(),
					cliente.c_str());
			instrucciones[num_instrucciones++] = instruccion;

        }

		// generar update para cuando se modifica el pedido y/o su dirección de entrega


		/*
		  * fin de direccion de entrega
		  -----------------------------------
		*/

		/*para actualizar el valor del pedido si es que es cero*/
		//nueva consulta del valor del pedido
		BufferRespuestas* resp_valorpedido=NULL;
		valor=0;
		//se creara un bufer para obtener el valor del parametro del valor del pedido
		//liberar la memoria ocupada de esta consulta
		try{
			instruccion.sprintf("SELECT * FROM pedidosventa WHERE referencia = @folio ");
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_valorpedido);
			if (resp_valorpedido->ObtieneNumRegistros()>0){
				valor=mFg.CadenaAFlotante(resp_valorpedido->ObtieneDato("valor")); //valor del pedido
			}
		}__finally{
			if (resp_valorpedido!=NULL) delete resp_valorpedido;
		}

		if (valor==0) {
			instruccion.sprintf("select @valor:=sum(precioimp*cantidad) from dpedidosventa where referencia=@folio");
			instrucciones[num_instrucciones++] = instruccion;

			instruccion.sprintf("update pedidosventa set valor=@valor where referencia=@folio");
			instrucciones[num_instrucciones++] = instruccion;

		}
        //--------------------------------
			// Pedidos de Kits
		//--------------------------------
		mCantidadKits=StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
		// Si se está modificando entonces borra los kits asosiados.
		if (tarea=="M" && mCantidadKits>0) {
			instruccion.sprintf("delete from pedidoskits where pedido=@folio");
			instrucciones[num_instrucciones++]=instruccion;
		}
		for (int i = 0; i < mCantidadKits; i++) {
			datos.AsignaTabla("pedidoskits");
			parametros+=datos.InsCamposDesdeBuffer(parametros);
			datos.InsCampo("pedido", "@folio",1);
			instrucciones[num_instrucciones++]=datos.GenerarSqlInsert();
		}
		//---------------------------------
			//Detalle de pedidos de Kits
		//---------------------------------
		mCantArtKits=StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
		// Si se está modificando entonces borra los articulos de kits asosiados.
		if (tarea=="M" && mCantArtKits>0) {
			instruccion.sprintf("delete from dpedidoskits where pedido=@folio");
			instrucciones[num_instrucciones++]=instruccion;
		}
		for (int i = 0; i <mCantArtKits; i++) {
			datos.AsignaTabla("dpedidoskits");
			parametros+=datos.InsCamposDesdeBuffer(parametros);
			datos.InsCampo("pedido", "@folio",1);
			instrucciones[num_instrucciones++]=datos.GenerarSqlInsert();
		}
		//----------------------------------
			//Fin de  pedidos y detalle de pedidos de Kits
		//----------------------------------

		instrucciones[num_instrucciones++]="COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			instruccion.sprintf("select 0 as error, @folio as folio, '%s' as errorFinal", errorfinal);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	} __finally {
		delete buffer_sql;
		delete[] instrucciones;
	}
}

//------------------------------------------------------------------------------
//ID_CANC_PEDIDO_CLI
void ServidorVentas::CancelaPedidoCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  CANCELA PEDIDO DE CLIENTE
	char *buffer_sql=new char[1024*32];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instrucciones[50], instruccion;
	int num_instrucciones=0;
	AnsiString clave, usuario;
	int i,error=0;
	TDate fecha=Today();
	TTime hora=Time();
	AnsiString producto, present, multiplo, nombre ;

	try{
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave de la venta (referencia)
		usuario=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que esta cancelando el pedido.

		//cuando se quiera cancelar un pedido a proveedor y en ella este algun producto inactivo, es necesario evitar
		//este resgistro, esto, con la finalidad de evitar productos incongruentes
		BufferRespuestas* buffer_resp_artinactivos=NULL;
		try {
			instruccion.sprintf("SELECT @error:=IF(a.activo=0, 1, 0) AS error, a.producto,\
					a.present, a.multiplo, a.factor,pro.nombre \
					FROM pedidosventa pv \
					LEFT JOIN dpedidosventa dpv ON dpv.referencia=pv.referencia \
					LEFT JOIN articulos a ON a.articulo=dpv.articulo \
					LEFT JOIN productos pro ON pro.producto=a.producto \
					WHERE pv.referencia='%s' AND a.activo=0",clave );
			if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), buffer_resp_artinactivos)) {
				if (buffer_resp_artinactivos->ObtieneNumRegistros()>0){
					producto = buffer_resp_artinactivos->ObtieneDato("producto");
					present  = buffer_resp_artinactivos->ObtieneDato("present");
					multiplo  = buffer_resp_artinactivos->ObtieneDato("multiplo");
					nombre  = buffer_resp_artinactivos->ObtieneDato("nombre");
					error=1;
					throw (Exception("Hay artículos inactivos en el detalle de pedidos a clientes a cancelar.\n \
					Favor de activar el artículo o quitarlo de la lista.\
					\n  Artículo:"+nombre+" "+present+" "+multiplo));
				}
			} else throw (Exception("Error al consultar detalle de los articulos inactivos en pedidos a clientes "));
		} __finally {
			if (buffer_resp_artinactivos!=NULL) delete buffer_resp_artinactivos;
		}

		if(error==0){
			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

			// Quita de la cola de impresion
			instruccion.sprintf("delete from colaimpresion where foliodoc='%s' and tipo=3", clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Cancela el pedido
			instruccion.sprintf("update pedidosventa set cancelado=1, fechamodi='%s', horamodi='%s', usumodi='%s' where referencia='%s' and cancelado=0", mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario, clave);
			instrucciones[num_instrucciones++]=instruccion;

			instrucciones[num_instrucciones++]="COMMIT";
		}

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
//ID_CON_PEDIDO_CLI
void ServidorVentas::ConsultaPedidoCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA PEDIDOS DE CLIENTES
	AnsiString instruccion;
	AnsiString clave, menos_devoluciones;
	AnsiString ordenMultiplo;
	//TStringList* mListaReferencias = new TStringList();


	clave=mFg.ExtraeStringDeBuffer(&parametros);
	ordenMultiplo=mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	/*mListaReferencias->Delimiter = '|';
	mListaReferencias->DelimitedText = clave;*/

	// Obtiene todos los generales (de cabecera) del pedido
	instruccion.sprintf("select p.*, p.cotizacion as tipodoc, \
		pm.mensaje, \
		col.nombre as nomcolonia, loc.nombre as nomlocalidad, \
		emp1.nombre as nomcajero, \
		emp2.nombre as nomvendedor, \
		emp3.nombre as nomusualta, \
		concat(emp3.nombre,' ',emp3.appat) as nomusualta, \
		concat(emp4.nombre,' ',emp4.appat) as nomchofer, \
		ifnull(kt.nombre,'') as nombrekit, IFNULL(kt.desglosado,1) as desglosadokit,  \
		p.tipoorigen, emb.concartaporte, p.cotzEspecial \
		from pedidosventa p  inner join  clientes c \
		left join colonias col on c.colonia=col.colonia \
		left join localidades loc on col.localidad=loc.localidad \
		left join empleados emp1 on p.usumodi=emp1.empleado \
		left join empleados emp2 on p.vendedor=emp2.empleado \
		left join embarques emb on emb.embarque=p.embarque \
		left join empleados emp3 on p.usualta=emp3.empleado \
		left join empleados emp4 on emb.chofer=emp4.empleado \
		left join kits kt on p.kit=kt.kit \
		left join pedidosmensajes pm on pm.referencia=p.referencia \
		where p.referencia='%s' and p.cliente=c.cliente", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene todos los datos del cliente del pedido
	instruccion.sprintf("select cli.* from clientes cli, pedidosventa p where p.referencia='%s' and p.cliente=cli.cliente", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Los pedidos siempre permiten modificarse (claro que depende si el usuario puede modificar)
	instruccion.sprintf("select 1 as modificar");
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene los datos detallados del pedido
	instruccion="select d.referencia, d.articulo, d.cantidad, d.costobase, ";
	instruccion+="d.porcdesc, d.precio, d.precioimp, a.factor, ";
	instruccion+="a.present, p.producto, p.nombre, a.multiplo, a.porccomi, ";
	instruccion+="d.claveimp1, i1.tipoimpu as tipoimpuesto1, i1.porcentaje as porcentajeimp1, t1.nombre as nomtipoimp1, ";
	instruccion+="d.claveimp2, i2.tipoimpu as tipoimpuesto2, i2.porcentaje as porcentajeimp2, t2.nombre as nomtipoimp2, ";
	instruccion+="d.claveimp3, i3.tipoimpu as tipoimpuesto3, i3.porcentaje as porcentajeimp3, t3.nombre as nomtipoimp3, ";
	instruccion+="d.claveimp4, i4.tipoimpu as tipoimpuesto4, i4.porcentaje as porcentajeimp4, t4.nombre as nomtipoimp4, ";
	instruccion+="(a.factor / pmm.cajalogisticafactor) AS cajasbultos ";
	if(ordenMultiplo=="1")
		instruccion+=",IF(a.factor=1,1,2) ORDEN,p.clasif1  "; //Agrupacion para ordenar por multiplo
	instruccion+=",IFNULL(dpk.kit,'') as kit ";
	instruccion+="from dpedidosventa d  inner join  articulos a  inner join  productos p ";
	instruccion+="left join impuestos i1 on i1.impuesto=d.claveimp1 ";
	instruccion+="left join impuestos i2 on i2.impuesto=d.claveimp2 ";
	instruccion+="left join impuestos i3 on i3.impuesto=d.claveimp3 ";
	instruccion+="left join impuestos i4 on i4.impuesto=d.claveimp4 ";
	instruccion+="left join tiposdeimpuestos t1 on t1.tipoimpu=i1.tipoimpu ";
	instruccion+="left join tiposdeimpuestos t2 on t2.tipoimpu=i2.tipoimpu ";
	instruccion+="left join tiposdeimpuestos t3 on t3.tipoimpu=i3.tipoimpu ";
	instruccion+="left join tiposdeimpuestos t4 on t4.tipoimpu=i4.tipoimpu ";
	instruccion+="left join dpedidoskits dpk on dpk.pedido=d.referencia and dpk.articulo=d.articulo ";
	instruccion+="LEFT JOIN presentacionesminmax pmm ON a.producto=pmm.producto AND a.present=pmm.present ";
	instruccion+="where d.referencia='";
	instruccion+=clave;
	instruccion+="' and d.articulo=a.articulo and a.producto=p.producto ";
	instruccion+=" group by p.nombre, a.present, a.multiplo ";
	if(ordenMultiplo=="1")
		instruccion+="ORDER BY ORDEN DESC,IF(ORDEN='1',p.clasif1,left(a.multiplo,3)),p.nombre, a.present, a.multiplo  ";
	else
		instruccion+=" order by d.id, p.nombre, a.present, a.multiplo ";

	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	//validar que tenga una direccion de entrega
	instruccion.sprintf("select * from pedidosventa where referencia='%s' \
						and referencia not in (select referencia from pedidosdirent)", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

    //Obtiene  los kist asosiados al pedido
	instruccion.sprintf("select pk.*,k.nombre from pedidoskits pk inner join kits k on k.kit=pk.kit \
						 where pk.pedido='%s'", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
 }
//------------------------------------------------------------------------------

//ID_GRA_DEVOL_CLI
void ServidorVentas::GrabaDevolCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros, bool factura_web
			, bool pendientegenerar, bool generardeticket)
{
	//  GRABA DEVOLUCION DE CLIENTE
	char *buffer_sql=new char[10000*1000];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString tarea_devol, clave_devol, clave_venta, usuario, terminal, tipo_ncredito;
	int num_partidas, i;
	int num_instrucciones=0;
	AnsiString instruccion;
	AnsiString * instrucciones = new AnsiString[10000];
	double valor;
	TDateTime fecha_dev;
	int error=0;
	TDate fecha=Today();
	TTime hora=Time();
	AnsiString articulo;
	AnsiString foliofisico;
	AnsiString parametrometodo="", metodopago="", digitos="";
	AnsiString folio_vta;
	AnsiString refCorte;
	AnsiString notaResumida="";

	try{
		clave_devol=mFg.ExtraeStringDeBuffer(&parametros); // Clave de la devolucion.
		foliofisico=mFg.ExtraeStringDeBuffer(&parametros); // Folio físico de la nota
		tarea_devol=mFg.ExtraeStringDeBuffer(&parametros); // Indicador si se va a agregar o modificar.
		folio_vta=mFg.ExtraeStringDeBuffer(&parametros); //Folio de venta para saber si cre nota de credito al cancelar ticket

		// Verificaciones para no permitir modificaciones cuando no sea permitido.
		if (tarea_devol=="M")
			throw Exception("Con la nueva facturación electrónica, ya no se deben modificar las notas de crédito, mejor cancele y vuelva a crear.");

		usuario=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que esta grabando la devolucion.
		terminal=mFg.ExtraeStringDeBuffer(&parametros); // Terminal que esta grabando la devolucion.
		/*
		*  aca se agregaran los nuevos paramaetroa para generar u no una nota de credito
		* segun el metodo de pago de la venta, donde se aplicara esta nota de credito
		*/
		 parametrometodo=mFg.ExtraeStringDeBuffer(&parametros);
		 metodopago=mFg.ExtraeStringDeBuffer(&parametros);
		digitos =mFg.ExtraeStringDeBuffer(&parametros);
		refCorte =mFg.ExtraeStringDeBuffer(&parametros);

		if(folio_vta == " "){
			// Obtiene los datos de la tabla de notas de crédito
			datos.AsignaTabla("notascredcli");
			parametros+=datos.InsCamposDesdeBuffer(parametros);
			// Extrae los datos que necesitamos para crear las letras y transacciones.
			datos.AsignaValorCampo("referencia", "@folionc", 1);
			valor=StrToFloat(datos.ObtieneValorCampo("valor"));
			clave_venta=datos.ObtieneValorCampo("venta");
			tipo_ncredito = datos.ObtieneValorCampo("tipo");
		}
		else
			clave_venta = folio_vta;

		fecha_dev=fecha;

		// Verificaciones para no permitir modificaciones cuando no sea permitido.
		if (tarea_devol=="M") {

			// Obtiene el folio de la venta correspondiente y el valor de la nota
			instruccion.sprintf("select @venta:=venta, @valor:=valor from notascredcli where referencia='%s'",clave_devol);
			mServidorVioleta->EjecutaSelectSqlNulo(Respuesta,  MySQL, instruccion.c_str());

			// Verifica que no haya notas de crédito posteriores de la misma venta
			instruccion.sprintf("select @error:=if((COALESCE(sum(t.valor),0))<0, 1, 0) as error from notascredcli n, transxcob t where n.venta=@venta and n.referencia>'%s' and n.venta=t.referencia and n.cancelado=0 and t.cancelada=0 and t.tipo='DEVO'", clave_devol);
			mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 2, error);

			// Verifica que la fecha de la nota sea posterior a la fecha de cierre.
			instruccion.sprintf("select @error:=if(n.fechanot<=cast(e.valor as datetime), 1, 0) as error from notascredcli n left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' where n.referencia='%s'",FormServidor->ObtieneClaveSucursal(), clave_devol);
			mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 3, error);

		} else {

			// Verifica que la fecha de la devolución sea posterior a la fecha de cierre.
			instruccion.sprintf("select @error:=if('%s'<=cast(e.valor as datetime), 1, 0) as error from estadosistemaemp e where e.estado='FUCIERRE' AND e.sucursal = '%s' ", mFg.DateToMySqlDate(fecha_dev), FormServidor->ObtieneClaveSucursal());
			mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 4, error);
		}


		if (error==0) {

			// Obtiene el contenido del campo acredito
			instruccion.sprintf("SELECT v.acredito FROM ventas v WHERE v.referencia='%s'", clave_venta);
			AnsiString venta_acredito;
			bool encontrada_venta_acredito;
			venta_acredito=mServidorVioleta->EjecutaSelectSqlCadUnica(Respuesta,  MySQL, instruccion.c_str(), encontrada_venta_acredito);
			if (!encontrada_venta_acredito)
				throw Exception("No se encontro el campo v.acredito en ID_GRA_DEVOL_CLI");

			// Inicia el conjunto de instrucciones que dan de alta la nota de crédito
			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

			instruccion.sprintf("select @seccion:=seccion, @depart:=depart, @asigfolncred:=asigfolncred, @anchofolncred:=anchofolncred from terminales where terminal='%s'", terminal);
			instrucciones[num_instrucciones++]=instruccion;

			// Obtiene el folio para la nota
			if (tarea_devol=="A") {
				// Folio físico.
				instruccion.sprintf("select @foliofisicaux:=folncred, @serieaux:=seriencred from terminales where terminal='%s'", terminal);
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliofisicsig=@foliofisicaux+1");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliofisic=if(@asigfolncred=0, '%s', if(@asigfolncred=1, concat(@serieaux, lpad(@foliofisicaux,@anchofolncred,'0') ), '') )", foliofisico);
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("update terminales t set t.folncred=@foliofisicsig where t.terminal='%s' and  @asigfolncred=1", terminal);
				instrucciones[num_instrucciones++]=instruccion;

				// Folio del sistema para la nota
				instruccion.sprintf("select @folioaux:=valor from foliosemp where folio='NOTASCRCLI' AND sucursal = '%s' %s ",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliosig=@folioaux+1");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @folioaux=cast(@folioaux as char)");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @folionc=concat('%s', lpad(@folioaux,9,'0'))", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("update foliosemp set valor=@foliosig where folio='NOTASCRCLI' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
			} else {
				instruccion.sprintf("set @foliofisic='%s'", foliofisico);
				instrucciones[num_instrucciones++]=instruccion;

				instruccion.sprintf("set @folionc='%s'", clave_devol);
				instrucciones[num_instrucciones++]=instruccion;
			}

			// Si se está modificando entonces borra el detalle
			if (tarea_devol=="M") {
				instruccion.sprintf("delete from dnotascredcli where referencia=@folionc");
				instrucciones[num_instrucciones++]=instruccion;
			}

			// Graba las partidas en "dnotascredcli"
			num_partidas=StrToInt(mFg.ExtraeStringDeBuffer(&parametros)); // Obtiene el número de partidas
			if(folio_vta == " "){

			// Graba la cabecera en la tabla "notascredcli"
			datos.InsCampo("foliofisic", "@foliofisic", 1);
			datos.InsCampo("usumodi", usuario);
			datos.InsCampo("fechamodi", mFg.DateToAnsiString(fecha));
			datos.InsCampo("horamodi", mFg.TimeToAnsiString(hora));
			if (tarea_devol=="A") {
				datos.InsCampo("fechanot",mFg.DateToAnsiString(fecha));
				datos.InsCampo("terminal", terminal);
				datos.InsCampo("usualta", usuario);
				datos.InsCampo("fechaalta", mFg.DateToAnsiString(fecha));
				datos.InsCampo("horaalta", mFg.TimeToAnsiString(hora));
				datos.InsCampo("cancelado", "0");
				notaResumida.sprintf("(SELECT ticket FROM ventas v WHERE v.referencia='%s')",clave_venta);
				datos.InsCampo("resumidofglobal", notaResumida);


				instrucciones[num_instrucciones++]=datos.GenerarSqlInsert();
			} else {
				instrucciones[num_instrucciones++]=datos.GenerarSqlUpdate("referencia=@folionc");
			}

			for (i=0; i<num_partidas; i++) {
				datos.AsignaTabla("dnotascredcli");
				parametros+=datos.InsCamposDesdeBuffer(parametros);
					datos.InsCampo("referencia", "@folionc", 1);
				articulo=datos.ObtieneValorCampo("articulo");

				AnsiString paramdifnc;
				AnsiString busqueda;
				double diferencia;
				BufferRespuestas* resp_paramdifnc=NULL;
				try {
					busqueda.sprintf("SELECT valor FROM parametrosemp where parametro = 'DIFNC' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
					if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, busqueda.c_str(), resp_paramdifnc)) {
							paramdifnc=resp_paramdifnc->ObtieneDato("valor");
					} else throw (Exception("Error al consultar en tabla parametros"));
				} __finally {
					if (resp_paramdifnc!=NULL) delete resp_paramdifnc;
				}

				if(paramdifnc == 1){
					diferencia = mFg.CadenaAFlotante(datos.ObtieneValorCampo("precioimp")) - mFg.CadenaAFlotante(datos.ObtieneValorCampo("precio"));
					if(diferencia < -0.05 )
						throw Exception("El precio con impuestos no puede ser menor que el precio sin impuestos.");
				}

	/*            // Obtiene el almacén del que se sacó el artículo.
				articulo=datos.ObtieneValorCampo("articulo");
				instruccion.sprintf("select @almacen:=almacen from dventas where referencia=@folio and articulo='%s'", articulo);
				instrucciones[num_instrucciones++]=instruccion;
				datos.InsCampo("almacen", "@almacen", 1);*/

				instrucciones[num_instrucciones++]=datos.GenerarSqlInsert();

				//Actualiza las existencias
				instruccion.sprintf("UPDATE articulos a INNER JOIN dnotascredcli d ON a.articulo = d.articulo  \
				INNER JOIN notascredcli n ON d.referencia = n.referencia AND n.tipo = 0  \
				INNER JOIN ventas v ON n.venta = v.referencia              \
				INNER JOIN dventas dv ON dv.referencia = n.venta AND dv.articulo = '%s'   \
				INNER JOIN  existenciasactuales ea ON a.producto = ea.producto AND a.present = ea.present \
				AND dv.almacen = ea.almacen \
				SET ea.cantidad = (ea.cantidad + (d.cantidad * a.factor)) \
				, ea.devventas = (ea.devventas + (d.cantidad * a.factor)) \
					WHERE a.articulo = '%s' AND d.referencia = @folionc",articulo,articulo, articulo);
				instrucciones[num_instrucciones++]=instruccion;
			}
			}
				else
					{
						instruccion.sprintf("SELECT IF(@foliofisic = '(null)','',@foliofisic) as foliofisic");
						AnsiString foliofisic;
						bool encontrada;
						foliofisic=mServidorVioleta->EjecutaSelectSqlCadUnica(Respuesta,  MySQL, instruccion.c_str(), encontrada);
						AnsiString cadena_corte;
						if (refCorte==" ")
							cadena_corte="NULL";
						else
							cadena_corte.sprintf("'%s'",refCorte);

						// Graba la cabecera en la tabla "notascredcli"
						String pendienteStr = "0";
						if(pendientegenerar){
						pendienteStr = "1";
						}

						instruccion.sprintf("INSERT INTO notascredcli VALUES (@folionc,'%s','%s','0',NULL,'0', \
						'%s','%s','%s','%s','%s','%s','%s',(SELECT valor FROM ventas WHERE referencia = '%s'), \
						NULL,NULL,'%s',%s,(SELECT ticket FROM ventas v WHERE v.referencia='%s'), '%s' )", foliofisic.c_str(),folio_vta,usuario, usuario,
						mFg.DateToMySqlDate(fecha),mFg.TimeToMySqlTime(hora),mFg.DateToMySqlDate(fecha),
						mFg.TimeToMySqlTime(hora),mFg.DateToMySqlDate(fecha),folio_vta,terminal,cadena_corte,folio_vta, pendienteStr);
						instrucciones[num_instrucciones++]=instruccion;

						// Graba las partidas en "dnotascredcli"
						instruccion.sprintf("INSERT INTO dnotascredcli (referencia, articulo, cantidad, precio,precioimp ) \
						SELECT @folionc, dv.articulo,dv.cantidad,dv.precio, dv.precioimp FROM ventas v INNER JOIN \
						dventas dv ON v.referencia = dv.referencia WHERE v.referencia = '%s'",folio_vta);
						instrucciones[num_instrucciones++]=instruccion;

						//Actualiza las existencias
						instruccion="update existenciasactuales ea, \
						(select SUM(d.cantidad * a.factor) as sumCantidad, a.producto , a.present, dv.almacen \
						FROM dnotascredcli d \
						INNER JOIN notascredcli n ON d.referencia = n.referencia AND n.tipo = 0 \
						INNER JOIN articulos a ON a.articulo = d.articulo \
						INNER JOIN ventas v ON n.venta = v.referencia \
						INNER JOIN dventas dv ON dv.referencia = v.referencia AND dv.articulo=d.articulo \
						WHERE d.referencia = @folionc \
						GROUP BY a.producto, a.present, dv.almacen \
						) as tmov \
						SET ea.cantidad = ea.cantidad + tmov.sumCantidad , ea.devventas =ea.devventas + tmov.sumCantidad \
						WHERE ea.producto=tmov.producto AND ea.present=tmov.present AND ea.almacen=tmov.almacen";
						instrucciones[num_instrucciones++]=instruccion;
					}

			// Modifica el precalculo de ventas mensuales
			int mes_modif;
			if (tarea_devol=="A") {

				//int mes_modif=MonthOf(fecha_dev)+12;

				if (mFg.IntToAnsiString(YearOf(fecha_dev))==mFg.IntToAnsiString(YearOf(fecha)))
					mes_modif=MonthOf(fecha_dev)+24;
				else
					mes_modif=MonthOf(fecha_dev)+12;

				// Notese que vm.ventascorte no se afecta porque tiene otro objetivo y por diseño es la contabilización de ventas desde el ultimo corte sin descontar notas
				instruccion.sprintf( \
					"UPDATE \
					  ventasxmes vm \
					  INNER JOIN \
						(SELECT dv.almacen, a.producto, a.present, SUM(dn.cantidad * a.factor) AS cantidad \
						FROM notascredcli n \
						  INNER JOIN dnotascredcli dn ON n.referencia=dn.referencia \
						  INNER JOIN articulos a ON a.articulo = dn.articulo \
						  INNER JOIN dventas dv ON n.venta=dv.referencia AND dv.articulo=dn.articulo \
						WHERE n.referencia = @folionc AND n.tipo='0' \
						GROUP BY dv.almacen, a.producto, a.present) ncre \
						ON vm.almacen = ncre.almacen \
						AND vm.producto = ncre.producto \
						AND vm.present = ncre.present SET vm.cant%s = vm.cant%s - ncre.cantidad, \
						vm.ventas30 = vm.ventas30 - ncre.cantidad, \
						vm.ventas60 = vm.ventas60 - ncre.cantidad, \
						vm.ventas90 = vm.ventas90 - ncre.cantidad, \
						vm.ventas180 = vm.ventas180 - ncre.cantidad ",
					mFg.IntToAnsiString(mes_modif), mFg.IntToAnsiString(mes_modif) );
				instrucciones[num_instrucciones++]=instruccion;
			}

			//Solo si la venta es a credito afectamos saldos
			if(venta_acredito == "1"){
				//Hace un abono que refleje en clientes la nota de crédito.
				double valor_final_transxcob = valor;

				if (tarea_devol=="A") {
					// Obtiene el folio para la NUEVA transaccion
					instruccion.sprintf("select @foliotranaux:=valor from foliosemp where folio='TRANCLI' AND sucursal = '%s' %s ",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
					instrucciones[num_instrucciones++]=instruccion;
					instruccion.sprintf("set @foliosigtran=@foliotranaux+1");
					instrucciones[num_instrucciones++]=instruccion;
					instruccion.sprintf("set @foliotranaux=cast(@foliotranaux as char)");
					instrucciones[num_instrucciones++]=instruccion;
					instruccion.sprintf("set @foliotran=concat('%s', lpad(@foliotranaux,9,'0'))", FormServidor->ObtieneClaveSucursal());
					instrucciones[num_instrucciones++]=instruccion;
					instruccion.sprintf("update foliosemp set valor=@foliosigtran where folio='TRANCLI' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
					instrucciones[num_instrucciones++]=instruccion;

					instruccion.sprintf("insert into transxcob \
						(tracredito, referencia, notacli, concepto, destino, tipo, cancelada, aplicada, valor, fechaalta, horaalta, fechamodi, horamodi, fechaapl, usualta, usumodi) \
						values (@foliotran, '%s', @folionc, 'A', 'C', 'DEVO', 0,1, -%12.2f, '%s', '%s', '%s', '%s', '%s', '%s', '%s')",
						 clave_venta, valor_final_transxcob, mFg.DateToMySqlDate(fecha_dev), mFg.TimeToMySqlTime(hora), mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), mFg.DateToMySqlDate(fecha_dev), usuario, usuario);
					instrucciones[num_instrucciones++]=instruccion;

					instruccion.sprintf("update ventasconsaldo set\
						saldo = saldo - %12.2f where referencia = '%s'",
						valor_final_transxcob,clave_venta);
					instrucciones[num_instrucciones++]=instruccion;

				} else {
					//se hace la operación de ventasconsaldo antes que en transxcob

					// Obtiene el folio de la transaccion ya existente
					instruccion.sprintf("select @foliotran:=tracredito from transxcob where referencia='%s' and notacli=@folionc and concepto='A' and destino='C' and tipo='DEVO' and cancelada=0", clave_venta);
					instrucciones[num_instrucciones++]=instruccion;
					instruccion.sprintf("update transxcob set valor=-%12.2f, fechamodi='%s', horamodi='%s', fechaapl='%s', usumodi='%s' where tracredito=@foliotran", valor_final_transxcob, mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), mFg.DateToMySqlDate(fecha_dev), usuario);
					instrucciones[num_instrucciones++]=instruccion;
				}
				//instrucciones[num_instrucciones++]="COMMIT";

			}
		}
		else{
			//Se ejecutara si se realizara la nota de credito al facturar un ticket de otro dia
			if(folio_vta != " "){
				throw Exception(IntToStr(error));
			}
		}

        int tipo_peticion = 0;
		   if(factura_web){
			instruccion.sprintf("INSERT INTO bitacoratimbradonotasdecreditoweb (id, fecha_solicitud, hora_solicitud, usuario, \
						terminal,referencia, solicitud, estatus                    \
						) VALUES (NULL, CURDATE(), CURTIME(), NULL, NULL, @folionc , 'FACURA_WEB', 0)");
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @codigobitacoraId=LAST_INSERT_ID()");
			instrucciones[num_instrucciones++]=instruccion;
			tipo_peticion = 3;
		   }
		   if(generardeticket){
		   instruccion.sprintf("INSERT INTO bitacoratimbradonotasdecreditoweb (id, fecha_solicitud, hora_solicitud, usuario, \
							terminal,referencia, solicitud, estatus                    \
							) VALUES (NULL, CURDATE(), CURTIME(),  '%s',  '%s', @folionc , 'GENERANOTA_CRED', 0)", usuario, terminal);
		   instrucciones[num_instrucciones++]=instruccion;
		   instruccion.sprintf("set @codigobitacoraId=LAST_INSERT_ID()");
		   instrucciones[num_instrucciones++]=instruccion;
		   tipo_peticion = 3;
		   factura_web=true;
		   }


		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			// Verifica que el saldo de la venta no quede negativo.
			instruccion.sprintf("SELECT @ERROR := IF(saldo < 0, 1, 0) AS ERROR FROM ventasconsaldo WHERE referencia = '%s'", clave_venta);
			mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 5, error);

			if (error == 0) {
				// Crea el CFD (si la configuracion así lo indica) y hace commit
				ComprobanteFiscalDigital *cfd=NULL;
				try {
					cfd=new ComprobanteFiscalDigital(mServidorVioleta);
					cfd->factWeb = factura_web;
					//nueva funcion de asignar metodo de pago temporalmente
					cfd->AsignaValores(parametrometodo, metodopago, digitos,"","","");

					if (cfd->EmitirCFDI40(Respuesta, MySQL, "NCRE")){
						// Le pasamos el 3 indicando que no tendra restricciones
						if(factura_web){
							cfd->CreaCFDINotaCredito40(Respuesta, MySQL, tipo_peticion, pendientegenerar);
						} else {
						   cfd->CreaCFDINotaCredito40(Respuesta, MySQL);
						}
					}
					/*else
						throw Exception(L"La versión del CFDI tipo NCRE ya no es soportada");*/
				} __finally {
					if(cfd!=NULL) delete cfd;
				}

				if(folio_vta == " "){
					instruccion.sprintf("select %d as error, @folionc as folio, @foliofisic as foliofisic", error);
					mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
				}
			} else {
				error=5;
				instruccion.sprintf("select %d as error", error);
                mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
				//El saldo resulto negativo, por lo tanto se ejecuta ROLLBACK
				instruccion="ROLLBACK";
				if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL, instruccion.c_str())) {
					throw Exception("No se pudo ejecutar el ROLLBACK, abortamos para evitar el COMMIT");
				}
			}
		}
	} __finally {
		delete buffer_sql;
		delete[] instrucciones;
	}
}

//------------------------------------------------------------------------------
//ID_GRA_NOTAS_CREDITO_WEB
void ServidorVentas::GrabaCfdiXmlWeb(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  GRABA DEVOLUCION DE CLIENTE
	char *buffer_sql=new char[2000*500];
	char *aux_buffer_sql=buffer_sql;
	AnsiString clave_devol,  usuario, terminal, peticionInd;

	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[500];
	double valor;
	TDateTime fecha_dev;
	int error=0;
	AnsiString parametrometodo="", metodopago="", digitos="";
	AnsiString folio_vta;

	try{
			clave_devol=mFg.ExtraeStringDeBuffer(&parametros); // Clave de la devolucion.
			peticionInd=mFg.ExtraeStringDeBuffer(&parametros); // Peticion Individual
			terminal=mFg.ExtraeStringDeBuffer(&parametros); // Terminal
			usuario=mFg.ExtraeStringDeBuffer(&parametros); //  Usuario

			folio_vta=" "; //Folio de venta para saber si cre nota de credito al cancelar ticket

			int peticionIndInt =StrToInt(peticionInd);

			if(peticionInd == "1"){   // Factura Global
			   peticionInd = "FACT_GLOBAL";
			} else if(peticionInd == "2"){  // Individual
				peticionInd = "INDIVIDUAL";
			}

			// Inicia el conjunto de instrucciones que dan de alta la nota de crédito
			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

			instruccion.sprintf("set @folionc='%s'", clave_devol);
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("INSERT INTO bitacoratimbradonotasdecreditoweb (id, fecha_solicitud, hora_solicitud, usuario, \
						terminal,referencia, solicitud, estatus                    \
						) VALUES (NULL, CURDATE(), CURTIME(),  '%s',  '%s', @folionc , '%s', 0)", usuario, terminal, peticionInd);
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("set @codigobitacoraId=LAST_INSERT_ID()");
			instrucciones[num_instrucciones++]=instruccion;

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (int i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			// Crea el CFD (si la configuracion así lo indica) y hace commit
			ComprobanteFiscalDigital *cfd=NULL;
			try {
				cfd=new ComprobanteFiscalDigital(mServidorVioleta);
				cfd->factWeb = true;
				//nueva funcion de asignar metodo de pago temporalmente
				cfd->AsignaValores(parametrometodo, metodopago, digitos,"","","");
				if (cfd->EmitirCFDI40(Respuesta, MySQL, "NCRE")){
				   cfd->CreaCFDINotaCredito40(Respuesta, MySQL,peticionIndInt, false);
				}
			} __finally {
				if(cfd!=NULL) delete cfd;
			}


			if(folio_vta == " "){
			// instruccion.sprintf("SELECT '0' as ERROR, 'LA000000186' as folio, 'LA000000186' as foliofisic");
			instruccion.sprintf("select %d as error, @folionc as folio, @foliofisic as foliofisic", error);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
			}

		}
	} __finally {
		delete buffer_sql;
	}
}

//------------------------------------------------------------------------------
//ID_CANC_DEVOL_CLI
void ServidorVentas::CancelaDevolCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  CANCELA DEVOLUCION DE CLIENTE
	char *buffer_sql=new char[1024*64];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instrucciones[50], instruccion;
	int num_instrucciones=0;
	AnsiString clave, usuario,pac;
	int i, error=0;
	TDate fecha=Today();
	TTime hora=Time();
	BufferRespuestas* resp_fecha_not=NULL;
    AnsiString producto, present, multiplo, nombre ;

	try{
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave de la devolución
		usuario=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que esta cancelando la devolución.
		pac=mFg.ExtraeStringDeBuffer(&parametros);// validar el pac

		// Obtiene el folio de la venta correspondiente y el valor de la nota
		instruccion.sprintf("select @venta:=venta, @valor:=valor from notascredcli where referencia='%s'",clave);
		mServidorVioleta->EjecutaSelectSqlNulo(Respuesta,  MySQL, instruccion.c_str());

		// Verifica que no haya notas de crédito posteriores de la misma venta
		instruccion.sprintf("select @error:=if((COALESCE(sum(t.valor),0))<0, 1, 0) as error from notascredcli n, transxcob t where n.venta=@venta and n.referencia>'%s' and n.venta=t.referencia and n.cancelado=0 and t.cancelada=0 and t.tipo='DEVO'", clave);
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 2, error);

		// Verifica que la fecha de la nota sea posterior a la fecha de cierre.
		instruccion.sprintf("select @error:=if(n.fechanot<=cast(e.valor as datetime), 1, 0) as error from notascredcli n left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' where n.referencia='%s'",FormServidor->ObtieneClaveSucursal(), clave);
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 3, error);


		//cuando se quiera cancelar una nota de credito cliente y en ella este algun producto inactivo, es necesario evitar
		//este resgistro, esto, con la finalidad de evitar productos incongruentes
		BufferRespuestas* buffer_resp_artinactivos=NULL;
		try {
			instruccion.sprintf("SELECT @error:=IF(a.activo=0, 1, 0) AS error,              \
							a.producto, a.present, a.multiplo, a.factor,pro.nombre, nc.tipo \
							FROM notascredcli nc                                            \
							LEFT JOIN dnotascredcli dnc ON dnc.referencia=nc.referencia     \
							LEFT JOIN articulos a ON a.articulo=dnc.articulo                \
							LEFT JOIN productos pro ON pro.producto=a.producto              \
							WHERE nc.referencia='%s' AND a.activo=0",clave );
			if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), buffer_resp_artinactivos)) {
				if (buffer_resp_artinactivos->ObtieneNumRegistros()>0){
					producto = buffer_resp_artinactivos->ObtieneDato("producto");
					present  = buffer_resp_artinactivos->ObtieneDato("present");
					multiplo = buffer_resp_artinactivos->ObtieneDato("multiplo");
					nombre   = buffer_resp_artinactivos->ObtieneDato("nombre");
					error=4;
					throw (Exception("Hay artículos inactivos en el detalle de nota crédito cliente a cancelar.\n Favor de activar el artículo o quitarlo de la lista.\n  Artículo:"+nombre+" "+present+" "+multiplo));
				}
			} else throw (Exception("Error al consultar detalle de los articulos inactivos en nota de crédito cliente "));
		} __finally {
			if (buffer_resp_artinactivos!=NULL) delete buffer_resp_artinactivos;
		}

		if (error==0) {
			// Cancela el CFDI con el PAC (si no se trata de un CFDI simplemente no se hace nada con ningún PAC)

			if(pac=="0"){
			//con el formulario de forzar la cancelación, esta variable de PAC, tendra el valor==1
			//lo que significa que se ha cancelado directamente en el portal del SAT, solo
			//hace falta cancelarlo a nivel sistema.
			//por ello, se saltara llas siguentes lineas, porque ya ha sido cancelado
                // SOLO CUANDO NO ESTA EN DEPURACION
				#ifndef _DEBUG
				ComprobanteFiscalDigital *cfd=NULL;
				try {
					cfd=new ComprobanteFiscalDigital(mServidorVioleta);
					cfd->cancelarCFDI(Respuesta, MySQL, "NCRE",clave, "", "02", "");
				} __finally {
					if(cfd!=NULL) delete cfd;
				}
				#endif
			}
			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

			// Quita de la cola de impresion
			instruccion.sprintf("delete from colaimpresion where foliodoc='%s' and tipo=1", clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Marca como cancelado
			instruccion.sprintf("update notascredcli set cancelado=1, usumodi='%s', fechamodi='%s', horamodi='%s' where referencia='%s' and cancelado=0", usuario, mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Cancela el CFD
			instruccion.sprintf("update cfd set estado=0, fechacancelamin=curdate(), fechacancela='%s %s' where referencia='%s' and tipocomprobante='NCRE'", mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Cancela el abono correspondiente de la devolución
			instruccion.sprintf("select @foliotran:=tracredito from transxcob where notacli='%s' and concepto='A' and destino='C' and tipo='DEVO' and cancelada=0", clave);
			instrucciones[num_instrucciones++]=instruccion;
			/**/instruccion.sprintf("update transxcob set cancelada=1, fechamodi='%s', horamodi='%s', usumodi='%s' where tracredito=@foliotran", mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario );
			instrucciones[num_instrucciones++]=instruccion;

            instruccion.sprintf("SELECT @val:=ABS(valor), @ref:=referencia FROM transxcob WHERE notacli='%s' ", clave);
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("UPDATE ventasconsaldo SET saldo = saldo + @val WHERE referencia = @ref");
			instrucciones[num_instrucciones++]=instruccion;

			//actualizar existencia
			instruccion="CREATE temporary TABLE tmpcantidad(referencia VARCHAR(11) NOT NULL,\
			producto VARCHAR(8) NOT NULL, present VARCHAR(255) NOT NULL, almacen VARCHAR(4) NOT NULL,  \
			cantidad DECIMAL(12,3) NOT NULL, PRIMARY KEY (referencia, producto, present, almacen))";
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("INSERT INTO tmpcantidad SELECT d.referencia, a.producto, a.present, dv.almacen,   \
			SUM(d.cantidad * a.factor) AS cantidad FROM dnotascredcli d INNER JOIN notascredcli n ON  \
			n.referencia = d.referencia AND n.tipo='0' INNER JOIN articulos a ON a.articulo = d.articulo \
			INNER JOIN ventas v ON n.venta = v.referencia AND n.tipo = 0 INNER JOIN dventas dv ON n.venta = dv.referencia \
			AND dv.articulo = a.articulo WHERE d.referencia = '%s' GROUP BY a.producto, a.present, dv.almacen ", clave);
			instrucciones[num_instrucciones++]=instruccion;

			instruccion = "UPDATE tmpcantidad tmp INNER JOIN existenciasactuales ea ON ea.producto = tmp.producto \
			AND ea.present = tmp.present AND ea.almacen = tmp.almacen SET ea.cantidad = (ea.cantidad  - tmp.cantidad) \
			, ea.devventas = (ea.devventas - tmp.cantidad) ";
			instrucciones[num_instrucciones++]=instruccion;

			// Modifica el precalculo de ventas mensuales
			int mes_modif;
			TDate fecha_not;
			instruccion.sprintf("select fechanot from notascredcli where referencia='%s' ", clave);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_fecha_not);
			fecha_not=StrToDate(resp_fecha_not->ObtieneDato("fechanot"));

			if (YearOf(fecha_not)>=YearOf(fecha)-1) {
				// Solo afecta a ventas de los ultimos dos años
				/*if (YearOf(fecha_not)==YearOf(fecha))
					mes_modif=MonthOf(fecha_not)+12;
				else
					mes_modif=MonthOf(fecha_not); */

				if (mFg.IntToAnsiString(YearOf(fecha_not))==mFg.IntToAnsiString(YearOf(fecha)))
					mes_modif=MonthOf(fecha_not)+24;
				else
					mes_modif=MonthOf(fecha_not)+12;

				instruccion.sprintf( \
					"UPDATE \
					  ventasxmes vm \
					  INNER JOIN \
						(SELECT dv.almacen, a.producto, a.present, SUM(dn.cantidad * a.factor) AS cantidad, n.fechanot \
						FROM notascredcli n \
						  INNER JOIN dnotascredcli dn ON n.referencia=dn.referencia \
						  INNER JOIN articulos a ON a.articulo = dn.articulo \
						  INNER JOIN dventas dv ON n.venta=dv.referencia AND dv.articulo=dn.articulo \
						WHERE n.referencia = '%s' AND n.tipo='0' \
						GROUP BY dv.almacen, a.producto, a.present) ncre \
						ON vm.almacen = ncre.almacen \
						AND vm.producto = ncre.producto \
						AND vm.present = ncre.present \
						SET vm.cant%s = vm.cant%s + ncre.cantidad, \
						vm.ventas30 = IF(ncre.fechanot>=DATE_SUB(CURDATE(), INTERVAL 30 DAY) AND ncre.fechanot<=CURDATE(),vm.ventas30 + ncre.cantidad, vm.ventas30), \
						vm.ventas60 = IF(ncre.fechanot>=DATE_SUB(CURDATE(), INTERVAL 60 DAY) AND ncre.fechanot<=CURDATE(),vm.ventas60 + ncre.cantidad, vm.ventas60), \
						vm.ventas90 = IF(ncre.fechanot>=DATE_SUB(CURDATE(), INTERVAL 90 DAY) AND ncre.fechanot<=CURDATE(),vm.ventas90 + ncre.cantidad, vm.ventas90), \
						vm.ventas180 = IF(ncre.fechanot>=DATE_SUB(CURDATE(), INTERVAL 180 DAY) AND ncre.fechanot<=CURDATE(),vm.ventas180 + ncre.cantidad, vm.ventas180) ",
					clave, mFg.IntToAnsiString(mes_modif), mFg.IntToAnsiString(mes_modif) );
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
			instruccion.sprintf("select %d as error", error);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	} __finally {
		delete buffer_sql;
		if (resp_fecha_not!=NULL) delete resp_fecha_not;
	}
}

//------------------------------------------------------------------------------
//ID_CON_DEVOL_CLI
void ServidorVentas::ConsultaDevolCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA NOTAS DE CREDITO DE CLIENTES
	AnsiString instruccion;
	AnsiString clave;

	clave=mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los generales (de cabecera) de la nota de credito.
	instruccion.sprintf("select notascredcli.*, @folioventa:=notascredcli.venta, \
		cfd.folio as cfdfolio, cfd.serie as cfdserie, cfdxml.xmlgenerado as cfdxml, cfdxml.cadenaoriginal as cfdcadenaoriginal, \
		cfd.version, cfd.muuid, cfd.pactimbrador \
		from notascredcli \
		left join cfd on notascredcli.referencia=cfd.referencia and cfd.tipocomprobante='NCRE' \
		left join cfdxml on cfd.compfiscal=cfdxml.compfiscal \
		where notascredcli.referencia='%s'", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene los datos de la venta respectiva.
	instruccion.sprintf("select v.*, \
		if(ifnull(v.foliofisic,'')='',concat(ifnull(cfd.serie,''),ifnull(cfd.folio,'')),v.foliofisic) as foliofisocfd \
		from ventas v \
		left join cfd on v.referencia=cfd.referencia and cfd.tipocomprobante='VENT' \
		where v.referencia=@folioventa");
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Calcula el saldo de la venta (SALDO B)
	instruccion.sprintf("select sum(if(cancelada=0,valor,0)) as saldo from transxcob where referencia=@folioventa group by referencia");
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene todos los datos del cliente de la nota de credito.
	instruccion.sprintf("select cli.*, CONCAT(rf.regimenfiscal, ' - ', rf.descripcion) AS regimenfis \
	 from clientes cli \
	 inner join ventas v ON v.cliente=cli.cliente \
	 left join cregimenfiscal rf ON cli.regimenfiscal=rf.regimenfiscal \
	 where v.referencia=@folioventa ");
    mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

    // NULO:  Verifica que la fecha de la nota de credito sea posterior a la fecha de cierre.
    instruccion.sprintf("select if(n.fechanot<=cast(e.valor as datetime), 1, 0) as \
	modificar from notascredcli n left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' where n.referencia='%s'",FormServidor->ObtieneClaveSucursal(), clave);
    mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Suma las notas de crédito y deja el resultado en una tabla temporal,
    // para luego hacerle un left join con las ventas.
	instruccion="create temporary table  dnotascredcliaux ";
    instruccion+="select d.articulo, sum(if(n.tipo='0',d.cantidad,0)) as cantidad, ";
	instruccion+="sum(if(n.tipo<>'0',d.precio,0)) as precio, ";
	instruccion+="sum(if(n.tipo<>'0',d.precioimp,0)) as precioimp ";
	instruccion+="from notascredcli n, dnotascredcli d ";
	instruccion+="where n.venta=@folioventa";
	instruccion+=" and n.cancelado=0 and n.referencia=d.referencia ";
    instruccion+="group by d.articulo";
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query EjecutaSelectSqlNulo"));

	// Sumatoria de todas las notas de crédito.
	instruccion.sprintf("select ifnull(sum(valor),0) as sumnotas from notascredcli where cancelado=0 and venta=@folioventa");
    mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

    // Obtiene los datos de lo que resta de las ventas
    // menos las notas de credito ya aplicadas.
    instruccion="select dv.referencia, dv.articulo, sum(dv.cantidad-ifnull(dn.cantidad,0)) as cantidad, ";
    instruccion+="sum(dv.precio-ifnull(dn.precio,0)) as precio, ";
    instruccion+="sum(dv.precioimp-ifnull(dn.precioimp,0)) as precioimp, ";
	instruccion+="a.present, p.producto, p.nombre, a.multiplo, ";
    instruccion+="dv.claveimp1, i1.tipoimpu as tipoimpuesto1, i1.porcentaje as porcentajeimp1, t1.nombre as nomtipoimp1, ";
    instruccion+="dv.claveimp2, i2.tipoimpu as tipoimpuesto2, i2.porcentaje as porcentajeimp2, t2.nombre as nomtipoimp2, ";
    instruccion+="dv.claveimp3, i3.tipoimpu as tipoimpuesto3, i3.porcentaje as porcentajeimp3, t3.nombre as nomtipoimp3, ";
    instruccion+="dv.claveimp4, i4.tipoimpu as tipoimpuesto4, i4.porcentaje as porcentajeimp4, t4.nombre as nomtipoimp4 ";
    instruccion+="from ventas v  inner join  dventas dv inner join  articulos a inner join  productos p ";
    instruccion+="left join impuestos i1 on i1.impuesto=dv.claveimp1 ";
    instruccion+="left join impuestos i2 on i2.impuesto=dv.claveimp2 ";
    instruccion+="left join impuestos i3 on i3.impuesto=dv.claveimp3 ";
    instruccion+="left join impuestos i4 on i4.impuesto=dv.claveimp4 ";
    instruccion+="left join tiposdeimpuestos t1 on t1.tipoimpu=i1.tipoimpu ";
    instruccion+="left join tiposdeimpuestos t2 on t2.tipoimpu=i2.tipoimpu ";
    instruccion+="left join tiposdeimpuestos t3 on t3.tipoimpu=i3.tipoimpu ";
    instruccion+="left join tiposdeimpuestos t4 on t4.tipoimpu=i4.tipoimpu ";
    instruccion+="left join dnotascredcliaux dn on dn.articulo=dv.articulo ";
    instruccion+="where v.referencia=@folioventa";
	instruccion+=" and v.referencia=dv.referencia ";
    instruccion+="and dv.articulo=a.articulo and a.producto=p.producto ";
    instruccion+="group by dv.articulo";
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene todo el detalle de la nota con algunos datos extras que necesita el cliente
	instruccion="select dn.referencia, dn.articulo, dn.cantidad, dn.precio, dn.precioimp, ";
	instruccion+="a.present, p.producto, p.nombre, a.multiplo, dn.clave_motivo, ";
	instruccion+="IFNULL(dn.empleado_responsable, 'SIN EMPLEADO') as empleado_responsable, ";
	instruccion+="dv.claveimp1, i1.tipoimpu as tipoimpuesto1, i1.porcentaje as porcentajeimp1, t1.nombre as nomtipoimp1, ";
    instruccion+="dv.claveimp2, i2.tipoimpu as tipoimpuesto2, i2.porcentaje as porcentajeimp2, t2.nombre as nomtipoimp2, ";
	instruccion+="dv.claveimp3, i3.tipoimpu as tipoimpuesto3, i3.porcentaje as porcentajeimp3, t3.nombre as nomtipoimp3, ";
	instruccion+="dv.claveimp4, i4.tipoimpu as tipoimpuesto4, i4.porcentaje as porcentajeimp4, t4.nombre as nomtipoimp4 ";
	instruccion+="from notascredcli nc inner join  dnotascredcli dn inner join  dventas dv inner join  articulos a inner join  productos p ";
	instruccion+="left join impuestos i1 on i1.impuesto=dv.claveimp1 ";
	instruccion+="left join impuestos i2 on i2.impuesto=dv.claveimp2 ";
	instruccion+="left join impuestos i3 on i3.impuesto=dv.claveimp3 ";
	instruccion+="left join impuestos i4 on i4.impuesto=dv.claveimp4 ";
	instruccion+="left join tiposdeimpuestos t1 on t1.tipoimpu=i1.tipoimpu ";
	instruccion+="left join tiposdeimpuestos t2 on t2.tipoimpu=i2.tipoimpu ";
	instruccion+="left join tiposdeimpuestos t3 on t3.tipoimpu=i3.tipoimpu ";
	instruccion+="left join tiposdeimpuestos t4 on t4.tipoimpu=i4.tipoimpu ";
	instruccion+="where nc.referencia='";
	instruccion+=clave;
	instruccion+="' and nc.venta=dv.referencia and nc.referencia=dn.referencia ";
	instruccion+=" and dv.articulo=dn.articulo and dn.articulo=a.articulo ";
	instruccion+=" and a.producto=p.producto and a.producto=p.producto";
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);




}

//------------------------------------------------------------------------------
//ID_GRA_PAGO_CLI
void ServidorVentas::GrabaPagoCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  GRABA PAGO DE CLIENTE
	char *buffer_sql=new char[1024*64*50];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	int i, num_transacciones;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[5100];
	AnsiString clave_pago, tarea_pago;
	AnsiString cliente, identificador, forma_pago, valor, ajuste;
	AnsiString num_cheque, banco_cheque, tipo_cheque, fecha_cobro_cheque, usuario, cobrador;
	AnsiString folio_venta_sistema, valor_tran, folio_tran;
	AnsiString aplicada="1", estado_cheque;
	BufferRespuestas* resp_verificacion=NULL;
	BufferRespuestas* resp_param_actcfdipagos=NULL;
	BufferRespuestas* resp_ocupa_cfdipago=NULL;
	bool ocupa_cfdipago=false;
	int error;
	TDate fecha=Today();
	TTime hora=Time();
	AnsiString terminal, bancos_movbancos, bancos_NumCta, bancos_fechabancos;
    AnsiString prepago;

	AnsiString valor_banco, valor_tran_banco;
	double valor_banco_double, valor_tran_banco_double, valor_tran_banco_acumulado_double=0, ajuste_double, ajuste_double_restante;

	AnsiString archivo_temp1 = "", archivo_temp2 = "";

	try{
		// Grabar pago de cliente
		clave_pago=mFg.ExtraeStringDeBuffer(&parametros);
		tarea_pago=mFg.ExtraeStringDeBuffer(&parametros);
		cliente=mFg.ExtraeStringDeBuffer(&parametros);
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
		cobrador=mFg.ExtraeStringDeBuffer(&parametros);

		terminal=mFg.ExtraeStringDeBuffer(&parametros);
		bancos_movbancos=mFg.ExtraeStringDeBuffer(&parametros);
		bancos_NumCta=mFg.ExtraeStringDeBuffer(&parametros);
		bancos_fechabancos=mFg.ExtraeStringDeBuffer(&parametros);

		// El valor de banco se obtiene sumando al valor del pago el ajuste (puede ser negativo el ajuste)
		ajuste_double=mFg.CadenaAFlotante(ajuste);
		valor_banco_double=mFg.CadenaAFlotante(valor)+ajuste_double;
		valor_banco=mFg.FormateaCantidad(valor_banco_double, 2, false);
		ajuste_double_restante=fabs(ajuste_double);  // Le quita el signo a ajuste_double_restante en caso de ajustes negativos.


		// Consulta parametros de bancos
		AnsiString paramautomat;
		AnsiString idmovbancoant;
		TDate paramfechaini;
		BufferRespuestas* resp_parambancos=NULL;
		AnsiString paramautoconsolidado;
		try {
			instruccion.sprintf("SELECT (SELECT valor FROM parambancos WHERE parametro='AUTOMAT') AS paramautomat, \
								(SELECT valor FROM parambancos WHERE parametro='FECHAINI') AS paramfechaini, \
								(SELECT valor FROM parametrosemp WHERE parametro='AUTOCONSOLIDADO' AND sucursal = '%s' ) AS paramautoconsol, \
								(SELECT bt.idmovbanco FROM bancosxcob bc inner join transxcob t on t.tracredito=bc.tracredito inner join bancostransacc bt on bt.transacc=bc.transacc where t.pago='%s' limit 1) as idmovbancoant", FormServidor->ObtieneClaveSucursal(), clave_pago);
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

				/*instruccion.sprintf("SELECT @sucursaltran:=sec.sucursal from transxcob t \
						inner join ventas v on t.referencia=v.referencia \
						inner join terminales ter on v.terminal=ter.terminal \
						inner join secciones sec on sec.seccion=ter.seccion \
					where t.tracredito=@foliotran " );
				instrucciones[num_instrucciones++]=instruccion;*/

				// Se insertan registros nuevos en bancosmov,
				// ALTA EN BANCOS
				if (bancos_movbancos=="0") {
					// Si viene folio 0 entonces se asigna un nuevo movimiento bancario
					instruccion.sprintf("insert into bancosmov \
						(idmovbanco, idnumcuenta, conceptomov, descripcion, identificador, cancelado, aplicado, \
						subtotal, ivabanco, total, fechaaplbanco, fechaalta, horaalta, fechamodi, horamodi, usualta, usumodi, terminal, origen, sucursal, afectacion) \
						values (NULL, %s, 'D', 'PAGO DE CLIENTE', '%s', 0, 1, \
						%s, 0.00, %s, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', 'PACLI', '%s', 'C')",
						bancos_NumCta, identificador,
						valor_banco, valor_banco, mFg.StrToMySqlDate(bancos_fechabancos),
						mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora),
						mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora),
						usuario, usuario, terminal, FormServidor->ObtieneClaveSucursal());
					instrucciones[num_instrucciones++]=instruccion;

					instruccion.sprintf("set @idmovbanconuevo=LAST_INSERT_ID()");
					instrucciones[num_instrucciones++]=instruccion;
				} else {
					// Si viene folio diferente de 0 entonces se usa dicho folio.
					instruccion.sprintf("set @idmovbanconuevo=%s", bancos_movbancos);
					instrucciones[num_instrucciones++]=instruccion;

				}

			}

			instruccion.sprintf("select @folioaux:=valor from foliosemp where folio='PAGCLI' AND sucursal = '%s' %s ",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @foliosig=@folioaux+1");
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @folioaux=cast(@folioaux as char)");
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @folio=concat('%s', lpad(@folioaux,9,'0'))", FormServidor->ObtieneClaveSucursal());
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("update foliosemp set valor=@foliosig where folio='PAGCLI' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("insert into pagoscli \
				(pago, ident, cliente, fecha, hora, fechamodi, horamodi, valor, cancelado, usualta, usumodi, ajuste, formapag, cobrador,terminal) \
				values (@folio, '%s', '%s', '%s', '%s', '%s', '%s', %s, 0, '%s', '%s', %s, '%s', '%s', '%s')",
				identificador, cliente, mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora),
				mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), valor,
				usuario, usuario, ajuste, forma_pago, cobrador,terminal);
			instrucciones[num_instrucciones++]=instruccion;

			if (forma_pago=="C") {
				// Si es cheque entonces crea un registro en la tabla de cheques.
				instruccion.sprintf("select @foliocheqaux:=valor from foliosemp where folio='CHEQCLI' AND sucursal = '%s' %s",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliocheqsig=@foliocheqaux+1");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliocheqaux=cast(@foliocheqaux as char)");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliocheq=concat('%s', lpad(@foliocheqaux,9,'0'))", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("update foliosemp set valor=@foliocheqsig where folio='CHEQCLI' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("insert into chequesclientes \
					(chequecli,folio,banco,fechaalta,fechacob,valor,cliente,estado,clasif) \
					values (@foliocheq, '%s', '%s', '%s', '%s', %s, '%s', '%s','%s')",
					num_cheque, banco_cheque, mFg.DateToMySqlDate(fecha),
					mFg.StrToMySqlDate(fecha_cobro_cheque), valor, cliente, estado_cheque, tipo_cheque);
				instrucciones[num_instrucciones++]=instruccion;

				// Crea la relación cheque-pago
				instruccion="insert into cheqxcob (chequecli,pago) values (@foliocheq, @folio)";
				instrucciones[num_instrucciones++]=instruccion;
			}

		} else {
			// MODIFICACION
			instruccion.sprintf("set @folio='%s'", clave_pago);
			instrucciones[num_instrucciones++]=instruccion;

			// Al modificar un pago, en PAGOSCLI solo se actualizan los campos FECHAMODI, HORAMODI y USUMODI e IDENTIFICADOR
			instruccion.sprintf("update pagoscli set fechamodi='%s', horamodi='%s', usumodi='%s', ident='%s' where pago=@folio and cancelado=0", mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario, identificador);
			instrucciones[num_instrucciones++]=instruccion;

			// Marca como canceladas todas las transacciones, para
			// posteriormente descancelar solo a las que el usuario haga modificaciones.
			/**/instruccion.sprintf("update transxcob set cancelada=1, fechamodi='%s', horamodi='%s', usumodi='%s' where transxcob.pago=@folio", mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario );
			instrucciones[num_instrucciones++]=instruccion;

			if (forma_pago!="E" && idmovbancoant!="" && idmovbancoant!="0") {
				instruccion.sprintf("set @idmovbanconuevo=%s", idmovbancoant);
				instrucciones[num_instrucciones++]=instruccion;

				// El auxiliar de bancos solo se actualiza cuando el pago original ya tenía registro de bancos.
				// Tanto para alta como modificacion de pagos se insertan registros nuevos en bancosmov,
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
				instruccion.sprintf("SELECT @transaccBorrar:=bxc.transacc, @tracreditoBorrar:=bxc.tracredito, t.pago \
					FROM bancosxcob bxc \
					INNER JOIN transxcob t ON bxc.tracredito=t.tracredito \
					WHERE t.pago=@folio AND bxc.transacc IN \
						(SELECT transacc FROM bancostransacc WHERE idmovbanco=@idmovbanconuevo) ");
				instrucciones[num_instrucciones++]=instruccion;

				instruccion.sprintf("delete from bancosxcob where transacc=@transaccBorrar");
				instrucciones[num_instrucciones++]=instruccion;

				instruccion.sprintf("delete from bancostransacc where transacc=@transaccBorrar");
				instrucciones[num_instrucciones++]=instruccion;

			}

		}

		num_transacciones=StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
		for (i=0; i<num_transacciones; i++) {
			folio_tran=mFg.ExtraeStringDeBuffer(&parametros);
			folio_venta_sistema=mFg.ExtraeStringDeBuffer(&parametros);
			valor_tran=mFg.ExtraeStringDeBuffer(&parametros);

			if (folio_tran=="?") {
				// Crea cada transacción
				instruccion.sprintf("select @foliotranaux:=valor from foliosemp where folio='TRANCLI' AND sucursal = '%s' %s ",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliotransig=@foliotranaux+1");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliotranaux=cast(@foliotranaux as char)");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliotran=concat('%s', lpad(@foliotranaux,9,'0'))", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("update foliosemp set valor=@foliotransig where folio='TRANCLI' AND sucursal = '%s'  ", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("insert into transxcob \
					(tracredito, referencia, pago, concepto, destino, tipo, \
					cancelada, aplicada, valor, fechaalta, horaalta, fechamodi, horamodi, fechaapl, usualta, usumodi) values \
					(@foliotran, '%s', @folio, 'A', 'C', 'PAGO', \
					0, %s, -%s, '%s', '%s', '%s', '%s', '%s', '%s', '%s')",
					folio_venta_sistema, aplicada, valor_tran, mFg.DateToMySqlDate(fecha),
					mFg.TimeToMySqlTime(hora), mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora),
					mFg.DateToMySqlDate(fecha), usuario, usuario);
				instrucciones[num_instrucciones++]=instruccion;

				instruccion.sprintf("update ventasconsaldo set\
					saldo=saldo - '%s' where referencia = '%s'",
					valor_tran,folio_venta_sistema);
				instrucciones[num_instrucciones++]=instruccion;

			} else {
				// Aqui no es necesario actualizar la fecha de modificación ni el usuario
				// que modificó debido a que ya se hizo al cancelar todos los abonos que forman parte del pago.
				instruccion.sprintf("update transxcob set cancelada=0 where tracredito='%s'", folio_tran);
				instrucciones[num_instrucciones++]=instruccion;

				instruccion.sprintf("set @foliotran='%s'", folio_tran);
				instrucciones[num_instrucciones++]=instruccion;
			}

			// Si está activada automatización de bancos y la fecha actual es igual o posterior a la fecha programada de inicio de registro automático
			// Los registros de bancostransacc y bancosxcob se borran en las modificaciones, por lo que también aplica el volver a crearlos.
			if (forma_pago!="E" && ((idmovbancoant!="" && idmovbancoant!="0") || (paramautomat=="1" && fecha>=paramfechaini))) {
				AnsiString identificador_detalle="";
				if (forma_pago=="C") {
					identificador_detalle=num_cheque+"-"+banco_cheque;
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
				instruccion.sprintf("insert into bancosxcob (transacc, tracredito, valorconsiderado ) \
					values (@transacc, @foliotran, %s)", valor_tran);
				instrucciones[num_instrucciones++]=instruccion;

			}

		}

		prepago=mFg.ExtraeStringDeBuffer(&parametros);
		if (prepago!=" ") {
			if(tarea_pago=="A"){
				// Actualiza el prepago a aplicado
				instruccion.sprintf("UPDATE prepagoscli set aplicado = 1 WHERE pago = '%s' ", prepago);
				instrucciones[num_instrucciones++]=instruccion;

				// Actualiza el prepago a aplicado
				instruccion.sprintf("UPDATE prepagoscli set referencia = @folio WHERE pago = '%s' ", prepago);
				instrucciones[num_instrucciones++]=instruccion;

				// Actualiza el prepago a aplicado, le asigna el identificador que se le dio desde cliente
				instruccion.sprintf("UPDATE prepagoscli set identificador = '%s' WHERE pago = '%s' ", identificador, prepago);
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

		// Cuando se trató de agregar un detalle a un movimiento ya existente actualiza los totales con la sumatoria de los detalles.
		if (forma_pago!="E" && bancos_movbancos!="0") {
			instruccion.sprintf("SELECT @totalmov:=SUM(total) AS total, @subtotalmov:=SUM(subtotal) AS subtotal, @ivabancomov:=SUM(ivabanco) AS ivabanco FROM bancostransacc WHERE idmovbanco=@idmovbanconuevo");
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("update bancosmov \
				set \
				subtotal=@subtotalmov, total=@totalmov, ivabanco=@ivabancomov, fechamodi='%s', horamodi='%s', usumodi='%s' \
				where idmovbanco=@idmovbanconuevo ",
				mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora),
				usuario);
			instrucciones[num_instrucciones++]=instruccion;
		}


		////////////////// INICIO CALCULO DE SALDOS DE VENTAS

		// Crea una tabla para almacenar los folios de las ventas afectadas por el pago
		// para posteriormente recalcular saldos de estas ventas.
		instruccion="create temporary table ventasaux (venta varchar(11), PRIMARY KEY (venta)) Engine = InnoDB";
		instrucciones[num_instrucciones++]=instruccion;

        archivo_temp1 = mServidorVioleta->ObtieneArchivoTemp
				(num_instrucciones, Respuesta->Id);

		//instruccion="insert into ventasaux (venta) ";

		instruccion.sprintf("select t.referencia as venta from transxcob t where \
		t.pago=@folio and t.cancelada=0 INTO OUTFILE '%s' ",archivo_temp1);
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE ventasaux (venta) ",
				archivo_temp1);
			instrucciones[num_instrucciones++] = instruccion;

		// Crea una tabla donde se van a poner los saldos de las ventas
		// afectadas por la cancelación
		instruccion="create temporary table auxventassaldos (venta varchar(11), saldo decimal(16,2), numpagos int(11), PRIMARY KEY (venta)) Engine = InnoDB";
		instrucciones[num_instrucciones++]=instruccion;

		// Calcula los saldos de las ventas relacionadas con el pago (SALDO B)
		//instruccion="insert into auxventassaldos (venta, saldo, numpagos) ";

		archivo_temp2 = mServidorVioleta->ObtieneArchivoTemp
				(num_instrucciones, Respuesta->Id);

		instruccion.sprintf("select v.referencia as venta, sum(t.valor) as saldo, sum(if(t.tipo='PAGO',1,0)) as numpagos \
		from ventas v, ventasaux vaux, transxcob t  \
		where v.referencia=vaux.venta and	\
		t.referencia=v.referencia and t.cancelada=0 and v.cancelado=0 \
		group by v.referencia INTO OUTFILE '%s' ",archivo_temp2);
		instrucciones[num_instrucciones++]=instruccion;

        instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE auxventassaldos (venta, saldo, numpagos) ",
				archivo_temp2);
			instrucciones[num_instrucciones++] = instruccion;

		////////////////// FIN CALCULO DE SALDOS DE VENTAS

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			// VERIFICA SI QUEDO ALGUN SALDO NEGATIVO DE LAS FACTURAS EN CUESTION
			//
			instruccion="select * from auxventassaldos where saldo<0";
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_verificacion);
			if (resp_verificacion->ObtieneNumRegistros()==0) {
				error=0;

				// Consulta de parámetro que determina si está activa emisión de CFDI de pagos
				AnsiString param_actcfdipagos;
				instruccion.sprintf("SELECT valor FROM parametrosglobemp WHERE parametro='ACTCFDIPAGOS' AND idempresa = %s ", FormServidor->ObtieneClaveEmpresa());
				if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_param_actcfdipagos)) {
					if (resp_param_actcfdipagos->ObtieneNumRegistros()>0){
						param_actcfdipagos=resp_param_actcfdipagos->ObtieneDato("valor");
					} else throw (Exception("No se encuentra registro ACTCFDIPAGOS en tabla parametrosglobemp"));
				} else throw (Exception("Error al consultar en tabla parametrosglobemp"));

				if (param_actcfdipagos=="1") {
					// Lista si hay facturas generadas con CFDI 3.3 o 4.0 con pago en parcialidades para ver si ocupa complemento de pago
					// AND cfd.version='3.3' or cfd.version='3.3' AND cfd.metodopago33='PPD'
					instruccion.sprintf( "SELECT \
						  t.tracredito, t.referencia, t.pago, t.concepto, t.tipo, t.cancelada, @valorpago33:=ifnull(sum(t.valor * - 1),0) AS valorpago33, \
						  p.cliente, p.valor AS valorpago, cfd.seriefolio, cfd.muuid \
						FROM \
						  transxcob t \
						  INNER JOIN pagoscli p ON p.pago=t.pago \
						  INNER JOIN ventas v \
						  INNER JOIN cfd ON v.referencia=cfd.referencia AND cfd.tipocomprobante='VENT' AND (cfd.version='3.3' or cfd.version='4.0') AND cfd.metodopago33='PPD' \
						WHERE t.referencia = v.referencia \
						  AND t.pago = @folio \
						  AND t.cancelada = 0" );
					if (!mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_ocupa_cfdipago))
						throw Exception("No se pudo revisar si ocupa CFDI de pago");
					if (resp_ocupa_cfdipago->ObtieneNumRegistros()!=0) {
						if (mFg.EsCero(resp_ocupa_cfdipago->ObtieneDato("valorpago33")))
							ocupa_cfdipago=false;
						else
							ocupa_cfdipago=true;
					} else
						ocupa_cfdipago=false;
				}


				// Crea el CFDI de complemento de pagos (si la configuracion así lo indica) y hace commit
				if (ocupa_cfdipago) {
					ComprobanteFiscalDigital *cfd=NULL;
					try {
							cfd=new ComprobanteFiscalDigital(mServidorVioleta);
							if (cfd->EmitirCFDI40(Respuesta, MySQL, "PAGO")) {
								// Solo se emite el CFDI de pago si se ocupa, es decir si tiene pagos a ventas emitidas con CFDI 3.3
								cfd->CreaCFDIPago40(Respuesta, MySQL);
							} else {
								cfd->CreaCFDIPago33(Respuesta, MySQL);
								// Cuando no está configurado para emitir CFDI 3.3 y se ocupa emitir cfdi de pago entonces manda excepción.
								//throw Exception("Se ocupa CFDI de pago pero el sistema no está configurado para emitir versión 3.3 o 4.0");
							}
					} __finally {
						if(cfd!=NULL) delete cfd;
					}
				} else {
					instruccion="COMMIT";
					if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL, instruccion.c_str())) {
						throw Exception("No se pudo ejecutar el COMMIT al grabar el pago ");
					}
				}


			} else {
				error=1;
				// Sí quedó saldo negativo entonces Se ejecuta el rollback
				instruccion="ROLLBACK";
				if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL, instruccion.c_str())) {
					throw Exception("No se pudo ejecutar el ROLLBACK, abortamos para evitar el COMMIT");
				}
			}

			// Prepara resultado de grabar pago.
			instruccion.sprintf("select %d as error, @folio as pago", error);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
			if (error==1) {
				instruccion="select * from auxventassaldos where saldo<0";
				mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
			}

		}
		if (archivo_temp1 != "")
			mServidorVioleta->BorraArchivoTemp(archivo_temp1);
		if (archivo_temp2 != "")
			mServidorVioleta->BorraArchivoTemp(archivo_temp2);
	} __finally {
		if(resp_verificacion!=NULL) delete resp_verificacion;
		if(resp_ocupa_cfdipago!=NULL) delete resp_ocupa_cfdipago;
		if(resp_param_actcfdipagos!=NULL) delete resp_param_actcfdipagos;
		delete buffer_sql;
	}
}

//------------------------------------------------------------------------------
//ID_CON_PAGO_CLI
void ServidorVentas::ConsultaPagoCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA PAGO DE CLIENTE
	AnsiString instruccion;
	AnsiString clave;

	clave=mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los generales del pago
	instruccion.sprintf("select p.*, concat(emp.nombre,' ',emp.appat,' ',emp.apmat) as nomusualta, \
	col.nombre as nomcolonia, loc.nombre as nomlocalidad, \
	cfd.folio as cfdfolio, cfd.serie as cfdserie, cfdxml.xmlgenerado as cfdxml, cfdxml.cadenaoriginal as cfdcadenaoriginal, \
	cfd.version, cfd.muuid, cfd.pactimbrador, cfd.metodopago, cfd.digitos, pp.pago AS prepago, pp.fecha AS fechaprepago \
	from pagoscli p \
	inner join clientes c ON p.cliente=c.cliente \
	inner join empleados emp ON p.usualta=emp.empleado \
	left join cfd on p.pago=cfd.referencia and cfd.tipocomprobante='PAGO' \
	left join cfdxml on cfd.compfiscal=cfdxml.compfiscal \
	left join colonias col on c.colonia=col.colonia \
	left join localidades loc on col.localidad=loc.localidad \
	left join prepagoscli pp on pp.referencia=p.pago \
	where p.pago='%s' ", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene todos los datos del cheque en cuestión
	instruccion.sprintf("select chequesclientes.* \
		from chequesclientes, cheqxcob \
		where cheqxcob.pago='%s' and \
		cheqxcob.chequecli=chequesclientes.chequecli", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene todas las transacciones correspondientes al pago.
	instruccion.sprintf("select t.tracredito, t.referencia, t.notacli, t.pago, t.concepto, tfv.descripcion as tipoventa, \
		t.tipo, v.fechavta, t.cancelada, (t.valor*-1) as valor, t.fechaalta, t.horaalta, t.fechamodi, t.usualta, t.usumodi, \
		if(ifnull(v.foliofisic,'')='',concat(ifnull(cfd.serie,''),ifnull(cfd.folio,'')),v.foliofisic) as foliofisic, cfd.version, cfd.muuid,cfd.rfcreceptor,cfd.sucursal \
		from transxcob t inner join ventas v inner join tiposfacturasventas tfv \
		left join cfd on v.referencia=cfd.referencia and cfd.tipocomprobante='VENT' \
		where t.referencia=v.referencia and t.pago='%s' and t.cancelada=0 and v.tipofac=tfv.tipo", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Verifica que la fecha del pago no sea posterior a la fecha de cierre.
	instruccion.sprintf("select @error:=if((ifnull(chc.fechacob, '1900-01-01')<=cast(e.valor as datetime) \
		and chc.estado=ifnull(chc.estado, 'C')), 1, 0) \
		as error from pagoscli p  inner join  cheqxcob chxc  inner join  chequesclientes chc \
		left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' where p.pago='%s' \
		and chxc.pago=p.pago and chxc.chequecli=chc.chequecli",FormServidor->ObtieneClaveSucursal(), clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene todos los datos de bancos
	instruccion.sprintf("SELECT bm.idmovbanco, bcue.idnumcuenta, bcue.numerocuenta, bm.fechaaplbanco, \
								bm.origen, bcob.valorconsiderado, bcue.banco, bcue.descripcion  \
		FROM bancosxcob bcob \
		INNER JOIN bancostransacc bt ON bcob.transacc=bt.transacc \
		INNER JOIN bancosmov bm ON bt.idmovbanco=bm.idmovbanco \
		INNER JOIN bancoscuentas bcue ON bcue.idnumcuenta=bm.idnumcuenta \
		INNER JOIN transxcob t ON t.tracredito=bcob.tracredito \
		WHERE t.pago='%s' and bm.aplicado=1 and bm.cancelado=0 GROUP BY  bm.idmovbanco ORDER BY bm.idmovbanco", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

}

//------------------------------------------------------------------------------
//ID_CANC_PAGO_CLI
void ServidorVentas::CancelaPagoCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  CANCELA PAGO DE CLIENTES
	char *buffer_sql=new char[1024*32];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instrucciones[50], instruccion;
	int num_instrucciones=0;
	AnsiString clave, usuario, estado, terminal, f_cancelar, prepago;
	int i, error=0;
	TDate fecha=Today();
	TTime hora=Time();
	bool resultado_cancelacion_paq=false;

	AnsiString validacion, refvalida = "";
	bool validatransacc = false;
	BufferRespuestas* resp_validapagos=NULL;

	try{
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave del pago a cancelar.
		usuario=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que está cancelando.
		estado=mFg.ExtraeStringDeBuffer(&parametros);   // Estado con el que quedará el cheque al ser cancelado,
														// en el caso de pagos en efectivo se debe recibir ""
		terminal=mFg.ExtraeStringDeBuffer(&parametros);
		f_cancelar =mFg.ExtraeStringDeBuffer(&parametros);// parametro de forzar cancelar pago de clientes
		prepago=mFg.ExtraeStringDeBuffer(&parametros);


		// Verifica que la fecha del pago no sea posterior a la fecha de cierre.
		instruccion.sprintf("select @error:=if((ifnull(chc.fechacob, '1900-01-01')<=cast(e.valor as datetime) \
			and chc.estado=ifnull(chc.estado, 'C')), 1, 0) \
			as error from pagoscli p  inner join  cheqxcob chxc  inner join  chequesclientes chc \
			left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' where p.pago='%s' \
			and chxc.pago=p.pago and chxc.chequecli=chc.chequecli",FormServidor->ObtieneClaveSucursal(), clave);
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 1, error);

		try{
			validacion.sprintf("SELECT bpe.idbancospagosefe  				\
				FROM bancosxcob bc                                              \
					INNER JOIN transxcob t ON t.tracredito=bc.tracredito        \
					INNER JOIN bancostransacc bt ON bt.transacc=bc.transacc     \
					INNER JOIN pagoscli p ON p.pago=t.pago                      \
					INNER JOIN bancosmov bm ON bt.idmovbanco = bm.idmovbanco    \
					LEFT JOIN bancosxpagosefe bpe ON bpe.idmovbanco = bt.idmovbanco \
				WHERE t.pago='%s' AND p.formapag='E' AND bm.cancelado = 0", clave);
			if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, validacion.c_str(), resp_validapagos)) {
					if (resp_validapagos->ObtieneNumRegistros()>0){
						validatransacc = true;
						refvalida = resp_validapagos->ObtieneDato("idbancospagosefe");
					}
						else
							validatransacc = false;
			}
			else throw (Exception("Error al consultar los pagos"));
		}
		__finally {
			if (resp_validapagos!=NULL) delete resp_validapagos;
		}

		if(validatransacc)
			throw Exception("No se puede eliminar el pago en efectivo por tener movimientos bancarios relacionados. Si quiere cancelar el pago, entonces primero cancele en el módulo llamado 'Relación de pagos en efectivo con movs. bancarios' la relación con folio:" + refvalida);


		if (error==0) {

			// Cancela el CFDI con el PAC
			// (si no se trata de un CFDI simplemente no se hace nada con ningún PAC)
			// Si no tiene CFDI relacionado tampoco se hace nada con ningún PAC.
			// SOLO CUANDO NO ESTA EN DEPURACION
		   if (f_cancelar == "0"){
				#ifndef _DEBUG
				ComprobanteFiscalDigital *cfd=NULL;
				try {
					cfd=new ComprobanteFiscalDigital(mServidorVioleta);
					cfd->cancelarCFDI(Respuesta, MySQL, "PAGO",clave,"","02","");
					// ObtieneResultadoCancelacion() solo regresa true cuando realmente se canceló el CFDI con el paq.
					// Por ejemplo cuando no se encontró registro a cancelar no hay error, pero se regresa false, esto
					// para asegurar si realmente ocurrió una cancelación con el PAQ
					// (en caso de sustitución de CFDI cancelados esto se requiere)
					resultado_cancelacion_paq=cfd->ObtieneResultadoCancelacion();
				} __finally {
					if(cfd!=NULL) delete cfd;
				}
				#endif


				// Cuando estamos en depuráción marca como sí ya se hubiera cancelado correctamente con el PAQ (para pruebas).
				#ifdef _DEBUG
				resultado_cancelacion_paq=true;
				#endif
			}
			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

			instruccion.sprintf("update pagoscli set cancelado=1, fechamodi='%s', horamodi='%s', usumodi='%s' where pago='%s' and cancelado=0", mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario, clave);
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("update transxcob set cancelada=1, aplicada=0, fechamodi='%s', horamodi='%s', usumodi='%s' where pago='%s' and cancelada=0", mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario, clave);
			instrucciones[num_instrucciones++]=instruccion;

			/*edit se le regresa el valor del pago a ventasconsaldo*/
			instruccion.sprintf("UPDATE ventasconsaldo vs \
				INNER JOIN transxcob tr ON vs.referencia=tr.referencia \
					AND tr.tipo = 'PAGO'\
				SET vs.saldo = saldo + (tr.valor*-1) \
				WHERE vs.referencia IN \
				(SELECT referencia FROM transxcob\
				WHERE pago = '%s' and tipo = 'PAGO')",clave);
				instrucciones[num_instrucciones++]=instruccion;

			if (estado!="") {
				instruccion.sprintf("update chequesclientes, cheqxcob \
					set chequesclientes.estado='%s' \
					where cheqxcob.pago='%s' and chequesclientes.chequecli=cheqxcob.chequecli", estado, clave);
				instrucciones[num_instrucciones++]=instruccion;
			}

			// *********************************************************************************************
			// ------ INICIO Cancela el movimiento o suprime las transacciones parciales segun corresponda.
			//        (EXCEPTO EN PAGOS EN EFECTIVO).
			instruccion.sprintf("SELECT @idmovbancoanterior:=bt.idmovbanco, @ajuste:=p.ajuste FROM bancosxcob bc \
				inner join transxcob t on t.tracredito=bc.tracredito \
				inner join bancostransacc bt on bt.transacc=bc.transacc \
				INNER JOIN pagoscli p ON p.pago=t.pago \
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
				  INNER JOIN bancosxcob bxc ON bt.transacc=bxc.transacc \
				  INNER JOIN transxcob txc ON bxc.tracredito=txc.tracredito AND txc.pago='%s' \
				  WHERE bt.idmovbanco=@idmovbancoanterior", clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Borra las transacciones de bancosxcob correspondientes a las transacciones del movimiento
			instruccion.sprintf("delete from bancosxcob where transacc in (select transacc from bancostransacctmp)");
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

			// Cancela el CFD
			instruccion.sprintf("update cfd set estado=0, fechacancelamin=curdate(), fechacancela='%s %s' where referencia='%s' and tipocomprobante='PAGO'", mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Inserta en la tabla de pendientes de sustituir. Los que cumplen las siguientes condiciones:
			// Que tengan un CFDI de pago y que dicho cfd este timbrado
			if (resultado_cancelacion_paq==true) {
				/*instruccion.sprintf("select @compfiscalcancelado:=cfd.compfiscal FROM cfd where referencia='%s' and tipocomprobante='PAGO'", clave);
				instrucciones[num_instrucciones++]=instruccion;

				instruccion.sprintf("INSERT INTO cfdsustitucion \
					(compfiscal, sustituido, compfiscalsustituto, fechasustitucion, fechasustitucionmin) \
					VALUES (@compfiscalcancelado, '0', null, '%s %s', '%s') ",
					mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), mFg.DateToMySqlDate(fecha));
				instrucciones[num_instrucciones++]=instruccion;
				*/
				instruccion.sprintf("INSERT INTO cfdsustitucion \
					(compfiscal, sustituido, compfiscalsustituto, fechasustitucion, fechasustitucionmin) \
					select cfd.compfiscal, '0' as sustituido, \
					null as compfiscalsustituto, '%s %s' as fechasustitucion, '%s' as fechasustitucionmin \
						FROM cfd where referencia='%s' and tipocomprobante='PAGO' ",
					mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), mFg.DateToMySqlDate(fecha), clave);
				instrucciones[num_instrucciones++]=instruccion;

			}



			// ------ FIN Cancela el movimiento o suprime las transacciones parciales segun corresponda.
			// *********************************************************************************************


			/*
			// ------ INICIO Sección que inserta el movimiento bancario contrario al del pago original
			//        (EXCEPTO EN PAGOS EN EFECTIVO).

			instruccion.sprintf("SELECT @idmovbancoanterior:=bt.idmovbanco FROM bancosxcob bc \
				inner join transxcob t on t.tracredito=bc.tracredito \
				inner join bancostransacc bt on bt.transacc=bc.transacc \
				INNER JOIN pagoscli p ON p.pago=t.pago \
				where t.pago='%s' AND p.formapag<>'E' limit 1", clave);
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("INSERT INTO bancosmov \
				(idmovbanco, idnumcuenta, conceptomov, descripcion, identificador, cancelado, aplicado, subtotal, \
					ivabanco, total, fechaaplbanco, fechaalta, horaalta, fechamodi, horamodi, usualta, usumodi, terminal, origen, sucursal, afectacion) \
				SELECT NULL, idnumcuenta, conceptomov, 'CANCELA PAGO DE CLIENTE' as descripcion, identificador, cancelado, aplicado, subtotal, \
					ivabanco, total, fechaaplbanco, fechaalta, horaalta, '%s' as fechamodi, '%s' as horamodi, usualta, '%s' as usumodi, '%s' as terminal, 'CANPC', \
					'%s' as sucursal, 'A' as afectacion \
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
					FROM bancostransacc bt \
					  INNER JOIN bancosxcob bxc ON bt.transacc=bxc.transacc \
					  INNER JOIN transxcob txc ON bxc.tracredito=txc.tracredito AND txc.pago='%s' \
				where idmovbanco=@idmovbancoanterior", clave );
			instrucciones[num_instrucciones++]=instruccion;


			instruccion="create temporary table transxcobtmp ( \
				idtemp int(11)  AUTO_INCREMENT, \
				tracredito varchar(11) not null, \
				valorconsiderado decimal(16,2) not null, \
				PRIMARY KEY (idtemp) ) Engine = InnoDB";
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("insert into transxcobtmp (idtemp, tracredito, valorconsiderado) \
				SELECT NULL as idtemp, bxc.tracredito, bxc.valorconsiderado \
				  FROM bancostransacc bt \
				  INNER JOIN bancosxcob bxc ON bt.transacc=bxc.transacc \
				  INNER JOIN transxcob txc ON bxc.tracredito=txc.tracredito AND txc.pago='%s' \
				  WHERE bt.idmovbanco=@idmovbancoanterior", clave);
			instrucciones[num_instrucciones++]=instruccion;

			instruccion="create temporary table bancostransacctmp ( \
				idtemp int(11)  AUTO_INCREMENT, \
				transacc int(11), \
				PRIMARY KEY (idtemp) ) Engine = InnoDB";
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("insert into bancostransacctmp (idtemp, transacc) \
				SELECT NULL as idtemp, bt.transacc \
				  FROM bancostransacc bt \
				  WHERE bt.idmovbanco=@idmovbanconuevo");
			instrucciones[num_instrucciones++]=instruccion;

			instruccion="INSERT INTO bancosxcob \
					(transacc, tracredito, valorconsiderado) \
				SELECT b.transacc, t.tracredito, t.valorconsiderado \
					FROM transxcobtmp t \
					inner join bancostransacctmp b on b.idtemp=t.idtemp";
			instrucciones[num_instrucciones++]=instruccion;

			// ------ FIN Sección que inserta el movimiento bancario contrario al del pago original.
			*/
			if (prepago!="") {
				instruccion.sprintf("UPDATE prepagoscli SET aplicado = 0 WHERE pago = '%s' ", prepago);
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
			instruccion.sprintf("select %d as error", error);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	} __finally {
		delete buffer_sql;
	}
}

//------------------------------------------------------------------------------
//ID_MODIF_FECHA_PAGO_CLI
void ServidorVentas::ModificaFechaPagoCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  MODIFICA LA FECHA DE UN PAGO DE CLIENTES
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
			as error from pagoscli p INNER JOIN estadosistemaemp AS e \
			ON e.estado = 'FUCIERRE' AND e.sucursal = '%s' \
			where p.pago='%s' ",FormServidor->ObtieneClaveSucursal(), clave);
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 1, error);

		// Verifica que la fecha nueva sea posterior a la fecha de cierre.
		instruccion.sprintf("select @error:=if('%s'<=cast(e.valor as datetime), 1, 0) as error from estadosistemaemp e where e.estado='FUCIERRE' AND e.sucursal = '%s' ", mFg.DateToMySqlDate(StrToDate(fecha_nueva)), FormServidor->ObtieneClaveSucursal());
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 2, error);

		if (error==0) {
			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

			instruccion.sprintf("update pagoscli set fecha='%s', fechamodi='%s', horamodi='%s', usumodi='%s' where pago='%s' and cancelado=0", mFg.StrToMySqlDate(fecha_nueva), mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario, clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Marca las transacciones como aplicadas y cambia las fechas.
			instruccion.sprintf("update transxcob set aplicada=1, fechaapl='%s', fechaalta='%s', fechamodi='%s', horamodi='%s', usumodi='%s' where pago='%s' and cancelada=0", mFg.StrToMySqlDate(fecha_nueva) , mFg.StrToMySqlDate(fecha_nueva) ,mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario, clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Si existe cheque lo marca consolidado y modifica sus fechas.
			instruccion.sprintf("update chequesclientes, cheqxcob \
				set chequesclientes.estado='C', chequesclientes.fechacob='%s', chequesclientes.fechaalta='%s' \
				where cheqxcob.pago='%s' and chequesclientes.chequecli=cheqxcob.chequecli and chequesclientes.estado<>'X'",  mFg.StrToMySqlDate(fecha_nueva),  mFg.StrToMySqlDate(fecha_nueva), clave);
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
//ID_CON_PAGOS_CLI_DIA
void ServidorVentas::ConsultaPagosCliDelDia(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    // CONSULTA PAGOS DE CLIENTE DEL DIA
    AnsiString instruccion;
	AnsiString cobrador, fecha, formapag, tipocheque, revision_tipo_cheque, mostrar_todas_las_formas=" ", sucursal;
	AnsiString mostrar_por_cobrador=" ", condicion_sucursal=" ", join_sucursal=" ";

	fecha=mFg.ExtraeStringDeBuffer(&parametros);
    formapag=mFg.ExtraeStringDeBuffer(&parametros);
	revision_tipo_cheque=" ";
	if (formapag=="C") {
		mostrar_todas_las_formas.sprintf(" and p.formapag='%s' ",formapag);
		tipocheque=mFg.ExtraeStringDeBuffer(&parametros);
		if (tipocheque!="")
			revision_tipo_cheque.sprintf(" and chc.clasif='%s'",tipocheque);
	}else{
		if(formapag!="")
			mostrar_todas_las_formas.sprintf(" and p.formapag='%s' ",formapag);
	}
	cobrador=mFg.ExtraeStringDeBuffer(&parametros);
	if(cobrador!=""){
		mostrar_por_cobrador.sprintf(" and p.cobrador='%s' ",cobrador);
	}
	sucursal=mFg.ExtraeStringDeBuffer(&parametros);
	if(sucursal!=" "){
		join_sucursal.sprintf("INNER JOIN terminales ter ON ter.terminal = p.terminal \
		INNER JOIN secciones s ON s.seccion = ter.seccion ");
		condicion_sucursal.sprintf("AND s.sucursal = '%s' ", sucursal);
	}else{
        join_sucursal.sprintf("INNER JOIN terminales ter ON ter.terminal = p.terminal \
		INNER JOIN secciones s ON s.seccion = ter.seccion \
		INNER JOIN sucursales suc ON suc.sucursal = s.sucursal \ ");
		condicion_sucursal.sprintf("AND suc.idempresa = %s ", FormServidor->ObtieneClaveEmpresa());
	}


	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	instruccion.sprintf("select p.ident as identpago, t.tracredito as foliotran, \
	if(ifnull(v.foliofisic,'')='',concat(ifnull(cfd.serie,''),ifnull(cfd.folio,'')),v.foliofisic) as ventfolfisic, \
	cli.rsocial as nombrecli, \
	(t.valor*-1) as valor, (case when p.formapag='E' then 'EFECTIVO' when p.formapag='C' then 'CHEQUE' \
	when p.formapag='D' then 'DEPOSITO' when p.formapag='T' then 'TRANSFERENCIA BANCARIA' end) as formapag, \
	chc.folio as numcheque, b.nombre as nombanco, \
	ifnull(bm.idmovbanco,''), bc.numerocuenta, bm.fechaaplbanco  \
	from transxcob t  inner join  pagoscli p \
	  left join ventas v on v.cancelado=0 and v.referencia=t.referencia \
	  left join clientes cli on cli.cliente=v.cliente \
	  left join cheqxcob chxc on chxc.pago=p.pago \
	  left join chequesclientes chc on chc.chequecli=chxc.chequecli \
	  left join chequesclasif chclasif on chclasif.clasif=chc.clasif \
	  left join bancos b on b.banco=chc.banco \
	  left join cfd on v.referencia=cfd.referencia and cfd.tipocomprobante='VENT' \
	  LEFT JOIN bancosxcob bxc ON bxc.tracredito = t.tracredito \
	  LEFT JOIN bancostransacc bt ON bt.transacc = bxc.transacc      \
	  LEFT JOIN bancosmov bm ON bm.idmovbanco = bt.idmovbanco       \
	  LEFT JOIN bancoscuentas bc ON bc.idnumcuenta = bm.idnumcuenta  \
	  %s \
	  where \
	  p.fecha='%s' and p.pago=t.pago and \
	  t.cancelada=0 and p.cancelado=0 %s %s \
	  %s %s GROUP BY t.tracredito ",
	  join_sucursal,
	  mFg.DateToMySqlDate(StrToDate(fecha)), mostrar_por_cobrador, condicion_sucursal, mostrar_todas_las_formas,
	  revision_tipo_cheque);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	//Se suman los ajustes
	if (formapag=="C") {
		instruccion.sprintf("select sum(p.ajuste) as ajuste \
			from pagoscli p, cheqxcob chxc, chequesclientes chc where \
			  p.fecha='%s' and chxc.pago=p.pago and chc.chequecli=chxc.chequecli  \
			  and p.cancelado=0 \
			  %s %s %s \
			  group by p.fecha",
			  mFg.StrToMySqlDate(fecha), mostrar_por_cobrador, mostrar_todas_las_formas,
			  revision_tipo_cheque);
	} else {
		instruccion.sprintf("select sum(p.ajuste) as ajuste \
			from pagoscli p where \
			  p.fecha='%s' \
			  and p.cancelado=0 \
			  %s %s\
			  group by p.fecha",
			  mFg.StrToMySqlDate(fecha), mostrar_por_cobrador, mostrar_todas_las_formas);
	}
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}

//------------------------------------------------------------------------------
//ID_CON_AUX_CLI
void ServidorVentas::ConsultaAuxiliarCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA AUXILIAR DE CLIENTES
	AnsiString instruccion, instruccion_revisar_saldo, instruccion_transacciones, instruccion_numReferencia;
	AnsiString cliente, fecha_inicial_venta, fecha_final_venta;
	AnsiString mostrar_todas_transacciones;
	AnsiString mostrar_todo,numReferencia,mEstadoActivaOCancelada;
	AnsiString mAcredito, condicion_empresa = " ", empresa;

	cliente=mFg.ExtraeStringDeBuffer(&parametros);
	fecha_inicial_venta=mFg.ExtraeStringDeBuffer(&parametros);
	fecha_final_venta=mFg.ExtraeStringDeBuffer(&parametros);
	mostrar_todas_transacciones=mFg.ExtraeStringDeBuffer(&parametros);
	mostrar_todo=mFg.ExtraeStringDeBuffer(&parametros);
	numReferencia=mFg.ExtraeStringDeBuffer(&parametros);
	mEstadoActivaOCancelada=mFg.ExtraeStringDeBuffer(&parametros);
	empresa = mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	if (mostrar_todas_transacciones=="0")
		instruccion_transacciones.sprintf(" t.fechaalta>='%s' AND t.fechaalta<='%s' AND ",mFg.StrToMySqlDate(fecha_inicial_venta), mFg.StrToMySqlDate(fecha_final_venta));
	else
		instruccion_transacciones=" ";

	if(numReferencia!=" ")
	   instruccion_numReferencia.sprintf("   v.referencia='%s' AND " , numReferencia);
	else
	   instruccion_numReferencia=" ";

	if (mostrar_todo=="0")
		instruccion_revisar_saldo=" vs.saldor>0 and ";
	else
		instruccion_revisar_saldo=" ";

	if(empresa != " "){
	   condicion_empresa.sprintf(" AND suc.idempresa = %s ", empresa);
	}

	// Crea una tabla donde se van a poner los saldos de las ventas
	// afectadas por la cancelación
	instruccion="create temporary table auxventassaldos (venta char(11), saldor decimal(16,2), chcnc decimal(16,2), PRIMARY KEY (venta)) Engine = InnoDB";
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query EjecutaSelectSqlNulo"));

	// Calcula los saldos de las ventas del cliente
	instruccion.sprintf("insert into auxventassaldos (venta, saldor, chcnc) \
	select v.referencia as venta, \
	sum(if(t.aplicada=1, t.valor, 0)) as saldor, \
	sum(if(t.aplicada=0, t.valor, 0)) as chcnc \
	from ventas v \
	INNER JOIN transxcob t ON t.referencia=v.referencia \
	INNER JOIN terminales ter ON ter.terminal = v.terminal \
	INNER JOIN secciones sec ON sec.seccion = ter.seccion \
	INNER JOIN sucursales suc ON suc.sucursal = sec.sucursal  \
	where v.cliente='%s' and v.fechavta>='%s' and v.fechavta<='%s' \
	and t.cancelada=0 and v.cancelado=%s and %s v.acredito=1 %s  \
	group by v.referencia",
	cliente, mFg.StrToMySqlDate(fecha_inicial_venta),
	mFg.StrToMySqlDate(fecha_final_venta), mEstadoActivaOCancelada, instruccion_numReferencia, condicion_empresa );

	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query EjecutaSelectSqlNulo"));

	instruccion.sprintf("select t.fechaapl as fechatran, t.tipo, t.tracredito, \
	v.referencia, if(t.tipo='VENT',1,2) as esventa, \
	if(ifnull(n.foliofisic,'')='',concat(ifnull(cfdn.serie,''),ifnull(cfdn.folio,'')),n.foliofisic) as notfoliofisic, \
	if(ifnull(nc.foliofisic,'')='',concat(ifnull(cfdnc.serie,''),ifnull(cfdnc.folio,'')),nc.foliofisic) as notcfoliofisic, \
	p.ident as identpago, \
	if(ifnull(v.foliofisic,'')='',concat(ifnull(cfd.serie,''),ifnull(cfd.folio,'')),v.foliofisic) as ventfoliofisic, \
	(case when p.formapag='E' then 'EFECTIVO' when p.formapag='C' then 'CHEQUE' \
	when p.formapag='D' then 'DEPOSITO' when p.formapag='T' then 'TRANSFERENCIA BANCARIA' end) as formapag, \
	chc.fechacob as fechacheque,  chc.estado as statuscheque, chc.clasif as clasifcheque, concat(chc.folio,' - ',b.nombre) as numbanco, \
	t.valor, v.fechavenc, t.horaalta as horatran, vs.saldor, (vs.chcnc*-1) as chcnc, v.vendedor, \
	v.usualta, concat(em.nombre, ' ', em.appat) as capturo, \
	v.acredito as terminos, n.referencia AS notacredref, n.tipo AS tiponotacred, p.pago AS refpago \
	from ventas v  inner join  auxventassaldos vs  inner join  transxcob t  inner join  empleados em \
	  left join pagoscli p on p.pago=t.pago \
	  left join cheqxcob chxc on chxc.pago=p.pago \
	  left join chequesclientes chc on chc.chequecli=chxc.chequecli \
	  left join bancos b on chc.banco=b.banco \
	  left join notascredcli n on n.referencia=t.notacli \
	  left join notascarcli nc on nc.referencia=t.notacar \
	  left join cfd on v.referencia=cfd.referencia and cfd.tipocomprobante='VENT' \
	  left join cfd cfdn on n.referencia=cfdn.referencia and cfdn.tipocomprobante='NCRE' \
	  left join cfd cfdnc on n.referencia=cfdnc.referencia and cfdnc.tipocomprobante='NCAR' \
	where %s %s %s\
	v.cancelado=%s and v.fechavta>='%s' and v.fechavta<='%s' and v.usualta=em.empleado and \
	v.cliente='%s' and v.referencia=vs.venta and \
	v.referencia=t.referencia and t.cancelada=0 \
	order by t.referencia, esventa,t.fechaapl, t.fechaalta,t.tracredito", instruccion_revisar_saldo, instruccion_transacciones,instruccion_numReferencia,
	mEstadoActivaOCancelada, mFg.StrToMySqlDate(fecha_inicial_venta), mFg.StrToMySqlDate(fecha_final_venta),
	cliente);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}

//------------------------------------------------------------------------------
//ID_CONSOLIDA_CHEQ_CLI
void ServidorVentas::ConsolidaChequesCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  CONSOLIDA UNA SERIE DE CHEQUES DE CLIENTE
	char *buffer_sql=new char[1024*1000];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instrucciones[10000];
	AnsiString instruccion;
	int num_instrucciones=0;
	AnsiString cheque;
	int num_cheques;
	int i;
	AnsiString FechaSigAplicacion;
	AnsiString StatusCheque;
	AnsiString Usuario;

	try{
		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		num_cheques=StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
		FechaSigAplicacion=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		Usuario=mFg.ExtraeStringDeBuffer(&parametros);

		for (i=0; i<num_cheques; i++) {
			cheque=mFg.ExtraeStringDeBuffer(&parametros); // Clave del cheque
			instruccion.sprintf("update chequesclientes chc set chc.estado='C' \
				where chc.chequecli='%s' and chc.estado='P'", cheque);
			instrucciones[num_instrucciones++]=instruccion;

			/**/instruccion.sprintf("update chequesclientes chc, pagoscli p, cheqxcob chxc, transxcob t \
				set t.aplicada=1, t.fechaapl='%s', t.horamodi='%s', t.fechamodi='%s', t.usumodi='%s' \
				where chc.chequecli=chxc.chequecli and chxc.pago=p.pago and \
				p.pago=t.pago and chc.chequecli='%s' and t.cancelada=0",
				mFg.DateToMySqlDate(mFg.MySqlDateToTDate(FechaSigAplicacion.c_str())-1)
				,mFg.TimeToMySqlTime(Time()),mFg.DateToMySqlDate(Date()),Usuario,cheque);
			instrucciones[num_instrucciones++]=instruccion;
		}

		instruccion.sprintf("update estadosistemaemp ssis set ssis.valor='%s' \
			where ssis.estado='FACHEQCLI' AND ssis.sucursal = '%s' ", FechaSigAplicacion, FormServidor->ObtieneClaveSucursal());
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
//ID_CON_CHEQXFECH_CLI
void ServidorVentas::ConsultaChequesxfechaCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA CHEQUES DEL CLIENTE DE UN DIA X
    AnsiString instruccion;
    AnsiString fecha_inicio, fecha_fin, tipocheque, revision_tipo_cheque, banco_cheque, revision_banco_cheque;

    fecha_inicio=mFg.ExtraeStringDeBuffer(&parametros);
    fecha_fin=mFg.ExtraeStringDeBuffer(&parametros);
	banco_cheque=mFg.ExtraeStringDeBuffer(&parametros);
    tipocheque=mFg.ExtraeStringDeBuffer(&parametros);

    revision_tipo_cheque=" ";
    revision_banco_cheque=" ";
    if (tipocheque!="")
        revision_tipo_cheque.sprintf(" and chc.clasif='%s'",tipocheque);
    if (banco_cheque!="")
        revision_banco_cheque.sprintf(" and chc.banco='%s'",banco_cheque);

    mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	instruccion.sprintf("select chc.chequecli, v.foliofisic as ventfolfisic, \
        if(cl.tipoempre=0, concat(cl.nombre, ' ', cl.appat, ' ', cl.apmat), rsocial) as nombrecli, \
        (t.valor*-1) as valor, chc.folio as numcheque, b.banco as cvebanco, b.nombre as nombanco, \
        chclasif.clasif as clasif, chclasif.descripcion as tipocheque, chc.fechacob, chc.estado as status \
        from transxcob t  inner join  pagoscli p  inner join  \
        cheqxcob chxc  inner join  chequesclientes chc \
        left join ventas v on v.referencia=t.referencia and v.cancelado=0 \
        left join clientes cl on cl.cliente=v.cliente \
        left join chequesclasif chclasif on chclasif.clasif=chc.clasif \
        left join bancos b on b.banco=chc.banco \
        where \
        chc.fechacob>='%s' and chc.fechacob<='%s' \
        and chxc.pago=p.pago and chc.chequecli=chxc.chequecli and \
        p.pago=t.pago and t.cancelada=0 and p.cancelado=0 \
        %s %s",
		mFg.StrToMySqlDate(fecha_inicio),mFg.StrToMySqlDate(fecha_fin),
        revision_tipo_cheque, revision_banco_cheque);
    mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

    //Se suman los ajustes
    instruccion.sprintf("select sum(p.ajuste) as ajuste \
        from pagoscli p, cheqxcob chxc, chequesclientes chc \
		  where \
          chc.fechacob>='%s' and chc.fechacob<='%s' and chxc.pago=p.pago and chc.chequecli=chxc.chequecli  \
          and p.cancelado=0 \
		  %s \
          group by p.cancelado",
		  mFg.StrToMySqlDate(fecha_inicio),mFg.StrToMySqlDate(fecha_fin),
          revision_tipo_cheque);
    mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}

//------------------------------------------------------------------------------
//ID_MOD_CHEQ_CLI
void ServidorVentas::ModificaChequeCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  MODIFICA UN CHEQUE DE CLIENTE
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

	try{
		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		cheque=mFg.ExtraeStringDeBuffer(&parametros);
		folio=mFg.ExtraeStringDeBuffer(&parametros);
		banco=mFg.ExtraeStringDeBuffer(&parametros);
		fechacob=mFg.ExtraeStringDeBuffer(&parametros);
		estado=mFg.ExtraeStringDeBuffer(&parametros);
		clasif=mFg.ExtraeStringDeBuffer(&parametros);
		usuario=mFg.ExtraeStringDeBuffer(&parametros);


		instruccion.sprintf("update chequesclientes chc set chc.estado=if(chc.estado='C','C','%s'), \
			chc.folio='%s', chc.banco='%s', chc.fechacob='%s', \
			chc.clasif='%s' \
			where chc.chequecli='%s'",
			estado, folio, banco, mFg.StrToMySqlDate(fechacob), clasif, cheque);
		instrucciones[num_instrucciones++]=instruccion;

		if (estado=="C")
			aplicada="1";
		else
			aplicada="0";

		/**/instruccion.sprintf("update transxcob t, pagoscli p, cheqxcob chxc, \
			chequesclientes chc \
			set t.aplicada=%s, t.fechamodi='%s', t.horamodi='%s', t.usumodi='%s' \
			where t.pago=p.pago and t.cancelada=0 and p.pago=chxc.pago and chxc.chequecli=chc.chequecli",
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
//ID_QRY_VENTAS_CLI
void ServidorVentas::ConsultaVentasCliente(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA LAS VENTAS DE UN CLIENTE
	AnsiString instruccion;
	AnsiString cliente, uso, datos_extendidos=" ";
	AnsiString fecha_inicial, fecha_final;
	AnsiString condicion_todas_las_ventas=" ",condicion_sin_ventas_congeladas=" ", auxiliar_valor_inicial=" ";

	cliente=mFg.ExtraeStringDeBuffer(&parametros);
	// 0=solo venats que tienen saldo
	// 1=todas las ventas independientemente de su saldo
	// 2=solo venats que tienen saldo y datos adicionales
	uso=mFg.ExtraeStringDeBuffer(&parametros);


	// Si se desean todas las ventas independientemente de su saldo
	// se debera especificar la fecha inicial y la final para no sobrecargar al sistema
	if(uso=="1"){
		fecha_inicial=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_final=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
	}

	//verificamos si se quieren todas las ventas o solo las que tienen saldo
	if (uso=="0") {
		condicion_todas_las_ventas=" and vs.saldo > 0 ";
	}else{
		if(uso=="1"){
			condicion_todas_las_ventas.sprintf(" and v.fechaalta >= '%s' and v.fechaalta <= '%s' ",
			fecha_inicial, fecha_final);
			auxiliar_valor_inicial=" v.valor, ";
		} else {
			if(uso=="2"){
				condicion_todas_las_ventas=" and vs.saldo > 0 ";
				datos_extendidos=" , v.valor, v.fechavenc ";
			}
			else {
				if(uso=="3"){
					condicion_todas_las_ventas=" and vs.saldo > 0 ";
					datos_extendidos=" , v.valor, v.fechavenc ";
					condicion_sin_ventas_congeladas=" AND v.referencia NOT IN(select referencia from ventascongeladas where activo=1) ";
				} else {
					throw Exception("El uso solo puede ser 0,1,2 o 3");
				}
			}
		}
	}

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Carga todos los datos del cliente.
	instruccion.sprintf("select c.cliente, c.tipoempre as tipocliente, c.nombre, c.appat, c.apmat, c.rfc, c.rsocial, \
		c.calle, c.colonia, c.cp, c.plazo, c.limcred, c.excederlc, c.numpedidos, tdb.clasific, tdb.descripcion, cet.vendedor, \
		col.nombre as nomcolonia, loc.nombre as nomlocalidad, mun.municipio, est.nombre as estado, \
		tdp.tipoprec, tdp.descripcion as descprecio \
		from clientes c \
		left join colonias col on c.colonia=col.colonia \
		left join localidades loc on col.localidad=loc.localidad \
		left join municipios mun on loc.municipio=mun.municipio \
		left join estados est on mun.estado=est.estado \
		left join tiposdebloqueos tdb on c.bloqueo=tdb.tipo \
		left join clientesemp cet on cet.cliente=c.cliente AND cet.idempresa=%s \
		left join tiposdeprecios tdp on tdp.tipoprec=cet.tipoprec and tdp.idempresa=%s \
		where c.cliente='%s' ",
		FormServidor->ObtieneClaveEmpresa(), FormServidor->ObtieneClaveEmpresa(), cliente);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Crea una tabla donde se van a poner los saldos de las ventas del cliente
	instruccion="create temporary table auxventassaldos (venta char(11), saldo decimal(16,2), PRIMARY KEY (venta)) Engine = InnoDB";
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query EjecutaSelectSqlNulo"));

	// Calcula los saldos de las ventas del cliente (SALDOB (saldo virtual))
	instruccion.sprintf("insert into auxventassaldos (venta, saldo) \
		select v.referencia as venta, sum(t.valor) as saldo \
		from ventas v, transxcob t \
		where v.cliente='%s' and \
		t.referencia=v.referencia and t.cancelada=0 and v.cancelado=0 and v.acredito=1 %s\
		group by v.referencia",
		cliente,condicion_sin_ventas_congeladas);
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query EjecutaSelectSqlNulo"));

	instruccion.sprintf("select v.referencia as Ref, \
		if(ifnull(v.foliofisic,'')='',concat(ifnull(cfd.serie,''),ifnull(cfd.folio,'')),v.foliofisic) as Factura, \
		%s vs.saldo as Saldo, tfv.descripcion, v.fechavta %s , cfd.version, cfd.muuid ,\
		 cfd.rfcreceptor ,cfd.sucursal, ifnull (vc.activo,0) as congelada, IF(dp.pago IS NULL,0,1) AS tieneprepago \
		from ventas v inner join auxventassaldos vs inner join tiposfacturasventas tfv \
		left join cfd on v.referencia=cfd.referencia and cfd.tipocomprobante='VENT' \
		LEFT JOIN ventascongeladas AS vc on vc.referencia=v.referencia and vc.activo=1 \
		LEFT JOIN ( \
		SELECT SUM(dp.valor) AS pago, dp.referencia \
		FROM ventas v \
		INNER JOIN predpagoscli dp ON dp.referencia = v.referencia \
		INNER JOIN prepagoscli p ON p.pago = dp.pago \
		WHERE v.cliente = '%s' AND v.cancelado=0 GROUP BY v.referencia ) dp ON dp.referencia = v.referencia \
		INNER JOIN terminales ter ON ter.terminal = v.terminal \
		INNER JOIN secciones secc ON secc.seccion = ter.seccion \
		INNER JOIN sucursales suc ON suc.sucursal = secc.sucursal \
		where v.referencia=vs.venta and v.tipofac=tfv.tipo \
		%s \
		and v.cancelado=0 and v.cliente='%s' %s AND suc.idempresa = %s",
		auxiliar_valor_inicial, datos_extendidos, cliente, condicion_todas_las_ventas,
		cliente,condicion_sin_ventas_congeladas, FormServidor->ObtieneClaveEmpresa());
		////select rfcreceptor from cfd
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}
//------------------------------------------------------------------------------
//ID_QRY_VENTAS_CLI_NCREDITO
void ServidorVentas::ConsultaVentasClienteParaNCredito(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{

	   // CONSULTA LAS VENTAS DE UN CLIENTE DE LAS CUALES SE LES PUEDE REALIZAR UNA NOTA DE CREDITO
	   AnsiString instruccion;
	   AnsiString cliente, incluircortadas, archivo_temp1;
       AnsiString validacion_fecha_inner, validacion_fecha_cond;
	   BufferRespuestas* resp_param_acfacpago = NULL;
	   AnsiString param_contado = "";

	   /*
	   Consulta en la tabla  'parametrosglobemp' el valor 'ANCDCONT', el cual nos indica
	   si podemos realizar notas de crédito por devolución a notas de 'CONTADO'.
	   */

	   try{

		  instruccion.sprintf( "SELECT valor FROM parametrosglobemp WHERE parametro='ANCDCONT' AND idempresa = %s ", FormServidor->ObtieneClaveEmpresa());
		  if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_param_acfacpago)) {

			  if (resp_param_acfacpago->ObtieneNumRegistros() > 0) {
				  param_contado = resp_param_acfacpago->ObtieneDato("valor");
			  }
			  else
				  throw(Exception("No se encuentra registro ANCDCONT en tabla parametrosglobemp"));

		  }
		  else
			  throw(Exception("Error al consultar en tabla parametrosglobemp"));

	  }
	  __finally {
		  delete resp_param_acfacpago;
	  }



	// CONSULTA LAS VENTAS DE UN CLIENTE DE LAS CUALES SE LES PUEDE REALIZAR UNA NOTA DE CREDITO

	cliente=mFg.ExtraeStringDeBuffer(&parametros);
	incluircortadas=mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);


	// Carga todos los datos del cliente.
	instruccion.sprintf("select c.cliente, c.tipoempre as tipocliente, c.nombre, c.appat, c.apmat, c.rfc, c.rsocial, \
		c.calle, c.colonia, c.cp, c.plazo, c.limcred, c.excederlc, c.numpedidos, tdb.clasific, tdb.descripcion, cet.vendedor, \
		col.nombre as nomcolonia, loc.nombre as nomlocalidad, mun.municipio, est.nombre as estado, tdp.tipoprec, \
		tdp.descripcion as descprecio, c.enviarcfd, c.email, c.email2, CONCAT(rf.regimenfiscal, ' - ', rf.descripcion) AS regimenfiscal \
		from clientes c \
		left join colonias col on c.colonia=col.colonia \
		left join localidades loc on col.localidad=loc.localidad \
		left join municipios mun on loc.municipio=mun.municipio \
		left join estados est on mun.estado=est.estado \
		left join tiposdebloqueos tdb on c.bloqueo=tdb.tipo \
		left join clientesemp cet on cet.cliente=c.cliente AND cet.idempresa=%s \
		left join tiposdeprecios tdp on tdp.tipoprec=cet.tipoprec and tdp.idempresa=%s \
		left join cregimenfiscal rf ON c.regimenfiscal = rf.regimenfiscal \
		where c.cliente='%s' ",
		FormServidor->ObtieneClaveEmpresa(), FormServidor->ObtieneClaveEmpresa(), cliente);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Crea una tabla donde se van a poner los saldos de las ventas del cliente
	instruccion="create temporary table auxventassaldos (venta char(11), saldo decimal(16,2), PRIMARY KEY (venta)) Engine = InnoDB";
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query EjecutaSelectSqlNulo"));

	// Calcula los saldos de las ventas del cliente (SALDOB (saldo virtual))
    // Generar un archivo temporal
	archivo_temp1=mServidorVioleta->ObtieneArchivoTemp(100, Respuesta->Id);

	if (incluircortadas=="0")
	{
		 validacion_fecha_inner = "inner join estadosistemaemp e ON e.estado='FUCIERRE' AND e.sucursal = '"+ FormServidor->ObtieneClaveSucursal() +"' ";
		 validacion_fecha_cond  = " and v.fechavta > cast(e.valor as date)  ";
	}else
		{
			validacion_fecha_inner = " ";
			validacion_fecha_cond  = " ";
		}

	instruccion.sprintf("select v.referencia as venta, sum(t.valor) as saldo \
		from ventas v INNER JOIN  transxcob t ON t.referencia=v.referencia  AND  t.cancelada=0 %s \
		where v.cliente='%s'  and v.cancelado=0 %s \
		group by v.referencia INTO OUTFILE '%s' ",validacion_fecha_inner,cliente,validacion_fecha_cond,archivo_temp1);
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query OUTFILE auxventassaldos"));

	instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE auxventassaldos (venta, saldo) ",archivo_temp1);
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
	{
	   throw (Exception("Error en query INTO TABLE auxventassaldos"));
	}else
		{
			mServidorVioleta->BorraArchivoTemp(archivo_temp1);
		}

	if (incluircortadas=="0") {          //   Verificamos que las ventas sean superior a la fecha de corte
		instruccion.sprintf("(select  v.referencia as Ref, \
			if(ifnull(v.foliofisic,'')='',concat(ifnull(cfd.serie,''),ifnull(cfd.folio,'')),v.foliofisic) as Factura, \
			vs.saldo as Saldo, v.fechavta,'CRÉDITO' as tipofactreal, v.corte \
			from ventas v inner join estadosistemaemp e inner join auxventassaldos vs inner join tiposfacturasventas tv \
			INNER JOIN terminales ter ON ter.terminal = v.terminal \
			INNER JOIN secciones sec ON sec.seccion = ter.seccion \
			INNER JOIN sucursales suc ON suc.sucursal = sec.sucursal \
			left join cfd on v.referencia=cfd.referencia and cfd.tipocomprobante='VENT' \
			where v.referencia=vs.venta and e.estado='FUCIERRE' AND e.sucursal = '%s' and \
			v.tipofac=tv.tipo and \
			tv.permitirncredito=1 and \
			vs.saldo > 0 and v.fechavta > cast(e.valor as date) \
			and cancelado=0 and cliente='%s' AND suc.idempresa = %s \
			order by v.fechavta desc, v.referencia desc )",
			FormServidor->ObtieneClaveSucursal(),cliente, FormServidor->ObtieneClaveEmpresa());
	} else {                             //   No verificamos que las ventas sean superior a la fecha de corte
		instruccion.sprintf("(select v.referencia as Ref, \
			if(ifnull(v.foliofisic,'')='',concat(ifnull(cfd.serie,''),ifnull(cfd.folio,'')),v.foliofisic) as Factura, \
			vs.saldo as Saldo, v.fechavta,'CRÉDITO' as tipofactreal, v.corte \
			from ventas v inner join auxventassaldos vs inner join tiposfacturasventas tv \
			INNER JOIN terminales ter ON ter.terminal = v.terminal \
			INNER JOIN secciones sec ON sec.seccion = ter.seccion \
			INNER JOIN sucursales suc ON suc.sucursal = sec.sucursal \
			left join cfd on v.referencia=cfd.referencia and cfd.tipocomprobante='VENT' \
			where v.referencia=vs.venta and \
			v.tipofac=tv.tipo and \
			tv.permitirncredito=1 and \
			vs.saldo > 0 \
			and cancelado=0 and cliente='%s' AND suc.idempresa = %s \
			order by v.fechavta desc, v.referencia desc )",
			cliente, FormServidor->ObtieneClaveEmpresa());
	}


	   /*
	   Consulta en la tabla  'parametrosglobemp' el valor 'ANCDCONT', el cual nos indica
	   si podemos realizar notas de crédito por devolución a notas de 'CONTADO'.

	   En caso de que el valor del parámetro = 1
	   La consulta se une a la siguiente consulta para combinar las facturas de crédito y de contado

	   Tenemos que tener en cuenta que en los siguientes casos no podemos ver las las ventas generada con tickets
	   */


	  if( param_contado == "1" )
	   {


		   if ( incluircortadas == "0" )   //   Verificamos que las ventas sean superior a la fecha de corte
		   {

				instruccion+="UNION ALL \
				(SELECT v.referencia AS Ref, IF(IFNULL(v.foliofisic,'')='', CONCAT(IFNULL(cfd.serie,''), \
						 IFNULL(cfd.folio,'')),v.foliofisic) AS Factura,v.valor AS Saldo,v.fechavta,'CONTADO' AS tipofactreal, v.corte \
				FROM     ventas v \
						 INNER JOIN estadosistemaemp e \
						 INNER JOIN tiposfacturasventas tv \
						 INNER JOIN terminales ter ON ter.terminal = v.terminal \
						 INNER JOIN secciones sec ON sec.seccion = ter.seccion \
						 INNER JOIN sucursales suc ON suc.sucursal = sec.sucursal \
						 LEFT JOIN cfd ON v.referencia=cfd.referencia AND cfd.tipocomprobante='VENT' \
				WHERE 	 v.tipofac=tv.tipo AND cancelado=0 AND cliente='"+cliente+"' AND v.acredito=0 AND \
						 v.fechavta > CAST(e.valor AS DATE) AND e.estado='FUCIERRE' AND e.sucursal = '"+FormServidor->ObtieneClaveSucursal()+"' AND v.ticket='0' AND suc.idempresa = "+FormServidor->ObtieneClaveEmpresa()+" \
				ORDER BY v.fechavta DESC, v.referencia DESC)";

			}
			else {                     //   No verificamos que las ventas sean superior a la fecha de corte

					instruccion+="UNION ALL \
					(SELECT v.referencia AS Ref, IF(IFNULL(v.foliofisic,'')='', CONCAT(IFNULL(cfd.serie,''), \
							 IFNULL(cfd.folio,'')),v.foliofisic) AS Factura,v.valor AS Saldo,v.fechavta,'CONTADO' AS tipofactreal, v.corte \
					FROM     ventas v \
							 INNER JOIN tiposfacturasventas tv \
							 INNER JOIN terminales ter ON ter.terminal = v.terminal \
							 INNER JOIN secciones sec ON sec.seccion = ter.seccion \
							 INNER JOIN sucursales suc ON suc.sucursal = sec.sucursal \
							 LEFT JOIN cfd ON v.referencia=cfd.referencia AND cfd.tipocomprobante='VENT' \
					WHERE 	 v.tipofac=tv.tipo AND cancelado=0 AND cliente='"+cliente+"' AND v.acredito=0  AND v.ticket='0' AND suc.idempresa = "+FormServidor->ObtieneClaveEmpresa()+" \
					ORDER BY v.fechavta DESC, v.referencia DESC)";

				}


	  }

	  mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),Respuesta->TamBufferResultado);

  }



//------------------------------------------------------------------------------
//ID_GRA_CARGO_CLI
void ServidorVentas::GrabaCargoCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  GRABA UN CARGO A CLIENTE
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
	AnsiString venta, tipo, foliofisic;
	TDateTime fecha_not;
	double valor_cargo;

	try{
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave del cargo.
		tarea=mFg.ExtraeStringDeBuffer(&parametros); // Indicador si se va a agregar o modificar.

		// Verificaciones para no permitir modificaciones cuando no sea permitido.
		if (tarea=="M")
			throw Exception("Con la nueva facturación electrónica, ya no se deben modificar las notas de cargo, mejor cancele y vuelva a crear.");

		usuario=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que esta grabando el cargo.
		terminal=mFg.ExtraeStringDeBuffer(&parametros); // Terminal de donde se manda grabar.
		venta=mFg.ExtraeStringDeBuffer(&parametros); // Clave de la venta que se va a afectar
		tipo=mFg.ExtraeStringDeBuffer(&parametros); // Tipo del cargo.
		foliofisic=mFg.ExtraeStringDeBuffer(&parametros); // Folio físico del cargo.
		fecha_not=fecha;            // Fecha de la nota.
		valor_cargo=StrToFloat(mFg.ExtraeStringDeBuffer(&parametros)); // Valor del cargo que cobra el banco.

		// Verificaciones para no permitir modificaciones cuando no sea permitido.
		if (tarea=="M") {
			// Verifica que la fecha del cargo (la grabada) sea posterior a la fecha de cierre.
			instruccion.sprintf("select @error:=if(n.fechanot<=cast(e.valor as datetime), 1, 0) as error from notascarcli n left join estadosistemaemp as e on e.estado='FUCIERRE' where n.referencia='%s' AND e.sucursal = '%s' ", clave, FormServidor->ObtieneClaveSucursal());
			mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 3, error);
		}
		// Verifica que la fecha del cargo (la que se le va a grabar) sea posterior a la fecha de cierre.
		instruccion.sprintf("select @error:=if('%s'<=cast(e.valor as datetime), 1, 0) as error from estadosistemaemp e where e.estado='FUCIERRE' AND e.sucursal = '%s' ",  mFg.DateToMySqlDate(fecha_not), FormServidor->ObtieneClaveSucursal());
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 4, error);


		if (error==0) {
			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

			instruccion.sprintf("select @seccion:=seccion, @depart:=depart, @asigfolncar:=asigfolncar, @anchofolncar:=anchofolncar from terminales where terminal='%s'", terminal);
			instrucciones[num_instrucciones++]=instruccion;

			// Obtiene el folio para el cargo
			if (tarea=="A") {
				// Folio físico.
				instruccion.sprintf("select @foliofisicaux:=folncar, @serieaux:=seriencar from terminales where terminal='%s'", terminal);
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliofisicsig=@foliofisicaux+1");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliofisic=if(@asigfolncar=0, '%s', if(@asigfolncar=1, concat(@serieaux, lpad(@foliofisicaux,@anchofolncar,'0') ), '') )", foliofisic);
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("update terminales t set t.folncar=@foliofisicsig where t.terminal='%s' and  @asigfolncar=1", terminal);
				instrucciones[num_instrucciones++]=instruccion;

				// Folio del sistema
				instruccion.sprintf("select @folioaux:=valor from foliosemp where folio='NOTASCACLI' AND sucursal = '%s' %s ",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliosig=@folioaux+1");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @folioaux=cast(@folioaux as char)");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @folio=concat('%s', lpad(@folioaux,9,'0'))", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("update foliosemp set valor=@foliosig where folio='NOTASCACLI' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
			} else {
				instruccion.sprintf("set @foliofisic='%s'", foliofisic);
				instrucciones[num_instrucciones++]=instruccion;

				instruccion.sprintf("set @folio='%s'", clave);
				instrucciones[num_instrucciones++]=instruccion;
			}

			// Graba la cabecera en la tabla "notascarcli"
			datos.AsignaTabla("notascarcli");
			datos.InsCampo("fechamodi", mFg.DateToAnsiString(fecha));
			datos.InsCampo("horamodi", mFg.TimeToAnsiString(hora));
			datos.InsCampo("usumodi", usuario);
			datos.InsCampo("fechanot", mFg.DateToAnsiString(fecha_not));
			datos.InsCampo("foliofisic", "@foliofisic", 1);
			datos.InsCampo("valor", mFg.CadenaAFlotante(valor_cargo));
			if (tarea=="A") {
				instruccion.sprintf("select @cliente:=cliente from ventas where referencia='%s'", venta);
				instrucciones[num_instrucciones++]=instruccion;

				instruccion.sprintf("select @impuesto:=valor from parametrosemp where parametro='IMPCARGOS' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;

				datos.InsCampo("cveimp", "@impuesto", 1);
				datos.InsCampo("referencia", "@folio", 1);
				datos.InsCampo("usualta", usuario);
				datos.InsCampo("fechaalta", mFg.DateToAnsiString(fecha));
				datos.InsCampo("horaalta", mFg.TimeToAnsiString(hora));
				datos.InsCampo("cancelado", "0");
				datos.InsCampo("tipo", "O");
				datos.InsCampo("terminal", terminal);
				datos.InsCampo("cliente", "@cliente", 1);
				instrucciones[num_instrucciones++]=datos.GenerarSqlInsert();
			} else {
				instrucciones[num_instrucciones++]=datos.GenerarSqlUpdate("referencia=@folio");
			}


			// Hace la transacción del CARGO
			if (tarea=="A") {
				// Obtiene el folio para la NUEVA transaccion
				instruccion.sprintf("select @foliotranaux:=valor from foliosemp where folio='TRANCLI' AND sucursal = '%s' %s ",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliosigtran=@foliotranaux+1");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliotranaux=cast(@foliotranaux as char)");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliotran=concat('%s', lpad(@foliotranaux,9,'0'))", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("update foliosemp set valor=@foliosigtran where folio='TRANCLI' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				/**/instruccion.sprintf("insert into transxcob \
					(tracredito, referencia, notacar, concepto, destino, tipo, cancelada, aplicada, valor, fechaalta, horaalta, fechamodi, horamodi, fechaapl, usualta, usumodi) \
					values (@foliotran, '%s', @folio, 'C', 'C', '%s', 0,1, %12.2f, '%s', '%s', '%s', '%s', '%s', '%s', '%s')",
					 venta, tipo, valor_cargo, mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), mFg.DateToMySqlDate(fecha_not), usuario, usuario);
				instrucciones[num_instrucciones++]=instruccion;
			} else {
				// Actualiza solo el valor, la fecha de la nota, la fecha y hora de modificación y el usuario que modificó.
				/**/instruccion.sprintf("update transxcob set valor=%12.2f, fechamodi='%s', horamodi='%s', fechaapl='%s', usumodi='%s' where notacar='%s' and cancelada=0", valor_cargo, mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), mFg.DateToMySqlDate(fecha_not), usuario, clave);
				instrucciones[num_instrucciones++]=instruccion;
			}

			// instrucciones[num_instrucciones++]="COMMIT";
		}

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {

			// Crea el CFD (si la configuracion así lo indica) y hace commit
			ComprobanteFiscalDigital *cfd=NULL;
			try {
				cfd=new ComprobanteFiscalDigital(mServidorVioleta);
				if (cfd->EmitirCFDI40(Respuesta, MySQL, "NCAR"))
					cfd->CreaCFDINotaCargo40(Respuesta, MySQL);
				else
					cfd->CreaCFDINotaCargo33(Respuesta, MySQL);
			} __finally {
				if(cfd!=NULL) delete cfd;
			}

			instruccion.sprintf("select %d as error, @folio as folio, @foliofisic as foliofisic", error);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	} __finally {
		delete buffer_sql;
	}
}

//------------------------------------------------------------------------------
//ID_CANC_CARGO_CLI
void ServidorVentas::CancelaCargoCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  CANCELA CARGO DE CLIENTE       Original //ID_CANC_CARGO_CLI
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

	try{
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave de la devolución
		usuario=mFg.ExtraeStringDeBuffer(&parametros); // Usuario

		// Verifica que la fecha de la nota sea posterior a la fecha de cierre.
		// instruccion.sprintf("select @error:=if(n.fechanot<=cast(e.valor as datetime), 1, 0) as error from notascarcli n left join estadosistema as e on e.estado='FUCIERRE' where n.referencia='%s'", clave);
		// mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 3, error);

		if (error==0) {
			// Cancela el CFDI con el PAC (si no se trata de un CFDI simplemente no se hace nada con ningún PAC)

			// SOLO CUANDO NO ESTA EN DEPURACION
			#ifndef _DEBUG
			ComprobanteFiscalDigital *cfd=NULL;
			try {
				cfd=new ComprobanteFiscalDigital(mServidorVioleta);
				cfd->cancelarCFDI(Respuesta, MySQL, "NCAR",clave,"","02","");
			} __finally {
				if(cfd!=NULL) delete cfd;
			}
			#endif

			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

			// Quita de la cola de impresion
			instruccion.sprintf("delete from colaimpresion where foliodoc='%s' and tipo=2", clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Obtiene el folio de la venta a la que se aplicó el cargo
			instruccion.sprintf("select @venta:=t.referencia from transxcob t where t.notacar='%s' and t.cancelada=0", clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Cancela el registro en notascarcli
			instruccion.sprintf("update notascarcli set cancelado=1, usumodi='%s', fechamodi='%s', horamodi='%s' where referencia='%s' and cancelado=0", usuario, mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Cancela el CFD
			instruccion.sprintf("update cfd set estado=0, fechacancelamin=curdate(), fechacancela='%s %s' where referencia='%s' and tipocomprobante='NCAR'", mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), clave);
			instrucciones[num_instrucciones++]=instruccion;


			// Cancela el cargo
			/**/instruccion.sprintf("update transxcob set cancelada=1, fechamodi='%s', horamodi='%s', usumodi='%s' where notacar='%s' and cancelada=0",  mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario, clave);
			instrucciones[num_instrucciones++]=instruccion;

			////////////////// INICIO CALCULO DE SALDOS DE VENTAS

			// Crea una tabla para almacenar los folios de las ventas afectadas por el pago
			// para posteriormente recalcular saldos de estas ventas.
			instruccion="create temporary table ventasaux (venta char(11), PRIMARY KEY (venta)) Engine = InnoDB";
			instrucciones[num_instrucciones++]=instruccion;

			instruccion="insert into ventasaux (venta) values (@venta)";
			instrucciones[num_instrucciones++]=instruccion;

			// Crea una tabla donde se van a poner los saldos de las ventas
			// afectadas por la cancelación
			instruccion="create temporary table auxventassaldos (venta char(11), saldo decimal(16,2), PRIMARY KEY (venta)) Engine = InnoDB";
			instrucciones[num_instrucciones++]=instruccion;

			// Calcula los saldos de las ventas relacionadas con el pago (SALDO B)
			instruccion="insert into auxventassaldos (venta, saldo) ";
			instruccion+="select v.referencia as venta, sum(t.valor) as saldo ";
			instruccion+="from ventas v, ventasaux vaux, transxcob t ";
			instruccion+="where v.referencia=vaux.venta and ";
			instruccion+="t.referencia=v.referencia and t.cancelada=0 and v.cancelado=0 ";
			instruccion+="group by v.referencia";
			instrucciones[num_instrucciones++]=instruccion;

			////////////////// FIN CALCULO DE SALDOS DE VENTAS

			// Crea el buffer con todas las instrucciones SQL
			aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
			for (i=0; i<num_instrucciones; i++)
				aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

			mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
			if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
				// VERIFICA SI QUEDO ALGUN SALDO NEGATIVO DE LAS FACTURAS EN CUESTION
				//
				instruccion="select * from auxventassaldos where saldo<0";
				mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_verificacion);
				if (resp_verificacion->ObtieneNumRegistros()==0) {
					instruccion="COMMIT";
					error=0;
				} else {
					instruccion="ROLLBACK";
					error=1;
				}
			}

		}

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
		if (mServidorVioleta->EjecutaAccionSql(Respuesta, MySQL, instruccion.c_str())) {
			instruccion.sprintf("select %d as error", error);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
			if (error==1) {
				instruccion="select * from auxventassaldos where saldo<0";
				mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
			}
		}
	} __finally {
		if(resp_verificacion!=NULL) delete resp_verificacion;
		delete buffer_sql;
	}
}

//------------------------------------------------------------------------------
//ID_CON_CARGO_CLI
void ServidorVentas::ConsultaCargoCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA CARGO DE CLIENTE
	AnsiString instruccion;
	AnsiString clave;

	clave=mFg.ExtraeStringDeBuffer(&parametros);
    mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los generales (de cabecera) de la nota de cargo
	instruccion.sprintf("select n.*, i.porcentaje as porcimpu, \
		cfd.folio as cfdfolio, cfd.serie as cfdserie, cfdxml.xmlgenerado as cfdxml, cfdxml.cadenaoriginal as cfdcadenaoriginal, \
		cfd.version, cfd.muuid, cfd.pactimbrador \
		from notascarcli n inner join impuestos i \
		left join cfd on n.referencia=cfd.referencia and cfd.tipocomprobante='NCAR' \
		left join cfdxml on cfd.compfiscal=cfdxml.compfiscal \
		where n.referencia='%s' and n.cveimp=i.impuesto", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene lo datos de cargo.
	instruccion.sprintf("select @venta:=t.referencia, t.* from transxcob t where t.notacar='%s' and \
		t.concepto='C' and t.destino='C' \
		group by t.referencia", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// NULO:  Verifica que la fecha de la nota de cargo sea posterior a la fecha de cierre.
	instruccion.sprintf("select if(n.fechanot<=cast(e.valor as datetime), 1, 0) as \
	modificar from notascarcli n left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' where n.referencia='%s'",FormServidor->ObtieneClaveSucursal(), clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene lo datos de la venta.
	instruccion.sprintf("select \
		if(ifnull(v.foliofisic,'')='',concat(ifnull(cfd.serie,''),ifnull(cfd.folio,'')),v.foliofisic) as foliofisic, \
		v.* from ventas v \
		left join cfd on v.referencia=cfd.referencia and cfd.tipocomprobante='VENT' \
		where v.referencia=@venta");
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene todos los datos del cliente de la nota de cargo.
    instruccion.sprintf("select c.* from clientes c, notascarcli nc where nc.referencia='%s' and nc.cliente=c.cliente", clave);
    mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

}

//------------------------------------------------------------------------------
//ID_GRA_CARGO_REBOTE_CLI
void ServidorVentas::GrabaCargoReboteCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    //  GRABA UN CARGO A CLIENTE POR UN REBOTE DE CHEQUE
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
	AnsiString cheque, foliofisic, transac_detalle, venta_detalle;
	AnsiString venta_cargo_banco, transac_cargo_banco;
	double valor_cargo_banco, valor_detalle;

	try{
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave del cargo
		tarea=mFg.ExtraeStringDeBuffer(&parametros); // Indicador si se va a agregar o modificar.

		if (tarea=="M")
			throw Exception("Con la nueva facturación electrónica, ya no se deben modificar las notas de cargo, mejor cancele y vuelva a crear.");

		usuario=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que esta grabando el cargo.
		terminal=mFg.ExtraeStringDeBuffer(&parametros); // Terminal de donde se manda grabar.
		cheque=mFg.ExtraeStringDeBuffer(&parametros); // Clave del cheque que se va a rebotar.
		foliofisic=mFg.ExtraeStringDeBuffer(&parametros); // Folio físico del cargo.
		fecha_not=fecha; // Fecha de la nota.
		valor_cargo_banco=StrToFloat(mFg.ExtraeStringDeBuffer(&parametros)); // Valor del cargo que cobra el banco.

		// Verificaciones para no permitir modificaciones cuando no sea permitido.
		if (tarea=="M") {
			// Verifica que la fecha del cargo (la grabada) sea posterior a la fecha de cierre.
			instruccion.sprintf("select @error:=if(n.fechanot<=cast(e.valor as datetime), 1, 0) as error from notascarcli n left join estadosistemaemp as e on e.estado='FUCIERRE' where n.referencia='%s' AND e.sucursal = '%s' ", clave, FormServidor->ObtieneClaveSucursal());
			mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 3, error);
		} else {
			// Verifica que el cheque esté vigente (no tenga los siguientes estatus "N", "X", "R")
			instruccion.sprintf("select @error:=if(ch.estado='X' or ch.estado='X' or ch.estado='X', \
					1, 0) as error from chequesclientes ch where ch.chequecli='%s'", cheque);
			mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 5, error);
		}
		// Verifica que la fecha del cargo (la que se le va a grabar) sea posterior a la fecha de cierre.
		instruccion.sprintf("select @error:=if('%s'<=cast(e.valor as datetime), 1, 0) as error from estadosistemaemp e where e.estado='FUCIERRE' AND sucursal = '%s' ", mFg.DateToMySqlDate(fecha_not), FormServidor->ObtieneClaveSucursal());
		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 4, error);


		if (error==0) {
			// Asigna a variales de MySql el folio del pago al que corresponde
			// el cheque, el cliente y el valor total del cargo.
			instruccion.sprintf("select @pago:=chxc.pago, @cliente:=ch.cliente, @valorcargo:=(ch.valor+%12.2f) \
				from cheqxcob chxc, chequesclientes ch, pagoscli p \
				where chxc.chequecli='%s' and chxc.chequecli=ch.chequecli \
				and chxc.pago=p.pago and p.cancelado=0",
				valor_cargo_banco, cheque);
			mServidorVioleta->EjecutaSelectSqlNulo(Respuesta,  MySQL, instruccion.c_str());
			if (tarea=="A") {
				instruccion.sprintf("select t.tracredito, t.referencia as venta, t.valor as valor \
					from pagoscli p, transxcob t \
					where p.pago=@pago and t.pago=p.pago \
					and p.cancelado=0 and t.cancelada=0 \
					order by t.tracredito");
				mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_lista);
			}

			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

			// Obtiene el folio para el cargo
			if (tarea=="A") {
				// Folio físico.
				instruccion.sprintf("select @foliofisicaux:=folncar, @serieaux:=seriencar, @anchofolncar:=anchofolncar from terminales where terminal='%s'", terminal);
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliofisicsig=@foliofisicaux+1");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliofisic=if(@asigfolncar=0, '%s', if(@asigfolncar=1, concat(@serieaux, lpad(@foliofisicaux,@anchofolncar,'0') ), '') )", foliofisic);
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("update terminales t set t.folncar=@foliofisicsig where t.terminal='%s' and  @asigfolncar=1", terminal);
				instrucciones[num_instrucciones++]=instruccion;

				// Folio del sistema
				instruccion.sprintf("select @folioaux:=valor from foliosemp where folio='NOTASCACLI' AND sucursal = '%s' %s ",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliosig=@folioaux+1");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @folioaux=cast(@folioaux as char)");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @folio=concat('%s', lpad(@folioaux,9,'0'))", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("update foliosemp set valor=@foliosig where folio='NOTASCACLI' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
			} else {
				instruccion.sprintf("set @foliofisic='%s'", foliofisic);
				instrucciones[num_instrucciones++]=instruccion;

				instruccion.sprintf("set @folio='%s'", clave);
				instrucciones[num_instrucciones++]=instruccion;
			}

			// Graba la cabecera en la tabla "notascarcli"
			datos.AsignaTabla("notascarcli");
			datos.InsCampo("fechamodi", mFg.DateToAnsiString(fecha));
			datos.InsCampo("horamodi", mFg.TimeToAnsiString(hora));
			datos.InsCampo("usumodi", usuario);
			datos.InsCampo("fechanot", mFg.DateToAnsiString(fecha_not));
			datos.InsCampo("foliofisic", "@foliofisic", 1);
			//datos.InsCampo("valor", "@valorcargo", 1);
			datos.InsCampo("valor", valor_cargo_banco);
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
				datos.InsCampo("pago", "@pago", 1);
				datos.InsCampo("cliente", "@cliente", 1);
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
						venta_detalle=resp_lista->ObtieneDato("venta");
						valor_detalle=StrToFloat(resp_lista->ObtieneDato("valor"))*-1;
						resp_lista->IrAlSiguienteRegistro();

						// Obtenemos la primer venta que es a la que se le
						// va a hacer el cargo cobrado por el banco por el rebote.
						if (i==0) {
							venta_cargo_banco=venta_detalle;
							transac_cargo_banco=transac_detalle;
						}

						// Obtiene el folio para la NUEVA transaccion
						instruccion.sprintf("select @foliotranaux:=valor from foliosemp where folio='TRANCLI' AND sucursal = '%s' %s",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
						instrucciones[num_instrucciones++]=instruccion;
						instruccion.sprintf("set @foliosigtran=@foliotranaux+1");
						instrucciones[num_instrucciones++]=instruccion;
						instruccion.sprintf("set @foliotranaux=cast(@foliotranaux as char)");
						instrucciones[num_instrucciones++]=instruccion;
						instruccion.sprintf("set @foliotran=concat('%s', lpad(@foliotranaux,9,'0'))", FormServidor->ObtieneClaveSucursal());
						instrucciones[num_instrucciones++]=instruccion;
						instruccion.sprintf("update foliosemp set valor=@foliosigtran where folio='TRANCLI' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
						instrucciones[num_instrucciones++]=instruccion;
						/**/instruccion.sprintf("insert into transxcob \
							(tracredito, referencia, notacar, concepto, destino, tipo, cancelada, aplicada, valor, fechaalta, horaalta, fechamodi, horamodi, fechaapl, usualta, usumodi) \
							values (@foliotran, '%s', @folio, 'C', 'C', 'CHDE', 0,1, %12.2f, '%s', '%s', '%s', '%s', '%s', '%s', '%s')",
							 venta_detalle, valor_detalle, mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), mFg.DateToMySqlDate(fecha_not), usuario, usuario);
						instrucciones[num_instrucciones++]=instruccion;
					}
				}
			}

			// Hace un abono que refleje en clientes la nota de cargo del banco
			if (tarea=="A") {
				// Obtiene el folio para la NUEVA transaccion
				instruccion.sprintf("select @foliotranaux:=valor from foliosemp where folio='TRANCLI' AND sucursal = '%s' %s ",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliosigtran=@foliotranaux+1");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliotranaux=cast(@foliotranaux as char)");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliotran=concat('%s', lpad(@foliotranaux,9,'0'))", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("update foliosemp set valor=@foliosigtran where folio='TRANCLI' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				/**/instruccion.sprintf("insert into transxcob \
					(tracredito, referencia, notacar, concepto, destino, tipo, cancelada, aplicada, valor, fechaalta, horaalta, fechamodi, horamodi, fechaapl, usualta, usumodi) \
					values (@foliotran, '%s', @folio, 'C', 'C', 'NCAR', 0,1, %12.2f, '%s', '%s', '%s', '%s', '%s', '%s', '%s')",
					 venta_cargo_banco, valor_cargo_banco, mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), mFg.DateToMySqlDate(fecha_not), usuario, usuario);
				instrucciones[num_instrucciones++]=instruccion;
			} else {
				// Actualiza solo el valor, la fecha de la nota, la fecha y hora de modificación y el usuario que modificó.
				/**/instruccion.sprintf("update transxcob set valor=%12.2f, fechamodi='%s', horamodi='%s', fechaapl='%s', usumodi='%s' where tracredito='%s'", valor_cargo_banco, mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), mFg.DateToMySqlDate(fecha_not), usuario, transac_cargo_banco);
				instrucciones[num_instrucciones++]=instruccion;
			}

			// Marca el pago como cancelado
			instruccion.sprintf("update pagoscli set cancelado=1, fechamodi='%s', horamodi='%s', usumodi='%s' where pago=@pago and cancelado=0", mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario);
			instrucciones[num_instrucciones++]=instruccion;

			// Marca el cheque como rebotado
			instruccion.sprintf("update chequesclientes, cheqxcob \
					set chequesclientes.estado='R' \
				where cheqxcob.pago=@pago and chequesclientes.chequecli=cheqxcob.chequecli");
			instrucciones[num_instrucciones++]=instruccion;

			//instrucciones[num_instrucciones++]="COMMIT";
		}

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {

			// Crea el CFD (si la configuracion así lo indica) y hace commit
			ComprobanteFiscalDigital *cfd=NULL;
			try {
				cfd=new ComprobanteFiscalDigital(mServidorVioleta);
				if (cfd->EmitirCFDI40(Respuesta, MySQL, "NCAR"))
					cfd->CreaCFDINotaCargo40(Respuesta, MySQL);
				else
					cfd->CreaCFDINotaCargo33(Respuesta, MySQL);
			} __finally {
				if(cfd!=NULL) delete cfd;
			}

			instruccion.sprintf("select %d as error, @folio as folio, @foliofisic as foliofisic", error);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	} __finally {
		if(resp_lista!=NULL) delete resp_lista;
		delete buffer_sql;
	}
}

//------------------------------------------------------------------------------
//ID_CANC_CARGO_REBOTE_CLI
void ServidorVentas::CancelaCargoReboteCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
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

	try{
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave de la devolución
		usuario=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que esta cancelando la devolución.

		// Verifica que la fecha de la nota sea posterior a la fecha de cierre.
	//    instruccion.sprintf("select @error:=if(n.fechanot<=cast(e.valor as datetime), 1, 0)
	//    as error from notascarcli n left join estadosistema as e on e.estado='FUCIERRE' where n.referencia='%s'", clave);
	//    mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 3, error);

		if (error==0) {
			// Cancela el CFDI con el PAC (si no se trata de un CFDI simplemente no se hace nada con ningún PAC)
			// SOLO CUANDO NO ESTA EN DEPURACION

			#ifndef _DEBUG
			ComprobanteFiscalDigital *cfd=NULL;
			try {
				cfd=new ComprobanteFiscalDigital(mServidorVioleta);
				cfd->cancelarCFDI(Respuesta, MySQL, "NCAR",clave,"","02","");
			} __finally {
				if(cfd!=NULL) delete cfd;
			}
			#endif

			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

			// Obtiene el folio del pago al que se aplicó el cargo por rebote.
			instruccion.sprintf("select @foliopago:=pago from notascarcli where referencia='%s' and cancelado=0", clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Cancela el registro en notascarcli
			instruccion.sprintf("update notascarcli set cancelado=1, usumodi='%s', fechamodi='%s', horamodi='%s' where referencia='%s' and cancelado=0", usuario, mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Cancela el CFD
			instruccion.sprintf("update cfd set estado=0,fechacancelamin=curdate(), fechacancela='%s %s' where referencia='%s' and tipocomprobante='NCAR'", mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Cancela el cargo hecho por el banco
			/**/instruccion.sprintf("update transxcob set cancelada=1, fechamodi='%s', horamodi='%s', usumodi='%s' where notacar='%s' and concepto='C' and destino='C' and tipo='NCAR' and cancelada=0",  mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario, clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Cancela todos los cargos hechos
			/**/instruccion.sprintf("update transxcob set cancelada=1, fechamodi='%s', horamodi='%s', usumodi='%s' where notacar='%s' and concepto='C' and destino='C' and tipo='CHDE' and cancelada=0",  mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario, clave);
			instrucciones[num_instrucciones++]=instruccion;

			// Marca el pago como NO cancelado
			instruccion.sprintf("update pagoscli set cancelado=0, fechamodi='%s', horamodi='%s', usumodi='%s' where pago=@foliopago and cancelado=1", mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), usuario);
			instrucciones[num_instrucciones++]=instruccion;

			// Marca el cheque como COBRADO
			instruccion.sprintf("update chequesclientes, cheqxcob \
				set chequesclientes.estado='C' \
				where cheqxcob.pago=@foliopago and chequesclientes.chequecli=cheqxcob.chequecli");
			instrucciones[num_instrucciones++]=instruccion;

			////////////////// INICIO CALCULO DE SALDOS DE VENTAS

			// Crea una tabla para almacenar los folios de las ventas afectadas por el pago
			// para posteriormente recalcular saldos de estas ventas.
			instruccion="create temporary table ventasaux (venta char(11), PRIMARY KEY (venta)) Engine = InnoDB";
			instrucciones[num_instrucciones++]=instruccion;

			instruccion="insert into ventasaux (venta) ";
			instruccion+="select t.referencia as venta from transxcob t where t.pago=@foliopago and t.cancelada=0 ";
			instrucciones[num_instrucciones++]=instruccion;

			// Crea una tabla donde se van a poner los saldos de las ventas
			// afectadas por la cancelación
			instruccion="create temporary table auxventassaldos (venta char(11), saldo decimal(16,2), PRIMARY KEY (venta)) Engine = InnoDB";
			instrucciones[num_instrucciones++]=instruccion;

			// Calcula los saldos de las ventas relacionadas con el pago (SALDO B)
			instruccion="insert into auxventassaldos (venta, saldo) ";
			instruccion+="select v.referencia as venta, sum(t.valor) as saldo ";
			instruccion+="from ventas v, ventasaux vaux, transxcob t ";
			instruccion+="where v.referencia=vaux.venta and ";
			instruccion+="t.referencia=v.referencia and t.cancelada=0 and v.cancelado=0 ";
			instruccion+="group by v.referencia";
			instrucciones[num_instrucciones++]=instruccion;

			////////////////// FIN CALCULO DE SALDOS DE VENTAS

			// Crea el buffer con todas las instrucciones SQL
			aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
			for (i=0; i<num_instrucciones; i++)
				aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

			mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
			if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
				// VERIFICA SI QUEDO ALGUN SALDO NEGATIVO DE LAS FACTURAS EN CUESTION
				//
				instruccion="select * from auxventassaldos where saldo<0";
				mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_verificacion);
				if (resp_verificacion->ObtieneNumRegistros()==0) {
					instruccion="COMMIT";
					error=0;
				} else {
					instruccion="ROLLBACK";
					error=1;
				}
			}

		}

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
		if (mServidorVioleta->EjecutaAccionSql(Respuesta, MySQL, instruccion.c_str())) {
			instruccion.sprintf("select %d as error", error);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
			if (error==1) {
				instruccion="select * from auxventassaldos where saldo<0";
				mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
			}
		}
	} __finally {
		if(resp_verificacion!=NULL) delete resp_verificacion;
		delete buffer_sql;
	}
}

//------------------------------------------------------------------------------
//ID_CON_CARGO_REBOTE_CLI
void ServidorVentas::ConsultaCargoReboteCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA CARGO POR REBOTE DEL CLIENTE
	AnsiString instruccion;
	AnsiString clave;

	clave=mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	instruccion.sprintf("select n.*, i.porcentaje as porcimpu, \
		cfd.folio as cfdfolio, cfd.serie as cfdserie, cfdxml.xmlgenerado as cfdxml,  cfdxml.cadenaoriginal as cfdcadenaoriginal, \
		cfd.version, cfd.muuid, cfd.pactimbrador \
		from notascarcli n inner join impuestos i \
		left join cfd on n.referencia=cfd.referencia and cfd.tipocomprobante='NCAR' \
		left join cfdxml on cfd.compfiscal=cfdxml.compfiscal \
		where n.referencia='%s' and n.cveimp=i.impuesto", clave);
	// Obtiene todos los generales (de cabecera) de la nota de cargo
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene lo datos de cargo cobrado por el banco.
	instruccion.sprintf("select * from transxcob where notacar='%s' and \
		concepto='C' and destino='C' and tipo='NCAR' \
		group by referencia", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// NULO:  Verifica que la fecha de la nota de cargo sea posterior a la fecha de cierre.
	instruccion.sprintf("select if(n.fechanot<=cast(e.valor as datetime), 1, 0) as \
		modificar from notascarcli n left join estadosistemaemp as e on e.estado='FUCIERRE' AND e.sucursal = '%s' where n.referencia='%s'",FormServidor->ObtieneClaveSucursal(), clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene las transacciones que eran afectadas por el cheque devuelto
	instruccion.sprintf("select chc.folio, \
		if(ifnull(v.foliofisic,'')='',concat(ifnull(cfd.serie,''),ifnull(cfd.folio,'')),v.foliofisic) as foliofisic, \
		(t.valor*-1) as valor \
		from transxcob t inner join cheqxcob chxc inner join pagoscli p  inner join chequesclientes chc  inner join notascarcli n  inner join ventas v \
		left join cfd on v.referencia=cfd.referencia and cfd.tipocomprobante='VENT' \
		where n.referencia='%s' and \
		n.pago=p.pago and \
		p.pago=t.pago and \
		t.pago=chxc.pago and \
		t.referencia=v.referencia and \
		chxc.chequecli=chc.chequecli and \
		t.cancelada=0 order by t.tracredito", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene los datos del cheque devuelto
	instruccion.sprintf("select chc.chequecli, chc.folio, b.nombre, chc.fechacob \
		from transxcob t, cheqxcob chxc, pagoscli p, chequesclientes chc, notascarcli n, ventas v, bancos b \
		where n.referencia='%s' and \
		n.pago=p.pago and \
		p.pago=t.pago and \
		t.pago=chxc.pago and \
		t.referencia=v.referencia and \
		chxc.chequecli=chc.chequecli and \
		chc.banco=b.banco and \
		t.cancelada=0 order by t.tracredito", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene todos los datos del cliente de la nota de cargo.
	instruccion.sprintf("select c.* from clientes c, notascarcli nc where nc.referencia='%s' and nc.cliente=c.cliente", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

}

//------------------------------------------------------------------------------
//ID_QRY_SALDO_CLI
void ServidorVentas::ConsultaSaldoCliente(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA DE SALDO DE UN CLIENTE A UNA FECHA DADA
	AnsiString instruccion;
	AnsiString clave;
    TDate fecha;

    clave=mFg.ExtraeStringDeBuffer(&parametros);
	fecha=StrToDate(mFg.ExtraeStringDeBuffer(&parametros));
    mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

    // Calcula el saldo del cliente.
	instruccion.sprintf("select sum(t.valor) as saldo from transxcob t, ventas v \
        where v.cliente='%s' and v.cancelado=0 and t.referencia=v.referencia and \
        t.fechaapl<='%s' and t.cancelada=0 group by v.cliente",
        clave, mFg.DateToMySqlDate(fecha));
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}

//------------------------------------------------------------------------------
//ID_QRY_CLIENTE_PARA_VENTA
void ServidorVentas::ConsultaClienteParaVenta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA TODOS LOS DATOS QUE SE REQUIEREN CON RESPECTO A UN CLIENTE AL HACERLE UNA VENTA.
	AnsiString instruccion;
	AnsiString clave, solo_activos, condicion_activos=" ";
	bool calcular_saldo;
	TDate fecha=Today();

	clave=mFg.ExtraeStringDeBuffer(&parametros);
	solo_activos=mFg.ExtraeStringDeBuffer(&parametros);
	calcular_saldo=StrToInt(mFg.ExtraeStringDeBuffer(&parametros));

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	if (solo_activos=="1") {
		condicion_activos=" c.activo=1 and ";
	}

	instruccion.sprintf("select c.tipoempre as tipocliente, c.nombre, c.appat, c.apmat, c.rfc, c.rsocial, c.nomnegocio ,\
		CONCAT(TRIM(CONCAT(TRIM(IFNULL(c.calle,'')),' ',TRIM(IFNULL(c.numext,'')),' ',TRIM(IFNULL(c.numint,''))))) AS calleynum, \
		c.calle, c.colonia, c.cp, c.plazo, c.credito, c.limcred, c.excederlc, c.numpedidos, tdb.clasific, \
		tdb.descripcion, tdb.clasific as clasifbloq, \
		cet.vendedor, cet.cobrador, vend.porccomi as porccomivend, vend.tolercomision as tolcomivend, \
		concat(ven.nombre, ' ', ven.appat, ' ', ven.apmat) as nomvendedor, \
		concat(cob.nombre, ' ', cob.appat, ' ', cob.apmat) as nomcobrador, \
		col.nombre as nomcolonia, \
		loc.nombre as nomlocalidad, mun.nombre as municipio, est.nombre as estado, \
		tdp.tipoprec, tdp.descripcion as descprecio, c.venxvol, c.enviarcfd, c.email, c.email2, tdp.verventmayoreo, \
		c.metododef, c.metodosup, c.usocfdi, c.valorsup,c.numext, c.numint, \
		FORMAT(X(ubicaciongis),6) as latitud, FORMAT(Y(ubicaciongis),6) as longitud, sec.nombre as nomsect, c.referenciadomic, \
		cet.tipoprecmin, c.esparterelac, c.sucremotarelacion, c.usuremotorelacion, CONCAT(c.regimenfiscal, ', ', rf.descripcion ) AS regimenfiscal \
		from clientes c \
		left join clientesemp cet ON cet.cliente=c.cliente AND cet.idempresa=%s \
		left join colonias col on c.colonia=col.colonia \
		LEFT JOIN sectores sec ON sec.sector=col.sector \
		left join localidades loc on col.localidad=loc.localidad \
		left join municipios mun on loc.municipio=mun.municipio \
		left join estados est on mun.estado=est.estado \
		left join tiposdebloqueos tdb on c.bloqueo=tdb.tipo \
		left join tiposdeprecios tdp on tdp.tipoprec=cet.tipoprec \
		left join empleados ven on cet.vendedor=ven.empleado \
		left join empleados cob on cet.cobrador=cob.empleado \
		left join vendedores vend on cet.vendedor=vend.empleado \
		left join cregimenfiscal rf ON c.regimenfiscal = rf.regimenfiscal \
		where %s c.cliente='%s' ", FormServidor->ObtieneClaveEmpresa(), condicion_activos, clave);

	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	instruccion="set @saldo=0";
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query EjecutaSelectSqlNulo"));

	instruccion="set @pedxfact=0";
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query EjecutaSelectSqlNulo"));

	//calcula el saldo, solo si es requerido
	if(calcular_saldo==true){
		// Calcula el saldo real del cliente.
		instruccion.sprintf("select @saldo:=sum(if(t.aplicada=1, t.valor, 0)) from transxcob t \
			INNER JOIN ventas v FORCE INDEX (cliente) ON t.referencia=v.referencia \
			INNER JOIN terminales ter ON ter.terminal = v.terminal \
			INNER JOIN secciones sec ON sec.seccion = ter.seccion \
			INNER JOIN sucursales suc ON suc.sucursal = sec.sucursal \
			where v.cliente='%s' and v.cancelado=0 and t.referencia=v.referencia and \
			t.fechaapl<='%s' and t.cancelada=0 AND suc.idempresa = %s ",
			clave, mFg.DateToMySqlDate(fecha), FormServidor->ObtieneClaveEmpresa());
		mServidorVioleta->EjecutaSelectSqlNulo(Respuesta,  MySQL, instruccion.c_str());

		instruccion.sprintf("SELECT @pedxfact:=sum(p.valor) as pedxfact FROM pedidosventa p \
				INNER JOIN terminales ter ON ter.terminal = p.terminal \
				INNER JOIN secciones sec ON sec.seccion = ter.seccion \
				INNER JOIN sucursales suc ON suc.sucursal = sec.sucursal \
				where p.cliente='%s' and p.cancelado=0 and p.facturado=0 and p.acredito=1 and p.pedimpreso=1 \
				and p.fechaped>=DATE_SUB(CURDATE(), INTERVAL 7 DAY) AND suc.idempresa = %s ",
				clave, FormServidor->ObtieneClaveEmpresa());
		mServidorVioleta->EjecutaSelectSqlNulo(Respuesta,  MySQL, instruccion.c_str());

	}
	instruccion="select ifnull(@saldo,0) as saldo, ifnull(@pedxfact,0) as pedxfact";
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	instruccion.sprintf("select tipo, lada, telefono, extencionTel  from telefonosclientes where cliente='%s'",clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	//consulta de la direccion de entrega adiscional
	instruccion.sprintf("select dir.*,c.nombre as nomcolonia, l.nombre as nomlocalidad, m.nombre as nommunicipio, e.nombre as nomestado, \
                         X(dir.ubicaciongis) AS latitud, Y(dir.ubicaciongis) AS longitud, c.sector \
						from direccionesentregaclientes dir  \
						LEFT JOIN colonias AS c ON c.colonia=dir.colonia  \
						LEFT JOIN localidades AS l ON l.localidad=c.localidad \
						LEFT JOIN municipios AS m ON m.municipio=l.municipio \
						LEFT JOIN estados AS e ON e.estado=m.estado \
						where  dir.cliente='%s' ",clave);
    mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	//Verifica si es parte relacionada
	instruccion.sprintf("SELECT \
	IF(pcfd.rfcemisor=REPLACE(REPLACE(c.rfc,'-',''),' ',''),1,0)  AS relacion \
	FROM clientes c \
	LEFT JOIN parametroscfd pcfd ON pcfd.rfcemisor = REPLACE(REPLACE(c.rfc,'-',''),' ','') \
	WHERE c.cliente = '%s' \
	GROUP BY c.cliente ",clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	//Verifica si es parte relacionada sucursal remota
	instruccion.sprintf(" SELECT \
	IF(pcfd.rfcemisor<>REPLACE(REPLACE(c.rfc,'-',''),' ',''),1,0)  AS relacion  \
	FROM clientes c \
	LEFT JOIN parametroscfd pcfd ON pcfd.rfcemisor <> REPLACE(REPLACE(c.rfc,'-',''),' ','') \
	WHERE c.cliente = '%s' 	AND c.esparterelac=1 \
	GROUP BY c.cliente", clave);
    mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

}


//------------------------------------------------------------------------------
//ID_CON_PED_IMPRESION
void ServidorVentas::ConsultaPedidoParaImprimir(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*64*10];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString tarea, clave, ident_cola, terminal;
	int num_partidas, i;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[1000];
	char *resultado;
	double totalng, totalg, subtotalg, subtotalgiva, total, subtotal, subtotaliva;
	ArregloDetalle ArregloVenta;
	AnsiString total_enunciado, subtotal_enunciado, subtotaliva_enunciado;
	BufferRespuestas* resp_telefonos=NULL;
	AnsiString telefonos, telefonos_principales, tipo, lada, telefono,extencion;
	int conta_telefonos_principales;
	double totalpeso, totalvol, totalcajasbultos, peso, volumen, cajasbultos;
	double totalcantidad, cantidad;
	bool agregado_total_iva, agregado_total_iesps;
	AnsiString mensaje_dividido[3];

	double iesps20, iesps25, iesps30, iesps50;
	AnsiString iesps_detallados;
	AnsiString ordenValor,identificadorCola;

	try{
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave del pedido
		terminal=mFg.ExtraeStringDeBuffer(&parametros); // Terminal desde la que se imprime.
		identificadorCola=mFg.ExtraeStringDeBuffer(&parametros); //indentificador de cola " "
		ordenValor=mFg.ExtraeStringDeBuffer(&parametros); //tipo de orden, default|por multiplo agarra el numero de la terminal

		// Obtiene el precio público
		instruccion.sprintf("select @cvepreciopublico:=valor from parametrosemp where parametro='PRECIOPUBLIC' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
		instrucciones[num_instrucciones++]=instruccion;

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {

			// Obtiene los teléfonos
			instruccion.sprintf("select t.tipo as tipo, t.lada as lada, t.telefono as telefono, t.extencionTel  \
				from telefonosclientes t, pedidosventa p where p.cliente=t.cliente and p.referencia='%s'",clave);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_telefonos);
			telefonos="";
			telefonos_principales="";
			conta_telefonos_principales=0;
			for(i=0; i<resp_telefonos->ObtieneNumRegistros(); i++){
					resp_telefonos->IrAlRegistroNumero(i);
					tipo=resp_telefonos->ObtieneDato("tipo");
					lada=resp_telefonos->ObtieneDato("lada");
					telefono=resp_telefonos->ObtieneDato("telefono");
					extencion=resp_telefonos->ObtieneDato("extencionTel");
					if (tipo=="Negocio" || tipo=="Particular") {
						if (conta_telefonos_principales<2 ) {

							telefonos_principales+=lada;
							telefonos_principales+=" ";
							telefonos_principales+=telefono;
							telefonos_principales+="  ";
							//condicion para ingresar una extención
							if (extencion!="") {
								telefonos_principales+="Ext. ";
								telefonos_principales+=extencion;
								telefonos_principales+="  ";
							}
							telefonos+=tipo;
							telefonos+=" ";
							telefonos+=lada;
							telefonos+=" ";
							telefonos+=telefono;
							telefonos+="  ";

						}
						conta_telefonos_principales++;
					}
			}
			telefonos+=" ";
			telefonos_principales+=" ";

			// Obtiene todos los generales (de cabecera) del pedido
			instruccion.sprintf("select c.*, p.*, pmen.mensaje, \
				if(p.cotizacion=1,'COTIZACION','PEDIDO') as nomtipoped, \
				if(p.acredito=1,'CREDITO','CONTADO') as nomacredito, \
				'%s' as telefonos, '%s' as telprincipales, \
				@forma:=t.formaped as forma, t.tiporfc, \
				col.nombre as nomcolonia, \
				concat(loc.nombre, '  ', est.estado) as ciudadestado, \
				CONCAT(TRIM(CONCAT(TRIM(IFNULL(c.calle,'')),' ',TRIM(IFNULL(c.numext,'')),' ',TRIM(IFNULL(c.numint,'')))), ' COL.: ', col.nombre) AS direccioncol, \
				loc.localidad as cvelocalidad, loc.nombre as nomlocalidad, \
				mun.municipio as cvemunicipio, mun.nombre as nommunicipio, \
				sec.nombre as nomsector, \
				est.estado as cveestado, est.nombre as nomestado, \
				emp1.nombre as nomcajero, \
				p.comisionada as comisionada, emp2.nombre as nomvendedor, \
				tp.descripcion as nomtermino, \
				emp3.nombre as nomusualta \
				from pedidosventa p  inner join  clientes c  inner join  tiposfacturasventas t  inner join  terminosdepago tp \
				left join pedidosmensajes pmen on pmen.referencia=p.referencia \
				left join colonias col on c.colonia=col.colonia \
				left join sectores sec on col.sector=sec.sector \
				left join localidades loc on col.localidad=loc.localidad \
				left join municipios mun on mun.municipio=loc.municipio \
				left join estados est on est.estado=mun.estado \
				left join empleados emp1 on p.usumodi=emp1.empleado \
				left join empleados emp2 on p.vendedor=emp2.empleado \
				left join empleados emp3 on p.usualta=emp3.empleado \
				where p.termino=tp.termino and p.referencia='%s' and p.cliente=c.cliente and p.tipofac=t.tipo",
				telefonos, telefonos_principales, clave);

			resultado=Respuesta->ObtieneDirLibreBuffer();
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
			BufferRespuestas resp_venta(resultado);

			// Obtiene los datos del pedido completo
			instruccion="select d.referencia, d.articulo, d.cantidad, d.costobase, ";
			instruccion+="d.porcdesc, cast(d.precio as decimal(16,6)) as precio, cast(d.precioimp as decimal(16,6)) as precioimp, cast(pp.precio/a.factor as decimal(16,6)) as preciopub, ";
			instruccion+="a.present, p.producto, p.nombre as nombre, a.multiplo, ";
			instruccion+="concat(p.nombre,' ',a.present) as nomcompleto, ";
			instruccion+="a.factor, a.volumen,a.peso as pesounit,(a.peso*d.cantidad) As peso, ";
			instruccion+="(a.factor / pmm.cajalogisticafactor) AS cajasbultos, ";
			instruccion+="concat(left(a.multiplo,3),'-',a.factor) as multfactor, ";
			instruccion+="d.claveimp1, i1.tipoimpu as tipoimpuesto1, i1.porcentaje as porcentajeimp1, t1.nombre as nomtipoimp1, ";
			instruccion+="d.claveimp2, i2.tipoimpu as tipoimpuesto2, i2.porcentaje as porcentajeimp2, t2.nombre as nomtipoimp2, ";
			instruccion+="d.claveimp3, i3.tipoimpu as tipoimpuesto3, i3.porcentaje as porcentajeimp3, t3.nombre as nomtipoimp3, ";
			instruccion+="d.claveimp4, i4.tipoimpu as tipoimpuesto4, i4.porcentaje as porcentajeimp4, t4.nombre as nomtipoimp4, ";
			instruccion+="cast(precioimp*cantidad as decimal(16,6)) as importe, ";
			instruccion+="cast(d.precio*cantidad as decimal(16,6)) as importedesg, ";
			instruccion+="@porciva:=(if(i1.tipoimpu='IVA',i1.porcentaje,0)+if(i2.tipoimpu='IVA',i2.porcentaje, 0)+if(i3.tipoimpu='IVA',i3.porcentaje, 0)+if(i4.tipoimpu='IVA',i4.porcentaje, 0)) as porciva, ";
			instruccion+="@porciesps:=(if(i1.tipoimpu='IESPS',i1.porcentaje,0)+if(i2.tipoimpu='IESPS',i2.porcentaje, 0)+if(i3.tipoimpu='IESPS',i3.porcentaje, 0)+if(i4.tipoimpu='IESPS',i4.porcentaje, 0)) as porciesps, ";
			instruccion+="cast((precioimp*cantidad)/(1+@porciva/100) as decimal(16,6)) as importeivadesg, ";
			instruccion+="cast((precioimp)/(1+@porciva/100) as decimal(16,6)) as precioivadesg ";
			if(ordenValor=="1")
				instruccion+=",IF(a.factor=1,1,2) ORDEN,p.clasif1  "; //Agrupacion para ordenar por multiplo

			instruccion+="from dpedidosventa d  inner join  articulos a  inner join  productos p ";
			instruccion+="left join impuestos i1 on i1.impuesto=d.claveimp1 ";
			instruccion+="left join impuestos i2 on i2.impuesto=d.claveimp2 ";
			instruccion+="left join impuestos i3 on i3.impuesto=d.claveimp3 ";
			instruccion+="left join impuestos i4 on i4.impuesto=d.claveimp4 ";
			instruccion+="left join tiposdeimpuestos t1 on t1.tipoimpu=i1.tipoimpu ";
			instruccion+="left join tiposdeimpuestos t2 on t2.tipoimpu=i2.tipoimpu ";
			instruccion+="left join tiposdeimpuestos t3 on t3.tipoimpu=i3.tipoimpu ";
			instruccion+="left join tiposdeimpuestos t4 on t4.tipoimpu=i4.tipoimpu ";
			instruccion+="left join precios pp on pp.tipoprec=@cvepreciopublico and pp.articulo=d.articulo ";
			instruccion+="left join tiposdeprecios tp ON tp.tipoprec=pp.tipoprec and tp.idempresa="+FormServidor->ObtieneClaveEmpresa()+" ";
			instruccion+="left join presentacionesminmax pmm ON a.producto=pmm.producto AND a.present=pmm.present ";
			instruccion+="where d.referencia='";
			instruccion+=clave;
			instruccion+="' and d.articulo=a.articulo and a.producto=p.producto ";
			instruccion+=" group by p.nombre, a.present, a.multiplo ";
			if(ordenValor=="1")
				instruccion+="ORDER BY ORDEN DESC,IF(ORDEN='1',p.clasif1,left(a.multiplo,3)),p.nombre, a.present, a.multiplo  ";
			else
				instruccion+="ORDER by d.id, p.nombre, a.present, a.multiplo ";

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
					peso=StrToFloat(resp_detalle.ObtieneDato("pesounit"));
					volumen=StrToFloat(resp_detalle.ObtieneDato("volumen"));
					cajasbultos=StrToFloat(resp_detalle.ObtieneDato("cajasbultos"));

					totalcantidad+=cantidad;
					totalpeso+=cantidad*peso;
					totalvol+=cantidad*volumen;
					totalcajasbultos+= cantidad*cajasbultos;
			}

			// *********************************************************
			//  Calcula totales fiscales.
			ArregloVenta.UsoDelArreglo=PEDI_VENTA;
			ArregloVenta.PosicionArreglo=SIN_POSICION;
			ArregloVenta.CargaBufferEnArregloDetalleVentas(&resp_detalle);
			ArregloVenta.AsignaModoAjusteRedondeo(StrToInt(resp_venta.ObtieneDato("redondeoantiguo")));
			ArregloVenta.RecalculaTotalesVenta();

			// Total no grabado
			totalng=mFg.CadenaAFlotante(ArregloVenta.ArregloContenedorTotales[1][4]);
			// Total grabado  -> SIN RFC
			totalg=mFg.CadenaAFlotante(ArregloVenta.ArregloContenedorTotales[1][5]);
			for (i=6; i<=ArregloVenta.IndiceArregloTotales; i++)
				totalg+=mFg.CadenaAFlotante(ArregloVenta.ArregloContenedorTotales[1][i]);
			// Subtotal grabado (con todos los impuestos desglosados) -> CON RFC (IESPS)
			subtotalg=mFg.CadenaAFlotante(ArregloVenta.ArregloContenedorTotales[1][5]);
			// SubTotal grabado (con iva desglosado) -> CON RFC
			subtotalgiva=0;
			for (i=5; i<=ArregloVenta.IndiceArregloTotales; i++) {
				if (ArregloVenta.ArregloContenedorTotales[2][i]!="IVA")
					subtotalgiva+=mFg.CadenaAFlotante(ArregloVenta.ArregloContenedorTotales[1][i]);
			}
			// Total
			total=0;
			for (i=4; i<=ArregloVenta.IndiceArregloTotales; i++)
				total+=mFg.CadenaAFlotante(ArregloVenta.ArregloContenedorTotales[1][i]);
			// Total enunciado
			total_enunciado=mFg.GeneraEnunciadoDeCantidad(total);
			// Subtotal (el subtotal de lo grabado + lo no grabado = total-impuestos)
			subtotal=subtotalg+totalng;
			// Subtotaliva (El subtotal sin iva de lo grabado + lo no grabado = total-iva)
			subtotaliva=subtotalgiva+totalng;
			// SubTotal enunciado
			subtotal_enunciado=mFg.GeneraEnunciadoDeCantidad(subtotal);
			// SubTotalIva enunciado
			subtotaliva_enunciado=mFg.GeneraEnunciadoDeCantidad(subtotaliva);
			// Divide el mensaje
			mFg.WordWrap(resp_venta.ObtieneDato("mensaje"), mensaje_dividido, 50, 3);

			// Los diferentes IESPS
			iesps20=ArregloVenta.ObtieneTotalImpuesto("IESPS", 20);
			iesps25=ArregloVenta.ObtieneTotalImpuesto("IESPS", 25);
			iesps30=ArregloVenta.ObtieneTotalImpuesto("IESPS", 30);
			iesps50=ArregloVenta.ObtieneTotalImpuesto("IESPS", 50);
			if ( (iesps20+iesps25+iesps30+iesps50) > 0) {
				if ( iesps20>0) {
					iesps_detallados+=mFg.FormateaCantidad(iesps20,2,false);
					iesps_detallados+="(20%)  ";
				}
				if ( iesps25>0) {
					iesps_detallados+=mFg.FormateaCantidad(iesps25,2,false);
					iesps_detallados+="(25%)  ";
				}
				if ( iesps30>0) {
					iesps_detallados+=mFg.FormateaCantidad(iesps30,2,false);
					iesps_detallados+="(30%)  ";
				}
				if ( iesps50>0) {
					iesps_detallados+=mFg.FormateaCantidad(iesps50,2,false);
					iesps_detallados+="(50%)  ";
				}
			}

			// Totales de la venta
			instruccion="select '";
			instruccion+=mensaje_dividido[0];
			instruccion+="' as mensajep1, '";
			instruccion+=mensaje_dividido[1];
			instruccion+="' as mensajep2, '";
			instruccion+=mensaje_dividido[2];
			instruccion+="' as mensajep3, '";
			instruccion+=total_enunciado;
			instruccion+="' as pedtotalenunciado, '";
			instruccion+=subtotal_enunciado;
			instruccion+="' as pedsubtotenunciado, '";
			instruccion+=subtotaliva_enunciado;
			instruccion+="' as pedsubivaenunciado, '";
			instruccion+=iesps_detallados;
			instruccion+="' as pediespsdetallados, ";
			instruccion+=mFg.FormateaCantidad(totalng,2,false);
			instruccion+=" as pedtotalng, ";
			instruccion+=mFg.FormateaCantidad(totalg,2,false);
			instruccion+=" as pedtotalg, ";
			instruccion+=mFg.FormateaCantidad(subtotal,2,false);
			instruccion+=" as pedsubtotal, ";
			instruccion+=mFg.FormateaCantidad(subtotaliva,2,false);
			instruccion+=" as pedsubtotaliva, ";
			instruccion+=mFg.FormateaCantidad(subtotalg,2,false);
			instruccion+=" as pedsubtotalg, ";
			instruccion+=mFg.FormateaCantidad(subtotalgiva,2,false);
			instruccion+=" as pedsubtotalgiva, ";
			instruccion+=mFg.FormateaCantidad(total,2,false);
			instruccion+=" as pedtotal, ";

			instruccion+=mFg.FormateaCantidad(iesps20,2,false);
			instruccion+=" as pediesps20, ";
			instruccion+=mFg.FormateaCantidad(iesps25,2,false);
			instruccion+=" as pediesps25, ";
			instruccion+=mFg.FormateaCantidad(iesps30,2,false);
			instruccion+=" as pediesps30, ";
			instruccion+=mFg.FormateaCantidad(iesps50,2,false);
			instruccion+=" as pediesps50, ";

			instruccion+=mFg.FormateaCantidad(totalcantidad,3,false);
			instruccion+=" as pedtotalcant, ";
			instruccion+=mFg.FormateaCantidad(totalpeso,2,false);
			instruccion+=" as pedtotalpeso, ";
			instruccion+=mFg.FormateaCantidad(totalvol,2,false);
			instruccion+=" as pedtotalvol, ";
			instruccion+=mFg.FormateaCantidad(totalcajasbultos,2,false);
			instruccion+=" as pedtotalcajas ";
			agregado_total_iva=false;
			agregado_total_iesps=false;
			for (i=6; i<=ArregloVenta.IndiceArregloTotales; i++) {
				instruccion+=", ";
				instruccion+=mFg.FormateaCantidad(ArregloVenta.ArregloContenedorTotales[1][i],2,false);
				instruccion+=" as pedimp";
				instruccion+=ArregloVenta.ArregloContenedorTotales[2][i].LowerCase();
				if (ArregloVenta.ArregloContenedorTotales[2][i].LowerCase()=="iva")
					agregado_total_iva=true;
				if (ArregloVenta.ArregloContenedorTotales[2][i].LowerCase()=="iesps")
					agregado_total_iesps=true;
			}
			// Si no se agregó un total de iva y de iesps, lo agrega con valor=0
			if (!agregado_total_iva) instruccion+=", 0.00 as pedimpiva ";
			if (!agregado_total_iesps) instruccion+=", 0.00 as pedimpiesps ";
			// Ejecuta la instrucción
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Obtiene todos los datos de la forma
			instruccion.sprintf("select * from formas where forma=@forma");
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Obtiene todo el detalle de la forma
			instruccion.sprintf("select * from dformas where forma=@forma");
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);


			instruccion.sprintf(" select (CASE WHEN  concat(cli.calle, cli.numext,cli.numint,cli.colonia,cli.cp)=concat(pde.calle, pde.numext, pde.numint,pde.colonia ,pde.cp) THEN 1 ELSE 0 END) AS fiscal, \
						pde.*,l.nombre AS localidad, m.nombre AS municipio,e.nombre AS estado,'' AS referenciadomic, \
						'' AS ubicacion, c.colonia AS cveColonia, l.localidad AS cveLocalidad, m.municipio AS cveMunicipio, e.estado AS cveEstado,'' AS latitud, '' AS longitud  \
						, c.nombre AS NombreColonia,c.sector AS DirEnSector, sec.nombre as nombreSectorEntrega \
						FROM clientes cli  \
						inner join pedidosdirent pde on  pde.cliente=cli.cliente  \
						inner join pedidosventa p ON pde.referencia=p.referencia AND pde.cliente=p.cliente  \
						LEFT JOIN colonias c ON c.colonia=pde.colonia \
						INNER JOIN	sectores sec ON sec.sector = c.sector \
						LEFT JOIN localidades l ON l.localidad=c.localidad \
						LEFT JOIN municipios m ON m.municipio=l.municipio  \
						LEFT JOIN estados e ON e.estado=m.estado \
						WHERE p.referencia IN ('%s')",clave);

			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

		}
	} __finally {
		if(resp_telefonos!=NULL) delete resp_telefonos;
		delete buffer_sql;
	}
}


//------------------------------------------------------------------------------
// ID_CON_VTA_IMPRESION
void ServidorVentas::ConsultaVentaParaImprimir(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*64*10];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString tarea, clave, ident_cola, terminal;
	int num_partidas, i;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[1100];
	char *resultado;
	double totalng, totalg, subtotalg, subtotalgiva, total, subtotal, subtotaliva;
	ArregloDetalle ArregloVenta;
	AnsiString total_enunciado, subtotal_enunciado, subtotaliva_enunciado;
	BufferRespuestas* resp_telefonos=NULL;
	AnsiString telefonos, telefonos_principales, tipo, lada, telefono,extencion;
	int conta_telefonos_principales;
	double totalpeso, totalvol, peso, volumen;
	double totalcantidad, cantidad;
	bool agregado_total_iva, agregado_total_iesps;
	AnsiString cambiar_foliofisico;
	bool detdesglos;
	AnsiString mensaje_dividido[3];
	AnsiString cvecliente, ventacredito;
	double iesps20, iesps25, iesps30, iesps50;
	AnsiString iesps_detallados;
	AnsiString ordenValor;
	AnsiString conVentasCongeladas;
	try{
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave de la venta
		ident_cola=mFg.ExtraeStringDeBuffer(&parametros); // Clave identificadora en la cola de impresión (si es vacia entonces es que es de impresión directa)
		terminal=mFg.ExtraeStringDeBuffer(&parametros); // Terminal desde la que se imprime.
		cambiar_foliofisico=mFg.ExtraeStringDeBuffer(&parametros); // Indica si se debe cambiar el folio fìsico o no
		ordenValor=mFg.ExtraeStringDeBuffer(&parametros); //tipo de orden, default|por multiplo agarra el numero de la terminal
		conVentasCongeladas=mFg.ExtraeStringDeBuffer(&parametros); //parametro "0" sin ventas congeladas y con 1 si incluye ventas congeladas para el saldo

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		if (cambiar_foliofisico=="1") {
			// Obtiene el folio físico
			instruccion.sprintf("select @folioaux:=folvta, @serieaux:=serievta, @anchofolvta:=anchofolvta from terminales where terminal='%s'", terminal);
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @foliosig=@folioaux+1");
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @folioaux=cast(@folioaux as char)");
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @folio=concat(@serieaux, lpad(@folioaux,@anchofolvta,'0') )");
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("update terminales t, ventas v set t.folvta=@foliosig, v.foliofisic=@folio where t.terminal='%s' and v.referencia='%s'", terminal,clave);
			instrucciones[num_instrucciones++]=instruccion;
		}
		instruccion.sprintf("select @folio:=foliofisic from ventas where referencia='%s'", clave);
		instrucciones[num_instrucciones++]=instruccion;


		// Marca como impresa la venta.
		instruccion.sprintf("update colaimpresion set yaimpreso=1 where ident='%s'",ident_cola);
		instrucciones[num_instrucciones++]=instruccion;

		instrucciones[num_instrucciones++]="COMMIT";

		// Obtiene el precio público
		instruccion.sprintf("select @cvepreciopublico:=valor from parametrosemp where parametro='PRECIOPUBLIC' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
		instrucciones[num_instrucciones++]=instruccion;

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {


			// Obtiene los teléfonos
			instruccion.sprintf("select t.tipo as tipo, t.lada as lada, t.telefono as telefono, t.extencionTel as ext\
				from telefonosclientes t, ventas v where v.cliente=t.cliente and v.referencia='%s'",clave);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_telefonos);
			telefonos="";
			telefonos_principales="";
			conta_telefonos_principales=0;
			for(i=0; i<resp_telefonos->ObtieneNumRegistros(); i++){
					resp_telefonos->IrAlRegistroNumero(i);
					tipo=resp_telefonos->ObtieneDato("tipo");
					lada=resp_telefonos->ObtieneDato("lada");
					telefono=resp_telefonos->ObtieneDato("telefono");
					extencion=resp_telefonos->ObtieneDato("ext");
					if (tipo=="Negocio" || tipo=="Particular" || tipo=="Celular"|| tipo=="Fax") {
						if (conta_telefonos_principales<2 ) {
							telefonos_principales+=lada;
							telefonos_principales+=" ";
							telefonos_principales+=telefono;
							telefonos_principales+="  ";

							//condicion para ingresar una extención
							if (extencion!="") {
								telefonos_principales+="Ext. ";
								telefonos_principales+=extencion;
								telefonos_principales+="  ";
							}

							telefonos+=tipo;
							telefonos+=" ";
							telefonos+=lada;
							telefonos+=" ";
							telefonos+=telefono;
							telefonos+="  ";
						}
						conta_telefonos_principales++;
					}
			}
			telefonos+=" ";
			telefonos_principales+=" ";

			// Obtiene todos los generales (de cabecera) de la venta
			instruccion.sprintf("select c.*, v.*, vm.mensaje, \
				if(v.acredito=1,'CREDITO','CONTADO') as nomacredito, \
				'%s' as telefonos, '%s' as telprincipales, \
				@forma:=t.forma as forma, t.tiporfc, \
				col.nombre as nomcolonia, \
				concat(loc.nombre, '  ', est.estado) as ciudadestado, \
				CONCAT(TRIM(CONCAT(TRIM(IFNULL(c.calle,'')),' ',TRIM(IFNULL(c.numext,'')),' ',TRIM(IFNULL(c.numint,'')))), ' COL.: ', col.nombre) AS direccioncol, \
				loc.localidad as cvelocalidad, loc.nombre as nomlocalidad, \
				mun.municipio as cvemunicipio, mun.nombre as nommunicipio, \
				sec.nombre as nomsector,\
				est.estado as cveestado, est.nombre as nomestado, \
				ter.nombre as nomterminal, emp1.nombre as nomcajero, \
				v.comisionada as comisionada, CONCAT(emp2.nombre, ' ', emp2.appat) as nomvendedor, \
				CONCAT(emp3.nombre, ' ', emp3.appat) as nomcobrador, emp4.nombre as nomchofer, dvfp2.descripcion as nomtermino, \
				@cantkits:=ifnull(v.cantkits,0) as cantkitsb, \
				@nombrekit:=ifnull(kt.nombre,'') as nombrekit, ifnull(kt.desglosado,1) as detdesglos, \
				cfd.desgloseimpuestos33, \
				ifnull(concat( vde.calle,' ', vde.numext, ' ', vde.numint,' ', co.nombre,' ', vde.cp,' ',vde.referenciadom), '') as vtadirentrega \
				,se.sucursal , co.sector AS SecDirEnt , xec.nombre as DirEntNomsector \
				from ventas v  inner join  clientes c  inner join  tiposfacturasventas t \
				INNER JOIN \
						  (SELECT fp2.descripcion, dvfp3.referencia \
						   FROM dventasfpago dvfp3 \
						   INNER JOIN formasdepago fp2 ON fp2.formapag=dvfp3.formapag \
						   WHERE dvfp3.referencia='%s' AND dvfp3.valor = \
						   (SELECT max(dvfp4.valor) FROM dventasfpago dvfp4 \
						   WHERE dvfp4.referencia = '%s') LIMIT 1 \
						   ) dvfp2 ON dvfp2.referencia = v.referencia \
				left join cfd on v.referencia=cfd.referencia and cfd.tipocomprobante='VENT' \
				left join ventasmensajes vm on vm.referencia=v.referencia \
				left join colonias col on c.colonia=col.colonia \
				left join sectores sec on col.sector=sec.sector \
				left join localidades loc on col.localidad=loc.localidad \
				left join municipios mun on mun.municipio=loc.municipio \
				left join estados est on est.estado=mun.estado \
				left join terminales ter on v.terminal=ter.terminal \
				left join empleados emp1 on v.usumodi=emp1.empleado \
				left join empleados emp2 on v.vendedor=emp2.empleado \
				left join empleados emp3 on v.cobrador=emp3.empleado \
				left join embarques emb on v.embarque=emb.embarque \
				left join empleados emp4 on emb.chofer=emp4.empleado \
				left join kits kt on v.kit=kt.kit \
				LEFT join ventadirent vde on vde.referencia=v.referencia and vde.cliente=v.cliente \
				LEFT JOIN colonias co ON vde.colonia=co.colonia  \
				LEFT JOIN sectores xec ON co.sector=xec.sector \
                LEFT JOIN secciones se ON se.seccion=ter.seccion \
				where v.referencia='%s' and v.cliente=c.cliente and v.tipofac=t.tipo",
				telefonos, telefonos_principales, clave, clave, clave);
			resultado=Respuesta->ObtieneDirLibreBuffer();
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
			BufferRespuestas resp_venta(resultado);
			detdesglos=StrToInt(resp_venta.ObtieneDato("detdesglos"));
			cvecliente=resp_venta.ObtieneDato("cliente");
			ventacredito=resp_venta.ObtieneDato("acredito");

			// Obtiene los datos de la venta completa
			instruccion="select d.referencia, d.articulo, d.cantidad, d.costobase,";
			instruccion+="d.porcdesc, d.tipoprec, cast(d.precio as decimal(16,6)) as precio, cast(d.precioimp as decimal(16,6)) as precioimp, cast(pp.precio/a.factor as decimal(16,6)) as preciopub, ";
			instruccion+="a.present, p.producto, p.nombre as nombre, a.multiplo, ";
			instruccion+="concat(p.nombre,' ',a.present) as nomcompleto, ";
			instruccion+="d.almacen, a.factor, a.volumen, a.peso, ";
			instruccion+="concat(left(a.multiplo,3),'-',a.factor) as multfactor, ";
			instruccion+="d.claveimp1, i1.tipoimpu as tipoimpuesto1, i1.porcentaje as porcentajeimp1, t1.nombre as nomtipoimp1, ";
			instruccion+="d.claveimp2, i2.tipoimpu as tipoimpuesto2, i2.porcentaje as porcentajeimp2, t2.nombre as nomtipoimp2, ";
			instruccion+="d.claveimp3, i3.tipoimpu as tipoimpuesto3, i3.porcentaje as porcentajeimp3, t3.nombre as nomtipoimp3, ";
			instruccion+="d.claveimp4, i4.tipoimpu as tipoimpuesto4, i4.porcentaje as porcentajeimp4, t4.nombre as nomtipoimp4, ";
			instruccion+="d.almacen, ";
			instruccion+="a.ean13, ";
			instruccion+="cast(precioimp*d.cantidad as decimal(16,6)) as importe, ";
			instruccion+="cast(d.precio*d.cantidad as decimal(16,6)) as importedesg, ";
			instruccion+="@porciva:=(if(i1.tipoimpu='IVA',i1.porcentaje,0)+if(i2.tipoimpu='IVA',i2.porcentaje, 0)+if(i3.tipoimpu='IVA',i3.porcentaje, 0)+if(i4.tipoimpu='IVA',i4.porcentaje, 0)) as porciva, ";
			instruccion+="@porciesps:=(if(i1.tipoimpu='IESPS',i1.porcentaje,0)+if(i2.tipoimpu='IESPS',i2.porcentaje, 0)+if(i3.tipoimpu='IESPS',i3.porcentaje, 0)+if(i4.tipoimpu='IESPS',i4.porcentaje, 0)) as porciesps, ";
			instruccion+="cast((precioimp*d.cantidad)/(1+@porciva/100) as decimal(16,6)) as importeivadesg, ";
			instruccion+="cast((precioimp)/(1+@porciva/100) as decimal(16,6)) as precioivadesg ";
			if(ordenValor=="1")
				instruccion+=",IF(a.factor=1,1,2) ORDEN,p.clasif1  "; //Agrupacion para ordenar por multiplo
			instruccion+=", ifnull(dvk.kit,'') as kit, ifnull(k.nombre,'') as nkit, ifnull(vk.desglosado,1) as desglosado,ifnull(k.ean13,'') AS kean13 ";
			instruccion+="from dventas d  inner join  articulos a  inner join  productos p ";
			instruccion+="left join impuestos i1 on i1.impuesto=d.claveimp1 ";
			instruccion+="left join impuestos i2 on i2.impuesto=d.claveimp2 ";
			instruccion+="left join impuestos i3 on i3.impuesto=d.claveimp3 ";
			instruccion+="left join impuestos i4 on i4.impuesto=d.claveimp4 ";
			instruccion+="left join tiposdeimpuestos t1 on t1.tipoimpu=i1.tipoimpu ";
			instruccion+="left join tiposdeimpuestos t2 on t2.tipoimpu=i2.tipoimpu ";
			instruccion+="left join tiposdeimpuestos t3 on t3.tipoimpu=i3.tipoimpu ";
			instruccion+="left join tiposdeimpuestos t4 on t4.tipoimpu=i4.tipoimpu ";
			instruccion+="left join precios pp on pp.tipoprec=@cvepreciopublico and pp.articulo=d.articulo ";
			instruccion+="left join tiposdeprecios tp ON tp.tipoprec=pp.tipoprec and tp.idempresa="+FormServidor->ObtieneClaveEmpresa()+" ";
			instruccion+="left join dventaskits dvk on dvk.venta=d.referencia and dvk.articulo=d.articulo ";
			instruccion+="left join ventaskits vk on vk.venta=d.referencia and vk.kit=dvk.kit ";
			instruccion+="left join kits k on k.kit=vk.kit ";
			instruccion+="where d.referencia='";
			instruccion+=clave;
			instruccion+="' and d.articulo=a.articulo and a.producto=p.producto ";
			instruccion+=" group by p.nombre, a.present, a.multiplo ";
			if(ordenValor=="1")
				instruccion+=" ORDER BY ORDEN DESC,IF(ORDEN='1',p.clasif1,left(a.multiplo,3)),p.nombre, a.present, a.multiplo ";
			else
				instruccion+=" order by d.id, p.nombre, a.present, a.multiplo ";


			resultado=Respuesta->ObtieneDirLibreBuffer();
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
			BufferRespuestas resp_detalle(resultado);

			// *********************************************************
			// Calcula total de articulos, total de peso y total de volumen.
			totalcantidad=0;
			totalpeso=0;
			totalvol=0;
			for(i=0; i<resp_detalle.ObtieneNumRegistros(); i++){
					resp_detalle.IrAlRegistroNumero(i);
					cantidad=mFg.CadenaAFlotante(resp_detalle.ObtieneDato("cantidad"));
					peso=StrToFloat(resp_detalle.ObtieneDato("peso"));
					volumen=StrToFloat(resp_detalle.ObtieneDato("volumen"));
					totalcantidad+=cantidad;
					totalpeso+=cantidad*peso;
					totalvol+=cantidad*volumen;
			}

			// *********************************************************
			//  Calcula totales fiscales.
			ArregloVenta.UsoDelArreglo=FACT_VENTA;
			ArregloVenta.PosicionArreglo=SIN_POSICION;
			ArregloVenta.CargaBufferEnArregloDetalleVentas(&resp_detalle);
			ArregloVenta.AsignaModoAjusteRedondeo(StrToInt(resp_venta.ObtieneDato("redondeoantiguo")));
			ArregloVenta.RecalculaTotalesVenta();

			// Total no grabado
			totalng=mFg.CadenaAFlotante(ArregloVenta.ArregloContenedorTotales[1][4]);
			// Total grabado  -> SIN RFC
			totalg=mFg.CadenaAFlotante(ArregloVenta.ArregloContenedorTotales[1][5]);
			for (i=6; i<=ArregloVenta.IndiceArregloTotales; i++)
				totalg+=mFg.CadenaAFlotante(ArregloVenta.ArregloContenedorTotales[1][i]);
			// Subtotal grabado (con todos los impuestos desglosados) -> CON RFC (IESPS)
			subtotalg=mFg.CadenaAFlotante(ArregloVenta.ArregloContenedorTotales[1][5]);
			// SubTotal grabado (con iva desglosado) -> CON RFC
			subtotalgiva=0;
			for (i=5; i<=ArregloVenta.IndiceArregloTotales; i++) {
				if (ArregloVenta.ArregloContenedorTotales[2][i]!="IVA")
					subtotalgiva+=mFg.CadenaAFlotante(ArregloVenta.ArregloContenedorTotales[1][i]);
			}
			// Total
			total=0;
			for (i=4; i<=ArregloVenta.IndiceArregloTotales; i++)
				total+=mFg.CadenaAFlotante(ArregloVenta.ArregloContenedorTotales[1][i]);
			// Total enunciado
			total_enunciado=mFg.GeneraEnunciadoDeCantidad(total);
			// Subtotal (el subtotal de lo grabado + lo no grabado = total-impuestos)
			subtotal=subtotalg+totalng;
			// Subtotaliva (El subtotal sin iva de lo grabado + lo no grabado = total-iva)
			subtotaliva=subtotalgiva+totalng;
			// SubTotal enunciado
			subtotal_enunciado=mFg.GeneraEnunciadoDeCantidad(subtotal);
			// SubTotalIva enunciado
			subtotaliva_enunciado=mFg.GeneraEnunciadoDeCantidad(subtotaliva);
			// Divide el mensaje
			mFg.WordWrap(resp_venta.ObtieneDato("mensaje"), mensaje_dividido, 50, 3);
			// Los diferentes IESPS
			iesps20=ArregloVenta.ObtieneTotalImpuesto("IESPS", 20);
			iesps25=ArregloVenta.ObtieneTotalImpuesto("IESPS", 25);
			iesps30=ArregloVenta.ObtieneTotalImpuesto("IESPS", 30);
			iesps50=ArregloVenta.ObtieneTotalImpuesto("IESPS", 50);
			if ( (iesps20+iesps25+iesps30+iesps50) > 0) {
				if ( iesps20>0) {
					iesps_detallados+=mFg.FormateaCantidad(iesps20,2,false);
					iesps_detallados+="(20%)  ";
				}
				if ( iesps25>0) {
					iesps_detallados+=mFg.FormateaCantidad(iesps25,2,false);
					iesps_detallados+="(25%)  ";
				}
				if ( iesps30>0) {
					iesps_detallados+=mFg.FormateaCantidad(iesps30,2,false);
					iesps_detallados+="(30%)  ";
				}
				if ( iesps50>0) {
					iesps_detallados+=mFg.FormateaCantidad(iesps50,2,false);
					iesps_detallados+="(50%)  ";
				}
			}


			// Totales de la venta
			instruccion="select '";
			instruccion+=mensaje_dividido[0];
			instruccion+="' as mensajep1, '";
			instruccion+=mensaje_dividido[1];
			instruccion+="' as mensajep2, '";
			instruccion+=mensaje_dividido[2];
			instruccion+="' as mensajep3, '";
			instruccion+=total_enunciado;
			instruccion+="' as venttotalenunciado, '";
			instruccion+=subtotal_enunciado;
			instruccion+="' as ventsubtotenunciado, '";
			instruccion+=subtotaliva_enunciado;
			instruccion+="' as ventsubivaenunciado, '";
			instruccion+=iesps_detallados;
			instruccion+="' as ventiespsdetallados, ";
			instruccion+=mFg.FormateaCantidad(totalng,2,false);
			instruccion+=" as venttotalng, ";
			instruccion+=mFg.FormateaCantidad(totalg,2,false);
			instruccion+=" as venttotalg, ";
			instruccion+=" @importedesg:=";
			instruccion+=mFg.FormateaCantidad(subtotal,2,false);
			instruccion+=" as ventsubtotal, ";
			instruccion+=" @importeivadesg:=";
			instruccion+=mFg.FormateaCantidad(subtotaliva,2,false);
			instruccion+=" as ventsubtotaliva, ";
			instruccion+=mFg.FormateaCantidad(subtotalg,2,false);
			instruccion+=" as ventsubtotalg, ";
			instruccion+=mFg.FormateaCantidad(subtotalgiva,2,false);
			instruccion+=" as ventsubtotalgiva, ";
			instruccion+=" @importe:=";
			instruccion+=mFg.FormateaCantidad(total,2,false);
			instruccion+=" as venttotal, ";
			instruccion+=mFg.FormateaCantidad(iesps20,2,false);
			instruccion+=" as ventiesps20, ";
			instruccion+=mFg.FormateaCantidad(iesps25,2,false);
			instruccion+=" as ventiesps25, ";
			instruccion+=mFg.FormateaCantidad(iesps30,2,false);
			instruccion+=" as ventiesps30, ";
			instruccion+=mFg.FormateaCantidad(iesps50,2,false);
			instruccion+=" as ventiesps50, ";
			instruccion+=mFg.FormateaCantidad(totalcantidad,3,false);
			instruccion+=" as venttotalcant, ";
			instruccion+=mFg.FormateaCantidad(totalpeso,2,false);
			instruccion+=" as venttotalpeso, ";
			instruccion+=mFg.FormateaCantidad(totalvol,2,false);
			instruccion+=" as venttotalvol ";
			agregado_total_iva=false;
			agregado_total_iesps=false;
			for (i=6; i<=ArregloVenta.IndiceArregloTotales; i++) {
				instruccion+=", ";
				instruccion+=mFg.FormateaCantidad(ArregloVenta.ArregloContenedorTotales[1][i],2,false);
				instruccion+=" as ventimp";
				instruccion+=ArregloVenta.ArregloContenedorTotales[2][i].LowerCase();
				if (ArregloVenta.ArregloContenedorTotales[2][i].LowerCase()=="iva")
					agregado_total_iva=true;
				if (ArregloVenta.ArregloContenedorTotales[2][i].LowerCase()=="iesps")
					agregado_total_iesps=true;
			}
			// Si no se agregó un total de iva y de iesps, lo agrega con valor=0
			if (!agregado_total_iva) instruccion+=", 0.00 as ventimpiva ";
			if (!agregado_total_iesps) instruccion+=", 0.00 as ventimpiesps ";
			// Si la venta es de crédito, calcula el saldo del cliente
			if (ventacredito=="1") {
					instruccion+=", (select sum(t.valor) as saldo ";
					instruccion+="from transxcob t ";
					instruccion+="inner join ventas v on t.referencia=v.referencia ";
					instruccion+="inner join clientes c on c.cliente=v.cliente ";
					instruccion+="where t.cancelada=0 and v.cancelado=0 and ";
					if (conVentasCongeladas=="0") {
						instruccion+=" v.referencia NOT IN(select referencia from ventascongeladas where activo=1) AND ";
					}
					instruccion+="t.aplicada=1 and ";
					instruccion+="c.cliente='";
					instruccion+=cvecliente;
					instruccion+="' group by v.cliente ) as ventsaldo ";
			} else {
				instruccion+=", 'N/A' as ventsaldo ";
			}
			// Ejecuta la instrucción
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Obtiene todos los datos de la forma
			instruccion.sprintf("select * from formas where forma=@forma");
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Obtiene todo el detalle de la forma
			instruccion.sprintf("select * from dformas where forma=@forma");
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Si el Kit es resumido, entonces totaliza.
			if (!detdesglos) {
				// Obtiene los datos de la venta completa
				instruccion="select '' as referencia, '' as articulo, (@nombrekit) as nomcompleto, ";
				instruccion+="(@nombrekit) as nombre, (@cantkits) as cantidad, 0 as costobase, ";
				instruccion+=" 0 as porcdesc, '' as tipoprec, cast(@importedesg/@cantkits as decimal(16,4)) as precio, cast(@importe/@cantkits as decimal(16,4)) as precioimp, cast(0 as decimal(16,2)) as preciopub, ";
				instruccion+="'' as present, '' as producto, '' as multiplo, ";
				instruccion+="'' as almacen, 0 as factor, 0 as volumen, 0 as peso, ";
				instruccion+="'KIT' as multfactor, ";
				instruccion+="'' as claveimp1, '' as tipoimpuesto1, 0 as porcentajeimp1, '' as nomtipoimp1, ";
				instruccion+="'' as claveimp2, '' as tipoimpuesto2, 0 as porcentajeimp2, '' as nomtipoimp2, ";
				instruccion+="'' as claveimp3, '' as tipoimpuesto3, 0 as porcentajeimp3, '' as nomtipoimp3, ";
				instruccion+="'' as claveimp4, '' as tipoimpuesto4, 0 as porcentajeimp4, '' as nomtipoimp4, ";
				instruccion+="cast(@importe*1 as decimal(16,2)) as importe, ";
				instruccion+="cast(@importedesg*1 as decimal(16,2)) as importedesg, ";
				instruccion+="0 as porciva, ";
				instruccion+="0 as porciesps, ";
				instruccion+="cast(@importeivadesg*1 as decimal(16,2)) as importeivadesg, ";
				instruccion+="cast(@importeivadesg/@cantkits as decimal(16,4)) as precioivadesg ";
				mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
			}
            // Obtiene los kits en la venta
			instruccion.sprintf("SELECT vk.venta,vk.kit as kit,vk.desglosado,vk.cantidad,vk.modificado,kt.nombre \
								,ifnull(kt.ean13,'') AS kean13 \
								FROM ventaskits vk \
								INNER JOIN kits kt ON vk.kit=kt.kit \
								WHERE vk.venta='%s' and vk.desglosado=0 \
								order by vk.venta,vk.kit",clave);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

		}
	} __finally {
		if(resp_telefonos!=NULL) delete resp_telefonos;
		delete buffer_sql;
	}
}



//------------------------------------------------------------------------------
//ID_CON_VTA_IMPRESION_SURT
void ServidorVentas::ConsultaVentaParaImprimirSurt(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*64*10];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString ident_cola, terminal;
	int i;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[1000];
	char *resultado;

	try{
		ident_cola=mFg.ExtraeStringDeBuffer(&parametros); // Clave identificadora en la cola de impresión (si es vacia entonces es que es de impresión directa)
		terminal=mFg.ExtraeStringDeBuffer(&parametros); // Terminal desde la que se imprime.

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		// Marca como impresa el registro correspondiente de la cola.
		instruccion.sprintf("update colaimpsurtido set yaimpreso=1 where ident='%s'",ident_cola);
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("select @referencia:=referencia, @almacen:=almacen from colaimpsurtido where ident='%s'",ident_cola);
		instrucciones[num_instrucciones++]=instruccion;

		instrucciones[num_instrucciones++]="COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {

			// Obtiene todos los generales (de cabecera) de la venta
			instruccion.sprintf("select v.*, c.rsocial, (@almacen) as almacen, \
				ter.nombre as nomterminal, emp1.nombre as nomcajero, \
				v.comisionada as comisionada, emp2.nombre as nomvendedor, \
				@nombrekit:=ifnull(kt.nombre,'') as nombrekit, ifnull(kt.desglosado,1) as detdesglos \
				from ventas v  inner join  clientes c \
				left join terminales ter on v.terminal=ter.terminal \
				left join empleados emp1 on v.usumodi=emp1.empleado \
				left join empleados emp2 on v.vendedor=emp2.empleado \
				left join kits kt on v.kit=kt.kit \
				where v.referencia=@referencia and v.cliente=c.cliente");
			resultado=Respuesta->ObtieneDirLibreBuffer();
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
			BufferRespuestas resp_venta(resultado);

			// Obtiene los datos de la venta utiles para el surtido
			instruccion="select d.articulo, d.cantidad, d.costobase, ";
			instruccion+="a.present, p.producto, p.nombre as nombre, a.multiplo, ";
			instruccion+="concat(p.nombre,' ',a.present) as nomcompleto, ";
			instruccion+="d.almacen, a.factor, a.volumen, a.peso, ";
			instruccion+="concat(left(a.multiplo,3),'-',a.factor) as multfactor, ";
			instruccion+="d.almacen ";
			instruccion+="from dventas d, articulos a, productos p ";
			instruccion+="where d.referencia=@referencia and d.almacen=@almacen and d.articulo=a.articulo and a.producto=p.producto ";
			instruccion+=" group by p.nombre, a.present, a.multiplo ";
			instruccion+=" order by d.id, p.nombre, a.present, a.multiplo ";
			resultado=Respuesta->ObtieneDirLibreBuffer();
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
			BufferRespuestas resp_detalle(resultado);
		}
	} __finally {
		delete buffer_sql;
	}
}
//------------------------------------------------------------------------------
//ID_CON_NCRED_CLI_IMPRESION
void ServidorVentas::ConsultaNCredCliParaImprimir(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*64*10];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString tarea, clave, ident_cola, terminal;
	int num_partidas, i;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[1000];
	char *resultado;
	double totalng, totalg, subtotalg, subtotalgiva, total, subtotal, subtotaliva;
	ArregloDetalle ArregloVenta;
	AnsiString total_enunciado, subtotal_enunciado, subtotaliva_enunciado;
	BufferRespuestas* resp_telefonos=NULL, *resp_sum_notas=NULL;
	AnsiString telefonos, telefonos_principales, tipo, lada, telefono;
	int conta_telefonos_principales;
	double totalpeso, totalvol, peso, volumen;
	double totalcantidad, cantidad;
	bool agregado_total_iva, agregado_total_iesps;
	int tiponota;
	AnsiString cambiar_foliofisico;
	double suma_notas;

	double iesps20, iesps25, iesps30, iesps50;
	AnsiString iesps_detallados;

	try{
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave de la venta
		ident_cola=mFg.ExtraeStringDeBuffer(&parametros); // Clave identificadora en la cola de impresión (si es vacia entonces es que es de impresión directa)
		terminal=mFg.ExtraeStringDeBuffer(&parametros); // Terminal desde la que se imprime.
		cambiar_foliofisico=mFg.ExtraeStringDeBuffer(&parametros); // Indica si se debe cambiar el folio fìsico o no


		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		// Obtiene el folio físico
		if (cambiar_foliofisico=="1") {
			instruccion.sprintf("select @folioaux:=folncred, @serieaux:=seriencred, @anchofolncred:=anchofolncred from terminales where terminal='%s'", terminal);
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @foliosig=@folioaux+1");
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @folioaux=cast(@folioaux as char)");
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @folio=concat(@serieaux, lpad(@folioaux,@anchofolncred,'0') )");
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("update terminales t, notascredcli n set t.folncred=@foliosig, n.foliofisic=@folio where t.terminal='%s' and n.referencia='%s'", terminal,clave);
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("select @folio:=foliofisic from notascredcli where referencia='%s'",clave);
			instrucciones[num_instrucciones++]=instruccion;
		}

		// Marca como impresa la nota.
		instruccion.sprintf("update colaimpresion set yaimpreso=1 where ident='%s'",ident_cola);
		instrucciones[num_instrucciones++]=instruccion;

		instrucciones[num_instrucciones++]="COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {

			instruccion.sprintf("select t.tipo as tipo, t.lada as lada, t.telefono as telefono,t.extencionTel as ext \
				from telefonosclientes t, notascredcli n, ventas v where n.venta=v.referencia and v.cliente=t.cliente and n.referencia='%s'",clave);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_telefonos);
			telefonos="";
			telefonos_principales="";
			conta_telefonos_principales=0;
			for(i=0; i<resp_telefonos->ObtieneNumRegistros(); i++){
					resp_telefonos->IrAlRegistroNumero(i);
					tipo=resp_telefonos->ObtieneDato("tipo");
					lada=resp_telefonos->ObtieneDato("lada");
					telefono=resp_telefonos->ObtieneDato("telefono");
					if (tipo=="Negocio" || tipo=="Particular") {
						if (conta_telefonos_principales<2 ) {
							telefonos_principales+=lada;
							telefonos_principales+=" ";
							telefonos_principales+=telefono;
							telefonos_principales+="  ";

							telefonos+=tipo;
							telefonos+=" ";
							telefonos+=lada;
							telefonos+=" ";
							telefonos+=telefono;
							telefonos+="  ";
						}
						conta_telefonos_principales++;
					}
			}
			telefonos+=" ";
			telefonos_principales+=" ";

			// Obtiene todos los generales (de cabecera) de la nota
			instruccion.sprintf("select c.*, n.*, \
				v.vendedor, v.cobrador, @folioventa:=v.referencia as vta, v.foliofisic as foliofisicvta, v.tipofac, \
				cfd.seriefolio as foliocfdvta, cfd.desgloseimpuestos33, cfdn.agruparncre as agruparncrecfdi, \
				(case when n.tipo=0 then 'DEVOLUCION' when n.tipo=1 then 'BONIFICACION' \
				   when n.tipo=2 then 'DESCUENTO' end) as nomtiponot, \
				   if (n.tipo=2, concat('DESCUENTO ',n.descuento,'%%'), '') as rotulodesc1, \
				   if (n.tipo=2, concat('**** DESCUENTO GENERAL ',n.descuento,'%% ******'), '') as rotulodesc2, \
				@forma:=t.formancred as forma, t.tiporfc, \
				'%s' as telefonos, '%s' as telprincipales, \
				n.valor as valor, \
				col.nombre as nomcolonia, \
				concat(loc.nombre, '  ', est.estado) as ciudadestado, \
				CONCAT(TRIM(CONCAT(TRIM(IFNULL(c.calle,'')),' ',TRIM(IFNULL(c.numext,'')),' ',TRIM(IFNULL(c.numint,'')))), ' COL.: ', col.nombre) AS direccioncol, \
				loc.localidad as cvelocalidad, loc.nombre as nomlocalidad, \
				mun.municipio as cvemunicipio, mun.nombre as nommunicipio, \
				est.estado as cveestado, est.nombre as nomestado, sec.nombre AS nomsector,\
				emp1.nombre as nomvendedor, \
				emp2.nombre as nomcobrador, \
				emp3.nombre as nomusualta, \
				emp4.nombre as nomusumodi, \
                v.redondeoantiguo \
				from notascredcli n \
				inner join  ventas v ON v.referencia = n.venta \
				inner join  clientes c  inner join  tiposfacturasventas t \
				left join cfd on v.referencia=cfd.referencia and cfd.tipocomprobante='VENT' \
				left join cfd cfdn on n.referencia=cfdn.referencia and cfdn.tipocomprobante='NCRE' \
				left join colonias col on c.colonia=col.colonia \
                LEFT JOIN sectores sec ON col.sector=sec.sector \
				left join localidades loc on col.localidad=loc.localidad \
				left join municipios mun on mun.municipio=loc.municipio \
				left join estados est on est.estado=mun.estado \
				left join empleados emp1 on v.vendedor=emp1.empleado \
				left join empleados emp2 on v.cobrador=emp2.empleado \
				left join empleados emp3 on n.usualta=emp3.empleado \
				left join empleados emp4 on n.usumodi=emp4.empleado \
				where n.referencia='%s' and v.cliente=c.cliente \
				 and v.tipofac=t.tipo",
				telefonos, telefonos_principales, clave);
			resultado=Respuesta->ObtieneDirLibreBuffer();
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
			BufferRespuestas resp_venta(resultado);
			tiponota=StrToInt(resp_venta.ObtieneDato("tipo"));

			// Obtiene todo el detalle de la nota con algunos datos extras que necesita el cliente
			instruccion="select dn.referencia, dn.articulo, dn.cantidad, cast(dn.precio as decimal(16,6)) as precio, cast(dn.precioimp as decimal(16,6)) as precioimp, ";
			instruccion+="a.present, p.producto, p.nombre as nombre, a.multiplo, ";
			instruccion+="dv.almacen, a.factor, a.volumen, a.peso, ";
			instruccion+="concat(left(a.multiplo,3),'-',a.factor) as multfactor, ";
			instruccion+="dv.claveimp1, i1.tipoimpu as tipoimpuesto1, i1.porcentaje as porcentajeimp1, t1.nombre as nomtipoimp1, ";
			instruccion+="dv.claveimp2, i2.tipoimpu as tipoimpuesto2, i2.porcentaje as porcentajeimp2, t2.nombre as nomtipoimp2, ";
			instruccion+="dv.claveimp3, i3.tipoimpu as tipoimpuesto3, i3.porcentaje as porcentajeimp3, t3.nombre as nomtipoimp3, ";
			instruccion+="dv.claveimp4, i4.tipoimpu as tipoimpuesto4, i4.porcentaje as porcentajeimp4, t4.nombre as nomtipoimp4, ";
			instruccion+="cast(dn.precioimp*dn.cantidad as decimal(16,6)) as importe, ";
			instruccion+="cast(dn.precio*dn.cantidad as decimal(16,6)) as importedesg, ";
			instruccion+="@porciva:=(if(i1.tipoimpu='IVA',i1.porcentaje,0)+if(i2.tipoimpu='IVA',i2.porcentaje, 0)+if(i3.tipoimpu='IVA',i3.porcentaje, 0)+if(i4.tipoimpu='IVA',i4.porcentaje, 0)) as porciva, ";
			instruccion+="@porciesps:=(if(i1.tipoimpu='IESPS',i1.porcentaje,0)+if(i2.tipoimpu='IESPS',i2.porcentaje, 0)+if(i3.tipoimpu='IESPS',i3.porcentaje, 0)+if(i4.tipoimpu='IESPS',i4.porcentaje, 0)) as porciesps, ";
			instruccion+="cast((dn.precioimp*dn.cantidad)/(1+@porciva/100) as decimal(16,6)) as importeivadesg, ";
			instruccion+="cast((dn.precioimp)/(1+@porciva/100) as decimal(16,6)) as precioivadesg ";
			instruccion+="from notascredcli nc  inner join  dnotascredcli dn  inner join  dventas dv  inner join  articulos a  inner join  productos p ";
			instruccion+="left join impuestos i1 on i1.impuesto=dv.claveimp1 ";
			instruccion+="left join impuestos i2 on i2.impuesto=dv.claveimp2 ";
			instruccion+="left join impuestos i3 on i3.impuesto=dv.claveimp3 ";
			instruccion+="left join impuestos i4 on i4.impuesto=dv.claveimp4 ";
			instruccion+="left join tiposdeimpuestos t1 on t1.tipoimpu=i1.tipoimpu ";
			instruccion+="left join tiposdeimpuestos t2 on t2.tipoimpu=i2.tipoimpu ";
			instruccion+="left join tiposdeimpuestos t3 on t3.tipoimpu=i3.tipoimpu ";
			instruccion+="left join tiposdeimpuestos t4 on t4.tipoimpu=i4.tipoimpu ";
			instruccion+="where nc.referencia='";
			instruccion+=clave;
			instruccion+="' and nc.venta=dv.referencia and nc.referencia=dn.referencia ";
			instruccion+=" and dv.articulo=dn.articulo and dn.articulo=a.articulo ";
			instruccion+=" and a.producto=p.producto and a.producto=p.producto";

			resultado=Respuesta->ObtieneDirLibreBuffer();
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
			BufferRespuestas resp_detalle(resultado);

			// Sumatoria de todas las notas de crédito.
			instruccion.sprintf("select ifnull(sum(valor),0) as sumnotas from notascredcli where cancelado=0 and venta=@folioventa");
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_sum_notas);
			suma_notas=StrToFloat(resp_sum_notas->ObtieneDato("sumnotas"));

			// *********************************************************
			// Calcula total de articulos, total de peso y total de volumen.
			totalcantidad=0;
			totalpeso=0;
			totalvol=0;
			for(i=0; i<resp_detalle.ObtieneNumRegistros(); i++){
					resp_detalle.IrAlRegistroNumero(i);
					cantidad=mFg.CadenaAFlotante(resp_detalle.ObtieneDato("cantidad"));
					peso=StrToFloat(resp_detalle.ObtieneDato("peso"));
					volumen=StrToFloat(resp_detalle.ObtieneDato("volumen"));
					totalcantidad+=cantidad;
					totalpeso+=cantidad*peso;
					totalvol+=cantidad*volumen;
			}

			// *********************************************************
			//  Calcula totales fiscales.

			//0=Devolución
			//1=Bonificación de un solo artículo
			//2=Descuento a toda la factura
			ArregloVenta.PorcentajeDescuento=0;
			switch(tiponota) {
			case 0:
				ArregloVenta.UsoDelArreglo=NCRE_DEVO_VENTA;
				break;
			case 1:
				ArregloVenta.UsoDelArreglo=NCRE_DESP_VENTA;
				break;
			case 2:
				ArregloVenta.UsoDelArreglo=NCRE_DESG_VENTA;
			}
			ArregloVenta.PosicionArreglo=ABAJO;
			ArregloVenta.CargaBufferEnArregloDetalleVentas(&resp_detalle);
			ArregloVenta.SumatoriaNCredito=suma_notas;
			ArregloVenta.ValorOriginal=StrToFloat(resp_venta.ObtieneDato("valor"));
			ArregloVenta.AsignaModoAjusteRedondeo(StrToInt(resp_venta.ObtieneDato("redondeoantiguo")));
			ArregloVenta.RecalculaTotalesVenta();

			// Total no grabado
			totalng=mFg.CadenaAFlotante(ArregloVenta.ArregloContenedorTotales[1][4]);
			// Total grabado  -> SIN RFC
			totalg=mFg.CadenaAFlotante(ArregloVenta.ArregloContenedorTotales[1][5]);
			for (i=6; i<=ArregloVenta.IndiceArregloTotales; i++)
				totalg+=mFg.CadenaAFlotante(ArregloVenta.ArregloContenedorTotales[1][i]);
			// Subtotal grabado (con todos los impuestos desglosados) -> CON RFC (IESPS)
			subtotalg=mFg.CadenaAFlotante(ArregloVenta.ArregloContenedorTotales[1][5]);
			// SubTotal grabado (con iva desglosado) -> CON RFC
			subtotalgiva=0;
			for (i=5; i<=ArregloVenta.IndiceArregloTotales; i++) {
				if (ArregloVenta.ArregloContenedorTotales[2][i]!="IVA")
					subtotalgiva+=mFg.CadenaAFlotante(ArregloVenta.ArregloContenedorTotales[1][i]);
			}
			// Total
			total=0;
			for (i=4; i<=ArregloVenta.IndiceArregloTotales; i++)
				total+=mFg.CadenaAFlotante(ArregloVenta.ArregloContenedorTotales[1][i]);
			// Total enunciado
			total_enunciado=mFg.GeneraEnunciadoDeCantidad(total);
			// Subtotal (el subtotal de lo grabado + lo no grabado = total-impuestos)
			subtotal=subtotalg+totalng;
			// Subtotaliva (El subtotal sin iva de lo grabado + lo no grabado = total-iva)
			subtotaliva=subtotalgiva+totalng;
			// SubTotal enunciado
			subtotal_enunciado=mFg.GeneraEnunciadoDeCantidad(subtotal);
			// SubTotalIva enunciado
			subtotaliva_enunciado=mFg.GeneraEnunciadoDeCantidad(subtotaliva);
			// Los diferentes IESPS
			iesps20=ArregloVenta.ObtieneTotalImpuesto("IESPS", 20);
			iesps25=ArregloVenta.ObtieneTotalImpuesto("IESPS", 25);
			iesps30=ArregloVenta.ObtieneTotalImpuesto("IESPS", 30);
			iesps50=ArregloVenta.ObtieneTotalImpuesto("IESPS", 50);
			if ( (iesps20+iesps25+iesps30+iesps50) > 0) {
				if ( iesps20>0) {
					iesps_detallados+=mFg.FormateaCantidad(iesps20,2,false);
					iesps_detallados+="(20%)  ";
				}
				if ( iesps25>0) {
					iesps_detallados+=mFg.FormateaCantidad(iesps25,2,false);
					iesps_detallados+="(25%)  ";
				}
				if ( iesps30>0) {
					iesps_detallados+=mFg.FormateaCantidad(iesps30,2,false);
					iesps_detallados+="(30%)  ";
				}
				if ( iesps50>0) {
					iesps_detallados+=mFg.FormateaCantidad(iesps50,2,false);
					iesps_detallados+="(50%)  ";
				}
			}


			// Totales de la nota
			instruccion="select '";
			instruccion+=total_enunciado;
			instruccion+="' as nottotalenunciado, '";
			instruccion+=subtotal_enunciado;
			instruccion+="' as notsubtotenunciado, '";
			instruccion+=iesps_detallados;
			instruccion+="' as notiespsdetallados, '";
			instruccion+=subtotaliva_enunciado;
			instruccion+="' as notsubivaenunciado, ";
			instruccion+=mFg.FormateaCantidad(totalng,2,false);
			instruccion+=" as nottotalng, ";
			instruccion+=mFg.FormateaCantidad(totalg,2,false);
			instruccion+=" as nottotalg, ";
			instruccion+=mFg.FormateaCantidad(subtotal,2,false);
			instruccion+=" as notsubtotal, ";
			instruccion+=mFg.FormateaCantidad(subtotaliva,2,false);
			instruccion+=" as notsubtotaliva, ";
			instruccion+=mFg.FormateaCantidad(subtotalg,2,false);
			instruccion+=" as notsubtotalg, ";
			instruccion+=mFg.FormateaCantidad(subtotalgiva,2,false);
			instruccion+=" as notsubtotalgiva, ";
			instruccion+=mFg.FormateaCantidad(total,2,false);
			instruccion+=" as nottotal, ";
			instruccion+=mFg.FormateaCantidad(iesps20,2,false);
			instruccion+=" as notiesps20, ";
			instruccion+=mFg.FormateaCantidad(iesps25,2,false);
			instruccion+=" as notiesps25, ";
			instruccion+=mFg.FormateaCantidad(iesps30,2,false);
			instruccion+=" as notiesps30, ";
			instruccion+=mFg.FormateaCantidad(iesps50,2,false);
			instruccion+=" as notiesps50, ";
			instruccion+=mFg.FormateaCantidad(totalcantidad,3,false);
			instruccion+=" as nottotalcant, ";
			instruccion+=mFg.FormateaCantidad(totalpeso,2,false);
			instruccion+=" as nottotalpeso, ";
			instruccion+=mFg.FormateaCantidad(totalvol,2,false);
			instruccion+=" as nottotalvol ";
			agregado_total_iva=false;
			agregado_total_iesps=false;
			for (i=6; i<=ArregloVenta.IndiceArregloTotales; i++) {
				instruccion+=", ";
				instruccion+=mFg.FormateaCantidad(ArregloVenta.ArregloContenedorTotales[1][i],2,false);
				instruccion+=" as notimp";
				instruccion+=ArregloVenta.ArregloContenedorTotales[2][i].LowerCase();
				if (ArregloVenta.ArregloContenedorTotales[2][i].LowerCase()=="iva")
					agregado_total_iva=true;
				if (ArregloVenta.ArregloContenedorTotales[2][i].LowerCase()=="iesps")
					agregado_total_iesps=true;
			}
			// Si no se agregó un total de iva y de iesps, lo agrega con valor=0
			if (!agregado_total_iva) instruccion+=", 0.00 as notimpiva ";
			if (!agregado_total_iesps) instruccion+=", 0.00 as notimpiesps ";
			// Ejecuta la instrucción
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Obtiene todos los datos de la forma
			instruccion.sprintf("select * from formas where forma=@forma");
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Obtiene todo el detalle de la forma
			instruccion.sprintf("select * from dformas where forma=@forma");
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	} __finally {
		if(resp_telefonos!=NULL) delete resp_telefonos;
		if(resp_sum_notas!=NULL) delete resp_sum_notas;
		delete buffer_sql;
	}
}

//-----------------------------------------------------------------------------
//ID_CON_NCAR_CLI_IMPRESION
void ServidorVentas::ConsultaNCarCliParaImprimir(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*64*10];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString tarea, clave, ident_cola, terminal;
	int num_partidas, i;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[1000];
	char *resultado;
	AnsiString total_enunciado, subtotal_enunciado;
	BufferRespuestas* resp_telefonos=NULL;
	AnsiString telefonos, telefonos_principales, tipo, lada, telefono;
	int conta_telefonos_principales;
	double total, subtotal, impuestos;
	AnsiString cambiar_foliofisico;

	try{
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave de la nota
		ident_cola=mFg.ExtraeStringDeBuffer(&parametros); // Clave identificadora en la cola de impresión (si es vacia entonces es que es de impresión directa)
		terminal=mFg.ExtraeStringDeBuffer(&parametros); // Terminal desde la que se imprime.
		cambiar_foliofisico=mFg.ExtraeStringDeBuffer(&parametros); // Indica si se debe cambiar el folio fìsico o no

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		if (cambiar_foliofisico=="1") {
			// Obtiene el folio físico
			instruccion.sprintf("select @folioaux:=folncar, @serieaux:=seriencar, @anchofolncar:=anchofolncar from terminales where terminal='%s'", terminal);
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @foliosig=@folioaux+1");
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @folioaux=cast(@folioaux as char)");
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @folio=concat(@serieaux, lpad(@folioaux,@anchofolncar,'0') )");
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("update terminales t, notascarcli n set t.folncar=@foliosig, n.foliofisic=@folio where t.terminal='%s' and n.referencia='%s'", terminal, clave);
			instrucciones[num_instrucciones++]=instruccion;
		}
		instruccion.sprintf("select @folio:=foliofisic from notascarcli where referencia='%s'",clave);
		instrucciones[num_instrucciones++]=instruccion;

		// Marca como impresa la nota.
		instruccion.sprintf("update colaimpresion set yaimpreso=1 where ident='%s'",ident_cola);
		instrucciones[num_instrucciones++]=instruccion;

		instrucciones[num_instrucciones++]="COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {

			instruccion.sprintf("select t.tipo as tipo, t.lada as lada, t.telefono as telefono, t.extencionTel as ext \
				from telefonosclientes t, notascarcli n where n.cliente=t.cliente and n.referencia='%s'",clave);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_telefonos);
			telefonos="";
			telefonos_principales="";
			conta_telefonos_principales=0;
			for(i=0; i<resp_telefonos->ObtieneNumRegistros(); i++){
					resp_telefonos->IrAlRegistroNumero(i);
					tipo=resp_telefonos->ObtieneDato("tipo");
					lada=resp_telefonos->ObtieneDato("lada");
					telefono=resp_telefonos->ObtieneDato("telefono");
					if (tipo=="Negocio" || tipo=="Particular") {
						if (conta_telefonos_principales<2 ) {
							telefonos_principales+=lada;
							telefonos_principales+=" ";
							telefonos_principales+=telefono;
							telefonos_principales+="  ";

							telefonos+=tipo;
							telefonos+=" ";
							telefonos+=lada;
							telefonos+=" ";
							telefonos+=telefono;
							telefonos+="  ";
						}
						conta_telefonos_principales++;
					}
			}
			telefonos+=" ";
			telefonos_principales+=" ";

			// Obtiene todos los generales (de cabecera) de la nota
			instruccion.sprintf("select c.*, n.*, @impuesto:=n.cveimp, v.tipofac, \
				v.vendedor, v.cobrador, v.referencia as vta, v.foliofisic as foliofisicvta, \
				cfd.seriefolio as foliocfdvta, cfd.desgloseimpuestos33, \
				@forma:=t.formancar as forma, t.tiporfc, \
				@transac:=tc.tracredito as tracredito, \
				'%s' as telefonos, '%s' as telprincipales, \
				col.nombre as nomcolonia, \
				concat(loc.nombre, '  ', est.estado) as ciudadestado, \
				CONCAT(TRIM(CONCAT(TRIM(IFNULL(c.calle,'')),' ',TRIM(IFNULL(c.numext,'')),' ',TRIM(IFNULL(c.numint,'')))), ' COL.: ', col.nombre) AS direccioncol, \
				loc.localidad as cvelocalidad, loc.nombre as nomlocalidad, \
				mun.municipio as cvemunicipio, mun.nombre as nommunicipio, \
				est.estado as cveestado, est.nombre as nomestado, sec.nombre AS nomsector, \
				emp1.nombre as nomvendedor, \
				emp2.nombre as nomcobrador, \
				emp3.nombre as nomusualta, \
				emp4.nombre as nomusumodi \
				from notascarcli n  inner join  transxcob tc  inner join  ventas v  inner join  clientes c  inner join  tiposfacturasventas t \
				left join cfd on v.referencia=cfd.referencia and cfd.tipocomprobante='VENT' \
				left join colonias col on c.colonia=col.colonia \
				LEFT JOIN sectores sec ON col.sector=sec.sector \
				left join localidades loc on col.localidad=loc.localidad \
				left join municipios mun on mun.municipio=loc.municipio \
				left join estados est on est.estado=mun.estado \
				left join empleados emp1 on v.vendedor=emp1.empleado \
				left join empleados emp2 on v.cobrador=emp2.empleado \
				left join empleados emp3 on n.usualta=emp3.empleado \
				left join empleados emp4 on n.usumodi=emp4.empleado \
				where n.referencia='%s' and n.cliente=c.cliente and \
				tc.notacar=n.referencia and tc.cancelada=n.cancelado and tc.tipo<>'CHDE' and \
				tc.referencia=v.referencia and v.tipofac=t.tipo",
				telefonos, telefonos_principales, clave);
			resultado=Respuesta->ObtieneDirLibreBuffer();
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
			BufferRespuestas resp_generales(resultado);


			// Obtiene el detalle, en el caso de los cargos el detalle solo consiste
			// de la transacción relacionada con el cargo.
			instruccion.sprintf("select t.tipo as cvetipotran, t.cancelada, t.valor as valortran,  \
					ttc.descripcion as nomtipotran, chcli.folio as numcheque, \
					i.tipoimpu as tipoimpuesto, \
					i.porcentaje as porcentajeimp, ti.nombre as nomtipoimp, \
					((t.valor)/(1+i.porcentaje/100)) as valortrandesg, \
					(t.valor-((t.valor)/(1+i.porcentaje/100))) as valortranimp \
				from transxcob t  inner join  impuestos i  inner join  tiposdeimpuestos ti  inner join  tiposdetranxcob ttc \
				left join notascarcli nccli on nccli.referencia=t.notacar \
				left join pagoscli p on nccli.pago=p.pago \
				left join cheqxcob chpc on p.pago=chpc.pago \
				left join chequesclientes chcli on chpc.chequecli=chcli.chequecli \
				where t.tracredito=@transac and \
					i.impuesto=@impuesto and \
					ti.tipoimpu=i.tipoimpu and \
					ttc.tipo=t.tipo");
			resultado=Respuesta->ObtieneDirLibreBuffer();
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
			BufferRespuestas resp_detalle(resultado);


			// CALCULA TOTALES
			total=StrToFloat(resp_detalle.ObtieneDato("valortran"));
			subtotal=StrToFloat(resp_detalle.ObtieneDato("valortrandesg"));
			impuestos=StrToFloat(resp_detalle.ObtieneDato("valortranimp"));
			total_enunciado=mFg.GeneraEnunciadoDeCantidad(total);
			subtotal_enunciado=mFg.GeneraEnunciadoDeCantidad(subtotal);

			// Totales del cargo
			instruccion="select '";
			instruccion+=total_enunciado;
			instruccion+="' as nottotalenunciado, '";
			instruccion+=subtotal_enunciado;
			instruccion+="' as notsubtotenunciado, ";
			instruccion+=mFg.FormateaCantidad(total,2,false);
			instruccion+=" as nottotal, ";
			instruccion+=mFg.FormateaCantidad(subtotal,2,false);
			instruccion+=" as notsubtotal, ";
			instruccion+=mFg.FormateaCantidad(impuestos,2,false);
			instruccion+=" as notimpuestos ";
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Obtiene todos los datos de la forma
			instruccion.sprintf("select * from formas where forma=@forma");
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Obtiene todo el detalle de la forma
			instruccion.sprintf("select * from dformas where forma=@forma");
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	} __finally {
		if(resp_telefonos!=NULL) delete resp_telefonos;
		delete buffer_sql;
	}
}

//------------------------------------------------------------------------------

void ServidorVentas::ConsultaColaImpresion(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
//ID_CON_COLA_IMPRESION
{
	// CONSULTA LAS VENTAS PENDIENTES DE IMPRIR (que están en la cola de impresión)
	AnsiString instruccion;
	AnsiString terminal;

	terminal=mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Todos los elementos en la cola.
	instruccion.sprintf("select c.ident,c.foliodoc, \
			if(c.tipo=0, v.foliofisic, if(c.tipo=1, ncred.foliofisic, ncar.foliofisic)) as foliofisic, \
			c.tipo, c.termorigen \
		from colaimpresion c \
		left join ventas v on v.referencia=c.foliodoc \
		left join notascredcli ncred on ncred.referencia=foliodoc \
		left join notascarcli ncar on ncar.referencia=foliodoc \
		where c.yaimpreso=0 and c.terminal='%s' order by c.ident", terminal);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Folio físico que se asigna  a la siguiente venta que se imprima en caso
	// de que la terminal esté configurada para asignar el folio automáticamente
	// al momento de imprimir.
	instruccion.sprintf("select serievta, folvta, anchofolvta, seriencred, folncred, anchofolncred, seriencar, folncar, anchofolncar from terminales where terminal='%s'", terminal);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}

//------------------------------------------------------------------------------

void ServidorVentas::ConsultaColaImpresionSurt(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
//ID_CON_COLA_IMPRESION_SURT
{
	// CONSULTA LAS VENTAS PENDIENTES DE IMPRIMIR SU SURTIDO (que están en la cola de impresión)
	AnsiString instruccion;
	AnsiString terminal;

	terminal=mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Todos los elementos en la cola de impresión de surtido
	instruccion.sprintf("select c.ident, c.referencia, c.almacen, c.termorigen \
		from colaimpsurtido c \
		where c.yaimpreso=0 and c.terminal='%s' order by c.ident", terminal);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

}

//------------------------------------------------------------------------------

void ServidorVentas::ModificaColaImpresion(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
//ID_MOD_COLA_IMPRESION
{
	// Modifica la cola de impresión, actualmente se hacen las siguientes modificaciones:

	// a) Marcar como NO impreso X número de documentos que se acaban de imprimir.- Esto es útil
	//    para cuando se tienen que reimprimir facturas que se dañaron físicamente o que
	//    no había tinta en la impresora, y se imprimieron mal, o no habia papel, o no habia
	//    papel y además se tiene que sustituir el papel por otra serie.
	// b) Marcar como YA impresos X número de documentos que son los siguientes a imprimir.-
	//    esto es útil para revertir la opción a)
	// c) Reasigna opcionalmente el campo de folio de la terminal y el campo de serie.

	char *buffer_sql=new char[1024*10];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instrucciones[100];
	AnsiString instruccion;
	int num_instrucciones=0;
	//AnsiString terminal, tarea, num, reasignar, folio, foliodoc, serie, tipo, termorigen;
	AnsiString terminal, tarea, foliodoc, tipo, termorigen;
	int i;

	try{
		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		terminal=mFg.ExtraeStringDeBuffer(&parametros);
		tarea=mFg.ExtraeStringDeBuffer(&parametros);

		if (tarea=="INSERTA") {
			foliodoc=mFg.ExtraeStringDeBuffer(&parametros);
			tipo=mFg.ExtraeStringDeBuffer(&parametros);
			termorigen=mFg.ExtraeStringDeBuffer(&parametros);

			// Asigna el folio
			instruccion.sprintf("select @folioaux:=valor from foliosemp where folio='COLAIMP' AND sucursal = '%s' %s ",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @foliosig=@folioaux+1");
			instrucciones[num_instrucciones++]=instruccion;
	//        instruccion.sprintf("set @folioaux=cast(@folioaux as char)");
	//        instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @folio=@folioaux");
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("update foliosemp set valor=@foliosig where folio='COLAIMP' AND sucursal = '%s' ",FormServidor->ObtieneClaveSucursal());
			instrucciones[num_instrucciones++]=instruccion;

			// Crea el registro en COLAIMPRESION
			instruccion.sprintf("insert into colaimpresion \
				(ident, terminal, foliodoc, tipo, termorigen, yaimpreso) values \
				(@folio, '%s', '%s', %s, '%s', 0)",
				terminal, foliodoc, tipo, termorigen);
			instrucciones[num_instrucciones++]=instruccion;
		}
		if (tarea=="NOIMP") {
			instruccion.sprintf("select @idcola:=ident from colaimpresion \
				where terminal='%s' and yaimpreso=1 \
				order by ident desc limit 1 ",
				terminal);
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("update colaimpresion set yaimpreso=0 \
				where ident=@idcola ",
				terminal);
			instrucciones[num_instrucciones++]=instruccion;
		}
		if (tarea=="YAIMP") {
			instruccion.sprintf("select @idcola:=ident from colaimpresion \
				where terminal='%s' and yaimpreso=0 \
				order by ident limit 1 ",
				terminal);
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("update colaimpresion set yaimpreso=1 \
				where ident=@idcola ",
				terminal);
			instrucciones[num_instrucciones++]=instruccion;
		}

	/*    reasignar=mFg.ExtraeStringDeBuffer(&parametros);
		folio=mFg.ExtraeStringDeBuffer(&parametros);
		serie=mFg.ExtraeStringDeBuffer(&parametros);
		// Reasigna opcionalmente el campo de folio de la terminal y el campo de serie.
		if (reasignar=="1") {
			instruccion.sprintf("update terminales t set t.folvta=%s, t.serievta='%s' \
				where t.terminal='%s' ",
				folio, serie, terminal);
			instrucciones[num_instrucciones++]=instruccion;
		}*/

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

void ServidorVentas::ModificaColaImpresionSurt(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
//ID_MOD_COLA_IMPRESION_SURT
{
    // Modifica la cola de impresión de surtido, actualmente se hacen las siguientes modificaciones:

    // a) Marcar como NO impreso X número de documentos que se acaban de imprimir.- Esto es útil
    //    para cuando se tienen que reimprimir facturas que se dañaron físicamente o que
    //    no había tinta en la impresora, y se imprimieron mal, o no habia papel, o no habia
    //    papel y además se tiene que sustituir el papel por otra serie.
	// b) Marcar como YA impresos X número de documentos que son los siguientes a imprimir.-
	//    esto es útil para revertir la opción a)
    // c) Reasigna opcionalmente el campo de folio de la terminal y el campo de serie.

    char *buffer_sql=new char[1024*10];
    char *aux_buffer_sql=buffer_sql;
    AnsiString instrucciones[100];
    AnsiString instruccion;
    int num_instrucciones=0;
	AnsiString terminal, tarea, foliodoc, almacen, termorigen;
    int i;
    BufferRespuestas* resp_almacenes_impr=NULL;

	try{
		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		tarea=mFg.ExtraeStringDeBuffer(&parametros);

		if (tarea=="INSERTA") {
			foliodoc=mFg.ExtraeStringDeBuffer(&parametros);
			termorigen=mFg.ExtraeStringDeBuffer(&parametros);

			instruccion.sprintf("SELECT distinct a.almacen, a.termimpsur \
				FROM ventas v, dventas dv, almacenes a \
				where v.referencia='%s' and v.referencia=dv.referencia and \
				dv.almacen=a.almacen", foliodoc);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_almacenes_impr);

			for(i=0; i<resp_almacenes_impr->ObtieneNumRegistros(); i++){
				resp_almacenes_impr->IrAlRegistroNumero(i);
				almacen=resp_almacenes_impr->ObtieneDato("almacen");
				terminal=resp_almacenes_impr->ObtieneDato("termimpsur");

				// Asigna el folio
				instruccion.sprintf("select @folioaux:=valor from foliosemp where folio='COLAIMPSUR' AND sucursal = '%s' %s ",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliosig=@folioaux+1");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @folio=@folioaux");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("update foliosemp set valor=@foliosig where folio='COLAIMPSUR' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;

				// Crea el registro en COLAIMPSURTIDO
				instruccion.sprintf("insert into colaimpsurtido \
					(ident, terminal, referencia, almacen, termorigen, yaimpreso) values \
					(@folio, '%s', '%s', '%s', '%s', 0)",
					terminal, foliodoc, almacen, termorigen);
				instrucciones[num_instrucciones++]=instruccion;
			}

		}

		if (tarea=="NOIMP") {
			terminal=mFg.ExtraeStringDeBuffer(&parametros);
			instruccion.sprintf("select @idcola:=ident from colaimpsurtido \
				where terminal='%s' and yaimpreso=1 \
				order by ident desc limit 1 ",
				terminal);
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("update colaimpsurtido set yaimpreso=0 \
				where ident=@idcola ",
				terminal);
			instrucciones[num_instrucciones++]=instruccion;
		}
		if (tarea=="YAIMP") {
			terminal=mFg.ExtraeStringDeBuffer(&parametros);
			instruccion.sprintf("select @idcola:=ident from colaimpsurtido \
				where terminal='%s' and yaimpreso=0 \
				order by ident limit 1 ",
				terminal);
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("update colaimpsurtido set yaimpreso=1 \
				where ident=@idcola ",
				terminal);
			instrucciones[num_instrucciones++]=instruccion;
		}

		instrucciones[num_instrucciones++]="COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);
	} __finally {
		if(resp_almacenes_impr!=NULL) delete resp_almacenes_impr;
		delete buffer_sql;
	}
}



//------------------------------------------------------------------------------
//ID_CON_CHEQXFECH_CLI_NC
void ServidorVentas::ConsultaChequesxfechaCliNC(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA CHEQUES DEL CLIENTE DE UN DIA X
	AnsiString instruccion;
	AnsiString fecha_inicio, fecha_fin;
    AnsiString cliente;

    fecha_inicio=mFg.ExtraeStringDeBuffer(&parametros);
    fecha_fin=mFg.ExtraeStringDeBuffer(&parametros);
    cliente=mFg.ExtraeStringDeBuffer(&parametros);

    mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	instruccion.sprintf("select chc.chequecli, \
	if(ifnull(v.foliofisic,'')='',concat(ifnull(cfd.serie,''),ifnull(cfd.folio,'')),v.foliofisic) as ventfolcli, \
	(t.valor*-1) as valor, chc.folio as numcheque, b.banco as cvebanco, b.nombre as nombanco, \
	chc.fechacob, chc.estado as status \
	from ventas v inner join transxcob t inner join pagoscli p inner join tiposfacturasventas tv inner join \
		cheqxcob chxc inner join chequesclientes chc inner join \
		bancos b \
		left join cfd on v.referencia=cfd.referencia and cfd.tipocomprobante='VENT' \
      where \
      v.tipofac=tv.tipo and \
      tv.permitirncargo=1 and \
      v.cliente='%s' and \
      chc.fechacob>='%s' and chc.fechacob<='%s' \
      and chxc.pago=p.pago and chc.chequecli=chxc.chequecli and \
      p.pago=t.pago and v.cancelado=0 and  \
      b.banco=chc.banco and \
      v.referencia=t.referencia and t.cancelada=0 and p.cancelado=0 and t.aplicada=1",
	  cliente,mFg.StrToMySqlDate(fecha_inicio),mFg.StrToMySqlDate(fecha_fin));
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}

//------------------------------------------------------------------------------
//ID_QRY_COMISIONES_VEND
void ServidorVentas::ConsultaComisionesVendedor(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA COMISIONES VENDEDOR
	AnsiString instruccion;
    AnsiString fecha_inicio, fecha_fin;
    AnsiString vendedor, condicion_vendedor=" ";

    mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

    fecha_inicio=mFg.ExtraeStringDeBuffer(&parametros);
    fecha_fin=mFg.ExtraeStringDeBuffer(&parametros);
    vendedor=mFg.ExtraeStringDeBuffer(&parametros);

    if(vendedor!=""){
        condicion_vendedor.sprintf(" v.vendedor='%s' and  ",vendedor);
	}

	instruccion.sprintf("SET SESSION sql_log_bin = 0 ");
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query EjecutaSelectSqlNulo"));

	instruccion.sprintf("SET SESSION tx_isolation='READ-COMMITTED' ");
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query EjecutaSelectSqlNulo"));

	instruccion.sprintf("CREATE temporary TABLE `auxcomisiones1` ( \
	  `tracredito` varchar(11) NOT NULL DEFAULT '', \
	  `tranpagoorigen` varchar(11) DEFAULT '', \
	  `cliente` varchar(11) NOT NULL DEFAULT '', \
	  `rsocial` varchar(255) DEFAULT NULL, \
	  `ident` varchar(20) DEFAULT '', \
	  `tipo` varchar(4) DEFAULT NULL, \
	  `valor` decimal(17,2) DEFAULT NULL, \
	  `formapag` varchar(1) DEFAULT NULL, \
	  `clasif` varchar(4) DEFAULT '', \
	  `fechaapl` date NOT NULL DEFAULT '0000-00-00', \
	  `fecharecup` date NOT NULL, \
	  `valorventa` decimal(16,2) DEFAULT NULL, \
	  `totcomiventa` decimal(16,4) NOT NULL DEFAULT 0.0000, \
	  `referencia` varchar(11) NOT NULL DEFAULT '0', \
	  `tolercomision` int(3) NOT NULL DEFAULT 0, \
	  `foliofisic` varchar(30) DEFAULT NULL, \
	  `fechalimite` date DEFAULT NULL, \
	  `diasdeventa` int(7) DEFAULT NULL, \
	  `aplicacomi` int(1) DEFAULT NULL, \
	  `porccomi` decimal(16,6) DEFAULT NULL, \
	  `valorcomi` decimal(16,6) DEFAULT NULL, \
	   INDEX referencia (referencia) \
		) ENGINE=InnoDB DEFAULT CHARSET=latin1;	");
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query EjecutaSelectSqlNulo"));

	// Comisiones de ventas a crédito pagadas
	instruccion.sprintf("insert into auxcomisiones1 \
		select t.tracredito, tnc.tracredito as tranpagoorigen, \
		c.cliente, c.rsocial, p.ident, \
		t.tipo, (t.valor*-1) as valor, p.formapag, ch.clasif, t.fechaapl, \
		ifnull(ch.fechacob, t.fechaapl) as fecharecup, \
		v.valor as valorventa, v.totcomision as totcomiventa, v.referencia, v.tolercomision, \
		if(ifnull(v.foliofisic,'')='',concat(ifnull(cfd.serie,''),ifnull(cfd.folio,'')),v.foliofisic) as foliofisic, \
		date(date_add(v.fechavta,interval v.tolercomision day)) as fechalimite, \
		(to_days(ifnull(ch.fechacob,t.fechaapl))-to_days(v.fechavta)) as diasdeventa, \
		@aplicacomi:=if( (t.referencia=tnc.referencia and \
		(to_days(ifnull(ch.fechacob,t.fechaapl))-to_days(v.fechavta))<v.tolercomision) OR ((to_days(ifnull(ch.fechacob,t.fechaapl))-to_days(v.fechavta))<v.tolercomision), 1,0) as aplicacomi, \
		@porccomi:=((v.totcomision*100)/v.valor) as porccomi, \
		(t.valor*(@porccomi/100)*@aplicacomi*-1) as valorcomi \
		from transxcob t force index(fechaapl)  inner join  clientes c  inner join  ventas v \
		INNER JOIN terminales ter ON ter.terminal = v.terminal \
		INNER JOIN secciones sec ON sec.seccion = ter.seccion \
		INNER JOIN sucursales suc ON suc.sucursal = sec.sucursal \
		left join pagoscli p on p.pago=t.pago \
		left join cheqxcob chxc on chxc.pago=p.pago \
		left join chequesclientes ch on ch.chequecli=chxc.chequecli \
		left join notascarcli nc on nc.referencia=t.notacar and t.tipo='CHDE' and nc.cancelado=0 \
		left join pagoscli pnc on pnc.pago=nc.pago \
		left join transxcob tnc on tnc.pago=pnc.pago and tnc.referencia=t.referencia and tnc.cancelada=0 and tnc.tipo='PAGO' \
		left join cfd on v.referencia=cfd.referencia and cfd.tipocomprobante='VENT' \
		where %s \
			(t.tipo='PAGO' or t.tipo='CHDE') and \
			t.referencia=v.referencia and \
			v.cliente=c.cliente and \
			t.fechaapl>='%s' and t.fechaapl<='%s' and \
			t.cancelada=0 and t.aplicada=1 AND suc.idempresa = %s ",
			condicion_vendedor,mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin),
			FormServidor->ObtieneClaveEmpresa());
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query EjecutaSelectSqlNulo"));

	// Comisiones de ventas de contado
	instruccion.sprintf("insert into auxcomisiones1 "
		"SELECT '' as tracredito, '' AS tranpagoorigen, 	c.cliente, c.rsocial, '' as ident, "
		"'CONT' as tipo, v.valor, 'E' AS formapag, "
		"'' as clasif, v.fechavta AS fechaapl, v.fechavta AS fecharecup, "
		"v.valor AS valorventa, v.totcomision AS totcomiventa, v.referencia, v.tolercomision, "
		"if(IFNULL(v.foliofisic,'')='', CONCAT(IFNULL(cfd.serie,''), IFNULL(cfd.folio,'')),v.foliofisic) AS foliofisic, "
		"DATE(DATE_ADD(v.fechavta, INTERVAL v.tolercomision DAY)) AS fechalimite, "
		"0 AS diasdeventa, "
		"1 AS aplicacomi, "
		"@porccomi:=((v.totcomision*100)/v.valor) AS porccomi, "
		"(v.valor*(@porccomi/100)) AS valorcomi "
		"FROM ventas v "
		"INNER JOIN clientes c ON v.cliente=c.cliente "
		"INNER JOIN terminales ter ON ter.terminal = v.terminal "
		"INNER JOIN secciones sec ON sec.seccion = ter.seccion "
		"INNER JOIN sucursales suc ON suc.sucursal = sec.sucursal "
		"LEFT JOIN cfd ON v.referencia=cfd.referencia AND cfd.tipocomprobante='VENT' "
		"WHERE %s  v.acredito=0 AND	v.cancelado=0	AND v.fechavta BETWEEN '%s' AND '%s' AND suc.idempresa = %s ",
		condicion_vendedor,mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin),
		FormServidor->ObtieneClaveEmpresa());
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query EjecutaSelectSqlNulo"));

	instruccion.sprintf("select a.tracredito, a.tranpagoorigen, a.cliente, a.rsocial, a.ident, a.tipo, a.valor, a.formapag, a.clasif, a.fechaapl, a.fecharecup, a.valorventa, a.totcomiventa, \
		a.referencia, a.tolercomision, a.foliofisic, a.fechalimite, a.diasdeventa, a.aplicacomi, a.porccomi, a.valorcomi, \
		ifnull(pc.costovta,0)*-1 as costovta, ifnull(pv.subtotal,0) as subtotalvta, (ifnull(pv.subtotal,0)-ifnull(pc.costovta,0)*-1) as totmargen, \
		if(ifnull(pv.subtotal,0)=0, 0,(ifnull(pv.subtotal,0)-ifnull(pc.costovta,0)*-1)/ifnull(pv.subtotal,0))*100 as porcmargen \
		from auxcomisiones1 a \
		left join precalculocostosventa pc on pc.referencia=a.referencia \
		left join precalculoventas pv on pv.referencia=a.referencia \
		order by a.fecharecup, a.tracredito, a.referencia");
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	instruccion.sprintf("SET SESSION tx_isolation='REPEATABLE-READ' ");
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query EjecutaSelectSqlNulo"));

	instruccion.sprintf("SET SESSION sql_log_bin=1");
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL,
		instruccion.c_str()))
		throw(Exception("Error en query EjecutaSelectSqlNulo"));

}
//------------------------------------------------------------------------------
//ID_QRY_COBRANZA_COBRADOR
void ServidorVentas::ConsultaCobranzaCobrador(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros){

	// CONSULTA COMISIONES VENDEDOR
	AnsiString instruccion;
	AnsiString muestrasolofacturassincheque=" ";
	AnsiString busquedaporsector=" ";
	AnsiString busqueda_vencidos=" ";
	AnsiString busquedaporlocalidad=" ";
	AnsiString innerJoinColonias= " ";
	AnsiString fecha_inicio, fecha_fin, cobrador, sector, localidad, mostrarfacturascheque, fecha_saldo;
	AnsiString condicion_cobrador=" ",condicion_status=" ";
	AnsiString vendedor, status, solo_vencidos;
	AnsiString cliente, condicion_cliente=" ";
	AnsiString forzar_indice=" ";
	AnsiString archivo_temp1;
	int num_instrucciones=1;
	TDate date_fecha_inicio, date_fecha_fin, date_fecha_saldo;


	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	fecha_inicio=mFg.ExtraeStringDeBuffer(&parametros);
	fecha_fin=mFg.ExtraeStringDeBuffer(&parametros);
	cobrador=mFg.ExtraeStringDeBuffer(&parametros);
	sector=mFg.ExtraeStringDeBuffer(&parametros);
	localidad=mFg.ExtraeStringDeBuffer(&parametros);
	mostrarfacturascheque=mFg.ExtraeStringDeBuffer(&parametros);
	status=mFg.ExtraeStringDeBuffer(&parametros);
	cliente=mFg.ExtraeStringDeBuffer(&parametros);
	fecha_saldo=mFg.ExtraeStringDeBuffer(&parametros);
	solo_vencidos=mFg.ExtraeStringDeBuffer(&parametros);//Mostrar Solo Vencidos
	date_fecha_inicio=StrToDate(fecha_inicio);
	date_fecha_fin=StrToDate(fecha_fin);
	date_fecha_saldo=StrToDate(fecha_saldo);

	if(cliente!=""){
		condicion_cliente.sprintf(" and v.cliente='%s' ", cliente);
	}
	if(status!=""){
		condicion_status.sprintf(" and v.ubicacion='%s' ", status);
	}
	if(cobrador!=""){
		condicion_cobrador.sprintf(" and v.cobrador='%s' ", cobrador);
		// Optimización:
		// En esta parte se debe poner el cobrador que tenga la mayoría de ventas x cobrar
		// en el caso de la violeta es TIEN
		if (cobrador=="TIEN") {
			forzar_indice=" force index(PRIMARY) ";
		}
	}
	if(mostrarfacturascheque!="1"){
		muestrasolofacturassincheque=" and vs.saldor<>(vs.chcnc*-1) ";
	}
	if(sector!=""){
		busquedaporsector.sprintf(" and col.sector='%s' ",sector);
		innerJoinColonias=" inner join colonias col on c.colonia=col.colonia ";
	}
	if(localidad!=""){
		busquedaporlocalidad.sprintf(" and col.localidad='%s' ",localidad);
		innerJoinColonias=" inner join colonias col on c.colonia=col.colonia ";
	}

	// Optimización:
	// Si el rango de fechas es pequeño, arbitrariamente elejimos menor de 2 meses, se
	// forza el indice fechavta, pues reduce el tamañó de la muestra.
	if (abs(double(date_fecha_fin-date_fecha_inicio))<=62) {
		forzar_indice=" force index(fechavta) ";
	}

	// Crea una tabla donde se van a poner los saldos de las ventas
	// afectadas por la cancelación
	instruccion = "DROP TABLE IF EXISTS auxventassaldos";
	mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL, instruccion.c_str());

	instruccion="create temporary table auxventassaldos (venta char(11), saldor decimal(16,2), \
		chcnc decimal(16,2), tncredito decimal(16,2), tncargo decimal(16,2), fechach date, \
		cliente VARCHAR(11), tipofac VARCHAR(11), foliofisic VARCHAR(11), fechavta DATE, valor DECIMAL(16,2), \
		ubicacion VARCHAR(10), plazo INT(3), fechavenc DATE, rsocial varchar(255), cobrador VARCHAR(120), \
		PRIMARY KEY (venta)) Engine = InnoDB";
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query EjecutaSelectSqlNulo"));

	// Calcula los saldos de las ventas del cliente
	archivo_temp1=mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);
	instruccion.sprintf("select v.referencia as venta, \
		sum(if(t.aplicada=1, t.valor, 0)) as saldor, \
		sum(if(t.aplicada=0, t.valor, 0)) as chcnc, \
		sum(if(t.aplicada=1 and t.tipo='DEVO', t.valor, 0)) as tncredito, \
		sum(if(t.aplicada=1 and (t.tipo='NCAR' or t.tipo='INTE'), t.valor, 0)) as tncargo, \
		max(ifnull(chcl.fechacob, cast('1900-01-01' as date))) as fechach, \
		v.cliente as cliente, v.tipofac, v.foliofisic, v.fechavta, v.valor, v.ubicacion, v.plazo,  \
		v.fechavenc, c.rsocial, CONCAT(e.nombre, ' ', e.appat, ' ', e.apmat) \
		from ventas v %s \
		INNER JOIN clientes c ON c.cliente = v.cliente \
		INNER JOIN empleados e ON e.empleado = v.cobrador \
		%s %s %s \
		INNER JOIN transxcob t ON t.referencia=v.referencia\
		INNER JOIN terminales ter ON ter.terminal = v.terminal \
		INNER JOIN secciones sec ON sec.seccion = ter.seccion \
		INNER JOIN sucursales suc ON suc.sucursal = sec.sucursal \
		left join pagoscli p on t.pago=p.pago and t.aplicada=0 \
		left join cheqxcob chxc on p.pago=chxc.pago \
		left join chequesclientes chcl on chxc.chequecli=chcl.chequecli \
		where v.fechavta>='%s' and v.fechavta<='%s' and t.fechaapl<='%s'\
		and t.cancelada=0 and v.cancelado=0 AND suc.idempresa = %s \
		%s %s %s \
		group by v.referencia INTO OUTFILE '%s'",
		forzar_indice, innerJoinColonias,   busquedaporsector, busquedaporlocalidad,
		mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin), mFg.StrToMySqlDate(fecha_saldo),
		FormServidor->ObtieneClaveEmpresa(), condicion_cobrador, condicion_status, condicion_cliente, archivo_temp1);

	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query EjecutaSelectSqlNulo"));


	instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE auxventassaldos ",
		archivo_temp1);
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error en query EjecutaSelectSqlNulo"));

	if(solo_vencidos=="1")
		busqueda_vencidos.sprintf(" datediff('%s',vs.fechavta)-ifnull(vs.plazo,0)>0 and ",mFg.StrToMySqlDate(fecha_saldo));

	instruccion.sprintf("DROP TABLE IF EXISTS aux_max_valor_ventas");
	mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL, instruccion.c_str());

	instruccion.sprintf("CREATE TEMPORARY TABLE IF NOT EXISTS aux_max_valor_ventas(\
		maxvalor DECIMAL(16,2), \
		referencia VARCHAR(11), \
		INDEX(maxvalor), \
		INDEX(referencia) \
		)");
	mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL, instruccion.c_str());
	AnsiString at_MaxValorVentas;
	try{
		at_MaxValorVentas = mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);
		instruccion.sprintf("SELECT max(dvfp4.valor) AS maxvalor, dvfp4.referencia \
				FROM auxventassaldos v \
				inner join dventasfpago dvfp4 ON v.venta = dvfp4.referencia \
				WHERE v.fechavta>='%s' and v.fechavta<='%s' GROUP BY v.venta \
				INTO OUTFILE '%s'",
				mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin), at_MaxValorVentas);
		mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL, instruccion.c_str());


		instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE aux_max_valor_ventas", at_MaxValorVentas);
		mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL, instruccion.c_str());
	}__finally{
		mServidorVioleta->BorraArchivoTemp(at_MaxValorVentas);
	}


    instruccion.sprintf("select vs.tipofac, vs.venta as referencia, \
		if(ifnull(vs.foliofisic,'')='',concat(ifnull(cfd.serie,''),ifnull(cfd.folio,'')),vs.foliofisic) as foliofisic, \
		vs.cliente as codigocli, vs.rsocial, \
		vs.saldor, (vs.chcnc*-1) as chnocob, (vs.tncredito*-1) as tncredito, vs.tncargo, vs.fechach, \
		vs.fechavta, vs.valor, vs.ubicacion, \
		datediff('%s',vs.fechavta) as dias, \
		datediff('%s',vs.fechavta)-ifnull(vs.plazo,0) as diasvencido, vs.cobrador, \
		ifnull(vs.plazo,0) as plazo, vs.fechavenc, fp.termino \
		from auxventassaldos vs \
			LEFT JOIN (SELECT dvfp3.referencia, dvfp3.formapag \
			FROM aux_max_valor_ventas dvfp4 \
			INNER JOIN dventasfpago dvfp3 ON dvfp4.referencia = dvfp3.referencia AND dvfp3.valor = dvfp4.maxvalor \
			GROUP BY dvfp3.referencia \
			) dvfp2 ON dvfp2.referencia = vs.venta \
			left JOIN formasdepago fp ON fp.formapag = dvfp2.formapag \
			left join cfd on vs.venta=cfd.referencia and cfd.tipocomprobante='VENT' \
		where %s vs.saldor>0 and \
			vs.fechavta>='%s' and vs.fechavta<='%s' \
			%s \
			order by vs.venta ",
			mFg.StrToMySqlDate(fecha_saldo),mFg.StrToMySqlDate(fecha_saldo),
			busqueda_vencidos,
			mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin),
			muestrasolofacturassincheque);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	mServidorVioleta->BorraArchivoTemp(archivo_temp1);

}
//------------------------------------------------------------------------------
//ID_QRY_PAGOS_PROVEEDOR
void ServidorVentas::ConsultaPagosProveedor(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros){

	AnsiString instruccion;
	AnsiString muestrasolofacturassincheque=" ";
	AnsiString busquedaporsector=" ";
	AnsiString busquedaporlocalidad=" ";
	AnsiString fecha_inicio, fecha_fin, proveedor, sucursal, fecha_vence_ini, fecha_vence,usar_fechavence,con_saldo, fecha_saldo;
	AnsiString condicion_cobrador=" ",condicion_status=" ";
	AnsiString vendedor, status;
	AnsiString cliente, condicion_cliente=" ";
	AnsiString forzar_indice=" ";
	TDate date_fecha_inicio, date_fecha_fin, date_fecha_vence_ini, date_fecha_vence, date_fecha_saldo;
	AnsiString condicion_proveedor=" ", condicion_sucursal=" ", condicion_sucursal2=" ", condicion_fechavence=" ";
	AnsiString condicion_con_saldo=" ", condicion_left=" ", mostrarfacturascheque;
	AnsiString omitiruuid;
	AnsiString condicion_mostraruuid=" ";
	AnsiString tipospagos;
	AnsiString idEmpresa, condicion_empresa = " ";

    int agrupaprov;

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	fecha_inicio=mFg.ExtraeStringDeBuffer(&parametros);
	fecha_fin=mFg.ExtraeStringDeBuffer(&parametros);
	fecha_vence_ini=mFg.ExtraeStringDeBuffer(&parametros);
	fecha_vence=mFg.ExtraeStringDeBuffer(&parametros);
	proveedor=mFg.ExtraeStringDeBuffer(&parametros);
	sucursal=mFg.ExtraeStringDeBuffer(&parametros);
	usar_fechavence=mFg.ExtraeStringDeBuffer(&parametros);
	con_saldo=mFg.ExtraeStringDeBuffer(&parametros);
	mostrarfacturascheque=mFg.ExtraeStringDeBuffer(&parametros);
	fecha_saldo=mFg.ExtraeStringDeBuffer(&parametros);
	omitiruuid=mFg.ExtraeStringDeBuffer(&parametros);
	tipospagos=mFg.ExtraeStringDeBuffer(&parametros);
	agrupaprov=StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
	idEmpresa = mFg.ExtraeStringDeBuffer(&parametros);

	date_fecha_inicio=StrToDate(fecha_inicio);
	date_fecha_fin=StrToDate(fecha_fin);
	date_fecha_vence=StrToDate(fecha_vence);
	date_fecha_vence_ini=StrToDate(fecha_vence_ini);
	date_fecha_saldo=StrToDate(fecha_saldo);

	if(proveedor!=""){
		condicion_proveedor.sprintf(" and c.proveedor='%s' ", proveedor);
	}
	if(sucursal!=""){
		condicion_sucursal.sprintf(" and sec.sucursal='%s' ", sucursal);
		condicion_sucursal2.sprintf(" and c.sucursal='%s' ", sucursal);
	} else if (idEmpresa != ""){
		condicion_empresa.sprintf(" AND suc.idempresa = %s ", idEmpresa);
	}
	if (usar_fechavence == "1") {
		condicion_fechavence.sprintf(" and c.fechavenc>='%s' and c.fechavenc<='%s' ", mFg.StrToMySqlDate(fecha_vence_ini), mFg.StrToMySqlDate(fecha_vence));
	}
	if (con_saldo == "1") {
		condicion_con_saldo.sprintf(" WHERE a.saldor>0 ");
	}
	if(mostrarfacturascheque == "0"){
		muestrasolofacturassincheque=" AND cs.saldor<>(cs.chcnc*-1) ";
	}
	if (omitiruuid=="1") {
        condicion_mostraruuid="AND c.muuid IS NOT NULL";
	}

	if (tipospagos == "0" || tipospagos == "2" ) {

		// Crea una tabla donde se van a poner los saldos de las ventas
		// afectadas por la cancelación
		instruccion="CREATE TEMPORARY TABLE auxcomprasprove (compra CHAR(11),sucursal CHAR(3), saldor DECIMAL(16,2), \
			chcnc DECIMAL(16,2), tncredito DECIMAL(16,2), tncargo DECIMAL(16,2), fechach DATE, PRIMARY KEY (compra)) ENGINE = INNODB";
		if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
			throw (Exception("Error en query EjecutaSelectSqlNulo"));

		// Crea una tabla donde se van a poner las compras que tienen notas de crédito a las que les falta capturar el UUID
		instruccion="CREATE TEMPORARY TABLE auxNotasSinUuid (compra CHAR(11), notasSinUuid tinyint, \
			PRIMARY KEY (compra)) ENGINE = INNODB";
		if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
			throw (Exception("Error en query EjecutaSelectSqlNulo"));

		// Calcula notas de crédito a las que les falta capturar el UUID
		instruccion.sprintf("insert into auxNotasSinUuid (compra,notasSinUuid) \
			SELECT c.referencia AS compra, \
			max(if(LENGTH(IFNULL(nc.muuid,''))=0,1,0)) as notaSinUuid \
			FROM compras c \
			inner join terminales ter on ter.terminal=c.terminal \
			inner JOIN secciones sec ON sec.seccion=ter.seccion \
			INNER JOIN sucursales suc ON suc.sucursal = sec.sucursal \
			INNER JOIN notascredprov nc ON nc.compra=c.referencia and nc.cancelado=0 \
			WHERE c.fechacom>='%s' AND c.fechacom<='%s' and nc.fechanot<='%s' \
			%s %s %s %s \
			AND c.cancelado = 0 \
			GROUP BY c.referencia ",
			mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin), mFg.StrToMySqlDate(fecha_saldo),
			condicion_proveedor, condicion_sucursal, condicion_empresa, condicion_fechavence);
		if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
			throw (Exception("Error en query EjecutaSelectSqlNulo"));

		// Calcula los saldos de las ventas del proveedor
		instruccion.sprintf("insert into auxcomprasprove (compra,sucursal, saldor, chcnc, tncredito, tncargo,fechach) \
			SELECT a.compra,a.sucursal,a.saldor,a.chcnc,a.tncredito,a.tncargo,a.fechach FROM ( \
			SELECT c.referencia AS compra, \
			sec.sucursal , \
			SUM(IF(t.aplicada=1, t.valor, 0)) AS saldor, \
			SUM(IF(t.aplicada=0, t.valor, 0)) AS chcnc, \
			SUM(IF(t.aplicada=1 AND t.tipo='DEVO', t.valor, 0)) AS tncredito, \
			SUM(IF(t.aplicada=1 AND (t.tipo='NCAR' OR t.tipo='INTE'), t.valor, 0)) AS tncargo, \
			MAX(IFNULL(chcl.fechacob, CAST('1900-01-01' AS DATE))) AS fechach \
			FROM compras c INNER JOIN  transxpag t \
			inner join terminales ter on ter.terminal=c.terminal \
			inner JOIN secciones sec ON sec.seccion=ter.seccion \
			INNER JOIN sucursales suc ON sec.sucursal = suc.sucursal \
			LEFT JOIN pagosprov p ON t.pago=p.pago AND t.aplicada=0 \
			LEFT JOIN cheqxpag chxc ON p.pago=chxc.pago \
			LEFT JOIN chequesproveedores chcl ON chxc.chequeprov=chcl.chequeprov \
			WHERE c.fechacom>='%s' AND c.fechacom<='%s' AND t.referencia=c.referencia AND t.fechaapl<='%s'\
			%s %s %s %s \
			AND t.cancelada = 0 \
			AND c.cancelado = 0 \
			GROUP BY c.referencia \
			) AS a %s ",
			mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin),mFg.StrToMySqlDate(fecha_saldo),
			condicion_proveedor, condicion_sucursal, condicion_empresa, condicion_fechavence,
			condicion_con_saldo);
		if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
			throw (Exception("Error en query EjecutaSelectSqlNulo"));

	}
	if (tipospagos == "1" || tipospagos == "2") {

		// Crea una tabla donde se van a poner los saldos de los gastos afectadas por la cancelación
		instruccion="CREATE TEMPORARY TABLE auxgastosprove ( \
		gasto CHAR(11), \
		sucursal CHAR(3), \
		saldor DECIMAL(16,2), \
		chcnc DECIMAL(16,2), \
		tncredito DECIMAL(16,2), \
		tncargo DECIMAL(16,2), \
		fechach DATE, \
		PRIMARY KEY (gasto)) ENGINE = INNODB";
		if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
			throw (Exception("Error en query EjecutaSelectSqlNulo"));

		// Crea una tabla donde se van a poner las compras que tienen notas de crédito a las que les falta capturar el UUID
		instruccion="CREATE TEMPORARY TABLE auxNotasGastosSinUuid ( \
		gasto CHAR(11), \
		notasSinUuid tinyint, \
		PRIMARY KEY (gasto)) ENGINE = INNODB";
		if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
			throw (Exception("Error en query EjecutaSelectSqlNulo"));


		// Calcula notas de crédito a las que les falta capturar el UUID
		instruccion.sprintf("insert into auxNotasGastosSinUuid (gasto,notasSinUuid) \
			SELECT c.referencia AS gasto, \
				   max(if(LENGTH(IFNULL(nc.muuid,''))=0,1,0)) AS notaSinUuid \
			FROM gastos c \
			INNER JOIN terminales ter ON ter.terminal=c.terminal \
			INNER JOIN secciones sec ON sec.seccion=ter.seccion \
			INNER JOIN sucursales suc ON sec.sucursal = suc.sucursal \
			INNER JOIN notascredgasto nc ON nc.gasto=c.referencia \
			AND nc.cancelado=0 \
			WHERE c.fechagas>='%s' AND c.fechagas<='%s' AND nc.fechanot<='%s' \
			%s %s %s %s\
			AND c.cancelado = 0 \
			GROUP BY c.referencia",
			mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin), mFg.StrToMySqlDate(fecha_saldo),
			condicion_proveedor, condicion_sucursal2, condicion_empresa, condicion_fechavence);
		if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
			throw (Exception("Error en query EjecutaSelectSqlNulo"));


		// Calcula los saldos de las ventas del proveedor
		instruccion.sprintf("INSERT INTO auxgastosprove (gasto,sucursal, saldor, chcnc, tncredito, tncargo,fechach) \
		SELECT a.gasto, a.sucursal, a.saldor, a.chcnc, a.tncredito, a.tncargo, a.fechach \
		FROM (SELECT c.referencia AS gasto, c.sucursal , SUM(IF(t.aplicada=1, t.valor, 0)) AS saldor, \
		SUM(IF(t.aplicada=0, t.valor, 0)) AS chcnc, SUM(IF(t.aplicada=1 AND t.tipo='DEVO', t.valor, 0)) AS tncredito, \
		SUM(IF(t.aplicada=1 AND (t.tipo='NCAR' OR t.tipo='INTE'), t.valor, 0)) AS tncargo, MAX(IFNULL(chcl.fechacob, CAST('1900-01-01' AS DATE))) AS fechach \
		FROM gastos c \
		INNER JOIN transxpaggastos t ON t.referencia=c.referencia \
		INNER JOIN terminales ter ON ter.terminal=c.terminal \
		INNER JOIN secciones sec ON sec.seccion=ter.seccion \
		INNER JOIN sucursales suc ON sec.sucursal = suc.sucursal \
		LEFT JOIN pagosgastos p ON t.pago=p.pago AND t.aplicada=0 \
		LEFT JOIN cheqxgas chxc ON p.pago=chxc.pago \
		LEFT JOIN chequesgastos chcl ON chxc.chequegasto=chcl.chequegasto \
		WHERE c.fechagas>='%s' AND c.fechagas<='%s' AND t.fechaapl<='%s' \
		%s %s %s %s \
		AND t.cancelada = 0 \
		AND c.cancelado = 0 \
		GROUP BY c.referencia) AS a  %s ",
		mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin),mFg.StrToMySqlDate(fecha_saldo),
		condicion_proveedor, condicion_sucursal2, condicion_empresa, condicion_fechavence,
		condicion_con_saldo);
		if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
			throw (Exception("Error en query EjecutaSelectSqlNulo"));
	}

	if (tipospagos == "0") {

		if(agrupaprov==1){
			instruccion.sprintf("SELECT t.referencia, t.sucursal, t.folio, \
			t.uuid, t.codigoprove, t.razonsocial, t.saldor, t.chnocob, \
			t.tncredito, t.tncargo, t.fechach, t.fecha, t.valor, \
			t.fechavenc, t.diasvenc,  t.notasSinUuid, t.tipo, t.emitecpago \
			FROM ( \
			(SELECT \
				c.referencia, \
				cs.sucursal, \
				IFNULL(c.folioprov,'') AS folio, \
				c.muuid as uuid, \
				c.proveedor AS codigoprove, \
				p.razonsocial, \
				SUM(cs.saldor) AS saldor, \
				SUM(cs.chcnc*-1) AS chnocob, \
				SUM(cs.tncredito*-1) AS tncredito, \
				SUM(cs.tncargo) AS tncargo, \
				cs.fechach, \
				c.fechacom AS fecha, \
				SUM(c.valor) AS valor, \
				c.fechavenc, \
				MAX(DATEDIFF('%s',c.fechacom)- IFNULL(c.plazo,0)) AS diasvenc, \
				MAX(ifnull(nsu.notasSinUuid,0)) as notasSinUuid, \
				'Compra' AS tipo, IF(cfdi.emitecpago=1, 'Si', IF(cfdi.emitecpago=0, 'No', '')) AS emitecpago \
			FROM proveedores p INNER JOIN compras c INNER JOIN auxcomprasprove cs \
			LEFT JOIN auxNotasSinUuid nsu ON c.referencia=nsu.compra \
			LEFT JOIN cfdicompras cfdi ON cfdi.referencia=c.referencia \
			WHERE p.proveedor=c.proveedor %s %s \
			AND cs.compra=c.referencia AND cs.saldor>0 AND \
			c.fechacom>='%s' AND c.fechacom<='%s' AND p.agrupapagfact = 1 \
			GROUP BY p.proveedor \
			ORDER BY c.referencia \
			) UNION ALL ( \
			SELECT \
				c.referencia, \
				cs.sucursal, \
				IFNULL(c.folioprov,'') AS folio, \
				c.muuid as uuid, \
				c.proveedor AS codigoprove, \
				p.razonsocial, \
				cs.saldor, \
				(cs.chcnc*-1) AS chnocob, \
				(cs.tncredito*-1) AS tncredito, \
				cs.tncargo, \
				cs.fechach, \
				c.fechacom AS fecha, \
				c.valor, \
				c.fechavenc, \
				DATEDIFF('%s',c.fechacom)- IFNULL(c.plazo,0) AS diasvenc, \
				ifnull(nsu.notasSinUuid,0) as notasSinUuid, 'Compra' AS tipo, \
				IF(cfdi.emitecpago=1, 'Si', IF(cfdi.emitecpago=0, 'No', '')) AS emitecpago \
			FROM proveedores p INNER JOIN compras c INNER JOIN auxcomprasprove cs \
			LEFT JOIN auxNotasSinUuid nsu ON c.referencia=nsu.compra \
            LEFT JOIN cfdicompras cfdi ON cfdi.referencia=c.referencia \
			WHERE p.proveedor=c.proveedor %s %s \
			AND cs.compra=c.referencia AND cs.saldor>0 AND \
			c.fechacom>='%s' AND c.fechacom<='%s' AND p.agrupapagfact = 0 ORDER BY c.referencia \
			)) t \
			GROUP BY t.referencia",
			mFg.StrToMySqlDate(fecha_saldo), muestrasolofacturassincheque, condicion_mostraruuid,
			mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin),

			mFg.StrToMySqlDate(fecha_saldo), muestrasolofacturassincheque, condicion_mostraruuid,
			mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin));
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		} else {
			instruccion.sprintf("SELECT c.referencia,cs.sucursal ,IFNULL(c.folioprov,'') AS folio,c.muuid as uuid, c.proveedor AS codigoprove, p.razonsocial, \
			cs.saldor, (cs.chcnc*-1) AS chnocob, (cs.tncredito*-1) AS tncredito, cs.tncargo, \
			cs.fechach, c.fechacom AS fecha, c.valor, c.fechavenc, DATEDIFF('%s',c.fechacom)- IFNULL(c.plazo,0) AS diasvenc, \
			ifnull(nsu.notasSinUuid,0) as notasSinUuid, 'Compra' AS tipo, IF(cfdi.emitecpago=1, 'Si', IF(cfdi.emitecpago=0, 'No', '')) AS emitecpago \
			FROM proveedores p INNER JOIN compras c INNER JOIN auxcomprasprove cs \
			LEFT JOIN auxNotasSinUuid nsu ON c.referencia=nsu.compra \
            LEFT JOIN cfdicompras cfdi ON cfdi.referencia=c.referencia \
			WHERE p.proveedor=c.proveedor %s %s \
			AND cs.compra=c.referencia AND cs.saldor>0 AND \
			c.fechacom>='%s' AND c.fechacom<='%s' ORDER BY c.referencia ",
			mFg.StrToMySqlDate(fecha_saldo), muestrasolofacturassincheque, condicion_mostraruuid,
			mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin));
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	} else if (tipospagos == "1") {

			if(agrupaprov==1){
				instruccion.sprintf("SELECT t.referencia, t.sucursal, t.folio, \
				t.uuid, t.codigoprove, t.razonsocial, t.saldor, t.chnocob, \
				t.tncredito, t.tncargo, t.fechach, t.fecha, t.valor, \
				t.fechavenc, t.diasvenc,  t.notasSinUuid, t.tipo, t.emitecpago \
				FROM ( \
				(SELECT \
					c.referencia, \
					cs.sucursal, \
					IFNULL(c.folioprov,'') AS folio, \
					c.muuid as uuid, \
					c.proveedor AS codigoprove, \
					p.razonsocial, \
					SUM(cs.saldor) AS saldor, \
					SUM(cs.chcnc*-1) AS chnocob, \
					SUM(cs.tncredito*-1) AS tncredito, \
					SUM(cs.tncargo) AS tncargo, \
					cs.fechach, \
					c.fechagas AS fecha, \
					SUM(c.valor) AS valor, \
					c.fechavenc, \
					MAX(DATEDIFF('%s',c.fechagas)- IFNULL(c.plazo,0)) AS diasvenc, \
					MAX(ifnull(nsu.notasSinUuid,0)) as notasSinUuid, \
					'Gasto' AS tipo, ' ' AS emitecpago \
				FROM proveedores p INNER JOIN gastos c INNER JOIN auxgastosprove cs \
				LEFT JOIN auxNotasGastosSinUuid nsu ON c.referencia=nsu.gasto \
				WHERE p.proveedor=c.proveedor %s %s \
				AND cs.gasto=c.referencia AND cs.saldor>0 AND \
				c.fechagas>='%s' AND c.fechagas<='%s' AND p.agrupapaggast = 1 \
				GROUP BY p.proveedor \
				ORDER BY c.referencia \
				) UNION ALL ( \
				SELECT \
					c.referencia, \
					cs.sucursal, \
					IFNULL(c.folioprov,'') AS folio, \
					c.muuid as uuid, \
					c.proveedor AS codigoprove, \
					p.razonsocial, \
					cs.saldor, \
					(cs.chcnc*-1) AS chnocob, \
					(cs.tncredito*-1) AS tncredito, \
					cs.tncargo, \
					cs.fechach, \
					c.fechagas AS fecha, \
					c.valor, \
					c.fechavenc, \
					DATEDIFF('%s',c.fechagas)- IFNULL(c.plazo,0) AS diasvenc, \
					ifnull(nsu.notasSinUuid,0) as notasSinUuid, \
					'Gasto' AS tipo, ' ' AS emitecpago \
				FROM proveedores p INNER JOIN gastos c INNER JOIN auxgastosprove cs \
				LEFT JOIN auxNotasGastosSinUuid nsu ON c.referencia=nsu.gasto \
				WHERE p.proveedor=c.proveedor %s %s \
				AND cs.gasto=c.referencia AND cs.saldor>0 AND \
				c.fechagas>='%s' AND c.fechagas<='%s' AND p.agrupapaggast = 0 \
				ORDER BY c.referencia \
				)) t \
				GROUP BY t.referencia ",

				mFg.StrToMySqlDate(fecha_saldo), muestrasolofacturassincheque, condicion_mostraruuid,
				mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin),

				mFg.StrToMySqlDate(fecha_saldo), muestrasolofacturassincheque, condicion_mostraruuid,
				mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin));
				mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
			}else{
				instruccion.sprintf("SELECT c.referencia,cs.sucursal ,IFNULL(c.folioprov,'') AS folio,c.muuid as uuid, c.proveedor AS codigoprove, p.razonsocial, \
				cs.saldor, (cs.chcnc*-1) AS chnocob, (cs.tncredito*-1) AS tncredito, cs.tncargo, \
				cs.fechach, c.fechagas AS fecha, c.valor, c.fechavenc, DATEDIFF('%s',c.fechagas)- IFNULL(c.plazo,0) AS diasvenc, \
				ifnull(nsu.notasSinUuid,0) as notasSinUuid, 'Gasto' AS tipo, ' ' AS emitecpago \
				FROM proveedores p INNER JOIN gastos c INNER JOIN auxgastosprove cs \
				LEFT JOIN auxNotasGastosSinUuid nsu ON c.referencia=nsu.gasto \
				WHERE p.proveedor=c.proveedor %s %s \
				AND cs.gasto=c.referencia AND cs.saldor>0 AND \
				c.fechagas>='%s' AND c.fechagas<='%s' ORDER BY c.referencia ",
				mFg.StrToMySqlDate(fecha_saldo), muestrasolofacturassincheque, condicion_mostraruuid,
				mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin));
				mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
			}
	} else {
		if(agrupaprov==1){
            instruccion.sprintf("SELECT t.referencia, t.sucursal, t.folio, t.uuid, t.codigoprove, \
			t.razonsocial, t.saldor, t.chnocob, t.tncredito, t.tncargo, t.fechach, t.fecha, t.valor, t.fechavenc, \
			t.diasvenc, t.notasSinUuid, t.tipo, t.emitecpago FROM ( \
			(SELECT c.referencia,cs.sucursal ,IFNULL(c.folioprov,'') AS folio,c.muuid as uuid, c.proveedor AS codigoprove, p.razonsocial, \
			cs.saldor, (cs.chcnc*-1) AS chnocob, (cs.tncredito*-1) AS tncredito, cs.tncargo, \
			cs.fechach, c.fechacom AS fecha, c.valor, c.fechavenc, DATEDIFF('%s',c.fechacom)- IFNULL(c.plazo,0) AS diasvenc, \
			ifnull(nsu.notasSinUuid,0) as notasSinUuid, 'Compra' AS tipo, IF(cfdi.emitecpago=1, 'Si', IF(cfdi.emitecpago=0, 'No', '')) AS emitecpago \
			FROM proveedores p INNER JOIN compras c INNER JOIN auxcomprasprove cs \
			LEFT JOIN auxNotasSinUuid nsu ON c.referencia=nsu.compra \
            LEFT JOIN cfdicompras cfdi ON cfdi.referencia=c.referencia \
			WHERE p.proveedor=c.proveedor %s %s \
			AND cs.compra=c.referencia AND cs.saldor>0 AND \
			c.fechacom>='%s' AND c.fechacom<='%s' AND p.agrupapagfact = 0 \
			ORDER BY c.referencia \
			) UNION ALL ( \
			SELECT c.referencia,cs.sucursal ,IFNULL(c.folioprov,'') AS folio,c.muuid as uuid, c.proveedor AS codigoprove, p.razonsocial, \
			SUM(cs.saldor) AS saldor, SUM(cs.chcnc*-1) AS chnocob, SUM(cs.tncredito*-1) AS tncredito, SUM(cs.tncargo) AS tncargo, \
			cs.fechach, c.fechacom AS fecha, SUM(c.valor) AS valor, c.fechavenc, MAX(DATEDIFF('%s',c.fechacom)- IFNULL(c.plazo,0)) AS diasvenc, \
			MAX(ifnull(nsu.notasSinUuid,0)) as notasSinUuid, 'Compra' AS tipo, IF(cfdi.emitecpago=1, 'Si', IF(cfdi.emitecpago=0, 'No', '')) AS emitecpago \
			FROM proveedores p INNER JOIN compras c INNER JOIN auxcomprasprove cs \
			LEFT JOIN auxNotasSinUuid nsu ON c.referencia=nsu.compra \
            LEFT JOIN cfdicompras cfdi ON cfdi.referencia=c.referencia \
			WHERE p.proveedor=c.proveedor %s %s \
			AND cs.compra=c.referencia AND cs.saldor>0 AND \
			c.fechacom>='%s' AND c.fechacom<='%s' AND p.agrupapagfact = 1 \
			GROUP BY p.proveedor \
			ORDER BY c.referencia \
			) UNION ALL ( \
			SELECT c.referencia,cs.sucursal ,IFNULL(c.folioprov,'') AS folio,c.muuid as uuid, c.proveedor AS codigoprove, p.razonsocial, \
			cs.saldor, (cs.chcnc*-1) AS chnocob, (cs.tncredito*-1) AS tncredito, cs.tncargo, \
			cs.fechach, c.fechagas AS fecha, c.valor, c.fechavenc, DATEDIFF('%s',c.fechagas)- IFNULL(c.plazo,0) AS diasvenc, \
			ifnull(nsu.notasSinUuid,0) as notasSinUuid, 'Gasto' AS tipo, ' ' AS emitecpago \
			FROM proveedores p INNER JOIN gastos c INNER JOIN auxgastosprove cs \
			LEFT JOIN auxNotasGastosSinUuid nsu ON c.referencia=nsu.gasto \
			WHERE p.proveedor=c.proveedor %s %s \
			AND cs.gasto=c.referencia AND cs.saldor>0 AND \
			c.fechagas>='%s' AND c.fechagas<='%s' AND p.agrupapaggast = 0 \
			ORDER BY c.referencia \
			) UNION ALL ( \
			SELECT c.referencia,cs.sucursal ,IFNULL(c.folioprov,'') AS folio,c.muuid as uuid, c.proveedor AS codigoprove, p.razonsocial, \
			SUM(cs.saldor) AS saldor, SUM(cs.chcnc*-1) AS chnocob, SUM(cs.tncredito*-1) AS tncredito, SUM(cs.tncargo) AS tncargo, \
			cs.fechach, c.fechagas AS fecha, SUM(c.valor) AS valor, c.fechavenc, MAX(DATEDIFF('%s',c.fechagas)- IFNULL(c.plazo,0)) AS diasvenc, \
			MAX(ifnull(nsu.notasSinUuid,0)) as notasSinUuid, 'Gasto' AS tipo, ' ' AS emitecpago \
			FROM proveedores p INNER JOIN gastos c INNER JOIN auxgastosprove cs \
			LEFT JOIN auxNotasGastosSinUuid nsu ON c.referencia=nsu.gasto \
			WHERE p.proveedor=c.proveedor %s %s \
			AND cs.gasto=c.referencia AND cs.saldor>0 AND \
			c.fechagas>='%s' AND c.fechagas<='%s' AND p.agrupapaggast = 1 \
			GROUP BY p.proveedor \
			ORDER BY c.referencia )) t ORDER BY t.referencia",
			mFg.StrToMySqlDate(fecha_saldo),
			muestrasolofacturassincheque, condicion_mostraruuid,
			mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin),
			mFg.StrToMySqlDate(fecha_saldo),
			muestrasolofacturassincheque, condicion_mostraruuid,
			mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin),
			mFg.StrToMySqlDate(fecha_saldo),
			muestrasolofacturassincheque, condicion_mostraruuid,
			mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin),
			mFg.StrToMySqlDate(fecha_saldo),
			muestrasolofacturassincheque, condicion_mostraruuid,
			mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin));
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		} else {
			instruccion.sprintf("SELECT t.referencia, t.sucursal, t.folio, t.uuid, t.codigoprove, \
			t.razonsocial, t.saldor, t.chnocob, t.tncredito, t.tncargo, t.fechach, t.fecha, t.valor, t.fechavenc, \
			t.diasvenc, t.notasSinUuid, t.tipo, t.emitecpago FROM ( \
			(SELECT c.referencia,cs.sucursal ,IFNULL(c.folioprov,'') AS folio,c.muuid as uuid, c.proveedor AS codigoprove, p.razonsocial, \
			cs.saldor, (cs.chcnc*-1) AS chnocob, (cs.tncredito*-1) AS tncredito, cs.tncargo, \
			cs.fechach, c.fechacom AS fecha, c.valor, c.fechavenc, DATEDIFF('%s',c.fechacom)- IFNULL(c.plazo,0) AS diasvenc, \
			ifnull(nsu.notasSinUuid,0) as notasSinUuid, 'Compra' AS tipo, IF(cfdi.emitecpago=1, 'Si', IF(cfdi.emitecpago=0, 'No', '')) AS emitecpago  \
			FROM proveedores p INNER JOIN compras c INNER JOIN auxcomprasprove cs \
			LEFT JOIN auxNotasSinUuid nsu ON c.referencia=nsu.compra \
            LEFT JOIN cfdicompras cfdi ON cfdi.referencia=c.referencia \
			WHERE p.proveedor=c.proveedor %s %s \
			AND cs.compra=c.referencia AND cs.saldor>0 AND \
			c.fechacom>='%s' AND c.fechacom<='%s' ORDER BY c.referencia \
			) UNION ALL ( \
			SELECT c.referencia,cs.sucursal ,IFNULL(c.folioprov,'') AS folio,c.muuid as uuid, c.proveedor AS codigoprove, p.razonsocial, \
			cs.saldor, (cs.chcnc*-1) AS chnocob, (cs.tncredito*-1) AS tncredito, cs.tncargo, \
			cs.fechach, c.fechagas AS fecha, c.valor, c.fechavenc, DATEDIFF('%s',c.fechagas)- IFNULL(c.plazo,0) AS diasvenc, \
			ifnull(nsu.notasSinUuid,0) as notasSinUuid, 'Gasto' AS tipo, ' ' AS emitecpago \
			FROM proveedores p INNER JOIN gastos c INNER JOIN auxgastosprove cs \
			LEFT JOIN auxNotasGastosSinUuid nsu ON c.referencia=nsu.gasto \
			WHERE p.proveedor=c.proveedor %s %s \
			AND cs.gasto=c.referencia AND cs.saldor>0 AND \
			c.fechagas>='%s' AND c.fechagas<='%s' ORDER BY c.referencia )) t ORDER BY t.referencia",
			mFg.StrToMySqlDate(fecha_saldo),
			muestrasolofacturassincheque, condicion_mostraruuid,
			mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin),
			mFg.StrToMySqlDate(fecha_saldo),
			muestrasolofacturassincheque, condicion_mostraruuid,
			mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin));
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
    }

}
//------------------------------------------------------------------------------
//ID_CAMBIA_COBRADOR_VENTA
void ServidorVentas::CambiaCobradorVenta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros){

	// CAMBIA COBRADOR A VENTA O VENTAS
	char *buffer_sql=new char[1024*64];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instrucciones[50];
	int num_instrucciones=0;
	AnsiString instruccion;
	int soloseleccionada;
	AnsiString nuevocobrador,referenciaoclave,cobradoractual;

	try{
		soloseleccionada=StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
		nuevocobrador=mFg.ExtraeStringDeBuffer(&parametros);
		cobradoractual=mFg.ExtraeStringDeBuffer(&parametros);
		referenciaoclave=mFg.ExtraeStringDeBuffer(&parametros);

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		if(soloseleccionada){
			instruccion.sprintf("update ventas v \
			set v.cobrador='%s' \
			where v.referencia='%s'",nuevocobrador,referenciaoclave);
			instrucciones[num_instrucciones++]=instruccion;
		}else{

			// Crea una tabla donde se van a poner los saldos de las ventas
			// NOTA: NO SE PONE AUX como prefijo ni como sufijo a esta tabla temporal para que sí se replique
			// de lo contrario e update puede dar problemas.
			instruccion="create temporary table tmpventassaldos (venta char(11), saldor decimal(16,2), PRIMARY KEY (venta)) Engine = InnoDB";
			instrucciones[num_instrucciones++]=instruccion;

			// Calcula los saldos de las ventas del cliente
			instruccion.sprintf("insert into tmpventassaldos (venta, saldor) \
				select v.referencia as venta, \
				sum(if(t.aplicada=1, t.valor, 0)) as saldor \
				from ventas v, transxcob t \
				where v.cobrador='%s' and \
				t.referencia=v.referencia and t.cancelada=0 and v.cancelado=0 \
				group by v.referencia",
				cobradoractual);

			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("update ventas v, tmpventassaldos vs \
				set cobrador='%s' \
				where v.referencia=vs.venta and vs.saldor>0 and \
				cliente='%s'",nuevocobrador,referenciaoclave);
			instrucciones[num_instrucciones++]=instruccion;

			//instruccion.sprintf("update clientes c set c.cobrador='%s' where c.cliente='%s'",nuevocobrador, referenciaoclave);
			 instruccion.sprintf("UPDATE clientesemp cemp INNER JOIN clientes c ON cemp.cliente = c.cliente \
			 SET cemp.cobrador='%s' \
			 WHERE cemp.cliente='%s' AND cemp.idempresa=%s",nuevocobrador, referenciaoclave, FormServidor->ObtieneClaveEmpresa() );

			instrucciones[num_instrucciones++]=instruccion;
		}

		instrucciones[num_instrucciones++]="COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (int i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);
	} __finally {
		delete buffer_sql;
	}
}
//------------------------------------------------------------------------------
//ID_CAMBIA_STATUS_VENTA
void ServidorVentas::CambiaStatusVenta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros){

	// CAMBIA COBRADOR A VENTA O VENTAS
	char *buffer_sql=new char[1024*64];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instrucciones[50];
	int num_instrucciones=0;
	AnsiString instruccion;
	int soloseleccionada;
	AnsiString nuevostatus,referenciaoclave,cobradoractual,condicion_cob=" ";

	try{
		soloseleccionada=StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
		nuevostatus=mFg.ExtraeStringDeBuffer(&parametros);
		cobradoractual=mFg.ExtraeStringDeBuffer(&parametros);
		referenciaoclave=mFg.ExtraeStringDeBuffer(&parametros);
		if(cobradoractual==""){
			condicion_cob=" ";
		} else{
			condicion_cob.sprintf("v.cobrador='%s' and", cobradoractual);
        }

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		if(soloseleccionada){
			instruccion.sprintf("update ventas v \
			set v.ubicacion='%s' \
			where v.referencia='%s'",nuevostatus,referenciaoclave);
			instrucciones[num_instrucciones++]=instruccion;
		}else{

			// Crea una tabla donde se van a poner los saldos de las ventas
			// NOTA: NO SE PONE AUX como prefijo ni como sufijo a esta tabla temporal para que sí se replique
			// de lo contrario e update puede dar problemas.
			instruccion="create temporary table tmpventassaldos (venta char(11), saldor decimal(16,2), PRIMARY KEY (venta)) Engine = InnoDB";
			instrucciones[num_instrucciones++]=instruccion;

			// Calcula los saldos de las ventas del cliente
			instruccion.sprintf("insert into tmpventassaldos (venta, saldor) \
				select v.referencia as venta, \
				sum(if(t.aplicada=1, t.valor, 0)) as saldor \
				from ventas v, transxcob t \
				where %s \
				t.referencia=v.referencia and t.cancelada=0 and v.cancelado=0 \
				group by v.referencia",
			   condicion_cob );

			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("update ventas v, tmpventassaldos vs \
			set ubicacion='%s' \
			where v.referencia=vs.venta and vs.saldor>0 and \
			cliente='%s'",nuevostatus,referenciaoclave);
			instrucciones[num_instrucciones++]=instruccion;

		}

		instrucciones[num_instrucciones++]="COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (int i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);
	} __finally {
		delete buffer_sql;
	}
}
//------------------------------------------------------------------------------
// ID_ASIG_FOLFISIC_VTA
void ServidorVentas::AsignaFolioFisicoVenta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  ASIGNACION MANUAL DE FOLIO FISICO
	char *buffer_sql=new char[1024*64*10];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString referencia, foliofisico, terminal;
	int num_partidas, i;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[1000];

	try{
		referencia=mFg.ExtraeStringDeBuffer(&parametros); // Clave de la venta.
		foliofisico=mFg.ExtraeStringDeBuffer(&parametros); // Folio físico que se va a asignar a la venta.
		terminal=mFg.ExtraeStringDeBuffer(&parametros); // Terminal donde se manda a imprimir.

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		// Folio físico.
		instruccion.sprintf("update ventas v set v.foliofisic='%s' where v.referencia='%s' and  @asigfolvta=0", foliofisico, referencia);
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
// ID_MOD_SIGFOLFISIC
void ServidorVentas::ModificaSigFolioFisico(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  ASIGNACION MANUAL DE FOLIO FISICO
	char *buffer_sql=new char[1024*64*10];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString tipo, serie, condicion_serie, folio, ancho, terminal;
	int num_partidas, i;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[1000];

	try{
		tipo=mFg.ExtraeStringDeBuffer(&parametros); // Tipo de documento al que se le va a cambiar algo.
		serie=mFg.ExtraeStringDeBuffer(&parametros); // Serie que se va a asignar a la venta.
		folio=mFg.ExtraeStringDeBuffer(&parametros); // Folio físico que se va a asignar a la venta.
		ancho=mFg.ExtraeStringDeBuffer(&parametros); // Ancho del Folio físico que se va a asignar a la venta.
		terminal=mFg.ExtraeStringDeBuffer(&parametros); // Terminal donde se manda a imprimir.

		// Si se manda un espacio en blanco significa que
		// va SIN NUMERO DE SERIE
		if (serie==" ") {
			condicion_serie="''";
		} else {
			condicion_serie.sprintf("'%s'", serie);
		}

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		// Cambia el Folio físico y la serie
		if (tipo=="0")
			instruccion.sprintf("update terminales t set t.serievta=%s, t.folvta=%s, t.anchofolvta=%s where t.terminal='%s'", condicion_serie, folio, ancho, terminal);
		if (tipo=="1")
			instruccion.sprintf("update terminales t set t.seriencred=%s, t.folncred=%s, t.anchofolncred=%s where t.terminal='%s'", condicion_serie, folio, ancho, terminal);
		if (tipo=="2")
			instruccion.sprintf("update terminales t set t.seriencar=%s, t.folncar=%s, t.anchofolncar=%s where t.terminal='%s'", condicion_serie, folio, ancho, terminal);
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
//ID_CON_VTA_CONCENTRADA_IMPRESION
void ServidorVentas::ConsultaVentaConcentradaParaImprimir(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	int i;
    char *resultado;
	double totalng, totalg, subtotalg, subtotalgiva, total;
    ArregloDetalle ArregloVenta;
    AnsiString total_enunciado;
	BufferRespuestas* resp_telefonos=NULL;
    AnsiString telefonos, telefonos_principales, tipo, lada, telefono;
	int conta_telefonos_principales;
    double totalpeso, totalvol, peso, volumen;
    double totalcantidad, cantidad;
    bool agregado_total_iva, agregado_total_iesps;
    TDate fecha;
	AnsiString cliente, forma, terminal;
    AnsiString terminal_vta, seccion_vta, sucursal_vta;
    AnsiString condicion_terminal_vta, condicion_seccion_vta, condicion_sucursal_vta;
	AnsiString instruccion;

	try{
		cliente=mFg.ExtraeStringDeBuffer(&parametros); // Clave del cliente que se va a usar para la factura
		forma=mFg.ExtraeStringDeBuffer(&parametros); // Clave de la forma para imprimir concentrados
		fecha=StrToDate(mFg.ExtraeStringDeBuffer(&parametros)); // Fecha de la que se quiere concentrar
		terminal_vta=mFg.ExtraeStringDeBuffer(&parametros); // Terminal de venta
		seccion_vta=mFg.ExtraeStringDeBuffer(&parametros); // Seccion de venta
		sucursal_vta=mFg.ExtraeStringDeBuffer(&parametros); // Sucursal de venta
		terminal=mFg.ExtraeStringDeBuffer(&parametros); // Terminal desde la que se imprime.

		if (terminal_vta==" ") {
			condicion_terminal_vta=" ";
		} else {
			condicion_terminal_vta.sprintf(" and v.terminal='%s' ",terminal_vta);
		}
		if (seccion_vta==" ") {
			condicion_seccion_vta=" ";
		} else {
			condicion_seccion_vta.sprintf(" and s.seccion='%s' ",seccion_vta);
		}
		if (sucursal_vta==" ") {
			condicion_sucursal_vta=" ";
		} else {
			condicion_sucursal_vta.sprintf(" s.sucursal='%s' ",sucursal_vta);
		}


		instruccion.sprintf("select t.tipo as tipo, t.lada as lada, t.telefono as telefono, t.extencionTel as ext \
			from telefonosclientes t where t.cliente='%s' ",
			cliente);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_telefonos);
		telefonos="";
		telefonos_principales="";
		conta_telefonos_principales=0;
		for(i=0; i<resp_telefonos->ObtieneNumRegistros(); i++){
				resp_telefonos->IrAlRegistroNumero(i);
				tipo=resp_telefonos->ObtieneDato("tipo");
				lada=resp_telefonos->ObtieneDato("lada");
				telefono=resp_telefonos->ObtieneDato("telefono");
				if (tipo=="Negocio" || tipo=="Particular") {
					if (conta_telefonos_principales<2 ) {
						telefonos_principales+=lada;
						telefonos_principales+=" ";
						telefonos_principales+=telefono;
						telefonos_principales+="  ";

						telefonos+=tipo;
						telefonos+=" ";
						telefonos+=lada;
						telefonos+=" ";
						telefonos+=telefono;
						telefonos+="  ";
					}
					conta_telefonos_principales++;
				}
		}
		telefonos+=" ";
		telefonos_principales+=" ";

		// Obtiene todos los generales (de cabecera) de la venta
		instruccion.sprintf("select c.*, \
			'CONTADO' as nomacredito, \
			'%s' as telefonos, '%s' as telprincipales, \
			@forma:='%s' as forma, '2' as tiporfc, \
			col.nombre as nomcolonia, \
			concat(loc.nombre, '  ', est.estado) as ciudadestado, \
			CONCAT(TRIM(CONCAT(TRIM(IFNULL(c.calle,'')),' ',TRIM(IFNULL(c.numext,'')),' ',TRIM(IFNULL(c.numint,'')))), ' COL.: ', col.nombre) AS direccioncol, \
			loc.localidad as cvelocalidad, loc.nombre as nomlocalidad, \
			mun.municipio as cvemunicipio, mun.nombre as nommunicipio, \
			est.estado as cveestado, est.nombre as nomestado, \
			'' as nomterminal, '' as nomcajero, \
			0 as comisionada, '' as nomvendedor, \
			'' as nomcobrador, '' as nomtermino \
			from clientes c \
			left join colonias col on c.colonia=col.colonia \
			left join localidades loc on col.localidad=loc.localidad \
			left join municipios mun on mun.municipio=loc.municipio \
			left join estados est on est.estado=mun.estado \
			where c.cliente='%s' ",
			telefonos, telefonos_principales, forma, cliente);
		resultado=Respuesta->ObtieneDirLibreBuffer();
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		BufferRespuestas resp_venta(resultado);

		// Obtiene los datos de la venta completa
		instruccion="select d.referencia, d.articulo, d.cantidad, d.costobase, ";
		instruccion+="d.porcdesc, d.tipoprec, d.precio, d.precioimp, ";
		instruccion+="a.present, p.producto, p.nombre, a.multiplo, ";
		instruccion+="d.almacen, a.factor, a.volumen, a.peso, ";
		instruccion+="d.claveimp1, i1.tipoimpu as tipoimpuesto1, i1.porcentaje as porcentajeimp1, t1.nombre as nomtipoimp1, ";
		instruccion+="d.claveimp2, i2.tipoimpu as tipoimpuesto2, i2.porcentaje as porcentajeimp2, t2.nombre as nomtipoimp2, ";
		instruccion+="d.claveimp3, i3.tipoimpu as tipoimpuesto3, i3.porcentaje as porcentajeimp3, t3.nombre as nomtipoimp3, ";
		instruccion+="d.claveimp4, i4.tipoimpu as tipoimpuesto4, i4.porcentaje as porcentajeimp4, t4.nombre as nomtipoimp4, ";
		instruccion+="d.almacen, ";
		instruccion+="cast(precioimp*cantidad as decimal(16,2)) as importe, ";
		instruccion+="cast(precio*cantidad as decimal(16,2)) as importedesg, ";
		instruccion+="@porciva:=(if(i1.tipoimpu='IVA',i1.porcentaje,0)+if(i2.tipoimpu='IVA',i2.porcentaje, 0)+if(i3.tipoimpu='IVA',i3.porcentaje, 0)+if(i4.tipoimpu='IVA',i4.porcentaje, 0)) as porciva, ";
		instruccion+="@porciesps:=(if(i1.tipoimpu='IESPS',i1.porcentaje,0)+if(i2.tipoimpu='IESPS',i2.porcentaje, 0)+if(i3.tipoimpu='IESPS',i3.porcentaje, 0)+if(i4.tipoimpu='IESPS',i4.porcentaje, 0)) as porciesps, ";
		instruccion+="cast((precioimp*cantidad)/(1+@porciva/100) as decimal(16,2)) as importeivadesg, ";
		instruccion+="((precioimp)/(1+@porciva/100)) as precioivadesg ";
		instruccion+="from ventas v  inner join  dventas d  inner join  articulos a  inner join  productos p  inner join  terminales ter  inner join  secciones sec ";
		instruccion+="left join impuestos i1 on i1.impuesto=d.claveimp1 ";
		instruccion+="left join impuestos i2 on i2.impuesto=d.claveimp2 ";
		instruccion+="left join impuestos i3 on i3.impuesto=d.claveimp3 ";
		instruccion+="left join impuestos i4 on i4.impuesto=d.claveimp4 ";
		instruccion+="left join tiposdeimpuestos t1 on t1.tipoimpu=i1.tipoimpu ";
		instruccion+="left join tiposdeimpuestos t2 on t2.tipoimpu=i2.tipoimpu ";
		instruccion+="left join tiposdeimpuestos t3 on t3.tipoimpu=i3.tipoimpu ";
		instruccion+="left join tiposdeimpuestos t4 on t4.tipoimpu=i4.tipoimpu ";
		instruccion+="where v.referencia=d.referencia and d.articulo=a.articulo and a.producto=p.producto ";
		instruccion+=condicion_terminal_vta;
		instruccion+=condicion_seccion_vta;
		instruccion+=condicion_sucursal_vta;
		resultado=Respuesta->ObtieneDirLibreBuffer();
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		BufferRespuestas resp_detalle(resultado);

		// *********************************************************
		// Calcula total de articulos, total de peso y total de volumen.
		totalcantidad=0;
		totalpeso=0;
		totalvol=0;
		for(i=0; i<resp_detalle.ObtieneNumRegistros(); i++){
				resp_detalle.IrAlRegistroNumero(i);
				cantidad=mFg.CadenaAFlotante(resp_detalle.ObtieneDato("cantidad"));
				peso=StrToFloat(resp_detalle.ObtieneDato("peso"));
				volumen=StrToFloat(resp_detalle.ObtieneDato("volumen"));
				totalcantidad+=cantidad;
				totalpeso+=cantidad*peso;
				totalvol+=cantidad*volumen;
		}

		// *********************************************************
		//  Calcula totales fiscales.
		ArregloVenta.UsoDelArreglo=FACT_VENTA;
		ArregloVenta.PosicionArreglo=SIN_POSICION;
		ArregloVenta.CargaBufferEnArregloDetalleVentas(&resp_detalle);
		ArregloVenta.AsignaModoAjusteRedondeo(1); // Usa modo de redondeo antiguo, aunque esta función ya no es usada.
		ArregloVenta.RecalculaTotalesVenta();

		// Total no grabado
		totalng=mFg.CadenaAFlotante(ArregloVenta.ArregloContenedorTotales[1][4]);
		// Total grabado  -> SIN RFC
		totalg=mFg.CadenaAFlotante(ArregloVenta.ArregloContenedorTotales[1][5]);
		for (i=6; i<=ArregloVenta.IndiceArregloTotales; i++)
			totalg+=mFg.CadenaAFlotante(ArregloVenta.ArregloContenedorTotales[1][i]);
		// Subtotal grabado (con todos los impuestos desglosados) -> CON RFC (IESPS)
		subtotalg=mFg.CadenaAFlotante(ArregloVenta.ArregloContenedorTotales[1][5]);
		// SubTotal grabado (con iva desglosado) -> CON RFC
		subtotalgiva=0;
		for (i=5; i<=ArregloVenta.IndiceArregloTotales; i++) {
			if (ArregloVenta.ArregloContenedorTotales[2][i]!="IVA")
				subtotalgiva+=mFg.CadenaAFlotante(ArregloVenta.ArregloContenedorTotales[1][i]);
		}
		// Total
		total=0;
		for (i=4; i<=ArregloVenta.IndiceArregloTotales; i++)
			total+=mFg.CadenaAFlotante(ArregloVenta.ArregloContenedorTotales[1][i]);
		// Total enunciado
		total_enunciado=mFg.GeneraEnunciadoDeCantidad(total);

		// Totales de la venta
		instruccion="select '";
		instruccion+=total_enunciado;
		instruccion+="' as venttotalenunciado, ";
		instruccion+=mFg.FormateaCantidad(totalng,2,false);
		instruccion+=" as venttotalng, ";
		instruccion+=mFg.FormateaCantidad(totalg,2,false);
		instruccion+=" as venttotalg, ";
		instruccion+=mFg.FormateaCantidad(subtotalg,2,false);
		instruccion+=" as ventsubtotalg, ";
		instruccion+=mFg.FormateaCantidad(subtotalgiva,2,false);
		instruccion+=" as ventsubtotalgiva, ";
		instruccion+=mFg.FormateaCantidad(total,2,false);
		instruccion+=" as venttotal, ";
		instruccion+=mFg.FormateaCantidad(totalcantidad,3,false);
		instruccion+=" as venttotalcant, ";
		instruccion+=mFg.FormateaCantidad(totalpeso,2,false);
		instruccion+=" as venttotalpeso, ";
		instruccion+=mFg.FormateaCantidad(totalvol,2,false);
		instruccion+=" as venttotalvol ";
		agregado_total_iva=false;
		agregado_total_iesps=false;
		for (i=6; i<=ArregloVenta.IndiceArregloTotales; i++) {
			instruccion+=", ";
			instruccion+=mFg.FormateaCantidad(ArregloVenta.ArregloContenedorTotales[1][i],2,false);
			instruccion+=" as ventimp";
			instruccion+=ArregloVenta.ArregloContenedorTotales[2][i].LowerCase();
			if (ArregloVenta.ArregloContenedorTotales[2][i].LowerCase()=="iva")
				agregado_total_iva=true;
			if (ArregloVenta.ArregloContenedorTotales[2][i].LowerCase()=="iesps")
				agregado_total_iesps=true;
		}
		// Si no se agregó un total de iva y de iesps, lo agrega con valor=0
		if (!agregado_total_iva) instruccion+=", 0.00 as ventimpiva ";
		if (!agregado_total_iesps) instruccion+=", 0.00 as ventimpiesps ";
		// Ejecuta la instrucción
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

		// Obtiene todos los datos de la forma
		instruccion.sprintf("select * from formas where forma=@forma");
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

		// Obtiene todo el detalle de la forma
		instruccion.sprintf("select * from dformas where forma=@forma");
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	} __finally {
		if(resp_telefonos!=NULL) delete resp_telefonos;
	}
}



//------------------------------------------------------------------------------
//ID_CON_CARTAPORTE_IMPRESION
void ServidorVentas::ConsultaCartaPorteParaImprimir(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*64*10];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString clave;
	int i;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[1000];
	char *resultado;
	double total;
	AnsiString total_enunciado;
	BufferRespuestas* resp_telefonos=NULL;
	AnsiString telefonos, telefonos_principales, tipo, lada, telefono;
	int conta_telefonos_principales;

	try{
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave del pago

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {

			instruccion.sprintf("SELECT t.tipo AS tipo, t.lada AS lada, t.telefono AS telefono,t.extencionTel AS ext \
				FROM cartasporte cp \
				INNER JOIN telefonosclientes t ON cp.clientepaga=t.cliente \
				WHERE cp.referencia='%s' ",clave);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_telefonos);
			telefonos="";
			telefonos_principales="";
			conta_telefonos_principales=0;
			for(i=0; i<resp_telefonos->ObtieneNumRegistros(); i++){
					resp_telefonos->IrAlRegistroNumero(i);
					tipo=resp_telefonos->ObtieneDato("tipo");
					lada=resp_telefonos->ObtieneDato("lada");
					telefono=resp_telefonos->ObtieneDato("telefono");
					if (tipo=="Negocio" || tipo=="Particular") {
						if (conta_telefonos_principales<2 ) {
							telefonos_principales+=lada;
							telefonos_principales+=" ";
							telefonos_principales+=telefono;
							telefonos_principales+="  ";

							telefonos+=tipo;
							telefonos+=" ";
							telefonos+=lada;
							telefonos+=" ";
							telefonos+=telefono;
							telefonos+="  ";
						}
						conta_telefonos_principales++;
					}
			}
			telefonos+=" ";
			telefonos_principales+=" ";

			// Obtiene todos los generales (de cabecera) del pago
			instruccion.sprintf("SELECT  cp.*, c.*, \
				'%s' AS telefonos, '%s' AS telprincipales, \
				col.nombre AS nomcolonia, \
				CONCAT(loc.nombre, '  ', est.estado) AS ciudadestado, \
				CONCAT(TRIM(CONCAT(TRIM(IFNULL(c.calle,'')),' ',TRIM(IFNULL(c.numext,'')),' ',TRIM(IFNULL(c.numint,'')))), ' COL.: ', col.nombre) AS direccioncol, \
				loc.localidad AS cvelocalidad, loc.nombre AS nomlocalidad, \
				mun.municipio AS cvemunicipio, mun.nombre AS nommunicipio, \
				est.estado AS cveestado, est.nombre AS nomestado, sec.nombre AS nomsector, \
				emp3.nombre AS nomusualta, \
				emp4.nombre AS nomusumodi \
			  FROM cartasporte cp \
				INNER JOIN  clientes c ON cp.clientepaga=c.cliente \
				LEFT JOIN cfdcartasporte cfd ON cfd.referencia=cp.referencia \
				LEFT JOIN colonias col ON c.colonia=col.colonia \
				LEFT JOIN sectores sec ON col.sector=sec.sector \
				LEFT JOIN localidades loc ON col.localidad=loc.localidad \
				LEFT JOIN municipios mun ON mun.municipio=loc.municipio \
				LEFT JOIN estados est ON est.estado=mun.estado \
				LEFT JOIN empleados emp3 ON cp.usualta=emp3.empleado \
				LEFT JOIN empleados emp4 ON cp.usumodi=emp4.empleado \
				WHERE cp.referencia='%s'",
				telefonos, telefonos_principales, clave);
			resultado=Respuesta->ObtieneDirLibreBuffer();
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
			BufferRespuestas resp_pago(resultado);

			// Total
			total=StrToFloat(resp_pago.ObtieneDato("valor"));
			// Total enunciado
			total_enunciado=mFg.GeneraEnunciadoDeCantidad(total);

			// Totales del pago
			instruccion="select '";
			instruccion+=total_enunciado;
			instruccion+="' as cartatotalenunciado, ";
			instruccion+=mFg.FormateaCantidad(total,2,false);
			instruccion+=" as cartatotal ";
			// Ejecuta la instrucción
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

		}
	} __finally {
		if(resp_telefonos!=NULL) delete resp_telefonos;
		delete buffer_sql;
	}
}

//------------------------------------------------------------------------------
//ID_GRA_FACTURAGLOBALCFD
void ServidorVentas::GrabaFacturaGlobalCfd(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	throw Exception(L"Esta función ID_GRA_FACTURAGLOBALCFD ya no es soportada");
}

//----------------------------------------------------------------------------
//ID_REINTENTA_TIMBRADO
void ServidorVentas::ReintentaTimbrado(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	ComprobanteFiscalDigital *cfd=NULL;
	cfd=new ComprobanteFiscalDigital(mServidorVioleta);
	cfd->InicializarInternet();
	AnsiString instruccion;
	AnsiString clave_documento, tipo_documento;
	BufferRespuestas* resp_config_cfd=NULL;
	BufferRespuestas* resp_generales=NULL;
	unsigned char *xmlgenerado=NULL;
	TMemoryStream *res=NULL;
	TMemoryStream *aux_mstream=NULL;
	TStringStream *aux_stream=NULL;
	AnsiString serie, folio, sucursal, compfiscal, RfcEmisor, RfcReceptor;
	AnsiString xml_base64_inicial;
	unsigned char *xml_base64_final=NULL;
	UnicodeString sello;
	int xmllongitud;
	_di_IXMLDocument xmlDoc = NewXMLDocument();
	_di_IXMLNode rootNode, rowNode;
	UTF8String xml_utf8;
	int pac_timbrador;
	UnicodeString muuid;
	AnsiString version;




	try {
		clave_documento=mFg.ExtraeStringDeBuffer(&parametros); // Clave de la venta.
		tipo_documento=mFg.ExtraeStringDeBuffer(&parametros);
		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

		if (tipo_documento=="VENT") {
			// Obtiene todos los generales (de cabecera) de la venta
			instruccion.sprintf("select cfd.sucursal, \
				cfd.compfiscal, cfd.folio as cfdfolio, cfd.serie as cfdserie, cfdxml.xmlgenerado as cfdxml, \
				cfd.version, cfd.muuid, cfd.pactimbrador, cfd.estado \
				from ventas v  \
				inner join cfd on v.referencia=cfd.referencia and cfd.tipocomprobante='VENT' \
				inner join cfdxml on cfd.compfiscal=cfdxml.compfiscal \
				where v.referencia='%s'", clave_documento);
		} else {

			if (tipo_documento=="NCRE") {
				// Obtiene parámetros de facturación
				instruccion.sprintf("select cfd.sucursal, \
					cfd.compfiscal, cfd.folio as cfdfolio, cfd.serie as cfdserie, cfdxml.xmlgenerado as cfdxml, \
					cfd.version, cfd.muuid, cfd.pactimbrador, cfd.estado \
					from notascredcli n  \
					inner join cfd on n.referencia=cfd.referencia and cfd.tipocomprobante='NCRE' \
					inner join cfdxml on cfd.compfiscal=cfdxml.compfiscal \
					where n.referencia='%s'", clave_documento);

			} else {
				if (tipo_documento=="NCAR") {
					// Obtiene parámetros de facturación
					instruccion.sprintf("select cfd.sucursal, \
						cfd.compfiscal, cfd.folio as cfdfolio, cfd.serie as cfdserie, cfdxml.xmlgenerado as cfdxml, \
						cfd.version, cfd.muuid, cfd.pactimbrador, cfd.estado \
						from notascarcli n  \
						inner join cfd on n.referencia=cfd.referencia and cfd.tipocomprobante='NCAR' \
						inner join cfdxml on cfd.compfiscal=cfdxml.compfiscal \
						where n.referencia='%s'", clave_documento);
				} else {

					if (tipo_documento=="TICK") {
						// Obtiene parámetros de facturación
						instruccion.sprintf("select cfd.sucursal, \
						cfd.compfiscal, cfd.folio as cfdfolio, cfd.serie as cfdserie, cfdxml.xmlgenerado as cfdxml, \
						cfd.version, cfd.muuid, cfd.pactimbrador, cfd.estado \
						from cfd \
						inner join cfdxml on cfd.compfiscal=cfdxml.compfiscal \
						where cfd.compfiscal='%s'", clave_documento);
					} else {
						if (tipo_documento=="PAGO") {
							// Obtiene todos los generales (de cabecera) de la venta
							instruccion.sprintf("select cfd.sucursal, \
								cfd.compfiscal, cfd.folio as cfdfolio, cfd.serie as cfdserie, cfdxml.xmlgenerado as cfdxml, \
								cfd.version, cfd.muuid, cfd.pactimbrador, cfd.estado \
								from pagoscli p \
								inner join cfd on p.pago=cfd.referencia and cfd.tipocomprobante='PAGO' \
								inner join cfdxml on cfd.compfiscal=cfdxml.compfiscal \
								where p.pago='%s'", clave_documento);
						} else {
							if (tipo_documento=="CAR2") {
								// Obtiene todos los generales (de cabecera) de la venta
								instruccion.sprintf("select cfd.sucursal, \
								cfd.compfiscal, cfd.folio as cfdfolio, cfd.serie as cfdserie, cfdxml.xmlgenerado as cfdxml, \
								cfd.version, cfd.muuid, cfd.pactimbrador, cfd.estado \
								from cartasporte20 cartas20 \
								inner join cfd on cartas20.cartaporte20=cfd.referencia and cfd.tipocomprobante='CAR2' \
								inner join cfdxml on cfd.compfiscal=cfdxml.compfiscal \
								where cartas20.cartaporte20='%s'", clave_documento);
							} else {
								throw Exception("El tipo en ReintentaTimbrado no es válido");
							}
						}

					}
				}
			}

		}

		if (!mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_generales))
			throw Exception("No se puede consultar el documento con referencia="+clave_documento);
		if (resp_generales->ObtieneNumRegistros()==0)
			throw Exception("No se encontró registro del documento");
		if (resp_generales->ObtieneDato("estado")=="0")
			throw Exception("No se permite reintentar timbrado en un documento cancelado");
		if (resp_generales->ObtieneDato("version")!="3.2" && resp_generales->ObtieneDato("version")!="3.3"  && resp_generales->ObtieneDato("version")!="4.0")
			throw Exception("Solo se pueden reintentar timbrado en CFDI versión 3.2 , 3.3 y 4.0");

		xml_base64_inicial=resp_generales->ObtieneDato("cfdxml");
		sucursal=resp_generales->ObtieneDato("sucursal");
		compfiscal=resp_generales->ObtieneDato("compfiscal");

		// Revisa si el documento está ya timbrado simplemente lo ignora ya que otro proceso pudo haberlo timbrado.
		if (resp_generales->ObtieneDato("muuid").Length()!=36) {

			xmlgenerado = cfd->ObtieneFuncOpenssl()->FromBase64((unsigned char *)xml_base64_inicial.data(), &xmllongitud);

			// Pasa el XML a un XMLDoc y parsea sello y rfc del receptor
			aux_mstream = new TMemoryStream();
			aux_mstream->WriteBuffer((unsigned char *)xmlgenerado, xmllongitud-1);
			xmlDoc->LoadFromStream(aux_mstream, xetUTF_8);
			rootNode = xmlDoc->DocumentElement;
			rowNode = rootNode;

			if (rootNode->HasAttribute("Version")) {
				version =rootNode->GetAttribute("Version");	
			} else {
				if (rootNode->HasAttribute("version")) {
					version =rootNode->GetAttribute("version");
				} else {
					throw Exception("No se encontró atributo de versión en el XML");
				}
			}

			if (version==L"3.2") {
				serie = rootNode->GetAttribute("serie");
				folio = rootNode->GetAttribute("folio");
				sello = rootNode->GetAttribute("sello");

				rowNode = rootNode->ChildNodes->FindNode("Emisor");
				RfcEmisor=rowNode->GetAttribute("rfc");

				rowNode = rootNode->ChildNodes->FindNode("Receptor");
				RfcReceptor=rowNode->GetAttribute("rfc");
			} else {
				serie = rootNode->GetAttribute("Serie");
				folio = rootNode->GetAttribute("Folio");
				sello = rootNode->GetAttribute("Sello");

				rowNode = rootNode->ChildNodes->FindNode("Emisor");
				RfcEmisor=rowNode->GetAttribute("Rfc");

				rowNode = rootNode->ChildNodes->FindNode("Receptor");
				RfcReceptor=rowNode->GetAttribute("Rfc");
			}

			// Pasa el XML a un UTF8String y parsea algunos campos que se van a usar
			aux_stream= new TStringStream(UnicodeString(L""), TEncoding::UTF8, false);
			aux_stream->WriteBuffer((unsigned char *)xmlgenerado, xmllongitud-1);
			xml_utf8=UTF8String(aux_stream->DataString);

			// Obtiene parámetros de facturación    p.versioncfdi,
			instruccion.sprintf("select \
				p.llaveprivada, p.passcert, p.numseriecert, \
				p.rfcemisor, p.nombreemisor, p.calleemisor, p.numextemisor, p.numintemisor, p.coloniaemisor, \
				p.municipioemisor, p.localidademisor, p.estadoemisor, p.referenemisor, p.cpemisor, p.incluirexp, p.calleexp, \
				p.numextexp, p.numintexp, p.coloniaexp, p.municipioexp, p.localidadexp, p.estadoexp, p.cpexp, p.referenexp, \
				p.fechaexpicert, p.fechavenccert, p.regimenfiscal, \
				p.pacseleccionado, p.certificado, p.timbradoprueba, \
				p.urledicom, p.usuarioedicom, p.passwordedicom, p.urlwfactura, p.usuariowfactura, \
				p.urlcomerciodigital, p.usuariocomerciodigital, p.passcomerciodigital \
				from parametroscfd p \
				where p.sucursal='%s'", sucursal);
			if (!mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_config_cfd))
				throw Exception("No se pudieron consultar los parámetros de facturación");
			if (resp_config_cfd->ObtieneNumRegistros()==0)
				throw Exception("No se encontraron parámetros de facturación");


			// Crea el CFD (si la configuracion así lo indica) y hace commit
			muuid=cfd->ReintentaTimbrarXML(clave_documento, serie, folio,
					xml_utf8, resp_config_cfd,
					RfcEmisor, RfcReceptor, pac_timbrador, sello, tipo_documento, compfiscal, Respuesta, MySQL);

			xml_base64_final=cfd->ObtieneFuncOpenssl()->ToBase64((unsigned char *)xml_utf8.data(), xml_utf8.Length());
			instruccion.sprintf("update cfd set \
				muuid='%s', pactimbrador=%d \
				where compfiscal='%s' ",
				AnsiString(muuid),pac_timbrador, compfiscal);
			// Hace el update en la tabla Cfd
			if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
				throw Exception("(GRAVE: Avise a sistemas) No se pudo actualizar el registro del CFDI");

			instruccion.sprintf("update cfdxml set xmlgenerado='%s' \
				where compfiscal='%s' ",
				xml_base64_final, compfiscal);
			// Hace el update en la tabla CfdXML
			if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
				throw Exception("(GRAVE: Avise a sistemas) No se pudo actualizar el registro del CFDXML");

			#ifdef _DEBUG
			try {
				// Graba el archivo del XML
				AnsiString nombrearch_xml="C:\\Temp\\CFD"+serie+folio+".xml";
				res = new TMemoryStream();
				res->WriteBuffer((unsigned char *)xml_utf8.data(), xml_utf8.Length());
				res->SaveToFile(nombrearch_xml);
			} __finally {
				if (res!=NULL) {
					delete res;
				}
			}
			#endif

		}

		instruccion.sprintf("select cfdxml.xmlgenerado as cfdxml, cfd.muuid, cfd.version, cfd.pactimbrador \
					from cfd \
					inner join cfdxml on cfd.compfiscal=cfdxml.compfiscal \
					where cfd.compfiscal=%s", compfiscal);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	} __finally {
		if (resp_config_cfd!=NULL) delete resp_config_cfd;
		if (resp_generales!=NULL) delete resp_generales;
		if (xml_base64_final!=NULL) cfd->ObtieneFuncOpenssl()->Liberar(xml_base64_final);
		if (xmlgenerado != NULL) cfd->ObtieneFuncOpenssl()->Liberar(xmlgenerado);
		if (aux_mstream != NULL) delete aux_mstream;
		if (aux_stream!=NULL) delete aux_stream;
		// Al final liberar esta
		if(cfd!=NULL) delete cfd;
	}
}


//----------------------------------------------------------------------------
//ID_REINTENTA_TIMBRADO_REG_XML
void ServidorVentas::ReintentaTimbradoRegXML(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	ComprobanteFiscalDigital *cfd=NULL;
	cfd=new ComprobanteFiscalDigital(mServidorVioleta);
	cfd->InicializarInternet();
	AnsiString instruccion;
	AnsiString clave_documento, tipo_documento, nuevoMetodo, nuevaClave, nuevosDigitos, nuevoUsoCFDI;
	BufferRespuestas* resp_config_cfd=NULL;
	BufferRespuestas* resp_generales=NULL;
	unsigned char *xmlgenerado=NULL;
	TMemoryStream *res=NULL;
	TMemoryStream *aux_mstream=NULL;
	TStringStream *aux_stream=NULL;
	AnsiString serie, folio, sucursal, compfiscal, RfcEmisor, RfcReceptor;
	AnsiString xml_base64_inicial;
	unsigned char *xml_base64_final=NULL;
	UnicodeString sello;
	int xmllongitud;
	_di_IXMLDocument xmlDoc = NewXMLDocument();
	_di_IXMLNode rootNode, rowNode;
	UTF8String xml_utf8;
	int pac_timbrador;
	UnicodeString muuid;
	AnsiString version;

	try {
		clave_documento=mFg.ExtraeStringDeBuffer(&parametros); // Clave de la venta.
		tipo_documento=mFg.ExtraeStringDeBuffer(&parametros);


		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

		if (tipo_documento=="VENT") {
			// Obtiene todos los generales (de cabecera) de la venta
			instruccion.sprintf("select cfd.sucursal, \
				cfd.compfiscal, cfd.folio as cfdfolio, cfd.serie as cfdserie, cfdxml.xmlgenerado as cfdxml, \
				cfd.version, cfd.muuid, cfd.pactimbrador, cfd.estado \
				from ventas v  \
				inner join cfd on v.referencia=cfd.referencia and cfd.tipocomprobante='VENT' \
				inner join cfdxml on cfd.compfiscal=cfdxml.compfiscal \
				where v.referencia='%s'", clave_documento);
		} else if (tipo_documento=="NCRE") {
				// Obtiene parámetros de facturación
				instruccion.sprintf("select cfd.sucursal, \
					cfd.compfiscal, cfd.folio as cfdfolio, cfd.serie as cfdserie, cfdxml.xmlgenerado as cfdxml, \
					cfd.version, cfd.muuid, cfd.pactimbrador, cfd.estado \
					from notascredcli n  \
					inner join cfd on n.referencia=cfd.referencia and cfd.tipocomprobante='NCRE' \
					inner join cfdxml on cfd.compfiscal=cfdxml.compfiscal \
					where n.referencia='%s'", clave_documento);

		} else if (tipo_documento=="NCAR") {
					// Obtiene parámetros de facturación
					instruccion.sprintf("select cfd.sucursal, \
						cfd.compfiscal, cfd.folio as cfdfolio, cfd.serie as cfdserie, cfdxml.xmlgenerado as cfdxml, \
						cfd.version, cfd.muuid, cfd.pactimbrador, cfd.estado \
						from notascarcli n  \
						inner join cfd on n.referencia=cfd.referencia and cfd.tipocomprobante='NCAR' \
						inner join cfdxml on cfd.compfiscal=cfdxml.compfiscal \
						where n.referencia='%s'", clave_documento);
		} else
			throw Exception("El tipo en ReintentaTimbrado con regeneración de xml no es válido");


		if (!mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_generales))
			throw Exception("No se puede consultar el documento con referencia="+clave_documento);
		if (resp_generales->ObtieneNumRegistros()==0)
			throw Exception("No se encontró registro del documento");
		if (resp_generales->ObtieneDato("estado")=="0")
			throw Exception("No se permite reintentar timbrado en un documento cancelado");
		if (resp_generales->ObtieneDato("version")!="3.2" && resp_generales->ObtieneDato("version")!="3.3"  && resp_generales->ObtieneDato("version")!="4.0")
			throw Exception("Solo se pueden reintentar timbrado en CFDI versión 3.2 , 3.3 y 4.0");

		xml_base64_inicial=resp_generales->ObtieneDato("cfdxml");
		sucursal=resp_generales->ObtieneDato("sucursal");
		compfiscal=resp_generales->ObtieneDato("compfiscal");

		// Revisa si el documento está ya timbrado simplemente lo ignora ya que otro proceso pudo haberlo timbrado.
		if (resp_generales->ObtieneDato("muuid").Length()!=36) {

			//Regenera el XML y lo timbra
			ComprobanteFiscalDigital *cfd=NULL;

			if (tipo_documento=="VENT") {

				cfd=new ComprobanteFiscalDigital(mServidorVioleta);
				cfd->AsignaValores("", "", "", "", " ", "");

				if (cfd->EmitirCFDI40(Respuesta, MySQL, "VENT"))
					cfd->CreaCFDIVenta40(Respuesta, MySQL, " ", "", "", true, clave_documento);

			}else if (tipo_documento=="NCRE") {

				cfd=new ComprobanteFiscalDigital(mServidorVioleta);
				cfd->AsignaValores("", "", "",""," ","");

				if (cfd->EmitirCFDI40(Respuesta, MySQL, "NCRE"))
					cfd->CreaCFDINotaCredito40(Respuesta, MySQL,0, false, true, clave_documento );
			  /*else
					throw Exception(L"La versión del CFDI tipo NCRE ya no es soportada");*/

			}else if (tipo_documento=="NCAR"){

				cfd=new ComprobanteFiscalDigital(mServidorVioleta);

				if (cfd->EmitirCFDI40(Respuesta, MySQL, "NCAR"))
					cfd->CreaCFDINotaCargo40(Respuesta, MySQL, true, clave_documento);

			}


		}

		instruccion.sprintf("select cfdxml.xmlgenerado as cfdxml, cfd.muuid, cfd.version, cfd.pactimbrador \
					from cfd \
					inner join cfdxml on cfd.compfiscal=cfdxml.compfiscal \
					where cfd.compfiscal=%s", compfiscal);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	} __finally {
		if (resp_config_cfd!=NULL) delete resp_config_cfd;
		if (resp_generales!=NULL) delete resp_generales;
		if (xml_base64_final!=NULL) cfd->ObtieneFuncOpenssl()->Liberar(xml_base64_final);
		if (xmlgenerado != NULL) cfd->ObtieneFuncOpenssl()->Liberar(xmlgenerado);
		if (aux_mstream != NULL) delete aux_mstream;
		if (aux_stream!=NULL) delete aux_stream;
		// Al final liberar esta
		if(cfd!=NULL) delete cfd;

	}
}


//----------------------------------------------------------------------------
// ID_GRA_SIMILARVXVOL
void ServidorVentas::GrabaSimilarVxvol(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{

	char *buffer_sql=new char[1024*64*10];
	char *aux_buffer_sql=buffer_sql;
	int i, num_instrucciones=0;
	AnsiString instruccion, instrucciones[1000];
	AnsiString producto, present, grupo;

	try{
		producto=mFg.ExtraeStringDeBuffer(&parametros);
		present=mFg.ExtraeStringDeBuffer(&parametros);
		grupo=mFg.ExtraeStringDeBuffer(&parametros);

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";


		// Por si ya tenia una asignación con un grupo la borra
		instruccion.sprintf("delete from dsimilarvxvol where producto='%s' and present='%s'",
			producto, present);
		instrucciones[num_instrucciones++]=instruccion;

		if (grupo==" ") {

			// Si no se especifica grupo, se crea uno nuevo.
			instruccion.sprintf("insert into similarvxvol values ()");
			instrucciones[num_instrucciones++]=instruccion;

			instrucciones[num_instrucciones++]="SET @AUXILIAR = LAST_INSERT_ID()";

			instruccion.sprintf("insert into dsimilarvxvol (idsimilar, producto, present) values (@AUXILIAR, '%s', '%s')",
				producto, present);
			instrucciones[num_instrucciones++]=instruccion;
		} else {
			// Cuando sí se especifica grupo
			instruccion.sprintf("insert into dsimilarvxvol (idsimilar, producto, present) values (%s, '%s', '%s')",
				grupo, producto, present);
			instrucciones[num_instrucciones++]=instruccion;
		}

		instrucciones[num_instrucciones++]="COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			if (grupo==" ") {
				instruccion.sprintf("select 0 as error, @AUXILIAR as folio");
			} else {
				instruccion.sprintf("select 0 as error, %s as folio", grupo);
			}
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	} __finally {
		delete buffer_sql;
	}

}

//------------------------------------------------------------------------------
//ID_CON_PEDIDO_CLI_TOTAL
void ServidorVentas::ConsultaPedidoCliTotal(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA EL PRECIO AL QUE TENTATIVAMENTE SE VA A PONER UN PEDIDO DE CLIENTE, DADO SU PRECIO DE DEFAULT
	// ESTO ES UTIL PARA VERIFICAR QUE SU LIMITE DE CREDITO LE ALCANCE ANTES DE GRABAR EL PEDIDO.
	char *buffer_sql=new char[1024*64*20];
	char *aux_buffer_sql=buffer_sql;
	int num_partidas, i;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[2000];
	AnsiString cliente, articulo, cantidad;

	try{
		cliente=mFg.ExtraeStringDeBuffer(&parametros); // Clave del pedido

		instruccion="create temporary table dpedidosventaaux (articulo varchar(9), \
			cantidad decimal(12,3), \
			precioimp decimal (16,6)) Engine = InnoDB";
		instrucciones[num_instrucciones++]=instruccion;

		// Graba las partidas en "dpedidosventaaux" con su precio
		num_partidas=StrToInt(mFg.ExtraeStringDeBuffer(&parametros)); // Obtiene el número de partidas
		for (i=0; i<num_partidas; i++) {
			articulo=mFg.ExtraeStringDeBuffer(&parametros); // articulo
			cantidad=mFg.ExtraeStringDeBuffer(&parametros); // cantidad

			// Pone el precio en variables de mysql
			instruccion.sprintf("SELECT @precioart:=pre.precio FROM clientes c \
				INNER JOIN clientesemp cet ON cet.cliente=c.cliente AND cet.idempresa=%s \
				INNER JOIN precios pre ON pre.tipoprec=cet.tipoprec \
                INNER JOIN tiposdeprecios tp ON tp.tipoprec=pre.tipoprec AND tp.idempresa=%s \
				INNER JOIN articulos a ON pre.articulo=a.articulo \
				WHERE c.cliente='%s' AND a.articulo='%s' ",
				FormServidor->ObtieneClaveEmpresa(),  FormServidor->ObtieneClaveEmpresa(),
				cliente, articulo );
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("insert into dpedidosventaaux (articulo, cantidad, precioimp) values \
				('%s', %s, @precioart)",articulo,cantidad);
			instrucciones[num_instrucciones++]=instruccion;
		}

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			instruccion.sprintf("select sum(cantidad*precioimp) as total from dpedidosventaaux");
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	} __finally {
		delete buffer_sql;
	}
}

//------------------------------------------------------------------------------
//ID_GRA_FACTURAGLOBALCFD_33
void ServidorVentas::GrabaFacturaGlobalCfd33(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*64*10];
	char *aux_buffer_sql=buffer_sql;
	BufferRespuestas* respNotasCredito = NULL;

	BufferRespuestas* resp_exite_cfd = NULL;
	int i;
	int num_instrucciones=0;
	bool reset_FechaReporte=true;
	AnsiString instruccion, instrucciones[1000];

	ComprobanteFiscalDigital *cfd=NULL;
	try {
		cfd=new ComprobanteFiscalDigital(mServidorVioleta);
		cfd->DatosGenerales.Total=StrToFloat(mFg.ExtraeStringDeBuffer(&parametros));
		cfd->DatosGenerales.FormaPago=mFg.ExtraeStringDeBuffer(&parametros);
		cfd->DatosGenerales.Cliente=mFg.ExtraeStringDeBuffer(&parametros);
		cfd->DatosGenerales.Terminal=mFg.ExtraeStringDeBuffer(&parametros);
		cfd->DatosGenerales.Usuario=mFg.ExtraeStringDeBuffer(&parametros);
		cfd->DatosGenerales.Referencia=mFg.ExtraeStringDeBuffer(&parametros);
		cfd->DatosGenerales.TipoComprobante=mFg.ExtraeStringDeBuffer(&parametros); // VENT, NCAR NCRE, TICK y otro
		cfd->DatosGenerales.Ticksubiva0=StrToFloat(mFg.ExtraeStringDeBuffer(&parametros));
		cfd->DatosGenerales.Ticksubiva0ieps=StrToFloat(mFg.ExtraeStringDeBuffer(&parametros));
		cfd->DatosGenerales.Ticksubiva16=StrToFloat(mFg.ExtraeStringDeBuffer(&parametros));
		cfd->DatosGenerales.Ticksubiva16ieps=StrToFloat(mFg.ExtraeStringDeBuffer(&parametros));

		// Pone los conceptos en un arreglo para pasarlo como parámetro
		cfd->Conceptos.Length =StrToInt(mFg.ExtraeStringDeBuffer(&parametros)); // Obtiene el número de partidas
		for (i=0; i<cfd->Conceptos.Length; i++) {
			cfd->Conceptos[i].NoIdentificacion=mFg.ExtraeStringDeBuffer(&parametros);
			cfd->Conceptos[i].ValorUnitario=mFg.CadenaAFlotante(mFg.ExtraeStringDeBuffer(&parametros));
			cfd->Conceptos[i].Cantidad=1.0;
			cfd->Conceptos[i].Unidad="";
			cfd->Conceptos[i].Descripcion="Venta";
		}

		// Pone los impuestos en un arreglo para pasarlo como parámetro
		cfd->Impuestos.Length = StrToInt(mFg.ExtraeStringDeBuffer(&parametros)); // Obtiene el número de partidas
		for (i=0; i<cfd->Impuestos.Length; i++) {
			cfd->Impuestos[i].Impuesto=mFg.ExtraeStringDeBuffer(&parametros);
			cfd->Impuestos[i].Tasa=mFg.CadenaAFlotante(mFg.ExtraeStringDeBuffer(&parametros));
			cfd->Impuestos[i].Importe=mFg.CadenaAFlotante(mFg.ExtraeStringDeBuffer(&parametros));
		}

		// Pone los impuestos en un arreglo para pasarlo como parámetro
		cfd->Impuestos33.Length = StrToInt(mFg.ExtraeStringDeBuffer(&parametros)); // Obtiene el número de partidas
		for (i=0; i<cfd->Impuestos33.Length; i++) {
			cfd->Impuestos33[i].Referencia=mFg.ExtraeStringDeBuffer(&parametros);
			cfd->Impuestos33[i].TasaIva=mFg.CadenaAFlotante(mFg.ExtraeStringDeBuffer(&parametros));
			cfd->Impuestos33[i].ImporteIva=mFg.CadenaAFlotante(mFg.ExtraeStringDeBuffer(&parametros));
			cfd->Impuestos33[i].BaseIva=mFg.CadenaAFlotante(mFg.ExtraeStringDeBuffer(&parametros));
			cfd->Impuestos33[i].TasaIeps=mFg.CadenaAFlotante(mFg.ExtraeStringDeBuffer(&parametros));
			cfd->Impuestos33[i].ImporteIeps=mFg.CadenaAFlotante(mFg.ExtraeStringDeBuffer(&parametros));
			cfd->Impuestos33[i].BaseIeps=mFg.CadenaAFlotante(mFg.ExtraeStringDeBuffer(&parametros));

		}


		// Pone los impuestos en un arreglo para pasarlo como parámetro
		/*cfd->Impuestos33.Length = StrToInt(mFg.ExtraeStringDeBuffer(&parametros)); // Obtiene el número de partidas
		for (i=0; i<cfd->Impuestos33.Length; i++) {
			cfd->Impuestos33[i].TasaIva=mFg.CadenaAFlotante(mFg.ExtraeStringDeBuffer(&parametros));
			cfd->Impuestos33[i].ImporteIva=mFg.CadenaAFlotante(mFg.ExtraeStringDeBuffer(&parametros));
			cfd->Impuestos33[i].BaseIva=mFg.CadenaAFlotante(mFg.ExtraeStringDeBuffer(&parametros));
			cfd->Impuestos33[i].TasaIeps=mFg.CadenaAFlotante(mFg.ExtraeStringDeBuffer(&parametros));
			cfd->Impuestos33[i].ImporteIeps=mFg.CadenaAFlotante(mFg.ExtraeStringDeBuffer(&parametros));
			cfd->Impuestos33[i].BaseIeps=mFg.CadenaAFlotante(mFg.ExtraeStringDeBuffer(&parametros));

		}*/

		//Verifica que el día no tenga factura acumuladora
		instruccion.sprintf("SELECT cfd.* FROM cfd \
			INNER JOIN terminales t on t.terminal='%s' \
			INNER JOIN secciones sec ON sec.seccion=t.seccion \
			WHERE referencia='%s' AND tipocomprobante='TICK' AND estado=1 \
			and cfd.sucursal=sec.sucursal ", cfd->DatosGenerales.Terminal, cfd->DatosGenerales.Referencia );
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_exite_cfd);
		if(resp_exite_cfd->ObtieneNumRegistros()>=1)
			throw Exception("¡ALERTA!\n\nLa factura acumuladora del día ya se encuentra registrada, para poder visualizar la factura acumuladora, se requiere dar click en 'Mostrar Reporte'");

		if(cfd->DatosGenerales.Referencia == mFechaReporteGlobal )
		{
			reset_FechaReporte=false;
			throw Exception("¡ALERTA!\n\nLa creación del CFD está en proceso, favor de esperar tres minutos y después dar click en 'Mostrar Reporte'");
		}


		mFechaReporteGlobal = cfd->DatosGenerales.Referencia;

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			// Crea el CFD (si la configuracion así lo indica) y hace commit
			if (cfd->EmitirCFDI40(Respuesta, MySQL, "TICK"))
				cfd->CreaCFDIFacturaGlobal40(Respuesta, MySQL);
			else
				cfd->CreaCFDIFacturaGlobal33(Respuesta, MySQL);
				//throw Exception(L"La versión del CFDI tipo TICK ya no es soportada");

			instruccion.sprintf("select 0 as error, cfd.compfiscal FROM cfd where cfd.sucursal=@sucursal and cfd.serie=@serie and cfd.folio=@folioactual");

			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);


			// -----------------------------------------------------------------------------
			// CREAREMOS LAS NOTAS DE CREDITO CORRESPONDIENTES A LAS FACTURAS CREADAS EN EL DIA
			// -----------------------------------------------------------------------------

		AnsiString referencia_g = "";
		if(cfd->facturaglobaltimbrada){
		instruccion.sprintf("SELECT n.referencia,v.valor, s.sucursal FROM notascredcli n  \
						LEFT JOIN ventas v ON  v.referencia = n.venta                 \
						INNER JOIN cfdiweb w ON w.refticket = n.venta AND w.activo = 1     \
						INNER JOIN terminales t ON v.terminal = t.terminal                   \
						INNER JOIN secciones s ON s.seccion = t.seccion                                  \
						LEFT JOIN cfd c ON c.referencia =  n.referencia  AND c.estado = 1 AND c.tipocomprobante = 'NCRE'       \
						WHERE  c.referencia IS NULL AND n.fechaalta = CURDATE() AND n.generarxfactgl=1     \
						AND s.sucursal = (SELECT s.sucursal FROM  terminales t INNER JOIN secciones s ON s.seccion = t.seccion   \
						WHERE t.terminal ='%s' LIMIT 1) \
						GROUP BY n.referencia ", cfd->DatosGenerales.Terminal);

		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,
			instruccion.c_str(), respNotasCredito);

		AnsiString errorB=0;
		AnsiString errorMensaje = "prueba";
		int total =   respNotasCredito->ObtieneNumRegistros();

		for (i = 0; i < respNotasCredito->ObtieneNumRegistros(); i++) {
			respNotasCredito->IrAlRegistroNumero(i);
			referencia_g = respNotasCredito->ObtieneDato("referencia");
			char *buffer_nota_cred=new char[1024*64*10];
			char *aux_buffer_nota_cred=buffer_nota_cred;

			try{
					try{
						aux_buffer_nota_cred=mFg.AgregaStringABuffer(referencia_g, aux_buffer_nota_cred);
						aux_buffer_nota_cred=mFg.AgregaStringABuffer("1", aux_buffer_nota_cred); // Pasamos el 1 que es de factura global
						aux_buffer_nota_cred=mFg.AgregaStringABuffer(cfd->DatosGenerales.Terminal, aux_buffer_nota_cred);// Terminal
						aux_buffer_nota_cred=mFg.AgregaStringABuffer(cfd->DatosGenerales.Usuario, aux_buffer_nota_cred); // Cliente

						GrabaCfdiXmlWeb(Respuesta, MySQL, buffer_nota_cred);
						}
						catch(Exception &e) {
						if (e.Message=="1" || e.Message=="2" || e.Message=="3" || e.Message=="4")
						errorB = "1" + e.Message;
						else{
						errorB = "20";
						errorMensaje= e.Message;
					}
					}

				}__finally {
						delete buffer_nota_cred;
				}
		 }
		}
        }



	} __finally {
		if (respNotasCredito != NULL) delete respNotasCredito;
		if(cfd!=NULL) delete cfd;
		if(resp_exite_cfd!=NULL) delete resp_exite_cfd;
		delete buffer_sql;
		if(reset_FechaReporte)	mFechaReporteGlobal="";
	}
}
//------------------------------------------------------------------------------
//ID_EXPORTA_VARIOS_PED_CLI
void ServidorVentas::ExportaPedidosCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA PEDIDOS DE CLIENTES
	AnsiString instruccion;
	AnsiString clave, menos_devoluciones, formulario;
	AnsiString ordenMultiplo;
	AnsiString leftjoin_ventadirent=" ", leftjoin_col=" ", campo_direntrega=" ";
	TStringList* mListaReferencias = new TStringList();

	try {
		clave=mFg.ExtraeStringDeBuffer(&parametros);
		ordenMultiplo=mFg.ExtraeStringDeBuffer(&parametros);
		formulario=mFg.ExtraeStringDeBuffer(&parametros);
		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);


		mListaReferencias->Delimiter = '|';
		mListaReferencias->DelimitedText = clave;
		//validacion de direccion de entrega
		if(mListaReferencias->Count > 1)
		{
			leftjoin_ventadirent.sprintf(" ");
			leftjoin_col.sprintf(" ");
			//campo_direntrega.sprintf(" ");
			campo_direntrega.sprintf(", ' ' as dircalle, ' ' as dirnumex ,\
				' ' as dirnumin, ' ' as dircol, ' ' as dirnomcolonia,' ' as dircp, ' ' as dirreferencia ");
		} else{
			leftjoin_ventadirent.sprintf("LEFT join pedidosdirent pde on pde.referencia=p.referencia and pde.cliente=p.cliente ");
			leftjoin_col.sprintf("LEFT JOIN colonias co ON pde.colonia=co.colonia ");
			campo_direntrega.sprintf(", ifnull(pde.calle,' ') as dircalle, ifnull(pde.numext, ' ') as dirnumex ,\
									ifnull(pde.numint,' ') as dirnumin, ifnull(co.colonia,' ') as dircol, co.nombre as dirnomcolonia,  \
									ifnull(pde.cp,' ') as dircp, ifnull(pde.referenciadom,' ') as dirreferencia,  IFNULL(co.sector,' ') as dirsect ");
		}
		// fin de validacion de direccion de enttrga
		if (formulario == "ventas") {

			// Obtiene todos los generales (de cabecera) del pedido
			instruccion="select sum(p.valor) total, p.*, p.cotizacion as tipodoc, pm.mensaje, ";
			instruccion+="col.nombre as nomcolonia, loc.nombre as nomlocalidad, ";
			instruccion+="emp1.nombre as nomcajero, emp2.nombre as nomvendedor, ";
			instruccion+="emp3.nombre as nomusualta, concat(emp3.nombre,' ',emp3.appat) as nomusualta, ";
			instruccion+="concat(emp4.nombre,' ',emp4.appat) as nomchofer, ifnull(kt.nombre,'') as nombrekit, IFNULL(kt.desglosado,1) as desglosadokit, emb.concartaporte ";
			instruccion+=campo_direntrega;
			instruccion+="from pedidosventa p  inner join  clientes c ";
			instruccion+="left join colonias col on c.colonia=col.colonia ";
			instruccion+="left join localidades loc on col.localidad=loc.localidad ";
			instruccion+="left join empleados emp1 on p.usumodi=emp1.empleado ";
			instruccion+="left join empleados emp2 on p.vendedor=emp2.empleado ";
			instruccion+="left join embarques emb on emb.embarque=p.embarque ";
			instruccion+="left join empleados emp3 on p.usualta=emp3.empleado ";
			instruccion+="left join empleados emp4 on emb.chofer=emp4.empleado ";
			instruccion+="left join kits kt on p.kit=kt.kit ";
			instruccion+="left join pedidosmensajes pm on pm.referencia=p.referencia ";
			instruccion+=leftjoin_ventadirent;
			instruccion+=leftjoin_col;
			instruccion+="where p.referencia in ('";
			instruccion+=mListaReferencias->Strings[0];
			if(mListaReferencias->Count > 1)
				instruccion+="',";
			else
				instruccion+="'";

			for (int i = 1; i < mListaReferencias->Count; i++) {
				instruccion+=" '"+mListaReferencias->Strings[i]+"'";
				if( (i+1) == mListaReferencias->Count )
					instruccion+=" ";
				else
					instruccion+=",";
			}
			instruccion+=") and p.cliente=c.cliente";

			/*instruccion+="where (p.referencia='";
			instruccion+=mListaReferencias->Strings[0];
			instruccion+="'";
			for (int i = 1; i < mListaReferencias->Count; i++) {
				instruccion+=" or p.referencia='"+mListaReferencias->Strings[i]+"'";
			}
			instruccion+=") and p.cliente=c.cliente"; */
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Obtiene todos los datos del cliente del pedido
			instruccion.sprintf("select cli.* from clientes cli, pedidosventa p where p.referencia='%s' and p.cliente=cli.cliente", AnsiString(mListaReferencias->Strings[0]));
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Los pedidos siempre permiten modificarse (claro que depende si el usuario puede modificar)
			instruccion.sprintf("select 1 as modificar");
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Obtiene los datos detallados del pedido
			instruccion="select d.referencia, d.articulo, sum(d.cantidad) cantidad, d.costobase, ";
			instruccion+="d.porcdesc, d.precio, d.precioimp, a.factor, ";
			instruccion+="a.present, p.producto, p.nombre, a.multiplo, a.porccomi, ";
			instruccion+="d.claveimp1, i1.tipoimpu as tipoimpuesto1, i1.porcentaje as porcentajeimp1, t1.nombre as nomtipoimp1, ";
			instruccion+="d.claveimp2, i2.tipoimpu as tipoimpuesto2, i2.porcentaje as porcentajeimp2, t2.nombre as nomtipoimp2, ";
			instruccion+="d.claveimp3, i3.tipoimpu as tipoimpuesto3, i3.porcentaje as porcentajeimp3, t3.nombre as nomtipoimp3, ";
			instruccion+="d.claveimp4, i4.tipoimpu as tipoimpuesto4, i4.porcentaje as porcentajeimp4, t4.nombre as nomtipoimp4 ";
			if(ordenMultiplo=="1")
				instruccion+=",IF(a.factor=1,1,2) ORDEN,p.clasif1  "; //Agrupacion para ordenar por multiplo
			instruccion+=",IFNULL(dpk.kit,'') as kit ";
			instruccion+="from dpedidosventa d  inner join  articulos a  inner join  productos p ";
			instruccion+="left join impuestos i1 on i1.impuesto=d.claveimp1 ";
			instruccion+="left join impuestos i2 on i2.impuesto=d.claveimp2 ";
			instruccion+="left join impuestos i3 on i3.impuesto=d.claveimp3 ";
			instruccion+="left join impuestos i4 on i4.impuesto=d.claveimp4 ";
			instruccion+="left join tiposdeimpuestos t1 on t1.tipoimpu=i1.tipoimpu ";
			instruccion+="left join tiposdeimpuestos t2 on t2.tipoimpu=i2.tipoimpu ";
			instruccion+="left join tiposdeimpuestos t3 on t3.tipoimpu=i3.tipoimpu ";
			instruccion+="left join tiposdeimpuestos t4 on t4.tipoimpu=i4.tipoimpu ";
			instruccion+="left join dpedidoskits dpk on dpk.pedido=d.referencia and dpk.articulo=d.articulo ";
			instruccion+="where d.referencia in ('";
			instruccion+=mListaReferencias->Strings[0];
			if(mListaReferencias->Count > 1)
				instruccion+="',";
			else
				instruccion+="'";

			for (int i = 1; i < mListaReferencias->Count; i++) {
				instruccion+=" '"+mListaReferencias->Strings[i]+"'";
				if( (i+1) == mListaReferencias->Count )
					instruccion+=" ";
				else
					instruccion+=",";
			}
			/*instruccion+="where (d.referencia='";
			instruccion+=mListaReferencias->Strings[0];
			instruccion+="'";
			for (int x = 1; x < mListaReferencias->Count; x++) {
				instruccion+=" or d.referencia='"+mListaReferencias->Strings[x]+"'";
			}*/
			instruccion+=") and d.articulo=a.articulo and a.producto=p.producto ";
			instruccion+=" group by p.nombre, a.present, a.multiplo ";

			if (ordenMultiplo=="2")
				instruccion+="ORDER BY d.referencia, p.nombre, a.present, a.multiplo   ";
			else if(ordenMultiplo=="1")
				instruccion+="ORDER BY ORDEN DESC,IF(ORDEN='1',p.clasif1,left(a.multiplo,3)),p.nombre, a.present, a.multiplo  ";
			else
				instruccion+=" order by d.id, p.nombre, a.present, a.multiplo ";


			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);


			//validar que tenga una direccion de entrega
			instruccion.sprintf("select * from pedidosventa p where p.referencia='%s' \
						and p.referencia not in (select referencia from pedidosdirent)",AnsiString(mListaReferencias->Strings[0]));
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

            //Obtiene los detalles de los kits contenidos en el/los pedido/s.
			instruccion.sprintf("select pk.*,k.nombre from pedidoskits pk \
						 inner join kits k on pk.kit=k.kit where pk.pedido='%s'",AnsiString(mListaReferencias->Strings[0]));
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);


		}else{
			// Obtiene los datos detallados del pedido
			instruccion="select d.referencia, d.articulo, sum(d.cantidad) cantidad, d.costobase, ";
			instruccion+="d.porcdesc, d.precio, d.precioimp, ";
			instruccion+="a.present, p.producto, p.nombre, a.multiplo, a.porccomi, ";
			instruccion+="d.claveimp1, i1.tipoimpu as tipoimpuesto1, i1.porcentaje as porcentajeimp1, t1.nombre as nomtipoimp1, ";
			instruccion+="d.claveimp2, i2.tipoimpu as tipoimpuesto2, i2.porcentaje as porcentajeimp2, t2.nombre as nomtipoimp2, ";
			instruccion+="d.claveimp3, i3.tipoimpu as tipoimpuesto3, i3.porcentaje as porcentajeimp3, t3.nombre as nomtipoimp3, ";
			instruccion+="d.claveimp4, i4.tipoimpu as tipoimpuesto4, i4.porcentaje as porcentajeimp4, t4.nombre as nomtipoimp4 ";
			if(ordenMultiplo=="1")
				instruccion+=",IF(a.factor=1,1,2) ORDEN,p.clasif1  "; //Agrupacion para ordenar por multiplo
			instruccion+=",IFNULL(dpk.kit,'') as kit ";
			instruccion+="from dpedidosventa d  inner join  articulos a  inner join  productos p ";
			instruccion+="left join impuestos i1 on i1.impuesto=d.claveimp1 ";
			instruccion+="left join impuestos i2 on i2.impuesto=d.claveimp2 ";
			instruccion+="left join impuestos i3 on i3.impuesto=d.claveimp3 ";
			instruccion+="left join impuestos i4 on i4.impuesto=d.claveimp4 ";
			instruccion+="left join tiposdeimpuestos t1 on t1.tipoimpu=i1.tipoimpu ";
			instruccion+="left join tiposdeimpuestos t2 on t2.tipoimpu=i2.tipoimpu ";
			instruccion+="left join tiposdeimpuestos t3 on t3.tipoimpu=i3.tipoimpu ";
			instruccion+="left join tiposdeimpuestos t4 on t4.tipoimpu=i4.tipoimpu ";
			instruccion+="left join dpedidoskits dpk on dpk.pedido=d.referencia and dpk.articulo=d.articulo ";
			instruccion+="where d.referencia in ('";
			instruccion+=mListaReferencias->Strings[0];
			if(mListaReferencias->Count > 1)
				instruccion+="',";
			else
				instruccion+="'";

			for (int i = 1; i < mListaReferencias->Count; i++) {
				instruccion+=" '"+mListaReferencias->Strings[i]+"'";
				if( (i+1) == mListaReferencias->Count )
					instruccion+=" ";
				else
					instruccion+=",";
			}
			/*instruccion+="where (d.referencia='";
			instruccion+=mListaReferencias->Strings[0];
			instruccion+="'";
			for (int x = 1; x < mListaReferencias->Count; x++) {
				instruccion+=" or d.referencia='"+mListaReferencias->Strings[x]+"'";
			}*/
			instruccion+=") and d.articulo=a.articulo and a.producto=p.producto ";
			instruccion+=" group by p.nombre, a.present, a.multiplo ";

			if (ordenMultiplo=="2")
				instruccion+="ORDER BY d.referencia, p.nombre, a.present, a.multiplo   ";
			else if(ordenMultiplo=="1")
				instruccion+="ORDER BY ORDEN DESC,IF(ORDEN='1',p.clasif1,left(a.multiplo,3)),p.nombre, a.present, a.multiplo  ";
			else
				instruccion+=" order by d.id, p.nombre, a.present, a.multiplo ";


			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);


			//validar que tenga una direccion de entrega
			instruccion.sprintf("select * from pedidosventa p where p.referencia='%s' \
						and p.referencia not in (select referencia from pedidosdirent)",AnsiString(mListaReferencias->Strings[0]));
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

            //Obtiene los detalles de los kits contenidos en el/los pedido/s.
			instruccion.sprintf("select pk.*,k.nombre from pedidoskits pk \
						 inner join kits k on pk.kit=k.kit where pk.pedido='%s'",AnsiString(mListaReferencias->Strings[0]));
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

		}
	}__finally{
		delete 	mListaReferencias;
	}
}
//------------------------------------------------------------------------------

//ID_CON_PAGO_CLI_IMPRESION
void ServidorVentas::ConsultaPagoCliParaImprimir(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*64*10];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString clave;
	int i;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[1000];
	char *resultado;
	double total;
	AnsiString total_enunciado;
	BufferRespuestas* resp_telefonos=NULL;
	AnsiString telefonos, telefonos_principales, tipo, lada, telefono;
	int conta_telefonos_principales;

	try{
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave del pago

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {

			instruccion.sprintf("select t.tipo as tipo, t.lada as lada, t.telefono as telefono,t.extencionTel as ext \
				from pagoscli pag \
				inner join telefonosclientes t ON pag.cliente=t.cliente \
				where pag.pago='%s'",clave);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_telefonos);
			telefonos="";
			telefonos_principales="";
			conta_telefonos_principales=0;
			for(i=0; i<resp_telefonos->ObtieneNumRegistros(); i++){
					resp_telefonos->IrAlRegistroNumero(i);
					tipo=resp_telefonos->ObtieneDato("tipo");
					lada=resp_telefonos->ObtieneDato("lada");
					telefono=resp_telefonos->ObtieneDato("telefono");
					if (tipo=="Negocio" || tipo=="Particular") {
						if (conta_telefonos_principales<2 ) {
							telefonos_principales+=lada;
							telefonos_principales+=" ";
							telefonos_principales+=telefono;
							telefonos_principales+="  ";

							telefonos+=tipo;
							telefonos+=" ";
							telefonos+=lada;
							telefonos+=" ";
							telefonos+=telefono;
							telefonos+="  ";
						}
						conta_telefonos_principales++;
					}
			}
			telefonos+=" ";
			telefonos_principales+=" ";

			// Obtiene todos los generales (de cabecera) del pago
			instruccion.sprintf("select c.*, pag.*, pag.fecha as fechapag, \
				cfd.seriefolio as foliocfdpag, \
				'%s' as telefonos, '%s' as telprincipales, \
				col.nombre as nomcolonia, \
				concat(loc.nombre, '  ', est.estado) as ciudadestado, \
				CONCAT(TRIM(CONCAT(TRIM(IFNULL(c.calle,'')),' ',TRIM(IFNULL(c.numext,'')),' ',TRIM(IFNULL(c.numint,'')))), ' COL.: ', col.nombre) AS direccioncol, \
				loc.localidad as cvelocalidad, loc.nombre as nomlocalidad, \
				mun.municipio as cvemunicipio, mun.nombre as nommunicipio, \
				est.estado as cveestado, est.nombre as nomestado, sec.nombre AS nomsector,\
				emp3.nombre as nomusualta, \
				emp4.nombre as nomusumodi \
			  from pagoscli pag \
				inner join  clientes c ON pag.cliente=c.cliente \
				left join cfd on pag.pago=cfd.referencia and cfd.tipocomprobante='PAGO' \
				left join colonias col on c.colonia=col.colonia \
				LEFT JOIN sectores sec ON col.sector=sec.sector \
				left join localidades loc on col.localidad=loc.localidad \
				left join municipios mun on mun.municipio=loc.municipio \
				left join estados est on est.estado=mun.estado \
				left join empleados emp3 on pag.usualta=emp3.empleado \
				left join empleados emp4 on pag.usumodi=emp4.empleado \
				where pag.pago='%s' ",
				telefonos, telefonos_principales, clave);
			resultado=Respuesta->ObtieneDirLibreBuffer();
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
			BufferRespuestas resp_pago(resultado);

			// Total
			total=StrToFloat(resp_pago.ObtieneDato("valor"));
			// Total enunciado
			total_enunciado=mFg.GeneraEnunciadoDeCantidad(total);

			// Totales del pago
			instruccion="select '";
			instruccion+=total_enunciado;
			instruccion+="' as pagtotalenunciado, ";
			instruccion+=mFg.FormateaCantidad(total,2,false);
			instruccion+=" as pagtotal ";
			// Ejecuta la instrucción
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

		}
	} __finally {
		if(resp_telefonos!=NULL) delete resp_telefonos;
		delete buffer_sql;
	}
}
//------------------------------------------------------------------------------

//ID_CON_DIRECCION_ENTREGA
void ServidorVentas::ConsultaDireccionEntrega(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*32];
	AnsiString instruccion;
	AnsiString cliente,tarea,tipo,referencia;


	try{
		cliente=mFg.ExtraeStringDeBuffer(&parametros);      // Clave del cliente
		tarea=mFg.ExtraeStringDeBuffer(&parametros);        //1.- direccionentrega 2.-ventas/pedidos
		tipo=mFg.ExtraeStringDeBuffer(&parametros);          // Indicador para saber si se abre desde ventas/pedidos
		referencia=mFg.ExtraeStringDeBuffer(&parametros);    //obtener la referencia del movimiento pedidos/venta

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

		if(tarea=="1")
		{   // si esta peticion viene desde el formulario de direccion de entregas
			instruccion.sprintf(" (SELECT  0 AS iddireccion, cli.calle, cli.numext,cli.numint,col.nombre,cli.cp, \
					l.nombre AS localidad, m.nombre AS municipio,e.nombre AS estado,'' AS referenciadomic, \
					'' AS ubicacion , '2' AS principal, col.colonia AS cveColonia, \
					l.localidad AS cveLocalidad, m.municipio AS cveMunicipio, e.estado AS cveEstado, \
					'' AS latitud, '' AS longitud, '' AS sector \
					FROM clientes cli \
					LEFT JOIN colonias col ON col.colonia=cli.colonia \
					LEFT JOIN localidades AS l ON l.localidad=col.localidad  \
					LEFT JOIN municipios AS m ON m.municipio=l.municipio   \
					LEFT JOIN estados AS e ON e.estado=m.estado  \
					WHERE cli.cliente ='%s') \
					UNION \
					(SELECT d.iddireccion, d.calle, d.numext, d.numint, c.nombre AS colonia, d.cp, \
					l.nombre AS localidad, m.nombre AS municipio,e.nombre AS estado,d.referenciadomic, \
					CONCAT(X(d.ubicaciongis) ,',', Y(d.ubicaciongis)) AS ubicacion , \
					d.dafault AS principal, c.colonia AS cveColonia, \
					l.localidad AS cveLocalidad, m.municipio AS cveMunicipio, e.estado AS cveEstado, \
					X(d.ubicaciongis) AS latitud, Y(d.ubicaciongis) AS longitud, c.sector \
					FROM direccionesentregaclientes AS d  \
					LEFT JOIN colonias AS c ON c.colonia=d.colonia  \
					LEFT JOIN localidades AS l ON l.localidad=c.localidad \
					LEFT JOIN municipios AS m ON m.municipio=l.municipio  \
					LEFT JOIN estados AS e ON e.estado=m.estado  \
					WHERE d.cliente ='%s')",cliente,cliente);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}else if(tarea=="2"){
			//aca es necesario saber si se hará peticion desde ventas o pedidos, ya que cada una cuenta con una referencia
			// a las tablas padres correspondientes
			// en la condicion se puso vde.referencia<>'', por la razon de que el campo nomCol no sera vacio, siempre trae valor
            // y como resgistros anteriores estan vacios, no se requiere mostrar solo la colonia
			 if(tipo=="Venta"){

				instruccion.sprintf("SELECT vde.*, col.nombre as nomCol , col.sector AS nomSec \
						FROM ventas v \
						INNER JOIN clientes c on v.cliente=c.cliente \
						LEFT JOIN ventadirent vde ON vde.referencia=v.referencia AND vde.cliente=v.cliente \
						LEFT JOIN colonias col ON vde.colonia=col.colonia \
						LEFT JOIN localidades loc ON col.localidad=loc.localidad \
						WHERE v.referencia='%s' AND v.cliente='%s' and vde.referencia<>''",referencia,cliente);
				mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
			 }
			 if(tipo=="Pedido"){
				instruccion.sprintf("SELECT pde.*, col.nombre as nomCol, col.sector AS nomSec  \
						FROM pedidosventa pv  \
						INNER JOIN clientes c on pv.cliente=c.cliente  \
						LEFT JOIN pedidosdirent pde ON pde.referencia=pv.referencia AND pde.cliente=pv.cliente \
                        LEFT JOIN colonias col ON pde.colonia=col.colonia \
						LEFT JOIN localidades loc ON col.localidad=loc.localidad \
						WHERE pv.referencia='%s' AND pv.cliente='%s' AND pde.referencia<>'' ",referencia,cliente);
				mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
			 }

		}

	} __finally {
		delete buffer_sql;
	}
}
//------------------------------------------------------------------------------
//ID_MODIF_FECHA_NOTA
void ServidorVentas::ModificaFechaNota(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{

	char *buffer_sql=new char[1024*32];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instrucciones[10], instruccion;
	int num_instrucciones=0,i=0;
	AnsiString referencia , fecha_mod , hora_mod ;

	try{

		referencia = mFg.ExtraeStringDeBuffer(&parametros);
		fecha_mod  = mFg.ExtraeStringDeBuffer(&parametros);
		hora_mod   = mFg.ExtraeStringDeBuffer(&parametros);

		// UPDATE transxcob SET fechaalta='2018-01-01' , horaalta='12:12:12',fechaapl='2018-01-01',fechamodi='2018-01-01' where notacar="S1000004880";
		// UPDATE notascarcli SET fechaalta='2018-01-01' , horaalta='12:12:12',fechamodi='2018-01-01',horamodi='12:12:12',fechanot='2018-01-01' where  referencia="S1000004880";

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		// Actualizar fecha y hora relacionadas a la tabla     transxcob
		instruccion.sprintf("UPDATE transxcob SET fechaalta='%s' , horaalta='%s',fechaapl='%s',fechamodi='%s', horamodi='%s' where notacar='%s' ",fecha_mod,hora_mod,fecha_mod,fecha_mod,hora_mod,referencia);
		instrucciones[num_instrucciones++]=instruccion;

		// Actualizar fecha y hora relacionadas a la tabla     notascarcli
		instruccion.sprintf("UPDATE notascarcli SET fechaalta='%s' , horaalta='%s',fechamodi='%s',horamodi='%s',fechanot='%s' where  referencia='%s' ", fecha_mod,hora_mod,fecha_mod,hora_mod,fecha_mod,referencia);
		instrucciones[num_instrucciones++]=instruccion;

		instrucciones[num_instrucciones++]="COMMIT";


		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);


		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {

		}


	} __finally {
		delete buffer_sql;
	}
}
//------------------------------------------------------------------------------
//ID_EMBARQUE_ACTUALIZAR_UBICACION_CLIENTE
void ServidorVentas::ActualizaEmbarqueCliente(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*5];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instruccion, instrucciones[3];
	int num_instrucciones=0;
	int i;
	AnsiString cliente_mod;

	try{
		cliente_mod=mFg.ExtraeStringDeBuffer(&parametros);

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		// Actualizar datos del cliente en la tabla  embarquesruta
		instruccion="UPDATE embarquesruta er  INNER JOIN  clientes cl   SET er.ubicaciongis = cl.ubicaciongis WHERE er.cliente = cl.cliente  AND cl.cliente='"+cliente_mod+"' ";
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
//ID_BITACORA_CONTPAQ_SDK
void ServidorVentas::BitacoraCONTPAQ(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{

	char *buffer_sql=new char[1024*32];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instrucciones[10], instruccion;
	int num_instrucciones=0,i=0;
	AnsiString ClvSucursal_poliza,ClvUsuario,nPoliza,GuidPoliza,tipo_poliza,tipo_folio,folio,muuid,valor,resumen ;

	try{

		ClvSucursal_poliza = mFg.ExtraeStringDeBuffer(&parametros);
		ClvUsuario  = mFg.ExtraeStringDeBuffer(&parametros);
		nPoliza   = mFg.ExtraeStringDeBuffer(&parametros);
		GuidPoliza   = mFg.ExtraeStringDeBuffer(&parametros);
		tipo_poliza   = mFg.ExtraeStringDeBuffer(&parametros);
		folio   = mFg.ExtraeStringDeBuffer(&parametros);
        tipo_folio   = mFg.ExtraeStringDeBuffer(&parametros);
		muuid   = mFg.ExtraeStringDeBuffer(&parametros);
		valor   = mFg.ExtraeStringDeBuffer(&parametros);
		resumen   = mFg.ExtraeStringDeBuffer(&parametros);

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		instruccion.sprintf("INSERT INTO bitacoracontpaqi VALUES (NULL, '%s','%s',NOW(),'%s','%s','%s','%s','%s','%s','%s','%s') ",ClvSucursal_poliza,ClvUsuario,nPoliza,GuidPoliza,tipo_poliza,folio,tipo_folio,muuid,valor,resumen);
		instrucciones[num_instrucciones++]=instruccion;

		instrucciones[num_instrucciones++]="COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);

		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {

		}


	} __finally {
		delete buffer_sql;
	}
}
//------------------------------------------------------------------------------
//ID_OBTENER_CANT
void ServidorVentas::ObtenerCantAcumulada(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
	AnsiString ClvTerminal;

	ClvTerminal  = mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
	instruccion.sprintf("SELECT \
	SUM(t.cantidad) AS cant_acum \
	FROM ( \
			( \
				SELECT dvfp.valor AS cantidad \
				FROM ventas v \
				INNER JOIN dventasfpago dvfp ON dvfp.referencia = v.referencia \
                LEFT JOIN cfdiweb w ON w.factgenerada=v.referencia \
				WHERE v.terminal = '%s' \
				AND v.fechavta = CURDATE() \
				AND v.cancelado = 0 \
				AND dvfp.formapag = 'FCONEF' \
				AND w.factgenerada IS NULL  \
			) \
			UNION ALL \
			( \
				SELECT (SUM(m.cantretirado)) AS cantidad \
				FROM movsefectivocaja m \
				WHERE m.terminal = '%s' \
				AND m.fechaalta = CURDATE() \
			) \
		 ) t ",
	ClvTerminal,
	ClvTerminal
	);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}
//------------------------------------------------------------------------------
//ID_GRA_RETIRO
void ServidorVentas::ActualizarRetiroEfectivo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{

	char *buffer_sql=new char[1024*32];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instrucciones[15], instruccion;
	int num_instrucciones=0,i=0;
	AnsiString ClvTerminal, ClvUsuario, ClvUsuarioAuto, ClvSucursal, tipo, refCorte, concepto, venta;
	double cantRetirado;

	try{

		ClvTerminal    = mFg.ExtraeStringDeBuffer(&parametros);
		ClvUsuarioAuto = mFg.ExtraeStringDeBuffer(&parametros);
		cantRetirado   = StrToFloat(mFg.ExtraeStringDeBuffer(&parametros));
		ClvUsuario     = mFg.ExtraeStringDeBuffer(&parametros);
		tipo     	   = mFg.ExtraeStringDeBuffer(&parametros);
		refCorte       = mFg.ExtraeStringDeBuffer(&parametros);
		concepto       = mFg.ExtraeStringDeBuffer(&parametros);
		venta          = mFg.ExtraeStringDeBuffer(&parametros);

		if (tipo=="R") {
			cantRetirado = (cantRetirado*-1);
		}

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		instruccion.sprintf( "select @folioaux:=valor from foliosemp where folio='RETIROS' AND sucursal = '%s'  for update ", FormServidor->ObtieneClaveSucursal());
		instrucciones[num_instrucciones++]=instruccion;

		instruccion = "set @foliosig=@folioaux+1 ";
		instrucciones[num_instrucciones++]=instruccion;

		instruccion = " set @folioaux=cast(@folioaux as char)";
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf(" set @folio=concat('%s', lpad(@folioaux,9,'0')) ", FormServidor->ObtieneClaveSucursal());
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf( " update foliosemp set valor=@foliosig where folio='RETIROS' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
		instrucciones[num_instrucciones++]=instruccion;

		if(venta != " "){
			instruccion.sprintf("INSERT INTO movsefectivocaja \
			(claveretiro, terminal, fechaalta, horaalta, fecharetiro, horaretiro, usuario, usuarioautoriza, concepto, cantretirado, corte, conceptomovs, venta) \
			VALUES (@folio, '%s', CURDATE(), CURTIME(), CURDATE(), CURTIME(), '%s', '%s', '%s', %f, '%s', '%s', '%s') ",
			ClvTerminal,
			ClvUsuario,
			ClvUsuarioAuto,
			tipo,
			cantRetirado,
			refCorte,
			concepto,
			venta);
		} else {
			instruccion.sprintf("INSERT INTO movsefectivocaja \
			(claveretiro, terminal, fechaalta, horaalta, fecharetiro, horaretiro, usuario, usuarioautoriza, concepto, cantretirado, corte, conceptomovs) \
			VALUES (@folio, '%s', CURDATE(), CURTIME(), CURDATE(), CURTIME(), '%s', '%s', '%s', %f, '%s', '%s') ",
			ClvTerminal,
			ClvUsuario,
			ClvUsuarioAuto,
			tipo,
			cantRetirado,
			refCorte,
			concepto);
        }
		instrucciones[num_instrucciones++]=instruccion;

		instrucciones[num_instrucciones++]="COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);
	} __finally {
		if (buffer_sql!=NULL)	delete buffer_sql;
	}
}
// ---------------------------------------------------------------------------
//ID_CONSULTA_CORTE_CAJA
void ServidorVentas::EjecutaConsultaCorteCaja(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString Instruccion;

	AnsiString ClvTerminal, ClvSucursal, corteSuc;
	AnsiString condicion=" ";

	ClvTerminal = mFg.ExtraeStringDeBuffer(&parametros);
	ClvSucursal = mFg.ExtraeStringDeBuffer(&parametros);
	corteSuc = mFg.ExtraeStringDeBuffer(&parametros);

	if (corteSuc=="0") {
		condicion.sprintf(" AND c.terminal = '%s' ", ClvTerminal);
	}   else {
		condicion.sprintf(" AND c.sucursal = '%s' ", ClvSucursal);
	}

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_MIN);
	Instruccion.sprintf("SELECT referencia \
	FROM cortesdecaja c \
	WHERE c.fechaapertura = CURDATE() \
	AND c.estatus = 'A' \
	%s ",
	condicion);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------
//ID_GUARDAR_CORTE_CAJA
void ServidorVentas::EjecutaGuardaCorteCaja(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*64*20];
	char *aux_buffer_sql=buffer_sql;
	int num_partidas, i;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[15];
	AnsiString Instruccion;

	AnsiString ClvTerminal, ClvSucursal, ClvUsuario;

	ClvTerminal = mFg.ExtraeStringDeBuffer(&parametros);
	ClvSucursal = mFg.ExtraeStringDeBuffer(&parametros);
	ClvUsuario = mFg.ExtraeStringDeBuffer(&parametros);
	try {

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		instruccion.sprintf( "select @folioaux:=valor from foliosemp where folio='CORTES' AND sucursal = '%s'  for update ",FormServidor->ObtieneClaveSucursal());
		instrucciones[num_instrucciones++]=instruccion;

		instruccion = "set @foliosig=@folioaux+1 ";
		instrucciones[num_instrucciones++]=instruccion;

		instruccion = " set @folioaux=cast(@folioaux as char)";
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf(" set @folio=concat('%s', lpad(@folioaux,9,'0')) ", FormServidor->ObtieneClaveSucursal());
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf(" update foliosemp set valor=@foliosig where folio='CORTES' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("INSERT INTO cortesdecaja \
		(referencia, fechaapertura, horaapertura, terminal, sucursal, usuarioapertura, estatus) \
		VALUES \
		(@folio, CURDATE(), CURTIME(), '%s', '%s', '%s', 'A') ",
		ClvTerminal,
		ClvSucursal,
		ClvUsuario);
		instrucciones[num_instrucciones++]=instruccion;

		instrucciones[num_instrucciones++]="COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			instruccion.sprintf("SELECT referencia FROM cortesdecaja c \
			WHERE c.fechaapertura = CURDATE() AND c.estatus = 'A' AND c.terminal = '%s'",
			ClvTerminal);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	} __finally {
		delete buffer_sql;
	}
}
// ---------------------------------------------------------------------------
//ID_CERRAR_CORTE_CAJA
void ServidorVentas::EjecutaCerrarCorteCaja(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*64*20];
	char *aux_buffer_sql=buffer_sql;
	int num_partidas, i;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[10];
	AnsiString Instruccion;

	AnsiString ClvUsuario, refCorte;

	ClvUsuario = mFg.ExtraeStringDeBuffer(&parametros);
	refCorte = mFg.ExtraeStringDeBuffer(&parametros);

	try {

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		instruccion.sprintf("UPDATE cortesdecaja SET fechacierre = CURDATE(), \
		horacierre = CURTIME(), estatus = 'C', usuariocierre = '%s' WHERE referencia = '%s' ",
		ClvUsuario,
		refCorte);
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
// ---------------------------------------------------------------------------
//ID_VERIFCORTES
void ServidorVentas::VerificaCortesAbiertos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString Instruccion;
	AnsiString ClvSucursal, FechaVerifi;
	AnsiString condicion_sucursal=" ", condicion_fechaverifi= " ";

	ClvSucursal = mFg.ExtraeStringDeBuffer(&parametros);
	FechaVerifi = mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

	if (ClvSucursal!=" ") {
		condicion_sucursal.sprintf(" AND c.sucursal = '%s' ", ClvSucursal);
	}else{
		condicion_sucursal.sprintf(" AND c.sucursal IN ( %s ) ", FormServidor->ObtieneSucursalesEmpAct());
	}

	if(FechaVerifi != " "){
		condicion_fechaverifi.sprintf(" AND c.fechaapertura <= '%s'", FechaVerifi);
	}

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
	Instruccion.sprintf("SELECT GROUP_CONCAT(' ', c.referencia) as cortes, \
	COUNT(c.referencia) AS numCortes \
	FROM cortesdecaja c \
	WHERE c.estatus = 'A' %s %s",
	condicion_sucursal,
	condicion_fechaverifi);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------
//ID_CERRAR_CORTES_CAJA
void ServidorVentas::EjecutaCerrarCorteCajaPendientes(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*64*20];
	char *aux_buffer_sql=buffer_sql;
	int num_partidas, i;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[1000];
	AnsiString Instruccion;

	AnsiString ClvUsuario, refCortes, corte, fechaverifica;
	AnsiString condicion_fechaverifica = " ";

	ClvUsuario = mFg.ExtraeStringDeBuffer(&parametros);
	refCortes = mFg.ExtraeStringDeBuffer(&parametros);
	fechaverifica = mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

	TStringDynArray cortesR(SplitString(refCortes, ","));

	try {

        AnsiString fechaCierre, horaCierre;

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		if(ClvUsuario == " "){
			fechaCierre = "fechaapertura";
			horaCierre = "'23:59:59'";
            ClvUsuario = "NULL";
		} else {
			fechaCierre = "CURDATE()";
			horaCierre = "CURTIME()";
            ClvUsuario = "'"+ClvUsuario+"'";
		}

		if(fechaverifica != " "){
			condicion_fechaverifica.sprintf("AND fechaapertura <= '%s' ", fechaverifica);
		}

		for (int c = 0; c < cortesR.Length; c++){
			corte = cortesR[c].Trim();
			instruccion.sprintf("UPDATE cortesdecaja SET fechacierre = %s, \
			horacierre = %s, estatus = 'C', usuariocierre = %s WHERE referencia = '%s' %s ",
			fechaCierre, horaCierre, ClvUsuario, corte, condicion_fechaverifica);
			instrucciones[num_instrucciones++]=instruccion;
		}

		instrucciones[num_instrucciones++]="COMMIT";
		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=1";

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
// ---------------------------------------------------------------------------
//ID_GUARDA_TRN_VENTA
void ServidorVentas::GuardaTranVenta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*64*20];
	char *aux_buffer_sql=buffer_sql;
	int num_partidas, i;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[15];
	AnsiString Instruccion;

	AnsiString campos, valores, id_trn, tipo, referencia;

	campos = mFg.ExtraeStringDeBuffer(&parametros);
	valores = mFg.ExtraeStringDeBuffer(&parametros);
	id_trn = mFg.ExtraeStringDeBuffer(&parametros);
	tipo = mFg.ExtraeStringDeBuffer(&parametros);
	referencia = mFg.ExtraeStringDeBuffer(&parametros);

	try {

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		if (tipo=="AJU") {
			instruccion.sprintf("UPDATE dventasfpago SET trn_id = NULL WHERE trn_id = %s ",
			id_trn);
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("DELETE FROM dettrnxventa WHERE trn_id = %s AND dcs_form = 'T060S000' ",
			id_trn);
			instrucciones[num_instrucciones++]=instruccion;
		}

		instruccion.sprintf("INSERT INTO dettrnxventa \
		(%s) \
		VALUES \
		(%s) ",
		campos,
		valores);
		instrucciones[num_instrucciones++]=instruccion;

		if (tipo=="AJU") {
			instruccion.sprintf("UPDATE dventasfpago SET trn_id = %s WHERE referencia = '%s' ",
			id_trn, referencia);
			instrucciones[num_instrucciones++]=instruccion;
		}

		instrucciones[num_instrucciones++]="COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			instruccion.sprintf("SELECT trn_internal_respcode, \
			trn_id, \
			IF(trn_pre_type='1', 'FTARCR', 'FTARDE') AS tipo_tar, \
			trn_amount AS monto, \
			trn_host_date AS fecha, \
			trn_host_hour AS hora \
			from dettrnxventa where trn_id = '%s' ", id_trn);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	} __finally {
		delete buffer_sql;
	}
}
// ---------------------------------------------------------------------------
//ID_GUARDA_BITACORA_TRN_VENTA
void ServidorVentas::GuardaBitacoraTranVenta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*64*20];
	char *aux_buffer_sql=buffer_sql;
	int num_partidas, i;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[100];
	AnsiString Instruccion;

	AnsiString campos, valores, id_trn;

	campos = mFg.ExtraeStringDeBuffer(&parametros);
	valores = mFg.ExtraeStringDeBuffer(&parametros);
	id_trn = mFg.ExtraeStringDeBuffer(&parametros);
	try {

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		instruccion.sprintf("INSERT INTO bitacoratransaccionesventas \
		(%s) \
		VALUES \
		(%s) ",
		campos,
		valores);
		instrucciones[num_instrucciones++]=instruccion;

		instrucciones[num_instrucciones++]="COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		if(mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)){
			instruccion.sprintf("SELECT LAST_INSERT_ID() AS id");
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	} __finally {
		delete buffer_sql;
	}
}
// ---------------------------------------------------------------------------
//ID_CON_TRN_VENTA
void ServidorVentas::ConsultaTranVenta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString Instruccion;
	AnsiString IDTrn, Transacciones;

	IDTrn = mFg.ExtraeStringDeBuffer(&parametros);

	TStringDynArray IDsTransacciones(SplitString(IDTrn, ","));
	for (int i = 0; i < IDsTransacciones.Length; i++) {
		if (IDsTransacciones[i]!="0") {
			Transacciones  = Transacciones + "'" + IDsTransacciones[i] +"'";
			if( i != (IDsTransacciones.Length-1) )
				Transacciones = Transacciones + ",";
		}
	}

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	Instruccion.sprintf("SELECT \
	dtrnvnta.mer_legend1 AS nomnegocio, \
	dtrnvnta.mer_legend2 AS dirnegocio, \
	dtrnvnta.mer_legend3 AS ciudnegocio, \
	dtrnvnta.trn_external_mer_id AS negocio, \
	dtrnvnta.trn_external_ter_id AS termnegocio, \
	dtrnvnta.trn_host_date AS fecha, \
	dtrnvnta.trn_host_hour AS hora \
	FROM dettrnxventa dtrnvnta \
	WHERE dtrnvnta.trn_id IN (%s) \
	GROUP BY dtrnvnta.mer_legend1  ",
	Transacciones);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), Respuesta->TamBufferResultado);

	Instruccion.sprintf("SELECT \
	dtrnvnta.trn_AID AS aid, \
	lpad(RIGHT(dtrnvnta.trn_aco_id,4),16,'*') AS aco, \
	dtrnvnta.trn_auth_code AS auth_code, \
	lpad(dtrnvnta.trn_id,9,'0') AS id, \
	dtrnvnta.trn_amount AS monto, \
	dtrnvnta.trn_pro_name AS proname, \
	dtrnvnta.trn_fii, \
	IF(dtrnvnta.trn_qty_pay=1, 'COMPRA NORMAL', CONCAT(dtrnvnta.trn_qty_pay, ' MESES SIN INTERESES')) AS tipocompra, \
	IF(dtrnvnta.trn_fii!='', -1, IF(dtrnvnta.trn_fe=1, 1, 0)) AS tipofirma, \
	dtrnvnta.trn_internal_ter_id AS trnterminal, \
	dtrnvnta.trn_host_hour AS hora \
	FROM dettrnxventa dtrnvnta \
	WHERE dtrnvnta.trn_id IN (%s) ",
	Transacciones);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------
//ID_CON_TRN_FOLIO
void ServidorVentas::GeneraFolioTransaccion(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{

	char *buffer_sql=new char[1024*64*20];
	char *aux_buffer_sql=buffer_sql;
	int num_partidas, i;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[100];
	AnsiString Instruccion;
	try {

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		// Obtiene el folio para la NUEVA transaccion
		instruccion.sprintf("select @foliotrnpinpad:=valor from foliosemp where folio='TRNPINPAD' AND sucursal = '%s' %s ",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
		instrucciones[num_instrucciones++]=instruccion;
		instruccion.sprintf("set @foliosigtrn=@foliotrnpinpad+1");
		instrucciones[num_instrucciones++]=instruccion;
		instruccion.sprintf("set @foliotrnpinpad=cast(@foliotrnpinpad as char)");
		instrucciones[num_instrucciones++]=instruccion;
		instruccion.sprintf("set @foliotrn=concat('%s', lpad(@foliotrnpinpad,9,'0'))", FormServidor->ObtieneClaveSucursal());
		instrucciones[num_instrucciones++]=instruccion;
		instruccion.sprintf("update foliosemp set valor=@foliosigtrn where folio='TRNPINPAD' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
		instrucciones[num_instrucciones++]=instruccion;

		instrucciones[num_instrucciones++]="COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			instruccion.sprintf("SELECT @foliotrn AS folio ");
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	} __finally {
		delete buffer_sql;
	}
}
//ID_GRA_CARTAPORTE_V20
void ServidorVentas::GrabaCartaPorte_V20(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  GRABA CARTA PORTE V20
	char *buffer_sql=new char[1024*64*10];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	DatosTabla datos_detalle(mServidorVioleta->Tablas);
	AnsiString referencia, usuario, terminal, mensaje, tipo;
	int num_partidas, i;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[2000], instruccion_fcha;
	int error=0;
	BufferRespuestas* resp_restricciones1=NULL;
	BufferRespuestas* resp_restricciones2=NULL;
	BufferRespuestas* resp_restricciones3=NULL;
	BufferRespuestas* resp_restricciones4=NULL;
	BufferRespuestas* resp_restricciones5=NULL;
	BufferRespuestas* resp_restricciones6=NULL;
	BufferRespuestas* resp_restricciones7=NULL;
    BufferRespuestas* resp_restricciones8=NULL;
	BufferRespuestas* resp_calcfechllegada=NULL;
	BufferRespuestas* resp_param_cartas=NULL;
	AnsiString cp20,refven,refped,cancelada,mensajeExcepcion="";
	TDate fecha=Today();
	TTime hora=Time();
	AnsiString tabla_movimiento="";
	AnsiString viaembarque=" ",empleado=" ", remolque=" ";
	AnsiString condicion_movimiento=" ",refpedcom="";
	AnsiString idconfigvehicular, claveconfigvehicular;
	AnsiString calleNumOrigen="",coloniaOrigen="",codPosOrigen="";
	AnsiString ubicaciongis="";
    AnsiString FechaHoraLLegada;
	AnsiString FechaLLegada;
	AnsiString HoraLLegada;
	double VersCartaPorte;

	try {
		referencia=mFg.ExtraeStringDeBuffer(&parametros); // referencia del movimiento.
		usuario=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que da de alta la carta porte.
		tipo=mFg.ExtraeStringDeBuffer(&parametros); // tipo de cartaporte venta o pedido.
		terminal=mFg.ExtraeStringDeBuffer(&parametros); // tipo de cartaporte venta o pedido.
		if (tipo=="PED_COMPRA") { //parametros extra solo cuando es pedido de compra
			viaembarque=mFg.ExtraeStringDeBuffer(&parametros);//Clave unica de via de embarque
			remolque=mFg.ExtraeStringDeBuffer(&parametros);//Clave de remolque asiganado
			empleado=mFg.ExtraeStringDeBuffer(&parametros);//Clave de chofer
			idconfigvehicular=mFg.ExtraeStringDeBuffer(&parametros);//id clave figura transporte
			claveconfigvehicular=mFg.ExtraeStringDeBuffer(&parametros);// clave figura transporte
			calleNumOrigen=mFg.ExtraeStringDeBuffer(&parametros);//Calle y numero de origen o recolección de pedido
			coloniaOrigen=mFg.ExtraeStringDeBuffer(&parametros);//Colonia  de origen o relcoleccion de pedido
			codPosOrigen=mFg.ExtraeStringDeBuffer(&parametros);//Codigo postal del origen o recoleccion del pedido
            ubicaciongis=mFg.ExtraeStringDeBuffer(&parametros);//UbicacionGis del origen o recoleccion del pedido
			if (remolque==" " || remolque=="") {
				remolque="NULL";
			}
		}
		if (tipo=="VENTA") {
		   condicion_movimiento="referencia";
		} else if (tipo=="PEDIDO") {
			condicion_movimiento="pedido";
		} else if (tipo=="PED_COMPRA") {
			condicion_movimiento="pedidocompra";
		}

		instruccion.sprintf("SELECT versioncarta FROM paramcartport20 WHERE idparamcp20 = 1 ");
		if (!mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_param_cartas)) {
			throw Exception("No se puede consultar los parametros para  cartas porte");
		} else {
			if (resp_param_cartas->ObtieneNumRegistros()==0)
				throw Exception("No se encontró registro de los parametros");
			else
				VersCartaPorte=mFg.CadenaAFlotante(resp_param_cartas->ObtieneDato("versioncarta"));
		}

		instruccion.sprintf("SELECT cp20.cartaporte20,cp20.referencia, cp20.pedido,cp20.pedidocompra,cp20.cancelada \
			FROM cartasporte20 cp20 \
			WHERE cp20.%s='%s' \
			AND cp20.cancelada=0 ORDER BY cp20.fechacp desc, cp20.horacp desc", condicion_movimiento,referencia);
		if (!mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_restricciones1))
			throw Exception("No se pudieron consultar las restricciones");
		if (resp_restricciones1->ObtieneNumRegistros()>0){
			resp_restricciones1->IrAlRegistroNumero(0);
			cp20 = resp_restricciones1->ObtieneDato("cartaporte20");
			refven = resp_restricciones1->ObtieneDato("referencia");
			refped = resp_restricciones1->ObtieneDato("pedido");
			refpedcom = resp_restricciones1->ObtieneDato("pedidocompra");
			cancelada = resp_restricciones1->ObtieneDato("cancelada");
			if (refven!="") {
				mensajeExcepcion="El pedido de compra: ";
			}else{
				if (refven!="") {
				   mensajeExcepcion="La venta: ";
				}else{
				   mensajeExcepcion="El pedido: ";
				}
			}

			mensajeExcepcion+=referencia+" ,tiene la carta porte activa: "+cp20+".";
			throw Exception(mensajeExcepcion);
		}else{
			if (tipo=="PEDIDO") {
				instruccion.sprintf("Select p.venta from pedidosventa p where referencia='%s' and facturado=1",referencia);
				if (!mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_restricciones2))
					throw Exception("No se pudieron consultar las restricciones");
				if (resp_restricciones2->ObtieneNumRegistros()>0){
					if (resp_restricciones2->ObtieneDato("venta")!="") {
						throw Exception("Ese pedido ya cuenta con venta, genere carta porte desde la venta.");
					}
				}
			} else if (tipo=="VENTA") {
				instruccion.sprintf("SELECT v.embarque, cfd.muuid FROM ventas v LEFT JOIN cfd ON cfd.referencia = v.referencia WHERE v.referencia='%s' ",referencia);
				if (!mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_restricciones2))
					throw Exception("No se pudieron consultar las restricciones");
				if (resp_restricciones2->ObtieneNumRegistros()>0){
					if (resp_restricciones2->ObtieneDato("muuid")=="") {
						throw Exception("Esa venta debe de estar facturada.");
					}
					if (resp_restricciones2->ObtieneDato("embarque")=="") {
						throw Exception("Esa venta debe de estar embarcada.");
					}
				}
			} else if (tipo=="PED_COMPRA") {
				instruccion.sprintf("Select p.compra from pedidos p where referencia='%s' and facturado=1",referencia);
				if (!mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_restricciones2))
					throw Exception("No se pudieron consultar las restricciones");
				if (resp_restricciones2->ObtieneNumRegistros()>0){
					if (resp_restricciones2->ObtieneDato("venta")!="") {
						throw Exception("Ese pedido ya cuenta con compra.");
					}
				}
			}
		}
	} __finally {
		if (resp_restricciones1!=NULL)	delete resp_restricciones1;
		if (resp_restricciones2!=NULL)	delete resp_restricciones2;
		if (resp_param_cartas!=NULL) delete resp_param_cartas;
	}
	//valida que al menos la clave de un articulo del movimiento(venta o pedido) se encuentre
	//en el catálogo de productos y servicios de cartas porte "ccpclaveprod" para poder generar
	//la carta porte.
	try {
		if (tipo=="PEDIDO") {
			instruccion.sprintf("SELECT if(COUNT(ccps.cveprodserv)=SUM(ISNULL(ccpc.cveprod)),TRUE, FALSE) AS sinclavescarta\
								FROM dpedidosventa dpv  \
								INNER JOIN articulos a ON a.articulo= dpv.articulo  \
								INNER JOIN productos p ON p.producto=a.producto \
								INNER JOIN cclaveprodserv ccps ON ccps.idclaveprodserv=p.idclaveprodservcfdi    \
								LEFT JOIN ccpclaveprod ccpc ON ccpc.cveprod=ccps.cveprodserv    \
								WHERE dpv.referencia='%s' GROUP BY dpv.referencia", referencia);

		}else if(tipo=="VENTA"){
			instruccion.sprintf("SELECT if(COUNT(ccps.cveprodserv)=SUM(ISNULL(ccpc.cveprod)),TRUE, FALSE) AS sinclavescarta \
								FROM dventas dv	\
								INNER JOIN articulos a ON a.articulo= dv.articulo   \
								INNER JOIN productos p ON p.producto=a.producto \
								INNER JOIN cclaveprodserv ccps ON ccps.idclaveprodserv=p.idclaveprodservcfdi	\
								LEFT JOIN ccpclaveprod ccpc ON ccpc.cveprod=ccps.cveprodserv    \
								WHERE dv.referencia='%s' GROUP BY dv.referencia", referencia);
		}else if(tipo=="PED_COMPRA"){
			instruccion.sprintf("SELECT if(COUNT(ccps.cveprodserv)=SUM(ISNULL(ccpc.cveprod)),TRUE, FALSE) AS sinclavescarta \
								FROM dpedidos dp	\
								INNER JOIN articulos a ON a.articulo= dp.articulo   \
								INNER JOIN productos p ON p.producto=a.producto \
								INNER JOIN cclaveprodserv ccps ON ccps.idclaveprodserv=p.idclaveprodservcfdi	\
								LEFT JOIN ccpclaveprod ccpc ON ccpc.cveprod=ccps.cveprodserv    \
								WHERE dp.referencia='%s' GROUP BY dp.referencia", referencia);
		}

		if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_restricciones3)) {
			if (resp_restricciones3->ObtieneDato("sinclavescarta")==1){
				throw (Exception("El "+tipo+" debe tener al menos un articulo con clave de producto contenido en el catálogo de productos y servicios de carta porte"));
			}
		} else throw (Exception("Error al consultar el catálogo de productos y servicios del SAT"));
	} __finally {
			if (resp_restricciones3!=NULL) delete resp_restricciones3;
	}
	/*valida remolques en caso de tenerlos y tipos de permisos según su denominacion de uso.*/
	try {
		if (tipo == "PEDIDO") {
			instruccion.sprintf("SELECT ifnull(ctp.cvetippermiso,'') AS tipoperm ,ifnull(ve.numpermisosct,'') AS nperm ,ifnull(ca.clave,'') AS caseguradora,	\
			ifnull(ve.polizaaseguradora,'') AS npoliza ,ve.remolque, ifnull(ve.idremolque,'') AS idrem  \
			FROM pedidosventa pv	\
			INNER JOIN embarques e ON e.embarque=pv.embarque    \
			INNER JOIN viasembarque ve ON ve.viaembarq=e.viaembarq  \
			LEFT JOIN ccptipopermiso ctp ON ctp.idclavetipperm=ve.tipopermiso AND ctp.activo=1	\
			LEFT JOIN catalogoaseguradoras ca ON ca.clave=ve.claveaseguradora AND ca.activa=1	\
			LEFT JOIN catalogoremolques cr ON cr.idremolque=ve.idremolque AND cr.activo=1   \
			WHERE pv.referencia='%s'", referencia);

		}else if(tipo == "VENTA"){
			instruccion.sprintf("SELECT ifnull(ctp.cvetippermiso,'') AS tipoperm ,ifnull(ve.numpermisosct,'') AS nperm ,ifnull(ca.clave,'') AS caseguradora,    \
			ifnull(ve.polizaaseguradora,'') AS npoliza ,ve.remolque, ifnull(e.idremolque,'0') AS idrem  \
			FROM ventas v   \
			INNER JOIN embarques e ON e.embarque=v.embarque	\
			INNER JOIN viasembarque ve ON ve.viaembarq=e.viaembarq  \
			LEFT JOIN ccptipopermiso ctp ON ctp.idclavetipperm=ve.tipopermiso AND ctp.activo=1  \
			LEFT JOIN catalogoaseguradoras ca ON ca.clave=ve.claveaseguradora AND ca.activa=1   \
			LEFT JOIN catalogoremolques cr ON cr.idremolque=ve.idremolque AND cr.activo=1   \
			WHERE v.referencia='%s'", referencia);
		}else if(tipo == "PED_COMPRA"){
			instruccion.sprintf("SELECT ifnull(ctp.cvetippermiso,'') AS tipoperm,	\
			ifnull(ve.numpermisosct,'') AS nperm ,ifnull(ca.clave,'') AS caseguradora,  \
			ifnull(ve.polizaaseguradora,'') AS npoliza ,ve.remolque, IF('%s'<>' ','%s','0') idrem \
			FROM viasembarque ve    \
			LEFT JOIN ccptipopermiso ctp ON ctp.cvetippermiso=ve.tipopermiso AND ctp.activo=1   \
			LEFT JOIN catalogoaseguradoras ca ON ca.clave=ve.claveaseguradora AND ca.activa=1   \
			LEFT JOIN catalogoremolques cr ON cr.idremolque=ve.idremolque AND cr.activo=1   \
			WHERE ve.viaembarq='%s'", remolque, remolque, viaembarque);
		}

		if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_restricciones4)) {
			if (resp_restricciones4->ObtieneDato("tipoperm")=="" && resp_restricciones4->ObtieneDato("nperm")==""){
				throw (Exception("Revise el permiso de la vía de embarque del "+tipo));
			}
			if (resp_restricciones4->ObtieneDato("caseguradora")=="" && resp_restricciones4->ObtieneDato("npoliza")==""){
				throw (Exception("Revise la poliza de la vía de embarque del "+tipo));
			}
			if (resp_restricciones4->ObtieneDato("remolque")=="1") {
				if (resp_restricciones4->ObtieneDato("idrem")=="0"){
					throw (Exception("Revise el remolque de la vía de embarque del "+tipo));
				}
			}
		} else throw (Exception("Error al consultar datos de la vía de embarque."));
	} __finally {
			if (resp_restricciones4!=NULL) delete resp_restricciones4;
	}
	/*Valida que si el embarque ya contiene cartas porte timbradas, que las que
	 faltan por generar contengan los mismos datos del embarque original, si se desea cambiar,
	 se recomienda cancelar y volver a generar con los datos actualizados.*/

	try {
		if (tipo!="PED_COMPRA") {
			if (tipo=="PEDIDO") {
				tabla_movimiento="pedidosventa";
			}else if(tipo=="VENTA"){
				tabla_movimiento="ventas";
			}

			instruccion.sprintf(" SELECT f.* FROM ( \
			(SELECT cp20.rfcfig,cp20.numlicfig,cp20.viaembarq,cp20.tiporemolque,cp20.placaremolque \
				FROM cartasporte20 cp20 \
				WHERE cp20.embarque=(SELECT @embarque:=embarque FROM %s WHERE referencia='%s') \
				AND cp20.cancelada=0) \
			UNION \
				(SELECT emp.rfc,chof.numlicencia,emb.viaembarq,ifnull(csr.cvesubtiprem,''),ifnull(cr.numeroplaca,'') \
				FROM embarques emb \
				INNER JOIN choferes chof ON emb.chofer=chof.empleado \
				INNER JOIN empleados emp ON emp.empleado=chof.empleado \
				INNER JOIN viasembarque vemb ON vemb.viaembarq=emb.viaembarq \
				LEFT JOIN catalogoremolques cr ON cr.idremolque=vemb.idremolque \
				LEFT JOIN ccpsubtiporem csr ON csr.idclavesubtiprem=cr.ccvesubtiporem	\
				WHERE emb.embarque=@embarque) \
			) AS f",tabla_movimiento,referencia);

			if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_restricciones5)) {
				if (resp_restricciones5->ObtieneNumRegistros()>1){
					throw (Exception("Los datos del embarque no coinciden con los de las carta porte ya generadas, cancele y reintente"));
				}
			} else throw (Exception("Error al consultar si los datos de la nueva carta porte son iguales a preexistentes"));
		}
	} __finally {
			if (resp_restricciones5!=NULL) delete resp_restricciones5;
	}
	/*Valida los códigos postales de emisor,origen y destino*/
	try {
		if (tipo == "PEDIDO") {
			instruccion.sprintf("SELECT max(t.codposdest) AS codposdest,    \
			max(t.codposorige) as codposorige, max(t.codposemis) AS codposemis  \
			FROM (SELECT ccp.ccodigopostal AS codposdest, '' AS codposorige, '' AS codposemis \
			FROM pedidosventa pv	\
			INNER JOIN pedidosdirent pd ON pd.referencia=pv.referencia  \
			INNER JOIN ccpclavecodpos ccp ON ccp.ccodigopostal=pd.cp    \
			WHERE pv.referencia='%s'   \
			UNION   \
			SELECT '' AS codposdest, ccp1.ccodigopostal AS codposorige, ccp2.ccodigopostal AS codposemis    \
			FROM terminales t   \
			INNER JOIN secciones s ON s.seccion=t.seccion   \
			INNER JOIN parametroscfd pcfd ON pcfd.sucursal=s.sucursal   \
			INNER JOIN ccpclavecodpos ccp1 ON ccp1.ccodigopostal=pcfd.cpexp \
			INNER JOIN ccpclavecodpos ccp2 ON ccp2.ccodigopostal=pcfd.cpemisor  \
			WHERE t.terminal='%s') AS t", referencia,terminal);

		}else if(tipo == "VENTA"){
			instruccion.sprintf("SELECT max(t.codposdest) AS codposdest,    \
			max(t.codposorige) as codposorige, max(t.codposemis) AS codposemis  \
			FROM (SELECT ccp.ccodigopostal AS codposdest, '' AS codposorige, '' AS codposemis	\
			FROM ventas v   \
			INNER JOIN ventadirent vd ON vd.referencia=v.referencia \
			INNER JOIN ccpclavecodpos ccp ON ccp.ccodigopostal=vd.cp    \
			WHERE v.referencia='%s'    \
			UNION   \
			SELECT '' AS codposdest, ccp1.ccodigopostal AS codposorige, ccp2.ccodigopostal AS codposemis    \
			FROM terminales t   \
			INNER JOIN secciones s ON s.seccion=t.seccion   \
			INNER JOIN parametroscfd pcfd ON pcfd.sucursal=s.sucursal   \
			INNER JOIN ccpclavecodpos ccp1 ON ccp1.ccodigopostal=pcfd.cpexp \
			INNER JOIN ccpclavecodpos ccp2 ON ccp2.ccodigopostal=pcfd.cpemisor  \
			WHERE t.terminal='%s') AS t", referencia, terminal);
		}else if(tipo == "PED_COMPRA"){
			instruccion.sprintf("SELECT  max(t.codposdest) AS codposdest, max(t.codposorige) as codposorige,	\
			max(t.codposemis) AS codposemis FROM (  \
			SELECT '' AS codposdest, ccp.ccodigopostal AS codposorige, '' AS codposemis \
			FROM ccpclavecodpos ccp	\
			WHERE ccp.ccodigopostal='%s'	\
			UNION   \
			SELECT ccp1.ccodigopostal AS codposdest, '' AS codposorige, ccp2.ccodigopostal AS codposemis    \
			FROM terminales t   \
			INNER JOIN secciones s ON s.seccion=t.seccion   \
			INNER JOIN parametroscfd pcfd ON pcfd.sucursal=s.sucursal   \
			INNER JOIN ccpclavecodpos ccp1 ON ccp1.ccodigopostal=pcfd.cpexp \
			INNER JOIN ccpclavecodpos ccp2 ON ccp2.ccodigopostal=pcfd.cpemisor  \
			WHERE t.terminal='%s') AS t;", codPosOrigen, terminal);
		}

		if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_restricciones6)) {
			if (resp_restricciones6->ObtieneDato("codposdest")==""){
				if (tipo != "PED_COMPRA") {
					throw (Exception("El código postal del cliente no es válido."));
				}else{
					throw (Exception("El código postal de la sucursal de destino no es válido"));
				}
			}
			if (resp_restricciones6->ObtieneDato("codposorige")==""){
				if (tipo != "PED_COMPRA") {
					throw (Exception("El código postal de la sucursal de origen no es válido"));
				}else{
					throw (Exception("El código postal del proveedor de origen no es válido"));
				}
			}
			if (resp_restricciones6->ObtieneDato("codposemis")==""){
				throw (Exception("El código postal fiscal de la violeta no es válido."));
			}
		} else throw (Exception("Error al consultar códigos postales."));
	} __finally {
			if (resp_restricciones6!=NULL) delete resp_restricciones6;
	}
	/*Valida la venta a la que se le quiere hacer carta porte, no provenga de un pedido con carta porte activa*/
	if(tipo == "VENTA"){
		try {
			if(tipo == "VENTA"){
				instruccion.sprintf("SELECT pv.referencia,cp20.cartaporte20 \
				FROM pedidosventa pv    \
				INNER JOIN cartasporte20 cp20 ON cp20.pedido=pv.referencia AND cp20.cancelada=0  \
				WHERE pv.venta='%s'", referencia);
			}

			if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_restricciones7)) {
				if (resp_restricciones7->ObtieneNumRegistros()>0){
					throw (Exception("El pedido: "+resp_restricciones7->ObtieneDato("referencia")+", de la venta: "+referencia+
					", ya cuenta con la carta porte: "+resp_restricciones7->ObtieneDato("cartaporte20")+
					". Cancele la carta porte del pedido y reintente."));
				}
			} else throw (Exception("Error al consultar cartas porte de pedidos de la venta: "+referencia));
		} __finally {
				if (resp_restricciones7!=NULL) delete resp_restricciones7;
		}

	}

	//valida que los datos del chofer como licencia o RFC  no sean vacios, nulos o en caso de RFC, generico
	try {
		if (tipo!="PED_COMPRA") {
            if (tipo=="PEDIDO") {
				tabla_movimiento="pedidosventa";
			}else if(tipo=="VENTA"){
				tabla_movimiento="ventas";
			}
			instruccion.sprintf("SELECT emp.rfc,chof.numlicencia,chof.empleado  \
			FROM %s vop   \
			inner join embarques emb ON vop.embarque=emb.embarque \
			INNER JOIN choferes chof ON emb.chofer=chof.empleado    \
			INNER JOIN empleados emp ON emp.empleado=chof.empleado  \
			WHERE ((emp.rfc='' OR emp.rfc IS NULL OR emp.rfc='XAXX010101000')   \
			OR (chof.numlicencia='' OR chof.numlicencia IS NULL)) AND vop.referencia='%s'",
			tabla_movimiento,referencia); // Referencia de venta o pedido
		}else{
			instruccion.sprintf("SELECT emp.rfc,chof.numlicencia	\
			FROM  choferes chof \
			INNER JOIN empleados emp ON emp.empleado=chof.empleado  \
			WHERE ((emp.rfc='' OR emp.rfc IS NULL OR emp.rfc='XAXX010101000')	\
			OR (chof.numlicencia='' OR chof.numlicencia IS NULL)) AND chof.empleado='%s'",empleado); //Chofer
		}
		if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_restricciones8)) {
			if (resp_restricciones8->ObtieneNumRegistros()>0){
				throw (Exception("El RFC y/o el número de licencia del chofer son incorrectas."));
			}
		} else throw (Exception("Error al consultar el RFC y el número de licencia del chofer"));
	} __finally {
			if (resp_restricciones8!=NULL) delete resp_restricciones8;
	}

	try {
			if (error==0) {
				instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
				instrucciones[num_instrucciones++]="START TRANSACTION";

				//instruccion.sprintf("select @seccion:=seccion, @depart:=depart, @asigfolvta:=asigfolvta, @anchofolvta:=anchofolvta from terminales where terminal='%s'", terminal);
				//instrucciones[num_instrucciones++]=instruccion;

				// Folio de sistema.
				instruccion.sprintf("select @folioaux:=valor from foliosemp where folio='CARTASP20' AND sucursal = '%s' %s",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliosig=@folioaux+1");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @folioaux=cast(@folioaux as char)");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @folio=concat('%s', lpad(@folioaux,9,'0'))", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("update foliosemp set valor=@foliosig where folio='CARTASP20' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;

				instruccion.sprintf("SET @ccveunpes=(SELECT ccveunidadpeso FROM paramcartport20)");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("SET @ccvetipofig=(SELECT ccvetransporte FROM paramcartport20)");
				instrucciones[num_instrucciones++]=instruccion;

				try {
					instruccion_fcha.sprintf("SELECT \
					IF( \
						DATE_ADD('%s %s', INTERVAL 12 HOUR) BETWEEN '%s 18:00:00' AND '%s 12:00:00', \
						DATE_ADD('%s %s', INTERVAL 24 HOUR), \
						DATE_ADD('%s %s', INTERVAL 12 HOUR) \
					) AS dateLlegada",
					mFg.DateToMySqlDate(fecha),
					mFg.TimeToMySqlTime(hora),
					mFg.DateToMySqlDate(fecha),
					mFg.DateToMySqlDate(fecha+1),
					mFg.DateToMySqlDate(fecha),
					mFg.TimeToMySqlTime(hora),
					mFg.DateToMySqlDate(fecha),
					mFg.TimeToMySqlTime(hora),
					mFg.DateToMySqlDate(fecha),
					mFg.TimeToMySqlTime(hora));

					if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion_fcha.c_str(), resp_calcfechllegada)) {
						FechaHoraLLegada = resp_calcfechllegada->ObtieneDato("dateLlegada");

						TStringDynArray DateLlegada(SplitString(FechaHoraLLegada, " "));
						FechaLLegada = DateLlegada[0];
						HoraLLegada = DateLlegada[1];
					}
				} __finally {
					if (resp_calcfechllegada!=NULL) delete resp_calcfechllegada;
				}

				if (tipo=="VENTA") {

					instruccion.sprintf("SET @numtotmerc=(SELECT SUM(cantidad) FROM dventas WHERE referencia='%s')",referencia);
					instrucciones[num_instrucciones++]=instruccion;

					instruccion.sprintf("SET @pesototmerc=(SELECT sum(a.peso*dv.cantidad) FROM dventas dv \
											INNER JOIN articulos a ON a.articulo=dv.articulo \
											WHERE dv.referencia='%s')",referencia);
					instrucciones[num_instrucciones++]=instruccion;

					instruccion.sprintf("INSERT INTO cartasporte20(cartaporte20,referencia,rfcremitente,cpremitente,fechasalida,horasalida, \
					rfcdestinatario,cpdestinatario,fechallegada,horallegada,viaembarq,placavm,aniomodelovm,asegurcivil,polizarespcivil, \
					idremolque,tiporemolque,placaremolque,permsct,numpermsct,pesobruttotal,unidadpeso,pesonettot,numtotmerc,usualta, \
					usumodi,fechaalta,horaalta,fechacp,horacp,fechamod,horamod,terminal,embarque,tipofig,chofer,rfcfig,numlicfig,idconfigvehicular,configvehicular, version) \
					SELECT @folio,v.referencia,pcfd.rfcemisor,ccp2.idclavecodpos,'%s','%s',	\
					IFNULL(cfd.rfcreceptor,if(cfd.rfcreceptor='','XAXX010101000',cfd.rfcreceptor)), ifnull(ccp0.idclavecodpos,ccp1.idclavecodpos),\
					'%s','%s',emb.viaembarq,ve.placas,ve.modelo,ve.claveaseguradora,ve.polizaaseguradora,emb.idremolque,ifnull(csr.cvesubtiprem,''), \
					IFNULL(cr.numeroplaca,''),ve.tipopermiso,ve.numpermisosct,@pesototmerc,@ccveunpes,@pesototmerc,@numtotmerc,'%s', \
					'%s','%s','%s','%s','%s','%s','%s','%s',v.embarque, @ccvetipofig,chof.empleado, emp.rfc, chof.numlicencia, cca.idclaveautotrans, cca.cveautotrans, %f \
					FROM ventas v \
					LEFT JOIN clientes cl ON cl.cliente=v.cliente \
					LEFT JOIN terminales ter ON ter.terminal=v.terminal \
					LEFT JOIN embarques emb ON emb.embarque=v.embarque \
					LEFT JOIN viasembarque ve ON ve.viaembarq=emb.viaembarq \
					LEFT JOIN cconfigautotransporte cca ON cca.idclaveautotrans = ve.configvehicular \
					LEFT JOIN catalogoremolques cr ON cr.idremolque=ve.idremolque \
					LEFT JOIN ccpsubtiporem csr ON csr.idclavesubtiprem=cr.ccvesubtiporem \
					LEFT JOIN cfd ON cfd.referencia=v.referencia \
					LEFT JOIN secciones sec ON ter.seccion=sec.seccion \
					LEFT JOIN sucursales su ON sec.seccion=su.sucursal \
					LEFT JOIN parametroscfd pcfd ON pcfd.sucursal=cfd.sucursal \
					LEFT JOIN empleados emp on emb.chofer=emp.empleado \
					LEFT JOIN choferes chof on emb.chofer=chof.empleado \
					left join ventadirent vd on vd.referencia=v.referencia  \
					left JOIN ccpclavecodpos ccp0 ON ccp0.ccodigopostal=vd.cp \
					left JOIN ccpclavecodpos ccp1 ON ccp1.ccodigopostal=cl.cp \
					INNER JOIN ccpclavecodpos ccp2 ON ccp2.ccodigopostal=pcfd.cpexp	\
					WHERE v.referencia='%s'",
					mFg.DateToMySqlDate(fecha),mFg.TimeToMySqlTime(hora), //fecha y hora de salida
					FechaLLegada, HoraLLegada, //fecha y hora de llegada (mismo dia, hora de alta mas 12 horas)
					usuario,usuario,
					mFg.DateToMySqlDate(fecha),mFg.TimeToMySqlTime(hora), //fecha y hora de alta
					mFg.DateToMySqlDate(fecha),mFg.TimeToMySqlTime(hora), //fecha y hora de carta porte
					mFg.DateToMySqlDate(fecha),mFg.TimeToMySqlTime(hora), //fecha y hora de modificación
					terminal, VersCartaPorte, referencia);
					instrucciones[num_instrucciones++]=instruccion;

					instruccion.sprintf("INSERT INTO dcartasporte20(cartaporte20,bienestransp,descripcion,cantidad,claveunidad,pesokg,articulo) \
					SELECT @folio,dvc.idclaveprodserv,CONCAT(TRIM(p.nombre),' ',a.present,' ',a.multiplo),dv.cantidad,cu.idclaveunidad,(a.peso*dv.cantidad),dv.articulo \
					FROM dventas dv \
					INNER JOIN articulos a ON a.articulo=dv.articulo \
					INNER JOIN productos p ON p.producto=a.producto \
					INNER JOIN dventascfdi dvc ON dvc.referencia=dv.referencia AND dvc.articulo=dv.articulo \
					INNER JOIN cclaveunidad cu ON cu.idclaveunidad=dvc.idclaveunidad \
					WHERE dv.referencia='%s'",referencia);
					instrucciones[num_instrucciones++]=instruccion;

				} else if (tipo=="PEDIDO"){

					instruccion.sprintf("SET @numtotmerc=(SELECT SUM(cantidad) FROM dpedidosventa WHERE referencia='%s')",referencia);
					instrucciones[num_instrucciones++]=instruccion;

					instruccion.sprintf("SET @pesototmerc=(SELECT sum(a.peso*pv.cantidad) FROM dpedidosventa pv \
											INNER JOIN articulos a ON a.articulo=pv.articulo \
											WHERE pv.referencia='%s')",referencia);
					instrucciones[num_instrucciones++]=instruccion;

					instruccion.sprintf("INSERT INTO cartasporte20(cartaporte20,pedido,rfcremitente,cpremitente,fechasalida,horasalida, \
					rfcdestinatario,cpdestinatario,fechallegada,horallegada,viaembarq,placavm,aniomodelovm,asegurcivil,polizarespcivil, \
					idremolque,tiporemolque,placaremolque,permsct,numpermsct,pesobruttotal,unidadpeso,pesonettot,numtotmerc,usualta, \
					usumodi,fechaalta,horaalta,fechacp,horacp,fechamod,horamod,terminal,embarque,tipofig,chofer,rfcfig,numlicfig,idconfigvehicular,configvehicular, version) \
					SELECT @folio,v.referencia,pcfd.rfcemisor,ccp2.idclavecodpos,'%s','%s',IFNULL(cl.rfc,if(cl.rfc='','XAXX010101000',cl.rfc)), \
					IFNULL(ccp1.idclavecodpos,ccp0.idclavecodpos),'%s','%s',emb.viaembarq,ve.placas,ve.modelo,ve.claveaseguradora,ve.polizaaseguradora, \
					emb.idremolque,ifnull(csr.cvesubtiprem,''),IFNULL(cr.numeroplaca,''),ve.tipopermiso,ve.numpermisosct,@pesototmerc,@ccveunpes,@pesototmerc,@numtotmerc,'%s', \
					'%s','%s','%s','%s','%s','%s','%s','%s',v.embarque, @ccvetipofig,chof.empleado, emp.rfc, chof.numlicencia, cca.idclaveautotrans, cca.cveautotrans, %f \
					FROM pedidosventa v \
					INNER JOIN clientes cl ON cl.cliente=v.cliente \
					INNER JOIN terminales ter ON ter.terminal=v.terminal \
					INNER JOIN secciones sec ON ter.seccion=sec.seccion \
					INNER JOIN sucursales su ON sec.sucursal=su.sucursal \
					LEFT JOIN embarques emb ON emb.embarque=v.embarque \
					LEFT JOIN viasembarque ve ON ve.viaembarq=emb.viaembarq \
					LEFT JOIN cconfigautotransporte cca ON cca.idclaveautotrans = ve.configvehicular \
					LEFT JOIN catalogoremolques cr ON cr.idremolque=ve.idremolque \
					LEFT JOIN ccpsubtiporem csr ON csr.idclavesubtiprem=cr.ccvesubtiporem \
					LEFT JOIN parametroscfd pcfd ON pcfd.sucursal=su.sucursal \
					LEFT JOIN empleados emp on emb.chofer=emp.empleado \
					LEFT JOIN choferes chof on emb.chofer=chof.empleado \
					left join pedidosdirent pd on pd.referencia=v.referencia \
					left JOIN ccpclavecodpos ccp0 ON ccp0.ccodigopostal=cl.cp \
					left JOIN ccpclavecodpos ccp1 ON ccp1.ccodigopostal=pd.cp \
					INNER JOIN ccpclavecodpos ccp2 ON ccp2.ccodigopostal=pcfd.cpexp	\
					WHERE v.referencia='%s'",
					mFg.DateToMySqlDate(fecha),mFg.TimeToMySqlTime(hora), //fecha y hora de salida
					FechaLLegada, HoraLLegada, //fecha y hora de llegada (mismo dia, hora de alta mas 12 horas)
					usuario,usuario,
					mFg.DateToMySqlDate(fecha),mFg.TimeToMySqlTime(hora), //fecha y hora de alta
					mFg.DateToMySqlDate(fecha),mFg.TimeToMySqlTime(hora), //fecha y hora de carta porte
					mFg.DateToMySqlDate(fecha),mFg.TimeToMySqlTime(hora), //fecha y hora de modificación
					terminal,VersCartaPorte, referencia);
					instrucciones[num_instrucciones++]=instruccion;

					instruccion.sprintf("INSERT INTO dcartasporte20(cartaporte20,bienestransp,descripcion,cantidad,claveunidad,pesokg,articulo) \
					SELECT @folio,ccp.idclaveprodserv,CONCAT(TRIM(p.nombre),' ',a.present,' ',a.multiplo),dv.cantidad,cu.idclaveunidad,(a.peso*dv.cantidad),dv.articulo \
					FROM dpedidosventa dv \
					INNER JOIN articulos a ON a.articulo=dv.articulo \
					INNER JOIN productos p ON p.producto=a.producto \
					INNER JOIN cclaveunidad cu ON cu.idclaveunidad=a.idclaveunidadcfdi \
					INNER JOIN cclaveprodserv ccp ON ccp.idclaveprodserv=p.idclaveprodservcfdi \
					WHERE dv.referencia='%s'",referencia);
					instrucciones[num_instrucciones++]=instruccion;

				}else if (tipo=="PED_COMPRA"){

					instruccion.sprintf("SET @numtotmerc=(SELECT SUM(cantidad) FROM dpedidos WHERE referencia='%s')",referencia);
					instrucciones[num_instrucciones++]=instruccion;

					instruccion.sprintf("SET @pesototmerc=(SELECT sum(a.peso*p.cantidad) FROM dpedidos p \
											INNER JOIN articulos a ON a.articulo=p.articulo \
											WHERE p.referencia='%s')",referencia);
					instrucciones[num_instrucciones++]=instruccion;

					instruccion.sprintf("SELECT @viaembarque:=ve.viaembarq,@placavm:=ve.placas,@modelovm:=ve.modelo,	\
					@cveaseg:=ve.claveaseguradora,@poliza:=ve.polizaaseguradora,@subtiprem:=IFNULL(csr.cvesubtiprem,''),    \
					@numplarem:=IFNULL(cr.numeroplaca,''),@tipperm:=ve.tipopermiso,@numperm:=ve.numpermisosct   \
					FROM viasembarque ve    \
					LEFT JOIN catalogoremolques cr ON ve.idremolque=cr.idremolque   \
					LEFT JOIN ccpsubtiporem csr ON csr.idclavesubtiprem=cr.ccvesubtiporem   \
					WHERE ve.viaembarq='%s'",viaembarque);
					instrucciones[num_instrucciones++]=instruccion;

					instruccion.sprintf("SELECT @subtiprem:=IFNULL(csr.cvesubtiprem,''), @numplarem:=IFNULL(cr.numeroplaca,'')	\
					FROM  catalogoremolques cr \
					LEFT JOIN ccpsubtiporem csr ON csr.idclavesubtiprem=cr.ccvesubtiporem   \
					WHERE cr.idremolque ='%s'",remolque);
					instrucciones[num_instrucciones++]=instruccion;

					instruccion.sprintf("SELECT @rfcfig:=emp.rfc,@numjlicfig:=ch.numlicencia	\
					FROM choferes ch    \
					INNER JOIN empleados emp ON emp.empleado=ch.empleado    \
					WHERE ch.empleado='%s'",empleado);
					instrucciones[num_instrucciones++]=instruccion;

					instruccion.sprintf("INSERT INTO pedcompdirorigen   \
						(referencia,proveedor,calle,colonia,id_cp,ubicaciongis,fechaalta)   \
						SELECT referencia,proveedor,'%s', '%s',    \
						(SELECT idclavecodpos FROM ccpclavecodpos WHERE ccodigopostal='%s'), \
						POINT(%s),'%s' \
						FROM pedidos WHERE referencia='%s'",calleNumOrigen,coloniaOrigen,codPosOrigen,
						ubicaciongis,mFg.DateToMySqlDate(fecha),referencia);
					instrucciones[num_instrucciones++]=instruccion;

					instruccion.sprintf("INSERT INTO cartasporte20(cartaporte20,pedidocompra,rfcremitente,cpremitente,fechasalida,horasalida, \
					rfcdestinatario,cpdestinatario,fechallegada,horallegada,viaembarq,placavm,aniomodelovm,asegurcivil,polizarespcivil, \
					idremolque,tiporemolque,placaremolque,permsct,numpermsct,pesobruttotal,unidadpeso,pesonettot,numtotmerc,usualta, \
					usumodi,fechaalta,horaalta,fechacp,horacp,fechamod,horamod,terminal,tipofig,chofer,rfcfig,numlicfig,idconfigvehicular,configvehicular, version) \
					SELECT @folio,p.referencia,IFNULL(prv.rfc,if(prv.rfc<>'','XAXX010101000',prv.rfc))	\
					,(SELECT idclavecodpos FROM ccpclavecodpos WHERE ccodigopostal='%s') \
					,'%s','%s', pcfd.rfcemisor,ccp2.idclavecodpos,'%s','%s',	\
					@viaembarque,@placavm,@modelovm,@cveaseg,@poliza,%s,@subtiprem,@numplarem,@tipperm,@numperm,   \
					@pesototmerc,@ccveunpes,@pesototmerc,@numtotmerc,'%s','%s', \
					'%s','%s', '%s' ,'%s',    \
					'%s','%s','%s',@ccvetipofig,'%s',@rfcfig,@numjlicfig, %s, '%s', %f \
					FROM pedidos p  \
					INNER JOIN proveedores prv ON p.proveedor=prv.proveedor \
					INNER JOIN almacenes alm ON p.almacen=alm.almacen   \
					INNER JOIN secciones sec ON alm.seccion=sec.seccion \
					INNER JOIN sucursales suc ON sec.sucursal=suc.sucursal  \
					INNER JOIN parametroscfd pcfd ON suc.sucursal=pcfd.sucursal \
					INNER JOIN ccpclavecodpos ccp2 ON ccp2.ccodigopostal=pcfd.cpexp	\
					WHERE p.referencia='%s'",
                    codPosOrigen,
					mFg.DateToMySqlDate(fecha),mFg.TimeToMySqlTime(hora), //fecha y hora de salida
					FechaLLegada, HoraLLegada, //fecha y hora de llegada (mismo dia, hora de alta mas 12 horas)
					remolque,
					usuario,usuario,
					mFg.DateToMySqlDate(fecha),mFg.TimeToMySqlTime(hora), //fecha y hora de alta
					mFg.DateToMySqlDate(fecha),mFg.TimeToMySqlTime(hora), //fecha y hora de carta porte
					mFg.DateToMySqlDate(fecha),mFg.TimeToMySqlTime(hora), //fecha y hora de modificación
					terminal,empleado,idconfigvehicular, claveconfigvehicular,VersCartaPorte,referencia);
					instrucciones[num_instrucciones++]=instruccion;

					instruccion.sprintf("INSERT INTO dcartasporte20(cartaporte20,bienestransp,descripcion,cantidad,claveunidad,pesokg,articulo) \
					SELECT @folio,ccp.idclaveprodserv,CONCAT(TRIM(p.nombre),' ',a.present,' ',a.multiplo),dp.cantidad,cu.idclaveunidad,(a.peso*dp.cantidad),dp.articulo \
					FROM dpedidos dp \
					INNER JOIN articulos a ON a.articulo=dp.articulo \
					INNER JOIN productos p ON p.producto=a.producto \
					INNER JOIN cclaveunidad cu ON cu.idclaveunidad=a.idclaveunidadcfdi \
					INNER JOIN cclaveprodserv ccp ON ccp.idclaveprodserv=p.idclaveprodservcfdi \
					WHERE dp.referencia='%s'",referencia);
					instrucciones[num_instrucciones++]=instruccion;

				}
			}

			// Crea el buffer con todas las instrucciones SQL
			aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
			for (i=0; i<num_instrucciones; i++)
				aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

			mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
			if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
				if (error==0) {

					char *buffer_carta_porte2=new char[1024*64*10];
					char *aux_buffer_carta_porte2=buffer_carta_porte2;
					////////////////////////////////////////////////////////////////
					// Crea el CFD (si la configuracion así lo indica) y hace commit
					ComprobanteFiscalDigital *cfd=NULL;
					try {
						cfd=new ComprobanteFiscalDigital(mServidorVioleta);

						aux_buffer_carta_porte2=mFg.AgregaStringABuffer(tipo, aux_buffer_carta_porte2);

						if (cfd->EmitirCFDI40(Respuesta, MySQL, "CART2")) {
							cfd->CreaCFDICartaPorte40(Respuesta, MySQL, buffer_carta_porte2);
						} else {
							cfd->CreaCFDICartaPorte20(Respuesta, MySQL, buffer_carta_porte2);
						}
					}
					__finally {
						if(cfd!=NULL) delete cfd;
						if(buffer_carta_porte2!=NULL) delete buffer_carta_porte2;
					}
					instruccion="COMMIT";
					if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
						throw Exception("No se pudo hacer commit");
					///////////////////////////////////////////////////////////////                                                        */
					instruccion.sprintf("select %d as error, c.referencia as folio, c.fechacp, c.horacp from cartasporte20 c where c.referencia=@folio", error);
				} else {
					instruccion.sprintf("select %d as error", error);
				}
				mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
			}
		}__finally {
			delete buffer_sql;
		}
}


//----------------------------------------------------------------------------
//ID_CANC_CARTAPORTE_V20
void ServidorVentas::CancelaCartaPorte_V20(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  CANCELA CARTA PORTE V20
	char *buffer_sql=new char[1024*32];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instrucciones[50], instruccion;
	int num_instrucciones=0;
	AnsiString referencia, usuario, f_cancelar;
	bool resultado_cancelacion_paq=false;
	int error=0;
	int i;
	TDate fecha=Today();
	TTime hora=Time();

	try{
		referencia=mFg.ExtraeStringDeBuffer(&parametros); //  (referencia CARTA PORTE )
		usuario=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que esta cancelando la venta.
		f_cancelar =mFg.ExtraeStringDeBuffer(&parametros);// parametro de forzar cancelar pago de clientes

		// Verifica que la fecha de la carta porte sea posterior a la fecha de cierre.
		//instruccion.sprintf("select @error:=if(cp.fechacp<=cast(e.valor as datetime), 1, 0) as error from cartasporte cp left join estadosistema as e on e.estado='FUCIERRE' where cp.referencia='%s'", clave);
		//mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 3, error);

		if (error==0) {
            // Cancela el CFDI con el PAC
			// (si no se trata de un CFDI simplemente no se hace nada con ningún PAC)
			// Si no tiene CFDI relacionado tampoco se hace nada con ningún PAC.
			// SOLO CUANDO NO ESTA EN DEPURACION
		   if (f_cancelar == "0"){
				#ifndef _DEBUG
				ComprobanteFiscalDigital *cfd=NULL;
				try {
					cfd=new ComprobanteFiscalDigital(mServidorVioleta);
					#ifdef _DEBUG
						mFg.AppMessageBox("Se está cancelando con el pac en modo de depuración, esto sólo se deberia de hacer antes de liberar a produccion en cartas porte","ALERTA!!!",MB_OK);
					#endif
					cfd->cancelarCFDI(Respuesta, MySQL, "CAR2",referencia,"","02","");
					// ObtieneResultadoCancelacion() solo regresa true cuando realmente se canceló el CFDI con el paq.
					// Por ejemplo cuando no se encontró registro a cancelar no hay error, pero se regresa false, esto
					// para asegurar si realmente ocurrió una cancelación con el PAQ
					// (en caso de sustitución de CFDI cancelados esto se requiere)
					resultado_cancelacion_paq=cfd->ObtieneResultadoCancelacion();
				} __finally {
					if(cfd!=NULL) delete cfd;
				}
				#endif


				// Cuando estamos en depuráción marca como sí ya se hubiera cancelado correctamente con el PAQ (para pruebas).
//				#ifdef _DEBUG
				resultado_cancelacion_paq=true;
//				#endif
			}
			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

			// Cancela el CFD
			instruccion.sprintf("update cartasporte20 set cancelada=1, fechamod='%s', horamod='%s', usumodi='%s' \
			where cartaporte20='%s'",
			mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora),usuario, referencia);
			instrucciones[num_instrucciones++]=instruccion;

			// Cancela el CFD
			instruccion.sprintf("update cfd set estado=0, fechacancelamin=curdate(), fechacancela='%s %s' where referencia='%s' and tipocomprobante='CAR2'", mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), referencia);
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
//ID_CON_CARTAPORTE_V20
void ServidorVentas::ConsultaCartaPorte_V20(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA CARTA PORTE V20
	AnsiString instruccion;
	AnsiString referecia,tipo,soloCP20Activa,condicionSoloCP20Activa=" ";
	AnsiString columnaTipo="";

	referecia=mFg.ExtraeStringDeBuffer(&parametros);
	tipo=mFg.ExtraeStringDeBuffer(&parametros);
	soloCP20Activa=mFg.ExtraeStringDeBuffer(&parametros);
	if (soloCP20Activa=="true") {
		condicionSoloCP20Activa="and cp20.cancelada=0 ";
	}
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_REP_GRANDE);

	if (tipo=="VENTA") {
		// Obtiene todos los generales (de cabecera) de la carta porte
		instruccion.sprintf("SELECT v.referencia AS refevta,'' AS refped,v.embarque AS embvta, \
		CONCAT(cfdvta.serie,cfdvta.folio) AS CFDIvta, 	v.fechavta as fechavta, v.cancelado AS vtacancelada, \
		IFNULL(cp20.cartaporte20,'N/A') AS refcp20, IFNULL(cfdcp.seriefolio,'N/A') AS cfdcp20,  \
		IFNULL(cp20.fechacp,'N/A') AS fechacp,IFNULL(cp20.horacp,'N/A') AS horacp, IFNULL(cp20.cancelada,0) AS cp20cancelada, \
		suc.sucursal AS sucorigen,suc.nombre AS norigen, CONCAT(pcfd.calleexp,' ',pcfd.numextexp) AS cynorigen, \
		pcfd.coloniaexp AS colorigen, pcfd.localidadexp AS nlocorigen, 	pcfd.municipioexp AS munOrigen,  \
		pcfd.estadoexp AS nestadoorigen,pcfd.rfcemisor AS rfcRemitente, cp20.fechasalida,cp20.horasalida,	\
		COALESCE(ccp1.ccodigopostal,pcfd.cpexp) as cporigen, 	\
		v.cliente AS destino,COALESCE(cli.rsocial, CONCAT(cli.nombre,' ',cli.appat,' ',cli.apmat)) AS nDestino, \
		CONCAT(vdir.calle,' ',vdir.numext) AS CyNDestino,col2.nombre AS colDestino, ifnull(loc2.nombre,'-') AS locDestino, \
		ifnull(mun2.municipio,'-') AS mundestino,est2.nombre AS nomEstDestino,IF(cli.rfc='','XAXX010101000',cli.rfc) AS rfcDestinatario, \
		IFNULL(cp20.fechallegada,'N/A') AS fechallegada, IFNULL(cp20.horallegada,'N/A') AS horallegada, \
		COALESCE(ccp2.ccodigopostal,vdir.cp) as cpdestino, 	\
		COALESCE(cp20.placavm,vemb.placas,'N/A') AS placavm, \
		COALESCE(cp20.aniomodelovm,vemb.modelo,'N/A') AS aniomodelovm, COALESCE(CONCAT(ccat.cveautotrans,' - ',ccat.descripcion),'N/A') AS confvehicular, \
		ccat.idclaveautotrans AS clvcveh, \
		COALESCE(cp20.asegurcivil,caseg.nombre,'N/A') AS asegurcivil, \
		COALESCE(cp20.polizarespcivil,vemb.polizaaseguradora,'N/A') AS polizarespcivil, \
		COALESCE(if(cp20.tiporemolque='',NULL,cp20.tiporemolque),csr.remolque,'N/A') AS tiporemolque, \
		COALESCE(if(cp20.placaremolque='',NULL,cp20.placaremolque),cr.numeroplaca,'N/A') AS placaremolque, \
		COALESCE(cp20.permsct,'N/A') AS permsct, COALESCE(cp20.numpermsct,vemb.numpermisosct,'N/A') AS numpermsct, \
		IFNULL(cp20.tipofig,'N/A') AS tipofigura,COALESCE(cp20.rfcfig,chof2.rfc,'N/A') AS rfcFigura,COALESCE(cp20.numlicfig,dchof2.numlicencia,'N/A') AS nlicencia, \
		COALESCE(CONCAT(chof.nombre,' ',chof.appat,' ',chof.apmat),CONCAT(chof2.nombre,' ',chof2.appat,' ',chof2.apmat),'N/A') AS nfigura, \
		cfdcp.folio AS cfdfolio,cfdcp.serie AS cfdserie, cfdxml.xmlgenerado AS cfdxml,cfdxml.cadenaoriginal AS cfdcadenaoriginal, \
		cfdcp.version, cfdcp.muuid, cfdcp.pactimbrador, cp20.cancelada, cp20.version AS versioncarta \
		FROM ventas v \
		LEFT JOIN cfd cfdvta ON cfdvta.referencia=v.referencia AND cfdvta.tipocomprobante = 'VENT' \
		LEFT JOIN cartasporte20 cp20 ON v.referencia=cp20.referencia  %s  \
		LEFT JOIN cfd cfdcp ON cfdcp.referencia=cp20.cartaporte20 AND cfdcp.tipocomprobante = 'CAR2'    \
		LEFT JOIN cfdxml ON cfdcp.compfiscal=cfdxml.compfiscal  \
		LEFT JOIN terminales ter ON ter.terminal=v.terminal \
		LEFT JOIN secciones sec ON sec.seccion=ter.seccion  \
		LEFT JOIN sucursales suc ON suc.sucursal=sec.sucursal   \
		LEFT JOIN colonias col1 ON col1.colonia=suc.colonia \
		LEFT JOIN localidades loc1 ON loc1.localidad=col1.localidad \
		LEFT JOIN municipios mun1 ON mun1.municipio=loc1.municipio  \
		LEFT JOIN estados est1 ON est1.estado=mun1.municipio    \
		LEFT JOIN parametroscfd pcfd ON pcfd.sucursal=suc.sucursal  \
		INNER JOIN clientes cli ON cli.cliente=v.cliente    \
		LEFT JOIN ventadirent vdir ON vdir.referencia=v.referencia  \
		LEFT JOIN colonias col2 ON col2.colonia=vdir.colonia    \
		LEFT JOIN localidades loc2 ON loc2.localidad=col2.localidad \
		LEFT JOIN municipios mun2 ON mun2.municipio=loc2.municipio  \
		LEFT JOIN estados est2 ON est2.estado=mun2.estado   \
		LEFT JOIN embarques emb ON emb.embarque=v.embarque \
		LEFT JOIN viasembarque vemb ON vemb.viaembarq=emb.viaembarq \
		LEFT JOIN catalogoaseguradoras caseg ON caseg.clave=vemb.claveaseguradora \
		LEFT JOIN catalogoremolques cr ON cr.idremolque=emb.idremolque  \
		LEFT JOIN ccpsubtiporem csr ON csr.idclavesubtiprem=cr.ccvesubtiporem  \
		LEFT JOIN empleados chof ON chof.empleado=cp20.chofer \
		left JOIN choferes dchof ON dchof.empleado=chof.empleado AND cp20.numlicfig=dchof.numlicencia \
		left JOIN empleados chof2 ON chof2.empleado=emb.chofer \
		left JOIN choferes dchof2 ON dchof2.empleado=chof2.empleado \
		LEFT JOIN cconfigautotransporte ccat ON ccat.idclaveautotrans=vemb.configvehicular \
		LEFT JOIN ccpclavecodpos ccp1 on ccp1.idclavecodpos=cp20.cpremitente  \
		LEFT JOIN ccpclavecodpos ccp2 on ccp2.idclavecodpos=cp20.cpdestinatario  \
		WHERE v.referencia='%s' \
		ORDER BY cp20.fechamod DESC,cp20.fechacp DESC, cp20.horacp DESC LIMIT 1",condicionSoloCP20Activa,referecia);
		columnaTipo="referencia";
	}else if (tipo=="PEDIDO"){
		// Obtiene todos los generales (de cabecera) de la carta porte
		instruccion.sprintf("SELECT '' AS refevta,p.referencia AS refped,p.embarque AS embvta, CONCAT(cfdvta.serie,cfdvta.folio) AS CFDIvta,    \
		p.fechaped as fechavta, p.cancelado AS vtacancelada, IFNULL(cp20.cartaporte20,'N/A') AS refcp20, IFNULL(cfdcp.seriefolio,'N/A') AS cfdcp20, \
		IFNULL(cp20.fechacp,'N/A') AS fechacp, IFNULL(cp20.horacp,'N/A') AS horacp, IFNULL(cp20.cancelada,0) AS cp20cancelada, \
		suc.sucursal AS sucorigen,suc.nombre AS norigen,CONCAT(pcfd.calleexp,' ',pcfd.numextexp) AS cynorigen, \
		pcfd.coloniaexp AS colorigen, pcfd.localidadexp AS nlocorigen, pcfd.municipioexp AS munOrigen, pcfd.estadoexp AS nestadoorigen, \
		pcfd.rfcemisor AS rfcRemitente, cp20.fechasalida,cp20.horasalida,	\
		COALESCE(ccp1.ccodigopostal,pcfd.cpexp) as cporigen, 	\
		p.cliente AS destino, COALESCE(cli.rsocial,   \
		CONCAT(cli.nombre,' ',cli.appat,' ',cli.apmat)) AS nDestino, CONCAT(pdir.calle,' ',pdir.numext) AS CyNDestino,	\
		col2.nombre AS colDestino, ifnull(loc2.nombre,'-') AS locDestino, ifnull(mun2.municipio,'-') AS mundestino,est2.nombre AS nomEstDestino, \
		cli.rfc AS rfcDestinatario, IFNULL(cp20.fechallegada,'N/A') AS fechallegada, IFNULL(cp20.horallegada,'N/A') AS horallegada, \
		COALESCE(ccp2.ccodigopostal,pdir.cp) as cpdestino, 	\
		COALESCE(cp20.placavm,vemb.placas,'N/A') AS placavm, COALESCE(cp20.aniomodelovm,vemb.modelo,'N/A') AS aniomodelovm,	\
		COALESCE(CONCAT(ccat.cveautotrans,' - ',ccat.descripcion),'N/A') AS confvehicular, \
		COALESCE(cp20.asegurcivil,caseg.nombre,'N/A') AS asegurcivil, COALESCE(cp20.polizarespcivil,vemb.polizaaseguradora,'N/A') AS polizarespcivil, \
		COALESCE(if(cp20.tiporemolque='',NULL,cp20.tiporemolque),csr.remolque,'N/A') AS tiporemolque, \
		COALESCE(if(cp20.placaremolque='',NULL,cp20.placaremolque),cr.numeroplaca,'N/A') AS placaremolque, \
		COALESCE(cp20.permsct,'N/A') AS permsct, COALESCE(cp20.numpermsct,vemb.numpermisosct,'N/A') AS numpermsct, \
		ccat.idclaveautotrans AS clvcveh, \
		IFNULL(cp20.tipofig,'N/A') AS tipofigura,COALESCE(cp20.rfcfig,chof2.rfc,'N/A') AS rfcFigura,COALESCE(cp20.numlicfig,dchof2.numlicencia,'N/A') AS nlicencia, \
		COALESCE(CONCAT(chof.nombre,' ',chof.appat,' ',chof.apmat),CONCAT(chof2.nombre,' ',chof2.appat,' ',chof2.apmat),'N/A') AS nfigura, \
		cfdcp.folio AS cfdfolio, cfdcp.serie AS cfdserie, cfdxml.xmlgenerado AS cfdxml, cfdxml.cadenaoriginal AS cfdcadenaoriginal, \
		cfdcp.version, cfdcp.muuid, cfdcp.pactimbrador, cp20.cancelada, cp20.version AS versioncarta  \
		FROM pedidosventa p \
		LEFT JOIN cfd cfdvta ON cfdvta.referencia=p.referencia AND cfdvta.tipocomprobante not in ('CAR2','NCAR','NCRE','PAGO','TICK','VENT')  \
		LEFT JOIN cartasporte20 cp20 ON p.referencia=cp20.pedido  %s  \
		LEFT JOIN cfd cfdcp ON cfdcp.referencia=cp20.cartaporte20 AND cfdcp.tipocomprobante = 'CAR2'    \
		LEFT JOIN cfdxml ON cfdcp.compfiscal=cfdxml.compfiscal  \
		LEFT JOIN terminales ter ON ter.terminal=p.terminal \
		LEFT JOIN secciones sec ON sec.seccion=ter.seccion  \
		LEFT JOIN sucursales suc ON suc.sucursal=sec.sucursal   \
		LEFT JOIN colonias col1 ON col1.colonia=suc.colonia \
		LEFT JOIN localidades loc1 ON loc1.localidad=col1.localidad \
		LEFT JOIN municipios mun1 ON mun1.municipio=loc1.municipio  \
		LEFT JOIN estados est1 ON est1.estado=mun1.municipio    \
		LEFT JOIN parametroscfd pcfd ON pcfd.sucursal=suc.sucursal  \
		INNER JOIN clientes cli ON cli.cliente=p.cliente    \
		LEFT JOIN pedidosdirent pdir ON pdir.referencia=p.referencia    \
		LEFT JOIN colonias col2 ON col2.colonia=pdir.colonia    \
		LEFT JOIN localidades loc2 ON loc2.localidad=col2.localidad \
		LEFT JOIN municipios mun2 ON mun2.municipio=loc2.municipio  \
		LEFT JOIN estados est2 ON est2.estado=mun2.estado   \
		LEFT JOIN embarques emb ON emb.embarque=p.embarque  \
		LEFT JOIN viasembarque vemb ON vemb.viaembarq=emb.viaembarq \
		LEFT JOIN catalogoaseguradoras caseg ON caseg.clave=vemb.claveaseguradora   \
		LEFT JOIN catalogoremolques cr ON cr.idremolque=vemb.idremolque  \
		LEFT JOIN ccpsubtiporem csr ON csr.idclavesubtiprem=cr.ccvesubtiporem  \
		LEFT JOIN empleados chof ON chof.empleado=cp20.chofer \
		left JOIN choferes dchof ON dchof.empleado=chof.empleado AND cp20.numlicfig=dchof.numlicencia \
		left JOIN empleados chof2 ON chof2.empleado=emb.chofer \
		left JOIN choferes dchof2 ON dchof2.empleado=chof2.empleado \
		LEFT JOIN cconfigautotransporte ccat ON ccat.idclaveautotrans=vemb.configvehicular \
		LEFT JOIN ccpclavecodpos ccp1 on ccp1.idclavecodpos=cp20.cpremitente  \
		LEFT JOIN ccpclavecodpos ccp2 on ccp2.idclavecodpos=cp20.cpdestinatario  \
		WHERE p.referencia='%s' \
		ORDER BY cp20.fechamod DESC,cp20.fechacp DESC, cp20.horacp DESC LIMIT 1",condicionSoloCP20Activa, referecia);
        columnaTipo="pedido";
	}else if (tipo=="PED_COMPRA"){
		// Obtiene todos los generales (de cabecera) de la carta porte
		instruccion.sprintf("SELECT '' AS refevta,p.referencia AS refped,'' AS embvta, IFNULL(c.folioprov,'') AS CFDIvta,	\
		p.fechaped AS fechavta, p.cancelado AS vtacancelada,    \
		IFNULL(cp20.cartaporte20,'N/A') AS refcp20, IFNULL(cfdcp.seriefolio,'N/A') AS cfdcp20, \
		IFNULL(cp20.fechacp,'N/A') AS fechacp, IFNULL(cp20.horacp,'N/A') AS horacp, IFNULL(cp20.cancelada,0) AS cp20cancelada,	\
		p.proveedor AS sucorigen, prov.razonsocial AS norigen, COALESCE(ccp.ccodigopostal,prov.cp) AS cporigen, \
		COALESCE(pcdo.calle,prov.calle) AS cynorigen, COALESCE(pcdo.colonia,'-') AS cvecolorigen, \
		IFNULL(colorig.nombre,'-') AS colorigen,loc3.localidad AS cvelocorigen,  \
		IFNULL(loc3.nombre,'-') AS nlocorigen,mun3.municipio AS cvemunorigen,  \
		IFNULL(mun3.nombre,'-') AS munOrigen,est3.estado AS cveestorigen,   \
		COALESCE (est3.nombre,prov.estado) AS nestadoorigen,X(pcdo.ubicaciongis) AS latitud,Y(pcdo.ubicaciongis) AS longitud,   \
		prov.rfc AS rfcRemitente, cp20.fechasalida, cp20.horasalida, \
		pcfddes.sucursal AS destino, CONCAT(pcfddes.nombreemisor,' - ',alm.nombre) AS nDestino,pcfddes.cpexp AS cpdestino, \
		concat(pcfddes.calleexp,' ',pcfddes.numextexp)  AS CyNDestino, pcfddes.coloniaexp AS colDestino,     \
		pcfddes.localidadexp AS locDestino,  \
		ccat.idclaveautotrans AS clvcveh, \
		 pcfddes.municipioexp AS mundestino,pcfddes.estadoexp AS nomEstDestino, pcfddes.rfcemisor AS rfcDestinatario, \
		IFNULL(cp20.fechallegada,'N/A') AS fechallegada, IFNULL(cp20.horallegada,'N/A') AS horallegada,    \
		cp20.placavm AS placavm, cp20.aniomodelovm AS aniomodelovm, \
		COALESCE(CONCAT(ccat.cveautotrans,' - ',ccat.descripcion),'N/A') AS confvehicular, \
		COALESCE(cp20.asegurcivil,caseg.nombre,'N/A') AS asegurcivil, COALESCE(cp20.polizarespcivil,   \
		vemb.polizaaseguradora,'N/A') AS polizarespcivil,  \
		COALESCE(if(cp20.tiporemolque='', NULL,cp20.tiporemolque),csr.remolque,'N/A') AS tiporemolque, \
		COALESCE(if(cp20.placaremolque='', NULL,cp20.placaremolque),cr.numeroplaca,'N/A') AS placaremolque,    \
		COALESCE(cp20.permsct,'N/A') AS permsct, COALESCE(cp20.numpermsct,vemb.numpermisosct,'N/A') AS numpermsct, \
		IFNULL(cp20.tipofig,'N/A') AS tipofigura, COALESCE(cp20.rfcfig,'N/A') AS rfcFigura, 	\
		COALESCE(cp20.numlicfig,'N/A') AS nlicencia, COALESCE(CONCAT(chof.nombre,' ',chof.appat,' ',chof.apmat),'N/A') AS nfigura, \
		cfdcp.folio AS cfdfolio, cfdcp.serie AS cfdserie, cfdxml.xmlgenerado AS cfdxml, cfdxml.cadenaoriginal AS cfdcadenaoriginal, \
		cfdcp.version, cfdcp.muuid, cfdcp.pactimbrador, cp20.cancelada, cp20.version AS versioncarta \
		FROM pedidos p  \
		LEFT JOIN compras c ON c.referencia=p.compra    \
		LEFT JOIN cartasporte20 cp20 ON cp20.pedidocompra=p.referencia  %s\
		LEFT JOIN cfd cfdcp ON cfdcp.referencia=cp20.cartaporte20 AND cfdcp.tipocomprobante = 'CAR2'    \
		LEFT JOIN cfdxml ON cfdcp.compfiscal=cfdxml.compfiscal  \
		LEFT JOIN proveedores prov ON prov.proveedor=p.proveedor    \
		LEFT JOIN almacenes alm ON p.almacen=alm.almacen    \
		LEFT JOIN secciones secdes ON secdes.seccion=alm.seccion    \
		LEFT JOIN sucursales sucdes ON sucdes.sucursal=secdes.sucursal  \
		LEFT JOIN localidades loc2 ON loc2.localidad=sucdes.localidad   \
		LEFT JOIN municipios mun2 ON mun2.municipio=loc2.municipio  \
		LEFT JOIN estados est2 ON est2.estado=mun2.estado   \
		LEFT JOIN parametroscfd pcfddes ON pcfddes.sucursal=sucdes.sucursal \
		LEFT JOIN viasembarque vemb ON vemb.viaembarq=cp20.viaembarq    \
		LEFT JOIN catalogoaseguradoras caseg ON caseg.clave=vemb.claveaseguradora   \
		LEFT JOIN catalogoremolques cr ON cr.idremolque=vemb.idremolque \
		LEFT JOIN ccpsubtiporem csr ON csr.idclavesubtiprem=cr.ccvesubtiporem   \
		LEFT JOIN empleados chof ON chof.empleado=cp20.chofer    \
		left JOIN choferes dchof ON dchof.empleado=chof.empleado AND cp20.numlicfig=dchof.numlicencia   \
		LEFT JOIN cconfigautotransporte ccat ON ccat.idclaveautotrans=vemb.configvehicular  \
		LEFT JOIN (SELECT id_pedcompdir,referencia,proveedor,calle,colonia,id_cp,ubicaciongis,fechaalta  \
		FROM pedcompdirorigen WHERE referencia='%s' ORDER BY id_pedcompdir DESC LIMIT 1) pcdo ON pcdo.referencia=p.referencia \
		LEFT JOIN ccpclavecodpos ccp ON ccp.idclavecodpos=cp20.cpremitente  \
		LEFT JOIN colonias colorig ON colorig.colonia=pcdo.colonia  \
		LEFT JOIN localidades loc3 ON loc3.localidad=colorig.localidad  \
		LEFT JOIN municipios mun3 ON mun3.municipio=loc3.municipio  \
		LEFT JOIN estados est3 ON est3.estado=mun3.estado   \
		WHERE p.referencia='%s'    \
		ORDER BY cp20.fechamod DESC,cp20.fechacp DESC, cp20.horacp DESC LIMIT 1",condicionSoloCP20Activa, referecia,referecia);
		columnaTipo="pedidocompra";
	}
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene los datos conceptos que se declaran en la carta porte
	instruccion="SELECT dcp20.*,ccps.cveprodserv AS ccveBienes,ccps.descripcion AS ccvebienesDesc, ";
	instruccion+="ccun.claveunidad AS ccveunidad,ccun.nombre AS ccveunidadnom ";
	instruccion+=" FROM dcartasporte20 dcp20 ";
	instruccion+=" INNER JOIN cclaveprodserv ccps ON dcp20.bienestransp=ccps.idclaveprodserv ";
	instruccion+=" INNER JOIN cclaveunidad ccun ON dcp20.claveunidad=ccun.idclaveunidad ";
	instruccion+=" WHERE dcp20.cartaporte20=(SELECT max(cp20.cartaporte20) FROM cartasporte20 cp20 WHERE cp20."+ columnaTipo+"='"+referecia+"' ";
	instruccion+= condicionSoloCP20Activa+") order by descripcion asc";
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

}
//--------------------------------------------------------------------------------

void ServidorVentas::GuardaenBitacoraWeb(RespuestaServidor *Respuesta, MYSQL *MySQL,
 AnsiString clave_bitacora, AnsiString mensaje_bitacora,AnsiString detalles_bitacora, AnsiString msg)
{
			AnsiString instruccion_bitacora;
			instruccion_bitacora.sprintf("INSERT INTO bitacorafacturacionwebdetalles VALUES (NULL, %s,'%s','%s' )",clave_bitacora, mensaje_bitacora, detalles_bitacora);
			mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion_bitacora.c_str());
			throw (Exception(msg));
}

//--------------------------------------------------------------------------------
//ID_GRA_FACTURA_WEB
void ServidorVentas::GrabaFacturaWeb(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  GRABA FACTURA WEB
	char *buffer_sql=new char[4000*1000];
	char *aux_buffer_sql=buffer_sql;

	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[6000];

	int error=0;
	TDate fecha_vta=Today();
	TTime hora=Time();
	BufferRespuestas* resp_ticket_original=NULL;
	BufferRespuestas* resp_cliente=NULL;
	AnsiString foliofisico;
	AnsiString errorMensaje = "prueba";

	bool cambiaCP = false;
    bool CPIncorrecto = false;
    AnsiString cp_actual = "";

	AnsiString referencia_ticket, rfc_cliente, codigopostal_cliente, usocfdi_cliente;
	AnsiString terminal_ticket, usualta_ticket, ticket_cancelado, refnc="";
	TDate fechavta_ticket;

	int forma_cancelar_ticket=2; // Siempre se debe generar una nota de credito
    bool pendientetimbrar = false;
	int i;
	AnsiString cfdirelacionado;
	AnsiString  alta_cliente, nombres, appate, apmat, razonsocial, terminal_fw, tipoempre_cliente;
	AnsiString paramNuevoMetodo="1", paramNuevaClaveFpago="01", paramNuevosDigitos=" ";
	AnsiString paramUuidRelacionado=" ", paramCampoFiscalRel=" ";

	AnsiString  update_regimen, regimenfiscal, clave_cliente, clave_bitacora, update_user;
	AnsiString instruccion_bitacora, mensaje_bitacora, detalles_bitacora;

	// Recibe los parámetros
	referencia_ticket=mFg.ExtraeStringDeBuffer(&parametros); // Referencia del ticket
	rfc_cliente=mFg.ExtraeStringDeBuffer(&parametros); // rfc del cliente
	codigopostal_cliente=mFg.ExtraeStringDeBuffer(&parametros); // código postal del cliente
	usocfdi_cliente=mFg.ExtraeStringDeBuffer(&parametros); // usocfdi del cliente

	alta_cliente=mFg.ExtraeStringDeBuffer(&parametros); // "1" cuando se requiere dar de alta el cliente

	update_regimen=mFg.ExtraeStringDeBuffer(&parametros); //  Agregar el regimen fiscal al cliente
	regimenfiscal=mFg.ExtraeStringDeBuffer(&parametros); // Regimén fiscal para facturar
	clave_cliente=mFg.ExtraeStringDeBuffer(&parametros); // Clave del usuario que se va a facturar

	clave_bitacora=mFg.ExtraeStringDeBuffer(&parametros); // Clave de la bitacora que se creo en el cliente

	update_user=mFg.ExtraeStringDeBuffer(&parametros); // "1" cuando se requiere actualizar datos del cliente
	nombres=mFg.ExtraeStringDeBuffer(&parametros);
	appate=mFg.ExtraeStringDeBuffer(&parametros);
	apmat=mFg.ExtraeStringDeBuffer(&parametros);
	razonsocial=mFg.ExtraeStringDeBuffer(&parametros);
	terminal_fw=mFg.ExtraeStringDeBuffer(&parametros);

	// Limpia el Rfc y nombres, también los cambia a mayúsculas.
	rfc_cliente=rfc_cliente.Trim().UpperCase();
	nombres=nombres.Trim().UpperCase();
	appate=appate.Trim().UpperCase();
	apmat=apmat.Trim().UpperCase();
	razonsocial=razonsocial.Trim().UpperCase();
	clave_cliente=clave_cliente.Trim().UpperCase();

	if (alta_cliente!="1" &&  update_user == "1")  {
		nombres=ReplaceRegExpr("(')", nombres, "''", true);
		appate=ReplaceRegExpr("(')", appate, "''", true);
		apmat=ReplaceRegExpr("(')", apmat, "''", true);
		razonsocial=ReplaceRegExpr("(')", razonsocial, "''", true);
	}

	mensaje_bitacora = "";
	detalles_bitacora = " ";

	AnsiString clave_venta=" ";

	if (rfc_cliente.Length()==13) {
		// Si tiene un rfc de persona fisica se llena la razón social con el nombre y apellidos
		razonsocial=nombres+" "+appate+" "+apmat;
		tipoempre_cliente="0";
	} else {
		if (rfc_cliente.Length()==12) {
			nombres="";
			appate="";
			apmat="";
			tipoempre_cliente="1";
		} else{
			mensaje_bitacora = "RFC_INVALIDO";
			detalles_bitacora = "El RFC del cliente no tiene una longitud válida: "+mFg.IntToAnsiString(rfc_cliente.Length());
			GuardaenBitacoraWeb(Respuesta, MySQL, clave_bitacora, mensaje_bitacora,detalles_bitacora, detalles_bitacora);
		}
	}

	// Limpia el código postal del cliente
	codigopostal_cliente=codigopostal_cliente.Trim().UpperCase();

	try {
		// Valida el rfc
		if ( !mFg.validaRFC(rfc_cliente) ){
			mensaje_bitacora = "RFC_INVALIDO2";
			detalles_bitacora = "El RFC del cliente no es válido: "+rfc_cliente;
			GuardaenBitacoraWeb(Respuesta, MySQL, clave_bitacora, mensaje_bitacora,detalles_bitacora, detalles_bitacora);
		}


		// Valida el código postal
		if ( codigopostal_cliente.Length()==5 && mFg.ValidaNumero(codigopostal_cliente) ){
			mensaje_bitacora = "CPINVALIDO";
            CPIncorrecto = true;
			detalles_bitacora = "El código postal del cliente no es válido: "+codigopostal_cliente;
            GuardaenBitacoraWeb(Respuesta, MySQL, clave_bitacora, mensaje_bitacora,detalles_bitacora, detalles_bitacora);
		}

		// Verifica que se pueda facturar de acuerdo con las restricciones de fecha
        instruccion.sprintf("SELECT ( CASE WHEN (v.fechavta <= CAST(e.valor AS DATETIME)) THEN 1 \
			ELSE CASE WHEN (d.valor <= 0) THEN ( IF( v.fechavta = CURDATE(), 0, 2)) \
			ELSE CASE WHEN (CURDATE() <= limite_factura.fecha_ultima ) THEN 0 \
			ELSE 3  END END END ) AS error \
			FROM ventas v \
			LEFT JOIN estadosistemaemp AS e ON e.estado='FUCIERRE' AND e.sucursal = '%s' \
			LEFT JOIN parametroscfdiweb AS d ON d.parametro='DIASFACTWEB' AND d.idempresa = %s \
			INNER JOIN ( \
				SELECT DATE_FORMAT( \
				CASE WHEN MONTH(DATE_ADD(v.fechavta, INTERVAL dias.valor DAY)) <> MONTH(v.fechavta) AND \
				DAYOFMONTH(DATE_ADD(v.fechavta, INTERVAL dias.valor DAY)) > dfc.valor \
				THEN CONCAT (YEAR(DATE_ADD( v.fechavta, INTERVAL dias.valor DAY)),'-', MONTH(DATE_ADD(v.fechavta, INTERVAL dias.valor DAY)), \
				'-', dfc.valor) \
				ELSE DATE_ADD( v.fechavta, INTERVAL dias.valor DAY) \
				END, '%%Y-%%m-%%d' ) AS fecha_ultima \
				FROM ventas v \
				LEFT JOIN parametroscfdiweb dias ON dias.parametro = 'DIASFACTWEB' AND dias.idempresa = %s \
				LEFT JOIN parametroscfdiweb dfc ON dfc.parametro = 'DIASFUERACIERRE' AND dfc.idempresa = %s \
				WHERE v.referencia = '%s' \
			) AS limite_factura \
			WHERE v.referencia = '%s' ",
			FormServidor->ObtieneClaveSucursal(), FormServidor->ObtieneClaveEmpresa(),
			FormServidor->ObtieneClaveEmpresa(), FormServidor->ObtieneClaveEmpresa(),
			referencia_ticket, referencia_ticket );

		mServidorVioleta->EjecutaVerificacion(Respuesta,  MySQL, instruccion.c_str(), 3, error);

		if (error==0) {
			// Obtiene datos importantes del ticket original.
			instruccion.sprintf("select v.fechavta, v.terminal, v.usualta, @sucursal_ticket:=sec.sucursal, "
				"@referfacturaglobal_orig:=DATE_FORMAT(v.fechavta, '%%d/%%m/%%Y') as referfacturaglobal "
				"from ventas v "
				"inner join terminales t on v.terminal=t.terminal "
				"inner join secciones sec on t.seccion=sec.seccion "
				"where v.referencia='%s' and v.cancelado=0 ",
				 referencia_ticket);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_ticket_original);
			if (resp_ticket_original->ObtieneNumRegistros()==0){
				mensaje_bitacora = "TICKET INACTIVO";
				detalles_bitacora = "No se encontró un ticket activo con el folio "+referencia_ticket;
				GuardaenBitacoraWeb(Respuesta, MySQL, clave_bitacora, mensaje_bitacora,detalles_bitacora, detalles_bitacora);
			}

			fechavta_ticket=StrToDate(resp_ticket_original->ObtieneDato("fechavta"));
			terminal_ticket=resp_ticket_original->ObtieneDato("terminal");
			usualta_ticket=resp_ticket_original->ObtieneDato("usualta");

			// Determina la forma de cancelar el ticket original (1=Cancelar en la tabla ventas, 2=Hacer una nota de crédito)
			/*if (StrToDate(fechavta_ticket)<fecha_vta)
				forma_cancelar_ticket=2; // Si el ticket es de fecha anterior se va a crear nota de crédito
			else
				forma_cancelar_ticket=1; // Si el ticket es de fecha igual se va a cancelar el ticket
			*/

			// DAR DE BAJA LOS TICKETS O GENERAR LA NOTA DE CREDITO
			if (forma_cancelar_ticket == 2 ) {

				//Validación al crear notas de crédito por facturar tickets,
				// no se podrán facturar tickets que previamente se les hayan creado una nota de crédito (lo que significa que ya se hayan facturado).
				BufferRespuestas* resp_ref=NULL;
				try{
					instruccion.sprintf("  SELECT nc.referencia AS refnotcred, nc.cancelado AS cancelado \
					FROM ventas v INNER JOIN terminales t INNER JOIN empleados ec INNER JOIN empleados ev \
					INNER JOIN clientes c   LEFT JOIN cfd ON v.referencia=cfd.referencia AND \
					cfd.tipocomprobante='VENT' LEFT JOIN notascredcli AS nc ON  v.referencia = nc.venta  \
					LEFT JOIN clientesemp cemp ON c.cliente= cemp.cliente \
					WHERE v.referencia='%s' AND v.cliente=c.cliente AND v.terminal=t.terminal AND \
					v.usumodi=ec.empleado AND cemp.vendedor=ev.empleado AND nc.cancelado = '0' AND cemp.idempresa=%s ",referencia_ticket, FormServidor->ObtieneClaveEmpresa());
					mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_ref);
					if (resp_ref->ObtieneNumRegistros()>0){
						 refnc =  resp_ref->ObtieneDato("refnotcred");
					}
				}
				__finally {
					if (resp_ref!=NULL) delete resp_ref;
				}

				if (refnc != "") {
					mensaje_bitacora = "NOTA_CREDITO";
					detalles_bitacora = "No se pueden facturar tickets que tienen asignada una nota de crédito";
					GuardaenBitacoraWeb(Respuesta, MySQL, clave_bitacora, mensaje_bitacora,detalles_bitacora, detalles_bitacora);
				}

				// 1.- Si la nota de crédito es para un ticket entonces buscamos el UUID de la factura global
				//  correspondiente porque esta tiene el UUID relacionado.
				BufferRespuestas* resp_cfdglobal=NULL;
				try {
					instruccion.sprintf("SELECT cfd.muuid as cfdirelacionado \
						FROM ventas v \
						inner join terminales t on t.terminal=v.terminal \
						inner join secciones sec on t.seccion=sec.seccion \
						LEFT JOIN cfd ON cfd.sucursal=sec.sucursal and cfd.referencia=@referfacturaglobal_orig AND cfd.tipocomprobante='TICK' AND cfd.estado=1 \
						WHERE v.referencia='%s'", referencia_ticket);
					if (!mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_cfdglobal)){
						mensaje_bitacora = "FACTURA_GLOBAL1";
						detalles_bitacora = "No se pudo consultar la factura global que corresponde al ticket";
						instruccion_bitacora.sprintf("INSERT INTO bitacorafacturacionwebdetalles VALUES (NULL, %s, '%s','%s' )",clave_bitacora, mensaje_bitacora, detalles_bitacora);
						mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion_bitacora.c_str());
						//throw Exception("No se pudo consultar la factura global que corresponde al ticket");
						pendientetimbrar = true;
					}

					if (resp_cfdglobal->ObtieneNumRegistros()!=1){
						mensaje_bitacora = "FACTURA_GLOBAL2";
						detalles_bitacora = "El ticket debe tener una (y solo una) factura global generada y activa";
						instruccion_bitacora.sprintf("INSERT INTO bitacorafacturacionwebdetalles VALUES (NULL, %s, '%s','%s' )",clave_bitacora, mensaje_bitacora, detalles_bitacora);
						mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion_bitacora.c_str());
						pendientetimbrar = true;
						//throw Exception("El ticket debe tener una (y solo una) factura global generada y activa");
					}

					cfdirelacionado= resp_cfdglobal->ObtieneDato("cfdirelacionado");
					if (cfdirelacionado==""){
						mensaje_bitacora = "FACTURA_GLOBAL3";
						detalles_bitacora = "No se encontró CFDI relacionado (de tipo factura global) para el ticket";
						instruccion_bitacora.sprintf("INSERT INTO bitacorafacturacionwebdetalles VALUES (NULL, %s,'%s','%s' )",clave_bitacora, mensaje_bitacora, detalles_bitacora);
						mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion_bitacora.c_str());
						pendientetimbrar = true;
						//throw Exception("No se encontró CFDI relacionado (de tipo factura global) para el ticket");
					}

				} __finally {
					if (resp_cfdglobal!=NULL) delete resp_cfdglobal;
				}

			}
			// Estas condicion se omitira porque esta definido para nota de crédito
			if (forma_cancelar_ticket == 1 ) {
				// Si es ticket del mismo día, revisamos si hay factura global generada,
				// ya que de existir factura global se tiene que hacer nota de crédito.
				BufferRespuestas* resp_cfdglobal=NULL;
				try {
					instruccion.sprintf("SELECT cfd.muuid as cfdirelacionado \
						FROM ventas v \
						inner join terminales t on t.terminal=v.terminal \
						inner join secciones sec on t.seccion=sec.seccion \
						LEFT JOIN cfd ON cfd.sucursal=sec.sucursal and cfd.referencia=@referfacturaglobal_orig AND cfd.tipocomprobante='TICK' AND cfd.estado=1 \
						WHERE v.referencia='%s'", referencia_ticket);
					if (!mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_cfdglobal)){
						mensaje_bitacora = "FACTURA_GLOBAL21";
						detalles_bitacora = "No se pudo consultar la factura global que corresponde al ticket";
						GuardaenBitacoraWeb(Respuesta, MySQL, clave_bitacora, mensaje_bitacora,detalles_bitacora, detalles_bitacora);

					}
					if (resp_cfdglobal->ObtieneNumRegistros()==1) {
						cfdirelacionado= resp_cfdglobal->ObtieneDato("cfdirelacionado");
						if (cfdirelacionado!="")
							forma_cancelar_ticket=2;
					}
				} __finally {
					if (resp_cfdglobal!=NULL) delete resp_cfdglobal;
				}

			}

			// Obtiene cliente con base al rfc dando preferencia a los activos
			// Y valida que en caso de pasar parámetros de alta cliente sean válidos.

			//SI trae la clave del cliente ese es el que se va tomar
			if( clave_cliente.Length()>2 && alta_cliente!="1" ){
			instruccion.sprintf("select @cliente:=cliente "
				"from clientes where cliente='%s' and activo=1 order by fechamodi desc limit 1"
				, clave_cliente);
			}else{
				instruccion.sprintf("select @cliente:=cliente "
				"from clientes where rfc='%s'  and activo=1 order by fechamodi desc limit 1"
				, rfc_cliente);
			}

			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_cliente);
			if (resp_cliente->ObtieneNumRegistros()==0) {
				if (alta_cliente!="1"){
					mensaje_bitacora = "SIN CLIENTE";
					detalles_bitacora = "No se encontró un cliente con el rfc "+rfc_cliente;
					GuardaenBitacoraWeb(Respuesta, MySQL, clave_bitacora, mensaje_bitacora,detalles_bitacora, detalles_bitacora);
				}
			} else {
				if (alta_cliente=="1") {
					mensaje_bitacora = "ALTA CLIENTE";
					detalles_bitacora = "Se encontró un cliente ya existente con el rfc "+rfc_cliente;
					GuardaenBitacoraWeb(Respuesta, MySQL, clave_bitacora, mensaje_bitacora,detalles_bitacora, detalles_bitacora);
					}
			}

			// Obtiene la clave forma de pago para cfdi
			BufferRespuestas* resp_fpago=NULL;
			try{
				instruccion.sprintf("SELECT fp.formapagocfdi "
					"FROM dventasfpago df "
					"INNER JOIN formasdepago fp ON fp.formapag=df.formapag "
					"WHERE df.referencia='%s' "
					"GROUP BY df.formapag "
					"ORDER BY valor DESC "
					"LIMIT 1 ",	referencia_ticket);
				mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_fpago);
				if (resp_fpago->ObtieneNumRegistros()>0){
					 paramNuevaClaveFpago =  resp_fpago->ObtieneDato("formapagocfdi");
				} else{
					mensaje_bitacora = "FORMA_PAGO";
					detalles_bitacora = "No se encontró forma de pago para el cfdi";
					GuardaenBitacoraWeb(Respuesta, MySQL, clave_bitacora, mensaje_bitacora,detalles_bitacora, detalles_bitacora);
				}


			}
			__finally {
				if (resp_fpago!=NULL) delete resp_fpago;
			}

			// ---------------------------- INICIA EL PROCESO DE LAS INSTRUCCIONES ---------------------
			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

			//	Guardamos el regimen fiscal del cliente
			if (alta_cliente!="1" && update_regimen=="1" && update_user != "1")  {
			 instruccion.sprintf("UPDATE clientes set regimenfiscal = '%s' WHERE cliente = '%s' ",regimenfiscal,clave_cliente );
			 instrucciones[num_instrucciones++] = instruccion;
			 instruccion.sprintf("INSERT INTO bitacoraclientes "
						"(id_bitacoraclientes,empleado,tipo_modificacion,fecha,cliente,razon_social, "
						"rfc,giro_negocio,clasif_canal,activo_ventas,superv_gerencia, "
						"parte_relacionada,tipo_empresa,credito_autorizado,plazo_credito, "
						"limite_credito,permit_exce_limite,vent_credit_permit,imp_doc_saldo_CFDI, "
						"status_cliente,agente_venta,agente_cobrador,tipo_precio_asignado, "
						"ventas_volumen,email1,email2,env_comprov_cfd,forma_pago_def,forma_pago_vent_may, "
						"valor_vent_form_pago,uso_CFDI,agrupar_concep_CFDI, regimenfiscal) "
					"SELECT  NULL, '%s' as empleado, 'MODIFICFACTWEB' as tipo_modificacion, NOW() as fecha, c.cliente,rsocial,rfc,giro, "
						"canal,activo,sgerencia,esparterelac,tipoempre,credito,plazo,limcred,excederlc,numpedidos, "
						"imprsaldos,bloqueo,vendedor,cobrador,cet.tipoprec,venxvol,email,email2,enviarcfd,metododef, "
						"metodosup,valorsup,usocfdi,agruparncre,regimenfiscal "
						"FROM clientes c LEFT JOIN clientesemp cet ON cet.cliente=c.cliente AND cet.idempresa=%s "
						" WHERE c.cliente = '%s' ", usualta_ticket, FormServidor->ObtieneClaveEmpresa(), clave_cliente);

			 instrucciones[num_instrucciones++]=instruccion;
			}
			// Actualizamos los datos del usuario
			if (alta_cliente!="1" &&  update_user == "1")  {
			if (tipoempre_cliente=="1")  {    // Persona Moral
			instruccion.sprintf("UPDATE  clientes set  rsocial = '%s', \
								 usocfdi = '%s', regimenfiscal = '%s', cp='%s' WHERE cliente = '%s' ",
								 razonsocial,   usocfdi_cliente, regimenfiscal, codigopostal_cliente, clave_cliente );
			}else {
			instruccion.sprintf("UPDATE clientes set nombre = '%s', appat = '%s',apmat = '%s', rsocial = '%s', \
								 usocfdi = '%s', regimenfiscal = '%s' , cp='%s' WHERE cliente = '%s' ",
								nombres,appate, apmat, razonsocial,   usocfdi_cliente, regimenfiscal, codigopostal_cliente, clave_cliente );
			}

			 instrucciones[num_instrucciones++] = instruccion;

			 instruccion.sprintf("INSERT INTO bitacoraclientes "
						"(id_bitacoraclientes,empleado,tipo_modificacion,fecha,cliente,razon_social, "
						"rfc,giro_negocio,clasif_canal,activo_ventas,superv_gerencia, "
						"parte_relacionada,tipo_empresa,credito_autorizado,plazo_credito, "
						"limite_credito,permit_exce_limite,vent_credit_permit,imp_doc_saldo_CFDI, "
						"status_cliente,agente_venta,agente_cobrador,tipo_precio_asignado, "
						"ventas_volumen,email1,email2,env_comprov_cfd,forma_pago_def,forma_pago_vent_may, "
						"valor_vent_form_pago,uso_CFDI,agrupar_concep_CFDI, regimenfiscal) "
					"SELECT  NULL, '%s' as empleado, 'MODIFICFACTWEB' as tipo_modificacion, NOW() as fecha, clientes.cliente,rsocial,rfc,giro, "
						"canal,activo,sgerencia,esparterelac,tipoempre,credito,plazo,limcred,excederlc,numpedidos, "
						"imprsaldos,bloqueo,vendedor,cobrador,cet.tipoprec,venxvol,email,email2,enviarcfd,metododef, "
						"metodosup,valorsup,usocfdi,agruparncre,regimenfiscal "
						"FROM clientes  LEFT JOIN clientesemp cet ON cet.cliente = clientes.cliente AND cet.idempresa=%s "
						"WHERE clientes.cliente = '%s' ", usualta_ticket, FormServidor->ObtieneClaveEmpresa(), clave_cliente);

			 instrucciones[num_instrucciones++]=instruccion;
			}else if(!CPIncorrecto){
				//Consulta el cp del cliente
				BufferRespuestas* resp_cpcliente=NULL;
				try{
					AnsiString select;
					select.sprintf(" SELECT cp FROM clientes WHERE cliente = '%s' ", clave_cliente);

					mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, select.c_str(), resp_cpcliente);
					if (resp_cpcliente->ObtieneNumRegistros()>0){
						cp_actual = resp_cpcliente->ObtieneDato("cp");
					}

				}
				__finally {
					if (resp_cpcliente!=NULL) delete resp_cpcliente;
				}

				if(codigopostal_cliente != cp_actual ){
					instruccion.sprintf("UPDATE clientes set cp='%s' WHERE cliente = '%s' ",
					 codigopostal_cliente, clave_cliente );

					instrucciones[num_instrucciones++]=instruccion;

					cambiaCP = true;
				}

			}



			// Hace el alta del cliente de así aplicar
			if (alta_cliente=="1")  {
				// Calcula el folio para el nuevo cliente
				instruccion.sprintf(
					"select @folioaux:=valor from foliosemp where folio='CLIENTES' AND sucursal = '%s' %s ",
					FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
				instrucciones[num_instrucciones++] = instruccion;
				instruccion.sprintf("set @foliosig=@folioaux+1");
				instrucciones[num_instrucciones++] = instruccion;
				instruccion.sprintf("set @folioaux=cast(@folioaux as char)");
				instrucciones[num_instrucciones++] = instruccion;
				instruccion.sprintf("set @cliente=concat('%s', lpad(@folioaux,5,'0'))", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++] = instruccion;
				instruccion.sprintf(
					"update foliosemp set valor=@foliosig where folio='CLIENTES' AND sucursal = '%s' ",FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++] = instruccion;

				instruccion.sprintf("INSERT INTO clientes "
					"( "
						" cliente, nombre, appat, apmat, sucursal, "
						" fnaccli, titulo, rsocial, nomnegocio, contacto, "
						" contacfnac, tipoempre, calle, numext, numint, "
						" referenciadomic, colonia, cp, ubicaciongis, bloqueo, "
						" credito, clasifpag, clasifcom, fechaalta, fechamodi, "
						" fechabloq, homonimos, curp, rfc, fechauven, "
						" limcred, excederlc, plazo, riesgo, "
						" email, email2, medio, numpedidos, "
						" activo, sgerencia, venxvol, enviarcfd, metododef, "
						" metodosup, usocfdi, valorsup, digitosdef, digitossup, "
						" esparterelac, imprsaldos, giro, agruparncre, canal, "
						" forzarimprimirvertical, regimenfiscal, porccrecimiento "
					") "
					"SELECT "
						"@cliente as cliente, '%s' as nombre, '%s' as appat, '%s' as apmat, @sucursal_ticket as sucursal, "
						"'2000-01-01' as fnaccli, '' as titulo, '%s' as rsocial, '' as nomnegocio, '' as contacto, "
						"'2000-01-01' as contacfnac, '%s' as tipoempre, '.' calle, '' as numext, '' as numint, "
						"'' as referenciadomic, c.colonia, '%s' as cp, c.ubicaciongis, c.bloqueo, "
						"0 as credito, c.clasifpag, c.clasifcom, CURDATE() as fechaalta, CURDATE() as fechamodi, "
						"'2000-01-01' as fechabloq, '0' as homonimos, '' as curp, '%s' as rfc, '2000-01-01' as fechauven, "
						"0 as limcred, 0 as excederlc, 0 as plazo, c.riesgo, "
						"'' as email, '' as email2, c.medio, 0 as numpedidos, "
						"1 as activo, 0 as sgerencia, 0 as venxvol, 0 as enviarcfd, c.metododef, "
						"c.metodosup, '%s' as usocfdi, c.valorsup, c.digitosdef, c.digitossup, "
						"0 as esparterelac, 0 as imprsaldos, c.giro, 0 as agruparncre, c.canal, "
						"0 as forzarimprimirvertical, '%s' as regimenfiscal, 0 as porccrecimiento "
						"	FROM parametrosemp p "
						"	INNER JOIN clientes c ON c.cliente = p.valor "
						"	WHERE p.parametro = 'AUTOSUCLI' AND p.sucursal = '%s' ", nombres, appate, apmat,
						razonsocial, tipoempre_cliente, codigopostal_cliente, rfc_cliente, usocfdi_cliente, regimenfiscal, FormServidor->ObtieneClaveSucursal() );
				instrucciones[num_instrucciones++]=instruccion;

				BufferRespuestas* resp_emp=NULL;

				try {
					instruccion = "SELECT idempresa FROM empresas ";

					if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_emp)) {

						for(int i=0; i<resp_emp->ObtieneNumRegistros(); i++){
							resp_emp->IrAlRegistroNumero(i);

							if(FormServidor->ObtieneClaveEmpresa() == resp_emp->ObtieneDato("idempresa")){
								//se crea registro de tipos de precios del cliente
								instruccion.sprintf(" INSERT INTO clientesemp (cliente, idempresa, tipoprecmin, tipoprec, vendedor, cobrador ) \
									SELECT @cliente as cliente, cet.idempresa as empresa, cet.tipoprecmin, cet.tipoprec, cet.vendedor, cet.cobrador  \
									FROM parametrosemp p \
									LEFT JOIN clientesemp cet ON cet.cliente = p.valor AND cet.idempresa=%s \
									WHERE p.parametro = 'AUTOSUCLI' AND p.sucursal = '%s' "
									, FormServidor->ObtieneClaveEmpresa(), FormServidor->ObtieneClaveSucursal() );
								instrucciones[num_instrucciones++]=instruccion;

							}else{
								//se crea registro de tipos de precios del cliente
								instruccion.sprintf(" INSERT INTO clientesemp (cliente, idempresa, tipoprecmin, tipoprec, vendedor, cobrador ) \
									SELECT @cliente as cliente, cet.idempresa as empresa, cet.tipoprecmin, cet.tipoprec, cet.vendedor, cet.cobrador  \
									FROM parametrosemp p \
									INNER JOIN sucursales suc ON suc.sucursal = p.sucursal AND suc.idempresa=%s \
									LEFT JOIN clientesemp cet ON cet.cliente = p.valor AND cet.idempresa=%s \
									WHERE p.parametro = 'AUTOSUCLI' LIMIT 1 "
									, resp_emp->ObtieneDato("idempresa"), resp_emp->ObtieneDato("idempresa") );
								instrucciones[num_instrucciones++]=instruccion;

							}

						}

					} else
						throw (Exception("Error al consultar la tabla empresas"));
				} __finally {
					if (resp_emp!=NULL) delete resp_emp;
				}


				instruccion.sprintf("INSERT INTO bitacoraclientes "
						"(id_bitacoraclientes,empleado,tipo_modificacion,fecha,cliente,razon_social, "
						"rfc,giro_negocio,clasif_canal,activo_ventas,superv_gerencia, "
						"parte_relacionada,tipo_empresa,credito_autorizado,plazo_credito, "
						"limite_credito,permit_exce_limite,vent_credit_permit,imp_doc_saldo_CFDI, "
						"status_cliente,agente_venta,agente_cobrador,tipo_precio_asignado, "
						"ventas_volumen,email1,email2,env_comprov_cfd,forma_pago_def,forma_pago_vent_may, "
						"valor_vent_form_pago,uso_CFDI,agrupar_concep_CFDI, regimenfiscal) "
					"SELECT  NULL, '%s' as empleado, 'ALTAFACTWEB' as tipo_modificacion, NOW() as fecha, clientes.cliente,rsocial,rfc,giro, "
						"canal,activo,sgerencia,esparterelac,tipoempre,credito,plazo,limcred,excederlc,numpedidos, "
						"imprsaldos,bloqueo,vendedor,cobrador,cet.tipoprec,venxvol,email,email2,enviarcfd,metododef, "
						"metodosup,valorsup,usocfdi,agruparncre,regimenfiscal "
						"FROM clientes LEFT JOIN clientesemp cet ON cet.cliente = clientes.cliente AND cet.idempresa=%s "
						" WHERE clientes.cliente = @cliente ", usualta_ticket, FormServidor->ObtieneClaveEmpresa());
				instrucciones[num_instrucciones++]=instruccion;
			}

			// --------------- INICIA EL PROCESO PARA GENERAR LA VENTA DE ACUERDO AL TICKET -----------------------------

			// Obtiene el folio para la venta
			instruccion.sprintf("select @folioaux:=valor from foliosemp where folio='VENTAS' AND sucursal = '%s' %s",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @foliosig=@folioaux+1");
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @folioaux=cast(@folioaux as char)");
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @folio=concat('%s', lpad(@folioaux,9,'0'))", FormServidor->ObtieneClaveSucursal());
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("update foliosemp set valor=@foliosig where folio='VENTAS' AND sucursal = '%s' ",FormServidor->ObtieneClaveSucursal());
			instrucciones[num_instrucciones++]=instruccion;
			//quitar despues de pruebas
			instruccion.sprintf(" select @folio");
			instrucciones[num_instrucciones++]=instruccion;


			// Graba registro en ventas
			instruccion.sprintf(
				"insert into ventas "
				"	(referencia, foliofisic, tipofac, terminal, cancelado, acredito, cliente, vendedor, embarque, usuauto, usualta, usumodi, "
				"	fechaalta, horaalta, fechamodi, horamodi, fechavta, horavta, fechainic, fechavenc, porcint, valor, anticipo, letras, "
				"	periodic, plazo, comisionada, cobrador, totcomision, tolercomision, ticket, folioticket, kit, cantkits, ubicacion, "
				"	ventasuper, almacen, resumirart, conceptoresum, redondeoantiguo, embarquediv, tipoorigen, corte, cortecancel, terminalcancel) "
				" select @folio as referencia, foliofisic, 'IVADES' as tipofac, terminal, 0 as cancelado,"
				"   0 as acredito, @cliente, vendedor, '' as embarque, usuauto, usualta, usumodi, "
				"	CURDATE() as fechaalta, CURTIME() as horaalta, CURDATE() as fechamodi, "
				"   CURTIME() as horamodi, CURDATE() as fechavta, CURTIME() as horavta, "
				"   CURDATE() as fechainic, CURDATE() as fechavenc, porcint, valor, anticipo, "
				"   letras, periodic, plazo, 0 as comisionada, cobrador, 0 as totcomision, 0 as tolercomision, "
				"   0 as ticket, '' as folioticket, kit, cantkits, ubicacion, 0 as ventasuper, "
				"   almacen, 0 as resumirart, '' as conceptoresum, 0 as redondeoantiguo, 1 as embarquediv, "
				"   tipoorigen, NULL as corte, NULL as cortecancel, NULL as terminalcancel "
				" from ventas where referencia='%s' and ticket=1 and cancelado=0", referencia_ticket);
			instrucciones[num_instrucciones++]=instruccion;

			// Graba las partidas en "dventas"
			instruccion.sprintf("INSERT INTO dventas "
				"	(referencia, articulo, almacen, cantidad, claveimp1, claveimp2, claveimp3, claveimp4, costobase, precio, precioimp, porcdesc, tipoprec, porccomi, id) "
				" select @folio as referencia, articulo, almacen, cantidad, claveimp1, claveimp2, claveimp3, claveimp4, costobase, precio, precioimp, porcdesc, tipoprec, porccomi, id "
				"	FROM dventas "
				"	WHERE referencia='%s' ",  referencia_ticket);
			instrucciones[num_instrucciones++]=instruccion;

			// DIRECCION DE ENTREGA en facturación web será el domicilio fiscal (de tabla clientes).
			instruccion.sprintf("insert into ventadirent "
				" (id_ventaent,referencia, tipo, cliente,calle ,numext ,numint , colonia, cp, "
				"     ubicaciongis ,referenciadom,fechaalta,fechamodi) "
				" SELECT NULL as id_ventaent, @folio as referencia, 'Venta' as tipo, @cliente, "
				" calle, numext, numint, colonia, cp, "
				"	ubicaciongis, referenciadomic as referenciadom, '%s' as fechaalta, '%s' as fechamodi "
				" FROM clientes WHERE cliente=@cliente ",
				mFg.DateToMySqlDate(fecha_vta), mFg.DateToMySqlDate(fecha_vta),
				referencia_ticket);
			instrucciones[num_instrucciones++] = instruccion;

			int mes_modif=MonthOf(fecha_vta)+24;
			// Modifica el precalculo de ventas mensuales

			instruccion.sprintf(
				"UPDATE ventasxmes vm \
						INNER JOIN ( \
							SELECT dv.almacen, a.producto, a.present, SUM(dv.cantidad * a.factor) AS cantidad \
							FROM dventas dv INNER JOIN articulos a ON a.articulo=dv.articulo \
							WHERE dv.referencia = @folio GROUP BY dv.almacen, a.producto, a.present \
						) vent ON vm.almacen=vent.almacen AND vm.producto=vent.producto AND vm.present=vent.present \
					SET vm.cant%s = vm.cant%s + vent.cantidad, vm.ventas30 = vm.ventas30 + vent.cantidad, \
					vm.ventas60 = vm.ventas60 + vent.cantidad, vm.ventas90 = vm.ventas90 + vent.cantidad, \
					vm.ventas180 = vm.ventas180 + vent.cantidad, vm.ventascorte = vm.ventascorte + vent.cantidad ",
				mFg.IntToAnsiString(mes_modif), mFg.IntToAnsiString(mes_modif));
			instrucciones[num_instrucciones++]=instruccion;

			// Blanquea las referencias para ticket cancelado y nota de crédito for facturación de ticket
			// (para solo llenarlas cuando su proceso se haya completado correctamente)
			instruccion.sprintf("set @refticketcancelado=''");
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @folionc=''");
			instrucciones[num_instrucciones++]=instruccion;

			// Formas de pago
			instruccion.sprintf("INSERT INTO dventasfpago (referencia, formapag, valor, porcentaje, trn_id) "
				" SELECT  @folio as referencia, formapag, valor, porcentaje, trn_id from dventasfpago "
				" WHERE referencia='%s' ",referencia_ticket);
			instrucciones[num_instrucciones++]=instruccion;

				// Crea tabla temporal para almacenar articulos antes de cancelar
				instruccion="CREATE temporary TABLE tmpcantidad(referencia VARCHAR(11) NOT NULL,\
				producto VARCHAR(8) NOT NULL, present VARCHAR(255) NOT NULL, almacen VARCHAR(4) NOT NULL,  \
				cantidad DECIMAL(12,3) NOT NULL, PRIMARY KEY (referencia, producto, present, almacen))";
				instrucciones[num_instrucciones++]=instruccion;

				instruccion.sprintf("INSERT INTO tmpcantidad SELECT d.referencia, a.producto, a.present, d.almacen,  \
				SUM(d.cantidad * a.factor) AS cantidad FROM dventas d INNER JOIN \
				articulos a ON a.articulo = d.articulo WHERE d.referencia = '%s' \
				GROUP BY a.producto, a.present, d.almacen", referencia_ticket);
				instrucciones[num_instrucciones++]=instruccion;

				instruccion = "UPDATE tmpcantidad tmp INNER JOIN existenciasactuales ea ON ea.producto = tmp.producto \
				AND ea.present = tmp.present AND ea.almacen = tmp.almacen SET ea.cantidad = (ea.cantidad  - tmp.cantidad) \
				, ea.ventas = (ea.ventas + tmp.cantidad)";
				instrucciones[num_instrucciones++]=instruccion;


				instruccion="DROP TABLE IF EXISTS tmpcantidad";
				instrucciones[num_instrucciones++]=instruccion;


			// Agrega tipo TICK a bitacoraFactTick
			instruccion.sprintf("INSERT INTO bitacoraFactTick (referencia, tipo, referenciafact, fecha, hora) VALUES ('%s','TICK',@folio,DATE(NOW()),TIME(NOW()));",referencia_ticket );
			instrucciones[num_instrucciones++]=instruccion;

			/*****************************************************************************/
			//******************************VERIFICAR TICKET ACTIVO****************************///
			/*****************************************************************************/
			if (forma_cancelar_ticket == 2 ) {
				BufferRespuestas* resp=NULL;
				try{
					instruccion.sprintf("SELECT cancelado FROM ventas  where referencia='%s'",referencia_ticket);
					mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp);
					if (resp->ObtieneNumRegistros()>0)
						ticket_cancelado =  resp->ObtieneDato("cancelado");
						else
							ticket_cancelado =  "0";
				}
				__finally {
					if (resp!=NULL) delete resp;
				}

				if (ticket_cancelado == "1") {
					mensaje_bitacora = "TICKET_CANCELADO";
					detalles_bitacora = "No se pueden facturar tickets que ya están cancelados";
					GuardaenBitacoraWeb(Respuesta, MySQL, clave_bitacora, mensaje_bitacora,detalles_bitacora, detalles_bitacora);
				}

			}
		}


		//--------------------------------
		//Ventas de Kits
		//--------------------------------
		instruccion.sprintf("INSERT INTO ventaskits (venta, kit, desglosado, cantidad, modificado) "
				" SELECT @folio as venta, kit, desglosado, cantidad, modificado FROM ventaskits "
				" WHERE venta='%s' ",referencia_ticket);
		instrucciones[num_instrucciones++]=instruccion;

		//Detalle de Ventas de Kits
		//---------------------------------
		instruccion.sprintf("INSERT INTO dventaskits (venta, kit, articulo, cantidad, precio) "
				" SELECT @folio as venta, kit, articulo, cantidad, precio FROM dventaskits "
				" WHERE venta='%s' ",referencia_ticket);
		instrucciones[num_instrucciones++]=instruccion;


		// Solo se creara el cfdi web cuando se haya timbrado la factura
		instruccion.sprintf("INSERT INTO cfdiweb "
			"(refticket, factgenerada, rfc, codigopostal, usocfdi, fechaalta, horaalta, pdfgenerado) "
			"select '%s' as referencia_ticket, @folio as factgenerada, '%s' as rfc, '%s' as codigopostal, '%s' as usocfdi, "
			"'%s' as fechaalta, '%s' as horaalta, NULL as pdfgenerado ",
			referencia_ticket, rfc_cliente, codigopostal_cliente, usocfdi_cliente,
			mFg.DateToMySqlDate(fecha_vta), mFg.TimeToMySqlTime(hora) );
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("UPDATE  bitacorafacturacionweb SET  factura = @folio  where id = %s",clave_bitacora);
		instrucciones[num_instrucciones++]=instruccion;




		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);


		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		  if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			if (error==0) {
				// Crea el CFD (si la configuracion así lo indica) y hace commit
				ComprobanteFiscalDigital *cfd=NULL;
				AnsiString errorB = "";

				AnsiString muuid_insert_return;

				try {
						cfd=new ComprobanteFiscalDigital(mServidorVioleta);

						cfd->AsignaValores(paramNuevoMetodo, paramNuevaClaveFpago, paramNuevosDigitos, usocfdi_cliente, paramUuidRelacionado, paramCampoFiscalRel);

						if (!cfd->EmitirCFDIWeb44(Respuesta, MySQL, "VENT")) {
							mensaje_bitacora = "VERSION CFDI";
							detalles_bitacora = "La versión del CFDI tipo VENT ya no es soportada";
							GuardaenBitacoraWeb(Respuesta, MySQL, clave_bitacora, mensaje_bitacora,detalles_bitacora, detalles_bitacora);
						}

						cfd->AsignaCancelarTicket(referencia_ticket);

						cfd->AsignaFormaCancelarTicket(forma_cancelar_ticket);


						cfd->CreaCFDIVenta40(Respuesta, MySQL,	paramUuidRelacionado,clave_bitacora,referencia_ticket, false, "", terminal_fw);

						muuid_insert_return = cfd->muuid_insert_return;
						num_instrucciones=0;

						//Si se cambio el CP se regresa al anterior
						if(cambiaCP == true && cp_actual.Trim() != "") {
							instruccion.sprintf("UPDATE clientes set cp='%s' WHERE cliente = '%s' ",
								cp_actual, clave_cliente );

							if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str())){
								GuardaenBitacoraWeb(Respuesta, MySQL, clave_bitacora, "UPDATE cp cliente ","Error en regresar el valor original del cp", "Error en regresar el valor original del cp");
							}
						}


						if(muuid_insert_return!="NULL"){

							if (forma_cancelar_ticket == 2 )
							{
							// -----------------------------------------------------------------------------
							// CREAR NOTA DE CREDITO PARA EL TICKET FACTURADO
							// -----------------------------------------------------------------------------
							char *buffer_nota_cred=new char[1024*64*10];
							char *aux_buffer_nota_cred=buffer_nota_cred;
							try{
								try{

									aux_buffer_nota_cred=mFg.AgregaStringABuffer("", aux_buffer_nota_cred);
									aux_buffer_nota_cred=mFg.AgregaStringABuffer("", aux_buffer_nota_cred);
									aux_buffer_nota_cred=mFg.AgregaStringABuffer("A", aux_buffer_nota_cred);
									aux_buffer_nota_cred=mFg.AgregaStringABuffer(referencia_ticket, aux_buffer_nota_cred);
									aux_buffer_nota_cred=mFg.AgregaStringABuffer(usualta_ticket, aux_buffer_nota_cred);
									aux_buffer_nota_cred=mFg.AgregaStringABuffer(terminal_ticket, aux_buffer_nota_cred);

									//Parametros para el CFD
									aux_buffer_nota_cred=mFg.AgregaStringABuffer(paramNuevoMetodo, aux_buffer_nota_cred);
									aux_buffer_nota_cred=mFg.AgregaStringABuffer(paramNuevaClaveFpago, aux_buffer_nota_cred);
									aux_buffer_nota_cred=mFg.AgregaStringABuffer(paramNuevosDigitos, aux_buffer_nota_cred);
									aux_buffer_nota_cred=mFg.AgregaStringABuffer(" ", aux_buffer_nota_cred); // refcorte

									aux_buffer_nota_cred=mFg.AgregaStringABuffer("0", aux_buffer_nota_cred); // Num partidas se manda cero

									GrabaDevolCli(Respuesta, MySQL, buffer_nota_cred, true, pendientetimbrar);

									instruccion.sprintf("INSERT INTO bitacoraFactTick (referencia, tipo, referenciafact, fecha, hora) VALUES ('%s','NOTA',@folio,DATE(NOW()),TIME(NOW()))",
									referencia_ticket);
									if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str())){
										GuardaenBitacoraWeb(Respuesta, MySQL, clave_bitacora, "INSERT bitacoraFactTick","Error en query EjecutaSelectSqlNulo", "Error en query EjecutaSelectSqlNulo");
									}

									instruccion="COMMIT";
									if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str())) {
											GuardaenBitacoraWeb(Respuesta, MySQL, clave_bitacora, "COMMIT NCREDITO","No se pudo hacer commit", "No se pudo hacer commit");
                                    }
								} catch(Exception &e) {
										if (e.Message=="1" || e.Message=="2" || e.Message=="3" || e.Message=="4")
												errorB = "1" + e.Message;
												else{
													errorB = "20";
													errorMensaje= e.Message;
												}
								}
							}


							__finally {
								delete buffer_nota_cred;
							}

						}


				   }

				} __finally {
					if(cfd!=NULL) delete cfd;
				}

				if(errorB != "")
					error = StrToInt(errorB);

				instruccion.sprintf("select %d as error, '%s' as errorMensaje ,@folionc as folionotacred, "
					" @refticketcancelado as refticketcancelado, "
					" v.referencia as folio, v.foliofisic, v.folioticket, v.fechavta, v.horavta from ventas v where v.referencia=@folio ", error, errorMensaje);
			} else {
				instruccion.sprintf("select %d as error, '%s' as errorMensaje, '' as folionotacred, "
				" '' as refticketcancelado ", error, errorMensaje);
			}

			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}

	} __finally {
		if (resp_ticket_original!=NULL) delete resp_ticket_original;
		if (resp_cliente!=NULL) delete resp_cliente;

		delete buffer_sql;
	}
}
//---------------------------------------------------------------------------
// ID_REV_CALCULADOR_PEDIDOS_EXISTENCIAS_REMOTAS
void ServidorVentas::RevisaExistenciasRemotasCalculadorPedidos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[25000*1000];
	char *aux_buffer_sql=buffer_sql;
	int i, num_partidas;
	double cantidad, cantidad_mod, cantidad_final, cantidad_art, cantidad_act;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[25000];
	AnsiString tarea, articulo_aux, sucursal, tipo_mov;
	AnsiString almacen, condicion_almacen=" ";
	AnsiString almacen_aux, cad_conjunto_almacenes=" ";
	TDate fecha=Today();
	AnsiString archivo_temp1, archivo_temp2, archivo_temp3, archivo_temp4, archivo_temp5;
	BufferRespuestas* resp_almacenes=NULL;
	AnsiString modo_calcular_existencias;
	AnsiString having,envio_datos;

	try{
		sucursal=mFg.ExtraeStringDeBuffer(&parametros);
		almacen=mFg.ExtraeStringDeBuffer(&parametros);
		tarea=mFg.ExtraeStringDeBuffer(&parametros); // Indicador si se va a agregar o modificar.
		tipo_mov=mFg.ExtraeStringDeBuffer(&parametros); // Ver si se realizara desde movimientos de almacen


		envio_datos = mFg.ExtraeStringDeBuffer(&parametros);// todos los articulos existencia remota


		//nueva consulta de un parametro del método para revisar las existencias con base a la tabla de existencias actuales.
		BufferRespuestas* resp_parametros=NULL;
		try{
			instruccion.sprintf("SELECT * FROM parametrosemp WHERE parametro = 'CALCEXISTRV' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_parametros);
			if (resp_parametros->ObtieneNumRegistros()>0){
				modo_calcular_existencias=resp_parametros->ObtieneDato("valor");
			}
		}__finally{
			if (resp_parametros!=NULL) delete resp_parametros;
		}

		if(almacen!=" "){
			// Se le da prioridad al parámetro de almacén.

			// Todos los almacenes que pertenecen a la misma sucursal que el almacén indicado.
			instruccion.sprintf("SELECT a.almacen FROM almacenes a \
				INNER JOIN secciones s ON a.seccion=s.seccion \
				WHERE s.sucursal IN (SELECT s.sucursal FROM almacenes al \
				INNER JOIN secciones s ON s.seccion=al.seccion \
				WHERE almacen='%s')", almacen);

		} else {
			if (sucursal!=" ") {
					// Todos los almacenes que pertenecen a la sucursal dada.
					instruccion.sprintf("SELECT a.almacen FROM almacenes a \
						INNER JOIN secciones s ON a.seccion=s.seccion \
						WHERE s.sucursal='%s'", sucursal);

			} else
				throw Exception("Debe indicarse una sucursal o un almacén (RevisaExistenciasVenta)");

		}

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

		// Crea una tabla temporal para recibir los artículos que se van a vender.
		instruccion="create temporary table auxarticulos ( \
			articulo varchar(9), producto varchar(8), present varchar(255), \
			cantidad decimal(12,3), factor decimal(10,3), \
			INDEX(articulo)) Engine = InnoDB";
		instrucciones[num_instrucciones++]=instruccion;

		// Crea una tabla temporal para agrupar los productos por producto-present-almacen
		instruccion="create temporary table auxagrupados ( \
			producto varchar(8), present varchar(255), almacen char(4), \
			INDEX(producto, present)) Engine = InnoDB";
		instrucciones[num_instrucciones++]=instruccion;

		// Crea una tabla temporal para agrupar producto presentacion
		instruccion="create temporary table auxagrupadospp ( \
			producto varchar(8), present varchar(255), \
			cantidad decimal(12,3), INDEX(producto, present)) Engine = InnoDB";
		instrucciones[num_instrucciones++]=instruccion;

		// Crea una tabla donde se pondrán las existencias que se vayan encontrando.
		instruccion="create temporary table existenciasaux ( \
			producto varchar(8), present varchar(255), almacen char(4), \
			tipo char(2), cantidad decimal(12,3), INDEX(producto, present, almacen)) Engine = InnoDB";
		instrucciones[num_instrucciones++]=instruccion;

		// Crea una tabla temporal para agrupar producto presentacion  y cantidad de las existencias actuales
		instruccion="create temporary table auxcantidades ( \
			producto varchar(8), present varchar(255), \
			cantidad decimal(12,3), INDEX(producto, present)) Engine = InnoDB";
		instrucciones[num_instrucciones++]=instruccion;

		// Recibe los articulos y su respectiva cantidad que se va a vender.
		num_partidas=StrToInt(mFg.ExtraeStringDeBuffer(&parametros)); // Obtiene el número de partidas
		for (i=0; i<num_partidas; i++) {
			articulo_aux=mFg.ExtraeStringDeBuffer(&parametros);
			cantidad=mFg.CadenaAFlotante(mFg.ExtraeStringDeBuffer(&parametros));
			cantidad_mod=mFg.CadenaAFlotante(mFg.ExtraeStringDeBuffer(&parametros));

			cantidad_final = cantidad_mod - cantidad;

			if(tipo_mov == "MOVALM"){
				if(tarea == "M"){
					if(cantidad_final < 0){
						cantidad_art = fabs(cantidad_final);
						cantidad_act = 0;
					}
						else{
							cantidad_art = 0;
							cantidad_act = cantidad_final;
						}

					instruccion.sprintf("INSERT INTO existenciasaux (producto, present, almacen, tipo, cantidad) \
					SELECT a.producto,a.present,'%s', 'DM', (%s * a.factor) as cantidad FROM articulos a WHERE a.articulo = '%s'",
					almacen, AnsiString(FloatToStr(cantidad_act)), articulo_aux);
					instrucciones[num_instrucciones++]=instruccion;
				}
					else
						cantidad_art = cantidad;
			}
			else
				cantidad_art = cantidad;

			instruccion.sprintf("insert into auxarticulos ( \
				articulo, cantidad) values ('%s',%s)", articulo_aux, AnsiString(FloatToStr(cantidad_art)));
			instrucciones[num_instrucciones++]=instruccion;

		}

		// Agrega el factor , producto y presentacion a cada artículo
		instruccion.sprintf("update auxarticulos aux, articulos a \
			set aux.factor=a.factor, aux.producto=a.producto, \
				aux.present=a.present  \
			where aux.articulo=a.articulo ");
		instrucciones[num_instrucciones++]=instruccion;

		// Agrupa por producto-presentacion-almacén
		//aux.producto, aux.present,
		if (cad_conjunto_almacenes!=" ") {
			condicion_almacen.sprintf("where alm.almacen in (%s) ", cad_conjunto_almacenes);
		} else condicion_almacen=" ";
		instruccion.sprintf("insert into auxagrupados (producto, present, almacen) \
			select aux.producto, aux.present, alm.almacen \
			from auxarticulos aux, almacenes alm \
			%s \
			group by aux.articulo, alm.almacen",condicion_almacen);
		instrucciones[num_instrucciones++]=instruccion;

		// Agrupa por producto-presentacion-cantidad
		//aux.producto, aux.present
		instruccion.sprintf("insert into auxagrupadospp (producto, present, cantidad) \
			select aux.producto, aux.present, sum(aux.factor*aux.cantidad) as cantidad \
			from auxarticulos aux \
			group by aux.articulo");
		instrucciones[num_instrucciones++]=instruccion;

		// Calcula base (0) de los artículos en cuestión la existencia al momento del corte previo.
		// La utilidad de esto es simplemente para tomar en cuenta todos los articulos aunque no
		// tengan ningun movimiento
		instruccion.sprintf("insert into existenciasaux (producto, present, almacen, tipo, cantidad) \
			select aux.producto, aux.present, aux.almacen as almacen, 'BA' as tipo, 0 as cantidad \
			from auxagrupados aux");
		instrucciones[num_instrucciones++]=instruccion;

		//agrega datos a la nueva tabla auxiliar de cantidades
		instruccion.sprintf("insert into auxcantidades (producto, present, cantidad) \
			SELECT p.producto, exa.present,  \
			(IFNULL(SUM(cantinicial) ,0)+IFNULL(SUM(compras),0)+IFNULL((SUM(devcompras)*-1),0)+IFNULL((SUM(ventas)*-1),0)+IFNULL(SUM(devventas),0)+IFNULL(SUM(entradas),0)+IFNULL((SUM(salidas)*-1),0)) as canttot \
			FROM existenciasactuales exa \
			INNER JOIN almacenes alm ON exa.almacen=alm.almacen \
			INNER JOIN productos p ON exa.producto=p.producto \
			 %s \
			GROUP BY exa.producto, exa.present ",condicion_almacen);
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
		//cuando se envie desde el calculador de pedidos ocupo que se envien todos los articulos
        //con la finalidad de extraer la cantidad actual de cada articulo
		if(envio_datos=="1"){
			having.sprintf(" ");
		}else{
			having.sprintf("having cantexist<cantidped");
        }

		instruccion="SET SESSION sql_log_bin = 1";
		instrucciones[num_instrucciones++]=instruccion;

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			 /*  ******* cod antes
			 from existenciasaux e, auxagrupados aux, productos p, auxagrupadospp auxpp \

			 , pmm.maxmult='CAJA'
			 canttot<ventas
			 cantexistmay<cantexistunid
			 auxpp.cantidad as cantidped,
			 ((@cantunidds*ax.factor)-(ax.cantidad*ax.factor))  AS XY,
			 */
			// Desglose de movimientos por almacén.
			// aux.producto, aux.present
			instruccion.sprintf("SELECT p.nombre, e.present, \
				TRUNCATE(auc.cantidad/pmm.maxfactor,0) AS cantexistmay,\
				MOD(SUM(e.cantidad), pmm.maxfactor) AS cantexistunid, \
				TRUNCATE(auc.cantidad/pmm.maxfactor,0) AS cantmayor,\
				MOD(auc.cantidad, pmm.maxfactor) AS cantunidad,\
				CONCAT(pmm.maxmult,'X',pmm.maxfactor), \
				pmm.minmult, \
				SUM(auc.cantidad) AS cantexist, \
				ax.cantidad AS cantidped, \
				ax.articulo, @cantunidds:=TRUNCATE(auc.cantidad /ax.factor,0) AS cantidadentfact, \
				auc.cantidad,\
				ax.factor, \
				pmm.maxfactor AS maxFact  \
				FROM existenciasaux e \
				inner join  auxagrupados aux \
				inner join productos p \
				inner join auxagrupadospp auxpp \
				inner join presentacionesminmax pmm \
				inner join articulos amax \
				INNER JOIN auxarticulos ax \
				INNER JOIN auxcantidades auc ON auc.producto=e.producto AND auc.present=e.present \
				where e.producto=aux.producto and e.present=aux.present and \
				e.producto=auxpp.producto and e.present=auxpp.present and \
				p.producto=aux.producto and e.almacen=aux.almacen \
				and pmm.producto=aux.producto AND pmm.present=aux.present \
				AND pmm.producto=auxpp.producto AND pmm.present=auxpp.present \
				and amax.producto=aux.producto AND amax.present=aux.present \
				AND amax.producto=auxpp.producto AND amax.present=auxpp.present and amax.multiplo=pmm.maxmult \
				AND e.producto=ax.producto AND e.present=ax.present \
				group by  ax.articulo  \
				order by  p.nombre, e.present\
				%s ",having);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	} __finally {
		if(resp_almacenes!=NULL) delete resp_almacenes;
		delete buffer_sql;

		mServidorVioleta->BorraArchivoTemp(archivo_temp1);
		
	}
}
//------------------------------------------------------------------------------
//ID_CON_PREPAGO_CLI
void ServidorVentas::ConsultaPrePagoCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA PAGO DE CLIENTE
	AnsiString instruccion;
	AnsiString clave;

	clave=mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los generales del pago
	instruccion.sprintf("select p.*, concat(emp.nombre,' ',emp.appat,' ',emp.apmat) as nomusualta, \
		col.nombre as nomcolonia, loc.nombre as nomlocalidad \
		from prepagoscli p \
		inner join clientes c ON p.cliente=c.cliente \
		inner join empleados emp ON p.cobrador=emp.empleado \
		left join colonias col on c.colonia=col.colonia \
		left join localidades loc on col.localidad=loc.localidad \
		where pago='%s' ", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene todas las transacciones correspondientes al pago.
	instruccion.sprintf("select '?' AS pago, t.referencia, tfv.descripcion as tipoventa, \
		t.valor, v.fechavta, \
		if(ifnull(v.foliofisic,'')='',concat(ifnull(cfd.serie,''),ifnull(cfd.folio,'')),v.foliofisic) as foliofisic, cfd.version, cfd.muuid,cfd.rfcreceptor,cfd.sucursal \
		FROM prepagoscli p \
		INNER JOIN predpagoscli t ON p.pago = t.pago \
		INNER JOIN ventas v ON t.referencia=v.referencia \
		INNER JOIN tiposfacturasventas tfv ON v.tipofac=tfv.tipo \
		left join cfd on v.referencia=cfd.referencia and cfd.tipocomprobante='VENT' \
		where t.pago='%s' \
		ORDER BY t.referencia ASC", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Verifica que la fecha del pago no sea posterior a la fecha de cierre.
	instruccion.sprintf("select @error:=if(IFNULL(p.fecha, '1900-01-01')<= CAST(e.valor AS DATETIME), 1, 0) AS error \
	FROM prepagoscli p \
	LEFT JOIN estadosistemaemp AS e ON e.estado='FUCIERRE' AND e.sucursal = '%s' \
	where p.pago='%s' ",FormServidor->ObtieneClaveSucursal(), clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------
//ID_OBTIENE_TRANSACCION_VENTA
void ServidorVentas::ConsultaTrnVenta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString Instruccion;
	AnsiString referencia;
	AnsiString condicion_referencia;

	referencia = mFg.ExtraeStringDeBuffer(&parametros);

	condicion_referencia.sprintf(" AND d.referencia = '%s' ", referencia);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	Instruccion.sprintf("SELECT \
		IF(dt.dcs_form = 'T120S001', 1,0) AS ajustado, \
		v.cancelado, \
		dt.trn_referencia_1 \
	FROM ventas v \
	INNER JOIN dventasfpago d ON d.referencia = v.referencia \
	INNER JOIN dettrnxventa dt ON dt.trn_id = d.trn_id \
	WHERE v.referencia = '%s' \
		AND v.fechavta = CURDATE() \
	AND v.cancelado = 0  \
	GROUP BY v.referencia ",
	referencia);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), Respuesta->TamBufferResultado);

	Instruccion.sprintf("SELECT \
		d.trn_id, \
		d.referencia, \
		f.descripcion, \
		d.formapag, \
		d.porcentaje, \
		d.valor \
	FROM ventas v \
	INNER JOIN dventasfpago d ON d.referencia = v.referencia \
	INNER JOIN formasdepago f ON f.formapag = d.formapag \
	WHERE 1 %s \
	AND v.fechavta = CURDATE() \
	AND v.cancelado = 0 \
	AND d.trn_id IS NOT NULL \
	ORDER BY d.valor DESC ",
	condicion_referencia);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), Respuesta->TamBufferResultado);

	Instruccion.sprintf("SELECT \
		d.trn_id, \
		d.formapag, \
		d.porcentaje, \
		d.valor \
	FROM ventas v \
	INNER JOIN dventasfpago d ON d.referencia = v.referencia \
	INNER JOIN formasdepago f ON f.formapag = d.formapag \
	WHERE 1 %s \
	AND v.fechavta = CURDATE() \
	AND v.cancelado = 0 \
	ORDER BY d.valor DESC ",
	condicion_referencia);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), Respuesta->TamBufferResultado);
}

 //------------------------------------------------------------------------------
//ID_OBTIENE_PDF_WEB
void ServidorVentas::ConsultaPdfFactura(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros){

	AnsiString instruccion, condicion;
	AnsiString referencia2;

	referencia2=mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_REP);

	// Obtiene todos los datos del cheque en cuestión
	condicion.sprintf("AND	f.factgenerada = '%s'                         \
		AND CURDATE() <= DATE_ADD(f.fechaalta, INTERVAL ddispon.valor DAY) \
			ORDER BY f.fechaalta DESC, f.horaalta DESC", referencia2 );

		instruccion = "SELECT f.activo as pdfgenerado, f.pdfgenerado as generado    \
		FROM cfdiweb f                                                                                                        \
		INNER JOIN ventas v ON f.refticket = v.referencia                                                                    \
		INNER JOIN ventas vfact ON f.factgenerada = vfact.referencia                                                         \
		INNER JOIN clientes c ON vfact.cliente = c.cliente                                                                   \
		INNER JOIN cfd cf ON vfact.referencia = cf.referencia AND cf.tipocomprobante='VENT'                                  \
		INNER JOIN cfdxml fx ON cf.compfiscal = fx.compfiscal                                                                 \
		LEFT JOIN errorestimbradofactweb errweb ON errweb.factgenerada = vfact.referencia                                     \										 \
		INNER JOIN parametroscfdiweb AS ddispon ON ddispon.parametro='DIASCONSULTAWEB' AND ddispon.idempresa="+
		 FormServidor->ObtieneClaveEmpresa()+"  \
		WHERE v.folioticket != '' AND (v.cancelado = 0 OR cf.estado=1) AND v.ticket = 1                                       \
		"+condicion;

   if(mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado)){

   }else { // ID_CON_PAGO_PROV
   ShowMessage("NO SE PYUDO SE EJECUTO");
   }
}

//---------------------------------------------------------------------------
//------------------------------------------------------------------------------
//ID_GRA_NOTA_CREDITO_WEB_POR_TIKCET
void ServidorVentas::GrabaNotadeCreditoPorTicket(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  GRABA DEVOLUCION DE CLIENTE
	char *buffer_sql=new char[2000*500];

	char *aux_buffer_sql=buffer_sql;
	AnsiString referencia_ticket,  usualta_ticket, terminal, peticionInd, detalles_bitacora,mensaje_bitacora, refnc,instruccion_bitacora;
    bool pendientetimbrar = false;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[500], cfdirelacionado,errorB;
	double valor;
	TDateTime fecha_dev;
	int error=0;
	AnsiString parametrometodo="", metodopago="", digitos="";
	AnsiString folio_vta;

	BufferRespuestas* buffer_nota_cred=NULL;
		AnsiString paramNuevoMetodo="1", paramNuevaClaveFpago="01", paramNuevosDigitos=" ";
	AnsiString paramUuidRelacionado=" ", paramCampoFiscalRel=" ",errorMensaje;

	try{
			referencia_ticket=mFg.ExtraeStringDeBuffer(&parametros); // Clave de la devolucion.
			terminal=mFg.ExtraeStringDeBuffer(&parametros); // Terminal
			usualta_ticket=mFg.ExtraeStringDeBuffer(&parametros); //  Usuario

			folio_vta=" "; //Folio de venta para saber si cre nota de credito al cancelar ticket

			int peticionIndInt =2;


			peticionInd = "GENERANOTA_CRED";

			BufferRespuestas* resp_ticket_original=NULL;
			// Obtiene datos importantes del ticket original.
			try{
			instruccion.sprintf("select v.fechavta, v.terminal, v.usualta, @sucursal_ticket:=sec.sucursal, "
				"@referfacturaglobal_orig:=DATE_FORMAT(v.fechavta, '%%d/%%m/%%Y') as referfacturaglobal "
				"from ventas v "
				"inner join terminales t on v.terminal=t.terminal "
				"inner join secciones sec on t.seccion=sec.seccion "
				"where v.referencia='%s' and v.cancelado=0 ",
				 referencia_ticket);

			detalles_bitacora = "No se encontró un ticket activo con el folio "+referencia_ticket;
				if(mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_ticket_original)){
					if (resp_ticket_original->ObtieneNumRegistros()==0){
					throw (Exception(detalles_bitacora));
					}
				}else{
						throw (Exception(detalles_bitacora));
				}
			}__finally {
					if (resp_ticket_original!=NULL) delete resp_ticket_original;
			}


			//Validación al crear notas de crédito por facturar tickets,
			// no se podrán facturar tickets que previamente se les hayan creado una nota de crédito (lo que significa que ya se hayan facturado).
			BufferRespuestas* resp_ref=NULL;
			try{
					instruccion.sprintf("  SELECT nc.referencia AS refnotcred, nc.cancelado AS cancelado \
					FROM ventas v INNER JOIN terminales t INNER JOIN empleados ec INNER JOIN empleados ev \
					INNER JOIN clientes c   LEFT JOIN cfd ON v.referencia=cfd.referencia AND \
					cfd.tipocomprobante='VENT' LEFT JOIN notascredcli AS nc ON  v.referencia = nc.venta  \
					LEFT JOIN clientesemp cemp ON c.cliente=cemp.cliente \
					WHERE v.referencia='%s' AND v.cliente=c.cliente AND v.terminal=t.terminal AND \
					v.usumodi=ec.empleado AND cemp.vendedor=ev.empleado AND nc.cancelado = '0' AND cemp.idempresa=%s ",referencia_ticket, FormServidor->ObtieneClaveEmpresa());
					mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_ref);
					if (resp_ref->ObtieneNumRegistros()>0){
						 refnc =  resp_ref->ObtieneDato("refnotcred");
					}
			}__finally {
					if (resp_ref!=NULL) delete resp_ref;
			}

			if (refnc != "") {
					mensaje_bitacora = "NOTA_CREDITO";
					detalles_bitacora = "No se pueden facturar tickets que tienen asignada una nota de crédito";
					throw (Exception(detalles_bitacora));
			}

				// 1.- Si la nota de crédito es para un ticket entonces buscamos el UUID de la factura global
				//  correspondiente porque esta tiene el UUID relacionado.
				BufferRespuestas* resp_cfdglobal=NULL;
				try {
					instruccion.sprintf("SELECT cfd.muuid as cfdirelacionado \
						FROM ventas v \
						inner join terminales t on t.terminal=v.terminal \
						inner join secciones sec on t.seccion=sec.seccion \
						LEFT JOIN cfd ON cfd.sucursal=sec.sucursal and cfd.referencia=@referfacturaglobal_orig AND cfd.tipocomprobante='TICK' AND cfd.estado=1 \
						WHERE v.referencia='%s'", referencia_ticket);
					if (!mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_cfdglobal)){
						mensaje_bitacora = "FACTURA_GLOBAL1";
						detalles_bitacora = "No se pudo consultar la factura global que corresponde al ticket";
						//throw Exception("No se pudo consultar la factura global que corresponde al ticket");
						pendientetimbrar = false;
						throw (Exception(detalles_bitacora));
					}

					if (resp_cfdglobal->ObtieneNumRegistros()!=1){
						mensaje_bitacora = "FACTURA_GLOBAL2";
						detalles_bitacora = "El ticket debe tener una (y solo una) factura global generada y activa";
						pendientetimbrar = false;
						throw (Exception(detalles_bitacora));
					}

					cfdirelacionado= resp_cfdglobal->ObtieneDato("cfdirelacionado");
					if (cfdirelacionado==""){
						mensaje_bitacora = "FACTURA_GLOBAL3";
						detalles_bitacora = "No se encontró CFDI relacionado (de tipo factura global) para el ticket";
						pendientetimbrar = false;
						throw (Exception(detalles_bitacora));
					}

				} __finally {
					if (resp_cfdglobal!=NULL) delete resp_cfdglobal;
				}

				mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
				if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
				// -----------------------------------------------------------------------------
				// CREAR NOTA DE CREDITO PARA EL TICKET FACTURADO
				// -----------------------------------------------------------------------------
				char *buffer_nota_cred=new char[1024*64*10];
				char *aux_buffer_nota_cred=buffer_nota_cred;
							try{
								try{
									aux_buffer_nota_cred=mFg.AgregaStringABuffer("", aux_buffer_nota_cred);
									aux_buffer_nota_cred=mFg.AgregaStringABuffer("", aux_buffer_nota_cred);
									aux_buffer_nota_cred=mFg.AgregaStringABuffer("A", aux_buffer_nota_cred);
									aux_buffer_nota_cred=mFg.AgregaStringABuffer(referencia_ticket, aux_buffer_nota_cred);
									aux_buffer_nota_cred=mFg.AgregaStringABuffer(usualta_ticket, aux_buffer_nota_cred);
									aux_buffer_nota_cred=mFg.AgregaStringABuffer(terminal, aux_buffer_nota_cred);

									//Parametros para el CFD
									aux_buffer_nota_cred=mFg.AgregaStringABuffer(paramNuevoMetodo, aux_buffer_nota_cred);
									aux_buffer_nota_cred=mFg.AgregaStringABuffer(paramNuevaClaveFpago, aux_buffer_nota_cred);
									aux_buffer_nota_cred=mFg.AgregaStringABuffer(paramNuevosDigitos, aux_buffer_nota_cred);
									aux_buffer_nota_cred=mFg.AgregaStringABuffer(" ", aux_buffer_nota_cred); // refcorte

									aux_buffer_nota_cred=mFg.AgregaStringABuffer("0", aux_buffer_nota_cred); // Num partidas se manda cero

									GrabaDevolCli(Respuesta, MySQL, buffer_nota_cred, false, pendientetimbrar, true);

									instruccion.sprintf("INSERT INTO bitacoraFactTick (referencia, tipo, referenciafact, fecha, hora) VALUES ('%s','NOTA',@folio,DATE(NOW()),TIME(NOW()))",
									referencia_ticket);
									if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str())){
										throw (Exception("Error en query EjecutaSelectSqlNulo"));
									}

									instruccion="COMMIT";
									if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str())) {
										throw (Exception("No se pudo hacer commit"));
									}
								} catch(Exception &e) {
									   error	=StrToInt(e.Message);
								}
							}
							__finally {
								delete buffer_nota_cred;
							}
				}

				instruccion.sprintf("select %d as error, @folionc as folio, @foliofisic as foliofisic", error);
				mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	} __finally {
		delete buffer_sql;
	}
}
//------------------------------------------------------------------------------
//ID_GENERA_PAGOS_PROVEEDOR_TXT
void ServidorVentas::GeneraPagosProveedorTxt(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{

	AnsiString instruccion;
	AnsiString muestrasolofacturassincheque=" ";
	AnsiString busquedaporsector=" ";
	AnsiString busquedaporlocalidad=" ";
	AnsiString fecha_inicio, fecha_fin, proveedor, sucursal, fecha_vence_ini, fecha_vence,usar_fechavence,con_saldo, fecha_saldo;
	AnsiString condicion_cobrador=" ",condicion_status=" ";
	AnsiString vendedor, status;
	AnsiString cliente, condicion_cliente=" ";
	AnsiString forzar_indice=" ";
	TDate date_fecha_inicio, date_fecha_fin, date_fecha_vence_ini, date_fecha_vence, date_fecha_saldo;
	AnsiString condicion_proveedor=" ", condicion_sucursal=" ", condicion_sucursal2=" ", condicion_fechavence=" ";
	AnsiString condicion_con_saldo=" ", condicion_left=" ", mostrarfacturascheque;
	AnsiString omitiruuid;
	AnsiString condicion_mostraruuid=" ";
	AnsiString tipospagos;

    int agrupaprov;

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	fecha_inicio=mFg.ExtraeStringDeBuffer(&parametros);
	fecha_fin=mFg.ExtraeStringDeBuffer(&parametros);
	fecha_vence_ini=mFg.ExtraeStringDeBuffer(&parametros);
	fecha_vence=mFg.ExtraeStringDeBuffer(&parametros);
	proveedor=mFg.ExtraeStringDeBuffer(&parametros);
	sucursal=mFg.ExtraeStringDeBuffer(&parametros);
	usar_fechavence=mFg.ExtraeStringDeBuffer(&parametros);
	con_saldo=mFg.ExtraeStringDeBuffer(&parametros);
	mostrarfacturascheque=mFg.ExtraeStringDeBuffer(&parametros);
	fecha_saldo=mFg.ExtraeStringDeBuffer(&parametros);
	omitiruuid=mFg.ExtraeStringDeBuffer(&parametros);
	tipospagos=mFg.ExtraeStringDeBuffer(&parametros);
	agrupaprov=StrToInt(mFg.ExtraeStringDeBuffer(&parametros));

	date_fecha_inicio=StrToDate(fecha_inicio);
	date_fecha_fin=StrToDate(fecha_fin);
	date_fecha_vence=StrToDate(fecha_vence);
	date_fecha_vence_ini=StrToDate(fecha_vence_ini);
	date_fecha_saldo=StrToDate(fecha_saldo);

	if(proveedor!=""){
		condicion_proveedor.sprintf(" and c.proveedor='%s' ", proveedor);
	}
	if(sucursal!=""){
		condicion_sucursal.sprintf(" and sec.sucursal='%s' ", sucursal);
		condicion_sucursal2.sprintf(" and c.sucursal='%s' ", sucursal);
	}
	if (usar_fechavence == "1") {
		condicion_fechavence.sprintf(" and c.fechavenc>='%s' and c.fechavenc<='%s' ", mFg.StrToMySqlDate(fecha_vence_ini), mFg.StrToMySqlDate(fecha_vence));
	}
	if (con_saldo == "1") {
		condicion_con_saldo.sprintf(" WHERE a.saldor>0 ");
	}
	if(mostrarfacturascheque == "0"){
		muestrasolofacturassincheque=" AND cs.saldor<>(cs.chcnc*-1) ";
	}
	if (omitiruuid=="1") {
        condicion_mostraruuid="AND c.muuid IS NOT NULL";
	}

	if (tipospagos == "0" || tipospagos == "2" ) {

		// Crea una tabla donde se van a poner los saldos de las ventas
		// afectadas por la cancelación
		instruccion="CREATE TEMPORARY TABLE auxcomprasprove (compra CHAR(11),sucursal CHAR(3), saldor DECIMAL(16,2), \
			chcnc DECIMAL(16,2), tncredito DECIMAL(16,2), tncargo DECIMAL(16,2), fechach DATE, PRIMARY KEY (compra)) ENGINE = INNODB";
		if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
			throw (Exception("Error en query EjecutaSelectSqlNulo"));

		// Calcula los saldos de las ventas del proveedor
		instruccion.sprintf("insert into auxcomprasprove (compra,sucursal, saldor, chcnc, tncredito, tncargo,fechach) \
			SELECT a.compra,a.sucursal,a.saldor,a.chcnc,a.tncredito,a.tncargo,a.fechach FROM ( \
			SELECT c.referencia AS compra, \
			sec.sucursal , \
			SUM(IF(t.aplicada=1, t.valor, 0)) AS saldor, \
			SUM(IF(t.aplicada=0, t.valor, 0)) AS chcnc, \
			SUM(IF(t.aplicada=1 AND t.tipo='DEVO', t.valor, 0)) AS tncredito, \
			SUM(IF(t.aplicada=1 AND (t.tipo='NCAR' OR t.tipo='INTE'), t.valor, 0)) AS tncargo, \
			MAX(IFNULL(chcl.fechacob, CAST('1900-01-01' AS DATE))) AS fechach \
			FROM compras c INNER JOIN  transxpag t \
			inner join terminales ter on ter.terminal=c.terminal \
			inner JOIN secciones sec ON sec.seccion=ter.seccion \
			LEFT JOIN pagosprov p ON t.pago=p.pago AND t.aplicada=0 \
			LEFT JOIN cheqxpag chxc ON p.pago=chxc.pago \
			LEFT JOIN chequesproveedores chcl ON chxc.chequeprov=chcl.chequeprov \
			WHERE c.fechacom>='%s' AND c.fechacom<='%s' AND t.referencia=c.referencia AND t.fechaapl<='%s'\
			%s %s %s \
			AND t.cancelada = 0 \
			AND c.cancelado = 0 \
			GROUP BY c.referencia \
			) AS a %s ",
			mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin),mFg.StrToMySqlDate(fecha_saldo),
			condicion_proveedor, condicion_sucursal, condicion_fechavence,
			condicion_con_saldo);
		if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
			throw (Exception("Error en query EjecutaSelectSqlNulo"));

	}
	if (tipospagos == "1" || tipospagos == "2") {

		// Crea una tabla donde se van a poner los saldos de los gastos afectadas por la cancelación
		instruccion="CREATE TEMPORARY TABLE auxgastosprove ( \
		gasto CHAR(11), \
		sucursal CHAR(3), \
		saldor DECIMAL(16,2), \
		chcnc DECIMAL(16,2), \
		tncredito DECIMAL(16,2), \
		tncargo DECIMAL(16,2), \
		fechach DATE, \
		PRIMARY KEY (gasto)) ENGINE = INNODB";
		if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
			throw (Exception("Error en query EjecutaSelectSqlNulo"));


		// Calcula los saldos de las ventas del proveedor
		instruccion.sprintf("INSERT INTO auxgastosprove (gasto,sucursal, saldor, chcnc, tncredito, tncargo,fechach) \
		SELECT a.gasto, a.sucursal, a.saldor, a.chcnc, a.tncredito, a.tncargo, a.fechach \
		FROM (SELECT c.referencia AS gasto, c.sucursal , SUM(IF(t.aplicada=1, t.valor, 0)) AS saldor, \
		SUM(IF(t.aplicada=0, t.valor, 0)) AS chcnc, SUM(IF(t.aplicada=1 AND t.tipo='DEVO', t.valor, 0)) AS tncredito, \
		SUM(IF(t.aplicada=1 AND (t.tipo='NCAR' OR t.tipo='INTE'), t.valor, 0)) AS tncargo, MAX(IFNULL(chcl.fechacob, CAST('1900-01-01' AS DATE))) AS fechach \
		FROM gastos c \
		INNER JOIN transxpaggastos t ON t.referencia=c.referencia \
		INNER JOIN terminales ter ON ter.terminal=c.terminal \
		INNER JOIN secciones sec ON sec.seccion=ter.seccion \
		LEFT JOIN pagosgastos p ON t.pago=p.pago AND t.aplicada=0 \
		LEFT JOIN cheqxgas chxc ON p.pago=chxc.pago \
		LEFT JOIN chequesgastos chcl ON chxc.chequegasto=chcl.chequegasto \
		WHERE c.fechagas>='%s' AND c.fechagas<='%s' AND t.fechaapl<='%s' \
		%s %s %s \ \
		AND t.cancelada = 0 \
		AND c.cancelado = 0 \
		GROUP BY c.referencia) AS a  %s ",
		mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin),mFg.StrToMySqlDate(fecha_saldo),
		condicion_proveedor, condicion_sucursal2, condicion_fechavence,
		condicion_con_saldo);
		if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
			throw (Exception("Error en query EjecutaSelectSqlNulo"));
	}

	if (tipospagos == "0") {

		if(agrupaprov==1){
			instruccion.sprintf("SELECT t.clavepagobnc, t.asuntobenef, t.asuntoorden, \
			t.divisa, t.importe, t.motivo, t.tipo, t.referencias, t.proveedor, t.total, t.numerocuenta, t.saldo \
			FROM ( \
			(SELECT \
				IF(p.cuentadefault = 1, tc1.clave, IF(p.cuentadefault = 2, tc2.clave, tc3.clave)) AS clavepagobnc, \
				LPAD(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),18,'0') AS asuntobenef, \
				LPAD(bc.numerocuenta,18,'0') AS asuntoorden, \
				'MXP' AS divisa, \
				LPAD(SUM(cs.saldor),16,'0') AS importe, \
				RPAD(SUBSTRING(IF(p.tiporefpago = 0, p.referenciafija, IF(p.tiporefpago = 1, REPLACE(GROUP_CONCAT(DISTINCT(c.refpagprov)),',',''), IF(p.tiporefpago = 2, REPLACE(GROUP_CONCAT(DISTINCT(c.folioprov)),',',''), IF(p.tipoempre = 1, p.razonsocial, p.replegal)))),1,30),30,' ') AS motivo, \
				'C' AS tipo, \
				GROUP_CONCAT(c.referencia) AS referencias, \
				p.proveedor, \
				SUM(cs.saldor) AS total, \
				bc.idnumcuenta AS numerocuenta, \
				GROUP_CONCAT(cs.saldor) AS saldo \
			FROM proveedores p \
			INNER JOIN compras c \
			INNER JOIN auxcomprasprove cs \
			LEFT JOIN tiposcuentasbancarias tc1 ON tc1.clave = p.tipocuenta1 \
			LEFT JOIN tiposcuentasbancarias tc2 ON tc2.clave = p.tipocuenta2 \
			LEFT JOIN tiposcuentasbancarias tc3 ON tc3.clave = p.tipocuenta3 \
			RIGHT JOIN bancoscuentas bc ON bc.principal = 1 \
			WHERE p.proveedor=c.proveedor %s %s \
			AND cs.compra=c.referencia AND cs.saldor>0 AND \
			c.fechacom>='%s' AND c.fechacom<='%s' AND p.agrupapagfact = 1 \
			GROUP BY p.proveedor \
			HAVING clavepagobnc = 'PTC' \
			) UNION ALL ( \
			SELECT \
				IF(p.cuentadefault = 1, tc1.clave, IF(p.cuentadefault = 2, tc2.clave, tc3.clave)) AS clavepagobnc, \
				LPAD(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),18,'0') AS asuntobenef, \
				LPAD(bc.numerocuenta,18,'0') AS asuntoorden, \
				'MXP' AS divisa, \
				LPAD(SUM(cs.saldor),16,'0') AS importe, \
				RPAD(SUBSTRING(IF(p.tiporefpago = 0, p.referenciafija, IF(p.tiporefpago = 1, REPLACE(GROUP_CONCAT(DISTINCT(c.refpagprov)),',',''), IF(p.tiporefpago = 2, REPLACE(GROUP_CONCAT(DISTINCT(c.folioprov)),',',''), IF(p.tipoempre = 1, p.razonsocial, p.replegal)))),1,30),30,' ') AS motivo, \
				'C' AS tipo, \
				GROUP_CONCAT(c.referencia) AS referencias, \
				p.proveedor, \
				SUM(cs.saldor) AS total, \
				bc.idnumcuenta AS numerocuenta, \
				GROUP_CONCAT(cs.saldor) AS saldo \
			FROM proveedores p \
			INNER JOIN compras c \
			INNER JOIN auxcomprasprove cs \
			LEFT JOIN tiposcuentasbancarias tc1 ON tc1.clave = p.tipocuenta1 \
			LEFT JOIN tiposcuentasbancarias tc2 ON tc2.clave = p.tipocuenta2 \
			LEFT JOIN tiposcuentasbancarias tc3 ON tc3.clave = p.tipocuenta3 \
			RIGHT JOIN bancoscuentas bc ON bc.principal = 1 \
			WHERE p.proveedor=c.proveedor %s %s \
			AND cs.compra=c.referencia AND cs.saldor>0 AND \
			c.fechacom>='%s' AND c.fechacom<='%s' AND p.agrupapagfact = 0 \
			GROUP BY c.referencia \
			HAVING clavepagobnc = 'PTC' \
			)) t ",
			muestrasolofacturassincheque, condicion_mostraruuid,
			mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin),

			muestrasolofacturassincheque, condicion_mostraruuid,
			mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin));
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			instruccion.sprintf("SELECT t.clavepagobnc, t.asuntobenef, t.asuntoorden, \
			t.divisa, t.importe, t.titularasuntoben, t.tipocuenta, t.numbancbenef, t.motivo, \
			t.refnum, t.disponibilidad, t.tipo, t.referencias, t.proveedor, t.total, t.numerocuenta, t.saldo \
			FROM ( \
			(SELECT \
				IF(p.cuentadefault = 1, tc1.clave, IF(p.cuentadefault = 2, tc2.clave, tc3.clave)) AS clavepagobnc, \
				LPAD(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),18,'0') AS asuntobenef, \
				LPAD(bc.numerocuenta,18,'0') AS asuntoorden, \
				'MXP' AS divisa, \
				LPAD(SUM(cs.saldor),16,'0') AS importe, \
				RPAD(SUBSTRING(IF(p.tipoempre = 1, p.razonsocial, p.replegal),1,30),30,' ') AS titularasuntoben, \
				'40' AS tipocuenta, \
				SUBSTRING(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),1,3) AS numbancbenef, \
				RPAD(SUBSTRING(IF(p.tiporefpago = 0, p.referenciafija, IF(p.tiporefpago = 1, REPLACE(GROUP_CONCAT(DISTINCT(c.refpagprov)),',',''), IF(p.tiporefpago = 2, REPLACE(GROUP_CONCAT(DISTINCT(c.folioprov)),',',''), IF(p.tipoempre = 1, pcfd.nombreemisor, p.replegal)))),1,30),30,' ') AS motivo, \
				RPAD('',7,' ') AS refnum, \
				'H' AS disponibilidad, \
				'C' AS tipo, \
				GROUP_CONCAT(c.referencia) AS referencias, \
				p.proveedor, \
				SUM(cs.saldor) AS total, \
				bc.idnumcuenta AS numerocuenta, \
				GROUP_CONCAT(cs.saldor) AS saldo \
			FROM proveedores p \
			INNER JOIN compras c \
			INNER JOIN auxcomprasprove cs \
			LEFT JOIN tiposcuentasbancarias tc1 ON tc1.clave = p.tipocuenta1 \
			LEFT JOIN tiposcuentasbancarias tc2 ON tc2.clave = p.tipocuenta2 \
			LEFT JOIN tiposcuentasbancarias tc3 ON tc3.clave = p.tipocuenta3 \
			LEFT JOIN parametroscfd pcfd ON pcfd.sucursal = '%s' \
			RIGHT JOIN bancoscuentas bc ON bc.principal = 1 \
			WHERE p.proveedor=c.proveedor %s %s \
			AND cs.compra=c.referencia AND cs.saldor>0 AND \
			c.fechacom>='%s' AND c.fechacom<='%s' AND p.agrupapagfact = 1 \
			GROUP BY p.proveedor \
			HAVING clavepagobnc = 'PSC' \
			) UNION ALL ( \
			SELECT \
				IF(p.cuentadefault = 1, tc1.clave, IF(p.cuentadefault = 2, tc2.clave, tc3.clave)) AS clavepagobnc, \
				LPAD(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),18,'0') AS asuntobenef, \
				LPAD(bc.numerocuenta,18,'0') AS asuntoorden, \
				'MXP' AS divisa, \
				LPAD(SUM(cs.saldor),16,'0') AS importe, \
				RPAD(SUBSTRING(IF(p.tipoempre = 1, p.razonsocial, p.replegal),1,30),30,' ') AS titularasuntoben, \
				'40' AS tipocuenta, \
				SUBSTRING(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),1,3) AS numbancbenef, \
				RPAD(SUBSTRING(IF(p.tiporefpago = 0, p.referenciafija, IF(p.tiporefpago = 1, REPLACE(GROUP_CONCAT(DISTINCT(c.refpagprov)),',',''), IF(p.tiporefpago = 2, REPLACE(GROUP_CONCAT(DISTINCT(c.folioprov)),',',''), IF(p.tipoempre = 1, pcfd.nombreemisor, p.replegal)))),1,30),30,' ') AS motivo, \
				RPAD('',7,' ') AS refnum, \
				'H' AS disponibilidad, \
				'C' AS tipo, \
				GROUP_CONCAT(c.referencia) AS referencias, \
				p.proveedor, \
				SUM(cs.saldor) AS total, \
				bc.idnumcuenta AS numerocuenta, \
				GROUP_CONCAT(cs.saldor) AS saldo \
			FROM proveedores p \
			INNER JOIN compras c \
			INNER JOIN auxcomprasprove cs \
			LEFT JOIN tiposcuentasbancarias tc1 ON tc1.clave = p.tipocuenta1 \
			LEFT JOIN tiposcuentasbancarias tc2 ON tc2.clave = p.tipocuenta2 \
			LEFT JOIN tiposcuentasbancarias tc3 ON tc3.clave = p.tipocuenta3 \
			LEFT JOIN parametroscfd pcfd ON pcfd.sucursal = '%s' \
			RIGHT JOIN bancoscuentas bc ON bc.principal = 1 \
			WHERE p.proveedor=c.proveedor %s %s \
			AND cs.compra=c.referencia AND cs.saldor>0 AND \
			c.fechacom>='%s' AND c.fechacom<='%s' AND p.agrupapagfact = 0 \
			GROUP BY c.referencia \
			HAVING clavepagobnc = 'PSC' \
			)) t ",
			FormServidor->ObtieneClaveSucursal(),
			muestrasolofacturassincheque, condicion_mostraruuid,
			mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin),

			FormServidor->ObtieneClaveSucursal(),
			muestrasolofacturassincheque, condicion_mostraruuid,
			mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin));
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			instruccion.sprintf("SELECT t.clavepagobnc, t.conceptocie, t.conveniocie, t.asuntoorden, \
			t.importe, t.motivo, t.refcie, t.tipo, t.referencias, t.proveedor, t.total, t.numerocuenta, t.saldo \
			FROM ( \
			(SELECT \
				IF(p.cuentadefault = 1, tc1.clave, IF(p.cuentadefault = 2, tc2.clave, tc3.clave)) AS clavepagobnc, \
				RPAD(SUBSTRING(IF(p.tipoempre = 1, p.razonsocial, p.replegal),1,30),30,' ') AS conceptocie, \
				LPAD(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),7,'0') AS conveniocie, \
				LPAD(bc.numerocuenta,18,'0') AS asuntoorden, \
				LPAD(SUM(cs.saldor),16,'0') AS importe, \
				RPAD('',30,' ') AS motivo, \
				RPAD(SUBSTRING(IF(p.tiporefpago = 0, p.referenciafija, IF(p.tiporefpago = 1, REPLACE(GROUP_CONCAT(DISTINCT(c.refpagprov)),',',''), IF(p.tiporefpago = 2, REPLACE(GROUP_CONCAT(DISTINCT(c.folioprov)),',',''), IF(p.tipoempre = 1, p.razonsocial, p.replegal)))),1,20),20,' ') AS refcie, \
				'C' AS tipo, \
				GROUP_CONCAT(c.referencia) AS referencias, \
				p.proveedor, \
				SUM(cs.saldor) AS total, \
				bc.idnumcuenta AS numerocuenta, \
				GROUP_CONCAT(cs.saldor) AS saldo \
			FROM proveedores p \
			INNER JOIN compras c \
			INNER JOIN auxcomprasprove cs \
			LEFT JOIN tiposcuentasbancarias tc1 ON tc1.clave = p.tipocuenta1 \
			LEFT JOIN tiposcuentasbancarias tc2 ON tc2.clave = p.tipocuenta2 \
			LEFT JOIN tiposcuentasbancarias tc3 ON tc3.clave = p.tipocuenta3 \
			RIGHT JOIN bancoscuentas bc ON bc.principal = 1 \
			WHERE p.proveedor=c.proveedor %s %s \
			AND cs.compra=c.referencia AND cs.saldor>0 AND \
			c.fechacom>='%s' AND c.fechacom<='%s' AND p.agrupapagfact = 1 \
			GROUP BY p.proveedor \
			HAVING clavepagobnc = 'CIL' \
			) UNION ALL ( \
			SELECT \
				IF(p.cuentadefault = 1, tc1.clave, IF(p.cuentadefault = 2, tc2.clave, tc3.clave)) AS clavepagobnc, \
				RPAD(SUBSTRING(IF(p.tipoempre = 1, p.razonsocial, p.replegal),1,30),30,' ') AS conceptocie, \
				LPAD(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),7,'0') AS conveniocie, \
				LPAD(bc.numerocuenta,18,'0') AS asuntoorden, \
				LPAD(SUM(cs.saldor),16,'0') AS importe, \
				RPAD('',30,' ') AS motivo, \
				RPAD(SUBSTRING(IF(p.tiporefpago = 0, p.referenciafija, IF(p.tiporefpago = 1, REPLACE(GROUP_CONCAT(DISTINCT(c.refpagprov)),',',''), IF(p.tiporefpago = 2, REPLACE(GROUP_CONCAT(DISTINCT(c.folioprov)),',',''), IF(p.tipoempre = 1, p.razonsocial, p.replegal)))),1,20),20,' ') AS refcie, \
				'C' AS tipo, \
				GROUP_CONCAT(c.referencia) AS referencias, \
				p.proveedor, \
				SUM(cs.saldor) AS total, \
				bc.idnumcuenta AS numerocuenta, \
				GROUP_CONCAT(cs.saldor) AS saldo \
			FROM proveedores p \
			INNER JOIN compras c \
			INNER JOIN auxcomprasprove cs \
			LEFT JOIN tiposcuentasbancarias tc1 ON tc1.clave = p.tipocuenta1 \
			LEFT JOIN tiposcuentasbancarias tc2 ON tc2.clave = p.tipocuenta2 \
			LEFT JOIN tiposcuentasbancarias tc3 ON tc3.clave = p.tipocuenta3 \
			RIGHT JOIN bancoscuentas bc ON bc.principal = 1 \
			WHERE p.proveedor=c.proveedor %s %s \
			AND cs.compra=c.referencia AND cs.saldor>0 AND \
			c.fechacom>='%s' AND c.fechacom<='%s' AND p.agrupapagfact = 0 \
			GROUP BY c.referencia \
			HAVING clavepagobnc = 'CIL' \
			)) t ",
			muestrasolofacturassincheque, condicion_mostraruuid,
			mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin),

			muestrasolofacturassincheque, condicion_mostraruuid,
			mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin));
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		} else {
			instruccion.sprintf("SELECT \
				IF(p.cuentadefault = 1, tc1.clave, IF(p.cuentadefault = 2, tc2.clave, tc3.clave)) AS clavepagobnc, \
				LPAD(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),18,'0') AS asuntobenef, \
				LPAD(bc.numerocuenta,18,'0') AS asuntoorden, \
				'MXP' AS divisa, \
				LPAD(SUM(cs.saldor),16,'0') AS importe, \
				RPAD(SUBSTRING(IF(p.tiporefpago = 0, p.referenciafija, IF(p.tiporefpago = 1, REPLACE(GROUP_CONCAT(DISTINCT(c.refpagprov)),',',''), IF(p.tiporefpago = 2, REPLACE(GROUP_CONCAT(DISTINCT(c.folioprov)),',',''), IF(p.tipoempre = 1, p.razonsocial, p.replegal)))),1,30),30,' ') AS motivo, \
				'C' AS tipo, \
				(c.referencia) AS referencias, \
				p.proveedor, \
				SUM(cs.saldor) AS total, \
				bc.idnumcuenta AS numerocuenta, \
				(cs.saldor) AS saldo \
			FROM proveedores p \
			INNER JOIN compras c \
			INNER JOIN auxcomprasprove cs \
			LEFT JOIN tiposcuentasbancarias tc1 ON tc1.clave = p.tipocuenta1 \
			LEFT JOIN tiposcuentasbancarias tc2 ON tc2.clave = p.tipocuenta2 \
			LEFT JOIN tiposcuentasbancarias tc3 ON tc3.clave = p.tipocuenta3 \
			RIGHT JOIN bancoscuentas bc ON bc.principal = 1 \
			WHERE p.proveedor=c.proveedor %s %s \
			AND cs.compra=c.referencia AND cs.saldor>0 AND \
			c.fechacom>='%s' AND c.fechacom<='%s' \
			GROUP BY c.referencia \
			HAVING clavepagobnc = 'PTC' ",
			muestrasolofacturassincheque, condicion_mostraruuid,
			mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin));
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			instruccion.sprintf("SELECT \
				IF(p.cuentadefault = 1, tc1.clave, IF(p.cuentadefault = 2, tc2.clave, tc3.clave)) AS clavepagobnc, \
				LPAD(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),18,'0') AS asuntobenef, \
				LPAD(bc.numerocuenta,18,'0') AS asuntoorden, \
				'MXP' AS divisa, \
				LPAD(SUM(cs.saldor),16,'0') AS importe, \
				RPAD(SUBSTRING(IF(p.tipoempre = 1, p.razonsocial, p.replegal),1,30),30,' ') AS titularasuntoben, \
				'40' AS tipocuenta, \
				SUBSTRING(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),1,3) AS numbancbenef, \
				RPAD(SUBSTRING(IF(p.tiporefpago = 0, p.referenciafija, IF(p.tiporefpago = 1, REPLACE(GROUP_CONCAT(DISTINCT(c.refpagprov)),',',''), IF(p.tiporefpago = 2, REPLACE(GROUP_CONCAT(DISTINCT(c.folioprov)),',',''), IF(p.tipoempre = 1, pcfd.nombreemisor, p.replegal)))),1,30),30,' ') AS motivo, \
				RPAD('',7,' ') AS refnum, \
				'H' AS disponibilidad, \
				'C' AS tipo, \
				(c.referencia) AS referencias, \
				p.proveedor, \
				SUM(cs.saldor) AS total, \
				bc.idnumcuenta AS numerocuenta, \
				(cs.saldor) AS saldo \
			FROM proveedores p \
			INNER JOIN compras c \
			INNER JOIN auxcomprasprove cs \
			LEFT JOIN tiposcuentasbancarias tc1 ON tc1.clave = p.tipocuenta1 \
			LEFT JOIN tiposcuentasbancarias tc2 ON tc2.clave = p.tipocuenta2 \
			LEFT JOIN tiposcuentasbancarias tc3 ON tc3.clave = p.tipocuenta3 \
			LEFT JOIN parametroscfd pcfd ON pcfd.sucursal = '%s' \
			RIGHT JOIN bancoscuentas bc ON bc.principal = 1 \
			WHERE p.proveedor=c.proveedor %s %s \
			AND cs.compra=c.referencia AND cs.saldor>0 AND \
			c.fechacom>='%s' AND c.fechacom<='%s' \
			GROUP BY c.referencia \
			HAVING clavepagobnc = 'PSC' ",
			FormServidor->ObtieneClaveSucursal(),
			muestrasolofacturassincheque, condicion_mostraruuid,
			mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin));
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			instruccion.sprintf("SELECT \
				IF(p.cuentadefault = 1, tc1.clave, IF(p.cuentadefault = 2, tc2.clave, tc3.clave)) AS clavepagobnc, \
				RPAD(SUBSTRING(IF(p.tipoempre = 1, p.razonsocial, p.replegal),1,30),30,' ') AS conceptocie, \
				LPAD(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),7,'0') AS conveniocie, \
				LPAD(bc.numerocuenta,18,'0') AS asuntoorden, \
				LPAD(SUM(cs.saldor),16,'0') AS importe, \
				RPAD('',30,' ') AS motivo, \
				RPAD(SUBSTRING(IF(p.tiporefpago = 0, p.referenciafija, IF(p.tiporefpago = 1, REPLACE(GROUP_CONCAT(DISTINCT(c.refpagprov)),',',''), IF(p.tiporefpago = 2, REPLACE(GROUP_CONCAT(DISTINCT(c.folioprov)),',',''), IF(p.tipoempre = 1, p.razonsocial, p.replegal)))),1,20),20,' ') AS refcie, \
				'C' AS tipo, \
				(c.referencia) AS referencias, \
				p.proveedor, \
				SUM(cs.saldor) AS total, \
				bc.idnumcuenta AS numerocuenta, \
				(cs.saldor) AS saldo \
			FROM proveedores p \
			INNER JOIN compras c \
			INNER JOIN auxcomprasprove cs \
			LEFT JOIN tiposcuentasbancarias tc1 ON tc1.clave = p.tipocuenta1 \
			LEFT JOIN tiposcuentasbancarias tc2 ON tc2.clave = p.tipocuenta2 \
			LEFT JOIN tiposcuentasbancarias tc3 ON tc3.clave = p.tipocuenta3 \
			RIGHT JOIN bancoscuentas bc ON bc.principal = 1 \
			WHERE p.proveedor=c.proveedor %s %s \
			AND cs.compra=c.referencia AND cs.saldor>0 AND \
			c.fechacom>='%s' AND c.fechacom<='%s' \
			GROUP BY c.referencia \
			HAVING clavepagobnc = 'CIL' ",
			muestrasolofacturassincheque, condicion_mostraruuid,
			mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin));
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	} else if (tipospagos == "1") {

			if(agrupaprov==1){
				instruccion.sprintf("SELECT t.clavepagobnc, t.asuntobenef, t.asuntoorden, \
				t.divisa, t.importe, t.motivo, t.tipo, t.referencias, t.proveedor, t.total, t.numerocuenta, t.saldo \
				FROM ( \
				(SELECT \
					IF(p.cuentadefault = 1, tc1.clave, IF(p.cuentadefault = 2, tc2.clave, tc3.clave)) AS clavepagobnc, \
					LPAD(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),18,'0') AS asuntobenef, \
					LPAD(bc.numerocuenta,18,'0') AS asuntoorden, \
					'MXP' AS divisa, \
					LPAD(SUM(cs.saldor),16,'0') AS importe, \
					RPAD(SUBSTRING(IF(p.tiporefpago = 0, p.referenciafija, IF(p.tiporefpago = 1, REPLACE(GROUP_CONCAT(DISTINCT(c.refpagprov)),',',''), IF(p.tiporefpago = 2, REPLACE(GROUP_CONCAT(DISTINCT(c.folioprov)),',',''), IF(p.tipoempre = 1, p.razonsocial, p.replegal)))),1,30),30,' ') AS motivo, \
					'G' AS tipo, \
					GROUP_CONCAT(c.referencia) AS referencias, \
					p.proveedor, \
					SUM(cs.saldor) AS total, \
					bc.idnumcuenta AS numerocuenta, \
					GROUP_CONCAT(cs.saldor) AS saldo \
				FROM proveedores p \
				INNER JOIN gastos c \
				INNER JOIN auxgastosprove cs \
				LEFT JOIN tiposcuentasbancarias tc1 ON tc1.clave = p.tipocuenta1 \
				LEFT JOIN tiposcuentasbancarias tc2 ON tc2.clave = p.tipocuenta2 \
				LEFT JOIN tiposcuentasbancarias tc3 ON tc3.clave = p.tipocuenta3 \
				RIGHT JOIN bancoscuentas bc ON bc.principal = 1 \
				WHERE p.proveedor=c.proveedor %s %s \
				AND cs.gasto=c.referencia AND cs.saldor>0 AND \
				c.fechagas>='%s' AND c.fechagas<='%s' AND p.agrupapaggast = 1 \
				GROUP BY p.proveedor \
				HAVING clavepagobnc = 'PTC' \
				) UNION ALL ( \
				SELECT \
					IF(p.cuentadefault = 1, tc1.clave, IF(p.cuentadefault = 2, tc2.clave, tc3.clave)) AS clavepagobnc, \
					LPAD(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),18,'0') AS asuntobenef, \
					LPAD(bc.numerocuenta,18,'0') AS asuntoorden, \
					'MXP' AS divisa, \
					LPAD(SUM(cs.saldor),16,'0') AS importe, \
					RPAD(SUBSTRING(IF(p.tiporefpago = 0, p.referenciafija, IF(p.tiporefpago = 1, REPLACE(GROUP_CONCAT(DISTINCT(c.refpagprov)),',',''), IF(p.tiporefpago = 2, REPLACE(GROUP_CONCAT(DISTINCT(c.folioprov)),',',''), IF(p.tipoempre = 1, p.razonsocial, p.replegal)))),1,30),30,' ') AS motivo, \
					'G' AS tipo, \
					GROUP_CONCAT(c.referencia) AS referencias, \
					p.proveedor, \
					SUM(cs.saldor) AS total, \
					bc.idnumcuenta AS numerocuenta, \
					GROUP_CONCAT(cs.saldor) AS saldo \
				FROM proveedores p \
				INNER JOIN gastos c \
				INNER JOIN auxgastosprove cs \
				LEFT JOIN tiposcuentasbancarias tc1 ON tc1.clave = p.tipocuenta1 \
				LEFT JOIN tiposcuentasbancarias tc2 ON tc2.clave = p.tipocuenta2 \
				LEFT JOIN tiposcuentasbancarias tc3 ON tc3.clave = p.tipocuenta3 \
				RIGHT JOIN bancoscuentas bc ON bc.principal = 1 \
				WHERE p.proveedor=c.proveedor %s %s \
				AND cs.gasto=c.referencia AND cs.saldor>0 AND \
				c.fechagas>='%s' AND c.fechagas<='%s' AND p.agrupapaggast = 0 \
				GROUP BY c.referencia \
				HAVING clavepagobnc = 'PTC' \
				)) t ",

				muestrasolofacturassincheque, condicion_mostraruuid,
				mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin),

				muestrasolofacturassincheque, condicion_mostraruuid,
				mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin));
				mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

				instruccion.sprintf("SELECT t.clavepagobnc, t.asuntobenef, t.asuntoorden, \
				t.divisa, t.importe, t.titularasuntoben, t.tipocuenta, t.numbancbenef, t.motivo, \
				t.refnum, t.disponibilidad, t.tipo, t.referencias, t.proveedor, t.total, t.numerocuenta, t.saldo \
				FROM ( \
				(SELECT \
					IF(p.cuentadefault = 1, tc1.clave, IF(p.cuentadefault = 2, tc2.clave, tc3.clave)) AS clavepagobnc, \
					LPAD(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),18,'0') AS asuntobenef, \
					LPAD(bc.numerocuenta,18,'0') AS asuntoorden, \
					'MXP' AS divisa, \
					LPAD(SUM(cs.saldor),16,'0') AS importe, \
					RPAD(SUBSTRING(IF(p.tipoempre = 1, p.razonsocial, p.replegal),1,30),30,' ') AS titularasuntoben, \
					'40' AS tipocuenta, \
					SUBSTRING(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),1,3) AS numbancbenef, \
					RPAD(SUBSTRING(IF(p.tiporefpago = 0, p.referenciafija, IF(p.tiporefpago = 1, REPLACE(GROUP_CONCAT(DISTINCT(c.refpagprov)),',',''), IF(p.tiporefpago = 2, REPLACE(GROUP_CONCAT(DISTINCT(c.folioprov)),',',''), IF(p.tipoempre = 1, pcfd.nombreemisor, p.replegal)))),1,30),30,' ') AS motivo, \
					RPAD('',7,' ') AS refnum, \
					'H' AS disponibilidad, \
					'G' AS tipo, \
					GROUP_CONCAT(c.referencia) AS referencias, \
					p.proveedor, \
					SUM(cs.saldor) AS total, \
					bc.idnumcuenta AS numerocuenta, \
					GROUP_CONCAT(cs.saldor) AS saldo \
				FROM proveedores p \
				INNER JOIN gastos c \
				INNER JOIN auxgastosprove cs \
				LEFT JOIN tiposcuentasbancarias tc1 ON tc1.clave = p.tipocuenta1 \
				LEFT JOIN tiposcuentasbancarias tc2 ON tc2.clave = p.tipocuenta2 \
				LEFT JOIN tiposcuentasbancarias tc3 ON tc3.clave = p.tipocuenta3 \
				LEFT JOIN parametroscfd pcfd ON pcfd.sucursal = '%s' \
				RIGHT JOIN bancoscuentas bc ON bc.principal = 1 \
				WHERE p.proveedor=c.proveedor %s %s \
				AND cs.gasto=c.referencia AND cs.saldor>0 AND \
				c.fechagas>='%s' AND c.fechagas<='%s' AND p.agrupapaggast = 1 \
				GROUP BY p.proveedor \
				HAVING clavepagobnc = 'PSC' \
				) UNION ALL ( \
				SELECT \
					IF(p.cuentadefault = 1, tc1.clave, IF(p.cuentadefault = 2, tc2.clave, tc3.clave)) AS clavepagobnc, \
					LPAD(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),18,'0') AS asuntobenef, \
					LPAD(bc.numerocuenta,18,'0') AS asuntoorden, \
					'MXP' AS divisa, \
					LPAD(SUM(cs.saldor),16,'0') AS importe, \
					RPAD(SUBSTRING(IF(p.tipoempre = 1, p.razonsocial, p.replegal),1,30),30,' ') AS titularasuntoben, \
					'40' AS tipocuenta, \
					SUBSTRING(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),1,3) AS numbancbenef, \
					RPAD(SUBSTRING(IF(p.tiporefpago = 0, p.referenciafija, IF(p.tiporefpago = 1, REPLACE(GROUP_CONCAT(DISTINCT(c.refpagprov)),',',''), IF(p.tiporefpago = 2, REPLACE(GROUP_CONCAT(DISTINCT(c.folioprov)),',',''), IF(p.tipoempre = 1, pcfd.nombreemisor, p.replegal)))),1,30),30,' ') AS motivo, \
					RPAD('',7,' ') AS refnum, \
					'H' AS disponibilidad, \
					'G' AS tipo, \
					GROUP_CONCAT(c.referencia) AS referencias, \
					p.proveedor, \
					SUM(cs.saldor) AS total, \
					bc.idnumcuenta AS numerocuenta, \
					GROUP_CONCAT(cs.saldor) AS saldo \
				FROM proveedores p \
				INNER JOIN gastos c \
				INNER JOIN auxgastosprove cs \
				LEFT JOIN tiposcuentasbancarias tc1 ON tc1.clave = p.tipocuenta1 \
				LEFT JOIN tiposcuentasbancarias tc2 ON tc2.clave = p.tipocuenta2 \
				LEFT JOIN tiposcuentasbancarias tc3 ON tc3.clave = p.tipocuenta3 \
				LEFT JOIN parametroscfd pcfd ON pcfd.sucursal = '%s' \
				RIGHT JOIN bancoscuentas bc ON bc.principal = 1 \
				WHERE p.proveedor=c.proveedor %s %s \
				AND cs.gasto=c.referencia AND cs.saldor>0 AND \
				c.fechagas>='%s' AND c.fechagas<='%s' AND p.agrupapaggast = 0 \
				GROUP BY c.referencia \
				HAVING clavepagobnc = 'PSC' \
				)) t ",

				FormServidor->ObtieneClaveSucursal(),
				muestrasolofacturassincheque, condicion_mostraruuid,
				mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin),

				FormServidor->ObtieneClaveSucursal(),
				muestrasolofacturassincheque, condicion_mostraruuid,
				mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin));
				mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

				instruccion.sprintf("SELECT t.clavepagobnc, t.conceptocie, t.conveniocie, t.asuntoorden, \
				t.importe, t.motivo, t.refcie, t.tipo, t.referencias, t.proveedor, t.total, t.numerocuenta, t.saldo \
				FROM ( \
				(SELECT \
					IF(p.cuentadefault = 1, tc1.clave, IF(p.cuentadefault = 2, tc2.clave, tc3.clave)) AS clavepagobnc, \
					RPAD(SUBSTRING(IF(p.tipoempre = 1, p.razonsocial, p.replegal),1,30),30,' ') AS conceptocie, \
					LPAD(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),7,'0') AS conveniocie, \
					LPAD(bc.numerocuenta,18,'0') AS asuntoorden, \
					LPAD(SUM(cs.saldor),16,'0') AS importe, \
					RPAD('',30,' ') AS motivo, \
					RPAD(SUBSTRING(IF(p.tiporefpago = 0, p.referenciafija, IF(p.tiporefpago = 1, REPLACE(GROUP_CONCAT(DISTINCT(c.refpagprov)),',',''), IF(p.tiporefpago = 2, REPLACE(GROUP_CONCAT(DISTINCT(c.folioprov)),',',''), IF(p.tipoempre = 1, p.razonsocial, p.replegal)))),1,20),20,' ') AS refcie, \
					'G' AS tipo, \
					GROUP_CONCAT(c.referencia) AS referencias, \
					p.proveedor, \
					SUM(cs.saldor) AS total, \
					bc.idnumcuenta AS numerocuenta, \
					GROUP_CONCAT(cs.saldor) AS saldo \
				FROM proveedores p \
				INNER JOIN gastos c \
				INNER JOIN auxgastosprove cs \
				LEFT JOIN tiposcuentasbancarias tc1 ON tc1.clave = p.tipocuenta1 \
				LEFT JOIN tiposcuentasbancarias tc2 ON tc2.clave = p.tipocuenta2 \
				LEFT JOIN tiposcuentasbancarias tc3 ON tc3.clave = p.tipocuenta3 \
				RIGHT JOIN bancoscuentas bc ON bc.principal = 1 \
				WHERE p.proveedor=c.proveedor %s %s \
				AND cs.gasto=c.referencia AND cs.saldor>0 AND \
				c.fechagas>='%s' AND c.fechagas<='%s' AND p.agrupapaggast = 1 \
				GROUP BY p.proveedor \
				HAVING clavepagobnc = 'CIL' \
				) UNION ALL ( \
				SELECT \
					IF(p.cuentadefault = 1, tc1.clave, IF(p.cuentadefault = 2, tc2.clave, tc3.clave)) AS clavepagobnc, \
					RPAD(SUBSTRING(IF(p.tipoempre = 1, p.razonsocial, p.replegal),1,30),30,' ') AS conceptocie, \
					LPAD(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),7,'0') AS conveniocie, \
					LPAD(bc.numerocuenta,18,'0') AS asuntoorden, \
					LPAD(SUM(cs.saldor),16,'0') AS importe, \
					RPAD('',30,' ') AS motivo, \
					RPAD(SUBSTRING(IF(p.tiporefpago = 0, p.referenciafija, IF(p.tiporefpago = 1, REPLACE(GROUP_CONCAT(DISTINCT(c.refpagprov)),',',''), IF(p.tiporefpago = 2, REPLACE(GROUP_CONCAT(DISTINCT(c.folioprov)),',',''), IF(p.tipoempre = 1, p.razonsocial, p.replegal)))),1,20),20,' ') AS refcie, \
					'G' AS tipo, \
					GROUP_CONCAT(c.referencia) AS referencias, \
					p.proveedor, \
					SUM(cs.saldor) AS total, \
					bc.idnumcuenta AS numerocuenta, \
					GROUP_CONCAT(cs.saldor) AS saldo \
				FROM proveedores p \
				INNER JOIN gastos c \
				INNER JOIN auxgastosprove cs \
				LEFT JOIN tiposcuentasbancarias tc1 ON tc1.clave = p.tipocuenta1 \
				LEFT JOIN tiposcuentasbancarias tc2 ON tc2.clave = p.tipocuenta2 \
				LEFT JOIN tiposcuentasbancarias tc3 ON tc3.clave = p.tipocuenta3 \
				RIGHT JOIN bancoscuentas bc ON bc.principal = 1 \
				WHERE p.proveedor=c.proveedor %s %s \
				AND cs.gasto=c.referencia AND cs.saldor>0 AND \
				c.fechagas>='%s' AND c.fechagas<='%s' AND p.agrupapaggast = 0 \
				GROUP BY c.referencia \
				HAVING clavepagobnc = 'CIL' \
				)) t ",

				muestrasolofacturassincheque, condicion_mostraruuid,
				mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin),

				muestrasolofacturassincheque, condicion_mostraruuid,
				mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin));
				mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
			}else{
				instruccion.sprintf("SELECT \
				IF(p.cuentadefault = 1, tc1.clave, IF(p.cuentadefault = 2, tc2.clave, tc3.clave)) AS clavepagobnc, \
				LPAD(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),18,'0') AS asuntobenef, \
				LPAD(bc.numerocuenta,18,'0') AS asuntoorden, \
				'MXP' AS divisa, \
				LPAD(SUM(cs.saldor),16,'0') AS importe, \
				RPAD(SUBSTRING(IF(p.tiporefpago = 0, p.referenciafija, IF(p.tiporefpago = 1, REPLACE(GROUP_CONCAT(DISTINCT(c.refpagprov)),',',''), IF(p.tiporefpago = 2, REPLACE(GROUP_CONCAT(DISTINCT(c.folioprov)),',',''), IF(p.tipoempre = 1, p.razonsocial, p.replegal)))),1,30),30,' ') AS motivo, \
				'G' AS tipo, \
				c.referencia AS referencias, \
				p.proveedor, \
				SUM(cs.saldor) AS total, \
				bc.idnumcuenta AS numerocuenta, \
				cs.saldor AS saldo \
				FROM proveedores p \
				INNER JOIN gastos c \
				INNER JOIN auxgastosprove cs \
				LEFT JOIN tiposcuentasbancarias tc1 ON tc1.clave = p.tipocuenta1 \
				LEFT JOIN tiposcuentasbancarias tc2 ON tc2.clave = p.tipocuenta2 \
				LEFT JOIN tiposcuentasbancarias tc3 ON tc3.clave = p.tipocuenta3 \
				RIGHT JOIN bancoscuentas bc ON bc.principal = 1 \
				WHERE p.proveedor=c.proveedor %s %s \
				AND cs.gasto=c.referencia AND cs.saldor>0 AND \
				c.fechagas>='%s' AND c.fechagas<='%s' GROUP BY c.referencia \
				HAVING clavepagobnc = 'PTC'  ",
				muestrasolofacturassincheque, condicion_mostraruuid,
				mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin));
				mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

				instruccion.sprintf("SELECT \
				IF(p.cuentadefault = 1, tc1.clave, IF(p.cuentadefault = 2, tc2.clave, tc3.clave)) AS clavepagobnc, \
				LPAD(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),18,'0') AS asuntobenef, \
				LPAD(bc.numerocuenta,18,'0') AS asuntoorden, \
				'MXP' AS divisa, \
				LPAD(SUM(cs.saldor),16,'0') AS importe, \
				RPAD(SUBSTRING(IF(p.tipoempre = 1, p.razonsocial, p.replegal),1,30),30,' ') AS titularasuntoben, \
				'40' AS tipocuenta, \
				SUBSTRING(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),1,3) AS numbancbenef, \
				RPAD(SUBSTRING(IF(p.tiporefpago = 0, p.referenciafija, IF(p.tiporefpago = 1, REPLACE(GROUP_CONCAT(DISTINCT(c.refpagprov)),',',''), IF(p.tiporefpago = 2, REPLACE(GROUP_CONCAT(DISTINCT(c.folioprov)),',',''), IF(p.tipoempre = 1, pcfd.nombreemisor, p.replegal)))),1,30),30,' ') AS motivo, \
				RPAD('',7,' ') AS refnum, \
				'H' AS disponibilidad, \
				'G' AS tipo, \
				c.referencia AS referencias, \
				p.proveedor, \
				SUM(cs.saldor) AS total, \
				bc.idnumcuenta AS numerocuenta, \
				cs.saldor AS saldo \
				FROM proveedores p \
				INNER JOIN gastos c \
				INNER JOIN auxgastosprove cs \
				LEFT JOIN tiposcuentasbancarias tc1 ON tc1.clave = p.tipocuenta1 \
				LEFT JOIN tiposcuentasbancarias tc2 ON tc2.clave = p.tipocuenta2 \
				LEFT JOIN tiposcuentasbancarias tc3 ON tc3.clave = p.tipocuenta3 \
				LEFT JOIN parametroscfd pcfd ON pcfd.sucursal = '%s' \
				RIGHT JOIN bancoscuentas bc ON bc.principal = 1 \
				WHERE p.proveedor=c.proveedor %s %s \
				AND cs.gasto=c.referencia AND cs.saldor>0 AND \
				c.fechagas>='%s' AND c.fechagas<='%s' GROUP BY c.referencia \
				HAVING clavepagobnc = 'PSC'  ",
				FormServidor->ObtieneClaveSucursal(),
				muestrasolofacturassincheque, condicion_mostraruuid,
				mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin));
				mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

				instruccion.sprintf("SELECT \
				IF(p.cuentadefault = 1, tc1.clave, IF(p.cuentadefault = 2, tc2.clave, tc3.clave)) AS clavepagobnc, \
				RPAD(SUBSTRING(IF(p.tipoempre = 1, p.razonsocial, p.replegal),1,30),30,' ') AS conceptocie, \
				LPAD(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),7,'0') AS conveniocie, \
				LPAD(bc.numerocuenta,18,'0') AS asuntoorden, \
				LPAD(SUM(cs.saldor),16,'0') AS importe, \
				RPAD('',30,' ') AS motivo, \
				RPAD(SUBSTRING(IF(p.tiporefpago = 0, p.referenciafija, IF(p.tiporefpago = 1, REPLACE(GROUP_CONCAT(DISTINCT(c.refpagprov)),',',''), IF(p.tiporefpago = 2, REPLACE(GROUP_CONCAT(DISTINCT(c.folioprov)),',',''), IF(p.tipoempre = 1, p.razonsocial, p.replegal)))),1,20),20,' ') AS refcie, \
				'G' AS tipo, \
				c.referencia AS referencias, \
				p.proveedor, \
				SUM(cs.saldor) AS total, \
				bc.idnumcuenta AS numerocuenta, \
				cs.saldor AS saldo \
				FROM proveedores p \
				INNER JOIN gastos c \
				INNER JOIN auxgastosprove cs \
				LEFT JOIN tiposcuentasbancarias tc1 ON tc1.clave = p.tipocuenta1 \
				LEFT JOIN tiposcuentasbancarias tc2 ON tc2.clave = p.tipocuenta2 \
				LEFT JOIN tiposcuentasbancarias tc3 ON tc3.clave = p.tipocuenta3 \
				RIGHT JOIN bancoscuentas bc ON bc.principal = 1 \
				WHERE p.proveedor=c.proveedor %s %s \
				AND cs.gasto=c.referencia AND cs.saldor>0 AND \
				c.fechagas>='%s' AND c.fechagas<='%s' GROUP BY c.referencia \
				HAVING clavepagobnc = 'CIL'  ",
				muestrasolofacturassincheque, condicion_mostraruuid,
				mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin));
				mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
			}
	} else {
		if(agrupaprov==1){
			instruccion.sprintf("SELECT t.clavepagobnc, t.asuntobenef, t.asuntoorden, \
			t.divisa, t.importe, t.motivo, t.tipo, t.referencias, t.proveedor, t.total, t.numerocuenta, t.saldo FROM ( \
			(SELECT \
				IF(p.cuentadefault = 1, tc1.clave, IF(p.cuentadefault = 2, tc2.clave, tc3.clave)) AS clavepagobnc, \
				LPAD(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),18,'0') AS asuntobenef, \
				LPAD(bc.numerocuenta,18,'0') AS asuntoorden, \
				'MXP' AS divisa, \
				LPAD(SUM(cs.saldor),16,'0') AS importe, \
				RPAD(SUBSTRING(IF(p.tiporefpago = 0, p.referenciafija, IF(p.tiporefpago = 1, REPLACE(GROUP_CONCAT(DISTINCT(c.refpagprov)),',',''), IF(p.tiporefpago = 2, REPLACE(GROUP_CONCAT(DISTINCT(c.folioprov)),',',''), IF(p.tipoempre = 1, p.razonsocial, p.replegal)))),1,30),30,' ') AS motivo, \
				'C' AS tipo, \
				GROUP_CONCAT(c.referencia) AS referencias, \
				p.proveedor, \
				SUM(cs.saldor) AS total, \
				bc.idnumcuenta AS numerocuenta, \
				GROUP_CONCAT(cs.saldor) AS saldo \
			FROM proveedores p \
			INNER JOIN compras c \
			INNER JOIN auxcomprasprove cs \
			LEFT JOIN tiposcuentasbancarias tc1 ON tc1.clave = p.tipocuenta1 \
			LEFT JOIN tiposcuentasbancarias tc2 ON tc2.clave = p.tipocuenta2 \
			LEFT JOIN tiposcuentasbancarias tc3 ON tc3.clave = p.tipocuenta3 \
			RIGHT JOIN bancoscuentas bc ON bc.principal = 1 \
			WHERE p.proveedor=c.proveedor %s %s \
			AND cs.compra=c.referencia AND cs.saldor>0 AND \
			c.fechacom>='%s' AND c.fechacom<='%s' AND p.agrupapagfact = 0 \
			GROUP BY c.referencia \
			HAVING clavepagobnc = 'PTC' \
			) UNION ALL ( \
			SELECT \
				IF(p.cuentadefault = 1, tc1.clave, IF(p.cuentadefault = 2, tc2.clave, tc3.clave)) AS clavepagobnc, \
				LPAD(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),18,'0') AS asuntobenef, \
				LPAD(bc.numerocuenta,18,'0') AS asuntoorden, \
				'MXP' AS divisa, \
				LPAD(SUM(cs.saldor),16,'0') AS importe, \
				RPAD(SUBSTRING(IF(p.tiporefpago = 0, p.referenciafija, IF(p.tiporefpago = 1, REPLACE(GROUP_CONCAT(DISTINCT(c.refpagprov)),',',''), IF(p.tiporefpago = 2, REPLACE(GROUP_CONCAT(DISTINCT(c.folioprov)),',',''), IF(p.tipoempre = 1, p.razonsocial, p.replegal)))),1,30),30,' ') AS motivo, \
				'C' AS tipo, \
				GROUP_CONCAT(c.referencia) AS referencias, \
				p.proveedor, \
				SUM(cs.saldor) AS total, \
				bc.idnumcuenta AS numerocuenta, \
				GROUP_CONCAT(cs.saldor) AS saldo \
			FROM proveedores p \
			INNER JOIN compras c \
			INNER JOIN auxcomprasprove cs \
			LEFT JOIN tiposcuentasbancarias tc1 ON tc1.clave = p.tipocuenta1 \
			LEFT JOIN tiposcuentasbancarias tc2 ON tc2.clave = p.tipocuenta2 \
			LEFT JOIN tiposcuentasbancarias tc3 ON tc3.clave = p.tipocuenta3 \
			RIGHT JOIN bancoscuentas bc ON bc.principal = 1 \
			WHERE p.proveedor=c.proveedor %s %s \
			AND cs.compra=c.referencia AND cs.saldor>0 AND \
			c.fechacom>='%s' AND c.fechacom<='%s' AND p.agrupapagfact = 1 \
			GROUP BY p.proveedor \
			HAVING clavepagobnc = 'PTC' \
			) UNION ALL ( \
			SELECT \
				IF(p.cuentadefault = 1, tc1.clave, IF(p.cuentadefault = 2, tc2.clave, tc3.clave)) AS clavepagobnc, \
				LPAD(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),18,'0') AS asuntobenef, \
				LPAD(bc.numerocuenta,18,'0') AS asuntoorden, \
				'MXP' AS divisa, \
				LPAD(SUM(cs.saldor),16,'0') AS importe, \
				RPAD(SUBSTRING(IF(p.tiporefpago = 0, p.referenciafija, IF(p.tiporefpago = 1, REPLACE(GROUP_CONCAT(DISTINCT(c.refpagprov)),',',''), IF(p.tiporefpago = 2, REPLACE(GROUP_CONCAT(DISTINCT(c.folioprov)),',',''), IF(p.tipoempre = 1, p.razonsocial, p.replegal)))),1,30),30,' ') AS motivo, \
				'G' AS tipo, \
				GROUP_CONCAT(c.referencia) AS referencias, \
				p.proveedor, \
				SUM(cs.saldor) AS total, \
				bc.idnumcuenta AS numerocuenta, \
				GROUP_CONCAT(cs.saldor) AS saldo \
			FROM proveedores p \
			INNER JOIN gastos c \
			INNER JOIN auxgastosprove cs \
			LEFT JOIN tiposcuentasbancarias tc1 ON tc1.clave = p.tipocuenta1 \
			LEFT JOIN tiposcuentasbancarias tc2 ON tc2.clave = p.tipocuenta2 \
			LEFT JOIN tiposcuentasbancarias tc3 ON tc3.clave = p.tipocuenta3 \
			RIGHT JOIN bancoscuentas bc ON bc.principal = 1 \
			WHERE p.proveedor=c.proveedor %s %s \
			AND cs.gasto=c.referencia AND cs.saldor>0 AND \
			c.fechagas>='%s' AND c.fechagas<='%s' AND p.agrupapaggast = 0 \
			GROUP BY c.referencia \
			HAVING clavepagobnc = 'PTC' \
			) UNION ALL ( \
			SELECT \
				IF(p.cuentadefault = 1, tc1.clave, IF(p.cuentadefault = 2, tc2.clave, tc3.clave)) AS clavepagobnc, \
				LPAD(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),18,'0') AS asuntobenef, \
				LPAD(bc.numerocuenta,18,'0') AS asuntoorden, \
				'MXP' AS divisa, \
				LPAD(SUM(cs.saldor),16,'0') AS importe, \
				RPAD(SUBSTRING(IF(p.tiporefpago = 0, p.referenciafija, IF(p.tiporefpago = 1, REPLACE(GROUP_CONCAT(DISTINCT(c.refpagprov)),',',''), IF(p.tiporefpago = 2, REPLACE(GROUP_CONCAT(DISTINCT(c.folioprov)),',',''), IF(p.tipoempre = 1, p.razonsocial, p.replegal)))),1,30),30,' ') AS motivo, \
				'G' AS tipo, \
				GROUP_CONCAT(c.referencia) AS referencias, \
				p.proveedor, \
				SUM(cs.saldor) AS total, \
				bc.idnumcuenta AS numerocuenta, \
				GROUP_CONCAT(cs.saldor) AS saldo \
			FROM proveedores p \
			INNER JOIN gastos c \
			INNER JOIN auxgastosprove cs \
			LEFT JOIN tiposcuentasbancarias tc1 ON tc1.clave = p.tipocuenta1 \
			LEFT JOIN tiposcuentasbancarias tc2 ON tc2.clave = p.tipocuenta2 \
			LEFT JOIN tiposcuentasbancarias tc3 ON tc3.clave = p.tipocuenta3 \
			RIGHT JOIN bancoscuentas bc ON bc.principal = 1 \
			WHERE p.proveedor=c.proveedor %s %s \
			AND cs.gasto=c.referencia AND cs.saldor>0 AND \
			c.fechagas>='%s' AND c.fechagas<='%s' AND p.agrupapaggast = 1 \
			GROUP BY p.proveedor \
			HAVING clavepagobnc = 'PTC' )) t",
			muestrasolofacturassincheque, condicion_mostraruuid,
			mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin),

			muestrasolofacturassincheque, condicion_mostraruuid,
			mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin),

			muestrasolofacturassincheque, condicion_mostraruuid,
			mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin),

			muestrasolofacturassincheque, condicion_mostraruuid,
			mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin));
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			instruccion.sprintf("SELECT t.clavepagobnc, t.asuntobenef, t.asuntoorden, \
			t.divisa, t.importe, t.titularasuntoben, t.tipocuenta, t.numbancbenef, t.motivo, \
			t.refnum, t.disponibilidad, t.tipo, t.referencias, t.proveedor, t.total, t.numerocuenta, t.saldo FROM ( \
			(SELECT \
				IF(p.cuentadefault = 1, tc1.clave, IF(p.cuentadefault = 2, tc2.clave, tc3.clave)) AS clavepagobnc, \
				LPAD(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),18,'0') AS asuntobenef, \
				LPAD(bc.numerocuenta,18,'0') AS asuntoorden, \
				'MXP' AS divisa, \
				LPAD(SUM(cs.saldor),16,'0') AS importe, \
				RPAD(SUBSTRING(IF(p.tipoempre = 1, p.razonsocial, p.replegal),1,30),30,' ') AS titularasuntoben, \
				'40' AS tipocuenta, \
				SUBSTRING(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),1,3) AS numbancbenef, \
				RPAD(SUBSTRING(IF(p.tiporefpago = 0, p.referenciafija, IF(p.tiporefpago = 1, REPLACE(GROUP_CONCAT(DISTINCT(c.refpagprov)),',',''), IF(p.tiporefpago = 2, REPLACE(GROUP_CONCAT(DISTINCT(c.folioprov)),',',''), IF(p.tipoempre = 1, pcfd.nombreemisor, p.replegal)))),1,30),30,' ') AS motivo, \
				RPAD('',7,' ') AS refnum, \
				'H' AS disponibilidad, \
				'C' AS tipo, \
				GROUP_CONCAT(c.referencia) AS referencias, \
				p.proveedor, \
				SUM(cs.saldor) AS total, \
				bc.idnumcuenta AS numerocuenta, \
				GROUP_CONCAT(cs.saldor) AS saldo \
			FROM proveedores p \
			INNER JOIN compras c \
			INNER JOIN auxcomprasprove cs \
			LEFT JOIN tiposcuentasbancarias tc1 ON tc1.clave = p.tipocuenta1 \
			LEFT JOIN tiposcuentasbancarias tc2 ON tc2.clave = p.tipocuenta2 \
			LEFT JOIN tiposcuentasbancarias tc3 ON tc3.clave = p.tipocuenta3 \
			LEFT JOIN parametroscfd pcfd ON pcfd.sucursal = '%s' \
			RIGHT JOIN bancoscuentas bc ON bc.principal = 1 \
			WHERE p.proveedor=c.proveedor %s %s \
			AND cs.compra=c.referencia AND cs.saldor>0 AND \
			c.fechacom>='%s' AND c.fechacom<='%s' AND p.agrupapagfact = 0 \
			GROUP BY c.referencia \
			HAVING clavepagobnc = 'PSC' \
			) UNION ALL ( \
			SELECT \
				IF(p.cuentadefault = 1, tc1.clave, IF(p.cuentadefault = 2, tc2.clave, tc3.clave)) AS clavepagobnc, \
				LPAD(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),18,'0') AS asuntobenef, \
				LPAD(bc.numerocuenta,18,'0') AS asuntoorden, \
				'MXP' AS divisa, \
				LPAD(SUM(cs.saldor),16,'0') AS importe, \
				RPAD(SUBSTRING(IF(p.tipoempre = 1, p.razonsocial, p.replegal),1,30),30,' ') AS titularasuntoben, \
				'40' AS tipocuenta, \
				SUBSTRING(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),1,3) AS numbancbenef, \
				RPAD(SUBSTRING(IF(p.tiporefpago = 0, p.referenciafija, IF(p.tiporefpago = 1, REPLACE(GROUP_CONCAT(DISTINCT(c.refpagprov)),',',''), IF(p.tiporefpago = 2, REPLACE(GROUP_CONCAT(DISTINCT(c.folioprov)),',',''), IF(p.tipoempre = 1, pcfd.nombreemisor, p.replegal)))),1,30),30,' ') AS motivo, \
				RPAD('',7,' ') AS refnum, \
				'H' AS disponibilidad, \
				'C' AS tipo, \
				GROUP_CONCAT(c.referencia) AS referencias, \
				p.proveedor, \
				SUM(cs.saldor) AS total, \
				bc.idnumcuenta AS numerocuenta, \
				GROUP_CONCAT(cs.saldor) AS saldo \
			FROM proveedores p \
			INNER JOIN compras c \
			INNER JOIN auxcomprasprove cs \
			LEFT JOIN tiposcuentasbancarias tc1 ON tc1.clave = p.tipocuenta1 \
			LEFT JOIN tiposcuentasbancarias tc2 ON tc2.clave = p.tipocuenta2 \
			LEFT JOIN tiposcuentasbancarias tc3 ON tc3.clave = p.tipocuenta3 \
			LEFT JOIN parametroscfd pcfd ON pcfd.sucursal = '%s' \
			RIGHT JOIN bancoscuentas bc ON bc.principal = 1 \
			WHERE p.proveedor=c.proveedor %s %s \
			AND cs.compra=c.referencia AND cs.saldor>0 AND \
			c.fechacom>='%s' AND c.fechacom<='%s' AND p.agrupapagfact = 1 \
			GROUP BY p.proveedor \
			HAVING clavepagobnc = 'PSC' \
			) UNION ALL ( \
			SELECT \
				IF(p.cuentadefault = 1, tc1.clave, IF(p.cuentadefault = 2, tc2.clave, tc3.clave)) AS clavepagobnc, \
				LPAD(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),18,'0') AS asuntobenef, \
				LPAD(bc.numerocuenta,18,'0') AS asuntoorden, \
				'MXP' AS divisa, \
				LPAD(SUM(cs.saldor),16,'0') AS importe, \
				RPAD(SUBSTRING(IF(p.tipoempre = 1, p.razonsocial, p.replegal),1,30),30,' ') AS titularasuntoben, \
				'40' AS tipocuenta, \
				SUBSTRING(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),1,3) AS numbancbenef, \
				RPAD(SUBSTRING(IF(p.tiporefpago = 0, p.referenciafija, IF(p.tiporefpago = 1, REPLACE(GROUP_CONCAT(DISTINCT(c.refpagprov)),',',''), IF(p.tiporefpago = 2, REPLACE(GROUP_CONCAT(DISTINCT(c.folioprov)),',',''), IF(p.tipoempre = 1, pcfd.nombreemisor, p.replegal)))),1,30),30,' ') AS motivo, \
				RPAD('',7,' ') AS refnum, \
				'H' AS disponibilidad, \
				'G' AS tipo, \
				GROUP_CONCAT(c.referencia) AS referencias, \
				p.proveedor, \
				SUM(cs.saldor) AS total, \
				bc.idnumcuenta AS numerocuenta, \
				GROUP_CONCAT(cs.saldor) AS saldo \
			FROM proveedores p \
			INNER JOIN gastos c \
			INNER JOIN auxgastosprove cs \
			LEFT JOIN tiposcuentasbancarias tc1 ON tc1.clave = p.tipocuenta1 \
			LEFT JOIN tiposcuentasbancarias tc2 ON tc2.clave = p.tipocuenta2 \
			LEFT JOIN tiposcuentasbancarias tc3 ON tc3.clave = p.tipocuenta3 \
			LEFT JOIN parametroscfd pcfd ON pcfd.sucursal = '%s' \
			RIGHT JOIN bancoscuentas bc ON bc.principal = 1 \
			WHERE p.proveedor=c.proveedor %s %s \
			AND cs.gasto=c.referencia AND cs.saldor>0 AND \
			c.fechagas>='%s' AND c.fechagas<='%s' AND p.agrupapaggast = 0 \
			GROUP BY c.referencia \
			HAVING clavepagobnc = 'PSC' \
			) UNION ALL ( \
			SELECT \
				IF(p.cuentadefault = 1, tc1.clave, IF(p.cuentadefault = 2, tc2.clave, tc3.clave)) AS clavepagobnc, \
				LPAD(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),18,'0') AS asuntobenef, \
				LPAD(bc.numerocuenta,18,'0') AS asuntoorden, \
				'MXP' AS divisa, \
				LPAD(SUM(cs.saldor),16,'0') AS importe, \
				RPAD(SUBSTRING(IF(p.tipoempre = 1, p.razonsocial, p.replegal),1,30),30,' ') AS titularasuntoben, \
				'40' AS tipocuenta, \
				SUBSTRING(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),1,3) AS numbancbenef, \
				RPAD(SUBSTRING(IF(p.tiporefpago = 0, p.referenciafija, IF(p.tiporefpago = 1, REPLACE(GROUP_CONCAT(DISTINCT(c.refpagprov)),',',''), IF(p.tiporefpago = 2, REPLACE(GROUP_CONCAT(DISTINCT(c.folioprov)),',',''), IF(p.tipoempre = 1, pcfd.nombreemisor, p.replegal)))),1,30),30,' ') AS motivo, \
				RPAD('',7,' ') AS refnum, \
				'H' AS disponibilidad, \
				'G' AS tipo, \
				GROUP_CONCAT(c.referencia) AS referencias, \
				p.proveedor, \
				SUM(cs.saldor) AS total, \
				bc.idnumcuenta AS numerocuenta, \
				GROUP_CONCAT(cs.saldor) AS saldo \
			FROM proveedores p \
			INNER JOIN gastos c \
			INNER JOIN auxgastosprove cs \
			LEFT JOIN tiposcuentasbancarias tc1 ON tc1.clave = p.tipocuenta1 \
			LEFT JOIN tiposcuentasbancarias tc2 ON tc2.clave = p.tipocuenta2 \
			LEFT JOIN tiposcuentasbancarias tc3 ON tc3.clave = p.tipocuenta3 \
			LEFT JOIN parametroscfd pcfd ON pcfd.sucursal = '%s' \
			RIGHT JOIN bancoscuentas bc ON bc.principal = 1 \
			WHERE p.proveedor=c.proveedor %s %s \
			AND cs.gasto=c.referencia AND cs.saldor>0 AND \
			c.fechagas>='%s' AND c.fechagas<='%s' AND p.agrupapaggast = 1 \
			GROUP BY p.proveedor \
			HAVING clavepagobnc = 'PSC' )) t",
			FormServidor->ObtieneClaveSucursal(),
			muestrasolofacturassincheque, condicion_mostraruuid,
			mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin),

			FormServidor->ObtieneClaveSucursal(),
			muestrasolofacturassincheque, condicion_mostraruuid,
			mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin),

			FormServidor->ObtieneClaveSucursal(),
			muestrasolofacturassincheque, condicion_mostraruuid,
			mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin),

			FormServidor->ObtieneClaveSucursal(),
			muestrasolofacturassincheque, condicion_mostraruuid,
			mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin));
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			instruccion.sprintf("SELECT t.clavepagobnc, t.conceptocie, t.conveniocie, t.asuntoorden, \
			t.importe, t.motivo, t.refcie, t.tipo, t.referencias, t.proveedor, t.total, t.numerocuenta, t.saldo FROM ( \
			(SELECT \
				IF(p.cuentadefault = 1, tc1.clave, IF(p.cuentadefault = 2, tc2.clave, tc3.clave)) AS clavepagobnc, \
				RPAD(SUBSTRING(IF(p.tipoempre = 1, p.razonsocial, p.replegal),1,30),30,' ') AS conceptocie, \
				LPAD(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),7,'0') AS conveniocie, \
				LPAD(bc.numerocuenta,18,'0') AS asuntoorden, \
				LPAD(SUM(cs.saldor),16,'0') AS importe, \
				RPAD('',30,' ') AS motivo, \
				RPAD(SUBSTRING(IF(p.tiporefpago = 0, p.referenciafija, IF(p.tiporefpago = 1, REPLACE(GROUP_CONCAT(DISTINCT(c.refpagprov)),',',''), IF(p.tiporefpago = 2, REPLACE(GROUP_CONCAT(DISTINCT(c.folioprov)),',',''), IF(p.tipoempre = 1, p.razonsocial, p.replegal)))),1,20),20,' ') AS refcie, \
				'C' AS tipo, \
				GROUP_CONCAT(c.referencia) AS referencias, \
				p.proveedor, \
				SUM(cs.saldor) AS total, \
				bc.idnumcuenta AS numerocuenta, \
				GROUP_CONCAT(cs.saldor) AS saldo \
			FROM proveedores p \
			INNER JOIN compras c \
			INNER JOIN auxcomprasprove cs \
			LEFT JOIN tiposcuentasbancarias tc1 ON tc1.clave = p.tipocuenta1 \
			LEFT JOIN tiposcuentasbancarias tc2 ON tc2.clave = p.tipocuenta2 \
			LEFT JOIN tiposcuentasbancarias tc3 ON tc3.clave = p.tipocuenta3 \
			RIGHT JOIN bancoscuentas bc ON bc.principal = 1 \
			WHERE p.proveedor=c.proveedor %s %s \
			AND cs.compra=c.referencia AND cs.saldor>0 AND \
			c.fechacom>='%s' AND c.fechacom<='%s' AND p.agrupapagfact = 0 \
			GROUP BY c.referencia \
			HAVING clavepagobnc = 'CIL' \
			) UNION ALL ( \
			SELECT \
				IF(p.cuentadefault = 1, tc1.clave, IF(p.cuentadefault = 2, tc2.clave, tc3.clave)) AS clavepagobnc, \
				RPAD(SUBSTRING(IF(p.tipoempre = 1, p.razonsocial, p.replegal),1,30),30,' ') AS conceptocie, \
				LPAD(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),7,'0') AS conveniocie, \
				LPAD(bc.numerocuenta,18,'0') AS asuntoorden, \
				LPAD(SUM(cs.saldor),16,'0') AS importe, \
				RPAD('',30,' ') AS motivo, \
				RPAD(SUBSTRING(IF(p.tiporefpago = 0, p.referenciafija, IF(p.tiporefpago = 1, REPLACE(GROUP_CONCAT(DISTINCT(c.refpagprov)),',',''), IF(p.tiporefpago = 2, REPLACE(GROUP_CONCAT(DISTINCT(c.folioprov)),',',''), IF(p.tipoempre = 1, p.razonsocial, p.replegal)))),1,20),20,' ') AS refcie, \
				'C' AS tipo, \
				GROUP_CONCAT(c.referencia) AS referencias, \
				p.proveedor, \
				SUM(cs.saldor) AS total, \
				bc.idnumcuenta AS numerocuenta, \
				GROUP_CONCAT(cs.saldor) AS saldo \
			FROM proveedores p \
			INNER JOIN compras c \
			INNER JOIN auxcomprasprove cs \
			LEFT JOIN tiposcuentasbancarias tc1 ON tc1.clave = p.tipocuenta1 \
			LEFT JOIN tiposcuentasbancarias tc2 ON tc2.clave = p.tipocuenta2 \
			LEFT JOIN tiposcuentasbancarias tc3 ON tc3.clave = p.tipocuenta3 \
			RIGHT JOIN bancoscuentas bc ON bc.principal = 1 \
			WHERE p.proveedor=c.proveedor %s %s \
			AND cs.compra=c.referencia AND cs.saldor>0 AND \
			c.fechacom>='%s' AND c.fechacom<='%s' AND p.agrupapagfact = 1 \
			GROUP BY p.proveedor \
			HAVING clavepagobnc = 'CIL' \
			) UNION ALL ( \
			SELECT \
				IF(p.cuentadefault = 1, tc1.clave, IF(p.cuentadefault = 2, tc2.clave, tc3.clave)) AS clavepagobnc, \
				RPAD(SUBSTRING(IF(p.tipoempre = 1, p.razonsocial, p.replegal),1,30),30,' ') AS conceptocie, \
				LPAD(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),7,'0') AS conveniocie, \
				LPAD(bc.numerocuenta,18,'0') AS asuntoorden, \
				LPAD(SUM(cs.saldor),16,'0') AS importe, \
				RPAD('',30,' ') AS motivo, \
				RPAD(SUBSTRING(IF(p.tiporefpago = 0, p.referenciafija, IF(p.tiporefpago = 1, REPLACE(GROUP_CONCAT(DISTINCT(c.refpagprov)),',',''), IF(p.tiporefpago = 2, REPLACE(GROUP_CONCAT(DISTINCT(c.folioprov)),',',''), IF(p.tipoempre = 1, p.razonsocial, p.replegal)))),1,20),20,' ') AS refcie, \
				'G' AS tipo, \
				GROUP_CONCAT(c.referencia) AS referencias, \
				p.proveedor, \
				SUM(cs.saldor) AS total, \
				bc.idnumcuenta AS numerocuenta, \
				GROUP_CONCAT(cs.saldor) AS saldo \
			FROM proveedores p \
			INNER JOIN gastos c \
			INNER JOIN auxgastosprove cs \
			LEFT JOIN tiposcuentasbancarias tc1 ON tc1.clave = p.tipocuenta1 \
			LEFT JOIN tiposcuentasbancarias tc2 ON tc2.clave = p.tipocuenta2 \
			LEFT JOIN tiposcuentasbancarias tc3 ON tc3.clave = p.tipocuenta3 \
			RIGHT JOIN bancoscuentas bc ON bc.principal = 1 \
			WHERE p.proveedor=c.proveedor %s %s \
			AND cs.gasto=c.referencia AND cs.saldor>0 AND \
			c.fechagas>='%s' AND c.fechagas<='%s' AND p.agrupapaggast = 0 \
			GROUP BY c.referencia \
			HAVING clavepagobnc = 'CIL' \
			) UNION ALL ( \
			SELECT \
				IF(p.cuentadefault = 1, tc1.clave, IF(p.cuentadefault = 2, tc2.clave, tc3.clave)) AS clavepagobnc, \
				RPAD(SUBSTRING(IF(p.tipoempre = 1, p.razonsocial, p.replegal),1,30),30,' ') AS conceptocie, \
				LPAD(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),7,'0') AS conveniocie, \
				LPAD(bc.numerocuenta,18,'0') AS asuntoorden, \
				LPAD(SUM(cs.saldor),16,'0') AS importe, \
				RPAD('',30,' ') AS motivo, \
				RPAD(SUBSTRING(IF(p.tiporefpago = 0, p.referenciafija, IF(p.tiporefpago = 1, REPLACE(GROUP_CONCAT(DISTINCT(c.refpagprov)),',',''), IF(p.tiporefpago = 2, REPLACE(GROUP_CONCAT(DISTINCT(c.folioprov)),',',''), IF(p.tipoempre = 1, p.razonsocial, p.replegal)))),1,20),20,' ') AS refcie, \
				'G' AS tipo, \
				GROUP_CONCAT(c.referencia) AS referencias, \
				p.proveedor, \
				SUM(cs.saldor) AS total, \
				bc.idnumcuenta AS numerocuenta, \
				GROUP_CONCAT(cs.saldor) AS saldo \
			FROM proveedores p \
			INNER JOIN gastos c \
			INNER JOIN auxgastosprove cs \
			LEFT JOIN tiposcuentasbancarias tc1 ON tc1.clave = p.tipocuenta1 \
			LEFT JOIN tiposcuentasbancarias tc2 ON tc2.clave = p.tipocuenta2 \
			LEFT JOIN tiposcuentasbancarias tc3 ON tc3.clave = p.tipocuenta3 \
			RIGHT JOIN bancoscuentas bc ON bc.principal = 1 \
			WHERE p.proveedor=c.proveedor %s %s \
			AND cs.gasto=c.referencia AND cs.saldor>0 AND \
			c.fechagas>='%s' AND c.fechagas<='%s' AND p.agrupapaggast = 1 \
			GROUP BY p.proveedor \
			HAVING clavepagobnc = 'CIL' )) t",
			muestrasolofacturassincheque, condicion_mostraruuid,
			mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin),

			muestrasolofacturassincheque, condicion_mostraruuid,
			mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin),

			muestrasolofacturassincheque, condicion_mostraruuid,
			mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin),

			muestrasolofacturassincheque, condicion_mostraruuid,
			mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin));
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		} else {
			instruccion.sprintf("SELECT t.clavepagobnc, t.asuntobenef, t.asuntoorden, \
			t.divisa, t.importe, t.motivo, t.tipo, t.referencias, t.proveedor, t.total, t.numerocuenta, t.saldo FROM ( \
			(SELECT \
				IF(p.cuentadefault = 1, tc1.clave, IF(p.cuentadefault = 2, tc2.clave, tc3.clave)) AS clavepagobnc, \
				LPAD(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),18,'0') AS asuntobenef, \
				LPAD(bc.numerocuenta,18,'0') AS asuntoorden, \
				'MXP' AS divisa, \
				LPAD(SUM(cs.saldor),16,'0') AS importe, \
				RPAD(SUBSTRING(IF(p.tiporefpago = 0, p.referenciafija, IF(p.tiporefpago = 1, REPLACE(GROUP_CONCAT(DISTINCT(c.refpagprov)),',',''), IF(p.tiporefpago = 2, REPLACE(GROUP_CONCAT(DISTINCT(c.folioprov)),',',''), IF(p.tipoempre = 1, p.razonsocial, p.replegal)))),1,30),30,' ') AS motivo, \
				'C' AS tipo, \
				GROUP_CONCAT(c.referencia) AS referencias, \
				p.proveedor, \
				SUM(cs.saldor) AS total, \
				bc.idnumcuenta AS numerocuenta, \
				GROUP_CONCAT(cs.saldor) AS saldo \
			FROM proveedores p \
			INNER JOIN compras c \
			INNER JOIN auxcomprasprove cs \
			LEFT JOIN tiposcuentasbancarias tc1 ON tc1.clave = p.tipocuenta1 \
			LEFT JOIN tiposcuentasbancarias tc2 ON tc2.clave = p.tipocuenta2 \
			LEFT JOIN tiposcuentasbancarias tc3 ON tc3.clave = p.tipocuenta3 \
			RIGHT JOIN bancoscuentas bc ON bc.principal = 1 \
			WHERE p.proveedor=c.proveedor %s %s \
			AND cs.compra=c.referencia AND cs.saldor>0 AND \
			c.fechacom>='%s' AND c.fechacom<='%s' \
			GROUP BY c.referencia \
			HAVING clavepagobnc = 'PTC' \
			) UNION ALL ( \
			SELECT \
				IF(p.cuentadefault = 1, tc1.clave, IF(p.cuentadefault = 2, tc2.clave, tc3.clave)) AS clavepagobnc, \
				LPAD(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),18,'0') AS asuntobenef, \
				LPAD(bc.numerocuenta,18,'0') AS asuntoorden, \
				'MXP' AS divisa, \
				LPAD(SUM(cs.saldor),16,'0') AS importe, \
				RPAD(SUBSTRING(IF(p.tiporefpago = 0, p.referenciafija, IF(p.tiporefpago = 1, REPLACE(GROUP_CONCAT(DISTINCT(c.refpagprov)),',',''), IF(p.tiporefpago = 2, REPLACE(GROUP_CONCAT(DISTINCT(c.folioprov)),',',''), IF(p.tipoempre = 1, p.razonsocial, p.replegal)))),1,30),30,' ') AS motivo, \
				'G' AS tipo, \
				GROUP_CONCAT(c.referencia) AS referencias, \
				p.proveedor, \
				SUM(cs.saldor) AS total, \
				bc.idnumcuenta AS numerocuenta, \
				GROUP_CONCAT(cs.saldor) AS saldo \
			FROM proveedores p \
			INNER JOIN gastos c \
			INNER JOIN auxgastosprove cs \
			LEFT JOIN tiposcuentasbancarias tc1 ON tc1.clave = p.tipocuenta1 \
			LEFT JOIN tiposcuentasbancarias tc2 ON tc2.clave = p.tipocuenta2 \
			LEFT JOIN tiposcuentasbancarias tc3 ON tc3.clave = p.tipocuenta3 \
			RIGHT JOIN bancoscuentas bc ON bc.principal = 1 \
			WHERE p.proveedor=c.proveedor %s %s \
			AND cs.gasto=c.referencia AND cs.saldor>0 AND \
			c.fechagas>='%s' AND c.fechagas<='%s' GROUP BY c.referencia \
			HAVING clavepagobnc = 'PTC' )) t ",
			muestrasolofacturassincheque, condicion_mostraruuid,
			mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin),

			muestrasolofacturassincheque, condicion_mostraruuid,
			mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin));

			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			instruccion.sprintf("SELECT t.clavepagobnc, t.asuntobenef, t.asuntoorden, \
			t.divisa, t.importe, t.titularasuntoben, t.tipocuenta, t.numbancbenef, t.motivo, \
			t.refnum, t.disponibilidad, t.tipo, t.referencias, t.proveedor, t.total, t.numerocuenta, t.saldo FROM ( \
			(SELECT \
				IF(p.cuentadefault = 1, tc1.clave, IF(p.cuentadefault = 2, tc2.clave, tc3.clave)) AS clavepagobnc, \
				LPAD(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),18,'0') AS asuntobenef, \
				LPAD(bc.numerocuenta,18,'0') AS asuntoorden, \
				'MXP' AS divisa, \
				LPAD(SUM(cs.saldor),16,'0') AS importe, \
				RPAD(SUBSTRING(IF(p.tipoempre = 1, p.razonsocial, p.replegal),1,30),30,' ') AS titularasuntoben, \
				'40' AS tipocuenta, \
				SUBSTRING(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),1,3) AS numbancbenef, \
				RPAD(SUBSTRING(IF(p.tiporefpago = 0, p.referenciafija, IF(p.tiporefpago = 1, REPLACE(GROUP_CONCAT(DISTINCT(c.refpagprov)),',',''), IF(p.tiporefpago = 2, REPLACE(GROUP_CONCAT(DISTINCT(c.folioprov)),',',''), IF(p.tipoempre = 1, pcfd.nombreemisor, p.replegal)))),1,30),30,' ') AS motivo, \
				RPAD('',7,' ') AS refnum, \
				'H' AS disponibilidad, \
				'C' AS tipo, \
				GROUP_CONCAT(c.referencia) AS referencias, \
				p.proveedor, \
				SUM(cs.saldor) AS total, \
				bc.idnumcuenta AS numerocuenta, \
				GROUP_CONCAT(cs.saldor) AS saldo \
			FROM proveedores p \
			INNER JOIN compras c \
			INNER JOIN auxcomprasprove cs \
			LEFT JOIN tiposcuentasbancarias tc1 ON tc1.clave = p.tipocuenta1 \
			LEFT JOIN tiposcuentasbancarias tc2 ON tc2.clave = p.tipocuenta2 \
			LEFT JOIN tiposcuentasbancarias tc3 ON tc3.clave = p.tipocuenta3 \
			LEFT JOIN parametroscfd pcfd ON pcfd.sucursal = '%s' \
			RIGHT JOIN bancoscuentas bc ON bc.principal = 1 \
			WHERE p.proveedor=c.proveedor %s %s \
			AND cs.compra=c.referencia AND cs.saldor>0 AND \
			c.fechacom>='%s' AND c.fechacom<='%s' \
			GROUP BY c.referencia \
			HAVING clavepagobnc = 'PSC' \
			) UNION ALL ( \
			SELECT \
				IF(p.cuentadefault = 1, tc1.clave, IF(p.cuentadefault = 2, tc2.clave, tc3.clave)) AS clavepagobnc, \
				LPAD(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),18,'0') AS asuntobenef, \
				LPAD(bc.numerocuenta,18,'0') AS asuntoorden, \
				'MXP' AS divisa, \
				LPAD(SUM(cs.saldor),16,'0') AS importe, \
				RPAD(SUBSTRING(IF(p.tipoempre = 1, p.razonsocial, p.replegal),1,30),30,' ') AS titularasuntoben, \
				'40' AS tipocuenta, \
				SUBSTRING(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),1,3) AS numbancbenef, \
				RPAD(SUBSTRING(IF(p.tiporefpago = 0, p.referenciafija, IF(p.tiporefpago = 1, REPLACE(GROUP_CONCAT(DISTINCT(c.refpagprov)),',',''), IF(p.tiporefpago = 2, REPLACE(GROUP_CONCAT(DISTINCT(c.folioprov)),',',''), IF(p.tipoempre = 1, pcfd.nombreemisor, p.replegal)))),1,30),30,' ') AS motivo, \
				RPAD('',7,' ') AS refnum, \
				'H' AS disponibilidad, \
				'G' AS tipo, \
				GROUP_CONCAT(c.referencia) AS referencias, \
				p.proveedor, \
				SUM(cs.saldor) AS total, \
				bc.idnumcuenta AS numerocuenta, \
				GROUP_CONCAT(cs.saldor) AS saldo \
			FROM proveedores p \
			INNER JOIN gastos c \
			INNER JOIN auxgastosprove cs \
			LEFT JOIN tiposcuentasbancarias tc1 ON tc1.clave = p.tipocuenta1 \
			LEFT JOIN tiposcuentasbancarias tc2 ON tc2.clave = p.tipocuenta2 \
			LEFT JOIN tiposcuentasbancarias tc3 ON tc3.clave = p.tipocuenta3 \
			LEFT JOIN parametroscfd pcfd ON pcfd.sucursal = '%s' \
			RIGHT JOIN bancoscuentas bc ON bc.principal = 1 \
			WHERE p.proveedor=c.proveedor %s %s \
			AND cs.gasto=c.referencia AND cs.saldor>0 AND \
			c.fechagas>='%s' AND c.fechagas<='%s' GROUP BY c.referencia \
			HAVING clavepagobnc = 'PSC' )) t ",
			FormServidor->ObtieneClaveSucursal(),
			muestrasolofacturassincheque, condicion_mostraruuid,
			mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin),

			FormServidor->ObtieneClaveSucursal(),
			muestrasolofacturassincheque, condicion_mostraruuid,
			mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin));

			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			instruccion.sprintf("SELECT t.clavepagobnc, t.conceptocie, t.conveniocie, t.asuntoorden, \
			t.importe, t.motivo, t.refcie, t.tipo, t.referencias, t.proveedor, t.total, t.numerocuenta, t.saldo FROM ( \
			(SELECT \
				IF(p.cuentadefault = 1, tc1.clave, IF(p.cuentadefault = 2, tc2.clave, tc3.clave)) AS clavepagobnc, \
				RPAD(SUBSTRING(IF(p.tipoempre = 1, p.razonsocial, p.replegal),1,30),30,' ') AS conceptocie, \
				LPAD(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),7,'0') AS conveniocie, \
				LPAD(bc.numerocuenta,18,'0') AS asuntoorden, \
				LPAD(SUM(cs.saldor),16,'0') AS importe, \
				RPAD('',30,' ') AS motivo, \
				RPAD(SUBSTRING(IF(p.tiporefpago = 0, p.referenciafija, IF(p.tiporefpago = 1, REPLACE(GROUP_CONCAT(DISTINCT(c.refpagprov)),',',''), IF(p.tiporefpago = 2, REPLACE(GROUP_CONCAT(DISTINCT(c.folioprov)),',',''), IF(p.tipoempre = 1, p.razonsocial, p.replegal)))),1,20),20,' ') AS refcie, \
				'C' AS tipo, \
				GROUP_CONCAT(c.referencia) AS referencias, \
				p.proveedor, \
				SUM(cs.saldor) AS total, \
				bc.idnumcuenta AS numerocuenta, \
				GROUP_CONCAT(cs.saldor) AS saldo \
			FROM proveedores p \
			INNER JOIN compras c \
			INNER JOIN auxcomprasprove cs \
			LEFT JOIN tiposcuentasbancarias tc1 ON tc1.clave = p.tipocuenta1 \
			LEFT JOIN tiposcuentasbancarias tc2 ON tc2.clave = p.tipocuenta2 \
			LEFT JOIN tiposcuentasbancarias tc3 ON tc3.clave = p.tipocuenta3 \
			RIGHT JOIN bancoscuentas bc ON bc.principal = 1 \
			WHERE p.proveedor=c.proveedor %s %s \
			AND cs.compra=c.referencia AND cs.saldor>0 AND \
			c.fechacom>='%s' AND c.fechacom<='%s' \
			GROUP BY c.referencia \
			HAVING clavepagobnc = 'CIL' \
			) UNION ALL ( \
			SELECT \
				IF(p.cuentadefault = 1, tc1.clave, IF(p.cuentadefault = 2, tc2.clave, tc3.clave)) AS clavepagobnc, \
				RPAD(SUBSTRING(IF(p.tipoempre = 1, p.razonsocial, p.replegal),1,30),30,' ') AS conceptocie, \
				LPAD(IF(p.cuentadefault = 1, p.cuentab1, IF(p.cuentadefault = 2, p.cuentab2, p.cuentab3)),7,'0') AS conveniocie, \
				LPAD(bc.numerocuenta,18,'0') AS asuntoorden, \
				LPAD(SUM(cs.saldor),16,'0') AS importe, \
				RPAD('',30,' ') AS motivo, \
				RPAD(SUBSTRING(IF(p.tiporefpago = 0, p.referenciafija, IF(p.tiporefpago = 1, REPLACE(GROUP_CONCAT(DISTINCT(c.refpagprov)),',',''), IF(p.tiporefpago = 2, REPLACE(GROUP_CONCAT(DISTINCT(c.folioprov)),',',''), IF(p.tipoempre = 1, p.razonsocial, p.replegal)))),1,20),20,' ') AS refcie, \
				'G' AS tipo, \
				GROUP_CONCAT(c.referencia) AS referencias, \
				p.proveedor, \
				SUM(cs.saldor) AS total, \
				bc.idnumcuenta AS numerocuenta, \
				GROUP_CONCAT(cs.saldor) AS saldo \
			FROM proveedores p \
			INNER JOIN gastos c \
			INNER JOIN auxgastosprove cs \
			LEFT JOIN tiposcuentasbancarias tc1 ON tc1.clave = p.tipocuenta1 \
			LEFT JOIN tiposcuentasbancarias tc2 ON tc2.clave = p.tipocuenta2 \
			LEFT JOIN tiposcuentasbancarias tc3 ON tc3.clave = p.tipocuenta3 \
			RIGHT JOIN bancoscuentas bc ON bc.principal = 1 \
			WHERE p.proveedor=c.proveedor %s %s \
			AND cs.gasto=c.referencia AND cs.saldor>0 AND \
			c.fechagas>='%s' AND c.fechagas<='%s' GROUP BY c.referencia \
			HAVING clavepagobnc = 'CIL' )) t ",
			muestrasolofacturassincheque, condicion_mostraruuid,
			mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin),

			muestrasolofacturassincheque, condicion_mostraruuid,
			mFg.StrToMySqlDate(fecha_inicio), mFg.StrToMySqlDate(fecha_fin));

			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
    }

}

//------------------------------------------------------------------------------
//ID_CON_NC_DETALLE
void ServidorVentas::ConsultaNCDetalle(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
	AnsiString clave, tipo;
	AnsiString condicion_tipo = " ";

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	tipo = mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	if(tipo != " "){
		condicion_tipo.sprintf(" AND nc.tipo='%s' ", tipo);
	}

	// Obtiene todos los datos de la sucursal
	instruccion.sprintf("SELECT * FROM notascredcli nc WHERE nc.referencia='%s' %s ",
		clave, condicion_tipo);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

	// Obtiene el detalle
	instruccion.sprintf("SELECT dnc.articulo, dnc.cantidad, \
	dnc.precioimp, dnc.clave_motivo, dnc.empleado_responsable, \
	CONCAT(em.nombre, ' ', em.appat, ' ', em.apmat) AS nombre \
	FROM notascredcli nc \
	INNER JOIN dnotascredcli dnc ON dnc.referencia = nc.referencia \
    LEFT JOIN empleados em ON em.empleado = dnc.empleado_responsable \
	WHERE nc.referencia = '%s' %s ", clave, condicion_tipo);

	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

}
//------------------------------------------------------------------------------
//ID_CAMBIA_MOT_EMP_NC
void ServidorVentas::CambiaMotEmpNC(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros){

	// CAMBIA COBRADOR A VENTA O VENTAS
	char *buffer_sql=new char[1024*64];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instrucciones[50];
	int num_instrucciones=0;
	AnsiString instruccion;
	AnsiString referencia_nota, articulo, motivo, empleado;
	AnsiString act_motivo = "NULL", act_empleado = "NULL";

	TDate fecha=Today();
	TTime hora=Time();

	try{
		referencia_nota=mFg.ExtraeStringDeBuffer(&parametros);
		articulo = mFg.ExtraeStringDeBuffer(&parametros);
		motivo=mFg.ExtraeStringDeBuffer(&parametros);
		empleado=mFg.ExtraeStringDeBuffer(&parametros);

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		if(motivo.Trim() != "")
			act_motivo.sprintf(" '%s' ", motivo);

		if(empleado.Trim() != "")
			act_empleado.sprintf(" '%s' ", empleado);

			// mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora)
		instruccion.sprintf("UPDATE notascredcli nc \
			INNER JOIN dnotascredcli dnc ON dnc.referencia = nc.referencia \
			SET nc.fechamodi='%s', nc.horamodi='%s', \
			dnc.clave_motivo = %s, dnc.empleado_responsable = %s \
			WHERE nc.referencia='%s' AND dnc.articulo='%s' ",
            mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora),
			act_motivo, act_empleado, referencia_nota, articulo);

		instrucciones[num_instrucciones++]=instruccion;


		instrucciones[num_instrucciones++]="COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (int i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);
	} __finally {
		delete buffer_sql;
	}
}

//------------------------------------------------------------------------------
// ID_CON_SURTIDOS
void ServidorVentas::ConsultaSurtidos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
    char *buffer_sql=new char[1024*64*10];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instrucciones[50];
	int num_instrucciones=0;

	AnsiString surtido, pedido, estado, embarque, condicion_pedido = " ", condicion_dpedido = " ", having_estado = " ";
	AnsiString condicion_pedido_rem = " ", condicion_dpedidos = " ";
	try {
		surtido = mFg.ExtraeStringDeBuffer(&parametros);
		pedido = mFg.ExtraeStringDeBuffer(&parametros);
		estado = mFg.ExtraeStringDeBuffer(&parametros);
		embarque = mFg.ExtraeStringDeBuffer(&parametros);

		if(pedido.Trim() != ""){
			condicion_pedido.sprintf(" AND pv.referencia = '%s' ", pedido);
			condicion_dpedido.sprintf(" AND dsp.pedido = '%s' ", pedido);
			condicion_pedido_rem.sprintf(" AND sar.pedido = '%s' ", pedido);
			condicion_dpedidos.sprintf(" AND dpv.referencia = '%s' ", pedido);
		}

		if(estado == "1"){
			having_estado.sprintf(" HAVING (SUM(dp.cantidad) - IFNULL(dsp.cantidad, 0)) = 0 ");
		}else if (estado == "0") {
			having_estado.sprintf(" HAVING (SUM(dp.cantidad) - IFNULL(dsp.cantidad, 0)) <> 0 ");
		}


        instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		instruccion.sprintf(" CREATE TEMPORARY TABLE auxproductospresentacion ( \
			producto VARCHAR(8), \
			present VARCHAR(255), \
			orden INT \
		) ENGINE = INNODB ");
		instrucciones[num_instrucciones++]=instruccion;

		// TODOS LOS PEDIDOS

		//Se guardan todos los productos-presentación del dpedidos y dsurtido
		instruccion.sprintf(" INSERT IGNORE INTO auxproductospresentacion(producto, present, orden) \
			SELECT producto, present, DENSE_RANK() OVER (ORDER BY producto, present) AS orden \
			FROM ( \
				(SELECT ap.producto, ap.present \
				FROM pedidosventa pv \
				INNER JOIN dpedidosventa dp ON dp.referencia = pv.referencia  \
				INNER JOIN articulos ap ON ap.articulo = dp.articulo \
				INNER JOIN surtidopedidos sp ON sp.embarque = pv.embarque \
				WHERE pv.embarque = '%s' %s \
				GROUP BY ap.producto, ap.present \
				)UNION( \
				SELECT a.producto, a.present \
				FROM dsurtidopedidos dsp \
				INNER JOIN articulos a ON a.articulo = dsp.articulo \
				WHERE dsp.surtido = '%s' AND dsp.reemplazado = 0 %s \
				GROUP BY a.producto, a.present) \
			) AS prodpres ", embarque, condicion_pedido, surtido, condicion_dpedido);
		instrucciones[num_instrucciones++]=instruccion;

		//Para guardar cálculos de los reemplazos exactos
		instruccion.sprintf(" CREATE TEMPORARY TABLE conteo_ree_exac ( \
				pedido VARCHAR(11) NOT NULL, \
				producto VARCHAR(8) NOT NULL DEFAULT '', \
				present VARCHAR(255) NOT NULL DEFAULT '', \
				pedidos_cantidad INT(11) NULL DEFAULT 0, \
				surtido_cantidad INT(11) NULL DEFAULT 0, \
				coincidencias_art INT(11) NULL DEFAULT 0,  \
				reemplazado TINYINT(1) NULL DEFAULT 0, \
				PRIMARY KEY (pedido, producto, present) USING BTREE \
		) ENGINE = INNODB ");
		instrucciones[num_instrucciones++]=instruccion;

		//Para guardar cálculos de los art con reemplazos relacionados
		instruccion.sprintf(" CREATE TEMPORARY TABLE conteo_ree_rel ( \
				surtido VARCHAR(11) NOT NULL DEFAULT '0', \
				pedido VARCHAR(11) NOT NULL, \
				producto VARCHAR(8) NOT NULL DEFAULT '', \
				present VARCHAR(255) NOT NULL DEFAULT '', \
				pedidos_cantidad INT(11) NULL DEFAULT 0, \
				surtido_cantidad INT(11) NULL DEFAULT 0, \
				reemplazado TINYINT(1) NULL DEFAULT 0, \
				PRIMARY KEY (surtido, pedido, producto, present) USING BTREE \
				) ENGINE = INNODB ");
		instrucciones[num_instrucciones++]=instruccion;


		//Se guardarán los reemplazos de los articulos y sus tipos
		instruccion.sprintf(" CREATE TEMPORARY TABLE art_rem_cat ( \
			surtido VARCHAR(11) NOT NULL DEFAULT '0', \
			pedido VARCHAR(11) NOT NULL, \
			articulo VARCHAR(9) NOT NULL, \
			reemplazo_rel TINYINT(1) NULL DEFAULT 0, \
			reemplazo_exacto TINYINT(1) NULL DEFAULT 0, \
			PRIMARY KEY (surtido, pedido, articulo) USING BTREE \
		) ENGINE = INNODB ");
		instrucciones[num_instrucciones++]=instruccion;


		// Calculo de reemplazos exactos
		instruccion.sprintf(" INSERT IGNORE INTO conteo_ree_exac (pedido, producto, present, pedidos_cantidad, surtido_cantidad, coincidencias_art, reemplazado) \
			SELECT dpv.referencia AS pedido, a.producto, a.present, \
			COUNT(DISTINCT dpv.referencia, dpv.articulo) AS pedidos_cant, \
			COUNT(DISTINCT ds.surtido, ds.pedido, ds.articulo, ds.division, ds.almacen) AS surtidos_cant, \
			COUNT(DISTINCT CASE WHEN dpv.articulo = ds.articulo THEN ds.articulo END) AS coincidencias_art, \
			COUNT(DISTINCT sr.surtido, sr.division, sr.articuloped, sr.articulosurt) AS reemplazados \
			FROM dpedidosventa dpv \
			INNER JOIN articulos a ON a.articulo = dpv.articulo \
			LEFT JOIN dsurtidopedidos ds ON ds.pedido = dpv.referencia AND EXISTS( \
				SELECT 1 FROM articulos asur \
				WHERE asur.articulo = ds.articulo \
				AND asur.producto = a.producto AND asur.present = a.present \
			) \
            LEFT JOIN articulos asur ON asur.articulo = ds.articulo AND asur.producto = a.producto AND asur.present = a.present \
			LEFT JOIN surtidoartreemplazados sr ON sr.pedido = ds.pedido AND sr.articuloped = ds.articulo \
			WHERE  dpv.referencia IN ( \
				SELECT d.pedido \
				FROM dsurtidopedidos d \
				WHERE d.surtido = '%s' \
			) %s     \
			GROUP BY dpv.referencia, a.producto, a.present \
			HAVING pedidos_cant = surtidos_cant AND pedidos_cant = coincidencias_art AND reemplazados > 0 ",
			 surtido, condicion_dpedidos);
		instrucciones[num_instrucciones++]=instruccion;

		//Se guardar los articulos con reemplazos exactos
		instruccion.sprintf(" INSERT INTO art_rem_cat(surtido, pedido, articulo, reemplazo_rel, reemplazo_exacto) \
			SELECT ds.surtido, ar.pedido, a.articulo, 0 AS rem, 1 AS rem_exacto \
			FROM conteo_ree_exac ar \
			INNER JOIN dpedidosventa dpv ON dpv.referencia = ar.pedido \
			INNER JOIN articulos a ON a.articulo = dpv.articulo AND a.producto = ar.producto AND a.present = ar.present \
			INNER JOIN dsurtidopedidos ds ON ds.pedido = dpv.referencia AND ds.articulo = a.articulo \
			LEFT JOIN surtidoartreemplazados sr ON sr.pedido = ds.pedido AND sr.articuloped = ds.articulo \
			WHERE ds.surtido = '%s' AND ds.reemplazado = 1 ", surtido);
		instrucciones[num_instrucciones++]=instruccion;

		//Se calculan los reemplazos relacionados o conjugados
		instruccion.sprintf(" INSERT IGNORE INTO conteo_ree_rel (surtido, pedido, producto, present, pedidos_cantidad, surtido_cantidad, reemplazado) \
			SELECT ds.surtido, dpv.referencia, a.producto, a.present, COUNT(DISTINCT dpv.referencia, a.articulo) AS cantidad_dpedido, \
			COUNT(DISTINCT ds.surtido, ds.pedido, ds.articulo, a.producto, a.present, ds.division, ds.almacen) AS cantidad_dsur, \
			MAX(ds.reemplazado) AS algun_reemplazo \
			FROM dpedidosventa dpv \
			INNER JOIN articulos a ON a.articulo = dpv.articulo \
			INNER JOIN dsurtidopedidos ds ON ds.pedido = dpv.referencia \
			INNER JOIN articulos asur ON asur.articulo = ds.articulo AND asur.producto = a.producto AND asur.present = a.present \
			LEFT JOIN conteo_ree_exac cre ON cre.pedido = dpv.referencia \
				AND cre.producto = a.producto AND cre.present = a.present \
			WHERE  dpv.referencia IN ( \
				SELECT d.pedido \
				FROM dsurtidopedidos d \
				WHERE d.surtido = '%s' \
			) AND cre.pedido IS NULL %s \
			GROUP BY dpv.referencia, a.producto, a.present \
			HAVING algun_reemplazo > 0 ", surtido, condicion_dpedidos);
		instrucciones[num_instrucciones++]=instruccion;

        //Se guardar los articulos con reemplazos directos y relacionados
		instruccion.sprintf(" INSERT IGNORE INTO art_rem_cat(surtido, pedido, articulo, reemplazo_rel, reemplazo_exacto) \
			SELECT rd.surtido, rd.pedido, a.articulo, 1 AS rem_rel, \
			IF(rd.pedidos_cantidad = 1 AND rd.surtido_cantidad = 1, 1, 0) AS rem_exacto \
			FROM conteo_ree_rel rd \
			INNER JOIN dpedidosventa dpv ON dpv.referencia = rd.pedido \
			INNER JOIN articulos a ON a.articulo = dpv.articulo AND a.producto = rd.producto  \
				AND a.present = rd.present \
			LEFT JOIN dsurtidopedidos ds ON ds.pedido = dpv.referencia AND ds.articulo = a.articulo \
			AND ds.cantidad = dpv.cantidad AND ds.reemplazado = 0 \
			WHERE ds.surtido IS NULL ");
		instrucciones[num_instrucciones++]=instruccion;



		instrucciones[num_instrucciones++]="COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (int i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
		if(mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)){

			//pedidos relacionados al surtido
			instruccion.sprintf("SELECT pv.referencia, pv.fechaped AS fechapedido \
				FROM surtidopedidos s \
				INNER JOIN embarques e ON e.embarque = s.embarque \
				INNER JOIN pedidosventa pv ON pv.embarque = e.embarque \
				WHERE s.surtido = '%s'", surtido);
			mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Articulos reemplazados del pedido en el surtido
			instruccion.sprintf(" SELECT \
				sar.pedido, ap.producto AS producto_ped, ppe.nombre AS nombre_ped, ap.present AS present_ped, \
				ar.articulo AS art_sur, ar.producto as producto_sur, psu.nombre AS nombre_sur, \
				ar.present as present_sur, ar.multiplo AS multiplo_sur, dps.cantidad AS cant_sur \
				FROM surtidoartreemplazados sar \
				INNER JOIN articulos ap ON ap.articulo = sar.articuloped \
				INNER JOIN productos ppe ON ppe.producto = ap.producto \
				INNER JOIN articulos ar ON ar.articulo = sar.articulosurt \
				INNER JOIN productos psu ON psu.producto = ar.producto \
				INNER JOIN dsurtidopedidos dps ON dps.surtido = sar.surtido \
					AND dps.pedido = sar.pedido AND dps.articulo = ap.articulo \
				WHERE sar.surtido = '%s' %s ",
				surtido, condicion_pedido_rem);
			mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);


			// Detalles de pedidos
			instruccion.sprintf(" SELECT pv.referencia AS pedido, pv.embarque, sp.surtido, \
				dp.articulo, SUM(dp.cantidad) as cantidad_pedida, \
				ap.producto, ap.present, ap.multiplo, ap.factor, opp.orden, p.nombre, \
				COUNT(DISTINCT  dp.referencia, ap.articulo) AS cant_art, \
				 SUM(IFNULL(arc.reemplazo_rel, 0)) AS sum_rem_rel, \
				 SUM(IFNULL(arc.reemplazo_exacto, 0)) AS sum_rem_exa, \
                 pre.permitfrac AS permite_fraccion \
				FROM pedidosventa pv \
				INNER JOIN dpedidosventa dp ON dp.referencia = pv.referencia \
				INNER JOIN articulos ap ON ap.articulo = dp.articulo \
				INNER JOIN productos p ON p.producto = ap.producto \
				INNER JOIN surtidopedidos sp ON sp.embarque = pv.embarque \
				INNER JOIN auxproductospresentacion opp ON opp.producto = ap.producto AND opp.present = ap.present \
				LEFT JOIN art_rem_cat arc ON arc.pedido = dp.referencia AND arc.articulo = ap.articulo \
				LEFT JOIN presentaciones pre ON pre.producto = ap.producto AND pre.present = ap.present \
				WHERE pv.embarque = '%s' %s \
				GROUP BY ap.articulo \
				ORDER BY opp.orden, ap.factor DESC ", embarque, condicion_pedido);
			mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Detalles de lo surtido
			instruccion.sprintf(" SELECT dsp.surtido, dsp.articulo, SUM(dsp.cantidad) as cantidad, \
				dsp.reemplazado, dsp.division , dsp.pedido, a.producto, a.present, \
				a.multiplo, a.factor, opp.orden \
				FROM dsurtidopedidos dsp \
				INNER JOIN articulos a ON a.articulo = dsp.articulo \
				INNER JOIN auxproductospresentacion opp ON opp.producto = a.producto \
				AND opp.present = a.present \
				WHERE dsp.surtido = '%s' AND dsp.reemplazado = 0 %s \
				GROUP BY a.articulo \
				ORDER BY opp.orden, a.factor DESC, dsp.pedido ", surtido, condicion_dpedido);
			mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

		}


	}
	__finally {
        delete buffer_sql;
	}
}
//------------------------------------------------------------------------------
//ID_GUARDA_BITACORA_VENTA_BBVA
void ServidorVentas::GuardaBitacoraVentaPinPadBBVA(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    char *buffer_sql=new char[1024*64*20];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[100];
	AnsiString Instruccion;
	AnsiString campos, valores;

	campos = mFg.ExtraeStringDeBuffer(&parametros);
	valores = mFg.ExtraeStringDeBuffer(&parametros);

	try {

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		instruccion.sprintf("INSERT INTO detoperbbva \
		(%s) \
		VALUES \
		(%s) ",
		campos,
		valores);
		instrucciones[num_instrucciones++]=instruccion;

		instrucciones[num_instrucciones++]="COMMIT";

		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (int i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);


		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);

	} __finally {
		delete buffer_sql;
	}
}
//------------------------------------------------------------------------------
//ID_AES_WS_INTERMEDIARIO
void ServidorVentas::WSIntermediarioAPIKey(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
	AnsiString cod, parametro;
	int idEmpresa = 0;

	cod = mFg.ExtraeStringDeBuffer(&parametros);
	parametro = mFg.ExtraeStringDeBuffer(&parametros);
	idEmpresa = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	instruccion = "SELECT IFNULL(AES_DECRYPT( valor ,'" + cod + "'),'') as valor FROM paramseguridad \
					where parametro = '" + parametro + "' AND idempresa = "+ idEmpresa;

	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}
//------------------------------------------------------------------------------
