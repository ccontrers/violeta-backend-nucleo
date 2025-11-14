#include <vcl.h>
// ---------------------------------------------------------------------------
#include "pch.h"

#pragma hdrstop

#include "ClassServidorCatalogos.h"
#include "ClassServidorVioleta.h"
#include "ClassBufferRespuestas.h"
#include "violetaS.h"
#include "FormServidorVioleta.h"
#include "comunes.h"

#include "IdComponent.hpp"
#include "IdIntercept.hpp"
#include "IdLogFile.hpp"
#include "IdMessage.hpp"
#include "IdSMTP.hpp"
#include <IdText.hpp>
#include <IdAttachmentMemory.hpp>
#include <IdSSLOpenSSL.hpp>

// ---------------------------------------------------------------------------

#pragma package(smart_init)

// ---------------------------------------------------------------------------

// PARAMETROS

// ---------------------------------------------------------------------------
// ID_CON_PARAMETROS
void ServidorCatalogos::ConsultaParametros(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {

	AnsiString instruccion;

	// CONSULTA PARAMETROS
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
	// Obtiene todos los parametros

	instruccion.sprintf("select * from parametrosemp where sucursal='%s' ", FormServidor->ObtieneClaveSucursal());
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,
		instruccion.c_str(), Respuesta->TamBufferResultado);
}

// ---------------------------------------------------------------------------

// GENERICOS

// ---------------------------------------------------------------------------
//
void ServidorCatalogos::GrabaGenerico(RespuestaServidor *Respuesta,
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

// ---------------------------------------------------------------------------
void ServidorCatalogos::BajaGenerico(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros, AnsiString Tabla, AnsiString CampoClave) {
	// BAJA GENERICO
	DatosTabla datos(mServidorVioleta->Tablas);
	char *buffer_sql = new char[1024 * 5];
	char *aux_buffer_sql = buffer_sql;
	AnsiString instruccion;
	int num_instrucciones = 1;
	AnsiString clave;
	AnsiString clave_formateada;
	char tipo;

	try {
		clave = mFg.ExtraeStringDeBuffer(&parametros);

		// Formatea el valor clave
		datos.AsignaTabla(Tabla.c_str());
		tipo = datos.ObtieneTipoCampo(CampoClave);
		if (tipo == 'I' || tipo == 'N' || tipo == 'F')
			clave_formateada = clave;
		else
			clave_formateada = "'" + clave + "'";

		// Genera la instruccion que borra y la inserta al buffer de acciones.
		instruccion = "delete from " + Tabla + " where " + CampoClave + "=" +
			clave_formateada;
		aux_buffer_sql = mFg.AgregaStringABuffer
			(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		mFg.AgregaStringABuffer(instruccion, aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta,
			TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,
			buffer_sql);
	}
	__finally {
		delete buffer_sql;
	}
}

// ---------------------------------------------------------------------------
void ServidorCatalogos::ConsultaGenerico(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros, AnsiString Tabla, AnsiString CampoClave) {
	// CONSULTA GENERICO
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instruccion;
	AnsiString clave;
	AnsiString clave_formateada;
	char tipo;

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Formatea el valor clave
	datos.AsignaTabla(Tabla.c_str());
	tipo = datos.ObtieneTipoCampo(CampoClave);
	if (tipo == 'I' || tipo == 'N' || tipo == 'F')
		clave_formateada = clave;
	else
		clave_formateada = "'" + clave + "'";

	// Obtiene todos los datos de la tabla en cuestión
	instruccion = "select * from " + Tabla + " where " + CampoClave + "=" +
		clave_formateada;
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

// CLIENTES

// ---------------------------------------------------------------------------
// ID_GRA_CLIENTE
void ServidorCatalogos::GrabaCliente(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA CLIENTE
	char *buffer_sql = new char[1024 * 5];
	char *aux_buffer_sql = buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString tarea_cliente, tarea_credito, clave_cliente, latitud_cliente, longitud_cliente, ubicacion_gis_entrega; // , ventasxvolumen;
	int num_telefonos, num_direcciones_entrega, i;
	int num_instrucciones = 0;
	AnsiString instruccion, instrucciones[100];

	try {
		// ventasxvolumen=mFg.ExtraeStringDeBuffer(&parametros);

		tarea_cliente = mFg.ExtraeStringDeBuffer(&parametros);
		// Indicador si se va a agregar o modificar
		clave_cliente = mFg.ExtraeStringDeBuffer(&parametros);
		// Clave del cliente
		latitud_cliente = mFg.ExtraeStringDeBuffer(&parametros);
		// Latitud del cliente
		longitud_cliente = mFg.ExtraeStringDeBuffer(&parametros);
		// Longitud del cliente


		datos.AsignaTabla("clientes");
		parametros += datos.InsCamposDesdeBuffer(parametros);
		datos.AsignaValorCampo("cliente", "@folio", 1);
//		datos.AsignaValorCampo("ubicaciongis", "@ubicacion", 2);

		instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++] = "START TRANSACTION";

		datos.InsCampo("fechamodi", mFg.DateToAnsiString(Today()));
		// datos.InsCampo("venxvol", ventasxvolumen.ToInt());

		if (tarea_cliente == "A") {
			instruccion.sprintf(
				"select @folioaux:=valor from foliosemp where folio='CLIENTES' AND sucursal = '%s' %s ",
				FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
			instrucciones[num_instrucciones++] = instruccion;
			instruccion.sprintf("set @foliosig=@folioaux+1");
			instrucciones[num_instrucciones++] = instruccion;
			instruccion.sprintf("set @folioaux=cast(@folioaux as char)");
			instrucciones[num_instrucciones++] = instruccion;
			instruccion.sprintf
				("set @folio=concat('%s', lpad(@folioaux,5,'0'))", FormServidor->ObtieneClaveSucursal());
			instrucciones[num_instrucciones++] = instruccion;
			instruccion.sprintf
				("set @ubicacion=POINT(%s,%s)", latitud_cliente, longitud_cliente);
			instrucciones[num_instrucciones++] = instruccion;
			instruccion.sprintf(
				"update foliosemp set valor=@foliosig where folio='CLIENTES' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
			instrucciones[num_instrucciones++] = instruccion;

			datos.InsCampo("fechaalta", mFg.DateToAnsiString(Today()));
			datos.InsCampo("sucursal", FormServidor->ObtieneClaveSucursal());
			if (latitud_cliente.Length()>3 && longitud_cliente.Length()>3) {
				datos.InsCampo("ubicaciongis", "@ubicacion",1);
			}

			//datos.InsCampo("metododef","99");//valor por defecto de cformapago
			//datos.InsCampo("metodosup","99");//valor por defecto de cformapago

			instrucciones[num_instrucciones++] = datos.GenerarSqlInsert();

		}
		else {
			instruccion.sprintf("set @folio='%s'", clave_cliente);
			instrucciones[num_instrucciones++] = instruccion;
			instruccion.sprintf("set @ubicacion=POINT(%s,%s)", latitud_cliente,longitud_cliente);
			instrucciones[num_instrucciones++] = instruccion;
			if (latitud_cliente.Length()>3 && longitud_cliente.Length()>3) {
				datos.InsCampo("ubicaciongis", "@ubicacion",1);
			}
			instrucciones[num_instrucciones++] = datos.GenerarSqlUpdate
				("cliente='" + clave_cliente + "'");
		}


		//Presentacionescb
		datos.AsignaTabla("clientesemp");
		parametros += datos.InsCamposDesdeBuffer(parametros);

		if (tarea_cliente == "A") {
			BufferRespuestas* resp_emp=NULL;

			try {
				instruccion = "SELECT idempresa FROM empresas ";

				if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_emp)) {

					for(int i=0; i<resp_emp->ObtieneNumRegistros(); i++){
						resp_emp->IrAlRegistroNumero(i);

						if(FormServidor->ObtieneClaveEmpresa() == resp_emp->ObtieneDato("idempresa")){
							AnsiString tipopre, tipoprecmin, vendedor, cobrador;
							AnsiString campos="cliente, idempresa";
							AnsiString valor="@folio,"+resp_emp->ObtieneDato("idempresa");
							AnsiString Insert = "";

							tipopre = datos.ObtieneValorCampo("tipoprec", false);
							tipoprecmin = datos.ObtieneValorCampo("tipoprecmin", false);
							vendedor = datos.ObtieneValorCampo("vendedor", false);
							cobrador = datos.ObtieneValorCampo("cobrador", false);

							if(tipopre != "" && tipopre != "(NULL)" && tipopre != "(null)") {
								campos += ",tipoprec";
								valor += ",'"+tipopre+"'";
							}
							if(tipoprecmin != "" && tipoprecmin != "(NULL)" && tipoprecmin != "(null)") {
								campos += ",tipoprecmin";
								valor += ",'"+tipoprecmin+"'";
							}
							if(vendedor != "" && vendedor != "(NULL)" && vendedor != "(null)") {
								campos += ",vendedor";
								valor += ",'"+vendedor+"'";
							}
							if(cobrador != "" && cobrador != "(NULL)" && cobrador != "(null)") {
								campos += ",cobrador";
								valor += ",'"+cobrador+"'";
							}

							instruccion = "INSERT INTO clientesemp ("+campos+") VALUES ("+valor+")";
							instrucciones[num_instrucciones++]=instruccion;
						}else{
							BufferRespuestas* resp_param=NULL;
							BufferRespuestas* resp_paramVend=NULL;
							BufferRespuestas* resp_paramCobr=NULL;
							try {
								AnsiString select;
								select.sprintf("SELECT valor FROM parametrosemp p \
									INNER JOIN sucursales suc ON suc.sucursal = p.sucursal AND suc.idempresa = %s \
									WHERE parametro='TIPOPREDF' LIMIT 1",
								resp_emp->ObtieneDato("idempresa"));

								// para vendedor - cobrador
								AnsiString selectVen, selectCobr;
								AnsiString valVend, valCobr;

								selectVen.sprintf("SELECT valor FROM parametrosglobemp \
									WHERE parametro = 'DEFAVEND' AND idempresa=%s",resp_emp->ObtieneDato("idempresa"));
								mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, selectVen.c_str(), resp_paramVend);
								valVend = resp_paramVend->ObtieneDato("valor");

								selectCobr.sprintf("SELECT valor FROM parametrosglobemp \
									WHERE parametro = 'DEFACOBR' AND idempresa=%s",resp_emp->ObtieneDato("idempresa"));
								mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, selectCobr.c_str(), resp_paramCobr);
								valCobr = resp_paramCobr->ObtieneDato("valor");
								//
								if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, select.c_str(), resp_param)) {
									if (resp_param->ObtieneNumRegistros()>0){
										AnsiString tprecio_default= resp_param->ObtieneDato("valor");

										instruccion.sprintf("INSERT INTO clientesemp (cliente, idempresa, tipoprec, tipoprecmin, vendedor, cobrador) VALUES                                     								   \
										(@folio,%s,'%s','%s','%s','%s')",
										resp_emp->ObtieneDato("idempresa"), tprecio_default, tprecio_default, valVend, valCobr);
										instrucciones[num_instrucciones++]=instruccion;

									} else
										throw (Exception("No se encuentra registro TIPOPREDF en tabla parametrosglobemp "));

								} else
									throw (Exception("Error al consultar en tabla parametrosglobemp"));

							} __finally {
								if (resp_param!=NULL) delete resp_param;
								if (resp_paramVend!=NULL) delete resp_paramVend;
								if (resp_paramCobr!=NULL) delete resp_paramCobr;
							}

						}


					}

				} else
					throw (Exception("Error al consultar la tabla empresas"));
			} __finally {
				if (resp_emp!=NULL) delete resp_emp;
			}

		}else{
            instrucciones[num_instrucciones++] = datos.GenerarSqlUpdate
				("cliente='" + clave_cliente + "' AND idempresa="+FormServidor->ObtieneClaveEmpresa() );
		}


		if (mFg.ExtraeStringDeBuffer(&parametros) == "1") {
			// Para ver si hay modificaciones en los teléfonos
			if (tarea_cliente == "M") {
				instruccion.sprintf(
					"delete from telefonosclientes where cliente=@folio");
				instrucciones[num_instrucciones++] = instruccion;
			}
			num_telefonos = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
			// Obtiene el número de teléfonos
			for (i = 0; i < num_telefonos; i++) {
				datos.AsignaTabla("telefonosclientes");
				parametros += datos.InsCamposDesdeBuffer(parametros);
				datos.AsignaValorCampo("cliente", "@folio", 1);
				instrucciones[num_instrucciones++] = datos.GenerarSqlInsert();
			}
		}

        if (mFg.ExtraeStringDeBuffer(&parametros) == "1") {
			// Para ver si hay modificaciones en las direcciones de entrega
			if (tarea_cliente == "M") {
				instruccion.sprintf(
					"delete from direccionesentregaclientes where cliente=@folio");
				instrucciones[num_instrucciones++] = instruccion;
			}
			num_direcciones_entrega = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
			// Obtiene el número de direcciones de entrega
			for (i = 0; i < num_direcciones_entrega; i++) {
				datos.AsignaTabla("direccionesentregaclientes");
				parametros += datos.InsCamposDesdeBuffer(parametros);
				datos.AsignaValorCampo("cliente", "@folio", 1);
				datos.AsignaValorCampo("fechaalta", mFg.DateToAnsiString(Today()));
				ubicacion_gis_entrega = mFg.ExtraeStringDeBuffer(&parametros);
				if (ubicacion_gis_entrega.Length()>5) {
                    instruccion.sprintf("set @ubicacionent=POINT(%s)", ubicacion_gis_entrega);
					instrucciones[num_instrucciones++] = instruccion;
					datos.InsCampo("ubicaciongis", "@ubicacionent",1);
				}
				instrucciones[num_instrucciones++] = datos.GenerarSqlInsert();
			}
		}

		if (mFg.ExtraeStringDeBuffer(&parametros) == "1") {
			// Para ver si hay datos de crédito
			tarea_credito = mFg.ExtraeStringDeBuffer(&parametros);
			// Indicador si se va a agregar o modificar
			datos.AsignaTabla("datoscredito");
			parametros += datos.InsCamposDesdeBuffer(parametros);
			if (tarea_credito == "A") {
				datos.AsignaValorCampo("cliente", "@folio", 1);
				instrucciones[num_instrucciones++] = datos.GenerarSqlInsert();
			}
			else
				instrucciones[num_instrucciones++] = datos.GenerarSqlUpdate
					("cliente=@folio");
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
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,
				buffer_sql)) {
			instruccion.sprintf("select @folio as folio");
			mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,
				instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	}
	__finally {
		delete buffer_sql;
	}
}
// ---------------------------------------------------------------------------
// ID_GRA_RUTAEMBARQUE
void ServidorCatalogos::GrabaRutaEmbarque(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA RUTA EMBARQUE
	char *buffer_sql = new char[300 * 500];
	char *aux_buffer_sql = buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString clave_embarque, ubicacion_gis_entrega, ubicaciondellegada_gis, ubicaciondesalida_gis;
	int num_direcciones_ruta, i;
	int num_instrucciones = 0;
	AnsiString instruccion, instrucciones[500];

	try {
		clave_embarque = mFg.ExtraeStringDeBuffer(&parametros);

		instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++] = "START TRANSACTION";
		instruccion.sprintf("set @embarque='%s'", clave_embarque);
		instrucciones[num_instrucciones++] = instruccion;

		// Obtiene el número de direcciones de la Ruta de Embarque
		num_direcciones_ruta = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
		if (num_direcciones_ruta>0) {

			instruccion.sprintf("delete from embarquesruta where embarque=@embarque");
			instrucciones[num_instrucciones++] = instruccion;


			for (i = 0; i < num_direcciones_ruta; i++) {
				datos.AsignaTabla("embarquesruta");
				parametros += datos.InsCamposDesdeBuffer(parametros);

				ubicacion_gis_entrega  =  mFg.ExtraeStringDeBuffer(&parametros);
				ubicaciondellegada_gis =  mFg.ExtraeStringDeBuffer(&parametros);
				ubicaciondesalida_gis  =  mFg.ExtraeStringDeBuffer(&parametros);


				if (ubicacion_gis_entrega.Length()>5) {
					instruccion.sprintf("set @ubicacion=POINT(%s)", ubicacion_gis_entrega);
					instrucciones[num_instrucciones++] = instruccion;
					datos.InsCampo("ubicaciongis", "@ubicacion",1);
				}
				if (ubicaciondellegada_gis.Length()>5) {
					instruccion.sprintf("set @ubicacion_ubicaciondellegada=POINT(%s)", ubicaciondellegada_gis);
					instrucciones[num_instrucciones++] = instruccion;
					datos.InsCampo("ubicaciondellegada", "@ubicacion_ubicaciondellegada",1);
				}
				if (ubicaciondesalida_gis.Length()>5) {
					instruccion.sprintf("set @ubicacion_ubicaciondesalida=POINT(%s)", ubicaciondesalida_gis);
					instrucciones[num_instrucciones++] = instruccion;
					datos.InsCampo("ubicaciondesalida", "@ubicacion_ubicaciondesalida",1);
				}


				datos.InsCampo("fechaalta","'"+ mFg.DateToMySqlDate(Today())+"'",1);
				instrucciones[num_instrucciones++] = datos.GenerarSqlInsert();
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

		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,
				buffer_sql)) {
			instruccion.sprintf("select '%s' as embarque " , clave_embarque);
			mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,
				instruccion.c_str(), Respuesta->TamBufferResultado);
		}

	}
	__finally {
		delete buffer_sql;
	}
}


// ---------------------------------------------------------------------------
// ID_BAJ_CLIENTE
void ServidorCatalogos::BajaCliente(RespuestaServidor *Respuesta, MYSQL *MySQL,
	char *parametros) {
	// BAJA CLIENTE
	char *buffer_sql = new char[1024 * 5];
	char *aux_buffer_sql = buffer_sql;
	AnsiString instruccion;
	int num_instrucciones = 7;
	AnsiString clave_cliente;

	try {
		clave_cliente = mFg.ExtraeStringDeBuffer(&parametros);
		aux_buffer_sql = mFg.AgregaStringABuffer
			(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);

		instruccion = "SET AUTOCOMMIT=0";
		aux_buffer_sql = mFg.AgregaStringABuffer(instruccion, aux_buffer_sql);

		instruccion = "START TRANSACTION";
		aux_buffer_sql = mFg.AgregaStringABuffer(instruccion, aux_buffer_sql);

		instruccion.sprintf("delete from telefonosclientes where cliente='%s'",
			clave_cliente);
		aux_buffer_sql = mFg.AgregaStringABuffer(instruccion, aux_buffer_sql);

		instruccion.sprintf("delete from datoscredito where cliente='%s'",
			clave_cliente);
		aux_buffer_sql = mFg.AgregaStringABuffer(instruccion, aux_buffer_sql);
		// Instruccion 1

        instruccion.sprintf("delete from clientesemp where cliente='%s'",
			clave_cliente);
		aux_buffer_sql = mFg.AgregaStringABuffer(instruccion, aux_buffer_sql);

		instruccion.sprintf("delete from clientes where cliente='%s'",
			clave_cliente);
		aux_buffer_sql = mFg.AgregaStringABuffer(instruccion, aux_buffer_sql);
		// Instruccion 1

		instruccion = "COMMIT";
		mFg.AgregaStringABuffer(instruccion, aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta,
			TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,
			buffer_sql);
	}
	__finally {
		delete buffer_sql;
	}
}

// ---------------------------------------------------------------------------
// ID_CON_CLIENTE
void ServidorCatalogos::ConsultaCliente(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA CLIENTE
	AnsiString instruccion;
	AnsiString clave_cliente;
	AnsiString persona;


	AnsiString paramversioncfdi;
	BufferRespuestas* resp_paramversioncfdi=NULL;
	BufferRespuestas* resp_tipoPersona=NULL;
	try {
		//instruccion="select p.* from parametroscfd p where p.sucursal='"+ FormServidor->ObtieneClaveSucursal()+"'" ;
		instruccion.sprintf("SELECT valor FROM parametrosglobemp WHERE parametro='VERSIONCFDI' AND idempresa = %s ", FormServidor->ObtieneClaveEmpresa());
		if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_paramversioncfdi)) {
			if (resp_paramversioncfdi->ObtieneNumRegistros()>0){
				//paramversioncfdi=resp_paramversioncfdi->ObtieneDato("versioncfdi");
				paramversioncfdi=resp_paramversioncfdi->ObtieneDato("valor");
			} else throw (Exception("No se encuentra registro VERSIONCFDI en tabla parametrosglobemp "));
			//throw (Exception("No se encuentra registro versioncfdi en tabla parametroscfd"));
		} else throw (Exception("Error al consultar en tabla parametrosglobemp"));
		//throw (Exception("Error al consultar en tabla parametroscfd"));
	} __finally {
		if (resp_paramversioncfdi!=NULL) delete resp_paramversioncfdi;
	}

	clave_cliente = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
   if(clave_cliente != ""){
		try {
			//instruccion="select p.* from parametroscfd p where p.sucursal='"+ FormServidor->ObtieneClaveSucursal()+"'" ;
			instruccion.sprintf("SELECT tipoempre FROM clientes WHERE cliente='%s'", clave_cliente);
			if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_tipoPersona)) {
				if (resp_tipoPersona->ObtieneNumRegistros()>0){
					//paramversioncfdi=resp_paramversioncfdi->ObtieneDato("versioncfdi");
					persona=resp_tipoPersona->ObtieneDato("tipoempre");
				} else throw (Exception("No se encuentra un registro con esa clave de cliente "));
				//throw (Exception("No se encuentra registro versioncfdi en tabla parametroscfd"));
			} else throw (Exception("Error al consultar en tabla clientes el campo de tipoempre"));
			//throw (Exception("Error al consultar en tabla parametroscfd"));
		} __finally {
			if (resp_tipoPersona!=NULL) delete resp_tipoPersona;
		}
   }


	// Obtiene todos los datos del cliente
	instruccion.sprintf("select c.*, cet.vendedor AS vendedor, cet.cobrador AS cobrador, FORMAT(X(ubicaciongis),6) as latitud, FORMAT(Y(ubicaciongis),6) as longitud,  \
	cet.tipoprecmin as tipoprecmin2 , cet.tipoprec as tipoprec2, c.credMax \
	from clientes c \
	left join clientesemp cet on cet.cliente=c.cliente and cet.idempresa=%s \
	where c.cliente='%s' ",
	   FormServidor->ObtieneClaveEmpresa(), clave_cliente);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los telefonos del cliente
	instruccion.sprintf(
		"select tipo, lada, telefono,extencionTel from telefonosclientes where cliente='%s'"
		, clave_cliente);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todas las direcciones de entrega del cliente
	instruccion.sprintf(
		"select d.iddireccion, d.calle, d.numext, d.numint, c.nombre as colonia, d.cp, \
		l.nombre as localidad, m.nombre as municipio,e.nombre as estado,d.referenciadomic, \
		CONCAT(X(d.ubicaciongis) ,',', Y(d.ubicaciongis)) as ubicacion , \
		d.dafault as principal, c.colonia as cveColonia, \
		l.localidad as cveLocalidad, m.municipio as cveMunicipio, e.estado as cveEstado, \
        X(d.ubicaciongis) as latitud, Y(d.ubicaciongis) as longitud \
		from direccionesentregaclientes as d left join \
		colonias as c on c.colonia=d.colonia left join \
		localidades as l on l.localidad=c.localidad left join \
		municipios as m on m.municipio=l.municipio left join \
		estados as e on e.estado=m.estado \
		 where d.cliente ='%s'"	, clave_cliente);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los datos DE CREDITO del cliente
	instruccion.sprintf("select * from datoscredito where cliente='%s'",
		clave_cliente);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los datos de TODOS los vendedores
	instruccion =
		"select empleados.empleado, concat(empleados.nombre,' ',empleados.appat,' ',empleados.apmat) AS Nombre from empleados, vendedores where empleados.empleado=vendedores.empleado and vendedores.activo=1 and empleados.activo=1 order by empleados.nombre,empleados.appat,empleados.apmat";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los datos de TODOS los cobradores
	instruccion = "select empleados.empleado, concat(empleados.nombre,' ',empleados.appat,' ',empleados.apmat) AS Nombre from empleados, \
		cobradores where empleados.empleado=cobradores.empleado and cobradores.activo=1 and empleados.activo=1 \
		order by empleados.nombre,empleados.appat,empleados.apmat";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los datos de TODOS los tipos de precios
	instruccion.sprintf("select tipoprec, descripcion, porcutil, verventmayoreo from tiposdeprecios where idempresa=%s order by porcutil desc",
	FormServidor->ObtieneClaveEmpresa());
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los datos de TODOS los tipos de bloqueos
	instruccion = "select tipo, descripcion, clasific from tiposdebloqueos";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	if(paramversioncfdi=="3.2"){
		// Obtiene todos los datos de TODOS los formas de pago
		instruccion =
		"SELECT formapago, descripcion, IF(condigitos=1,'Con dígitos', 'Sin dígitos') AS digitos, condigitos \
		FROM cformapago WHERE esver33='0' ORDER BY formapago";
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	}else{
    // Obtiene todos los datos de TODOS los formas de pago
		instruccion =
		"SELECT formapago, descripcion\
		FROM cformapago WHERE formapago<>'NA' AND formapago<>'99' ORDER BY formapago";
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	}

	if(persona==1){
		// obtendra los datos de la tabla cusocfdi, solo cuando la version == 3.33, dependiendo del tipo de persona
		instruccion = "select * from cusocfdi where pmoral<> 0 ORDER BY usocfdi";
	}else{
		instruccion = "select * from cusocfdi where pfisica<> 0 ORDER BY usocfdi";
	}
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),Respuesta->TamBufferResultado);

	//Verifica si es parte relacionada sucursal remota
	instruccion.sprintf(" SELECT \
	IF(pcfd.rfcemisor<>REPLACE(REPLACE(c.rfc,'-',''),' ',''),1,0)  AS relacion  \
	FROM clientes c \
	LEFT JOIN parametroscfd pcfd ON pcfd.rfcemisor <> REPLACE(REPLACE(c.rfc,'-',''),' ','') \
	WHERE c.cliente = '%s' 	AND c.esparterelac=1 \
	GROUP BY c.cliente", clave_cliente );
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	//Obtiene detalle de ecommerce
	instruccion.sprintf(" SELECT c.marketing, c.num_pedidos, c.verificaciontel, c.verificacionemail, \
    c.telefono, c.email, c.activo \
	FROM clientesdetalleecommerce c \
	WHERE c.cliente = '%s' ", clave_cliente );
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	//Obtiene detalle de ecommerce
	instruccion.sprintf(" SELECT dir.id, dir.calle, dir.numext, dir.numint, dir.codigopostal, \
    dir.telefono, dir.localidad, dir.colonia, dir.ciudad, dir.estado, dir.referencias, dir.esdefault \
	FROM direccionesentregaclientesecommerce dir \
	WHERE dir.cliente = '%s' ", clave_cliente );
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}

// ---------------------------------------------------------------------------

// STOCKS

// ---------------------------------------------------------------------------
// ID_CON_STOCKPRODUCTO
void ServidorCatalogos::ConsultaStockProducto(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	char *buffer_sql = new char[1024 * 64 * 10];
	char *aux_buffer_sql = buffer_sql;
	int i, dias_rango;
	double dias_venta;
	int num_instrucciones = 0;
	AnsiString instruccion, instrucciones[1000];
	AnsiString fecha;
	AnsiString considerar_cortes, mostrar_ventas_promedio,
	calculo_promedio_ventas;
	AnsiString producto, condicion_producto = " ";
	AnsiString presentacion, condicion_presentacion = " ";
	TDate fecha_inicial_promedio;
	int meses;
	AnsiString archivo_temp1, archivo_temp2, archivo_temp3, archivo_temp4,
	archivo_temp5;
	AnsiString idEmpresa;

	try {
		producto = mFg.ExtraeStringDeBuffer(&parametros);
		presentacion = mFg.ExtraeStringDeBuffer(&parametros);
		mostrar_ventas_promedio = mFg.ExtraeStringDeBuffer(&parametros);
		meses = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
		idEmpresa = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));

		fecha = mFg.DateToMySqlDate(Today());
		if (mostrar_ventas_promedio == "1")
			considerar_cortes = "0";
		else
			considerar_cortes = "1";
		fecha_inicial_promedio = IncMonth(mFg.MySqlDateToTDate(fecha.c_str()),
			meses*-1);
		if (meses>0)
			dias_venta=DaysBetween(fecha_inicial_promedio,Today())/meses;
		else
			dias_venta=1.00;

		if (producto != " ") {
			condicion_producto.sprintf("a.producto='%s' and ", producto);
		}
		if (presentacion != " ") {
			condicion_presentacion.sprintf("a.present='%s' and ", presentacion);
		}

		instruccion.sprintf("set @fechafinal='%s'", fecha);
		instrucciones[num_instrucciones++] = instruccion;

		// Obtiene fecha del corte más próximo previo a la fecha dada
		if (considerar_cortes == "1") {
			instruccion.sprintf("select @fechacorte:=ifnull(max(p.fecha),'1900-01-01') as fcorte \
				 from puntoscorte p where p.fecha<='%s'", fecha);
		}
		else {
			instruccion.sprintf("set @fechacorte='1900-01-01'");
		}
		instrucciones[num_instrucciones++] = instruccion;

		// Crea una tabla donde se pondrán las existencias que se vayan encontrando.
		instruccion =
			"create temporary table existenciasaux ( \
			almacen varchar(4), \
			tipo varchar(2), cantidad decimal(12,3), cantvpprom decimal(12,3)) Engine = InnoDB";
		instrucciones[num_instrucciones++] = instruccion;

		// Crea una tabla donde se pondrán las existencias sumadas
		instruccion = "create temporary table auxexistsumadas ( \
			almacen varchar(4), \
			cantidad decimal(12,3), cantvpprom decimal(12,3)) Engine = InnoDB";
		instrucciones[num_instrucciones++] = instruccion;

		// Calcula base (0) de los artículos en cuestión.
		// La utilidad de esto es simplemente para tomar en cuenta todos los articulos aunque no
		// tengan ningun movimiento
		instruccion.sprintf("insert into existenciasaux (almacen, tipo, cantidad) \
			select al.almacen, 'BA' as tipo, \
			0 as cantidad \
			from articulos a, almacenes al \
			where a.producto='%s' and a.present='%s' \
			group by al.almacen", producto, presentacion);
		instrucciones[num_instrucciones++] = instruccion;

		// Calcula las ventas.
        //hace la conversion de meses a dias
		dias_rango = meses*30;
		if (meses > 0 && considerar_cortes == "0") {
			calculo_promedio_ventas.sprintf(
				" vxm.ventas%d ",dias_rango);
		}
		else {
			calculo_promedio_ventas = "0.000";
		}
		archivo_temp2 = mServidorVioleta->ObtieneArchivoTemp(num_instrucciones,
			Respuesta->Id);

		instruccion.sprintf(" \
			select vxm.almacen, 'VE' as tipo, 0.000 as cantidad, \
			%s as cantvpprom \
			from ventasxmes vxm \
			where vxm.producto='%s' and vxm.present='%s' \
			INTO OUTFILE '%s' ", calculo_promedio_ventas,
			producto, presentacion, archivo_temp2);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf(
			"LOAD DATA INFILE '%s' INTO TABLE existenciasaux (almacen, tipo, cantidad, cantvpprom) ", archivo_temp2);
		instrucciones[num_instrucciones++] = instruccion;

		// Suma los movimientos para obtener las existencias
		instruccion.sprintf("insert into auxexistsumadas (almacen, cantidad, cantvpprom) \
				select e.almacen, ea.cantidad as cantidad,  (sum(if(tipo='VE',cantvpprom,0))/%d) as cantvpprom \
				from existenciasaux e \
				left join existenciasactuales ea on ea.almacen = e.almacen AND ea.producto = '%s' AND ea.present = '%s' \
				group by e.almacen ", meses, producto, presentacion);
		instrucciones[num_instrucciones++] = instruccion;

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql = mFg.AgregaStringABuffer
			(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i = 0; i < num_instrucciones; i++)
			aux_buffer_sql = mFg.AgregaStringABuffer(instrucciones[i],
			aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta,
			TAM_MAX_BUFFER_RESPUESTA_REP);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,
				buffer_sql)) {

			mServidorVioleta->BorraArchivoTemp(archivo_temp2);

			// Posibles multiplos del producto-presentacion
			instruccion.sprintf("select multiplo, factor from articulos a \
					where a.producto='%s' and a.present='%s' \
					and a.activo=1 order by a.factor desc", producto,
				presentacion);
			mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,
				instruccion.c_str(), Respuesta->TamBufferResultado);

			// Resultado final
			instruccion.sprintf("select sec.sucursal, sum(e.cantidad) as cantidad, sum(e.cantvpprom) as cantvpprom, \
				ifnull(s.minimo,0) as minimo, \
				ifnull(s.reorden,0) as reorden, \
				ifnull(s.maximo,0) as maximo, \
				sum(e.cantvpprom)/%.6f as cantvppromdia  \
				from auxexistsumadas e \
				INNER JOIN almacenes al ON al.almacen=e.almacen \
				INNER JOIN secciones sec ON al.seccion=sec.seccion \
                INNER JOIN sucursales suc ON suc.sucursal = sec.sucursal \
				left join stock s ON sec.sucursal=s.sucursal and s.producto='%s' and s.present='%s' \
                WHERE suc.idempresa = %s \
				group by sec.sucursal \
				order by sec.sucursal ", dias_venta, producto, presentacion, idEmpresa);
			mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,
				instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	}
	__finally {
		delete buffer_sql;
	}
}

// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// ID_GRA_STOCKPRODUCTO
void ServidorCatalogos::GrabaStockProducto(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA STOCK DE UN PRODUCTO
	char *buffer_sql = new char[1024 * 5];
	char *aux_buffer_sql = buffer_sql;
	AnsiString instruccion;
	int num_instrucciones = 5;
	AnsiString sucursal, producto, presentacion, minimo, maximo, reorden, confPedGlobal,
		usuario, minimoAnt, maximoAnt, reordenAnt;

	AnsiString idempresa;

	try {
		sucursal = mFg.ExtraeStringDeBuffer(&parametros);
		producto = mFg.ExtraeStringDeBuffer(&parametros);
		presentacion = mFg.ExtraeStringDeBuffer(&parametros);
		minimo = mFg.ExtraeStringDeBuffer(&parametros);
		maximo = mFg.ExtraeStringDeBuffer(&parametros);
		reorden = mFg.ExtraeStringDeBuffer(&parametros);
		confPedGlobal = mFg.ExtraeStringDeBuffer(&parametros);

		usuario = mFg.ExtraeStringDeBuffer(&parametros);
		minimoAnt = mFg.ExtraeStringDeBuffer(&parametros);
		maximoAnt = mFg.ExtraeStringDeBuffer(&parametros);
		reordenAnt = mFg.ExtraeStringDeBuffer(&parametros);
		idempresa = mFg.ExtraeStringDeBuffer(&parametros);

		//idempresa = FormServidor->ObtieneClaveEmpresa();

		if (sucursal=="TODAS") {
			num_instrucciones = 5;
		}
		aux_buffer_sql = mFg.AgregaStringABuffer
			(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);

		instruccion = "SET AUTOCOMMIT=0";
		aux_buffer_sql = mFg.AgregaStringABuffer(instruccion, aux_buffer_sql);

		instruccion = "START TRANSACTION";
		aux_buffer_sql = mFg.AgregaStringABuffer(instruccion, aux_buffer_sql);

		if (sucursal!="TODAS") {
			instruccion.sprintf("replace into stock (sucursal,producto,present, minimo,maximo,reorden) \
			values ('%s','%s','%s',%s,%s,%s)", sucursal, producto, presentacion,
			minimo, maximo, reorden);
			aux_buffer_sql = mFg.AgregaStringABuffer(instruccion, aux_buffer_sql);

            instruccion.sprintf(
			"INSERT INTO bitacorastock (producto, present, sucursal, minimoant, minimodesp,  \
										maximoant, maximodesp, reordenant, reordendesp,  \
										usuario, fecha, hora ) \
										VALUES( '%s','%s','%s',%s,%s,%s,%s,%s,%s,'%s',CURDATE(),CURTIME() ) ",
										producto, presentacion ,sucursal ,minimoAnt ,minimo ,maximoAnt
										,maximo ,reordenAnt, reorden, usuario );

			 aux_buffer_sql = mFg.AgregaStringABuffer(instruccion, aux_buffer_sql);

		}else{
			// Inserta los elementos con los datos actualizados para todas las sucursales que faltan.
			instruccion.sprintf(
			"INSERT IGNORE INTO stock (sucursal,producto,present, minimo,maximo,reorden) \
				SELECT s.sucursal, '%s' AS producto, '%s' AS present \
				, %s AS  minimo, %s AS maximo, %s AS reorden\
				FROM sucursales s WHERE s.idempresa = %s ",
				 producto, presentacion ,minimo, maximo, reorden, idempresa);
			 aux_buffer_sql = mFg.AgregaStringABuffer(instruccion, aux_buffer_sql);

			instruccion.sprintf("UPDATE stock s \
			INNER JOIN sucursales suc ON suc.sucursal = s.sucursal \
			SET minimo=%s,maximo=%s,reorden=%s  \
			WHERE producto='%s' and present='%s' AND suc.idempresa = %s "
			 ,minimo, maximo, reorden, producto, presentacion, idempresa );
			 aux_buffer_sql = mFg.AgregaStringABuffer(instruccion, aux_buffer_sql);
		}

		instruccion = "COMMIT";
		mFg.AgregaStringABuffer(instruccion, aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta,
			TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,
			buffer_sql);
	}
	__finally {
		delete buffer_sql;
	}
}

// ---------------------------------------------------------------------------

// EMPLEADOS

// ---------------------------------------------------------------------------
// ID_GRA_EMPLEADO
void ServidorCatalogos::GrabaEmpleado(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA EMPLEADO
	char *buffer_sql = new char[1024 * 6];
	char *aux_buffer_sql = buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
    AnsiString instruccion;
	AnsiString instrucciones[60];
	AnsiString tablas_adicionales[] = {"bodegueros", "cajeros", "choferes", "cobradores", "compradores","maniobristas", "surtidores","vendedores","recepcionistas","empleadossoporte"};
	int num_instrucciones = 0;
	AnsiString clave_empleado;
	AnsiString tarea,emp_activo;
	int i;
	BufferRespuestas* resp_edoempleado=NULL;

	try {
		// Indicador si se va a agregar o modificar
		tarea = mFg.ExtraeStringDeBuffer(&parametros);
		// Clave del empleado
		clave_empleado = mFg.ExtraeStringDeBuffer(&parametros);

		// Datos generales del empleado
		datos.AsignaTabla("empleados");
		parametros += datos.InsCamposDesdeBuffer(parametros);
		if (tarea == "A"){
			datos.InsCampo("empleadoidcorto", "SUBSTRING(SHA2(empleado,256), FLOOR(1 + RAND() * (58 - 1 +1)),6)", 1);
			instrucciones[num_instrucciones++] = datos.GenerarSqlInsert();
		}else{
			instrucciones[num_instrucciones++] = datos.GenerarSqlUpdate
				("empleado='" + clave_empleado + "'");
		}

		if(tarea == "M") {
			instruccion.sprintf("SELECT * FROM empleados WHERE empleado='%s'", clave_empleado);
			if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_edoempleado)) {
				if (resp_edoempleado->ObtieneNumRegistros()>0){
					emp_activo=resp_edoempleado->ObtieneDato("activo");
				}
			}
			if(emp_activo=="0"){
				instruccion.sprintf("update usuarios set activo=0 where empleado='%s'",clave_empleado);
				instrucciones[num_instrucciones++] = instruccion;
			}
		}

		// Datos de las tablas adicionales
		for (i = 0; i <= 9; i++) {
			if (mFg.ExtraeStringDeBuffer(&parametros) == "1")
			// Para ver si hay datos de cada tabla
			{
				datos.AsignaTabla(tablas_adicionales[i].c_str());
				tarea = mFg.ExtraeStringDeBuffer(&parametros);
				// Agregar o modificar
				parametros += datos.InsCamposDesdeBuffer(parametros);
				// Obtiene los datos de los campos
				if(i==7 && datos.ObtieneIndiceCampo("vendedor_imitar") > -1){
						AnsiString nuevo = datos.ObtieneValorCampo("vendedor_imitar");
						if(nuevo == "") datos.ReemplazaCampoValorNull("vendedor_imitar");
				}

				if (tarea == "A")
					instrucciones[num_instrucciones++] = datos.GenerarSqlInsert
						();
				else{
					instrucciones[num_instrucciones++] = datos.GenerarSqlUpdate("empleado='" + clave_empleado + "'");;
				}

			}
		}

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
		if (resp_edoempleado!=NULL) delete resp_edoempleado;
	}


}

// ---------------------------------------------------------------------------
// ID_BAJ_EMPLEADO
void ServidorCatalogos::BajaEmpleado(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// BAJA EMPLEADO
	char *buffer_sql = new char[1024 * 6];
	char *aux_buffer_sql = buffer_sql;
	char *tablas[] = {
		"bodegueros", "vendedores", "cajeros", "surtidores", "cobradores",
		"choferes", "compradores","maniobristas", "usuarios", "empleados"
	};

	AnsiString instruccion;
	int num_instrucciones = 11;
	int num_tablas = 10;
	AnsiString clave_empleado;

	try {
		clave_empleado = mFg.ExtraeStringDeBuffer(&parametros);

		aux_buffer_sql = mFg.AgregaStringABuffer
			(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);

		instruccion.sprintf(
			"delete from asignacionprivilegios where usuario='%s'",
			clave_empleado);
		aux_buffer_sql = mFg.AgregaStringABuffer(instruccion, aux_buffer_sql);

		for (int i = 0; i < num_tablas; i++) {
			instruccion.sprintf("delete from %s where empleado='%s'",
				tablas[i], clave_empleado);
			aux_buffer_sql = mFg.AgregaStringABuffer(instruccion,
				aux_buffer_sql);
		}

		mServidorVioleta->InicializaBuffer(Respuesta,
			TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,
			buffer_sql);
	}
	__finally {
		delete buffer_sql;
	}
}

// ---------------------------------------------------------------------------
// ID_CON_EMPLEADO
void ServidorCatalogos::ConsultaEmpleado(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA EMPLEADO
	AnsiString instruccion;
	AnsiString clave_empleado;
	AnsiString activos, sucursal, condicion_sucursal = " " , condicion_LaborEmpleado, empresa, porApellido;
	AnsiString filtroempleado, puesto, tipo_usuario;
	AnsiString condicion_filtroempleado = " ", condicion_puesto, condicion_tipo_usuario=" ", condicion_empresa=" ";

	clave_empleado = mFg.ExtraeStringDeBuffer(&parametros);
	activos=mFg.ExtraeStringDeBuffer(&parametros);
	sucursal=mFg.ExtraeStringDeBuffer(&parametros);
	condicion_LaborEmpleado=mFg.ExtraeStringDeBuffer(&parametros);
	filtroempleado=mFg.ExtraeStringDeBuffer(&parametros);
	puesto=mFg.ExtraeStringDeBuffer(&parametros);
	tipo_usuario=mFg.ExtraeStringDeBuffer(&parametros);
	empresa=mFg.ExtraeStringDeBuffer(&parametros);
	porApellido = mFg.ExtraeStringDeBuffer(&parametros);

	if(sucursal.Trim() != "" ){
		condicion_sucursal.sprintf("  and em.sucursal = '%s' ", sucursal);
	}

	if(filtroempleado.Trim() != ""){
		condicion_filtroempleado.sprintf(" AND CONCAT(em.nombre,' ',em.appat,' ',em.apmat) LIKE '%%%s%%%'",
		filtroempleado);
	}

	if(puesto.Trim() != ""){
		condicion_puesto.sprintf(" and puesto = '%s'" ,puesto);
	}else{
        condicion_puesto.sprintf(" ");
	}

	if(tipo_usuario.Trim() == "1" || tipo_usuario.Trim() == "0"){
		condicion_tipo_usuario.sprintf(" AND em.esSistema = %s ", tipo_usuario);
	}

	if (empresa.Trim() != "") {
        condicion_empresa.sprintf(" AND em.sucursal IN (SELECT sucursal FROM sucursales WHERE idempresa = %s)", empresa);
	}

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
	AnsiString orden = (porApellido=="1")?" em.appat, em.apmat, em.nombre ":" em.nombre, em.appat, em.apmat ";
	AnsiString nombreMostrar = (porApellido=="1")?" em.appat,' ',em.apmat,' ', em.nombre ":" em.nombre,' ',em.appat,' ',em.apmat ";

	// Obtiene todos los empleados
	instruccion.sprintf("select em.empleado AS Empleado, concat(%s) AS Nombre \
	,em.sucursal, em.activo  from empleados em %s where 1 %s %s %s %s %s %s order by %s " , nombreMostrar,
	condicion_LaborEmpleado, condicion_tipo_usuario, activos, condicion_sucursal, condicion_filtroempleado,
	condicion_puesto, condicion_empresa, orden);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);


	if(clave_empleado.Trim()=="")
	{
		// Cargar datos de el primer empleado de la lista , en caso de no existir clave de empleado
		// Obtiene todos los datos del empleado
		instruccion.sprintf("select df.* from empleados df , \
		( select em.empleado from empleados em %s where 1 %s %s %s %s order by em.nombre, em.appat, em.apmat limit 1) as em \
		WHERE em.empleado = df.empleado",
		condicion_LaborEmpleado, condicion_tipo_usuario, activos, condicion_empresa, condicion_sucursal);

		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),Respuesta->TamBufferResultado);

	}else{
		// Obtiene todos los datos del empleado
		instruccion.sprintf("select em.* from empleados em where em.empleado='%s' %s ",clave_empleado, condicion_tipo_usuario);
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),Respuesta->TamBufferResultado);
	}

	// Obtiene todos los departamentos
	instruccion =
		"select depart AS Departamento, nombre AS Nombre from departamentosrh order by nombre";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los puestos
	instruccion =
		"select puesto AS Puesto, nombre AS Nombre from puestos order by nombre";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los posibles jefes
	instruccion =
		"select empleado AS Empleado, concat(nombre,' ',appat,' ',apmat) AS Nombre from empleados order by nombre, apmat, appat";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

	// Obtiene todos los posibles almacenes
	instruccion = "select almacen, nombre from almacenes order by nombre";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los posibles puntos de venta
	instruccion =
		"select terminal, nombre from terminales where puntoventa=1 order by nombre";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Datos como BODEGUERO
	instruccion.sprintf("select * from bodegueros where empleado='%s'",
		clave_empleado);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Datos como VENDEDOR
	instruccion.sprintf("select * from vendedores where empleado='%s'",
		clave_empleado);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Datos como CAJERO
	instruccion.sprintf("select * from cajeros where empleado='%s'",
		clave_empleado);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Datos como SURTIDOR
	instruccion.sprintf("select * from surtidores where empleado='%s'",
		clave_empleado);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Datos como COBRADOR
	instruccion.sprintf("select * from cobradores where empleado='%s'",
		clave_empleado);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Datos como CHOFER
	instruccion.sprintf("select * from choferes where empleado='%s'",
		clave_empleado);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Datos como COMPRADOR
	instruccion.sprintf("select * from compradores where empleado='%s'",
		clave_empleado);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Datos como MANIOBRISTA
	instruccion.sprintf("select * from maniobristas where empleado='%s'",
		clave_empleado);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Datos como RECEPCIONISTAS
	instruccion.sprintf("select * from recepcionistas where empleado='%s'",
		clave_empleado);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Datos como SOPORTE
	instruccion.sprintf("select * from empleadossoporte where empleado='%s'",
		clave_empleado);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Datos USUARIOS
	instruccion.sprintf("select * from usuarios where empleado='%s'",
		clave_empleado);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Datos CLIENTE x EMPLEADO
	instruccion.sprintf("select c.*, cet.tipoprec as tipoprec2, cet.tipoprecmin as tipoprecmin2, cet.vendedor as vendedor, cet.cobrador as cobrador \
	from clientes c \
	left join clientesemp cet on cet.cliente=c.cliente AND cet.idempresa=%s \
	where c.claveempleado = '%s'",
		FormServidor->ObtieneClaveEmpresa(), clave_empleado);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------

// ALMACENES

// ---------------------------------------------------------------------------
// ID_GRA_ALMACEN
void ServidorCatalogos::GrabaAlmacen(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA ALMACEN
	AnsiString Tabla="almacenes";
	AnsiString CampoClave="almacen";

	// GRABA GENERICO
	char *buffer_sql = new char[1024 * 20];
	char *aux_buffer_sql = buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instrucciones[10];
	AnsiString instruccion;
	int num_instrucciones = 0;
	AnsiString clave;
	AnsiString tarea;
	AnsiString clave_formateada, ventasxvolumen;
	AnsiString almacen;
	char tipo;
	int i;

	try {
		tarea = mFg.ExtraeStringDeBuffer(&parametros);
		// Indicador si se va a agregar o modificar
		clave = mFg.ExtraeStringDeBuffer(&parametros); // Clave

		instruccion = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]=instruccion;
		instruccion = "START TRANSACTION";
		instrucciones[num_instrucciones++]=instruccion;

		datos.AsignaTabla(Tabla.c_str());
		parametros += datos.InsCamposDesdeBuffer(parametros);

		if (tarea == "A") {
			instrucciones[num_instrucciones++] = datos.GenerarSqlInsert();

			// En altas Inserta en existencias actuales un registro por cada presentación con cero en todas las columnas.
			almacen=datos.ObtieneValorCampo("almacen");
			instruccion.sprintf("INSERT INTO existenciasactuales "
				"(producto, present,almacen, cantidad, ventas, devventas, compras, devcompras, entradas, salidas, cantinicial) "
				"SELECT producto, present, '%s' AS almacen, 0 AS cantidad, 0 AS ventas, 0 AS devventas, 0 AS compras, 0 AS devcompras, 0 AS entradas, 0 AS salidas, 0 AS cantinicial "
				"FROM articulos "
				"GROUP BY producto,present ", almacen);
			instrucciones[num_instrucciones++]=instruccion;

		} else {
			tipo = datos.ObtieneTipoCampo(CampoClave);
			if (tipo == 'I' || tipo == 'N' || tipo == 'F')
				clave_formateada = clave;
			else
				clave_formateada = "'" + clave + "'";
			instrucciones[num_instrucciones++] = datos.GenerarSqlUpdate
				(CampoClave + "=" + clave_formateada);
		}

		instruccion = "COMMIT";
		instrucciones[num_instrucciones++]=instruccion;

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

// ---------------------------------------------------------------------------
// ID_BAJ_ALMACEN
void ServidorCatalogos::BajaAlmacen(RespuestaServidor *Respuesta, MYSQL *MySQL,
	char *parametros) {
	// BAJA ALMACEN
	BajaGenerico(Respuesta, MySQL, parametros, "almacenes", "almacen");
}

// ---------------------------------------------------------------------------
// ID_CON_ALMACEN
void ServidorCatalogos::ConsultaAlmacen(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA ALMACEN
	AnsiString instruccion;
	AnsiString clave_almacen;

	clave_almacen = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos del almacen
	instruccion.sprintf("select * from almacenes where almacen='%s'",
		clave_almacen);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los almacenes
	instruccion =
		"SELECT a.almacen AS Almacen, a.nombre AS Nombre, s.sucursal AS Sucursal, a.permitecompras AS PermiteCompras, a.activo AS Activo  \
		from almacenes a  \
		INNER JOIN secciones s ON a.seccion = s.seccion  \
		order BY a.nombre";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todas las secciones
	instruccion =
		"select seccion AS Seccion, nombre AS Nombre from secciones order by nombre";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los posibles encargados (todos los empleados)
	instruccion =
		"select empleado AS Empleado, concat(nombre,' ',appat,' ',apmat) AS Nombre from empleados order by nombre, apmat, appat";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todas las terminales
	instruccion =
		"select terminal AS Terminal, nombre AS Nombre from terminales";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

}
// ---------------------------------------------------------------------------

// PROVEEDORES

// ---------------------------------------------------------------------------
// ID_GRA_PROVEEDOR
void ServidorCatalogos::GrabaProveedor(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA PROVEEDOR
	char *buffer_sql = new char[1024 * 5];
	char *aux_buffer_sql = buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString tarea_proveedor, clave_proveedor, plazoStr, usuario;
	int plazo;
	int num_telefonos, i, x;
	int num_instrucciones = 0;
	AnsiString instruccion, instrucciones[100];
	AnsiString	idcondicion,proveedor,fecha_pacto,fecha_vigencia,empleado,representante,comentarios;

	try {
		tarea_proveedor = mFg.ExtraeStringDeBuffer(&parametros);
		// Indicador si se va a agregar o modificar
		clave_proveedor = mFg.ExtraeStringDeBuffer(&parametros);
		// Clave del proveedor
		plazo =	StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
		//plazo del proveedor
		usuario = mFg.ExtraeStringDeBuffer(&parametros);
		//usuario de alta o de modificación

		datos.AsignaTabla("proveedores");
		parametros += datos.InsCamposDesdeBuffer(parametros);
		/** */ datos.AsignaValorCampo("proveedor", "@folio", 1);

		instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++] = "START TRANSACTION";
		if (tarea_proveedor == "A") {
			/* */ instruccion.sprintf(
				"select @folioaux:=valor from foliosemp where folio='PROV' AND sucursal = '%s' %s ",
				FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
			/* */ instrucciones[num_instrucciones++] = instruccion;
			/* */ instruccion.sprintf("set @foliosig=@folioaux+1");
			/* */ instrucciones[num_instrucciones++] = instruccion;
			instruccion.sprintf("set @folioaux=cast(@folioaux as char)");
			instrucciones[num_instrucciones++] = instruccion;
			instruccion.sprintf
				("set @folio=concat('%s', lpad(@folioaux,6,'0'))", FormServidor->ObtieneClaveSucursal());
			instrucciones[num_instrucciones++] = instruccion;
			/* */ instruccion.sprintf(
				"update foliosemp set valor=@foliosig where folio='PROV' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
			/* */ instrucciones[num_instrucciones++] = instruccion;
			//se agrega fecha de Alta del Proveedor
			datos.InsCampo("fechaalta", mFg.DateToAnsiString(Today()));
			datos.InsCampo("usualta", usuario);  //usuario alta
			instrucciones[num_instrucciones++] = datos.GenerarSqlInsert();
		}
		else {

			/* */ instruccion.sprintf("set @folio='%s'", clave_proveedor);
			/* */ instrucciones[num_instrucciones++] = instruccion;

			//Fecha de modificación
            datos.InsCampo("fechamodi", mFg.DateToAnsiString(Today()));
			datos.InsCampo("usumodi", usuario);  //usuario modificación
			instrucciones[num_instrucciones++] = datos.GenerarSqlUpdate
				("proveedor='" + clave_proveedor + "'");

			BufferRespuestas* resp_plazo=NULL;
			int plazobd;
			try {
			instruccion.sprintf("SELECT plazo FROM proveedores WHERE proveedor='%s'",clave_proveedor);
			if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_plazo))
				plazobd = StrToInt(resp_plazo->ObtieneDato("plazo"));
			}
			 __finally {
						if (resp_plazo!=NULL) delete resp_plazo;
			}

			if(plazobd != plazo){
				/*FUNCION PARA CALCULAR LOS SALDOS Y SABER QUE MOVIMIENTOS APLICAR*/
				// Crea una tabla donde se van a poner los saldos de las compras del proveedor
				instruccion="create temporary table tmpcomprassaldos (compra char(11), saldo decimal(16,2), \
				PRIMARY KEY (compra)) Engine = InnoDB";
				if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
					throw (Exception("Error en query EjecutaSelectSqlNulo"));

				// Calcula los saldos de las compras del proveedor (SALDOB (saldo virtual))
				instruccion.sprintf("insert into tmpcomprassaldos (compra, saldo) \
				select c.referencia as compra, sum(t.valor) as saldo \
				from compras c, transxpag t \
				where c.proveedor='%s' and \
				t.referencia=c.referencia and t.cancelada=0 and c.cancelado=0 \
				group by c.referencia",
				clave_proveedor);
				if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
					throw (Exception("Error en query EjecutaSelectSqlNulo"));

				plazoStr = IntToStr(plazo);
				instruccion.sprintf("UPDATE tmpcomprassaldos au INNER JOIN compras c ON c.referencia = au.compra \
				AND au.saldo > 0 SET c.plazo = %s, c.periodic = %s, c.fechainic = ADDDATE(c.fechacom, INTERVAL %s DAY), \
				c.fechavenc = ADDDATE(c.fechacom, INTERVAL %s DAY) ", plazoStr , plazoStr , plazoStr, plazoStr);
				instrucciones[num_instrucciones++] = instruccion;
			}

		}

		if (mFg.ExtraeStringDeBuffer(&parametros) == "1") {
			// Para ver si hay modificaciones en los teléfonos
			if (tarea_proveedor == "M") {
				instruccion.sprintf(
					"delete from telefonosproveedores where proveedor='%s'",
					clave_proveedor);
				instrucciones[num_instrucciones++] = instruccion;
			}
			num_telefonos = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
			// Obtiene el número de teléfonos
			for (i = 0; i < num_telefonos; i++) {
				datos.AsignaTabla("telefonosproveedores");
				parametros += datos.InsCamposDesdeBuffer(parametros);
				datos.ReemplazaCampo("proveedor", "@folio", 1);
				instrucciones[num_instrucciones++] = datos.GenerarSqlInsert();
			}
		}

		//ingresa los datos de condiciones comerciales
		if(mFg.ExtraeStringDeBuffer(&parametros) == "1"){
			if (tarea_proveedor == "A") {
				datos.AsignaTabla("condicionescomerprov");
				parametros += datos.InsCamposDesdeBuffer(parametros);
				datos.ReemplazaCampo("proveedor", "@folio", 1);
				instrucciones[num_instrucciones++] = datos.GenerarSqlInsert();
			}else{
				clave_proveedor;
				idcondicion = mFg.ExtraeStringDeBuffer(&parametros);
				fecha_pacto = mFg.ExtraeStringDeBuffer(&parametros);
				fecha_vigencia = mFg.ExtraeStringDeBuffer(&parametros);
				empleado  = mFg.ExtraeStringDeBuffer(&parametros);
				representante = mFg.ExtraeStringDeBuffer(&parametros);
				comentarios = mFg.ExtraeStringDeBuffer(&parametros);
				if(mFg.EsCero(idcondicion)){
					//insert
					instruccion.sprintf("SELECT @folio:=MAX(idcondicion)+1  FROM condicionescomerprov");
					instrucciones[num_instrucciones++] = instruccion;

					instruccion.sprintf("insert into condicionescomerprov \
					(proveedor,fecha_alta,fecha_update,fecha_pacto,fecha_vigencia,empleado,representante,condicion_comer)\
					 values ('%s', CURDATE(),CURDATE(),'%s','%s','%s','%s','%s')",
					clave_proveedor,mFg.StrToMySqlDate(fecha_pacto) ,mFg.StrToMySqlDate(fecha_vigencia),empleado,representante, comentarios);
					instrucciones[num_instrucciones++] = instruccion;
				}else{
					//update
					instruccion.sprintf(
					"update condicionescomerprov set fecha_update=CURDATE(),fecha_pacto='%s',fecha_vigencia='%s',\
					empleado='%s',representante='%s',condicion_comer='%s' where idcondicion=%s",
					mFg.StrToMySqlDate(fecha_pacto) ,mFg.StrToMySqlDate(fecha_vigencia),empleado,representante, comentarios, idcondicion);
					instrucciones[num_instrucciones++] = instruccion;
				}
            }
		}

		if(tarea_proveedor == "A"){
			datos.AsignaTabla("cuentas_proveedor");
			parametros += datos.InsCamposDesdeBuffer(parametros);
			datos.ReemplazaCampo("proveedor", "@folio", 1);
			instrucciones[num_instrucciones++] = datos.GenerarSqlInsert();
		}else{
			AnsiString cuenta_iva = mFg.ExtraeStringDeBuffer(&parametros);
			AnsiString cuenta_isr = mFg.ExtraeStringDeBuffer(&parametros);

			if(cuenta_iva.Trim() != ""){
				instruccion.sprintf( "update cuentas_proveedor set cuenta_retencion_iva='%s' \
				where proveedor='%s' ", cuenta_iva, clave_proveedor);
				instrucciones[num_instrucciones++] = instruccion;
			}else{
				instruccion.sprintf( "update cuentas_proveedor set cuenta_retencion_iva=NULL \
				where proveedor='%s' ", clave_proveedor);
				instrucciones[num_instrucciones++] = instruccion;
			}

			if(cuenta_isr.Trim() != ""){
				instruccion.sprintf( "update cuentas_proveedor set cuenta_retencion_isr='%s' \
				where proveedor='%s' ", cuenta_isr, clave_proveedor);
				instrucciones[num_instrucciones++] = instruccion;
			}else{
				instruccion.sprintf( "update cuentas_proveedor set cuenta_retencion_isr=NULL \
				where proveedor='%s' ", clave_proveedor);
				instrucciones[num_instrucciones++] = instruccion;
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
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,
				buffer_sql)) {
			instruccion.sprintf("select @folio as folio");
			mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,
				instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	}
	__finally {
		delete buffer_sql;
	}
}

// ---------------------------------------------------------------------------
// ID_BAJ_PROVEEDOR
void ServidorCatalogos::BajaProveedor(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// BAJA PROVEEDOR
	char *buffer_sql = new char[1024 * 6];
	char *aux_buffer_sql = buffer_sql;
	AnsiString instruccion;
	int num_instrucciones = 6;
	AnsiString clave_proveedor;

	try {
		clave_proveedor = mFg.ExtraeStringDeBuffer(&parametros);
		aux_buffer_sql = mFg.AgregaStringABuffer
			(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);

		instruccion = "SET AUTOCOMMIT=0";
		aux_buffer_sql = mFg.AgregaStringABuffer(instruccion, aux_buffer_sql);

		instruccion = "START TRANSACTION";
		aux_buffer_sql = mFg.AgregaStringABuffer(instruccion, aux_buffer_sql);

		instruccion.sprintf(
			"delete from telefonosproveedores where proveedor='%s'",
			clave_proveedor);
		aux_buffer_sql = mFg.AgregaStringABuffer(instruccion, aux_buffer_sql);

		//cambiar el valor de cancelado de las condiciones comerciales
		instruccion.sprintf("delete from condicionescomerprov where proveedor='%s'",
			clave_proveedor);
		aux_buffer_sql = mFg.AgregaStringABuffer(instruccion, aux_buffer_sql);

		instruccion.sprintf("delete from proveedores where proveedor='%s'",
		clave_proveedor);
		aux_buffer_sql = mFg.AgregaStringABuffer(instruccion, aux_buffer_sql);

		instruccion = "COMMIT";
		mFg.AgregaStringABuffer(instruccion, aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta,
			TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,
			buffer_sql);
	}
	__finally {
		delete buffer_sql;
	}
}

// ---------------------------------------------------------------------------
// ID_CON_PROVEEDOR
void ServidorCatalogos::ConsultaProveedor(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA PROVEEDOR
	AnsiString instruccion;
	AnsiString clave_proveedor;

	clave_proveedor = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos del proveedor
	instruccion.sprintf("select p.*, imp.porcentaje AS isrret from proveedores p left join impuestos imp ON imp.impuesto = p.impuestoret where p.proveedor='%s'",
		clave_proveedor);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los telefonos del proveedor
	instruccion.sprintf(
		"select tipo, lada, telefono,extencionTel from telefonosproveedores where proveedor='%s'"
		, clave_proveedor);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los bancos
	instruccion = "select banco, nombre from bancos order by nombre, banco";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

	// Obtiene las cuentas de retencion de gastos
	instruccion.sprintf("SELECT cp.cuenta_retencion_iva, cp.cuenta_retencion_isr \
	 FROM cuentas_proveedor cp WHERE cp.proveedor='%s' ", clave_proveedor);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

	//obtiene todos las condiciones comerciales del provedor order by activo, idcondicion DESC,ccp.fecha_alta
	instruccion.sprintf("SELECT ccp.idcondicion,ccp.proveedor,ccp.fecha_pacto,ccp.fecha_vigencia,  \
				CONCAT(em.nombre, ' ', em.appat, ' ', em.apmat) AS empleado,  \
				ccp.representante, ccp.condicion_comer , IF(ccp.fecha_vigencia < CURDATE(),1, 0)AS activo,\
				ccp.cancelado,ccp.cumplido\
				FROM condicionescomerprov ccp    \
				LEFT JOIN empleados em  ON ccp.empleado=em.empleado  \
				where proveedor='%s' order by ccp.fecha_pacto DESC, idcondicion DESC ",clave_proveedor);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),Respuesta->TamBufferResultado);

}
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

// SUCURSALES

// ---------------------------------------------------------------------------
// ID_GRA_SUCURSAL
void ServidorCatalogos::GrabaSucursal(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA SUCURSAL
	char *buffer_sql = new char[1024 * 16];
	char *aux_buffer_sql = buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString tarea_articulo, tarea_producto;  //tarea_producto-> alta o modificacion
	AnsiString tarea_presentacion, tarea_precios, latitud, longitud;
	AnsiString clave_articulo, clave_producto, clave_present, clave_multiplo;
	AnsiString clave_presentacion_prod, clave_presentacion_pres;
	AnsiString usuario, usuario_new, terminal, terminal_new;//, almacen;
	int i, x;
	int num_instrucciones = 0;
	int num_precios;
	AnsiString instruccion, instrucciones[100];
	bool cambiar_utilidad_multiplos = false;
	AnsiString modoutil;
	AnsiString costobase;
	AnsiString porcutil, tipoprec, producto, present, utilesp;
	AnsiString precioAsig;
	AnsiString comisionant,costobaseant,activo,activoant, comision;
	AnsiString select;
	AnsiString producto_ea, present_ea;

	AnsiString sucursal, numid, nombre, calle, colonia, localidad, cp, email, telefono1, telefono2, telefono3, telefono4, activa, venxvol;
	AnsiString seccion, almacen, fechaalta, horaalta, sucursal_act;
	AnsiString salidaotrasuc, diasentregas, vtasecom, defaultecom, pickup, empresa, sucursal_empresa, diasdistribucion;

	AnsiString paramcambioprec;
	BufferRespuestas* resp_paramcambioprec=NULL;


	try {
		instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++] = "START TRANSACTION";

		modoutil= mFg.ExtraeStringDeBuffer(&parametros);
		sucursal= mFg.ExtraeStringDeBuffer(&parametros);
		numid= mFg.ExtraeStringDeBuffer(&parametros);
		nombre= mFg.ExtraeStringDeBuffer(&parametros);
		calle= mFg.ExtraeStringDeBuffer(&parametros);
		colonia= mFg.ExtraeStringDeBuffer(&parametros);
		localidad= mFg.ExtraeStringDeBuffer(&parametros);
		cp= mFg.ExtraeStringDeBuffer(&parametros);
		email= mFg.ExtraeStringDeBuffer(&parametros);
		telefono1= mFg.ExtraeStringDeBuffer(&parametros);
		telefono2= mFg.ExtraeStringDeBuffer(&parametros);
		telefono3= mFg.ExtraeStringDeBuffer(&parametros);
		telefono4= mFg.ExtraeStringDeBuffer(&parametros);
		activa= mFg.ExtraeStringDeBuffer(&parametros);
		venxvol= mFg.ExtraeStringDeBuffer(&parametros);
		salidaotrasuc=mFg.ExtraeStringDeBuffer(&parametros);
		usuario= mFg.ExtraeStringDeBuffer(&parametros);
		terminal= mFg.ExtraeStringDeBuffer(&parametros);
		sucursal_act= mFg.ExtraeStringDeBuffer(&parametros);
		latitud = mFg.ExtraeStringDeBuffer(&parametros);
		longitud = mFg.ExtraeStringDeBuffer(&parametros);
		diasentregas = mFg.ExtraeStringDeBuffer(&parametros);
		vtasecom = mFg.ExtraeStringDeBuffer(&parametros);
		defaultecom = mFg.ExtraeStringDeBuffer(&parametros);
		pickup = mFg.ExtraeStringDeBuffer(&parametros);
		empresa = mFg.ExtraeStringDeBuffer(&parametros);
		diasdistribucion = mFg.ExtraeStringDeBuffer(&parametros);

		if (modoutil=="A") {
			//parametros para los nuevos registros
			seccion= sucursal+"S1";
			almacen= "AL"+sucursal;
			x=usuario.Length();
			if (x>4)
				usuario_new=sucursal + usuario.SubString(x-3,4);
			else
				usuario_new=sucursal + usuario;
			terminal_new=sucursal+"T001";
			fechaalta= mFg.DateToMySqlDate(Today());
			horaalta= mFg.TimeToMySqlTime(Time());

			sucursal_empresa = FormServidor->ObtieneClaveSucursal();

			if (latitud.Length()>3 && longitud.Length()>3 ) {
				//inserta la nueva Sucursal
				instruccion.sprintf("insert into sucursales (sucursal, numid, nombre, calle, colonia, localidad, cp, ubicaciongis, \
				email, telefono1, telefono2, telefono3, telefono4, activa, venxvol,salidaotrasucursal, vtasecom, defaultecom, pickup, idempresa)\
				values ('%s', %s, '%s', '%s', '%s', '%s', '%s', POINT(%s,%s), '%s', '%s', '%s', '%s', '%s', %s, %s, '%s', %s, %s, %s, %s)",
				sucursal, numid, nombre, calle, colonia, localidad, cp,latitud,longitud , email, telefono1, telefono2, telefono3,
				 telefono4, activa, venxvol,salidaotrasuc, vtasecom, defaultecom, pickup, empresa );
				instrucciones[num_instrucciones++]=instruccion;
			}else{
				//inserta la nueva Sucursal
				instruccion.sprintf("insert into sucursales (sucursal, numid, nombre, calle, colonia, localidad, cp, \
				email, telefono1, telefono2, telefono3, telefono4, activa, venxvol,salidaotrasucursal, vtasecom, defaultecom, pickup, idempresa)\
				values ('%s', %s, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', %s, %s, '%s', %s, %s, %s, %s)",
				sucursal, numid, nombre, calle, colonia, localidad, cp,latitud,longitud , email, telefono1, telefono2, telefono3,
				 telefono4, activa, venxvol,salidaotrasuc, vtasecom, defaultecom, pickup, empresa );
				instrucciones[num_instrucciones++]=instruccion;
			}

			instruccion.sprintf("set @sucursal='%s'",	sucursal);
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @seccion='%s'",	seccion);
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @almacen='%s'",	almacen);
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @usuario='%s'",	usuario_new);
			instrucciones[num_instrucciones++]=instruccion;

			//inserta la nueva sección relacionada a la sucursal
			instruccion.sprintf("insert into secciones (seccion, nombre, sucursal, almaunid, almamult, activa)\
			values ('%s', '%s', '%s', '%s', '%s', '%s')",
			seccion, nombre,sucursal, almacen, almacen, activa );
			instrucciones[num_instrucciones++]=instruccion;

			//inserta el nuevo almacen relacionado a la sucursal y la sección
			instruccion.sprintf("insert into almacenes (almacen, nombre, seccion,  termimpsur, \
			calle, colonia, localidad, cp, email, telefono1, telefono2, telefono3, telefono4, activo)\
			values ('%s', 'ALMACEN %s', '%s',  '', '%s', '%s', '%s', \
			'%s', '%s', '%s', '%s', '%s', '%s', %s)",
			almacen, nombre, seccion, calle, colonia, localidad, cp, email, telefono1, telefono2, telefono3, telefono4, activa);
			instrucciones[num_instrucciones++]=instruccion;

			//insertar datos para tabla de articulos por seccion  'articuloxseccion'
			instruccion.sprintf("INSERT INTO articuloxseccion(articulo, seccion, almacen)\
			select articulo, '%s' as seccion, '%s' as almacen from articulos ",
			seccion, almacen);
			instrucciones[num_instrucciones++]=instruccion;

			//agregando registro para bancos en tabla de bancosconfigsuc
			instruccion.sprintf("insert into bancosconfigsuc(sucursal,numid,cuentapagcli,cuentapagprov)\
			select '%s' as sucursal, %s as numid,cuentapagcli,cuentapagprov from bancosconfigsuc order by numid asc limit 1 ",
			sucursal, numid);
			instrucciones[num_instrucciones++]=instruccion;

			//se inserta el empleado para las relaciones con usuarios
			instruccion.sprintf("insert into empleados(empleado,nombre,appat,apmat,calle,colonia,cp,localidad,telefono1,\
			telefono2,fechaalta,fechabaja,puesto,depart,jefe,sucursal,activo,esSistema)\
			select @usuario,nombre,appat,apmat,calle,colonia,cp,localidad,telefono1,telefono2,fechaalta,fechabaja,puesto,\
			depart,jefe,@sucursal,activo,1 from empleados where empleado='%s'",
			usuario);
			instrucciones[num_instrucciones++]=instruccion;

			//se inserta el usuario para las relaciones con empleado del anterior que se dio de alta
			instruccion.sprintf("insert into usuarios(empleado,password,activo,fechaalta,fechabaja)\
			select @usuario,password,activo,fechaalta,fechabaja from usuarios where empleado='%s'",
			usuario);
			instrucciones[num_instrucciones++]=instruccion;

			//inserta el nuevo usuario
			instruccion = "INSERT IGNORE INTO usuariosucursal (usuario, sucursal) VALUES (@usuario, @sucursal)";
			instrucciones[num_instrucciones++]=instruccion;

			//se inserta una Terminal para la sucursal
			instruccion.sprintf("INSERT INTO terminales(terminal,nombre,usuario,depart,seccion,almadefa,uso,ubicacion,puntoventa,tipoimprfo,asigfolvta,destimprvta,\
			serievta,folvta,imprautovta,tipoimprti,destimprncred,asigfolncred,seriencred,folncred,imprautoncred,destimprncar,asigfolncar,seriencar,\
			folncar,foltickets,validusuvta,imprautoncar,imprticksur,esmovil,anchofolvta,anchofolncar,anchofolncred,imprecpagcli,nomimpresoracfd,\
			rotuloventc1,rotuloventc2,rotuloventc3,snselloventc1,snselloventc2,snselloventc3,impcfdventc1,impcfdventc2,impcfdventc3,rotuloncarc1,\
			rotuloncarc2,rotuloncarc3,snselloncarc1,snselloncarc2,snselloncarc3,impcfdncarc1,impcfdncarc2,impcfdncarc3,rotuloncrec1,rotuloncrec2,\
			rotuloncrec3,snselloncrec1,snselloncrec2,snselloncrec3,impcfdncrec1,impcfdncrec2,impcfdncrec3,solocredvent1,solocredvent2,solocredvent3,\
			nomimpresoracfd2,nomimpresoracfd3,nomimpresorancar,nomimpresorancar2,nomimpresorancar3,nomimpresorancre,nomimpresorancre2,\
			nomimpresorancre3,impresoratickets,tipobascula,puertobascula,nomimpresorapago1,nomimpresorapago2,nomimpresorapago3,rotulopago1,\
			rotulopago2,rotulopago3,impcfdpago1,impcfdpago2,impcfdpago3)\
			select '%s','%s TERMINAL',@usuario,depart,'%s','%s',uso,ubicacion,puntoventa,tipoimprfo,asigfolvta,'%s',\
			serievta,folvta,imprautovta,tipoimprti,'%s',asigfolncred,seriencred,folncred,imprautoncred,'%s',asigfolncar,seriencar,\
			folncar,1,validusuvta,imprautoncar,imprticksur,esmovil,anchofolvta,anchofolncar,anchofolncred,imprecpagcli,nomimpresoracfd,\
			rotuloventc1,rotuloventc2,rotuloventc3,snselloventc1,snselloventc2,snselloventc3,impcfdventc1,impcfdventc2,impcfdventc3,rotuloncarc1,\
			rotuloncarc2,rotuloncarc3,snselloncarc1,snselloncarc2,snselloncarc3,impcfdncarc1,impcfdncarc2,impcfdncarc3,rotuloncrec1,rotuloncrec2, \
			rotuloncrec3,snselloncrec1,snselloncrec2,snselloncrec3,impcfdncrec1,impcfdncrec2,impcfdncrec3,solocredvent1,solocredvent2,solocredvent3,\
			nomimpresoracfd2,nomimpresoracfd3,nomimpresorancar,nomimpresorancar2,nomimpresorancar3,nomimpresorancre,nomimpresorancre2,\
			nomimpresorancre3,impresoratickets,tipobascula,puertobascula,nomimpresorapago1,nomimpresorapago2,nomimpresorapago3,rotulopago1,\
			rotulopago2,rotulopago3,impcfdpago1,impcfdpago2,impcfdpago3\
			from terminales where terminal='%s'",
			terminal_new, sucursal, seccion, almacen, terminal_new, terminal_new, terminal_new, terminal);
			instrucciones[num_instrucciones++]=instruccion;

			//actualiza la terminal que va relacionada al almacen,  como terminal inicial
			instruccion.sprintf("update almacenes set termimpsur='%s' \
			where almacen='%s' ",
			terminal_new, almacen );
			instrucciones[num_instrucciones++]=instruccion;

			//se insertan las existencias actuales, de inicio en ceros para poder actualizar al insertar las primeras compras por sucursal
			instruccion.sprintf(" insert into existenciasactuales (producto, present, almacen, cantidad, ventas, \
			devventas, compras, devcompras, entradas, salidas) \
			select DISTINCT producto, present,'%s', 0,0,0,0,0,0,0 \
			from existenciasactuales",almacen);
			instrucciones[num_instrucciones++]=instruccion;

			//Se insertan campos para Folios de Facturas y DoctosFiscales
			instruccion.sprintf("INSERT INTO folioscfd(sucursal,folioinicial,foliofinal,serie,numaprobacion,fechaaprobacion,tipocomprobante,fechaalta,\
			fechamodi,horaalta,horamodi,usualta,usumodi,activa,folioactual)\
			(select @sucursal,1,9999999,CONCAT(@sucursal,'V'),1,'%s','VENT','%s',\
			'%s','%s','%s',@usuario,@usuario,1,1)\
			UNION\
			(select @sucursal,1,9999999,CONCAT(@sucursal,'C'),1,'%s','NCAR','%s',\
			'%s','%s','%s',@usuario,@usuario,1,1)\
			UNION\
			(select @sucursal,1,9999999,CONCAT(@sucursal,'N'),1,'%s','NCRE','%s',\
			'%s','%s','%s',@usuario,@usuario,1,1)\
			UNION\
			(select @sucursal,1,9999999,CONCAT(@sucursal,'P'),1,'%s','PAGO','%s',\
			'%s','%s','%s',@usuario,@usuario,1,1) ",
			fechaalta, fechaalta,fechaalta,horaalta,horaalta,
			fechaalta, fechaalta,fechaalta,horaalta,horaalta,
			fechaalta, fechaalta,fechaalta,horaalta,horaalta,
			fechaalta, fechaalta,fechaalta,horaalta,horaalta);
			instrucciones[num_instrucciones++]=instruccion;

			// se inserta registro de tabla sucursalesreplicacion
			instruccion.sprintf("insert into sucursalesreplicacion(sucursal,nombre,hostservidor,puertoservidor,puertonameserv,nombreservicio,\
			claveterminal,esprincipal)\
			select @sucursal,'%s','192.168.0.0',puertoservidor,puertonameserv,CONCAT('servidor',@sucursal),'%s',esprincipal\
			from sucursalesreplicacion  order by idSucreplica desc limit 1",
			nombre,terminal_new);
			instrucciones[num_instrucciones++]=instruccion;

			//insertar configuración para Pólizas
			instruccion.sprintf("insert into plantillaspoliz(tipocomprobante,tipoplantilla,tipopartida,orden,numcuenta,tipoafectacion,expresion, \
			sucursal,parametros,clasif1,clasif2,clasif3,termino,impuesto,acredito,clasifcont,clasifcont2,tipoimpu,negimpuesto,impuesto2,tipoimpu2,\
			negimpuesto2,parterel,fechainivent,fechafinvent,sucdetalle,formaspago,tiporfc, idnumcuenta, agrupabanco, segmentodefault)\
			SELECT tipocomprobante,tipoplantilla,tipopartida,orden,numcuenta,tipoafectacion,expresion,@sucursal,parametros,clasif1,\
			clasif2,clasif3,termino,impuesto,acredito,clasifcont,clasifcont2,tipoimpu,negimpuesto,impuesto2,tipoimpu2,negimpuesto2,parterel,\
			fechainivent,fechafinvent,@sucursal,formaspago,tiporfc, idnumcuenta, agrupabanco, segmentodefault\
			FROM plantillaspoliz WHERE sucursal='%s'",
			sucursal_act);
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("INSERT INTO plantillasxsuc(sucursal,tipocomprobante,tipoplantilla,idnumcuenta)\
			SELECT @sucursal,tipocomprobante,tipoplantilla,idnumcuenta FROM plantillasxsuc WHERE sucursal='%s'",
			sucursal_act);
			instrucciones[num_instrucciones++]=instruccion;

			//se asignan los privilegios por usuarios
			instruccion.sprintf("INSERT INTO asignacionprivilegios(usuario,objeto,privilegio) \
			SELECT @usuario,objeto,privilegio FROM asignacionprivilegios WHERE usuario='%s'",
			usuario);
			instrucciones[num_instrucciones++]=instruccion;

            //se obtienen datos principales de primer sucursal para parametroscfd
			instruccion.sprintf(
				"SELECT @parametro:= parametro+1 FROM parametroscfd ORDER BY parametro desc LIMIT 1");
			instrucciones[num_instrucciones++] = instruccion;

			instruccion.sprintf(
				"SELECT @segmento:= segmento+1 FROM parametroscfd ORDER BY segmento desc LIMIT 1");
			instrucciones[num_instrucciones++] = instruccion;

			//se agrega registro en tabla de parametroscfd con los datos de la Sucursal actual
			instruccion.sprintf("insert into parametroscfd (parametro,sucursal,certificado,llaveprivada,passcert,numseriecert,requerimiento,nombrecert, \
				nombrellavepriv,nombrereq,fechaalta,fechamodi,horaalta, \
				horamodi,usualta,usumodi,rfcemisor,nombreemisor,calleemisor,numextemisor,numintemisor, \
				coloniaemisor,municipioemisor,localidademisor,estadoemisor,referenemisor,cpemisor, \
				incluirexp,calleexp,numextexp,numintexp,coloniaexp,municipioexp,localidadexp,estadoexp, \
				cpexp,referenexp,servidor,usuario,clave,empresa,telefonos,telefonosexp, \
				email,emailexp,pagweb,pagwebexp,usuariocontpaq,instancia,imagen1,imagen2,imagen3, \
				nombreimg1,nombreimg2,nombreimg3,fechaexpicert,fechavenccert,poltipovent,poltiponcarcli, \
				poltiponcrecli,poltipocomp,poltiponcarprov,poltiponcreprov,poltipocobr,poltipopago, \
				nomenviocfd,emailenviocfd,htmlemail,hostsmtp,portsmtp,usersmtp,passsmtp,envioauto, \
				segmento,regimenfiscal,urledicom,usuarioedicom,passwordedicom,nomcertedicom,certedicom, \
				passcertedicom,urlwfactura,urlcancwfactura,usuariowfactura,timbradoprueba, \
				pacseleccionado,retimbrarauto,imprdocspend,htmlemailpedidos,urlcomerciodigital,urlcanccomerciodigital,\
				usuariocomerciodigital,passcomerciodigital, pfxnombre_efirma, pfx_efirma,   \
				pem_efirma, htmlemailprepagos, imagencotped, nombreimgcotped )  \
				SELECT @parametro,@sucursal,certificado,llaveprivada,passcert,numseriecert,requerimiento, \
				nombrecert,nombrellavepriv,nombrereq,fechaalta,fechamodi,horaalta, \
				horamodi,usualta,usumodi,rfcemisor,nombreemisor,calleemisor,numextemisor,numintemisor, \
				coloniaemisor,municipioemisor,localidademisor,estadoemisor,referenemisor,cpemisor, \
				incluirexp,'%s','','','%s','%s','%s', \
				estadoexp,'%s',referenexp,servidor,usuario,clave,empresa,telefonos,telefonosexp, \
				email,emailexp,pagweb,pagwebexp,usuariocontpaq,instancia,imagen1,imagen2,imagen3,   \
				nombreimg1,nombreimg2,nombreimg3,fechaexpicert,fechavenccert,poltipovent,poltiponcarcli, \
				poltiponcrecli,poltipocomp,poltiponcarprov,poltiponcreprov,poltipocobr,poltipopago, \
				nomenviocfd,emailenviocfd,htmlemail,hostsmtp,portsmtp,usersmtp,passsmtp,envioauto, \
				@segmento,regimenfiscal,urledicom,usuarioedicom,passwordedicom,nomcertedicom,certedicom, \
				passcertedicom,urlwfactura,urlcancwfactura,usuariowfactura,timbradoprueba, \
				pacseleccionado,retimbrarauto,imprdocspend, htmlemailpedidos,urlcomerciodigital,\
				urlcanccomerciodigital,usuariocomerciodigital,passcomerciodigital,pfxnombre_efirma, pfx_efirma,   \
				pem_efirma, htmlemailprepagos, imagencotped, nombreimgcotped \
				FROM parametroscfd WHERE sucursal='%s'",
				calle, colonia, localidad, localidad, cp, sucursal_act );
			instrucciones[num_instrucciones++]=instruccion;

			//se agrega registro en tabla de articulosped con los datos de la Sucursal actual
			instruccion.sprintf("INSERT INTO articulosped (sucursal, producto, present, proveedor, claveproductoproveedor, \
				duracionreorden, duracionmax, descontinuado, redondeocaja, multiplopedir) \
				SELECT @sucursal, producto, present, proveedor, claveproductoproveedor, \
				duracionreorden, duracionmax, descontinuado, redondeocaja, multiplopedir \
				FROM articulosped WHERE sucursal='%s'",
				sucursal_act);
			instrucciones[num_instrucciones++]=instruccion;


			//se agrega registro en tabla de articulosxsuc con los datos de la Sucursal actual
			instruccion.sprintf("INSERT INTO articulosxsuc (sucursal, producto, 	present, fechamodi,	horamodi, usumodi) \
				SELECT @sucursal, producto, present, '%s', '%s', '%s' FROM articulosxsuc WHERE sucursal='%s'",
				fechaalta, horaalta, usuario_new, sucursal_act);
			instrucciones[num_instrucciones++]=instruccion;

            instruccion.sprintf("INSERT INTO parametrosemp (sucursal, parametro, descripcion, valor) \
								SELECT '%s' AS sucursal, p.parametro,  p.descripcion, p.valor  \
								FROM parametrosemp p WHERE p.sucursal = '%s' ", sucursal, sucursal_empresa);
			instrucciones[num_instrucciones++]=instruccion;

            instruccion.sprintf("UPDATE parametrosemp SET valor = '%s'  \
								WHERE sucursal = '%s' AND parametro = 'SUCURSALCLI' ",sucursal, sucursal);
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("INSERT INTO estadosistemaemp (sucursal, estado, descripcion, valor) \
								SELECT '%s' AS sucursal, e.estado,  e.descripcion, e.valor      \
								FROM estadosistemaemp e WHERE e.sucursal = '%s' ", sucursal, sucursal_empresa);
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("INSERT INTO foliosemp (sucursal, folio, descripcion, valor)  \
								SELECT '%s' AS sucursal, f.folio,  f.descripcion, f.valor     \
								FROM foliosemp f WHERE f.sucursal = '%s' ", sucursal, sucursal_empresa);
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("UPDATE foliosemp SET valor = 1 WHERE sucursal = '%s' ", sucursal);
			instrucciones[num_instrucciones++]=instruccion;

		}
		else{

			instruccion.sprintf("UPDATE sucursales SET defaultecom = 0");
			instrucciones[num_instrucciones++]=instruccion;

			if (latitud.Length()>3 && longitud.Length()>3 ) {
				instruccion.sprintf("update sucursales set numid=%s, nombre='%s', calle='%s', colonia='%s', localidad='%s',\
				cp='%s', email='%s', telefono1='%s', telefono2='%s', telefono3='%s', telefono4='%s', activa=%s, venxvol=%s, \
				ubicaciongis=POINT(%s,%s), salidaotrasucursal=%s, vtasecom=%s, defaultecom=%s, pickup=%s, idempresa=%s, diasdistribucion = %s \
				where sucursal='%s'",
				numid, nombre, calle, colonia, localidad, cp, email, telefono1, telefono2, telefono3, telefono4, activa, venxvol,
				latitud, longitud,salidaotrasuc, vtasecom, defaultecom, pickup, empresa,  diasdistribucion, sucursal );
				instrucciones[num_instrucciones++]=instruccion;
			}else{
				instruccion.sprintf("update sucursales set numid=%s, nombre='%s', calle='%s', colonia='%s', localidad='%s',\
				cp='%s', email='%s', telefono1='%s', telefono2='%s', telefono3='%s', telefono4='%s', activa=%s, venxvol=%s \
				, salidaotrasucursal=%s, vtasecom=%s, defaultecom=%s, pickup=%s, idempresa=%s, diasdistribucion = %s \
				where sucursal='%s'",
				numid, nombre, calle, colonia, localidad, cp, email, telefono1, telefono2, telefono3, telefono4, activa, venxvol,
				 salidaotrasuc, vtasecom, defaultecom, pickup, empresa, diasdistribucion, sucursal );
				instrucciones[num_instrucciones++]=instruccion;
			}

			instruccion.sprintf("DELETE FROM diasoperativos WHERE sucursal = '%s' ",
			sucursal );
			instrucciones[num_instrucciones++]=instruccion;

		}

		TStringDynArray dias(SplitString(diasentregas, ","));
		for (int i = 0; i < dias.Length; i++) {

			instruccion.sprintf("INSERT INTO diasoperativos (clave, sucursal, orden) \
			VALUES \
			('%s', '%s', '%s')",
			AnsiString(dias[i]), sucursal, AnsiString(i));
			instrucciones[num_instrucciones++]=instruccion;
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
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,
				buffer_sql)) {

			instruccion.sprintf("select @sucursal as sucursal");
			mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,
				instruccion.c_str(), Respuesta->TamBufferResultado);
		}

	}
	__finally {
		delete buffer_sql;
	}

//	GrabaGenerico(Respuesta, MySQL, parametros, "sucursales", "sucursal");

}

// ---------------------------------------------------------------------------
// ID_GRA_TRANSPORTISTAS
void ServidorCatalogos::GrabaTransportistas(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA TRANSPORTISTAS
	GrabaGenerico(Respuesta, MySQL, parametros, "transportistas",
		"empresatrans");
}

// ---------------------------------------------------------------------------
// ID_BAJ_SUCURSAL
void ServidorCatalogos::BajaSucursal(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {

	AnsiString instruccion;
	AnsiString claveSucursalEliminar = mFg.ExtraeStringDeBuffer(&parametros);

    mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	instruccion.sprintf("START TRANSACTION");
	mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL, instruccion.c_str());

	try{

		instruccion.sprintf("SET @sucursalEliminar = '%s'", claveSucursalEliminar);
		mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL, instruccion.c_str());

		instruccion.sprintf("DELETE FROM folioscfd cfd \
		WHERE cfd.sucursal = @sucursalEliminar AND folioinicial = 1 AND foliofinal = 9999999 AND numaprobacion = 1 \
		AND (serie, tipocomprobante) IN ((CONCAT(@sucursalEliminar, 'V'), 'VENT'), (CONCAT(@sucursalEliminar, 'C'), 'NCAR'), (CONCAT(@sucursalEliminar, 'N'), 'NCRE'), \
		(CONCAT(@sucursalEliminar, 'P'), 'PAGO'))");
		mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL, instruccion.c_str());


		char * bf_conteoParametrosCfd = new char[1024];
		try{
            instruccion.sprintf("SELECT COUNT(sucursal) AS conteo FROM parametroscfd WHERE sucursal = @sucursalEliminar");
			mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), bf_conteoParametrosCfd);
			BufferRespuestas br_conteoParametrosCfd(bf_conteoParametrosCfd);

			if(br_conteoParametrosCfd.ObtieneDato("conteo") == "1"){
				instruccion.sprintf("DELETE FROM parametroscfd");
                mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL, instruccion.c_str());
			}
		}__finally{
			delete bf_conteoParametrosCfd;
		}

		instruccion.sprintf("DELETE FROM diasoperativos WHERE sucursal = @sucursalEliminar");
		mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL, instruccion.c_str());

		instruccion.sprintf("DELETE FROM estadosistemaemp WHERE sucursal = @sucursalEliminar");
		mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL, instruccion.c_str());

		instruccion.sprintf("SET @usuarioEliminar = (SELECT empleado FROM empleados WHERE sucursal = @sucursalEliminar ORDER BY fechaalta ASC LIMIT 1)");
		mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL, instruccion.c_str());

		instruccion.sprintf("DELETE FROM articulosxsuc WHERE usumodi = @usuarioEliminar");
		mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL, instruccion.c_str());

		instruccion.sprintf("DELETE FROM terminales WHERE usuario = @usuarioEliminar");
		mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL, instruccion.c_str());

		instruccion.sprintf("DELETE FROM usuariosucursal WHERE usuario = @usuarioEliminar");
		mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL, instruccion.c_str());

		instruccion.sprintf("DELETE FROM usuarios WHERE empleado = @usuarioEliminar");
		mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL, instruccion.c_str());

        instruccion.sprintf("DELETE FROM empleados WHERE sucursal = 'MN' AND empleado = @usuarioEliminar");
		mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL, instruccion.c_str());

		instruccion.sprintf("DELETE FROM articulosped WHERE sucursal = @sucursalEliminar");
		mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL, instruccion.c_str());

		instruccion.sprintf("DELETE FROM foliosemp WHERE sucursal = @sucursalEliminar and valor = 1");
		mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL, instruccion.c_str());

		instruccion.sprintf("DELETE FROM parametrosemp WHERE sucursal = @sucursalEliminar");
		mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL, instruccion.c_str());

		instruccion.sprintf("SET @seccionEliminar = CONCAT(@sucursalEliminar, 'S1')");
		mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL, instruccion.c_str());

		instruccion.sprintf("SET @almacenEliminar = CONCAT('AL', @sucursalEliminar)");
		mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL, instruccion.c_str());

		instruccion.sprintf("DELETE FROM articuloxseccion WHERE seccion = @seccionEliminar");
		mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL, instruccion.c_str());

		instruccion.sprintf("DELETE FROM ventasxmes WHERE almacen = @almacenEliminar AND ventas180 = 0.0 AND ventascorte = 0.0");
		mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL, instruccion.c_str());

		instruccion.sprintf("DELETE FROM almacenes where almacen = @almacenEliminar");
		mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL, instruccion.c_str());

		instruccion.sprintf("DELETE FROM secciones where seccion = @seccionEliminar");
		mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL, instruccion.c_str());

		instruccion.sprintf("DELETE FROM sucursales WHERE sucursal = @sucursalEliminar");
		mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL, instruccion.c_str());

		instruccion.sprintf("DELETE FROM bancosconfigsuc WHERE sucursal = @sucursalEliminar");
		mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL, instruccion.c_str());

		instruccion.sprintf("DELETE FROM plantillasxsuc WHERE sucursal = @sucursalEliminar");
		mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL, instruccion.c_str());

		instruccion.sprintf("DELETE FROM existenciasactuales WHERE almacen = @almacenEliminar AND cantidad = 0.0");
		mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL, instruccion.c_str());

		instruccion.sprintf("COMMIT");
		mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL, instruccion.c_str());
	} catch(Exception &e){
		instruccion.sprintf("ROLLBACK");
		mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL, instruccion.c_str());
		throw e;
	}



	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, "SELECT 0 as error", Respuesta->TamBufferResultado);
}

// ---------------------------------------------------------------------------
// ID_BAJ_TRANSPORTISTAS
void ServidorCatalogos::BajaTransportistas(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// BAJA TRANSPORTISTAS
	BajaGenerico(Respuesta, MySQL, parametros, "transportistas",
		"empresatrans");
}

// ---------------------------------------------------------------------------
// ID_CON_SUCURSAL
void ServidorCatalogos::ConsultaSucursal(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA SUCURSAL
	AnsiString instruccion;
	AnsiString clave_sucursal, clave_empresa;
	AnsiString condicion_empresa = " ";

	clave_sucursal = mFg.ExtraeStringDeBuffer(&parametros);
	clave_empresa = mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	if(clave_empresa != " "){
		condicion_empresa.sprintf(" AND s.idempresa = %s ", clave_empresa);
	}

	// Obtiene todos los datos de la sucursal
	instruccion.sprintf("select *,ifnull( FORMAT(X(ubicaciongis),6) ,'') as latitud, ifnull( FORMAT(Y(ubicaciongis),6) ,'') as longitud \
	 from sucursales where sucursal='%s'",
		clave_sucursal);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todas las sucursales
	instruccion.sprintf("select sucursal AS Sucursal, numid as NumId, s.nombre AS Nombre, if(venxvol=1,'Sí','No') as VenXVol, salidaotrasucursal as Salida, activa as Activa, em.nombre AS nomempresa \
		from sucursales s \
		left join empresas em ON em.idempresa = s.idempresa \
		where 1 %s \
		order by nomempresa,s.nombre ", condicion_empresa);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene los días operativos
	instruccion.sprintf(
		"SELECT clave FROM diasoperativos WHERE sucursal = '%s' ORDER BY orden ",clave_sucursal);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}

// ---------------------------------------------------------------------------
// ID_CON_TRANSPORTISTAS
void ServidorCatalogos::ConsultaTransportistas(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA SUCURSAL
	AnsiString instruccion;
	AnsiString clave_sucursal;

	clave_sucursal = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos de la sucursal
	instruccion.sprintf("select * from transportistas where empresatrans='%s'",
		clave_sucursal);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todas las sucursales
	instruccion =
		"SELECT empresatrans AS Sucursal, nombre AS Nombre, calle AS Calle FROM transportistas ORDER BY nombre";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

// BANCOS

// ---------------------------------------------------------------------------
// ID_GRA_BANCO
void ServidorCatalogos::GrabaBanco(RespuestaServidor *Respuesta, MYSQL *MySQL,
	char *parametros) {
	// GRABA BANCO
	GrabaGenerico(Respuesta, MySQL, parametros, "bancos", "banco");
}

// ---------------------------------------------------------------------------
// ID_BAJ_BANCO
void ServidorCatalogos::BajaBanco(RespuestaServidor *Respuesta, MYSQL *MySQL,
	char *parametros) {
	// BAJA BANCO
	BajaGenerico(Respuesta, MySQL, parametros, "bancos", "banco");
}

// ---------------------------------------------------------------------------
// ID_CON_CONCEPTOSEMBARQUE
void ServidorCatalogos::ConsultaConceptosEmbarque(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	AnsiString instruccion;
	AnsiString clave;

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos
	instruccion.sprintf("select * from conceptosembarque where clave='%s'",
		clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos
	instruccion =
		"select clave AS Clave, descripcion AS Descripcion from conceptosembarque order by descripcion";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}

// ---------------------------------------------------------------------------
// ID_GRA_CONCEPTOSEMBARQUE
void ServidorCatalogos::GrabaConceptosEmbarque(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA CONCEPTOS DE EMBARQUE
	GrabaGenerico(Respuesta, MySQL, parametros, "conceptosembarque", "clave");
}

// ---------------------------------------------------------------------------
// ID_BAJ_CONCEPTOSEMBARQUE
void ServidorCatalogos::BajaConceptosEmbarque(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// BAJA CONCEPTOS DE EMBARQUE
	BajaGenerico(Respuesta, MySQL, parametros, "conceptosembarque", "clave");
}

// ---------------------------------------------------------------------------
// ID_CON_BANCO
void ServidorCatalogos::ConsultaBanco(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA BANCO
	AnsiString instruccion;
	AnsiString clave_banco;

	clave_banco = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos del banco
	instruccion.sprintf("select * from bancos where banco='%s'", clave_banco);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los bancos
	instruccion =
		"select banco AS Banco, nombre AS Nombre, activoapp AS Visible from bancos order by nombre";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}

// ---------------------------------------------------------------------------
// ID_GRA_BANCONAT
void ServidorCatalogos::GrabaBancoNat(RespuestaServidor *Respuesta, MYSQL *MySQL,
	char *parametros) {
	// GRABA BANCONAT
	GrabaGenerico(Respuesta, MySQL, parametros, "bancosnaturalezas", "naturaleza");
}

// ---------------------------------------------------------------------------
// ID_CON_BANCONAT
void ServidorCatalogos::ConsultaBancoNat(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA BANCONAT
	AnsiString instruccion;
	AnsiString clave_banco;

	clave_banco = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos del banco
	instruccion.sprintf("select * from bancosnaturalezas where naturaleza='%s'", clave_banco);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los bancos
	instruccion =
		"select  naturaleza AS Naturaleza, descripcion AS Descripcion from bancosnaturalezas order by descripcion";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}

// ---------------------------------------------------------------------------

// ID_BAJ_BANCONAT
void ServidorCatalogos::BajaBancoNat(RespuestaServidor *Respuesta, MYSQL *MySQL,
	char *parametros) {
	// BAJA BANCO
	BajaGenerico(Respuesta, MySQL, parametros, "bancosnaturalezas", "naturaleza");
}


// ---------------------------------------------------------------------------
// ID_GRA_BANCOCTA
void ServidorCatalogos::GrabaBancoCta(RespuestaServidor *Respuesta, MYSQL *MySQL,
	char *parametros) {
	// GRABA BANCO
	GrabaGenerico(Respuesta, MySQL, parametros, "bancoscuentas", "numerocuenta");
}
// ---------------------------------------------------------------------------

// ID_CON_BANCOCTA
void ServidorCatalogos::ConsultaBancoCta(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA BANCO
	AnsiString instruccion;
	AnsiString num_cuenta, clave_banco;
	AnsiString idEmpresa;

	num_cuenta = mFg.ExtraeStringDeBuffer(&parametros);
	clave_banco = mFg.ExtraeStringDeBuffer(&parametros);
	idEmpresa = FormServidor->ObtieneClaveEmpresa();

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos del banco
	instruccion.sprintf("SELECT numerocuenta, descripcion, banco, naturaleza, saldoinicial, \
	clabe, principal, mostrarenfactura, idempresa \
	FROM bancoscuentas WHERE numerocuenta = '%s' AND banco='%s' and idempresa = %s \
	ORDER BY idnumcuenta",
	num_cuenta, clave_banco, idEmpresa);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene todos los bancos
	instruccion.sprintf("SELECT numerocuenta, descripcion, banco, naturaleza, saldoinicial, \
	clabe, principal, mostrarenfactura, emp.nombre  \
	FROM bancoscuentas bc \
	LEFT JOIN empresas emp ON emp.idempresa = bc.idempresa \
	WHERE emp.idempresa = %s \
	ORDER BY idnumcuenta", idEmpresa);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------

// ID_BAJ_BANCOCTA
void ServidorCatalogos::BajaBancoCta(RespuestaServidor *Respuesta, MYSQL *MySQL,
	char *parametros) {
	// BAJA BANCO
	BajaGenerico(Respuesta, MySQL, parametros, "bancoscuentas", "idnumcuenta");
}

// ---------------------------------------------------------------------------


// ---------------------------------------------------------------------------
// ID_GRA_BANCOORIGEN
void ServidorCatalogos::GrabaBancoOrigen(RespuestaServidor *Respuesta, MYSQL *MySQL,
	char *parametros) {
	// GRABA BANCO ORIGEN
	GrabaGenerico(Respuesta, MySQL, parametros, "bancosorigenes", "origen");
}
// ---------------------------------------------------------------------------
// ID_CON_BANCOORIGEN
void ServidorCatalogos::ConsultaBancoOrigen(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA BANCO ORIGEN
	AnsiString instruccion;
	AnsiString clave_banco_origen;

	clave_banco_origen = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos del origen dado como parámetro.
	instruccion.sprintf("select origen,descripcion, afectacion, essistema from bancosorigenes where origen='%s'", clave_banco_origen);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los origenes de mov. de bancos
	instruccion =
		"select origen AS Origen, descripcion AS Descripcion, afectacion, essistema from bancosorigenes order by essistema, descripcion";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------

// ID_BAJ_BANCOORIGEN
void ServidorCatalogos::BajaBancoOrigen(RespuestaServidor *Respuesta, MYSQL *MySQL,
	char *parametros) {
	// BAJA BANCO
	BajaGenerico(Respuesta, MySQL, parametros, "bancosorigenes", "origen");
}

// ---------------------------------------------------------------------------

// CONCEPTOS POR MOVIMIENTOS DE ALMACEN

// ---------------------------------------------------------------------------
// ID_GRA_CONCEPTOMOVALMA
void ServidorCatalogos::GrabaConceptoMovAlma(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA CONCEPTO POR MOVIMIENTOS DE ALMACEN
	GrabaGenerico(Respuesta, MySQL, parametros, "conceptosmovalma", "concepto");
}

// ---------------------------------------------------------------------------
// ID_BAJ_CONCEPTOMOVALMA
void ServidorCatalogos::BajaConceptoMovAlma(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// BAJA CONCEPTO POR MOVIMIENTOS DE ALMACEN
	BajaGenerico(Respuesta, MySQL, parametros, "conceptosmovalma", "concepto");
}

// ---------------------------------------------------------------------------
// ID_CON_CONCEPTOMOVALMA
void ServidorCatalogos::ConsultaConceptoMovAlma(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA CONCEPTO POR MOVIMIENTOS DE ALMACEN
	AnsiString instruccion;
	AnsiString clave_concepto;

	clave_concepto = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos del concepto
	instruccion.sprintf("select * from conceptosmovalma where concepto='%s'",
		clave_concepto);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los conceptos
	instruccion =
		"select concepto, descripcion, tipomov, costofijo, protegido, activo, traspasosucnouni \
				 from conceptosmovalma order by descripcion";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

// DEPARTAMENTOS

// ---------------------------------------------------------------------------
// ID_GRA_DEPARTAMENTO
void ServidorCatalogos::GrabaDepartamento(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA DEPARTAMENTO
	GrabaGenerico(Respuesta, MySQL, parametros, "departamentosrh", "depart");
}

// ---------------------------------------------------------------------------
// ID_BAJ_DEPARTAMENTO
void ServidorCatalogos::BajaDepartamento(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// BAJA DEPARTAMENTO
	BajaGenerico(Respuesta, MySQL, parametros, "departamentosrh", "depart");
}

// ---------------------------------------------------------------------------
// ID_CON_DEPARTAMENTO
void ServidorCatalogos::ConsultaDepartamento(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA DEPARTAMENTO
	AnsiString instruccion;
	AnsiString clave_departamento, nombre_departamento;
	AnsiString condicion_nombre_departamento = " ";

	clave_departamento = mFg.ExtraeStringDeBuffer(&parametros);
	nombre_departamento = mFg.ExtraeStringDeBuffer(&parametros);

	if(nombre_departamento != ""){
		condicion_nombre_departamento.sprintf(" AND nombre LIKE '%%%s%' ", nombre_departamento);
	}

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos del departamento
	instruccion.sprintf("select * from departamentosrh where depart='%s'",
		clave_departamento);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los departamentos
	instruccion.sprintf("SELECT depart AS Departamento, nombre AS Nombre \
	FROM departamentosrh \
	WHERE 1 %s \
	ORDER BY nombre", condicion_nombre_departamento);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}

// ---------------------------------------------------------------------------

// PUESTOS

// ---------------------------------------------------------------------------
// ID_GRA_PUESTO
void ServidorCatalogos::GrabaPuesto(RespuestaServidor *Respuesta, MYSQL *MySQL,
	char *parametros) {

	char *buffer_sql = new char[300 * 500];
	char *aux_buffer_sql = buffer_sql;
	int i;
	int num_instrucciones = 0;
	AnsiString instruccion, instrucciones[500];
	AnsiString tarea, roles;
	AnsiString clvepuesto, nompuesto;
	AnsiString clvejefatura, nomjefatura;
	AnsiString clvegerencia, nomgerencia;
    AnsiString diasdefa;

	try {
		tarea = mFg.ExtraeStringDeBuffer(&parametros);
		clvepuesto = mFg.ExtraeStringDeBuffer(&parametros);
		nompuesto = mFg.ExtraeStringDeBuffer(&parametros);
		clvejefatura = mFg.ExtraeStringDeBuffer(&parametros);
		nomjefatura = mFg.ExtraeStringDeBuffer(&parametros);
		clvegerencia = mFg.ExtraeStringDeBuffer(&parametros);
		nomgerencia = mFg.ExtraeStringDeBuffer(&parametros);
		roles = mFg.ExtraeStringDeBuffer(&parametros);
		diasdefa = mFg.ExtraeStringDeBuffer(&parametros);

		instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++] = "START TRANSACTION";

		instruccion.sprintf("DELETE FROM rolesxpuesto WHERE puesto='%s'", clvepuesto);
		instrucciones[num_instrucciones++] = instruccion;

		if (tarea=="A") {

			if(clvejefatura=="" && clvegerencia==""){

				instruccion.sprintf("INSERT INTO puestos (puesto, nombre, \
				jefatura, gerencia, clvjefatura, clvgerencia, diasdefa) \
				VALUES \
				('%s','%s',NULL,NULL,NULL,NULL, %s)",
				clvepuesto, nompuesto, diasdefa);
				instrucciones[num_instrucciones++] = instruccion;
			}

			if(clvejefatura!="" && clvegerencia==""){

				instruccion.sprintf("INSERT INTO puestos (puesto, nombre, \
				jefatura, gerencia, clvjefatura, clvgerencia, diasdefa) \
				VALUES \
				('%s','%s','%s',NULL,'%s',NULL, %s)",
				clvepuesto, nompuesto,nomjefatura, clvejefatura, diasdefa);
				instrucciones[num_instrucciones++] = instruccion;
			}

			if(clvejefatura=="" && clvegerencia!=""){

				instruccion.sprintf("INSERT INTO puestos (puesto, nombre, \
				jefatura, gerencia, clvjefatura, clvgerencia, diasdefa) \
				VALUES \
				('%s','%s',NULL,'%s',NULL,'%s', %s)",
				clvepuesto, nompuesto,nomgerencia, clvegerencia, diasdefa);
				instrucciones[num_instrucciones++] = instruccion;
			}

			if(clvejefatura!="" && clvegerencia!=""){

				instruccion.sprintf("INSERT INTO puestos (puesto, nombre, \
				jefatura, gerencia, clvjefatura, clvgerencia, diasdefa) \
				VALUES \
				('%s','%s','%s','%s','%s','%s', %s)",
				clvepuesto, nompuesto, nomjefatura, nomgerencia, clvejefatura,
				clvegerencia, diasdefa);
				instrucciones[num_instrucciones++] = instruccion;
			}

		} else {



			if(clvejefatura=="" && clvegerencia==""){

				instruccion.sprintf("UPDATE puestos \
				SET nombre = '%s', jefatura = NULL, gerencia = NULL, \
				clvjefatura = NULL, clvgerencia = NULL, diasdefa = %s \
				WHERE puesto = '%s' ",
				nompuesto, diasdefa, clvepuesto);
				instrucciones[num_instrucciones++] = instruccion;
			}

			if(clvejefatura!="" && clvegerencia==""){

				instruccion.sprintf("UPDATE puestos \
				SET nombre = '%s', jefatura = '%s', gerencia = NULL, \
				clvjefatura = '%s', clvgerencia = NULL, diasdefa = %s \
				WHERE puesto = '%s' ",
				nompuesto, nomjefatura, clvejefatura, diasdefa, clvepuesto);
				instrucciones[num_instrucciones++] = instruccion;
			}

			if(clvejefatura=="" && clvegerencia!=""){

				instruccion.sprintf("UPDATE puestos \
				SET nombre = '%s', jefatura = NULL, gerencia = '%s', \
				clvjefatura = NULL, clvgerencia = '%s', diasdefa = %s \
				WHERE puesto = '%s' ",
				nompuesto, nomgerencia, clvegerencia, diasdefa, clvepuesto);
				instrucciones[num_instrucciones++] = instruccion;
			}

			if(clvejefatura!="" && clvegerencia!=""){

				instruccion.sprintf("UPDATE puestos \
				SET nombre = '%s', jefatura = '%s', gerencia = '%s', \
				clvjefatura = '%s', clvgerencia = '%s', diasdefa = %s \
				WHERE puesto = '%s' ",
				nompuesto, nomjefatura, nomgerencia, clvejefatura, clvegerencia,
				diasdefa, clvepuesto);
				instrucciones[num_instrucciones++] = instruccion;
			}

		}

		if(roles!=" "){
			TStringDynArray rol(SplitString(roles, ","));
			for (int i = 0; i < rol.Length; i++) {
				instruccion.sprintf("INSERT INTO rolesxpuesto (puesto, rol) VALUES ('%s', %s)",
				clvepuesto, AnsiString(rol[i]));
				instrucciones[num_instrucciones++] = instruccion;
			}
		}

		instrucciones[num_instrucciones++] = "COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql = mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i = 0; i < num_instrucciones; i++)
			aux_buffer_sql = mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);

	}
	__finally {
		delete buffer_sql;
	}
}

// ---------------------------------------------------------------------------
// ID_BAJ_PUESTO
void ServidorCatalogos::BajaPuesto(RespuestaServidor *Respuesta, MYSQL *MySQL,
	char *parametros) {
	// BAJA PUESTO
	BajaGenerico(Respuesta, MySQL, parametros, "puestos", "puesto");
}

// ---------------------------------------------------------------------------
// ID_CON_PUESTO
void ServidorCatalogos::ConsultaPuesto(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA PUESTO
	AnsiString instruccion;
	AnsiString clave_puesto;
	AnsiString nombre_puesto;
	AnsiString condicion_nombre_puesto = " ";

	clave_puesto  = mFg.ExtraeStringDeBuffer(&parametros);
	nombre_puesto  = mFg.ExtraeStringDeBuffer(&parametros);

	if(nombre_puesto != ""){
		condicion_nombre_puesto.sprintf(" AND p.nombre LIKE '%%%s%' ", nombre_puesto);
	}

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos del puesto
	instruccion.sprintf("SELECT p.puesto, p.nombre, p.jefatura, p.gerencia, p.clvjefatura, p.clvgerencia, \
	IFNULL(GROUP_CONCAT(rxp.rol),'') AS roles, diasdefa \
	FROM puestos p \
	LEFT JOIN rolesxpuesto rxp ON rxp.puesto = p.puesto \
	WHERE p.puesto = '%s' ",
	clave_puesto, condicion_nombre_puesto);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene todos los puestos
	instruccion.sprintf("SELECT p.puesto AS Puesto, \
	p.nombre AS Nombre, \
	IFNULL(GROUP_CONCAT(rxp.rol),'') AS roles, diasdefa \
	FROM puestos p \
	LEFT JOIN rolesxpuesto rxp ON rxp.puesto = p.puesto \
	WHERE 1 %s \
	GROUP BY p.puesto \
	ORDER BY p.nombre", condicion_nombre_puesto);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene las jefaturas del puesto
	instruccion.sprintf("select clvjefatura, jefatura from puestos where puesto='%s' order by jefatura", clave_puesto);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene las gerencias del puesto
	instruccion.sprintf("select clvgerencia, gerencia from puestos where puesto='%s' order by gerencia", clave_puesto);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}

// ---------------------------------------------------------------------------

// USUARIOS

// ---------------------------------------------------------------------------
// ID_GRA_USUARIO
void ServidorCatalogos::GrabaUsuarios(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA USUARIO
	GrabaUsuario(Respuesta, MySQL, parametros, "usuarios", "empleado");
}

// ---------------------------------------------------------------------------
// ID_BAJ_USUARIO
void ServidorCatalogos::BajaUsuario(RespuestaServidor *Respuesta, MYSQL *MySQL,
	char *parametros) {
	// BAJA USUARIO
	BajaGenerico(Respuesta, MySQL, parametros, "usuarios", "empleado");
}

// ---------------------------------------------------------------------------
// ID_CON_USUARIO
void ServidorCatalogos::ConsultaUsuario(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA USUARIO
	AnsiString instruccion;
	AnsiString clave_usuario;
	AnsiString activos;
	AnsiString filtrousuario;
	AnsiString condicion_filtrousuario = " ";

	clave_usuario = mFg.ExtraeStringDeBuffer(&parametros);
	activos=mFg.ExtraeStringDeBuffer(&parametros);
	filtrousuario=mFg.ExtraeStringDeBuffer(&parametros);

	if(filtrousuario != " "){
	   condicion_filtrousuario.sprintf(" AND ( e.nombre LIKE '%%%s%%' OR e.appat LIKE '%%%s%%' OR e.apmat LIKE '%%%s%%' ) ",
										 filtrousuario, filtrousuario, filtrousuario);
	}

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos del usuario
	instruccion.sprintf("select * from usuarios where empleado='%s'",
		clave_usuario);


	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los usuarios
	instruccion.sprintf("SELECT u.empleado AS Usuario, concat(e.nombre,' ',e.appat,' ',e.apmat) AS nombre , \
	(SELECT b.fecha FROM bitacorausuario b WHERE u.empleado=b.usuario ORDER BY fecha DESC LIMIT 1) AS fecha, \
	u.activo from usuarios u INNER JOIN empleados e ON e.empleado=u.empleado WHERE true %s %s \
	order by Nombre ",activos, condicion_filtrousuario);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los empleados
	instruccion = "select empleado AS Empleado, concat(nombre,' ',appat,' ',apmat) AS Nombre \
				 from empleados order by nombre";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

}

// ---------------------------------------------------------------------------

// TERMINALES

// ---------------------------------------------------------------------------
// ID_GRA_TERMINAL
void ServidorCatalogos::GrabaTerminal(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA TERMINAL
	GrabaGenerico(Respuesta, MySQL, parametros, "terminales", "terminal");
}

// ---------------------------------------------------------------------------
// ID_BAJ_TERMINAL
void ServidorCatalogos::BajaTerminal(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// BAJA TERMINAL
	BajaGenerico(Respuesta, MySQL, parametros, "terminales", "terminal");
}

// ---------------------------------------------------------------------------
// ID_CON_TERMINAL
void ServidorCatalogos::ConsultaTerminal(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA TERMINAL
	AnsiString instruccion;
	AnsiString clave_terminal, sucursal, indiciobusq;
	AnsiString condicion_indiciobusq=" ";

	clave_terminal = mFg.ExtraeStringDeBuffer(&parametros);
	sucursal = mFg.ExtraeStringDeBuffer(&parametros);
	indiciobusq = mFg.ExtraeStringDeBuffer(&parametros);

	if (indiciobusq!=" ") {
	   condicion_indiciobusq.sprintf(" AND (t.terminal LIKE '%%%s%%' OR t.nombre LIKE '%%%s%%') ", indiciobusq, indiciobusq);
	}

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos de la terminal
	instruccion.sprintf("select * from terminales where terminal='%s'",
		clave_terminal);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todas las terminales
	if(sucursal == ""){
		instruccion.sprintf("select terminal AS Terminal, nombre AS Nombre from terminales t where 1 %s ", condicion_indiciobusq);
	}else{
		instruccion.sprintf("select t.terminal AS Terminal, t.nombre AS Nombre from terminales t INNER JOIN \
							   secciones s ON s.seccion	= t.seccion where s.sucursal = '%s' %s",sucursal, condicion_indiciobusq);
	}
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los usuarios
	instruccion =
		"select usuarios.empleado AS Usuario, concat(empleados.nombre,' ',empleados.appat,' ',empleados.apmat) AS Nombre ";
	instruccion += "from usuarios, empleados ";
	instruccion +=
		"where empleados.empleado=usuarios.empleado and usuarios.activo=1 order by nombre";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los departamentos
	instruccion =
		"select depart AS Departamento, nombre AS Nombre from departamentos order by nombre";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todas las secciones
	instruccion =
		"select seccion AS Seccion, nombre AS Nombre from secciones order by nombre";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	instruccion = "select almacen, nombre from almacenes order by nombre";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}

// ---------------------------------------------------------------------------

// TIPOS DE BLOQUEOS

// ---------------------------------------------------------------------------
// ID_GRA_TIPODEBLOQUEO
void ServidorCatalogos::GrabaTipoDeBloqueo(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA TIPO DE BLOQUEOS
	GrabaGenerico(Respuesta, MySQL, parametros, "tiposdebloqueos", "tipo");
}

// ---------------------------------------------------------------------------
// ID_BAJ_TIPODEBLOQUEO
void ServidorCatalogos::BajaTipoDeBloqueo(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// BAJA TIPO DE BLOQUEOS
	BajaGenerico(Respuesta, MySQL, parametros, "tiposdebloqueos", "tipo");
}

// ---------------------------------------------------------------------------
// ID_CON_TIPODEBLOQUEO
void ServidorCatalogos::ConsultaTipoDeBloqueo(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA TIPOS DE BLOQUEOS
	AnsiString instruccion;
	AnsiString clave;

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos del tipo de bloqueo
	instruccion.sprintf("select * from tiposdebloqueos where tipo='%s'", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los tipos de bloqueos
	instruccion =
		"select tipo AS TipoBloqueo, descripcion AS Descripcion from tiposdebloqueos order by descripcion";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}

// ---------------------------------------------------------------------------

// TIPOS DE IMPUESTOS

// ---------------------------------------------------------------------------
// ID_GRA_TIPODEIMPUESTO
void ServidorCatalogos::GrabaTipoDeImpuesto(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA TIPO DE IMPUESTO
	GrabaGenerico(Respuesta, MySQL, parametros, "tiposdeimpuestos", "tipoimpu");
}

// ---------------------------------------------------------------------------
// ID_BAJ_TIPODEIMPUESTO
void ServidorCatalogos::BajaTipoDeImpuesto(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// BAJA TIPO DE IMPUESTO
	BajaGenerico(Respuesta, MySQL, parametros, "tiposdeimpuestos", "tipoimpu");
}

// ---------------------------------------------------------------------------
// ID_CON_TIPODEIMPUESTO
void ServidorCatalogos::ConsultaTipoDeImpuesto(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA TIPO DE IMPUESTO
	AnsiString instruccion;
	AnsiString clave;

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos del tipo de impuesto
	instruccion.sprintf("select * from tiposdeimpuestos where tipoimpu='%s'",
		clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los tipos de impuestos
	instruccion =
		"select tipoimpu AS TipoImpuesto, nombre AS Nombre from tiposdeimpuestos order by nombre";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}

// ---------------------------------------------------------------------------

// TIPOS DE PRIVILEGIOS

// ---------------------------------------------------------------------------
// ID_GRA_TIPODEPRIVILEGIO
void ServidorCatalogos::GrabaTipoDePrivilegio(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA TIPO DE PRIVILEGIO
	GrabaGenerico(Respuesta, MySQL, parametros, "tiposdeprivilegios",
		"tipopriv");
}

// ---------------------------------------------------------------------------
// ID_BAJ_TIPODEPRIVILEGIO
void ServidorCatalogos::BajaTipoDePrivilegio(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// BAJA TIPO DE PRIVILEGIO
	BajaGenerico(Respuesta, MySQL, parametros, "tiposdeprivilegios",
		"tipopriv");
}

// ---------------------------------------------------------------------------
// ID_CON_TIPODEPRIVILEGIO
void ServidorCatalogos::ConsultaTipoDePrivilegio(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA TIPO DE PRIVILEGIO
	AnsiString instruccion;
	AnsiString clave;

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos del tipo de privilegio
	instruccion.sprintf("select * from tiposdeprivilegios where tipopriv='%s'",
		clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los tipos de privilegio
	instruccion =
		"select tipopriv AS TipoPrivilegio, descripcion AS Descripcion from tiposdeprivilegios order by tipopriv";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}

// ---------------------------------------------------------------------------

// TIPOS DE TRANSACCIONES X COBRAR

// ---------------------------------------------------------------------------
// ID_GRA_TIPODETRANXCOB
void ServidorCatalogos::GrabaTipoDeTranxcob(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA TIPO DE TRANSACCIONES X COBRAR
	GrabaGenerico(Respuesta, MySQL, parametros, "tiposdetranxcob", "tipo");
}

// ---------------------------------------------------------------------------
// ID_BAJ_TIPODETRANXCOB
void ServidorCatalogos::BajaTipoDeTranxcob(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// BAJA TIPO DE TRANSACCIONES X COBRAR
	BajaGenerico(Respuesta, MySQL, parametros, "tiposdetranxcob", "tipo");
}

// ---------------------------------------------------------------------------
// ID_CON_TIPODETRANXCOB
void ServidorCatalogos::ConsultaTipoDeTranxcob(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA TIPO DE TRANSACCIONES X COBRAR
	AnsiString instruccion;
	AnsiString clave;

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos del tipo de transacciones x cobrar
	instruccion.sprintf("select * from tiposdetranxcob where tipo='%s'", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los tipos de transacciones x cobrar
	instruccion =
		"select tipo AS TipoTransaccion, descripcion AS Descripcion from tiposdetranxcob order by descripcion";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}

// ---------------------------------------------------------------------------

// TIPOS DE TRANSACCIONES X PAGAR

// ---------------------------------------------------------------------------
// ID_GRA_TIPODETRANXPAG
void ServidorCatalogos::GrabaTipoDeTranxpag(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA TIPO DE TRANSACCIONES X PAGAR
	GrabaGenerico(Respuesta, MySQL, parametros, "tiposdetranxpag", "tipo");
}

// ---------------------------------------------------------------------------
// ID_BAJ_TIPODETRANXPAG
void ServidorCatalogos::BajaTipoDeTranxpag(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// BAJA TIPO DE TRANSACCIONES X PAGAR
	BajaGenerico(Respuesta, MySQL, parametros, "tiposdetranxpag", "tipo");
}

// ---------------------------------------------------------------------------
// ID_CON_TIPODETRANXPAG
void ServidorCatalogos::ConsultaTipoDeTranxpag(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA TIPO DE TRANSACCIONES X PAGAR
	AnsiString instruccion;
	AnsiString clave;

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos del tipo de transacciones x pagar
	instruccion.sprintf("select * from tiposdetranxpag where tipo='%s'", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los tipos de transacciones x pagar
	instruccion =
		"select tipo AS TipoTransaccion, descripcion AS Descripcion from tiposdetranxpag order by descripcion";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}

// ---------------------------------------------------------------------------

// TIPOS DE PRECIOS

// ---------------------------------------------------------------------------
// ID_GRA_TIPODEPRECIO
void ServidorCatalogos::GrabaTipoDePrecio(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA TIPO DE PRECIOS
	char *buffer_sql = new char[1024 * 5];
	char *aux_buffer_sql = buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instrucciones[10];
	AnsiString instruccion;
	int num_instrucciones = 0;
	AnsiString clave, tarea, recalcular;
	AnsiString porc_ut, porcentaje_utilidad, porcentaje_utilidad2, porcentaje_utilidad3, porcentaje_utilidad4, porcentaje_utilidad5;
	AnsiString empresa;
	char tipo;
	int i;


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
		tarea = mFg.ExtraeStringDeBuffer(&parametros);
		// Indicador si se va a agregar o modificar
		clave = mFg.ExtraeStringDeBuffer(&parametros); // Clave
		recalcular = mFg.ExtraeStringDeBuffer(&parametros); // Indica si se van a recalcular automáticamente los precios.

		instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++] = "START TRANSACTION";

		datos.AsignaTabla("tiposdeprecios");
		parametros += datos.InsCamposDesdeBuffer(parametros);
		porcentaje_utilidad = datos.ObtieneValorCampo("porcutil");
		porcentaje_utilidad2 = datos.ObtieneValorCampo("porcutil2");
		porcentaje_utilidad3 = datos.ObtieneValorCampo("porcutil3");
		porcentaje_utilidad4 = datos.ObtieneValorCampo("porcutil4");
		porcentaje_utilidad5 = datos.ObtieneValorCampo("porcutil5");
		empresa = datos.ObtieneValorCampo("idempresa");

		if (tarea == "A") {
			instrucciones[num_instrucciones++] = datos.GenerarSqlInsert();

			// Calcular los precios para todos los artículos con base al nuevo tipo
			// NOTE QUE TAMBIEN SE ASIGNAN PRECIOS SIN IMPORTAR SI ARTICULOS.UTILESP=TRUE,
			// PORQUE DE CUALQUIER FORMA EL USUARIO DEBE MODIFICARLO MANUALMENTE.
			instruccion.sprintf(
				"replace into precios \
					(articulo, tipoprec, costo, porcutil, precio, precioproximo) \
				select a.articulo as articulo, \
					'%s' as tipoprec, \
					present.costobase as costo, \
					CASE ae.tipoutil  \
						WHEN '0' THEN (%s)  \
						WHEN '1' THEN (%s)  \
						WHEN '2' THEN (%s)  \
						WHEN '3' THEN (%s)  \
						WHEN '4' THEN (%s)  \
						WHEN '5' THEN (%s)  \
					END AS porcutil, \
					CASE ae.tipoutil  \
						WHEN '0' THEN ROUND(ROUND(present.costobase*(1+%s/100),pr.valor)*a.factor,pr.valor)  \
						WHEN '1' THEN ROUND(ROUND(present.costobase*(1+%s/100),pr.valor)*a.factor,pr.valor)  \
						WHEN '2' THEN ROUND(ROUND(present.costobase*(1+%s/100),pr.valor)*a.factor,pr.valor)  \
						WHEN '3' THEN ROUND(ROUND(present.costobase*(1+%s/100),pr.valor)*a.factor,pr.valor)  \
						WHEN '4' THEN ROUND(ROUND(present.costobase*(1+%s/100),pr.valor)*a.factor,pr.valor)  \
						WHEN '5' THEN ROUND(ROUND(present.costobase*(1+%s/100),pr.valor)*a.factor,pr.valor)  \
					END AS precio, \
					CASE ae.tipoutil  \
						WHEN '0' THEN ROUND(ROUND(present.costobase*(1+%s/100),pr.valor)*a.factor,pr.valor)  \
						WHEN '1' THEN ROUND(ROUND(present.costobase*(1+%s/100),pr.valor)*a.factor,pr.valor)  \
						WHEN '2' THEN ROUND(ROUND(present.costobase*(1+%s/100),pr.valor)*a.factor,pr.valor)  \
						WHEN '3' THEN ROUND(ROUND(present.costobase*(1+%s/100),pr.valor)*a.factor,pr.valor)  \
						WHEN '4' THEN ROUND(ROUND(present.costobase*(1+%s/100),pr.valor)*a.factor,pr.valor)  \
						WHEN '5' THEN ROUND(ROUND(present.costobase*(1+%s/100),pr.valor)*a.factor,pr.valor)  \
					END AS precioproximo \
					from articulos a, articulosemp ae, presentacionescb present, parametrosemp pr  \
					where a.present=present.present and a.producto=present.producto \
					and ae.articulo = a.articulo and ae.idempresa=%s \
					and pr.parametro='DIGREDOND' and present.idempresa=%s and pr.sucursal='%s' ",
					clave,
					porcentaje_utilidad, porcentaje_utilidad, porcentaje_utilidad2, porcentaje_utilidad3, porcentaje_utilidad4, porcentaje_utilidad5,
					porcentaje_utilidad, porcentaje_utilidad, porcentaje_utilidad2, porcentaje_utilidad3, porcentaje_utilidad4, porcentaje_utilidad5,
					porcentaje_utilidad, porcentaje_utilidad, porcentaje_utilidad2, porcentaje_utilidad3, porcentaje_utilidad4, porcentaje_utilidad5,
					FormServidor->ObtieneClaveEmpresa(), FormServidor->ObtieneClaveEmpresa(), FormServidor->ObtieneClaveSucursal());
			instrucciones[num_instrucciones++] = instruccion;
		}
		else {
			instrucciones[num_instrucciones++] = datos.GenerarSqlUpdate
				("tipoprec='" + clave + "' and idempresa=" + empresa);

			// Si el usuario modifica el porcentaje de un precio y además decide
			// que el sistema recalcule los precios automáticamente entonces eso
			// hacemos.
			if (recalcular == "1") {
				// Cambia el costo y los precios en la tabla de precios.
				AnsiString asignacion_precio;
				if (paramcambioprec=="0")
					asignacion_precio.sprintf("prec.precio =   \
												CASE ae.tipoutil  \
													WHEN 0 THEN ROUND(ROUND(present.costobase*(1+prec.porcutil/100),pr.valor)*a.factor,pr.valor) 		\
													WHEN 1 THEN ROUND(ROUND(present.costobase*(1+%s/100),pr.valor)*a.factor,pr.valor)                  \
													WHEN 2 THEN ROUND(ROUND(present.costobase*(1+%s/100),pr.valor)*a.factor,pr.valor)                  \
													WHEN 3 THEN ROUND(ROUND(present.costobase*(1+%s/100),pr.valor)*a.factor,pr.valor)                  \
													WHEN 4 THEN ROUND(ROUND(present.costobase*(1+%s/100),pr.valor)*a.factor,pr.valor)                  \
													WHEN 5 THEN ROUND(ROUND(present.costobase*(1+%s/100),pr.valor)*a.factor,pr.valor) END, \
												prec.precioproximo =                  \
												CASE ae.tipoutil                       \
													WHEN 0 THEN ROUND(ROUND(present.costobase*(1+prec.porcutil/100),pr.valor)*a.factor,pr.valor) 		\
													WHEN 1 THEN ROUND(ROUND(present.costobase*(1+%s/100),pr.valor)*a.factor,pr.valor)                  \
													WHEN 2 THEN ROUND(ROUND(present.costobase*(1+%s/100),pr.valor)*a.factor,pr.valor)                  \
													WHEN 3 THEN ROUND(ROUND(present.costobase*(1+%s/100),pr.valor)*a.factor,pr.valor)                  \
													WHEN 4 THEN ROUND(ROUND(present.costobase*(1+%s/100),pr.valor)*a.factor,pr.valor)                  \
													WHEN 5 THEN ROUND(ROUND(present.costobase*(1+%s/100),pr.valor)*a.factor,pr.valor) END ",
					porcentaje_utilidad, porcentaje_utilidad2, porcentaje_utilidad3, porcentaje_utilidad4, porcentaje_utilidad5,
					porcentaje_utilidad, porcentaje_utilidad2, porcentaje_utilidad3, porcentaje_utilidad4, porcentaje_utilidad5);
				else
					asignacion_precio.sprintf("prec.precioproximo =                  \
												CASE ae.tipoutil                       \
													WHEN 0 THEN ROUND(ROUND(present.costobase*(1+prec.porcutil/100),pr.valor)*a.factor,pr.valor) 		\
													WHEN 1 THEN ROUND(ROUND(present.costobase*(1+%s/100),pr.valor)*a.factor,pr.valor)                  \
													WHEN 2 THEN ROUND(ROUND(present.costobase*(1+%s/100),pr.valor)*a.factor,pr.valor)                  \
													WHEN 3 THEN ROUND(ROUND(present.costobase*(1+%s/100),pr.valor)*a.factor,pr.valor)                  \
													WHEN 4 THEN ROUND(ROUND(present.costobase*(1+%s/100),pr.valor)*a.factor,pr.valor)                  \
													WHEN 5 THEN ROUND(ROUND(present.costobase*(1+%s/100),pr.valor)*a.factor,pr.valor) END  ",
					porcentaje_utilidad, porcentaje_utilidad2, porcentaje_utilidad3, porcentaje_utilidad4, porcentaje_utilidad5);

				instruccion.sprintf(
					"update articulos a, articulosemp ae ,presentacionescb present, precios prec, tiposdeprecios tp , parametrosemp pr \
					set prec.costo=present.costobase, \
						prec.porcutil=%s, \
						%s \
					where a.present=present.present and a.producto=present.producto and  \
						prec.articulo=a.articulo and prec.tipoprec=tp.tipoprec and \
						present.costobase<>0 and prec.tipoprec='%s' and ae.tipoutil<>0 \
                        and ae.articulo=a.articulo and ae.idempresa=%s \
						and pr.parametro='DIGREDOND' and pr.sucursal='%s' and present.idempresa=%s and tp.idempresa=%s "
					, porcentaje_utilidad, asignacion_precio, clave, FormServidor->ObtieneClaveEmpresa(),
					 FormServidor->ObtieneClaveSucursal(), FormServidor->ObtieneClaveEmpresa(), FormServidor->ObtieneClaveEmpresa());

				instrucciones[num_instrucciones++] = instruccion;
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

// ---------------------------------------------------------------------------
// ID_BAJ_TIPODEPRECIO
void ServidorCatalogos::BajaTipoDePrecio(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// BAJA TIPO PRECIO
	char *buffer_sql = new char[1024 * 5];
	char *aux_buffer_sql = buffer_sql;
	AnsiString instruccion;
	int num_instrucciones = 5;
	AnsiString clave_precio;

	try {
		clave_precio = mFg.ExtraeStringDeBuffer(&parametros);

		aux_buffer_sql = mFg.AgregaStringABuffer
			(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);

		instruccion = "SET AUTOCOMMIT=0";
		aux_buffer_sql = mFg.AgregaStringABuffer(instruccion, aux_buffer_sql);

		instruccion = "START TRANSACTION";
		aux_buffer_sql = mFg.AgregaStringABuffer(instruccion, aux_buffer_sql);

		instruccion.sprintf("delete from precios where tipoprec='%s'",
			clave_precio);
		aux_buffer_sql = mFg.AgregaStringABuffer(instruccion, aux_buffer_sql);

		instruccion.sprintf("delete from tiposdeprecios where tipoprec='%s'",
			clave_precio);
		aux_buffer_sql = mFg.AgregaStringABuffer(instruccion, aux_buffer_sql);

		instruccion = "COMMIT";
		mFg.AgregaStringABuffer(instruccion, aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta,
			TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,
			buffer_sql);
	}
	__finally {
		delete buffer_sql;
	}
}

// ---------------------------------------------------------------------------
// ID_CON_TIPODEPRECIO
void ServidorCatalogos::ConsultaTipoDePrecio(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA TIPO DE PRECIOS
	AnsiString instruccion;
	AnsiString clave, empresa;

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	empresa = mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos del tipo de precio
	instruccion.sprintf("select * from tiposdeprecios where tipoprec='%s' ",
		clave );
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los tipos de precios
	instruccion.sprintf("select tipoprec AS TipoPrecio, descripcion AS Descripcion, porcutil, porcutil2, porcutil3, porcutil4, \
		porcutil5,listamovil as Movil, verventmayoreo as Mayoreo, verprecdif as Visible \
		from tiposdeprecios \
		where idempresa=%s \
		order by porcutil desc ", empresa);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------

// VIAS DE EMBARQUE

// ---------------------------------------------------------------------------
// ID_GRA_VIAEMBARQUE
void ServidorCatalogos::GrabaViaEmbarque(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA VIAS DE EMBARQUE
	GrabaGenerico(Respuesta, MySQL, parametros, "viasembarque", "viaembarq");
}

// ---------------------------------------------------------------------------
// ID_BAJ_VIAEMBARQUE
void ServidorCatalogos::BajaViaEmbarque(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// BAJA VIAS DE EMBARQUE
	BajaGenerico(Respuesta, MySQL, parametros, "viasembarque", "viaembarq");
}

// ---------------------------------------------------------------------------
// ID_CON_VIAEMBARQUE
void ServidorCatalogos::ConsultaViaEmbarque(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA VIAS DE EMBARQUE
	AnsiString instruccion;
	AnsiString clave;
	AnsiString activos;

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	activos=mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos de la via de embarque
	instruccion.sprintf("select * from viasembarque where viaembarq='%s' ",
		clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todas las vias de embarque
	instruccion.sprintf (
		"SELECT \
		ve.viaembarq AS ViaEmbarque, \
		ve.descripcion AS Descripcion, \
		IF(ve.transportista=1,'Activo','Inactivo'), \
		IF(ve.tipo = 'CX', 'Camioneta por tonelaje', IF(ve.tipo = 'TR', 'Trailer', IF(ve.tipo = 'RA', 'Rabón', 'Tortón'))) AS tipo, \
		ve.marca, \
		ve.modelo, \
		ve.volumen, ve.capacidad, ve.capnormal, IF(ve.tipoplacas='E', 'Estatales', 'Federales'), ve.placas, ve.tarjeta, \
		ve.nummotor, ve.kilometraje, IF(ve.estado='D','Disponible',IF(ve.estado='N','No disponible','En reparación')) AS estado, \
		concat(e.nombre,' ',e.appat,' ',e.apmat) AS chofer, ve.fechamant, ve.fechasegur, ve.polizaaseguradora, ca.nombre, \
		cca.cveautotrans, ve.numpermisosct, ve.tipopermiso, IF(ve.remolque=1,'Sí','No'), CONCAT(cr.numeroplaca, ' ', ccsr.cvesubtiprem, ' ', cr.senasparticulares) AS remolqueinfo \
		from viasembarque ve \
		INNER JOIN choferes cho ON cho.empleado = ve.chofer \
		INNER JOIN empleados e ON e.empleado = cho.empleado \
		INNER JOIN catalogoaseguradoras ca ON ca.clave = ve.claveaseguradora \
		INNER JOIN cconfigautotransporte cca ON cca.idclaveautotrans = ve.configvehicular \
		LEFT JOIN catalogoremolques cr ON cr.idremolque = ve.idremolque \
		LEFT JOIN ccpsubtiporem ccsr ON ccsr.idclavesubtiprem = cr.ccvesubtiporem %s ",activos);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los choferes
	instruccion =
		"select choferes.empleado AS Chofer, concat(empleados.nombre,' ',empleados.appat,' ',empleados.apmat) AS Nombre ";
	instruccion += "from choferes, empleados ";
	instruccion +=
		"where empleados.empleado=choferes.empleado and empleados.activo=1 order by empleados.nombre";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

}
// ---------------------------------------------------------------------------

// IMPUESTOS

// ---------------------------------------------------------------------------
// ID_GRA_IMPUESTO
void ServidorCatalogos::GrabaImpuesto(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA IMPUESTO
	GrabaGenerico(Respuesta, MySQL, parametros, "impuestos", "impuesto");
}

// ---------------------------------------------------------------------------
// ID_BAJ_IMPUESTO
void ServidorCatalogos::BajaImpuesto(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// BORRA UN IMPUESTO DADO COMO PARAMETRO
	char *buffer_sql = new char[1024 * 32];
	char *aux_buffer_sql = buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instrucciones[50], instruccion;
	int num_instrucciones = 0;
	AnsiString clave;
	int error = 0;
	int i;

	try {
		clave = mFg.ExtraeStringDeBuffer(&parametros); // Clave del impuesto

		// Verifica que el impuesto no este en uso ya
		if (error == 0) {
			instruccion.sprintf(
				"SELECT @error:=IF(SUM(1)>0,1,0) AS error FROM dcompras d WHERE d.claveimp1=%s OR d.claveimp2=%s OR d.claveimp3=%s OR d.claveimp4=%s", clave, clave, clave, clave);
			mServidorVioleta->EjecutaVerificacion(Respuesta, MySQL,
				instruccion.c_str(), 2, error);
		}
		if (error == 0) {
			instruccion.sprintf(
				"SELECT @error:=IF(SUM(1)>0,1,0) AS error FROM dpedidos d WHERE d.claveimp1=%s OR d.claveimp2=%s OR d.claveimp3=%s OR d.claveimp4=%s", clave, clave, clave, clave);
			mServidorVioleta->EjecutaVerificacion(Respuesta, MySQL,
				instruccion.c_str(), 3, error);
		}
		if (error == 0) {
			instruccion.sprintf(
				"SELECT @error:=IF(SUM(1)>0,1,0) AS error FROM dpedidosventa d WHERE d.claveimp1=%s OR d.claveimp2=%s OR d.claveimp3=%s OR d.claveimp4=%s", clave, clave, clave, clave);
			mServidorVioleta->EjecutaVerificacion(Respuesta, MySQL,
				instruccion.c_str(), 4, error);
		}
		if (error == 0) {
			instruccion.sprintf(
				"SELECT @error:=IF(SUM(1)>0,1,0) AS error FROM dventas d WHERE d.claveimp1=%s OR d.claveimp2=%s OR d.claveimp3=%s OR d.claveimp4=%s", clave, clave, clave, clave);
			mServidorVioleta->EjecutaVerificacion(Respuesta, MySQL,
				instruccion.c_str(), 5, error);
		}
		if (error == 0) {
			instruccion.sprintf(
				"SELECT @error:=IF(SUM(1)>0,1,0) AS error FROM productos p WHERE p.claveimpv1=%s OR p.claveimpv2=%s OR p.claveimpv3=%s OR p.claveimpv4=%s OR  p.claveimpc1=%s OR p.claveimpc2=%s OR p.claveimpc3=%s OR p.claveimpc4=%s ", clave, clave, clave, clave, clave, clave, clave, clave);
			mServidorVioleta->EjecutaVerificacion(Respuesta, MySQL,
				instruccion.c_str(), 6, error);
		}
		if (error == 0) {
			instruccion.sprintf(
				"SELECT @error:=IF(SUM(1)>0,1,0) AS error FROM notascarcli n WHERE n.cveimp=%s ", clave);
			mServidorVioleta->EjecutaVerificacion(Respuesta, MySQL,
				instruccion.c_str(), 7, error);
		}
		if (error == 0) {
			instruccion.sprintf(
				"SELECT @error:=IF(SUM(1)>0,1,0) AS error FROM notascarprov n WHERE n.cveimp=%s ", clave);
			mServidorVioleta->EjecutaVerificacion(Respuesta, MySQL,
				instruccion.c_str(), 8, error);
		}

		if (error == 0) {
			instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++] = "START TRANSACTION";

			// Elimina el impuesto
			instruccion.sprintf("delete from impuestos where impuesto='%s'",
				clave);
			instrucciones[num_instrucciones++] = instruccion;

			instrucciones[num_instrucciones++] = "COMMIT";
		}

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql = mFg.AgregaStringABuffer
			(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i = 0; i < num_instrucciones; i++)
			aux_buffer_sql = mFg.AgregaStringABuffer(instrucciones[i],
			aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta,
			TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,
				buffer_sql)) {
			instruccion.sprintf("select %d as error", error);
			mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,
				instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	}
	__finally {
		delete buffer_sql;
	}
}

// ---------------------------------------------------------------------------
// ID_CON_IMPUESTO
void ServidorCatalogos::ConsultaImpuesto(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA IMPUESTO
	AnsiString instruccion;
	AnsiString clave;

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos del impuesto
	instruccion.sprintf("select * from impuestos where impuesto='%s'", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los impuestos
	instruccion =
		"select impuestos.impuesto AS Impuesto, tiposdeimpuestos.nombre AS Nombre, ";
	instruccion += "  impuestos.porcentaje AS Porcentaje, ";
	instruccion += "  if(impuestos.activo=1,'Sí','No') AS activo ";
	instruccion += "from impuestos, tiposdeimpuestos ";
	instruccion +=
		"where impuestos.tipoimpu=tiposdeimpuestos.tipoimpu order by tiposdeimpuestos.nombre, impuestos.porcentaje";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los tipos de impuestos
	instruccion =
		"select tipoimpu AS TipoImpuesto, nombre AS Nombre from tiposdeimpuestos order by nombre";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los productos con impuestos inactivos.
	instruccion = "SELECT lista.* from \
		((SELECT p.producto, p.nombre, p.marca, i.impuesto, i.tipoimpu, i.porcentaje FROM productos p \
		INNER JOIN impuestos i ON i.impuesto=p.claveimpc1 AND i.activo=0) \
		UNION \
		(SELECT p.producto, p.nombre, p.marca, i.impuesto, i.tipoimpu, i.porcentaje FROM productos p \
		INNER JOIN impuestos i ON i.impuesto=p.claveimpc2 AND i.activo=0) \
		UNION \
		(SELECT p.producto, p.nombre, p.marca, i.impuesto, i.tipoimpu, i.porcentaje FROM productos p \
		INNER JOIN impuestos i ON i.impuesto=p.claveimpc3 AND i.activo=0) \
		UNION \
		(SELECT p.producto, p.nombre, p.marca, i.impuesto, i.tipoimpu, i.porcentaje FROM productos p \
		INNER JOIN impuestos i ON i.impuesto=p.claveimpc4 AND i.activo=0) \
		UNION \
		(SELECT p.producto, p.nombre, p.marca, i.impuesto, i.tipoimpu, i.porcentaje FROM productos p \
		INNER JOIN impuestos i ON i.impuesto=p.claveimpv1 AND i.activo=0) \
		UNION \
		(SELECT p.producto, p.nombre, p.marca, i.impuesto, i.tipoimpu, i.porcentaje FROM productos p \
		INNER JOIN impuestos i ON i.impuesto=p.claveimpv2 AND i.activo=0) \
		UNION \
		(SELECT p.producto, p.nombre, p.marca, i.impuesto, i.tipoimpu, i.porcentaje FROM productos p \
		INNER JOIN impuestos i ON i.impuesto=p.claveimpv3 AND i.activo=0) \
		UNION \
		(SELECT p.producto, p.nombre, p.marca, i.impuesto, i.tipoimpu, i.porcentaje FROM productos p \
		INNER JOIN impuestos i ON i.impuesto=p.claveimpv4 AND i.activo=0)) lista \
		order by lista.nombre, lista.impuesto ";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}

// ---------------------------------------------------------------------------

// MONEDAS

// ---------------------------------------------------------------------------
// ID_GRA_MONEDA
void ServidorCatalogos::GrabaMoneda(RespuestaServidor *Respuesta, MYSQL *MySQL,
	char *parametros) {
	// GRABA MONEDA
	GrabaGenerico(Respuesta, MySQL, parametros, "monedas", "moneda");
}

// ---------------------------------------------------------------------------
// ID_BAJ_MONEDA
void ServidorCatalogos::BajaMoneda(RespuestaServidor *Respuesta, MYSQL *MySQL,
	char *parametros) {
	// BAJA MONEDA
	BajaGenerico(Respuesta, MySQL, parametros, "monedas", "moneda");
}

// ---------------------------------------------------------------------------
// ID_CON_MONEDA
void ServidorCatalogos::ConsultaMoneda(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA MONEDA
	AnsiString instruccion;
	AnsiString clave;

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos de la moneda
	instruccion.sprintf("select * from monedas where moneda='%s'", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todas las monedas
	instruccion =
		"select moneda AS Moneda, nombre AS Nombre from monedas order by nombre";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}

// ---------------------------------------------------------------------------

// FOLIOS

// ---------------------------------------------------------------------------
// ID_GRA_FOLIO
void ServidorCatalogos::GrabaFolio(RespuestaServidor *Respuesta, MYSQL *MySQL,
	char *parametros) {
	// GRABA FOLIO
	char *buffer_sql = new char[1024 * 100];
	char *aux_buffer_sql = buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);

	int num_instrucciones = 0;
	int i = 0;
	AnsiString instruccion, instrucciones[100];
    AnsiString tarea, clave, descripcion, valor, sucursal;

	tarea = mFg.ExtraeStringDeBuffer(&parametros);
	clave = mFg.ExtraeStringDeBuffer(&parametros);
	descripcion = mFg.ExtraeStringDeBuffer(&parametros);
	valor=mFg.ExtraeStringDeBuffer(&parametros);
	sucursal = mFg.ExtraeStringDeBuffer(&parametros);

	try {
		instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++] = "START TRANSACTION";

		if (tarea == "A"){
		   instruccion.sprintf("INSERT INTO foliosemp                                                \
								SELECT '%s' AS folio, sucursal, '%s' AS descripcion, '%s' AS valor    \
								FROM foliosemp GROUP BY sucursal ",clave, descripcion,valor );
		   instrucciones[num_instrucciones++] = instruccion;
		}else{

			instruccion.sprintf("UPDATE foliosemp SET descripcion='%s', valor='%s' WHERE folio='%s' AND sucursal = '%s' ", descripcion, valor, clave, sucursal);
			instrucciones[num_instrucciones++] = instruccion;
		}

		instrucciones[num_instrucciones++] = "COMMIT";

		aux_buffer_sql = mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);

		for (i = 0; i < num_instrucciones; i++){
		  aux_buffer_sql = mFg.AgregaStringABuffer(instrucciones[i],aux_buffer_sql);
		}

		mServidorVioleta->InicializaBuffer(Respuesta,TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,buffer_sql);

	}__finally {
		delete buffer_sql;
	}
}

// ---------------------------------------------------------------------------
// ID_BAJ_FOLIO
void ServidorCatalogos::BajaFolio(RespuestaServidor *Respuesta, MYSQL *MySQL,
	char *parametros) {
	// BAJA FOLIO
	
	char *buffer_sql = new char[1024 * 5];
	char *aux_buffer_sql = buffer_sql;
	AnsiString instruccion;
	int num_instrucciones = 1;
	int i = 0;
	AnsiString instrucciones[100];
	AnsiString clave, sucursal;

	clave=mFg.ExtraeStringDeBuffer(&parametros);
	sucursal = mFg.ExtraeStringDeBuffer(&parametros);

	try {

		instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++] = "START TRANSACTION";

		instruccion.sprintf("DELETE FROM foliosemp WHERE folio='%s' AND sucursal = '%s' ", clave, sucursal);
		instrucciones[num_instrucciones++] = instruccion;

		instrucciones[num_instrucciones++] = "COMMIT";

		aux_buffer_sql = mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);

		for (i = 0; i < num_instrucciones; i++){
		  aux_buffer_sql = mFg.AgregaStringABuffer(instrucciones[i],aux_buffer_sql);
		}

		mServidorVioleta->InicializaBuffer(Respuesta,TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,buffer_sql);
	}
	__finally {
		delete buffer_sql;
	}
}

// ---------------------------------------------------------------------------
// ID_CON_FOLIO
void ServidorCatalogos::ConsultaFolio(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA FOLIO
	AnsiString instruccion;
	AnsiString clave, sucursal, sucursal_filtro;
	AnsiString condicion_sucursal = " ";

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	sucursal = mFg.ExtraeStringDeBuffer(&parametros);
	sucursal_filtro = mFg.ExtraeStringDeBuffer(&parametros);

	if(sucursal_filtro != " "){
		condicion_sucursal.sprintf( " AND sucursal = '%s' ", sucursal_filtro );
	}

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos del folio
	instruccion.sprintf("select * from foliosemp where folio='%s' AND sucursal = '%s' ",clave, sucursal);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los folios
	instruccion.sprintf(
		" select folio AS Folio, descripcion AS Descripcion, valor AS Valor, sucursal AS Sucursal from foliosemp WHERE 1 %s order by sucursal ", condicion_sucursal);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}

// ---------------------------------------------------------------------------

// PARAMETROS

// ---------------------------------------------------------------------------
// ID_GRA_PARAMETRO
void ServidorCatalogos::GrabaParametro(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA PARAMETRO

	char *buffer_sql = new char[1024 * 100];
	char *aux_buffer_sql = buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);

	int num_instrucciones = 0;
    int i = 0;
	AnsiString instruccion, instrucciones[100];
    AnsiString tarea, clave, descripcion, valor, sucursal;

	tarea = mFg.ExtraeStringDeBuffer(&parametros);
	clave = mFg.ExtraeStringDeBuffer(&parametros);
	descripcion = mFg.ExtraeStringDeBuffer(&parametros);
	valor=mFg.ExtraeStringDeBuffer(&parametros);
	sucursal = mFg.ExtraeStringDeBuffer(&parametros);

	try {
		instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++] = "START TRANSACTION";

		if (tarea == "A"){

		   instruccion.sprintf("INSERT INTO parametrosemp                                                \
								SELECT '%s' AS parametro, sucursal, '%s' AS descripcion, '%s' AS valor    \
								FROM parametrosemp GROUP BY sucursal ",clave, descripcion,valor );
		   instrucciones[num_instrucciones++] = instruccion;

		}else{
		   instruccion.sprintf("UPDATE parametrosemp SET descripcion='%s', valor='%s' WHERE parametro='%s' AND sucursal = '%s' ", descripcion, valor, clave, sucursal);
		   instrucciones[num_instrucciones++] = instruccion;
		}

		instrucciones[num_instrucciones++] = "COMMIT";

		aux_buffer_sql = mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);

		for (i = 0; i < num_instrucciones; i++){
		  aux_buffer_sql = mFg.AgregaStringABuffer(instrucciones[i],aux_buffer_sql);
		}

		mServidorVioleta->InicializaBuffer(Respuesta,TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,buffer_sql);

	}__finally {
		delete buffer_sql;
	}
}

// ---------------------------------------------------------------------------
// ID_BAJ_PARAMETRO
void ServidorCatalogos::BajaParametro(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// BAJA PARAMETRO

	char *buffer_sql = new char[1024 * 5];
	char *aux_buffer_sql = buffer_sql;
	AnsiString instruccion;
	int num_instrucciones = 1;
	int i = 0;
	AnsiString instrucciones[100];
	AnsiString clave, sucursal;

	clave=mFg.ExtraeStringDeBuffer(&parametros);
	sucursal = mFg.ExtraeStringDeBuffer(&parametros);

	try {

		instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++] = "START TRANSACTION";

		instruccion.sprintf("DELETE FROM parametrosemp WHERE parametro='%s' AND sucursal = '%s' ", clave, sucursal);
		instrucciones[num_instrucciones++] = instruccion;

		instrucciones[num_instrucciones++] = "COMMIT";

		aux_buffer_sql = mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);

		for (i = 0; i < num_instrucciones; i++){
		  aux_buffer_sql = mFg.AgregaStringABuffer(instrucciones[i],aux_buffer_sql);
		}

		mServidorVioleta->InicializaBuffer(Respuesta,TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,buffer_sql);
	}
	__finally {
		delete buffer_sql;
	}
}

// ---------------------------------------------------------------------------
// ID_CON_PARAMETRO
void ServidorCatalogos::ConsultaParametro(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA PARAMETRO
	AnsiString instruccion;
	AnsiString clave, indiciparametro, sucursal, sucursalFiltro;
	AnsiString condicion_indiciparametro = " ", condicion_sucursal = " ";

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	indiciparametro = mFg.ExtraeStringDeBuffer(&parametros);
	sucursal = mFg.ExtraeStringDeBuffer(&parametros);
	sucursalFiltro = mFg.ExtraeStringDeBuffer(&parametros);

	if (indiciparametro!=" ") {
	   condicion_indiciparametro.sprintf(" AND (Parametro LIKE '%%%s%%' OR Descripcion LIKE '%%%s%%' OR Valor LIKE '%%%s%%' OR Sucursal LIKE '%%%s%%' ) ", indiciparametro, indiciparametro, indiciparametro, indiciparametro);
	}

	if(sucursalFiltro != " "){
		condicion_sucursal.sprintf( " AND sucursal = '%s' ", sucursalFiltro );
	}

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos del parametro
	instruccion.sprintf("select * from parametrosemp where parametro='%s' AND sucursal = '%s' ", clave, sucursal);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	if (sucursal != " " && sucursalFiltro != " ") {
		// Obtiene todos los parametros
		instruccion.sprintf("select parametro AS Parametro, descripcion AS Descripcion, valor AS Valor, sucursal AS Sucursal from parametrosemp WHERE 1 %s %s order by sucursal", condicion_indiciparametro, condicion_sucursal);
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	} else {
		instruccion.sprintf("select parametro AS Parametro, descripcion AS Descripcion, valor AS Valor, sucursal AS Sucursal from parametrosemp WHERE null");
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	}

}

// ---------------------------------------------------------------------------

// ID_GRA_PARAMETROGLOB
void ServidorCatalogos::GrabaParametroGlobal(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA PARAMETRO
	char *buffer_sql = new char[1024 * 100];
	char *aux_buffer_sql = buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);

	int num_instrucciones = 0;
    int i = 0;
	AnsiString instruccion, instrucciones[100];
	AnsiString tarea, clave, descripcion, valor, empresa;

	tarea = mFg.ExtraeStringDeBuffer(&parametros);
	clave = mFg.ExtraeStringDeBuffer(&parametros);
	descripcion = mFg.ExtraeStringDeBuffer(&parametros);
	valor=mFg.ExtraeStringDeBuffer(&parametros);
	empresa = mFg.ExtraeStringDeBuffer(&parametros);

	try {
		instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++] = "START TRANSACTION";

		if (tarea == "A"){
		   instruccion.sprintf("INSERT INTO parametrosglobemp                                                \
								SELECT '%s' AS parametro, idempresa, '%s' AS descripcion, '%s' AS valor    \
								FROM parametrosglobemp GROUP BY idempresa ",clave, descripcion,valor );
		   instrucciones[num_instrucciones++] = instruccion;

		}else{
		   instruccion.sprintf("UPDATE parametrosglobemp SET descripcion='%s', valor='%s' WHERE parametro='%s' AND idempresa = '%s' ", descripcion, valor, clave, empresa);
		   instrucciones[num_instrucciones++] = instruccion;
		}

		instrucciones[num_instrucciones++] = "COMMIT";

		aux_buffer_sql = mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);

		for (i = 0; i < num_instrucciones; i++){
		  aux_buffer_sql = mFg.AgregaStringABuffer(instrucciones[i],aux_buffer_sql);
		}

		mServidorVioleta->InicializaBuffer(Respuesta,TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,buffer_sql);

	}__finally {
		delete buffer_sql;
	}
}
// ---------------------------------------------------------------------------

// ID_CON_PARAMETROGLOB
void ServidorCatalogos::ConsultaParametroGlobal(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA PARAMETRO
	AnsiString instruccion;
	AnsiString clave, indiciparametro, empresa, empresaFiltro;
	AnsiString condicion_indiciparametro = " ",  condicion_empresa = " ";

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	indiciparametro = mFg.ExtraeStringDeBuffer(&parametros);
	empresa = mFg.ExtraeStringDeBuffer(&parametros);
	empresaFiltro = mFg.ExtraeStringDeBuffer(&parametros);

	if (indiciparametro!=" ") {
	   condicion_indiciparametro.sprintf(" AND (Parametro LIKE '%%%s%%' OR Descripcion LIKE '%%%s%%' OR Valor LIKE '%%%s%%' ) ",
	   indiciparametro, indiciparametro, indiciparametro);
	}

	if(empresaFiltro != " "){
		condicion_empresa.sprintf( " AND p.idempresa = %s ", empresaFiltro );
	}

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos del parametro
	instruccion.sprintf("select * from parametrosglobemp where parametro='%s' AND idempresa=%s ", clave, empresa);

	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los parametros
	if (empresa != " " && empresaFiltro != " ") {
        instruccion.sprintf(
		"SELECT p.parametro AS Parametro, p.descripcion AS Descripcion, p.valor AS Valor, p.idempresa AS idEmpresa, e.nombre AS Nombre  \
		from parametrosglobemp p  \
		INNER JOIN empresas e ON e.idempresa = p.idempresa   \
		where 1 %s %s order BY p.idempresa, parametro", condicion_indiciparametro, condicion_empresa);
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	} else {
        instruccion.sprintf(
		"SELECT p.parametro AS Parametro, p.descripcion AS Descripcion, p.valor AS Valor, p.idempresa AS idEmpresa, e.nombre AS Nombre  \
		from parametrosglobemp p  \
		INNER JOIN empresas e ON e.idempresa = p.idempresa   \
		where null");
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
    }

}
// ---------------------------------------------------------------------------

// MARCAS

// ---------------------------------------------------------------------------
// ID_GRA_MARCA
void ServidorCatalogos::GrabaMarca(RespuestaServidor *Respuesta, MYSQL *MySQL,
	char *parametros) {
	// GRABA MARCA
	GrabaGenerico(Respuesta, MySQL, parametros, "marcas", "marca");
}

// ---------------------------------------------------------------------------
// ID_BAJ_MARCA
void ServidorCatalogos::BajaMarca(RespuestaServidor *Respuesta, MYSQL *MySQL,
	char *parametros) {
	char *buffer_sql = new char[1024 * 5];
	char *aux_buffer_sql = buffer_sql;
	AnsiString instruccion;
	int num_instrucciones = 5;
	AnsiString marca;

	try{
		marca = mFg.ExtraeStringDeBuffer(&parametros);
		aux_buffer_sql = mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);

		instruccion = "SET AUTOCOMMIT = 0";
		aux_buffer_sql = mFg.AgregaStringABuffer(instruccion, aux_buffer_sql);

        instruccion = "START TRANSACTION";
		aux_buffer_sql = mFg.AgregaStringABuffer(instruccion, aux_buffer_sql);

		instruccion.sprintf("DELETE FROM marcasexistsim WHERE marca = '%s'", marca);
		aux_buffer_sql = mFg.AgregaStringABuffer(instruccion, aux_buffer_sql);

		instruccion.sprintf("DELETE FROM marcas WHERE marca = '%s' ", marca);
		aux_buffer_sql = mFg.AgregaStringABuffer(instruccion, aux_buffer_sql);

        instruccion = "COMMIT";
		mFg.AgregaStringABuffer(instruccion, aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);

	}__finally{
		delete buffer_sql;
	}
}

// ---------------------------------------------------------------------------
// ID_CON_MARCA
void ServidorCatalogos::ConsultaMarca(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA MARCA
	AnsiString instruccion;
	AnsiString clave;

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos de la marca
	instruccion.sprintf("select * from marcas where marca='%s'", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todas las marcas
	instruccion =
		"select marca AS Marca, nombre AS Nombre from marcas order by nombre";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}

// ---------------------------------------------------------------------------
// ID_APLICA_CONCEPTOS_VENTA
void ServidorCatalogos::AplicaConceptosVenta(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	char *buffer_sql = new char[1024 * 32];
	char *aux_buffer_sql = buffer_sql;
	AnsiString ClaveQuitar, ClaveBuscar, EmpresaQuitar, EmpresaBuscar, instruccion, instrucciones[50],
	minimo, maximo;
	int TotalQuitar, TotalRegistros, num_instrucciones = 0, i;
	DatosTabla datos(mServidorVioleta->Tablas);
	BufferRespuestas* resp_verificacion = NULL;

	try {
		instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++] = "START TRANSACTION";

		TotalQuitar = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
		for (i = 0; i < TotalQuitar; i++) {
			ClaveQuitar = mFg.ExtraeStringDeBuffer(&parametros);
			EmpresaQuitar = mFg.ExtraeStringDeBuffer(&parametros);
			instruccion.sprintf("delete from dconceptosventa where clave='%s'",
				ClaveQuitar);
			instrucciones[num_instrucciones++] = instruccion;
			instruccion.sprintf("delete from conceptosventa where clave='%s' and idempresa=%s ",
				ClaveQuitar, EmpresaQuitar);
			instrucciones[num_instrucciones++] = instruccion;
		}

		TotalRegistros = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
		for (i = 0; i < TotalRegistros; i++) {
			datos.AsignaTabla("conceptosventa");
			parametros += datos.InsCamposDesdeBuffer(parametros);
			ClaveBuscar = datos.ObtieneValorCampo("clave");
			EmpresaBuscar = datos.ObtieneValorCampo("idempresa");
			minimo = datos.ObtieneValorCampo("mindef");
			maximo = datos.ObtieneValorCampo("maxdef");
			instruccion.sprintf
				("select * from conceptosventa where clave='%s' and idempresa=%s ", ClaveBuscar, EmpresaBuscar);
			mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,
				instruccion.c_str(), resp_verificacion);
			if (resp_verificacion->ObtieneNumRegistros() != 0) {
				instrucciones[num_instrucciones++] = datos.GenerarSqlUpdate
					("clave='" + ClaveBuscar + "'" + " and idempresa="+EmpresaBuscar);
				instrucciones[num_instrucciones++].sprintf(
					"update dconceptosventa set cantmin='%s',cantmax='%s' where clave='%s' and esdefault='1'", minimo, maximo, ClaveBuscar);
			}
			else {
				instrucciones[num_instrucciones++] = datos.GenerarSqlInsert();
				instrucciones[num_instrucciones++].sprintf(
					"insert into dconceptosventa(producto,present,clave,cantmin,cantmax) \
					SELECT P.producto, P.present, c.clave, c.mindef, c.maxdef FROM presentaciones p, \
					conceptosventa c WHERE c.clave='%s' AND c.idempresa=%s ",
						datos.ObtieneValorCampo("clave"), EmpresaBuscar );
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
		if (resp_verificacion != NULL)
			delete resp_verificacion;
		delete buffer_sql;
	}
}

// ---------------------------------------------------------------------------
// ID_CON_CONCEPTOS_VENTA
void ServidorCatalogos::ConsultaConceptosVenta(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA CONCEPTO
	AnsiString instruccion;
	AnsiString clave, empresa;
    AnsiString condicion_clave = " ", condicion_empresa = " ";

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	empresa = mFg.ExtraeStringDeBuffer(&parametros);

	if (clave!="") {
		condicion_clave.sprintf(" and c.clave = '%s' ", clave);
	}

	if (empresa != "") {
		condicion_empresa.sprintf(" and c.idempresa = %s ", empresa);
	}

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos del concepto de venta
	instruccion.sprintf("select * from conceptosventa c where c.clave='%s' %s ",
	clave, condicion_empresa);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los conceptos de venta
	instruccion.sprintf("select c.clave, c.concepto, c.mindef, c.maxdef, c.tipoprec, tp.descripcion, c.imprimir \
							from conceptosventa c \
							left join tiposdeprecios tp on tp.tipoprec=c.tipoprec \
							where 1 %s %s \
							order by mindef", condicion_empresa, condicion_clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}

// ---------------------------------------------------------------------------

// CLASIFICACION1

// ---------------------------------------------------------------------------
// ID_GRA_CLASIFICACION1
void ServidorCatalogos::GrabaClasificacion1(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA CLASIFICACION1
	GrabaGenerico(Respuesta, MySQL, parametros, "clasificacion1", "clasif1");
}

// ---------------------------------------------------------------------------
// ID_BAJ_CLASIFICACION1
void ServidorCatalogos::BajaClasificacion1(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// BAJA CLASIFICACION1
	BajaGenerico(Respuesta, MySQL, parametros, "clasificacion1", "clasif1");
}

// ---------------------------------------------------------------------------
// ID_CON_CLASIFICACION1
void ServidorCatalogos::ConsultaClasificacion1(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA CLASIFICACION1
	ConsultaGenerico(Respuesta, MySQL, parametros, "clasificacion1", "clasif1");
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

// CLASIFICACION2

// ---------------------------------------------------------------------------
// ID_GRA_CLASIFICACION2
void ServidorCatalogos::GrabaClasificacion2(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA CLASIFICACION2
	GrabaGenerico(Respuesta, MySQL, parametros, "clasificacion2", "clasif2");
}

// ---------------------------------------------------------------------------
// ID_BAJ_CLASIFICACION2
void ServidorCatalogos::BajaClasificacion2(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// BAJA CLASIFICACION2
	BajaGenerico(Respuesta, MySQL, parametros, "clasificacion2", "clasif2");
}

// ---------------------------------------------------------------------------
// ID_CON_CLASIFICACION2
void ServidorCatalogos::ConsultaClasificacion2(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA CLASIFICACION2
	ConsultaGenerico(Respuesta, MySQL, parametros, "clasificacion2", "clasif2");
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

// CLASIFICACION3

// ---------------------------------------------------------------------------
// ID_GRA_CLASIFICACION3
void ServidorCatalogos::GrabaClasificacion3(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA CLASIFICACION3
	GrabaGenerico(Respuesta, MySQL, parametros, "clasificacion3", "clasif3");
}

// ---------------------------------------------------------------------------
// ID_BAJ_CLASIFICACION3
void ServidorCatalogos::BajaClasificacion3(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// BAJA CLASIFICACION3
	BajaGenerico(Respuesta, MySQL, parametros, "clasificacion3", "clasif3");
}

// ---------------------------------------------------------------------------
// ID_CON_CLASIFICACION3
void ServidorCatalogos::ConsultaClasificacion3(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA CLASIFICACION3
	ConsultaGenerico(Respuesta, MySQL, parametros, "clasificacion3", "clasif3");
}
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

// ARTICULO

// ---------------------------------------------------------------------------
// ID_GRA_ARTICULO
void ServidorCatalogos::GrabaArticulo(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA ARTICULO
	char *buffer_sql = new char[1024 * 100];
	char *aux_buffer_sql = buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString tarea_articulo, tarea_producto;  //tarea_producto-> alta o modificacion
	AnsiString tarea_presentacion, tarea_precios;
	AnsiString clave_articulo, clave_producto, clave_present, clave_multiplo;
	AnsiString clave_presentacion_prod, clave_presentacion_pres;
	AnsiString usuario, almacen;
	int i, x;
	int num_instrucciones = 0;
	int num_precios;
	AnsiString instruccion, instrucciones[210];
	bool cambiar_utilidad_multiplos = false;
	AnsiString modoutil;
	AnsiString costobase;
	AnsiString porcutil, tipoprec, producto, present, utilesp;
	AnsiString precioAsig;
    AnsiString precioMod;
	AnsiString comisionant,costobaseant,activo,activoant, comision;
	AnsiString select;
	AnsiString producto_ea, present_ea;
	int num_sectores;
	AnsiString listaSegmentos;
	AnsiString articuloSegmento;
	AnsiString unicoLista;
	TStringList* mListaSegmentos = new TStringList();
	AnsiString excepcionpreciosbloqueados;
	AnsiString supervisado, empleadosupervisor;
	double porcutilantes=0;
	AnsiString mListaPorcentajes,MayorListaPorcentaj, mListaPorcentajesActual,MayorListaPorcenActual;
	int grabados=0;
	TStringList *mListaPorcentManual = new TStringList();
    TStringList *mListaPorcentManActual = new TStringList();
	AnsiString valorcampoPorcComi="";
	double peso, volumen, pesoant=0, volumenant=0;
	double factor;
	AnsiString empresa;
	AnsiString redondea_precios;

	AnsiString campo_precios;
	AnsiString precio = "p.precioproximo";
	AnsiString condicion_producto;

	AnsiString desde1="0.01", hasta1="0.24", redondeo1="0.00";
	AnsiString desde2="0.25", hasta2="0.49", redondeo2="0.50";
	AnsiString desde3="0.50", hasta3="0.74", redondeo3="0.50";
	AnsiString desde4="0.75", hasta4="0.99", redondeo4="0.90";

	BufferRespuestas* resp_suc=NULL;
	BufferRespuestas* resp_redondeo_param=NULL;
	BufferRespuestas* resp_productos=NULL;


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
		instruccion.sprintf("SELECT valor FROM parametrosglobemp WHERE parametro='REDONPREDIF' AND idempresa = %s ",
		FormServidor->ObtieneClaveEmpresa());
		if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_redondeo_param)) {
			if (resp_redondeo_param->ObtieneNumRegistros()>0){
				redondea_precios=resp_redondeo_param->ObtieneDato("valor");
			}
		}
	} __finally {
		if (resp_redondeo_param!=NULL) delete resp_redondeo_param;
	}

	try {
		instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++] = "START TRANSACTION";

		// Grabar producto?
		tarea_producto = mFg.ExtraeStringDeBuffer(&parametros);
		clave_producto = mFg.ExtraeStringDeBuffer(&parametros);
		empresa = mFg.ExtraeStringDeBuffer(&parametros);
		usuario = mFg.ExtraeStringDeBuffer(&parametros);
		excepcionpreciosbloqueados=mFg.ExtraeStringDeBuffer(&parametros); // Si se hace excepción de permitir editar precios bloqueados.
		supervisado = mFg.ExtraeStringDeBuffer(&parametros);
		empleadosupervisor = mFg.ExtraeStringDeBuffer(&parametros);

		datos.AsignaTabla("productos");
		parametros += datos.InsCamposDesdeBuffer(parametros);
		if (tarea_producto == "A"){
			instrucciones[num_instrucciones++] = datos.GenerarSqlInsert();

			// Consulta si los últimos 5 dígitos no estan repetidos
			// * Si es mayor que 0 encontro un articulo dado de alta con los digitos del producto que se dará de alta y manda una excepcion
			// * si es 1 entonces se da de ealta el rrpdocuto
			AnsiString paramverificaart;
			BufferRespuestas* resp_verificaart=NULL;
			try {
				instruccion.sprintf("SELECT CAST(RIGHT(articulo,5) AS UNSIGNED) AS secarticulo FROM articulos WHERE \
				CAST(RIGHT(articulo,5) AS UNSIGNED) = (SELECT valor FROM foliosemp AS f WHERE folio = 'ARTICULOS' AND f.sucursal = '%s' )",FormServidor->ObtieneClaveSucursal());
				if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_verificaart)) {
					if (resp_verificaart->ObtieneNumRegistros()>0){
					   throw (Exception("Un artículo con esos digitos ya esta registrado"));
					   }
				} else throw (Exception("Error al consultar en tabla articulos"));
			} __finally {
				if (resp_verificaart!=NULL) delete resp_verificaart;
			}

		}
		else
			instrucciones[num_instrucciones++] = datos.GenerarSqlUpdate
				("producto='" + clave_producto + "'");



		// Grabar presentacion?
		tarea_presentacion = mFg.ExtraeStringDeBuffer(&parametros);
		clave_presentacion_prod = mFg.ExtraeStringDeBuffer(&parametros);
		clave_presentacion_pres = mFg.ExtraeStringDeBuffer(&parametros);
		datos.AsignaTabla("presentaciones");
		parametros += datos.InsCamposDesdeBuffer(parametros);
		if (tarea_presentacion == "A"){
			instrucciones[num_instrucciones++] = datos.GenerarSqlInsert();

			producto_ea = datos.ObtieneValorCampo("producto");
			present_ea = datos.ObtieneValorCampo("present");

			/*Agregar producto y presentacion a existencias actuales si no existe*/

			instruccion.sprintf("insert into existenciasactuales (producto, present, almacen, cantidad, ventas \
			, devventas, compras, devcompras, entradas, salidas ) \
			SELECT '%s' AS producto, '%s' AS present, al.almacen, \
			IFNULL(ea.cantidad,0.000) AS cantidad,IFNULL(ea.ventas,0.000) AS ventas,   \
			IFNULL(ea.devventas,0.000) AS devventas, IFNULL(ea.compras,0.000) AS compras, \
			IFNULL(ea.devcompras,0.000) AS devcompras ,IFNULL(ea.entradas,0.000) AS entradas, \
			IFNULL(ea.salidas,0.000) AS salidas FROM almacenes al LEFT JOIN existenciasactuales ea \
			ON ea.producto = '%s' AND ea.present = '%s' AND al.almacen = ea.almacen",
			producto_ea, present_ea,producto_ea, present_ea );
			instrucciones[num_instrucciones++]=instruccion;

		}
		else
			instrucciones[num_instrucciones++] = datos.GenerarSqlUpdate
				("producto='" + clave_presentacion_prod + "'" + " and " +
			"present='" + clave_presentacion_pres + "'");

		modoutil = datos.ObtieneValorCampo("modoutil");


		//Presentacionescb
		datos.AsignaTabla("presentacionescb");
		parametros += datos.InsCamposDesdeBuffer(parametros);

		if(tarea_presentacion == "A"){
			AnsiString instruccion;
			BufferRespuestas* resp_emp=NULL;

			try {
				instruccion = "SELECT idempresa FROM empresas ";

				if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_emp)) {

					for(int i=0; i<resp_emp->ObtieneNumRegistros(); i++){
						resp_emp->IrAlRegistroNumero(i);

						instruccion.sprintf("INSERT IGNORE INTO presentacionescb (idempresa, producto, present, costobase, segmento) VALUES  \
						(%s,'%s','%s',%s, '%s')",
						resp_emp->ObtieneDato("idempresa"), datos.ObtieneValorCampo("producto"), datos.ObtieneValorCampo("present"),
						datos.ObtieneValorCampo("costobase"), datos.ObtieneValorCampo("segmento") );
						instrucciones[num_instrucciones++]=instruccion;
					}

				} else
					throw (Exception("Error al consultar la tabla empresas"));
			} __finally {
				if (resp_emp!=NULL) delete resp_emp;
			}

		}else{
			instrucciones[num_instrucciones++] = datos.GenerarSqlUpdate
			("producto='" + clave_presentacion_prod + "'" + " and " + "present='" + clave_presentacion_pres +
			 "' and idempresa=" + empresa);
		}


		costobase = datos.ObtieneValorCampo("costobase");
		costobaseant = datos.ObtieneValorCampo("costobase");

		if (modoutil == "0")
			cambiar_utilidad_multiplos = true;

		// Grabar articulo?
		tarea_articulo = mFg.ExtraeStringDeBuffer(&parametros);
		clave_articulo = mFg.ExtraeStringDeBuffer(&parametros);

		//obtienes valores para la bitacora
		datos.AsignaTabla("articulos");

		parametros += datos.InsCamposDesdeBuffer(parametros);

		//nuevos datos de la tabla fechamodi,horamodi,usumodi
		datos.InsCampo("fechamodi", mFg.DateToAnsiString(Today()));
		datos.InsCampo("horamodi", mFg.TimeToAnsiString(Time()));
		datos.InsCampo("usumodi", usuario);

		datos.AsignaValorCampo("articulo", "@folio", 1);
		//utilesp = datos.ObtieneValorCampo("tipoutil");
		clave_multiplo = datos.ObtieneValorCampo("multiplo");
		activo = datos.ObtieneValorCampo("activo");

		factor= StrToFloat(datos.ObtieneValorCampo("factor"));
		peso=mFg.CadenaAFlotante(datos.ObtieneValorCampo("peso"));
		volumen=mFg.CadenaAFlotante(datos.ObtieneValorCampo("volumen"));

		if (tarea_articulo == "A") {

			//datos del alta
			datos.InsCampo("usualta", usuario);
			datos.InsCampo("fechaalta", mFg.DateToAnsiString(Today()));
			datos.InsCampo("horaalta", mFg.TimeToAnsiString(Time()));

			instruccion.sprintf(
				"select @folioaux:=valor from foliosemp where folio='ARTICULOS' AND sucursal = '%s' %s ", FormServidor->ObtieneClaveSucursal()
				, MODO_BLOQUEO_CLAVES_UNICAS);
			instrucciones[num_instrucciones++] = instruccion;
			instruccion.sprintf("set @foliosig=@folioaux+1");
			instrucciones[num_instrucciones++] = instruccion;
			instruccion.sprintf("set @folioaux=cast(@folioaux as char)");
			instrucciones[num_instrucciones++] = instruccion;
			instruccion.sprintf
				("set @folio=concat('%s', lpad(@folioaux,7,'0'))", FormServidor->ObtieneClaveSucursal());
			instrucciones[num_instrucciones++] = instruccion;
			instruccion.sprintf(
				"update foliosemp set valor=@foliosig where folio='ARTICULOS' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
			instrucciones[num_instrucciones++] = instruccion;

			instrucciones[num_instrucciones++] = datos.GenerarSqlInsert();

			instruccion.sprintf("select @producto_a:=producto, @presentacion_a:=present from articulos where articulo=@folio");
			instrucciones[num_instrucciones++]=instruccion;

			// Crea una relación con ARTICULOXSECCION
			instruccion.sprintf("replace into articuloxseccion (articulo,seccion,almacen) \
				select a.articulo, s.seccion, if(a.factor=1,s.almaunid,s.almamult) as almacen \
				from secciones s, articulos a \
				where a.articulo=@folio");
			instrucciones[num_instrucciones++] = instruccion;

			instrucciones[num_instrucciones++].sprintf(
				"replace into dconceptosventa (producto,present,clave,cantmin,cantmax) \
				SELECT '%s' as producto, '%s' as present, c.clave, c.mindef, c.maxdef FROM conceptosventa c "
				, clave_presentacion_prod, clave_presentacion_pres);

			comision = datos.ObtieneValorCampo("porcComi");
			if(comision == NULL){
				 comision = 0.00;
			}
			instruccion.sprintf("INSERT INTO bitacoraart (idBitacoraArt, articulo, idempresa, usuario, fecha, hora, tipo,\
			costoBase,comision,activo) values (NULL,@folio, %s,'%s','%s','%s','%s',%s,%s, 1)",empresa,usuario,mFg.DateToMySqlDate(Today()),
			 mFg.TimeToMySqlTime(Time()),tarea_articulo,costobase,datos.ObtieneValorCampo("porcComi"));
			instrucciones[num_instrucciones++] = instruccion;

			instruccion.sprintf("INSERT INTO bitacoracostos  VALUES ('CATALOGO',@folio,'CATA','%s','%s',CURDATE(),CURTIME(),'%s', %s) "
			,costobaseant,costobase,usuario, empresa);

			instrucciones[num_instrucciones++] = instruccion;

			//Se agrega la configuración de calculador de pedidos por defecto a los art
            // Inserta los elementos con los datos actualizados para todas las sucursales.
			instruccion.sprintf(
				" INSERT IGNORE INTO articulosped (sucursal, producto, present, proveedor, claveproductoproveedor, multiplopedir, \
						duracionreorden, duracionmax, descontinuado, redondeocaja, stockminimo) \
					SELECT s.sucursal, '%s' AS producto, '%s' AS present, NULL AS proveedor, NULL AS claveproductoproveedor, \
					NULL AS multiplopedir, 7 AS duracionreorden, 15 AS duracionmax, \
					0 AS descontinuado, 1 AS redondeocaja, 1 as stockminimo \
					FROM sucursales s ",
					 clave_presentacion_prod, clave_presentacion_pres);
			instrucciones[num_instrucciones++] = instruccion;

		}
		else {

			if (clave_articulo != "")
				instruccion.sprintf(
					"select @folio:=articulo, @producto_a:=producto, @presentacion_a:=present from articulos where articulo='%s'", clave_articulo);
			else
				instruccion.sprintf(
					"select @folio:=articulo, @producto_a:=producto, @presentacion_a:=present from articulos where producto='%s' and present='%s' and multiplo='%s'", clave_producto, clave_presentacion_pres, clave_multiplo);
			instrucciones[num_instrucciones++] = instruccion;

			instrucciones[num_instrucciones++] = datos.GenerarSqlUpdate
				("articulo=@folio");

			select.sprintf("SELECT a.porccomi, a.activo, p.costo, \
				a.peso, a.volumen \
				FROM articulos a \
				INNER JOIN precios p ON a.articulo = p.articulo \
				INNER JOIN tiposdeprecios tp ON tp.tipoprec = p.tipoprec AND tp.idempresa=%s  \
				WHERE a.articulo= '%s' GROUP BY a.activo limit 1", empresa, clave_articulo);

			BufferRespuestas* resp_datos = NULL;

			try{

				mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, select.c_str(), resp_datos);

				resp_datos->IrAlRegistroNumero(0);
				comisionant = resp_datos->ObtieneDato("porccomi");
				activoant = resp_datos->ObtieneDato("activo");
				costobaseant = resp_datos->ObtieneDato("costo");
				valorcampoPorcComi	= datos.ObtieneValorCampo("porcComi");
				pesoant=mFg.CadenaAFlotante(resp_datos->ObtieneDato("peso"));
				volumenant=mFg.CadenaAFlotante(resp_datos->ObtieneDato("volumen"));
			}
			__finally{
				if(resp_datos!=NULL) delete resp_datos;
			}

			/*instruccion.sprintf("INSERT INTO bitacoraart (idBitacoraArt, articulo, usuario, fecha, hora,\
			tipo,costoBase,comision,costobaseant,activoant,activo,comisionant)values \    \
			(NULL,@folio,'%s','%s','%s','%s',%s,%s,%s,%s,%s,%s)",usuario,mFg.DateToMySqlDate(Today()),
			mFg.TimeToMySqlTime(Time()),tarea_articulo,costobase,datos.ObtieneValorCampo("porcComi"),
			costobaseant,activoant,activo,comisionant);
			instrucciones[num_instrucciones++] = instruccion; */

			instruccion.sprintf("INSERT INTO bitacoracostos  VALUES ('CATALOGO',@folio,'CATM','%s','%s',CURDATE(),CURTIME(),'%s', %s) "
			,costobaseant,costobase, usuario, empresa );
			instrucciones[num_instrucciones++] = instruccion;

			if(activoant == 1 && activo == 0){

			instruccion.sprintf("DELETE FROM articulosequiv where articulo = '%s'",clave_articulo);
			instrucciones[num_instrucciones++] = instruccion;
			}
		}

		// Consulta si no están bloqueados los cambios de precios para el artículo.
		bool bloqueadoprecio;
		if (excepcionpreciosbloqueados=="1") {
			// Si hay excepcion entonces no bloquea precios.
			bloqueadoprecio=false;
		} else {
			// Si no hay excepción de bloqueo de precios entonces nos basamos en la configuración de precios bloqueados del artículo.
			bloqueadoprecio=false;
			BufferRespuestas* resp_bloqueadoprecio=NULL;
			try {
				instruccion.sprintf("SELECT pb.producto, pb.present, pb.fechaVigencia \
						from preciosbloqueados pb where pb.producto='%s' and pb.present='%s' \
							 and pb.fechaVigencia>='%s' and pb.idempresa=%s ",
							 clave_presentacion_prod,clave_presentacion_pres,
							 mFg.DateToMySqlDate(Today()), FormServidor->ObtieneClaveEmpresa());
				if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_bloqueadoprecio)) {
					if (resp_bloqueadoprecio->ObtieneNumRegistros()>0){
						bloqueadoprecio=true;
					}
				} else throw (Exception("Error al consultar en tabla preciosbloqueados"));
			} __finally {
				if (resp_bloqueadoprecio!=NULL) delete resp_bloqueadoprecio;
			}
		}


		//  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		// Afectar los CAMBIOS A PESO en los multiplos de la misma presentación (donde aplique)
		if (  (tarea_producto == "A" && factor>1 && peso>0) ||
			  (tarea_producto == "M" && peso>0 && !mFg.SonIguales(peso, pesoant))   )
		{
			// Si es alta de factor>1 y peso>0 o si es modificación
			// Actualiza los productos con misma presentación que el dado como parámetro (excepto el mismo)
			instruccion.sprintf("UPDATE articulos SET peso = factor*(%s/%s) \
				WHERE producto='%s' AND present='%s' and articulo<>@folio",
					mFg.FormateaCantidad(peso,3,false), AnsiString(FloatToStr(factor)),
					clave_presentacion_prod, clave_presentacion_pres);
			instrucciones[num_instrucciones++]=instruccion;
		}

		//  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		// Afectar los CAMBIOS A VOLUMEN en los multiplos de la misma presentación (donde aplique)
		if (  (tarea_producto == "A" && factor>1 && volumen>0) ||
			  (tarea_producto == "M" && volumen>0 && !mFg.SonIguales(volumen, volumenant))   )
		{
			// Si es alta de factor>1 y volumen>0 o si es modificación
			// Actualiza los productos con misma presentación que el dado como parámetro (excepto el mismo)
			instruccion.sprintf("UPDATE articulos SET volumen = factor*(%s/%s) \
				WHERE producto='%s' AND present='%s' and articulo<>@folio",
					mFg.FormateaCantidad(volumen,3,false), AnsiString(FloatToStr(factor)),
					clave_presentacion_prod, clave_presentacion_pres);
			instrucciones[num_instrucciones++]=instruccion;
		}

		//  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		// Afectar los CAMBIOS A PESO con base al peso unitario
		if (  (tarea_producto == "A" && factor>1 && peso==0) )
		{
			// Obtiene el peso de la unidad
			instruccion.sprintf("SELECT @pesounidad:=peso from articulos \
				WHERE producto='%s' AND present='%s' and factor=1",
				clave_presentacion_prod, clave_presentacion_pres);
			instrucciones[num_instrucciones++]=instruccion;

			// Si es alta de factor>1 y peso=0, actualizar todos los múltiplos con base al multiplo
			//   que tenga peso>0 del factor unidad.
			instruccion.sprintf("UPDATE articulos SET peso = factor*(@pesounidad) \
				WHERE articulo=@folio and @pesounidad>0",
					mFg.FormateaCantidad(peso,3,false), AnsiString(FloatToStr(factor)),
					clave_presentacion_prod, clave_presentacion_pres);
			instrucciones[num_instrucciones++]=instruccion;
		}

		//  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		// Afectar los CAMBIOS A VOLUMEN con base al volumen unitario
		if (  (tarea_producto == "A" && factor>1 && volumen==0) )
		{
			// Obtiene el volumen de la unidad
			instruccion.sprintf("SELECT @volumenunidad:=volumen from articulos \
				WHERE producto='%s' AND present='%s' and factor=1",
				clave_presentacion_prod, clave_presentacion_pres);
			instrucciones[num_instrucciones++]=instruccion;

			// Si es alta de factor>1 y volumen=0, actualizar solo el multiplo que se está guardando
			// con base al multiplo que tenga volumen>0 del factor unidad.
			instruccion.sprintf("UPDATE articulos SET volumen = factor*(@volumenunidad) \
				WHERE articulo=@folio and @volumenunidad>0",
					mFg.FormateaCantidad(volumen,3,false), AnsiString(FloatToStr(factor)),
					clave_presentacion_prod, clave_presentacion_pres);
			instrucciones[num_instrucciones++]=instruccion;
		}

		//Grabar en tabla de articulosemp
		datos.AsignaTabla("articulosemp");
		parametros += datos.InsCamposDesdeBuffer(parametros);

		utilesp = datos.ObtieneValorCampo("tipoutil");

		if(tarea_articulo == "A"){
			AnsiString instruccion;

			instruccion.sprintf("INSERT IGNORE INTO articulosemp (idempresa, articulo, tipoutil, activoecom, activokiosko) \
				SELECT em.idempresa, @folio AS articulo, %s AS tipoutil, IF(em.idempresa = %s, %s, 0 ) AS activoecom, \
				IF(em.idempresa = %s, %s, 0 ) AS activokiosko \
				FROM empresas em ",
			 datos.ObtieneValorCampo("tipoutil"), empresa, datos.ObtieneValorCampo("activoecom"),
			   empresa, datos.ObtieneValorCampo("activokiosko"));

			instrucciones[num_instrucciones++]=instruccion;

		}else{

			instruccion.sprintf(" UPDATE articulosemp SET activoecom=%s, activokiosko=%s \
				WHERE idempresa=%s AND articulo=@folio ",
				datos.ObtieneValorCampo("activoecom"), datos.ObtieneValorCampo("activokiosko"), empresa);

			instrucciones[num_instrucciones++]=instruccion;
		}

		//  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		// Grabar en la tabla de precios.

		tarea_precios = mFg.ExtraeStringDeBuffer(&parametros);
		num_precios = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));


		if (num_precios >0){
			for (i = 0; i < num_precios; i++) {
				datos.AsignaTabla("precios");
				parametros += datos.InsCamposDesdeBuffer(parametros);
				porcutil = datos.ObtieneValorCampo("porcutil");
				tipoprec = datos.ObtieneValorCampo("tipoprec");
				precioAsig = datos.ObtieneValorCampo("precio");
				//precioMod = datos.ObtieneValorCampo("preciomod");
				producto = clave_producto;
				present = clave_presentacion_pres;
				datos.InsCampo("articulo", "@folio", 1);
				datos.InsCampo("precioproximo", precioAsig);



				if(utilesp=="0"){
					//ejemplo obtenido de    ID_GRA_Venta
					//linea 1160
					BufferRespuestas* resp_porcentajemanual=NULL;
					char *resultado;

					try {
						instruccion.sprintf("SELECT * from precios WHERE articulo='%s' and tipoprec='%s' ",clave_articulo,tipoprec);
						mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),resp_porcentajemanual);
						if (resp_porcentajemanual->ObtieneNumRegistros()>0){
								porcutilantes =  mFg.CadenaAFlotante(resp_porcentajemanual->ObtieneDato("porcutil")) ;
							   int vervalor = mFg.CompararFlotantes(porcutilantes, StrToFloat(porcutil),2);
								if( mFg.CompararFlotantes(porcutilantes, StrToFloat(porcutil) ) != 0  ){
									mListaPorcentajes+=porcutilantes;
									mListaPorcentajesActual+=porcutil;
									grabados++;
									if (grabados<num_precios) {
										mListaPorcentajes+="|";
										mListaPorcentajesActual+="|";
									}
								}

						}
						 //else throw (Exception("Error al consultar en tabla precios para la bitacoraart"));
					} __finally {
						if (resp_porcentajemanual!=NULL) delete resp_porcentajemanual;
					}
				}
				/*************************/


				if (tarea_precios == "A") {
                    //Se insertan los precios para la empresa en uso
					instrucciones[num_instrucciones++] = datos.GenerarSqlInsert();

                    //Registro de precios por defecto para las otras empresas.
					instruccion.sprintf(" INSERT IGNORE INTO precios (articulo, tipoprec, costo, precio, porcutil, fechamodi, preciofechmod, precioproximo, preciomod, horamodi, fechamodiprox, horamodiprox, actualizarpendiente) \
						SELECT a.articulo  AS articulo \
							 , tp.tipoprec  AS tipoprec \
							 , presentcb.costobase AS costo \
							 , CASE ae.tipoutil \
									  WHEN '0' \
											 THEN ROUND(ROUND(presentcb.costobase*(1+tp.porcutil/100),pr.valor)*a.factor,pr.valor) \
									  WHEN '1'                                                                                     \
											 THEN ROUND(ROUND(presentcb.costobase*(1+tp.porcutil/100),pr.valor)*a.factor,pr.valor) \
									  WHEN '2'                                                                                     \
											 THEN ROUND(ROUND(presentcb.costobase*(1+tp.porcutil2/100),pr.valor)*a.factor,pr.valor)\
									  WHEN '3'                                                                                     \
											 THEN ROUND(ROUND(presentcb.costobase*(1+tp.porcutil3/100),pr.valor)*a.factor,pr.valor)\
									  WHEN '4'                                                                                     \
											 THEN ROUND(ROUND(presentcb.costobase*(1+tp.porcutil4/100),pr.valor)*a.factor,pr.valor)\
									  WHEN '5'                                                                                     \
											 THEN ROUND(ROUND(presentcb.costobase*(1+tp.porcutil5/100),pr.valor)*a.factor,pr.valor)\
							   END AS precio                                                                                       \
							 , CASE ae.tipoutil                                                                                     \
									  WHEN '0'                                                                                     \
											 THEN (tp.porcutil)                                                                    \
									  WHEN '1'                                                                                     \
											 THEN (tp.porcutil)                                                                    \
									  WHEN '2'                                                                                     \
											 THEN (tp.porcutil2)                                                                   \
									  WHEN '3'                                                                                     \
											 THEN (tp.porcutil3)                                                                   \
									  WHEN '4'                                                                                     \
											 THEN (tp.porcutil4)                                                                   \
									  WHEN '5'                                                                                     \
											 THEN (tp.porcutil5)                                                                   \
							   END AS porcutil                                                                                     \
							, CURDATE() AS fechamodi                                                                               \
							, CASE ae.tipoutil                                                                                      \
									  WHEN '0'                                                                                     \
											 THEN ROUND(ROUND(presentcb.costobase*(1+tp.porcutil/100),pr.valor)*a.factor,pr.valor) \
									  WHEN '1'                                                                                     \
											 THEN ROUND(ROUND(presentcb.costobase*(1+tp.porcutil/100),pr.valor)*a.factor,pr.valor) \
									  WHEN '2'                                                                                     \
											 THEN ROUND(ROUND(presentcb.costobase*(1+tp.porcutil2/100),pr.valor)*a.factor,pr.valor)\
									  WHEN '3'                                                                                     \
											 THEN ROUND(ROUND(presentcb.costobase*(1+tp.porcutil3/100),pr.valor)*a.factor,pr.valor)\
									  WHEN '4'                                                                                     \
											 THEN ROUND(ROUND(presentcb.costobase*(1+tp.porcutil4/100),pr.valor)*a.factor,pr.valor)\
									  WHEN '5'                                                                                     \
											 THEN ROUND(ROUND(presentcb.costobase*(1+tp.porcutil5/100),pr.valor)*a.factor,pr.valor)\
							   END AS preciofechmod                                                                                \
																																   \
							 , CASE ae.tipoutil                                                                                     \
									  WHEN '0'                                                                                     \
											 THEN ROUND(ROUND(presentcb.costobase*(1+tp.porcutil/100),pr.valor)*a.factor,pr.valor) \
									  WHEN '1'                                                                                     \
											 THEN ROUND(ROUND(presentcb.costobase*(1+tp.porcutil/100),pr.valor)*a.factor,pr.valor) \
									  WHEN '2'                                                                                     \
											 THEN ROUND(ROUND(presentcb.costobase*(1+tp.porcutil2/100),pr.valor)*a.factor,pr.valor)\
									  WHEN '3'                                                                                     \
											 THEN ROUND(ROUND(presentcb.costobase*(1+tp.porcutil3/100),pr.valor)*a.factor,pr.valor)\
									  WHEN '4'                                                                                     \
											 THEN ROUND(ROUND(presentcb.costobase*(1+tp.porcutil4/100),pr.valor)*a.factor,pr.valor)\
									  WHEN '5'                                                                                     \
											 THEN ROUND(ROUND(presentcb.costobase*(1+tp.porcutil5/100),pr.valor)*a.factor,pr.valor)\
							   END AS precioproximo                                                                                \
																																   \
							   , CASE ae.tipoutil                                                                                   \
									  WHEN '0'                                                                                     \
											 THEN ROUND(ROUND(presentcb.costobase*(1+tp.porcutil/100),pr.valor)*a.factor,pr.valor) \
									  WHEN '1'                                                                                     \
											 THEN ROUND(ROUND(presentcb.costobase*(1+tp.porcutil/100),pr.valor)*a.factor,pr.valor) \
									  WHEN '2'                                                                                     \
											 THEN ROUND(ROUND(presentcb.costobase*(1+tp.porcutil2/100),pr.valor)*a.factor,pr.valor)\
									  WHEN '3'                                                                                     \
											 THEN ROUND(ROUND(presentcb.costobase*(1+tp.porcutil3/100),pr.valor)*a.factor,pr.valor)\
									  WHEN '4'                                                                                     \
											 THEN ROUND(ROUND(presentcb.costobase*(1+tp.porcutil4/100),pr.valor)*a.factor,pr.valor)\
									  WHEN '5'                                                                                     \
											 THEN ROUND(ROUND(presentcb.costobase*(1+tp.porcutil5/100),pr.valor)*a.factor,pr.valor)\
							   END AS preciomod                                                                                    \
							   ,CURTIME() AS horamodi                                                                              \
							   ,CURDATE() AS fechamodiprox                                                                         \
							   ,CURTIME() AS horamodiprox                                                                          \
							   ,0 AS actualizarpendiente                                                                           \																								   \
						FROM  articulos   a                                                                                        \
							  INNER JOIN presentaciones present ON a.producto=present.producto AND a.present=present.present       \
							  INNER JOIN empresas em ON em.idempresa <> %s \
							  INNER JOIN presentacionescb presentcb ON presentcb.producto=present.producto AND presentcb.present=present.present \
							   AND presentcb.idempresa=em.idempresa \
							  INNER JOIN articulosemp ae ON ae.articulo = a.articulo AND ae.idempresa = em.idempresa \
							  INNER JOIN tiposdeprecios tp ON tp.idempresa=em.idempresa \
							  INNER JOIN parametrosemp pr ON pr.parametro='DIGREDOND' AND pr.sucursal = em.sucprincipal \
						WHERE a.articulo=@folio ", empresa);

                        instrucciones[num_instrucciones++] = instruccion;

				}
				else {
					// ----------
					// ----------
					if (!bloqueadoprecio) {
						AnsiString update_precio=datos.GenerarSqlUpdate("articulo=@folio and tipoprec='" + tipoprec + "'");
						// Si es diferido la asignación solo se hace en precioproximo
						if (paramcambioprec=="1")
							instrucciones[num_instrucciones++] = StringReplace(update_precio,"precio=","precioproximo=",TReplaceFlags()<<rfReplaceAll);
						else
							instrucciones[num_instrucciones++] = update_precio;
					} else {
						//instrucciones[num_instrucciones++] = "UPDATE precios SET preciomod="+precioMod+" WHERE articulo = @folio AND tipoprec='"+tipoprec+"'";
                    }


				}

				if (!bloqueadoprecio && cambiar_utilidad_multiplos == true) {
					// Cambia el porcentaje de los precios de todos los artículos de la presentación.
					instruccion.sprintf(
						"update precios p, articulos a, articulosemp ae \
						set ae.tipoutil=%s, p.porcutil=%s \
						where p.tipoprec='%s' and p.articulo=a.articulo and ae.articulo = a.articulo and a.producto='%s' and a.present='%s' \
						and ae.idempresa=%s ",
						 utilesp, porcutil, tipoprec, producto, present, empresa);
					instrucciones[num_instrucciones++] = instruccion;
				}
				// aqui podria intentar ir conociendo el mayor de los porcentajes de los precios

			}

			if(mListaPorcentajes!=""){
				mListaPorcentManual->Delimiter = '|';
				mListaPorcentManual->StrictDelimiter = true;
				mListaPorcentManual->DelimitedText = mListaPorcentajes;



				mListaPorcentManActual->Delimiter = '|';
				mListaPorcentManActual->StrictDelimiter = true;
				mListaPorcentManActual->DelimitedText = mListaPorcentajesActual;

				MayorListaPorcentaj=mListaPorcentManual->Strings[0];
				MayorListaPorcenActual=mListaPorcentManActual->Strings[0];
				for (int x=0; x<mListaPorcentManual->Count-1; x++) {

				if ( mFg.CadenaAFlotante(mListaPorcentManual->Strings[x]) > mFg.CadenaAFlotante(MayorListaPorcentaj)) {

						MayorListaPorcentaj=mListaPorcentManual->Strings[x];
						MayorListaPorcenActual=mListaPorcentManActual->Strings[x];

					}
				}
			}
			if (tarea_precios == "M") {
				instruccion.sprintf("INSERT INTO bitacoraart (idBitacoraArt, articulo, idempresa, usuario, fecha, hora,\
				tipo,costoBase,comision,costobaseant,activoant,activo,comisionant,porprecio, porprecioant)values \    \
				(NULL,@folio,%s,'%s','%s','%s','%s',%s,%s,%s,%s,%s,%s,%s,%s)",empresa,usuario,mFg.DateToMySqlDate(Today()),
				mFg.TimeToMySqlTime(Time()),tarea_articulo,costobase,valorcampoPorcComi,
				costobaseant,activoant,activo,comisionant,MayorListaPorcenActual, MayorListaPorcentaj);
				instrucciones[num_instrucciones++] = instruccion;
			}

		}else{
			instruccion.sprintf("INSERT INTO precios (articulo, tipoprec, costo, precio, porcutil, fechamodi, preciofechmod, precioproximo) \
								SELECT a.articulo,t.tipoprec,pre.costobase costo ,  \
								ROUND(ROUND(pre.costobase*(1+t.porcutil/100),pr.valor)*a.factor,pr.valor) AS precio, t.porcutil,NOW() AS fechamodi,0.0 AS preciofechmod, \
								ROUND(ROUND(pre.costobase*(1+t.porcutil/100),pr.valor)*a.factor,pr.valor) AS precioproximo \
								FROM tiposdeprecios t \
								INNER JOIN presentacionescb pre ON pre.producto='%s' AND pre.present='%s' \
								INNER JOIN articulos a ON a.producto=pre.producto AND a.present=pre.present AND a.articulo=@folio \
								,parametrosemp as pr where pr.parametro='DIGREDOND' AND pr.sucursal='%s' AND t.idempresa=%s AND pre.idempresa=%s ",
								clave_producto, clave_presentacion_pres, FormServidor->ObtieneClaveSucursal(), empresa, empresa);
			instrucciones[num_instrucciones++] = instruccion;
		}

	   /*	if(mListaPorcentajes!=""){
				mListaSegmentos->Delimiter = '|';
				mListaSegmentos->StrictDelimiter = true;
				mListaSegmentos->DelimitedText = mListaPorcentajes;

				MayorListaPorcentaje=mListaSegmentos->Strings[0];
				for (int x=0; x<mListaSegmentos->Count-1; x++) {

					if ( mFg.CadenaAFlotante(mListaSegmentos->Strings[x]) > mFg.CadenaAFlotante(MayorListaPorcentaje)) {

						MayorListaPorcentaje=mListaSegmentos->Strings[x];

					}
				}
		} */

		if (!bloqueadoprecio) {
			// ----------
			// ----------
			// Calcula el costo de todos los artículos de la misma presentación
			AnsiString asignacion_precio;
			if (paramcambioprec=="0"){
				asignacion_precio.sprintf("p.precio=ROUND(ROUND(%s*(1+p.porcutil/100),pr.valor)*a.factor,pr.valor) , \
									p.precioproximo=ROUND(ROUND(%s*(1+p.porcutil/100),pr.valor)*a.factor,pr.valor)  ",
									  costobase, costobase);
			}else
				asignacion_precio.sprintf("p.precioproximo=ROUND(ROUND(%s*(1+p.porcutil/100),pr.valor)*a.factor,pr.valor) ", costobase);

			instruccion.sprintf(
				"update precios p, tiposdeprecios tp, articulos a, parametrosemp as pr set p.costo=%s, %s \
				where p.articulo=a.articulo and p.tipoprec=tp.tipoprec and a.producto='%s' and a.present='%s' and pr.parametro='DIGREDOND' \
				and pr.sucursal='%s' and tp.idempresa=%s ",
				costobase, asignacion_precio, clave_producto, clave_presentacion_pres,
				FormServidor->ObtieneClaveSucursal(), empresa);
			instrucciones[num_instrucciones++] = instruccion;


		}


		if (tarea_articulo == "M")
		{

			if (activoant==1 && activo==0) {
				AnsiString Instruccion;
				Instruccion.sprintf("SELECT sucursal FROM articulosxsuc WHERE producto = '%s' AND present = '%s'", clave_presentacion_prod, clave_presentacion_pres);
				mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), resp_suc);
				for(int i=0; i<resp_suc->ObtieneNumRegistros(); i++){
					resp_suc->IrAlRegistroNumero(i);

					instruccion.sprintf("INSERT INTO bitacoraarticulosxsuc (sucursal, producto, present, evento, fecha, hora, usuario) VALUES                                     								   \
					('%s','%s','%s','B',CURDATE(),CURTIME(),'%s')",
					resp_suc->ObtieneDato("sucursal"), clave_presentacion_prod, clave_presentacion_pres, usuario);
					instrucciones[num_instrucciones++]=instruccion;
				}

				instruccion.sprintf("DELETE FROM articulosxsuc WHERE producto = '%s' AND present = '%s'  \
						 AND (producto,present) NOT IN (select producto,present from articulos \
						 WHERE producto = '%s' AND present = '%s'  AND activo=1 )", clave_presentacion_prod, clave_presentacion_pres,clave_presentacion_prod, clave_presentacion_pres);
				instrucciones[num_instrucciones++]=instruccion;

                instruccion.sprintf("UPDATE pedidoautomatico_presentaciones_global\
				SET multiplopedir = NULL WHERE multiplopedir = '%s'", clave_articulo);
				instrucciones[num_instrucciones++]=instruccion;

                instruccion.sprintf("UPDATE pedidoautomatico_presentaciones_global\
				SET multiplopedir = NULL WHERE multiplodistribucion = '%s'", clave_articulo);
				instrucciones[num_instrucciones++]=instruccion;

				instruccion.sprintf("UPDATE pedidoautomatico_presentaciones_empresa\
				SET multiplopedir = NULL WHERE multiplopedir = '%s'", clave_articulo);
				instrucciones[num_instrucciones++]=instruccion;

				instruccion.sprintf("UPDATE pedidoautomatico_presentaciones_sucursal\
				SET multiplopedir = NULL WHERE multiplopedir = '%s'", clave_articulo);
				instrucciones[num_instrucciones++]=instruccion;
			}
		}

		// Consultar la comisión de cierto el articulo y aplicarlo a los mismos que contengan la misma presentación y producto.
		instruccion.sprintf("SET @comision_upd = (SELECT porccomi FROM articulos WHERE articulo = @folio)");
		instrucciones[num_instrucciones++]=instruccion;

		// Actualizar la comisión de todos los múltiplos de la misma presentación.
		instruccion.sprintf("UPDATE articulos SET porccomi =  @comision_upd  WHERE producto=@producto_a and present=@presentacion_a");
		instrucciones[num_instrucciones++]=instruccion;

		// Actualiza el registro de la tabla presentacionesminmax
		instruccion.sprintf("DELETE FROM presentacionesminmax where producto = @producto_a and present=@presentacion_a");
			instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("INSERT INTO presentacionesminmax (producto,present,maxfactor,maxmult,minmult,activo) \
			(SELECT amm.producto, amm.present, amm.maxfactor, \
					 amax.multiplo AS maxmult, amin.multiplo AS minmult, 1 AS activo \
			FROM \
				(select a.producto, a.present, MAX(a.factor) AS maxfactor \
					from articulos a WHERE a.activo=1 and a.producto=@producto_a AND a.present=@presentacion_a \
						group by a.producto, a.present) amm \
			INNER JOIN articulos amax ON amm.producto=amax.producto AND amm.present=amax.present AND amm.maxfactor=amax.factor AND amax.activo=1 \
			INNER JOIN articulos amin ON amm.producto=amin.producto AND amm.present=amin.present AND amin.factor=1 AND amin.activo=1 \
			group by amm.producto, amm.present) \
		UNION ALL \
			(SELECT amm.producto, amm.present, amm.maxfactor, \
					 amax.multiplo AS maxmult, amin.multiplo AS minmult, 0 AS activo \
			FROM \
				(select a.producto, a.present, MAX(a.factor) AS maxfactor, MAX(a.activo) AS maxactivo \
					from articulos a WHERE a.producto=@producto_a AND a.present=@presentacion_a \
						group by a.producto, a.present \
						HAVING maxactivo=0 \
						) amm \
			INNER JOIN articulos amax ON amm.producto=amax.producto AND amm.present=amax.present AND amm.maxfactor=amax.factor \
			INNER JOIN articulos amin ON amm.producto=amin.producto AND amm.present=amin.present AND amin.factor=1 \
			group by amm.producto, amm.present)");
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("update presentacionesminmax pmm \
			LEFT JOIN presentcajlogisfactor pclf ON pmm.producto=pclf.producto AND pmm.present=pclf.present \
			set pmm.cajalogisticafactor=COALESCE(pclf.cajalogisticafactor, pmm.maxfactor) \
			where pmm.producto = @producto_a and pmm.present=@presentacion_a");
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf(" INSERT IGNORE INTO articuloempresacfg (articulo, idempresa, permitecompras, permiteventas, permitemovalma) \
			SELECT @folio, e.idempresa, pmm.activo AS pcompras, pmm.activo  AS pventas, pmm.activo AS pmovalm \
			FROM presentacionesminmax pmm \
			INNER JOIN empresas e \
			WHERE pmm.producto=@producto_a AND pmm.present=@presentacion_a ");
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf(" INSERT IGNORE INTO presentacionesactivemp (idempresa, producto, present, activo) \
			SELECT e.idempresa, pmm.producto, pmm.present, pmm.activo \
			FROM presentacionesminmax pmm \
			INNER JOIN empresas e \
			WHERE pmm.producto=@producto_a AND pmm.present=@presentacion_a  ");
		instrucciones[num_instrucciones++] = instruccion;

		select.sprintf("SELECT usuario FROM articulossupervisados WHERE producto = '%s' AND present = '%s'",
		clave_presentacion_prod, clave_presentacion_pres);

		BufferRespuestas* resp_supervisadoprod = NULL;
		bool existe;

		try{

			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, select.c_str(), resp_supervisadoprod);
			if (resp_supervisadoprod->ObtieneNumRegistros()>0) {
				existe = true;
			} else {
                existe = false;
            }
		}
		__finally{
			if(resp_supervisadoprod!=NULL) delete resp_supervisadoprod;
		}

		if (existe) {
			if (supervisado=="0") {
				instruccion.sprintf("DELETE FROM articulossupervisados where producto = '%s' and present='%s'",
				clave_presentacion_prod, clave_presentacion_pres);
				instrucciones[num_instrucciones++]=instruccion;

				instruccion.sprintf("INSERT INTO bitacoraarticulossupervisados (producto, present, evento, fecha, hora, usuario) VALUES                                     								   \
				('%s','%s','B', CURDATE(), CURTIME(), '%s')",
				clave_presentacion_prod, clave_presentacion_pres, usuario);
				instrucciones[num_instrucciones++]=instruccion;
			} else {
				instruccion.sprintf("UPDATE articulossupervisados SET usuario= '%s' where producto = '%s' and present='%s'",
				empleadosupervisor, clave_presentacion_prod, clave_presentacion_pres);
				instrucciones[num_instrucciones++]=instruccion;

				instruccion.sprintf("INSERT INTO bitacoraarticulossupervisados (producto, present, evento, fecha, hora, usuario) VALUES                                     								   \
				('%s','%s','M', CURDATE(), CURTIME(), '%s')",
				clave_presentacion_prod, clave_presentacion_pres, usuario);
				instrucciones[num_instrucciones++]=instruccion;
			}
		} else {
			if (supervisado=="1") {
				instruccion.sprintf("INSERT INTO articulossupervisados (producto, present, usuario) VALUES                                     								   \
				('%s','%s','%s')",
				clave_presentacion_prod, clave_presentacion_pres, empleadosupervisor);
				instrucciones[num_instrucciones++]=instruccion;

				instruccion.sprintf("INSERT INTO bitacoraarticulossupervisados (producto, present, evento, fecha, hora, usuario) VALUES                                     								   \
				('%s','%s','A', CURDATE(), CURTIME(), '%s')",
				clave_presentacion_prod, clave_presentacion_pres, usuario);
				instrucciones[num_instrucciones++]=instruccion;
			}
		}

		/*if(redondea_precios == "1"){
            campo_precios.sprintf("(CASE WHEN MOD(%s,1) = 0.0 THEN %s ELSE \
			CASE WHEN MOD(%s,1) >=%s AND MOD(%s,1) <=%s THEN ((%s-MOD(%s,1))+%s) ELSE \
			CASE WHEN MOD(%s,1) >=%s AND MOD(%s,1) <=%s THEN ((%s-MOD(%s,1))+%s) ELSE \
			CASE WHEN MOD(%s,1) >=%s AND MOD(%s,1) <=%s THEN ((%s-MOD(%s,1))+%s) ELSE \
			CASE WHEN MOD(%s,1) >=%s AND MOD(%s,1) <=%s THEN ((%s-MOD(%s,1))+%s) END END END END END)",
			precio, precio,
			precio, desde1, precio, hasta1, redondeo1, precio, precio,
			precio, desde2, precio, hasta2, redondeo2, precio, precio,
			precio, desde3, precio, hasta3, redondeo3, precio, precio,
			precio, desde4, precio, hasta4, redondeo4, precio, precio);

			condicion_producto.sprintf(" AND pro.producto='%s' AND a.present='%s' ", clave_presentacion_prod, clave_presentacion_pres);

			instruccion.sprintf("CREATE TEMPORARY TABLE tmppreciosmod ( \
				articulo VARCHAR(9), \
				producto VARCHAR(8), \
				present VARCHAR(255), \
				multiplo VARCHAR(10), \
				factor DECIMAL(10,3), \
				tipoprec VARCHAR(2), \
				precio DECIMAL(16,6), \
				precioproximo DECIMAL(16,6), \
				preciomod DECIMAL(16,6), \
				INDEX(articulo), INDEX(tipoprec) \
			)ENGINE=INNODB");
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("INSERT INTO tmppreciosmod \
			SELECT \
				a.articulo, \
				a.producto, \
				a.present, \
				a.multiplo, \
				a.factor, \
				p.tipoprec, \
				p.precio, \
				p.precioproximo, \
				%s AS preciomod \
			FROM precios p \
			INNER JOIN articulos a ON p.articulo=a.articulo \
			INNER JOIN tiposdeprecios tp ON tp.tipoprec=p.tipoprec \
			WHERE tp.idempresa = %s AND a.producto = '%s' AND a.present = '%s' AND a.factor = 1 \
			HAVING p.precio <> preciomod",
			campo_precios, empresa, clave_presentacion_prod, clave_presentacion_pres);
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("UPDATE precios p \
			INNER JOIN tmppreciosmod a ON a.articulo = p.articulo AND p.tipoprec = a.tipoprec \
			INNER JOIN tiposdeprecios tp ON tp.tipoprec = p.tipoprec \
			SET p.preciomod = a.preciomod \
			WHERE tp.idempresa = %s AND a.producto = '%s' AND a.present = '%s' AND a.factor = 1 ",
			empresa, clave_presentacion_prod, clave_presentacion_pres);
			instrucciones[num_instrucciones++]=instruccion;

			try {
				instruccion.sprintf("SELECT \
					p.tipoprec, \
					p.preciomod \
				FROM articulos a \
				INNER JOIN precios p ON p.articulo = a.articulo \
				INNER JOIN tiposdeprecios tp ON tp.tipoprec = p.tipoprec \
				WHERE a.producto = '%s' AND a.present = '%s' AND a.factor = 1 AND tp.idempresa = %s \
				ORDER BY a.multiplo",
				clave_presentacion_prod, clave_presentacion_pres, empresa);
				if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_productos)) {
					for (int i = 0; i < resp_productos->ObtieneNumRegistros(); i++) {
						resp_productos->IrAlRegistroNumero(i);

						AnsiString tipo_precio = resp_productos->ObtieneDato("tipoprec");
						double precio_mod = mFg.CadenaAFlotante(resp_productos->ObtieneDato("preciomod"));

						instruccion.sprintf("UPDATE precios p \
						INNER JOIN articulos a ON a.articulo = p.articulo \
						INNER JOIN tiposdeprecios tp ON tp.tipoprec = p.tipoprec \
						SET p.preciomod = (CASE WHEN MOD((%f * a.factor),1) = 0.0 THEN (%f * a.factor) ELSE \
						CASE WHEN MOD((%f * a.factor),1) >= 0.01 AND MOD((%f * a.factor),1) <= 0.24 THEN ((0.00-MOD((%f * a.factor),1))+(%f * a.factor)) ELSE \
						CASE WHEN MOD((%f * a.factor),1) >= 0.25 AND MOD((%f * a.factor),1) <= 0.49 THEN ((0.50-MOD((%f * a.factor),1))+(%f * a.factor)) ELSE \
						CASE WHEN MOD((%f * a.factor),1) >= 0.50 AND MOD((%f * a.factor),1) <= 0.74 THEN ((0.50-MOD((%f * a.factor),1))+(%f * a.factor)) ELSE \
						CASE WHEN MOD((%f * a.factor),1) >= 0.75 AND MOD((%f * a.factor),1) <= 0.99 THEN ((0.90-MOD((%f * a.factor),1))+(%f * a.factor)) END END END END END) \
						WHERE tp.idempresa = %s \
						AND a.producto = '%s' \
						AND a.present = '%s' \
						AND p.tipoprec = '%s' \
						AND a.factor <> 1 ",
						precio_mod, precio_mod,
						precio_mod, precio_mod, precio_mod, precio_mod,
						precio_mod, precio_mod, precio_mod, precio_mod,
						precio_mod, precio_mod, precio_mod, precio_mod,
						precio_mod, precio_mod, precio_mod, precio_mod,
						empresa, clave_presentacion_prod, clave_presentacion_pres, tipo_precio);
						instrucciones[num_instrucciones++]=instruccion;

					}
				}
			} __finally {
				if (resp_productos!=NULL) delete resp_productos;
			}

			instruccion.sprintf("INSERT INTO bitacorapreciosdiferidos SELECT NULL,CURDATE(),CURTIME(),'%s'",
			usuario);
			instrucciones[num_instrucciones++]=instruccion;

			instrucciones[num_instrucciones++]="set @referencia:=LAST_INSERT_ID()";

			instruccion.sprintf("INSERT INTO bitacorapreciosdiferidosdetalle \
			SELECT NULL,@referencia, p.articulo, p.precio, %s, p.tipoprec \
			FROM precios p \
			INNER JOIN tiposdeprecios tp ON tp.tipoprec=p.tipoprec AND tp.idempresa=%s \
			INNER JOIN articulos a ON a.articulo=p.articulo \
			INNER JOIN productos pro ON a.producto=pro.producto \
			WHERE (%s<>p.precio or p.precio is null) %s",
			campo_precios, empresa, campo_precios, condicion_producto);
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("UPDATE precios AS p \
			INNER JOIN tiposdeprecios tp ON tp.tipoprec=p.tipoprec AND tp.idempresa=%s \
			INNER JOIN articulos a ON a.articulo=p.articulo \
			INNER JOIN productos pro ON a.producto=pro.producto \
			SET p.actualizarpendiente=1 \
			WHERE (%s<>p.precio or p.precio is null) %s ", empresa,
			campo_precios, condicion_producto);
			instrucciones[num_instrucciones++]=instruccion;
		}*/

		instrucciones[num_instrucciones++] = "COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql = mFg.AgregaStringABuffer
			(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i = 0; i < num_instrucciones; i++)
			aux_buffer_sql = mFg.AgregaStringABuffer(instrucciones[i],
			aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta,
			TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,
				buffer_sql)) {

			instruccion.sprintf("select @folio as folio");
			mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,
				instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	}
	__finally {
		//if (resp_porcentajemanual!=NULL) delete resp_porcentajemanual;

		if (resp_suc!=NULL) delete resp_suc;
		delete buffer_sql;


	}
}

// ---------------------------------------------------------------------------
// ID_BAJ_ARTICULO
void ServidorCatalogos::BajaArticulo(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// BAJA ARTICULO
	char *buffer_sql = new char[1024 * 10];
	char *aux_buffer_sql = buffer_sql;
	AnsiString instruccion;
	AnsiString clave, usuario, empresa;
	AnsiString instrucciones[55];
	int num_instrucciones = 0;
	int i;

	BufferRespuestas* resp_articulo = NULL;
	BufferRespuestas* resp_presentacion = NULL;
	AnsiString select_articulo, select_presentacion;
	AnsiString idEmpresa = " ";

	BufferRespuestas * respuesta_empresas = NULL;

	try {
		clave = mFg.ExtraeStringDeBuffer(&parametros);
		usuario = mFg.ExtraeStringDeBuffer(&parametros);
		empresa = mFg.ExtraeStringDeBuffer(&parametros);

		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, "SELECT idempresa FROM empresas", respuesta_empresas);

		instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++] = "START TRANSACTION";

        select_presentacion.sprintf("SELECT @present:=present, @producto:=producto, @activo:=activo FROM articulos where articulo='%s'", clave);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, select_presentacion.c_str(), resp_presentacion);

        select_articulo.sprintf("SELECT producto, present FROM articulos WHERE producto=@producto and present=@present");
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, select_articulo.c_str(), resp_articulo);

		instruccion.sprintf("select @costoBase:=costoBase from presentacionescb WHERE @producto=producto AND @present=present AND idempresa=%s ", empresa);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("select @porcComi:=porcComi FROM articulos WHERE articulo='%s'",clave);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("select @porcentUtil:=p.porcutil FROM precios p \
		INNER JOIN tiposdeprecios tp ON tp.tipoprec=p.tipoprec \
		WHERE p.articulo='%s' and tp.idempresa=%s ",clave, empresa);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("delete from codigosbarras where articulo='%s'",
			clave);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("delete from precios \
		where articulo='%s' ", clave);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("delete from preciolocal where articulo='%s'", clave);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("delete from articuloxseccion where articulo='%s'",
			clave);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("delete from articulosequiv where articulo='%s'", clave);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("delete from articulosunif where articulo='%s'", clave);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("delete from ecommerceproductos where articulo='%s'", clave);
		instrucciones[num_instrucciones++] = instruccion;

		// Actualiza el registro de la tabla presentacionesminmax
		instruccion.sprintf("DELETE FROM presentacionesminmax where producto = @producto and present=@present");
		instrucciones[num_instrucciones++] = instruccion;

        // Actualiza el registro de la tabla Presentcajlogisfactor
		instruccion.sprintf("DELETE FROM Presentcajlogisfactor where producto = @producto and present=@present");
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf(
			"DELETE FROM bitacoracostos WHERE articulo='%s'", clave);
		instrucciones[num_instrucciones++] = instruccion;

        instruccion.sprintf(
			"DELETE FROM articuloempresacfg WHERE articulo='%s'", clave);
		instrucciones[num_instrucciones++] = instruccion;

        instruccion.sprintf(
			"DELETE FROM articulosemp WHERE articulo='%s'", clave);
		instrucciones[num_instrucciones++] = instruccion;

        instruccion.sprintf("DELETE FROM presentacionesactivemp where producto = @producto and present=@present");
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("delete from articulos where articulo='%s'", clave);
		instrucciones[num_instrucciones++] = instruccion;

		if (resp_articulo->ObtieneNumRegistros()<=1) {

			for(int i = 0; i < respuesta_empresas->ObtieneNumRegistros(); i++){
				respuesta_empresas->IrAlRegistroNumero(i);
				AnsiString registroEmpresa = respuesta_empresas->ObtieneDato();

				instruccion.sprintf(
				"DELETE FROM precalculocostospromedio%s WHERE present=@present AND producto=@producto ", registroEmpresa);
				instrucciones[num_instrucciones++] = instruccion;

                instruccion.sprintf(
				"DELETE FROM precalculocostos%s WHERE present=@present and producto=@producto ", registroEmpresa);
				instrucciones[num_instrucciones++] = instruccion;

				instruccion.sprintf(
				"DELETE FROM precalculomensual%s WHERE present=@present AND producto=@producto ", registroEmpresa);
				instrucciones[num_instrucciones++] = instruccion;
			}

			instruccion.sprintf(
				"DELETE FROM prodbloqueadocompra WHERE present=@present AND producto=@producto ");
			instrucciones[num_instrucciones++] = instruccion;

			instruccion.sprintf(
				"DELETE FROM existenciasactuales WHERE present=@present AND producto=@producto ");
			instrucciones[num_instrucciones++] = instruccion;

			instruccion.sprintf(
				"delete from articulosped where present=@present and producto=@producto ");
			instrucciones[num_instrucciones++] = instruccion;

			instruccion.sprintf(
				"delete from stock where present=@present and producto=@producto ");
			instrucciones[num_instrucciones++] = instruccion;

			instruccion.sprintf(
				"DELETE FROM dconceptosventa WHERE present=@present AND producto=@producto ");
			instrucciones[num_instrucciones++] = instruccion;

			instruccion.sprintf(
				"delete from dsimilarvxvol where present=@present and producto=@producto ");
			instrucciones[num_instrucciones++] = instruccion;

			instruccion.sprintf(
				"delete from ventasxmes where present=@present and producto=@producto ");
			instrucciones[num_instrucciones++] = instruccion;

			instruccion.sprintf(
				"delete from bitacoraarticulosxsuc where present=@present and producto=@producto ");
			instrucciones[num_instrucciones++] = instruccion;

			instruccion.sprintf(
				"delete from preciosbloqueados where present=@present and producto=@producto ");
			instrucciones[num_instrucciones++] = instruccion;

			instruccion.sprintf(
				"delete from articulostagsasignados where present=@present and producto=@producto ");
			instrucciones[num_instrucciones++] = instruccion;

			instruccion.sprintf(
				"delete from articulosxsuc  where present=@present and producto=@producto ");
			instrucciones[num_instrucciones++] = instruccion;

			instruccion.sprintf(
				"DELETE FROM articulossupervisados where present=@present and producto=@producto ");
			instrucciones[num_instrucciones++] = instruccion;

			instruccion.sprintf(
				"DELETE FROM bitacoraarticulossupervisados where present=@present and producto=@producto ");
			instrucciones[num_instrucciones++] = instruccion;

			instruccion.sprintf(
			   "DELETE FROM bitacoraarticulosped where present=@present and producto=@producto ");
			instrucciones[num_instrucciones++] = instruccion;

			instruccion.sprintf(
				"delete from presentacionescb where present=@present and producto=@producto ");
			instrucciones[num_instrucciones++] = instruccion;

			instruccion.sprintf(
				"delete from presentaciones where present=@present and producto=@producto ");
			instrucciones[num_instrucciones++] = instruccion;
		}

		instruccion.sprintf(
			"delete from prodanotaciones where producto=@producto and producto not in (select producto from presentaciones where producto=@producto)");
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf(
			"delete from productos where producto=@producto and producto not in (select producto from presentaciones where producto=@producto)");
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("INSERT INTO bitacoraart values (NULL,'%s',%s,'%s','%s','%s','B',@costoBase,@porcComi,@costoBase,@activo,@activo,@porcComi,@porcentUtil,@porcentUtil)",clave,empresa,usuario,mFg.DateToMySqlDate(Today()), mFg.TimeToMySqlTime(Time()));
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("INSERT INTO presentacionesminmax (producto,present,maxfactor,maxmult,minmult,activo) \
			(SELECT amm.producto, amm.present, amm.maxfactor, \
					 amax.multiplo AS maxmult, amin.multiplo AS minmult, 1 AS activo \
			FROM \
				(select a.producto, a.present, MAX(a.factor) AS maxfactor \
					from articulos a WHERE a.activo=1 and a.producto=@producto AND a.present=@present \
						group by a.producto, a.present) amm \
			INNER JOIN articulos amax ON amm.producto=amax.producto AND amm.present=amax.present AND amm.maxfactor=amax.factor AND amax.activo=1 \
			INNER JOIN articulos amin ON amm.producto=amin.producto AND amm.present=amin.present AND amin.factor=1 AND amin.activo=1 \
			group by amm.producto, amm.present) \
		UNION ALL \
			(SELECT amm.producto, amm.present, amm.maxfactor, \
					 amax.multiplo AS maxmult, amin.multiplo AS minmult, 0 AS activo \
			FROM \
				(select a.producto, a.present, MAX(a.factor) AS maxfactor, MAX(a.activo) AS maxactivo \
					from articulos a WHERE a.producto=@producto AND a.present=@present \
						group by a.producto, a.present \
						HAVING maxactivo=0 \
						) amm \
			INNER JOIN articulos amax ON amm.producto=amax.producto AND amm.present=amax.present AND amm.maxfactor=amax.factor \
			INNER JOIN articulos amin ON amm.producto=amin.producto AND amm.present=amin.present AND amin.factor=1 \
			group by amm.producto, amm.present)");
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("update presentacionesminmax pmm \
			LEFT JOIN presentcajlogisfactor pclf ON pmm.producto=pclf.producto AND pmm.present=pclf.present \
			set pmm.cajalogisticafactor=COALESCE(pclf.cajalogisticafactor, pmm.maxfactor) \
			where pmm.producto = @producto and pmm.present=@present");
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
		if (resp_articulo!=NULL) delete resp_articulo;
		delete respuesta_empresas;
	}
}

// ---------------------------------------------------------------------------
// ID_CON_ARTICULO
void ServidorCatalogos::ConsultaArticulo(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {

	// CONSULTA ARTICULO
	AnsiString instruccion;
	AnsiString clave_articulo, clave_producto, clave_present;
	AnsiString clave_clasif1, clave_clasif2;
	AnsiString clave_clasifcont;
	AnsiString solo_activos, condicion_solo_activos = " ";
	AnsiString empresa;
	char *resultado;

	//Recibe parámetros
	clave_articulo = mFg.ExtraeStringDeBuffer(&parametros);
	solo_activos = mFg.ExtraeStringDeBuffer(&parametros);
	empresa = mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	if (solo_activos == "1") {
		condicion_solo_activos = " a.activo=1 and ";
	}

	// Obtiene todos los datos del artículo
	instruccion.sprintf("select a.*, ae.tipoutil as tipoutil2, ae.activoecom as activoecom2, ae.activokiosko as activokiosko2 \
		from articulos a \
		inner join articulosemp ae on ae.articulo = a.articulo \
		where %s a.articulo='%s' and ae.idempresa=%s ",
		condicion_solo_activos, clave_articulo, empresa);
	resultado = Respuesta->ObtieneDirLibreBuffer();
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	BufferRespuestas resp_articulo(resultado);
	if (resp_articulo.ObtieneNumRegistros()) {
		clave_present = resp_articulo.ObtieneDato("present");
		clave_producto = resp_articulo.ObtieneDato("producto");

		//Obtiene los datos de llave foraneas de clave producto y clave  unidad
		instruccion.sprintf(
			"SELECT c.cveprodserv, a.idclaveunidadcfdi FROM articulos a INNER JOIN  productos p ON p.producto = a.producto \
			 INNER JOIN cclaveprodserv c ON c.idclaveprodserv = p.idclaveprodservcfdi  INNER JOIN presentaciones pr \
			 ON a.producto = pr.producto AND a.present = pr.present LEFT JOIN cclaveunidad cu ON  \
			 cu.idclaveunidad = a.idclaveunidadcfdi WHERE a.articulo='%s'", clave_articulo);
		// resultado=Respuesta->ObtieneDirLibreBuffer();
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

		// Obtiene todos los datos de la presentacion
		// -- incluyendo bandera de precios bloqueados.
		// -- incluyendo
		instruccion.sprintf(
			"SELECT pre.producto, pre.present, pre.modoutil, pre.sgerencia, pre.permitfrac, pre.presentsat, \
				pre.idempaque, pre.idunidad, pre.cotizable, pre.iepscuota, pre.factortarima/artmax.factor as factortarima, pcb.costobase, \
				pcb.segmento, IF(ISNULL(pb.producto), 0,1) AS preciosbloqueados, pmm.cajalogisticafactor \
				FROM presentaciones pre \
				LEFT JOIN presentacionesminmax pmm ON pmm.producto=pre.producto AND pmm.present=pre.present \
                LEFT JOIN articulos artmax ON artmax.producto = pre.producto AND artmax.present = pre.present AND artmax.multiplo = pmm.maxmult \
				LEFT JOIN preciosbloqueados pb ON pb.producto=pre.producto AND pb.present=pre.present AND \
				pb.fechaVigencia>='%s' AND pb.idempresa=%s \
				LEFT JOIN presentacionescb pcb ON pcb.producto = pre.producto AND pcb.present = pre.present AND pcb.idempresa = %s \
			WHERE pre.producto='%s' AND pre.present='%s' ",
			  mFg.DateToMySqlDate(Today()), empresa, empresa,
		   clave_producto, clave_present);
		// resultado=Respuesta->ObtieneDirLibreBuffer();
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

		// Obtiene todos los datos del producto
		instruccion.sprintf("select * from productos where producto='%s'",
			clave_producto);
		resultado = Respuesta->ObtieneDirLibreBuffer();
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,
			instruccion.c_str(), Respuesta->TamBufferResultado);
		BufferRespuestas resp_producto(resultado);
		clave_clasif1 = resp_producto.ObtieneDato("clasif1");
		clave_clasif2 = resp_producto.ObtieneDato("clasif2");

		clave_clasifcont = resp_producto.ObtieneDato("clasifcont");

		// Clasificacion 1
		instruccion.sprintf(
			"select clasif1, nombre from clasificacion1 order by nombre");
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,
			instruccion.c_str(), Respuesta->TamBufferResultado);

		// Clasificacion 2
		instruccion.sprintf(
			"select clasif2, nombre from clasificacion2 where clasif1='%s' order by nombre"
			, clave_clasif1);
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,
			instruccion.c_str(), Respuesta->TamBufferResultado);

		// Clasificacion 3
		instruccion.sprintf(
			"select clasif3, nombre from clasificacion3 where clasif2='%s' order by nombre"
			, clave_clasif2);
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,
			instruccion.c_str(), Respuesta->TamBufferResultado);

		// Marcas
		instruccion.sprintf("select marca, nombre from marcas order by nombre");
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,
			instruccion.c_str(), Respuesta->TamBufferResultado);

		// Clasificaciones para contabilidad
		instruccion.sprintf(
			"select clasif, nombre,tipoprod from clasifcont order by nombre");
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,
			instruccion.c_str(), Respuesta->TamBufferResultado);

		// Clasificacion 2 para contabilidad
		instruccion.sprintf(
			"select clasif2, nombre,idclave from clasifcont2 where clasif='%s' order by nombre"
			, clave_clasifcont);
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,
			instruccion.c_str(), Respuesta->TamBufferResultado);

		// Impuestos que usa el producto que se esta cargando y que estan INACTIVOS (para alertar al usuario)
		instruccion.sprintf(" \
			(SELECT ic1.* FROM productos p \
			INNER JOIN impuestos ic1 ON ic1.impuesto=p.claveimpc1 AND ic1.activo=0 \
			WHERE p.producto='%s') \
			UNION \
			(SELECT ic2.* FROM productos p \
			INNER JOIN impuestos ic2 ON ic2.impuesto=p.claveimpc2 AND ic2.activo=0 \
			WHERE p.producto='%s') \
			UNION \
			(SELECT ic3.* FROM productos p \
			INNER JOIN impuestos ic3 ON ic3.impuesto=p.claveimpc3 AND ic3.activo=0 \
			WHERE p.producto='%s') \
			UNION \
			(SELECT ic4.* FROM productos p \
			INNER JOIN impuestos ic4 ON ic4.impuesto=p.claveimpc4 AND ic4.activo=0 \
			WHERE p.producto='%s') \
			UNION \
			(SELECT iv1.* FROM productos p \
			INNER JOIN impuestos iv1 ON iv1.impuesto=p.claveimpv1 AND iv1.activo=0 \
			WHERE p.producto='%s') \
			UNION \
			(SELECT iv2.* FROM productos p \
			INNER JOIN impuestos iv2 ON iv2.impuesto=p.claveimpv2 AND iv2.activo=0 \
			WHERE p.producto='%s') \
			UNION \
			(SELECT iv3.* FROM productos p \
			INNER JOIN impuestos iv3 ON iv3.impuesto=p.claveimpv3 AND iv3.activo=0 \
			WHERE p.producto='%s') \
			UNION \
			(SELECT iv4.* FROM productos p \
			INNER JOIN impuestos iv4 ON iv4.impuesto=p.claveimpv4 AND iv4.activo=0 \
			WHERE p.producto='%s')", clave_producto, clave_producto,
			clave_producto, clave_producto, clave_producto, clave_producto,
			clave_producto, clave_producto);
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,
			instruccion.c_str(), Respuesta->TamBufferResultado);

		// Impuestos
		instruccion.sprintf(
			"select i.impuesto, t.nombre, i.porcentaje, if(i.activo=1,'ACTIVO','NO ACTIVO') as textoactivo from impuestos i, tiposdeimpuestos t where i.tipoimpu=t.tipoimpu AND i.tipoimpu <> 'ISR' order by i.impuesto");
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,
			instruccion.c_str(), Respuesta->TamBufferResultado);

		// Impuestos sin IESPS (para los impuestos 2,3 y 4 ya que el IESPS solo puede ser primer)
		instruccion.sprintf(
			"select i.impuesto, t.nombre, i.porcentaje, if(i.activo=1,'ACTIVO','NO ACTIVO') as textoactivo from impuestos i, tiposdeimpuestos t where i.tipoimpu=t.tipoimpu and i.tipoimpu<>'IESPS' AND i.tipoimpu <> 'ISR' order by i.impuesto");
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,
			instruccion.c_str(), Respuesta->TamBufferResultado);

		// Presentaciones del producto en cuestión.
		instruccion.sprintf(
			"select present from presentaciones where producto='%s' order by present"
			, clave_producto);
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,
			instruccion.c_str(), Respuesta->TamBufferResultado);

		// Precios del artículo en cuestión
		instruccion.sprintf(
			"select p.tipoprec, t.descripcion, t.porcutil as porcutilt, t.porcutil2, t.porcutil3, t.porcutil4, \
			 t.porcutil5, p.porcutil as porcutilp, p.precio, p.precioproximo, p.preciomod, p.actualizarpendiente  from tiposdeprecios t, precios p where \
			 p.tipoprec=t.tipoprec and p.articulo='%s' and t.idempresa=%s order by p.tipoprec", clave_articulo, empresa);
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,
			instruccion.c_str(), Respuesta->TamBufferResultado);

		// Presentaciones y múltiplos del producto en cuestión.
		instruccion.sprintf(
			"SELECT a.articulo, pre.present, a.multiplo, a.factor,a.activo,  IFNULL(ds.idsimilar,'No') AS grupo\
			FROM articulos a \
			INNER JOIN productos p ON a.producto=p.producto \
			INNER JOIN presentaciones pre ON a.producto=pre.producto AND a.present=pre.present \
			LEFT JOIN dsimilarvxvol ds ON ds.producto=a.producto AND ds.present=a.present\
			where %s p.producto='%s' \
			order by pre.present ASC, a.factor DESC", condicion_solo_activos, clave_producto);
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,
			instruccion.c_str(), Respuesta->TamBufferResultado);

		//articulossupervisados
		instruccion.sprintf(
			"SELECT usuario FROM articulossupervisados where producto='%s' and present = '%s'",
			clave_producto, clave_present);
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,
			instruccion.c_str(), Respuesta->TamBufferResultado);

				// segmentos
		/*instruccion.sprintf(
			"SELECT  if(ps.producto=p.producto,1,0) as marcado, s.*  \
			FROM segmentos s \
			left JOIN productosegmentos ps ON s.segmento=ps.segmento AND ps.articulo='%s' AND ps.producto='%s'  \
			LEFT JOIN productos p ON p.producto=ps.producto ", clave_articulo,clave_producto);  */
        instruccion.sprintf(
			"SELECT   s.*  \
			FROM segmentos s \
			inner join presentacionescb p on p.segmento=s.segmento \
			where  p.producto='%s' and p.idempresa=%s ",
			clave_producto, empresa);
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,
			instruccion.c_str(), Respuesta->TamBufferResultado);
	}
}

// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------

// PRODUCTO

// ---------------------------------------------------------------------------
// ID_CON_PRODUCTO
void ServidorCatalogos::ConsultaProducto(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA PRODUCTO
	AnsiString instruccion;
	AnsiString clave_producto;
	AnsiString clave_clasif1, clave_clasif2;
	AnsiString clave_clasifcont;
	char *resultado;

	clave_producto = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos del producto
	instruccion.sprintf("select p.*, c.cveprodserv AS clavecfdi from productos p INNER JOIN cclaveprodserv c \
	ON c.idclaveprodserv = p.idclaveprodservcfdi where p.producto='%s'", clave_producto);
	resultado = Respuesta->ObtieneDirLibreBuffer();
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	BufferRespuestas resp_producto(resultado);

	if (resp_producto.ObtieneNumRegistros() > 0) {
		clave_clasif1 = resp_producto.ObtieneDato("clasif1");
		clave_clasif2 = resp_producto.ObtieneDato("clasif2");

		clave_clasifcont = resp_producto.ObtieneDato("clasifcont");

		// Clasificacion 2
		instruccion.sprintf(
			"select clasif2, nombre from clasificacion2 where clasif1='%s' order by nombre"
			, clave_clasif1);
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,
			instruccion.c_str(), Respuesta->TamBufferResultado);

		// Clasificacion 3
		instruccion.sprintf(
			"select clasif3, nombre from clasificacion3 where clasif2='%s' order by nombre"
			, clave_clasif2);
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,
			instruccion.c_str(), Respuesta->TamBufferResultado);

		// Clasificacion 2 para contabilidad
		instruccion.sprintf(
			"select clasif2, nombre,idclave from clasifcont2 where clasif='%s' order by nombre"
			, clave_clasifcont);
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,
			instruccion.c_str(), Respuesta->TamBufferResultado);
	}

	// Clasificacion 1
	instruccion.sprintf(
		"select clasif1, nombre from clasificacion1 order by nombre");
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

	// Marcas
	instruccion.sprintf("select marca, nombre from marcas  order by nombre");
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

	// Clasificaciones para contabilidad
	instruccion.sprintf(
		"select clasif, nombre,tipoprod from clasifcont order by nombre");
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

	// Impuestos que usa el producto que se esta cargando y que estan INACTIVOS (para alertar al usuario)
	instruccion.sprintf(" \
			(SELECT ic1.* FROM productos p \
			INNER JOIN impuestos ic1 ON ic1.impuesto=p.claveimpc1 AND ic1.activo=0 \
			WHERE p.producto='%s') \
			UNION \
			(SELECT ic2.* FROM productos p \
			INNER JOIN impuestos ic2 ON ic2.impuesto=p.claveimpc2 AND ic2.activo=0 \
			WHERE p.producto='%s') \
			UNION \
			(SELECT ic3.* FROM productos p \
			INNER JOIN impuestos ic3 ON ic3.impuesto=p.claveimpc3 AND ic3.activo=0 \
			WHERE p.producto='%s') \
			UNION \
			(SELECT ic4.* FROM productos p \
			INNER JOIN impuestos ic4 ON ic4.impuesto=p.claveimpc4 AND ic4.activo=0 \
			WHERE p.producto='%s') \
			UNION \
			(SELECT iv1.* FROM productos p \
			INNER JOIN impuestos iv1 ON iv1.impuesto=p.claveimpv1 AND iv1.activo=0 \
			WHERE p.producto='%s') \
			UNION \
			(SELECT iv2.* FROM productos p \
			INNER JOIN impuestos iv2 ON iv2.impuesto=p.claveimpv2 AND iv2.activo=0 \
			WHERE p.producto='%s') \
			UNION \
			(SELECT iv3.* FROM productos p \
			INNER JOIN impuestos iv3 ON iv3.impuesto=p.claveimpv3 AND iv3.activo=0 \
			WHERE p.producto='%s') \
			UNION \
			(SELECT iv4.* FROM productos p \
			INNER JOIN impuestos iv4 ON iv4.impuesto=p.claveimpv4 AND iv4.activo=0 \
			WHERE p.producto='%s')", clave_producto, clave_producto,
		clave_producto, clave_producto, clave_producto, clave_producto,
		clave_producto, clave_producto);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

	// Impuestos
	instruccion.sprintf(
		"select i.impuesto, t.nombre, i.porcentaje from impuestos i, tiposdeimpuestos t where i.tipoimpu=t.tipoimpu and i.activo=1 AND i.tipoimpu <> 'ISR' order by i.impuesto");
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

	// Impuestos sin IESPS (para los impuestos 2,3 y 4 ya que el IESPS solo puede ser primer)
	instruccion.sprintf(
		"select i.impuesto, t.nombre, i.porcentaje from impuestos i, tiposdeimpuestos t where i.tipoimpu=t.tipoimpu and i.activo=1 and i.tipoimpu<>'IESPS' AND i.tipoimpu <> 'ISR' order by i.impuesto");
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

	// Presentaciones del producto en cuestión.
	instruccion.sprintf(
		"select present from presentaciones where producto='%s' order by present"
		, clave_producto);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

	// Presentaciones y múltiplos del producto en cuestión.
	instruccion.sprintf(
		"select a.articulo, p.present, a.multiplo, a.factor, a.activo from articulos a, presentaciones p where a.present=p.present and a.producto=p.producto and p.producto='%s' order by p.present, a.multiplo", clave_producto);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------

// PRESENTACION

// ---------------------------------------------------------------------------
// ID_CON_PRESENTACION
void ServidorCatalogos::ConsultaPresentacion(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA PRESENTACION
	AnsiString instruccion;
	AnsiString clave_producto, clave_present, empresa;

	clave_producto = mFg.ExtraeStringDeBuffer(&parametros);
	clave_present = mFg.ExtraeStringDeBuffer(&parametros);
	empresa = mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos de la presentacion
	instruccion.sprintf(
		"SELECT pre.*, pcb.costobase, pcb.segmento, IF(ISNULL(pb.producto), 0,1) AS preciosbloqueados \
		 FROM presentaciones pre \
		 LEFT JOIN preciosbloqueados pb ON pb.producto=pre.producto AND pb.present=pre.present AND \
		 pb.fechaVigencia>='%s' AND pb.idempresa=%s \
		 LEFT JOIN presentacionescb pcb ON pcb.producto = pre.producto AND pcb.present = pre.present AND pcb.idempresa = %s \
		 WHERE pre.producto='%s' AND pre.present='%s' ",
		  mFg.DateToMySqlDate(Today()), empresa, empresa,
		  clave_producto, clave_present);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------

// MULTIPLO

// ---------------------------------------------------------------------------
// ID_CON_MULTIPLO
void ServidorCatalogos::ConsultaMultiplo(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA PRESENTACION
	AnsiString instruccion;
	AnsiString clave_articulo, clave_producto;
	AnsiString clave_present, clave_multiplo, empresa;
	char *resultado;

	clave_producto = mFg.ExtraeStringDeBuffer(&parametros);
	clave_present = mFg.ExtraeStringDeBuffer(&parametros);
	clave_multiplo = mFg.ExtraeStringDeBuffer(&parametros);
	empresa = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos del múltiplo
	instruccion.sprintf(
		"select * from articulos where producto='%s' and present='%s' and multiplo='%s'"
		, clave_producto, clave_present, clave_multiplo);
	resultado = Respuesta->ObtieneDirLibreBuffer();
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	BufferRespuestas resp_multiplo(resultado);
	if (resp_multiplo.ObtieneNumRegistros() > 0) {
		// Precios del artículo en cuestión
		clave_articulo = resp_multiplo.ObtieneDato("articulo");
		instruccion.sprintf(
			"select p.tipoprec, t.descripcion, t.porcutil AS UtilidadBase, p.porcutil as UtilidadUsada, p.precio, p.precioproximo, p.actualizarpendiente \
			from tiposdeprecios t, precios p where p.tipoprec=t.tipoprec and p.articulo='%s' and t.idempresa=%s order by p.tipoprec",
			 clave_articulo, empresa);
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,
			instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	else {
		instruccion.sprintf(
			"select t.tipoprec, t.descripcion, t.porcutil As UtilidadBase, t.porcutil2, \
			t.porcutil3, t.porcutil4, t.porcutil5, t.porcutil as UtilidadUsada , 0.0 AS precio, 0.0 AS precioproximo, 0 AS actualizarpendiente \
			from tiposdeprecios t where t.idempresa=%s order by t.tipoprec", empresa);
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,
			instruccion.c_str(), Respuesta->TamBufferResultado);
	}
}
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------

// COLONIAS

// ---------------------------------------------------------------------------
// ID_GRA_COLONIA
void ServidorCatalogos::GrabaColonia(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA COLONIA
	GrabaGenerico(Respuesta, MySQL, parametros, "colonias", "colonia");
}

// ---------------------------------------------------------------------------
// ID_BAJ_COLONIA
void ServidorCatalogos::BajaColonia(RespuestaServidor *Respuesta, MYSQL *MySQL,
	char *parametros) {
	// BAJA COLONIAS
	BajaGenerico(Respuesta, MySQL, parametros, "colonias", "colonia");
}

// ---------------------------------------------------------------------------
// ID_CON_COLONIA
void ServidorCatalogos::ConsultaColonia(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA COLONIA
	AnsiString instruccion;
	AnsiString clave, localidad;

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	localidad = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos de la colonia
	instruccion.sprintf(
		"select colonia, nombre, sector from colonias where colonia='%s'",
		clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todas las colonias que pertenezcan a la localidad
	instruccion.sprintf(
		"select colonia, nombre, sector from colonias where localidad='%s'",
		localidad);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los sectores
	instruccion = "select sector AS Clave, nombre AS Nombre from sectores";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}

// ---------------------------------------------------------------------------

// GRUPOS OBJETOS

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// ID_GRA_GRUPOOBJETOS
void ServidorCatalogos::GrabaGrupoObjetos(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA GRUPO
	GrabaGenerico(Respuesta, MySQL, parametros, "gruposobjetos", "grupo");
}

// ---------------------------------------------------------------------------
// ID_BAJ_GRUPOOBJETOS
void ServidorCatalogos::BajaGrupoObjetos(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// BAJA GRUPO
	BajaGenerico(Respuesta, MySQL, parametros, "gruposobjetos", "grupo");
}

// ---------------------------------------------------------------------------
// ID_CON_GRUPOOBJETOS
void ServidorCatalogos::ConsultaGrupoObjetos(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA GRUPO
	AnsiString instruccion;
	AnsiString clave_grupo;

	clave_grupo = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos del grupo
	instruccion.sprintf("select * from gruposobjetos where grupo='%s'",
		clave_grupo);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los grupos
	instruccion =
		"select grupo AS Grupo, nombre AS Nombre from gruposobjetos order by nombre";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}

// ---------------------------------------------------------------------------

// OBJETOS SISTEMA

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// ID_GRA_OBJETOSISTEMA
void ServidorCatalogos::GrabaObjetoSistema(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA OBJETO
	char *buffer_sql = new char[1024 * 5];
	char *aux_buffer_sql = buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instrucciones[5];
	AnsiString instruccion;
	int num_instrucciones = 0;
	AnsiString clave;
	AnsiString tarea;
	int i;

	try {
		tarea = mFg.ExtraeStringDeBuffer(&parametros);
		// Indicador si se va a agregar o modificar
		clave = mFg.ExtraeStringDeBuffer(&parametros); // Clave del objeto

		datos.AsignaTabla("objetossistema");
		parametros += datos.InsCamposDesdeBuffer(parametros);
		if (tarea == "A") {
			instrucciones[num_instrucciones++] = datos.GenerarSqlInsert();
			instruccion.sprintf(
				"insert into privilegios (objeto, privilegio, descripcion) values('%s', 'CON', 'CONSULTAS')", clave);
			instrucciones[num_instrucciones++] = instruccion;
			instruccion.sprintf(
				"insert into privilegios (objeto, privilegio, descripcion) values('%s', 'MOD', 'MODIFICACIONES')", clave);
			instrucciones[num_instrucciones++] = instruccion;
			instruccion.sprintf(
				"insert into privilegios (objeto, privilegio, descripcion) values('%s', 'ALT', 'ALTAS')", clave);
			instrucciones[num_instrucciones++] = instruccion;
			instruccion.sprintf(
				"insert into privilegios (objeto, privilegio, descripcion) values('%s', 'BAJ', 'BAJAS')", clave);
			instrucciones[num_instrucciones++] = instruccion;
		}
		else {
			instrucciones[num_instrucciones++] = datos.GenerarSqlUpdate
				("objeto='" + clave + "'");
		}

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

// ---------------------------------------------------------------------------
// ID_BAJ_OBJETOSISTEMA
void ServidorCatalogos::BajaObjetoSistema(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// BAJA OBJETO
	char *buffer_sql = new char[1024 * 5];
	char *aux_buffer_sql = buffer_sql;
	AnsiString instrucciones[5];
	AnsiString instruccion;
	int num_instrucciones = 0;
	AnsiString clave;
	int i;

	try {
		clave = mFg.ExtraeStringDeBuffer(&parametros); // Clave del objeto

		instruccion = "delete from asignacionprivilegios where objeto='" +
			clave + "'";
		instrucciones[num_instrucciones++] = instruccion;
		instruccion = "delete from privilegios where objeto='" + clave + "'";
		instrucciones[num_instrucciones++] = instruccion;
		instruccion = "delete from objetossistema where objeto='" + clave + "'";
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

// ---------------------------------------------------------------------------
// ID_CON_OBJETOSISTEMA
void ServidorCatalogos::ConsultaObjetoSistema(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA OBJETO
	AnsiString instruccion;
	AnsiString clave_objeto;

	clave_objeto = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos del objeto
	instruccion.sprintf("select * from objetossistema where objeto='%s'",
		clave_objeto);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los objetos
	instruccion =
		"select objeto AS Objeto, nombre AS Nombre, grupo AS Grupo from objetossistema order by objeto, grupo, nombre";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los grupos
	instruccion.sprintf("select grupo, nombre from gruposobjetos");
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los privilegios del objeto
	instruccion.sprintf(
		"select privilegio, descripcion from privilegios where objeto='%s'",
		clave_objeto);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}

// ---------------------------------------------------------------------------

// PRIVILEGIOS

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// ID_GRA_PRIVILEGIO
void ServidorCatalogos::GrabaPrivilegio(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA GENERICO
	char *buffer_sql = new char[1024 * 5];
	char *aux_buffer_sql = buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instrucciones[5];
	int num_instrucciones = 0;
	AnsiString clave_privilegio, clave_objeto;
	AnsiString tarea;
	int i;

	try {
		tarea = mFg.ExtraeStringDeBuffer(&parametros);
		// Indicador si se va a agregar o modificar
		clave_privilegio = mFg.ExtraeStringDeBuffer(&parametros);
		// Clave del privilegio
		clave_objeto = mFg.ExtraeStringDeBuffer(&parametros);
		// Clave del objeto al que aplica el privilegio

		datos.AsignaTabla("privilegios");
		parametros += datos.InsCamposDesdeBuffer(parametros);
		if (tarea == "A")
			instrucciones[num_instrucciones] = datos.GenerarSqlInsert();
		else
			instrucciones[num_instrucciones] = datos.GenerarSqlUpdate
				("privilegio='" + clave_privilegio + "' and objeto='" +
			clave_objeto + "'");
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

// ---------------------------------------------------------------------------
// ID_BAJ_PRIVILEGIO
void ServidorCatalogos::BajaPrivilegio(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// BAJA GENERICO
	char *buffer_sql = new char[1024 * 5];
	char *aux_buffer_sql = buffer_sql;
	AnsiString instruccion;
	int num_instrucciones = 1;
	AnsiString clave_privilegio, clave_objeto;

	try {
		clave_privilegio = mFg.ExtraeStringDeBuffer(&parametros);
		// Clave del privilegio
		clave_objeto = mFg.ExtraeStringDeBuffer(&parametros);
		// Clave del objeto al que aplica el privilegio

		// Genera la instruccion que borra y la inserta al buffer de acciones.
		instruccion.sprintf(
			"delete from privilegios where privilegio='%s' and objeto='%s'",
			clave_privilegio, clave_objeto);
		aux_buffer_sql = mFg.AgregaStringABuffer
			(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		mFg.AgregaStringABuffer(instruccion, aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta,
			TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,
			buffer_sql);
	}
	__finally {
		delete buffer_sql;
	}
}

// ---------------------------------------------------------------------------
// ID_CON_PRIVILEGIO
void ServidorCatalogos::ConsultaPrivilegio(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA OBJETO
	AnsiString instruccion;
	AnsiString clave_privilegio, clave_objeto;

	clave_privilegio = mFg.ExtraeStringDeBuffer(&parametros);
	// Clave del privilegio
	clave_objeto = mFg.ExtraeStringDeBuffer(&parametros);
	// Clave del objeto al que aplica el privilegio
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos del privilegio
	instruccion.sprintf(
		"select * from privilegios where privilegio='%s' and objeto='%s'",
		clave_privilegio, clave_objeto);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los privilegios del objeto
	instruccion.sprintf(
		"select privilegio, descripcion from privilegios where objeto='%s' order by descripcion", clave_objeto);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------

// TIPOS DE CLASIFICACIONES DE CHEQUES

// ---------------------------------------------------------------------------
// ID_GRA_CHEQUECLASIF
void ServidorCatalogos::GrabaClasifCheque(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA TIPO DE CLASIFICACIONES DE CHEQUES
	GrabaGenerico(Respuesta, MySQL, parametros, "chequesclasif", "clasif");
}

// ---------------------------------------------------------------------------
// ID_BAJ_CHEQUECLASIF
void ServidorCatalogos::BajaClasifCheque(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// BAJA TIPO DE CLASIFICACIONES DE CHEQUES
	BajaGenerico(Respuesta, MySQL, parametros, "chequesclasif", "clasif");
}

// ---------------------------------------------------------------------------
// ID_CON_CHEQUECLASIF
void ServidorCatalogos::ConsultaClasifCheque(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA TIPO DE CLASIFICACIONES DE CHEQUES
	AnsiString instruccion;
	AnsiString clave;

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos del tipo de clasificaciones de cheques
	instruccion.sprintf("select * from chequesclasif where clasif='%s'", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los tipos de clasificaciones de cheques
	instruccion =
		"select clasif, descripcion from chequesclasif order by descripcion";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------

// SECCIONES

// ---------------------------------------------------------------------------
// ID_GRA_SECCION
void ServidorCatalogos::GrabaSeccion(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA SECCION
	char *buffer_sql = new char[1024 * 5];
	char *aux_buffer_sql = buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instrucciones[5];
	AnsiString instruccion;
	int num_instrucciones = 0;
	AnsiString clave;
	AnsiString tarea;
	AnsiString clave_formateada;
	char tipo;
	int i;

	try {
		tarea = mFg.ExtraeStringDeBuffer(&parametros);
		// Indicador si se va a agregar o modificar
		clave = mFg.ExtraeStringDeBuffer(&parametros); // Clave

		instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++] = "START TRANSACTION";

		datos.AsignaTabla("secciones");
		parametros += datos.InsCamposDesdeBuffer(parametros);
		if (tarea == "A") {
			instrucciones[num_instrucciones++] = datos.GenerarSqlInsert();

			// Se supone que no debe haber correspondencia con ARTICULOXSECCION
			// pero para asegurar lo borramos.
			instruccion.sprintf(
				"delete from articuloxseccion where seccion='%s'", clave);
			instrucciones[num_instrucciones++] = instruccion;

		}
		else {
			instrucciones[num_instrucciones++] = datos.GenerarSqlUpdate
				("seccion='" + clave + "'");
			// Cuando se asignen almacenes para cada tipo de articulo, se establece
			// la relación en ARTICULOXSECCION (de no existir ya)
			if (datos.ObtieneValorCampo("almaunid",
					false) != "" && datos.ObtieneValorCampo("almamult",
					false) != "") {
				// Para las nuevas secciones se crean los registros correspondientes
				// en articuloxseccion de acuerdo a los campos ALMAUNID y ALMAMULT
				instruccion.sprintf("insert ignore into articuloxseccion (articulo,seccion,almacen) \
					select a.articulo, s.seccion, if(a.factor=1,s.almaunid,s.almamult) as almacen \
					from secciones s, articulos a \
					where s.seccion='%s'", clave);
				instrucciones[num_instrucciones++] = instruccion;
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

// ---------------------------------------------------------------------------
// ID_BAJ_SECCION
void ServidorCatalogos::BajaSeccion(RespuestaServidor *Respuesta, MYSQL *MySQL,
	char *parametros) {
	// BAJA SECCION
	char *buffer_sql = new char[1024 * 5];
	char *aux_buffer_sql = buffer_sql;
	AnsiString instruccion;
	AnsiString clave;
	AnsiString instrucciones[5];
	int num_instrucciones = 0;
	int i;

	try {
		clave = mFg.ExtraeStringDeBuffer(&parametros);

		instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++] = "START TRANSACTION";

		instruccion.sprintf("delete from articuloxseccion where seccion='%s'",
			clave);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("delete from secciones where seccion='%s'", clave);
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

// ---------------------------------------------------------------------------
// ID_CON_SECCION
void ServidorCatalogos::ConsultaSeccion(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA SECCION
	AnsiString instruccion;
	AnsiString clave;

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos de la seccion
	instruccion.sprintf("select * from secciones where seccion='%s'", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todas las secciones
	instruccion =
		"select seccion AS Seccion, nombre AS Nombre from secciones order by nombre";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todas las sucursales
	instruccion =
		"select sucursal AS Sucursal, nombre AS Nombre from sucursales order by nombre";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los almacenes que pertenecen a la sección
	instruccion.sprintf(
		"select almacen AS Almacen, nombre AS Nombre from almacenes where seccion='%s' order by nombre", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

}

// ---------------------------------------------------------------------------
// ID_GRA_TIPOSFACTURAS
void ServidorCatalogos::GrabaTiposFacturas(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA TIPOSFACTURAS
	GrabaGenerico(Respuesta, MySQL, parametros, "tiposfacturasventas", "tipo");
}

// ---------------------------------------------------------------------------
// ID_CON_TIPOSFACTURAS
void ServidorCatalogos::ConsultaTiposFacturas(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA TIPOS FACTURAS
	AnsiString instruccion;
	AnsiString clave;

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos del tipo
	instruccion.sprintf("select * from tiposfacturasventas where tipo='%s'",
		clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los tipos de facturas
	instruccion = "select tipo, descripcion from tiposfacturasventas";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todas las formas de venta
	instruccion = "select forma, nombre from formas where tipo=0";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todas las formas de notas de crédito
	instruccion = "select forma, nombre from formas where tipo=1";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todas las formas de notas de cargo
	instruccion = "select forma, nombre from formas where tipo=2";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todas las formas de pedidos
	instruccion = "select forma, nombre from formas where tipo=3";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

}

// ---------------------------------------------------------------------------
// ID_BAJ_TIPOSFACTURAS
void ServidorCatalogos::BajaTiposFacturas(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// BAJA TIPOS FACTURAS
	BajaGenerico(Respuesta, MySQL, parametros, "tiposfacturasventas", "tipo");

}

// ---------------------------------------------------------------------------
// ID_GRA_TERMINOPAGO
void ServidorCatalogos::GrabaTerminoPago(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA TERMINO DE PAGO
	GrabaGenerico(Respuesta, MySQL, parametros, "terminosdepago", "termino");
}

// ---------------------------------------------------------------------------
// ID_CON_TERMINOPAGO
void ServidorCatalogos::ConsultaTerminoPago(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA TERMINO DE PAGO
	AnsiString instruccion;
	AnsiString clave;

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos del termino de pago
	instruccion.sprintf("select * from terminosdepago where termino='%s'",
		clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los terminos de pago
	instruccion =
		"select termino, descripcion, diasdefoult, terminoreal, activoappvendedores, activomayoreo, activosuper from terminosdepago";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}

// ---------------------------------------------------------------------------
// ID_BAJ_TERMINOPAGO
void ServidorCatalogos::BajaTerminoPago(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// BAJA TERMINO PAGO
	BajaGenerico(Respuesta, MySQL, parametros, "terminosdepago", "termino");
}

// ---------------------------------------------------------------------------
// ID_CON_FOLIOSCFD
void ServidorCatalogos::ConsultaFoliosCFD(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA FOLIOS DE COMPROBANTES FISCALES DIGITALES
	AnsiString instruccion;
	AnsiString clave;

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos del folio
	instruccion.sprintf("select * from folioscfd where folios='%s'", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los folios
	instruccion =
		"select activa, folios, sucursal, folioinicial, foliofinal, serie, numaprobacion, fechaaprobacion, tipocomprobante, folioactual from folioscfd";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}

// ---------------------------------------------------------------------------
// ID_BAJ_FOLIOSCFD
void ServidorCatalogos::BajaFoliosCFD(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// BAJA FOLIO DE COMPROBANTES FISCALES DIGITALES
	BajaGenerico(Respuesta, MySQL, parametros, "folioscfd", "folios");
}

// ---------------------------------------------------------------------------
// ID_GRA_FOLIOSCFD
void ServidorCatalogos::GrabaFoliosCFD(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA FOLIOS
	char *buffer_sql = new char[1024 * 5];
	char *aux_buffer_sql = buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString tarea_cliente, clave_folio, usuario;
	int i;
	int num_instrucciones = 0;
	AnsiString instruccion, instrucciones[100];

	try {
		usuario = mFg.ExtraeStringDeBuffer(&parametros);
		// Usuario que esta grabando el folio.
		tarea_cliente = mFg.ExtraeStringDeBuffer(&parametros);
		// Indicador si se va a agregar o modificar
		clave_folio = mFg.ExtraeStringDeBuffer(&parametros); // Clave del folio

		datos.AsignaTabla("folioscfd");
		parametros += datos.InsCamposDesdeBuffer(parametros);
		datos.AsignaValorCampo("folios", "@folios", 1);

		instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++] = "START TRANSACTION";

		datos.InsCampo("fechamodi", mFg.DateToAnsiString(Today()));
		datos.InsCampo("horamodi", mFg.TimeToAnsiString(Time()));
		datos.InsCampo("usumodi", usuario);

		if (tarea_cliente == "A") {

			datos.InsCampo("usualta", usuario);
			datos.ReemplazaCampo("folioactual",
				datos.ObtieneValorCampo("folioinicial"));
			datos.InsCampo("fechaalta", mFg.DateToAnsiString(Today()));
			datos.InsCampo("horaalta", mFg.TimeToAnsiString(Time()));

			instrucciones[num_instrucciones++] = datos.GenerarSqlInsert();
		}
		else {
			instruccion.sprintf("set @folios='%s'", clave_folio);
			instrucciones[num_instrucciones++] = instruccion;
			instrucciones[num_instrucciones++] = datos.GenerarSqlUpdate
				("folios='" + clave_folio + "'");
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
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,
				buffer_sql)) {
			instruccion.sprintf("select @folio as folios");
			mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,
				instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	}
	__finally {
		delete buffer_sql;
	}
}

// ---------------------------------------------------------------------------
// ID_CON_CUENTASCONT
void ServidorCatalogos::ConsultaCuentasContables(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA CUENTAS
	AnsiString instruccion;
	AnsiString clave;

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene las cuentas
	instruccion.sprintf(
		"select numcuenta, nombrecuenta, sucursal from cuentascont where sucursal='%s'", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}

// ---------------------------------------------------------------------------
// ID_GRA_CUENTASCONT
void ServidorCatalogos::GrabaCuentasContables(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	char *buffer_sql = new char[1000000];
	char *aux_buffer_sql = buffer_sql;
	int num_instrucciones = 0, num_cuentas, i;
	AnsiString instrucciones[10000], instruccion, sucursal;
	DatosTabla datos(mServidorVioleta->Tablas);
	try {
		sucursal = mFg.ExtraeStringDeBuffer(&parametros);
		instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++] = "START TRANSACTION";
		instruccion.sprintf("delete from cuentascont where sucursal='%s'",
			sucursal);
		instrucciones[num_instrucciones++] = instruccion;
		num_cuentas = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
		for (i = 0; i < num_cuentas; i++) {
			datos.AsignaTabla("cuentascont");
			parametros += datos.InsCamposDesdeBuffer(parametros);
			instrucciones[num_instrucciones++] = datos.GenerarSqlInsert();
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

// ---------------------------------------------------------------------------
// ID_GRA_POLIZA_INTERNA
void ServidorCatalogos::GrabaPolizaInterna(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA UNA POLIZA_INTERNA
	char *buffer_sql = new char[1024 * 64 * 50];
	char *aux_buffer_sql = buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString tarea, clave, usuario, nombre, desglosado, codigobarras;
	int num_partidas, i;
	int num_instrucciones = 0;
	AnsiString instruccion, instrucciones[3000];
	AnsiString sucursal, tipocomprobante, tipoplantilla, fechaini, fechafin, proveedor, pago, pagocliente, cliente;
	AnsiString concepto, fecha, cargos, abonos;
	AnsiString cuenta, tipomov, importe, descrip;
	AnsiString mov_sucdetalle, mov_segmento;

	try {
		sucursal = mFg.ExtraeStringDeBuffer(&parametros);
		tipocomprobante = mFg.ExtraeStringDeBuffer(&parametros);
		tipoplantilla = mFg.ExtraeStringDeBuffer(&parametros);
		fechaini = mFg.ExtraeStringDeBuffer(&parametros);
		fechafin = mFg.ExtraeStringDeBuffer(&parametros);
		proveedor = mFg.ExtraeStringDeBuffer(&parametros);
		pago = mFg.ExtraeStringDeBuffer(&parametros);
		cliente = mFg.ExtraeStringDeBuffer(&parametros);
		pagocliente = mFg.ExtraeStringDeBuffer(&parametros);

		concepto = mFg.ExtraeStringDeBuffer(&parametros);
		// Concepto de la póliza
		fecha = mFg.ExtraeStringDeBuffer(&parametros);
		// Fecha de la póliza en formato dd/mm/yyyy
		cargos = mFg.ExtraeStringDeBuffer(&parametros); // Cargos de la póliza
		abonos = mFg.ExtraeStringDeBuffer(&parametros); // Abonos de la póliza

		instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++] = "START TRANSACTION";

		instruccion = "select @GuidPoliza:=UPPER(UUID()) ";
		instrucciones[num_instrucciones++] = instruccion;


		// Graba la cabecera en la tabla "polizascont"
		datos.AsignaTabla("polizascont");
		datos.InsCampo("sucursal", sucursal);
		datos.InsCampo("tipocomprobante", tipocomprobante);
		datos.InsCampo("tipoplantilla", tipoplantilla);
		datos.InsCampo("fechaini", fechaini);
		datos.InsCampo("fechafin", fechafin);
		datos.InsCampo("neje", fecha.SubString(7, 4));
		datos.InsCampo("nper", fecha.SubString(4, 2));
		datos.InsCampo("ntipopol", "0");
		datos.InsCampo("nclase", "1"); // Normal
		datos.InsCampo("bimpresa", "F"); // No impresa
		datos.InsCampo("mConcepto", concepto);
		datos.InsCampo("dFecha", fecha.SubString(7, 4) + fecha.SubString(4,
				2) + fecha.SubString(1, 2));
		datos.InsCampo("cargos", cargos);
		datos.InsCampo("abonos", abonos);
		datos.InsCampo("ndiario", "2"); // No importa este dato para nuestro propósito
		datos.InsCampo("nSisOrigen", "10"); // Sistema externo a contpaq
		datos.InsCampo("enviado", "0"); // Marca como no enviado a contpaq
		datos.InsCampo("Guid","@GuidPoliza",1); //  "Guid"
		if (tipocomprobante == "PAGO") {
			if (proveedor != " ") {
				datos.InsCampo("proveedor", proveedor);
			}
			if (pago != " ") {
				datos.InsCampo("pagoprov", pago);
			}
		}
		if (tipocomprobante == "COBR" && AnsiString(LeftStr(tipoplantilla,2))=="TR") {
			if (cliente != " ") {
				datos.InsCampo("cliente", cliente);
			}
			if (pagocliente != " ") {
				datos.InsCampo("pagocli", pagocliente);
			}
        }
		instrucciones[num_instrucciones++] = datos.GenerarSqlInsert();

		instruccion = "select @folio:=last_insert_id() ";
		instrucciones[num_instrucciones++] = instruccion;

		// Graba las partidas en "movimientoscont"

		num_partidas = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
		// Obtiene el número de partidas
		for (i = 0; i < num_partidas; i++) {
			cuenta = mFg.ExtraeStringDeBuffer(&parametros);
			tipomov = mFg.ExtraeStringDeBuffer(&parametros);
			importe = mFg.ExtraeStringDeBuffer(&parametros);
			descrip = mFg.ExtraeStringDeBuffer(&parametros);
			mov_sucdetalle = mFg.ExtraeStringDeBuffer(&parametros);
			mov_segmento = mFg.ExtraeStringDeBuffer(&parametros);

			datos.AsignaTabla("movimientoscont");
			datos.InsCampo("npoliza", "@folio", 1);
			datos.InsCampo("neje", fecha.SubString(7, 4));
			datos.InsCampo("nper", fecha.SubString(4, 2));
			datos.InsCampo("ntipopol", "0");
			datos.InsCampo("nmovto", IntToStr(i + 1));
			datos.InsCampo("szcuenta", cuenta);
			datos.InsCampo("btipomov", tipomov);
			datos.InsCampo("Importe", importe);
			datos.InsCampo("nDiario", "2");
			datos.InsCampo("nMoneda", "0");
			datos.InsCampo("szdescrip", descrip);
			datos.InsCampo("dFecha", fecha.SubString(7, 4) + fecha.SubString(4,
					2) + fecha.SubString(1, 2));
			datos.InsCampo("sucdetalle", mov_sucdetalle);
			datos.InsCampo("segmento", mov_segmento);
			datos.InsCampo("Guid", "UPPER(UUID())",1);

			instrucciones[num_instrucciones++] = datos.GenerarSqlInsert();
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
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,
				buffer_sql)) {
			instruccion.sprintf("select @folio as folio, @GuidPoliza as  GuidPoliza");
			mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,
				instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	}
	__finally {
		delete buffer_sql;
	}
}

// ---------------------------------------------------------------------------
// ID_CON_PLANTILLASCFD
void ServidorCatalogos::ConsultaPlantillasCFD(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA PLANTILLAS
	AnsiString instruccion;
	AnsiString empresa, sucursal, tipocomprobante, tipoplantilla, clave;
	int llenarGrid;

	empresa = mFg.ExtraeStringDeBuffer(&parametros);
	sucursal = mFg.ExtraeStringDeBuffer(&parametros);
	tipocomprobante = mFg.ExtraeStringDeBuffer(&parametros);
	tipoplantilla = mFg.ExtraeStringDeBuffer(&parametros);
	clave = mFg.ExtraeStringDeBuffer(&parametros);
	llenarGrid= StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene la plantilla en específico
	instruccion.sprintf("SELECT p.plantilla, p.orden, p.tipopartida, p.numcuenta, p.sucdetalle, \
		 p.tipoafectacion, p.expresion, p.parametros, p.sucursal, p.tipocomprobante, \
		 p.fechainivent, p.fechafinvent, \
		 p.clasifcont, p.clasifcont2, p.clasif1, p.clasif2, p.clasif3, \
		 concat(p.acredito) as acredito, p.termino, \
		 p.tipoimpu, concat(p.impuesto) as impuesto, concat(p.negimpuesto) as negimpuesto, \
		 p.tipoimpu2, concat(p.impuesto2) as impuesto2, concat(p.negimpuesto2) as negimpuesto2, \
		 concat(p.parterel) as parterel, p.formaspago, p.tiporfc, p.idnumcuenta, p.agrupabanco, p.segmentodefault, p.tipogasto \
		FROM plantillaspoliz AS p \
		WHERE p.plantilla='%s'", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

	if (llenarGrid==1) {
		// Obtiene todas las plantillas
		instruccion.sprintf("SELECT p.plantilla, p.orden, p.tipopartida, p.numcuenta, p.sucdetalle, IF(p.numcuenta='AUTO','AUTOMATICA',c.nombrecuenta), \
			 p.tipoafectacion, p.expresion, p.parametros, \
			 if(p.fechainivent='2000-01-01','',p.fechainivent) as fechainivent, \
			 if(p.fechafinvent='2099-12-31','',p.fechafinvent) as fechafinvent, \
			 p.clasifcont, p.clasifcont2, p.clasif1, p.clasif2, p.clasif3, \
			 concat(p.acredito) as acredito, p.termino, \
			 p.tipoimpu, concat(p.impuesto) as impuesto, concat(p.negimpuesto) as negimpuesto, \
			 p.tipoimpu2, concat(p.impuesto2) as impuesto2, concat(p.negimpuesto2) as negimpuesto2, \
			 concat(p.parterel) as parterel, p.formaspago, p.tiporfc, p.idnumcuenta, b.numerocuenta, b.descripcion, \
			 p.agrupabanco, p.segmentodefault, p.tipogasto \
			FROM plantillaspoliz AS p \
			LEFT JOIN cuentascont AS c ON c.numcuenta=p.numcuenta AND c.sucursal=p.sucdetalle \
			LEFT JOIN bancoscuentas AS b ON b.idnumcuenta=p.idnumcuenta \
            LEFT JOIN sucursales suc ON suc.sucursal = p.sucdetalle \
			WHERE suc.idempresa = %s AND p.sucursal='%s' AND p.tipocomprobante='%s' AND p.tipoplantilla='%s' \
			ORDER BY p.orden",
			empresa, sucursal, tipocomprobante, tipoplantilla);
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
			Respuesta->TamBufferResultado);
	}
}

// ---------------------------------------------------------------------------
// ID_BAJ_PLANTILLACFD
void ServidorCatalogos::BajaPlantillasCFD(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// BAJA PLANTILLA DE COMPROBANTES FISCALES DIGITALES
	BajaGenerico(Respuesta, MySQL, parametros, "plantillaspoliz", "plantilla");
}

// ---------------------------------------------------------------------------
// ID_GRA_PLANTILLACFD
void ServidorCatalogos::GrabaPlantillaCFD(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA PLANTILLAS
	char *buffer_sql = new char[1024 * 100];
	char *aux_buffer_sql;
	int num_instrucciones;

	AnsiString instruccion;

	AnsiString actividad = mFg.ExtraeStringDeBuffer(&parametros);
	int plantilla = 0;
	int orden = 0;

	try {
		if (actividad == "M") {
			plantilla = mFg.ExtraeStringDeBuffer(&parametros).ToInt();
			orden = mFg.ExtraeStringDeBuffer(&parametros).ToInt();
		}

		AnsiString tipoComp = mFg.ExtraeStringDeBuffer(&parametros);
		AnsiString tipoPlantilla = mFg.ExtraeStringDeBuffer(&parametros);
		AnsiString tipoPartida = mFg.ExtraeStringDeBuffer(&parametros);
		AnsiString cuenta = mFg.ExtraeStringDeBuffer(&parametros);
		AnsiString afectacion = mFg.ExtraeStringDeBuffer(&parametros);
		AnsiString expresion = mFg.ExtraeStringDeBuffer(&parametros);
		AnsiString sucursal = mFg.ExtraeStringDeBuffer(&parametros);
		AnsiString params = mFg.ExtraeStringDeBuffer(&parametros);

		AnsiString fechainivent = mFg.StrToMySqlDate
			(mFg.ExtraeStringDeBuffer(&parametros));
		AnsiString fechafinvent = mFg.StrToMySqlDate
			(mFg.ExtraeStringDeBuffer(&parametros));
		AnsiString clasifcont = mFg.ExtraeStringDeBuffer(&parametros);
		AnsiString clasifcont2 = mFg.ExtraeStringDeBuffer(&parametros);
		AnsiString clasif1 = mFg.ExtraeStringDeBuffer(&parametros);
		AnsiString clasif2 = mFg.ExtraeStringDeBuffer(&parametros);
		AnsiString clasif3 = mFg.ExtraeStringDeBuffer(&parametros);
		AnsiString acredito = mFg.ExtraeStringDeBuffer(&parametros);
		AnsiString termino = mFg.ExtraeStringDeBuffer(&parametros);

		AnsiString tipo_impuesto = mFg.ExtraeStringDeBuffer(&parametros);
		AnsiString impuesto = mFg.ExtraeStringDeBuffer(&parametros);
		AnsiString negar_impuestos = mFg.ExtraeStringDeBuffer(&parametros);
		AnsiString tipo_impuesto2 = mFg.ExtraeStringDeBuffer(&parametros);
		AnsiString impuesto2 = mFg.ExtraeStringDeBuffer(&parametros);
		AnsiString negar_impuestos2 = mFg.ExtraeStringDeBuffer(&parametros);

		AnsiString parte_relacionada = mFg.ExtraeStringDeBuffer(&parametros);

		AnsiString suc_detalle = mFg.ExtraeStringDeBuffer(&parametros);

		AnsiString formaspago = mFg.ExtraeStringDeBuffer(&parametros);
		AnsiString tiporfc = mFg.ExtraeStringDeBuffer(&parametros);
		AnsiString idnumcuenta = mFg.ExtraeStringDeBuffer(&parametros);
		AnsiString segmento = mFg.ExtraeStringDeBuffer(&parametros);
		AnsiString agrupabanco = mFg.ExtraeStringDeBuffer(&parametros);
		AnsiString tipogasto = mFg.ExtraeStringDeBuffer(&parametros);


		// Varios parámetros pueden ser nulo, entonces forzamos a que se pueda asignar un campo nulo
		// en dichas columnas
		if (strlen(clasifcont.c_str()) == 0)
			clasifcont = "null";
		else
			clasifcont = "'" + clasifcont + "'";
		if (strlen(clasifcont2.c_str()) == 0)
			clasifcont2 = "null";
		else
			clasifcont2 = "'" + clasifcont2 + "'";

		if (strlen(clasif1.c_str()) == 0)
			clasif1 = "null";
		else
			clasif1 = "'" + clasif1 + "'";
		if (strlen(clasif2.c_str()) == 0)
			clasif2 = "null";
		else
			clasif2 = "'" + clasif2 + "'";
		if (strlen(clasif3.c_str()) == 0)
			clasif3 = "null";
		else
			clasif3 = "'" + clasif3 + "'";

		if (strlen(acredito.c_str()) == 0)
			acredito = "null";
		else
			acredito = "'" + acredito + "'";

		if (strlen(termino.c_str()) == 0)
			termino = "null";
		else
			termino = "'" + termino + "'";

		if (strlen(tipo_impuesto.c_str()) == 0)
			tipo_impuesto = "null";
		else
			tipo_impuesto = "'" + tipo_impuesto + "'";
		if (strlen(impuesto.c_str()) == 0)
			impuesto = "null";
		else
			impuesto = "'" + impuesto + "'";
		if (strlen(negar_impuestos.c_str()) == 0)
			negar_impuestos = "null";
		else
			negar_impuestos = "'" + negar_impuestos + "'";

		if (strlen(tipo_impuesto2.c_str()) == 0)
			tipo_impuesto2 = "null";
		else
			tipo_impuesto2 = "'" + tipo_impuesto2 + "'";
		if (strlen(impuesto2.c_str()) == 0)
			impuesto2 = "null";
		else
			impuesto2 = "'" + impuesto2 + "'";
		if (strlen(negar_impuestos2.c_str()) == 0)
			negar_impuestos2 = "null";
		else
			negar_impuestos2 = "'" + negar_impuestos2 + "'";

		if (strlen(parte_relacionada.c_str()) == 0)
			parte_relacionada = "null";
		else
			parte_relacionada = "'" + parte_relacionada + "'";

		if (sucursal!="GG") {
			// Cuando no se trata de pólizas de pagos la sucursal de detalle siempre será la misma que la sucursal de la póliza.
			suc_detalle="'" + sucursal + "'";
		} else {
				if (strlen(suc_detalle.c_str()) == 0) {
					// Cuando es pago y no viene sucursal de detalle, entonces la sucursal de detalle será la misma que la sucursal de la póliza
					suc_detalle="'" + sucursal + "'";
				} else
					suc_detalle = "'" + suc_detalle + "'";
		}

		if (strlen(idnumcuenta.c_str()) == 0)
			idnumcuenta = "null";
		else
			idnumcuenta = "'" + idnumcuenta + "'";

		if (strlen(tipogasto.c_str()) == 0)
			tipogasto = "null";
		else
			tipogasto = "'" + tipogasto + "'";





		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

		if (actividad == "A") {
			// Obtiene orden
			instruccion.sprintf("select MAX(orden) as orden from plantillaspoliz \
				where sucursal='%s' and tipocomprobante='%s' and tipoplantilla='%s' ", sucursal,
				tipoComp, tipoPlantilla);
			mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,
				instruccion.c_str(), Respuesta->TamBufferResultado);
			BufferRespuestas respuestas(Respuesta->BufferResultado);
			if (respuestas.ObtieneNumRegistros() > 0) {
				orden = respuestas.ObtieneDato("orden").ToInt() + 1;
			}
			else {
				orden = 1;
			}
		}

		num_instrucciones = 4;
		aux_buffer_sql = buffer_sql;
		aux_buffer_sql = mFg.AgregaStringABuffer
			(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);

		instruccion = "SET AUTOCOMMIT=0";
		aux_buffer_sql = mFg.AgregaStringABuffer(instruccion, aux_buffer_sql);

		instruccion = "START TRANSACTION";
		aux_buffer_sql = mFg.AgregaStringABuffer(instruccion, aux_buffer_sql);

		// Se guarda la plantilla
		if (actividad == "A") {
			instruccion.sprintf("insert into plantillaspoliz \
				(plantilla, tipocomprobante, tipoPlantilla, orden, numcuenta, tipopartida, tipoafectacion, \
				expresion, sucursal, parametros, clasif1, clasif2, clasif3, \
				fechainivent, fechafinvent, \
				clasifcont, clasifcont2, \
				acredito, termino, \
				tipoimpu, impuesto, negimpuesto, \
				tipoimpu2, impuesto2, negimpuesto2, parterel,sucdetalle, \
				formaspago, tiporfc, idnumcuenta, agrupabanco, segmentodefault, tipogasto \
				) values \
				(%d,'%s','%s',%d,'%s','%s','%s', \
				'%s','%s','%s',%s,%s,%s, \
				'%s','%s', \
				%s,%s, \
				%s, %s, \
				%s, %s, %s, \
				%s, %s, %s, %s, %s, \
				'%s', %s , %s, %s, %s, %s \
				)", plantilla, tipoComp, tipoPlantilla, orden, cuenta, tipoPartida,	afectacion,
				expresion, sucursal, params, clasif1, clasif2, clasif3,
				fechainivent, fechafinvent,
				clasifcont, clasifcont2,
				acredito, termino,
				tipo_impuesto, impuesto, negar_impuestos,
				tipo_impuesto2, impuesto2, negar_impuestos2, parte_relacionada, suc_detalle,
				formaspago, tiporfc, idnumcuenta, agrupabanco ,segmento, tipogasto);
			aux_buffer_sql = mFg.AgregaStringABuffer(instruccion,
				aux_buffer_sql);
		}
		if (actividad == "M") {
			instruccion.sprintf("update plantillaspoliz \
				set numcuenta='%s', tipopartida='%s', tipoafectacion='%s', expresion='%s', parametros='%s', \
				fechainivent='%s', fechafinvent='%s', \
				clasifcont=%s, clasifcont2=%s, \
				clasif1=%s, clasif2=%s, clasif3=%s, \
				acredito=%s, termino=%s, \
				tipoimpu=%s, impuesto=%s, negimpuesto=%s, \
				tipoimpu2=%s, impuesto2=%s, negimpuesto2=%s, parterel=%s, sucdetalle=%s, formaspago='%s', tiporfc=%s, \
				idnumcuenta=%s, agrupabanco=%s ,segmentodefault=%s, tipogasto=%s \
				where plantilla=%d", cuenta, tipoPartida, afectacion,
				expresion, params, fechainivent, fechafinvent, clasifcont,
				clasifcont2, clasif1, clasif2, clasif3, acredito, termino,
				tipo_impuesto, impuesto, negar_impuestos, tipo_impuesto2,
				impuesto2, negar_impuestos2, parte_relacionada, suc_detalle, formaspago, tiporfc,
				idnumcuenta, agrupabanco ,segmento, tipogasto, plantilla);
			aux_buffer_sql = mFg.AgregaStringABuffer(instruccion,
				aux_buffer_sql);
		}

		instruccion = "COMMIT";
		mFg.AgregaStringABuffer(instruccion, aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta,
			TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,
			buffer_sql);

	}
	__finally {
		delete buffer_sql;
	}
}
// ---------------------------------------------------------------------------

// ID_CON_DOMICILIOS
void ServidorCatalogos::ConsultaDomicilios(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA DOMICILIOS DE CLIENTES
	AnsiString instruccion;
	AnsiString clave;

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos del cliente
	instruccion.sprintf(
		"SELECT cliente, rsocial, calle, numext, numint, referenciadomic FROM clientes WHERE cliente='%s'", clave.c_str());
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los domicilios
	instruccion =
		"SELECT cliente, rsocial, calle, numext, numint, referenciadomic FROM clientes ORDER BY rsocial";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}

// ---------------------------------------------------------------------------
// ID_CON_DATOS_CLIENTES
void ServidorCatalogos::ConsultaDatosClientes(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA DATOS DE CLIENTES
	AnsiString instruccion;
	AnsiString clave;

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos del cliente
	instruccion.sprintf(
		"SELECT cliente, rsocial, rfc, cp, tipoempre FROM clientes WHERE cliente='%s'", clave.c_str());
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los clientes
	instruccion =
		"SELECT cliente, rsocial, rfc, cp, tipoempre FROM clientes ORDER BY rsocial";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}

// ---------------------------------------------------------------------------
// ID_GRA_VOLUMENES
void ServidorCatalogos::GrabaVolumenes(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// ALTA DE VOLUMENES
	char *buffer_sql = new char[1024 * 5];
	char *aux_buffer_sql = buffer_sql;
	AnsiString instruccion;
	AnsiString clave;
	AnsiString producto;
	AnsiString present;
	AnsiString minimo;
	AnsiString maximo;
	AnsiString aux_clave = "", aux_min = "", aux_max = "";
	AnsiString instrucciones[20];
	int num_instrucciones = 0;
	int i, TotalRegistros;

	try {
		instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++] = "START TRANSACTION";

		TotalRegistros = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
		for (i = 0; i < TotalRegistros; i++) {
			clave = mFg.ExtraeStringDeBuffer(&parametros);
			producto = mFg.ExtraeStringDeBuffer(&parametros);
			present = mFg.ExtraeStringDeBuffer(&parametros);
			minimo = mFg.ExtraeStringDeBuffer(&parametros);
			maximo = mFg.ExtraeStringDeBuffer(&parametros);
			instruccion.sprintf("update dconceptosventa set cantmin='%s', cantmax='%s', esdefault='0' where \
				producto='%s' and present='%s' and clave='%s'", minimo, maximo,
				producto, present, clave);
			instrucciones[num_instrucciones++] = instruccion;
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

// ---------------------------------------------------------------------------
// ID_GRA_CUOTAS
void ServidorCatalogos::GrabaCuotas(RespuestaServidor *Respuesta, MYSQL *MySQL,
	char *parametros) {
	GrabaGenerico(Respuesta, MySQL, parametros, "cuotastrans", "clave");
}

// ---------------------------------------------------------------------------
// ID_CON_CUOTAS
void ServidorCatalogos::ConsultaCuotas(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	AnsiString instruccion;
	AnsiString clave;

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	instruccion.sprintf("select * from cuotastrans where clave='%s'", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

	instruccion =
		"select clave, destino, cuota from cuotastrans order by destino";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}

// ---------------------------------------------------------------------------
// ID_BAJ_CUOTAS
void ServidorCatalogos::BajaCuotas(RespuestaServidor *Respuesta, MYSQL *MySQL,
	char *parametros) {
	BajaGenerico(Respuesta, MySQL, parametros, "cuotastrans", "clave");
}

// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------

// CLASIFCONT

// ---------------------------------------------------------------------------
// ID_GRA_CLASIFCONTABLE
void ServidorCatalogos::GrabaClasifcont(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA CLASIFCONTABLE
	GrabaGenerico(Respuesta, MySQL, parametros, "clasifcont", "clasif");
}

// ---------------------------------------------------------------------------
// ID_BAJ_CLASIFCONTABLE
void ServidorCatalogos::BajaClasifcont(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// BAJA CLASIFCONTABLE
	BajaGenerico(Respuesta, MySQL, parametros, "clasifcont", "clasif");
}

// ---------------------------------------------------------------------------
// ID_CON_CLASIFCONTABLE
void ServidorCatalogos::ConsultaClasifcont(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA CLASIFCONTABLE
	AnsiString instruccion;
	AnsiString clave;

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos de la CLASIFCONTABLE
	instruccion.sprintf("select * from clasifcont where clasif='%s'", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todas las CLASIF CONTABLES
	instruccion =
		"select clasif AS Clasif, nombre AS Nombre, tipoprod AS TipoProd from clasifcont order by nombre";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}

// ---------------------------------------------------------------------------

// CLASIFCONT 2

// ---------------------------------------------------------------------------
// ID_GRA_CLASIFCONTABLE2
void ServidorCatalogos::GrabaClasifcont2(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA CLASIFCONTABLE2
	GrabaGenerico(Respuesta, MySQL, parametros, "clasifcont2", "clasif2");
}

// ---------------------------------------------------------------------------
// ID_BAJ_CLASIFCONTABLE2
void ServidorCatalogos::BajaClasifcont2(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// BAJA CLASIFCONTABLE2
	BajaGenerico(Respuesta, MySQL, parametros, "clasifcont2", "clasif2");
}

// ---------------------------------------------------------------------------
// ID_CON_CLASIFCONTABLE2
void ServidorCatalogos::ConsultaClasifcont2(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA CLASIFCONTABLE2
	AnsiString instruccion;
	AnsiString clave, clavepadre;

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	clavepadre = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos de la CLASIFCONTABLE2
	instruccion.sprintf(
		"select c.clasif2, c.nombre, c.clasif, c.idclave from clasifcont2 c where c.clasif2='%s'", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todas las CLASIF CONTABLES2
	instruccion.sprintf("select clasif2 AS Clasif2, nombre AS Nombre, idclave AS IdClave \
		from clasifcont2 \
		where clasif='%s' \
		order by nombre", clavepadre);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}

// ---------------------------------------------------------------------------

// MODIFICAR CLAVE DE LA PRESENTACION

// ---------------------------------------------------------------------------
// ID_MOD_PRESENTACION
void ServidorCatalogos::GrabaModPresentacion(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA ID_MOD_PRESENTACION
	char *buffer_sql = new char[1024 * 10];
	char *aux_buffer_sql = buffer_sql;
	AnsiString clave_producto, clave_present_original, clave_present_nueva;
	AnsiString clave_articulo, usuario, idProgramado, sucursalProgramada, fechaProgramada;
	int num_instrucciones = 0;
	AnsiString instruccion, instrucciones[50];

	BufferRespuestas * respuesta_empresas = NULL;

	clave_producto = mFg.ExtraeStringDeBuffer(&parametros);
	clave_present_original = mFg.ExtraeStringDeBuffer(&parametros);
	clave_present_nueva = mFg.ExtraeStringDeBuffer(&parametros);
	clave_articulo = mFg.ExtraeStringDeBuffer(&parametros);
	usuario = mFg.ExtraeStringDeBuffer(&parametros);
	idProgramado = mFg.ExtraeStringDeBuffer(&parametros);
	sucursalProgramada = mFg.ExtraeStringDeBuffer(&parametros);
	fechaProgramada = mFg.ExtraeStringDeBuffer(&parametros);

	try {

		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, "SELECT idempresa FROM empresas", respuesta_empresas);

		instrucciones[num_instrucciones++]="SET SESSION sql_log_bin = 0";
		instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++] = "START TRANSACTION";

		// Borra el registro de presentacionesminmax (posteriormente se regenerará con la nueva).
		instruccion.sprintf("DELETE FROM presentacionesminmax where producto = '%s' and present='%s'",clave_producto, clave_present_original);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf(
			"update presentaciones set present='%s' where producto='%s' and present='%s'"
				, clave_present_nueva, clave_producto, clave_present_original);
		instrucciones[num_instrucciones++] = instruccion;

		// Borra el registro de presentacionesminmax
		instruccion.sprintf("INSERT INTO presentacionesminmax (producto,present,maxfactor,maxmult,minmult,activo) \
			(SELECT amm.producto, amm.present, amm.maxfactor, \
					 amax.multiplo AS maxmult, amin.multiplo AS minmult, 1 AS activo \
			FROM \
				(select a.producto, a.present, MAX(a.factor) AS maxfactor \
					from articulos a WHERE a.activo=1 and a.producto='%s' AND a.present='%s' \
						group by a.producto, a.present) amm \
			INNER JOIN articulos amax ON amm.producto=amax.producto AND amm.present=amax.present AND amm.maxfactor=amax.factor AND amax.activo=1 \
			INNER JOIN articulos amin ON amm.producto=amin.producto AND amm.present=amin.present AND amin.factor=1 AND amin.activo=1 \
			group by amm.producto, amm.present) \
		UNION ALL \
			(SELECT amm.producto, amm.present, amm.maxfactor, \
					 amax.multiplo AS maxmult, amin.multiplo AS minmult, 0 AS activo \
			FROM \
				(select a.producto, a.present, MAX(a.factor) AS maxfactor, MAX(a.activo) AS maxactivo \
					from articulos a WHERE a.producto='%s' AND a.present='%s' \
						group by a.producto, a.present \
						HAVING maxactivo=0 \
						) amm \
			INNER JOIN articulos amax ON amm.producto=amax.producto AND amm.present=amax.present AND amm.maxfactor=amax.factor \
			INNER JOIN articulos amin ON amm.producto=amin.producto AND amm.present=amin.present AND amin.factor=1 \
			group by amm.producto, amm.present)",
			clave_producto, clave_present_nueva, clave_producto, clave_present_nueva);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("update presentacionesminmax pmm \
			LEFT JOIN presentcajlogisfactor pclf ON pmm.producto=pclf.producto AND pmm.present=pclf.present \
			set pmm.cajalogisticafactor=COALESCE(pclf.cajalogisticafactor, pmm.maxfactor) \
			where pmm.producto = '%s' and pmm.present='%s'", clave_producto, clave_present_nueva);
		instrucciones[num_instrucciones++] = instruccion;

		for(int i = 0; i < respuesta_empresas->ObtieneNumRegistros(); i++){
			respuesta_empresas->IrAlRegistroNumero(i);
			AnsiString registroEmpresa = respuesta_empresas->ObtieneDato();

            instruccion.sprintf(
			"update precalculocostos%s set present='%s' where producto='%s' and present='%s'", registroEmpresa, clave_present_nueva, clave_producto, clave_present_original);
			instrucciones[num_instrucciones++] = instruccion;

            instruccion.sprintf(
				"update precalculocostospromedio%s set present='%s' where producto='%s' and present='%s'",
				registroEmpresa, clave_present_nueva, clave_producto, clave_present_original);
			instrucciones[num_instrucciones++] = instruccion;

			instruccion.sprintf(
				"update precalculomensual%s set present='%s' where producto='%s' and present='%s'"
				,registroEmpresa, clave_present_nueva, clave_producto, clave_present_original);
			instrucciones[num_instrucciones++] = instruccion;

            /*Actualiza precalculos historial existencia 03.04.2023 */
			instruccion.sprintf("UPDATE precalculohistoriaconexist%s  SET present = '%s' where producto = '%s' and \
			present = '%s' ", registroEmpresa, clave_present_nueva, clave_producto, clave_present_original);
			instrucciones[num_instrucciones++]=instruccion;


			/*Actualizado 03.04.2023 */
			instruccion.sprintf("UPDATE precalculohistoriaexist%s  SET present = '%s' where producto = '%s' and \
			present = '%s' ", registroEmpresa, clave_present_nueva, clave_producto, clave_present_original);
			instrucciones[num_instrucciones++]=instruccion;

			/*Actualizado 03.04.2023 */
			instruccion.sprintf("UPDATE precalculominmax%s  SET present = '%s' where producto = '%s' and \
			present = '%s' ", registroEmpresa, clave_present_nueva, clave_producto, clave_present_original);
			instrucciones[num_instrucciones++]=instruccion;


			/*Actualizado 03.04.2023 */
			instruccion.sprintf("UPDATE precalculominmaxfin%s  SET present = '%s' where producto = '%s' and \
			present = '%s' ", registroEmpresa, clave_present_nueva, clave_producto, clave_present_original);
			instrucciones[num_instrucciones++]=instruccion;
		}

        instruccion.sprintf(
			"update precalculocostosventadet set present='%s' where producto='%s' and present='%s'"
				, clave_present_nueva, clave_producto, clave_present_original);
		instrucciones[num_instrucciones++] = instruccion;

        instruccion.sprintf(
			"update precalculoventasdet set present='%s' where producto='%s' and present='%s'"
				, clave_present_nueva, clave_producto, clave_present_original);
		instrucciones[num_instrucciones++] = instruccion;


		instruccion.sprintf(
			"update prodbloqueadocompra set present='%s' where producto='%s' and present='%s'"
				, clave_present_nueva, clave_producto, clave_present_original);
		instrucciones[num_instrucciones++] = instruccion;


		/*Actualiza existenciasactuales*/
		instruccion.sprintf("UPDATE existenciasactuales SET present = '%s' where producto = '%s' and \
		present = '%s' ",clave_present_nueva, clave_producto, clave_present_original);
		instrucciones[num_instrucciones++]=instruccion;

		/*Actualizado 03.04.2023 */
		instruccion.sprintf("UPDATE precalculocostosmovalmadet  SET present = '%s' where producto = '%s' and \
		present = '%s' ",clave_present_nueva, clave_producto, clave_present_original);
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("INSERT INTO bitacoramodart \
		(articulo, presentant, presentnuevo, tipo, usumodi, fechamodi, horamodi) \
		VALUES \
		('%s', '%s', '%s', 2, '%s', CURDATE(), CURTIME()) ",
		clave_articulo, clave_present_original, clave_present_nueva, usuario);
		instrucciones[num_instrucciones++]=instruccion;

        instruccion.sprintf("DELETE FROM camprogprodpresent where id = %s", idProgramado);
		instrucciones[num_instrucciones++]=instruccion;

		instrucciones[num_instrucciones++] = "COMMIT";
		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=1";
		instrucciones[num_instrucciones++]="SET SESSION sql_log_bin = 1";

		instruccion.sprintf("INSERT INTO bitprodpresent (tipo, prod_origen, present_origen, present_destino, articulo, usuario, \
			fecha_prog, fecha_apli, suc_prog, mensaje, sucursal, correcto) VALUES ('MPRE','%s', '%s', '%s', '%s', \
			'%s', '%s', NOW(), '%s', 'Presentación cambiada correctamente.' , '%s', 1)",
			clave_producto, clave_present_original, clave_present_nueva, clave_articulo, usuario, fechaProgramada,
			sucursalProgramada, FormServidor->ObtieneClaveSucursal());
		instrucciones[num_instrucciones++]=instruccion;

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql = mFg.AgregaStringABuffer
			(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (int i = 0; i < num_instrucciones; i++)
			aux_buffer_sql = mFg.AgregaStringABuffer(instrucciones[i],
			aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta,
			TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,
			buffer_sql);

	}
	__finally {
		delete buffer_sql;
        delete respuesta_empresas;
	}
}

// ---------------------------------------------------------------------------

// MODIFICAR AUTORIZAR PRECIOS LOCALES

// ---------------------------------------------------------------------------
// ID_GRAB_PRECLOC
void ServidorCatalogos::GrabaAutorizarPrecioLocal(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*2000];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instrucciones[2000];
	AnsiString Instruccion;
	int num_instrucciones=0;
	AnsiString Select, select_present;
	int x, y, i;
	AnsiString sucursal, producto, present, costobase, Fecha, fechahoy, horaactual,usuario,tipo;
	BufferRespuestas* resp_present = NULL;
	BufferRespuestas* resp_sucursales = NULL;
	TTime hora=Time();

	sucursal = mFg.ExtraeStringDeBuffer(&parametros);
	producto = mFg.ExtraeStringDeBuffer(&parametros);
	present = mFg.ExtraeStringDeBuffer(&parametros);
	costobase = mFg.ExtraeStringDeBuffer(&parametros);
	Fecha = mFg.ExtraeStringDeBuffer(&parametros);
	fechahoy = mFg.ExtraeStringDeBuffer(&parametros);
	horaactual = mFg.ExtraeStringDeBuffer(&parametros);
	usuario = mFg.ExtraeStringDeBuffer(&parametros);
	tipo = mFg.ExtraeStringDeBuffer(&parametros);

	if(tipo == "1"){
		try{
			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

			select_present.sprintf("SELECT a.present,p.costo FROM articulos a RIGHT JOIN \
			precios p ON a.articulo = p.articulo \
			INNER JOIN tiposdeprecios tp ON tp.tipoprec = p.tipoprec \
			WHERE a.producto = '%s' AND tp.idempresa=%s GROUP BY a.present", producto, FormServidor->ObtieneClaveEmpresa());
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, select_present.c_str() ,resp_present);

			for(y = 0 ; y < resp_present->ObtieneNumRegistros(); y++ ){

				resp_present->IrAlRegistroNumero(y);
				Instruccion.sprintf("REPLACE into autorizarPrecios \
				(sucursal, producto, present, costobase, usuario, fechaCreacion, horaCreacion, fechaVigencia) \
				values ('%s','%s','%s',%s,'%s','%s','%s','%s'); ",
				sucursal,producto,resp_present->ObtieneDato("present"),resp_present->ObtieneDato("costo"),usuario,
				mFg.StrToMySqlDate(fechahoy),mFg.TimeToMySqlTime(hora), mFg.StrToMySqlDate(Fecha));
				instrucciones[num_instrucciones++]=Instruccion;

				Instruccion.sprintf("REPLACE INTO precioLocal (sucursal,tipoprec,precio,costo,porcutil,fechamodi,articulo) \
				SELECT '%s', p.tipoprec, precio, costo, p.porcutil, p.fechamodi, p.articulo \
				FROM  precios p \
				  INNER JOIN tiposdeprecios tp ON tp.tipoprec = p.tipoprec \
				  INNER JOIN articulos a ON a.articulo=p.articulo \
				  INNER JOIN productos pro ON a.producto=pro.producto \
				WHERE a.producto='%s' AND a.present='%s' AND tp.idempresa=%s \
				ORDER BY tp.tipoprec",sucursal,producto,resp_present->ObtieneDato("present"), FormServidor->ObtieneClaveEmpresa());
				instrucciones[num_instrucciones++]=Instruccion;
			}

			instrucciones[num_instrucciones++]="COMMIT";

			aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
			for (i=0; i<num_instrucciones; i++)
				aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

			mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
			mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);
		}   __finally {
			if(resp_present != NULL) delete resp_present;
			if(buffer_sql != NULL) delete buffer_sql;
			}
	}


	if(tipo == "2"){
		try{
			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

			select_present.sprintf("SELECT a.present,p.costo FROM articulos a RIGHT JOIN \
			precios p ON a.articulo = p.articulo \
			INNER JOIN tiposdeprecios tp ON tp.tipoprec=p.tipoprec AND tp.idempresa=%s \
			 WHERE a.producto = '%s' GROUP BY a.present", FormServidor->ObtieneClaveEmpresa(), producto);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, select_present.c_str() ,resp_present);

			Select= "select s.sucursal, s.nombre from sucursales s order by s.nombre";
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Select.c_str() ,resp_sucursales);

			select_present.sprintf("SELECT a.present,p.costo \
			 FROM articulos a RIGHT JOIN precios p ON a.articulo = p.articulo \
			 INNER JOIN tiposdeprecios tp ON tp.tipoprec=p.tipoprec AND tp.idempresa=%s \
			 WHERE a.producto = '%s' GROUP BY a.present", FormServidor->ObtieneClaveEmpresa(), producto);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, select_present.c_str() ,resp_present);

			for(x = 0 ; x < resp_sucursales->ObtieneNumRegistros(); x++ ){
					 for(y = 0 ; y < resp_present->ObtieneNumRegistros(); y++ ){

						resp_sucursales->IrAlRegistroNumero(x);
						resp_present->IrAlRegistroNumero(y);

						Instruccion.sprintf("REPLACE into autorizarPrecios \
						(sucursal, producto, present, costobase, usuario, fechaCreacion, horaCreacion, fechaVigencia) \
						values ('%s','%s','%s',%s,'%s','%s','%s','%s'); ",
						resp_sucursales->ObtieneDato("sucursal"),producto,resp_present->ObtieneDato("present"),
						resp_present->ObtieneDato("costo"),usuario,
						mFg.StrToMySqlDate(fechahoy),
						mFg.TimeToMySqlTime(hora),
						mFg.StrToMySqlDate(Fecha));
						instrucciones[num_instrucciones++]=Instruccion;

						Instruccion.sprintf("REPLACE INTO precioLocal (sucursal,tipoprec,precio,costo,porcutil,fechamodi,articulo) \
						SELECT '%s', p.tipoprec, precio, costo, p.porcutil, p.fechamodi, p.articulo \
						FROM  precios p \
						INNER JOIN tiposdeprecios tp ON tp.tipoprec = p.tipoprec \
						INNER JOIN articulos a ON a.articulo=p.articulo \
						INNER JOIN productos pro ON a.producto=pro.producto \
						WHERE a.producto='%s' AND a.present='%s' AND tp.idempresa=%s \
						ORDER BY tp.tipoprec",resp_sucursales->ObtieneDato("sucursal"),producto,resp_present->ObtieneDato("present"),
						FormServidor->ObtieneClaveEmpresa());
						instrucciones[num_instrucciones++]=Instruccion;

				  }
			}
						instrucciones[num_instrucciones++]="COMMIT";

						aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
						for (i=0; i<num_instrucciones; i++)
							aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

						mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
						mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);
			}
			__finally {
						if(resp_present != NULL) delete resp_present;
						if(resp_sucursales != NULL) delete resp_sucursales;
						if(buffer_sql != NULL) delete buffer_sql;
					  }

	}

	if(tipo == "3") {
		try{

			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

			Instruccion.sprintf("REPLACE into autorizarPrecios \
			(sucursal, producto, present, costobase, usuario, fechaCreacion, horaCreacion, fechaVigencia) \
			values ('%s','%s','%s',%s,'%s','%s','%s','%s'); ",
			sucursal,producto,present,costobase,usuario,
			mFg.StrToMySqlDate(fechahoy),
			mFg.TimeToMySqlTime(hora),
			mFg.StrToMySqlDate(Fecha));
			instrucciones[num_instrucciones++]=Instruccion;

			Instruccion.sprintf("REPLACE INTO precioLocal (sucursal,tipoprec,precio,costo,porcutil,fechamodi,articulo) \
			SELECT '%s', p.tipoprec, precio, costo, p.porcutil, p.fechamodi, p.articulo \
			FROM  precios p \
			INNER JOIN tiposdeprecios tp ON tp.tipoprec = p.tipoprec \
			INNER JOIN articulos a ON a.articulo=p.articulo \
			INNER JOIN productos pro ON a.producto=pro.producto \
			WHERE a.producto='%s' AND a.present='%s' AND tp.idempresa=%s \
			ORDER BY tp.tipoprec",sucursal,producto,present, FormServidor->ObtieneClaveEmpresa());
			instrucciones[num_instrucciones++]=Instruccion;

			instrucciones[num_instrucciones++]="COMMIT";

			aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
			for (i=0; i<num_instrucciones; i++)
				aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

			mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
			mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);
		}
		__finally{
			if(buffer_sql != NULL) delete buffer_sql;
		}

	}

	if(tipo == "4") {
		try{

			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

			Select= "select s.sucursal, s.nombre from sucursales s order by s.nombre";
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Select.c_str() ,resp_sucursales);

			for(x = 0 ; x < resp_sucursales->ObtieneNumRegistros(); x++ ){
				resp_sucursales->IrAlRegistroNumero(x);

				Instruccion.sprintf("REPLACE into autorizarPrecios \
				(sucursal, producto, present, costobase, usuario, fechaCreacion, horaCreacion, fechaVigencia) \
				values ('%s','%s','%s',%s,'%s','%s','%s','%s'); ",
				resp_sucursales->ObtieneDato("sucursal"),producto,present,costobase,usuario,
				mFg.StrToMySqlDate(fechahoy),
				mFg.TimeToMySqlTime(hora),
				mFg.StrToMySqlDate(Fecha));
				instrucciones[num_instrucciones++]=Instruccion;

				Instruccion.sprintf("REPLACE INTO precioLocal (sucursal,tipoprec,precio,costo,porcutil,fechamodi,articulo) \
				SELECT '%s', p.tipoprec, precio, costo, p.porcutil, p.fechamodi, p.articulo \
				FROM  precios p \
				  INNER JOIN tiposdeprecios tp ON tp.tipoprec = p.tipoprec \
				  INNER JOIN articulos a ON a.articulo=p.articulo \
				  INNER JOIN productos pro ON a.producto=pro.producto \
				WHERE a.producto='%s' AND a.present='%s' AND tp.idempresa=%s \
				ORDER BY tp.tipoprec",resp_sucursales->ObtieneDato("sucursal"),producto,present, FormServidor->ObtieneClaveEmpresa());
				instrucciones[num_instrucciones++]=Instruccion;

			}

			instrucciones[num_instrucciones++]="COMMIT";

			aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
			for (i=0; i<num_instrucciones; i++)
				aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

			mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
			mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);
			}
			 __finally{
				if(resp_sucursales != NULL) delete resp_sucursales;
				if(buffer_sql != NULL) delete buffer_sql;
			}



	}


}

// ID_GRA_PRECBLOQ
void ServidorCatalogos::GrabaPreciosBloqueados(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*3000];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i, y;
	AnsiString Instruccion, instrucciones[20000];
	AnsiString producto, fechahoy, horaactual, usuario, fechaVigencia;
	AnsiString marca, seleccionaMarca;
	AnsiString fab, selFab;
	AnsiString clasifM, selClasifM;
	AnsiString clasifG, selClasifG;
	AnsiString clasifE, selClasifE;
	AnsiString segCom, selSegCom;
	AnsiString clavProd, selClavProd, empresa;
	AnsiString condicion_marca=" ", condicion_fabr=" ", condicion_clasifm=" ", condicion_clasifg=" ";
	AnsiString condicion_clasife=" ", condicion_segCom=" ", condicion_clavProd=" ";

	AnsiString select_present;
	AnsiString insert, instruccion;
	BufferRespuestas* resp_present = NULL;
	TTime hora=Time();

	producto = mFg.ExtraeStringDeBuffer(&parametros);
	usuario = mFg.ExtraeStringDeBuffer(&parametros);
	fechahoy = mFg.ExtraeStringDeBuffer(&parametros);
	horaactual = mFg.ExtraeStringDeBuffer(&parametros);
	fechaVigencia = mFg.ExtraeStringDeBuffer(&parametros);

	seleccionaMarca = mFg.ExtraeStringDeBuffer(&parametros);
	marca = mFg.ExtraeStringDeBuffer(&parametros);

	selFab = mFg.ExtraeStringDeBuffer(&parametros);
	fab = mFg.ExtraeStringDeBuffer(&parametros);

	selClasifM = mFg.ExtraeStringDeBuffer(&parametros);
	clasifM = mFg.ExtraeStringDeBuffer(&parametros);

	selClasifG = mFg.ExtraeStringDeBuffer(&parametros);
	clasifG = mFg.ExtraeStringDeBuffer(&parametros);

	selClasifE = mFg.ExtraeStringDeBuffer(&parametros);
	clasifE = mFg.ExtraeStringDeBuffer(&parametros);

	selSegCom = mFg.ExtraeStringDeBuffer(&parametros);
	segCom = mFg.ExtraeStringDeBuffer(&parametros);

	selClavProd = mFg.ExtraeStringDeBuffer(&parametros);
	clavProd = mFg.ExtraeStringDeBuffer(&parametros);

	empresa = mFg.ExtraeStringDeBuffer(&parametros);



	//iniciar la transaccion
	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

		try{
		if((seleccionaMarca=="0" && selFab=="0" && selClasifM=="0" && selClasifG=="0" && selClasifE=="0" && selSegCom=="0" && selClavProd=="0") && producto!=""){

			select_present.sprintf("SELECT a.present FROM articulos a \
			RIGHT JOIN precios p ON a.articulo = p.articulo \
			LEFT JOIN tiposdeprecios tp ON tp.tipoprec=p.tipoprec \
			 WHERE a.producto = '%s' AND tp.idempresa=%s \
			 GROUP BY a.present", producto, FormServidor->ObtieneClaveEmpresa());

			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, select_present.c_str(), resp_present);

			for(y = 0 ; y < resp_present->ObtieneNumRegistros(); y++ ){

				resp_present->IrAlRegistroNumero(y);
				insert.sprintf("replace into preciosbloqueados \
				( producto, present, usuario, fechaCreacion, horaCreacion, fechaVigencia, idempresa) \
					values ('%s','%s','%s','%s','%s','%s', %s) ",
						producto, resp_present->ObtieneDato("present"), usuario,
						mFg.StrToMySqlDate(fechahoy),
						mFg.TimeToMySqlTime(hora),
						mFg.StrToMySqlDate(fechaVigencia), empresa);
				instrucciones[num_instrucciones++]=insert;
			}
		}

		if(seleccionaMarca == "1" || selFab == "1" || selClasifM == "1" || selClasifG == "1" || selClasifE == "1" ||
		selSegCom == "1" || selClavProd == "1"){

			if(seleccionaMarca == "1"){
				condicion_marca.sprintf(" AND p.marca = '%s' ", marca);
			}

			if(selFab == "1"){
				condicion_fabr.sprintf(" AND p.fabricante = '%s' ", fab);
			}

			if(selClasifM == "1"){
				condicion_clasifm.sprintf(" AND p.clasif1 = '%s' ", clasifM);
			}

			if(selClasifG == "1"){
				condicion_clasifg.sprintf(" AND p.clasif2 = '%s' ", clasifG);
			}

			if(selClasifE == "1"){
				condicion_clasife.sprintf(" AND p.clasif3 = '%s' ", clasifE);
			}

			if(selSegCom == "1"){
				condicion_segCom.sprintf(" AND ps.segmento = '%s' ", segCom);
			}

			if(selClavProd == "1"){
				condicion_clavProd.sprintf(" AND p.producto = '%s' ", clavProd);
			}

			select_present.sprintf("SELECT p.producto, a.present FROM articulos a \
			INNER JOIN productos p ON p.producto = a.producto \
			INNER JOIN presentacionescb ps ON ps.producto = a.producto AND ps.present = a.present AND ps.idempresa=%s \
			WHERE 1 %s %s %s %s %s %s %s GROUP BY p.nombre,p.producto,a.present ",
			empresa, condicion_marca, condicion_fabr, condicion_clasifm,
			condicion_clasifg, condicion_clasife, condicion_segCom, condicion_clavProd);

			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, select_present.c_str(), resp_present);

			for(y = 0 ; y < resp_present->ObtieneNumRegistros(); y++ ){

				resp_present->IrAlRegistroNumero(y);
				insert.sprintf("replace into preciosbloqueados \
				( producto, present, usuario, fechaCreacion, horaCreacion, fechaVigencia, idempresa) \
					values ('%s','%s','%s','%s','%s','%s',%s); ",
						resp_present->ObtieneDato("producto"), resp_present->ObtieneDato("present"), usuario,
						mFg.StrToMySqlDate(fechahoy),
						mFg.TimeToMySqlTime(hora),
						mFg.StrToMySqlDate(fechaVigencia), empresa);
				instrucciones[num_instrucciones++]=insert;
			}
		}

		instrucciones[num_instrucciones++]="COMMIT";

		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);
	   }
		__finally {
		if(resp_present != NULL) delete resp_present;
		if(buffer_sql != NULL) delete buffer_sql;
		}


}

//---------------------------------------------------------------------------
//ID_BUSQ_BITPRECDIF
void ServidorCatalogos::BuscaBitPrecDif(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// BUSQUEDA DE MOVIMIENTOS BANCARIOS
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instruccion;
	AnsiString fechaini, fechafin, producto, presentacion, cadenaproducto = " ", condicion_presentacion = " ", empresa;

	fechaini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
	fechafin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
	producto=mFg.ExtraeStringDeBuffer(&parametros);
	presentacion =mFg.ExtraeStringDeBuffer(&parametros);
	empresa =mFg.ExtraeStringDeBuffer(&parametros);

	if(producto != " ")
		cadenaproducto.sprintf(" AND a.producto = '%s' ", producto);

	if(presentacion != " ")
		condicion_presentacion.sprintf(" AND a.present = '%s' ", presentacion);

	if(empresa == " ")
		empresa =  FormServidor->ObtieneClaveEmpresa();


	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ);

	instruccion.sprintf("SELECT bpd.*, IF(e.nombre IS NULL, 'SISTEMA', CONCAT(e.nombre,' ',e.appat,' ',e.apmat) ) AS nomusuario   				\
				FROM bitacorapreciosdiferidos bpd LEFT JOIN empleados e ON bpd.usuario = e.empleado        \
					INNER JOIN bitacorapreciosdiferidosdetalle bpdd ON bpd.idbitacora = bpdd.referencia 		\
					INNER JOIN articulos a ON a.articulo = bpdd.articulo                               			\
					INNER JOIN productos pro ON a.producto = pro.producto                                       \
					INNER JOIN tiposdeprecios tp ON tp.tipoprec = bpdd.tipoprec AND tp.verprecdif = '1'         \
				WHERE bpd.fecha >= '%s' AND bpd.fecha <= '%s' AND tp.idempresa=%s %s %s                                  \
					 GROUP BY bpd.idbitacora ORDER BY bpd.fecha DESC , bpd.hora DESC",
					fechaini, fechafin, empresa,
					 cadenaproducto, condicion_presentacion);

	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);


}

//---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// ID_GRA_PRODSERV
void ServidorCatalogos::GrabaProdServ(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA PRDUCTO/SERVICIO
	GrabaGenerico(Respuesta, MySQL, parametros, "cclaveprodserv", "cveprodserv");
}

// ---------------------------------------------------------------------------
// ID_CON_PRODSERV
void ServidorCatalogos::ConsultaProdServ(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA PRODUCTOS/SERVICIOS
	AnsiString instruccion;
	AnsiString clave_prodserv, activos, condicion_activos, carga;
	AnsiString fechaaux;

	clave_prodserv = mFg.ExtraeStringDeBuffer(&parametros);
	activos = mFg.ExtraeStringDeBuffer(&parametros);
	carga = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	if(activos == "1")
		condicion_activos = " OR estado = '1'";
		else
			condicion_activos = " ";

	fechaaux="%d/%m/%Y";

	// Obtiene todos los datos del producto/servicio
	instruccion.sprintf("select idclaveprodserv,cveprodserv,descripcion,fechainivig,\
			IF(fechafinvig='0000-00-00', DATE_FORMAT( DATE_ADD(fechainivig, INTERVAL 50 YEAR), '%s') ,'') AS fechafinvig,\
			 incluiriva,incluirieps,estado from cclaveprodserv where cveprodserv='%s'",
		fechaaux,clave_prodserv);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todas los productos/servicios
	if(carga == "1"){
	instruccion.sprintf("SELECT idclaveprodserv,cveprodserv, descripcion, fechainivig, \
		IF(fechafinvig='0000-00-00', DATE_FORMAT( DATE_ADD(fechainivig, INTERVAL 50 YEAR), '%s') ,'') AS fechafinvig,\
		IF(incluiriva = 0 ,'OPCIONAL','SI') AS iva , IF(incluirieps = 0, 'OPCIONAL','NO') AS ieps, \
		estado FROM cclaveprodserv WHERE estado = '0' %s order by idclaveprodserv", fechaaux,condicion_activos);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
    }
}
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// ID_BAJ_PRODSERV
void ServidorCatalogos::BajaProdServ(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros) {
	// BAJA PRODUCTOS/SERVICIOS
	BajaGenerico(Respuesta, MySQL, parametros, "cclaveprodserv", "cveprodserv");
}

// ---------------------------------------------------------------------------

//---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// ID_GRA_CLAVEUNI
void ServidorCatalogos::GrabaClaveUni(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA CLAVE UNIDAD
	GrabaGenerico(Respuesta, MySQL, parametros, "cclaveunidad", "claveunidad");
}

// ---------------------------------------------------------------------------
// ID_CON_CLAVEUNI
void ServidorCatalogos::ConsultaClaveUni(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA CLAVE UNIDAD
	AnsiString instruccion;
	AnsiString clave_uni, activos, condicion_activos;

	clave_uni = mFg.ExtraeStringDeBuffer(&parametros);
	activos = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	if(activos == "1")
		condicion_activos = " OR estado = '1'";
		else
			condicion_activos = " ";

	// Obtiene todos los datos de la clave unidad
	instruccion.sprintf("select * from cclaveunidad where claveunidad='%s'",
		clave_uni);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todas las claves unidad
	instruccion.sprintf("SELECT idclaveunidad,claveunidad, nombre, descripcion, fechaIniVig, fechafinVig, \
		simbolo, estado FROM cclaveunidad WHERE estado = '0' %s order by idclaveunidad", condicion_activos);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// ID_BAJ_CLAVEUNI
void ServidorCatalogos::BajaClaveUni(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros) {
	// BAJA CLave unidad
	BajaGenerico(Respuesta, MySQL, parametros, "cclaveunidad", "claveunidad");
}

// ---------------------------------------------------------------------------

// ID_COPIA_PLANPOL
void ServidorCatalogos::CopiaPlantilla(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*100];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i;
	AnsiString instruccion, instrucciones[10];

	AnsiString tipoplantillaOrigen, sucursalOrigen;
	AnsiString sucursalDestino, tipoplantillaDestino, tipoComprobanteDestino;
	AnsiString sucespecial, Incrementa;

	tipoplantillaOrigen = mFg.ExtraeStringDeBuffer(&parametros);
	sucursalOrigen = mFg.ExtraeStringDeBuffer(&parametros);
	sucursalDestino = mFg.ExtraeStringDeBuffer(&parametros);
	tipoplantillaDestino = mFg.ExtraeStringDeBuffer(&parametros);
	tipoComprobanteDestino = mFg.ExtraeStringDeBuffer(&parametros);
	Incrementa = mFg.ExtraeStringDeBuffer(&parametros);

    try{
	//iniciar la transaccion
	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	if (Incrementa == "no") {
        instruccion.sprintf("DELETE FROM plantillaspoliz WHERE sucursal = '%s' AND \
				tipoplantilla = '%s' AND tipocomprobante = '%s'",
				sucursalDestino, tipoplantillaDestino, tipoComprobanteDestino);
		instrucciones[num_instrucciones++]=instruccion;
	}

	if(sucursalDestino == "GG"){
		sucespecial = sucursalOrigen;
        //se agrega la plantilla extra de la nueva sucursal si es que se está sumando a la plantilla global
		instruccion.sprintf(" set @OrdenSum=(select MAX(orden) from plantillaspoliz \
			where sucursal='%s' and tipoplantilla='%s' and tipocomprobante='%s')",
		sucursalDestino, tipoplantillaOrigen, tipoComprobanteDestino);
		instrucciones[num_instrucciones++]=instruccion;
	}
	else{
		sucespecial = sucursalDestino;

		instruccion.sprintf(" set @OrdenSum=0 ");
		instrucciones[num_instrucciones++]=instruccion;
	}

	instruccion.sprintf(" INSERT INTO plantillaspoliz  \
		SELECT NULL,tipocomprobante, '%s' AS tipoplantilla, tipopartida, orden+ifnull(@OrdenSum,0) as orden, numcuenta, tipoafectacion, \
		expresion, '%s' AS sucursal, parametros, clasif1, clasif2, clasif3, termino, impuesto, acredito, clasifcont, \
		clasifcont2, tipoimpu, negimpuesto, impuesto2, tipoimpu2, negimpuesto2, parterel, fechainivent,  fechafinvent, \
		'%s' AS sucdetalle, formaspago, tiporfc, idnumcuenta, agrupabanco, segmentodefault, tipogasto \
						FROM plantillaspoliz \
		WHERE sucursal = '%s' AND tipoplantilla = '%s' AND tipocomprobante = '%s'",
		tipoplantillaDestino, sucursalDestino,sucespecial, sucursalOrigen, tipoplantillaOrigen, tipoComprobanteDestino);
		instrucciones[num_instrucciones++]=instruccion;


	instrucciones[num_instrucciones++]="COMMIT";

	aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
	for (i=0; i<num_instrucciones; i++)
		aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
	mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);
	}
	__finally {
	if(buffer_sql != NULL) delete buffer_sql;
	}


}

//GIRONEGOCIO
//---------------------------------------------------------------------------
// ID_CON_GIRO
void ServidorCatalogos::ConsultaGiro(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA MARCA
	AnsiString instruccion;
	AnsiString clave;

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos del giro
	instruccion.sprintf("select * from gironegocio where giro='%s'", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los giros
	instruccion =
		"select giro AS Giro, nombre AS Nombre from gironegocio order by nombre";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}

// ---------------------------------------------------------------------------
// ID_GRA_GIRO
void ServidorCatalogos::GrabaGiro(RespuestaServidor *Respuesta, MYSQL *MySQL,
	char *parametros) {
	// GRABA GIRO
	GrabaGenerico(Respuesta, MySQL, parametros, "gironegocio", "giro");
}
// ---------------------------------------------------------------------------
// ID_BAJ_MARCA
void ServidorCatalogos::BajaGiro(RespuestaServidor *Respuesta, MYSQL *MySQL,
	char *parametros) {
	// BAJA MARCA
	BajaGenerico(Respuesta, MySQL, parametros, "gironegocio", "giro");
}

// ---------------------------------------------------------------------------
// ID_CON_VERIFICADORIMG
void ServidorCatalogos::ConsultaVerificadorImg(RespuestaServidor *Respuesta, MYSQL *MySQL,
	char *parametros) {
	// CONSULTA VerificadorImg
	AnsiString instruccion;
	AnsiString fecha_actual,validaf, ultima_consulta_DD, ultima_consulta_HH;

	validaf = mFg.ExtraeStringDeBuffer(&parametros);
	ultima_consulta_DD = mFg.ExtraeStringDeBuffer(&parametros);
	ultima_consulta_HH = mFg.ExtraeStringDeBuffer(&parametros);


	fecha_actual = mFg.DateToMySqlDate(Today());
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_REP*2);
	// Obtiene todos los datos de la tabla verificadorimg donde sean vigentes las fechas
	if (validaf=="S") {
		validaf=" and fechaalta>='" + ultima_consulta_DD + "' and horaalta>'" + ultima_consulta_HH + "' ";
		instruccion.sprintf("select * from verificadorimg where fechavigencia>='%s' %s limit 35",
		fecha_actual,validaf);
	}
	else
		instruccion.sprintf("select * from verificadorimg where fechavigencia>='%s' limit 35",
		fecha_actual);

	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

}
// ---------------------------------------------------------------------------
//ID_CARGA_EMBARQUE_RUTA
void ServidorCatalogos::CargaEmbarqueRuta(RespuestaServidor *Respuesta, MYSQL *MySQL,char *parametros) {

	AnsiString instruccion;
	AnsiString mClaveEmbarque;
	bool mExisteEmbarque;

	mClaveEmbarque = mFg.ExtraeStringDeBuffer(&parametros);
	BufferRespuestas* cons_embarque=NULL;
	BufferRespuestas* cons_rutas=NULL;

	try{
		instruccion.sprintf("SELECT * FROM embarquesruta WHERE embarque='%s' ",mClaveEmbarque);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), cons_embarque);

		instruccion.sprintf("SELECT p.cliente as suma \
			FROM pedidosventa p \
			LEFT JOIN pedidosdirent AS pde ON pde.referencia = p.referencia \
			INNER JOIN clientes c ON c.cliente = p.cliente \
			LEFT JOIN colonias col ON col.colonia = pde.colonia \
			LEFT JOIN localidades AS l ON l.localidad = col.localidad \
			LEFT JOIN municipios AS m ON m.municipio = l.municipio \
			LEFT JOIN pedidosmensajes AS pm ON pm.referencia = p.referencia \
			WHERE p.embarque='%s' GROUP BY p.cliente,pde.calle ",mClaveEmbarque);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), cons_rutas);

		if (cons_rutas->ObtieneNumRegistros() > cons_embarque->ObtieneNumRegistros())
			mExisteEmbarque=false;
		else
			mExisteEmbarque=true;
	}__finally{
		if (cons_embarque!=NULL) delete cons_embarque;
		if (cons_rutas!=NULL) delete cons_rutas;
	}

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	if(mExisteEmbarque)
	{


			instruccion.sprintf("             \
			SELECT                            \
			er.idorden,                       \
			er.cliente,                       \
			c.rsocial AS rsocial,             \
			CONCAT(er.calle, ' ', er.numext) AS calleynumero,                 \
			col.nombre AS colonia,                                            \
			l.nombre AS localidad,                                            \
			m.nombre AS municipio,                                            \
			IF (FORMAT(X(er.ubicaciongis),6) IS NOT NULL, IF(FORMAT(X(er.ubicaciongis),6)<>0.000000, FORMAT(X(er.ubicaciongis),6), ''), '') AS latitud,    \
			IF (FORMAT(Y(er.ubicaciongis),6) IS NOT NULL, IF(FORMAT(Y(er.ubicaciongis),6)<>0.000000, FORMAT(Y(er.ubicaciongis),6), ''), '') AS longitud,   \
			er.mensaje as pedidomensaje,                                                                                                                                           \
			' ',                                                                                                                                           \
			' ',                                                                                                                                           \
			CONCAT(IFNULL(FORMAT(X(er.ubicaciongis),6),0),',', IFNULL(FORMAT(Y(er.ubicaciongis),6),0)) AS ubicacion,                                       \
			er.calle,                                                                                                                                      \
			er.numext,                                                                                                                                     \
			er.numint,                                                                                                                                     \
			er.cp,                                                                                                                                         \
			er.colonia,                                                                                                                                    \
			er.referenciadomic,                                                                                                                            \
			er.choferllega,                                                                                                                                \
			er.chofersale,                                                                                                                                 \
			er.fechadellegada,                                                                                                                             \
			er.horadellegada,                                                                                                                              \
			er.fechadesalida,                                                                                                                              \
			er.horadesalida,                                                                                                                               \
			CONCAT(IFNULL(FORMAT(X(er.ubicaciondellegada),6),0),',', IFNULL(FORMAT(Y(er.ubicaciondellegada),6),0)) AS ubicaciondellegada,                  \
			CONCAT(IFNULL(FORMAT(X(er.ubicaciondesalida),6),0),',', IFNULL(FORMAT(Y(er.ubicaciondesalida),6),0)) AS ubicaciondesalida,                     \
			er.observacioneschofer,                                                                                                                        \
			er.etapa, case er.etapa when 0 then '' when 1 then 'EN RUTA'  when 2 then 'EN SITIO'  when 9 then 'COMPLETADO' END AS etapadescrip			   \
			FROM                                                                                                                                           \
			embarquesruta AS er                                                                                                                            \
			INNER JOIN 		clientes AS c ON c.cliente=er.cliente                                                                                          \
			LEFT JOIN 		colonias col ON col.colonia=er.colonia                                                                                         \
			LEFT JOIN 		localidades AS l ON l.localidad=col.localidad                                                                                  \
			LEFT JOIN 		municipios AS m ON m.municipio=l.municipio                                                                                     \
			LEFT JOIN 		estados AS e ON e.estado=m.estado                                                                                              \
			WHERE                                                                                                                                          \
			er.embarque='%s'   ORDER BY  er.idorden ",mClaveEmbarque);

			if (!mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado))
					 throw (Exception("Error al cargar el embarque"));


	}else
		{

				instruccion.sprintf("                \
				SELECT        						 \
				1 AS idorden,                        \
				p.cliente AS CodigoCliente,          \
				c.rsocial AS rsocial,                \
				CONCAT(IFNULL(pde.calle,''),  \
				' ',                                 \
				IFNULL(pde.numext,'')) AS calleynumero,         \
				IFNULL(col.nombre,'') AS colonia,           					    \
				IFNULL(l.nombre,'') AS localidad,                                  \
				IFNULL(m.nombre,'') AS municipio,                                  \
				CASE WHEN IFNULL(pde.id_pedirent,0)>0                   \
				THEN                                                    \
				IF (FORMAT(X(pde.ubicaciongis),6) IS NOT NULL, IF(FORMAT(X(pde.ubicaciongis),6)<>0.000000, FORMAT(X(pde.ubicaciongis),6), ''), '')        \
				ELSE                                                                                                                                      \
				(0.000000)																													              \
				END                                                                                                                                       \
				AS latitud,                                                                                                                               \
				CASE WHEN IFNULL(pde.id_pedirent,0)>0                                                                                                     \
				THEN                                                                                                                                      \
				IF (FORMAT(Y(pde.ubicaciongis),6) IS NOT NULL, IF(FORMAT(Y(pde.ubicaciongis),6)<>0.000000, FORMAT(Y(pde.ubicaciongis),6), ''), '')        \
				ELSE                                                                                                                                      \
				(0.000000) END AS longitud,   																			\
				IFNULL(LEFT(GROUP_CONCAT(pm.mensaje),128),'') as pedidomensaje,                                                                                     \
				' ',                                                                                                                                      \
				' ',                                                                                                                                      \
				CASE WHEN IFNULL(pde.id_pedirent,0)>0                                                                                                     \
				THEN                                                                                                                                      \
				CONCAT(IFNULL(FORMAT(X(pde.ubicaciongis),6),0),',', IFNULL(FORMAT(Y(pde.ubicaciongis),6),0))                                              \
				ELSE                                                                                                                                      \
				'0,0'                                                  													\
				END                                                                                                                                       \
				AS ubicacion,                                                                                                                             \
				IFNULL(pde.calle, '') AS calle,                                                                                                    \
				IFNULL(pde.numext, '') AS numext,                                                                                                 \
				IFNULL(pde.numint, '') AS numint,                                                                                                 \
				IFNULL(pde.cp, '') AS cp,                                                                                                             \
				IFNULL(pde.colonia, '') AS colonia,                                                                                              \
				IFNULL(pde.referenciadom, '') AS referenciadomic                                                                         \
				,' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '														                                      \
				FROM pedidosventa p                                                                                                                       \
				LEFT JOIN pedidosdirent AS pde ON pde.referencia = p.referencia                                                                           \
				INNER JOIN clientes c ON c.cliente = p.cliente                                                                                            \
				LEFT JOIN colonias col ON col.colonia = pde.colonia		                                                                  \
				LEFT JOIN localidades AS l ON l.localidad = col.localidad                                                                                \
				LEFT JOIN municipios AS m ON m.municipio = l.municipio                                                                                   \
				LEFT JOIN pedidosmensajes AS pm on pm.referencia = p.referencia                                                                           \
				WHERE p.embarque='%s'                                                                                                                     \
				GROUP BY p.cliente,pde.calle                                                                                                            \
				ORDER BY idorden,rsocial",mClaveEmbarque);

				if (!mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado))
						 throw (Exception("Error al cargar el embarque"));

		}


}
//------------------------------------------------------------------------------
//ID_MODIFICAR_ESTADO_ARTICULO
void ServidorCatalogos::ModificarEstadoArticulo(RespuestaServidor *Respuesta, MYSQL *MySQL,char *parametros) {

	// Modificar el estado "activo" de los articulos
	char *buffer_sql=new char[1024*100];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i;
	AnsiString instruccion, instrucciones[20];
	AnsiString articulo, empresa, estado, usuario, tipo_mod;
    empresa = FormServidor->ObtieneClaveEmpresa();

	articulo = mFg.ExtraeStringDeBuffer(&parametros);  // El artículo a modificar
	estado   = mFg.ExtraeStringDeBuffer(&parametros);  // Cambiar propiedad 'activo' ( 0 | 1)
	usuario  = mFg.ExtraeStringDeBuffer(&parametros);  // Usuario  ->bitacora
	tipo_mod = mFg.ExtraeStringDeBuffer(&parametros);  // Tipo de modificacion ->bitacora

	try
	{

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		instruccion.sprintf("SET @art='%s'", articulo);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("SET @estado='%s'", estado);
		instrucciones[num_instrucciones++] = instruccion;

		//  Asignar a una varible el valor de la producto del artículo que se pretende actualizar
		instruccion.sprintf("SET @producto = (SELECT producto FROM articulos WHERE articulo = @art)");
		instrucciones[num_instrucciones++]=instruccion;

		//  Asignar a una varible el valor de la presentación del artículo que se pretende actualizar
		instruccion.sprintf("SET @presentacion = (SELECT present FROM articulos WHERE articulo = @art)");
		instrucciones[num_instrucciones++]=instruccion;
		//  Asignar a una varible el valor de la comision del artículo, para poder ser agregado a la bitácora
		instruccion.sprintf("SET @comision = (SELECT porccomi FROM articulos WHERE articulo = @art)");
		instrucciones[num_instrucciones++]=instruccion;
		//  Asignar a una varible el valor de la costobase del artículo que se pretende actualizar
		instruccion.sprintf("SET @costobase = (SELECT pcb.costobase FROM presentaciones ps \
		   INNER JOIN presentacionescb pcb ON pcb.producto=ps.producto AND pcb.present=ps.present  \
		   WHERE ps.producto=@producto AND ps.present=@presentacion AND pcb.idempresa=%s )",
		   FormServidor->ObtieneClaveEmpresa());
		instrucciones[num_instrucciones++]=instruccion;
		// Actualizar la propiedad articulos.activo de los articulos que compartan la misma clave de producto y presentación
		instruccion.sprintf("UPDATE articulos SET activo = @estado WHERE producto=@producto AND present=@presentacion");
		instrucciones[num_instrucciones++]=instruccion;

		// Actualiza el registro de la tabla presentacionesminmax
		instruccion.sprintf("DELETE FROM presentacionesminmax where producto = @producto and present=@presentacion");
			instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("INSERT INTO presentacionesminmax (producto,present,maxfactor,maxmult,minmult,activo) \
			(SELECT amm.producto, amm.present, amm.maxfactor, \
					 amax.multiplo AS maxmult, amin.multiplo AS minmult, 1 AS activo \
			FROM \
				(select a.producto, a.present, MAX(a.factor) AS maxfactor \
					from articulos a WHERE a.activo=1 and a.producto=@producto AND a.present=@presentacion \
						group by a.producto, a.present) amm \
			INNER JOIN articulos amax ON amm.producto=amax.producto AND amm.present=amax.present AND amm.maxfactor=amax.factor AND amax.activo=1 \
			INNER JOIN articulos amin ON amm.producto=amin.producto AND amm.present=amin.present AND amin.factor=1 AND amin.activo=1 \
			group by amm.producto, amm.present) \
		UNION ALL \
			(SELECT amm.producto, amm.present, amm.maxfactor, \
					 amax.multiplo AS maxmult, amin.multiplo AS minmult, 0 AS activo \
			FROM \
				(select a.producto, a.present, MAX(a.factor) AS maxfactor, MAX(a.activo) AS maxactivo \
					from articulos a WHERE a.producto=@producto AND a.present=@presentacion \
						group by a.producto, a.present \
						HAVING maxactivo=0 \
						) amm \
			INNER JOIN articulos amax ON amm.producto=amax.producto AND amm.present=amax.present AND amm.maxfactor=amax.factor \
			INNER JOIN articulos amin ON amm.producto=amin.producto AND amm.present=amin.present AND amin.factor=1 \
			group by amm.producto, amm.present)");
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("update presentacionesminmax pmm \
			LEFT JOIN presentcajlogisfactor pclf ON pmm.producto=pclf.producto AND pmm.present=pclf.present \
			set pmm.cajalogisticafactor=COALESCE(pclf.cajalogisticafactor, pmm.maxfactor) \
			where pmm.producto = @producto and pmm.present=@presentacion");
		instrucciones[num_instrucciones++] = instruccion;

        // Almacenar los valores de referencia de modificación a la bitácora
		instruccion.sprintf("INSERT INTO bitacoraart (idBitacoraArt, articulo,idempresa, usuario, fecha, hora, tipo,costoBase,comision,activo) \
											  values (NULL,@art,%s,'%s','%s','%s','%s',@costobase,@comision, @estado)",
													 empresa,usuario, mFg.DateToMySqlDate(Today()), mFg.TimeToMySqlTime(Time()), tipo_mod );
		instrucciones[num_instrucciones++] = instruccion;

		instrucciones[num_instrucciones++]="COMMIT";

		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);

		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);

	}
	__finally {
	if(buffer_sql != NULL) delete buffer_sql;
	}

}
// ---------------------------------------------------------------------------

// ID_BAJ_Fabricante
void ServidorCatalogos::BajaFabricante(RespuestaServidor *Respuesta, MYSQL *MySQL,
	char *parametros) {
	// BAJA Fabricante
	BajaGenerico(Respuesta, MySQL, parametros, "fabricantes", "fabricante");
}

// ---------------------------------------------------------------------------
// ID_CON_Fabricante
void ServidorCatalogos::ConsultaFabricante(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA Fabricante
	AnsiString instruccion;
	AnsiString clave;

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos de Fabricante
	instruccion.sprintf("select * from fabricantes where fabricante='%s'", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todo Fabricante
	instruccion =
		"select fabricante AS Fabricante, nombre AS Nombre from fabricantes order by nombre";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------
// ID_GRA_Fabricante
void ServidorCatalogos::GrabaFabricante(RespuestaServidor *Respuesta, MYSQL *MySQL,
	char *parametros) {
	// GRABA Fabricante
	GrabaGenerico(Respuesta, MySQL, parametros, "fabricantes", "fabricante");
}
// ---------------------------------------------------------------------------
// ID_BAJ_SEGMENTO
void ServidorCatalogos::BajaSegmento(RespuestaServidor *Respuesta, MYSQL *MySQL,
	char *parametros) {
	// BAJA SEGMENTO
	BajaGenerico(Respuesta, MySQL, parametros, "segmentos", "segmento");
}

// ---------------------------------------------------------------------------
// ID_CON_SEGMENTO
void ServidorCatalogos::ConsultaSegmento(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA SEGMENTO
	AnsiString instruccion;
	AnsiString clave;

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos de Fabricante
	instruccion.sprintf("select * from segmentos where segmento='%s'", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todo Fabricante
	instruccion =
		"select segmento AS Segmento, nombre AS Nombre from segmentos order by nombre";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------
// ID_GRA_SEGMENTO
void ServidorCatalogos::GrabaSegmento(RespuestaServidor *Respuesta, MYSQL *MySQL,
	char *parametros) {
	// GRABA SEGMENTO
	GrabaGenerico(Respuesta, MySQL, parametros, "segmentos", "segmento");
}
//---------------------------------------------------------------------------
//ID_APLICA_PRODCLAVE
void ServidorCatalogos::AplicaProdClave(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*30*10];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i, y;
	AnsiString Instruccion, instrucciones[30];
	AnsiString consulta, consulta_final, order, parametro,tipo;
	AnsiString condicion_eliminar = " ", eliminar = "";
	AnsiString select_present;
	AnsiString insert, instruccion;

	consulta = mFg.ExtraeStringDeBuffer(&parametros);
	parametro = mFg.ExtraeStringDeBuffer(&parametros);
	tipo = mFg.ExtraeStringDeBuffer(&parametros);

	//iniciar la transaccion
	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

		try{

			instrucciones[num_instrucciones++]="CREATE TEMPORARY TABLE tempclaveunidadproducto (  \
			articulo VARCHAR(9), producto VARCHAR(8),nombre VARCHAR(60), present VARCHAR(255),  \
			multiplo VARCHAR(10), INDEX(articulo, producto)) ENGINE = INNODB ";

			Instruccion.sprintf("insert into tempclaveunidadproducto \
									SELECT a.articulo,p.producto,p.nombre,a.present,a.multiplo  \
											FROM articulos a \
											INNER JOIN productos p ON a.producto=p.producto \
											INNER JOIN presentaciones pre ON a.producto=pre.producto  \
											INNER JOIN cclaveunidad ccu ON ccu.idclaveunidad = a.idclaveunidadcfdi\
											INNER JOIN cclaveprodserv ccp ON ccp.idclaveprodserv = p.idclaveprodservcfdi \
									WHERE a.articulo IN (%s) GROUP BY a.articulo ",consulta);
			instrucciones[num_instrucciones++]=Instruccion;

			if(tipo == "1")
				instruccion.sprintf("update articulos a, tempclaveunidadproducto t \
				set a.idclaveunidadcfdi = '%s' where a.articulo=t.articulo ", parametro );

			if(tipo == "2")
				 instruccion.sprintf("update productos pro, tempclaveunidadproducto t \
				set pro.idclaveprodservcfdi = '%s' where pro.producto=t.producto ", parametro );

			instrucciones[num_instrucciones++]=instruccion;

		instrucciones[num_instrucciones++]="COMMIT";

		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);
	   }
		__finally {
		if(buffer_sql != NULL) delete buffer_sql;
		}


}
// ---------------------------------------------------------------------------
//ID_APLICA_RECLASIFICA_PRODUCTOS
void ServidorCatalogos::AplicaReclasificaProductos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*30*10];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[30];
	AnsiString clavesproductos, clasificacion1nueva, clasificacion2nueva, clasificacion3nueva;

	clavesproductos = mFg.ExtraeStringDeBuffer(&parametros);
	clasificacion1nueva = mFg.ExtraeStringDeBuffer(&parametros);
	clasificacion2nueva = mFg.ExtraeStringDeBuffer(&parametros);
	clasificacion3nueva = mFg.ExtraeStringDeBuffer(&parametros);

	//iniciar la transaccion
	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	try{
		Instruccion.sprintf("update productos pro \
			set pro.clasif1='%s', pro.clasif2='%s', pro.clasif3='%s' where pro.producto in (%s) ",
			clasificacion1nueva, clasificacion2nueva, clasificacion3nueva, clavesproductos );
		instrucciones[num_instrucciones++]=Instruccion;

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
// ---------------------------------------------------------------------------
//ID_APLICA_RECLASIFICA_CONT_PRODUCTOS
void ServidorCatalogos::AplicaReclasificatProdCon(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*30*10];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[30];
	AnsiString clavesproductos, clasificacioncont1nueva, clasificacioncont2nueva;

	clavesproductos = mFg.ExtraeStringDeBuffer(&parametros);
	clasificacioncont1nueva = mFg.ExtraeStringDeBuffer(&parametros);
	clasificacioncont2nueva = mFg.ExtraeStringDeBuffer(&parametros);

	//iniciar la transaccion
	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	try{
		Instruccion.sprintf("update productos pro \
			set pro.clasifcont='%s', pro.clasifcont2='%s' where pro.producto in (%s) ",
			clasificacioncont1nueva, clasificacioncont2nueva, clavesproductos );
		instrucciones[num_instrucciones++]=Instruccion;

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
// ---------------------------------------------------------------------------
//ID_APLICA_RECLASIFICA_FAB_PRODUCTOS
void ServidorCatalogos::AplicaReclasificatFabProd(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*30*10];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[30];
	AnsiString clavesproductos, fabricantenuevo;

	clavesproductos = mFg.ExtraeStringDeBuffer(&parametros);
	fabricantenuevo = mFg.ExtraeStringDeBuffer(&parametros);

	//iniciar la transaccion
	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	try{
		Instruccion.sprintf("update productos pro \
			set pro.fabricante='%s' where pro.producto in (%s) ",
			fabricantenuevo, clavesproductos );
		instrucciones[num_instrucciones++]=Instruccion;

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
// ---------------------------------------------------------------------------
//ID_APLICA_RECLASIFICA_SEG_COMPRAS
void ServidorCatalogos::AplicaReclasificatSegCom(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*30*10];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[30];
	AnsiString clavesproductos, segmentonuevo, empresa;

	clavesproductos = mFg.ExtraeStringDeBuffer(&parametros);
	segmentonuevo = mFg.ExtraeStringDeBuffer(&parametros);
	empresa = mFg.ExtraeStringDeBuffer(&parametros);

	//iniciar la transaccion
	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	try{
		Instruccion.sprintf("UPDATE presentacionescb pcb\
		INNER JOIN articulos art ON art.producto = pcb.producto AND art.present = pcb.present\
		SET pcb.segmento = '%s'\
		WHERE art.articulo IN (%s) AND pcb.idempresa=%s",
			segmentonuevo, clavesproductos, empresa );
		instrucciones[num_instrucciones++]=Instruccion;

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
// ---------------------------------------------------------------------------
// ID_GRABA_ARTVTAINTERNET
void ServidorCatalogos::GrabaAriculosVentaInternet(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*2000];
	char *aux_buffer_sql=buffer_sql;
	BufferRespuestas* resp_precio=NULL;
	BufferRespuestas* resp_cant=NULL;
	BufferRespuestas* resp_almacenes=NULL;
	AnsiString instrucciones[15];
	AnsiString Instruccion;
	int num_instrucciones=0;
	AnsiString producto, present,usuario,tarea, articulo,tipo, porcSub, sucursal;
	AnsiString almacen, cad_conjunto_almacenes=" ";
	double precio;
	int i, error = 0, cantidad=0;

	try{
		articulo = mFg.ExtraeStringDeBuffer(&parametros);
		//present = mFg.ExtraeStringDeBuffer(&parametros);
		usuario = mFg.ExtraeStringDeBuffer(&parametros);
		tipo = (mFg.ExtraeStringDeBuffer(&parametros));
		porcSub = mFg.ExtraeStringDeBuffer(&parametros);
		sucursal = mFg.ExtraeStringDeBuffer(&parametros);

		if (error == 0) {
			Instruccion.sprintf( "SELECT @ERROR:=if(articulo='%s',1,0) AS error \
					from ecommerceproductos WHERE articulo='%s' ",articulo, articulo );
			mServidorVioleta->EjecutaVerificacion(Respuesta, MySQL, Instruccion.c_str(), 1, error);
		}

		if(error == 0)
		{
            Instruccion.sprintf("SELECT a.almacen FROM almacenes a \
				INNER JOIN secciones s ON a.seccion=s.seccion \
				WHERE s.sucursal='%s'", sucursal);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), resp_almacenes);
			for(int i=0; i<resp_almacenes->ObtieneNumRegistros(); i++){
				resp_almacenes->IrAlRegistroNumero(i);
				almacen=resp_almacenes->ObtieneDato("almacen");

				cad_conjunto_almacenes+="'";
				cad_conjunto_almacenes+=almacen;
				cad_conjunto_almacenes+="'";
				if (i<resp_almacenes->ObtieneNumRegistros()-1)
					cad_conjunto_almacenes+=",";
			}

			Instruccion.sprintf( "SELECT SUM(ex.cantidad*%s) AS cantidad FROM existenciasactuales ex \
			INNER JOIN articulos a ON a.articulo = '%s'                                              \
			WHERE a.producto = ex.producto AND a.present = ex.present AND ex.almacen IN (%s) ",porcSub, articulo, cad_conjunto_almacenes );
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), resp_cant);
			cantidad=StrToFloat(resp_cant->ObtieneDato("cantidad"));

			//iniciar la transaccion
			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";


			Instruccion.sprintf("insert into ecommerceproductos (articulo,fechaalta,fechamodi,horaalta ,horamodi ,usualta,usumodi,activo,cantsis ) \
			values ('%s',CURDATE(),CURDATE(),CURTIME(),CURTIME(),'%s','%s',1, %d) ",articulo,usuario,usuario,cantidad);
			instrucciones[num_instrucciones++]=Instruccion;

			instrucciones[num_instrucciones++]="COMMIT";

		}

        // Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql = mFg.AgregaStringABuffer
			(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i = 0; i < num_instrucciones; i++)
			aux_buffer_sql = mFg.AgregaStringABuffer(instrucciones[i],
			aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta,
			TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,
				buffer_sql)) {
			Instruccion.sprintf("select %d as error", error);
			mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,
				Instruccion.c_str(), Respuesta->TamBufferResultado);
		}


	} __finally {
		if(buffer_sql != NULL) delete buffer_sql;
		if(resp_cant!=NULL) delete resp_cant;
		if(resp_almacenes!=NULL) delete resp_almacenes;
	}



}
// ---------------------------------------------------------------------------
// ID_CON_ARTVTAINTERNET
void ServidorCatalogos::ConsultaAriculosVentaInternet(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
	AnsiString sucursal, sucursalInv;
	AnsiString cliente;
	AnsiString almacen, cad_conjunto_almacenes=" ";
	AnsiString producto, condicion_producto=" ";
	BufferRespuestas* resp_almacenes=NULL;
    int i=0;

	//Extrae la sucursal
	sucursal=mFg.ExtraeStringDeBuffer(&parametros);
	cliente=mFg.ExtraeStringDeBuffer(&parametros);
	producto=mFg.ExtraeStringDeBuffer(&parametros);
	sucursalInv=mFg.ExtraeStringDeBuffer(&parametros);


	try {
		instruccion.sprintf("SELECT a.almacen FROM almacenes a \
		INNER JOIN secciones s ON a.seccion=s.seccion \
		WHERE s.sucursal='%s'", sucursalInv);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_almacenes);
		for(int i=0; i<resp_almacenes->ObtieneNumRegistros(); i++){
			resp_almacenes->IrAlRegistroNumero(i);
			almacen=resp_almacenes->ObtieneDato("almacen");

			cad_conjunto_almacenes+="'";
			cad_conjunto_almacenes+=almacen;
			cad_conjunto_almacenes+="'";
			if (i<resp_almacenes->ObtieneNumRegistros()-1)
				cad_conjunto_almacenes+=",";
		}
	} __finally {
		if(resp_almacenes!=NULL) delete resp_almacenes;
	}

	if (producto!=" ") {
		condicion_producto.sprintf(" AND CONCAT(pr.nombre, a.producto, a.present, a.articulo) LIKE '%%%s%%' ", producto);
	}

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	instruccion.sprintf("SELECT                                                                                              \
	a.articulo,                                                                                                              \
	REPLACE(pr.nombre, 'KILO', '') AS nombre,                                             									 \
	a.producto,                                                                                                              \
	IF(ec.fraccionar = fp.fraccion AND (a.present = 'KILO' OR ppa.pesoprom IS NOT NULL),                                     \
	REPLACE(a.present, 'KILO', ''), a.present) AS present,                                                                   \
	CONCAT(a.multiplo,'X',a.factor) AS multiplo,                                                                             \
	a.multiplo,                                                                                                              \
	a.factor,                                                                                                                \
	a.ean13,                                                                                                                 \
	IF(ppa.pesoprom IS NULL,a.peso,ppa.pesoprom) AS peso,                                                                    \
	IFNULL(IF(ppa.pesoprom IS NULL, IF(ec.fraccionar = fp.fraccion, ROUND(c.costounit*fp.porcentaje,2), c.costounit),0),     \
	ROUND(c.costounit*ppa.pesoprom,2)) AS costo, 																			 \
	IF(ppa.pesoprom IS NULL, IF(ec.tipodeprec = ' ', IF(ec.fraccionar = fp.fraccion, ROUND(IFNULL(ao.precio_oferta, preci.precio) * fp.porcentaje, 2), IFNULL(ao.precio_oferta, preci.precio)), IF(ec.fraccionar = fp.fraccion, ROUND(IFNULL(ao.precio_oferta, prec.precio) * fp.porcentaje, 2), IFNULL(ao.precio_oferta, prec.precio))), IF(ec.tipodeprec = ' ', ROUND(IFNULL(ao.precio_oferta, preci.precio) * ppa.pesoprom, 2), ROUND(IFNULL(ao.precio_oferta, prec.precio) * ppa.pesoprom, 2))) AS preciosistema,        \
	if(ec.tipodeprec = ' ', preci.tipoprec, prec.tipoprec )AS tipodeprec,                                                    \
	CONCAT(e1.nombre, ' ', e1.appat,' ', e1.apmat) AS usuarioalta,                                                           \
	IF(TRUNCATE(IF(ppa.pesoprom IS NULL, IF(ec.fraccionar = fp.fraccion, TRUNCATE(SUM(((ex.cantidad/a.factor)*1)*ec.fraccionar), 0), SUM((ex.cantidad/a.factor)*1)), TRUNCATE(SUM(((ex.cantidad/a.factor)*1)/ppa.pesoprom),0)),0)<=0,0, \
	TRUNCATE(IF(ppa.pesoprom IS NULL, IF(ec.fraccionar = fp.fraccion, TRUNCATE(SUM(((ex.cantidad/a.factor)*1)*ec.fraccionar), 0), SUM((ex.cantidad/a.factor)*1)), TRUNCATE(SUM(((ex.cantidad/a.factor)*1)/ppa.pesoprom),0)),0)) AS cantsis, \
	ec.inventario,                                                                      									 \
	ec.idProducto,                                                                                                           \
	ec.idInventario,                                                                                                         \
	ec.idVariant,                                                                                                            \
	ec.codigobarrasis,                                                                                                       \
	ec.precio,                                                                              								 \
	ec.tag,                                                                                                                  \
	if(a.multiplo<>'KILO', a.multiplo, IF(ppa.pesoprom IS NULL, fp.etiqueta, 'PIEZA')) AS tagsis,                   		 \
	ec.fraccionar,                                                                                                           \
	if(ec.rappi = 1, 'RAPPI', 'SHOPIFY') AS tienda,	                                                                     	 \
	ec.aplicadescrappi, \
	ec.fechainidesc, \
	ec.fechafindesc, \
	ec.porcdescrappi, \
	IF(ec.activo=1 && a.activo=1,'ACTIVE','ARCHIVED') AS activo, \
    IF(ao.precio_oferta IS NULL, 0, 1) AS tienepromo \
	FROM ecommerceproductos ec                                                                                               \
	INNER JOIN articulos a ON a.articulo=ec.articulo                                                                         \
	INNER JOIN productos pr ON pr.producto=a.producto                                                                        \
	INNER JOIN presentaciones pre ON pre.producto=a.producto AND pre.present=a.present                                       \
	INNER JOIN empleados e1 ON e1.empleado=ec.usualta                                                                        \
	INNER JOIN empleados e2 ON e2.empleado=ec.usumodi                                                                        \
	LEFT JOIN precalculocostospromedio%s c ON a.producto = c.producto AND a.present = c.present                               \
	LEFT JOIN (                                                                                                              \
				SELECT p.precio, p.articulo, p.tipoprec                                                                      \
				FROM tiposdeprecios tp                                                                                       \
				INNER JOIN precios p ON p.tipoprec = tp.tipoprec                                                             \
				INNER JOIN ecommerceproductos ec ON ec.articulo = p.articulo                                                 \
				WHERE tp.tipoprec = ec.tipodeprec AND tp.idempresa=%s                                                        \
	) prec ON prec.tipoprec = ec.tipodeprec AND prec.articulo = ec.articulo                                                  \
	LEFT JOIN (                                                                                                              \
				SELECT p.precio, p.articulo, p.tipoprec                                                                      \
				FROM clientes c                                                                                              \
				LEFT JOIN clientesemp ce ON ce.cliente = c.cliente                                                          \
				INNER JOIN precios p ON p.tipoprec = ce.tipoprec                                                              \
				INNER JOIN tiposdeprecios tp ON tp.tipoprec=p.tipoprec AND tp.idempresa=%s                                   \
				INNER JOIN ecommerceproductos ec ON ec.articulo = p.articulo                                                 \
				WHERE c.cliente = '%s'                                                                                       \
	) preci ON preci.articulo = ec.articulo                                                                                  \
	LEFT JOIN pesopromporarticulo ppa ON ppa.articulo = ec.articulo                                                          \
	LEFT JOIN fraccionamientoproductos fp ON fp.fraccion = ec.fraccionar \
	LEFT JOIN existenciasactuales ex ON ex.producto = a.producto AND ex.present = a.present AND ex.almacen IN (%s)           \
    LEFT JOIN articulos_oferta ao ON ao.articulo = ec.articulo AND ao.idempresa = %s AND CURDATE() BETWEEN ao.fecha_vigencia_inicio AND ao.fecha_vigencia_fin \
	WHERE 1 %s 					                                                                                             \
	GROUP BY a.articulo                                                                                                      \
	ORDER BY a.producto, a.present  ", FormServidor->ObtieneClaveEmpresa(), FormServidor->ObtieneClaveEmpresa(),
	FormServidor->ObtieneClaveEmpresa(), cliente, cad_conjunto_almacenes, FormServidor->ObtieneClaveEmpresa(),
	condicion_producto);

	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

}
// ---------------------------------------------------------------------------
// ID_BAJA_ARTVTAINTERNET
void ServidorCatalogos::BajaAriculosVentaInternet(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// BORRA UN IMPUESTO DADO COMO PARAMETRO
	char *buffer_sql = new char[1024 * 32];
	char *aux_buffer_sql = buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instrucciones[50], instruccion;
	int num_instrucciones = 0;
	AnsiString clave_prod, clave_present, clave_articulo;
	int error = 0;
	int i;

	try {
		clave_articulo = mFg.ExtraeStringDeBuffer(&parametros); // producto
		//clave_present = mFg.ExtraeStringDeBuffer(&parametros); // present

		// Verifica que el impuesto no este en uso ya
		/*if (error == 0) {
			instruccion.sprintf(
				"SELECT @error:=IF(SUM(1)>0,1,0) AS error FROM dcompras d WHERE d.claveimp1=%s OR d.claveimp2=%s OR d.claveimp3=%s OR d.claveimp4=%s", clave, clave, clave, clave);
			mServidorVioleta->EjecutaVerificacion(Respuesta, MySQL,
				instruccion.c_str(), 2, error);
		}*/


		if (error == 0) {
			instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++] = "START TRANSACTION";

			// Elimina el impuesto
			instruccion.sprintf("delete from ecommerceproductos where  articulo='%s' ",
				clave_articulo);
			instrucciones[num_instrucciones++] = instruccion;

			instrucciones[num_instrucciones++] = "COMMIT";
		}

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql = mFg.AgregaStringABuffer
			(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i = 0; i < num_instrucciones; i++)
			aux_buffer_sql = mFg.AgregaStringABuffer(instrucciones[i],
			aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta,
			TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,
				buffer_sql)) {
			instruccion.sprintf("select %d as error", error);
			mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,
				instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	}
	__finally {
		delete buffer_sql;
	}
}
// ---------------------------------------------------------------------------
//ID_GUARDAR_CODIGO_POSTAL
void ServidorCatalogos::GuardaCodigoPostal(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    char *buffer_sql=new char[1024*32];
    char *aux_buffer_sql=buffer_sql;
    AnsiString instrucciones[15];
    AnsiString Instruccion;
    int num_instrucciones=0;
    AnsiString sucursal, codigopostal;
    int i, error = 0;

    try{
        sucursal = mFg.ExtraeStringDeBuffer(&parametros);
        codigopostal = mFg.ExtraeStringDeBuffer(&parametros);

        if (error == 0) {
            Instruccion.sprintf( "SELECT @ERROR:=if(cubrecodigop='%s',1,0) AS error \
                    from ecommercesuccp WHERE cubrecodigop='%s' ",codigopostal, codigopostal );
            mServidorVioleta->EjecutaVerificacion(Respuesta, MySQL, Instruccion.c_str(), 1, error);
        }

        if (error == 0) {
            //iniciar la transaccion
            instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
            instrucciones[num_instrucciones++]="START TRANSACTION";

            Instruccion.sprintf("insert into ecommercesuccp (sucursal,cubrecodigop) \
            values ('%s','%s') ",sucursal,codigopostal);
            instrucciones[num_instrucciones++]=Instruccion;

            instrucciones[num_instrucciones++]="COMMIT";
        }

        // Crea el buffer con todas las instrucciones SQL
        aux_buffer_sql = mFg.AgregaStringABuffer
            (mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
        for (i = 0; i < num_instrucciones; i++)
            aux_buffer_sql = mFg.AgregaStringABuffer(instrucciones[i],
            aux_buffer_sql);

        mServidorVioleta->InicializaBuffer(Respuesta,
            TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
        if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,
                buffer_sql)) {
            Instruccion.sprintf("select e.sucursal, %d as error from ecommercesuccp e where e.cubrecodigop= '%s'", error, codigopostal);
            mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,
                Instruccion.c_str(), Respuesta->TamBufferResultado);
        }
    } __finally {
        if(buffer_sql != NULL) delete buffer_sql;
    }
}
// ---------------------------------------------------------------------------

//ID_ACTUALIZA_CODIGO_POSTAL
void ServidorCatalogos::ActualizaCodigoPostal(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    char *buffer_sql=new char[1024*30*10];
    char *aux_buffer_sql=buffer_sql;
    int num_instrucciones=0;
    AnsiString Instruccion, instrucciones[10];
    AnsiString sucursal, codigopostal, actualcodigopostal, actualsuc;
    AnsiString cad_conjunto_sucursales=" ", sucursales;
    int i, error = 0;

    BufferRespuestas* resp_sucursales=NULL;

    actualcodigopostal = mFg.ExtraeStringDeBuffer(&parametros);
    actualsuc = mFg.ExtraeStringDeBuffer(&parametros);
    sucursal = mFg.ExtraeStringDeBuffer(&parametros);
    codigopostal = mFg.ExtraeStringDeBuffer(&parametros);

    try{

        if (actualsuc == sucursal) {
            Instruccion.sprintf("SELECT sucursal FROM sucursales");
            mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), resp_sucursales);
            for(int i=0; i<resp_sucursales->ObtieneNumRegistros(); i++){
                resp_sucursales->IrAlRegistroNumero(i);
                sucursales=resp_sucursales->ObtieneDato("sucursal");

                cad_conjunto_sucursales+="'";
                cad_conjunto_sucursales+=sucursales;
                cad_conjunto_sucursales+="'";
                if (i<resp_sucursales->ObtieneNumRegistros()-1)
                    cad_conjunto_sucursales+=",";
            }
            Instruccion.sprintf( "SELECT @ERROR:=if(cubrecodigop='%s',1,0) AS error \
                    from ecommercesuccp WHERE cubrecodigop='%s' AND sucursal IN(%s)",codigopostal, codigopostal,cad_conjunto_sucursales );
            mServidorVioleta->EjecutaVerificacion(Respuesta, MySQL, Instruccion.c_str(), 1, error);
        }

        if (error == 0) {
            //iniciar la transaccion
            instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
            instrucciones[num_instrucciones++]="START TRANSACTION";

			Instruccion.sprintf("update ecommercesuccp \
				set sucursal='%s', cubrecodigop='%s' where cubrecodigop='%s' ",
                sucursal, codigopostal, actualcodigopostal );
            instrucciones[num_instrucciones++]=Instruccion;

            instrucciones[num_instrucciones++]="COMMIT";
        }

        // Crea el buffer con todas las instrucciones SQL
        aux_buffer_sql = mFg.AgregaStringABuffer
            (mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
        for (i = 0; i < num_instrucciones; i++)
            aux_buffer_sql = mFg.AgregaStringABuffer(instrucciones[i],
            aux_buffer_sql);

        mServidorVioleta->InicializaBuffer(Respuesta,
            TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
        if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,
                buffer_sql)) {
            Instruccion.sprintf("select e.sucursal, %d as error from ecommercesuccp e where e.cubrecodigop= '%s'", error, codigopostal);
            mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,
                Instruccion.c_str(), Respuesta->TamBufferResultado);
        }
    } __finally {
        if(buffer_sql != NULL) delete buffer_sql;
    }
}
// ---------------------------------------------------------------------------
// ID_GRA_ART_TAGS
void ServidorCatalogos::GrabaArticulosTags(RespuestaServidor *Respuesta, MYSQL *MySQL,
    char *parametros) {
    // GRABA tags articulos
	//GrabaGenerico(Respuesta, MySQL, parametros, "bancosnaturalezas", "naturaleza");
	char *buffer_sql=new char[1024*100];
	char *aux_buffer_sql=buffer_sql;
    int num_instrucciones=0, i;
	AnsiString instruccion, instrucciones[50];
    DatosTabla datos(mServidorVioleta->Tablas);
    TDate fecha=Today();
    TTime hora=Time();
    AnsiString tarea, clave;
    AnsiString descripcion, usualta, usomodi, referencia;
    int folio=0;
    AnsiString esSistema, mensaje;

    try{
        tarea=mFg.ExtraeStringDeBuffer(&parametros);
        referencia=mFg.ExtraeStringDeBuffer(&parametros);
        descripcion=mFg.ExtraeStringDeBuffer(&parametros);
        esSistema=mFg.ExtraeStringDeBuffer(&parametros);
        usualta=mFg.ExtraeStringDeBuffer(&parametros);
        usomodi=mFg.ExtraeStringDeBuffer(&parametros);

        // Obtiene los datos de la tabla de pedidos
        datos.AsignaTabla("articulostags");
        parametros+=datos.InsCamposDesdeBuffer(parametros);

            //nueva consulta del folio
        BufferRespuestas* resp_folio=NULL;
        //se creara un bufer para obtener el valor del parametro de precio minimo y posteriormete
        //liberar la memoria ocupada de esta consulta
        try{
            instruccion.sprintf("SELECT @folio:=if(max(idarticulostags) IS NULL,'1',max(idarticulostags) ) AS folio FROM articulostags");
            mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_folio);
            if (resp_folio->ObtieneNumRegistros()>0){
                folio=mFg.CadenaAFlotante(resp_folio->ObtieneDato("folio")); //valor de precio minimo
            }
        }__finally{
            if (resp_folio!=NULL) delete resp_folio;
        }


        instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

        // Graba la cabecera en la tabla "pedidosventa"
        if (tarea=="A") {
            //datos.InsCampo("referencia", "@folio",1);
			datos.InsCampo("descripcion", descripcion);
            datos.InsCampo("desistema", esSistema);
            datos.InsCampo("fechaalta", mFg.DateToAnsiString(fecha));
            datos.InsCampo("horaalta", mFg.TimeToAnsiString(hora));
            datos.InsCampo("usualta", usualta);
            datos.InsCampo("fechamodi", mFg.DateToAnsiString(fecha));
            datos.InsCampo("horamodi", mFg.TimeToAnsiString(hora));
            datos.InsCampo("usumodi", usomodi);
			instrucciones[num_instrucciones++]=datos.GenerarSqlInsert();

        } else {
            instruccion.sprintf("set @idart:='%s'", referencia);
            instrucciones[num_instrucciones++]=instruccion;

            datos.InsCampo("descripcion", descripcion);
			datos.InsCampo("desistema", esSistema);
            datos.InsCampo("usumodi", usomodi);
            datos.InsCampo("fechamodi", mFg.DateToAnsiString(fecha) );
            datos.InsCampo("horamodi", mFg.TimeToAnsiString(hora) );

            instrucciones[num_instrucciones++]=datos.GenerarSqlUpdate("idarticulostags=@idart");

            // Guarda el mensaje
            if (mensaje!=" ") { // Un espacio lo tomamos como nulo.
                //instruccion.sprintf("replace into pedidosmensajes (referencia, mensaje) values (@folio,'%s')",mensaje);
            } else {
                //instruccion.sprintf("delete from pedidosmensajes where referencia=@folio");
            }
            instrucciones[num_instrucciones++]=instruccion;
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
// ---------------------------------------------------------------------------
// ID_CON_ART_TAGS
void ServidorCatalogos::ConsultaArticulosTags(RespuestaServidor *Respuesta,
    MYSQL *MySQL, char *parametros) {
    // CONSULTA
    AnsiString instruccion;
    AnsiString clave_art_tags;

    clave_art_tags = mFg.ExtraeStringDeBuffer(&parametros);
    mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

    // Obtiene todos los datos
    instruccion.sprintf("select * from articulostags where idarticulostags='%s' ",clave_art_tags);
    mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
        Respuesta->TamBufferResultado);

    // Obtiene todos los datos   where idarticulostags='%s'
    instruccion.sprintf("select * from articulostags ");
    mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
        Respuesta->TamBufferResultado);

    // Obtiene numero del ultimo registro
    instruccion =
        "select  max(idarticulostags) as ultimo from articulostags";
    mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
        Respuesta->TamBufferResultado);

}
// ---------------------------------------------------------------------------
// ID_BAJ_ART_TAGS
void ServidorCatalogos::BajaArticulosTags(RespuestaServidor *Respuesta, MYSQL *MySQL,
    char *parametros) {
    // BORRA UNA ETIQUETA
    char *buffer_sql = new char[1024 * 32];
    char *aux_buffer_sql = buffer_sql;
    DatosTabla datos(mServidorVioleta->Tablas);
    AnsiString instrucciones[50], instruccion;
    int num_instrucciones = 0;
    AnsiString clave_tag;
    int error = 0;
    int i;

    try {
        clave_tag = mFg.ExtraeStringDeBuffer(&parametros); // producto

        if (error == 0) {
            instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
            instrucciones[num_instrucciones++] = "START TRANSACTION";

            // Elimina el impuesto
            instruccion.sprintf("delete from articulostags where  idarticulostags='%s' ",
                clave_tag);
            instrucciones[num_instrucciones++] = instruccion;

            instrucciones[num_instrucciones++] = "COMMIT";
        }

        // Crea el buffer con todas las instrucciones SQL
        aux_buffer_sql = mFg.AgregaStringABuffer
            (mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
        for (i = 0; i < num_instrucciones; i++)
            aux_buffer_sql = mFg.AgregaStringABuffer(instrucciones[i],
            aux_buffer_sql);

        mServidorVioleta->InicializaBuffer(Respuesta,
            TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
        if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,
                buffer_sql)) {
            instruccion.sprintf("select %d as error", error);
            mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,
                instruccion.c_str(), Respuesta->TamBufferResultado);
        }
    }
    __finally {
        delete buffer_sql;
    }
}
// ---------------------------------------------------------------------------
//ID_GUARDA_ORDERS
void ServidorCatalogos::GuardaOrdenes(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*64];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[10], instruccion;
	int noPedido=0, cantPedidos=0, cantProd=0;
	double total, subtotal, imptotal, iva, peso;
	AnsiString IDOrden, locationID, fchaalta, fchamodi, fchacancel, estatus_financiero, razon_cancelado;

	AnsiString nombre, direccion, telefono, codigopostal, ciudad, estado, usuario;

	BufferRespuestas* resp_pedido=NULL;
	int pedidoCargado=0;
	double tot, subtot, costoenvio;
	AnsiString fecha_mod, status_finan, razon_cancel, latitude, longitude, notas, email;

	//iniciar la transaccion
	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	try{
		noPedido = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
		IDOrden = mFg.ExtraeStringDeBuffer(&parametros);
		locationID = mFg.ExtraeStringDeBuffer(&parametros);
		fchaalta = mFg.ExtraeStringDeBuffer(&parametros);
		fchamodi = mFg.ExtraeStringDeBuffer(&parametros);
		fchacancel = mFg.ExtraeStringDeBuffer(&parametros);
		total = StrToFloat(mFg.ExtraeStringDeBuffer(&parametros));
		subtotal = StrToFloat(mFg.ExtraeStringDeBuffer(&parametros));
		imptotal = StrToFloat(mFg.ExtraeStringDeBuffer(&parametros));
		peso = StrToFloat(mFg.ExtraeStringDeBuffer(&parametros));
		estatus_financiero = mFg.ExtraeStringDeBuffer(&parametros);
		razon_cancelado = mFg.ExtraeStringDeBuffer(&parametros);
		iva = StrToFloat(mFg.ExtraeStringDeBuffer(&parametros));

		nombre = mFg.ExtraeStringDeBuffer(&parametros);
		direccion = mFg.ExtraeStringDeBuffer(&parametros);
		telefono = mFg.ExtraeStringDeBuffer(&parametros);
		codigopostal = mFg.ExtraeStringDeBuffer(&parametros);
		ciudad = mFg.ExtraeStringDeBuffer(&parametros);
		estado = mFg.ExtraeStringDeBuffer(&parametros);
		costoenvio = StrToFloat(mFg.ExtraeStringDeBuffer(&parametros));
		usuario = mFg.ExtraeStringDeBuffer(&parametros);
		latitude = mFg.ExtraeStringDeBuffer(&parametros);
		longitude = mFg.ExtraeStringDeBuffer(&parametros);
		notas = mFg.ExtraeStringDeBuffer(&parametros);
		email = mFg.ExtraeStringDeBuffer(&parametros);
		peso = (peso/1000);

		try {
			instruccion.sprintf("SELECT COUNT(nopedido) AS pedido,              \
			IFNULL(fchamodi,'') AS fchamodi,    								\
			IFNULL(estatus_financiero,'') AS estatus_financiero,                \
			IFNULL(razon_cancelado,'') AS razon_cancelado,                      \
			IFNULL(total,0) AS total, IFNULL(subtotal,0) AS subtotal            \
			FROM ecommerceorden WHERE nopedido=%d",noPedido);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_pedido);
			pedidoCargado=StrToInt(resp_pedido->ObtieneDato("pedido"));
			fecha_mod=resp_pedido->ObtieneDato("fchamodi");
			status_finan=resp_pedido->ObtieneDato("estatus_financiero");
			razon_cancel=resp_pedido->ObtieneDato("razon_cancelado");
			razon_cancel=resp_pedido->ObtieneDato("razon_cancelado");
			tot=StrToFloat(resp_pedido->ObtieneDato("total"));
			subtot=StrToFloat(resp_pedido->ObtieneDato("subtotal"));
		} __finally {
			if (resp_pedido != NULL) delete resp_pedido;
		}
		if (pedidoCargado==0) {

			if(codigopostal == "") {
				codigopostal = "58020";
			}

			if(telefono == "") {
				telefono = " ";
			}

			if(direccion == "") {
				direccion = " ";
			}

			if(nombre == "") {
				nombre = " ";
			}

			if(ciudad == "") {
				ciudad = " ";
			}

			if(estado == "") {
				estado = " ";
			}

			if(notas == "null") {
				notas = " ";
			}

			Instruccion.sprintf("INSERT INTO ecommerceorden (nopedido, id, locationid, 		   \
			fchaalta, fchamodi, fchacancel,                                                    \
			total, subtotal, imptotal, costoenvio, peso,                                       \
			estatus_financiero, razon_cancelado, iva, nombre, direccion, 					   \
			telefono, codigopostal, ciudad, estado, latitude, longitude, notas, email) VALUES  \
			(%d,%s,%s,'%s','%s','%s',%f,%f,%f,%f,%f,'%s','%s',%f,'%s','%s','%s','%s','%s','%s','%s','%s','%s','%s')",
			noPedido, IDOrden, locationID, fchaalta, fchamodi,
			fchacancel, total, subtotal, imptotal,costoenvio, peso, estatus_financiero, razon_cancelado, iva, nombre, direccion,
			telefono, codigopostal, ciudad, estado, latitude, longitude, notas, email
			);
			instrucciones[num_instrucciones++]=Instruccion;

			Instruccion.sprintf("INSERT INTO bitacoraecommerceorden (nopedido, usuario, fecha, 		   \
			hora, estatus) VALUES                                     								   \
			(%d,'%s',CURDATE(),CURTIME(),'PENDIENTE')",
			noPedido, usuario
			);
			instrucciones[num_instrucciones++]=Instruccion;
		} else if (pedidoCargado==1 && status_finan!= "" && razon_cancel != "" &&
		(status_finan != estatus_financiero || razon_cancel != razon_cancelado || total != tot || subtot != subtotal)) {
			Instruccion.sprintf("update ecommerceorden 	  \
				set fchamodi='%s', total=%f, subtotal=%f, \
				costoenvio=%f, estatus_financiero='%s',   \
				razon_cancelado='%s'					  \
				where nopedido=%d ",
			fchamodi, total, subtotal, costoenvio, estatus_financiero, razon_cancelado, noPedido
			);
			instrucciones[num_instrucciones++]=Instruccion;
		}

		instrucciones[num_instrucciones++]="COMMIT";

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
//ID_GUARDA_ORDER_DETALLE
void ServidorCatalogos::GuardaOrdenDetalle(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*64];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[10], instruccion;
	BufferRespuestas* resp_prodcarg=NULL;
	int pedidoCargado=0;

	int noPedido=0, prodCargado=0, cant=0, cantidad=0;
	double precio, descuento;
	AnsiString nombreProd, sku, id;

	//iniciar la transaccion
	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	try{
		noPedido = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
		id = mFg.ExtraeStringDeBuffer(&parametros);
		nombreProd = mFg.ExtraeStringDeBuffer(&parametros);
		cantidad = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
		sku = mFg.ExtraeStringDeBuffer(&parametros);
		precio = StrToFloat(mFg.ExtraeStringDeBuffer(&parametros));
		descuento = StrToFloat(mFg.ExtraeStringDeBuffer(&parametros));

		try {
			instruccion.sprintf("SELECT COUNT(nopedido) AS prods, cantidad         \
			FROM ecommercedetalle WHERE nopedido=%d AND sku = '%s'",noPedido,sku);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_prodcarg);
			prodCargado=StrToInt(resp_prodcarg->ObtieneDato("prods"));
			cant=StrToInt(resp_prodcarg->ObtieneDato("cantidad"));

		} __finally {
			if (resp_prodcarg != NULL) delete resp_prodcarg;
		}
		if (prodCargado==0) {
			Instruccion.sprintf("INSERT INTO ecommercedetalle (nopedido, id, nombre, cantidad, \
			sku, precio, descuento) VALUES                                     			   \
			(%d,'%s','%s',%d,'%s',%f,%f)",
			noPedido, id, nombreProd, cantidad, sku, precio, descuento
			);
			instrucciones[num_instrucciones++]=Instruccion;
		} /*else if (prodCargado==1 && cant != cantidad) {
			Instruccion.sprintf("update ecommercedetalle \
				set cantidad=%d, precio=%f, descuento=%f \
				where nopedido=%d AND sku = '%s'",
			cantidad, precio, descuento, noPedido, sku
			);
			instrucciones[num_instrucciones++]=Instruccion;
		} */

		instrucciones[num_instrucciones++]="COMMIT";

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
//ID_ORDENES_COMPRAS_ONLINE
void ServidorCatalogos::OrdenesCompra(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql = new char[1024 * 64];
	char *aux_buffer_sql = buffer_sql;
	int i;
	double dias_venta;
	int num_instrucciones = 0;
	AnsiString instruccion, instrucciones[10];
	AnsiString sucursal="";
	AnsiString condicion_sucursal=" ";
	int diasIntervalo=0;
	AnsiString archivo_temp1="";

	try {
		diasIntervalo = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
		sucursal = mFg.ExtraeStringDeBuffer(&parametros);

		if (sucursal!="") {
			condicion_sucursal.sprintf(" AND s.sucursal = '%s' ", sucursal);
		}

		instruccion = "CREATE TEMPORARY TABLE auxpediddetalle (orden INT, sku VARCHAR(9), \
		sucursal VARCHAR(2), disponible INT, INDEX(orden, sku)) ENGINE = INNODB";
		instrucciones[num_instrucciones++] = instruccion;

		archivo_temp1 = mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);

		instruccion.sprintf("SELECT ed.nopedido,                                          \
		ed.sku,                                                      \
		s.sucursal,                                                  \
		IF(ep.fraccionar!=0,IF(ed.cantidad/NULLIF(ep.fraccionar,0)>SUM(e.cantidad),0,1),IF(ed.cantidad>SUM(e.cantidad),0,1)) AS disponible \
		FROM ecommercedetalle ed                                     \
		INNER JOIN articulos a ON a.articulo = ed.sku                \
		INNER JOIN existenciasactuales e ON e.producto = a.producto  \
		INNER JOIN almacenes alm ON alm.almacen = e.almacen          \
		INNER JOIN secciones s ON s.seccion = alm.seccion            \
		LEFT JOIN ecommerceproductos ep ON ep.articulo = ed.sku \
		WHERE 1 %s                                                   \
		GROUP BY ed.nopedido, s.sucursal, ed.sku  \
		INTO OUTFILE '%s' ",
		condicion_sucursal, archivo_temp1);
		instrucciones[num_instrucciones++] = instruccion;

        instruccion.sprintf(
			"LOAD DATA INFILE '%s' INTO TABLE auxpediddetalle (orden, sku, sucursal, disponible) ", archivo_temp1);
		instrucciones[num_instrucciones++] = instruccion;

		aux_buffer_sql = mFg.AgregaStringABuffer (mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i = 0; i < num_instrucciones; i++)
			aux_buffer_sql = mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);


		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			instruccion.sprintf("SELECT e.nopedido,                                                  \
			e.id,                                                                                    \
			e.nombre,                                                                                \
			e.direccion,                                                                             \
			s.sucursal AS sucursal,                                                                  \
			IF(MIN(auxd.disponible)=0, 'NO DISPONIBLE', 'DISPONIBLE') AS disponble,                  \
			e.fchaalta,                                                                              \
			e.fchamodi,                                                                              \
			e.fchacancel,                                                                            \
			e.total,                                                               					 \
			e.costoenvio,                                                                            \
			UPPER(                                                                                   \
			IF(e.estatus_financiero='pending','Pendiente',                                           \
			IF(e.estatus_financiero='authorized','Autorizado',                                       \
			IF(e.estatus_financiero='partially_paid','Pagado parcialmente',                          \
			IF(e.estatus_financiero='paid','pagado',                                                 \
			IF(e.estatus_financiero='partially_refunded','Reembolsado parcialmente',                 \
			IF(e.estatus_financiero='refunded','reembolsado','Anulado'))))))) AS estatus_financiero, \
			UPPER(                                                                                   \
			IF(e.razon_cancelado='null','',                                                          \
			IF(e.razon_cancelado='customer','Cliente',                                               \
			IF(e.razon_cancelado='fraud','Fraude',                                                   \
			IF(e.razon_cancelado='inventory','Inventario',                                           \
			IF(e.razon_cancelado='declined','Rechazado', 'Otro')))))) AS razon_cancelado,            \
			UPPER(e.estatus) AS estatus,                                                             \
			e.subtotal,                                                                              \
			e.imptotal,                                                                              \
			e.iva,                                                                                   \
			e.peso,                                                                                  \
			e.cantreembolso,                                                                         \
			IF(e.reembolsado=1,'REEMBOLSADO','') AS reembolsado,                                     \
			e.locationid,                                                                            \
			e.latitude,                                                                              \
			e.longitude,                                                                             \
			e.visita                                                                                 \
            ,e.telefono \
			FROM ecommerceorden e                                                                    \
			INNER JOIN ecommercesuccp s ON e.codigopostal = s.cubrecodigop                           \
			INNER JOIN auxpediddetalle auxd ON auxd.orden = e.nopedido AND auxd.sucursal = s.sucursal\
			WHERE e.fchaalta >= date_add(CURDATE(), INTERVAL -%d DAY)                                \
            %s                                                                                       \
			GROUP BY e.nopedido                                                                      \
			ORDER BY e.nopedido DESC, e.fchaalta DESC",diasIntervalo, condicion_sucursal);

			mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
				Respuesta->TamBufferResultado);
		}
	} __finally {
		delete buffer_sql;

		if(archivo_temp1 != "")
			mServidorVioleta->BorraArchivoTemp(archivo_temp1);

	}
}
// ---------------------------------------------------------------------------
//ID_ACTU_PEDIDO
void ServidorCatalogos::ActualizarPedidos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*10];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[30];
	int nroPedido=0;
	AnsiString accion, usuario;

	nroPedido = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
	accion = mFg.ExtraeStringDeBuffer(&parametros);
	usuario = mFg.ExtraeStringDeBuffer(&parametros);

	//iniciar la transaccion
	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	try{
		Instruccion.sprintf("update ecommerceorden	 						       \
			set estatus='%s'												 	   \
			where nopedido=%d ",
		accion, nroPedido
		);
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("INSERT INTO bitacoraecommerceorden (nopedido, usuario, fecha, 		   \
		hora, estatus) VALUES                                     								   \
		(%d,'%s',CURDATE(),CURTIME(),'%s')",
		nroPedido, usuario, accion
		);
		instrucciones[num_instrucciones++]=Instruccion;

		instrucciones[num_instrucciones++]="COMMIT";

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
//ID_BITACORA_ORDENES
void ServidorCatalogos::BitacoraPedidos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	instruccion.sprintf("SELECT b.nopedido, CONCAT(e.nombre, ' ' , e.appat, ' ', e.apmat) AS nombre, \
	b.fecha, b.hora, b.estatus                                                                       \
	FROM bitacoraecommerceorden b                                                                    \
	INNER JOIN empleados e ON b.usuario = e.empleado                                                 \
	ORDER BY b.fecha DESC, b.hora DESC ");

	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------
//ID_ACTUALIZA_PRODUCTOS_ECOMMERCE
void ServidorCatalogos::ActualizaProdECommerce(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*64];
	char *aux_buffer_sql=buffer_sql;
	BufferRespuestas* resp_cant=NULL;
	BufferRespuestas* resp_almacenes=NULL;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[10];
	int inventario=0;
	double precio, cantidadS=0;
	AnsiString sku, idProd, idInv, idVariant;
	AnsiString porcSub, sucursal, almacen, cad_conjunto_almacenes=" ";
	AnsiString barcode;
	AnsiString tags;


	//iniciar la transaccion
	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	try{

		sku = mFg.ExtraeStringDeBuffer(&parametros);
		idProd = mFg.ExtraeStringDeBuffer(&parametros);
		idInv = mFg.ExtraeStringDeBuffer(&parametros);
		inventario = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
		idVariant = mFg.ExtraeStringDeBuffer(&parametros);
		precio = StrToFloat(mFg.ExtraeStringDeBuffer(&parametros));
		porcSub = mFg.ExtraeStringDeBuffer(&parametros);
		sucursal = mFg.ExtraeStringDeBuffer(&parametros);
		tags = mFg.ExtraeStringDeBuffer(&parametros);
		barcode = mFg.ExtraeStringDeBuffer(&parametros);

		Instruccion.sprintf("SELECT a.almacen FROM almacenes a \
		INNER JOIN secciones s ON a.seccion=s.seccion \
		WHERE s.sucursal='%s'", sucursal);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), resp_almacenes);
		for(int i=0; i<resp_almacenes->ObtieneNumRegistros(); i++){
			resp_almacenes->IrAlRegistroNumero(i);
			almacen=resp_almacenes->ObtieneDato("almacen");

			cad_conjunto_almacenes+="'";
			cad_conjunto_almacenes+=almacen;
			cad_conjunto_almacenes+="'";
			if (i<resp_almacenes->ObtieneNumRegistros()-1)
				cad_conjunto_almacenes+=",";
		}

		Instruccion.sprintf( "SELECT SUM(ex.cantidad*%s) AS cantidad FROM existenciasactuales ex \
		INNER JOIN articulos a ON a.articulo = '%s'                                              \
		WHERE a.producto = ex.producto AND a.present = ex.present AND ex.almacen IN (%s) ",porcSub, sku, cad_conjunto_almacenes );
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), resp_cant);
		cantidadS=StrToFloat(resp_cant->ObtieneDato("cantidad"));

		Instruccion.sprintf("update ecommerceproductos 			 	 \
			set idProducto='%s',  idInventario='%s', idVariant='%s', \
			inventario=%d, precio=%f, cantsis=%f,      \
			codigobarrasis='%s', tag='%s'							 \
			where articulo='%s' ",
		idProd,
		idInv,
		idVariant,
		inventario,
		precio,
		cantidadS,
		barcode,
		tags,
		sku
		);
		instrucciones[num_instrucciones++]=Instruccion;

		instrucciones[num_instrucciones++]="COMMIT";

		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);
	} __finally {
		if(buffer_sql != NULL) delete buffer_sql;
		if(resp_cant!=NULL) delete resp_cant;
		if(resp_almacenes!=NULL) delete resp_almacenes;
	}
}
//---------------------------------------------------------------------------
// ID_EJE_REPARTACTXSUC
void ServidorCatalogos::EjecutaArtActxSuc(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString sucursal, almacen, empresa;
	AnsiString instruccion;
	AnsiString cad_conjunto_almacenes=" ";
    AnsiString havings = " HAVING 1 ";

	BufferRespuestas* resp_almacenes=NULL;

	try {
		sucursal = mFg.ExtraeStringDeBuffer(&parametros);
		empresa = mFg.ExtraeStringDeBuffer(&parametros);
		AnsiString soloSinExistencias = mFg.ExtraeStringDeBuffer(&parametros);
		AnsiString soloDescontinuados = mFg.ExtraeStringDeBuffer(&parametros);
		AnsiString diasVtaPromedio = mFg.ExtraeStringDeBuffer(&parametros);
		AnsiString ordenVentaPromedio = mFg.ExtraeStringDeBuffer(&parametros);

		AnsiString orden = " p.nombre ASC, art.present ";

		 if(ordenVentaPromedio == "1"){
			orden = " ventaPromedioDiaria ASC ";
		 }

		AnsiString wheres = " pmm.activo=1 ";
		AnsiString whereAlmacenes;
		if(sucursal != " "){
			whereAlmacenes.sprintf(" AND art.sucursal='%s'", sucursal);
			wheres+= whereAlmacenes;
		} else if (empresa != " "){
			whereAlmacenes.sprintf(" AND suc.idempresa=%s", empresa);
			wheres+= whereAlmacenes;
		}

		if(soloSinExistencias == "1"){
			havings += " AND (existenciamax + existenciamin) = 0 ";
		} else if (soloDescontinuados == "1") {
			havings += " AND descontinuado = 1 ";
		}

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_REP);
		instruccion.sprintf("SELECT \
		art.producto, \
		p.nombre, \
		art.present, \
		GROUP_CONCAT(DISTINCT suc.sucursal ORDER BY suc.sucursal SEPARATOR ', '), \
		CONCAT(e.nombre, ' ', e.appat, ' ', e.apmat) AS usuario, \
		TRUNCATE(SUM(ext.cantidad)/pmm.maxfactor,0) AS existenciamax, \
		TRUNCATE(MOD(SUM(ext.cantidad),pmm.maxfactor),3) AS existenciamin, \
		CONCAT(pmm.maxmult,'X',maxfactor) AS multmax, \
        arc.articulo, \
		IFNULL(ap.descontinuado, ' ') AS descontinuado, \
		TRUNCATE((SUM(ventas%s)/%s)/pmm.maxfactor, 3) as ventaPromedioDiaria, \
		art.fechamodi, \
		art.horamodi \
		FROM articulosxsuc art \
		INNER JOIN sucursales suc ON suc.sucursal = art.sucursal \
		INNER JOIN secciones sec ON sec.sucursal = suc.sucursal \
		INNER JOIN almacenes alm ON alm.seccion = sec.seccion \
		INNER JOIN ventasxmes vxm ON art.producto = vxm.producto AND art.present = vxm.present AND vxm.almacen = alm.almacen \
		INNER JOIN existenciasactuales ext ON ext.producto = art.producto AND ext.present = art.present \
			AND ext.almacen = vxm.almacen \
		INNER JOIN presentacionesminmax pmm ON pmm.producto = art.producto AND pmm.present = art.present \
		INNER JOIN articulos arc ON arc.producto = pmm.producto AND arc.present = pmm.present AND pmm.maxmult = arc.multiplo \
		INNER JOIN productos p ON p.producto = art.producto \
		INNER JOIN empleados e ON art.usumodi = e.empleado \
		LEFT JOIN articulosped ap ON ap.producto=art.producto AND ap.present=art.present AND ap.sucursal = ext.almacen \
		WHERE %s \
		GROUP BY art.producto, art.present \
		%s \
		ORDER BY %s ", diasVtaPromedio, diasVtaPromedio , wheres, havings, orden);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	} __finally {
		if (resp_almacenes!=NULL) delete resp_almacenes;
	}
}
// ---------------------------------------------------------------------------
//ID_EJE_GUARDA_ARTACTXSUC
void ServidorCatalogos::GuardaArtActxSuc(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*10];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i, count=0;
	AnsiString Instruccion, instrucciones[1000];
	AnsiString sucursal, producto, presentacion, usuario;
	BufferRespuestas* resp_count=NULL;

	sucursal = mFg.ExtraeStringDeBuffer(&parametros);
	producto = mFg.ExtraeStringDeBuffer(&parametros);
	presentacion = mFg.ExtraeStringDeBuffer(&parametros);
	usuario = mFg.ExtraeStringDeBuffer(&parametros);
	AnsiString permisoOtroSupervisor = mFg.ExtraeStringDeBuffer(&parametros);

	Instruccion.sprintf("SELECT COUNT(*) AS count FROM articulosxsuc WHERE producto = '%s' AND present = '%s' AND sucursal IN (%s)",
		producto, presentacion, sucursal);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), resp_count);
	count=StrToInt(resp_count->ObtieneDato("count"));

	if (count==0) {
        bool permiso = permisoOtroSupervisor == "1";

		if(!permiso){
			char * bf_supervisor = new char[1000];
			try{
				AnsiString instruccion;
				instruccion.sprintf("SELECT usuario FROM articulossupervisados WHERE producto = '%s' AND present = '%s'",
					producto, presentacion);
				mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), bf_supervisor);
				BufferRespuestas br_supervisor(bf_supervisor);
				AnsiString supervisor =br_supervisor.ObtieneDato();
				permiso =( supervisor == usuario);
			}__finally{
				delete bf_supervisor;
			}
		}

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		if(permiso){
			//iniciar la transaccion
			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

			TStringList * listaSucursales = new TStringList();

			try{

				listaSucursales->DelimitedText = sucursal;

				for(int i = 0; i < listaSucursales->Count; i++){
					AnsiString suc =listaSucursales->Strings[i];

					Instruccion.sprintf("INSERT INTO articulosxsuc (sucursal, producto, present, fechamodi, horamodi, usumodi) VALUES                                     								   \
					(%s,'%s','%s',CURDATE(),CURTIME(),'%s')", suc, producto, presentacion, usuario);
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("INSERT INTO bitacoraarticulosxsuc (sucursal, producto, present, evento, fecha, hora, usuario) VALUES                                     								   \
					(%s,'%s','%s','A',CURDATE(),CURTIME(),'%s')", suc, producto, presentacion, usuario);
					instrucciones[num_instrucciones++]=Instruccion;
				}

				instrucciones[num_instrucciones++]="COMMIT";

				mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, "SELECT 1", Respuesta->TamBufferResultado);

				aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
				for (i=0; i<num_instrucciones; i++)
					aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);


				mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);
			} __finally {
				if (resp_count!=NULL) delete resp_count;
				delete buffer_sql;
				delete listaSucursales;
			}
		} else {
			mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, "SELECT 0", Respuesta->TamBufferResultado);
		}
	} else {
		throw(Exception("Ya se encuentra cargado el producto en la sucursal seleccionada."));
	}
}
// ---------------------------------------------------------------------------
//ID_EJE_BORRA_ARTACTXSUC
void ServidorCatalogos::BorraArtActxSuc(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*10];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[1000];
	AnsiString sucursal, producto, presentacion, usuario;

	sucursal = mFg.ExtraeStringDeBuffer(&parametros);
	producto = mFg.ExtraeStringDeBuffer(&parametros);
	presentacion = mFg.ExtraeStringDeBuffer(&parametros);
	usuario = mFg.ExtraeStringDeBuffer(&parametros);

	//iniciar la transaccion
	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	try{
		Instruccion.sprintf("DELETE FROM articulosxsuc WHERE producto = '%s' AND present = '%s' AND sucursal IN (%s)",
			producto, presentacion, sucursal);
		instrucciones[num_instrucciones++]=Instruccion;

		TStringList * sucursalesEliminar = new TStringList(",");
		try{
			sucursalesEliminar->CommaText = sucursal.c_str();
			for(int i = 0; i < sucursalesEliminar->Count; i++){
				Instruccion.sprintf("INSERT INTO bitacoraarticulosxsuc (sucursal, producto, present, evento, fecha, hora, usuario) VALUES \
					(%s,'%s','%s','B',CURDATE(),CURTIME(),'%s')",
					AnsiString(sucursalesEliminar->Strings[i]), producto, presentacion, usuario);
				instrucciones[num_instrucciones++]=Instruccion;
			}
		}__finally{
			delete sucursalesEliminar;
		}

		instrucciones[num_instrucciones++]="COMMIT";

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
//ID_EJE_ARTICULOS_ORDEN
void ServidorCatalogos::ArticulosOrden(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
	int nroOrder;

	nroOrder = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_REP);
	instruccion.sprintf("SELECT                                 \
	ed.nombre,                                                  \
	ed.sku,                                                     \
	IF(ep.fraccionar!=0 && ep.fraccionar!=1,ROUND(ed.cantidad/(100/ep.fraccionar),2),ed.cantidad) AS cantidad, \
	s.sucursal,                                                 \
	SUM(e.cantidad) AS existencias,                             \
	e.almacen,                                                  \
	ROUND(ed.precio*ed.cantidad,2) AS precio, \
	ed.descuento, 												\
	IF(ep.fraccionar!=0&& ep.fraccionar!=1,ROUND(ed.precio*(100/ep.fraccionar),2),ed.precio) AS preciounitario,	\
	IF(ppa.pesoprom IS NULL,a.peso,ppa.pesoprom) AS peso, \
	ep.tag\
	FROM ecommercedetalle ed                                    \
	INNER JOIN articulos a ON a.articulo = ed.sku               \
	INNER JOIN existenciasactuales e ON e.producto = a.producto AND e.present = a.present \
	INNER JOIN almacenes alm ON alm.almacen = e.almacen         \
	INNER JOIN secciones s ON s.seccion = alm.seccion           \
	INNER JOIN ecommerceproductos ep ON ep.articulo = ed.sku    \
	LEFT JOIN pesopromporarticulo ppa ON ppa.articulo = a.articulo  \
	WHERE ed.nopedido = %d                                      \
	GROUP BY s.sucursal, ed.sku                                 \
	ORDER BY ed.nombre, s.sucursal ",
	nroOrder);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}
//---------------------------------------------------------------------------

// ID_EJE_ARTICULOS_ORDEN_A_REEMBOLSAR
void ServidorCatalogos::ObtenerArticulosOrden(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString sucursal, almacen;
	AnsiString instruccion;

    int nopedido=0;

	nopedido=StrToInt(mFg.ExtraeStringDeBuffer(&parametros));

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
	instruccion.sprintf("SELECT \
	nombre,                     \
	sku,                        \
	cantidad,                   \
	precio                      \
	FROM ecommercedetalle       \
	WHERE nopedido = %d         \
	ORDER BY nombre DESC ",
	nopedido);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}
//---------------------------------------------------------------------------

// ID_EJE_ARTICULOS_A_REEMBOLSAR
void ServidorCatalogos::GuardaProdReembolsar(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*10];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[10];

	AnsiString articulo;
	int nopedido=0, cantidad=0;
	double precio=0, total=0;

	nopedido=StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
	articulo=mFg.ExtraeStringDeBuffer(&parametros);
	cantidad=StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
	precio=StrToFloat(mFg.ExtraeStringDeBuffer(&parametros));

	//iniciar la transaccion
	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	total = (cantidad*precio);
	try{

		Instruccion.sprintf("INSERT INTO ecommerceprodreembolsados \
		(nopedido, sku, cantidadreembolsada, precio, total) 	  \
		VALUES (%d, '%s', %d, %f, %f) ",
		nopedido, articulo, cantidad, precio, total
		);
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("UPDATE ecommercedetalle \
		SET cantidad = (cantidad - %d) 	  			 \
		WHERE nopedido =%d AND sku = '%s'",
		cantidad, nopedido, articulo
		);
		instrucciones[num_instrucciones++]=Instruccion;

		instrucciones[num_instrucciones++]="COMMIT";

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
// ID_EJE_ARTICULOS_REEMBOLSADOS
void ServidorCatalogos::ObtenerArtReembolsados(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString sucursal, almacen;
	AnsiString instruccion;

    int nopedido=0;

	nopedido=StrToInt(mFg.ExtraeStringDeBuffer(&parametros));

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
	instruccion.sprintf("SELECT ed.nombre,              							\
	r.sku, r.cantidadreembolsada, r.precio,            								\
	r.total, ed.id, eo.locationid				        	   		   				\
	FROM ecommerceprodreembolsados r                   								\
	INNER JOIN ecommerceorden eo ON eo.nopedido = r.nopedido    					\
	INNER JOIN ecommercedetalle ed ON ed.nopedido = eo.nopedido AND ed.sku = r.sku  \
	WHERE r.nopedido = %d                              								\
	GROUP BY r.sku                                              					\
	ORDER BY ed.nombre DESC ",
	nopedido);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}
//---------------------------------------------------------------------------
// ID_EJE_ARTICULOS_ACTUALIZAR_ORDEN
void ServidorCatalogos::ActualizarOrdenReembolsada(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*10];
	char *aux_buffer_sql=buffer_sql;
	BufferRespuestas* resp_reembolso=NULL;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[10];

	AnsiString usuario;
	int nopedido=0;
	double total=0;

	nopedido=StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
	usuario=mFg.ExtraeStringDeBuffer(&parametros);

	Instruccion.sprintf("SELECT SUM(cantidadreembolsada * precio) AS Total \
	FROM ecommerceprodreembolsados                                         \
	WHERE nopedido = %d ", nopedido);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), resp_reembolso);
	total=StrToFloat(resp_reembolso->ObtieneDato("Total"));

	//iniciar la transaccion
	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";
	try{

		Instruccion.sprintf("UPDATE ecommerceorden \
		SET reembolsado = 1, cantreembolso = %f    \
		WHERE nopedido =%d",
		total, nopedido
		);
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("INSERT INTO bitacoraecommerceorden  \
		(nopedido, usuario, fecha, hora, estatus) VALUES         \
		(%d,'%s',CURDATE(),CURTIME(),'%s')",
		nopedido, usuario, "REEMBOLSADO"
		);
		instrucciones[num_instrucciones++]=Instruccion;

		instrucciones[num_instrucciones++]="COMMIT";

		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);

	} __finally {
		if (resp_reembolso!=NULL) delete resp_reembolso;
		delete buffer_sql;
	}
}
//---------------------------------------------------------------------------
// ID_ACTU_PEDIDO_VTA_REL
void ServidorCatalogos::ActualizarPedidoVtaRel(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*10];
	char *aux_buffer_sql=buffer_sql;
	BufferRespuestas* resp_reembolso=NULL;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[10];

	AnsiString usuario, referenciarel;
	int nopedido=0;

	nopedido=StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
	referenciarel=mFg.ExtraeStringDeBuffer(&parametros);

	//iniciar la transaccion
	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";
	try{

		Instruccion.sprintf("UPDATE ecommerceorden \
		SET referencia = '%s'    				   \
		WHERE nopedido =%d",
		referenciarel, nopedido
		);
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("INSERT INTO bitacoraecommerceorden  \
		(nopedido, usuario, fecha, hora, estatus) VALUES         \
		(%d,'%s',CURDATE(),CURTIME(),'%s')",
		nopedido, usuario, "MODIFICADO"
		);
		instrucciones[num_instrucciones++]=Instruccion;

		instrucciones[num_instrucciones++]="COMMIT";

		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);

	} __finally {
		if (resp_reembolso!=NULL) delete resp_reembolso;
		delete buffer_sql;
	}
}
//---------------------------------------------------------------------------
// ID_EJE_ART_VTA_REL
void ServidorCatalogos::ArticulosVentaRelacionada(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;

	int nopedido=0;
	AnsiString referencia;

	nopedido=StrToInt(mFg.ExtraeStringDeBuffer(&parametros));

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
	instruccion.sprintf("SELECT                                         \
	t.nombre,                                                           \
	t.present,                                                          \
	t.multiplo,                                                         \
	t.sku,                                                              \
	SUM(t.cantidad) AS cantidad,                                        \
	SUM(t.cantsshop) AS cantsshop,                                      \
	SUM(t.precio) AS precio,                                            \
	SUM(t.precioshop) AS precioshop                                     \
	FROM (                                                              \
			(                                                           \
				SELECT                                                  \
				p.nombre,                                               \
				a.present,                                              \
				a.multiplo,                                             \
				ed.sku, 0 AS cantidad,                                  \
				ed.cantidad AS cantsshop,                               \
				0 AS precio,                                            \
				ed.precio AS precioshop                                 \
				FROM ecommerceorden eo                                  \
				INNER JOIN ecommercedetalle ed ON ed.nopedido = eo.nopedido \
				INNER JOIN articulos a ON a.articulo = ed.sku           \
				INNER JOIN productos p ON p.producto = a.producto       \
				WHERE eo.nopedido = %d                                  \
			)                                                           \
			UNION ALL                                                   \
			(                                                           \
				SELECT                                                  \
				p.nombre,                                               \
				a.present,                                              \
				a.multiplo,                                             \
				a.articulo,                                             \
				dv.cantidad,                                            \
				0 AS cantsshop,                                         \
				dv.precio,                                              \
				0 AS precioshop                                         \
				FROM ecommerceorden eo                                  \
				INNER JOIN ventas v ON v.referencia = eo.referencia     \
				INNER JOIN dventas dv ON dv.referencia = v.referencia   \
				INNER JOIN articulos a ON a.articulo = dv.articulo      \
				INNER JOIN productos p ON p.producto = a.producto       \
				WHERE eo.nopedido = %d                               	\
			)                                                           \
	) t                                                                 \
	GROUP BY t.sku                                                      \
	ORDER BY t.nombre ASC ",
	nopedido,
	nopedido);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}
//---------------------------------------------------------------------------
// ID_GUARDA_ARTICULO_A_SUC
void ServidorCatalogos::AsignaArticuloxSuc(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*64];
	char *aux_buffer_sql=buffer_sql;
	BufferRespuestas* resp=NULL;
	int num_instrucciones=0, i, j;
	AnsiString Instruccion, instrucciones[50];

	AnsiString usuario, producto, presentacion, sucursal;
	int totalSuc=0,estatusT=0,estatus=0;

	totalSuc=StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
	usuario=mFg.ExtraeStringDeBuffer(&parametros);
	producto=mFg.ExtraeStringDeBuffer(&parametros);
	presentacion=mFg.ExtraeStringDeBuffer(&parametros);
	AnsiString permisoOtroSupervisor = mFg.ExtraeStringDeBuffer(&parametros);

	bool permiso = permisoOtroSupervisor == "1";

	if(!permiso){
		char * bf_supervisor = new char[1000];
		try{
			AnsiString instruccion;
			instruccion.sprintf("SELECT usuario FROM articulossupervisados WHERE producto = '%s' AND present = '%s'",
				producto, presentacion);
			mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), bf_supervisor);
			BufferRespuestas br_supervisor(bf_supervisor);
			AnsiString supervisor =br_supervisor.ObtieneDato();
			permiso =( supervisor == usuario);
		}__finally{
			delete bf_supervisor;
		}
	}

	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";
	try{
		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);

		if(permiso){
			for (i = 0; i < totalSuc; i++) {
				sucursal=mFg.ExtraeStringDeBuffer(&parametros);
				estatus=StrToBool(mFg.ExtraeStringDeBuffer(&parametros));

				try {
					Instruccion.sprintf("SELECT count(*) AS estatus FROM articulosxsuc \
					WHERE producto = '%s' AND present = '%s' AND sucursal = '%s'", producto, presentacion, sucursal);
					mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), resp);
					estatusT=StrToInt(resp->ObtieneDato("estatus"));
				} __finally {
					if (resp!=NULL) delete resp;
				}

				if (estatusT==0) {
					if (estatus==1) {
						Instruccion.sprintf("INSERT INTO articulosxsuc		\
						(sucursal, producto, present, fechamodi, horamodi, usumodi) \
						VALUES ('%s', '%s', '%s', CURDATE(),CURTIME(), '%s') ",
						sucursal, producto, presentacion, usuario
						);
						instrucciones[num_instrucciones++]=Instruccion;

						Instruccion.sprintf("INSERT INTO bitacoraarticulosxsuc             \
						(sucursal, producto, present, evento, fecha, hora, usuario) VALUES \
						('%s','%s','%s','A',CURDATE(),CURTIME(),'%s')",
						sucursal, producto, presentacion, usuario
						);
						instrucciones[num_instrucciones++]=Instruccion;
					}
				} else if (estatusT==1) {
					if (estatusT!=estatus) {
						Instruccion.sprintf("DELETE FROM articulosxsuc WHERE sucursal = '%s' AND producto = '%s' AND present = '%s'",
						sucursal, producto, presentacion
						);
						instrucciones[num_instrucciones++]=Instruccion;

						Instruccion.sprintf("INSERT INTO bitacoraarticulosxsuc             \
						(sucursal, producto, present, evento, fecha, hora, usuario) VALUES \
						('%s','%s','%s','B',CURDATE(),CURTIME(),'%s')",
						sucursal, producto, presentacion, usuario
						);
						instrucciones[num_instrucciones++]=Instruccion;
					}
				}
			}

			instrucciones[num_instrucciones++]="COMMIT";

			mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, "SELECT 1", Respuesta->TamBufferResultado);

			aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
			for (j=0; j<num_instrucciones; j++)
				aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[j], aux_buffer_sql);

			mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);
		} else {
			mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, "SELECT 0", Respuesta->TamBufferResultado);
		}

	} __finally {
		if (buffer_sql!=NULL) delete buffer_sql;
	}
}
//---------------------------------------------------------------------------
// ID_APLICA_FRACCIONAR_PRODUCTOS
void ServidorCatalogos::EjecutaFraccionarproductos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*30];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[5];
	AnsiString articulos, tipo_tag=" ";
    double fraccion=0;

	fraccion = StrToFloat(mFg.ExtraeStringDeBuffer(&parametros));
	articulos = mFg.ExtraeStringDeBuffer(&parametros);

	//iniciar la transaccion
	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	try{

		Instruccion.sprintf("UPDATE ecommerceproductos ep          \
		INNER JOIN articulos a ON a.articulo = ep.articulo         \
		INNER JOIN presentaciones pre ON pre.producto = a.producto \
		AND pre.present = a.present                                \
		SET ep.fraccionar=%f				                       \
		WHERE ep.articulo IN(%s) AND pre.permitfrac = 1            \
		AND a.multiplo = 'KILO'",
		fraccion,
		articulos);
		instrucciones[num_instrucciones++]=Instruccion;

		instrucciones[num_instrucciones++]="COMMIT";

		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			Instruccion.sprintf("SELECT COUNT(*) AS reg_actu           \
			FROM ecommerceproductos ep                                 \
			INNER JOIN articulos a ON a.articulo = ep.articulo         \
			INNER JOIN presentaciones pre ON pre.producto = a.producto \
			AND pre.present = a.present                                \
			WHERE ep.articulo IN(%s) AND pre.permitfrac = 1            \
			AND a.multiplo = 'KILO'",articulos);

			mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, Instruccion.c_str(),
				Respuesta->TamBufferResultado);
		}

	} __finally {
		if(buffer_sql != NULL) delete buffer_sql;
	}
}
//---------------------------------------------------------------------------
// ID_ACTU_TIPOPREC_PRODUCTOS
void ServidorCatalogos::EjecutaActuTipoPrecioProductos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*30];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[10];
	AnsiString articulos, tipoPrecio;

	tipoPrecio = mFg.ExtraeStringDeBuffer(&parametros);
	articulos = mFg.ExtraeStringDeBuffer(&parametros);

	//iniciar la transaccion
	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	try{

		Instruccion.sprintf("UPDATE ecommerceproductos ep \
		SET ep.tipodeprec='%s'  						  \
		WHERE ep.articulo IN(%s)",
		tipoPrecio,
		articulos);
		instrucciones[num_instrucciones++]=Instruccion;

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
// ID_ACTU_DESCRAPPI
void ServidorCatalogos::EjecutaActuDescRappi(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*30];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[10];
	AnsiString articulos, fechaini, fechafin, porcdesc;
	int aplicadesc = 0;

	fechaini = mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
	fechafin = mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
	aplicadesc = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
	porcdesc = mFg.ExtraeStringDeBuffer(&parametros);
	articulos = mFg.ExtraeStringDeBuffer(&parametros);

	//iniciar la transaccion
	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	try{

		if (aplicadesc==1) {
			Instruccion.sprintf("UPDATE ecommerceproductos ep \
			SET ep.aplicadescrappi=%d, ep.fechainidesc='%s', ep.fechafindesc='%s', ep.porcdescrappi='%s' \
			WHERE ep.rappi = 1 AND ep.articulo IN(%s)",
			aplicadesc, fechaini, fechafin, porcdesc, articulos);
			instrucciones[num_instrucciones++]=Instruccion;
		} else {
			Instruccion.sprintf("UPDATE ecommerceproductos ep \
			SET ep.aplicadescrappi=0, ep.fechainidesc=NULL, ep.fechafindesc=NULL, ep.porcdescrappi=NULL \
			WHERE ep.rappi = 1 AND ep.articulo IN(%s)",
			articulos);
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
//---------------------------------------------------------------------------
// ID_ACTU_RAPPI_PRODUCTOS
void ServidorCatalogos::EjecutaActuRappiProductos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*30];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[10];
	AnsiString articulos, rappiEstatus;

	rappiEstatus = mFg.ExtraeStringDeBuffer(&parametros);
	articulos = mFg.ExtraeStringDeBuffer(&parametros);

	//iniciar la transaccion
	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	try{

		Instruccion.sprintf("UPDATE ecommerceproductos ep \
		SET ep.rappi='%s'  						  \
		WHERE ep.articulo IN(%s)",
		rappiEstatus,
		articulos);
		instrucciones[num_instrucciones++]=Instruccion;

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
// ID_CON_ARTPESOPROM
void ServidorCatalogos::EjecutaObtenerArtiPesoProm(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
	instruccion.sprintf("SELECT                              \
	ppa.articulo,                                            \
	ppa.pesoprom,                                            \
	p.nombre,                                                \
	a.producto,                                              \
	a.present,                                               \
	CONCAT(e.nombre, ' ', e.appat, ' ', e.apmat) AS usuario, \
	ppa.fchalta,                                             \
	ppa.fchamod                                              \
	FROM pesopromporarticulo ppa                             \
	INNER JOIN articulos a ON a.articulo = ppa.articulo      \
	INNER JOIN productos p ON a.producto = p.producto        \
	INNER JOIN empleados e ON e.empleado = ppa.usuario       \
	ORDER BY a.producto, a.present ");
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}
//---------------------------------------------------------------------------
// ID_GUARDA_ARTPESOPROM
void ServidorCatalogos::GuardaArtiPesoProm(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*30];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[5];
	AnsiString articulo, usuario;
    double pesoprom;

	articulo = mFg.ExtraeStringDeBuffer(&parametros);
	pesoprom = StrToFloat(mFg.ExtraeStringDeBuffer(&parametros));
	usuario = mFg.ExtraeStringDeBuffer(&parametros);

	try{

		Instruccion.sprintf("INSERT INTO pesopromporarticulo 	\
		(articulo, pesoprom, fchalta, horaalta, usuario) VALUES \
		('%s', %f, CURDATE(), CURTIME(), '%s')",
		articulo,
		pesoprom,
		usuario);
		instrucciones[num_instrucciones++]=Instruccion;

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
// ID_ACTU_ARTPESOPROM
void ServidorCatalogos::ActuArtiPesoProm(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*30];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[5];
	AnsiString articulo, usuario;
    double pesoprom;

	articulo = mFg.ExtraeStringDeBuffer(&parametros);
	pesoprom = StrToFloat(mFg.ExtraeStringDeBuffer(&parametros));

	//iniciar la transaccion
	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	try{

		Instruccion.sprintf("UPDATE pesopromporarticulo 	\
		SET pesoprom = %f, fchamod = CURDATE(), horamod = CURTIME() \
		WHERE articulo = '%s' ",
		pesoprom,
		articulo);
		instrucciones[num_instrucciones++]=Instruccion;

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
// ID_DELETE_ARTPESOPROM
void ServidorCatalogos::DeleteArtiPesoProm(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*30];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[5];
	AnsiString articulo, usuario;
    double pesoprom;

	articulo = mFg.ExtraeStringDeBuffer(&parametros);

	//iniciar la transaccion
	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	try{

		Instruccion.sprintf("DELETE FROM pesopromporarticulo 	\
		WHERE articulo = '%s' ",
		articulo);
		instrucciones[num_instrucciones++]=Instruccion;

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
// ---------------------------------------------------------------------------
//ID_ACTU_ORDER_VISITA
void ServidorCatalogos::ActuaOrdenVisita(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*10];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[30];
	int nroPedido=0;

	nroPedido = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));

	//iniciar la transaccion
	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	try{
		Instruccion.sprintf("update ecommerceorden \
			set visita=1					   \
			where nopedido=%d ",
		nroPedido
		);
		instrucciones[num_instrucciones++]=Instruccion;

		instrucciones[num_instrucciones++]="COMMIT";

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
//ID_EJE_FOLIO_TRANSF_ACTUAL
void ServidorCatalogos::EjecutaFolioTransfActual(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*10];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i;
	AnsiString instruccion, instrucciones[15];
	try{
		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		instruccion.sprintf("select @folioaux:=valor from foliosemp where folio='TRANSAC' AND sucursal = '%s' for update ",FormServidor->ObtieneClaveSucursal());
		instrucciones[num_instrucciones++]=instruccion;

		instruccion = "set @foliosig=@folioaux+1 ";
		instrucciones[num_instrucciones++]=instruccion;

		instruccion = " set @folioaux=cast(@folioaux as char)";
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf(" set @folio=concat('%s', lpad(@folioaux,9,'0')) ", FormServidor->ObtieneClaveSucursal());
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf( " update foliosemp set valor=@foliosig where folio='TRANSAC' AND sucursal = '%s'  ",FormServidor->ObtieneClaveSucursal());
		instrucciones[num_instrucciones++]=instruccion;

		instrucciones[num_instrucciones++]="COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql))
		{
			// Resultado final
			instruccion.sprintf("select @folio AS folioTrans");
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	} __finally {
		delete buffer_sql;
	}
}
// ---------------------------------------------------------------------------
//ID_EJE_GUARDA_BITACORA_TRANSACC_BILLETO
void ServidorCatalogos::EjecutaGuardaBitacoraTransaccionBilleto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*10];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[10];

	AnsiString idBitacora, idLocation, idEmpotencyKey, merchantTranId, estatus, terminal;
	AnsiString messageError, usuario, whatsappCliente, nombrecliente, codigoCliente;
	double totalcobrado = 0, cantReembolso = 0;
	int codigoRespuesta=0;

	idBitacora 		= mFg.ExtraeStringDeBuffer(&parametros);
	idLocation 		= mFg.ExtraeStringDeBuffer(&parametros);
	idEmpotencyKey 	= mFg.ExtraeStringDeBuffer(&parametros);
	merchantTranId 	= mFg.ExtraeStringDeBuffer(&parametros);
	estatus 		= mFg.ExtraeStringDeBuffer(&parametros);
	totalcobrado 	= StrToFloat(mFg.ExtraeStringDeBuffer(&parametros));
	messageError 	= mFg.ExtraeStringDeBuffer(&parametros);
	usuario 		= mFg.ExtraeStringDeBuffer(&parametros);
	whatsappCliente	= mFg.ExtraeStringDeBuffer(&parametros);
	nombrecliente 	= mFg.ExtraeStringDeBuffer(&parametros);
	codigoRespuesta = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
	codigoCliente 	= mFg.ExtraeStringDeBuffer(&parametros);
	cantReembolso 	= StrToFloat(mFg.ExtraeStringDeBuffer(&parametros));

	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	try{

		Instruccion.sprintf(" INSERT INTO bitacoratransaccionesbilleto \
		(idBilleto, fecha, hora, location_id, idempotency_key, merchant_transaction_id, total_cobrado, estatus, \
		cliente, whatsappcliente, codigocliente, usuario, mensajeservidor, codigorespuesta, total_reembolsado) VALUES \
		('%s', CURDATE(), CURTIME(), '%s', '%s', '%s', %f, '%s', '%s', '%s', '%s', '%s', '%s', %d, %f) ",
		idBitacora,
		idLocation,
		idEmpotencyKey,
		merchantTranId,
		totalcobrado,
		estatus,
		nombrecliente,
		whatsappCliente,
		codigoCliente,
		usuario,
		messageError,
		codigoRespuesta,
		cantReembolso);
		instrucciones[num_instrucciones++]=Instruccion;

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
//ID_EJE_OBTENER_TRANSF_CANCELAR
void ServidorCatalogos::EjecutaObtenerTransfCancelar(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString Instruccion;

	AnsiString referencia;

	referencia = mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
	Instruccion.sprintf("SELECT btb.idBilleto, btb.total_cobrado	\
	FROM billetopagos bp                            			 	\
	INNER JOIN bitacoratransaccionesbilleto btb     				\
	ON btb.merchant_transaction_id = bp.transaccion 				\
	WHERE btb.estatus = 'C' AND bp.venta = '%s'",
	referencia);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------
//ID_EJE_GUARDA_BITACORA_DEPOSITO_BILLETO
void ServidorCatalogos::EjecutaGuardaBitacoraDepositoBilleto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*10];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[10];

	AnsiString messageError, codigoError, keyDeposito, estatus, curpCli, nombreBenefi;
	AnsiString numExtBenefi, numIntBenefi, colonia, calle, referenciaDep, telefono, primerNom;
	AnsiString segundoNom, paternoNom, maternoNom, fechaNaci, nacionalidad, location_id, usuario;
    AnsiString terminal;
	int direccionReq=0, curpReq=0, codigoRespuesta=0;
	double cantidadDepo=0, comision=0;

	estatus 		= mFg.ExtraeStringDeBuffer(&parametros);
	messageError 	= mFg.ExtraeStringDeBuffer(&parametros);
	codigoRespuesta	= StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
	keyDeposito 	= mFg.ExtraeStringDeBuffer(&parametros);
	referenciaDep 	= mFg.ExtraeStringDeBuffer(&parametros);
	curpCli 		= mFg.ExtraeStringDeBuffer(&parametros);
	nombreBenefi 	= mFg.ExtraeStringDeBuffer(&parametros);
	numExtBenefi	= mFg.ExtraeStringDeBuffer(&parametros);
	numIntBenefi 	= mFg.ExtraeStringDeBuffer(&parametros);
	colonia 		= mFg.ExtraeStringDeBuffer(&parametros);
	calle	 		= mFg.ExtraeStringDeBuffer(&parametros);
	telefono 		= mFg.ExtraeStringDeBuffer(&parametros);
	primerNom 		= mFg.ExtraeStringDeBuffer(&parametros);
	segundoNom 		= mFg.ExtraeStringDeBuffer(&parametros);
	paternoNom 		= mFg.ExtraeStringDeBuffer(&parametros);
	maternoNom 		= mFg.ExtraeStringDeBuffer(&parametros);
	fechaNaci 		= mFg.ExtraeStringDeBuffer(&parametros);
	nacionalidad	= mFg.ExtraeStringDeBuffer(&parametros);
	cantidadDepo	= StrToFloat(mFg.ExtraeStringDeBuffer(&parametros));
	comision		= StrToFloat(mFg.ExtraeStringDeBuffer(&parametros));
	direccionReq	= StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
	curpReq			= StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
	location_id		= mFg.ExtraeStringDeBuffer(&parametros);
	usuario			= mFg.ExtraeStringDeBuffer(&parametros);
	terminal		= mFg.ExtraeStringDeBuffer(&parametros);

	if (curpCli=="") {
		curpCli=" ";
	}

	if (nombreBenefi=="") {
		nombreBenefi=" ";
	}

	if (numExtBenefi=="") {
		numExtBenefi=" ";
	}

	if (numIntBenefi=="") {
		numIntBenefi=" ";
	}

	if (colonia=="") {
		colonia=" ";
	}

	if (calle=="") {
		calle=" ";
	}

	if (nacionalidad=="") {
		nacionalidad=" ";
	}

	if (fechaNaci=="") {
		fechaNaci="0000-00-00";
	}

	if (primerNom=="") {
		primerNom=" ";
	}

	if (segundoNom=="") {
		segundoNom=" ";
	}

	if (paternoNom=="") {
		paternoNom=" ";
	}

	if (maternoNom=="") {
		maternoNom=" ";
	}

	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	try{

		Instruccion.sprintf(" INSERT INTO bitacoradepositosbilleto \
		(keydeposito, refdeposito, curp, fecha, hora, nombrebenefi, numeroext, numeroint, colonia, calle, \
		telefono, primernombre, segundonombre, apellpater, apellmater, fechanac, nacionalidad, monto, \
		direccreq, curpreq, comision, location_id, usuario, mensajeservidor, codigorespuesta, estatus, terminal) VALUES \
		('%s', '%s', '%s', CURDATE(), CURTIME(), '%s', '%s', '%s', '%s', '%s', \
		'%s', '%s', '%s', '%s', '%s', '%s', '%s', %f, \
		%d, %d, %f, '%s', '%s', '%s', %d, '%s', '%s')",
		keyDeposito,
		referenciaDep,
		curpCli,
		nombreBenefi,
		numExtBenefi,
		numIntBenefi,
		colonia,
		calle,

		telefono,
		primerNom,
		segundoNom,
		paternoNom,
		maternoNom,
		fechaNaci,
		nacionalidad,
		cantidadDepo,

		direccionReq,
		curpReq,
		comision,
		location_id,
		usuario,
		messageError,
		codigoRespuesta,
		estatus,
		terminal);
		instrucciones[num_instrucciones++]=Instruccion;

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
//ID_EJE_GUARDA_DEPOSITO_COMPLETO_BILLETO
void ServidorCatalogos::EjecutaGuardaDepositoCompletoBilleto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*10];
	char *aux_buffer_sql=buffer_sql;
	BufferRespuestas* resp=NULL;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[10];

	AnsiString keyDeposito, terminal, usuario;

	keyDeposito = mFg.ExtraeStringDeBuffer(&parametros);
	terminal 	= mFg.ExtraeStringDeBuffer(&parametros);
	usuario 	= mFg.ExtraeStringDeBuffer(&parametros);

	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	try {
		Instruccion.sprintf(" INSERT INTO billetodepositos \
		(keydeposito, terminal, usuario, fechaalta, horaalta) VALUES \
		('%s', '%s', '%s', CURDATE(), CURTIME())",
		keyDeposito,
		terminal,
		usuario);
		instrucciones[num_instrucciones++]=Instruccion;

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
//ID_EJE_GUARDA_BITACORA_RETIRO_BILLETO
void ServidorCatalogos::EjecutaGuardaBitacoraRetiroBilleto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*10];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[10];

	AnsiString messageError, codigoError, keyRetiro, estatus, curpCli, nombreBenefi;
	AnsiString numExtBenefi, numIntBenefi, colonia, calle, refRetiro, telefono, primerNom;
	AnsiString segundoNom, paternoNom, maternoNom, fechaNaci, nacionalidad, location_id, usuario;
    AnsiString terminal;
	int direccionReq=0, curpReq=0, telReq=0, codigoRespuesta=0;
	double cantidadOrig=0, cantidadAjus=0, comision=0, cantAdjust=0;

	estatus 		= mFg.ExtraeStringDeBuffer(&parametros);
	messageError 	= mFg.ExtraeStringDeBuffer(&parametros);
	codigoRespuesta	= StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
	keyRetiro	 	= mFg.ExtraeStringDeBuffer(&parametros);
	refRetiro	 	= mFg.ExtraeStringDeBuffer(&parametros);
	curpCli 		= mFg.ExtraeStringDeBuffer(&parametros);
	nombreBenefi 	= mFg.ExtraeStringDeBuffer(&parametros);
	numExtBenefi	= mFg.ExtraeStringDeBuffer(&parametros);
	numIntBenefi 	= mFg.ExtraeStringDeBuffer(&parametros);
	colonia 		= mFg.ExtraeStringDeBuffer(&parametros);
	calle	 		= mFg.ExtraeStringDeBuffer(&parametros);
	telefono 		= mFg.ExtraeStringDeBuffer(&parametros);
	primerNom 		= mFg.ExtraeStringDeBuffer(&parametros);
	segundoNom 		= mFg.ExtraeStringDeBuffer(&parametros);
	paternoNom 		= mFg.ExtraeStringDeBuffer(&parametros);
	maternoNom 		= mFg.ExtraeStringDeBuffer(&parametros);
	fechaNaci 		= mFg.ExtraeStringDeBuffer(&parametros);
	nacionalidad	= mFg.ExtraeStringDeBuffer(&parametros);
	cantidadOrig	= StrToFloat(mFg.ExtraeStringDeBuffer(&parametros));
	cantidadAjus	= StrToFloat(mFg.ExtraeStringDeBuffer(&parametros));
	comision		= StrToFloat(mFg.ExtraeStringDeBuffer(&parametros));
	direccionReq	= StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
	curpReq			= StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
	telReq			= StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
	cantAdjust		= StrToFloat(mFg.ExtraeStringDeBuffer(&parametros));
	location_id		= mFg.ExtraeStringDeBuffer(&parametros);
	usuario			= mFg.ExtraeStringDeBuffer(&parametros);
	terminal		= mFg.ExtraeStringDeBuffer(&parametros);

	if (curpCli=="") {
		curpCli=" ";
	}

	if (nombreBenefi=="") {
		nombreBenefi=" ";
	}

	if (numExtBenefi=="") {
		numExtBenefi=" ";
	}

	if (numIntBenefi=="") {
		numIntBenefi=" ";
	}

	if (colonia=="") {
		colonia=" ";
	}

	if (calle=="") {
		calle=" ";
	}

	if (nacionalidad=="") {
		nacionalidad=" ";
	}

	if (fechaNaci=="") {
		fechaNaci="0000-00-00";
	}

	if (primerNom=="") {
		primerNom=" ";
	}

	if (segundoNom=="") {
		segundoNom=" ";
	}

	if (paternoNom=="") {
		paternoNom=" ";
	}

	if (maternoNom=="") {
		maternoNom=" ";
	}

	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	try{

		Instruccion.sprintf(" INSERT INTO bitacoraretirosbilleto 			   \
		(keyretiro, refretiro, curp, fecha, hora, nombrebenefi, numeroext,     \
		numeroint, colonia, calle, telefono, primernombre, segundonombre,      \
		apellpater, apellmater, fechanac, nacionalidad, monto, montomodi, 	   \
		direccreq, curpreq, telreq, cantajustable, comision, location_id,      \
		usuario, mensajeservidor, codigorespuesta, estatus, terminal) VALUES   \
		('%s', '%s', '%s', CURDATE(), CURTIME(), '%s', '%s', '%s', '%s', '%s', \
		'%s', '%s', '%s', '%s', '%s', '%s', '%s', %f, %f, \
		%d, %d, %d, %f, %f, '%s', '%s', '%s', %d, '%s', '%s')",
		keyRetiro,
		refRetiro,
		curpCli,
		nombreBenefi,
		numExtBenefi,
		numIntBenefi,
		colonia,
		calle,

		telefono,
		primerNom,
		segundoNom,
		paternoNom,
		maternoNom,
		fechaNaci,
		nacionalidad,
		cantidadOrig,
		cantidadAjus,

		direccionReq,
		curpReq,
		telReq,
		cantAdjust,
		comision,
		location_id,
		usuario,
		messageError,
		codigoRespuesta,
		estatus,
		terminal);
		instrucciones[num_instrucciones++]=Instruccion;

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
//ID_EJE_GUARDA_RETIRO_COMPLETO_BILLETO
void ServidorCatalogos::EjecutaGuardaRetiroCompletoBilleto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*10];
	char *aux_buffer_sql=buffer_sql;
	BufferRespuestas* resp=NULL;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[10];

	AnsiString refRetiro, terminal, usuario;

	refRetiro = mFg.ExtraeStringDeBuffer(&parametros);
	terminal 	= mFg.ExtraeStringDeBuffer(&parametros);
	usuario 	= mFg.ExtraeStringDeBuffer(&parametros);

	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	try {
		Instruccion.sprintf(" INSERT INTO billetoretiros \
		(keyretiro, terminal, usuario, fechaalta, horaalta) VALUES \
		('%s', '%s', '%s', CURDATE(), CURTIME())",
		refRetiro,
		terminal,
		usuario);
		instrucciones[num_instrucciones++]=Instruccion;

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
//ID_CON_FORMAPAGO
void ServidorCatalogos::ConsultaFormasDePago(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA TERMINO DE PAGO
	AnsiString instruccion;
	AnsiString clave;

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos del termino de pago
	instruccion.sprintf("SELECT * FROM formasdepago WHERE formapag = '%s'",
		clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los terminos de pago
	instruccion =
		"SELECT formapag, descripcion, termino, activomayoreo, activosuper, formapagocfdi FROM formasdepago";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------
// ID_BAJ_FORMAPAGO
void ServidorCatalogos::BajaFormasDePago(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// BAJA TERMINO PAGO
	BajaGenerico(Respuesta, MySQL, parametros, "formasdepago", "formapag");
}

// ---------------------------------------------------------------------------
// ID_GRA_FORMAPAGO
void ServidorCatalogos::GrabaFormasDePago(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA TERMINO DE PAGO
	GrabaGenerico(Respuesta, MySQL, parametros, "formasdepago", "formapag");
}
// ---------------------------------------------------------------------------
//ID_EJE_CATALOGOFRACCECOMMERCE
void ServidorCatalogos::ConsultaFraccionEcommerce(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString Instruccion;

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
	Instruccion.sprintf("SELECT f.fraccion, f.etiqueta, f.porcentaje \
	FROM fraccionamientoproductos f \
	WHERE f.etiqueta NOT IN ('KILO') \
	ORDER BY f.fraccion");
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), Respuesta->TamBufferResultado);
}

// ---------------------------------------------------------------------------
//ID_GUARDA_FRACCECOMMERCE
void ServidorCatalogos::GuardaFraccionEcommerce(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*10];
	char *aux_buffer_sql=buffer_sql;
	BufferRespuestas* resp=NULL;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[10];

	AnsiString fraccion, etiqueta, porcentaje;

	fraccion = mFg.ExtraeStringDeBuffer(&parametros);
	etiqueta = mFg.ExtraeStringDeBuffer(&parametros);
	porcentaje = mFg.ExtraeStringDeBuffer(&parametros);

	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	try {
		Instruccion.sprintf(" INSERT INTO fraccionamientoproductos \
		(fraccion, etiqueta, porcentaje) VALUES \
		('%s', '%s', '%s')",
		fraccion,
		etiqueta,
		porcentaje);
		instrucciones[num_instrucciones++]=Instruccion;

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
//ID_BAJA_FRACCECOMMERCE
void ServidorCatalogos::BajaFraccionEcommerce(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*10];
	char *aux_buffer_sql=buffer_sql;
	BufferRespuestas* resp=NULL;
	BufferRespuestas* resp_total=NULL;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[10];
    double total = 0;
	AnsiString fraccion, etiqueta, porcentaje;

	fraccion = mFg.ExtraeStringDeBuffer(&parametros);

	Instruccion.sprintf("SELECT COUNT(e.fraccionar) total FROM ecommerceproductos e \
	WHERE e.fraccionar = '%s' ", fraccion);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), resp_total);
	total=StrToFloat(resp_total->ObtieneDato("total"));

	if (total>0) {
		if (resp_total!=NULL) delete resp_total;
		throw(Exception("No se puede eliminar debido a que hay productos relacionados con esa fracción."));
	} else {
		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		try {
			Instruccion.sprintf(" DELETE FROM fraccionamientoproductos \
			WHERE fraccion = '%s' ",
			fraccion);
			instrucciones[num_instrucciones++]=Instruccion;

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
}
// ---------------------------------------------------------------------------
//ID_EJE_GUARDA_ARTSUP
void ServidorCatalogos::GuardaArtSupervisados(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*10];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i, count=0;
	AnsiString Instruccion, instrucciones[10];
	AnsiString usuariosupervisor, producto, presentacion, usuario, usuasig;
	BufferRespuestas* resp_count=NULL;

	usuariosupervisor = mFg.ExtraeStringDeBuffer(&parametros);
	producto = mFg.ExtraeStringDeBuffer(&parametros);
	presentacion = mFg.ExtraeStringDeBuffer(&parametros);
	usuario = mFg.ExtraeStringDeBuffer(&parametros);

	Instruccion.sprintf("SELECT COUNT(*) AS count, CONCAT(e.nombre, ' ', e.appat, ' ', e.apmat) AS usuario \
	FROM articulossupervisados a \
	INNER JOIN empleados e ON e.empleado = a.usuario \
	WHERE a.producto = '%s' AND a.present = '%s' \
	",
	producto, presentacion, usuariosupervisor);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), resp_count);
	if (resp_count->ObtieneNumRegistros()>0) {
		count=StrToInt(resp_count->ObtieneDato("count"));
		usuasig=resp_count->ObtieneDato("usuario");
	}

	if (count==0) {
		//iniciar la transaccion
		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		try{

			Instruccion.sprintf("INSERT INTO articulossupervisados (usuario, producto, present) VALUES \
			('%s','%s','%s')",
			usuariosupervisor, producto, presentacion, usuario
			);
			instrucciones[num_instrucciones++]=Instruccion;

			Instruccion.sprintf("INSERT INTO bitacoraarticulossupervisados (producto, present, evento, fecha, hora, usuario) VALUES \
			('%s','%s','A',CURDATE(),CURTIME(),'%s')",
			producto, presentacion, usuario
			);
			instrucciones[num_instrucciones++]=Instruccion;

			instrucciones[num_instrucciones++]="COMMIT";

			aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
			for (i=0; i<num_instrucciones; i++)
				aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

			mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
			mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);

		} __finally {
			if (resp_count!=NULL) delete resp_count;
			delete buffer_sql;
		}
	} else {
		throw(Exception("Ya se encuentra asignado el producto al comprador " + usuasig));
	}
}
// ---------------------------------------------------------------------------
//ID_EJE_BORRA_ARTSUP
void ServidorCatalogos::BorraArtSupervisados(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*10];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[10];
	AnsiString usuariosupervisor, producto, presentacion, usuario;

	producto = mFg.ExtraeStringDeBuffer(&parametros);
	presentacion = mFg.ExtraeStringDeBuffer(&parametros);
	usuariosupervisor = mFg.ExtraeStringDeBuffer(&parametros);
	usuario = mFg.ExtraeStringDeBuffer(&parametros);

	//iniciar la transaccion
	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	try{

		Instruccion.sprintf("DELETE FROM articulossupervisados WHERE producto = '%s' AND present = '%s' AND usuario = '%s'",
		producto, presentacion, usuariosupervisor
		);
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("INSERT INTO bitacoraarticulossupervisados (producto, present, evento, fecha, hora, usuario) VALUES \
		('%s','%s','B',CURDATE(),CURTIME(),'%s')",
		producto, presentacion, usuario
		);
		instrucciones[num_instrucciones++]=Instruccion;

		instrucciones[num_instrucciones++]="COMMIT";

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
// ---------------------------------------------------------------------------

// OFERTAS

// ---------------------------------------------------------------------------
// ID_GRA_OFERTAS
void ServidorCatalogos::GrabaOfertas(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA OFERTAS
	GrabaGenerico(Respuesta, MySQL, parametros, "tiposdedescuentos", "tipo");
}

// ---------------------------------------------------------------------------
// ID_BAJ_OFERTAS
void ServidorCatalogos::BajaOfertas(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// BAJA OFERTAS
	BajaGenerico(Respuesta, MySQL, parametros, "tiposdedescuentos", "tipo");
}

// ---------------------------------------------------------------------------
// ID_CON_OFERTAS
void ServidorCatalogos::ConsultaOfertas(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA OFERTAS
	AnsiString instruccion;
	AnsiString clave_tipoOferta;

	clave_tipoOferta = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos del departamento
	instruccion.sprintf("select * from tiposdedescuentos where tipo='%s'",
		clave_tipoOferta);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los departamentos
	instruccion =
		"select * from tiposdedescuentos  order by tipo";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------
//ID_CON_OPCANCEL
void ServidorCatalogos::ConsultaCancelaciones(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA TERMINO DE PAGO
	AnsiString instruccion;
	AnsiString clave;

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos de las cancelaciones
	instruccion.sprintf("SELECT * FROM catalogocancel WHERE id = %s ",
		clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

	// Obtiene todas las cancelaciones
	instruccion =
		"SELECT id, nombre, activo, motivocanc FROM catalogocancel ORDER BY nombre ";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------
// ID_BAJ_OPCANCEL
void ServidorCatalogos::BajaCancelaciones(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// BAJA TERMINO PAGO
	BajaGenerico(Respuesta, MySQL, parametros, "catalogocancel", "id");
}

// ---------------------------------------------------------------------------
// ID_GRABA_OPCANCEL
void ServidorCatalogos::GrabaCancelaciones(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA TERMINO DE PAGO
	GrabaGenerico(Respuesta, MySQL, parametros, "catalogocancel", "id");
}
// ---------------------------------------------------------------------------
//ID_CON_FPAGOXORI
void ServidorCatalogos::ConsultaOrigenxPago(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
	AnsiString origen, formap;

	origen = mFg.ExtraeStringDeBuffer(&parametros);
	formap = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos de las cancelaciones
	instruccion.sprintf("SELECT f.tipoorigen, t.descripcion, f.formapag, p.descripcion \
	FROM formasdepagoxorigenv f \
	INNER JOIN tiposorigenventas t ON f.tipoorigen = t.tipoorigen \
	INNER JOIN formasdepago p ON p.formapag = f.formapag \
	WHERE f.tipoorigen = '%s' AND f.formapag = '%s' ",
		origen, formap);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

	// Obtiene todas las cancelaciones
	instruccion = "SELECT f.tipoorigen, t.descripcion, f.formapag, p.descripcion \
	FROM formasdepagoxorigenv f \
	INNER JOIN tiposorigenventas t ON f.tipoorigen = t.tipoorigen \
	INNER JOIN formasdepago p ON p.formapag = f.formapag \
	ORDER BY t.descripcion, p.descripcion ";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------
// ID_BAJ_FPAGOXORI
void ServidorCatalogos::BajaOrigenxPago(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*10];
	char *aux_buffer_sql=buffer_sql;
	BufferRespuestas* resp=NULL;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[10];

	AnsiString instruccion;
	AnsiString origen, formap;

	origen = mFg.ExtraeStringDeBuffer(&parametros);
	formap = mFg.ExtraeStringDeBuffer(&parametros);

	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	try {
		Instruccion.sprintf(" DELETE FROM formasdepagoxorigenv \
		WHERE tipoorigen = '%s' AND formapag = '%s' ",
		origen, formap);
		instrucciones[num_instrucciones++]=Instruccion;

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
// ID_GRABA_FPAGOXORI
void ServidorCatalogos::GrabaOrigenxPago(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*10];
	char *aux_buffer_sql=buffer_sql;
	BufferRespuestas* resp=NULL;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[10];

	AnsiString instruccion;
	AnsiString origen, formap, tipo_tarea;
	AnsiString origenventa_original, formadepago_original;

	origen = mFg.ExtraeStringDeBuffer(&parametros);
	formap = mFg.ExtraeStringDeBuffer(&parametros);
	tipo_tarea = mFg.ExtraeStringDeBuffer(&parametros);
	origenventa_original = mFg.ExtraeStringDeBuffer(&parametros);
	formadepago_original = mFg.ExtraeStringDeBuffer(&parametros);

	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	try {
		if (tipo_tarea=="A") {
			Instruccion.sprintf(" INSERT INTO formasdepagoxorigenv (tipoorigen, formapag) VALUES \
			('%s', '%s') ",
			origen, formap);
			instrucciones[num_instrucciones++]=Instruccion;
		} else {
			Instruccion.sprintf(" UPDATE formasdepagoxorigenv SET formapag = '%s' \
			WHERE tipoorigen = '%s' AND formapag = '%s' ",
			formap,
			origenventa_original, formadepago_original);
			instrucciones[num_instrucciones++]=Instruccion;
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
// ---------------------------------------------------------------------------
// MODIFICAR CLAVE DE PRODUCTO

// ---------------------------------------------------------------------------
// ID_MOD_CVE_PROD
void ServidorCatalogos::GrabaModClaveProducto(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA ID_MOD_PRODUCTO
	char *buffer_sql = new char[1024 * 10];
	char *aux_buffer_sql = buffer_sql;
	AnsiString clave_producto_original, clave_producto_nueva, clave_articulo, usuario;
	int num_instrucciones = 0;
	AnsiString instruccion, instrucciones[100];
	AnsiString idProgramado, sucursalProgramada, fechaProgramada;

	BufferRespuestas * respuesta_empresas = NULL;

	clave_producto_original = mFg.ExtraeStringDeBuffer(&parametros);
	clave_producto_nueva = mFg.ExtraeStringDeBuffer(&parametros);
	clave_articulo = mFg.ExtraeStringDeBuffer(&parametros);
	usuario = mFg.ExtraeStringDeBuffer(&parametros);
	idProgramado = mFg.ExtraeStringDeBuffer(&parametros);
	sucursalProgramada = mFg.ExtraeStringDeBuffer(&parametros);
	fechaProgramada = mFg.ExtraeStringDeBuffer(&parametros);

	try {
        mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, "SELECT idempresa FROM empresas", respuesta_empresas);

		instrucciones[num_instrucciones++]="SET SESSION sql_log_bin = 0";
		instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++] = "START TRANSACTION";

		// Borra los registros de presentacionesminmax (posteriormente se regenerara con la nueva clave d eproducto).
		instruccion.sprintf("DELETE FROM presentacionesminmax WHERE producto = '%s'",clave_producto_original);
		instrucciones[num_instrucciones++] = instruccion;

		// Actualiza la clave de producto en la tabla productos y en las que se hace referencia a esta.
		instruccion.sprintf(
			"update productos set producto='%s' where producto='%s'"
				,clave_producto_nueva, clave_producto_original);
		instrucciones[num_instrucciones++] = instruccion;

		// Inserta los registros pertinentes en presentacionesminmax con la nueva clave de producto.
		instruccion.sprintf("INSERT INTO presentacionesminmax (producto,present,maxfactor,maxmult,minmult,activo) \
			(SELECT amm.producto, amm.present, amm.maxfactor, \
					 amax.multiplo AS maxmult, amin.multiplo AS minmult, 1 AS activo \
			FROM \
				(select a.producto, a.present, MAX(a.factor) AS maxfactor \
					from articulos a WHERE a.activo=1 and a.producto='%s'  \
						group by a.producto, a.present) amm \
			INNER JOIN articulos amax ON amm.producto=amax.producto AND amm.present=amax.present AND amm.maxfactor=amax.factor AND amax.activo=1 \
			INNER JOIN articulos amin ON amm.producto=amin.producto AND amm.present=amin.present AND amin.factor=1 AND amin.activo=1 \
			group by amm.producto, amm.present) \
		UNION ALL \
			(SELECT amm.producto, amm.present, amm.maxfactor, \
					 amax.multiplo AS maxmult, amin.multiplo AS minmult, 0 AS activo \
			FROM \
				(select a.producto, a.present, MAX(a.factor) AS maxfactor, MAX(a.activo) AS maxactivo \
					from articulos a WHERE a.producto='%s'  \
						group by a.producto, a.present \
						HAVING maxactivo=0 \
						) amm \
			INNER JOIN articulos amax ON amm.producto=amax.producto AND amm.present=amax.present AND amm.maxfactor=amax.factor \
			INNER JOIN articulos amin ON amm.producto=amin.producto AND amm.present=amin.present AND amin.factor=1 \
			group by amm.producto, amm.present)",
			clave_producto_nueva, clave_producto_nueva);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("update presentacionesminmax pmm \
			LEFT JOIN presentcajlogisfactor pclf ON pmm.producto=pclf.producto AND pmm.present=pclf.present \
			set pmm.cajalogisticafactor=COALESCE(pclf.cajalogisticafactor, pmm.maxfactor) \
			where pmm.producto = '%s'", clave_producto_nueva);
		instrucciones[num_instrucciones++] = instruccion;

		/*TABLAS SIN LLAVE FORANEA CON REFERENCIA A PRODUCTO*/
		for(int i = 0; i < respuesta_empresas->ObtieneNumRegistros(); i++){
			respuesta_empresas->IrAlRegistroNumero(i);
			AnsiString registroEmpresa = respuesta_empresas->ObtieneDato();

			instruccion.sprintf(
				"update precalculocostos%s set producto='%s' where producto='%s'"
				, registroEmpresa, clave_producto_nueva, clave_producto_original);
			instrucciones[num_instrucciones++] = instruccion;

			instruccion.sprintf(
				"update precalculocostospromedio%s set producto='%s' where producto='%s'", registroEmpresa, clave_producto_nueva,
				clave_producto_original);
			instrucciones[num_instrucciones++] = instruccion;

			instruccion.sprintf(
			"update precalculomensual%s set producto='%s' where producto='%s'"
				,registroEmpresa, clave_producto_nueva, clave_producto_original);
			instrucciones[num_instrucciones++] = instruccion;

			instruccion.sprintf("UPDATE precalculohistoriaconexist%s SET producto = '%s' where producto = '%s'"
				,registroEmpresa, clave_producto_nueva, clave_producto_original);
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("UPDATE precalculohistoriaexist%s SET producto = '%s' where producto = '%s'"
				,registroEmpresa, clave_producto_nueva, clave_producto_original);
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("UPDATE precalculominmax%s SET producto = '%s' where producto = '%s'"
				,registroEmpresa, clave_producto_nueva, clave_producto_original);
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("UPDATE precalculominmaxfin%s SET producto = '%s' where producto = '%s'"
				, registroEmpresa, clave_producto_nueva, clave_producto_original);
			instrucciones[num_instrucciones++]=instruccion;
		}

		instruccion.sprintf(
			"update precalculocostosventadet set producto='%s' where producto='%s'"
				,clave_producto_nueva, clave_producto_original);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf(
			"update precalculoventasdet set producto='%s' where producto='%s' "
				,clave_producto_nueva, clave_producto_original);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf(
			"update prodbloqueadocompra set producto='%s' where producto='%s'"
				,clave_producto_nueva, clave_producto_original);
		instrucciones[num_instrucciones++] = instruccion;

		/*Actualiza existenciasactuales*/
		instruccion.sprintf("UPDATE existenciasactuales SET producto = '%s' where producto = '%s'"
			,clave_producto_nueva, clave_producto_original);
		instrucciones[num_instrucciones++]=instruccion;

		/*TABLAS SIN LLAVE FORANEA*/

		/*Actualiza precalculocostosmovalmadet*/
		instruccion.sprintf("UPDATE precalculocostosmovalmadet SET producto = '%s' where producto = '%s'"
			,clave_producto_nueva, clave_producto_original);
		instrucciones[num_instrucciones++]=instruccion;

		/*Actualiza unifcostoexistencias*/
		instruccion.sprintf("UPDATE unifcostoexistencias SET producto = '%s' where producto = '%s'"
			,clave_producto_nueva, clave_producto_original);
		instrucciones[num_instrucciones++]=instruccion;

		/*Actualiza unifexist*/
		instruccion.sprintf("UPDATE unifexist SET producto = '%s' where producto = '%s'"
			,clave_producto_nueva, clave_producto_original);
		instrucciones[num_instrucciones++]=instruccion;

		/*Actualiza unifexistcodigos*/
		instruccion.sprintf("UPDATE unifexistcodigos SET producto = '%s' where producto = '%s'"
			,clave_producto_nueva, clave_producto_original);
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("INSERT INTO bitacoramodart \
		(articulo, productoant, productonuevo, tipo, usumodi, fechamodi, horamodi) \
		VALUES \
		('%s', '%s', '%s', 1, '%s', CURDATE(), CURTIME()) ",
		clave_articulo, clave_producto_original, clave_producto_nueva, usuario);
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("DELETE FROM camprogprodpresent where id = %s", idProgramado);
		instrucciones[num_instrucciones++]=instruccion;

		instrucciones[num_instrucciones++] = "COMMIT";
		instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=1";
		instrucciones[num_instrucciones++]="SET SESSION sql_log_bin = 1";

		instruccion.sprintf("INSERT INTO bitprodpresent (tipo, prod_origen, prod_destino, articulo, usuario, \
			fecha_prog, fecha_apli, suc_prog, mensaje, sucursal, correcto) VALUES ('MPRO','%s', '%s', '%s', \
			'%s', '%s', NOW(), '%s', 'Clave de producto cambiada correctamente.' , '%s', 1)",
			clave_producto_original, clave_producto_nueva, clave_articulo, usuario, fechaProgramada,
			sucursalProgramada, FormServidor->ObtieneClaveSucursal());
		instrucciones[num_instrucciones++]=instruccion;

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql = mFg.AgregaStringABuffer
			(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (int i = 0; i < num_instrucciones; i++)
			aux_buffer_sql = mFg.AgregaStringABuffer(instrucciones[i],
			aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta,
			TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,
			buffer_sql);

	}
	__finally {
		delete buffer_sql;
		delete respuesta_empresas;
	}
}
// ---------------------------------------------------------------------------
//ID_CON_CAT_MOVSCAJA
void ServidorCatalogos::ConsultaMovsCaja(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA TERMINO DE PAGO
	AnsiString instruccion;
	AnsiString clave;

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos de las cancelaciones
	instruccion.sprintf("SELECT clave, descripcion, activo, IF(concepto='D', 0, 1) as concepto FROM catalogomovscaja WHERE clave = '%s' ",
		clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

	// Obtiene todas las cancelaciones
	instruccion =
		"SELECT clave, descripcion, activo, concepto FROM catalogomovscaja ORDER BY descripcion ";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------
// ID_BAJ_CAT_MOVSCAJA
void ServidorCatalogos::BajaMovsCaja(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*10];
	char *aux_buffer_sql=buffer_sql;
	BufferRespuestas* resp=NULL;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[10];

	AnsiString instruccion;
	AnsiString clave;

	clave = mFg.ExtraeStringDeBuffer(&parametros);

	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	try {
		Instruccion.sprintf(" DELETE FROM catalogomovscaja \
		WHERE clave = '%s' ", clave);
		instrucciones[num_instrucciones++]=Instruccion;

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
// ID_GRAB_CAT_MOVSCAJA
void ServidorCatalogos::GrabaMovsCaja(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*10];
	char *aux_buffer_sql=buffer_sql;
	BufferRespuestas* resp=NULL;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[10];

	AnsiString instruccion;
	AnsiString clave, descripcion, concepto, tarea;
	int estatus;

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	descripcion = mFg.ExtraeStringDeBuffer(&parametros);
	estatus = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
	concepto = mFg.ExtraeStringDeBuffer(&parametros);
	tarea = mFg.ExtraeStringDeBuffer(&parametros);

	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	try {
		if (tarea=="M") {
			Instruccion.sprintf(" UPDATE catalogomovscaja SET descripcion = '%s', activo = %d  WHERE clave = '%s' ",
			descripcion, estatus, clave);
			instrucciones[num_instrucciones++]=Instruccion;
		} else if (tarea=="A") {
			Instruccion.sprintf(" INSERT INTO catalogomovscaja (clave, descripcion, activo, concepto) VALUES ('%s', '%s', %d, '%s') ",
			clave, descripcion, estatus, concepto);
			instrucciones[num_instrucciones++]=Instruccion;
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
// ---------------------------------------------------------------------------
// ID_CON_PINPAD
void ServidorCatalogos::ConsultaPinpad(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
	AnsiString clave_terminal;

	clave_terminal = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos de la terminal
	instruccion.sprintf("SELECT tp.terminal, tp.product, tp.pn, tp.sn, \
	tp.fcc_id, tp.ic, tp.activo, AES_DECRYPT(tp.usuario,'AccesTRNLaVioleta') AS usuario, \
	AES_DECRYPT(tp.password,'AccesTRNLaVioleta') AS password, tipomoneda, permitecancel, \
    permitedevolu, permiteajuste \
	FROM terminalesxpinpad tp WHERE tp.terminal = '%s'",
	clave_terminal);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------
// ID_GRA_PINPAD
void ServidorCatalogos::GrabaPinpad(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*10];
	char *aux_buffer_sql=buffer_sql;
	BufferRespuestas* resp=NULL;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[10];

	AnsiString tarea, terminal, pinpadmodelo, pinpadpn, pinpadsn, pinpadfccid;
	AnsiString pinpadic, pinpadusu, pinpadpwd, terminalact;
	int pinpadactivo, permitecancel, permitedevol, permiteajuste;
    AnsiString tipomoneda;

	tarea = mFg.ExtraeStringDeBuffer(&parametros);
	terminal = mFg.ExtraeStringDeBuffer(&parametros);
	pinpadmodelo = mFg.ExtraeStringDeBuffer(&parametros);
	pinpadpn = mFg.ExtraeStringDeBuffer(&parametros);
	pinpadsn = mFg.ExtraeStringDeBuffer(&parametros);
	pinpadfccid = mFg.ExtraeStringDeBuffer(&parametros);
	pinpadic = mFg.ExtraeStringDeBuffer(&parametros);
	pinpadusu = mFg.ExtraeStringDeBuffer(&parametros);
	pinpadpwd = mFg.ExtraeStringDeBuffer(&parametros);
	pinpadactivo = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
	terminalact = mFg.ExtraeStringDeBuffer(&parametros);
	tipomoneda = mFg.ExtraeStringDeBuffer(&parametros);
	permitecancel = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
	permitedevol = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
	permiteajuste = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));

	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	try {
		if (tarea=="M") {
			Instruccion.sprintf(" UPDATE terminalesxpinpad SET product = '%s', pn = '%s', \
			sn = '%s', fcc_id = '%s', ic = '%s', activo = %d, terminal = '%s', tipomoneda= '%s', permitecancel= %d, \
            permitedevolu = %d, permiteajuste = %d, \
			usuario = AES_ENCRYPT('%s','AccesTRNLaVioleta'), password = AES_ENCRYPT('%s','AccesTRNLaVioleta') \
			WHERE terminal = '%s' ",
			pinpadmodelo, pinpadpn, pinpadsn, pinpadfccid, pinpadic, pinpadactivo, terminal, tipomoneda,
			permitecancel, permitedevol, permiteajuste, pinpadusu, pinpadpwd, terminalact);
			instrucciones[num_instrucciones++]=Instruccion;
		} else if (tarea=="A") {
			Instruccion.sprintf(" INSERT INTO terminalesxpinpad \
			(terminal, product, pn, sn, fcc_id, ic, activo, usuario, password, tipomoneda, permitecancel, permitedevolu, permiteajuste) VALUES \
			('%s', '%s', '%s', '%s', '%s', '%s', %d, \
			AES_ENCRYPT('%s','AccesTRNLaVioleta'), AES_ENCRYPT('%s','AccesTRNLaVioleta'),'%s', %d, %d, %d) ",
			terminal, pinpadmodelo, pinpadpn, pinpadsn, pinpadfccid, pinpadic, pinpadactivo, pinpadusu, pinpadpwd,
			tipomoneda, permitecancel, permitedevol, permiteajuste);
			instrucciones[num_instrucciones++]=Instruccion;
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
// ---------------------------------------------------------------------------
// ID_BAJ_PINPAD
void ServidorCatalogos::BajaPinpad(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// BAJA PINPAD
	BajaGenerico(Respuesta, MySQL, parametros, "terminalesxpinpad", "terminal");
}
// ---------------------------------------------------------------------------

// ASEGURADORAS

// ---------------------------------------------------------------------------
// ID_GRA_ASEGURADORA
void ServidorCatalogos::GrabaAseguradora(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// GRABA VIAS DE EMBARQUE
	GrabaGenerico(Respuesta, MySQL, parametros, "catalogoaseguradoras", "clave");
}
// ---------------------------------------------------------------------------
// ID_BAJ_ASEGURADORA
void ServidorCatalogos::BajaAseguradora(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// BAJA VIAS DE EMBARQUE
	BajaGenerico(Respuesta, MySQL, parametros, "catalogoaseguradoras", "clave");
}
// ---------------------------------------------------------------------------
// ID_CON_ASEGURADORA
void ServidorCatalogos::ConsultaAseguradora(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA VIAS DE EMBARQUE
	AnsiString instruccion;
	AnsiString clave;
	AnsiString activos;
	AnsiString condicion_activos=" ";

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	activos=mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	if (activos=="1") {
		condicion_activos.sprintf(" WHERE activa = 1 ");
	}

	// Obtiene todos los datos de la via de embarque
	instruccion.sprintf("select * from catalogoaseguradoras where clave='%s' ",
		clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todas las vias de embarque
	instruccion.sprintf (
		"select clave, nombre, activa from catalogoaseguradoras %s ", condicion_activos);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

}
// ---------------------------------------------------------------------------
// ID_GRA_REMOLQUE
void ServidorCatalogos::GrabaRemolque(RespuestaServidor *Respuesta,	MYSQL *MySQL, char *parametros) {
	char *buffer_sql=new char[1024*10];
	char *aux_buffer_sql=buffer_sql;
	BufferRespuestas* resp=NULL;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[10];

	AnsiString idRemolque="",tarea="A",numSerie="",numPlaca="",cvetiprem="",peso="0.0";
	AnsiString volumen="0.0",senas="",activo="1",suc="",usuario="";
	AnsiString fecha="",hora="";

	try {
		tarea=mFg.ExtraeStringDeBuffer(&parametros);
		idRemolque=mFg.ExtraeStringDeBuffer(&parametros);
		numSerie=mFg.ExtraeStringDeBuffer(&parametros);
		numPlaca=mFg.ExtraeStringDeBuffer(&parametros);
		cvetiprem=mFg.ExtraeStringDeBuffer(&parametros);
		peso=mFg.ExtraeStringDeBuffer(&parametros);
		volumen=mFg.ExtraeStringDeBuffer(&parametros);
		senas=mFg.ExtraeStringDeBuffer(&parametros);
		activo=mFg.ExtraeStringDeBuffer(&parametros);
		suc=mFg.ExtraeStringDeBuffer(&parametros);
		usuario=mFg.ExtraeStringDeBuffer(&parametros);
		fecha=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		hora= mFg.ExtraeStringDeBuffer(&parametros);

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		if (tarea=="M") {
			Instruccion.sprintf("UPDATE catalogoremolques SET numserie='%s',numeroplaca='%s',ccvesubtiporem=%s,pesolimite=%s, \
			volumenlimite=%s,senasparticulares='%s',activo=%s,usumod='%s',fechamod='%s',horamod='%s' WHERE idremolque=%s",
			numSerie,numPlaca,cvetiprem,peso,volumen,senas,activo,usuario,fecha,hora,idRemolque);
			instrucciones[num_instrucciones++]=Instruccion;
		} else if (tarea=="A") {
			Instruccion.sprintf("INSERT INTO catalogoremolques (numserie,numeroplaca,ccvesubtiporem,pesolimite,volumenlimite, \
			senasparticulares,activo,sucasignada,usualta,usumod,fechaalta,horaalta,fechamod,horamod) \
			VALUES('%s','%s',%s,%s,%s,'%s',%s,'%s','%s','%s','%s','%s','%s','%s')",
			numSerie,numPlaca,cvetiprem,peso,volumen,senas,activo,suc,usuario,usuario,fecha,hora,fecha,hora);
			instrucciones[num_instrucciones++]=Instruccion;
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

// ---------------------------------------------------------------------------
// ID_BAJ_REMOLQUE
void ServidorCatalogos::BajaRemolque(RespuestaServidor *Respuesta,MYSQL *MySQL, char *parametros) {
	// BAJA REMOLQUES
	BajaGenerico(Respuesta, MySQL, parametros, "catalogoremolques", "idremolque");
}
// ---------------------------------------------------------------------------
// ID_CON_REMOLQUE
void ServidorCatalogos::ConsultaRemolque(RespuestaServidor *Respuesta,	MYSQL *MySQL, char *parametros) {
	// CONSULTA REMOLQUES
	AnsiString instruccion;
	AnsiString idRemolque;

	idRemolque = mFg.ExtraeStringDeBuffer(&parametros);

	if (idRemolque==" ") {
		idRemolque="0";
	}

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos de un remolque especifico
	instruccion.sprintf("SELECT cr.idremolque,cr.numeroplaca,cr.ccvesubtiporem,ifnull(cr.numserie,'') as numserie,cr.pesolimite,    \
						cr.volumenlimite,ifnull(cr.senasparticulares,'') as senasparticulares,cr.activo,cr.sucasignada,cr.fechaalta,cr.horaalta,cr.fechamod,cr.horamod,cr.usualta,cr.usumod  \
						FROM catalogoremolques cr   \
						where idremolque=%s ",idRemolque);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
	Respuesta->TamBufferResultado);

	// Obtiene todos los remolques
	instruccion="SELECT cr.idremolque,cr.numeroplaca,cr.ccvesubtiporem,CONCAT(cstr.cvesubtiprem,' - ',cstr.remolque) AS ntiporem,ifnull(cr.numserie,'')  as numserie,cr.pesolimite, ";
	instruccion+="cr.volumenlimite,ifnull(cr.senasparticulares,'') as senasparticulares,cr.activo,cr.sucasignada,cr.fechaalta,cr.horaalta,cr.fechamod,cr.horamod,cr.usualta,cr.usumod ";
	instruccion+="FROM catalogoremolques cr ";
	instruccion+="INNER JOIN ccpsubtiporem cstr ON cstr.idclavesubtiprem=cr.ccvesubtiporem";

	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
	Respuesta->TamBufferResultado);

}
//---------------------------------------------------------------------------
// ID_ACTU_STATUS_PRODUCTOS
void ServidorCatalogos::EjecutaActuEstatusProductos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*30];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[10];
	AnsiString articulos;
	int estatus = 0;

	estatus = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
	articulos = mFg.ExtraeStringDeBuffer(&parametros);

	//iniciar la transaccion
	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	try{

		Instruccion.sprintf("UPDATE ecommerceproductos ep \
		SET ep.activo=%d WHERE ep.articulo IN(%s)",
		estatus,
		articulos);
		instrucciones[num_instrucciones++]=Instruccion;

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
// ---------------------------------------------------------------------------
// ID_CON_INFQR_TICKET
void ServidorCatalogos::ConsultaInfoQRTickets(RespuestaServidor *Respuesta,	MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
	AnsiString clave;

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos de las cancelaciones
	instruccion.sprintf("SELECT * FROM ticketsqr WHERE sucursal = '%s' ",
		clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

	// Obtiene todos los datos de un remolque especifico
	instruccion.sprintf("SELECT sucursal, valorqr, nombreimg, IF(tipoimpr=1,'Con Imagen', 'Con Texto') AS tipoimpr FROM ticketsqr ORDER BY sucursal");
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
	Respuesta->TamBufferResultado);

	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
	Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------
// ID_GRA_INFQR_TICKET
void ServidorCatalogos::GrabaInfoQRTickets(RespuestaServidor *Respuesta,	MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*1024];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[15];
	AnsiString sucursal, infoqr, frase1, frase2, frase3, nombreImg, imagen, tipo;
    int tipoImpr;

	sucursal = mFg.ExtraeStringDeBuffer(&parametros);
	tipo = mFg.ExtraeStringDeBuffer(&parametros);
	infoqr = mFg.ExtraeStringDeBuffer(&parametros);
	frase1 = mFg.ExtraeStringDeBuffer(&parametros);
	frase2 = mFg.ExtraeStringDeBuffer(&parametros);
	frase3 = mFg.ExtraeStringDeBuffer(&parametros);
	nombreImg = mFg.ExtraeStringDeBuffer(&parametros);
	imagen = mFg.ExtraeStringDeBuffer(&parametros);
	tipoImpr = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));

	if (infoqr=="") {
        infoqr="NULL";
	}

	//iniciar la transaccion
	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";
	try{

		if (tipo=="A") {
			if (tipoImpr==1) {
				Instruccion.sprintf("INSERT INTO ticketsqr (sucursal, valorqr, nombreimg, imagen, frase1, frase2, frase3, tipoimpr) \
				VALUES ('%s', '%s', '%s', '%s', NULL, NULL, NULL, %d)",
				sucursal, infoqr, nombreImg, imagen, tipoImpr);
				instrucciones[num_instrucciones++]=Instruccion;
			} else {
				Instruccion.sprintf("INSERT INTO ticketsqr (sucursal, valorqr, nombreimg, imagen, frase1, frase2, frase3, tipoimpr) \
				VALUES ('%s', '%s', NULL, NULL, '%s', '%s', '%s', %d)",
				sucursal, infoqr, frase1, frase2, frase3, tipoImpr);
				instrucciones[num_instrucciones++]=Instruccion;
			}
		} else if (tipo=="M") {
			if (tipoImpr==1) {
				Instruccion.sprintf("UPDATE ticketsqr SET sucursal = '%s', valorqr = '%s', nombreimg = '%s', \
				imagen = '%s', frase1 = NULL, frase2 = NULL, frase3 = NULL, tipoimpr = %d WHERE sucursal = '%s' ",
				sucursal, infoqr, nombreImg, imagen, tipoImpr, sucursal);
				instrucciones[num_instrucciones++]=Instruccion;
			} else {
				Instruccion.sprintf("UPDATE ticketsqr SET sucursal = '%s', valorqr = '%s', nombreimg = NULL, \
				imagen = NULL, frase1 = '%s', frase2 = '%s', frase3 = '%s', tipoimpr = %d WHERE sucursal = '%s' ",
				sucursal, infoqr, frase1, frase2, frase3, tipoImpr, sucursal);
				instrucciones[num_instrucciones++]=Instruccion;
            }
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
// ---------------------------------------------------------------------------
// ID_BAJ_INFQR_TICKET
void ServidorCatalogos::BajaInfoQRTickets(RespuestaServidor *Respuesta,	MYSQL *MySQL, char *parametros)
{
	// BAJA VIAS DE EMBARQUE
	BajaGenerico(Respuesta, MySQL, parametros, "ticketsqr", "sucursal");

}
// ---------------------------------------------------------------------------
//ID_CON_MOTIVOCANCEL
void ServidorCatalogos::ConsultaMotivoCancelaciones(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA TERMINO DE PAGO
	AnsiString instruccion;
	AnsiString clave;

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos de las cancelaciones
	instruccion.sprintf("SELECT * FROM ccancmotivo WHERE motivo = %s ",
		clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

	// Obtiene todas las cancelaciones
	instruccion =
		"SELECT motivo, descripcion, reqsustitucion FROM ccancmotivo ORDER BY motivo ";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------
// ID_BAJ_MOTIVOCANCEL
void ServidorCatalogos::BajaMotivoCancelaciones(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// BAJA TERMINO PAGO
	BajaGenerico(Respuesta, MySQL, parametros, "ccancmotivo", "motivo");
}

// ---------------------------------------------------------------------------
// ID_GRABA_MOTIVOCANCEL
void ServidorCatalogos::GrabaMotivoCancelaciones(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// GRABA TERMINO DE PAGO
	GrabaGenerico(Respuesta, MySQL, parametros, "ccancmotivo", "motivo");
}
// ---------------------------------------------------------------------------
// ID_GRA_BITA_ECOMMERCE
void ServidorCatalogos::GrabaBitacoraEcommerce(RespuestaServidor *Respuesta,	MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*30];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[10];
	AnsiString statuscode, mensajeserver, articulo, precio, costo, exist, usuario, terminal, url;

	statuscode = mFg.ExtraeStringDeBuffer(&parametros);
	mensajeserver = mFg.ExtraeStringDeBuffer(&parametros);
	articulo = mFg.ExtraeStringDeBuffer(&parametros);
	precio = mFg.ExtraeStringDeBuffer(&parametros);
	costo = mFg.ExtraeStringDeBuffer(&parametros);
	exist = mFg.ExtraeStringDeBuffer(&parametros);
	usuario = mFg.ExtraeStringDeBuffer(&parametros);
	terminal = mFg.ExtraeStringDeBuffer(&parametros);
	url = mFg.ExtraeStringDeBuffer(&parametros);

	//iniciar la transaccion
	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	try{

		if (articulo=="NULL") {
			Instruccion.sprintf("INSERT IGNORE bitacoraecommerceproducts \
			(status_code, mensage_server, articulo, \
			precio, costo, exist, \
			usuario, fecha, hora, terminal, url) \
			VALUES \
			(%s, '%s', NULL, \
			NULL, NULL, NULL, \
			'%s', CURDATE(), CURTIME(), '%s', '%s')",
			statuscode, mensajeserver,
			usuario, terminal, url);
			instrucciones[num_instrucciones++]=Instruccion;
		} else {
			Instruccion.sprintf("INSERT IGNORE bitacoraecommerceproducts \
			(status_code, mensage_server, articulo, \
			precio, costo, exist, \
			usuario, fecha, hora, terminal, url) \
			VALUES \
			(%s, '%s', '%s', \
			%s, %s, %s, \
			'%s', CURDATE(), CURTIME(), '%s', '%s')",
			statuscode, mensajeserver, articulo,
			precio, costo, exist,
			usuario, terminal, url);
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
// ---------------------------------------------------------------------------
// ID_UPDT_PREC_DIFERID
void ServidorCatalogos::ActuaPrecioMod(RespuestaServidor *Respuesta,	MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*2000];
	char *aux_buffer_sql=buffer_sql;
	int i;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[20];
	AnsiString redondearprecio;
	AnsiString campo_preciomod;
	AnsiString archivo_temp1;
	char *resultado;

	AnsiString lista;
	AnsiString inner_join_tags = " ";
	AnsiString condicion_tags = " ";

	AnsiString desde1, hasta1, redondeo1;
	AnsiString desde2, hasta2, redondeo2;
	AnsiString desde3, hasta3, redondeo3;
	AnsiString desde4, hasta4, redondeo4;
	AnsiString idEmpresa;

	AnsiString tipoutilidad, productos, precio, campo_tipoutilidad;
	AnsiString inner_tipoutilidad=" ";

	redondearprecio= mFg.ExtraeStringDeBuffer(&parametros);
	desde1= mFg.ExtraeStringDeBuffer(&parametros);
	hasta1= mFg.ExtraeStringDeBuffer(&parametros);
	redondeo1= mFg.ExtraeStringDeBuffer(&parametros);
	desde2= mFg.ExtraeStringDeBuffer(&parametros);
	hasta2= mFg.ExtraeStringDeBuffer(&parametros);
	redondeo2= mFg.ExtraeStringDeBuffer(&parametros);
	desde3= mFg.ExtraeStringDeBuffer(&parametros);
	hasta3= mFg.ExtraeStringDeBuffer(&parametros);
	redondeo3= mFg.ExtraeStringDeBuffer(&parametros);
	desde4= mFg.ExtraeStringDeBuffer(&parametros);
	hasta4= mFg.ExtraeStringDeBuffer(&parametros);
	redondeo4= mFg.ExtraeStringDeBuffer(&parametros);
	tipoutilidad= mFg.ExtraeStringDeBuffer(&parametros);
	productos= mFg.ExtraeStringDeBuffer(&parametros);
	idEmpresa= mFg.ExtraeStringDeBuffer(&parametros);

	if (tipoutilidad!=" ") {
		inner_tipoutilidad="INNER JOIN tmputilidadesprecios apre ON apre.articulo = precios.articulo AND apre.tipoprec = precios.tipoprec";
		precio="apre.precioutil";
		if (tipoutilidad=="1") {
			campo_tipoutilidad="tpre.porcutil";
		} else if (tipoutilidad=="2") {
			campo_tipoutilidad="tpre.porcutil2";
		} else if (tipoutilidad=="3") {
			campo_tipoutilidad="tpre.porcutil3";
		} else if (tipoutilidad=="4") {
			campo_tipoutilidad="tpre.porcutil4";
		} else if (tipoutilidad=="5") {
			campo_tipoutilidad="tpre.porcutil5";
		}
	} else {
		precio="precios.precioproximo";
    }

	if (redondearprecio=="1") {

		campo_preciomod.sprintf("(CASE WHEN MOD(%s,1) = 0.0 THEN %s ELSE \
		CASE WHEN MOD(%s,1) >=%s AND MOD(%s,1) <=%s THEN ((%s-MOD(%s,1))+ TRUNCATE(%s,2)) ELSE \
		CASE WHEN MOD(%s,1) >=%s AND MOD(%s,1) <=%s THEN ((%s-MOD(%s,1))+ TRUNCATE(%s,2)) ELSE \
		CASE WHEN MOD(%s,1) >=%s AND MOD(%s,1) <=%s THEN ((%s-MOD(%s,1))+ TRUNCATE(%s,2)) ELSE \
		CASE WHEN MOD(%s,1) >=%s AND MOD(%s,1) <=%s THEN ((%s-MOD(%s,1))+ TRUNCATE(%s,2)) END END END END END)",
		precio, precio,
		precio, desde1, precio, hasta1, redondeo1, precio, precio,
		precio, desde2, precio, hasta2, redondeo2, precio, precio,
		precio, desde3, precio, hasta3, redondeo3, precio, precio,
		precio, desde4, precio, hasta4, redondeo4, precio, precio);
	} else {
		campo_preciomod.sprintf("%s",precio);
	}

	//iniciar la transaccion
	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	try {
		/////Tabla temporal, la cual contendrá los valores mínimos y maximos de los artículos vendidos
		instruccion="CREATE TEMPORARY TABLE tmppreciosmod ( \
		articulo VARCHAR(9), \
		producto VARCHAR(8), \
		present VARCHAR(255), \
		multiplo VARCHAR(10), \
		tipoprec VARCHAR(2), \
		preciomod DECIMAL(16,6), \
		INDEX(articulo), INDEX(tipoprec) \
		)ENGINE=InnoDB";
		instrucciones[num_instrucciones++]=instruccion;

        if (tipoutilidad!=" ") {

			BufferRespuestas* res_digitosredondeo=NULL;
			AnsiString redondeo;
			try{
				instruccion.sprintf("SELECT valor FROM parametrosemp where parametro = 'DIGREDOND' AND sucursal = '%s' ", idEmpresa);
				mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), res_digitosredondeo);
				redondeo = res_digitosredondeo->ObtieneDato("valor");
			} __finally {
				if (res_digitosredondeo!=NULL) delete res_digitosredondeo;
			}

			/////Tabla temporal para guardar los precios calculados en base al porcentaje de utilidad
			instruccion="CREATE TEMPORARY TABLE tmputilidadesprecios ( \
			articulo VARCHAR(9), \
			producto VARCHAR(8), \
			present VARCHAR(255), \
			tipoprec VARCHAR(2), \
			precioutil DECIMAL(16,6), INDEX(articulo), INDEX(tipoprec) \
			)ENGINE=InnoDB";
			instrucciones[num_instrucciones++]=instruccion;

			/////se agregan los articulos con precio diferente y que sean precio 03  que viene el en parametro tipo_precio
			instruccion.sprintf("INSERT INTO tmputilidadesprecios \
			SELECT \
				a.articulo, \
				a.producto, \
				a.present, \
				pre.tipoprec, \
				ROUND(ROUND((1+%s/100)*costo, %s)*a.factor, %s) AS precioutil \
			FROM articulos a \
			INNER JOIN precios pre ON pre.articulo = a.articulo \
			INNER JOIN tiposdeprecios tpre ON tpre.tipoprec = pre.tipoprec AND tpre.idempresa=%s \
			WHERE (a.producto, a.present) IN (%s) ",
			campo_tipoutilidad, redondeo, redondeo,
			idEmpresa, productos);
			instrucciones[num_instrucciones++]=instruccion;
		}

		archivo_temp1 = mServidorVioleta->ObtieneArchivoTemp(num_instrucciones,Respuesta->Id);

		/////se agregan los artículos a una tabla temporal para actualizar el campo preciomod de la tabla precios
		instruccion.sprintf("SELECT a.articulo, a.producto, a.present, a.multiplo, precios.tipoprec, %s AS preciomod \
		FROM precios \
		%s \
		INNER JOIN articulos a ON precios.articulo=a.articulo \
		INNER JOIN tiposdeprecios tp ON tp.tipoprec=precios.tipoprec AND tp.idempresa=%s \
		WHERE a.activo=1 AND actualizarpendiente=0 \
        AND (a.producto) IN (%s) \
		INTO OUTFILE '%s' ",
		campo_preciomod, inner_tipoutilidad, idEmpresa, productos, archivo_temp1);
		instrucciones[num_instrucciones++]=instruccion;

		/////se agregan los artículos a una tabla temporal para actualizar el campo preciomod de la tabla precios
		instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE tmppreciosmod ",
		archivo_temp1);
		instrucciones[num_instrucciones++]=instruccion;

		/////Se actualizan el campo preciomod
		instruccion.sprintf("UPDATE precios p INNER JOIN tmppreciosmod a ON a.articulo = p.articulo AND a.tipoprec = p.tipoprec \
		INNER JOIN tiposdeprecios tp ON tp.tipoprec=p.tipoprec AND tp.idempresa=%s \
		SET p.preciomod = a.preciomod", idEmpresa);
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
		mServidorVioleta->BorraArchivoTemp(archivo_temp1);
	}

}
// ---------------------------------------------------------------------------
// ID_GRA_PRODUCTO_IESPS
void ServidorCatalogos::GrabaProductoSinIESPS(RespuestaServidor *Respuesta,	MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*64];
	char *aux_buffer_sql=buffer_sql;
	int i;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[10];
	char *resultado;

	AnsiString producto;

	producto= mFg.ExtraeStringDeBuffer(&parametros);

	//iniciar la transaccion
	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	try {

		/////se agregan los artículos a una tabla temporal para actualizar el campo preciomod de la tabla precios
		instruccion.sprintf("INSERT INTO excluirproductosiesps \
		(producto) VALUES ('%s')",
		producto);
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
// ID_CON_PRODUCTO_IESPS
void ServidorCatalogos::ConsultaProductoSinIESPS(RespuestaServidor *Respuesta,	MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todas las cancelaciones
	instruccion = "SELECT \
	a.articulo, \
	pro.nombre, \
	a.producto, \
	a.present, \
	a.multiplo, \
	a.factor \
	FROM excluirproductosiesps ex \
	INNER JOIN articulos a ON a.producto = ex.producto \
	INNER JOIN productos pro ON pro.producto = a.producto \
	INNER JOIN presentaciones pre ON pre.producto = pro.producto AND pre.present = a.present \
	ORDER BY pro.nombre, a.producto, a.present, a.multiplo";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------
// ID_BAJA_PRODUCTO_IESPS
void ServidorCatalogos::BajaProductoSinIESPS(RespuestaServidor *Respuesta,	MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*64];
	char *aux_buffer_sql=buffer_sql;
	int i;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[10];
	char *resultado;

	AnsiString producto;

	producto= mFg.ExtraeStringDeBuffer(&parametros);

	//iniciar la transaccion
	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	try {

		/////se agregan los artículos a una tabla temporal para actualizar el campo preciomod de la tabla precios
		instruccion.sprintf("DELETE FROM excluirproductosiesps WHERE producto = '%s'",
		producto);
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
// ID_CON_SOCIEDAD_MERCANTIL
void ServidorCatalogos::ConsultaSociedadMercantil(RespuestaServidor *Respuesta,	MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
	AnsiString id;

	id = mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos de las cancelaciones
	instruccion.sprintf("SELECT id, nombre, descripcion, activo \
	FROM catsociedadesmercantiles \
	WHERE id = %s \
	ORDER BY id ASC ",
	id);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene todas las cancelaciones
	instruccion = "SELECT id, nombre, descripcion, activo \
	FROM catsociedadesmercantiles \
	ORDER BY id ASC";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------
// ID_GRA_SOCIEDAD_MERCANTIL
void ServidorCatalogos::GrabaSociedadMercantil(RespuestaServidor *Respuesta,	MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*30];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i;
	AnsiString instruccion, instrucciones[15];
	AnsiString tipo, nombre, descripcion, estatus, id;

	tipo = mFg.ExtraeStringDeBuffer(&parametros);
	nombre = mFg.ExtraeStringDeBuffer(&parametros);
	descripcion = mFg.ExtraeStringDeBuffer(&parametros);
	estatus = mFg.ExtraeStringDeBuffer(&parametros);
	id = mFg.ExtraeStringDeBuffer(&parametros);

	//iniciar la transaccion
	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	try{

		if (tipo=="A") {

			instruccion.sprintf("INSERT INTO catsociedadesmercantiles (nombre, descripcion, activo) \
			VALUES ('%s', '%s', %s)",
			nombre, descripcion, estatus);
			instrucciones[num_instrucciones++]=instruccion;

		} else if (tipo=="M") {

			instruccion.sprintf("UPDATE catsociedadesmercantiles SET nombre = '%s', descripcion = '%s', activo = %s \
			WHERE id = %s ",
			nombre, descripcion, estatus, id);
			instrucciones[num_instrucciones++]=instruccion;

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
// ---------------------------------------------------------------------------
// ID_BAJ_SOCIEDAD_MERCANTIL
void ServidorCatalogos::BajaSociedadMercantil(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// BAJA TERMINO PAGO
	BajaGenerico(Respuesta, MySQL, parametros, "catsociedadesmercantiles", "id");
}
// ---------------------------------------------------------------------------
// ID_BAJ_SEGMENTO_CONT_ADI
void ServidorCatalogos::BajaSegmentoContAdi(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// BAJA SEGMENTO
	BajaGenerico(Respuesta, MySQL, parametros, "segmentosacontadi", "segmento");
}

// ---------------------------------------------------------------------------
// ID_CON_SEGMENTO_CONT_ADI
void ServidorCatalogos::ConsultaSegmentoContAdi(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA SEGMENTO
	AnsiString instruccion;
	AnsiString clave;

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos de Fabricante
	instruccion.sprintf("select * from segmentosacontadi where segmento='%s'", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todo Fabricante
	instruccion =
		"select segmento AS Segmento, nombre AS Nombre from segmentosacontadi order by segmento";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------
// ID_GRA_SEGMENTO_CONT_ADI
void ServidorCatalogos::GrabaSegmentoContAdi(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// GRABA SEGMENTO
	GrabaGenerico(Respuesta, MySQL, parametros, "segmentosacontadi", "segmento");
}
// ---------------------------------------------------------------------------

// ROLES

// ---------------------------------------------------------------------------
// ID_GRA_ROL
void ServidorCatalogos::GrabaRol(RespuestaServidor *Respuesta, MYSQL *MySQL,
	char *parametros) {
	// GRABA ROL
	GrabaGenerico(Respuesta, MySQL, parametros, "rolessistema", "claverol");
}

// ---------------------------------------------------------------------------
// ID_BAJ_ROL
void ServidorCatalogos::BajaRol(RespuestaServidor *Respuesta, MYSQL *MySQL,
	char *parametros) {
	// BAJA ROL
	BajaGenerico(Respuesta, MySQL, parametros, "rolessistema", "claverol");
}

// ---------------------------------------------------------------------------
// ID_CON_ROL
void ServidorCatalogos::ConsultaRol(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA ROL
	AnsiString instruccion;
	AnsiString clave_rol;

	clave_rol = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos del rol
	instruccion.sprintf("select * from rolessistema where claverol='%s'",
		clave_rol);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los roles
	instruccion =
		"select claverol AS Rol, nombre AS Nombre from rolessistema order by nombre";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------
// ID_CAN_PREPAGO
void ServidorCatalogos::EjecutaCancPrepago(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql = new char[1024 * 30];
	char *aux_buffer_sql = buffer_sql;
	int num_instrucciones = 0, i;
	AnsiString instruccion, instrucciones[15];
	AnsiString referencia, razoncancela, terminal, usuario, cliente, valor;

	referencia = mFg.ExtraeStringDeBuffer(&parametros);
	razoncancela = mFg.ExtraeStringDeBuffer(&parametros);
	terminal = mFg.ExtraeStringDeBuffer(&parametros);
	usuario = mFg.ExtraeStringDeBuffer(&parametros);
	cliente = mFg.ExtraeStringDeBuffer(&parametros);
	valor = mFg.ExtraeStringDeBuffer(&parametros);

	// iniciar la transaccion
	instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++] = "START TRANSACTION";

	try {

		instruccion.sprintf
			("UPDATE prepagoscli SET cancelado = 1, motivocancela = %s, usumodi = '%s' WHERE pago = '%s' ",
			razoncancela, usuario, referencia);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf
			("INSERT INTO bitacoraprepagos \
			(referencia, cliente, valor, tipo, estatus, fecha, hora, terminal, usuario) \
			VALUES \
			('%s','%s',%s,'AGENT','C',CURDATE(),CURTIME(),'%s','%s') ",
			referencia,cliente,valor,terminal, usuario);
		instrucciones[num_instrucciones++] = instruccion;

		instrucciones[num_instrucciones++] = "COMMIT";

		aux_buffer_sql = mFg.AgregaStringABuffer
			(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i = 0; i < num_instrucciones; i++)
			aux_buffer_sql = mFg.AgregaStringABuffer(instrucciones[i],
			aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta,
			TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,
			buffer_sql)) {
			instruccion.sprintf
				("SELECT token FROM tokensnotificaciones WHERE terminal = '%s' "
				, terminal);
			mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,
				instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	}
	__finally {
		if (buffer_sql != NULL)
			delete buffer_sql;
	}
}
// ---------------------------------------------------------------------------
// ID_GUARDA_NOTIFICACION
void ServidorCatalogos::GuardaNotificacion(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql = new char[1024 * 30];
	char *aux_buffer_sql = buffer_sql;
	int num_instrucciones = 0, i;
	AnsiString instruccion, instrucciones[15];
	AnsiString terminal, titulo, detalle, tipo, referencia, estatus, tipoapp;

	terminal = mFg.ExtraeStringDeBuffer(&parametros);
	titulo = mFg.ExtraeStringDeBuffer(&parametros);
	detalle = mFg.ExtraeStringDeBuffer(&parametros);
	tipo = mFg.ExtraeStringDeBuffer(&parametros);
	referencia = mFg.ExtraeStringDeBuffer(&parametros);
	estatus = mFg.ExtraeStringDeBuffer(&parametros);
	tipoapp = mFg.ExtraeStringDeBuffer(&parametros);

	// iniciar la transaccion
	instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++] = "START TRANSACTION";

	try {
		if (estatus=="1") {
			instruccion.sprintf("INSERT INTO notificacionesapp \
			(referencia, terminal, titulo, detalle, tipo, estatus, tipoapp, fecha, hora) \
			VALUES \
			('%s', '%s', '%s', '%s', '%s', '%s', '%s', CURDATE(), CURTIME()) ",
			referencia, terminal, titulo, detalle, tipo, estatus, tipoapp);
			instrucciones[num_instrucciones++] = instruccion;
		} else {
			instruccion.sprintf("INSERT INTO notificacionesapp \
			(referencia, terminal, titulo, detalle, tipo, estatus, tipoapp, fecha, hora) \
			VALUES \
			('%s', '%s', '%s', '%s', '%s', '%s', '%s', CURDATE(), CURTIME()) ",
			referencia, terminal, titulo, detalle, tipo, estatus, tipoapp);
			instrucciones[num_instrucciones++] = instruccion;
		}

		instrucciones[num_instrucciones++] = "COMMIT";

		aux_buffer_sql = mFg.AgregaStringABuffer
			(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i = 0; i < num_instrucciones; i++)
			aux_buffer_sql = mFg.AgregaStringABuffer(instrucciones[i],
			aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);
	}
	__finally {
		if (buffer_sql != NULL)
			delete buffer_sql;
	}
}
// ---------------------------------------------------------------------------

void ServidorCatalogos::GrabaUsuario(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros, AnsiString Tabla,
	AnsiString CampoClave) {
	// GRABA GENERICO
	char *buffer_sql = new char[1024 * 10];
	char *aux_buffer_sql = buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instrucciones[10], instruccion;
	int num_instrucciones = 0;
	AnsiString clave;
	AnsiString tarea;
	AnsiString clave_formateada, ventasxvolumen;
	char tipo;
	int i;

	try {
		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		tarea = mFg.ExtraeStringDeBuffer(&parametros);
		// Indicador si se va a agregar o modificar
		clave = mFg.ExtraeStringDeBuffer(&parametros); // Clave

		datos.AsignaTabla(Tabla.c_str());
		parametros += datos.InsCamposDesdeBuffer(parametros);
		if (tarea == "A"){
			instrucciones[num_instrucciones++] = datos.GenerarSqlInsert();

			instruccion="insert ignore into usuariosucursal(usuario, sucursal) VALUES ('";
			instruccion+=clave;
			instruccion+="', '";
			instruccion+=FormServidor->ObtieneClaveSucursal();
			instruccion+="') ";
			instrucciones[num_instrucciones++]=instruccion;
		}else {
			tipo = datos.ObtieneTipoCampo(CampoClave);
			if (tipo == 'I' || tipo == 'N' || tipo == 'F')
				clave_formateada = clave;
			else
				clave_formateada = "'" + clave + "'";
			instrucciones[num_instrucciones++] = datos.GenerarSqlUpdate
				(CampoClave + "=" + clave_formateada);
		}

		instrucciones[num_instrucciones++]="COMMIT";

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
// ---------------------------------------------------------------------------
// ID_MOD_VOLUMEN
void ServidorCatalogos::GrabaModVolumen(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {

	// ESTA FUNCION SE VA A ELIMINAR EN PROXIMAS VERSIONES (Las modificaciones serán por ID_GRA_ARTICULO)
	char *buffer_sql = new char[1024 * 10];
	char *aux_buffer_sql = buffer_sql;
	AnsiString clave_articulo,clave_producto, clave_present,clave_multiplo,
		volumenNuevo;
	int num_instrucciones = 0;
	AnsiString instruccion, instrucciones[10];

	clave_articulo = mFg.ExtraeStringDeBuffer(&parametros);
	clave_producto = mFg.ExtraeStringDeBuffer(&parametros);
	clave_present = mFg.ExtraeStringDeBuffer(&parametros);
	clave_multiplo = mFg.ExtraeStringDeBuffer(&parametros);
	volumenNuevo = mFg.ExtraeStringDeBuffer(&parametros);

	try {
		instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++] = "START TRANSACTION";

		if (clave_articulo != "")
				instruccion.sprintf("set @folio='%s'", clave_articulo);
			else
				instruccion.sprintf(
				"select @folio:=articulo from articulos where producto='%s' and \
				present='%s' and multiplo='%s'",
					clave_producto, clave_present, clave_multiplo);
			instrucciones[num_instrucciones++] = instruccion;

		///@volumen_recalcular
		instruccion.sprintf("SET @producto_a = (SELECT producto from articulos \
		WHERE articulo = @folio)");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("SET @presentacion_a= (SELECT present from articulos \
		WHERE articulo = @folio)");
		instrucciones[num_instrucciones++]=instruccion;

		// Conocer peso del articulo , antes de ser posiblemente modificado (Caso Modificación)
		instruccion.sprintf("SET  @volumen_recalcular =  (SELECT (volumen/factor) from articulos WHERE articulo = @folio)");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("SET @volu_unitario= (SELECT (%s/factor) from articulos WHERE articulo = @folio)",volumenNuevo);
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("UPDATE articulos SET volumen = factor*@volu_unitario  \
			WHERE producto=@producto_a AND present=@presentacion_a");
			instrucciones[num_instrucciones++]=instruccion;

		instrucciones[num_instrucciones++] = "COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql = mFg.AgregaStringABuffer
			(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (int i = 0; i < num_instrucciones; i++)
			aux_buffer_sql = mFg.AgregaStringABuffer(instrucciones[i],
			aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta,
			TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,
			buffer_sql);
	}__finally {
		delete buffer_sql;
	}
}


// ---------------------------------------------------------------------------
// ID_GRA_IMAGEN_ARTICULO
void ServidorCatalogos::GrabaImagenArticulo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql = new char[1024 * 1024 * 4];
	char *aux_buffer_sql = buffer_sql;
	BufferRespuestas* resp_orden=NULL;
	AnsiString instrucciones[50], instruccion;
	int num_instrucciones = 0;
	AnsiString valida_orden;

	AnsiString articulo, producto, present, nombreimagen, dataimagen;
	AnsiString formatoimagen, descripcionimagen, descripcionart;
	AnsiString id;
    AnsiString paraKiosko;

	articulo = mFg.ExtraeStringDeBuffer(&parametros);
	producto = mFg.ExtraeStringDeBuffer(&parametros);
	present  = mFg.ExtraeStringDeBuffer(&parametros);
	nombreimagen = mFg.ExtraeStringDeBuffer(&parametros);
	dataimagen = mFg.ExtraeStringDeBuffer(&parametros);
	formatoimagen = mFg.ExtraeStringDeBuffer(&parametros);
	descripcionimagen = mFg.ExtraeStringDeBuffer(&parametros);
	descripcionart = mFg.ExtraeStringDeBuffer(&parametros);
	id = mFg.ExtraeStringDeBuffer(&parametros);
    paraKiosko = mFg.ExtraeStringDeBuffer(&parametros);

	try {

		try{
			instruccion.sprintf("SELECT IFNULL(MAX(orden),'') AS orden FROM imagenesarticulos WHERE producto = '%s' AND present = '%s'", producto, present);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_orden);
			if (resp_orden->ObtieneNumRegistros()>0){
				valida_orden=resp_orden->ObtieneDato("orden");
			}
		}__finally{
			if (resp_orden!=NULL) delete resp_orden;
		}

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		if (valida_orden=="") {
			instruccion.sprintf("SET @ordensig:=0");
			instrucciones[num_instrucciones++]=instruccion;
		} else {

			instruccion.sprintf("SELECT @orden:=MAX(orden) FROM imagenesarticulos WHERE producto = '%s' AND present = '%s'", producto, present);
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("SET @ordensig:=@orden+1");
			instrucciones[num_instrucciones++]=instruccion;
		}

		if (id!=" "){
			if(dataimagen==" ") {
				if(descripcionimagen==""){
					instruccion.sprintf("UPDATE imagenesarticulos SET descripcion = NULL, parakiosko=%s, fecha_editada = CURDATE() WHERE producto = '%s' AND present = '%s' AND id = %s ",
					paraKiosko, producto, present, id);
				} else {
					instruccion.sprintf("UPDATE imagenesarticulos SET descripcion = '%s', parakiosko=%s, fecha_editada = CURDATE() WHERE producto = '%s' AND present = '%s' AND id = %s ",
					descripcionimagen, paraKiosko, producto, present, id);
                }
			} else {
				if(descripcionimagen==""){
					instruccion.sprintf("UPDATE imagenesarticulos SET imagen ='%s', formato='%s', descripcion = NULL, fecha_editada = CURDATE() WHERE producto = '%s' AND present = '%s' AND id = %s ",
					dataimagen, formatoimagen, producto, present, id);
				}else{
					instruccion.sprintf("UPDATE imagenesarticulos SET imagen ='%s', formato='%s', descripcion = '%s', fecha_editada = CURDATE() WHERE producto = '%s' AND present = '%s' AND id = %s ",
					dataimagen, formatoimagen, descripcionimagen, producto, present, id);
				}
			}

			instrucciones[num_instrucciones++]=instruccion;
		} else {
			if(descripcionimagen==""){
				instruccion.sprintf("INSERT INTO imagenesarticulos \
				(producto, present, nombre, imagen, formato, descripcion, orden, parakiosko, fecha_subida) \
				VALUES \
				('%s', '%s', CONCAT('%s',' ','%s'), '%s', '%s', NULL, @ordensig, %s, CURDATE())",
				producto, present, producto, present, dataimagen, formatoimagen, descripcionimagen, paraKiosko);
				instrucciones[num_instrucciones++]=instruccion;
			} else {
				instruccion.sprintf("INSERT INTO imagenesarticulos \
				(producto, present, nombre, imagen, formato, descripcion, orden, parakiosko, fecha_subida) \
				VALUES \
				('%s', '%s', CONCAT('%s',' ','%s'), '%s', '%s', '%s', @ordensig, %s, CURDATE())",
				producto, present, producto, present, dataimagen, formatoimagen, descripcionimagen, descripcionimagen, paraKiosko);
				instrucciones[num_instrucciones++]=instruccion;
			}
		}

		if(articulo != ""){
			if(descripcionart==""){
				instruccion.sprintf("REPLACE INTO descripcionarticulo \
				(articulo, descripcion) VALUES ('%s', NULL)",
				articulo);
				instrucciones[num_instrucciones++]=instruccion;
			} else {
				instruccion.sprintf("REPLACE INTO descripcionarticulo \
				(articulo, descripcion) VALUES ('%s', '%s')",
				articulo, descripcionart);
				instrucciones[num_instrucciones++]=instruccion;
			}
        }

		instrucciones[num_instrucciones++]="COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql = mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (int i = 0; i < num_instrucciones; i++)
			aux_buffer_sql = mFg.AgregaStringABuffer(instrucciones[i],aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta,TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);

		instruccion = "SELECT LAST_INSERT_ID()";
        mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);


	}
	__finally {
		delete buffer_sql;
	}
}

// ---------------------------------------------------------------------------
// ID_MOD_PESO
void ServidorCatalogos::GrabaModPeso(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {

	// ESTA FUNCION SE VA A ELIMINAR EN PROXIMAS VERSIONES (Las modificaciones serán por ID_GRA_ARTICULO)
	char *buffer_sql = new char[1024 * 10];
	char *aux_buffer_sql = buffer_sql;
	AnsiString clave_articulo,clave_producto, clave_present,clave_multiplo,
		pesoNuevo;
	int num_instrucciones = 0;
	AnsiString instruccion, instrucciones[10];

	clave_articulo = mFg.ExtraeStringDeBuffer(&parametros);
	clave_producto = mFg.ExtraeStringDeBuffer(&parametros);
	clave_present = mFg.ExtraeStringDeBuffer(&parametros);
	clave_multiplo = mFg.ExtraeStringDeBuffer(&parametros);
	pesoNuevo = mFg.ExtraeStringDeBuffer(&parametros);

	try {
		instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++] = "START TRANSACTION";

		if (clave_articulo != "")
				instruccion.sprintf("set @folio='%s'", clave_articulo);
			else
				instruccion.sprintf(
				"select @folio:=articulo from articulos where producto='%s' and \
				present='%s' and multiplo='%s'",
					clave_producto, clave_present, clave_multiplo);
			instrucciones[num_instrucciones++] = instruccion;

		///@volumen_recalcular
		instruccion.sprintf("SET @producto_a = (SELECT producto from articulos \
		WHERE articulo = @folio)");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("SET @presentacion_a= (SELECT present from articulos \
		WHERE articulo = @folio)");
		instrucciones[num_instrucciones++]=instruccion;

		// Conocer peso del articulo , antes de ser posiblemente modificado (Caso Modificación)
		instruccion.sprintf("SET  @peso_recalcular =  (SELECT (peso/factor) from articulos WHERE articulo = @folio)");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("SET @peso_unitario= (SELECT (%s/factor) from articulos WHERE articulo = @folio)",pesoNuevo);
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("UPDATE articulos SET peso = factor*@peso_unitario  \
			WHERE producto=@producto_a AND present=@presentacion_a");
			instrucciones[num_instrucciones++]=instruccion;

		instrucciones[num_instrucciones++] = "COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql = mFg.AgregaStringABuffer
			(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (int i = 0; i < num_instrucciones; i++)
			aux_buffer_sql = mFg.AgregaStringABuffer(instrucciones[i],
			aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta,
			TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,
			buffer_sql);
	}__finally {
		delete buffer_sql;
	}
}

// ---------------------------------------------------------------------------
// ID_CON_IMAGENES_ARTICULO
void ServidorCatalogos::ConsultaImagenesArticulo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
	AnsiString articulo, producto, present;

	articulo = mFg.ExtraeStringDeBuffer(&parametros);
	producto = mFg.ExtraeStringDeBuffer(&parametros);
	present  = mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	instruccion.sprintf("SELECT id, descripcion, formato, orden, parakiosko FROM imagenesarticulos \
		WHERE producto='%s' AND present = '%s' ORDER BY orden ASC ",
	producto, present);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	instruccion.sprintf("SELECT IFNULL(descripcion,'') AS descripcion FROM descripcionarticulo \
		WHERE articulo = '%s' ",
	articulo);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

}
// ---------------------------------------------------------------------------
// ID_CON_IMAGEN_ARTICULO
void ServidorCatalogos::ConsultaImagenArticulo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
	AnsiString id;

	id = mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos del rol
	instruccion.sprintf("SELECT imagen, nombre, formato, descripcion FROM imagenesarticulos WHERE id=%s ", id);

	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

}
// ---------------------------------------------------------------------------
// ID_BAJA_IMAGEN_ARTICULO
void ServidorCatalogos::BajaImagenArticulo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql = new char[1024 * 64];
	char *aux_buffer_sql = buffer_sql;
	AnsiString instrucciones[10], instruccion;
	int num_instrucciones = 0;

	AnsiString id;

	id = mFg.ExtraeStringDeBuffer(&parametros);

	try {
		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		instruccion.sprintf("DELETE FROM imagenesarticulos WHERE id = %s ",
		id);
		instrucciones[num_instrucciones++]=instruccion;

		instrucciones[num_instrucciones++]="COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql = mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (int i = 0; i < num_instrucciones; i++)
			aux_buffer_sql = mFg.AgregaStringABuffer(instrucciones[i],aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta,TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,buffer_sql);
	}
	__finally {
		delete buffer_sql;
	}
}
// ---------------------------------------------------------------------------
// ID_REORDEN_IMG_ART_ECOM
void ServidorCatalogos::ReordenarImagen(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql = new char[1024 * 1024 * 4];
	char *aux_buffer_sql = buffer_sql;
	BufferRespuestas* resp_imagenes=NULL;
	AnsiString instrucciones[50], instruccion;
	int num_instrucciones = 0;
	AnsiString valida_orden;

	AnsiString articulo, id, orden, tipo;
	AnsiString id_mod;

	articulo = mFg.ExtraeStringDeBuffer(&parametros);
	id = mFg.ExtraeStringDeBuffer(&parametros);
	orden = mFg.ExtraeStringDeBuffer(&parametros);
	tipo = mFg.ExtraeStringDeBuffer(&parametros);

	try {

		try{
			instruccion.sprintf("SELECT id  FROM imagenesarticulos WHERE articulo = '%s' AND orden = %s %s", articulo, orden, tipo);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_imagenes);
			if (resp_imagenes->ObtieneNumRegistros()>0){
				id_mod=resp_imagenes->ObtieneDato("id");
			}
		}__finally{
			if (resp_imagenes!=NULL) delete resp_imagenes;
		}

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		if (id_mod!="") {
			if (tipo=="-1") {

				instruccion.sprintf("UPDATE imagenesarticulos SET orden = orden  + 1 WHERE articulo = '%s' AND id = %s ",
				articulo, id_mod);
				instrucciones[num_instrucciones++]=instruccion;

				instruccion.sprintf("UPDATE imagenesarticulos SET orden = orden %s WHERE articulo = '%s' AND id = %s ",
				tipo, articulo, id);
				instrucciones[num_instrucciones++]=instruccion;

			} else {

				instruccion.sprintf("UPDATE imagenesarticulos SET orden = orden -1 WHERE articulo = '%s' AND id = %s ",
				articulo, id_mod);
				instrucciones[num_instrucciones++]=instruccion;

				instruccion.sprintf("UPDATE imagenesarticulos SET orden = orden %s WHERE articulo = '%s' AND id = %s ",
				tipo, articulo, id);
				instrucciones[num_instrucciones++]=instruccion;
            }
		} else {
			if (tipo=="-1") {
				throw new Exception("No se puede ordenar el elemento ya que se encuentra en la primer posición.");
			} else {
				throw new Exception("No se puede ordenar el elemento ya que se encuentra en la última posición.");
            }
        }

		instrucciones[num_instrucciones++]="COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql = mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (int i = 0; i < num_instrucciones; i++)
			aux_buffer_sql = mFg.AgregaStringABuffer(instrucciones[i],aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta,TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,buffer_sql);
	}
	__finally {
		delete buffer_sql;
	}
}

// ---------------------------------------------------------------------------
//ID_CON_ORIGENVTA
void ServidorCatalogos::ConsultaOrigenVenta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
	AnsiString origen;

	origen = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos de las cancelaciones
	instruccion.sprintf("SELECT tipoorigen, descripcion, activomayoreo, activosuper, clientepordefault, \
	CONCAT(cli.nombre, ' ', cli.appat, ' ', cli.apmat) AS nombre \
	FROM tiposorigenventas o \
	LEFT JOIN clientes cli ON cli.cliente = o.clientepordefault \
	WHERE o.tipoorigen = '%s' \
	ORDER BY tipoorigen ASC ",
	origen);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

	// Obtiene todas las cancelaciones
	instruccion = "SELECT tipoorigen, descripcion, activomayoreo, activosuper, \
	clientepordefault, CONCAT(cli.nombre, ' ', cli.appat, ' ', cli.apmat) AS nombre \
	FROM tiposorigenventas o \
	LEFT JOIN clientes cli ON cli.cliente = o.clientepordefault \
	ORDER BY tipoorigen ASC ";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------
// ID_BAJ_ORIGENVTA
void ServidorCatalogos::BajaOrigenVenta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*10];
	char *aux_buffer_sql=buffer_sql;
	BufferRespuestas* resp=NULL;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[10];

	AnsiString instruccion;
	AnsiString origen;

	origen = mFg.ExtraeStringDeBuffer(&parametros);

	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	try {
		Instruccion.sprintf(" DELETE FROM tiposorigenventas WHERE tipoorigen = '%s' ",
		origen);
		instrucciones[num_instrucciones++]=Instruccion;

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
// ID_GRABA_ORIGENVTA
void ServidorCatalogos::GrabaOrigenVenta(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*10];
	char *aux_buffer_sql=buffer_sql;
	BufferRespuestas* resp=NULL;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[10];

	AnsiString instruccion;
	AnsiString origen, descripcion, activo_mayoreo;
	AnsiString activo_super, cliente, tipo_tarea;

	tipo_tarea = mFg.ExtraeStringDeBuffer(&parametros);
	origen = mFg.ExtraeStringDeBuffer(&parametros);
	descripcion = mFg.ExtraeStringDeBuffer(&parametros);
	activo_mayoreo = mFg.ExtraeStringDeBuffer(&parametros);
	activo_super = mFg.ExtraeStringDeBuffer(&parametros);
	cliente = mFg.ExtraeStringDeBuffer(&parametros);

	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	try {
		if (tipo_tarea=="A") {
			if (cliente=="") {
				Instruccion.sprintf(" INSERT INTO tiposorigenventas \
				(tipoorigen, descripcion, activomayoreo, activosuper, clientepordefault) \
				VALUES \
				('%s', '%s', %s, %s, NULL) ",
				origen, descripcion, activo_mayoreo, activo_super);
				instrucciones[num_instrucciones++]=Instruccion;
			} else {
				Instruccion.sprintf(" INSERT INTO tiposorigenventas \
				(tipoorigen, descripcion, activomayoreo, activosuper, clientepordefault) \
				VALUES \
				('%s', '%s', %s, %s, '%s') ",
				origen, descripcion, activo_mayoreo, activo_super, cliente);
				instrucciones[num_instrucciones++]=Instruccion;
            }
		} else {
			if (cliente=="") {
				Instruccion.sprintf(" UPDATE tiposorigenventas SET descripcion = '%s', \
				activomayoreo = %s, activosuper = %s, clientepordefault = NULL \
				WHERE tipoorigen = '%s' ",
				descripcion, activo_mayoreo, activo_super, origen);
				instrucciones[num_instrucciones++]=Instruccion;
			} else {
				Instruccion.sprintf(" UPDATE tiposorigenventas SET descripcion = '%s', \
				activomayoreo = %s, activosuper = %s, clientepordefault = '%s' \
				WHERE tipoorigen = '%s' ",
				descripcion, activo_mayoreo, activo_super, cliente, origen);
				instrucciones[num_instrucciones++]=Instruccion;
			}
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
// ---------------------------------------------------------------------------

// ID_CON_PARAMETROCDFIWEB
void ServidorCatalogos::ConsultaParametrocfdiweb(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA PARAMETRO
	AnsiString instruccion;
	AnsiString clave, indiciparametro, empresa, empresa_filtro;
	AnsiString condicion_indiciparametro = " ", condicion_empresa = " ";

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	indiciparametro = mFg.ExtraeStringDeBuffer(&parametros);
	empresa = mFg.ExtraeStringDeBuffer(&parametros);
	empresa_filtro = mFg.ExtraeStringDeBuffer(&parametros);

	if (indiciparametro!=" ") {
	   condicion_indiciparametro.sprintf(" AND (Parametro LIKE '%%%s%%' OR Descripcion LIKE '%%%s%%' OR Valor LIKE '%%%s%%') ",
	   indiciparametro, indiciparametro, indiciparametro);
	}

	if(empresa_filtro.Trim() != ""){
		condicion_empresa.sprintf( " AND p.idempresa=%s ", empresa_filtro );
	}

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos del parametro
	instruccion.sprintf("select * from parametroscfdiweb where parametro='%s' and idempresa=%s ",
	clave, empresa );
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los parametros
	instruccion.sprintf(
		"select p.parametro AS Parametro, p.descripcion AS Descripcion, p.valor AS Valor, \
		e.nombre AS Nombre, p.idempresa AS idEmpresa  \
		from parametroscfdiweb p \
		INNER JOIN empresas e ON e.idempresa = p.idempresa   \
		where 1 %s %s order by p.idempresa, parametro",
		 condicion_empresa, condicion_indiciparametro);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------
// ID_GRA_PARAMETROCDFIWEB
void ServidorCatalogos::GrabaParametrocfdiweb(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA PARAMETRO CFDIWEB
	//GrabaGenerico(Respuesta, MySQL, parametros, "parametroscfdiweb", "parametro");
	char *buffer_sql = new char[1024 * 100];
	char *aux_buffer_sql = buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);

	int num_instrucciones = 0;
    int i = 0;
	AnsiString instruccion, instrucciones[100];
	AnsiString tarea, clave, empresa;

	tarea = mFg.ExtraeStringDeBuffer(&parametros);
	clave = mFg.ExtraeStringDeBuffer(&parametros);
	empresa = mFg.ExtraeStringDeBuffer(&parametros);

    datos.AsignaTabla("parametroscfdiweb");
	parametros += datos.InsCamposDesdeBuffer(parametros);

	try {
        instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++] = "START TRANSACTION";

		if (tarea == "A"){
		   instrucciones[num_instrucciones++] = datos.GenerarSqlInsert();
		}else{
		   instrucciones[num_instrucciones++] = datos.GenerarSqlUpdate
				("parametro='" + clave + "' AND idempresa = "+empresa+" ");
		}

		instrucciones[num_instrucciones++] = "COMMIT";

		aux_buffer_sql = mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);

		for (i = 0; i < num_instrucciones; i++){
		  aux_buffer_sql = mFg.AgregaStringABuffer(instrucciones[i],aux_buffer_sql);
		}

		mServidorVioleta->InicializaBuffer(Respuesta,TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,buffer_sql);

    }__finally {
		delete buffer_sql;
	}


}
// ---------------------------------------------------------------------------
// ID_GRA_DESCRIPCION_ARTICULO
void ServidorCatalogos::GrabaDescripcionArticulo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql = new char[1024 * 64];
	char *aux_buffer_sql = buffer_sql;
	AnsiString instrucciones[10], instruccion;
	int num_instrucciones = 0;
	int existe = 0;

	AnsiString articulo, descripcion;

	articulo = mFg.ExtraeStringDeBuffer(&parametros);
	descripcion = mFg.ExtraeStringDeBuffer(&parametros);

	try {

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		instruccion.sprintf("REPLACE INTO descripcionarticulo \
		(articulo, descripcion) VALUES ('%s', '%s')",
		articulo, descripcion);
		instrucciones[num_instrucciones++]=instruccion;

		instrucciones[num_instrucciones++]="COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql = mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (int i = 0; i < num_instrucciones; i++)
			aux_buffer_sql = mFg.AgregaStringABuffer(instrucciones[i],aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta,TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,buffer_sql);
	} __finally {
		delete buffer_sql;
	}
}
// ---------------------------------------------------------------------------
//ID_CON_TIPOS
void ServidorCatalogos::ConsultaTipos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion,tipo_consulta;
	AnsiString clave_tipo = " ", consulta = " ",activo = " ";

	tipo_consulta = mFg.ExtraeStringDeBuffer(&parametros);
	clave_tipo = mFg.ExtraeStringDeBuffer(&parametros);
	activo = mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_BUFFER);

	if(tipo_consulta == 1){
		instruccion.sprintf("select tipo,descripcion,activo from tiposcompra where tipo = %s", clave_tipo);
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
			Respuesta->TamBufferResultado);

		instruccion =
			"select tipo,descripcion,activo from tiposcompra ";
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
			Respuesta->TamBufferResultado);
	}else if(tipo_consulta == 2){
		instruccion.sprintf("update tiposcompra set activo = '%s' where tipo = '%s'",activo,clave_tipo);
		if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL,
			instruccion.c_str()))
			throw(Exception("Error en query EjecutaSelectSqlNulo"));

		instruccion =
			"select tipo,descripcion,activo from tiposcompra ";
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
			Respuesta->TamBufferResultado);
	}
}
// ---------------------------------------------------------------------------
//ID_CON_REGIMEN
void ServidorCatalogos::ConsultaRegimen(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
	AnsiString regimen_fiscal = " ";

	regimen_fiscal = mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_BUFFER);

	instruccion.sprintf("select regimenfiscal,descripcion,esfisica,esmoral,\
						activo,versioncfdi from cregimenfiscal\
						where regimenfiscal = %s", regimen_fiscal);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

	instruccion = "select regimenfiscal,descripcion,esfisica,esmoral,\
				   activo,versioncfdi from cregimenfiscal ";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	
}
// ---------------------------------------------------------------------------
//ID_GRA_REGIMEN
void ServidorCatalogos::GrabaRegimen(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	GrabaGenerico(Respuesta, MySQL, parametros, "cregimenfiscal", "regimenfiscal");
}
// ---------------------------------------------------------------------------
// ID_BAJ_REGIMEN
void ServidorCatalogos::BajaRegimen(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// BAJA CLASIFCONTABLE
	BajaGenerico(Respuesta, MySQL, parametros, "cregimenfiscal", "regimenfiscal");
}
// ---------------------------------------------------------------------------
//ID_CON_USO_CFDI
void ServidorCatalogos::ConsultaUsoCFDI(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
	AnsiString cfdi = " ";

	cfdi = mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_BUFFER);

	instruccion.sprintf("select usocfdi, descripcion from usoscfdiweb\
						where usocfdi = '%s'", cfdi);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

	instruccion = "select usocfdi,descripcion from usoscfdiweb ";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

}
// ---------------------------------------------------------------------------
//ID_GRA_USO_CFDI
void ServidorCatalogos::GrabaCFDI(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	GrabaGenerico(Respuesta, MySQL, parametros, "usoscfdiweb", "usocfdi");
}
// ---------------------------------------------------------------------------
//ID_BAJ_USO_CFDI
void ServidorCatalogos::BajaCFDI(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// BAJA CLASIFCONTABLE
	BajaGenerico(Respuesta, MySQL, parametros, "usoscfdiweb", "usocfdi");
}
// ---------------------------------------------------------------------------
//ID_CON_MSJ_PP
void ServidorCatalogos::ConsultaMsjPinPad(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
	AnsiString cod = " ";

	cod = mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_BUFFER);

	instruccion.sprintf("SELECT codigorespuesta, mesajerespuesta FROM mensajesrespuestapinpad  \
	WHERE  codigorespuesta = '%s'", cod);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

	instruccion = "SELECT codigorespuesta, mesajerespuesta FROM mensajesrespuestapinpad \
	ORDER BY codigorespuesta ";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

}
// ---------------------------------------------------------------------------
//ID_GRA_MSJ_PP
void ServidorCatalogos::GrabaMsjPinPad(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	GrabaGenerico(Respuesta, MySQL, parametros, "mensajesrespuestapinpad", "codigorespuesta");
}
// ---------------------------------------------------------------------------
//ID_BAJ_MSJ_PP
void ServidorCatalogos::BajaMsjPinPad(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// BAJA MENSAJE
	BajaGenerico(Respuesta, MySQL, parametros, "mensajesrespuestapinpad", "codigorespuesta");
}
// ---------------------------------------------------------------------------
//ID_CON_METPAG_ECOM
void ServidorCatalogos::ConsultaMetPagEcom(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
	AnsiString id;

	id = mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_BUFFER);

	instruccion.sprintf("SELECT * FROM metodosdepagoecomm WHERE id = '%s'", id);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

	instruccion = "SELECT id, tipo, descripcion, activo, termino FROM metodosdepagoecomm \
	ORDER BY id  ";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

	instruccion =
		"SELECT termino FROM terminosdepago";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

}
// ---------------------------------------------------------------------------
//ID_GRA_METPAG_ECOM
void ServidorCatalogos::GrabaMetPagEcom(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	GrabaGenerico(Respuesta, MySQL, parametros, "metodosdepagoecomm", "id");
}
// ---------------------------------------------------------------------------
//ID_BAJ_METPAG_ECOM
void ServidorCatalogos::BajaMetPagEcom(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	BajaGenerico(Respuesta, MySQL, parametros, "metodosdepagoecomm", "id");
}
// ---------------------------------------------------------------------------
//ID_CON_CART_PORT20
void ServidorCatalogos::ConsultaCartPorte20(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
	AnsiString id = " ";

	id = mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_BUFFER);

	instruccion.sprintf("SELECT * FROM paramcartport20 WHERE idparamcp20 = '%s'", id);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

	instruccion = "SELECT idparamcp20, desctransporte, nomunidadpeso, ccvetipoperm  FROM paramcartport20 ORDER BY idparamcp20 ";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

}
// ---------------------------------------------------------------------------
//ID_GRA_CART_PORT20
void ServidorCatalogos::GrabaCartPorte20(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	GrabaGenerico(Respuesta, MySQL, parametros, "paramcartport20", "idparamcp20");
}
// ---------------------------------------------------------------------------
//ID_BAJ_CART_PORT20
void ServidorCatalogos::BajaCartPorte20(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	BajaGenerico(Respuesta, MySQL, parametros, "paramcartport20", "idparamcp20");
}
// ---------------------------------------------------------------------------
//ID_CON_TNOTCREPAG
void ServidorCatalogos::ConsultaNOTCREPAG(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
	AnsiString tipo = " ";

	tipo = mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_BUFFER);

	instruccion.sprintf("select tipo, descripcion, activo from tiposdencregastos\
						where tipo = '%s'", tipo);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

	instruccion = "select tipo,descripcion, activo from tiposdencregastos ";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------
//ID_GRA_TNOTCREPAG
void ServidorCatalogos::GrabaTNOTCREPAG(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	GrabaGenerico(Respuesta, MySQL, parametros, "tiposdencregastos", "tipo");
}
// ---------------------------------------------------------------------------
//ID_BAJ_TNOTCREPAG
void ServidorCatalogos::BajaTNOTCREPAG(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// BAJA CLASIFCONTABLE
	BajaGenerico(Respuesta, MySQL, parametros, "tiposdencregastos", "tipo");
}
// ---------------------------------------------------------------------------
//ID_CON_TERCOB
void ServidorCatalogos::ConsultaTerminosCobranza(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
	AnsiString clave = " ";

	clave = mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_BUFFER);

	instruccion.sprintf("select clave, descripcion, tipo, orden from terminoscobranza\
						where clave = '%s'", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

	instruccion = "select clave,descripcion, tipo,orden from terminoscobranza ";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------
//ID_GRA_TERCOB
void ServidorCatalogos::GrabaTerminosCobranza(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	GrabaGenerico(Respuesta, MySQL, parametros, "terminoscobranza", "clave");
}
// ---------------------------------------------------------------------------
//ID_BAJ_TERCOB
void ServidorCatalogos::BajaTerminosCobranza(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	BajaGenerico(Respuesta, MySQL, parametros, "terminoscobranza", "clave");
}
// ---------------------------------------------------------------------------
//ID_CON_PBANCOS
void ServidorCatalogos::ConsultaParametroBancos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    AnsiString instruccion;
	AnsiString param = " ";

	param = mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_BUFFER);

	instruccion.sprintf("select parametro, descripcion, valor from parambancos\
						where parametro = '%s'", param);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

	instruccion = "select parametro, descripcion, valor from parambancos ";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------
//ID_GRA_PBANCOS
void ServidorCatalogos::GrabaParametroBancos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	GrabaGenerico(Respuesta, MySQL, parametros, "parambancos", "parametro");
}
// ---------------------------------------------------------------------------
//ID_BAJ_PBANCOS
void ServidorCatalogos::BajaParametroBancos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	BajaGenerico(Respuesta, MySQL, parametros, "parambancos", "parametro");
}
// ---------------------------------------------------------------------------
// ID_UPD_COSTO_X_GRUPO
void ServidorCatalogos::EjecutaActualizaCostosxGrupo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	BufferRespuestas* resp_paramcambioprec=NULL;
	BufferRespuestas* resp_datos = NULL;
	BufferRespuestas* resp_articulos = NULL;
	char *buffer_sql = new char[1024 * 64];
	char *aux_buffer_sql = buffer_sql;
	int i;
	int num_instrucciones = 0;
	AnsiString paramcambioprec;
	AnsiString instruccion, instrucciones[1000], Instruccion;
	AnsiString producto, presentaciones, usuario, costobase;

	AnsiString utilesp, porcutil;
	double porcutilantes=0;
	AnsiString mListaPorcentajes, mListaPorcentajesActual;
	AnsiString MayorListaPorcentaj, MayorListaPorcenActual;
	int grabados=0;
	int num_precios;
	TStringList *mListaPorcentManual = new TStringList();
    TStringList *mListaPorcentManActual = new TStringList();

	producto = mFg.ExtraeStringDeBuffer(&parametros);
	costobase = mFg.ExtraeStringDeBuffer(&parametros);
	presentaciones = mFg.ExtraeStringDeBuffer(&parametros);
	usuario = mFg.ExtraeStringDeBuffer(&parametros);

	// Consulta de parámetro para asignación de precios
	// * Si es 0 el cambio de precio es inmediato y se guarda en el campo precios.precio
	// * si es 1 entonces el cambio de precios es diferido y se guarda precios.precioproximo para aplicarse posteriormente)

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

		instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++] = "START TRANSACTION";


		TStringDynArray present(SplitString(presentaciones, ","));
		for (int p = 0; p < present.Length; p++) {
            try {
				Instruccion.sprintf("SELECT a.articulo, pcb.costobase, pcb.idempresa \
				FROM articulos a \
				INNER JOIN precios p ON p.articulo = a.articulo \
				INNER JOIN tiposdeprecios tp ON tp.tipoprec=p.tipoprec AND tp.idempresa=%s \
				INNER JOIN presentaciones prec ON prec.producto = a.producto AND prec.present = a.present \
				INNER JOIN presentacionescb pcb ON pcb.producto=prec.producto AND pcb.present=prec.present \
				WHERE a.producto = '%s' AND a.present = %s AND pcb.idempresa=%s \
				GROUP BY a.articulo", FormServidor->ObtieneClaveEmpresa(),
				producto, AnsiString(present[p]), FormServidor->ObtieneClaveEmpresa() );
				if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), resp_articulos)) {
					for (int i = 0; i < resp_articulos->ObtieneNumRegistros(); i++) {
                        resp_articulos->IrAlRegistroNumero(i);
						try {
							Instruccion.sprintf("SELECT a.porccomi, a.porcComi, \
							MAX(p.porcutil) AS porcutil, a.activo \
							FROM articulos a \
							INNER JOIN precios p on p.articulo = a.articulo \
							INNER JOIN tiposdeprecios tp ON tp.tipoprec=p.tipoprec AND tp.idempresa=%s \
							WHERE a.articulo= '%s' GROUP BY a.activo limit 1", FormServidor->ObtieneClaveEmpresa(),
							resp_articulos->ObtieneDato("articulo"));
							mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), resp_datos);

							if(resp_datos->ObtieneNumRegistros()>0){
								resp_datos->IrAlRegistroNumero(0);
								instruccion.sprintf("INSERT INTO bitacoracostos \
								VALUES \
								('CATALOGO','%s','CATM',%s,'%s',CURDATE(),CURTIME(),'%s', %s) ",
								resp_articulos->ObtieneDato("articulo"),
								resp_articulos->ObtieneDato("costobase"),
								costobase, usuario, resp_articulos->ObtieneDato("idempresa") );
								instrucciones[num_instrucciones++] = instruccion;
							}

						} __finally {
							if(resp_datos!=NULL) delete resp_datos;
						}
					}

					AnsiString asignacion_precio;
					if (paramcambioprec=="0") {
						asignacion_precio.sprintf("p.precio=ROUND(ROUND(%s*(1+p.porcutil/100),pr.valor)*a.factor,pr.valor) , \
						p.precioproximo=ROUND(ROUND(%s*(1+p.porcutil/100),pr.valor)*a.factor,pr.valor)  ",
						costobase, costobase);
					} else {
						asignacion_precio.sprintf("p.precioproximo=ROUND(ROUND(%s*(1+p.porcutil/100),pr.valor)*a.factor,pr.valor) ", costobase);
					}

					instruccion.sprintf("update precios p \
                    INNER JOIN tiposdeprecios tp ON tp.tipoprec = p.tipoprec \
					INNER JOIN articulos a ON p.articulo=a.articulo \
					INNER JOIN parametrosemp as pr ON pr.parametro='DIGREDOND' AND pr.sucursal='%s' \
					set p.costo=%s, %s \
					where a.producto='%s' and a.present=%s and tp.idempresa=%s ", FormServidor->ObtieneClaveSucursal(),
					costobase, asignacion_precio, producto, AnsiString(present[p]), FormServidor->ObtieneClaveEmpresa());
					instrucciones[num_instrucciones++] = instruccion;

					instruccion.sprintf("update presentacionescb pcb \
					SET pcb.costoultimo=pcb.costobase, pcb.costobase=%s \
					where pcb.idempresa=%s and pcb.producto='%s' and pcb.present=%s ",
					costobase, FormServidor->ObtieneClaveEmpresa(), producto, AnsiString(present[p]));
					instrucciones[num_instrucciones++] = instruccion;
				}
			} __finally {
				if (resp_articulos!=NULL) delete resp_articulos;
			}
		}

		instrucciones[num_instrucciones++] = "COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql = mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i = 0; i < num_instrucciones; i++)
			aux_buffer_sql = mFg.AgregaStringABuffer(instrucciones[i],aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta,TAM_MAX_BUFFER_RESPUESTA_REP);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,buffer_sql);

	}
	__finally {

		delete buffer_sql;
	}

}
// ---------------------------------------------------------------------------
// ID_CON_PLANTILLA_CREDENCIAL_EMPLEADO
void ServidorCatalogos::ConsultaPlantillaCredEmpleado(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion, sucursal, condicion_sucursal = " ";

	sucursal = mFg.ExtraeStringDeBuffer(&parametros);

	if(sucursal.Trim() != "")
		condicion_sucursal.sprintf(" AND suc.sucursal = '%s' ", sucursal);


	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	instruccion.sprintf("SELECT imagen, lado FROM caratulacredempleado cce \
		INNER JOIN sucursales suc ON suc.idempresa = cce.idempresa \
		WHERE 1 %s \
	 GROUP BY cce.lado ", condicion_sucursal);

	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

}
// ---------------------------------------------------------------------------
// ID_CAT_GUARDA_EMPLEADO
void ServidorCatalogos::GuardaEmpleadoCredencial(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	GrabaGenerico(Respuesta, MySQL, parametros, "empleados","empleado");
}
// ---------------------------------------------------------------------------
//ID_CON_OPCANCEL_PREPAGO
void ServidorCatalogos::ConsultaCancelacionPrepago(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA TERMINO DE PAGO
	AnsiString instruccion;
	AnsiString clave;

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos de las cancelaciones
	instruccion.sprintf("SELECT * FROM catalogocancelprepagos WHERE id = %s ",
		clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

	// Obtiene todas las cancelaciones
	instruccion =
		"SELECT id, nombre, activo FROM catalogocancelprepagos ORDER BY nombre ";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------
// ID_BAJ_OPCANCEL_PREPAGO
void ServidorCatalogos::BajaCancelacionPrepago(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// BAJA TERMINO PAGO
	BajaGenerico(Respuesta, MySQL, parametros, "catalogocancelprepagos", "id");
}

// ---------------------------------------------------------------------------
// ID_GRABA_OPCANCEL_PREPAGO
void ServidorCatalogos::GrabaCancelacionPrepago(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// GRABA TERMINO DE PAGO
	GrabaGenerico(Respuesta, MySQL, parametros, "catalogocancelprepagos", "id");
}

// ---------------------------------------------------------------------------
//ID_CON_CRED_EMPLEADO
void ServidorCatalogos::ConsultaCredEmpleado(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
	AnsiString empresa, clave, condicion_empresa = " ";

	empresa = mFg.ExtraeStringDeBuffer(&parametros);
	clave = mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	if(empresa.Trim() != "")
		condicion_empresa.sprintf(" AND emp.idempresa = %s ", empresa);


	instruccion.sprintf("SELECT lado, emp.nombre, imagen, emp.idempresa FROM caratulacredempleado ce \
	INNER JOIN empresas emp ON emp.idempresa = ce.idempresa \
	WHERE lado = '%s' %s ", clave, condicion_empresa);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

	instruccion = "SELECT IF(lado = 'F', 'Frente', 'Reverso'), lado, emp.nombre, emp.idempresa, imagen FROM caratulacredempleado ce \
	INNER JOIN empresas emp ON emp.idempresa = ce.idempresa \
	 ORDER BY emp.idempresa, lado ";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------
// ID_GRABA_CRED_EMPLEADO
void ServidorCatalogos::GrabaCredEmpleado(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// GRABA TERMINO DE PAGO
	GrabaGenerico(Respuesta, MySQL, parametros, "caratulacredempleado", "id");
}
// ---------------------------------------------------------------------------
//ID_CON_BANNERSIMG
void ServidorCatalogos::ConsultaBannerImg(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
	AnsiString clave;

	//clave = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	instruccion.sprintf("SELECT v.idverificadorimg, v.nombrearch,\
		v.fechaalta, v.horaalta, CONCAT(e.nombre,' ',e.appat,' ',e.apmat)\
		AS usuario, v.imagen, v.fechavigencia,v.activo,v.fechainiciovigencia\
		FROM verificadorban v INNER JOIN empleados e ON e.empleado=v.usualta");
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------
//ID_BAJ_BANNERSIMG
void ServidorCatalogos::BajaBannerImg(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{


    AnsiString instruccion;
	AnsiString id;

	id = mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_BUFFER);

    instruccion.sprintf("delete from verificadorban where id ='%s'",id);

    if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL,
			instruccion.c_str()))
			throw(Exception("Error en query EjecutaSelectSqlNulo"));
}
// ---------------------------------------------------------------------------
//ID_GRA_BITALMACEN
void ServidorCatalogos::GuardarBitacoraAlmacen(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    char *buffer_sql=new char[1024*30];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[10];

	AnsiString usuario, almacen, seccion, seccionant, nombre, nombreant, encargado, encargadoant;
	AnsiString terminal, terminalant, calle, calleant, colonia, coloniaant, localidad, localidadant, cp, cpant;
	AnsiString email, emailant, telefono, telefonoant, activo, activoant, permcompras, permcomprasant;

	almacen = mFg.ExtraeStringDeBuffer(&parametros);
	usuario = mFg.ExtraeStringDeBuffer(&parametros);
	seccion = mFg.ExtraeStringDeBuffer(&parametros);
	seccionant = mFg.ExtraeStringDeBuffer(&parametros);
	nombre = mFg.ExtraeStringDeBuffer(&parametros);
	nombreant = mFg.ExtraeStringDeBuffer(&parametros);
	encargado = mFg.ExtraeStringDeBuffer(&parametros);
	encargadoant = mFg.ExtraeStringDeBuffer(&parametros);
	terminal = mFg.ExtraeStringDeBuffer(&parametros);
	terminalant = mFg.ExtraeStringDeBuffer(&parametros);
	calle = mFg.ExtraeStringDeBuffer(&parametros);
	calleant = mFg.ExtraeStringDeBuffer(&parametros);
	colonia = mFg.ExtraeStringDeBuffer(&parametros);
	coloniaant = mFg.ExtraeStringDeBuffer(&parametros);
	localidad = mFg.ExtraeStringDeBuffer(&parametros);
	localidadant = mFg.ExtraeStringDeBuffer(&parametros);
	cp = mFg.ExtraeStringDeBuffer(&parametros);
	cpant = mFg.ExtraeStringDeBuffer(&parametros);
	email = mFg.ExtraeStringDeBuffer(&parametros);
	emailant = mFg.ExtraeStringDeBuffer(&parametros);
	telefono = mFg.ExtraeStringDeBuffer(&parametros);
	telefonoant = mFg.ExtraeStringDeBuffer(&parametros);
	activo = mFg.ExtraeStringDeBuffer(&parametros);
	activoant = mFg.ExtraeStringDeBuffer(&parametros);
	permcompras = mFg.ExtraeStringDeBuffer(&parametros);
	permcomprasant = mFg.ExtraeStringDeBuffer(&parametros);

    instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	try{
		Instruccion.sprintf("INSERT INTO bitacoraalmacen (almacen, seccion, seccionant, nombre, nombreant, \
														  encargado, encargadoant, terminal, terminalant, \
														  calle, calleant, colonia, coloniaant, localidad, localidadant, \
														  cp, cpant, email, emailant, telefono, telefonoant, \
														  activo, activoant, permcompras, permcomprasant, \
														  usuario, fecha, hora) \
														  VALUES( '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', \
																  '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s',  \
																  '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', CURDATE(), CURTIME() ) ",
															almacen, seccion, seccionant, nombre, nombreant, encargado, encargadoant,
															terminal, terminalant, calle, calleant, colonia, coloniaant,
															localidad, localidadant, cp, cpant, email, emailant, telefono, telefonoant,
															activo, activoant, permcompras, permcomprasant, usuario );
		instrucciones[num_instrucciones++]=Instruccion;

		instrucciones[num_instrucciones++]="COMMIT";

		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);

		for (i=0; i<num_instrucciones; i++){
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);
		}

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);

	} __finally {
		if(buffer_sql != NULL) delete buffer_sql;
	}
}	
// ---------------------------------------------------------------------------
//ID_BUSQ_ARTPEDM
void ServidorCatalogos::BuscArpedM(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instruccion;
	AnsiString fechaini, fechafin, producto, presentacion, cadenaproducto = " ", condicion_presentacion = " ";

	fechaini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
	fechafin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
	producto=mFg.ExtraeStringDeBuffer(&parametros);
	presentacion =mFg.ExtraeStringDeBuffer(&parametros);

	if(producto != " ")
		cadenaproducto.sprintf(" AND barped.producto = '%s' ", producto);

	if(presentacion != " ")
		condicion_presentacion.sprintf(" AND barped.present = '%s' ", presentacion);


	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ);

	instruccion.sprintf("SELECT barped.fechamodi, barped.horamodi, barped.usuario, CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS nomusuario, \
	barped.producto, barped.present, barped.claveprodprovantes, barped.claveprodprovdesp, \
	barped.dureordenantes, barped.dureordendes, barped.durnmaxantes, barped.durnmaxdes, barped.descontinuadoant, barped.descontinuadodesp, \
	barped.redondeocajaant, barped.redondeocajades, barped.multiplopedirant,barped.multiplopedirdes, \
	barped.stockminimoant,barped.stockminimodes,\
	barped.todassucursales, barped.sucursal\
	FROM bitacoraarticulosped barped \
	LEFT JOIN empleados e ON barped.usuario = e.empleado \
	WHERE barped.fechamodi >= '%s' AND barped.fechamodi <= '%s'   \
	%s %s   \
	ORDER BY barped.fechamodi DESC, barped.horamodi DESC",
	fechaini, fechafin, cadenaproducto, condicion_presentacion);

	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}

// ---------------------------------------------------------------------------
//ID_GRA_BITATIPOSPRECIOS
void ServidorCatalogos::GuardarBitacoraTiposPrecios(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros){
	char *buffer_sql=new char[1024*30];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[10];
	AnsiString tipoprec, usuario, descripcion, descripcionant;
	AnsiString porcutil, porcutilant, porcutil2, porcutil2ant, porcutil3, porcutil3ant, porcutil4, porcutil4ant, porcutil5, porcutil5ant;
	AnsiString listamovil, listamovilant, verventmayoreo, verventmayoreoant, verprecdif, verprecdifant;

	tipoprec=mFg.ExtraeStringDeBuffer(&parametros);
	usuario=mFg.ExtraeStringDeBuffer(&parametros);

	descripcion=mFg.ExtraeStringDeBuffer(&parametros);
	descripcionant=mFg.ExtraeStringDeBuffer(&parametros);

	porcutil=mFg.ExtraeStringDeBuffer(&parametros);
	porcutilant=mFg.ExtraeStringDeBuffer(&parametros);

	porcutil2=mFg.ExtraeStringDeBuffer(&parametros);
	porcutil2ant=mFg.ExtraeStringDeBuffer(&parametros);

	porcutil3=mFg.ExtraeStringDeBuffer(&parametros);
	porcutil3ant=mFg.ExtraeStringDeBuffer(&parametros);

	porcutil4=mFg.ExtraeStringDeBuffer(&parametros);
	porcutil4ant=mFg.ExtraeStringDeBuffer(&parametros);

	porcutil5=mFg.ExtraeStringDeBuffer(&parametros);
	porcutil5ant=mFg.ExtraeStringDeBuffer(&parametros);

	listamovil=mFg.ExtraeStringDeBuffer(&parametros);
	listamovilant=mFg.ExtraeStringDeBuffer(&parametros);

	verventmayoreo=mFg.ExtraeStringDeBuffer(&parametros);
	verventmayoreoant=mFg.ExtraeStringDeBuffer(&parametros);

	verprecdif=mFg.ExtraeStringDeBuffer(&parametros);
	verprecdifant=mFg.ExtraeStringDeBuffer(&parametros);

	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	try{

		Instruccion.sprintf("INSERT INTO bitacoratiposprecios(tipoprec, descripcion, descripcionant, \
															  porcutil, porcutilant, porcutil2, porcutil2ant, \
															  porcutil3, porcutil3ant, porcutil4, porcutil4ant, \
															  porcutil5, porcutil5ant, listamovil, listamovilant, \
															  verventmayoreo, verventmayoreoant, verprecdif, verprecdifant, \
															  usuario, fecha, hora) VALUES ( '%s','%s', '%s', %s, %s, \
															   %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, \
															   %s, '%s', CURDATE(), CURTIME()) ",
															  tipoprec, descripcion, descripcionant,
															  porcutil, porcutilant, porcutil2, porcutil2ant,
															  porcutil3, porcutil3ant, porcutil4, porcutil4ant,
															  porcutil5, porcutil5ant, listamovil, listamovilant,
															  verventmayoreo, verventmayoreoant, verprecdif, verprecdifant,
															  usuario);



		instrucciones[num_instrucciones++]=Instruccion;

		instrucciones[num_instrucciones++]="COMMIT";

		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);

		for (i=0; i<num_instrucciones; i++){
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);
		}

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);

    }__finally {
		if(buffer_sql != NULL) delete buffer_sql;
	}

}
// ---------------------------------------------------------------------------
//ID_CON_FACTOR_ARTICULO
void ServidorCatalogos::EjecutaConsultaFactorArticulo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{

	AnsiString producto;
	AnsiString factor;
	AnsiString present;
    AnsiString instruccion;

	producto = mFg.ExtraeStringDeBuffer(&parametros);
	factor = mFg.ExtraeStringDeBuffer(&parametros);
	present = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos de las cancelaciones
	instruccion.sprintf("SELECT * FROM articulos a WHERE producto = '%s' AND a.factor = %s and present = '%s' ",
		producto, factor, present);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

}
// ---------------------------------------------------------------------------
//ID_GRA_ASOCIA_CLIENTE
void ServidorCatalogos::GrabaAsociadosxCliente(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{

	BufferRespuestas* resp_datos = NULL;
	BufferRespuestas* resp_verificacion = NULL;
	char *buffer_sql = new char[1024 * 64];
	char *aux_buffer_sql = buffer_sql;
	int i;
	int num_instrucciones = 0;
	AnsiString paramcambioprec;
	AnsiString instruccion, instrucciones[1000], Instruccion;
	AnsiString cliente, nombre, puesto, depart, codbar, usuario;
	int numRegi;

	cliente = mFg.ExtraeStringDeBuffer(&parametros);
	usuario = mFg.ExtraeStringDeBuffer(&parametros);
	numRegi = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));

	try {

		instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++] = "START TRANSACTION";

		instruccion.sprintf("DELETE d.* FROM dclientesasociado d \
		INNER JOIN clientesasociado c ON c.id = d.id \
		WHERE c.cliente = '%s'",
		cliente);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("DELETE FROM clientesasociado \
		WHERE cliente = '%s'",
		cliente);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("INSERT INTO clientesasociado \
		(cliente, fechaalta, horaalta, usualta, fechavigencia) \
		VALUES \
		('%s', CURDATE(), CURTIME(), '%s', DATE_ADD(CURDATE(), INTERVAL 30 DAY)) ",
		cliente, usuario);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion = "select @id:=last_insert_id() ";
		instrucciones[num_instrucciones++] = instruccion;

		for (int i = 0; i < numRegi; i++) {
			nombre  = mFg.ExtraeStringDeBuffer(&parametros);
			puesto  = mFg.ExtraeStringDeBuffer(&parametros);
			depart  = mFg.ExtraeStringDeBuffer(&parametros);
			codbar  = mFg.ExtraeStringDeBuffer(&parametros);

			instruccion.sprintf("INSERT IGNORE INTO dclientesasociado \
			(id, empleado, puesto, departamento, codbarras) \
			VALUES \
			(@id, '%s', '%s', '%s', '%s') ",
			nombre, puesto, depart, codbar);
			instrucciones[num_instrucciones++] = instruccion;

		}

		instrucciones[num_instrucciones++] = "COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql = mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i = 0; i < num_instrucciones; i++)
			aux_buffer_sql = mFg.AgregaStringABuffer(instrucciones[i],aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta,TAM_MAX_BUFFER_RESPUESTA_REP);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,buffer_sql)) {
			instruccion.sprintf("SELECT \
				COUNT(d.id) AS numcli \
			FROM clientesasociado c \
			INNER JOIN dclientesasociado d ON d.id = c.id \
			WHERE c.cliente = '%s'", cliente);
			mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}

	}
	__finally {

		delete buffer_sql;
	}

}
// ---------------------------------------------------------------------------
//ID_CON_EMPLEADOS_ASOCIADOS
void ServidorCatalogos::ConsultaEmpleadosAsociados(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    AnsiString instruccion;
	AnsiString cliente;

	cliente = mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos de las cancelaciones
	instruccion.sprintf("SELECT dc.id, dc.empleado, dc.puesto, \
	dc.departamento, dc.codbarras, c.fechavigencia \
	FROM clientesasociado c \
	INNER JOIN dclientesasociado dc ON dc.id = c.id \
	WHERE c.cliente = '%s' \
	ORDER BY dc.empleado ",
	cliente);

	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

}
// ---------------------------------------------------------------------------
//ID_BAJA_EMPLEADOS_ASOCIADOS
void ServidorCatalogos::BajaEmpleadosAsociados(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*10];
	char *aux_buffer_sql=buffer_sql;
	BufferRespuestas* resp=NULL;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[10];
	AnsiString id;

	id = mFg.ExtraeStringDeBuffer(&parametros);

	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	try {
		Instruccion.sprintf("DELETE FROM dclientesasociado WHERE id = %s ", id);
		instrucciones[num_instrucciones++]=Instruccion;

		instrucciones[num_instrucciones++]="COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);

	} __finally {
		if(buffer_sql != NULL)
			delete buffer_sql;
	}

}
// ---------------------------------------------------------------------------
//ID_ELIMINA_ARTICULO
void ServidorCatalogos::EliminarArticulo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	BufferRespuestas* resp_producto=NULL;
	BufferRespuestas* resp_verificacion=NULL;
	BufferRespuestas* resp_verificacion_comp=NULL;
	BufferRespuestas* resp_verificacion_pedvta=NULL;
	BufferRespuestas* resp_verificacion_ped=NULL;
	BufferRespuestas* resp_verificacion_ped_vnt_sin_ex=NULL;
	BufferRespuestas* resp_verificacion_vnt_kits=NULL;
	BufferRespuestas* resp_verificacion_invent=NULL;
	BufferRespuestas* resp_verificacion_movalma=NULL;
	BufferRespuestas* resp_verificacion_notas_cred_cli=NULL;
	BufferRespuestas* resp_verificacion_notas_cred_cli_varias=NULL;
	BufferRespuestas* resp_verificacion_notas_cred_prov=NULL;
	BufferRespuestas* resp_verificacion_pedidos_kit=NULL;
	BufferRespuestas* resp_verificacion_articulos_pedidos_sinexistencia=NULL;
	BufferRespuestas* resp_verificacion_kits=NULL;
	BufferRespuestas* resp_verificacion_pedidos_ecomm_surtido=NULL;
	BufferRespuestas* resp_verificacion_pedidos_ecomm=NULL;
    BufferRespuestas* resp_verificacion_recepcion=NULL;
	BufferRespuestas* resp_verificacion_recepcion_devol=NULL;
	BufferRespuestas* resp_verificacion_cartas_porte20=NULL;
	BufferRespuestas* resp_verificacion_cartas_part=NULL;
	BufferRespuestas* resp_verificacion_ecomm_detalle=NULL;
	BufferRespuestas* resp_verificacion_resurtir_anaquel=NULL;
	BufferRespuestas* resp_factor_reemp=NULL;
	BufferRespuestas* resp_multiplo_reemp=NULL;

	char *buffer_sql=new char[1024*1024*128];
	char *aux_buffer_sql=buffer_sql;

	char * buffer_funciones = new char[1024];
    char * puntero_funciones = buffer_funciones;

	BufferRespuestas* resp=NULL;
	int num_instrucciones=0, i;
	AnsiString Instruccion;
	AnsiString* instrucciones = new AnsiString[100000];
	AnsiString articulo, producto, present, multiplo, factor, multiplo_reemp;
    AnsiString idProgramado, usuario, suc_prog, fecha_prog;

	AnsiString idEmpresa = " ";

	articulo = mFg.ExtraeStringDeBuffer(&parametros);
	producto = mFg.ExtraeStringDeBuffer(&parametros);
	present = mFg.ExtraeStringDeBuffer(&parametros);
	multiplo = mFg.ExtraeStringDeBuffer(&parametros);
	multiplo_reemp = mFg.ExtraeStringDeBuffer(&parametros);
	idProgramado = mFg.ExtraeStringDeBuffer(&parametros);
	usuario = mFg.ExtraeStringDeBuffer(&parametros);
	suc_prog = mFg.ExtraeStringDeBuffer(&parametros);
	fecha_prog = mFg.ExtraeStringDeBuffer(&parametros);


	BufferRespuestas * respuesta_empresas = NULL;

	instrucciones[num_instrucciones++]="SET SESSION sql_log_bin = 0";
	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	try {

		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, "SELECT idempresa FROM empresas", respuesta_empresas);

		idEmpresa = FormServidor->ObtieneClaveEmpresa();

		Instruccion.sprintf("SELECT @articulo_borrar:=a.articulo, \
		@producto:=a.producto, @present:=a.present, @multiplo:=a.multiplo \
		FROM articulos a \
		WHERE a.articulo='%s' AND a.factor=1 ", articulo);
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("SELECT @min_factor_reemplazo:=MIN(a.factor) \
		FROM articulos a \
		WHERE a.producto=@producto AND a.present=@present and a.factor<>1 ");
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("SELECT @articulo_reemplazo:=a.articulo, @factor_reemplazo:=a.factor, \
		@multiplo_reemplazo:=a.multiplo \
		FROM articulos a \
		WHERE a.producto=@producto AND a.present=@present AND a.factor=@min_factor_reemplazo ");
		instrucciones[num_instrucciones++]=Instruccion;

		/****************************************************
		*                                                   *
		*                                                   *
		*                                                   *
		*	INICIO UPDATES Y DELETES DE TABLAS DE DETALLES  *
		*                                                   *
		*                                                   *
		*                                                   *
		****************************************************/

		try {
			Instruccion.sprintf("SELECT t.referencia, t.numRef, t.producto, t.present, t.multiplo, t.tipo \
			FROM (( \
			SELECT \
				dv.referencia, \
				COUNT(dv.referencia) AS numRef, \
				a.producto, \
				a.present, \
				a.multiplo, \
				if(COUNT(a.multiplo) > 1, 'MULTIPLE', 'UNICO') AS tipo \
			FROM dventas dv \
			INNER JOIN articulos a ON a.articulo = dv.articulo \
			WHERE a.producto = '%s' AND a.present = '%s' \
				AND a.multiplo IN('%s', '%s') \
			GROUP BY dv.referencia, a.producto, a.present \
			HAVING COUNT(dv.referencia) > 1 \
			) UNION ALL ( \
			SELECT \
				dv.referencia, \
				COUNT(dv.referencia) AS numRef, \
				a.producto, \
				a.present, \
				a.multiplo, \
				if(COUNT(a.multiplo) = 1, 'UNICO', 'N/A') AS tipo \
			FROM dventas dv \
			INNER JOIN articulos a ON a.articulo = dv.articulo \
			WHERE a.producto = '%s' AND a.present = '%s' \
				AND a.multiplo IN('%s', '%s') \
			GROUP BY dv.referencia, a.producto, a.present, a.multiplo \
			HAVING COUNT(dv.referencia) = 1 AND multiplo != '%s')) t \
			GROUP BY t.referencia, t.producto, t.present \
			ORDER BY t.tipo ",
			producto, present, multiplo, multiplo_reemp,
			producto, present, multiplo, multiplo_reemp,
			multiplo_reemp);
			if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), resp_verificacion)) {
				if (resp_verificacion->ObtieneNumRegistros()>0){

					instrucciones[num_instrucciones++]="DROP TABLE IF EXISTS tmpdventas";
					Instruccion.sprintf("CREATE TEMPORARY TABLE tmpdventas ( \
					referencia VARCHAR(11), \
					articulo VARCHAR(9), \
					producto VARCHAR(8), \
					present VARCHAR(255), \
					multiplo VARCHAR(10), \
					cantidadsumar DECIMAL(12, 3), \
					costo_sumar DECIMAL(12, 3), \
					precio_sumar DECIMAL(12, 3), \
					precioimp_sumar DECIMAL(12, 3), \
					INDEX(articulo) \
					)ENGINE=InnoDB");
					instrucciones[num_instrucciones++]=Instruccion;

					instrucciones[num_instrucciones++]="DROP TABLE IF EXISTS tmpdventasconv";
					Instruccion.sprintf("CREATE TEMPORARY TABLE tmpdventasconv ( \
					referencia VARCHAR(11), \
					articulo VARCHAR(9), \
					cantidadsumar DECIMAL(12, 6), \
					costo_sumar DECIMAL(12, 6), \
					precio_sumar DECIMAL(12, 6), \
					precioimp_sumar DECIMAL(12, 6), \
					INDEX(articulo) \
					)ENGINE=INNODB");
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("INSERT INTO tmpdventas \
					SELECT \
						dv.referencia, \
						@articulo_reemplazo, \
						a.producto, \
						a.present, \
						@multiplo_reemplazo, \
						IF(a.multiplo=@multiplo_reemplazo,SUM(IF(a.multiplo=@multiplo_reemplazo, 0, (dv.cantidad/@min_factor_reemplazo))),(dv.cantidad/@min_factor_reemplazo)) AS cantidad_sumar,\
						(dv.costobase*@factor_reemplazo) AS costo_sumar, \
						SUM(dv.precio*dv.cantidad) AS precio_sumar, \
						SUM(dv.precioimp*dv.cantidad) AS precioimp_sumar \
					FROM dventas dv \
					INNER JOIN articulos a ON a.articulo = dv.articulo \
					WHERE a.producto = @producto AND a.present = @present \
						AND a.multiplo IN(@multiplo, @multiplo_reemplazo) \
					GROUP BY dv.referencia, a.producto, a.present \
					HAVING COUNT(dv.referencia) > 1");
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("INSERT INTO tmpdventasconv \
					SELECT \
						dv.referencia, \
						dv.articulo, \
						(dv.cantidad+IFNULL(tmpdv.cantidadsumar,0)) AS cantidad, \
						tmpdv.costo_sumar AS costo, \
						((dv.precio*tmpdv.cantidadsumar)/(dv.cantidad+ IFNULL(tmpdv.cantidadsumar,0))) AS precio, \
						((dv.precioimp*tmpdv.cantidadsumar)/(dv.cantidad+ IFNULL(tmpdv.cantidadsumar,0))) AS precioimp \
					FROM dventas dv \
					INNER JOIN tmpdventas tmpdv ON tmpdv.referencia = dv.referencia AND dv.articulo = tmpdv.articulo ");
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("UPDATE dventas dv \
					INNER JOIN tmpdventas tmpdv ON tmpdv.referencia = dv.referencia AND dv.articulo = tmpdv.articulo \
					SET \
					dv.cantidad = (dv.cantidad+IFNULL(tmpdv.cantidadsumar,0)), \
					dv.costobase = tmpdv.costo_sumar, \
					dv.precio = (tmpdv.precio_sumar/(dv.cantidad+tmpdv.cantidadsumar)), \
					dv.precioimp = (tmpdv.precioimp_sumar/(dv.cantidad+tmpdv.cantidadsumar)) ");
					instrucciones[num_instrucciones++]=Instruccion;

					for (int i = 0; i < resp_verificacion->ObtieneNumRegistros(); i++) {
						resp_verificacion->IrAlRegistroNumero(i);

						AnsiString ref_eliminar = "";
						ref_eliminar = resp_verificacion->ObtieneDato("referencia");

						if(resp_verificacion->ObtieneDato("tipo")=="MULTIPLE"){

							Instruccion.sprintf("DELETE FROM dventascfdi WHERE articulo = @articulo_borrar");
							instrucciones[num_instrucciones++]=Instruccion;

							Instruccion.sprintf("DELETE FROM dventas WHERE referencia = '%s' \
							AND articulo = @articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;
						} else {

							Instruccion.sprintf("UPDATE dventas dv \
							SET dv.articulo=@articulo_reemplazo, dv.cantidad=dv.cantidad/@factor_reemplazo, \
							dv.costobase=dv.costobase*@factor_reemplazo, \
							dv.precio=dv.precio*@factor_reemplazo, dv.precioimp=dv.precioimp*@factor_reemplazo \
							WHERE dv.referencia = '%s' AND dv.articulo=@articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;

						}

						if(num_instrucciones>=100000-100)
							throw Exception("El número de instrucciones sale de los límites en Eliminar artículo");

					}

				}
			}
		} __finally {
			if (resp_verificacion!=NULL) delete resp_verificacion;
		}

        try {
			Instruccion.sprintf("SELECT t.referencia, t.numRef, t.producto, t.present, t.multiplo, t.tipo \
			FROM (( \
			SELECT \
				dc.referencia, \
				COUNT(dc.referencia) AS numRef, \
				a.producto, \
				a.present, \
				a.multiplo, \
				if(COUNT(a.multiplo) > 1, 'MULTIPLE', 'UNICO') AS tipo \
			FROM dcompras dc \
			INNER JOIN articulos a ON a.articulo = dc.articulo \
			WHERE a.producto = '%s' AND a.present = '%s' \
				AND a.multiplo IN('%s', '%s') \
			GROUP BY dc.referencia, a.producto, a.present \
			HAVING COUNT(dc.referencia) > 1 \
			) UNION ALL ( \
			SELECT \
				dc.referencia, \
				COUNT(dc.referencia) AS numRef, \
				a.producto, \
				a.present, \
				a.multiplo, \
				if(COUNT(a.multiplo) = 1, 'UNICO', 'N/A') AS tipo \
			FROM dcompras dc \
			INNER JOIN articulos a ON a.articulo = dc.articulo \
			WHERE a.producto = '%s' AND a.present = '%s' \
				AND a.multiplo IN('%s', '%s') \
			GROUP BY dc.referencia, a.producto, a.present, a.multiplo \
			HAVING COUNT(dc.referencia) = 1 AND multiplo != '%s')) t \
			GROUP BY t.referencia, t.producto, t.present \
			ORDER BY t.tipo ",
			producto, present, multiplo, multiplo_reemp,
			producto, present, multiplo, multiplo_reemp,
			multiplo_reemp);
			if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), resp_verificacion_comp)) {
				if (resp_verificacion_comp->ObtieneNumRegistros()>0){

					instrucciones[num_instrucciones++]="DROP TABLE IF EXISTS tmpdcompras";
					Instruccion.sprintf("CREATE TEMPORARY TABLE tmpdcompras ( \
					referencia VARCHAR(11), \
					articulo VARCHAR(9), \
					producto VARCHAR(8), \
					present VARCHAR(255), \
					multiplo VARCHAR(10), \
					cantidadsumar DECIMAL(12, 3), \
					costo_sumar DECIMAL(12, 3), \
					costoimp_sumar DECIMAL(12, 3), \
					iepscuota_sumar DECIMAL(12, 3), \
					INDEX(articulo) \
					)ENGINE=InnoDB");
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("INSERT INTO tmpdcompras \
					SELECT \
						dc.referencia, \
						@articulo_reemplazo, \
						a.producto, \
						a.present, \
						@multiplo_reemplazo, \
						IF(a.multiplo=@multiplo_reemplazo,SUM(IF(a.multiplo=@multiplo_reemplazo, 0, (dc.cantidad/@min_factor_reemplazo))),(dc.cantidad/@min_factor_reemplazo)) AS cantidad_sumar,\
						SUM(dc.costo*dc.cantidad) AS costo_sumar, \
						SUM(dc.costoimp*dc.cantidad) AS precio_sumar, \
						SUM(dc.iepscuota*dc.cantidad) AS iepscuota_sumar \
					FROM dcompras dc \
					INNER JOIN articulos a ON a.articulo = dc.articulo \
					WHERE a.producto = @producto AND a.present = @present \
						AND a.multiplo IN(@multiplo, @multiplo_reemplazo) \
					GROUP BY dc.referencia, a.producto, a.present \
					HAVING COUNT(dc.referencia) > 1");
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("UPDATE dcompras dc \
					INNER JOIN tmpdcompras tmpdc ON tmpdc.referencia = dc.referencia AND dc.articulo = tmpdc.articulo \
					SET \
					dc.cantidad = (dc.cantidad+IFNULL(tmpdc.cantidadsumar,0)), \
					dc.costo = (tmpdc.costo_sumar/(dc.cantidad+tmpdc.cantidadsumar)), \
					dc.costoimp = (tmpdc.costoimp_sumar/(dc.cantidad+tmpdc.cantidadsumar)), \
					dc.iepscuota = (tmpdc.iepscuota_sumar/(dc.cantidad+tmpdc.cantidadsumar)) ");
					instrucciones[num_instrucciones++]=Instruccion;

					for (int i = 0; i < resp_verificacion_comp->ObtieneNumRegistros(); i++) {
						resp_verificacion_comp->IrAlRegistroNumero(i);

						AnsiString ref_eliminar = "";
						ref_eliminar = resp_verificacion_comp->ObtieneDato("referencia");

						if(resp_verificacion_comp->ObtieneDato("tipo")=="MULTIPLE"){

							Instruccion.sprintf("DELETE FROM dcompras WHERE referencia = '%s' \
							AND articulo = @articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;
						} else {

							Instruccion.sprintf("UPDATE dcompras dc \
							SET dc.articulo=@articulo_reemplazo, dc.cantidad=dc.cantidad/@factor_reemplazo, \
							dc.costo=dc.costo*@factor_reemplazo, dc.costoimp=dc.costoimp*@factor_reemplazo, \
							dc.iepscuota=dc.iepscuota*@factor_reemplazo \
							WHERE dc.referencia = '%s' AND dc.articulo=@articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;

						}

						if(num_instrucciones>=100000-100)
							throw Exception("El número de instrucciones sale de los límites en Eliminar artículo");

					}

				}
			}
		} __finally {
			if (resp_verificacion_comp!=NULL) delete resp_verificacion_comp;
		}

		try {
			Instruccion.sprintf("SELECT t.referencia, t.numRef, t.producto, t.present, t.multiplo, t.tipo \
			FROM (( \
			SELECT \
				d.referencia, \
				COUNT(d.referencia) AS numRef, \
				a.producto, \
				a.present, \
				a.multiplo, \
				if(COUNT(a.multiplo) > 1, 'MULTIPLE', 'UNICO') AS tipo \
			FROM dpedidosventa d \
			INNER JOIN articulos a ON a.articulo = d.articulo \
			WHERE a.producto = '%s' AND a.present = '%s' \
				AND a.multiplo IN('%s', '%s') \
			GROUP BY d.referencia, a.producto, a.present \
			HAVING COUNT(d.referencia) > 1 \
			) UNION ALL ( \
			SELECT \
				d.referencia, \
				COUNT(d.referencia) AS numRef, \
				a.producto, \
				a.present, \
				a.multiplo, \
				if(COUNT(a.multiplo) = 1, 'UNICO', 'N/A') AS tipo \
			FROM dpedidosventa d \
			INNER JOIN articulos a ON a.articulo = d.articulo \
			WHERE a.producto = '%s' AND a.present = '%s' \
				AND a.multiplo IN('%s', '%s') \
			GROUP BY d.referencia, a.producto, a.present, a.multiplo \
			HAVING COUNT(d.referencia) = 1 AND multiplo != '%s')) t \
			GROUP BY t.referencia, t.producto, t.present \
			ORDER BY t.tipo ",
			producto, present, multiplo, multiplo_reemp,
			producto, present, multiplo, multiplo_reemp,
			multiplo_reemp);
			if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), resp_verificacion_pedvta)) {
				if (resp_verificacion_pedvta->ObtieneNumRegistros()>0){

					instrucciones[num_instrucciones++]="DROP TABLE IF EXISTS tmpdpedidosventa";
					Instruccion.sprintf("CREATE TEMPORARY TABLE tmpdpedidosventa ( \
					referencia VARCHAR(11), \
					articulo VARCHAR(9), \
					producto VARCHAR(8), \
					present VARCHAR(255), \
					multiplo VARCHAR(10), \
					cantidadsumar DECIMAL(12, 3), \
					precio_sumar DECIMAL(12, 3), \
					precioimp_sumar DECIMAL(12, 3), \
					INDEX(articulo) \
					)ENGINE=InnoDB");
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("INSERT INTO tmpdpedidosventa \
					SELECT \
						d.referencia, \
						@articulo_reemplazo, \
						a.producto, \
						a.present, \
						@multiplo_reemplazo, \
						IF(a.multiplo=@multiplo_reemplazo,SUM(IF(a.multiplo=@multiplo_reemplazo, 0, (d.cantidad/@min_factor_reemplazo))),(d.cantidad/@min_factor_reemplazo)) AS cantidad_sumar,\
						SUM(d.precio*d.cantidad) AS costo_sumar, \
						SUM(d.precioimp*d.cantidad) AS precio_sumar \
					FROM dpedidosventa d \
					INNER JOIN articulos a ON a.articulo = d.articulo \
					WHERE a.producto = @producto AND a.present = @present \
						AND a.multiplo IN(@multiplo, @multiplo_reemplazo) \
					GROUP BY d.referencia, a.producto, a.present \
					HAVING COUNT(d.referencia) > 1");
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("UPDATE dpedidosventa d \
					INNER JOIN tmpdpedidosventa tmpd ON tmpd.referencia = d.referencia AND d.articulo = tmpd.articulo \
					SET \
					d.cantidad = (d.cantidad+IFNULL(tmpd.cantidadsumar,0)), \
					d.precio = (tmpd.precio_sumar/(d.cantidad+tmpd.cantidadsumar)), \
					d.precioimp = (tmpd.precioimp_sumar/(d.cantidad+tmpd.cantidadsumar)) ");
					instrucciones[num_instrucciones++]=Instruccion;

					for (int i = 0; i < resp_verificacion_pedvta->ObtieneNumRegistros(); i++) {
						resp_verificacion_pedvta->IrAlRegistroNumero(i);

						AnsiString ref_eliminar = "";
						ref_eliminar = resp_verificacion_pedvta->ObtieneDato("referencia");

						if(resp_verificacion_pedvta->ObtieneDato("tipo")=="MULTIPLE"){

							Instruccion.sprintf("DELETE FROM dpedidosventa WHERE referencia = '%s' \
							AND articulo = @articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;
						} else {

							Instruccion.sprintf("UPDATE dpedidosventa d \
							SET d.articulo=@articulo_reemplazo, d.cantidad=d.cantidad/@factor_reemplazo, \
							d.precio=d.precio*@factor_reemplazo, d.precioimp=d.precioimp*@factor_reemplazo \
							WHERE d.referencia = '%s' AND d.articulo=@articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;

						}

						if(num_instrucciones>=100000-100)
							throw Exception("El número de instrucciones sale de los límites en Eliminar artículo");

					}

				}
			}
		} __finally {
			if (resp_verificacion_pedvta!=NULL) delete resp_verificacion_pedvta;
		}

		try {
			Instruccion.sprintf("SELECT t.referencia, t.numRef, t.producto, t.present, t.multiplo, t.tipo \
			FROM (( \
			SELECT \
				d.referencia, \
				COUNT(d.referencia) AS numRef, \
				a.producto, \
				a.present, \
				a.multiplo, \
				if(COUNT(a.multiplo) > 1, 'MULTIPLE', 'UNICO') AS tipo \
			FROM dpedidos d \
			INNER JOIN articulos a ON a.articulo = d.articulo \
			WHERE a.producto = '%s' AND a.present = '%s' \
				AND a.multiplo IN('%s', '%s') \
			GROUP BY d.referencia, a.producto, a.present \
			HAVING COUNT(d.referencia) > 1 \
			) UNION ALL ( \
			SELECT \
				d.referencia, \
				COUNT(d.referencia) AS numRef, \
				a.producto, \
				a.present, \
				a.multiplo, \
				if(COUNT(a.multiplo) = 1, 'UNICO', 'N/A') AS tipo \
			FROM dpedidos d \
			INNER JOIN articulos a ON a.articulo = d.articulo \
			WHERE a.producto = '%s' AND a.present = '%s' \
				AND a.multiplo IN('%s', '%s') \
			GROUP BY d.referencia, a.producto, a.present, a.multiplo \
			HAVING COUNT(d.referencia) = 1 AND multiplo != '%s')) t \
			GROUP BY t.referencia, t.producto, t.present \
			ORDER BY t.tipo ",
			producto, present, multiplo, multiplo_reemp,
			producto, present, multiplo, multiplo_reemp,
			multiplo_reemp);
			if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), resp_verificacion_ped)) {
				if (resp_verificacion_ped->ObtieneNumRegistros()>0){

					instrucciones[num_instrucciones++]="DROP TABLE IF EXISTS tmpdpedidos";
					Instruccion.sprintf("CREATE TEMPORARY TABLE tmpdpedidos ( \
					referencia VARCHAR(11), \
					articulo VARCHAR(9), \
					producto VARCHAR(8), \
					present VARCHAR(255), \
					multiplo VARCHAR(10), \
					cantidadsumar DECIMAL(12, 3), \
					costo_sumar DECIMAL(12, 3), \
					costoimp_sumar DECIMAL(12, 3), \
					iepscuota_sumar DECIMAL(12, 3), \
					INDEX(articulo) \
					)ENGINE=InnoDB");
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("INSERT INTO tmpdpedidos \
					SELECT \
						d.referencia, \
						@articulo_reemplazo, \
						a.producto, \
						a.present, \
						@multiplo_reemplazo, \
						IF(a.multiplo=@multiplo_reemplazo,SUM(IF(a.multiplo=@multiplo_reemplazo, 0, (d.cantidad/@min_factor_reemplazo))),(d.cantidad/@min_factor_reemplazo)) AS cantidad_sumar,\
						SUM(d.costo*d.cantidad) AS costo_sumar, \
						SUM(d.costoimp*d.cantidad) AS costoimp_sumar, \
						SUM(d.iepscuota*d.cantidad) AS iepscuota_sumar \
					FROM dpedidos d \
					INNER JOIN articulos a ON a.articulo = d.articulo \
					WHERE a.producto = @producto AND a.present = @present \
						AND a.multiplo IN(@multiplo, @multiplo_reemplazo) \
					GROUP BY d.referencia, a.producto, a.present \
					HAVING COUNT(d.referencia) > 1");
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("UPDATE dpedidos d \
					INNER JOIN tmpdpedidos tmpd ON tmpd.referencia = d.referencia AND d.articulo = tmpd.articulo \
					SET \
					d.cantidad = (d.cantidad+IFNULL(tmpd.cantidadsumar,0)), \
					d.costo = (tmpd.costo_sumar/(d.cantidad+tmpd.cantidadsumar)), \
					d.costoimp = (tmpd.costoimp_sumar/(d.cantidad+tmpd.cantidadsumar)), \
					d.iepscuota = (tmpd.iepscuota_sumar/(d.cantidad+tmpd.cantidadsumar)) ");
					instrucciones[num_instrucciones++]=Instruccion;

					for (int i = 0; i < resp_verificacion_ped->ObtieneNumRegistros(); i++) {
						resp_verificacion_ped->IrAlRegistroNumero(i);

						AnsiString ref_eliminar = "";
						ref_eliminar = resp_verificacion_ped->ObtieneDato("referencia");

						if(resp_verificacion_ped->ObtieneDato("tipo")=="MULTIPLE"){

							Instruccion.sprintf("DELETE FROM dpedidos WHERE referencia = '%s' \
							AND articulo = @articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;
						} else {

							Instruccion.sprintf("UPDATE dpedidos d \
							SET d.articulo=@articulo_reemplazo, d.cantidad=d.cantidad/@factor_reemplazo, \
							d.costo=d.costo*@factor_reemplazo, d.costoimp=d.costoimp*@factor_reemplazo, \
							d.iepscuota=d.iepscuota*@factor_reemplazo \
							WHERE d.referencia = '%s' AND d.articulo=@articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;

						}

						if(num_instrucciones>=100000-100)
							throw Exception("El número de instrucciones sale de los límites en Eliminar artículo");

					}

				}
			}
		} __finally {
			if (resp_verificacion_ped!=NULL) delete resp_verificacion_ped;
		}

		try {
			Instruccion.sprintf("SELECT t.referencia, t.numRef, t.producto, t.present, t.multiplo, t.tipo \
			FROM (( \
			SELECT \
				d.referencia, \
				COUNT(d.referencia) AS numRef, \
				a.producto, \
				a.present, \
				a.multiplo, \
				if(COUNT(a.multiplo) > 1, 'MULTIPLE', 'UNICO') AS tipo \
			FROM dpedidosventasinexistencia d \
			INNER JOIN articulos a ON a.articulo = d.articulo \
			WHERE a.producto = '%s' AND a.present = '%s' \
				AND a.multiplo IN('%s', '%s') \
			GROUP BY d.referencia, a.producto, a.present \
			HAVING COUNT(d.referencia) > 1 \
			) UNION ALL ( \
			SELECT \
				d.referencia, \
				COUNT(d.referencia) AS numRef, \
				a.producto, \
				a.present, \
				a.multiplo, \
				if(COUNT(a.multiplo) = 1, 'UNICO', 'N/A') AS tipo \
			FROM dpedidosventasinexistencia d \
			INNER JOIN articulos a ON a.articulo = d.articulo \
			WHERE a.producto = '%s' AND a.present = '%s' \
				AND a.multiplo IN('%s', '%s') \
			GROUP BY d.referencia, a.producto, a.present, a.multiplo \
			HAVING COUNT(d.referencia) = 1 AND multiplo != '%s')) t \
			GROUP BY t.referencia, t.producto, t.present \
			ORDER BY t.tipo ",
			producto, present, multiplo, multiplo_reemp,
			producto, present, multiplo, multiplo_reemp,
			multiplo_reemp);
			if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), resp_verificacion_ped_vnt_sin_ex)) {
				if (resp_verificacion_ped_vnt_sin_ex->ObtieneNumRegistros()>0){

					instrucciones[num_instrucciones++]="DROP TABLE IF EXISTS tmpdpedidosventasinexistencia";
					Instruccion.sprintf("CREATE TEMPORARY TABLE tmpdpedidosventasinexistencia ( \
					referencia VARCHAR(11), \
					articulo VARCHAR(9), \
					producto VARCHAR(8), \
					present VARCHAR(255), \
					multiplo VARCHAR(10), \
					cantidadsumar DECIMAL(12, 3), \
					precio_sumar DECIMAL(12, 3), \
					precioimp_sumar DECIMAL(12, 3), \
					INDEX(articulo) \
					)ENGINE=InnoDB");
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("INSERT INTO tmpdpedidosventasinexistencia \
					SELECT \
						d.referencia, \
						@articulo_reemplazo, \
						a.producto, \
						a.present, \
						@multiplo_reemplazo, \
						IF(a.multiplo=@multiplo_reemplazo,SUM(IF(a.multiplo=@multiplo_reemplazo, 0, (d.cantidad/@min_factor_reemplazo))),(d.cantidad/@min_factor_reemplazo)) AS cantidad_sumar, \
						(d.precio/d.cantidad) AS precio_sumar, \
						(d.precioimp/d.cantidad) AS precioimp_sumar \
					FROM dpedidosventasinexistencia d \
					INNER JOIN articulos a ON a.articulo = d.articulo \
					WHERE a.producto = @producto AND a.present = @present \
						AND a.multiplo IN(@multiplo, @multiplo_reemplazo) \
					GROUP BY d.referencia, a.producto, a.present \
					HAVING COUNT(d.referencia) > 1");
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("UPDATE dpedidosventasinexistencia d \
					INNER JOIN tmpdpedidosventasinexistencia tmpd ON tmpd.referencia = d.referencia AND d.articulo = tmpd.articulo \
					SET \
					d.cantidad = (d.cantidad+IFNULL(tmpd.cantidadsumar,0)), \
					d.precio = (tmpd.precio_sumar/(d.cantidad+tmpd.cantidadsumar)), \
					d.precioimp = (tmpd.precioimp_sumar/(d.cantidad+tmpd.cantidadsumar)) ");
					instrucciones[num_instrucciones++]=Instruccion;

					for (int i = 0; i < resp_verificacion_ped_vnt_sin_ex->ObtieneNumRegistros(); i++) {
						resp_verificacion_ped_vnt_sin_ex->IrAlRegistroNumero(i);

						AnsiString ref_eliminar = "";
						ref_eliminar = resp_verificacion_ped_vnt_sin_ex->ObtieneDato("referencia");

						if(resp_verificacion_ped_vnt_sin_ex->ObtieneDato("tipo")=="MULTIPLE"){

							Instruccion.sprintf("DELETE FROM dpedidosventasinexistencia WHERE referencia = '%s' \
							AND articulo = @articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;
						} else {

							Instruccion.sprintf("UPDATE dpedidosventasinexistencia d \
							SET d.articulo=@articulo_reemplazo, d.cantidad=d.cantidad/@factor_reemplazo, \
							d.precio=d.precio*@factor_reemplazo, d.precioimp=d.precioimp*@factor_reemplazo \
							WHERE d.referencia = '%s' AND d.articulo=@articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;

						}

						if(num_instrucciones>=100000-100)
							throw Exception("El número de instrucciones sale de los límites en Eliminar artículo");

					}

				}
			}
		} __finally {
			if (resp_verificacion_ped_vnt_sin_ex!=NULL) delete resp_verificacion_ped_vnt_sin_ex;
		}

		try {
			Instruccion.sprintf("SELECT t.referencia, t.numRef, t.producto, t.present, t.multiplo, t.tipo \
			FROM (( \
			SELECT \
				d.kit AS referencia, \
				COUNT(d.kit) AS numRef, \
				a.producto, \
				a.present, \
				a.multiplo, \
				if(COUNT(a.multiplo) > 1, 'MULTIPLE', 'UNICO') AS tipo \
			FROM dventaskits d \
			INNER JOIN articulos a ON a.articulo = d.articulo \
			WHERE a.producto = '%s' AND a.present = '%s' \
				AND a.multiplo IN('%s', '%s') \
			GROUP BY d.kit, a.producto, a.present \
			HAVING COUNT(d.kit) > 1 \
			) UNION ALL ( \
			SELECT \
				d.kit AS referencia, \
				COUNT(d.kit) AS numRef, \
				a.producto, \
				a.present, \
				a.multiplo, \
				if(COUNT(a.multiplo) = 1, 'UNICO', 'N/A') AS tipo \
			FROM dventaskits d \
			INNER JOIN articulos a ON a.articulo = d.articulo \
			WHERE a.producto = '%s' AND a.present = '%s' \
				AND a.multiplo IN('%s', '%s') \
			GROUP BY d.kit, a.producto, a.present, a.multiplo \
			HAVING COUNT(d.kit) = 1 AND multiplo != '%s')) t \
			GROUP BY t.referencia, t.producto, t.present \
			ORDER BY t.tipo ",
			producto, present, multiplo, multiplo_reemp,
			producto, present, multiplo, multiplo_reemp,
			multiplo_reemp);
			if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), resp_verificacion_vnt_kits)) {
				if (resp_verificacion_vnt_kits->ObtieneNumRegistros()>0){

					instrucciones[num_instrucciones++]="DROP TABLE IF EXISTS tmpdventaskits";
					Instruccion.sprintf("CREATE TEMPORARY TABLE tmpdventaskits ( \
					referencia VARCHAR(11), \
					articulo VARCHAR(9), \
					producto VARCHAR(8), \
					present VARCHAR(255), \
					multiplo VARCHAR(10), \
					cantidadsumar DECIMAL(12, 3), \
					precio_sumar DECIMAL(12, 3), \
					INDEX(articulo) \
					)ENGINE=InnoDB");
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("INSERT INTO tmpdventaskits \
					SELECT \
						d.kit AS referencia, \
						@articulo_reemplazo, \
						a.producto, \
						a.present, \
						@multiplo_reemplazo, \
						IF(a.multiplo=@multiplo_reemplazo,SUM(IF(a.multiplo=@multiplo_reemplazo, 0, (d.cantidad/@min_factor_reemplazo))),(d.cantidad/@min_factor_reemplazo)) AS cantidad_sumar, \
						SUM(d.precio*d.cantidad) AS precio_sumar \
					FROM dventaskits d \
					INNER JOIN articulos a ON a.articulo = d.articulo \
					WHERE a.producto = @producto AND a.present = @present \
						AND a.multiplo IN(@multiplo, @multiplo_reemplazo) \
					GROUP BY d.kit, a.producto, a.present \
					HAVING COUNT(d.kit) > 1");
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("UPDATE dventaskits d \
					INNER JOIN tmpdventaskits tmpd ON tmpd.referencia = d.kit AND d.articulo = tmpd.articulo \
					SET \
					d.cantidad = (d.cantidad+IFNULL(tmpd.cantidadsumar,0)), \
					d.precio = (tmpd.precio_sumar/(d.cantidad+tmpd.cantidadsumar)) ");
					instrucciones[num_instrucciones++]=Instruccion;

					for (int i = 0; i < resp_verificacion_vnt_kits->ObtieneNumRegistros(); i++) {
						resp_verificacion_vnt_kits->IrAlRegistroNumero(i);

						AnsiString ref_eliminar = "";
						ref_eliminar = resp_verificacion_vnt_kits->ObtieneDato("referencia");

						if(resp_verificacion_vnt_kits->ObtieneDato("tipo")=="MULTIPLE"){

							Instruccion.sprintf("DELETE FROM dventaskits WHERE kit = '%s' \
							AND articulo = @articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;
						} else {

							Instruccion.sprintf("UPDATE dventaskits d \
							SET d.articulo=@articulo_reemplazo, d.cantidad=d.cantidad/@factor_reemplazo, \
							d.precio=d.precio*@factor_reemplazo \
							WHERE d.kit = '%s' AND d.articulo=@articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;

						}

						if(num_instrucciones>=100000-100)
							throw Exception("El número de instrucciones sale de los límites en Eliminar artículo");

					}

				}
			}
		} __finally {
			if (resp_verificacion_vnt_kits!=NULL) delete resp_verificacion_vnt_kits;
		}

		try {
			Instruccion.sprintf("SELECT t.referencia, t.numRef, t.producto, t.present, t.multiplo, t.tipo \
			FROM (( \
			SELECT \
				d.inventario AS referencia, \
				COUNT(d.inventario) AS numRef, \
				a.producto, \
				a.present, \
				a.multiplo, \
				if(COUNT(a.multiplo) > 1, 'MULTIPLE', 'UNICO') AS tipo \
			FROM dinventarios d \
			INNER JOIN articulos a ON a.articulo = d.articulo \
			WHERE a.producto = '%s' AND a.present = '%s' \
				AND a.multiplo IN('%s', '%s') \
			GROUP BY d.inventario, a.producto, a.present \
			HAVING COUNT(d.inventario) > 1 \
			) UNION ALL ( \
			SELECT \
				d.inventario AS referencia, \
				COUNT(d.inventario) AS numRef, \
				a.producto, \
				a.present, \
				a.multiplo, \
				if(COUNT(a.multiplo) = 1, 'UNICO', 'N/A') AS tipo \
			FROM dinventarios d \
			INNER JOIN articulos a ON a.articulo = d.articulo \
			WHERE a.producto = '%s' AND a.present = '%s' \
				AND a.multiplo IN('%s', '%s') \
			GROUP BY d.inventario, a.producto, a.present, a.multiplo \
			HAVING COUNT(d.inventario) = 1 AND multiplo != '%s')) t \
			GROUP BY t.referencia, t.producto, t.present \
			ORDER BY t.tipo ",
			producto, present, multiplo, multiplo_reemp,
			producto, present, multiplo, multiplo_reemp,
			multiplo_reemp);
			if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), resp_verificacion_invent)) {
				if (resp_verificacion_invent->ObtieneNumRegistros()>0){

					instrucciones[num_instrucciones++]="DROP TABLE IF EXISTS tmpdinventarios";
					Instruccion.sprintf("CREATE TEMPORARY TABLE tmpdinventarios ( \
					referencia VARCHAR(11), \
					articulo VARCHAR(9), \
					producto VARCHAR(8), \
					present VARCHAR(255), \
					multiplo VARCHAR(10), \
					cantidadsumar DECIMAL(12, 3), \
					INDEX(articulo) \
					)ENGINE=InnoDB");
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("INSERT INTO tmpdinventarios \
					SELECT \
						d.inventario AS referencia, \
						@articulo_reemplazo, \
						a.producto, \
						a.present, \
						@multiplo_reemplazo, \
						IF(a.multiplo=@multiplo_reemplazo,SUM(IF(a.multiplo=@multiplo_reemplazo, 0, (d.cantidad/@min_factor_reemplazo))),(d.cantidad/@min_factor_reemplazo)) AS cantidad_sumar \
					FROM dinventarios d \
					INNER JOIN articulos a ON a.articulo = d.articulo \
					WHERE a.producto = @producto AND a.present = @present \
						AND a.multiplo IN(@multiplo, @multiplo_reemplazo) \
					GROUP BY d.inventario, a.producto, a.present \
					HAVING COUNT(d.inventario) > 1");
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("UPDATE dinventarios d \
					INNER JOIN tmpdinventarios tmpd ON tmpd.referencia = d.inventario AND d.articulo = tmpd.articulo \
					SET \
					d.cantidad = (d.cantidad+IFNULL(tmpd.cantidadsumar,0)) ");
					instrucciones[num_instrucciones++]=Instruccion;

					for (int i = 0; i < resp_verificacion_invent->ObtieneNumRegistros(); i++) {
						resp_verificacion_invent->IrAlRegistroNumero(i);

						AnsiString ref_eliminar = "";
						ref_eliminar = resp_verificacion_invent->ObtieneDato("referencia");

						if(resp_verificacion_invent->ObtieneDato("tipo")=="MULTIPLE"){

							Instruccion.sprintf("DELETE FROM dinventarios WHERE inventario = '%s' \
							AND articulo = @articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;
						} else {

							Instruccion.sprintf("UPDATE dinventarios d \
							SET d.articulo=@articulo_reemplazo, d.cantidad=d.cantidad/@factor_reemplazo \
							WHERE d.inventario = '%s' AND d.articulo=@articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;

						}

						if(num_instrucciones>=100000-100)
							throw Exception("El número de instrucciones sale de los límites en Eliminar artículo");

					}

				}
			}
		} __finally {
			if (resp_verificacion_invent!=NULL) delete resp_verificacion_invent;
		}

		try {
			Instruccion.sprintf("SELECT t.referencia, t.numRef, t.producto, t.present, t.multiplo, t.tipo \
			FROM (( \
			SELECT \
				d.movimiento AS referencia, \
				COUNT(d.movimiento) AS numRef, \
				a.producto, \
				a.present, \
				a.multiplo, \
				if(COUNT(a.multiplo) > 1, 'MULTIPLE', 'UNICO') AS tipo \
			FROM dmovalma d \
			INNER JOIN articulos a ON a.articulo = d.articulo \
			WHERE a.producto = '%s' AND a.present = '%s' \
				AND a.multiplo IN('%s', '%s') \
			GROUP BY d.movimiento, a.producto, a.present \
			HAVING COUNT(d.movimiento) > 1 \
			) UNION ALL ( \
			SELECT \
				d.movimiento AS referencia, \
				COUNT(d.movimiento) AS numRef, \
				a.producto, \
				a.present, \
				a.multiplo, \
				if(COUNT(a.multiplo) = 1, 'UNICO', 'N/A') AS tipo \
			FROM dmovalma d \
			INNER JOIN articulos a ON a.articulo = d.articulo \
			WHERE a.producto = '%s' AND a.present = '%s' \
				AND a.multiplo IN('%s', '%s') \
			GROUP BY d.movimiento, a.producto, a.present, a.multiplo \
			HAVING COUNT(d.movimiento) = 1 AND multiplo != '%s')) t \
			GROUP BY t.referencia, t.producto, t.present \
			ORDER BY t.tipo ",
			producto, present, multiplo, multiplo_reemp,
			producto, present, multiplo, multiplo_reemp,
			multiplo_reemp);
			if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), resp_verificacion_movalma)) {
				if (resp_verificacion_movalma->ObtieneNumRegistros()>0){

					instrucciones[num_instrucciones++]="DROP TABLE IF EXISTS tmpdmovalma";
					Instruccion.sprintf("CREATE TEMPORARY TABLE tmpdmovalma ( \
					referencia VARCHAR(11), \
					articulo VARCHAR(9), \
					producto VARCHAR(8), \
					present VARCHAR(255), \
					multiplo VARCHAR(10), \
					cantidadsumar DECIMAL(12, 3), \
					costo_sumar DECIMAL(12, 3), \
					costobase_sumar DECIMAL(12, 3), \
					INDEX(articulo) \
					)ENGINE=InnoDB");
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("INSERT INTO tmpdmovalma \
					SELECT \
						d.movimiento AS referencia, \
						@articulo_reemplazo, \
						a.producto, \
						a.present, \
						@multiplo_reemplazo, \
						IF(a.multiplo=@multiplo_reemplazo,SUM(IF(a.multiplo=@multiplo_reemplazo, 0, (d.cantidad/@min_factor_reemplazo))),(d.cantidad/@min_factor_reemplazo)) AS cantidad_sumar, \
						SUM(d.costo*d.cantidad) AS costo_sumar, \
						(d.costobase*@min_factor_reemplazo) AS costobase_sumar \
					FROM dmovalma d \
					INNER JOIN articulos a ON a.articulo = d.articulo \
					WHERE a.producto = @producto AND a.present = @present \
						AND a.multiplo IN(@multiplo, @multiplo_reemplazo) \
					GROUP BY d.movimiento, a.producto, a.present \
					HAVING COUNT(d.movimiento) > 1");
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("UPDATE dmovalma d \
					INNER JOIN tmpdmovalma tmpd ON tmpd.referencia = d.movimiento AND d.articulo = tmpd.articulo \
					SET \
					d.cantidad = (d.cantidad+IFNULL(tmpd.cantidadsumar,0)), \
					d.costo = (tmpd.costo_sumar/(d.cantidad+tmpd.cantidadsumar)), \
					d.costobase = (tmpd.costobase_sumar) ");
					instrucciones[num_instrucciones++]=Instruccion;

					for (int i = 0; i < resp_verificacion_movalma->ObtieneNumRegistros(); i++) {
						resp_verificacion_movalma->IrAlRegistroNumero(i);

						AnsiString ref_eliminar = "";
						ref_eliminar = resp_verificacion_movalma->ObtieneDato("referencia");

						if(resp_verificacion_movalma->ObtieneDato("tipo")=="MULTIPLE"){

							Instruccion.sprintf("DELETE FROM dmovalma WHERE movimiento = '%s' \
							AND articulo = @articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;
						} else {

							Instruccion.sprintf("UPDATE dmovalma d \
							SET d.articulo=@articulo_reemplazo, d.cantidad=d.cantidad/@factor_reemplazo, \
							d.costo=d.costo*@factor_reemplazo, d.costobase=d.costobase*@factor_reemplazo \
							WHERE d.movimiento = '%s' AND d.articulo=@articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;

						}

						if(num_instrucciones>=100000-100)
							throw Exception("El número de instrucciones sale de los límites en Eliminar artículo");

					}

				}
			}
		} __finally {
			if (resp_verificacion_movalma!=NULL) delete resp_verificacion_movalma;
		}

		/****************************************************
		*                                                   *
		*                                                   *
		*                                                   *
		*	INICIO UPDATES Y DELETES DE TABLAS DE DETALLES  *
		*		DE NOTAS DE CREDITO DE TIPO DEVOLUCIÓN  	*
		*                                                   *
		*                                                   *
		*                                                   *
		****************************************************/

		try {
			Instruccion.sprintf("SELECT t.referencia, t.numRef, t.producto, t.present, t.multiplo, t.tipo \
			FROM (( \
			SELECT \
				d.referencia AS referencia, \
				COUNT(d.referencia) AS numRef, \
				a.producto, \
				a.present, \
				a.multiplo, \
				if(COUNT(a.multiplo) > 1, 'MULTIPLE', 'UNICO') AS tipo \
			FROM notascredcli n \
			INNER JOIN dnotascredcli d ON d.referencia = n.referencia \
			INNER JOIN articulos a ON a.articulo = d.articulo \
			WHERE a.producto = '%s' AND a.present = '%s' \
				AND a.multiplo IN('%s', '%s') AND n.tipo = 0 \
			GROUP BY d.referencia, a.producto, a.present \
			HAVING COUNT(d.referencia) > 1 \
			) UNION ALL ( \
			SELECT \
				d.referencia AS referencia, \
				COUNT(d.referencia) AS numRef, \
				a.producto, \
				a.present, \
				a.multiplo, \
				if(COUNT(a.multiplo) = 1, 'UNICO', 'N/A') AS tipo \
			FROM notascredcli n \
			INNER JOIN dnotascredcli d ON d.referencia = n.referencia \
			INNER JOIN articulos a ON a.articulo = d.articulo \
			WHERE a.producto = '%s' AND a.present = '%s' \
				AND a.multiplo IN('%s', '%s') AND n.tipo = 0 \
			GROUP BY d.referencia, a.producto, a.present, a.multiplo \
			HAVING COUNT(d.referencia) = 1 AND multiplo != '%s')) t \
			GROUP BY t.referencia, t.producto, t.present \
			ORDER BY t.tipo ",
			producto, present, multiplo, multiplo_reemp,
			producto, present, multiplo, multiplo_reemp,
			multiplo_reemp);
			if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), resp_verificacion_notas_cred_cli)) {
				if (resp_verificacion_notas_cred_cli->ObtieneNumRegistros()>0){

					instrucciones[num_instrucciones++]="DROP TABLE IF EXISTS tmpdnotascredclidevo";
					Instruccion.sprintf("CREATE TEMPORARY TABLE tmpdnotascredclidevo ( \
					referencia VARCHAR(11), \
					articulo VARCHAR(9), \
					producto VARCHAR(8), \
					present VARCHAR(255), \
					multiplo VARCHAR(10), \
					cantidadsumar DECIMAL(12, 3), \
					precio_sumar DECIMAL(12, 3), \
					precioimp_sumar DECIMAL(12, 3), \
					INDEX(articulo) \
					)ENGINE=InnoDB");
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("INSERT INTO tmpdnotascredclidevo \
					SELECT \
						d.referencia AS referencia, \
						@articulo_reemplazo, \
						a.producto, \
						a.present, \
						@multiplo_reemplazo, \
						IF(a.multiplo=@multiplo_reemplazo,SUM(IF(a.multiplo=@multiplo_reemplazo, 0, (d.cantidad/@min_factor_reemplazo))),(d.cantidad/@min_factor_reemplazo)) AS cantidad_sumar, \
						SUM(d.precio*d.cantidad) AS precio_sumar, \
						SUM(d.precioimp*d.cantidad) AS precioimp_sumar \
					FROM notascredcli n \
					INNER JOIN dnotascredcli d ON d.referencia = n.referencia \
					INNER JOIN articulos a ON a.articulo = d.articulo \
					WHERE a.producto = @producto AND a.present = @present \
						AND a.multiplo IN(@multiplo, @multiplo_reemplazo) AND n.tipo = 0 \
					GROUP BY d.referencia, a.producto, a.present \
					HAVING COUNT(d.referencia) > 1");
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("UPDATE dnotascredcli d \
					INNER JOIN tmpdnotascredclidevo tmpd ON tmpd.referencia = d.referencia AND d.articulo = tmpd.articulo \
					SET \
					d.cantidad = (d.cantidad+IFNULL(tmpd.cantidadsumar,0)), \
					d.precio = (tmpd.precio_sumar/(d.cantidad+tmpd.cantidadsumar)), \
					d.precioimp = (tmpd.precioimp_sumar/(d.cantidad+tmpd.cantidadsumar)) ");
					instrucciones[num_instrucciones++]=Instruccion;

					for (int i = 0; i < resp_verificacion_notas_cred_cli->ObtieneNumRegistros(); i++) {
						resp_verificacion_notas_cred_cli->IrAlRegistroNumero(i);

						AnsiString ref_eliminar = "";
						ref_eliminar = resp_verificacion_notas_cred_cli->ObtieneDato("referencia");

						if(resp_verificacion_notas_cred_cli->ObtieneDato("tipo")=="MULTIPLE"){

							Instruccion.sprintf("DELETE FROM dnotascredclicfdi WHERE referencia = '%s' \
							AND articulo = @articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;

							Instruccion.sprintf("DELETE FROM dnotascredcli WHERE referencia = '%s' \
							AND articulo = @articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;
						} else {

							Instruccion.sprintf("UPDATE dnotascredcli d \
							SET d.articulo=@articulo_reemplazo, d.cantidad=d.cantidad/@factor_reemplazo, \
							d.precio=d.precio*@factor_reemplazo, d.precioimp=d.precioimp*@factor_reemplazo \
							WHERE d.referencia = '%s' AND d.articulo=@articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;

						}

						if(num_instrucciones>=100000-100)
							throw Exception("El número de instrucciones sale de los límites en Eliminar artículo");

					}

				}
			}
		} __finally {
			if (resp_verificacion_notas_cred_cli!=NULL) delete resp_verificacion_notas_cred_cli;
		}

		/****************************************************
		*                                                   *
		*                                                   *
		*                                                   *
		*	INICIO UPDATES Y DELETES DE TABLAS DE DETALLES  *
		*	   DE NOTAS DE CREDITO DE TIPO BONIFICACION     *
		*					  Y DESCUENTO  					*
		*                                                   *
		*                                                   *
		*                                                   *
		****************************************************/    try {
			Instruccion.sprintf("SELECT t.referencia, t.numRef, t.producto, t.present, t.multiplo, t.tipo \
			FROM (( \
			SELECT \
				d.referencia AS referencia, \
				COUNT(d.referencia) AS numRef, \
				a.producto, \
				a.present, \
				a.multiplo, \
				if(COUNT(a.multiplo) > 1, 'MULTIPLE', 'UNICO') AS tipo \
			FROM notascredcli n \
			INNER JOIN dnotascredcli d ON d.referencia = n.referencia \
			INNER JOIN articulos a ON a.articulo = d.articulo \
			WHERE a.producto = '%s' AND a.present = '%s' \
				AND a.multiplo IN('%s', '%s') AND n.tipo <> 0 \
			GROUP BY d.referencia, a.producto, a.present \
			HAVING COUNT(d.referencia) > 1 \
			) UNION ALL ( \
			SELECT \
				d.referencia AS referencia, \
				COUNT(d.referencia) AS numRef, \
				a.producto, \
				a.present, \
				a.multiplo, \
				if(COUNT(a.multiplo) = 1, 'UNICO', 'N/A') AS tipo \
			FROM notascredcli n \
			INNER JOIN dnotascredcli d ON d.referencia = n.referencia \
			INNER JOIN articulos a ON a.articulo = d.articulo \
			WHERE a.producto = '%s' AND a.present = '%s' \
				AND a.multiplo IN('%s', '%s') AND n.tipo <> 0 \
			GROUP BY d.referencia, a.producto, a.present, a.multiplo \
			HAVING COUNT(d.referencia) = 1 AND multiplo != '%s')) t \
			GROUP BY t.referencia, t.producto, t.present \
			ORDER BY t.tipo ",
			producto, present, multiplo, multiplo_reemp,
			producto, present, multiplo, multiplo_reemp,
			multiplo_reemp);
			if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), resp_verificacion_notas_cred_cli_varias)) {
				if (resp_verificacion_notas_cred_cli_varias->ObtieneNumRegistros()>0){

					instrucciones[num_instrucciones++]="DROP TABLE IF EXISTS tmpdnotascredclideov2";
					Instruccion.sprintf("CREATE TEMPORARY TABLE tmpdnotascredclideov2 ( \
					referencia VARCHAR(11), \
					articulo VARCHAR(9), \
					producto VARCHAR(8), \
					present VARCHAR(255), \
					multiplo VARCHAR(10), \
					cantidadsumar DECIMAL(12, 3), \
					precio_sumar DECIMAL(12, 3), \
					precioimp_sumar DECIMAL(12, 3), \
					INDEX(articulo) \
					)ENGINE=InnoDB");
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("INSERT INTO tmpdnotascredclideov2 \
					SELECT \
						d.referencia AS referencia, \
						@articulo_reemplazo, \
						a.producto, \
						a.present, \
						@multiplo_reemplazo, \
						IF(a.multiplo=@multiplo_reemplazo,SUM(IF(a.multiplo=@multiplo_reemplazo, 0, (d.cantidad/@min_factor_reemplazo))),(d.cantidad/@min_factor_reemplazo)) AS cantidad_sumar, \
						SUM(d.precio*d.cantidad) AS precio_sumar, \
						SUM(d.precioimp*d.cantidad) AS precioimp_sumar \
					FROM notascredcli n \
					INNER JOIN dnotascredcli d ON d.referencia = n.referencia \
					INNER JOIN articulos a ON a.articulo = d.articulo \
					WHERE a.producto = @producto AND a.present = @present \
						AND a.multiplo IN(@multiplo, @multiplo_reemplazo) AND n.tipo <> 0 \
					GROUP BY d.referencia, a.producto, a.present \
					HAVING COUNT(d.referencia) > 1");
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("UPDATE dnotascredcli d \
					INNER JOIN tmpdnotascredclideov2 tmpd ON tmpd.referencia = d.referencia AND d.articulo = tmpd.articulo \
					SET \
					d.cantidad = (d.cantidad+IFNULL(tmpd.cantidadsumar,0)), \
					d.precio = (tmpd.precio_sumar/(d.cantidad+tmpd.cantidadsumar)), \
					d.precioimp = (tmpd.precioimp_sumar/(d.cantidad+tmpd.cantidadsumar)) ");
					instrucciones[num_instrucciones++]=Instruccion;

					for (int i = 0; i < resp_verificacion_notas_cred_cli_varias->ObtieneNumRegistros(); i++) {
						resp_verificacion_notas_cred_cli_varias->IrAlRegistroNumero(i);

						AnsiString ref_eliminar = "";
						ref_eliminar = resp_verificacion_notas_cred_cli_varias->ObtieneDato("referencia");

						if(resp_verificacion_notas_cred_cli_varias->ObtieneDato("tipo")=="MULTIPLE"){

							Instruccion.sprintf("DELETE FROM dnotascredclicfdi WHERE referencia = '%s' \
							AND articulo = @articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;

							Instruccion.sprintf("DELETE FROM dnotascredcli WHERE referencia = '%s' \
							AND articulo = @articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;

						} else {

							Instruccion.sprintf("UPDATE dnotascredcli d \
							SET d.articulo=@articulo_reemplazo, d.cantidad=d.cantidad/@factor_reemplazo, \
							d.precio=d.precio*@factor_reemplazo, d.precioimp=d.precioimp*@factor_reemplazo \
							WHERE d.referencia = '%s' AND d.articulo=@articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;

							Instruccion.sprintf("UPDATE notascredcli n \
							INNER JOIN tmpdventasconv t ON t.referencia = n.venta \
							INNER JOIN dnotascredcli d ON n.referencia = d.referencia \
							SET d.cantidad=t.cantidadsumar, \
							d.precio=(t.precio_sumar), d.precioimp=(t.precioimp_sumar) \
							WHERE d.referencia = '%s' AND d.articulo=@articulo_reemplazo ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;

						}

						if(num_instrucciones>=100000-100)
							throw Exception("El número de instrucciones sale de los límites en Eliminar artículo");

					}

				}
			}
		} __finally {
			if (resp_verificacion_notas_cred_cli_varias!=NULL) delete resp_verificacion_notas_cred_cli_varias;
		}

		try {
			Instruccion.sprintf("SELECT t.referencia, t.numRef, t.producto, t.present, t.multiplo, t.tipo \
			FROM (( \
			SELECT \
				d.referencia AS referencia, \
				COUNT(d.referencia) AS numRef, \
				a.producto, \
				a.present, \
				a.multiplo, \
				if(COUNT(a.multiplo) > 1, 'MULTIPLE', 'UNICO') AS tipo \
			FROM dnotascredprov d \
			INNER JOIN articulos a ON a.articulo = d.articulo \
			WHERE a.producto = '%s' AND a.present = '%s' \
				AND a.multiplo IN('%s', '%s') \
			GROUP BY d.referencia, a.producto, a.present \
			HAVING COUNT(d.referencia) > 1 \
			) UNION ALL ( \
			SELECT \
				d.referencia AS referencia, \
				COUNT(d.referencia) AS numRef, \
				a.producto, \
				a.present, \
				a.multiplo, \
				if(COUNT(a.multiplo) = 1, 'UNICO', 'N/A') AS tipo \
			FROM dnotascredprov d \
			INNER JOIN articulos a ON a.articulo = d.articulo \
			WHERE a.producto = '%s' AND a.present = '%s' \
				AND a.multiplo IN('%s', '%s') \
			GROUP BY d.referencia, a.producto, a.present, a.multiplo \
			HAVING COUNT(d.referencia) = 1 AND multiplo != '%s')) t \
			GROUP BY t.referencia, t.producto, t.present \
			ORDER BY t.tipo ",
			producto, present, multiplo, multiplo_reemp,
			producto, present, multiplo, multiplo_reemp,
			multiplo_reemp);
			if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), resp_verificacion_notas_cred_prov)) {
				if (resp_verificacion_notas_cred_prov->ObtieneNumRegistros()>0){

					instrucciones[num_instrucciones++]="DROP TABLE IF EXISTS tmpdnotascredprov";
					Instruccion.sprintf("CREATE TEMPORARY TABLE tmpdnotascredprov ( \
					referencia VARCHAR(11), \
					articulo VARCHAR(9), \
					producto VARCHAR(8), \
					present VARCHAR(255), \
					multiplo VARCHAR(10), \
					cantidadsumar DECIMAL(12, 3), \
					costo_sumar DECIMAL(12, 3), \
					costoimp_sumar DECIMAL(12, 3), \
					INDEX(articulo) \
					)ENGINE=InnoDB");
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("INSERT INTO tmpdnotascredprov \
					SELECT \
						d.referencia AS referencia, \
						@articulo_reemplazo, \
						a.producto, \
						a.present, \
						@multiplo_reemplazo, \
						IF(a.multiplo=@multiplo_reemplazo,SUM(IF(a.multiplo=@multiplo_reemplazo, 0, (d.cantidad/@min_factor_reemplazo))),(d.cantidad/@min_factor_reemplazo)) AS cantidad_sumar, \
						SUM(d.costo*d.cantidad) AS costo_sumar, \
						SUM(d.costoimp*d.cantidad) AS costoimp_sumar \
					FROM dnotascredprov d \
					INNER JOIN articulos a ON a.articulo = d.articulo \
					WHERE a.producto = @producto AND a.present = @present \
						AND a.multiplo IN(@multiplo, @multiplo_reemplazo) \
					GROUP BY d.referencia, a.producto, a.present \
					HAVING COUNT(d.referencia) > 1");
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("UPDATE dnotascredprov d \
					INNER JOIN tmpdnotascredprov tmpd ON tmpd.referencia = d.referencia AND d.articulo = tmpd.articulo \
					SET \
					d.cantidad = (d.cantidad+IFNULL(tmpd.cantidadsumar,0)), \
					d.costo = (tmpd.costo_sumar/(d.cantidad+tmpd.cantidadsumar)), \
					d.costoimp = (tmpd.costoimp_sumar/(d.cantidad+tmpd.cantidadsumar)) ");
					instrucciones[num_instrucciones++]=Instruccion;

					for (int i = 0; i < resp_verificacion_notas_cred_prov->ObtieneNumRegistros(); i++) {
						resp_verificacion_notas_cred_prov->IrAlRegistroNumero(i);

						AnsiString ref_eliminar = "";
						ref_eliminar = resp_verificacion_notas_cred_prov->ObtieneDato("referencia");

						if(resp_verificacion_notas_cred_prov->ObtieneDato("tipo")=="MULTIPLE"){

							Instruccion.sprintf("DELETE FROM dnotascredprov WHERE referencia = '%s' \
							AND articulo = @articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;
						} else {

							Instruccion.sprintf("UPDATE dnotascredprov d \
							SET d.articulo=@articulo_reemplazo, d.cantidad=d.cantidad/@factor_reemplazo, \
							d.costo=d.costo*@factor_reemplazo, d.costoimp=d.costoimp*@factor_reemplazo \
							WHERE d.referencia = '%s' AND d.articulo=@articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;

						}

						if(num_instrucciones>=100000-100)
							throw Exception("El número de instrucciones sale de los límites en Eliminar artículo");

					}

				}
			}
		} __finally {
			if (resp_verificacion_notas_cred_prov!=NULL) delete resp_verificacion_notas_cred_prov;
		}

		try {
			Instruccion.sprintf("SELECT t.referencia, t.numRef, t.producto, t.present, t.multiplo, t.tipo \
			FROM (( \
			SELECT \
				d.pedido AS referencia, \
				COUNT(d.pedido) AS numRef, \
				a.producto, \
				a.present, \
				a.multiplo, \
				if(COUNT(a.multiplo) > 1, 'MULTIPLE', 'UNICO') AS tipo \
			FROM dpedidoskits d \
			INNER JOIN articulos a ON a.articulo = d.articulo \
			WHERE a.producto = '%s' AND a.present = '%s' \
				AND a.multiplo IN('%s', '%s') \
			GROUP BY d.pedido, a.producto, a.present \
			HAVING COUNT(d.pedido) > 1 \
			) UNION ALL ( \
			SELECT \
				d.pedido AS referencia, \
				COUNT(d.pedido) AS numRef, \
				a.producto, \
				a.present, \
				a.multiplo, \
				if(COUNT(a.multiplo) = 1, 'UNICO', 'N/A') AS tipo \
			FROM dpedidoskits d \
			INNER JOIN articulos a ON a.articulo = d.articulo \
			WHERE a.producto = '%s' AND a.present = '%s' \
				AND a.multiplo IN('%s', '%s') \
			GROUP BY d.pedido, a.producto, a.present, a.multiplo \
			HAVING COUNT(d.pedido) = 1 AND multiplo != '%s')) t \
			GROUP BY t.referencia, t.producto, t.present \
			ORDER BY t.tipo ",
			producto, present, multiplo, multiplo_reemp,
			producto, present, multiplo, multiplo_reemp,
			multiplo_reemp);
			if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), resp_verificacion_pedidos_kit)) {
				if (resp_verificacion_pedidos_kit->ObtieneNumRegistros()>0){

					instrucciones[num_instrucciones++]="DROP TABLE IF EXISTS tmpdpedidoskits";
					Instruccion.sprintf("CREATE TEMPORARY TABLE tmpdpedidoskits ( \
					referencia VARCHAR(11), \
					articulo VARCHAR(9), \
					producto VARCHAR(8), \
					present VARCHAR(255), \
					multiplo VARCHAR(10), \
					cantidadsumar DECIMAL(12, 3), \
					precio_sumar DECIMAL(12, 3), \
					preciocom_sumar DECIMAL(12, 3), \
					INDEX(articulo) \
					)ENGINE=InnoDB");
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("INSERT INTO tmpdpedidoskits \
					SELECT \
						d.pedido AS referencia, \
						@articulo_reemplazo, \
						a.producto, \
						a.present, \
						@multiplo_reemplazo, \
						IF(a.multiplo=@multiplo_reemplazo,SUM(IF(a.multiplo=@multiplo_reemplazo, 0, (d.cantidad/@min_factor_reemplazo))),(d.cantidad/@min_factor_reemplazo)) AS cantidad_sumar, \
						SUM(d.precio*d.cantidad) AS precio_sumar, \
						SUM(d.preciocom*d.cantidad) AS preciocom_sumar \
					FROM dpedidoskits d \
					INNER JOIN articulos a ON a.articulo = d.articulo \
					WHERE a.producto = @producto AND a.present = @present \
						AND a.multiplo IN(@multiplo, @multiplo_reemplazo) \
					GROUP BY d.pedido, a.producto, a.present \
					HAVING COUNT(d.pedido) > 1");
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("UPDATE dpedidoskits d \
					INNER JOIN tmpdpedidoskits tmpd ON tmpd.referencia = d.pedido AND d.articulo = tmpd.articulo \
					SET \
					d.cantidad = (d.cantidad+IFNULL(tmpd.cantidadsumar,0)), \
					d.precio = (tmpd.precio_sumar/(d.cantidad+tmpd.cantidadsumar)), \
					d.preciocom = (tmpd.preciocom_sumar/(d.cantidad+tmpd.cantidadsumar)) ");
					instrucciones[num_instrucciones++]=Instruccion;

					for (int i = 0; i < resp_verificacion_pedidos_kit->ObtieneNumRegistros(); i++) {
						resp_verificacion_pedidos_kit->IrAlRegistroNumero(i);

						AnsiString ref_eliminar = "";
						ref_eliminar = resp_verificacion_pedidos_kit->ObtieneDato("referencia");

						if(resp_verificacion_pedidos_kit->ObtieneDato("tipo")=="MULTIPLE"){

							Instruccion.sprintf("DELETE FROM dpedidoskits WHERE pedido = '%s' \
							AND articulo = @articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;
						} else {

							Instruccion.sprintf("UPDATE dpedidoskits d \
							SET d.articulo=@articulo_reemplazo, d.cantidad=d.cantidad/@factor_reemplazo, \
							d.precio=d.precio*@factor_reemplazo, d.preciocom=d.preciocom*@factor_reemplazo \
							WHERE d.pedido = '%s' AND d.articulo=@articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;

						}

						if(num_instrucciones>=100000-100)
							throw Exception("El número de instrucciones sale de los límites en Eliminar artículo");

					}

				}
			}
		} __finally {
			if (resp_verificacion_pedidos_kit!=NULL) delete resp_verificacion_pedidos_kit;
		}

		try {
			Instruccion.sprintf("SELECT t.referencia, t.numRef, t.producto, t.present, t.multiplo, t.tipo \
			FROM (( \
			SELECT \
				d.kit AS referencia, \
				COUNT(d.kit) AS numRef, \
				a.producto, \
				a.present, \
				a.multiplo, \
				if(COUNT(a.multiplo) > 1, 'MULTIPLE', 'UNICO') AS tipo \
			FROM dkits d \
			INNER JOIN articulos a ON a.articulo = d.articulo \
			WHERE a.producto = '%s' AND a.present = '%s' \
				AND a.multiplo IN('%s', '%s') \
			GROUP BY d.kit, a.producto, a.present \
			HAVING COUNT(d.kit) > 1 \
			) UNION ALL ( \
			SELECT \
				d.kit AS referencia, \
				COUNT(d.kit) AS numRef, \
				a.producto, \
				a.present, \
				a.multiplo, \
				if(COUNT(a.multiplo) = 1, 'UNICO', 'N/A') AS tipo \
			FROM dkits d \
			INNER JOIN articulos a ON a.articulo = d.articulo \
			WHERE a.producto = '%s' AND a.present = '%s' \
				AND a.multiplo IN('%s', '%s') \
			GROUP BY d.kit, a.producto, a.present, a.multiplo \
			HAVING COUNT(d.kit) = 1 AND multiplo != '%s')) t \
			GROUP BY t.referencia, t.producto, t.present \
			ORDER BY t.tipo ",
			producto, present, multiplo, multiplo_reemp,
			producto, present, multiplo, multiplo_reemp,
			multiplo_reemp);
			if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), resp_verificacion_kits)) {
				if (resp_verificacion_kits->ObtieneNumRegistros()>0){

					instrucciones[num_instrucciones++]="DROP TABLE IF EXISTS tmpdkits";
					Instruccion.sprintf("CREATE TEMPORARY TABLE tmpdkits ( \
					referencia VARCHAR(11), \
					articulo VARCHAR(9), \
					producto VARCHAR(8), \
					present VARCHAR(255), \
					multiplo VARCHAR(10), \
					cantidadsumar DECIMAL(12, 3), \
					precioart_sumar DECIMAL(12, 3), \
					preciocomart_sumar DECIMAL(12, 3), \
					INDEX(articulo) \
					)ENGINE=InnoDB");
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("INSERT INTO tmpdkits \
					SELECT \
						d.kit AS referencia, \
						@articulo_reemplazo, \
						a.producto, \
						a.present, \
						@multiplo_reemplazo, \
						IF(a.multiplo=@multiplo_reemplazo,SUM(IF(a.multiplo=@multiplo_reemplazo, 0, (d.cantidad/@min_factor_reemplazo))),(d.cantidad/@min_factor_reemplazo)) AS cantidad_sumar, \
						SUM(d.precioart*d.cantidad) AS precioart_sumar, \
						SUM(d.preciocomart*d.cantidad) AS preciocomart_sumar \
					FROM dkits d \
					INNER JOIN articulos a ON a.articulo = d.articulo \
					WHERE a.producto = @producto AND a.present = @present \
						AND a.multiplo IN(@multiplo, @multiplo_reemplazo) \
					GROUP BY d.kit, a.producto, a.present \
					HAVING COUNT(d.kit) > 1");
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("UPDATE dkits d \
					INNER JOIN tmpdkits tmpd ON tmpd.referencia = d.kit AND d.articulo = tmpd.articulo \
					SET \
					d.cantidad = (d.cantidad+IFNULL(tmpd.cantidadsumar,0)), \
					d.precioart = (tmpd.precioart_sumar/(d.cantidad+tmpd.cantidadsumar)), \
					d.preciocomart = (tmpd.preciocomart_sumar/(d.cantidad+tmpd.cantidadsumar)) ");
					instrucciones[num_instrucciones++]=Instruccion;

					for (int i = 0; i < resp_verificacion_kits->ObtieneNumRegistros(); i++) {
						resp_verificacion_kits->IrAlRegistroNumero(i);

						AnsiString ref_eliminar = "";
						ref_eliminar = resp_verificacion_kits->ObtieneDato("referencia");

						if(resp_verificacion_kits->ObtieneDato("tipo")=="MULTIPLE"){

							Instruccion.sprintf("DELETE FROM dkits WHERE kit = '%s' \
							AND articulo = @articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;
						} else {

							Instruccion.sprintf("UPDATE dkits d \
							SET d.articulo=@articulo_reemplazo, d.cantidad=d.cantidad/@factor_reemplazo, \
							d.precioart=d.precioart*@factor_reemplazo, d.preciocomart=d.preciocomart*@factor_reemplazo \
							WHERE d.kit = '%s' AND d.articulo=@articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;

						}

						if(num_instrucciones>=100000-100)
							throw Exception("El número de instrucciones sale de los límites en Eliminar artículo");

					}

				}
			}
		} __finally {
			if (resp_verificacion_kits!=NULL) delete resp_verificacion_kits;
		}

		try {
			Instruccion.sprintf("SELECT t.referencia, t.numRef, t.producto, t.present, t.multiplo, t.tipo \
			FROM (( \
			SELECT \
				d.referencia AS referencia, \
				COUNT(d.referencia) AS numRef, \
				a.producto, \
				a.present, \
				a.multiplo, \
				if(COUNT(a.multiplo) > 1, 'MULTIPLE', 'UNICO') AS tipo \
			FROM dpedidosecommsurtido d \
			INNER JOIN articulos a ON a.articulo = d.articulo \
			WHERE a.producto = '%s' AND a.present = '%s' \
				AND a.multiplo IN('%s', '%s') \
			GROUP BY d.referencia, a.producto, a.present \
			HAVING COUNT(d.referencia) > 1 \
			) UNION ALL ( \
			SELECT \
				d.referencia AS referencia, \
				COUNT(d.referencia) AS numRef, \
				a.producto, \
				a.present, \
				a.multiplo, \
				if(COUNT(a.multiplo) = 1, 'UNICO', 'N/A') AS tipo \
			FROM dpedidosecommsurtido d \
			INNER JOIN articulos a ON a.articulo = d.articulo \
			WHERE a.producto = '%s' AND a.present = '%s' \
				AND a.multiplo IN('%s', '%s') \
			GROUP BY d.referencia, a.producto, a.present, a.multiplo \
			HAVING COUNT(d.referencia) = 1 AND multiplo != '%s')) t \
			GROUP BY t.referencia, t.producto, t.present \
			ORDER BY t.tipo ",
			producto, present, multiplo, multiplo_reemp,
			producto, present, multiplo, multiplo_reemp,
			multiplo_reemp);
			if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), resp_verificacion_pedidos_ecomm_surtido)) {
				if (resp_verificacion_pedidos_ecomm_surtido->ObtieneNumRegistros()>0){

					instrucciones[num_instrucciones++]="DROP TABLE IF EXISTS tmpdpedidosecommsurtido";
					Instruccion.sprintf("CREATE TEMPORARY TABLE tmpdpedidosecommsurtido ( \
					referencia VARCHAR(11), \
					articulo VARCHAR(9), \
					producto VARCHAR(8), \
					present VARCHAR(255), \
					multiplo VARCHAR(10), \
					cantidadsumar DECIMAL(12, 3), \
					costobase_sumar DECIMAL(12, 3), \
					precio_sumar DECIMAL(12, 3), \
					precioimp_sumar DECIMAL(12, 3), \
					INDEX(articulo) \
					)ENGINE=InnoDB");
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("INSERT INTO tmpdpedidosecommsurtido \
					SELECT \
						d.referencia AS referencia, \
						@articulo_reemplazo, \
						a.producto, \
						a.present, \
						@multiplo_reemplazo, \
						IF(a.multiplo=@multiplo_reemplazo,SUM(IF(a.multiplo=@multiplo_reemplazo, 0, (d.cantidad/@min_factor_reemplazo))),(d.cantidad/@min_factor_reemplazo)) AS cantidad_sumar, \
						(d.costobase*@min_factor_reemplazo) AS costobase_sumar, \
						SUM(d.precio*d.cantidad) AS precio_sumar, \
						SUM(d.precioimp*d.cantidad) AS precioimp_sumar \
					FROM dpedidosecommsurtido d \
					INNER JOIN articulos a ON a.articulo = d.articulo \
					WHERE a.producto = @producto AND a.present = @present \
						AND a.multiplo IN(@multiplo, @multiplo_reemplazo) \
					GROUP BY d.referencia, a.producto, a.present \
					HAVING COUNT(d.referencia) > 1");
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("UPDATE dpedidosecommsurtido d \
					INNER JOIN tmpdpedidosecommsurtido tmpd ON tmpd.referencia = d.referencia AND d.articulo = tmpd.articulo \
					SET \
					d.cantidad = (d.cantidad+IFNULL(tmpd.cantidadsumar,0)), \
					d.costobase = (tmpd.costobase_sumar), \
					d.precio = (d.precio/(d.cantidad+tmpd.cantidadsumar)), \
					d.precioimp = (d.precioimp/(d.cantidad+tmpd.cantidadsumar)) ");
					instrucciones[num_instrucciones++]=Instruccion;

					for (int i = 0; i < resp_verificacion_pedidos_ecomm_surtido->ObtieneNumRegistros(); i++) {
						resp_verificacion_pedidos_ecomm_surtido->IrAlRegistroNumero(i);

						AnsiString ref_eliminar = "";
						ref_eliminar = resp_verificacion_pedidos_ecomm_surtido->ObtieneDato("referencia");

						if(resp_verificacion_pedidos_ecomm_surtido->ObtieneDato("tipo")=="MULTIPLE"){

							Instruccion.sprintf("DELETE FROM dpedidosecommsurtido WHERE referencia = '%s' \
							AND articulo = @articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;
						} else {

							Instruccion.sprintf("UPDATE dpedidosecommsurtido d \
							SET d.articulo=@articulo_reemplazo, d.cantidad=d.cantidad/@factor_reemplazo, \
							d.costobase=d.costobase*@factor_reemplazo, d.precio=d.precio*@factor_reemplazo,\
							d.precioimp=d.precioimp*@factor_reemplazo \
							WHERE d.referencia = '%s' AND d.articulo=@articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;

						}

						if(num_instrucciones>=100000-100)
							throw Exception("El número de instrucciones sale de los límites en Eliminar artículo");

					}

				}
			}
		} __finally {
			if (resp_verificacion_pedidos_ecomm_surtido!=NULL) delete resp_verificacion_pedidos_ecomm_surtido;
		}

		try {
			Instruccion.sprintf("SELECT t.referencia, t.numRef, t.producto, t.present, t.multiplo, t.tipo \
			FROM (( \
			SELECT \
				d.referencia AS referencia, \
				COUNT(d.referencia) AS numRef, \
				a.producto, \
				a.present, \
				a.multiplo, \
				if(COUNT(a.multiplo) > 1, 'MULTIPLE', 'UNICO') AS tipo \
			FROM dpedidosecomm d \
			INNER JOIN articulos a ON a.articulo = d.articulo \
			WHERE a.producto = '%s' AND a.present = '%s' \
				AND a.multiplo IN('%s', '%s') \
			GROUP BY d.referencia, a.producto, a.present \
			HAVING COUNT(d.referencia) > 1 \
			) UNION ALL ( \
			SELECT \
				d.referencia AS referencia, \
				COUNT(d.referencia) AS numRef, \
				a.producto, \
				a.present, \
				a.multiplo, \
				if(COUNT(a.multiplo) = 1, 'UNICO', 'N/A') AS tipo \
			FROM dpedidosecomm d \
			INNER JOIN articulos a ON a.articulo = d.articulo \
			WHERE a.producto = '%s' AND a.present = '%s' \
				AND a.multiplo IN('%s', '%s') \
			GROUP BY d.referencia, a.producto, a.present, a.multiplo \
			HAVING COUNT(d.referencia) = 1 AND multiplo != '%s')) t \
			GROUP BY t.referencia, t.producto, t.present \
			ORDER BY t.tipo ",
			producto, present, multiplo, multiplo_reemp,
			producto, present, multiplo, multiplo_reemp,
			multiplo_reemp);
			if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), resp_verificacion_pedidos_ecomm)) {
				if (resp_verificacion_pedidos_ecomm->ObtieneNumRegistros()>0){

					instrucciones[num_instrucciones++]="DROP TABLE IF EXISTS tmpdpedidosecomm";
					Instruccion.sprintf("CREATE TEMPORARY TABLE tmpdpedidosecomm ( \
					referencia VARCHAR(11), \
					articulo VARCHAR(9), \
					producto VARCHAR(8), \
					present VARCHAR(255), \
					multiplo VARCHAR(10), \
					cantidadsumar DECIMAL(12, 3), \
					costobase_sumar DECIMAL(12, 3), \
					precio_sumar DECIMAL(12, 3), \
					precioimp_sumar DECIMAL(12, 3), \
					INDEX(articulo) \
					)ENGINE=InnoDB");
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("INSERT INTO tmpdpedidosecomm \
					SELECT \
						d.referencia AS referencia, \
						@articulo_reemplazo, \
						a.producto, \
						a.present, \
						@multiplo_reemplazo, \
						IF(a.multiplo=@multiplo_reemplazo,SUM(IF(a.multiplo=@multiplo_reemplazo, 0, (d.cantidad/@min_factor_reemplazo))),(d.cantidad/@min_factor_reemplazo)) AS cantidad_sumar, \
						(d.costobase*@min_factor_reemplazo) AS costobase_sumar, \
						SUM(d.precio*d.cantidad) AS precio_sumar, \
						SUM(d.precioimp*d.cantidad) AS precioimp_sumar \
					FROM dpedidosecomm d \
					INNER JOIN articulos a ON a.articulo = d.articulo \
					WHERE a.producto = @producto AND a.present = @present \
						AND a.multiplo IN(@multiplo, @multiplo_reemplazo) \
					GROUP BY d.referencia, a.producto, a.present \
					HAVING COUNT(d.referencia) > 1");
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("UPDATE dpedidosecomm d \
					INNER JOIN tmpdpedidosecomm tmpd ON tmpd.referencia = d.referencia AND d.articulo = tmpd.articulo \
					SET \
					d.cantidad = (d.cantidad+IFNULL(tmpd.cantidadsumar,0)), \
					d.costobase = (dv.costobase*@factor_reemplazo), \
					d.precio = (tmpd.precio_sumar/(d.cantidad+tmpd.cantidadsumar)), \
					d.precioimp = (tmpd.precioimp_sumar/(d.cantidad+tmpd.cantidadsumar)) ");
					instrucciones[num_instrucciones++]=Instruccion;

					for (int i = 0; i < resp_verificacion_pedidos_ecomm->ObtieneNumRegistros(); i++) {
						resp_verificacion_pedidos_ecomm->IrAlRegistroNumero(i);

						AnsiString ref_eliminar = "";
						ref_eliminar = resp_verificacion_pedidos_ecomm->ObtieneDato("referencia");

						if(resp_verificacion_pedidos_ecomm->ObtieneDato("tipo")=="MULTIPLE"){

							Instruccion.sprintf("DELETE FROM dpedidosecomm WHERE referencia = '%s' \
							AND articulo = @articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;
						} else {

							Instruccion.sprintf("UPDATE dpedidosecomm d \
							SET d.articulo=@articulo_reemplazo, d.cantidad=d.cantidad/@factor_reemplazo, \
							d.costobase=d.costobase*@factor_reemplazo, d.precio=d.precio*@factor_reemplazo,\
							d.precioimp=d.precioimp*@factor_reemplazo \
							WHERE d.referencia = '%s' AND d.articulo=@articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;

						}

						if(num_instrucciones>=100000-100)
							throw Exception("El número de instrucciones sale de los límites en Eliminar artículo");

					}

				}
			}
		} __finally {
			if (resp_verificacion_pedidos_ecomm!=NULL) delete resp_verificacion_pedidos_ecomm;
		}

		try {
			Instruccion.sprintf("SELECT t.referencia, t.numRef, t.producto, t.present, t.multiplo, t.tipo \
			FROM (( \
			SELECT \
				d.recepcion AS referencia, \
				COUNT(d.recepcion) AS numRef, \
				a.producto, \
				a.present, \
				a.multiplo, \
				if(COUNT(a.multiplo) > 1, 'MULTIPLE', 'UNICO') AS tipo \
			FROM drecepciones d \
			INNER JOIN articulos a ON a.articulo = d.articulo \
			WHERE a.producto = '%s' AND a.present = '%s' \
				AND a.multiplo IN('%s', '%s') \
			GROUP BY d.recepcion, a.producto, a.present \
			HAVING COUNT(d.recepcion) > 1 \
			) UNION ALL ( \
			SELECT \
				d.recepcion AS referencia, \
				COUNT(d.recepcion) AS numRef, \
				a.producto, \
				a.present, \
				a.multiplo, \
				if(COUNT(a.multiplo) = 1, 'UNICO', 'N/A') AS tipo \
			FROM drecepciones d \
			INNER JOIN articulos a ON a.articulo = d.articulo \
			WHERE a.producto = '%s' AND a.present = '%s' \
				AND a.multiplo IN('%s', '%s') \
			GROUP BY d.recepcion, a.producto, a.present, a.multiplo \
			HAVING COUNT(d.recepcion) = 1 AND multiplo != '%s')) t \
			GROUP BY t.referencia, t.producto, t.present \
			ORDER BY t.tipo ",
			producto, present, multiplo, multiplo_reemp,
			producto, present, multiplo, multiplo_reemp,
			multiplo_reemp);
			if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), resp_verificacion_recepcion)) {
				if (resp_verificacion_recepcion->ObtieneNumRegistros()>0){

					instrucciones[num_instrucciones++]="DROP TABLE IF EXISTS tmpdrecepciones";
					Instruccion.sprintf("CREATE TEMPORARY TABLE tmpdrecepciones ( \
					referencia VARCHAR(11), \
					articulo VARCHAR(9), \
					producto VARCHAR(8), \
					present VARCHAR(255), \
					multiplo VARCHAR(10), \
					cantidadsumar DECIMAL(12, 3), \
					INDEX(articulo) \
					)ENGINE=InnoDB");
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("INSERT INTO tmpdrecepciones \
					SELECT \
						d.recepcion AS referencia, \
						@articulo_reemplazo, \
						a.producto, \
						a.present, \
						@multiplo_reemplazo, \
						IF(a.multiplo=@multiplo_reemplazo,SUM(IF(a.multiplo=@multiplo_reemplazo, 0, (d.cantidad/@min_factor_reemplazo))),(d.cantidad/@min_factor_reemplazo)) AS cantidad_sumar \
					FROM drecepciones d \
					INNER JOIN articulos a ON a.articulo = d.articulo \
					WHERE a.producto = @producto AND a.present = @present \
						AND a.multiplo IN(@multiplo, @multiplo_reemplazo) \
					GROUP BY d.recepcion, a.producto, a.present \
					HAVING COUNT(d.recepcion) > 1");
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("UPDATE drecepciones d \
					INNER JOIN tmpdrecepciones tmpd ON tmpd.referencia = d.referencia AND d.articulo = tmpd.articulo \
					SET \
					d.cantidad = (d.cantidad+IFNULL(tmpd.cantidadsumar,0)) ");
					instrucciones[num_instrucciones++]=Instruccion;

					for (int i = 0; i < resp_verificacion_recepcion->ObtieneNumRegistros(); i++) {
						resp_verificacion_recepcion->IrAlRegistroNumero(i);

						AnsiString ref_eliminar = "";
						ref_eliminar = resp_verificacion_recepcion->ObtieneDato("referencia");

						if(resp_verificacion_recepcion->ObtieneDato("tipo")=="MULTIPLE"){

							Instruccion.sprintf("DELETE FROM drecepciones WHERE recepcion = '%s' \
							AND articulo = @articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;
						} else {

							Instruccion.sprintf("UPDATE drecepciones d \
							SET d.articulo=@articulo_reemplazo, d.cantidad=d.cantidad/@factor_reemplazo \
							WHERE d.recepcion = '%s' AND d.articulo=@articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;

						}

						if(num_instrucciones>=100000-100)
							throw Exception("El número de instrucciones sale de los límites en Eliminar artículo");

					}

				}
			}
		} __finally {
			if (resp_verificacion_recepcion!=NULL) delete resp_verificacion_recepcion;
		}

		try {
			Instruccion.sprintf("SELECT t.referencia, t.numRef, t.producto, t.present, t.multiplo, t.tipo \
			FROM (( \
			SELECT \
				d.devolucion AS referencia, \
				COUNT(d.devolucion) AS numRef, \
				a.producto, \
				a.present, \
				a.multiplo, \
				if(COUNT(a.multiplo) > 1, 'MULTIPLE', 'UNICO') AS tipo \
			FROM drecepciondevol d \
			INNER JOIN articulos a ON a.articulo = d.articulo \
			WHERE a.producto = '%s' AND a.present = '%s' \
				AND a.multiplo IN('%s', '%s') \
			GROUP BY d.devolucion, a.producto, a.present \
			HAVING COUNT(d.devolucion) > 1 \
			) UNION ALL ( \
			SELECT \
				d.devolucion AS referencia, \
				COUNT(d.devolucion) AS numRef, \
				a.producto, \
				a.present, \
				a.multiplo, \
				if(COUNT(a.multiplo) = 1, 'UNICO', 'N/A') AS tipo \
			FROM drecepciondevol d \
			INNER JOIN articulos a ON a.articulo = d.articulo \
			WHERE a.producto = '%s' AND a.present = '%s' \
				AND a.multiplo IN('%s', '%s') \
			GROUP BY d.devolucion, a.producto, a.present, a.multiplo \
			HAVING COUNT(d.devolucion) = 1 AND multiplo != '%s')) t \
			GROUP BY t.referencia, t.producto, t.present \
			ORDER BY t.tipo ",
			producto, present, multiplo, multiplo_reemp,
			producto, present, multiplo, multiplo_reemp,
			multiplo_reemp);
			if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), resp_verificacion_recepcion_devol)) {
				if (resp_verificacion_recepcion_devol->ObtieneNumRegistros()>0){

					instrucciones[num_instrucciones++]="DROP TABLE IF EXISTS tmpdrecepciondevol";
					Instruccion.sprintf("CREATE TEMPORARY TABLE tmpdrecepciondevol ( \
					referencia VARCHAR(11), \
					articulo VARCHAR(9), \
					producto VARCHAR(8), \
					present VARCHAR(255), \
					multiplo VARCHAR(10), \
					cantidadsumar DECIMAL(12, 3), \
					INDEX(articulo) \
					)ENGINE=InnoDB");
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("INSERT INTO tmpdrecepciondevol \
					SELECT \
						d.devolucion AS referencia, \
						@articulo_reemplazo, \
						a.producto, \
						a.present, \
						@multiplo_reemplazo, \
						IF(a.multiplo=@multiplo_reemplazo,SUM(IF(a.multiplo=@multiplo_reemplazo, 0, (d.cantidad/@min_factor_reemplazo))),(d.cantidad/@min_factor_reemplazo)) AS cantidad_sumar \
					FROM drecepciondevol d \
					INNER JOIN articulos a ON a.articulo = d.articulo \
					WHERE a.producto = @producto AND a.present = @present \
						AND a.multiplo IN(@multiplo, @multiplo_reemplazo) \
					GROUP BY d.devolucion, a.producto, a.present \
					HAVING COUNT(d.devolucion) > 1");
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("UPDATE drecepciondevol d \
					INNER JOIN tmpdrecepciondevol tmpd ON tmpd.referencia = d.devolucion AND d.articulo = tmpd.articulo \
					SET \
					d.cantidad = (d.cantidad+IFNULL(tmpd.cantidadsumar,0)) ");
					instrucciones[num_instrucciones++]=Instruccion;

					for (int i = 0; i < resp_verificacion_recepcion_devol->ObtieneNumRegistros(); i++) {
						resp_verificacion_recepcion_devol->IrAlRegistroNumero(i);

						AnsiString ref_eliminar = "";
						ref_eliminar = resp_verificacion_recepcion_devol->ObtieneDato("referencia");

						if(resp_verificacion_recepcion_devol->ObtieneDato("tipo")=="MULTIPLE"){

							Instruccion.sprintf("DELETE FROM drecepciondevol WHERE devolucion = '%s' \
							AND articulo = @articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;
						} else {

							Instruccion.sprintf("UPDATE drecepciondevol d \
							SET d.articulo=@articulo_reemplazo, d.cantidad=d.cantidad/@factor_reemplazo \
							WHERE d.devolucion = '%s' AND d.articulo=@articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;

						}

						if(num_instrucciones>=100000-100)
							throw Exception("El número de instrucciones sale de los límites en Eliminar artículo");

					}

				}
			}
		} __finally {
			if (resp_verificacion_recepcion_devol !=NULL) delete resp_verificacion_recepcion_devol;
		}

		try {
			Instruccion.sprintf("SELECT t.referencia, t.numRef, t.producto, t.present, t.multiplo, t.tipo \
			FROM (( \
			SELECT \
				d.cartaporte20 AS referencia, \
				COUNT(d.cartaporte20) AS numRef, \
				a.producto, \
				a.present, \
				a.multiplo, \
				if(COUNT(a.multiplo) > 1, 'MULTIPLE', 'UNICO') AS tipo \
			FROM dcartasporte20 d \
			INNER JOIN articulos a ON a.articulo = d.articulo \
			WHERE a.producto = '%s' AND a.present = '%s' \
				AND a.multiplo IN('%s', '%s') \
			GROUP BY d.cartaporte20, a.producto, a.present \
			HAVING COUNT(d.cartaporte20) > 1 \
			) UNION ALL ( \
			SELECT \
				d.cartaporte20 AS referencia, \
				COUNT(d.cartaporte20) AS numRef, \
				a.producto, \
				a.present, \
				a.multiplo, \
				if(COUNT(a.multiplo) = 1, 'UNICO', 'N/A') AS tipo \
			FROM dcartasporte20 d \
			INNER JOIN articulos a ON a.articulo = d.articulo \
			WHERE a.producto = '%s' AND a.present = '%s' \
				AND a.multiplo IN('%s', '%s') \
			GROUP BY d.cartaporte20, a.producto, a.present, a.multiplo \
			HAVING COUNT(d.cartaporte20) = 1 AND multiplo != '%s')) t \
			GROUP BY t.referencia, t.producto, t.present \
			ORDER BY t.tipo ",
			producto, present, multiplo, multiplo_reemp,
			producto, present, multiplo, multiplo_reemp,
			multiplo_reemp);
			if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), resp_verificacion_cartas_porte20)) {
				if (resp_verificacion_cartas_porte20->ObtieneNumRegistros()>0){

					instrucciones[num_instrucciones++]="DROP TABLE IF EXISTS tmpdcartasporte20";
					Instruccion.sprintf("CREATE TEMPORARY TABLE tmpdcartasporte20 ( \
					referencia VARCHAR(11), \
					articulo VARCHAR(9), \
					producto VARCHAR(8), \
					present VARCHAR(255), \
					multiplo VARCHAR(10), \
					cantidadsumar DECIMAL(12, 3), \
					INDEX(articulo) \
					)ENGINE=InnoDB");
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("INSERT INTO tmpdcartasporte20 \
					SELECT \
						d.cartaporte20 AS referencia, \
						@articulo_reemplazo, \
						a.producto, \
						a.present, \
						@multiplo_reemplazo, \
						IF(a.multiplo=@multiplo_reemplazo,SUM(IF(a.multiplo=@multiplo_reemplazo, 0, (d.cantidad/@min_factor_reemplazo))),(d.cantidad/@min_factor_reemplazo)) AS cantidad_sumar \
					FROM dcartasporte20 d \
					INNER JOIN articulos a ON a.articulo = d.articulo \
					WHERE a.producto = @producto AND a.present = @present \
						AND a.multiplo IN(@multiplo, @multiplo_reemplazo) \
					GROUP BY d.cartaporte20, a.producto, a.present \
					HAVING COUNT(d.cartaporte20) > 1");
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("UPDATE dcartasporte20 d \
					INNER JOIN tmpdcartasporte20 tmpd ON tmpd.referencia = d.cartaporte20 AND d.articulo = tmpd.articulo \
					SET \
					d.cantidad = (d.cantidad+IFNULL(tmpd.cantidadsumar,0)) ");
					instrucciones[num_instrucciones++]=Instruccion;

					for (int i = 0; i < resp_verificacion_cartas_porte20->ObtieneNumRegistros(); i++) {
						resp_verificacion_cartas_porte20->IrAlRegistroNumero(i);

						AnsiString ref_eliminar = "";
						ref_eliminar = resp_verificacion_cartas_porte20->ObtieneDato("referencia");

						if(resp_verificacion_cartas_porte20->ObtieneDato("tipo")=="MULTIPLE"){

							Instruccion.sprintf("DELETE FROM dcartasporte20 WHERE cartaporte20 = '%s' \
							AND articulo = @articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;
						} else {

							Instruccion.sprintf("UPDATE dcartasporte20 d \
							SET d.articulo=@articulo_reemplazo, d.cantidad=d.cantidad/@factor_reemplazo \
							WHERE d.cartaporte20 = '%s' AND d.articulo=@articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;

						}

						if(num_instrucciones>=100000-100)
							throw Exception("El número de instrucciones sale de los límites en Eliminar artículo");

					}

				}
			}
		} __finally {
			if (resp_verificacion_cartas_porte20!=NULL) delete resp_verificacion_cartas_porte20;
		}

		try {
			Instruccion.sprintf("SELECT t.referencia, t.numRef, t.producto, t.present, t.multiplo, t.tipo \
			FROM (( \
			SELECT \
				d.referencia AS referencia, \
				COUNT(d.referencia) AS numRef, \
				a.producto, \
				a.present, \
				a.multiplo, \
				if(COUNT(a.multiplo) > 1, 'MULTIPLE', 'UNICO') AS tipo \
			FROM dcartaspart d \
			INNER JOIN articulos a ON a.articulo = d.articulo \
			WHERE a.producto = '%s' AND a.present = '%s' \
				AND a.multiplo IN('%s', '%s') \
			GROUP BY d.referencia, a.producto, a.present \
			HAVING COUNT(d.referencia) > 1 \
			) UNION ALL ( \
			SELECT \
				d.referencia AS referencia, \
				COUNT(d.referencia) AS numRef, \
				a.producto, \
				a.present, \
				a.multiplo, \
				if(COUNT(a.multiplo) = 1, 'UNICO', 'N/A') AS tipo \
			FROM dcartaspart d \
			INNER JOIN articulos a ON a.articulo = d.articulo \
			WHERE a.producto = '%s' AND a.present = '%s' \
				AND a.multiplo IN('%s', '%s') \
			GROUP BY d.referencia, a.producto, a.present, a.multiplo \
			HAVING COUNT(d.referencia) = 1 AND multiplo != '%s')) t \
			GROUP BY t.referencia, t.producto, t.present \
			ORDER BY t.tipo ",
			producto, present, multiplo, multiplo_reemp,
			producto, present, multiplo, multiplo_reemp,
			multiplo_reemp);
			if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), resp_verificacion_cartas_part)) {
				if (resp_verificacion_cartas_part->ObtieneNumRegistros()>0){

					instrucciones[num_instrucciones++]="DROP TABLE IF EXISTS tmpdcartaspart";
					Instruccion.sprintf("CREATE TEMPORARY TABLE tmpdcartaspart ( \
					referencia VARCHAR(11), \
					articulo VARCHAR(9), \
					producto VARCHAR(8), \
					present VARCHAR(255), \
					multiplo VARCHAR(10), \
					cantidadsumar DECIMAL(12, 3), \
					INDEX(articulo) \
					)ENGINE=InnoDB");
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("INSERT INTO tmpdcartaspart \
					SELECT \
						d.referencia AS referencia, \
						@articulo_reemplazo, \
						a.producto, \
						a.present, \
						@multiplo_reemplazo, \
						IF(a.multiplo=@multiplo_reemplazo,SUM(IF(a.multiplo=@multiplo_reemplazo, 0, (d.cantidad/@min_factor_reemplazo))),(d.cantidad/@min_factor_reemplazo)) AS cantidad_sumar \
					FROM dcartaspart d \
					INNER JOIN articulos a ON a.articulo = d.articulo \
					WHERE a.producto = @producto AND a.present = @present \
						AND a.multiplo IN(@multiplo, @multiplo_reemplazo) \
					GROUP BY d.referencia, a.producto, a.present \
					HAVING COUNT(d.referencia) > 1");
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("UPDATE dcartaspart d \
					INNER JOIN tmpdcartaspart tmpd ON tmpd.referencia = d.referencia AND d.articulo = tmpd.articulo \
					SET \
					d.cantidad = (d.cantidad+IFNULL(tmpd.cantidadsumar,0)) ");
					instrucciones[num_instrucciones++]=Instruccion;

					for (int i = 0; i < resp_verificacion_cartas_part->ObtieneNumRegistros(); i++) {
						resp_verificacion_cartas_part->IrAlRegistroNumero(i);

						AnsiString ref_eliminar = "";
						ref_eliminar = resp_verificacion_cartas_part->ObtieneDato("referencia");

						if(resp_verificacion_cartas_part->ObtieneDato("tipo")=="MULTIPLE"){

							Instruccion.sprintf("DELETE FROM dcartaspart WHERE referencia = '%s' \
							AND articulo = @articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;
						} else {

							Instruccion.sprintf("UPDATE dcartaspart d \
							SET d.articulo=@articulo_reemplazo, d.cantidad=d.cantidad/@factor_reemplazo \
							WHERE d.referencia = '%s' AND d.articulo=@articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;

						}

						if(num_instrucciones>=100000-100)
							throw Exception("El número de instrucciones sale de los límites en Eliminar artículo");

					}

				}
			}
		} __finally {
			if (resp_verificacion_cartas_part!=NULL) delete resp_verificacion_cartas_part;
		}

		try {
			Instruccion.sprintf("SELECT t.referencia, t.numRef, t.producto, t.present, t.multiplo, t.tipo \
			FROM (( \
			SELECT \
				d.nopedido AS referencia, \
				COUNT(d.nopedido) AS numRef, \
				a.producto, \
				a.present, \
				a.multiplo, \
				if(COUNT(a.multiplo) > 1, 'MULTIPLE', 'UNICO') AS tipo \
			FROM ecommercedetalle d \
			INNER JOIN articulos a ON a.articulo = d.sku \
			WHERE a.producto = '%s' AND a.present = '%s' \
				AND a.multiplo IN('%s', '%s') \
			GROUP BY d.nopedido, a.producto, a.present \
			HAVING COUNT(d.nopedido) > 1 \
			) UNION ALL ( \
			SELECT \
				d.nopedido AS referencia, \
				COUNT(d.nopedido) AS numRef, \
				a.producto, \
				a.present, \
				a.multiplo, \
				if(COUNT(a.multiplo) = 1, 'UNICO', 'N/A') AS tipo \
			FROM ecommercedetalle d \
			INNER JOIN articulos a ON a.articulo = d.sku \
			WHERE a.producto = '%s' AND a.present = '%s' \
				AND a.multiplo IN('%s', '%s') \
			GROUP BY d.nopedido, a.producto, a.present, a.multiplo \
			HAVING COUNT(d.nopedido) = 1 AND multiplo != '%s')) t \
			GROUP BY t.referencia, t.producto, t.present \
			ORDER BY t.tipo ",
			producto, present, multiplo, multiplo_reemp,
			producto, present, multiplo, multiplo_reemp,
			multiplo_reemp);
			if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), resp_verificacion_ecomm_detalle)) {
				if (resp_verificacion_ecomm_detalle->ObtieneNumRegistros()>0){

					instrucciones[num_instrucciones++]="DROP TABLE IF EXISTS tmpecommercedetalle";
					Instruccion.sprintf("CREATE TEMPORARY TABLE tmpecommercedetalle ( \
					referencia VARCHAR(11), \
					articulo VARCHAR(9), \
					producto VARCHAR(8), \
					present VARCHAR(255), \
					multiplo VARCHAR(10), \
					cantidadsumar DECIMAL(12, 3), \
					precio_sumar DECIMAL(12, 3), \
					descuento_sumar DECIMAL(12, 3), \
					INDEX(articulo) \
					)ENGINE=InnoDB");
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("INSERT INTO tmpecommercedetalle \
					SELECT \
						d.nopedido AS referencia, \
						@articulo_reemplazo, \
						a.producto, \
						a.present, \
						@multiplo_reemplazo, \
						IF(a.multiplo=@multiplo_reemplazo,SUM(IF(a.multiplo=@multiplo_reemplazo, 0, (d.cantidad/@min_factor_reemplazo))),(d.cantidad/@min_factor_reemplazo)) AS cantidad_sumar, \
						SUM(d.precio*d.cantidad) AS precio_sumar, \
						SUM(d.descuento*d.cantidad) AS descuento_sumar \
					FROM ecommercedetalle d \
					INNER JOIN articulos a ON a.articulo = d.sku \
					WHERE a.producto = @producto AND a.present = @present \
						AND a.multiplo IN(@multiplo, @multiplo_reemplazo) \
					GROUP BY d.nopedido, a.producto, a.present \
					HAVING COUNT(d.nopedido) > 1");
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("UPDATE ecommercedetalle d \
					INNER JOIN tmpecommercedetalle tmpd ON tmpd.referencia = d.nopedido AND d.sku = tmpd.articulo \
					SET \
					d.cantidad = (d.cantidad+IFNULL(tmpd.cantidadsumar,0)), \
					d.precio = (tmpd.precio_sumar/(d.cantidad+tmpd.cantidadsumar)), \
					d.descuento = (tmpd.descuento_sumar/(d.cantidad+tmpd.cantidadsumar)) ");
					instrucciones[num_instrucciones++]=Instruccion;

					for (int i = 0; i < resp_verificacion_ecomm_detalle->ObtieneNumRegistros(); i++) {
						resp_verificacion_ecomm_detalle->IrAlRegistroNumero(i);

						AnsiString ref_eliminar = "";
						ref_eliminar = resp_verificacion_ecomm_detalle->ObtieneDato("referencia");

						if(resp_verificacion_ecomm_detalle->ObtieneDato("tipo")=="MULTIPLE"){

							Instruccion.sprintf("DELETE FROM ecommercedetalle WHERE nopedido = '%s' \
							AND sku = @articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;
						} else {

							Instruccion.sprintf("UPDATE ecommercedetalle d \
							SET d.sku=@articulo_reemplazo, d.cantidad=d.cantidad/@factor_reemplazo, \
							d.precio=d.precio*@factor_reemplazo, d.descuento=d.descuento*@factor_reemplazo \
							WHERE d.nopedido = %s AND d.sku=@articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;

						}

						if(num_instrucciones>=100000-100)
							throw Exception("El número de instrucciones sale de los límites en Eliminar artículo");

					}

				}
			}
		} __finally {
			if (resp_verificacion_ecomm_detalle!=NULL) delete resp_verificacion_ecomm_detalle;
		}

		try {
			Instruccion.sprintf("SELECT t.referencia, t.numRef, t.producto, t.present, t.multiplo, t.tipo \
			FROM (( \
			SELECT \
				d.id AS referencia, \
				COUNT(d.id) AS numRef, \
				a.producto, \
				a.present, \
				a.multiplo, \
				if(COUNT(a.multiplo) > 1, 'MULTIPLE', 'UNICO') AS tipo \
			FROM resurtiranaquel d \
			INNER JOIN articulos a ON a.articulo = d.articulo \
			WHERE a.producto = '%s' AND a.present = '%s' \
				AND a.multiplo IN('%s', '%s') \
			GROUP BY d.id, a.producto, a.present \
			HAVING COUNT(d.id) > 1 \
			) UNION ALL ( \
			SELECT \
				d.id AS referencia, \
				COUNT(d.id) AS numRef, \
				a.producto, \
				a.present, \
				a.multiplo, \
				if(COUNT(a.multiplo) = 1, 'UNICO', 'N/A') AS tipo \
			FROM resurtiranaquel d \
			INNER JOIN articulos a ON a.articulo = d.articulo \
			WHERE a.producto = '%s' AND a.present = '%s' \
				AND a.multiplo IN('%s', '%s') \
			GROUP BY d.id, a.producto, a.present, a.multiplo \
			HAVING COUNT(d.id) = 1 AND multiplo != '%s')) t \
			GROUP BY t.referencia, t.producto, t.present \
			ORDER BY t.tipo ",
			producto, present, multiplo, multiplo_reemp,
			producto, present, multiplo, multiplo_reemp,
			multiplo_reemp);
			if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), resp_verificacion_resurtir_anaquel)) {
				if (resp_verificacion_resurtir_anaquel->ObtieneNumRegistros()>0){

					instrucciones[num_instrucciones++]="DROP TABLE IF EXISTS tmpresurtiranaquel";
					Instruccion.sprintf("CREATE TEMPORARY TABLE tmpresurtiranaquel ( \
					referencia VARCHAR(11), \
					articulo VARCHAR(9), \
					producto VARCHAR(8), \
					present VARCHAR(255), \
					multiplo VARCHAR(10), \
					cantidadsumar DECIMAL(12, 3), \
					INDEX(articulo) \
					)ENGINE=InnoDB");
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("INSERT INTO tmpresurtiranaquel \
					SELECT \
						d.id AS referencia, \
						@articulo_reemplazo, \
						a.producto, \
						a.present, \
						@multiplo_reemplazo, \
						IF(a.multiplo=@multiplo_reemplazo,SUM(IF(a.multiplo=@multiplo_reemplazo, 0, (d.cantsurtir/@min_factor_reemplazo))),(d.cantsurtir/@min_factor_reemplazo)) AS cantidad_sumar \
					FROM resurtiranaquel d \
					INNER JOIN articulos a ON a.articulo = d.articulo \
					WHERE a.producto = @producto AND a.present = @present \
						AND a.multiplo IN(@multiplo, @multiplo_reemplazo) \
					GROUP BY d.id, a.producto, a.present \
					HAVING COUNT(d.id) > 1");
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("UPDATE resurtiranaquel d \
					INNER JOIN tmpresurtiranaquel tmpd ON tmpd.referencia = d.id AND d.articulo = tmpd.articulo \
					SET \
					d.cantsurtir = (d.cantsurtir+IFNULL(tmpd.cantidadsumar,0)) ");
					instrucciones[num_instrucciones++]=Instruccion;

					for (int i = 0; i < resp_verificacion_resurtir_anaquel->ObtieneNumRegistros(); i++) {
						resp_verificacion_resurtir_anaquel->IrAlRegistroNumero(i);

						AnsiString ref_eliminar = "";
						ref_eliminar = resp_verificacion_resurtir_anaquel->ObtieneDato("referencia");

						if(resp_verificacion_resurtir_anaquel->ObtieneDato("tipo")=="MULTIPLE"){

							Instruccion.sprintf("DELETE FROM resurtiranaquel WHERE id = %s \
							AND articulo = @articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;
						} else {

							Instruccion.sprintf("UPDATE resurtiranaquel d \
							SET d.articulo=@articulo_reemplazo, d.cantsurtir=d.cantsurtir/@factor_reemplazo \
							WHERE d.id = %s AND d.articulo=@articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;

						}

						if(num_instrucciones>=100000-100)
							throw Exception("El número de instrucciones sale de los límites en Eliminar artículo");

					}

				}
			}
		} __finally {
			if (resp_verificacion_resurtir_anaquel!=NULL) delete resp_verificacion_resurtir_anaquel;
		}

		try {
			Instruccion.sprintf("SELECT t.referencia, t.numRef, t.producto, t.present, t.multiplo, t.tipo \
			FROM (( \
			SELECT \
				d.articulo_local AS referencia, \
				COUNT(d.articulo_local) AS numRef, \
				a.producto, \
				a.present, \
				a.multiplo, \
				if(COUNT(a.multiplo) > 1, 'MULTIPLE', 'UNICO') AS tipo \
			FROM darticulospedidossinexistencia d \
			INNER JOIN articulos a ON a.articulo = d.articulo_local \
			WHERE a.producto = '%s' AND a.present = '%s' \
				AND a.multiplo IN('%s', '%s') \
			GROUP BY d.articulo_local, a.producto, a.present \
			HAVING COUNT(d.articulo_local) > 1 \
			) UNION ALL ( \
			SELECT \
				d.articulo_local AS referencia, \
				COUNT(d.articulo_local) AS numRef, \
				a.producto, \
				a.present, \
				a.multiplo, \
				if(COUNT(a.multiplo) = 1, 'UNICO', 'N/A') AS tipo \
			FROM darticulospedidossinexistencia d \
			INNER JOIN articulos a ON a.articulo = d.articulo_local \
			WHERE a.producto = '%s' AND a.present = '%s' \
				AND a.multiplo IN('%s', '%s') \
			GROUP BY d.articulo_local, a.producto, a.present, a.multiplo \
			HAVING COUNT(d.articulo_local) = 1 AND multiplo != '%s')) t \
			GROUP BY t.referencia, t.producto, t.present \
			ORDER BY t.tipo ",
			producto, present, multiplo, multiplo_reemp,
			producto, present, multiplo, multiplo_reemp,
			multiplo_reemp);
			if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), resp_verificacion_articulos_pedidos_sinexistencia)) {
				if (resp_verificacion_articulos_pedidos_sinexistencia->ObtieneNumRegistros()>0){

					instrucciones[num_instrucciones++]="DROP TABLE IF EXISTS tmpdarticulospedidossinexistencia";
					Instruccion.sprintf("CREATE TEMPORARY TABLE tmpdarticulospedidossinexistencia ( \
					referencia VARCHAR(11), \
					articulo VARCHAR(9), \
					producto VARCHAR(8), \
					present VARCHAR(255), \
					multiplo VARCHAR(10), \
					existencias_localsumar DECIMAL(12, 3), \
					cantidad_calculadasumar DECIMAL(12, 3), \
					cantidad_pedidasumar DECIMAL(12, 3), \
					INDEX(articulo) \
					)ENGINE=InnoDB");
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("INSERT INTO tmpdarticulospedidossinexistencia \
					SELECT \
						d.articulo_local AS referencia, \
						@articulo_reemplazo, \
						a.producto, \
						a.present, \
						@multiplo_reemplazo, \
						IF(a.multiplo=@multiplo_reemplazo,SUM(IF(a.multiplo=@multiplo_reemplazo, 0, (d.existencias_local/@min_factor_reemplazo))),(d.existencias_local/@min_factor_reemplazo)) AS existencias_localsumar, \
						IF(a.multiplo=@multiplo_reemplazo,SUM(IF(a.multiplo=@multiplo_reemplazo, 0, (d.cantidad_calculada/@min_factor_reemplazo))),(d.cantidad_calculada/@min_factor_reemplazo)) AS cantidad_calculadasumar, \
						IF(a.multiplo=@multiplo_reemplazo,SUM(IF(a.multiplo=@multiplo_reemplazo, 0, (d.cantidad_pedida/@min_factor_reemplazo))),(d.cantidad_pedida/@min_factor_reemplazo)) AS cantidad_pedidasumar \
					FROM darticulospedidossinexistencia d \
					INNER JOIN articulos a ON a.articulo = d.articulo_local \
					WHERE a.producto = @producto AND a.present = @present \
						AND a.multiplo IN(@multiplo, @multiplo_reemplazo) \
					GROUP BY d.articulo_local, a.producto, a.present \
					HAVING COUNT(d.articulo_local) > 1");
					instrucciones[num_instrucciones++]=Instruccion;

					Instruccion.sprintf("UPDATE darticulospedidossinexistencia d \
					INNER JOIN tmpdarticulospedidossinexistencia tmpd ON tmpd.referencia = d.articulo_local AND d.articulo_local = tmpd.articulo \
					SET \
					d.existencias_local = (d.existencias_local+IFNULL(tmpd.existencias_localsumar,0)), \
					d.cantidad_calculada = (d.cantidad_calculada+IFNULL(tmpd.cantidad_calculadasumar,0)), \
					d.cantidad_pedida = (d.cantidad_pedida+IFNULL(tmpd.cantidad_pedidasumar,0)) ");
					instrucciones[num_instrucciones++]=Instruccion;

					for (int i = 0; i < resp_verificacion_articulos_pedidos_sinexistencia->ObtieneNumRegistros(); i++) {
						resp_verificacion_articulos_pedidos_sinexistencia->IrAlRegistroNumero(i);

						AnsiString ref_eliminar = "";
						ref_eliminar = resp_verificacion_articulos_pedidos_sinexistencia->ObtieneDato("referencia");

						if(resp_verificacion_articulos_pedidos_sinexistencia->ObtieneDato("tipo")=="MULTIPLE"){

							Instruccion.sprintf("DELETE FROM darticulospedidossinexistencia WHERE articulo_local = '%s' \
							AND articulo_remoto = @articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;
						} else {

							Instruccion.sprintf("UPDATE darticulospedidossinexistencia d \
							SET d.articulo_local=@articulo_reemplazo, d.existencias_local=d.existencias_local/@factor_reemplazo, \
							d.cantidad_calculada=d.cantidad_calculada/@factor_reemplazo, \
							d.cantidad_pedida=d.cantidad_pedida/@factor_reemplazo \
							WHERE d.articulo_local = '%s' AND d.articulo_remoto=@articulo_borrar ", ref_eliminar);
							instrucciones[num_instrucciones++]=Instruccion;

						}

						if(num_instrucciones>=100000-100)
							throw Exception("El número de instrucciones sale de los límites en Eliminar artículo");

					}

				}
			}
		} __finally {
			if (resp_verificacion_articulos_pedidos_sinexistencia!=NULL) delete resp_verificacion_articulos_pedidos_sinexistencia;
		}

		/****************************************************
		*                                                   *
		*                                                   *
		*                                                   *
		*	INICIO UPDATE DE TABLAS HISTORICAS PRECACULADAS *
		*                                                   *
		*                                                   *
		*                                                   *
		****************************************************/


		Instruccion.sprintf("UPDATE precalculocostosmovalmadet SET cantidad = (cantidad/@min_factor_reemplazo) \
		WHERE producto = @producto AND present = @present");
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("UPDATE precalculocostosventadet SET cantidad = (cantidad/@min_factor_reemplazo) \
		WHERE producto = @producto AND present = @present");
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("UPDATE precalculoventasdet SET unidades = (unidades/@min_factor_reemplazo) \
		WHERE producto = @producto AND present = @present");
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("UPDATE presentcajlogisfactor \
		SET cajalogisticafactor = cajalogisticafactor/@min_factor_reemplazo \
		WHERE producto = @producto AND present = @present");
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("UPDATE puntoscorteexistencias \
		SET cantidad = cantidad/@min_factor_reemplazo \
		WHERE producto = @producto AND present = @present");
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("UPDATE ventasxmes \
		SET cant1 = (cant1/@min_factor_reemplazo), \
		cant2 = (cant2/@min_factor_reemplazo), \
		cant3 = (cant3/@min_factor_reemplazo), \
		cant4 = (cant4/@min_factor_reemplazo), \
		cant5 = (cant5/@min_factor_reemplazo), \
		cant6 = (cant6/@min_factor_reemplazo), \
		cant7 = (cant7/@min_factor_reemplazo), \
		cant8 = (cant8/@min_factor_reemplazo), \
		cant9 = (cant9/@min_factor_reemplazo), \
		cant10 = (cant10/@min_factor_reemplazo), \
		cant11 = (cant11/@min_factor_reemplazo), \
		cant12 = (cant12/@min_factor_reemplazo), \
		cant13 = (cant13/@min_factor_reemplazo), \
		cant14 = (cant14/@min_factor_reemplazo), \
		cant15 = (cant15/@min_factor_reemplazo), \
		cant16 = (cant16/@min_factor_reemplazo), \
		cant17 = (cant17/@min_factor_reemplazo), \
		cant18 = (cant18/@min_factor_reemplazo), \
		cant19 = (cant19/@min_factor_reemplazo), \
		cant20 = (cant20/@min_factor_reemplazo), \
		cant21 = (cant21/@min_factor_reemplazo), \
		cant22 = (cant22/@min_factor_reemplazo), \
		cant23 = (cant23/@min_factor_reemplazo), \
		cant24 = (cant24/@min_factor_reemplazo), \
		cant25 = (cant25/@min_factor_reemplazo), \
		cant26 = (cant26/@min_factor_reemplazo), \
		cant27 = (cant27/@min_factor_reemplazo), \
		cant28 = (cant28/@min_factor_reemplazo), \
		cant29 = (cant29/@min_factor_reemplazo), \
		cant30 = (cant30/@min_factor_reemplazo), \
		cant31 = (cant31/@min_factor_reemplazo), \
		cant32 = (cant32/@min_factor_reemplazo), \
		cant33 = (cant33/@min_factor_reemplazo), \
		cant34 = (cant34/@min_factor_reemplazo), \
		cant35 = (cant35/@min_factor_reemplazo), \
		cant36 = (cant36/@min_factor_reemplazo), \
		ventas30 = (ventas30/@min_factor_reemplazo), \
		ventas60 = (ventas60/@min_factor_reemplazo), \
		ventas90 = (ventas90/@min_factor_reemplazo), \
		ventas180 = (ventas180/@min_factor_reemplazo), \
		ventascorte = (ventascorte/@min_factor_reemplazo) \
		WHERE producto = @producto AND present = @present");
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("UPDATE existenciasactuales  \
		SET cantidad = (cantidad/@min_factor_reemplazo), ventas = (ventas/@min_factor_reemplazo), \
		devventas = (devventas/@min_factor_reemplazo), compras = (compras/@min_factor_reemplazo), \
		devcompras = (devcompras/@min_factor_reemplazo), entradas = (entradas/@min_factor_reemplazo), \
		salidas = (salidas/@min_factor_reemplazo), cantinicial = (cantinicial/@min_factor_reemplazo) \
		WHERE producto = @producto AND present = @present ");
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("UPDATE existenciasiniciales  \
		SET cantidad = (cantidad/@min_factor_reemplazo), ventas = (ventas/@min_factor_reemplazo), \
		devventas = (devventas/@min_factor_reemplazo), compras = (compras/@min_factor_reemplazo), \
		devcompras = (devcompras/@min_factor_reemplazo), entradas = (entradas/@min_factor_reemplazo), \
		salidas = (salidas/@min_factor_reemplazo), cantinicial = (cantinicial/@min_factor_reemplazo) \
		WHERE producto = @producto AND present = @present ");
		instrucciones[num_instrucciones++]=Instruccion;

		//-------------Tablas divididas por empresa---------------------------
		for(int i = 0; i < respuesta_empresas->ObtieneNumRegistros(); i++){

			respuesta_empresas->IrAlRegistroNumero(i);
			AnsiString registroEmpresa = respuesta_empresas->ObtieneDato();

			Instruccion.sprintf("UPDATE precalculocostos%s SET cantidad = (cantidad/@min_factor_reemplazo), \
				costounit = (costounit*@min_factor_reemplazo) \
				WHERE producto = @producto AND present = @present", registroEmpresa);
			instrucciones[num_instrucciones++]=Instruccion;

			Instruccion.sprintf("UPDATE precalculocostospromedio%s SET cantidad = (cantidad/@min_factor_reemplazo), \
			costounit = (costounit*@min_factor_reemplazo) \
			WHERE producto = @producto AND present = @present", registroEmpresa);
			instrucciones[num_instrucciones++]=Instruccion;

			Instruccion.sprintf("UPDATE precalculohistoriaexist%s SET cantidadent = (cantidadent/@min_factor_reemplazo), \
				cantidadsal = (cantidadsal/@min_factor_reemplazo), cantidadacum = (cantidadacum/@min_factor_reemplazo) \
				WHERE producto = @producto AND present = @present", registroEmpresa);
			instrucciones[num_instrucciones++]=Instruccion;

			/*
			Instruccion.sprintf("UPDATE precalculomensual%s SET acumcantidad = (acumcantidad/@min_factor_reemplazo), \
				costoprom = (costoprom*@min_factor_reemplazo) \
				WHERE producto = @producto AND present = @present", registroEmpresa);
			instrucciones[num_instrucciones++]=Instruccion;
			*/

			Instruccion.sprintf("UPDATE costospromedioiniciales%s SET cantidad = (cantidad/@min_factor_reemplazo), \
				costounit = (costounit*@min_factor_reemplazo) \
				WHERE producto = @producto AND present = @present", registroEmpresa);
			instrucciones[num_instrucciones++]=Instruccion;
		}//-------------Tablas divididas por empresa--------------------------

		/****************************************************
		*                                                   *
		*                                                   *
		*                                                   *
		*			   INICIO DELETE DE TABLAS 				*
		*                                                   *
		*                                                   *
		*                                                   *
		****************************************************/


		Instruccion.sprintf("DELETE FROM preciolocal WHERE articulo = @articulo_borrar");
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("DELETE FROM precios WHERE articulo = @articulo_borrar");
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("DELETE FROM bitacoracostos WHERE articulo = @articulo_borrar");
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("DELETE FROM bitacoradetalletagecommerce WHERE articulo = @articulo_borrar");
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("DELETE FROM bitacoradmovalmamod WHERE articulo = @articulo_borrar");
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("DELETE FROM bitacorasolicitudesnotascredcli WHERE articulo = @articulo_borrar");
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("DELETE FROM imagenesarticulos WHERE articulo = @articulo_borrar");
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("DELETE FROM articuloempresacfg WHERE articulo = @articulo_borrar");
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("DELETE FROM articulosequiv WHERE articulo = @articulo_borrar");
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("DELETE FROM articulosunif WHERE articulo = @articulo_borrar");
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("DELETE FROM articuloxseccion WHERE articulo = @articulo_borrar");
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("DELETE FROM articulosecomtagsasignados WHERE articulo = @articulo_borrar");
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("DELETE FROM descripcionarticulo WHERE articulo = @articulo_borrar");
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("DELETE FROM dprodxcotizar WHERE articulo = @articulo_borrar");
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("DELETE FROM ecommerceproductos WHERE articulo = @articulo_borrar");
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("DELETE FROM erroresembarques WHERE articulo = @articulo_borrar");
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("DELETE FROM codigosbarras WHERE articulo = @articulo_borrar");
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("DELETE FROM pesopromporarticulo WHERE articulo = @articulo_borrar");
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("UPDATE presentaciones SET factortarima = (factortarima/@min_factor_reemplazo) \
		WHERE producto = @producto AND present = @present");
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("UPDATE precios SET costo = costo*@min_factor_reemplazo \
		WHERE articulo=@articulo_reemplazo");
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("UPDATE presentacionescb SET costoultimo = costoultimo*@min_factor_reemplazo, \
		costobase = costobase*@min_factor_reemplazo \
		WHERE producto=@producto AND present=@present AND idempresa=%s", FormServidor->ObtieneClaveEmpresa());
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("UPDATE articulos SET factor = factor/@min_factor_reemplazo \
		WHERE producto=@producto AND present=@present AND factor>=@min_factor_reemplazo");
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("UPDATE presentacionesminmax SET minmult = @multiplo_reemplazo \
		WHERE producto = @producto AND present = @present");
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("UPDATE presentacionesminmax \
		SET maxfactor = maxfactor/@min_factor_reemplazo, \
		cajalogisticafactor = cajalogisticafactor/@min_factor_reemplazo \
		WHERE producto = @producto AND present = @present");
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("DELETE FROM articulos WHERE articulo = @articulo_borrar");
		instrucciones[num_instrucciones++]=Instruccion;

    	Instruccion.sprintf("DELETE FROM camprogprodpresent where id = %s", idProgramado);
		instrucciones[num_instrucciones++]=Instruccion;

		instrucciones[num_instrucciones++]="COMMIT";
		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=1";

		instrucciones[num_instrucciones++]="SET SESSION sql_log_bin = 1";

		Instruccion.sprintf("INSERT INTO bitprodpresent (articulo, prod_origen, present_origen, mult_origen, mult_destino, tipo, usuario, fecha_prog, mensaje, suc_prog, sucursal, correcto) VALUES \
			('%s', '%s', '%s', '%s', '%s', 'EA', '%s', '%s', 'Artículo eliminado correctamente.', '%s', '%s', 1)",
			articulo, producto, present, multiplo, multiplo_reemp, usuario, fecha_prog, suc_prog, FormServidor->ObtieneClaveSucursal());
			instrucciones[num_instrucciones++]=Instruccion;

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++) {
			if((aux_buffer_sql-buffer_sql)>1024*1024*(128-1))
				throw Exception("El buffer puede desbordarse, por lo que es mejor abortar la operación en Eliminar artículo.");

			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);
		}

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		if(mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)){
			try{
				mServidorVioleta->MuestraMensaje("-- * * * * * * INICIO Parte de ID_GRA_PRESENT_ACTIVA");
				puntero_funciones = mFg.AgregaStringABuffer(producto, puntero_funciones);
				puntero_funciones = mFg.AgregaStringABuffer(present, puntero_funciones);
				GrabaPresentacionActiva(Respuesta, MySQL, buffer_funciones);
				mServidorVioleta->MuestraMensaje("-- * * * * * * FIN Parte de ID_GRA_PRESENT_ACTIVA");
			}__finally{

			}

		}

	} __finally {

		if(buffer_sql != NULL)
			delete buffer_sql;
		delete[] instrucciones;
		delete respuesta_empresas;
    	delete buffer_funciones;
	}
}
// ---------------------------------------------------------------------------
//ID_ROMPE_CAJA
void ServidorCatalogos::RomperCaja(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	BufferRespuestas* resp_art=NULL;
	BufferRespuestas* resp_prod_pres=NULL;
	BufferRespuestas * resp_factor = NULL;
	char *buffer_sql=new char[1024*32];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[5000];
	AnsiString articulo, factcaja, multiplo, codbarra, usuario, idEmpresa, idProgramado;
	AnsiString articulos, sucursalProgramada, fecha_prog;

	articulo = mFg.ExtraeStringDeBuffer(&parametros);
	factcaja = mFg.ExtraeStringDeBuffer(&parametros);
	multiplo = mFg.ExtraeStringDeBuffer(&parametros);
	codbarra = mFg.ExtraeStringDeBuffer(&parametros);
	usuario = mFg.ExtraeStringDeBuffer(&parametros);
	idProgramado = mFg.ExtraeStringDeBuffer(&parametros);
	sucursalProgramada = mFg.ExtraeStringDeBuffer(&parametros);
	fecha_prog = mFg.ExtraeStringDeBuffer(&parametros);

	idEmpresa = FormServidor->ObtieneClaveEmpresa();

    BufferRespuestas * respuesta_empresas = NULL;

	try {
		Instruccion.sprintf("SELECT factor FROM articulos WHERE articulo = '%s'", articulo);
		if(mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), resp_factor)){
			if(resp_factor->ObtieneDato() != "1"){
				AnsiString mensaje;
				mensaje.sprintf("Se programó ROMPER CAJA para el artículo %s con factor %s. El factor debe ser 1", articulo, resp_factor->ObtieneDato());
				throw Exception(mensaje);
			}
		}

		Instruccion.sprintf("SELECT producto, present FROM articulos WHERE articulo = '%s' ",
		articulo);
		if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), resp_art)) {
			if (resp_art->ObtieneNumRegistros()>0){
                try {
					Instruccion.sprintf("SELECT articulo FROM articulos WHERE producto = '%s' \
					AND present = '%s' AND FACTOR <> 1 ",
					resp_art->ObtieneDato("producto"),
					resp_art->ObtieneDato("present"));
					if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), resp_prod_pres)) {
						for (int i = 0; i < resp_prod_pres->ObtieneNumRegistros(); i++) {
                            resp_prod_pres->IrAlRegistroNumero(i);
							articulos += "'" + resp_prod_pres->ObtieneDato("articulo") + "'";
							if(i != resp_prod_pres->ObtieneNumRegistros()-1)
                                articulos += ",";
						}
					}
				} __finally {
					if (resp_prod_pres!=NULL) delete resp_prod_pres;
				}
			}
		}
	} __finally {
		if (resp_art!=NULL) delete resp_art;
	}

	instrucciones[num_instrucciones++]="SET SESSION sql_log_bin = 0";

	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	Instruccion.sprintf("SELECT @producto:=a.producto, @present:=a.present \
	FROM articulos a WHERE a.articulo='%s'", articulo);
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("UPDATE articulos a \
	SET a.factor = (factor*%s) \
	WHERE a.articulo IN(%s) ", factcaja, articulos);
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("UPDATE articulos SET factor = %s WHERE articulo = '%s'", factcaja, articulo);
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("SELECT @folioaux:=valor FROM foliosemp WHERE folio='ARTICULOS' AND sucursal = '%s' %s", sucursalProgramada, MODO_BLOQUEO_CLAVES_UNICAS);
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("SET @foliosig=@folioaux+1");
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("SET @folioaux=cast(@folioaux as char)");
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("SET @folio=concat('%s', lpad(@folioaux,7,'0'))", sucursalProgramada);
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("UPDATE foliosemp SET valor=@foliosig WHERE folio='ARTICULOS' AND sucursal = '%s' ", sucursalProgramada);
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("INSERT INTO articulos \
	SELECT @folio, producto, present, '%s', %s, (peso/%s), (volumen/%s), altura, longitud, profundidad, \
	porccomi, 1, activo, asigautosu, fechaalta, fechamodi, horaalta, horamodi, usualta, \
	usumodi, idclaveunidadcfdi \
	FROM articulos \
	WHERE articulos.articulo = '%s'",
	multiplo, codbarra,
	factcaja, factcaja, articulo);
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("SET @nuevo_articulo = (SELECT a.articulo FROM articulos a \
		INNER JOIN articulos orig ON a.producto = orig.producto AND a.present = orig.present \
		WHERE orig.articulo = '%s' AND a.multiplo = '%s')", articulo, multiplo);
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("UPDATE presentcajlogisfactor pcl \
	INNER JOIN articulos a ON a.producto = pcl.producto AND a.present = pcl.present \
	SET pcl.cajalogisticafactor = pcl.cajalogisticafactor*%s \
	WHERE a.articulo = '%s'", factcaja, articulo);
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("UPDATE presentacionesminmax pmm \
	INNER JOIN articulos a ON a.producto = pmm.producto AND a.present = pmm.present \
	SET pmm.minmult = '%s', pmm.maxfactor = pmm.maxfactor*%s, pmm.cajalogisticafactor = pmm.cajalogisticafactor*%s \
	WHERE a.articulo = '%s'", multiplo, factcaja, factcaja, articulo);
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("INSERT INTO articuloxseccion \
	SELECT @folio, seccion, almacen FROM articuloxseccion \
	WHERE articulo = '%s'", articulo);
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("UPDATE precalculocostosmovalmadet SET cantidad = (cantidad*%s) \
	WHERE producto = @producto AND present = @present", factcaja);
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("UPDATE precalculocostosventadet SET cantidad = (cantidad*%s) \
	WHERE producto = @producto AND present = @present", factcaja);
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("UPDATE precalculoventasdet SET unidades = (unidades*%s) \
	WHERE producto = @producto AND present = @present", factcaja);
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("UPDATE puntoscorteexistencias SET cantidad = cantidad*%s \
	WHERE producto = @producto AND present = @present", factcaja);
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("UPDATE precios p \
	INNER JOIN articulos a ON a.articulo = p.articulo \
	SET p.costo =  (costo/%s) \
	WHERE a.producto = @producto AND a.present = @present",
	factcaja);
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("INSERT INTO articuloempresacfg \
		SELECT @nuevo_articulo, idempresa, permitecompras, permiteventas, permitemovalma \
		FROM articuloempresacfg orig \
		WHERE articulo = '%s' \
		ON DUPLICATE KEY UPDATE \
		permitecompras = orig.permitecompras, permiteventas = orig.permiteventas, \
		permitemovalma = orig.permitemovalma", articulo);
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("INSERT INTO articulosemp \
	SELECT idempresa, @nuevo_articulo, tipoutil, activoecom, activokiosko \
	FROM articulosemp ae \
	WHERE ae.articulo = '%s'", articulo);
	instrucciones[num_instrucciones++]=Instruccion;

	//-----------Tablas divididas por empresa---------------------------------
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, "SELECT idempresa FROM empresas", respuesta_empresas);
	for(int i = 0; i < respuesta_empresas->ObtieneNumRegistros(); i++){
		respuesta_empresas->IrAlRegistroNumero(i);
		AnsiString registroEmpresa = respuesta_empresas->ObtieneDato();

		Instruccion.sprintf("UPDATE precalculocostospromedio%s SET cantidad = (cantidad*%s), \
		costounit = (costounit/%s) \
		WHERE producto = @producto AND present = @present", registroEmpresa, factcaja, factcaja);
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("UPDATE precalculocostos%s SET cantidad = (cantidad*%s), \
			costounit = (costounit/%s) \
			WHERE producto = @producto AND present = @present", registroEmpresa, factcaja, factcaja);
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("UPDATE precalculohistoriaexist%s SET cantidadent = (cantidadent*%s), \
			cantidadsal = (cantidadsal*%s), cantidadacum = (cantidadacum*%s) \
			WHERE producto = @producto AND present = @present", registroEmpresa,  factcaja, factcaja, factcaja);
		instrucciones[num_instrucciones++]=Instruccion;
		/*
		Instruccion.sprintf("UPDATE precalculomensual%s SET acumcantidad = (acumcantidad*%s), \
			costoprom = (costoprom/%s) \
			WHERE producto = @producto AND present = @present", registroEmpresa, factcaja, factcaja);
		instrucciones[num_instrucciones++]=Instruccion;
		*/

		Instruccion.sprintf("UPDATE costospromedioiniciales%s SET cantidad = (cantidad*%s), \
		costounit = (costounit/%s) \
		WHERE producto = @producto AND present = @present", registroEmpresa, factcaja, factcaja);
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("INSERT INTO precios  \
		SELECT @folio, tp.tipoprec, costo, (costo*(1+p.porcutil/100)), \
		p.porcutil, CURDATE(), (costo*(1+p.porcutil/100)), (costo*(1+p.porcutil/100)), \
		(costo*(1+p.porcutil/100)), CURTIME(), CURDATE(), CURTIME(), 0 \
		FROM precios p \
		INNER JOIN articulos a ON a.articulo = p.articulo \
		INNER JOIN tiposdeprecios tp ON tp.tipoprec = p.tipoprec \
		WHERE a.articulo = '%s' AND tp.idempresa=%s ",
		articulo, registroEmpresa);
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("SELECT @costobase:= pre.costobase, @comision:=a.porcComi \
		FROM articulos a \
		INNER JOIN presentacionescb pre ON pre.producto = a.producto AND pre.present = a.present \
		WHERE a.producto = @producto AND a.present = @present AND pre.idempresa=%s ",
		registroEmpresa);
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("UPDATE presentacionescb  \
		SET costoultimo = (costoultimo/%s), costobase = (costobase/%s) \
		WHERE producto = @producto AND present = @present AND idempresa=%s ",
		factcaja, factcaja, registroEmpresa);
		instrucciones[num_instrucciones++]=Instruccion;

	}//-----------Tablas divididas por empresa--------------------------------

	Instruccion.sprintf("UPDATE ventasxmes \
	SET cant1 = (cant1*%s), \
	cant2 = (cant2*%s), \
	cant3 = (cant3*%s), \
	cant4 = (cant4*%s), \
	cant5 = (cant5*%s), \
	cant6 = (cant6*%s), \
	cant7 = (cant7*%s), \
	cant8 = (cant8*%s), \
	cant9 = (cant9*%s), \
	cant10 = (cant10*%s), \
	cant11 = (cant11*%s), \
	cant12 = (cant12*%s), \
	cant13 = (cant13*%s), \
	cant14 = (cant14*%s), \
	cant15 = (cant15*%s), \
	cant16 = (cant16*%s), \
	cant17 = (cant17*%s), \
	cant18 = (cant18*%s), \
	cant19 = (cant19*%s), \
	cant20 = (cant20*%s), \
	cant21 = (cant21*%s), \
	cant22 = (cant22*%s), \
	cant23 = (cant23*%s), \
	cant24 = (cant24*%s), \
	cant25 = (cant25*%s), \
	cant26 = (cant26*%s), \
	cant27 = (cant27*%s), \
	cant28 = (cant28*%s), \
	cant29 = (cant29*%s), \
	cant30 = (cant30*%s), \
	cant31 = (cant31*%s), \
	cant32 = (cant32*%s), \
	cant33 = (cant33*%s), \
	cant34 = (cant34*%s), \
	cant35 = (cant35*%s), \
	cant36 = (cant36*%s), \
	ventas30 = (ventas30*%s), \
	ventas60 = (ventas60*%s), \
	ventas90 = (ventas90*%s), \
	ventas180 = (ventas180*%s), \
	ventascorte = (ventascorte*%s) \
	WHERE producto = @producto AND present = @present",
	factcaja, factcaja, factcaja, factcaja, factcaja,
	factcaja, factcaja, factcaja, factcaja, factcaja,
	factcaja, factcaja, factcaja, factcaja, factcaja,
	factcaja, factcaja, factcaja, factcaja, factcaja,
	factcaja, factcaja, factcaja, factcaja, factcaja,
	factcaja, factcaja, factcaja, factcaja, factcaja,
	factcaja, factcaja, factcaja, factcaja, factcaja,
	factcaja, factcaja, factcaja, factcaja, factcaja,
	factcaja);
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("UPDATE existenciasactuales  \
	SET cantidad = (cantidad*%s), ventas = (ventas*%s), devventas = (devventas*%s), compras = (compras*%s), \
	devcompras = (devcompras*%s), entradas = (entradas*%s), salidas = (salidas*%s), cantinicial = (cantinicial*%s) \
	WHERE producto = @producto AND present = @present ",
	factcaja, factcaja, factcaja, factcaja,
	factcaja, factcaja, factcaja, factcaja);
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("UPDATE existenciasiniciales \
	SET cantidad = (cantidad*%s), ventas = (ventas*%s), devventas = (devventas*%s), compras = (compras*%s), \
	devcompras = (devcompras*%s), entradas = (entradas*%s), salidas = (salidas*%s), cantinicial = (cantinicial*%s) \
	WHERE producto = @producto AND present = @present ",
	factcaja, factcaja, factcaja, factcaja,
	factcaja, factcaja, factcaja, factcaja);
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("UPDATE presentaciones SET \
	factortarima = (factortarima*%s) \
	WHERE producto = @producto AND present = @present ",
	factcaja);
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("INSERT INTO bitacoraart  \
	(idBitacoraArt, articulo, idempresa, usuario, fecha, hora, tipo, costoBase,comision,activo) \
    VALUES \
	(NULL, @folio,%s, '%s', CURDATE(), CURTIME(), 'A', @costobase, @comision, 1) ",
	FormServidor->ObtieneClaveEmpresa(), usuario, articulo);
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("DELETE FROM camprogprodpresent where id = %s", idProgramado);
	instrucciones[num_instrucciones++]=Instruccion;

	instrucciones[num_instrucciones++]="COMMIT";
	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=1";
	instrucciones[num_instrucciones++]="SET SESSION sql_log_bin = 1";

	Instruccion.sprintf("INSERT INTO bitprodpresent (articulo, factor_nuevo, mult_destino, ean13, usuario, \
	tipo, fecha_prog, mensaje, suc_prog, sucursal, correcto) VALUES \
	('%s', '%s', '%s', '%s', '%s', 'RC', '%s', 'Operación de romper caja correcta.', '%s', '%s', 1)",
	articulo, factcaja, multiplo, codbarra, usuario, fecha_prog, sucursalProgramada, FormServidor->ObtieneClaveSucursal());
	instrucciones[num_instrucciones++]=Instruccion;

	try {
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);

	} __finally {
		if(buffer_sql != NULL)
			delete buffer_sql;
		delete respuesta_empresas;
		delete resp_factor;
	}
}
// ---------------------------------------------------------------------------
//ID_MOD_PROD_PRES
void ServidorCatalogos::ModificarProductoPresent(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	BufferRespuestas* resp_producto_ori=NULL;
	BufferRespuestas* resp_producto_des=NULL;
	char *buffer_sql=new char[1024*32];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[5000];
	AnsiString usuario, nombre_origen, nombre_destino;
	AnsiString producto_destino, present_destino, nombre_producto_destino;
	AnsiString prod_origen, pres_origen;
    AnsiString idProgramado, suc_prog, fecha_prog;

	AnsiString idEmpresa = FormServidor->ObtieneClaveEmpresa();

	prod_origen = mFg.ExtraeStringDeBuffer(&parametros);
	pres_origen = mFg.ExtraeStringDeBuffer(&parametros);
	producto_destino = mFg.ExtraeStringDeBuffer(&parametros);
	present_destino = mFg.ExtraeStringDeBuffer(&parametros);
   	nombre_origen = mFg.ExtraeStringDeBuffer(&parametros);
	nombre_destino = mFg.ExtraeStringDeBuffer(&parametros);
	usuario = mFg.ExtraeStringDeBuffer(&parametros);
	idProgramado = mFg.ExtraeStringDeBuffer(&parametros);
	suc_prog = mFg.ExtraeStringDeBuffer(&parametros);
	fecha_prog = mFg.ExtraeStringDeBuffer(&parametros);

	instrucciones[num_instrucciones++]="SET SESSION sql_log_bin = 0";

	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	Instruccion.sprintf("SET @nombre_destino='%s'", nombre_destino);
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("SET @producto_destino='%s'", producto_destino);
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("SET @present_destino='%s'", present_destino);
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("SET @producto_origen='%s'", prod_origen);
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("SET @present_origen='%s'", pres_origen);
	instrucciones[num_instrucciones++]=Instruccion;

	//Instruccion.sprintf("ALTER TABLE presentacionesminmax \
	//DROP FOREIGN KEY if EXISTS FK2_presentacionesminmax, \
	//DROP FOREIGN KEY if EXISTS FK1_presentacionesminmax");
	//instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("INSERT IGNORE INTO productos \
	(producto, nombre, marca, fabricante, clasif1, clasif2, clasif3, claveimpv1, claveimpv2, claveimpv3, \
	claveimpv4, claveimpc1, claveimpc2, claveimpc3, claveimpc4, clasifcont, clasifcont2, \
	idgraduacion, etiquetasgr, idclaveprodservcfdi, clasifecom1, clasifecom2, clasifecom3) \
	SELECT @producto_destino AS producto, @nombre_destino AS nombre, marca, fabricante, clasif1, clasif2, clasif3, claveimpv1, claveimpv2, claveimpv3, \
	claveimpv4, claveimpc1, claveimpc2, claveimpc3, claveimpc4, clasifcont, clasifcont2, \
	idgraduacion, etiquetasgr, idclaveprodservcfdi, clasifecom1, clasifecom2, clasifecom3 \
	FROM productos WHERE producto = @producto_origen");
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("UPDATE presentaciones \
	SET producto= @producto_destino, present=@present_destino \
	WHERE producto=@producto_origen AND present=@present_origen");
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("UPDATE presentacionesminmax \
	SET producto= @producto_destino, present=@present_destino \
	WHERE producto=@producto_origen AND present=@present_origen");
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("UPDATE bitacoraarticulosunif \
	SET producto= @producto_destino, present=@present_destino \
	WHERE producto=@producto_origen AND present=@present_origen");
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("UPDATE bitacorastock \
	SET producto= @producto_destino, present=@present_destino \
	WHERE producto=@producto_origen AND present=@present_origen");
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("UPDATE existenciasactuales \
	SET producto= @producto_destino, present=@present_destino \
	WHERE producto=@producto_origen AND present=@present_origen");
	instrucciones[num_instrucciones++]=Instruccion;

	/*
	Instruccion.sprintf("UPDATE precalculocostos%s \
	SET producto= @producto_destino, present=@present_destino \
	WHERE producto=@producto_origen AND present=@present_origen", idEmpresa);
	instrucciones[num_instrucciones++]=Instruccion;
    */

	Instruccion.sprintf("UPDATE precalculocostosmovalmadet \
	SET producto= @producto_destino, present=@present_destino \
	WHERE producto=@producto_origen AND present=@present_origen");
	instrucciones[num_instrucciones++]=Instruccion;

	/*
	Instruccion.sprintf("UPDATE precalculocostospromedio \
	SET producto= @producto_destino, present=@present_destino \
	WHERE producto=@producto_origen AND present=@present_origen");
	instrucciones[num_instrucciones++]=Instruccion;
    */

	Instruccion.sprintf("UPDATE precalculocostosventadet \
	SET producto= @producto_destino, present=@present_destino \
	WHERE producto=@producto_origen AND present=@present_origen");
	instrucciones[num_instrucciones++]=Instruccion;

	/*
	Instruccion.sprintf("UPDATE precalculohistoriaconexist%s \
	SET producto= @producto_destino, present=@present_destino \
	WHERE producto=@producto_origen AND present=@present_origen", idEmpresa);
	instrucciones[num_instrucciones++]=Instruccion;


	Instruccion.sprintf("UPDATE precalculohistoriaexist%s \
	SET producto= @producto_destino, present=@present_destino \
	WHERE producto=@producto_origen AND present=@present_origen", idEmpresa);
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("UPDATE precalculomensual%s \
	SET producto= @producto_destino, present=@present_destino \
	WHERE producto=@producto_origen AND present=@present_origen", idEmpresa);
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("UPDATE precalculominmax \
	SET producto= @producto_destino, present=@present_destino \
	WHERE producto=@producto_origen AND present=@present_origen");
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("UPDATE precalculominmaxfin \
	SET producto= @producto_destino, present=@present_destino \
	WHERE producto=@producto_origen AND present=@present_origen");
	instrucciones[num_instrucciones++]=Instruccion;
    */

	Instruccion.sprintf("UPDATE precalculoventasdet \
	SET producto= @producto_destino, present=@present_destino \
	WHERE producto=@producto_origen AND present=@present_origen");
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("UPDATE unif_articulos \
	SET producto= @producto_destino, present=@present_destino \
	WHERE producto=@producto_origen AND present=@present_origen");
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("UPDATE unif_presentaciones \
	SET producto= @producto_destino, present=@present_destino \
	WHERE producto=@producto_origen AND present=@present_origen");
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("UPDATE unif_presentacionesminmax \
	SET producto= @producto_destino, present=@present_destino \
	WHERE producto=@producto_origen AND present=@present_origen");
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("UPDATE unifcostoexistencias \
	SET producto= @producto_destino, present=@present_destino \
	WHERE producto=@producto_origen AND present=@present_origen");
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("UPDATE unifexist \
	SET producto= @producto_destino, present=@present_destino \
	WHERE producto=@producto_origen AND present=@present_origen");
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("UPDATE unifexistcodigos \
	SET producto= @producto_destino, present=@present_destino \
	WHERE producto=@producto_origen AND present=@present_origen");
	instrucciones[num_instrucciones++]=Instruccion;

	//Instruccion.sprintf("ALTER TABLE presentacionesminmax \
	//ADD CONSTRAINT FK1_presentacionesminmax FOREIGN KEY IF NOT EXISTS (producto, present, maxmult) REFERENCES articulos (producto, present, multiplo) ON UPDATE CASCADE ON DELETE RESTRICT");
	//instrucciones[num_instrucciones++]=Instruccion;

	//Instruccion.sprintf("ALTER TABLE presentacionesminmax \
	//ADD CONSTRAINT FK2_presentacionesminmax FOREIGN KEY IF NOT EXISTS (producto, present, minmult) REFERENCES articulos (producto, present, multiplo) ON UPDATE CASCADE ON DELETE RESTRICT");
	//instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("UPDATE prodanotaciones \
	SET producto= @producto_destino WHERE producto=@producto_origen");
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("SET @existe = (SELECT producto FROM excluirproductosiesps WHERE \
		producto = @producto_origen);");
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("DELETE FROM excluirproductosiesps \
	WHERE producto=@producto_origen AND producto NOT IN (SELECT producto FROM presentaciones)");
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("INSERT IGNORE INTO excluirproductosiesps \
	SELECT @producto_destino WHERE !ISNULL(@existe)");
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("delete from productos \
	where producto= @producto_origen AND producto NOT IN (SELECT producto FROM presentaciones)");
	instrucciones[num_instrucciones++]=Instruccion;

	Instruccion.sprintf("DELETE FROM camprogprodpresent where id = %s", idProgramado);
	instrucciones[num_instrucciones++]=Instruccion;

    instrucciones[num_instrucciones++]="COMMIT";
	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=1";

	instrucciones[num_instrucciones++]="SET SESSION sql_log_bin = 1";

	Instruccion.sprintf("INSERT INTO bitprodpresent (prod_origen, present_origen, prod_destino, \
		present_destino, nombre_origen, nombre_destino, tipo, fecha_prog, mensaje, usuario, suc_prog, \
		sucursal, correcto) VALUES \
		('%s', '%s', '%s', '%s', '%s', '%s', 'CPP', '%s', 'Operación de cambiar producto-presentación correcta.', \
		'%s', '%s', '%s', 1)",
		prod_origen, pres_origen, producto_destino, present_destino, nombre_origen, nombre_destino, fecha_prog, usuario, suc_prog,
		FormServidor->ObtieneClaveSucursal());
	instrucciones[num_instrucciones++]=Instruccion;

	try {
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);

	} __finally {
		if(buffer_sql != NULL)
			delete buffer_sql;
	}
}
// ---------------------------------------------------------------------------
//ID_GRA_USUARIO_SIS
void ServidorCatalogos::GrabaUsuarioSistema(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	BufferRespuestas* resp_usuario=NULL;
	BufferRespuestas* resp_usuario_sucursal=NULL;
	char *buffer_sql=new char[1024*32];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[15];
	AnsiString tipo, usuario, sucursal, estatus;
	AnsiString tarea, tarea_usu_suc;

	tipo  	 = mFg.ExtraeStringDeBuffer(&parametros);
	usuario  = mFg.ExtraeStringDeBuffer(&parametros);
	sucursal = mFg.ExtraeStringDeBuffer(&parametros);
	estatus  = mFg.ExtraeStringDeBuffer(&parametros);

	try {
		Instruccion.sprintf("SELECT empleado FROM usuarios WHERE empleado = '%s' ", usuario);

		if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), resp_usuario)) {
			if (resp_usuario->ObtieneNumRegistros()>0){
				if(tipo=="A"){
					throw (Exception("El empleado ya esta registrado como usuario"));
				}else{
					tarea = "M";
				}
			} else {
				tarea = "A";
            }
		}
	} __finally {
		if (resp_usuario!=NULL) delete resp_usuario;
	}

	try {
		Instruccion.sprintf("SELECT usuario, sucursal FROM usuariosucursal WHERE usuario = '%s' AND sucursal = '%s' ",
		usuario, sucursal);

		if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), resp_usuario_sucursal)) {
			if (resp_usuario_sucursal->ObtieneNumRegistros()>0){
				if(resp_usuario_sucursal->ObtieneDato("sucursal")!=sucursal){
					tarea_usu_suc = "M";
				}
			} else {
				tarea_usu_suc = "A";
            }
		}
	} __finally {
		if (resp_usuario_sucursal!=NULL) delete resp_usuario_sucursal;
	}

	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	if(tarea=="A"){

		Instruccion.sprintf("INSERT INTO usuarios (empleado,activo,fechaalta,fechabaja) \
		VALUES ('%s',1,CURDATE(),'2099-01-01')", usuario);
		instrucciones[num_instrucciones++]=Instruccion;

	} else {

		Instruccion.sprintf("UPDATE usuarios SET activo = %s \
		WHERE empleado = '%s'", estatus, usuario);
		instrucciones[num_instrucciones++]=Instruccion;

	}

	if(tarea_usu_suc=="A"){

		Instruccion.sprintf("INSERT INTO usuariosucursal (usuario,sucursal) \
		VALUES ('%s','%s')", usuario, sucursal);
		instrucciones[num_instrucciones++]=Instruccion;

	}

	instrucciones[num_instrucciones++]="COMMIT";

	try {
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);

	} __finally {
		if(buffer_sql != NULL)
			delete buffer_sql;
	}
}

// ---------------------------------------------------------------------------
// ID_CON_EMPRESA
void ServidorCatalogos::ConsultaEmpresa(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
	AnsiString empresa = " ";

	empresa = mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_BUFFER);

	//Obtiene los datos de una empresa
	instruccion.sprintf("SELECT clave AS empresa, idempresa AS numid, nombre, sucprincipal, essuper FROM empresas WHERE clave = '%s'", empresa);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
	Respuesta->TamBufferResultado);

	// Obtiene todas las empresas
	instruccion = "SELECT clave AS empresa, idempresa AS numid, nombre, sucprincipal, essuper FROM empresas";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
	Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------
//ID_GRA_EMPRESA
void ServidorCatalogos::GrabaEmpresa(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// GRABA NUEVA EMPRESA
	char *buffer_sql = new char[300 * 500];
	char *aux_buffer_sql = buffer_sql;
	AnsiString tarea_activa, id_empresa, clave_empresa, nombre_empresa, empresa_activa, sucursal_princ, essuper;
	int num_instrucciones = 0, i;
	AnsiString instruccion, instrucciones[500];

	try {
		tarea_activa = mFg.ExtraeStringDeBuffer(&parametros);
		id_empresa = mFg.ExtraeStringDeBuffer(&parametros);
		clave_empresa = mFg.ExtraeStringDeBuffer(&parametros);
		nombre_empresa = mFg.ExtraeStringDeBuffer(&parametros);
		empresa_activa = FormServidor->ObtieneClaveEmpresa();
		sucursal_princ = mFg.ExtraeStringDeBuffer(&parametros);
		essuper = mFg.ExtraeStringDeBuffer(&parametros);

		instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++] = "START TRANSACTION";
		if (tarea_activa == "A") {
			instruccion.sprintf("INSERT IGNORE INTO empresas (idempresa, clave, nombre, sucprincipal, essuper) VALUES (%s, '%s', '%s', '%s', %s)",
								id_empresa, clave_empresa, nombre_empresa, sucursal_princ, essuper);

			instrucciones[num_instrucciones++] = instruccion;

			instruccion.sprintf("INSERT INTO parametrosglobemp (idempresa, parametro, descripcion, valor)  \
								SELECT %s AS idempresa, p.parametro,  p.descripcion, p.valor                \
								FROM parametrosglobemp p WHERE p.idempresa = %s ", id_empresa, FormServidor->ObtieneClaveEmpresa());
			instrucciones[num_instrucciones++] = instruccion;

			instruccion.sprintf("INSERT INTO paramseguridad (idempresa, parametro, descripcion, valor)     \
								SELECT %s AS idempresa, p.parametro,  p.descripcion, p.valor                \
								FROM paramseguridad p WHERE p.idempresa = %s ", id_empresa, FormServidor->ObtieneClaveEmpresa());
			instrucciones[num_instrucciones++] = instruccion;

			instruccion.sprintf("DROP TABLE IF EXISTS precalculocostos%s", id_empresa);
			instrucciones[num_instrucciones++] = instruccion;
			instruccion.sprintf("CREATE TABLE IF NOT EXISTS precalculocostos%s LIKE precalculocostos%s", id_empresa, empresa_activa);
			instrucciones[num_instrucciones++] = instruccion;

			instruccion.sprintf("DROP TABLE IF EXISTS precalculomensual%s", id_empresa);
			instrucciones[num_instrucciones++] = instruccion;
			instruccion.sprintf("CREATE TABLE IF NOT EXISTS precalculomensual%s LIKE precalculomensual%s", id_empresa, empresa_activa);
			instrucciones[num_instrucciones++] = instruccion;

			instruccion.sprintf("DROP TABLE IF EXISTS precalculominmax%s", id_empresa);
			instrucciones[num_instrucciones++] = instruccion;
			instruccion.sprintf("CREATE TABLE IF NOT EXISTS precalculominmax%s LIKE precalculominmax%s", id_empresa, empresa_activa);
			instrucciones[num_instrucciones++] = instruccion;

			instruccion.sprintf("DROP TABLE IF EXISTS precalculominmaxfin%s", id_empresa);
			instrucciones[num_instrucciones++] = instruccion;
			instruccion.sprintf("CREATE TABLE IF NOT EXISTS precalculominmaxfin%s LIKE precalculominmaxfin%s", id_empresa, empresa_activa);
			instrucciones[num_instrucciones++] = instruccion;

            instruccion.sprintf("DROP TABLE IF EXISTS precalculohistoriaconexist%s", id_empresa);
			instrucciones[num_instrucciones++] = instruccion;
			instruccion.sprintf("CREATE TABLE IF NOT EXISTS precalculohistoriaconexist%s LIKE precalculohistoriaconexist%s", id_empresa, empresa_activa);
			instrucciones[num_instrucciones++] = instruccion;

			instruccion.sprintf("DROP TABLE IF EXISTS precalculohistoriaexist%s", id_empresa);
			instrucciones[num_instrucciones++] = instruccion;
			instruccion.sprintf("CREATE TABLE IF NOT EXISTS precalculohistoriaexist%s LIKE precalculohistoriaexist%s", id_empresa, empresa_activa);
			instrucciones[num_instrucciones++] = instruccion;

			instruccion.sprintf("DROP TABLE IF EXISTS costospromedioiniciales%s", id_empresa);
			instrucciones[num_instrucciones++] = instruccion;
			instruccion.sprintf("CREATE TABLE IF NOT EXISTS costospromedioiniciales%s LIKE costospromedioiniciales%s", id_empresa, empresa_activa);
			instrucciones[num_instrucciones++] = instruccion;

			instruccion.sprintf("DROP TABLE IF EXISTS precalculocostospromedio%s", id_empresa);
			instrucciones[num_instrucciones++] = instruccion;
			instruccion.sprintf("CREATE TABLE IF NOT EXISTS precalculocostospromedio%s LIKE precalculocostospromedio%s", id_empresa, empresa_activa);
			instrucciones[num_instrucciones++] = instruccion;

			instruccion.sprintf("INSERT INTO presentacionescb \
				SELECT %s, producto, present, 0, 0, NULL \
				FROM presentaciones", id_empresa);
			instrucciones[num_instrucciones++] = instruccion;

		} else if (tarea_activa == "M") {
            instruccion.sprintf("UPDATE empresas SET nombre = '%s' WHERE idempresa = %s AND clave = '%s'",
								nombre_empresa, id_empresa, clave_empresa);
			instrucciones[num_instrucciones++] = instruccion;
		}

		instrucciones[num_instrucciones++] = "COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++) {
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);
        }

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);

	} __finally {
		if(buffer_sql != NULL) {
            delete buffer_sql;
		}
	}
}
// ---------------------------------------------------------------------------
// ID_BAJ_EMPRESA
void ServidorCatalogos::BorraEmpresa(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// BORRA EMPRESA
	char *buffer_sql = new char[300 * 500];
	char *aux_buffer_sql = buffer_sql;
	AnsiString id_empresa;
	int num_instrucciones = 0, i;
	AnsiString instruccion, instrucciones[500];

	try {
		id_empresa = mFg.ExtraeStringDeBuffer(&parametros);

		instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++] = "START TRANSACTION";

		instruccion.sprintf("DELETE FROM parametrosglobemp WHERE idempresa = %s", id_empresa);
		instrucciones[num_instrucciones++] = instruccion;

        instruccion.sprintf("DELETE FROM paramseguridad WHERE idempresa = %s", id_empresa);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("DROP TABLE IF EXISTS precalculocostos%s", id_empresa);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("DROP TABLE IF EXISTS precalculomensual%s", id_empresa);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("DROP TABLE IF EXISTS precalculominmax%s", id_empresa);
        instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("DROP TABLE IF EXISTS precalculominmaxfin%s", id_empresa);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("DROP TABLE IF EXISTS precalculohistoriaconexist%s", id_empresa);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("DROP TABLE IF EXISTS precalculohistoriaexist%s", id_empresa);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("DROP TABLE IF EXISTS costospromedioiniciales%s", id_empresa);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("DROP TABLE IF EXISTS precalculocostospromedio%s", id_empresa);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("DELETE FROM presentacionescb WHERE idempresa = %s", id_empresa);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("DELETE FROM empresas WHERE idempresa = %s", id_empresa);
		instrucciones[num_instrucciones++] = instruccion;

		instrucciones[num_instrucciones++] = "COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++) {
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);
        }

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);

	} __finally {
		if(buffer_sql != NULL) {
            delete buffer_sql;
		}
	}
}
// ---------------------------------------------------------------------------
// ID_CON_POSICIONPROD
void ServidorCatalogos::ConsultaPosicionProducto(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros)
{
    char *buffer_sql=new char[1024*10];
	char *aux_buffer_sql=buffer_sql;
	BufferRespuestas* resp=NULL;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instrucciones[10];
	AnsiString articulo;

	articulo = mFg.ExtraeStringDeBuffer(&parametros);

	try {
        mServidorVioleta->InicializaBuffer(Respuesta,
		TAM_MAX_BUFFER_RESPUESTA_REP_GRANDE);
		Instruccion.sprintf("SELECT\
			CONCAT_WS('-', acl.almacen, acl.nombrecalle, afr.numfrente, apos.nivel, apos.fondo) AS posart,\
			art.producto,art.present\
			FROM almacenposarticulos apart\
			INNER JOIN almacenposiciones apos ON apart.idalmposicion = apos.idalmposicion\
			INNER JOIN almacenfrentes afr ON apos.idalmfrente = afr.idalmfrente\
			INNER JOIN almacencalles acl ON afr.idalmcalle = acl.idalmcalle\
			INNER JOIN articulos art ON apart.producto = art.producto AND apart.present=art.present\
			WHERE art.producto = '%s'\
			GROUP BY art.producto, art.present;", articulo);

		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, Instruccion.c_str(),
			Respuesta->TamBufferResultado);


	} __finally {
		if(buffer_sql != NULL)
			delete buffer_sql;
	}
}
// ---------------------------------------------------------------------------
//ID_CON_NUEVO_PRODUCTO
void ServidorCatalogos::EjecutaConsultaNuevoArticulo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    AnsiString producto;
	/*AnsiString factor;
	AnsiString present;*/
    AnsiString instruccion;

	producto = mFg.ExtraeStringDeBuffer(&parametros);
	/*factor = mFg.ExtraeStringDeBuffer(&parametros);
	present = mFg.ExtraeStringDeBuffer(&parametros);*/
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos de las cancelaciones
	instruccion.sprintf("SELECT * FROM productos a WHERE producto = '%s'",
		producto);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------
//ID_MOD_MULTIPLO
void ServidorCatalogos::ModificarMultiplo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros){
	AnsiString Instruccion, instrucciones[500];
	char *buffer_sql = new char[1024 * 10];
	char *aux_buffer_sql = buffer_sql;
	int num_instrucciones = 0;

	AnsiString idProgramado, articulo, prod_origen, present_origen, mult_origen, mult_destino, usuario, fecha_prog,
		suc_prog;

	try{
		idProgramado = mFg.ExtraeStringDeBuffer(&parametros);
		articulo = mFg.ExtraeStringDeBuffer(&parametros);
		prod_origen = mFg.ExtraeStringDeBuffer(&parametros);
		present_origen = mFg.ExtraeStringDeBuffer(&parametros);
		mult_origen = mFg.ExtraeStringDeBuffer(&parametros);
		mult_destino = mFg.ExtraeStringDeBuffer(&parametros);
		usuario = mFg.ExtraeStringDeBuffer(&parametros);
		fecha_prog = mFg.ExtraeStringDeBuffer(&parametros);
		suc_prog = mFg.ExtraeStringDeBuffer(&parametros);

		instrucciones[num_instrucciones++]="SET SESSION sql_log_bin = 0";
		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		Instruccion.sprintf("UPDATE articulos set multiplo='%s' where articulo = '%s'",
		mult_destino, articulo);
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("UPDATE articulosped SET multiplopedir = '%s' WHERE producto = '%s' \
		AND present = '%s' AND multiplopedir = '%s'",
		mult_destino, prod_origen, present_origen, mult_origen);
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("UPDATE presentacionesminmax SET maxmult = '%s' \
			WHERE producto = '%s' AND present = '%s' AND maxmult = '%s'",
			mult_destino, prod_origen, present_origen, mult_origen);
        instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("INSERT INTO bitacoramodart \
		(articulo, multiploant, multiplonuevo, tipo, usumodi, fechamodi, horamodi) \
		VALUES ('%s', '%s', '%s', 3, '%s', CURDATE(), CURTIME()) ",
		articulo, mult_origen, mult_destino, usuario);
		instrucciones[num_instrucciones++]=Instruccion;

		Instruccion.sprintf("DELETE FROM camprogprodpresent where id = %s", idProgramado);
		instrucciones[num_instrucciones++]=Instruccion;

		instrucciones[num_instrucciones++]="COMMIT";
		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=1";
		instrucciones[num_instrucciones++]="SET SESSION sql_log_bin = 1";

		Instruccion.sprintf("INSERT INTO bitprodpresent (tipo, articulo, prod_origen, present_origen, mult_origen, \
						mult_destino, usuario, fecha_prog, suc_prog, mensaje, sucursal, correcto) VALUES \
						('MMUL', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', 'Operación de cambiar múltiplo correcta.', '%s', 1)",
						articulo, prod_origen, present_origen, mult_origen, mult_destino, usuario, fecha_prog, suc_prog, FormServidor->ObtieneClaveSucursal()
						);
		instrucciones[num_instrucciones++]=Instruccion;

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql = mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (int i = 0; i < num_instrucciones; i++)
			aux_buffer_sql = mFg.AgregaStringABuffer(instrucciones[i],aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta,TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,buffer_sql);


	}__finally{
        delete buffer_sql;
	}

}
// ---------------------------------------------------------------------------
//ID_GRA_ARTEMPCFG
void ServidorCatalogos::GrabaArticuloEmpresaCfg(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros){
    AnsiString instruccion, instrucciones[500];

	char * buffer_funciones = new char [1024];
	char * puntero_funciones = buffer_funciones;
	char * resultado_funcion = NULL;

	int num_instrucciones = 0;

	AnsiString nuevoValor, articulo, empresa, columnaBdd;

	int correcto = 1;

	BufferRespuestas * respuesta_articulo = NULL;

	try{
		nuevoValor = mFg.ExtraeStringDeBuffer(&parametros);
		articulo = mFg.ExtraeStringDeBuffer(&parametros);
		empresa = mFg.ExtraeStringDeBuffer(&parametros);
		columnaBdd = mFg.ExtraeStringDeBuffer(&parametros);

		instruccion.sprintf("START TRANSACTION");
		mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL, instruccion.c_str());

		instruccion.sprintf("INSERT IGNORE INTO articuloempresacfg ( %s, articulo, idempresa)\
			VALUES (%s, '%s', %s) ON DUPLICATE KEY UPDATE %s = %s", columnaBdd, nuevoValor,
			articulo, empresa, columnaBdd, nuevoValor);
		mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL, instruccion.c_str());

		instruccion.sprintf("SELECT producto, present FROM articulos WHERE articulo = '%s' ",
			articulo);
        mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), respuesta_articulo);

		mServidorVioleta->MuestraMensaje("-- * * * * * * INICIO Parte de ID_GRA_PRESENT_ACTIVA");
		puntero_funciones = mFg.AgregaStringABuffer(respuesta_articulo->ObtieneDato("producto"), puntero_funciones);
		puntero_funciones = mFg.AgregaStringABuffer(respuesta_articulo->ObtieneDato("present"), puntero_funciones);
		GrabaPresentacionActiva(Respuesta, MySQL, buffer_funciones);
		mServidorVioleta->MuestraMensaje("-- * * * * * * FIN Parte de ID_GRA_PRESENT_ACTIVA");

		resultado_funcion = Respuesta->BufferResultado;
		resultado_funcion += sizeof(int); //Saltar tamaño de respuesta

		if(mFg.ExtraeStringDeBuffer(&resultado_funcion) == "0")//Obtener indicador de error
			instruccion.sprintf("COMMIT");
		else
			instruccion.sprintf("ROLLBACK");
		mServidorVioleta->EjecutaAccionSql(Respuesta, MySQL, instruccion.c_str());

	}__finally{
		delete buffer_funciones;
		if (respuesta_articulo != NULL) delete respuesta_articulo;
	}
}
// ---------------------------------------------------------------------------
//ID_GRA_PRESENT_ACTIVA
void ServidorCatalogos::GrabaPresentacionActiva(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros){
    AnsiString instruccion, instrucciones[500];
	char *buffer_sql = new char[1024 * 10];
	char *aux_buffer_sql = buffer_sql;
	int num_instrucciones = 0;

	AnsiString producto, present;

	try{
		producto = mFg.ExtraeStringDeBuffer(&parametros);
		present = mFg.ExtraeStringDeBuffer(&parametros);

		instruccion.sprintf("CREATE TEMPORARY TABLE if not exists auxpresentacionesactivemp \
			LIKE presentacionesactivemp");
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("TRUNCATE TABLE auxpresentacionesactivemp");
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("INSERT INTO auxpresentacionesactivemp \
			SELECT e.idempresa, a.producto, a.present, SUM(IFNULL(permitecompras,1)+IFNULL(permiteventas,1)+IFNULL(permitemovalma,1))>0 AS activonuevo \
			FROM articulos a \
			INNER JOIN empresas e \
			LEFT JOIN articuloempresacfg ac ON a.articulo=ac.articulo AND e.idempresa=ac.idempresa \
			WHERE a.producto = '%s' AND a.present = '%s' \
			GROUP BY a.producto, a.present, e.idempresa", producto, present);
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("INSERT IGNORE INTO presentacionesactivemp \
			SELECT * FROM auxpresentacionesactivemp aux \
			ON DUPLICATE KEY UPDATE \
			activo = aux.activo");
		instrucciones[num_instrucciones++] = instruccion;

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql = mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (int i = 0; i < num_instrucciones; i++)
			aux_buffer_sql = mFg.AgregaStringABuffer(instrucciones[i],aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta,TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,buffer_sql);

	}__finally{
		delete buffer_sql;
	}
}
// ---------------------------------------------------------------------------
// ID_CON_OBJ_VENDEDORES
void ServidorCatalogos::ConsultaObjetivosVendedores(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros)
{
    char *buffer_sql=new char[1024*10];
	char *aux_buffer_sql=buffer_sql;
	BufferRespuestas* resp=NULL;
	AnsiString instruccion;
	AnsiString mes = " ", condicion_mes = " ";

	try {
		mes = mFg.ExtraeStringDeBuffer(&parametros);

		if (mes != " ") {
			condicion_mes.sprintf(" WHERE mes = %s", mes);
		}

        mServidorVioleta->InicializaBuffer(Respuesta,
		TAM_MAX_BUFFER_RESPUESTA_REP_GRANDE);
		instruccion.sprintf("SELECT mes, objetivovts, objetivoart, nuevo\
			FROM objetivosvendedores %s ", condicion_mes);

		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	} __finally {
		if(buffer_sql != NULL) {
			delete buffer_sql;
		}
	}
}
// ---------------------------------------------------------------------------
// ID_MOD_OBJ_VENDEDORES
void ServidorCatalogos::ModificaObjetivosVendedores(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros)
{
    char *buffer_sql=new char[1024*30];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i;
	AnsiString instrucciones[10];
    AnsiString instruccion;
	AnsiString mes,porcentajeVts,porcentajeArt;

	mes = mFg.ExtraeStringDeBuffer(&parametros);
	porcentajeVts = mFg.ExtraeStringDeBuffer(&parametros);
	porcentajeArt = mFg.ExtraeStringDeBuffer(&parametros);

	try{
		instruccion.sprintf("UPDATE objetivosvendedores SET objetivovts = %s,\
			objetivoart = %s WHERE mes = '%s'", porcentajeVts, porcentajeArt , mes);
		instrucciones[num_instrucciones++]=instruccion;

		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);

		for (i=0; i<num_instrucciones; i++){
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);
		}

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);

	} __finally {
		if(buffer_sql != NULL) delete buffer_sql;
	}
}
// ---------------------------------------------------------------------------
// ID_CON_OBJ_SUCURSAL
void ServidorCatalogos::ConsultaObjetivosSucursales(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros)
{
    char *buffer_sql=new char[1024*10];
	char *aux_buffer_sql=buffer_sql;
	BufferRespuestas* resp=NULL;
	AnsiString instruccion;

	try {
		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_REP_GRANDE);

		instruccion.sprintf("SELECT os.sucursal, suc.nombre, os.nuevo\
			FROM objetivossucursales os\
			INNER JOIN sucursales suc ON suc.sucursal=os.sucursal\
			ORDER BY suc.nombre");

		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
			Respuesta->TamBufferResultado);


	} __finally {
		if(buffer_sql != NULL) {
			delete buffer_sql;
		}
	}
}
// ---------------------------------------------------------------------------
// ID_MOD_OBJ_SUCURSAL
void ServidorCatalogos::ModificaObjetivosSucursal(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros)
{
    char *buffer_sql=new char[1024*30];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i;
	AnsiString instrucciones[10];
	AnsiString clave,porcentajeVts,porcentajeArt, instruccion, objetivoini;
    AnsiString condicionVtas = " ";

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	porcentajeVts = mFg.ExtraeStringDeBuffer(&parametros);
	condicionVtas = mFg.ExtraeStringDeBuffer(&parametros);

	try{
		instruccion.sprintf("UPDATE objetivossucursales SET %s = %s WHERE sucursal = '%s'",
		condicionVtas,porcentajeVts,clave);
		instrucciones[num_instrucciones++]=instruccion;

		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);

		for (i=0; i<num_instrucciones; i++){
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);
		}

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);

	} __finally {
		if(buffer_sql != NULL) {
			delete buffer_sql;
		}
	}
}
// ---------------------------------------------------------------------------
// ID_REDONDEA_PRECIOS_CAT_ART
void ServidorCatalogos::RedondeaPrecioCatArt(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	BufferRespuestas* resp_param=NULL;
	BufferRespuestas* resp_productos=NULL;
    char *buffer_sql=new char[1024*1024];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i;
	AnsiString instrucciones[500], Instruccion, instruccion;
	AnsiString producto, present, idEmpresa, usuario;
	AnsiString campo_precios, campo_precios_mod;
	AnsiString precio = "p.precioproximo";
	AnsiString condicion_producto;

	AnsiString desde1="0.01", hasta1="0.24", redondeo1="0.00";
	AnsiString desde2="0.25", hasta2="0.49", redondeo2="0.50";
	AnsiString desde3="0.50", hasta3="0.74", redondeo3="0.50";
	AnsiString desde4="0.75", hasta4="0.99", redondeo4="0.90";

	int esRedondeo = 0;
	int redondear = 0;

	producto  = mFg.ExtraeStringDeBuffer(&parametros);
	present   = mFg.ExtraeStringDeBuffer(&parametros);
	idEmpresa = mFg.ExtraeStringDeBuffer(&parametros);
	usuario   = mFg.ExtraeStringDeBuffer(&parametros);
	redondear = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));

	try {
		Instruccion.sprintf("SELECT valor FROM parametrosglobemp WHERE parametro = 'REDONPREDIF' AND idempresa = %s ",
		idEmpresa);

		if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), resp_param)) {
			if (resp_param->ObtieneNumRegistros()>0){
				esRedondeo = StrToInt(resp_param->ObtieneDato("valor"));
			} else {
				throw (Exception("Error al consultar en tabla parametrosglobemp"));
			}
		}
	} __finally {
		if (resp_param!=NULL) delete resp_param;
	}

	if(redondear != esRedondeo){
		esRedondeo = redondear;
	}

	if(esRedondeo = 1) {
		campo_precios.sprintf("(CASE WHEN TRUNCATE(MOD(%s,1),2) = 0.0 THEN %s ELSE \
		CASE WHEN TRUNCATE(MOD(%s,1),2) >=%s AND TRUNCATE(MOD(%s,1),2) <=%s THEN ((%s-TRUNCATE(MOD(%s,1),2))+ TRUNCATE(%s, 2)) ELSE \
		CASE WHEN TRUNCATE(MOD(%s,1),2) >=%s AND TRUNCATE(MOD(%s,1),2) <=%s THEN ((%s-TRUNCATE(MOD(%s,1),2))+ TRUNCATE(%s, 2)) ELSE \
		CASE WHEN TRUNCATE(MOD(%s,1),2) >=%s AND TRUNCATE(MOD(%s,1),2) <=%s THEN ((%s-TRUNCATE(MOD(%s,1),2))+ TRUNCATE(%s, 2)) ELSE \
		CASE WHEN TRUNCATE(MOD(%s,1),2) >=%s AND TRUNCATE(MOD(%s,1),2) <=%s THEN ((%s-TRUNCATE(MOD(%s,1),2))+ TRUNCATE(%s, 2)) END END END END END)",
		precio, precio,
		precio, desde1, precio, hasta1, redondeo1, precio, precio,
		precio, desde2, precio, hasta2, redondeo2, precio, precio,
		precio, desde3, precio, hasta3, redondeo3, precio, precio,
		precio, desde4, precio, hasta4, redondeo4, precio, precio);

		campo_precios_mod = "(CASE WHEN TRUNCATE(MOD((pmod.preciomod * a.factor),1),2) = 0.0 THEN (pmod.preciomod * a.factor) ELSE \
		CASE WHEN TRUNCATE(MOD((pmod.preciomod * a.factor),1),2) >= 0.01 AND TRUNCATE(MOD((pmod.preciomod * a.factor),1),2) <= 0.24 THEN ((0.00-TRUNCATE(MOD((pmod.preciomod * a.factor),1),2))+(pmod.preciomod* a.factor)) ELSE \
		CASE WHEN TRUNCATE(MOD((pmod.preciomod * a.factor),1),2) >= 0.25 AND TRUNCATE(MOD((pmod.preciomod * a.factor),1),2) <= 0.49 THEN ((0.50-TRUNCATE(MOD((pmod.preciomod * a.factor),1),2))+(pmod.preciomod * a.factor)) ELSE \
		CASE WHEN TRUNCATE(MOD((pmod.preciomod * a.factor),1),2) >= 0.50 AND TRUNCATE(MOD((pmod.preciomod * a.factor),1),2) <= 0.74 THEN ((0.50-TRUNCATE(MOD((pmod.preciomod * a.factor),1),2))+(pmod.preciomod * a.factor)) ELSE \
		CASE WHEN TRUNCATE(MOD((pmod.preciomod * a.factor),1),2) >= 0.75 AND TRUNCATE(MOD((pmod.preciomod * a.factor),1),2) <= 0.99 THEN ((0.90-TRUNCATE(MOD((pmod.preciomod * a.factor),1),2))+(pmod.preciomod * a.factor)) END END END END END) ";
	} else {
		campo_precios = precio;

		campo_precios_mod = precio;
	}

	condicion_producto.sprintf(" AND pro.producto='%s' AND a.present='%s' ", producto, present);

	try{

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		instruccion.sprintf("CREATE TEMPORARY TABLE tmppreciosmod ( \
			articulo VARCHAR(9), \
			producto VARCHAR(8), \
			present VARCHAR(255), \
			multiplo VARCHAR(10), \
			factor DECIMAL(10, 3), \
			tipoprec VARCHAR(2), \
			precio DECIMAL(16,6), \
			precioproximo DECIMAL(16,6), \
			preciomod DECIMAL(16,6), \
			INDEX(articulo), INDEX(tipoprec) \
		)ENGINE=INNODB");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("INSERT INTO tmppreciosmod \
		SELECT \
			a.articulo, \
			a.producto, \
			a.present, \
			a.multiplo, \
			a.factor, \
			p.tipoprec, \
			p.precio, \
			p.precioproximo, \
			%s AS preciomod \
		FROM precios p \
		INNER JOIN articulos a ON p.articulo=a.articulo \
		INNER JOIN tiposdeprecios tp ON tp.tipoprec=p.tipoprec \
		WHERE tp.idempresa = %s AND a.producto = '%s' AND a.present = '%s' AND a.factor = 1",
		campo_precios, idEmpresa, producto, present);
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("UPDATE precios p \
		INNER JOIN tmppreciosmod a ON a.articulo = p.articulo AND p.tipoprec = a.tipoprec \
		INNER JOIN tiposdeprecios tp ON tp.tipoprec = p.tipoprec \
		SET p.preciomod = a.preciomod \
		WHERE tp.idempresa = %s AND a.producto = '%s' AND a.present = '%s' AND a.factor = 1",
		idEmpresa, producto, present);
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("UPDATE precios p \
		INNER JOIN articulos a ON a.articulo = p.articulo \
		INNER JOIN tiposdeprecios tp ON tp.tipoprec = p.tipoprec \
		LEFT JOIN ( \
			SELECT producto, present, tipoprec, preciomod \
			FROM tmppreciosmod  \
			WHERE factor = 1 \
		) pmod ON a.producto = pmod.producto AND a.present = pmod.present AND p.tipoprec = pmod.tipoprec \
		SET p.preciomod = %s \
		WHERE tp.idempresa = %s \
		AND a.producto = '%s' \
		AND a.present = '%s' \
		AND a.factor <> 1 ",
		campo_precios_mod,
		idEmpresa, producto, present);
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("INSERT INTO bitacorapreciosdiferidos SELECT NULL,CURDATE(),CURTIME(),'%s'",
		usuario);
		instrucciones[num_instrucciones++]=instruccion;

		instrucciones[num_instrucciones++]="set @referencia:=LAST_INSERT_ID()";

		instruccion.sprintf("INSERT INTO bitacorapreciosdiferidosdetalle \
		SELECT NULL,@referencia, p.articulo, p.precio, %s, p.tipoprec \
		FROM precios p \
		INNER JOIN tiposdeprecios tp ON tp.tipoprec=p.tipoprec AND tp.idempresa=%s \
		INNER JOIN articulos a ON a.articulo=p.articulo \
		INNER JOIN productos pro ON a.producto=pro.producto \
		WHERE (%s<>p.precio or p.precio is null) %s",
		campo_precios, idEmpresa, campo_precios, condicion_producto);
		instrucciones[num_instrucciones++]=instruccion;

		instrucciones[num_instrucciones++]="COMMIT";


		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);

		for (i=0; i<num_instrucciones; i++){
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);
		}

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);

	} __finally {
		if(buffer_sql != NULL) delete buffer_sql;
	}
}
// ---------------------------------------------------------------------------
// ID_CONF_PRECIOS_CAT_ART
void ServidorCatalogos::ConfPreciosCatArticulos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	BufferRespuestas* resp_param=NULL;
	BufferRespuestas* resp_productos=NULL;
    char *buffer_sql=new char[1024*1024];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i;
	AnsiString instrucciones[500], Instruccion, instruccion;
	AnsiString producto, present, idEmpresa, usuario;
	AnsiString campo_precios;
	AnsiString precio = "p.precioproximo";
	AnsiString condicion_producto;

	AnsiString desde1="0.01", hasta1="0.24", redondeo1="0.00";
	AnsiString desde2="0.25", hasta2="0.49", redondeo2="0.50";
	AnsiString desde3="0.50", hasta3="0.74", redondeo3="0.50";
	AnsiString desde4="0.75", hasta4="0.99", redondeo4="0.90";

	int esRedondeo = 0;
    int redondear = 0;

	producto  = mFg.ExtraeStringDeBuffer(&parametros);
	present   = mFg.ExtraeStringDeBuffer(&parametros);
	idEmpresa = mFg.ExtraeStringDeBuffer(&parametros);
	usuario   = mFg.ExtraeStringDeBuffer(&parametros);
	redondear = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));

	try {
		Instruccion.sprintf("SELECT valor FROM parametrosglobemp WHERE parametro = 'REDONPREDIF' AND idempresa = %s ",
		idEmpresa);

		if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, Instruccion.c_str(), resp_param)) {
			if (resp_param->ObtieneNumRegistros()>0){
				esRedondeo = StrToInt(resp_param->ObtieneDato("valor"));
			} else {
				throw (Exception("Error al consultar en tabla parametrosglobemp"));
			}
		}
	} __finally {
		if (resp_param!=NULL) delete resp_param;
	}

	if(redondear != esRedondeo){
		esRedondeo = redondear;
	}

	if(esRedondeo = 1) {
		campo_precios = "p.preciomod";
	} else {
		campo_precios = precio;
	}

	condicion_producto.sprintf(" AND pro.producto='%s' AND a.present='%s' ", producto, present);

	try{

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		instruccion.sprintf("UPDATE precios AS p \
		INNER JOIN tiposdeprecios tp ON tp.tipoprec=p.tipoprec AND tp.idempresa=%s \
		INNER JOIN articulos a ON a.articulo=p.articulo \
		INNER JOIN productos pro ON a.producto=pro.producto \
		SET p.actualizarpendiente=1 \
		WHERE (%s<>p.precio or p.precio is null) %s ", idEmpresa,
		campo_precios, condicion_producto);
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("INSERT INTO bitacoraconfiguracionprecios \
		(articulo, usuario, fecha, hora, idempresa, tipo, modulo) \
		SELECT articulo, '%s', CURDATE(), CURTIME(), %s, 0, 'CATART' FROM articulos WHERE producto = '%s' AND present = '%s'",
		usuario, idEmpresa, producto, present);
		instrucciones[num_instrucciones++]=instruccion;

		instrucciones[num_instrucciones++]="COMMIT";


		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);

		for (i=0; i<num_instrucciones; i++){
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);
		}

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);

	} __finally {
		if(buffer_sql != NULL) delete buffer_sql;
	}
}
// ---------------------------------------------------------------------------
// ID_ART_X_ACTIVIDAD
void ServidorCatalogos::EjecutaArticulosxActividad(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;

	int permitecompra, permitevender, permitemovalm, existencias;
	AnsiString empresa;
	AnsiString wheres = " ";
	AnsiString condicion_permite_compra  = " ";
	AnsiString condicion_permite_venta   = " ";
	AnsiString condicion_permite_movalma = " ";
	AnsiString condicion_existencias = " ";
	AnsiString multiplo, condicionMultiplo = " ";

	permitecompra = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
	permitevender = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
	permitemovalm = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
	existencias   = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
	multiplo = mFg.ExtraeStringDeBuffer(&parametros);
	empresa 	  = mFg.ExtraeStringDeBuffer(&parametros);
	AnsiString clasif1 = mFg.ExtraeStringDeBuffer(&parametros);
	AnsiString clasif2 = mFg.ExtraeStringDeBuffer(&parametros);
	AnsiString clasif3 = mFg.ExtraeStringDeBuffer(&parametros);
	AnsiString supervisor = mFg.ExtraeStringDeBuffer(&parametros);

	if(permitecompra == 0){
		condicion_permite_compra = " AND acfg.permitecompras = 1 OR ISNULL(acfg.permitecompras) ";
	} else if (permitecompra == 1) {
		condicion_permite_compra = " AND acfg.permitecompras = 0 ";
	}

	if(permitevender == 0){
		condicion_permite_venta = " AND acfg.permiteventas = 1 OR ISNULL(acfg.permiteventas) ";
	} else if (permitevender == 1) {
		condicion_permite_venta = " AND acfg.permiteventas = 0 ";
	}

	if(permitemovalm == 0){
		condicion_permite_movalma = " AND acfg.permitemovalma = 1 OR ISNULL(acfg.permitemovalma) ";
	} else if (permitemovalm == 1) {
		condicion_permite_movalma = " AND acfg.permitemovalma = 0 ";
	}

	if(existencias == 1){
		condicion_existencias = "AND sum(ea.cantidad) > 0 ";
	} else if (existencias == 0) {
		condicion_existencias = "AND sum(ea.cantidad) <= 0 ";
	}

	if(multiplo == "0"){
		condicionMultiplo = " AND pmm.minmult = a.multiplo ";
	} else if (multiplo == "1"){
		condicionMultiplo = " AND pmm.maxmult = a.multiplo ";
	}

	if(clasif1 != " "){
		AnsiString cla;
		cla.sprintf(" AND prod.clasif1 = '%s' ", clasif1);
		wheres += cla;
	}

	if(clasif2 != " "){
		AnsiString cla;
		cla.sprintf(" AND prod.clasif2 = '%s' ", clasif2);
		wheres += cla;
	}

	if(clasif3 != " "){
		AnsiString cla;
		cla.sprintf(" AND prod.clasif3 = '%s' ", clasif3);
		wheres += cla;
	}

	if(supervisor != " "){
		AnsiString cla;
		cla.sprintf(" AND asu.usuario = '%s' ", supervisor);
		wheres += cla;
	}
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos del producto
	instruccion.sprintf("SELECT \
		a.articulo, \
		prod.nombre, \
		a.present, \
		a.multiplo, \
		IFNULL(acfg.permitecompras, 1) AS permitecompra, \
		IFNULL(acfg.permiteventas, 1) AS permiteventa, \
		IFNULL(acfg.permitemovalma, 1) AS permitemovalma \
		FROM articulos a \
		INNER JOIN productos prod ON prod.producto = a.producto \
		INNER JOIN existenciasactuales ea ON ea.producto = a.producto AND ea.present = a.present \
		INNER JOIN almacenes alm ON alm.almacen = ea.almacen \
		INNER JOIN secciones sec ON sec.seccion = alm.seccion \
		INNER JOIN sucursales suc ON suc.sucursal = sec.sucursal \
		INNER JOIN presentacionesminmax pmm ON pmm.producto = a.producto AND pmm.present = a.present \
		INNER JOIN articulossupervisados asu ON asu.producto = a.producto AND asu.present = a.present \
		LEFT JOIN articuloempresacfg acfg ON acfg.articulo = a.articulo  \
		WHERE suc.idempresa = %s AND acfg.idempresa = %s \
		%s %s %s %s %s \
		GROUP BY a.articulo \
		HAVING 1 %s \
		ORDER BY prod.nombre ASC ",
	empresa, empresa,
	condicion_permite_compra, condicion_permite_venta, condicion_permite_movalma, condicionMultiplo, wheres,
	condicion_existencias);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------
//ID_CONF_ART_X_ACT
void ServidorCatalogos::EjecutaConfiguraArtxActividad(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*1024];
	char *aux_buffer_sql=buffer_sql;
	int num_partidas, i;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[1000];
	AnsiString Instruccion;

	AnsiString articulos, empresa, actividades;
	AnsiString condicion_compra=" ";
	AnsiString condicion_venta=" ";
	AnsiString condicion_movalma=" ";

	articulos 	= mFg.ExtraeStringDeBuffer(&parametros);
	empresa 	= mFg.ExtraeStringDeBuffer(&parametros);
	actividades = mFg.ExtraeStringDeBuffer(&parametros);

	TStringDynArray splitarticulos(SplitString(articulos, ","));
	TStringDynArray tipo_actividades(SplitString(actividades, ","));

	try {

		for (int i = 0; i < tipo_actividades.Length; i++) {

			TStringDynArray estatus_actividad(SplitString(tipo_actividades[i], "-"));

			AnsiString actividad = "";
			int estatus = 0;

			actividad = estatus_actividad[0];
			estatus   = StrToInt(estatus_actividad[1]);

			if(actividad=="COM"){
				if(estatus == 1){
					condicion_compra = 1;
				} else {
					condicion_compra = 0;
				}
			}

			if(actividad=="VEN"){
				if(estatus == 1){
					condicion_venta = 1;
				} else {
					condicion_venta = 0;
				}
			}

			if(actividad=="MAL"){
				if(estatus == 1){
					condicion_movalma = 1;
				} else {
					condicion_movalma = 0;
				}
			}
		}

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		for (int i = 0; i < splitarticulos.Length; i++) {
			instruccion.sprintf("REPLACE INTO articuloempresacfg \
			(articulo, idempresa, permitecompras, permiteventas, permitemovalma) \
			VALUES (%s, %s, %s, %s, %s)", AnsiString(splitarticulos[i]), empresa, condicion_compra, condicion_venta, condicion_movalma);
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
		delete buffer_sql;
	}
}
// ---------------------------------------------------------------------------
//ID_CON_ART_X_ACT
void ServidorCatalogos::EjecutaConsultaArtxActividad(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString Instruccion;

	AnsiString empresa, actividades;
	AnsiString condicion_compra=" ";
	AnsiString condicion_venta=" ";
	AnsiString condicion_movalma=" ";

	empresa 	= mFg.ExtraeStringDeBuffer(&parametros);
	actividades = mFg.ExtraeStringDeBuffer(&parametros);

	TStringDynArray tipo_actividades(SplitString(actividades, ","));

	for (int i = 0; i < tipo_actividades.Length; i++) {

		TStringDynArray estatus_actividad(SplitString(tipo_actividades[i], "-"));

		AnsiString actividad = "";
		AnsiString estatus = 0;

		actividad = estatus_actividad[0];
		estatus   = estatus_actividad[1];

		if(actividad=="COM"){
			condicion_compra.sprintf(" AND permitecompras = %s ", estatus);
		}

		if(actividad=="VEN"){
			condicion_venta.sprintf(" AND permiteventas = %s ", estatus);
		}

		if(actividad=="MAL"){
			condicion_movalma.sprintf(" AND permitemovalma = %s ", estatus);
		}
	}

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos del producto
	Instruccion.sprintf("SELECT \
	a.articulo, \
	prod.nombre, \
	a.present, \
	a.multiplo \
	FROM articulos a \
	INNER JOIN productos prod ON prod.producto = a.producto \
	LEFT JOIN articuloempresacfg acfg ON acfg.articulo = a.articulo  \
	WHERE acfg.idempresa = %s \
	%s %s %s \
	GROUP BY a.articulo \
	ORDER BY prod.nombre ASC",
	empresa,
	condicion_compra, condicion_venta, condicion_movalma);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, Instruccion.c_str(), Respuesta->TamBufferResultado);

}
// ---------------------------------------------------------------------------
//ID_CON_MOTIVOS_DEV_NC
void ServidorCatalogos::ConsultaMotivoDevolucionNC(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros)
{

	AnsiString instruccion;
	AnsiString clave_motivo;

	clave_motivo = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos del motivo de devolución
	instruccion.sprintf("select * from catalogo_motivos_devolucion where clave_motivo='%s'",
		clave_motivo);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

	// Obtiene todos los motivos de devolución
	instruccion =
		"select clave_motivo, descripcion, activo, perm_resp_vacio, perm_razon_vacio from catalogo_motivos_devolucion";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

}
// ---------------------------------------------------------------------------
//ID_GRA_MOTIVOS_DEV_NC
void ServidorCatalogos::GrabaMotivoDevolucionNC(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros)
{
	// GRABA motivo de devolución
	GrabaGenerico(Respuesta, MySQL, parametros, "catalogo_motivos_devolucion", "clave_motivo");
}
// ---------------------------------------------------------------------------
//ID_BAJ_MOTIVOS_DEV_NC
void ServidorCatalogos::BorraMotivoDevolucionNC(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros)
{
	// BAJA motivo de devolución
	BajaGenerico(Respuesta, MySQL, parametros, "catalogo_motivos_devolucion", "clave_motivo");
}
// ID_CON_PARAMETROGEN
void ServidorCatalogos::ConsultaParametroGeneral(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA PARAMETRO GENERAL
	AnsiString instruccion;
	AnsiString clave, indiciparametro;
	AnsiString condicion_indiciparametro = " ";

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	indiciparametro = mFg.ExtraeStringDeBuffer(&parametros);

	if (indiciparametro!=" ") {
	   condicion_indiciparametro.sprintf(" AND (Parametro LIKE '%%%s%%' OR Descripcion LIKE '%%%s%%' OR Valor LIKE '%%%s%%') ", indiciparametro, indiciparametro, indiciparametro);
	}


	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos del parametro
	instruccion.sprintf("select * from parametrosgen where parametro='%s' ", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	if (indiciparametro!=" ") {
		// Obtiene todos los parametros
		instruccion.sprintf("select parametro AS Parametro, descripcion AS Descripcion, valor AS Valor from parametrosgen WHERE 1 %s", condicion_indiciparametro);
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	} else {
		instruccion.sprintf("select parametro AS Parametro, descripcion AS Descripcion, valor AS Valor from parametrosgen");
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	}
}
// ---------------------------------------------------------------------------
// ID_GRA_PARAMETROGEN
void ServidorCatalogos::GrabaParametroGeneral(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// GRABA PARAMETRO GENERAL
	char *buffer_sql = new char[1024 * 100];
	char *aux_buffer_sql = buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);

	int num_instrucciones = 0;
	int i = 0;
	AnsiString instruccion, instrucciones[100];
	AnsiString tarea, clave, descripcion, valor;

	tarea = mFg.ExtraeStringDeBuffer(&parametros);
	clave = mFg.ExtraeStringDeBuffer(&parametros);
	descripcion = mFg.ExtraeStringDeBuffer(&parametros);
	valor=mFg.ExtraeStringDeBuffer(&parametros);

	try {
		instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++] = "START TRANSACTION";

		if (tarea == "A"){

		   instruccion.sprintf("INSERT INTO parametrosgen VALUES                                                \
								('%s', '%s' , '%s')",clave, descripcion, valor );
		   instrucciones[num_instrucciones++] = instruccion;

		}else{
		   instruccion.sprintf("UPDATE parametrosgen SET descripcion='%s', valor='%s' WHERE parametro='%s' ", descripcion, valor, clave);
		   instrucciones[num_instrucciones++] = instruccion;
		}

		instrucciones[num_instrucciones++] = "COMMIT";

		aux_buffer_sql = mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);

		for (i = 0; i < num_instrucciones; i++){
		  aux_buffer_sql = mFg.AgregaStringABuffer(instrucciones[i],aux_buffer_sql);
		}

		mServidorVioleta->InicializaBuffer(Respuesta,TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,buffer_sql);

	}__finally {
		delete buffer_sql;
	}
}
// ---------------------------------------------------------------------------
// ID_GRA_SOLI_TICKET_SOPORTE
void ServidorCatalogos::EjecutaGrabaSolTicketSoporte(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	BufferRespuestas* resp=NULL;
	char *buffer_sql = new char[1024 * 1024 *64];
	char *aux_buffer_sql = buffer_sql;
	int num_instrucciones = 0;
	int i = 0;
	AnsiString instruccion, instrucciones[100];
	AnsiString referencia, tarea, usuario, terminal;
	AnsiString areaatencion, descripproblema, fechapromesa, retroalimentacion, departamento;
	AnsiString puestosolicita, numvacantes, sueldo, horario, diastrabajo, actividades;
	AnsiString moduloreporte, nuevacaracter;
	AnsiString diseno, lugar, textoincluido, incluyeimagen, termicondic;
	AnsiString nombreimagen, formato, imagen;
	AnsiString otrosConocimientos, tipoBaja, nombreEmpleado;
	double ancho, alto;
	int tipodiseno, cantidad;
	int tiposol, solucionado = 0;
	int tiporeemplazo, escolaridad, sexo, laboradomingo;
	int resuelto, validado, prioridad;

	tarea=mFg.ExtraeStringDeBuffer(&parametros);
	referencia=mFg.ExtraeStringDeBuffer(&parametros);
	usuario=mFg.ExtraeStringDeBuffer(&parametros);
	terminal=mFg.ExtraeStringDeBuffer(&parametros);
	areaatencion=mFg.ExtraeStringDeBuffer(&parametros);
	resuelto=StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
	validado=StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
	descripproblema=mFg.ExtraeStringDeBuffer(&parametros);
	prioridad=StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
	fechapromesa=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
	retroalimentacion=mFg.ExtraeStringDeBuffer(&parametros);
	departamento=mFg.ExtraeStringDeBuffer(&parametros);
	nombreimagen=mFg.ExtraeStringDeBuffer(&parametros);
	formato=mFg.ExtraeStringDeBuffer(&parametros);
	imagen=mFg.ExtraeStringDeBuffer(&parametros);

	if(departamento == "RHH"){
		puestosolicita=mFg.ExtraeStringDeBuffer(&parametros);
		numvacantes=mFg.ExtraeStringDeBuffer(&parametros);
		tiporeemplazo=StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
		escolaridad=StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
		sexo=StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
		sueldo=mFg.ExtraeStringDeBuffer(&parametros);
		horario=mFg.ExtraeStringDeBuffer(&parametros);
		diastrabajo=mFg.ExtraeStringDeBuffer(&parametros);
		laboradomingo=StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
		actividades=mFg.ExtraeStringDeBuffer(&parametros);
		otrosConocimientos=mFg.ExtraeStringDeBuffer(&parametros);
		tipoBaja=mFg.ExtraeStringDeBuffer(&parametros);
		nombreEmpleado=mFg.ExtraeStringDeBuffer(&parametros);
	}

	if(departamento == "DES"){
		moduloreporte=mFg.ExtraeStringDeBuffer(&parametros);
		tiposol=StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
		nuevacaracter=mFg.ExtraeStringDeBuffer(&parametros);
	}

	if(departamento == "MER"){
		tipodiseno=StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
		diseno=mFg.ExtraeStringDeBuffer(&parametros);
		cantidad=StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
		textoincluido=mFg.ExtraeStringDeBuffer(&parametros);
		incluyeimagen=mFg.ExtraeStringDeBuffer(&parametros);
		termicondic=mFg.ExtraeStringDeBuffer(&parametros);
		if(tipodiseno == 0){
			ancho=mFg.CadenaAFlotante(mFg.ExtraeStringDeBuffer(&parametros));
			alto=mFg.CadenaAFlotante(mFg.ExtraeStringDeBuffer(&parametros));
			lugar=mFg.ExtraeStringDeBuffer(&parametros);
		}
	}

	try {
		instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++] = "START TRANSACTION";

		if (tarea == "A"){

			instruccion.sprintf("select @folioaux:=valor from foliosemp where folio='SOLTICKSOP' AND sucursal = '%s' %s",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @foliosig=@folioaux+1");
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @folioaux=cast(@folioaux as char)");
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("set @folio=concat('%s', lpad(@folioaux,9,'0'))", FormServidor->ObtieneClaveSucursal());
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("update foliosemp set valor=@foliosig where folio='SOLTICKSOP' AND sucursal = '%s' ",FormServidor->ObtieneClaveSucursal());
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("INSERT INTO solicitudtickets \
			(referencia, idJefatura, ususolicita, terminal, fechasol, \
			horasol, fechapromesa, prioridad, departamento, \
			fechamodi, horamodi, usumodi ) \
			VALUES \
			(@folio, '%s', '%s', '%s', CURDATE(), \
			CURTIME(), '%s', %d, '%s', \
			CURDATE(), CURTIME(), '%s') ",
			areaatencion, usuario, terminal, fechapromesa, prioridad, departamento, usuario);
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("INSERT INTO solicitudticketsmensajes \
			(referencia, descripproblema) VALUES (@folio, '%s') ",
			descripproblema);
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("INSERT INTO solicitudticketsbitacora \
			(referencia, idJefatura, ususolicita, terminal, fechasol, \
			horasol, fechapromesa, prioridad, departamento, tipo, \
			fechamodi, horamodi, usumodi ) \
			VALUES \
			(@folio, '%s', '%s', '%s', CURDATE(), \
			CURTIME(), '%s', %d, '%s', '%s', \
			CURDATE(), CURTIME(), '%s') ",
			areaatencion, usuario, terminal, fechapromesa, prioridad, departamento, tarea, usuario);
			instrucciones[num_instrucciones++]=instruccion;

			if(departamento == "MER"){
				if(tipodiseno == 0){
					if(nombreimagen != " ") {
						instruccion.sprintf("INSERT INTO solicitudticketsimagenes \
						(referencia, nombre, formato, imagen) \
						VALUES \
						(@folio, '%s', '%s', '%s') ",
						nombreimagen, formato, imagen);
						instrucciones[num_instrucciones++]=instruccion;
					}
				}
			} else {
				if(nombreimagen != " ") {
					instruccion.sprintf("INSERT INTO solicitudticketsimagenes \
					(referencia, nombre, formato, imagen) \
					VALUES \
					(@folio, '%s', '%s', '%s') ",
					nombreimagen, formato, imagen);
					instrucciones[num_instrucciones++]=instruccion;
				}
            }

			if(departamento == "RHH"){

				instruccion.sprintf("INSERT INTO solicitudticketsrh ( \
				referencia, puestosoli, tipoBaja, nombreEmpleado, numvacantes, \
				tiporeemplazo, escolaridad, sexo, sueldo, horariocubrir, \
				diastrabajo, laboradomingo, actividadespuesto, otrosconocimientos \
				) VALUES ( \
				@folio, '%s', '%s', '%s', %s, \
				%d, %d, %d, '%s', '%s', \
				'%s', %d, '%s', '%s' \
				) ",
				puestosolicita, tipoBaja, nombreEmpleado, numvacantes,
				tiporeemplazo, escolaridad, sexo, sueldo, horario,
				diastrabajo, laboradomingo, actividades, otrosConocimientos);
				instrucciones[num_instrucciones++]=instruccion;
			}

			if(departamento == "DES"){

				if(tiposol == 2) {
					instruccion.sprintf("INSERT INTO solicitudticketsdesarrollo ( \
					referencia, moduloreporte, tiposolicitud, descricaract \
					) VALUES ( \
					@folio, '%s', %d, '%s' \
					) ",
					moduloreporte, tiposol, nuevacaracter);
					instrucciones[num_instrucciones++]=instruccion;
				} else {
					instruccion.sprintf("INSERT INTO solicitudticketsdesarrollo ( \
					referencia, moduloreporte, tiposolicitud \
					) VALUES ( \
					@folio, '%s', %d \
					) ",
					moduloreporte, tiposol);
					instrucciones[num_instrucciones++]=instruccion;
				}
			}


			if(departamento == "MER"){
				if(tipodiseno == 0){
					instruccion.sprintf("INSERT INTO solicitudticketsmercadotecnia ( \
					referencia, tipodiseno, tipomaterial, cantidad, ancho, alto, \
					lugarcoloca, textoimagen, imagenesincluye, terminosycondiciones \
					) VALUES ( \
					@folio, %d, '%s', %d, %f, %f, \
					'%s', '%s', '%s', '%s' \
					) ",
					tipodiseno, diseno, cantidad, ancho, alto,
					lugar, textoincluido, incluyeimagen, termicondic);
					instrucciones[num_instrucciones++]=instruccion;
				} else {

					instruccion.sprintf("INSERT INTO solicitudticketsmercadotecnia ( \
					referencia, tipodiseno, tipomaterial, cantidad, \
					textoimagen, imagenesincluye, terminosycondiciones \
					) VALUES ( \
					@folio, %d, '%s', %d, \
					'%s', '%s', '%s' \
					) ",
					tipodiseno, diseno, cantidad,
					textoincluido, incluyeimagen, termicondic);
					instrucciones[num_instrucciones++]=instruccion;
                }
			}

		} else {

			instruccion.sprintf("UPDATE solicitudtickets \
			SET idJefatura = '%s', fechapromesa = '%s', prioridad = %d, \
			fechamodi = CURDATE(), horamodi = CURTIME(), usumodi = '%s' \
			WHERE referencia = '%s' ",
			areaatencion, fechapromesa, prioridad,
			usuario,
			referencia);
			instrucciones[num_instrucciones++]=instruccion;

            try {
				instruccion.sprintf("SELECT solucionado FROM solicitudtickets WHERE referencia = '%s' ", referencia);

				if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp)) {
					if (resp->ObtieneNumRegistros()>0){
						solucionado = StrToInt(resp->ObtieneDato("solucionado"));
					}
				}
			} __finally {
				if (resp!=NULL) delete resp;
			}

			if(resuelto == 1){
				if(solucionado != resuelto) {
					instruccion.sprintf("UPDATE solicitudtickets \
					SET fechasolucion = CURDATE(), \
					solucionado = %d WHERE referencia = '%s' ",
					resuelto, referencia);
					instrucciones[num_instrucciones++]=instruccion;
				}
			} else {
				instruccion.sprintf("UPDATE solicitudtickets \
				SET fechasolucion = '0000-00-00', solucionado = %d \
				WHERE referencia = '%s' ",
				resuelto, referencia);
				instrucciones[num_instrucciones++]=instruccion;
			}

			if(validado == 1){
				instruccion.sprintf("UPDATE solicitudtickets \
				SET validado = %d, usuvalida = '%s', \
				fechavalidado = CURDATE(), horavalidado = CURTIME() \
				WHERE referencia = '%s' ",
				validado, usuario,
				referencia);
				instrucciones[num_instrucciones++]=instruccion;
			} else {
				instruccion.sprintf("UPDATE solicitudtickets \
				SET validado = %d, usuvalida = NULL, \
				fechavalidado = '0000-00-00', horavalidado = NULL \
				WHERE referencia = '%s' ",
				validado,
				referencia);
				instrucciones[num_instrucciones++]=instruccion;
			}

			if(retroalimentacion==""){
				instruccion.sprintf("REPLACE INTO solicitudticketsmensajes \
				(referencia, descripproblema) \
				VALUES \
				('%s', '%s') ",
				referencia, descripproblema);
				instrucciones[num_instrucciones++]=instruccion;
			}else{
				instruccion.sprintf("REPLACE INTO solicitudticketsmensajes \
				(referencia, descripproblema, retro) \
				VALUES \
				('%s', '%s', '%s') ",
				referencia, descripproblema, retroalimentacion);
				instrucciones[num_instrucciones++]=instruccion;
			}


			if(validado == 1){
				instruccion.sprintf("INSERT INTO solicitudticketsbitacora \
				(referencia, idJefatura, ususolicita, usumodi, terminal, fechasol, \
				horasol, fechapromesa, prioridad, departamento, tipo, solucionado, validado, responsable, \
				fechamodi, horamodi, usuvalida, fechavalidado, horavalidado, fechaini, horaini, fechasolucion ) \
				SELECT referencia, '%s', '%s', '%s', '%s', fechasol, \
				horasol, '%s', %d, '%s', '%s', %d, %d, responsable, \
				CURDATE(), CURTIME(), '%s', CURDATE(), CURTIME(), fechaini, horaini, fechasolucion \
				FROM solicitudtickets WHERE referencia = '%s' ",
				areaatencion, usuario, usuario, terminal,
				fechapromesa, prioridad, departamento, tarea, resuelto, validado, usuario,
                usuario,
				referencia);
				instrucciones[num_instrucciones++]=instruccion;
			} else {
				instruccion.sprintf("INSERT INTO solicitudticketsbitacora \
				(referencia, idJefatura, ususolicita, usumodi, terminal, fechasol, \
				horasol, fechapromesa, prioridad, departamento, tipo, solucionado, validado, responsable, \
				fechamodi, horamodi, fechaini, horaini, fechasolucion ) \
				SELECT referencia, '%s', '%s', '%s', '%s', fechasol, horasol, '%s', %d, '%s', '%s', %d, %d, responsable, \
				CURDATE(), CURTIME(), fechaini, horaini, fechasolucion \
				FROM solicitudtickets WHERE referencia = '%s' ",
				areaatencion, usuario, usuario, terminal, fechapromesa, prioridad, departamento, tarea, resuelto, validado,
				referencia);
				instrucciones[num_instrucciones++]=instruccion;
			}

			if(departamento == "MER"){
				if(tipodiseno == 0){
					if(nombreimagen != " ") {
						instruccion.sprintf("REPLACE INTO solicitudticketsimagenes \
						(referencia, nombre, formato, imagen) \
						VALUES \
						('%s', '%s', '%s', '%s') ",
						referencia, nombreimagen, formato, imagen);
						instrucciones[num_instrucciones++]=instruccion;
					}
				}
			} else {
				if(nombreimagen != " ") {
					instruccion.sprintf("REPLACE INTO solicitudticketsimagenes \
					(referencia, nombre, formato, imagen) \
					VALUES \
					('%s', '%s', '%s', '%s') ",
					referencia, nombreimagen, formato, imagen);
					instrucciones[num_instrucciones++]=instruccion;
				}
			}

			if(departamento == "RHH"){

				instruccion.sprintf("UPDATE solicitudticketsrh SET \
				puestosoli = '%s', tipoBaja = '%s', nombreEmpleado = '%s', numvacantes = %s, tiporeemplazo = %d, escolaridad = %d, sexo = %d, \
				sueldo = '%s', horariocubrir = '%s', diastrabajo = '%s', laboradomingo = %d, \
				actividadespuesto = '%s' \
				WHERE referencia = '%s' ",
				puestosolicita, tipoBaja, nombreEmpleado, numvacantes, tiporeemplazo, escolaridad,
				sexo, sueldo, horario, diastrabajo, laboradomingo, actividades, referencia);
				instrucciones[num_instrucciones++]=instruccion;
			}

			if(departamento == "DES"){

				if(tiposol == 2) {
					instruccion.sprintf("UPDATE solicitudticketsdesarrollo SET \
					moduloreporte = '%s', tiposolicitud = %d, descricaract = '%s' \
					WHERE referencia = '%s' ",
					moduloreporte, tiposol, nuevacaracter, referencia);
					instrucciones[num_instrucciones++]=instruccion;
				} else {
					instruccion.sprintf("UPDATE solicitudticketsdesarrollo SET \
					moduloreporte = '%s', tiposolicitud = %d, descricaract = NULL \
					WHERE referencia = '%s' ",
					moduloreporte, tiposol, referencia);
					instrucciones[num_instrucciones++]=instruccion;
				}
			}

			if(departamento == "MER"){
				if(tipodiseno == 0){
					instruccion.sprintf("UPDATE solicitudticketsmercadotecnia SET \
					tipodiseno = %d, tipomaterial = '%s', cantidad = %d, ancho = %f, alto = %f, \
					lugarcoloca = '%s', textoimagen = '%s', imagenesincluye = '%s', terminosycondiciones = '%s' \
					WHERE referencia = '%s'",
					tipodiseno, diseno, cantidad, ancho, alto,
					lugar, textoincluido, incluyeimagen, termicondic, referencia);
					instrucciones[num_instrucciones++]=instruccion;
				} else {
					instruccion.sprintf("UPDATE solicitudticketsmercadotecnia SET \
					tipodiseno = %d, tipomaterial = '%s', cantidad = %d, ancho = NULL, alto = NULL, \
					lugarcoloca = NULL, textoimagen = '%s', imagenesincluye = '%s', terminosycondiciones = '%s' \
					WHERE referencia = '%s'",
					tipodiseno, diseno, cantidad,
					textoincluido, incluyeimagen, termicondic, referencia);
					instrucciones[num_instrucciones++]=instruccion;
                }
			}

			instruccion.sprintf("set @folio= '%s'", referencia);
			instrucciones[num_instrucciones++]=instruccion;
		}

		instrucciones[num_instrucciones++] = "COMMIT";

		aux_buffer_sql = mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i = 0; i < num_instrucciones; i++){
		  aux_buffer_sql = mFg.AgregaStringABuffer(instrucciones[i],aux_buffer_sql);
		}

		mServidorVioleta->InicializaBuffer(Respuesta,TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,buffer_sql)) {
			instruccion = "SELECT @folio AS folio";
			mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}

	}__finally {
		delete buffer_sql;
	}
}
// ---------------------------------------------------------------------------
// ID_CON_SOLI_TICKET_SOPORTE
void ServidorCatalogos::EjecutaConsultaSolTicketSoporte(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	BufferRespuestas* resp_departamento=NULL;
	AnsiString instruccion;
	AnsiString clave_solicitud, departamento;

	clave_solicitud = mFg.ExtraeStringDeBuffer(&parametros);

	try {
		instruccion.sprintf("SELECT departamento FROM solicitudtickets WHERE referencia = '%s' ",
		clave_solicitud);

		if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_departamento)) {
			if (resp_departamento->ObtieneNumRegistros()>0){
				departamento = resp_departamento->ObtieneDato("departamento");
			}
		}
	} __finally {
		if (resp_departamento!=NULL) delete resp_departamento;
	}

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);


	if(departamento == "CON" || departamento == "COM" || departamento == "MAC" || departamento == "ELR" || departamento == "REC" || departamento == "SME") {
		instruccion.sprintf("SELECT \
			s.referencia, \
			s.fechasol, \
			CONCAT(us.nombre,' ',us.appat,' ',us.apmat,' ') AS ususolicita, \
			CONCAT(r.nombre,' ',r.appat,' ',r.apmat,' ') AS responsable, \
			s.idJefatura, \
			s.solucionado, \
			s.validado, \
			sm.descripproblema, \
			s.prioridad, \
			s.fechapromesa, \
			s.fechaini, \
			s.fechasolucion, \
			s.fechavalidado, \
			IFNULL(sm.retro,'') AS retro, \
			CONCAT(ixst.nombre,'.',ixst.formato) AS imagen, \
			s.cancelado, \
			s.fechacancela, \
			s.horacancela \
		FROM solicitudtickets s \
		INNER JOIN solicitudticketsmensajes sm ON sm.referencia = s.referencia \
		INNER JOIN empleados us ON us.empleado = s.ususolicita \
		LEFT JOIN empleados r ON r.empleado = s.responsable \
		LEFT JOIN solicitudticketsimagenes ixst ON ixst.referencia = s.referencia \
		WHERE s.referencia = '%s'",
		clave_solicitud);
	}

	if(departamento == "RHH"){
		instruccion.sprintf("SELECT \
			s.referencia, \
			s.fechasol, \
			CONCAT(us.nombre,' ',us.appat,' ',us.apmat,' ') AS ususolicita, \
			CONCAT(r.nombre,' ',r.appat,' ',r.apmat,' ') AS responsable, \
			s.idJefatura, \
			s.solucionado, \
			s.validado, \
			sm.descripproblema, \
			s.prioridad, \
			s.fechapromesa, \
			s.fechaini, \
			s.fechasolucion, \
			s.fechavalidado, \
			IFNULL(sm.retro,'') AS retro, \
			strh.numvacantes, \
			strh.puestosoli, \
			strh.tiporeemplazo, \
			strh.escolaridad, \
			strh.sexo, \
			strh.sueldo, \
			strh.horariocubrir, \
			strh.diastrabajo, \
			strh.laboradomingo, \
			strh.actividadespuesto, \
			s.cancelado, \
			s.fechacancela, \
			s.horacancela, \
			strh.otrosconocimientos, \
			strh.capacitacion, \
			strh.fechainicapacitacion, \
			strh.fechafincapacitacion, \
			strh.tipoBaja, \
			strh.nombreEmpleado \
		FROM solicitudtickets s \
		INNER JOIN solicitudticketsrh strh ON strh.referencia = s.referencia \
		INNER JOIN solicitudticketsmensajes sm ON sm.referencia = s.referencia \
		INNER JOIN empleados us ON us.empleado = s.ususolicita \
		LEFT JOIN empleados r ON r.empleado = s.responsable \
		WHERE s.referencia = '%s'",
		clave_solicitud);
	}

	if(departamento == "DES"){
		instruccion.sprintf("SELECT \
			s.referencia, \
			s.fechasol, \
			CONCAT(us.nombre,' ',us.appat,' ',us.apmat,' ') AS ususolicita, \
			CONCAT(r.nombre,' ',r.appat,' ',r.apmat,' ') AS responsable, \
			s.idJefatura, \
			s.solucionado, \
			s.validado, \
			sm.descripproblema, \
			s.prioridad, \
			s.fechapromesa, \
			s.fechaini, \
			s.fechasolucion, \
			s.fechavalidado, \
			IFNULL(sm.retro,'') AS retro, \
			std.moduloreporte, \
			std.tiposolicitud, \
			std.descricaract, \
			s.cancelado, \
			s.fechacancela, \
			s.horacancela \
		FROM solicitudtickets s \
		INNER JOIN solicitudticketsdesarrollo std ON std.referencia = s.referencia \
		INNER JOIN solicitudticketsmensajes sm ON sm.referencia = s.referencia \
		INNER JOIN empleados us ON us.empleado = s.ususolicita \
		LEFT JOIN empleados r ON r.empleado = s.responsable \
		WHERE s.referencia = '%s'",
		clave_solicitud);
	}

	if(departamento == "MER"){
		instruccion.sprintf("SELECT \
			s.referencia, \
			s.fechasol, \
			CONCAT(us.nombre,' ',us.appat,' ',us.apmat,' ') AS ususolicita, \
			CONCAT(r.nombre,' ',r.appat,' ',r.apmat,' ') AS responsable, \
			s.idJefatura, \
			s.solucionado, \
			s.validado, \
			sm.descripproblema, \
			s.prioridad, \
			s.fechapromesa, \
			s.fechaini, \
			s.fechasolucion, \
			s.fechavalidado, \
			IFNULL(sm.retro,'') AS retro, \
			stm.tipodiseno, \
			stm.tipomaterial, \
			stm.cantidad, \
			stm.ancho, \
			stm.alto, \
			stm.lugarcoloca, \
			stm.textoimagen, \
			stm.imagenesincluye, \
			stm.terminosycondiciones, \
			s.cancelado, \
			s.fechacancela, \
			s.horacancela, \
			CONCAT(smi.nombre,'.',smi.formato) AS imagen \
		FROM solicitudtickets s \
		INNER JOIN solicitudticketsmercadotecnia stm ON stm.referencia = s.referencia \
		INNER JOIN solicitudticketsmensajes sm ON sm.referencia = s.referencia \
		INNER JOIN empleados us ON us.empleado = s.ususolicita \
		LEFT JOIN empleados r ON r.empleado = s.responsable \
        LEFT JOIN solicitudticketsimagenes smi ON smi.referencia = s.referencia \
		WHERE s.referencia = '%s'",
		clave_solicitud);
	}

	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------
// ID_BUSQ_SOLI_TICKET_SOPORTE
void ServidorCatalogos::BusquedaSolTicketSoporte(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
	AnsiString tipo_busqueda, departamento;
	AnsiString dato_buscado, fecha_ini, fecha_fin, sucursal;

	departamento=mFg.ExtraeStringDeBuffer(&parametros);
	tipo_busqueda=mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ);

	if (tipo_busqueda=="FECHASOL"){
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		sucursal=mFg.ExtraeStringDeBuffer(&parametros);

		instruccion.sprintf("SELECT \
			s.referencia, \
			s.fechasol, \
			s.horasol, \
			CONCAT(us.nombre,' ',us.appat,' ',us.apmat,' ') AS ususolicita, \
			s.departamento, \
			s.cancelado \
		FROM solicitudtickets s \
		INNER JOIN solicitudticketsmensajes sm ON sm.referencia = s.referencia \
		INNER JOIN empleados us ON us.empleado = s.ususolicita \
		INNER JOIN terminales ter ON ter.terminal = s.terminal \
		INNER JOIN secciones sec ON sec.seccion = ter.seccion \
		INNER JOIN sucursales suc ON sec.sucursal = suc.sucursal \
		WHERE s.fechasol BETWEEN '%s' AND '%s' \
        AND s.departamento = '%s' \
		GROUP BY s.referencia \
		ORDER BY s.fechasol, s.referencia \
		LIMIT 3000 ", fecha_ini, fecha_fin, departamento);
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}

	if (tipo_busqueda=="FECHAPROM"){
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		sucursal=mFg.ExtraeStringDeBuffer(&parametros);

		instruccion.sprintf("SELECT \
			s.referencia, \
			s.fechasol, \
			s.horasol, \
			CONCAT(us.nombre,' ',us.appat,' ',us.apmat,' ') AS ususolicita, \
			s.departamento, \
			s.cancelado \
		FROM solicitudtickets s \
		INNER JOIN solicitudticketsmensajes sm ON sm.referencia = s.referencia \
		INNER JOIN empleados us ON us.empleado = s.ususolicita \
		INNER JOIN terminales ter ON ter.terminal = s.terminal \
		INNER JOIN secciones sec ON sec.seccion = ter.seccion \
		INNER JOIN sucursales suc ON sec.sucursal = suc.sucursal \
		WHERE s.fechapromesa BETWEEN '%s' AND '%s' \
		AND s.departamento = '%s' \
		GROUP BY s.referencia \
		ORDER BY s.fechapromesa, s.referencia \
		LIMIT 3000 ", fecha_ini, fecha_fin, departamento);
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}

	if (tipo_busqueda=="FECHAPROM"){
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		sucursal=mFg.ExtraeStringDeBuffer(&parametros);

		instruccion.sprintf("SELECT \
			s.referencia, \
			s.fechasol, \
			s.horasol, \
			CONCAT(us.nombre,' ',us.appat,' ',us.apmat,' ') AS ususolicita, \
			s.departamento, \
			s.cancelado \
		FROM solicitudtickets s \
		INNER JOIN solicitudticketsmensajes sm ON sm.referencia = s.referencia \
		INNER JOIN empleados us ON us.empleado = s.ususolicita \
		INNER JOIN terminales ter ON ter.terminal = s.terminal \
		INNER JOIN secciones sec ON sec.seccion = ter.seccion \
		INNER JOIN sucursales suc ON sec.sucursal = suc.sucursal \
		WHERE s.fechasol BETWEEN '%s' AND '%s' \
		AND s.departamento = '%s' \
		GROUP BY s.referencia \
		ORDER BY s.fechapromesa, s.referencia \
		LIMIT 3000 ", fecha_ini, fecha_fin, departamento);
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}

	if (tipo_busqueda=="DEPART"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("SELECT \
			s.referencia, \
			s.fechasol, \
			s.horasol, \
			CONCAT(us.nombre,' ',us.appat,' ',us.apmat,' ') AS ususolicita, \
			s.departamento, \
			s.cancelado \
		FROM solicitudtickets s \
		INNER JOIN solicitudticketsmensajes sm ON sm.referencia = s.referencia \
		INNER JOIN empleados us ON us.empleado = s.ususolicita \
		INNER JOIN terminales ter ON ter.terminal = s.terminal \
		INNER JOIN secciones sec ON sec.seccion = ter.seccion \
		INNER JOIN sucursales suc ON sec.sucursal = suc.sucursal \
		WHERE s.fechasol BETWEEN '%s' AND '%s' AND s.departamento = '%s' \
		AND s.departamento = '%s' \
		GROUP BY s.referencia \
		ORDER BY s.fechapromesa, s.referencia \
		LIMIT 3000 ", fecha_ini, fecha_fin, dato_buscado, departamento);
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}

	if (tipo_busqueda=="REFE"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("SELECT \
			s.referencia, \
			s.fechasol, \
			s.horasol, \
			CONCAT(us.nombre,' ',us.appat,' ',us.apmat,' ') AS ususolicita, \
			s.departamento, \
			s.cancelado \
		FROM solicitudtickets s \
		INNER JOIN solicitudticketsmensajes sm ON sm.referencia = s.referencia \
		INNER JOIN empleados us ON us.empleado = s.ususolicita \
		INNER JOIN terminales ter ON ter.terminal = s.terminal \
		INNER JOIN secciones sec ON sec.seccion = ter.seccion \
		INNER JOIN sucursales suc ON sec.sucursal = suc.sucursal \
		WHERE s.fechasol BETWEEN '%s' AND '%s' AND s.referencia = '%s' \
		AND s.departamento = '%s' \
		GROUP BY s.referencia \
		ORDER BY s.fechapromesa, s.referencia \
		LIMIT 3000 ", fecha_ini, fecha_fin, dato_buscado, departamento);
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}

	if (tipo_busqueda=="") {
		// Obtiene todos los posibles terminales
		instruccion="SELECT departamento, nombre FROM departamentossoporte WHERE activo = 1 ORDER BY nombre";
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
}
// ---------------------------------------------------------------------------
// ID_ENVIA_CORREO_SOLI_TICKET_SOPORTE
void ServidorCatalogos::EjecutaEnvioCorreoTicketSoporte(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
	AnsiString email, bodyMsg;
	BufferRespuestas* resp_departamento = NULL;
	BufferRespuestas* resp_envio = NULL;
	BufferRespuestas* resp_config_cfd = NULL;

	AnsiString nomremitente, emailcfd, hostsmtp, portsmtp, usersmtp, passsmtp;
	AnsiString clave_solicitud, departamento, mensaje;

	clave_solicitud = mFg.ExtraeStringDeBuffer(&parametros);

	try {
		instruccion.sprintf("SELECT ds.nombre AS departamento, ds.email FROM solicitudtickets st \
		INNER JOIN departamentossoporte ds ON ds.departamento = st.departamento WHERE st.referencia = '%s' ",
		clave_solicitud);

		if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_departamento)) {
			if (resp_departamento->ObtieneNumRegistros()>0){
				departamento = resp_departamento->ObtieneDato("departamento");
				email = resp_departamento->ObtieneDato("email");
			}
		}
	} __finally {
		if (resp_departamento!=NULL) delete resp_departamento;
	}

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	bodyMsg = "Se ha enviado una solicitud de soporte al departamento " + departamento;

	// Se envia el CFD al correo del cliente
	TIdMessage *msg = NULL;
	TIdSMTP *smtp = NULL;
	TIdText *idtext = NULL;
	TIdSSLIOHandlerSocketOpenSSL *IdSSLIOHandlerSocketOpenSSL1 = NULL;

	#ifdef _DEBUG
	TIdLogFile *IdLogFile1 = NULL;
	#endif

	try {

		try {

			// Consulta de parametroscfd
			instruccion.sprintf("select p.* from parametroscfd p where p.sucursal='%s'", FormServidor->ObtieneClaveSucursal());
			if (!mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), resp_config_cfd))
				throw Exception("ERROR AL CONSULTAR!");
			for (int y = 0; y < resp_config_cfd->ObtieneNumRegistros(); y++) {
				emailcfd = resp_config_cfd->ObtieneDato("emailenviocfd");
				nomremitente = "Solicitud de soporte para el departamento " + departamento;
				hostsmtp = resp_config_cfd->ObtieneDato("hostsmtp");
				portsmtp = resp_config_cfd->ObtieneDato("portsmtp");
				usersmtp = resp_config_cfd->ObtieneDato("usersmtp");
				passsmtp = resp_config_cfd->ObtieneDato("passsmtp");
			}

			// Configuracion del componente TIdSMTP
			smtp = new TIdSMTP(NULL);
			smtp->MailAgent = "Indy10";
			smtp->PipeLine = false;
			smtp->UseEhlo = true;
			smtp->UseVerp = false;

			if (hostsmtp.Pos("amazon") > 0) {

				IdSSLIOHandlerSocketOpenSSL1 = new TIdSSLIOHandlerSocketOpenSSL(NULL);
				IdSSLIOHandlerSocketOpenSSL1->SSLOptions->Method = Idsslopenssl::sslvSSLv23;
				IdSSLIOHandlerSocketOpenSSL1->SSLOptions->Mode = Idsslopenssl::sslmClient;
				smtp->IOHandler = IdSSLIOHandlerSocketOpenSSL1;
				smtp->UseTLS = utUseExplicitTLS;
			} else {
				smtp->UseTLS = utNoTLSSupport;
			}

			smtp->ConnectTimeout = (1000 * 30);
			smtp->ReadTimeout = (1000 * 60);

			smtp->AuthType = satDefault;
			smtp->Host = hostsmtp;
			smtp->Username = usersmtp;
			smtp->Password = passsmtp;
			smtp->Port = portsmtp.ToInt();

			// Configuracion del componente TIdMessage
			msg = new TIdMessage(NULL);
			msg->ExceptionOnBlockedAttachments = false;
			msg->NoDecode = false;
			msg->NoEncode = false;
			msg->UseNowForDate = true;

			// Configuracion del componente TIdLogFile
			#ifdef _DEBUG
			IdLogFile1 = new TIdLogFile(NULL);
			IdLogFile1->Filename = ExtractFilePath(Application->ExeName) + "\\maillogfile.log";
			IdLogFile1->Active = true;
			IdLogFile1->LogTime = false;
			IdLogFile1->ReplaceCRLF = true;
			smtp->Intercept = IdLogFile1;
			#endif

			// ENVIO DEL CFD
			msg->Clear();
			msg->MessageParts->Clear();

			msg->From->Address = emailcfd;
			msg->From->Name = nomremitente;
			msg->Sender->Address = emailcfd;
			msg->Sender->Name = nomremitente;
			msg->ReplyTo->EMailAddresses = emailcfd;
			msg->Recipients->EMailAddresses = email;
			msg->Priority = mpNormal;
			msg->Subject = "Solicitud de ticket de soporte";
			msg->ContentType = "multipart/mixed";
			msg->ClearBody();
			msg->IsEncoded = true;
			msg->AttachmentEncoding = "MIME";
			msg->Encoding = meMIME;
			msg->ConvertPreamble = true;
			msg->CharSet = "UTF-8";

			idtext = new TIdText(msg->MessageParts, NULL);
			idtext->ContentType = "text/html";
			idtext->ContentDescription = "multipart-1";
			idtext->CharSet = "UTF-8";
			idtext->ContentTransfer = "8bit";
			idtext->Body->Clear();
			idtext->Body->Add(UTF8String(bodyMsg));

			smtp->Connect();
			smtp->Authenticate();
			smtp->Send(msg);
			smtp->Disconnect();

			AnsiString sqlinstruccion;
			sqlinstruccion.sprintf("SELECT 0 AS error");
			mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

		} catch (Exception &e) {
			mServidorVioleta->MuestraMensajeError("ERROR al enviar correo: " + e.Message);
			throw Exception("ERROR AL ENVIAR CORREO!" + e.Message);
		}

	} __finally {
		#ifdef _DEBUG
		if (IdLogFile1 != NULL) {
			IdLogFile1->Close();
			delete IdLogFile1;
		}
		#endif

		if (smtp != NULL)
			delete smtp;
		if (idtext != NULL)
			delete idtext;
		if (msg != NULL)
			delete msg;

		if (IdSSLIOHandlerSocketOpenSSL1 != NULL)
			delete IdSSLIOHandlerSocketOpenSSL1;
		if (resp_envio != NULL)
			delete resp_envio;
		if (resp_config_cfd != NULL)
			delete resp_config_cfd;

	}

}
// ---------------------------------------------------------------------------
// ID_CON_CAT_DEPART_SOPORTE
void ServidorCatalogos::EjecutaConsultaCatalogoDepartamentosSoporte(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
	AnsiString clave;

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos de las cancelaciones
	instruccion.sprintf("SELECT * FROM departamentossoporte WHERE departamento = '%s' ", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene todas las cancelaciones
	instruccion = "SELECT departamento, nombre, email, diasdefault, activo FROM departamentossoporte ORDER BY nombre";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------
// ID_GRA_CAT_DEPART_SOPORTE
void ServidorCatalogos::EjecutaGrabaCatalogoDepartamentosSoporte(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{

	char *buffer_sql = new char[1024 * 64];
	char *aux_buffer_sql = buffer_sql;
	int num_instrucciones = 0;
	int i = 0;
	AnsiString instruccion, instrucciones[20];
	AnsiString tarea, departamento, nombre, email;
	int activo, dias;

	tarea=mFg.ExtraeStringDeBuffer(&parametros);
	departamento=mFg.ExtraeStringDeBuffer(&parametros);
	nombre=mFg.ExtraeStringDeBuffer(&parametros);
	email=mFg.ExtraeStringDeBuffer(&parametros);
	dias=StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
	activo=StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
	try {
		instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++] = "START TRANSACTION";

		if (tarea == "A"){
			instruccion.sprintf("INSERT INTO departamentossoporte \
			(departamento, nombre, activo, email, diasdefault ) \
			VALUES \
			('%s', '%s', %d, '%s', %d) ",
			departamento, nombre, activo, email, dias);
			instrucciones[num_instrucciones++]=instruccion;
		} else {
			instruccion.sprintf("UPDATE departamentossoporte SET \
			nombre = '%s', activo = %d, email = '%s', diasdefault = %d \
			WHERE departamento = '%s' ",
			nombre, activo, email, dias, departamento);
			instrucciones[num_instrucciones++]=instruccion;
        }

		instrucciones[num_instrucciones++] = "COMMIT";

		aux_buffer_sql = mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i = 0; i < num_instrucciones; i++){
		  aux_buffer_sql = mFg.AgregaStringABuffer(instrucciones[i],aux_buffer_sql);
		}

		mServidorVioleta->InicializaBuffer(Respuesta,TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,buffer_sql);

	}__finally {
		delete buffer_sql;
	}
}

void ServidorCatalogos::GrabaCambiarProductoPresentacion(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA ID_CAMPRODPRES

	char * buffer_funciones = NULL;
	BufferRespuestas * respuesta_cambios = NULL;

	AnsiString instruccion;
	AnsiString id_campprogpres;
	AnsiString condicion_idcamprogpres = " ";
	int id_respuesta = Respuesta->Id;

	id_campprogpres = mFg.ExtraeStringDeBuffer(&parametros);

	if (id_campprogpres != " ") {
		condicion_idcamprogpres.sprintf(" AND id = '%s' ", id_campprogpres);
	}

	try{
		//Aplicar todos los cambios a producto-presentación
		instruccion.sprintf("SELECT id, prod_origen, present_origen,  prod_destino, present_destino, nombre_origen, \
		nombre_destino, tipo, usuario, suc_prog, DATE_FORMAT(fecha_prog,'%%Y-%%m-%%d %%H:%%i') as fecha_prog \
		FROM camprogprodpresent \
		WHERE tipo = 'CPP' %s ORDER BY id ASC", condicion_idcamprogpres);
		if(mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), respuesta_cambios)){

			instruccion.sprintf("INSERT INTO bitacoraservidor VALUES (NULL,'%s',NOW(), \
				'INICIO_CAMBIOS_PRODPRES')",FormServidor->ObtieneClaveSucursal());
			mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str());

			for(int i = 0; i < respuesta_cambios->ObtieneNumRegistros(); i++){
				MYSQL * mySql;
				try{
					try {
						mySql = mServidorVioleta->mFuncionesMySql->Conectarse();
						buffer_funciones = new char[1024 * 64];
						char * puntero_funciones = buffer_funciones;

						respuesta_cambios->IrAlRegistroNumero(i);

						puntero_funciones = mFg.AgregaStringABuffer(respuesta_cambios->ObtieneDato("prod_origen"), puntero_funciones);
						puntero_funciones = mFg.AgregaStringABuffer(respuesta_cambios->ObtieneDato("present_origen"), puntero_funciones);
						puntero_funciones = mFg.AgregaStringABuffer(respuesta_cambios->ObtieneDato("prod_destino"), puntero_funciones);
						puntero_funciones = mFg.AgregaStringABuffer(respuesta_cambios->ObtieneDato("present_destino"), puntero_funciones);
						puntero_funciones = mFg.AgregaStringABuffer(respuesta_cambios->ObtieneDato("nombre_origen"), puntero_funciones);
						puntero_funciones = mFg.AgregaStringABuffer(respuesta_cambios->ObtieneDato("nombre_destino"), puntero_funciones);
						puntero_funciones = mFg.AgregaStringABuffer(respuesta_cambios->ObtieneDato("usuario"), puntero_funciones);
						puntero_funciones = mFg.AgregaStringABuffer(respuesta_cambios->ObtieneDato("id"), puntero_funciones);
						puntero_funciones = mFg.AgregaStringABuffer(respuesta_cambios->ObtieneDato("suc_prog"), puntero_funciones);
						puntero_funciones = mFg.AgregaStringABuffer(respuesta_cambios->ObtieneDato("fecha_prog"), puntero_funciones);
						//ID_MOD_PROD_PRES para las tareas programadas
						ModificarProductoPresent(Respuesta, mySql, buffer_funciones);

					} catch (Exception &e) {
						mServidorVioleta->MuestraMensajeError("ERROR al aplicar cambios a producto-presentación: "+AnsiString(e.Message));
						mServidorVioleta->MuestraMensaje("-- ERROR al aplicar cambios a producto-presentación: "+AnsiString(e.Message), id_respuesta);

						instruccion.sprintf("INSERT INTO bitprodpresent (prod_origen, present_origen, prod_destino, present_destino, \
							nombre_destino, nombre_origen, tipo, usuario, fecha_prog, mensaje, suc_prog, sucursal, correcto) VALUES \
							('%s', '%s', '%s', '%s', '%s', '%s', 'CPP', '%s', '%s', '%s', '%s', '%s', 0)",
							respuesta_cambios->ObtieneDato("prod_origen"),
							respuesta_cambios->ObtieneDato("present_origen"),
							respuesta_cambios->ObtieneDato("prod_destino"),
							respuesta_cambios->ObtieneDato("present_destino"),
							respuesta_cambios->ObtieneDato("nombre_origen"),
							respuesta_cambios->ObtieneDato("nombre_destino"),
							respuesta_cambios->ObtieneDato("usuario"),
										respuesta_cambios->ObtieneDato("fecha_prog"),
							mFg.limpiarComillas(e.Message),
							respuesta_cambios->ObtieneDato("suc_prog"),
										FormServidor->ObtieneClaveSucursal()
							);
						mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL, instruccion.c_str());
					}
				}__finally{
					mServidorVioleta->mFuncionesMySql->Desconectarse(mySql);
				}
				delete buffer_funciones;
				buffer_funciones = NULL;
			}

			instruccion.sprintf("INSERT INTO bitacoraservidor VALUES (NULL,'%s',NOW(), \
				'FIN_CAMBIOS_PRODPRES')",FormServidor->ObtieneClaveSucursal());
			mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str());
		}
	}__finally{
		if(buffer_funciones != NULL) delete buffer_funciones;
		if(respuesta_cambios != NULL) delete respuesta_cambios;
	}

}

// ---------------------------------------------------------------------------
// ID_CON_CUENTAS_RETENCIONES
void ServidorCatalogos::ConsultaCuentasRetenciones(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA LAS CUENTAS DE RETENCIONES PARA GASTOS
	AnsiString instruccion;
	AnsiString cuenta, impuesto, sucursal;

	cuenta = mFg.ExtraeStringDeBuffer(&parametros);
	impuesto = mFg.ExtraeStringDeBuffer(&parametros);
	sucursal = mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos de la cuenta de retención
	instruccion.sprintf("SELECT * FROM cuentas_retenciones WHERE numero_cuenta='%s' AND impuesto='%s' AND sucursal='%s' ",
		cuenta, impuesto, sucursal );
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los tipos de precios
	instruccion.sprintf("SELECT cr.impuesto, cr.numero_cuenta, cc.nombrecuenta, cr.tasa, cr.sucursal \
		FROM cuentas_retenciones cr \
		INNER JOIN cuentascont cc ON cc.numcuenta = cr.numero_cuenta AND cc.sucursal=cr.sucursal \
		INNER JOIN sucursales suc ON suc.sucursal = cr.sucursal \
		WHERE suc.idempresa=%s \
		ORDER BY impuesto, numero_cuenta ", FormServidor->ObtieneClaveEmpresa());
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}

// ---------------------------------------------------------------------------
// ID_GRA_CUENTA_RETENCION
void ServidorCatalogos::GrabaCuentaRetencion(RespuestaServidor *Respuesta, MYSQL *MySQL,
	char *parametros) {

	char *buffer_sql = new char[1024 * 5];
	char *aux_buffer_sql = buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instrucciones[10];
	AnsiString instruccion;
	int num_instrucciones = 0;
	AnsiString cuenta, impuesto, sucursal, tarea;
	double tasa;
	char tipo;
	int i;

	try {
		cuenta = mFg.ExtraeStringDeBuffer(&parametros);
		impuesto = mFg.ExtraeStringDeBuffer(&parametros);
		sucursal = mFg.ExtraeStringDeBuffer(&parametros);
		AnsiString tasa_ansi = mFg.ExtraeStringDeBuffer(&parametros);
		tasa = tasa_ansi.ToDouble();
		tarea = mFg.ExtraeStringDeBuffer(&parametros);

		instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++] = "START TRANSACTION";
        /*
		datos.AsignaTabla("cuentas_retenciones");
		parametros += datos.InsCamposDesdeBuffer(parametros);
		tasa = datos.ObtieneValorCampo("tasa").ToDouble();  */

		if (tarea == "A") {
			instruccion.sprintf(
				"INSERT INTO  cuentas_retenciones (numero_cuenta, impuesto, sucursal, tasa) \
					VALUES ('%s', '%s', '%s', %f) ",
					cuenta, impuesto, sucursal, tasa);
			instrucciones[num_instrucciones++] = instruccion;
		}
		else {
			instruccion.sprintf(
				"UPDATE cuentas_retenciones SET tasa=%f \
					WHERE numero_cuenta='%s' AND impuesto='%s' AND sucursal='%s'  ",
					tasa, cuenta, impuesto, sucursal);
			instrucciones[num_instrucciones++] = instruccion;
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
// ---------------------------------------------------------------------------
// ID_BAJ_CUENTA_RETENCION
void ServidorCatalogos::BajaCuentaRetencion(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {

	char *buffer_sql = new char[1024 * 5];
	char *aux_buffer_sql = buffer_sql;
	AnsiString instruccion;
	int num_instrucciones = 2;
	AnsiString cuenta, impuesto, sucursal;

	try {
		cuenta = mFg.ExtraeStringDeBuffer(&parametros);
		impuesto = mFg.ExtraeStringDeBuffer(&parametros);
		sucursal = mFg.ExtraeStringDeBuffer(&parametros);

		aux_buffer_sql = mFg.AgregaStringABuffer
			(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);

		instruccion.sprintf("DELETE FROM cuentas_retenciones \
			WHERE numero_cuenta='%s' AND impuesto='%s' AND sucursal='%s' ",
		cuenta, impuesto, sucursal);
		aux_buffer_sql = mFg.AgregaStringABuffer(instruccion, aux_buffer_sql);


		mServidorVioleta->InicializaBuffer(Respuesta,
			TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,
			buffer_sql);
	}
	__finally {
		delete buffer_sql;
	}
}
// ---------------------------------------------------------------------------
// ID_REP_IMAGENES_PRESENT
void ServidorCatalogos::ReporteImagenesPresentaciones(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros){

		AnsiString producto = mFg.ExtraeStringDeBuffer(&parametros);
		AnsiString present = mFg.ExtraeStringDeBuffer(&parametros);
		AnsiString sucursal = mFg.ExtraeStringDeBuffer(&parametros);
		AnsiString clasif1 = mFg.ExtraeStringDeBuffer(&parametros);
		AnsiString clasif2 = mFg.ExtraeStringDeBuffer(&parametros);
		AnsiString clasif3 = mFg.ExtraeStringDeBuffer(&parametros);
        AnsiString imagen = mFg.ExtraeStringDeBuffer(&parametros);

		AnsiString wheres = " WHERE art.activo = 1 AND (cfg.permiteventas = 1 OR ISNULL(cfg.permiteventas)) ";

		if(producto != " "){
			AnsiString where;
			where.sprintf(" AND pro.producto = '%s' ", producto);
            wheres += where;
		}

		if(present != " "){
			AnsiString where;
			where.sprintf(" AND art.present = '%s' ", present);
			wheres += where;
		}

		if(sucursal != " "){
			AnsiString where;
			where.sprintf(" AND suc.sucursal = '%s' ", sucursal);
			wheres += where;
		}

		if(clasif3 != " "){
			AnsiString where;
			where.sprintf(" AND pro.clasif3 = '%s' ", clasif3);
			wheres += where;
		} else if(clasif2 != " "){
			AnsiString where;
			where.sprintf(" AND pro.clasif2 = '%s' ", clasif2);
			wheres += where;
		} else if(clasif1 != " "){
			AnsiString where;
			where.sprintf(" AND pro.clasif1 = '%s' ", clasif1);
			wheres += where;
		}

		if(imagen == "0"){
			wheres += " AND IFNULL(img.conteo, 0) = 0 ";
		} else if (imagen == "1"){
			wheres += " AND img.conteo > 0 ";
		}

		AnsiString instruccion;

		instruccion.sprintf(" SELECT art.producto, pro.nombre, art.present, IFNULL(img.conteo,0) AS conteo_imagenes, \
		IFNULL(img.conteo_kiosko, 0) AS conteo_kiosko, img.fecha_subida \
		FROM articulos art \
		INNER JOIN productos pro ON pro.producto = art.producto \
		INNER JOIN articulosxsuc axs ON axs.producto = art.producto AND axs.present = axs.present \
		INNER JOIN sucursales suc ON suc.sucursal = axs.sucursal \
		LEFT JOIN (SELECT img_.producto, img_.present, COUNT(img_.id)AS conteo, SUM(img_.parakiosko) as conteo_kiosko, \
			GROUP_CONCAT(fecha_subida) as fecha_subida from imagenesarticulos img_ \
			GROUP BY img_.producto, img_.present) img ON img.producto = art.producto AND img.present = art.present \
		LEFT JOIN articuloempresacfg cfg ON cfg.articulo = art.articulo AND cfg.idempresa = suc.idempresa \
		%s \
		GROUP by art.producto, art.present ", wheres);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);



}
// ---------------------------------------------------------------------------
// ID_REP_IMAGEN_PRESENT
void ServidorCatalogos::ImagenPresentacion(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros){

		AnsiString producto = mFg.ExtraeStringDeBuffer(&parametros);
		AnsiString present = mFg.ExtraeStringDeBuffer(&parametros);
		AnsiString id = mFg.ExtraeStringDeBuffer(&parametros);

		AnsiString wheres = " WHERE 1 ";

		if(producto != " "){
			AnsiString where;
			where.sprintf(" AND img.producto = '%s' ", producto);
			wheres += where;
		} else {
			throw new Exception("ID_REP_IMAGEN_PRESENT el parámetro 'producto' no puede estar vacío");
		}

		if(present != " "){
			AnsiString where;
			where.sprintf(" AND img.present = '%s' ", present);
			wheres += where;
		} else {
			throw new Exception("ID_REP_IMAGEN_PRESENT el parámetro 'present' no puede estar vacío");
        }

		AnsiString order = " ";
		if(id != " "){
			AnsiString ord;
			ord.sprintf("ORDER BY img.id = '%s' DESC ", id);
			order += ord;
		}

		AnsiString instruccion;

		instruccion.sprintf(" SELECT LAG(img.id) OVER (ORDER BY img.orden ASC) AS id_anterior, \
			LEAD(img.id) OVER (ORDER BY img.orden ASC) AS id_siguiente, \
			img.id, img.imagen, img.formato, img.descripcion, img.parakiosko FROM \
			imagenesarticulos img \
			%s %s LIMIT 1 ", wheres, order);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------
// ID_CANCELA_ORDEN_TRABAJO
void ServidorCatalogos::CancelaOrdenTrabajo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	BufferRespuestas* resp_param=NULL;
	BufferRespuestas* resp_productos=NULL;
	char *buffer_sql=new char[1024*64];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i;
	AnsiString instrucciones[10], Instruccion, instruccion;
	AnsiString referencia, usuario;

	referencia  = mFg.ExtraeStringDeBuffer(&parametros);
	usuario  = mFg.ExtraeStringDeBuffer(&parametros);

	try{

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		instruccion.sprintf("UPDATE solicitudtickets SET cancelado=1, \
        usucancela = '%s', fechacancela = CURDATE(), horacancela = CURTIME() \
		WHERE referencia = '%s' ", usuario, referencia);
		instrucciones[num_instrucciones++]=instruccion;

		instrucciones[num_instrucciones++]="COMMIT";


		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);

		for (i=0; i<num_instrucciones; i++){
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);
		}

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);

	} __finally {
		if(buffer_sql != NULL) delete buffer_sql;
	}
}
// ---------------------------------------------------------------------------
// ID_CON_JEFATURAS_ORDENES_TRABAJO
void ServidorCatalogos::ConsultaJefaturasOrdenesTrabajo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
	AnsiString clave;
	AnsiString condicion_clave=" ";

	clave = mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	if(clave != " "){
		condicion_clave.sprintf(" AND clave = '%s' ", clave);
	}

	instruccion.sprintf("SELECT clave, nombre, activo FROM catalogojefaturasordenestrabajo WHERE 1 %s ORDER BY nombre ASC ",
	condicion_clave);

	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	instruccion.sprintf("SELECT clave, nombre, activo FROM catalogojefaturasordenestrabajo ORDER BY nombre ASC");

	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------
// ID_GRA_JEFATURAS_ORDENES_TRABAJO
void ServidorCatalogos::GrabaJefaturasOrdenesTrabajo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql = new char[1024 * 64];
	char *aux_buffer_sql = buffer_sql;
	int num_instrucciones = 0;
	int i = 0;
	AnsiString instruccion, instrucciones[10];
	AnsiString clave, nombre, tarea;
	int activo;

	tarea=mFg.ExtraeStringDeBuffer(&parametros);
	clave=mFg.ExtraeStringDeBuffer(&parametros);
	nombre=mFg.ExtraeStringDeBuffer(&parametros);
	activo=StrToInt(mFg.ExtraeStringDeBuffer(&parametros));

	try {
		instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++] = "START TRANSACTION";

		if (tarea == "A"){
			instruccion.sprintf("INSERT INTO catalogojefaturasordenestrabajo \
			(clave, nombre, activo) \
			VALUES \
			('%s', '%s', %d) ",
			clave, nombre, activo);
			instrucciones[num_instrucciones++]=instruccion;
		} else {
			instruccion.sprintf("UPDATE catalogojefaturasordenestrabajo SET \
			nombre = '%s', activo = %d \
			WHERE clave = '%s' ",
			nombre, activo,
			clave);
			instrucciones[num_instrucciones++]=instruccion;
        }

		instrucciones[num_instrucciones++] = "COMMIT";

		aux_buffer_sql = mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i = 0; i < num_instrucciones; i++){
		  aux_buffer_sql = mFg.AgregaStringABuffer(instrucciones[i],aux_buffer_sql);
		}

		mServidorVioleta->InicializaBuffer(Respuesta,TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,buffer_sql);

	}__finally {
		delete buffer_sql;
	}
}
// ---------------------------------------------------------------------------
// ID_CON_CAT_BAJAS_RH
void ServidorCatalogos::EjecutaConsultaCatalogoBajasRH(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
	AnsiString clave;

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos de las cancelaciones
	instruccion.sprintf("SELECT * FROM catalogobajasrh WHERE clave = '%s' ", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene todas las cancelaciones
	instruccion = "SELECT clave, nombre, activo FROM catalogobajasrh ORDER BY nombre";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------
// ID_GRA_CAT_BAJAS_RH
void ServidorCatalogos::EjecutaGrabaCatalogoBajasRH(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{

	char *buffer_sql = new char[1024 * 64];
	char *aux_buffer_sql = buffer_sql;
	int num_instrucciones = 0;
	int i = 0;
	AnsiString instruccion, instrucciones[20];
	AnsiString tarea, clave, nombre, email;
	int activo, dias;

	tarea=mFg.ExtraeStringDeBuffer(&parametros);
	clave=mFg.ExtraeStringDeBuffer(&parametros);
	nombre=mFg.ExtraeStringDeBuffer(&parametros);
	activo=StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
	try {
		instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++] = "START TRANSACTION";

		if (tarea == "A"){
			instruccion.sprintf("INSERT INTO catalogobajasrh \
			(clave, nombre, activo) \
			VALUES \
			('%s', '%s', %d) ",
			clave, nombre, activo);
			instrucciones[num_instrucciones++]=instruccion;
		} else {
			instruccion.sprintf("UPDATE catalogobajasrh SET \
			nombre = '%s', activo = %d \
			WHERE clave = '%s' ",
			nombre, activo, clave);
			instrucciones[num_instrucciones++]=instruccion;
        }

		instrucciones[num_instrucciones++] = "COMMIT";

		aux_buffer_sql = mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i = 0; i < num_instrucciones; i++){
		  aux_buffer_sql = mFg.AgregaStringABuffer(instrucciones[i],aux_buffer_sql);
		}

		mServidorVioleta->InicializaBuffer(Respuesta,TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,buffer_sql);

	}__finally {
		delete buffer_sql;
	}
}
// ---------------------------------------------------------------------------
