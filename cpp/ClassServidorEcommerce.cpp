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
// ---------------------------------------------------------------------------
void ServidorEcommerce::GrabaGenerico(RespuestaServidor *Respuesta,
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
void ServidorEcommerce::BajaGenerico(RespuestaServidor *Respuesta,
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
void ServidorEcommerce::ConsultaGenerico(RespuestaServidor *Respuesta,
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
// ID_GRA_CLASIFICACION_ECOM1
void ServidorEcommerce::GrabaClasificacionEcom1(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA CLASIFICACION1
	GrabaGenerico(Respuesta, MySQL, parametros, "clasificacionecommerce1", "clasif1");
}
// ---------------------------------------------------------------------------
// ID_BAJ_CLASIFICACION_ECOM1
void ServidorEcommerce::BajaClasificacionEcom1(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// BAJA CLASIFICACION1
	BajaGenerico(Respuesta, MySQL, parametros, "clasificacionecommerce1", "clasif1");
}
// ---------------------------------------------------------------------------
// ID_CON_CLASIFICACION_ECOM1
void ServidorEcommerce::ConsultaClasificacionEcom1(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA CLASIFICACION1
	ConsultaGenerico(Respuesta, MySQL, parametros, "clasificacionecommerce1", "clasif1");
}
// ---------------------------------------------------------------------------
// ID_GRA_CLASIFICACION_ECOM2
void ServidorEcommerce::GrabaClasificacionEcom2(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA CLASIFICACION2
	GrabaGenerico(Respuesta, MySQL, parametros, "clasificacionecommerce2", "clasif2");
}
// ---------------------------------------------------------------------------
// ID_BAJ_CLASIFICACION_ECOM2
void ServidorEcommerce::BajaClasificacionEcom2(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// BAJA CLASIFICACION2
	BajaGenerico(Respuesta, MySQL, parametros, "clasificacionecommerce2", "clasif2");
}
// ---------------------------------------------------------------------------
// ID_CON_CLASIFICACION_ECOM2
void ServidorEcommerce::ConsultaClasificacionEcom2(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA CLASIFICACION2
	ConsultaGenerico(Respuesta, MySQL, parametros, "clasificacionecommerce2", "clasif2");
}
// ---------------------------------------------------------------------------
// ID_GRA_CLASIFICACION_ECOM3
void ServidorEcommerce::GrabaClasificacionEcom3(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// GRABA CLASIFICACION3
	GrabaGenerico(Respuesta, MySQL, parametros, "clasificacionecommerce3", "clasif3");
}
// ---------------------------------------------------------------------------
// ID_BAJ_CLASIFICACION_ECOM3
void ServidorEcommerce::BajaClasificacionEcom3(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// BAJA CLASIFICACION3
	BajaGenerico(Respuesta, MySQL, parametros, "clasificacionecommerce3", "clasif3");
}
// ---------------------------------------------------------------------------
// ID_CON_CLASIFICACION_ECOM3
void ServidorEcommerce::ConsultaClasificacionEcom3(RespuestaServidor *Respuesta,
	MYSQL *MySQL, char *parametros) {
	// CONSULTA CLASIFICACION3
	ConsultaGenerico(Respuesta, MySQL, parametros, "clasificacionecommerce3", "clasif3");
}
// ---------------------------------------------------------------------------
//ID_APLICA_RECLASIFICA_PRODUCTOS_ECOMMERCE
void ServidorEcommerce::AplicaReclasificaProductosEcommerce(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
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
			set pro.clasifecom1='%s', pro.clasifecom2='%s', pro.clasifecom3='%s' where pro.producto in (%s) ",
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
//ID_GRA_TAGS_ARTICULO_ECOMMERCE
void ServidorEcommerce::GuardaTagsArticulosEcommerce(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*30*10];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instruccion, instrucciones[50];
	AnsiString articulo, idTagsList, usuario;

	articulo = mFg.ExtraeStringDeBuffer(&parametros);
	idTagsList = mFg.ExtraeStringDeBuffer(&parametros);
	usuario = mFg.ExtraeStringDeBuffer(&parametros);

	//iniciar la transaccion
	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	TStringDynArray idTags(SplitString(idTagsList, ","));

	try{

		Instruccion.sprintf("DELETE FROM articulosecomtagsasignados WHERE articulo = '%s' ", articulo);
		instrucciones[num_instrucciones++]=Instruccion;

		for (int j = 0; j < idTags.Length; j++) {
			AnsiString id = idTags[j];
			Instruccion.sprintf("INSERT INTO articulosecomtagsasignados (id, articulo, fecha, hora, usuario) \
			VALUES (%s, '%s', CURDATE(), CURTIME(), '%s') ", id, articulo, usuario);
			instrucciones[num_instrucciones++]=Instruccion;
		}

		instrucciones[num_instrucciones++]="COMMIT";

		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		if(mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)){

			// Resultado final
			instruccion.sprintf("SELECT id FROM articulosecomtagsasignados a \
			WHERE a.articulo='%s' ORDER BY id", articulo);
			mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}

	} __finally {
		if(buffer_sql != NULL) delete buffer_sql;
	}
}
// ---------------------------------------------------------------------------
//ID_CON_TAGS_ARTICULO_ECOMMERCE
void ServidorEcommerce::ConsultaTagsArticulosEcommerce(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*30*10];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0, i;
	AnsiString Instruccion, instruccion, instrucciones[50];
	AnsiString articulo;

	articulo = mFg.ExtraeStringDeBuffer(&parametros);

	try{
		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

		instruccion.sprintf("SELECT id FROM articulosecomtagsasignados a \
		WHERE a.articulo='%s' ORDER BY id", articulo);
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	} __finally {
		if(buffer_sql != NULL) delete buffer_sql;
	}
}
// ---------------------------------------------------------------------------
// ID_GRA_ART_ECOM_TAGS
void ServidorEcommerce::GrabaArticulosEcomTags(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
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

		datos.AsignaTabla("articulosecommercetags");
		parametros+=datos.InsCamposDesdeBuffer(parametros);

			//nueva consulta del folio
		BufferRespuestas* resp_folio=NULL;
		//se creara un bufer para obtener el valor del parametro de precio minimo y posteriormete
		//liberar la memoria ocupada de esta consulta
		try{
			instruccion.sprintf("SELECT @folio:=if(max(id) IS NULL,'1',max(id) ) AS folio FROM articulosecommercetags");
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_folio);
			if (resp_folio->ObtieneNumRegistros()>0){
				folio=mFg.CadenaAFlotante(resp_folio->ObtieneDato("folio")); //valor de precio minimo
			}
		}__finally{
			if (resp_folio!=NULL) delete resp_folio;
		}


		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		if (tarea=="A") {
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

			instrucciones[num_instrucciones++]=datos.GenerarSqlUpdate("id=@idart");
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
// ID_CON_ART_ECOM_TAGS
void ServidorEcommerce::ConsultaArticulosEcomTags(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA
	AnsiString instruccion;
	AnsiString clave_art_tags;

	clave_art_tags = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos
	instruccion.sprintf("select * from articulosecommercetags where id='%s' ",clave_art_tags);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

	// Obtiene todos los datos   where idarticulostags='%s'
	instruccion.sprintf("select * from articulosecommercetags ");
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

	// Obtiene numero del ultimo registro
	instruccion =
		"select  max(id) as ultimo from articulosecommercetags";
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

}
// ---------------------------------------------------------------------------
// ID_BAJ_ART_ECOM_TAGS
void ServidorEcommerce::BajaArticulosEcomTags(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
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
			instruccion.sprintf("delete from articulosecommercetags where id='%s' ",
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
// ID_GRA_PARAMETRO_ECOM
void ServidorEcommerce::GrabaParametroEcom(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql = new char[1024 * 32];
	char *aux_buffer_sql = buffer_sql;
	AnsiString instrucciones[50], instruccion;
	int num_instrucciones = 0;
	AnsiString tarea, parametro, descripcion, valor, sucursal;
	int error = 0;
	int i;

	try {
		tarea 	 	= mFg.ExtraeStringDeBuffer(&parametros);
		parametro	= mFg.ExtraeStringDeBuffer(&parametros);
		descripcion = mFg.ExtraeStringDeBuffer(&parametros);
		valor 		= mFg.ExtraeStringDeBuffer(&parametros);
		sucursal 	= mFg.ExtraeStringDeBuffer(&parametros);

		instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++] = "START TRANSACTION";

		// Elimina el impuesto
		instruccion.sprintf("REPLACE INTO parametrosecommerce (parametro, descripcion, valor, sucursal) \
		VALUES \
		('%s', '%s', '%s', '%s') ",
		parametro, descripcion, valor, sucursal);
		instrucciones[num_instrucciones++] = instruccion;

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
// ID_BAJ_PARAMETRO_ECOM
void ServidorEcommerce::BajaParametroEcom(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	BajaGenerico(Respuesta, MySQL, parametros, "parametrosecommerce", "parametro");
}
// ---------------------------------------------------------------------------
// ID_CON_PARAMETRO_ECOM
void ServidorEcommerce::ConsultaParametroEcom(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
	AnsiString clave, indiciparametro, sucursal;
	AnsiString condicion_indiciparametro = " ";

	clave = mFg.ExtraeStringDeBuffer(&parametros);
	indiciparametro = mFg.ExtraeStringDeBuffer(&parametros);
	sucursal = mFg.ExtraeStringDeBuffer(&parametros);
	if (indiciparametro!=" ") {
	   condicion_indiciparametro.sprintf(" AND (Parametro LIKE '%%%s%%' OR Descripcion LIKE '%%%s%%' OR Valor LIKE '%%%s%%' OR Sucursal LIKE '%%%s%%') ", indiciparametro, indiciparametro, indiciparametro, indiciparametro);
	}
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos del parametro
	instruccion.sprintf("select * from parametrosecommerce where parametro='%s' AND sucursal='%s'", clave, sucursal);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
	// Obtiene todos los parametros
	instruccion.sprintf("select parametro AS Parametro, descripcion AS Descripcion, valor AS Valor, sucursal AS Sucursal from parametrosecommerce where 1 %s order by parametro", condicion_indiciparametro);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------
// ID_GRA_BANNERS
void ServidorEcommerce::GrabaBanners(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql = new char[1024 * 1024 * 64];
	char *aux_buffer_sql = buffer_sql;
	BufferRespuestas* resp_orden=NULL;
	AnsiString instrucciones[50], instruccion;
	int num_instrucciones = 0;
	AnsiString valida_orden;

	AnsiString sucursal, nombreimagen, dataimagen;
	AnsiString formatoimagen, descripcionimagen, descripcionart;
	AnsiString id, activo, activokisko, ruta;

	sucursal = mFg.ExtraeStringDeBuffer(&parametros);
	nombreimagen = mFg.ExtraeStringDeBuffer(&parametros);
	dataimagen = mFg.ExtraeStringDeBuffer(&parametros);
	formatoimagen = mFg.ExtraeStringDeBuffer(&parametros);
	descripcionimagen = mFg.ExtraeStringDeBuffer(&parametros);
	id = mFg.ExtraeStringDeBuffer(&parametros);
	activo = mFg.ExtraeStringDeBuffer(&parametros);
	activokisko = mFg.ExtraeStringDeBuffer(&parametros);
	ruta = mFg.ExtraeStringDeBuffer(&parametros);

	try {

		try{
			instruccion.sprintf("SELECT IFNULL(MAX(orden),'') AS orden FROM imagenesbanners WHERE sucursal = '%s'", sucursal);
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

			instruccion.sprintf("SELECT @orden:=MAX(orden) FROM imagenesbanners WHERE sucursal = '%s'",
			sucursal);
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("SET @ordensig:=@orden+1");
			instrucciones[num_instrucciones++]=instruccion;
		}

		if (id!=" ") {
			if(dataimagen != " ")  {
				instruccion.sprintf("UPDATE imagenesbanners \
					SET nombre = '%s', descripcion = '%s', activo = %s, esKiosko = %s, ruta = '%s', imagen ='%s', formato = '%s' WHERE sucursal = '%s' AND id = %s ",
					nombreimagen, descripcionimagen, activo, activokisko, ruta, dataimagen, formatoimagen,sucursal, id);
		} else {
			instruccion.sprintf("UPDATE imagenesbanners \
				SET nombre = '%s', descripcion = '%s', activo = %s, esKiosko = %s, ruta = '%s' WHERE sucursal = '%s' AND id = %s ",
				nombreimagen, descripcionimagen, activo, activokisko, ruta, sucursal, id);
				instrucciones[num_instrucciones++]=instruccion;
			}

		instrucciones[num_instrucciones++]=instruccion;
		} else {

			instruccion.sprintf("INSERT INTO imagenesbanners \
			(sucursal, nombre, imagen, formato, descripcion, orden, activo, esKiosko, ruta) \
			VALUES \
			('%s', '%s', '%s', '%s', '%s', @ordensig, %s, %s, '%s')",
			sucursal, nombreimagen, dataimagen, formatoimagen, descripcionimagen, activo, activokisko, ruta);
			instrucciones[num_instrucciones++]=instruccion;

		}

		instrucciones[num_instrucciones++]="COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql = mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (int i = 0; i < num_instrucciones; i++)
			aux_buffer_sql = mFg.AgregaStringABuffer(instrucciones[i],aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta,TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL,buffer_sql);

		instruccion = "SELECT LAST_INSERT_ID() as last";
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	}
	__finally {
		delete buffer_sql;
	}
}
// ---------------------------------------------------------------------------
// ID_CON_BANNERS
void ServidorEcommerce::ConsultaBanners(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
	AnsiString sucursal;

	sucursal = mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	instruccion.sprintf("SELECT id, nombre, formato, orden, sucursal, activo, esKiosko FROM imagenesbanners WHERE sucursal='%s' ORDER BY orden ASC ",
	sucursal);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

}
// ---------------------------------------------------------------------------
// ID_CON_BANNER
void ServidorEcommerce::ConsultaBanner(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
	AnsiString id;

	id = mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos del rol
	instruccion.sprintf("SELECT imagen, nombre, formato, descripcion, sucursal, activo, esKiosko FROM imagenesbanners WHERE id=%s ", id);

	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

}
// ---------------------------------------------------------------------------
// ID_BAJA_BANNER
void ServidorEcommerce::BajaBanner(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
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

		instruccion.sprintf("DELETE FROM imagenesbanners WHERE id = %s ",
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
// ID_REORDEN_BANNER
void ServidorEcommerce::ReordenarBanner(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql = new char[1024 * 1024 * 4];
	char *aux_buffer_sql = buffer_sql;
	BufferRespuestas* resp_imagenes=NULL;
	AnsiString instrucciones[50], instruccion;
	int num_instrucciones = 0;
	AnsiString valida_orden;

	AnsiString sucursal, id, orden, tipo;
	AnsiString id_mod;

	sucursal = mFg.ExtraeStringDeBuffer(&parametros);
	id = mFg.ExtraeStringDeBuffer(&parametros);
	orden = mFg.ExtraeStringDeBuffer(&parametros);
	tipo = mFg.ExtraeStringDeBuffer(&parametros);

	try {

		try{
			instruccion.sprintf("SELECT id  FROM imagenesbanners WHERE sucursal = '%s' AND orden = %s %s", sucursal, orden, tipo);
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

				instruccion.sprintf("UPDATE imagenesbanners SET orden = orden  + 1 WHERE sucursal = '%s' AND id = %s ",
				sucursal, id_mod);
				instrucciones[num_instrucciones++]=instruccion;

				instruccion.sprintf("UPDATE imagenesbanners SET orden = orden %s WHERE sucursal = '%s' AND id = %s ",
				tipo, sucursal, id);
				instrucciones[num_instrucciones++]=instruccion;

			} else {

				instruccion.sprintf("UPDATE imagenesbanners SET orden = orden -1 WHERE sucursal = '%s' AND id = %s ",
				sucursal, id_mod);
				instrucciones[num_instrucciones++]=instruccion;

				instruccion.sprintf("UPDATE imagenesbanners SET orden = orden %s WHERE sucursal = '%s' AND id = %s ",
				tipo, sucursal, id);
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
// ID_GRA_HORARIO_ENTREGA
void ServidorEcommerce::GrabaHorarioEntrega(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql = new char[1024 * 1024];
	char *aux_buffer_sql = buffer_sql;
	BufferRespuestas* resp_imagenes=NULL;
	AnsiString instrucciones[50], instruccion;
	int num_instrucciones = 0;

	AnsiString dia, sucursal, horaini, horafin, orden;

	dia = mFg.ExtraeStringDeBuffer(&parametros);
	sucursal = mFg.ExtraeStringDeBuffer(&parametros);
	horaini = mFg.ExtraeStringDeBuffer(&parametros);
	horafin = mFg.ExtraeStringDeBuffer(&parametros);
	orden = mFg.ExtraeStringDeBuffer(&parametros);

	try {

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		instruccion.sprintf("INSERT INTO horariosentrega (dia, sucursal, horaini, horafin, orden) \
		VALUES \
		('%s', '%s', '%s', '%s', '%s')",
		dia, sucursal, horaini, horafin, orden);
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
// ID_CON_HORARIO_ENTREGA
void ServidorEcommerce::ConsultaHorarioEntrega(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA
	AnsiString instruccion;
	AnsiString sucursal;

	sucursal = mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos
	instruccion.sprintf("SELECT \
	ho.clave, \
	ho.nombre, \
	CONCAT(he.horaini,' - ',he.horafin) AS horario \
	FROM horariosentrega he \
	INNER JOIN horariooperativo ho ON ho.clave = he.dia \
	WHERE sucursal = '%s' ORDER BY he.orden ASC, he.horaini ",sucursal);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------
// ID_BAJ_HORARIO_ENTREGA
void ServidorEcommerce::BajaHorarioEntrega(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql = new char[1024 * 1024];
	char *aux_buffer_sql = buffer_sql;
	BufferRespuestas* resp_imagenes=NULL;
	AnsiString instrucciones[50], instruccion;
	int num_instrucciones = 0;

	AnsiString dia, sucursal, horario;

	sucursal = mFg.ExtraeStringDeBuffer(&parametros);
	dia = mFg.ExtraeStringDeBuffer(&parametros);
	horario = mFg.ExtraeStringDeBuffer(&parametros);

	try {

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		instruccion.sprintf("DELETE FROM horariosentrega WHERE dia = '%s' \
		AND sucursal = '%s' AND CONCAT(horaini,' - ',horafin) = '%s' ",
		dia, sucursal, horario);
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
// ID_CON_IMAGEN_CLASIF
void ServidorEcommerce::ConsultaImagenClasif(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
	AnsiString clasif;
	int no_clasif;

	clasif = mFg.ExtraeStringDeBuffer(&parametros);
	no_clasif = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	instruccion.sprintf("SELECT imagen, nombre_imagen, formato FROM clasificacionecommerce%d WHERE clasif%d='%s' ",
	no_clasif, no_clasif, clasif);

	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

}
// ---------------------------------------------------------------------------
// ID_GRA_IMAGEN_CLASIF
void ServidorEcommerce::GrabaImagenClasif(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql = new char[1024 * 1024 * 4];
	char *aux_buffer_sql = buffer_sql;
	BufferRespuestas* resp_orden=NULL;
	AnsiString instrucciones[50], instruccion;
	int num_instrucciones = 0;

	AnsiString clasif, nombreimagen, dataimagen, formatoimagen;
	int no_clasif;

	clasif = mFg.ExtraeStringDeBuffer(&parametros);
	nombreimagen = mFg.ExtraeStringDeBuffer(&parametros);
	dataimagen = mFg.ExtraeStringDeBuffer(&parametros);
	formatoimagen = mFg.ExtraeStringDeBuffer(&parametros);
	no_clasif = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));

	try {


		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		instruccion.sprintf("UPDATE clasificacionecommerce%d \
		SET nombre_imagen = '%s', imagen = '%s', formato = '%s' WHERE clasif%d = '%s' ",
		no_clasif,
		nombreimagen, dataimagen, formatoimagen, no_clasif, clasif);
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
// ID_BAJA_IMAGEN_CLASIF
void ServidorEcommerce::BajaImagenClasif(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql = new char[1024 * 64];
	char *aux_buffer_sql = buffer_sql;
	AnsiString instrucciones[10], instruccion;
	int num_instrucciones = 0;

	AnsiString clasif;
	int no_clasif;

	clasif = mFg.ExtraeStringDeBuffer(&parametros);
	no_clasif = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));

	try {
		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		instruccion.sprintf("UPDATE clasificacionecommerce%d \
		SET nombre_imagen = '', imagen = '', formato = '' WHERE clasif%d = '%s' ",
		no_clasif, no_clasif, clasif);

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
//ID_CON_ORDENES_ECOM
void ServidorEcommerce::ConsultaOrdenesEcomm(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	instruccion.sprintf("SELECT \
	p.numorden, \
	p.referencia, \
	p.venta, \
	IF(p.tipocompra = 1, 'Envío a domicilio', 'Pick Up') AS tipocompra, \
	p.cliente, \
	CONCAT(cli.nombre,' ',cli.appat,' ',cli.apmat) AS nombre, \
	IF(p.tipocompra = 1, CONCAT(p.calle,', ',p.numext,IF(p.numint != '', CONCAT(', ',p.numint),''),', ',p.colonia,', ',p.codp,', ',p.ciudad,', ',est.descripcion), \
	CONCAT(pcfd.calleexp,', ',pcfd.numextexp ,IF(pcfd.numintexp != '', CONCAT(', ',pcfd.numintexp),''),', ',pcfd.coloniaexp,', ',pcfd.cpexp,', ',pcfd.localidadexp,', ',pcfd.estadoexp)) AS direccion, \
	p.fechaped, \
	p.fechasurt, \
	p.fechaentrega, \
	p.horaentrega, \
	p.valor AS total, \
	p.sucursal, \
	p.cupon, \
	p.descuento, \
	p.costoentrega, \
	p.referenciadom, \
	te.descripcion AS estadopedido, \
	ep.descripcion AS estadopago, \
	p.mensaje, \
	p.estadopedido \
	FROM pedidosecomm p \
	INNER JOIN clientes cli ON cli.cliente = p.cliente \
	INNER JOIN tipoestatus te ON te.id = p.estadopedido \
	INNER JOIN estatuspago ep ON ep.id = p.estadopago \
	LEFT JOIN cestados est ON est.c_estado = p.estado \
	LEFT JOIN parametroscfd pcfd ON pcfd.sucursal = p.sucursal \
	WHERE p.sucursal = '%s' \
	ORDER BY p.numorden DESC", FormServidor->ObtieneClaveSucursal());

	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------
//ID_CON_DETALLE_ORDEN_ECOM
void ServidorEcommerce::ConsultaDetalleOrdenesEcomm(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	BufferRespuestas* resp_pedido=NULL;
	AnsiString instruccion;
	AnsiString referencia;
	int estadopedido = 0;

	AnsiString campo_modificado;
	AnsiString campo_validado;
	AnsiString condicion_pedidos_detalle=" ";

	referencia = mFg.ExtraeStringDeBuffer(&parametros);

	try{
		instruccion.sprintf("SELECT estadopedido FROM pedidosecomm WHERE referencia = '%s'", referencia);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_pedido);
		if (resp_pedido->ObtieneNumRegistros()>0){
			estadopedido=StrToInt(resp_pedido->ObtieneDato("estadopedido"));
		}
	}__finally{
		if (resp_pedido!=NULL) delete resp_pedido;
	}

	if(estadopedido==1){
		campo_modificado="0";
		campo_validado="0";
		condicion_pedidos_detalle="INNER JOIN dpedidosecomm dp ON dp.referencia = p.referencia";
	} else {
	    campo_modificado="dp.modificado";
		campo_validado="1";
		condicion_pedidos_detalle="INNER JOIN dpedidosecommsurtido dp ON dp.referencia = p.referencia";
    }

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	instruccion.sprintf("SELECT \
    p.cliente, \
	p.referencia, \
	p.venta, \
	p.fechaalta, \
	p.numorden, \
	p.cancelado, \
	CONCAT(cli.nombre,' ',cli.appat,' ',cli.apmat) AS nombre, \
	IF(p.tipocompra = 1, CONCAT(p.calle,', ',p.numext,IF(p.numint != '', CONCAT(', ',p.numint),''),', ',p.colonia,', ',p.codp,', ',p.ciudad,', ',ce.descripcion), \
	CONCAT(pcfd.calleexp,', ',pcfd.numextexp,IF(pcfd.numintexp != '', CONCAT(', ',pcfd.numintexp),''),', ',pcfd.coloniaexp,', ',pcfd.cpexp,', ',pcfd.municipioexp,', ',pcfd.estadoexp)) AS domicilio, \
	p.tipocompra, \
	IF(p.tipocompra = 1, 'Envío a domicilio', 'Pick up') AS descripcioncompra, \
	p.fechaentrega, \
	p.horaentrega, \
	p.costoentrega, \
	te.descripcion AS estadopedido, \
	ep.descripcion AS estadopago, \
	(p.valor + p.costoentrega) AS total, \
	p.mensaje, \
	p.estadopedido AS estatus, \
	cemp.vendedor, \
	cemp.cobrador, \
	IF(p.tipocompra = 1, p.calle, pcfd.calleexp) AS calle, \
	IF(p.tipocompra = 1, p.numext, pcfd.numextexp) AS numext, \
	IF(p.tipocompra = 1, p.numint, pcfd.numintexp) AS numint, \
	IF(p.tipocompra = 1, p.codp, pcfd.cpexp) AS codpostal, \
	IF(p.tipocompra = 1, X(p.ubicaciongis), X(suc.ubicaciongis)) AS latitud, \
	IF(p.tipocompra = 1, Y(p.ubicaciongis), Y(suc.ubicaciongis)) AS longitud, \
	IF(p.tipocompra = 1, p.referenciadom, pcfd.referenexp) AS ref, \
	p.corte, \
    p.fechaalta \
	FROM pedidosecomm p \
	INNER JOIN clientes cli ON cli.cliente = p.cliente \
	INNER JOIN tipoestatus te ON te.id = p.estadopedido \
	INNER JOIN estatuspago ep ON ep.id = p.estadopago \
	INNER JOIN sucursales suc ON suc.sucursal = p.sucursal \
	LEFT JOIN cestados ce ON ce.c_estado = p.estado \
	LEFT JOIN parametroscfd pcfd ON pcfd.sucursal = p.sucursal \
	LEFT JOIN clientesemp cemp ON cli.cliente = cemp.cliente \
	WHERE p.referencia = '%s'", referencia);

	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	instruccion.sprintf("SELECT \
	a.articulo, \
	prod.nombre, \
	a.producto, \
	a.present, \
	a.multiplo, \
    a.factor, \
	dp.cantidad, \
	dp.precio, \
	dp.precioimp, \
	dp.costobase, \
	SUM(dp.cantidad * dp.precioimp) AS total, \
	dp.claveimp1, \
	i1.tipoimpu, \
	i1.porcentaje, \
	dp.claveimp2, \
	i2.tipoimpu, \
	i2.porcentaje, \
	dp.claveimp3, \
	i3.tipoimpu, \
	i3.porcentaje, \
	dp.claveimp4, \
	i4.tipoimpu, \
	i4.porcentaje, \
	dp.id, \
	%s AS modificado, \
	(IF(GROUP_CONCAT(cb.codigobarras) IS NULL,a.ean13, CONCAT(GROUP_CONCAT(cb.codigobarras),',',a.ean13))) AS codigobarras, \
	%s AS validado \
	FROM pedidosecomm p \
	%s \
	INNER JOIN articulos a ON a.articulo = dp.articulo \
	INNER JOIN productos prod ON prod.producto = a.producto \
	LEFT JOIN impuestos i1 on i1.impuesto = prod.claveimpv1 \
	LEFT JOIN impuestos i2 on i2.impuesto = prod.claveimpV2 \
	LEFT JOIN impuestos i3 ON i3.impuesto = prod.claveimpV3 \
	LEFT JOIN impuestos i4 ON i4.impuesto = prod.claveimpV4 \
	LEFT JOIN codigosbarras cb ON cb.articulo = a.articulo \
	WHERE p.referencia = '%s' \
	GROUP BY a.articulo \
	ORDER BY prod.nombre",
	campo_modificado, campo_validado, condicion_pedidos_detalle, referencia);

	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	instruccion.sprintf("SELECT \
	dfp.formapag, \
	fp.descripcion, \
	dfp.valor, \
	dfp.porcentaje, \
	dfp.trn_id \
	FROM dpedidosfpago dfp \
	INNER JOIN formasdepago fp ON fp.formapag = dfp.formapag \
	WHERE referencia = '%s'", referencia);

	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}
// ---------------------------------------------------------------------------
//ID_GRA_DET_ORDEN_ECOM_SURTIDA
void ServidorEcommerce::GrabaDetPedidoSurtidoEcomm(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql = new char[1024 * 1024 * 1024];
	char *aux_buffer_sql = buffer_sql;
	AnsiString instrucciones[1000], instruccion;
	int num_instrucciones = 0;
	DatosTabla datos_detalle(mServidorVioleta->Tablas);

	AnsiString referencia, corte;
    int num_partidas = 0;

	referencia = mFg.ExtraeStringDeBuffer(&parametros);
	num_partidas=StrToInt(mFg.ExtraeStringDeBuffer(&parametros)); // Obtiene el número de partidas
	corte = mFg.ExtraeStringDeBuffer(&parametros);

	try {
		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		for (int i=0; i<num_partidas; i++) {
			datos_detalle.AsignaTabla("dpedidosecommsurtido");
			parametros+=datos_detalle.InsCamposDesdeBuffer(parametros);

			instrucciones[num_instrucciones++] = datos_detalle.GenerarSqlInsert();
		}

		instruccion.sprintf("UPDATE pedidosecomm \
		SET fechasurt = CURDATE(), fechamodi = CURDATE(), \
		horamodi = CURTIME(), horasurt = CURTIME(), corte = '%s' \
		WHERE referencia = '%s' ",
		corte, referencia);
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
//ID_UPDT_ESTATUS_PEDIDO_ECOMM
void ServidorEcommerce::EjecutaActualizaEstatusPedidoEcomm(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*64];
	char *aux_buffer_sql=buffer_sql;
	int num_partidas, i;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[10];
	AnsiString Instruccion;

	AnsiString referencia;
	int estatus;

	referencia = mFg.ExtraeStringDeBuffer(&parametros);
	estatus = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));

	try {

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		instruccion.sprintf("UPDATE pedidosecomm SET estadopedido = %d WHERE referencia = '%s' ",
		estatus, referencia);
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
//ID_UPDT_PEDIDO_ECOMM
void ServidorEcommerce::EjecutaActualizaVentaPedidoEcomm(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*64];
	char *aux_buffer_sql=buffer_sql;
	int num_partidas, i;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[10];
	AnsiString Instruccion;

	AnsiString referencia, venta;

	referencia = mFg.ExtraeStringDeBuffer(&parametros);
	venta = mFg.ExtraeStringDeBuffer(&parametros);

	try {

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		instruccion.sprintf("UPDATE pedidosecomm SET fechamodi = CURDATE(), horamodi = CURDATE(), \
		venta = '%s' WHERE referencia = '%s' ",
		venta, referencia);
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
