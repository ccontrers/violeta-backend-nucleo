#include <vcl.h>
//---------------------------------------------------------------------------
#include "pch.h"

//---------------------------------------------------------------------------


#pragma hdrstop

#include <DateUtils.hpp>
#include "ClassIteradorCostos.h"
#include "ClassServidorAdminSistema.h"
#include "ClassServidorVioleta.h"
#include "ClassBufferRespuestas.h"
#include "violetaS.h"
#include "FormServidorVioleta.h"
#include "comunes.h"
#include <IdFTP.hpp>
#include <vector>

//---------------------------------------------------------------------------

#pragma package(smart_init)


//---------------------------------------------------------------------------
// ID_CALC_COSTOSPROM_AL_DIA
void ServidorAdminSistema::CalculaCostosPromedioAlDia(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_inventario_inicial=new char[1024*64*10];
	char *aux_buffer_inventario_inicial=buffer_inventario_inicial;
	TDate fecha_costo;
	AnsiString fecha_costo_cad;
	char *resultado_select;
	int NumeroRenglones;
	AnsiString archivo_temp1="",SucursalOrigen;
	TFileStream *FStream1=NULL;
	AnsiString nomproducto, presentacion, cveproducto, multiplo, articulo, cadena, cantidad_cad, costo_cad, costounit_cad;
	double cantidad, costo, costounit;
	AnsiString idEmpresa;

	// Busca la ultima fecha donde hay precalculo de costos
	BufferRespuestas* resp_estado=NULL;
	AnsiString instruccion;

	SucursalOrigen=FormServidor->ObtieneClaveSucursal();

	try {

		idEmpresa = mFg.ExtraeStringDeBuffer(&parametros);

		instruccion.sprintf("select valor from estadosistemaemp where estado='FPRECCOSTOS' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
		if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_estado)) {
			if (resp_estado->ObtieneNumRegistros()>0){
				fecha_costo=mFg.MySqlDateToTDate(resp_estado->ObtieneDato("valor").c_str());
				fecha_costo_cad=DateToStr(fecha_costo);
			} else throw (Exception("No se encuentra registro FPRECCOSTOS en tabla estadosistemaemp"));
		} else throw (Exception("Error al consultar en tabla estadosistemaemp"));
	} __finally {
		if (resp_estado!=NULL) delete resp_estado;
	}


	try {
		//Inventario inicial
		aux_buffer_inventario_inicial=mFg.AgregaStringABuffer(fecha_costo_cad, aux_buffer_inventario_inicial);
		aux_buffer_inventario_inicial=mFg.AgregaStringABuffer(" ", aux_buffer_inventario_inicial); // sucursal
		aux_buffer_inventario_inicial=mFg.AgregaStringABuffer(" ", aux_buffer_inventario_inicial); // almacen
		aux_buffer_inventario_inicial=mFg.AgregaStringABuffer(" ", aux_buffer_inventario_inicial); // clasif1
		aux_buffer_inventario_inicial=mFg.AgregaStringABuffer(" ", aux_buffer_inventario_inicial); // clasif2
		aux_buffer_inventario_inicial=mFg.AgregaStringABuffer(" ", aux_buffer_inventario_inicial); // clasif3
		aux_buffer_inventario_inicial=mFg.AgregaStringABuffer(" ", aux_buffer_inventario_inicial); // producto
		aux_buffer_inventario_inicial=mFg.AgregaStringABuffer(" ", aux_buffer_inventario_inicial); // presentacion
		aux_buffer_inventario_inicial=mFg.AgregaStringABuffer("0", aux_buffer_inventario_inicial); // No incluir columna de IVA
		aux_buffer_inventario_inicial=mFg.AgregaStringABuffer("0", aux_buffer_inventario_inicial); // costoexistencia
		aux_buffer_inventario_inicial=mFg.AgregaStringABuffer(" ", aux_buffer_inventario_inicial); // Envia el segmento
		aux_buffer_inventario_inicial=mFg.AgregaStringABuffer(" ", aux_buffer_inventario_inicial); // Envia el fabricante
		aux_buffer_inventario_inicial=mFg.AgregaStringABuffer(" ", aux_buffer_inventario_inicial); // Envia la marca
		aux_buffer_inventario_inicial = mFg.AgregaStringABuffer(idEmpresa, aux_buffer_inventario_inicial); //Envia empresa
		// ID_EJE_REPCOSTOEXISTENCIAS
		mServidorVioleta->Reportes->EjecutaRepCostoExistencias(Respuesta,  MySQL, buffer_inventario_inicial);

		resultado_select=Respuesta->BufferResultado;
		resultado_select+=sizeof(int); // Se salta el tamaño del resultado.
		mFg.ExtraeStringDeBuffer(&resultado_select); // Se salta el indicador de error
		resultado_select+=sizeof(int); // Se salta el tamaño del buffer
		NumeroRenglones=*((int *)resultado_select); // Obtiene número de registros
		resultado_select+=sizeof(int); // Se salta el numero de registros
		archivo_temp1=mServidorVioleta->ObtieneArchivoTemp(1, Respuesta->Id);
		FStream1 = new TFileStream(archivo_temp1, fmCreate | fmShareDenyNone);
		//Llena los datos del inventario inicial
		for(int i=0; i<NumeroRenglones; i++) {
			nomproducto=mFg.ExtraeStringDeBuffer(&resultado_select);
			presentacion=mFg.ExtraeStringDeBuffer(&resultado_select);
			cveproducto=mFg.ExtraeStringDeBuffer(&resultado_select);
			cantidad=mFg.CadenaAFlotante(mFg.ExtraeStringDeBuffer(&resultado_select));
			costo=mFg.CadenaAFlotante(mFg.ExtraeStringDeBuffer(&resultado_select));
			costounit=mFg.CadenaAFlotante(mFg.ExtraeStringDeBuffer(&resultado_select));

			cantidad_cad=mFg.FormateaCantidad(cantidad, 3, false);
			costo_cad=mFg.FormateaCantidad(costo, 6, false);
			costounit_cad=mFg.FormateaCantidad(costounit, 6, false);

			//Guardar en archivo temporal
			cadena=cveproducto +"\t"+
				presentacion +"\t"+
				cantidad_cad +"\t"+
				costo_cad +"\t"+
				costounit_cad +"\n";
			FStream1->Write(cadena.c_str(), cadena.Length());
		}

		instruccion="SET SESSION sql_log_bin = 0";
		mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str());

		instruccion.sprintf("truncate table precalculocostospromedio%s", idEmpresa);
		if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
			throw (Exception("Error CalculaCostosPromedioAlDia en TRUNCATE"));

		instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE precalculocostospromedio%s (producto, present, cantidad, costo, costounit) ",
		archivo_temp1, idEmpresa);
		if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
			throw (Exception("Error CalculaCostosPromedioAlDia en LOAD DATA"));

		instruccion="SET SESSION sql_log_bin = 1";
		mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str());
	} __finally {
		if (FStream1!=NULL) {
			delete FStream1;
			mServidorVioleta->BorraArchivoTemp(archivo_temp1);
		}
		delete buffer_inventario_inicial;

	}

}

//---------------------------------------------------------------------------
// ID_CALC_VENTAS_MENSUALES
void ServidorAdminSistema::CalculaVentasMensuales(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*30];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instruccion, instrucciones[30];
	int num_instrucciones=0;
	char *buffer_sql_vxmnr=new char[1024*30];
	char *aux_buffer_sql_vxmnr=buffer_sql_vxmnr;
	AnsiString instruccion_vxmnr, instrucciones_vxmnr[30];
	int num_instrucciones_vxmnr=0;
	int i;
	TDate fecha_hoy=Today();
	int anio_actual=YearOf(fecha_hoy);
	int anio_anterior=anio_actual-1;
	int anio_ante_anterior=anio_actual-2;
	AnsiString cad_anio_actual=mFg.IntToAnsiString(anio_actual);
	AnsiString cad_anio_anterior=mFg.IntToAnsiString(anio_anterior);
	AnsiString cad_anio_ante_anterior=mFg.IntToAnsiString(anio_ante_anterior);
	AnsiString cad_fecha_hoy=mFg.DateToMySqlDate(fecha_hoy);
	//AnsiString cad_fecha_inicio=cad_anio_anterior+"-01-01";
	AnsiString cad_fecha_inicio=cad_anio_ante_anterior+"-01-01";
	AnsiString archivo_temp,SucursalOrigen;
    AnsiString archivo_temp_vxmnr;

	SucursalOrigen=FormServidor->ObtieneClaveSucursal();

	try {

			instruccion.sprintf("INSERT INTO bitacoraservidor VALUES (NULL,'%s',NOW(),'INICIO_ID_CALC_VENTAS_MENSUALES')",SucursalOrigen);
			instrucciones[num_instrucciones++]=instruccion;

			instruccion="SET SESSION sql_log_bin = 0";
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("SET AUTOCOMMIT=0");
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("START TRANSACTION");
			instrucciones[num_instrucciones++]=instruccion;
			instruccion.sprintf("select @fechacalcvtasmens:=cast(e.valor as datetime), @fechahoy:=CURDATE() from estadosistemaemp e where e.estado='FUCALVTASMEN' AND e.sucursal = '%s' for update", FormServidor->ObtieneClaveSucursal());
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("select @fechacorte:=ifnull(max(p.fecha),'1900-01-01') as fcorte \
				 from puntoscorte p where p.fecha<='%s'", mFg.DateToMySqlDate(fecha_hoy));
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("truncate table ventasxmes");
			instrucciones[num_instrucciones++]=instruccion;

			archivo_temp=mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);

			instruccion.sprintf(
				"SELECT almacen, producto, present, \
					 SUM(cant1) AS cant1, \
					 SUM(cant2) AS cant2, \
					 SUM(cant3) AS cant3, \
					 SUM(cant4) AS cant4, \
					 SUM(cant5) AS cant5, \
					 SUM(cant6) AS cant6, \
					 SUM(cant7) AS cant7, \
					 SUM(cant8) AS cant8, \
					 SUM(cant9) AS cant9, \
					 SUM(cant10) AS cant10, \
					 SUM(cant11) AS cant11, \
					 SUM(cant12) AS cant12, \
					 SUM(cant13) AS cant13, \
					 SUM(cant14) AS cant14, \
					 SUM(cant15) AS cant15, \
					 SUM(cant16) AS cant16, \
					 SUM(cant17) AS cant17, \
					 SUM(cant18) AS cant18, \
					 SUM(cant19) AS cant19, \
					 SUM(cant20) AS cant20, \
					 SUM(cant21) AS cant21, \
					 SUM(cant22) AS cant22, \
					 SUM(cant23) AS cant23, \
					 SUM(cant24) AS cant24, \
					 SUM(cant25) AS cant25, \
					 SUM(cant26) AS cant26, \
					 SUM(cant27) AS cant27, \
					 SUM(cant28) AS cant28, \
					 SUM(cant29) AS cant29, \
					 SUM(cant30) AS cant30, \
					 SUM(cant31) AS cant31, \
					 SUM(cant32) AS cant32, \
					 SUM(cant33) AS cant33, \
					 SUM(cant34) AS cant34, \
					 SUM(cant35) AS cant35, \
					 SUM(cant36) AS cant36, \
					 SUM(ventas30) AS ventas30, \
					 SUM(ventas60) AS ventas60, \
					 SUM(ventas90) AS ventas90, \
					 SUM(ventas180) AS ventas180, \
					 SUM(ventascorte) AS ventascorte \
				FROM \
				((SELECT al.almacen AS almacen, a.producto, a.present, \
					0.000 AS cant1, 0.000 AS cant2, 0.000 AS cant3, 0.000 AS cant4, 0.000  AS cant5, 0.000  AS cant6, \
					0.000 AS cant7, 0.000 AS cant8, 0.000 AS cant9, 0.000 AS cant10, 0.000  AS cant11, 0.000  AS cant12, \
					0.000  AS cant13, 0.000  AS cant14, 0.000  AS cant15, 0.000  AS cant16, 0.000  AS cant17, 0.000 AS cant18, \
					0.000 AS cant19, 0.000  AS cant20, 0.000  AS cant21, 0.000  AS cant22, 0.000  AS cant23, 0.000  AS cant24, \
					0.000  AS cant25, 0.000 AS cant26, 0.000  AS cant27, 0.000 AS cant28, 0.000  AS cant29, 0.000  AS cant30,  \
					0.000  AS cant31, 0.000  AS cant32, 0.000  AS cant33, 0.000  AS cant34,	0.000  AS cant35,0.000 AS cant36, \
					0.000  AS ventas30, 0.000  AS ventas60, 0.000  AS ventas90, 0.000  AS ventas180, 0.000  AS ventascorte  \
				FROM articulos a INNER JOIN presentaciones p ON p.producto = a.producto AND p.present = a.present			\
				INNER JOIN almacenes al  GROUP BY a.producto, a.present, al.almacen)    \
				UNION ALL \
				 (SELECT dv.almacen AS almacen, a.producto, a.present, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=1, dv.cantidad * a.factor, 0)) AS cant1, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=2, dv.cantidad * a.factor, 0)) AS cant2,  \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=3, dv.cantidad * a.factor, 0)) AS cant3, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=4, dv.cantidad * a.factor, 0)) AS cant4, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=5, dv.cantidad * a.factor, 0)) AS cant5, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=6, dv.cantidad * a.factor, 0)) AS cant6, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=7, dv.cantidad * a.factor, 0)) AS cant7, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=8, dv.cantidad * a.factor, 0)) AS cant8, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=9, dv.cantidad * a.factor, 0)) AS cant9, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=10, dv.cantidad * a.factor, 0)) AS cant10,\
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=11, dv.cantidad * a.factor, 0)) AS cant11,\
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=12, dv.cantidad * a.factor, 0)) AS cant12,\
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=1, dv.cantidad * a.factor, 0)) AS cant13, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=2, dv.cantidad * a.factor, 0)) AS cant14, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=3, dv.cantidad * a.factor, 0)) AS cant15, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=4, dv.cantidad * a.factor, 0)) AS cant16, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=5, dv.cantidad * a.factor, 0)) AS cant17, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=6, dv.cantidad * a.factor, 0)) AS cant18, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=7, dv.cantidad * a.factor, 0)) AS cant19, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=8, dv.cantidad * a.factor, 0)) AS cant20, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=9, dv.cantidad * a.factor, 0)) AS cant21, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=10, dv.cantidad * a.factor, 0)) AS cant22,\
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=11, dv.cantidad * a.factor, 0)) AS cant23,\
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=12, dv.cantidad * a.factor, 0)) AS cant24,\
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=1, dv.cantidad * a.factor, 0)) AS cant25, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=2, dv.cantidad * a.factor, 0)) AS cant26, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=3, dv.cantidad * a.factor, 0)) AS cant27, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=4, dv.cantidad * a.factor, 0)) AS cant28, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=5, dv.cantidad * a.factor, 0)) AS cant29, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=6, dv.cantidad * a.factor, 0)) AS cant30, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=7, dv.cantidad * a.factor, 0)) AS cant31, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=8, dv.cantidad * a.factor, 0)) AS cant32, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=9, dv.cantidad * a.factor, 0)) AS cant33, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=10, dv.cantidad * a.factor, 0)) AS cant34,\
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=11, dv.cantidad * a.factor, 0)) AS cant35,\
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=12, dv.cantidad * a.factor, 0)) AS cant36,\
					SUM(IF(v.fechavta>=DATE_SUB('%s', INTERVAL 30 DAY) AND v.fechavta<='%s', dv.cantidad * a.factor, 0)) AS ventas30, \
					SUM(IF(v.fechavta>=DATE_SUB('%s', INTERVAL 60 DAY) AND v.fechavta<='%s', dv.cantidad * a.factor, 0)) AS ventas60, \
					SUM(IF(v.fechavta>=DATE_SUB('%s', INTERVAL 90 DAY) AND v.fechavta<='%s', dv.cantidad * a.factor, 0)) AS ventas90, \
					SUM(IF(v.fechavta>=DATE_SUB('%s', INTERVAL 180 DAY) AND v.fechavta<='%s', dv.cantidad * a.factor, 0)) AS ventas180, \
					SUM(IF(v.fechavta>@fechacorte AND v.fechavta<='%s', dv.cantidad * a.factor, 0)) AS ventascorte \
				  FROM ventas v FORCE INDEX(fechavta) \
					INNER JOIN dventas dv ON v.referencia = dv.referencia \
					INNER JOIN articulos a ON dv.articulo=a.articulo \
				  WHERE v.fechavta between '%s' AND '%s' \
					AND v.cancelado = 0 \
					GROUP BY dv.almacen, a.producto, a.present) \
				UNION ALL \
				(SELECT dv.almacen AS almacen, a.producto, a.present, \
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=1, dn.cantidad * a.factor*-1, 0)) AS cant1,\
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=2, dn.cantidad * a.factor*-1, 0)) AS cant2,\
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=3, dn.cantidad * a.factor*-1, 0)) AS cant3,\
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=4, dn.cantidad * a.factor*-1, 0)) AS cant4,\
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=5, dn.cantidad * a.factor*-1, 0)) AS cant5,\
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=6, dn.cantidad * a.factor*-1, 0)) AS cant6,\
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=7, dn.cantidad * a.factor*-1, 0)) AS cant7,\
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=8, dn.cantidad * a.factor*-1, 0)) AS cant8,\
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=9, dn.cantidad * a.factor*-1, 0)) AS cant9, \
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=10, dn.cantidad * a.factor*-1, 0)) AS cant10,\
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=11, dn.cantidad * a.factor*-1, 0)) AS cant11, \
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=12, dn.cantidad * a.factor*-1, 0)) AS cant12,  \
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=1, dn.cantidad * a.factor*-1, 0)) AS cant13, \
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=2, dn.cantidad * a.factor*-1, 0)) AS cant14, \
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=3, dn.cantidad * a.factor*-1, 0)) AS cant15, \
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=4, dn.cantidad * a.factor*-1, 0)) AS cant16, \
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=5, dn.cantidad * a.factor*-1, 0)) AS cant17, \
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=6, dn.cantidad * a.factor*-1, 0)) AS cant18, \
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=7, dn.cantidad * a.factor*-1, 0)) AS cant19, \
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=8, dn.cantidad * a.factor*-1, 0)) AS cant20, \
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=9, dn.cantidad * a.factor*-1, 0)) AS cant21, \
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=10, dn.cantidad * a.factor*-1, 0)) AS cant22,\
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=11, dn.cantidad * a.factor*-1, 0)) AS cant23,\
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=12, dn.cantidad * a.factor*-1, 0)) AS cant24,\
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=1, dn.cantidad * a.factor*-1, 0)) AS cant25, \
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=2, dn.cantidad * a.factor*-1, 0)) AS cant26, \
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=3, dn.cantidad * a.factor*-1, 0)) AS cant27, \
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=4, dn.cantidad * a.factor*-1, 0)) AS cant28, \
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=5, dn.cantidad * a.factor*-1, 0)) AS cant29, \
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=6, dn.cantidad * a.factor*-1, 0)) AS cant30, \
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=7, dn.cantidad * a.factor*-1, 0)) AS cant31, \
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=8, dn.cantidad * a.factor*-1, 0)) AS cant32, \
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=9, dn.cantidad * a.factor*-1, 0)) AS cant33, \
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=10, dn.cantidad * a.factor*-1, 0)) AS cant34,\
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=11, dn.cantidad * a.factor*-1, 0)) AS cant35,\
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=12, dn.cantidad * a.factor*-1, 0)) AS cant36,\
					SUM(IF(n.fechanot>=DATE_SUB('%s', INTERVAL 30 DAY) AND n.fechanot<='%s', dn.cantidad * a.factor*-1, 0)) AS ventas30, \
					SUM(IF(n.fechanot>=DATE_SUB('%s', INTERVAL 60 DAY) AND n.fechanot<='%s', dn.cantidad * a.factor*-1, 0)) AS ventas60, \
					SUM(IF(n.fechanot>=DATE_SUB('%s', INTERVAL 90 DAY) AND n.fechanot<='%s', dn.cantidad * a.factor*-1, 0)) AS ventas90, \
					SUM(IF(n.fechanot>=DATE_SUB('%s', INTERVAL 180 DAY) AND n.fechanot<='%s', dn.cantidad * a.factor*-1, 0)) AS ventas180, \
					0 AS ventascorte \
				  FROM notascredcli n \
					INNER JOIN dnotascredcli dn ON  n.referencia = dn.referencia \
					INNER JOIN ventas v ON  n.venta = v.referencia \
					INNER JOIN dventas dv ON v.referencia = dv.referencia AND dv.articulo=dn.articulo \
					INNER JOIN articulos a ON dv.articulo=a.articulo \
				  WHERE n.fechanot between '%s' AND '%s' \
					AND n.cancelado = 0 AND n.tipo = '0' \
					GROUP BY dv.almacen, a.producto, a.present )) t \
				GROUP BY almacen, producto, present INTO OUTFILE '%s' ",
					cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,
					cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,
					cad_anio_anterior,cad_anio_anterior, cad_anio_anterior, cad_anio_anterior, cad_anio_anterior, cad_anio_anterior,
					cad_anio_anterior,cad_anio_anterior, cad_anio_anterior, cad_anio_anterior, cad_anio_anterior, cad_anio_anterior,
					cad_anio_actual, cad_anio_actual,cad_anio_actual,cad_anio_actual,cad_anio_actual,cad_anio_actual,
					cad_anio_actual, cad_anio_actual,cad_anio_actual,cad_anio_actual,cad_anio_actual,cad_anio_actual,
					cad_fecha_hoy,cad_fecha_hoy,cad_fecha_hoy,cad_fecha_hoy,cad_fecha_hoy,cad_fecha_hoy,
					cad_fecha_hoy, cad_fecha_hoy, cad_fecha_hoy,
					cad_fecha_inicio,
					cad_fecha_hoy,
					cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,
					cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,
					cad_anio_anterior, cad_anio_anterior, cad_anio_anterior, cad_anio_anterior, cad_anio_anterior, cad_anio_anterior,
					cad_anio_anterior, cad_anio_anterior, cad_anio_anterior, cad_anio_anterior, cad_anio_anterior, cad_anio_anterior,
					cad_anio_actual, cad_anio_actual,cad_anio_actual,cad_anio_actual,cad_anio_actual,cad_anio_actual,
					cad_anio_actual, cad_anio_actual,cad_anio_actual,cad_anio_actual,cad_anio_actual,cad_anio_actual,
					cad_fecha_hoy, cad_fecha_hoy,cad_fecha_hoy, cad_fecha_hoy,cad_fecha_hoy,
					cad_fecha_hoy,cad_fecha_hoy, cad_fecha_hoy,
					cad_fecha_inicio,
					cad_fecha_hoy,
					archivo_temp );
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE ventasxmes (almacen,producto,present, \
					 cant1,cant2,cant3,cant4,cant5, cant6,cant7,cant8,cant9,cant10, \
					 cant11,cant12,cant13,cant14,cant15, cant16,cant17,cant18,cant19, \
					 cant20, cant21,cant22,cant23,cant24,  \
					 cant25,cant26,cant27, cant28,cant29,cant30,cant31, cant32, cant33,cant34,cant35,cant36, \
					 ventas30, ventas60, ventas90, ventas180,ventascorte) ",archivo_temp);
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("update estadosistemaemp e set e.valor=@fechahoy \
				where  e.estado='FUCALVTASMEN' AND e.sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("COMMIT");
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("SET AUTOCOMMIT=1");
			instrucciones[num_instrucciones++]=instruccion;

			instruccion="SET SESSION sql_log_bin = 1";
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("INSERT INTO bitacoraservidor VALUES (NULL,'%s',NOW(),'FIN_ID_CALC_VENTAS_MENSUALES')",SucursalOrigen);
			instrucciones[num_instrucciones++]=instruccion;

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);
	} __finally {
		mServidorVioleta->BorraArchivoTemp(archivo_temp);
		delete buffer_sql;
	}

	try {

		instruccion_vxmnr.sprintf("INSERT INTO bitacoraservidor VALUES (NULL,'%s',NOW(),'INICIO_ID_CALC_VENTAS_MENSUALES_NOREL')",SucursalOrigen);
		instrucciones_vxmnr[num_instrucciones_vxmnr++]=instruccion_vxmnr;

		instruccion_vxmnr="SET SESSION sql_log_bin = 0";
		instrucciones_vxmnr[num_instrucciones_vxmnr++]=instruccion_vxmnr;

		instruccion_vxmnr.sprintf("SET AUTOCOMMIT=0");
		instrucciones_vxmnr[num_instrucciones_vxmnr++]=instruccion_vxmnr;
		instruccion_vxmnr.sprintf("START TRANSACTION");
		instrucciones_vxmnr[num_instrucciones_vxmnr++]=instruccion_vxmnr;
		instruccion_vxmnr.sprintf("select @fechacalcvtasmens:=cast(e.valor as datetime), @fechahoy:=CURDATE() from estadosistemaemp e where e.estado='FUCALVTASMEN' AND e.sucursal = '%s' for update", FormServidor->ObtieneClaveSucursal());
		instrucciones_vxmnr[num_instrucciones_vxmnr++]=instruccion_vxmnr;

		instruccion_vxmnr.sprintf("select @fechacorte:=ifnull(max(p.fecha),'1900-01-01') as fcorte \
		from puntoscorte p where p.fecha<='%s'", mFg.DateToMySqlDate(fecha_hoy));
		instrucciones_vxmnr[num_instrucciones_vxmnr++]=instruccion_vxmnr;

		instruccion_vxmnr.sprintf("truncate table ventasxmes_norel");
		instrucciones_vxmnr[num_instrucciones_vxmnr++]=instruccion_vxmnr;

        archivo_temp_vxmnr=mServidorVioleta->ObtieneArchivoTemp(num_instrucciones_vxmnr, Respuesta->Id);

		instruccion_vxmnr.sprintf(
		"SELECT almacen, producto, present, \
			 SUM(cant1) AS cant1, \
			 SUM(cant2) AS cant2, \
			 SUM(cant3) AS cant3, \
			 SUM(cant4) AS cant4, \
			 SUM(cant5) AS cant5, \
			 SUM(cant6) AS cant6, \
			 SUM(cant7) AS cant7, \
			 SUM(cant8) AS cant8, \
			 SUM(cant9) AS cant9, \
			 SUM(cant10) AS cant10, \
			 SUM(cant11) AS cant11, \
			 SUM(cant12) AS cant12, \
			 SUM(cant13) AS cant13, \
			 SUM(cant14) AS cant14, \
			 SUM(cant15) AS cant15, \
			 SUM(cant16) AS cant16, \
			 SUM(cant17) AS cant17, \
			 SUM(cant18) AS cant18, \
			 SUM(cant19) AS cant19, \
			 SUM(cant20) AS cant20, \
			 SUM(cant21) AS cant21, \
			 SUM(cant22) AS cant22, \
			 SUM(cant23) AS cant23, \
			 SUM(cant24) AS cant24, \
			 SUM(cant25) AS cant25, \
			 SUM(cant26) AS cant26, \
			 SUM(cant27) AS cant27, \
			 SUM(cant28) AS cant28, \
			 SUM(cant29) AS cant29, \
			 SUM(cant30) AS cant30, \
			 SUM(cant31) AS cant31, \
			 SUM(cant32) AS cant32, \
			 SUM(cant33) AS cant33, \
			 SUM(cant34) AS cant34, \
			 SUM(cant35) AS cant35, \
			 SUM(cant36) AS cant36, \
			 SUM(ventas30) AS ventas30, \
			 SUM(ventas60) AS ventas60, \
			 SUM(ventas90) AS ventas90, \
			 SUM(ventas180) AS ventas180, \
			 SUM(ventascorte) AS ventascorte \
				FROM \
				((SELECT al.almacen AS almacen, a.producto, a.present, \
					0.000 AS cant1, 0.000 AS cant2, 0.000 AS cant3, 0.000 AS cant4, 0.000  AS cant5, 0.000  AS cant6, \
					0.000 AS cant7, 0.000 AS cant8, 0.000 AS cant9, 0.000 AS cant10, 0.000  AS cant11, 0.000  AS cant12, \
					0.000  AS cant13, 0.000  AS cant14, 0.000  AS cant15, 0.000  AS cant16, 0.000  AS cant17, 0.000 AS cant18, \
					0.000 AS cant19, 0.000  AS cant20, 0.000  AS cant21, 0.000  AS cant22, 0.000  AS cant23, 0.000  AS cant24, \
					0.000  AS cant25, 0.000 AS cant26, 0.000  AS cant27, 0.000 AS cant28, 0.000  AS cant29, 0.000  AS cant30,  \
					0.000  AS cant31, 0.000  AS cant32, 0.000  AS cant33, 0.000  AS cant34,	0.000  AS cant35,0.000 AS cant36, \
					0.000  AS ventas30, 0.000  AS ventas60, 0.000  AS ventas90, 0.000  AS ventas180, 0.000  AS ventascorte  \
				FROM articulos a INNER JOIN presentaciones p ON p.producto = a.producto AND p.present = a.present			\
				INNER JOIN almacenes al  GROUP BY a.producto, a.present, al.almacen)    \
				UNION ALL \
				(SELECT dv.almacen AS almacen, a.producto, a.present, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=1, dv.cantidad * a.factor, 0)) AS cant1, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=2, dv.cantidad * a.factor, 0)) AS cant2,  \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=3, dv.cantidad * a.factor, 0)) AS cant3, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=4, dv.cantidad * a.factor, 0)) AS cant4, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=5, dv.cantidad * a.factor, 0)) AS cant5, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=6, dv.cantidad * a.factor, 0)) AS cant6, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=7, dv.cantidad * a.factor, 0)) AS cant7, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=8, dv.cantidad * a.factor, 0)) AS cant8, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=9, dv.cantidad * a.factor, 0)) AS cant9, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=10, dv.cantidad * a.factor, 0)) AS cant10,\
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=11, dv.cantidad * a.factor, 0)) AS cant11,\
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=12, dv.cantidad * a.factor, 0)) AS cant12,\
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=1, dv.cantidad * a.factor, 0)) AS cant13, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=2, dv.cantidad * a.factor, 0)) AS cant14, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=3, dv.cantidad * a.factor, 0)) AS cant15, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=4, dv.cantidad * a.factor, 0)) AS cant16, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=5, dv.cantidad * a.factor, 0)) AS cant17, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=6, dv.cantidad * a.factor, 0)) AS cant18, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=7, dv.cantidad * a.factor, 0)) AS cant19, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=8, dv.cantidad * a.factor, 0)) AS cant20, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=9, dv.cantidad * a.factor, 0)) AS cant21, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=10, dv.cantidad * a.factor, 0)) AS cant22,\
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=11, dv.cantidad * a.factor, 0)) AS cant23,\
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=12, dv.cantidad * a.factor, 0)) AS cant24,\
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=1, dv.cantidad * a.factor, 0)) AS cant25, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=2, dv.cantidad * a.factor, 0)) AS cant26, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=3, dv.cantidad * a.factor, 0)) AS cant27, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=4, dv.cantidad * a.factor, 0)) AS cant28, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=5, dv.cantidad * a.factor, 0)) AS cant29, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=6, dv.cantidad * a.factor, 0)) AS cant30, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=7, dv.cantidad * a.factor, 0)) AS cant31, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=8, dv.cantidad * a.factor, 0)) AS cant32, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=9, dv.cantidad * a.factor, 0)) AS cant33, \
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=10, dv.cantidad * a.factor, 0)) AS cant34,\
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=11, dv.cantidad * a.factor, 0)) AS cant35,\
					SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=12, dv.cantidad * a.factor, 0)) AS cant36,\
					SUM(IF(v.fechavta>=DATE_SUB('%s', INTERVAL 30 DAY) AND v.fechavta<='%s', dv.cantidad * a.factor, 0)) AS ventas30, \
					SUM(IF(v.fechavta>=DATE_SUB('%s', INTERVAL 60 DAY) AND v.fechavta<='%s', dv.cantidad * a.factor, 0)) AS ventas60, \
					SUM(IF(v.fechavta>=DATE_SUB('%s', INTERVAL 90 DAY) AND v.fechavta<='%s', dv.cantidad * a.factor, 0)) AS ventas90, \
					SUM(IF(v.fechavta>=DATE_SUB('%s', INTERVAL 180 DAY) AND v.fechavta<='%s', dv.cantidad * a.factor, 0)) AS ventas180, \
					SUM(IF(v.fechavta>@fechacorte AND v.fechavta<='%s', dv.cantidad * a.factor, 0)) AS ventascorte \
				  FROM ventas v FORCE INDEX(fechavta) \
					INNER JOIN dventas dv ON v.referencia = dv.referencia \
					INNER JOIN articulos a ON dv.articulo=a.articulo \
                    INNER JOIN clientes cli ON cli.cliente = v.cliente AND cli.esparterelac = 0 \
				  WHERE v.fechavta between '%s' AND '%s' \
					AND v.cancelado = 0 \
					GROUP BY dv.almacen, a.producto, a.present) \
				UNION ALL \
				(SELECT dv.almacen AS almacen, a.producto, a.present, \
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=1, dn.cantidad * a.factor*-1, 0)) AS cant1,\
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=2, dn.cantidad * a.factor*-1, 0)) AS cant2,\
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=3, dn.cantidad * a.factor*-1, 0)) AS cant3,\
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=4, dn.cantidad * a.factor*-1, 0)) AS cant4,\
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=5, dn.cantidad * a.factor*-1, 0)) AS cant5,\
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=6, dn.cantidad * a.factor*-1, 0)) AS cant6,\
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=7, dn.cantidad * a.factor*-1, 0)) AS cant7,\
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=8, dn.cantidad * a.factor*-1, 0)) AS cant8,\
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=9, dn.cantidad * a.factor*-1, 0)) AS cant9, \
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=10, dn.cantidad * a.factor*-1, 0)) AS cant10,\
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=11, dn.cantidad * a.factor*-1, 0)) AS cant11, \
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=12, dn.cantidad * a.factor*-1, 0)) AS cant12,  \
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=1, dn.cantidad * a.factor*-1, 0)) AS cant13, \
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=2, dn.cantidad * a.factor*-1, 0)) AS cant14, \
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=3, dn.cantidad * a.factor*-1, 0)) AS cant15, \
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=4, dn.cantidad * a.factor*-1, 0)) AS cant16, \
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=5, dn.cantidad * a.factor*-1, 0)) AS cant17, \
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=6, dn.cantidad * a.factor*-1, 0)) AS cant18, \
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=7, dn.cantidad * a.factor*-1, 0)) AS cant19, \
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=8, dn.cantidad * a.factor*-1, 0)) AS cant20, \
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=9, dn.cantidad * a.factor*-1, 0)) AS cant21, \
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=10, dn.cantidad * a.factor*-1, 0)) AS cant22,\
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=11, dn.cantidad * a.factor*-1, 0)) AS cant23,\
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=12, dn.cantidad * a.factor*-1, 0)) AS cant24,\
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=1, dn.cantidad * a.factor*-1, 0)) AS cant25, \
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=2, dn.cantidad * a.factor*-1, 0)) AS cant26, \
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=3, dn.cantidad * a.factor*-1, 0)) AS cant27, \
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=4, dn.cantidad * a.factor*-1, 0)) AS cant28, \
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=5, dn.cantidad * a.factor*-1, 0)) AS cant29, \
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=6, dn.cantidad * a.factor*-1, 0)) AS cant30, \
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=7, dn.cantidad * a.factor*-1, 0)) AS cant31, \
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=8, dn.cantidad * a.factor*-1, 0)) AS cant32, \
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=9, dn.cantidad * a.factor*-1, 0)) AS cant33, \
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=10, dn.cantidad * a.factor*-1, 0)) AS cant34,\
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=11, dn.cantidad * a.factor*-1, 0)) AS cant35,\
					SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=12, dn.cantidad * a.factor*-1, 0)) AS cant36,\
					SUM(IF(n.fechanot>=DATE_SUB('%s', INTERVAL 30 DAY) AND n.fechanot<='%s', dn.cantidad * a.factor*-1, 0)) AS ventas30, \
					SUM(IF(n.fechanot>=DATE_SUB('%s', INTERVAL 60 DAY) AND n.fechanot<='%s', dn.cantidad * a.factor*-1, 0)) AS ventas60, \
					SUM(IF(n.fechanot>=DATE_SUB('%s', INTERVAL 90 DAY) AND n.fechanot<='%s', dn.cantidad * a.factor*-1, 0)) AS ventas90, \
					SUM(IF(n.fechanot>=DATE_SUB('%s', INTERVAL 180 DAY) AND n.fechanot<='%s', dn.cantidad * a.factor*-1, 0)) AS ventas180, \
					0 AS ventascorte \
				  FROM notascredcli n \
					INNER JOIN dnotascredcli dn ON  n.referencia = dn.referencia \
					INNER JOIN ventas v ON  n.venta = v.referencia \
					INNER JOIN dventas dv ON v.referencia = dv.referencia AND dv.articulo=dn.articulo \
					INNER JOIN articulos a ON dv.articulo=a.articulo \
                    INNER JOIN clientes cli ON cli.cliente = v.cliente AND cli.esparterelac = 0 \
				  WHERE n.fechanot between '%s' AND '%s' \
					AND n.cancelado = 0 AND n.tipo = '0' \
					GROUP BY dv.almacen, a.producto, a.present )) t \
				GROUP BY almacen, producto, present INTO OUTFILE '%s' ",
					cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,
					cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,
					cad_anio_anterior,cad_anio_anterior, cad_anio_anterior, cad_anio_anterior, cad_anio_anterior, cad_anio_anterior,
					cad_anio_anterior,cad_anio_anterior, cad_anio_anterior, cad_anio_anterior, cad_anio_anterior, cad_anio_anterior,
					cad_anio_actual, cad_anio_actual,cad_anio_actual,cad_anio_actual,cad_anio_actual,cad_anio_actual,
					cad_anio_actual, cad_anio_actual,cad_anio_actual,cad_anio_actual,cad_anio_actual,cad_anio_actual,
					cad_fecha_hoy,cad_fecha_hoy,cad_fecha_hoy,cad_fecha_hoy,cad_fecha_hoy,cad_fecha_hoy,
					cad_fecha_hoy, cad_fecha_hoy, cad_fecha_hoy,
					cad_fecha_inicio,
					cad_fecha_hoy,
					cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,
					cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,
					cad_anio_anterior, cad_anio_anterior, cad_anio_anterior, cad_anio_anterior, cad_anio_anterior, cad_anio_anterior,
					cad_anio_anterior, cad_anio_anterior, cad_anio_anterior, cad_anio_anterior, cad_anio_anterior, cad_anio_anterior,
					cad_anio_actual, cad_anio_actual,cad_anio_actual,cad_anio_actual,cad_anio_actual,cad_anio_actual,
					cad_anio_actual, cad_anio_actual,cad_anio_actual,cad_anio_actual,cad_anio_actual,cad_anio_actual,
					cad_fecha_hoy, cad_fecha_hoy,cad_fecha_hoy, cad_fecha_hoy,cad_fecha_hoy,
					cad_fecha_hoy,cad_fecha_hoy, cad_fecha_hoy,
					cad_fecha_inicio,
					cad_fecha_hoy,
					archivo_temp_vxmnr );
			instrucciones_vxmnr[num_instrucciones_vxmnr++]=instruccion_vxmnr;

			instruccion_vxmnr.sprintf("LOAD DATA INFILE '%s' INTO TABLE ventasxmes_norel (almacen,producto,present, \
					 cant1,cant2,cant3,cant4,cant5, cant6,cant7,cant8,cant9,cant10, \
					 cant11,cant12,cant13,cant14,cant15, cant16,cant17,cant18,cant19, \
					 cant20, cant21,cant22,cant23,cant24,  \
					 cant25,cant26,cant27, cant28,cant29,cant30,cant31, cant32, cant33,cant34,cant35,cant36, \
					 ventas30, ventas60, ventas90, ventas180,ventascorte) ",archivo_temp_vxmnr);
			instrucciones_vxmnr[num_instrucciones_vxmnr++]=instruccion_vxmnr;

			instruccion_vxmnr.sprintf("COMMIT");
			instrucciones_vxmnr[num_instrucciones_vxmnr++]=instruccion_vxmnr;

			instruccion_vxmnr.sprintf("SET AUTOCOMMIT=1");
			instrucciones_vxmnr[num_instrucciones_vxmnr++]=instruccion_vxmnr;

			instruccion_vxmnr="SET SESSION sql_log_bin = 1";
			instrucciones_vxmnr[num_instrucciones_vxmnr++]=instruccion_vxmnr;

			instruccion_vxmnr.sprintf("INSERT INTO bitacoraservidor VALUES (NULL,'%s',NOW(),'FIN_ID_CALC_VENTAS_MENSUALES_NOREL')",SucursalOrigen);
			instrucciones_vxmnr[num_instrucciones_vxmnr++]=instruccion_vxmnr;

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql_vxmnr=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones_vxmnr), aux_buffer_sql_vxmnr);
		for (i=0; i<num_instrucciones_vxmnr; i++)
			aux_buffer_sql_vxmnr=mFg.AgregaStringABuffer(instrucciones_vxmnr[i], aux_buffer_sql_vxmnr);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql_vxmnr);
	} __finally {
		mServidorVioleta->BorraArchivoTemp(archivo_temp_vxmnr);
		delete buffer_sql_vxmnr;
	}
}

//---------------------------------------------------------------------------

// ID_CON_ASIGPRIV
void ServidorAdminSistema::ConsultaAsigPriv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA ASIGNACION DE PRIVILEGIOS
    AnsiString instruccion;
	AnsiString usuario, grupo, objeto, sucursal, verPrivilegios;
	AnsiString CondicionGrupo, CondicionObjeto;
	char *resultado;

    usuario=mFg.ExtraeStringDeBuffer(&parametros);
    grupo=mFg.ExtraeStringDeBuffer(&parametros);
	objeto=mFg.ExtraeStringDeBuffer(&parametros);
	sucursal=mFg.ExtraeStringDeBuffer(&parametros);
	verPrivilegios=mFg.ExtraeStringDeBuffer(&parametros);
    mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
	CondicionGrupo=" ";
	CondicionObjeto=" ";

	//Obtiene todos los usuarios
	instruccion="SELECT us.usuario AS Usuario, CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS Nombre ";
	instruccion+="FROM usuariosucursal us ";
	instruccion+="INNER JOIN usuarios u ON u.empleado = us.usuario ";
	instruccion+="INNER JOIN empleados e ON e.empleado = u.empleado ";
	instruccion+="WHERE u.activo=1 GROUP BY us.usuario ORDER BY e.nombre, e.appat";
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

    // Obtiene todos los roles
	instruccion="select rolessistema.claverol AS Rol, rolessistema.nombre AS Nombre ";
	instruccion+="from rolessistema order by rolessistema.nombre";
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);


	//Obtiene todos los roles asignados al usuario
	instruccion="SELECT r.claverol AS Rol, r.nombre AS Descripcion ";
	instruccion+="FROM usuariorol u ";
	instruccion+="LEFT JOIN rolessistema r ON r.claverol = u.rol ";
	instruccion+="WHERE u.usuario = '";
	instruccion+=usuario;
	instruccion+="' ";
	resultado=Respuesta->ObtieneDirLibreBuffer();
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);


	//Obtiene todas las sucursales asignadas al usuario
	instruccion="SELECT CONCAT(s.sucursal, ' ', s.nombre ) AS sucursal, COUNT(us.sucursal) AS asignada ";
	instruccion+="FROM sucursales s ";
	instruccion+="LEFT JOIN usuarios u ON u.empleado = '";
	instruccion+=usuario;
	instruccion+="' LEFT JOIN usuariosucursal us ON us.usuario = u.empleado AND us.sucursal = s.sucursal ";
	instruccion+="GROUP BY s.sucursal";
	resultado=Respuesta->ObtieneDirLibreBuffer();
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	if(verPrivilegios == "globales"){

		// Obtiene todos los grupos
		instruccion="SELECT g.grupo, g.nombre, cast(count(asignaciontotal.privilegio)*100/count(p.privilegio) as decimal(16,2)) as porcentaje ";
		instruccion+="FROM gruposobjetos g inner join objetossistema o inner join privilegios p ";
		instruccion+="left join (( SELECT g.grupo, o.objeto, p.privilegio FROM usuarios u ";
		instruccion+=" INNER JOIN usuariorol ur ON ur.usuario = u.empleado ";
		instruccion+=" INNER JOIN asignacionprivrol ar ON ur.rol=ar.rol ";
		instruccion+=" INNER JOIN privilegios p ON ar.privilegio = p.privilegio AND ar.objeto = p.objeto ";
		instruccion+=" INNER JOIN objetossistema o ON p.objeto = o.objeto ";
		instruccion+=" INNER JOIN gruposobjetos g ON o.grupo = g.grupo ";
		instruccion+=" WHERE u.empleado = '";
		instruccion+=usuario;
		instruccion+="') UNION (SELECT g.grupo, o.objeto, p.privilegio FROM usuarios u";
		instruccion+=" INNER JOIN asignacionprivilegios a ON u.empleado=a.usuario ";
		instruccion+=" INNER JOIN privilegios p ON a.privilegio = p.privilegio AND a.objeto = p.objeto ";
		instruccion+=" INNER JOIN objetossistema o ON p.objeto = o.objeto ";
		instruccion+=" INNER JOIN gruposobjetos g ON o.grupo = g.grupo ";
		instruccion+=" WHERE u.empleado = '";
		instruccion+=usuario;
		instruccion+="') UNION (SELECT g.grupo, ar.objeto, ar.privilegio FROM usuarios u";
		instruccion+=" INNER JOIN empleados e ON e.empleado = u.empleado ";
		instruccion+=" INNER JOIN puestos p ON p.puesto = e.puesto ";
		instruccion+=" INNER JOIN rolesxpuesto rxp ON rxp.puesto = p.puesto ";
		instruccion+=" INNER JOIN asignacionprivrol ar ON rxp.rol=ar.rol ";
		instruccion+=" INNER JOIN objetossistema o ON ar.objeto = o.objeto ";
		instruccion+=" INNER JOIN gruposobjetos g ON g.grupo = g.grupo ";
		instruccion+=" WHERE u.empleado = '";
		instruccion+=usuario;
		instruccion+="' )) AS asignaciontotal ON asignaciontotal.objeto = o.objeto AND asignaciontotal.privilegio = p.privilegio ";
		instruccion+=" WHERE g.grupo=o.grupo AND o.objeto=p.objeto GROUP BY g.grupo";
		resultado=Respuesta->ObtieneDirLibreBuffer();
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

		// Si no se especificó ningún grupo, se toma el primero del resultado para
		// obtener sus objetos.
		if (grupo=="") {
			if (objeto=="") {
				BufferRespuestas resp_grupos(resultado);
				if (resp_grupos.ObtieneNumRegistros()) {
					grupo=resp_grupos.ObtieneDato("grupo");
					CondicionGrupo=" o.grupo='" + grupo+ "' and ";
				}
			}else{
			   CondicionGrupo= " o.grupo in( select grupo from objetossistema where objeto='" + objeto + "') AND ";
			}
		}else{
			CondicionGrupo=" o.grupo='" + grupo+ "' and ";
		}


		// Obtiene todos los objetos del grupo
		instruccion="SELECT o.objeto, o.nombre, cast(count(asignaciontotal.privilegio)*100/count(p.privilegio) as decimal(16,2)) as porcentaje ";
		instruccion+="FROM objetossistema o inner join privilegios p ";
		instruccion+="LEFT JOIN (( SELECT g.grupo, o.objeto, p.privilegio FROM usuarios u ";
		instruccion+="INNER JOIN usuariorol ur ON ur.usuario = u.empleado ";
		instruccion+="INNER JOIN asignacionprivrol ar ON ur.rol=ar.rol ";
		instruccion+="INNER JOIN privilegios p ON ar.privilegio = p.privilegio AND ar.objeto = p.objeto ";
		instruccion+="INNER JOIN objetossistema o ON p.objeto = o.objeto ";
		instruccion+="INNER JOIN gruposobjetos g ON o.grupo = g.grupo ";
		instruccion+="WHERE u.empleado = '";
		instruccion+=usuario;
		instruccion+="' ) UNION ( SELECT g.grupo, o.objeto, p.privilegio FROM usuarios u ";
		instruccion+="INNER JOIN asignacionprivilegios a ON u.empleado=a.usuario ";
		instruccion+="INNER JOIN privilegios p ON a.privilegio = p.privilegio AND a.objeto = p.objeto ";
		instruccion+="INNER JOIN objetossistema o ON p.objeto = o.objeto ";
		instruccion+="INNER JOIN gruposobjetos g ON o.grupo = g.grupo ";
		instruccion+="WHERE u.empleado = '";
		instruccion+=usuario;
		instruccion+="' ) UNION ( SELECT g.grupo, ar.objeto, ar.privilegio FROM usuarios u ";
		instruccion+="INNER JOIN empleados e ON e.empleado = u.empleado ";
		instruccion+="INNER JOIN puestos p ON p.puesto = e.puesto ";
		instruccion+="INNER JOIN rolesxpuesto rxp ON rxp.puesto = p.puesto ";
		instruccion+="INNER JOIN asignacionprivrol ar ON rxp.rol=ar.rol ";
		instruccion+="INNER JOIN objetossistema o ON ar.objeto = o.objeto  ";
		instruccion+="INNER JOIN gruposobjetos g ON o.grupo = g.grupo ";
		instruccion+="WHERE u.empleado = '";
		instruccion+=usuario;
		instruccion+="' )) AS asignaciontotal ON asignaciontotal.objeto = o.objeto AND asignaciontotal.privilegio = p.privilegio ";
		instruccion+="WHERE ";
		instruccion+=CondicionGrupo;
		instruccion+=" o.objeto=p.objeto GROUP BY o.objeto ORDER BY o.nombre ASC";
		resultado=Respuesta->ObtieneDirLibreBuffer();
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

		// Si no se especificó ningún objeto, se toma el primero del resultado para
		// obtener sus objetos.
		if (objeto=="") {
			BufferRespuestas resp_objetos(resultado);
			if (resp_objetos.ObtieneNumRegistros()) {
				objeto=resp_objetos.ObtieneDato("objeto");
				CondicionObjeto="p.objeto='"+objeto+"'  ";
			}
		}


		// Obtiene todos privilegios del objeto
		instruccion="SELECT concat(p.privilegio,' ',p.descripcion) as privilegio, count(asignaciontotal.privilegio) as concedido ";
		instruccion+="FROM privilegios p ";
		instruccion+="LEFT JOIN (( SELECT g.grupo, o.objeto, p.privilegio FROM usuarios u ";
		instruccion+="INNER JOIN usuariorol ur ON ur.usuario = u.empleado ";
		instruccion+="INNER JOIN asignacionprivrol ar ON ur.rol=ar.rol ";
		instruccion+="INNER JOIN privilegios p ON ar.privilegio = p.privilegio AND ar.objeto = p.objeto ";
		instruccion+="INNER JOIN objetossistema o ON p.objeto = o.objeto ";
		instruccion+="INNER JOIN gruposobjetos g ON o.grupo = g.grupo ";
		instruccion+="WHERE u.empleado = '";
		instruccion+=usuario;
		instruccion+="' ) UNION ( SELECT g.grupo, o.objeto, p.privilegio FROM usuarios u ";
		instruccion+="INNER JOIN asignacionprivilegios a ON u.empleado=a.usuario ";
		instruccion+="INNER JOIN privilegios p ON a.privilegio = p.privilegio AND a.objeto = p.objeto ";
		instruccion+="INNER JOIN objetossistema o ON p.objeto = o.objeto ";
		instruccion+="INNER JOIN gruposobjetos g ON o.grupo = g.grupo ";
		instruccion+="WHERE u.empleado = '";
		instruccion+=usuario;
		instruccion+="' ) UNION ( SELECT g.grupo, ar.objeto, ar.privilegio FROM usuarios u ";
		instruccion+="INNER JOIN empleados e ON e.empleado = u.empleado ";
		instruccion+="INNER JOIN puestos p ON p.puesto = e.puesto ";
		instruccion+="INNER JOIN rolesxpuesto rxp ON rxp.puesto = p.puesto ";
		instruccion+="INNER JOIN asignacionprivrol ar ON rxp.rol=ar.rol ";
		instruccion+="INNER JOIN objetossistema o ON ar.objeto = o.objeto ";
		instruccion+="INNER JOIN gruposobjetos g ON o.grupo = g.grupo ";
		instruccion+="WHERE u.empleado = '";
		instruccion+=usuario;
		instruccion+="' )) AS asignaciontotal ON asignaciontotal.objeto = p.objeto AND asignaciontotal.privilegio = p.privilegio ";
		instruccion+="where p.objeto='";
		instruccion+=objeto;
		instruccion+="' group by p.privilegio";
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);



		// Obtiene todos los usuarios con privilegios sobre el objeto
		instruccion="SELECT e.sucursal, CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS nombre, ";
		instruccion+="GROUP_CONCAT(DISTINCT asigtotal.privilegio ORDER BY asigtotal.privilegio) as privilegios, e.empleado ";
		instruccion+="FROM ((SELECT g.grupo, o.objeto, p.privilegio, u.empleado ";
		instruccion+="FROM usuarios u  ";
		instruccion+="INNER JOIN usuariorol ur ON ur.usuario = u.empleado  ";
		instruccion+="INNER JOIN asignacionprivrol ar on ur.rol=ar.rol  ";
		instruccion+="INNER JOIN privilegios p ON  ar.privilegio = p.privilegio AND ar.objeto = p.objeto  ";
		instruccion+="INNER JOIN objetossistema o ON p.objeto = o.objeto  ";
		instruccion+="INNER JOIN gruposobjetos g ON o.grupo = g.grupo  ";
		instruccion+=") UNION (SELECT g.grupo, o.objeto, p.privilegio, u.empleado FROM usuarios u ";
		instruccion+="INNER JOIN asignacionprivilegios ar ON u.empleado=ar.usuario  ";
		instruccion+="INNER JOIN privilegios p ON  ar.privilegio = p.privilegio AND ar.objeto = p.objeto  ";
		instruccion+="INNER JOIN objetossistema o ON p.objeto = o.objeto  ";
		instruccion+="INNER JOIN gruposobjetos g ON o.grupo = g.grupo ";
		instruccion+=") UNION (SELECT g.grupo, ar.objeto, ar.privilegio, u.empleado FROM usuarios u ";
		instruccion+="INNER JOIN empleados e ON e.empleado = u.empleado ";
		instruccion+="INNER JOIN puestos p ON p.puesto = e.puesto ";
		instruccion+="INNER JOIN rolesxpuesto rxp ON rxp.puesto = p.puesto ";
		instruccion+="INNER JOIN asignacionprivrol ar ON rxp.rol=ar.rol ";
		instruccion+="INNER JOIN objetossistema o ON ar.objeto = o.objeto ";
		instruccion+="INNER JOIN gruposobjetos g ON o.grupo = g.grupo )) AS asigtotal ";
		instruccion+="INNER JOIN empleados e ON e.empleado=asigtotal.empleado  ";
		instruccion+="INNER JOIN usuarios u ON e.empleado= u.empleado ";
		instruccion+="WHERE asigtotal.objeto='";
		instruccion+=objeto;
		instruccion+="' AND u.activo=1 ";

			if(sucursal != "")
			{
				instruccion+=" AND e.sucursal = '";
				instruccion+=sucursal;
				instruccion+="'";
			}

		instruccion+=" GROUP BY asigtotal.empleado ORDER BY e.nombre, e.empleado";
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	}else if (verPrivilegios == "personales") {

		// Obtiene todos los grupos
		instruccion="SELECT g.grupo, g.nombre, CAST(COUNT(a.privilegio)*100/ COUNT(p.privilegio) AS DECIMAL(16,2)) AS porcentaje \
		FROM gruposobjetos g INNER JOIN objetossistema o INNER JOIN privilegios p \
		LEFT JOIN usuarios u ON u.empleado = '";
		instruccion+=usuario;
		instruccion+="' LEFT JOIN asignacionprivilegios a ON a.usuario= u.empleado AND a.objeto = o.objeto \
		AND a.privilegio = p.privilegio WHERE g.grupo=o.grupo AND o.objeto=p.objeto GROUP BY g.grupo";
		resultado=Respuesta->ObtieneDirLibreBuffer();
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

		// Si no se especificó ningún grupo, se toma el primero del resultado para
		// obtener sus objetos.
		if (grupo=="") {
			if (objeto=="") {
				BufferRespuestas resp_grupos(resultado);
				if (resp_grupos.ObtieneNumRegistros()) {
					grupo=resp_grupos.ObtieneDato("grupo");
					CondicionGrupo=" o.grupo='" + grupo+ "' and ";
				}
			}else{
			   CondicionGrupo= " o.grupo in( select grupo from objetossistema where objeto='" + objeto + "') AND ";
			}
		}else{
			CondicionGrupo=" o.grupo='" + grupo+ "' and ";
		}

		// Obtiene todos los objetos del grupo
		instruccion="SELECT o.objeto, o.nombre, CAST(COUNT(a.privilegio)*100/ COUNT(p.privilegio) AS DECIMAL(16,2)) AS porcentaje \
		FROM objetossistema o INNER JOIN privilegios p LEFT JOIN usuarios u ON u.empleado = '";
		instruccion+=usuario;
		instruccion+="' LEFT JOIN asignacionprivilegios a ON a.usuario = u.empleado AND a.objeto = o.objeto \
		AND a.privilegio = p.privilegio WHERE ";
		instruccion+=CondicionGrupo;
		instruccion+=" o.objeto=p.objeto GROUP BY o.objeto ORDER BY o.nombre ASC";


		resultado=Respuesta->ObtieneDirLibreBuffer();
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

		// Si no se especificó ningún objeto, se toma el primero del resultado para
		// obtener sus objetos.
		if (objeto=="") {
			BufferRespuestas resp_objetos(resultado);
			if (resp_objetos.ObtieneNumRegistros()) {
				objeto=resp_objetos.ObtieneDato("objeto");
				CondicionObjeto="p.objeto='"+objeto+"'  ";
			}
		}

		// Obtiene todos privilegios del objeto
		instruccion="SELECT concat(p.privilegio,' ',p.descripcion) as privilegio, count(a.privilegio) as concedido \
		FROM privilegios p LEFT JOIN usuarios u ON u.empleado = '";
		instruccion+=usuario;
		instruccion+="' LEFT JOIN asignacionprivilegios a ON a.usuario = u.empleado AND a.objeto=p.objeto AND a.privilegio=p.privilegio \
		WHERE p.objeto='";
		instruccion+=objeto;
		instruccion+="' GROUP BY p.privilegio";
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);


		// Obtiene todos los usuarios con privilegios sobre el objeto
		instruccion="SELECT e.sucursal, CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS nombre, \
		GROUP_CONCAT(DISTINCT a.privilegio ORDER BY a.privilegio) as privilegios, e.empleado \
		FROM asignacionprivilegios a \
		INNER JOIN empleados e ON e.empleado = a.usuario \
		INNER JOIN usuarios u ON u.empleado = e.empleado \
		WHERE a.objeto='";
		instruccion+=objeto;
		instruccion+="' AND u.activo=1 ";

		if(sucursal != ""){
				instruccion+=" AND e.sucursal = '";
				instruccion+=sucursal;
				instruccion+="'";
		}
		instruccion+=" GROUP BY a.usuario ORDER BY e.nombre, e.empleado";
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);


	}

}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// ID_CON_PRIV
void ServidorAdminSistema::ConsultaPriv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA ASIGNACION DE PRIVILEGIOS
	AnsiString instruccion;
	AnsiString privilegio, objeto, sucursal, verPrivilegios, opcion_sucursal = " ";
	char *resultado;

	objeto=mFg.ExtraeStringDeBuffer(&parametros);
	privilegio=mFg.ExtraeStringDeBuffer(&parametros);
	sucursal=mFg.ExtraeStringDeBuffer(&parametros);
	verPrivilegios=mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	if(sucursal != "")
		opcion_sucursal.sprintf(" AND e.sucursal = '%s' ", sucursal);

	if(verPrivilegios=="globales"){
		instruccion.sprintf("SELECT atl.sucursal, CONCAT(atl.nombre,' ',atl.appat,' ',atl.apmat) AS nombre, atl.empleado \
		FROM ((SELECT e.sucursal, e.nombre, e.appat, e.apmat, ap.usuario \
			, e.empleado FROM asignacionprivilegios ap \
			INNER JOIN empleados e ON e.empleado=ap.usuario \
			INNER JOIN usuarios u ON e.empleado= u.empleado \
			WHERE ap.objeto='%s' AND u.activo=1 AND ap.privilegio='%s' %s \
			) UNION (SELECT e.sucursal, e.nombre, e.appat, e.apmat, u.empleado AS usuario \
			,	e.empleado FROM asignacionprivrol apr \
			INNER JOIN usuariorol ur ON ur.rol= apr.rol \
			INNER JOIN empleados e ON e.empleado= ur.usuario \
			INNER JOIN usuarios u ON e.empleado= u.empleado \
			WHERE apr.objeto='%s' AND u.activo=1 \
			AND apr.privilegio= '%s' %s \
			)  UNION (SELECT e.sucursal, e.nombre, e.appat, e.apmat, u.empleado AS usuario \
			,	e.empleado FROM usuarios u INNER JOIN empleados e ON e.empleado = u.empleado \
			INNER JOIN puestos p ON p.puesto = e.puesto \
			INNER JOIN rolesxpuesto rxp ON rxp.puesto = p.puesto \
			INNER JOIN asignacionprivrol ar ON rxp.rol=ar.rol \
			WHERE ar.objeto='%s' AND u.activo=1 \
			AND ar.privilegio= '%s' %s)  \
		) AS atl GROUP BY atl.usuario ORDER BY atl.nombre ",
		objeto, privilegio, opcion_sucursal, objeto, privilegio, opcion_sucursal, objeto, privilegio, opcion_sucursal);

	} else if(verPrivilegios=="personales"){
		instruccion.sprintf("SELECT e.sucursal, CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS nombre \
		, e.empleado FROM asignacionprivilegios ap \
		INNER JOIN empleados e ON e.empleado=ap.usuario \
		INNER JOIN usuarios u ON e.empleado= u.empleado \
		WHERE ap.objeto='%s' AND u.activo=1 AND ap.privilegio='%s' %s \
		GROUP BY ap.usuario ORDER BY e.nombre", objeto, privilegio, opcion_sucursal);
	}

	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// ID_CON_PRIVROL
void ServidorAdminSistema::ConsultaPrivRol(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA ASIGNACION DE PRIVILEGIOS
	AnsiString instruccion;
	AnsiString privilegio, objeto, opcion_sucursal = " ";
	char *resultado;

	objeto=mFg.ExtraeStringDeBuffer(&parametros);
	privilegio=mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los roles con privilegios sobre el objeto
	instruccion.sprintf("SELECT r.nombre AS rol \
			   ,	r.claverol \
	FROM asignacionprivrol ap \
			INNER JOIN rolessistema r ON r.claverol= ap.rol \
			WHERE ap.objeto='%s' \
			AND ap.privilegio = '%s' GROUP BY ap.rol ORDER BY r.nombre"
			, objeto, privilegio);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//ID_CONC_ASIGPRIV
void ServidorAdminSistema::ConcedeAsigPriv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*5];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instruccion, instrucciones[6];
	int num_instrucciones=0;
	int i;
	AnsiString tarea, usuarioSistema, usuario, grupo, objeto, privilegio;

	try{
		usuarioSistema=mFg.ExtraeStringDeBuffer(&parametros);
		usuario=mFg.ExtraeStringDeBuffer(&parametros);
		tarea=mFg.ExtraeStringDeBuffer(&parametros);
		if (tarea=="GRUPO") {
			grupo=mFg.ExtraeStringDeBuffer(&parametros);
			instruccion="insert ignore into asignacionprivilegios (usuario, objeto, privilegio) ";
			instruccion+="select '";
			instruccion+=usuario;
			instruccion+="' as usuario, o.objeto, p.privilegio ";
			instruccion+="from gruposobjetos g, objetossistema o, privilegios p ";
			instruccion+="where g.grupo='";
			instruccion+=grupo;
			instruccion+="' and g.grupo=o.grupo and o.objeto=p.objeto";
			instrucciones[num_instrucciones++]=instruccion;

			//Se registra en la bitácora de modificaciones de privilegios de usuarios
			instruccion.sprintf(" INSERT INTO bitacoramodprivusu \
			(fecha, hora, usuario, usuario_mod, tipo_mod, entidad_mod, entidad_nombre) \
			VALUES (CURDATE(), CURTIME(), '%s', '%s', 'CONCEDER', 'GRUPO', '%s') "
			, usuarioSistema, usuario, grupo);
			instrucciones[num_instrucciones++]=instruccion;
		}
		if (tarea=="OBJETO") {
			objeto=mFg.ExtraeStringDeBuffer(&parametros);
			instruccion="insert ignore into asignacionprivilegios (usuario, objeto, privilegio) ";
			instruccion+="select '";
			instruccion+=usuario;
			instruccion+="' as usuario, o.objeto, p.privilegio ";
			instruccion+="from objetossistema o, privilegios p ";
			instruccion+="where o.objeto='";
			instruccion+=objeto;
			instruccion+="' and o.objeto=p.objeto";
			instrucciones[num_instrucciones++]=instruccion;

			//Se registra en la bitácora de modificaciones de privilegios de usuarios
			instruccion.sprintf(" INSERT INTO bitacoramodprivusu \
			(fecha, hora, usuario, usuario_mod, tipo_mod, entidad_mod, entidad_nombre) \
			VALUES (CURDATE(), CURTIME(), '%s', '%s', 'CONCEDER', 'OBJETO', '%s') "
			, usuarioSistema, usuario, objeto);
			instrucciones[num_instrucciones++]=instruccion;
		}
		if (tarea=="PRIVILEGIO") {
			privilegio=mFg.ExtraeStringDeBuffer(&parametros);
			objeto=mFg.ExtraeStringDeBuffer(&parametros);
			instruccion="insert ignore into asignacionprivilegios (usuario, objeto, privilegio) ";
			instruccion+="select '";
			instruccion+=usuario;
			instruccion+="' as usuario, o.objeto, p.privilegio ";
			instruccion+="from objetossistema o, privilegios p ";
			instruccion+="where o.objeto='";
			instruccion+=objeto;
			instruccion+="' and o.objeto=p.objeto and p.privilegio='";
			instruccion+=privilegio;
			instruccion+="'";
			instrucciones[num_instrucciones++]=instruccion;

			//Se registra en la bitácora de modificaciones de privilegios de usuarios
			instruccion.sprintf(" INSERT INTO bitacoramodprivusu \
			(fecha, hora, usuario, usuario_mod, tipo_mod, entidad_mod, entidad_nombre) \
			VALUES (CURDATE(), CURTIME(), '%s', '%s', 'CONCEDER', 'PRIVILEGIO', '%s: %s') "
			, usuarioSistema, usuario, objeto, privilegio);
			instrucciones[num_instrucciones++]=instruccion;
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

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//ID_QUI_ASIGPRIV
void ServidorAdminSistema::QuitaAsigPriv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*5];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instruccion, instrucciones[5];
	int num_instrucciones=0;
	int i;
	AnsiString tarea, usuarioSistema, usuario, grupo, objeto, privilegio;

	try{
		usuarioSistema=mFg.ExtraeStringDeBuffer(&parametros);
		usuario=mFg.ExtraeStringDeBuffer(&parametros);
		tarea=mFg.ExtraeStringDeBuffer(&parametros);

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		if (tarea=="GRUPO") {
			grupo=mFg.ExtraeStringDeBuffer(&parametros);

			instruccion="delete from a ";
			instruccion+="using asignacionprivilegios a, gruposobjetos g, objetossistema o ";
			instruccion+="where a.usuario='";
			instruccion+=usuario;
			instruccion+="' and g.grupo='";
			instruccion+=grupo;
			instruccion+="' and g.grupo=o.grupo and a.objeto=o.objeto";
			instrucciones[num_instrucciones++]=instruccion;

            //Se registra en la bitácora de modificaciones de privilegios de usuarios
			instruccion.sprintf(" INSERT INTO bitacoramodprivusu \
			(fecha, hora, usuario, usuario_mod, tipo_mod, entidad_mod, entidad_nombre) \
			VALUES (CURDATE(), CURTIME(), '%s', '%s', 'QUITAR', 'GRUPO', '%s') "
			, usuarioSistema, usuario, grupo);
			instrucciones[num_instrucciones++]=instruccion;
		}
		if (tarea=="OBJETO") {
			objeto=mFg.ExtraeStringDeBuffer(&parametros);

			instruccion="delete from asignacionprivilegios ";
			instruccion+="where usuario='";
			instruccion+=usuario;
			instruccion+="' and objeto='";
			instruccion+=objeto;
			instruccion+="'";
			instrucciones[num_instrucciones++]=instruccion;

			//Se registra en la bitácora de modificaciones de privilegios de usuarios
			instruccion.sprintf(" INSERT INTO bitacoramodprivusu \
			(fecha, hora, usuario, usuario_mod, tipo_mod, entidad_mod, entidad_nombre) \
			VALUES (CURDATE(), CURTIME(), '%s', '%s', 'QUITAR', 'OBJETO', '%s') "
			, usuarioSistema, usuario, objeto);
			instrucciones[num_instrucciones++]=instruccion;
		}
		if (tarea=="PRIVILEGIO") {
			privilegio=mFg.ExtraeStringDeBuffer(&parametros);
			objeto=mFg.ExtraeStringDeBuffer(&parametros);

			instruccion="delete from asignacionprivilegios ";
			instruccion+="where usuario='";
			instruccion+=usuario;
			instruccion+="' and objeto='";
			instruccion+=objeto;
			instruccion+="' and privilegio='";
			instruccion+=privilegio;
			instruccion+="'";
			instrucciones[num_instrucciones++]=instruccion;

            //Se registra en la bitácora de modificaciones de privilegios de usuarios
			instruccion.sprintf(" INSERT INTO bitacoramodprivusu \
			(fecha, hora, usuario, usuario_mod, tipo_mod, entidad_mod, entidad_nombre) \
			VALUES (CURDATE(), CURTIME(), '%s', '%s', 'QUITAR', 'PRIVILEGIO', '%s: %s') "
			, usuarioSistema, usuario, objeto, privilegio);
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

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//ID_COP_ASIGPRIV
void ServidorAdminSistema::CopiaAsigPriv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*5];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instruccion, instrucciones[10];
	int num_instrucciones=0;
	int i;
	AnsiString usuario_sistema, usuario_origen, usuario_destino;

	try{
		usuario_sistema=mFg.ExtraeStringDeBuffer(&parametros);
		usuario_origen=mFg.ExtraeStringDeBuffer(&parametros);
		usuario_destino=mFg.ExtraeStringDeBuffer(&parametros);

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		// Crea una tabla temporal con los privilegios del usuario origen
		instruccion="create temporary table tmpcopiaprivil ";
		instruccion+="select '";
		instruccion+=usuario_destino;
		instruccion+="' as usuario, objeto, privilegio ";
		instruccion+="from asignacionprivilegios ";
		instruccion+="where usuario='";
		instruccion+=usuario_origen;
		instruccion+="'";
		instrucciones[num_instrucciones++]=instruccion;

		// Borra todos los privilegios del usuario destino
		instruccion="delete from asignacionprivilegios ";
		instruccion+="where usuario='";
		instruccion+=usuario_destino;
		instruccion+="'";
		instrucciones[num_instrucciones++]=instruccion;

		// Inserta los registros para el usuario destino
		instruccion="insert ignore into asignacionprivilegios (usuario, objeto, privilegio) ";
		instruccion+="select usuario,objeto,privilegio from tmpcopiaprivil";
		instrucciones[num_instrucciones++]=instruccion;

        //Se registra en la bitácora de modificaciones de privilegios de usuarios
		instruccion.sprintf(" INSERT INTO bitacoramodprivusu \
		(fecha, hora, usuario, usuario_mod, tipo_mod, entidad_mod, entidad_nombre) \
		VALUES (CURDATE(), CURTIME(), '%s', '%s', 'COPIAR', 'USUARIO', '%s') "
		, usuario_sistema, usuario_destino, usuario_origen);
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

//---------------------------------------------------------------------------
//ID_OBT_ASIGPRIV
void ServidorAdminSistema::ObtieneAsigPriv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    // CONSULTA PARAMETRO
    AnsiString instruccion;
    AnsiString usuario, objeto;

    usuario=mFg.ExtraeStringDeBuffer(&parametros);
    objeto=mFg.ExtraeStringDeBuffer(&parametros);
    mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

    // Obtiene todos los privilegios que tiene un usuario con respecto a un objeto
	instruccion.sprintf("select p.privilegio, p.descripcion from asignacionprivilegios a, privilegios p ,usuarios u where a.privilegio=p.privilegio and a.objeto=p.objeto and a.usuario=u.empleado and u.activo=1 and a.usuario='%s' and a.objeto='%s'", usuario, objeto);
    mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//ID_OBT_ASIGPRIVUSUAROL
void ServidorAdminSistema::ObtieneAsigPrivUsuaRol(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    // CONSULTA PARAMETRO
    AnsiString instruccion;
    AnsiString usuario, objeto;

    usuario=mFg.ExtraeStringDeBuffer(&parametros);
    objeto=mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los privilegios que tiene un usuario con respecto a un objeto
	// considerando los heredados por sus roles
	instruccion.sprintf("select p.privilegio, p.descripcion \
	from ((select o.objeto, p.privilegio, u.empleado \
	from usuarios u \
	inner join usuariorol ur on ur.usuario = u.empleado \
	inner join asignacionprivrol ar on ur.rol=ar.rol \
	inner join privilegios p on ar.privilegio = p.privilegio and ar.objeto = p.objeto \
	inner join objetossistema o on p.objeto = o.objeto \
	where u.activo=1 AND u.empleado = '%s' AND o.objeto = '%s' \
	) union \
	(select o.objeto, p.privilegio, u.empleado \
	from usuarios u  \
	inner join asignacionprivilegios a on u.empleado=a.usuario \
	inner join privilegios p on a.privilegio = p.privilegio and a.objeto = p.objeto \
	inner join objetossistema o on p.objeto = o.objeto \
	where u.activo=1 AND u.empleado = '%s' AND o.objeto = '%s' \
	) union \
	(SELECT ar.objeto, ar.privilegio, u.empleado FROM usuarios u \
	INNER JOIN empleados e ON e.empleado = u.empleado \
	INNER JOIN puestos p ON p.puesto = e.puesto \
	INNER JOIN rolesxpuesto rxp ON rxp.puesto = p.puesto \
	INNER JOIN asignacionprivrol ar ON rxp.rol=ar.rol \
	where u.activo=1 AND u.empleado = '%s' AND ar.objeto = '%s' \
	)) AS asigtotal INNER JOIN privilegios p ON asigtotal.privilegio=p.privilegio AND asigtotal.objeto=p.objeto"
	,usuario, objeto
	,usuario, objeto
	,usuario, objeto);

	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	instruccion.sprintf("SELECT valor FROM parametrosemp WHERE parametro =  'SUBVERSIONMIN' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	instruccion.sprintf("SELECT valor FROM parametrosemp WHERE parametro =  'VERSIONMINIMA' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

    instruccion.sprintf("SELECT valor FROM parametrosemp WHERE parametro =  'TIEMVALIDVERS' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//ID_OBT_ASIGPRIVUSUAROL_ESP
void ServidorAdminSistema::ObtieneAsigPrivUsuaRolEspec(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA PARAMETRO
	AnsiString instruccion;
	AnsiString usuario, objeto, privilegio;
	AnsiString condicion_privilegio = " ";


	usuario=mFg.ExtraeStringDeBuffer(&parametros);
	objeto=mFg.ExtraeStringDeBuffer(&parametros);
	privilegio=mFg.ExtraeStringDeBuffer(&parametros);

	if(privilegio != ""){
	   condicion_privilegio.sprintf(" and asigprivtotal.privilegio='%s'", privilegio);
	}

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene un privilegio de un objeto de un usuario y considerando los heredados por sus roles
	instruccion.sprintf("select * from(( \
	select u.empleado, ar.objeto, ar.privilegio from usuarios u \
	inner join usuariorol ur on ur.usuario = u.empleado inner join asignacionprivrol ar on ur.rol=ar.rol ) \
	union ( select a.usuario AS empleado, a.objeto, a.privilegio from asignacionprivilegios a ) UNION ( \
	SELECT u.empleado, ar.objeto, ar.privilegio FROM usuarios u INNER JOIN empleados e ON e.empleado = u.empleado \
	INNER JOIN puestos p ON p.puesto = e.puesto INNER JOIN rolesxpuesto rxp ON rxp.puesto = p.puesto \
	INNER JOIN asignacionprivrol ar ON rxp.rol=ar.rol )) as asigprivtotal \
	where asigprivtotal.empleado='%s' and asigprivtotal.objeto='%s' %s ", usuario, objeto, condicion_privilegio);

	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//ID_CONC_ASIGROL
void ServidorAdminSistema::ConcedeAsigRol(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*5];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instruccion, instrucciones[3];
	int num_instrucciones=0;
	int i;
	AnsiString usuarioSistema, usuario, rol;

	try{
		usuarioSistema=mFg.ExtraeStringDeBuffer(&parametros);
		usuario=mFg.ExtraeStringDeBuffer(&parametros);
		rol=mFg.ExtraeStringDeBuffer(&parametros);

		instruccion="insert ignore into usuariorol (usuario, rol) VALUES ('";
		instruccion+=usuario;
		instruccion+="', '";
		instruccion+=rol;
		instruccion+="') ";
		instrucciones[num_instrucciones++]=instruccion;

		//Se registra en la bitácora de modificaciones de privilegios de usuarios
		instruccion.sprintf(" INSERT INTO bitacoramodprivusu \
		(fecha, hora, usuario, usuario_mod, tipo_mod, entidad_mod, entidad_nombre) \
		VALUES (CURDATE(), CURTIME(), '%s', '%s', 'CONCEDER', 'ROL', '%s') "
		, usuarioSistema, usuario, rol);
		instrucciones[num_instrucciones++]=instruccion;

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

//---------------------------------------------------------------------------
//ID_QUI_ASIGROL
void ServidorAdminSistema::QuitaAsigRol(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*5];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instruccion, instrucciones[3];
	int num_instrucciones=0;
	int i;
	AnsiString usuarioSistema, usuario, rol;

	try{
		usuarioSistema=mFg.ExtraeStringDeBuffer(&parametros);
		usuario=mFg.ExtraeStringDeBuffer(&parametros);
		rol=mFg.ExtraeStringDeBuffer(&parametros);

		instruccion="delete from usuariorol where usuario='";
		instruccion+=usuario;
		instruccion+="' and rol='";
		instruccion+=rol;
		instruccion+="' ";
		instrucciones[num_instrucciones++]=instruccion;

        //Se registra en la bitácora de modificaciones de privilegios de usuarios
		instruccion.sprintf(" INSERT INTO bitacoramodprivusu \
		(fecha, hora, usuario, usuario_mod, tipo_mod, entidad_mod, entidad_nombre) \
		VALUES (CURDATE(), CURTIME(), '%s', '%s', 'QUITAR', 'ROL', '%s') "
		, usuarioSistema, usuario, rol);
		instrucciones[num_instrucciones++]=instruccion;

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

//---------------------------------------------------------------------------
//ID_CONC_ASIGSUC
void ServidorAdminSistema::ConcedeAsigSuc(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*5];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instruccion, instrucciones[3];
	int num_instrucciones=0;
	int i;
	AnsiString usuarioSistema, usuario, sucursal;

	try{
		usuarioSistema=mFg.ExtraeStringDeBuffer(&parametros);
		usuario=mFg.ExtraeStringDeBuffer(&parametros);
		sucursal=mFg.ExtraeStringDeBuffer(&parametros);

		instruccion="insert ignore into usuariosucursal(usuario, sucursal) VALUES ('";
		instruccion+=usuario;
		instruccion+="', '";
		instruccion+=sucursal;
		instruccion+="') ";
		instrucciones[num_instrucciones++]=instruccion;

		//Se registra en la bitácora de modificaciones de privilegios de usuarios
		instruccion.sprintf(" INSERT INTO bitacoramodprivusu \
		(fecha, hora, usuario, usuario_mod, tipo_mod, entidad_mod, entidad_nombre) \
		VALUES (CURDATE(), CURTIME(), '%s', '%s', 'CONCEDER', 'SUCURSAL', '%s') "
		, usuarioSistema, usuario, sucursal);
		instrucciones[num_instrucciones++]=instruccion;

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

//---------------------------------------------------------------------------
//ID_QUI_ASIGSUC
void ServidorAdminSistema::QuitaAsigSuc(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*5];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instruccion, instrucciones[3];
	int num_instrucciones=0;
	int i;
	AnsiString usuarioSistema, usuario, sucursal;

	try{
		usuarioSistema=mFg.ExtraeStringDeBuffer(&parametros);
		usuario=mFg.ExtraeStringDeBuffer(&parametros);
		sucursal=mFg.ExtraeStringDeBuffer(&parametros);

		instruccion="delete from usuariosucursal where usuario='";
		instruccion+=usuario;
		instruccion+="' and sucursal='";
		instruccion+=sucursal;
		instruccion+="' ";
		instrucciones[num_instrucciones++]=instruccion;

		//Se registra en la bitácora de modificaciones de privilegios de usuarios
		instruccion.sprintf(" INSERT INTO bitacoramodprivusu \
		(fecha, hora, usuario, usuario_mod, tipo_mod, entidad_mod, entidad_nombre) \
		VALUES (CURDATE(), CURTIME(), '%s', '%s', 'QUITAR', 'SUCURSAL', '%s') "
		, usuarioSistema, usuario, sucursal);
		instrucciones[num_instrucciones++]=instruccion;


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

//---------------------------------------------------------------------------
//ID_GRA_ASIGNACLAVE
void ServidorAdminSistema::AsignaClave(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// ASIGNA LA CLAVE DE ACCESO AL SISTEMA DE UN USUARIO DADO
	char *buffer_sql=new char[1024*5];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instruccion, instrucciones[5];
	int num_instrucciones=0;
	int i;
	AnsiString usuario, nueva_clave;

	try{
		usuario=mFg.ExtraeStringDeBuffer(&parametros);
		nueva_clave=mFg.ExtraeStringDeBuffer(&parametros);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

		// Se asigna una variable el identificador del usurio
		instruccion.sprintf("select @idusuario:=empleado from usuarios where empleado='%s'", usuario);
		instrucciones[num_instrucciones++]=instruccion;

		// Si la clave es correcta para el usuario dado, entonces cambia dicha clave por la nueva
		instruccion.sprintf("update usuarios set password='%s' where empleado='%s'", nueva_clave, usuario);
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

//---------------------------------------------------------------------------
//ID_GRA_CAMBIACLAVE
void ServidorAdminSistema::CambiaClave(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CAMBIA LA CLAVE DE ACCESO AL SISTEMA DE UN USUARIO DADO
	char *buffer_sql=new char[1024*5];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instruccion, instrucciones[5];
	int num_instrucciones=0;
	int i;
	AnsiString usuario, clave, nueva_clave;

	try{
		usuario=mFg.ExtraeStringDeBuffer(&parametros);
		clave=mFg.ExtraeStringDeBuffer(&parametros);
		nueva_clave=mFg.ExtraeStringDeBuffer(&parametros);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

		// Se asigna una variable el identificador del usurio que coincide con el identificador y el password
		instruccion.sprintf("select @idusuario:=empleado from usuarios where empleado='%s' and password='%s'", usuario, clave);
		instrucciones[num_instrucciones++]=instruccion;

		// Si la clave es correcta para el usuario dado, entonces cambia dicha clave por la nueva
		instruccion.sprintf("update usuarios set password='%s' where empleado='%s' and password='%s'", nueva_clave, usuario, clave);
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
//---------------------------------------------------------------------------
//ID_CON_ACCESO
void ServidorAdminSistema::VerificaAcceso(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	BufferRespuestas* resp_acceso=NULL;
    // CONSULTA SI EL USUARIO TIENE ACCESO AL SISTEMA
    AnsiString instruccion;
	AnsiString usuario, clave, IP, terminal;
	AnsiString tienePassword;

	usuario=mFg.ExtraeStringDeBuffer(&parametros);
	clave=mFg.ExtraeStringDeBuffer(&parametros);
	IP=mFg.ExtraeStringDeBuffer(&parametros);
	terminal=mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->MuestraMensaje("-- Usuario: " + usuario + " Terminal: " + terminal + " IP: " + IP);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	try {
		instruccion.sprintf("SELECT password FROM usuarios WHERE empleado = '%s' ",
		usuario);

		if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_acceso)) {
			if (resp_acceso->ObtieneNumRegistros()>0){
				tienePassword = resp_acceso->ObtieneDato("password");
			}
		}
	} __finally {
		if (resp_acceso!=NULL) delete resp_acceso;
	}

	if(tienePassword == ""){
		// Si la clave es correcta para el usuario dado, entonces Regresa el identificador de usuario
		instruccion.sprintf("SELECT empleado as usuario, 1 AS asignaPass, \
			IFNULL(AES_DECRYPT(usuariocontpaq,'LaVioletaSaSvMartinCastrejon2020UsuarioPassContpaq'),'') AS usuariocontpaq,\
			IFNULL(AES_DECRYPT(passwordcontpaq,'LaVioletaSaSvMartinCastrejon2020UsuarioPassContpaq'),'') as passwordcontpaq \
			FROM usuarios WHERE empleado='%s' and activo=1 ", usuario);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	} else {

		// Si la clave es correcta para el usuario dado, entonces Regresa el identificador de usuario
		instruccion.sprintf("SELECT empleado as usuario, 0 AS asignaPass, \
			IFNULL(AES_DECRYPT(usuariocontpaq,'LaVioletaSaSvMartinCastrejon2020UsuarioPassContpaq'),'') AS usuariocontpaq,\
			IFNULL(AES_DECRYPT(passwordcontpaq,'LaVioletaSaSvMartinCastrejon2020UsuarioPassContpaq'),'') as passwordcontpaq \
			FROM usuarios WHERE empleado='%s' and password='%s' and activo=1 ", usuario, clave);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
}

//---------------------------------------------------------------------------
//ID_CON_ACCESO_ESPECIFICO
void ServidorAdminSistema::VerificaAccesoEspecifico(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    // CONSULTA SI EL USUARIO TIENE ACCESO A UN PRIVILEGIO EN ESPECIFICO DEL SISTEMA
    AnsiString instruccion;
    AnsiString usuario, clave, objeto, privilegio;

    usuario=mFg.ExtraeStringDeBuffer(&parametros);
    clave=mFg.ExtraeStringDeBuffer(&parametros);
    objeto=mFg.ExtraeStringDeBuffer(&parametros);
    privilegio=mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Si la clave es correcta para el usuario dado, entonces Regresa el identificador de usuario
	instruccion.sprintf("SELECT * FROM ((SELECT u.empleado AS usuario \
	FROM usuarios u, asignacionprivilegios a \
	WHERE u.empleado='%s' AND u.password='%s' AND a.usuario=u.empleado \
	AND u.activo=1  AND a.objeto='%s' AND a.privilegio='%s') \
	UNION (SELECT u.empleado AS usuario FROM usuarios u \
	INNER JOIN usuariorol ur ON u.empleado = ur.usuario \
	INNER JOIN asignacionprivrol ar ON ar.rol = ur.rol \
	WHERE u.empleado='%s' AND u.password='%s' \
	AND u.activo=1  AND ar.objeto='%s' AND ar.privilegio='%s' \
	) UNION (SELECT u.empleado AS usuario FROM usuarios u \
	INNER JOIN empleados e ON e.empleado = u.empleado \
	INNER JOIN puestos p ON p.puesto = e.puesto \
	INNER JOIN rolesxpuesto rxp ON rxp.puesto = p.puesto \
	INNER JOIN asignacionprivrol ar ON rxp.rol=ar.rol \
	WHERE u.empleado='%s' AND u.password='%s' \
	AND u.activo=1  AND ar.objeto='%s' AND ar.privilegio='%s' \
	)) AS asigtotal ",
	usuario, clave, objeto, privilegio,
	usuario, clave, objeto, privilegio,
	usuario, clave, objeto, privilegio);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}


//---------------------------------------------------------------------------
//ID_QRY_USUARIOS
void ServidorAdminSistema::PideListaUsuarios(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    // CONSULTA USUARIO
    AnsiString instruccion;
	AnsiString clave_usuario;

	clave_usuario=mFg.ExtraeStringDeBuffer(&parametros);
    mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	AnsiString sucursal=FormServidor->ObtieneClaveSucursal();
	//Obtiene todos los usuarios que tengan asignada la sucursal en curso
	instruccion="SELECT u.empleado AS Usuario, CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS Nombre ";
	instruccion+="FROM usuarios u, empleados e INNER JOIN usuariosucursal us ";
	instruccion+="WHERE e.empleado=u.empleado AND us.usuario = u.empleado AND e.activo=1 AND u.activo=1 AND us.sucursal='"+sucursal+"' ";
	instruccion+="ORDER BY nombre ";

	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}

//---------------------------------------------------------------------------
//ID_QRY_FORMAS
void ServidorAdminSistema::PideListaFormas(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA FORMA
    AnsiString instruccion;
	AnsiString clave_forma;

    clave_forma=mFg.ExtraeStringDeBuffer(&parametros);
    mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

    // Obtiene todas las formas
    instruccion="select forma, nombre from formas";
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}


//---------------------------------------------------------------------------
//ID_QRY_ROLES
void ServidorAdminSistema::PideListaRoles(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA ROL
	AnsiString instruccion;
	AnsiString clave_rol;

	clave_rol=mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los roles
	instruccion="select rolessistema.claverol AS Rol, rolessistema.nombre AS Nombre ";
	instruccion+="from rolessistema order by rolessistema.nombre";

	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}


//---------------------------------------------------------------------------
//ID_GRA_CORTE
void ServidorAdminSistema::GrabaCorte(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*64*10];
	char *aux_buffer_sql=buffer_sql;
	int i;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[1000];
	AnsiString archivo_temp1, archivo_temp2, archivo_temp3, archivo_temp4, archivo_temp5, archivo_temp6;
	AnsiString fecha;
	AnsiString usumodi;
	TDate fechamodi=Today();
	TTime horamodi=Time();

	try{
		fecha=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		usumodi=mFg.ExtraeStringDeBuffer(&parametros);


		instruccion.sprintf("set @fechafinal='%s'", fecha);
		instrucciones[num_instrucciones++]=instruccion;

		// Calcula desde el inicio del sistema para evitar errores por modificaciones
		instruccion.sprintf("set @fechacorte='1900-01-01'");
		instrucciones[num_instrucciones++]=instruccion;

		// Crea una tabla donde se pondrán las existencias que se vayan encontrando.
		instruccion="create temporary table existenciastmp ( \
			producto varchar(8), present varchar(255), almacen varchar(4), \
			cantidad decimal(12,3),  \
			INDEX(producto, present, almacen)) Engine = InnoDB";
		instrucciones[num_instrucciones++]=instruccion;

		archivo_temp1 = mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);

		// Calcula las compras
		instruccion.sprintf(" select a.producto, a.present, c.almacen, \
			sum(d.cantidad*a.factor) as cantidad \
			from compras c, dcompras d, articulos a, productos prod \
			where \
			c.fechacom>@fechacorte and c.fechacom<=@fechafinal and c.referencia=d.referencia \
			and c.cancelado=0 and d.articulo=a.articulo and a.producto=prod.producto \
			group by a.producto, a.present, c.almacen INTO OUTFILE '%s' ", archivo_temp1);
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE existenciastmp ", archivo_temp1);
		instrucciones[num_instrucciones++]=instruccion;

		archivo_temp2 = mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);

		// Calcula las devoluciones a las compras
		instruccion.sprintf(" select a.producto, a.present, c.almacen, \
			(sum(d.cantidad*a.factor)*-1) as cantidad \
			from notascredprov n, compras c, dnotascredprov d, articulos a, productos prod \
			where \
			n.fechanot>@fechacorte and n.fechanot<=@fechafinal and n.tipo='0' \
			and n.referencia=d.referencia and n.compra=c.referencia and \
			n.cancelado=0 and d.articulo=a.articulo and a.producto=prod.producto \
			group by a.producto, a.present, c.almacen INTO OUTFILE '%s' ", archivo_temp2);
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE existenciastmp ", archivo_temp2);
		instrucciones[num_instrucciones++]=instruccion;

		archivo_temp3 = mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);

		// Calcula las ventas.
		instruccion.sprintf(" select a.producto, a.present, d.almacen, \
			(sum(d.cantidad*a.factor)*-1) as cantidad \
			from ventas v, dventas d, articulos a, productos prod \
			where \
			v.fechavta>@fechacorte and v.fechavta<=@fechafinal and v.referencia=d.referencia \
			and v.cancelado=0 and d.articulo=a.articulo and a.producto=prod.producto \
			group by a.producto, a.present, d.almacen INTO OUTFILE '%s' ", archivo_temp3);
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE existenciastmp ", archivo_temp3);
		instrucciones[num_instrucciones++]=instruccion;

		archivo_temp4 = mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);

		// Calcula las devoluciones a las ventas
		instruccion.sprintf(" select a.producto, a.present, dv.almacen, \
			sum(dn.cantidad*a.factor) as cantidad \
			from notascredcli n, dnotascredcli dn, ventas v, dventas dv, articulos a, productos prod \
			where \
			n.fechanot>@fechacorte and n.fechanot<=@fechafinal and n.tipo='0' \
			and n.referencia=dn.referencia and n.venta=v.referencia \
			and v.referencia=dv.referencia and \
			n.cancelado=0 and dn.articulo=a.articulo and dv.articulo=dn.articulo and a.producto=prod.producto \
			group by a.producto, a.present, dv.almacen INTO OUTFILE '%s' ", archivo_temp4);
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE existenciastmp ", archivo_temp4);
		instrucciones[num_instrucciones++]=instruccion;

		archivo_temp5 = mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);

		// Calcula las entradas de almacén
		instruccion.sprintf(" select a.producto, a.present, m.almaent, \
			sum(d.cantidad*a.factor) as cantidad \
			from movalma m, dmovalma d, articulos a, productos prod \
			where \
			m.fechamov>@fechacorte and m.fechamov<=@fechafinal and m.movimiento=d.movimiento \
			and m.tipo<>'S' \
			AND m.aplica=1 and m.cancelado=0 and d.articulo=a.articulo and a.producto=prod.producto \
			group by a.producto, a.present, m.almaent INTO OUTFILE '%s' ", archivo_temp5);
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE existenciastmp ", archivo_temp5);
		instrucciones[num_instrucciones++]=instruccion;

		archivo_temp6 = mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);

		// Calcula las salidas de almacén
		instruccion.sprintf(" select a.producto, a.present, m.almasal, \
			(sum(d.cantidad*a.factor)*-1) as cantidad \
			from movalma m, dmovalma d, articulos a, productos prod \
			where \
			m.fechamov>@fechacorte and m.fechamov<=@fechafinal and m.movimiento=d.movimiento \
			and m.tipo<>'E' \
			AND m.aplica=1 and m.cancelado=0 and d.articulo=a.articulo and a.producto=prod.producto \
			group by a.producto, a.present, m.almasal INTO OUTFILE '%s' ", archivo_temp6);
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE existenciastmp ", archivo_temp6);
		instrucciones[num_instrucciones++]=instruccion;

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		// Borra el corte
		instruccion.sprintf("delete from puntoscorteexistencias where fecha='%s'", fecha);
		instrucciones[num_instrucciones++]=instruccion;
		instruccion.sprintf("delete from puntoscorte where fecha='%s'", fecha);
		instrucciones[num_instrucciones++]=instruccion;

		// Graba la cabecera del corte
		instruccion.sprintf("insert into puntoscorte (fecha, usumodi, fechamodi, horamodi) \
			values ('%s', '%s', '%s', '%s')",
			fecha, usumodi, mFg.DateToMySqlDate(fechamodi), mFg.TimeToMySqlTime(horamodi));
		instrucciones[num_instrucciones++]=instruccion;

		// Suma los movimientos para obtener el detalle de las existencias
		instruccion.sprintf("insert into puntoscorteexistencias (fecha, producto, present, almacen, cantidad) \
				select '%s' as fecha, e.producto, e.present, e.almacen, sum(e.cantidad) as cantidad \
				from existenciastmp e \
				group by e.producto, e.present, e.almacen",
				fecha);
		instrucciones[num_instrucciones++]=instruccion;

		instrucciones[num_instrucciones++]="COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_REP);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);
	} __finally {
		delete buffer_sql;

		mServidorVioleta->BorraArchivoTemp(archivo_temp1);
		mServidorVioleta->BorraArchivoTemp(archivo_temp2);
		mServidorVioleta->BorraArchivoTemp(archivo_temp3);
		mServidorVioleta->BorraArchivoTemp(archivo_temp4);
		mServidorVioleta->BorraArchivoTemp(archivo_temp5);
		mServidorVioleta->BorraArchivoTemp(archivo_temp6);
	}
}

//---------------------------------------------------------------------------
//ID_CANC_CORTE
void ServidorAdminSistema::CancelaCorte(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*64*10];
	char *aux_buffer_sql=buffer_sql;
	int i;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[100];
	AnsiString fecha;

	try{
		fecha=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		// Borra el corte
		instruccion.sprintf("delete from puntoscorteexistencias where fecha='%s'", fecha);
		instrucciones[num_instrucciones++]=instruccion;
		instruccion.sprintf("delete from puntoscorte where fecha='%s'", fecha);
		instrucciones[num_instrucciones++]=instruccion;

		instrucciones[num_instrucciones++]="COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_REP);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);
	} __finally {
		delete buffer_sql;
	}
}

//---------------------------------------------------------------------------
//ID_CON_CORTE
void ServidorAdminSistema::ConsultaCorte(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA CORTES
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
	// Obtiene todos los cortes
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, "select fecha from puntoscorte order by fecha desc", Respuesta->TamBufferResultado);
}

//---------------------------------------------------------------------------
//ID_GRA_FORMA
void ServidorAdminSistema::GrabaForma(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  GRABA UNA FORMA
	char *buffer_sql=new char[1024*64*10];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString tarea, clave, usuario;
	int num_partidas, i;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[1000];
	TDate fecha=Today();

	try{
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave de la forma.
		tarea=mFg.ExtraeStringDeBuffer(&parametros); // Indicador si se va a agregar o modificar.
		usuario=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que esta grabando la forma.

		// Obtiene los datos de la tabla de formas
		datos.AsignaTabla("formas");
		parametros+=datos.InsCamposDesdeBuffer(parametros);

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		// Si se está modificando entonces borra el detalle que ya exista.
		if (tarea=="M") {
			instruccion.sprintf("delete from dformas where forma='%s'", clave);
			instrucciones[num_instrucciones++]=instruccion;
		}

		// Graba la cabecera en la tabla "movalma"
		if (tarea=="A") {
			datos.InsCampo("usualta", usuario);
			datos.InsCampo("usumodi", usuario);
			datos.InsCampo("fechaalta", mFg.DateToAnsiString(fecha));
			datos.InsCampo("fechamodi", mFg.DateToAnsiString(fecha));
			instrucciones[num_instrucciones++]=datos.GenerarSqlInsert();
		} else {
			datos.InsCampo("usumodi", usuario);
			datos.InsCampo("fechamodi", mFg.DateToAnsiString(fecha));
			instrucciones[num_instrucciones++]=datos.GenerarSqlUpdate("forma='"+clave+"'");
		}

		// Graba las partidas en "dformas"
		num_partidas=StrToInt(mFg.ExtraeStringDeBuffer(&parametros)); // Obtiene el número de partidas
		for (i=0; i<num_partidas; i++) {
			datos.AsignaTabla("dformas");
			parametros+=datos.InsCamposDesdeBuffer(parametros);
			datos.InsCampo("forma", clave);
			instrucciones[num_instrucciones++]=datos.GenerarSqlInsert();
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

//---------------------------------------------------------------------------
//ID_BAJ_FORMA
void ServidorAdminSistema::BajaForma(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// BAJA FORMA
	char *buffer_sql=new char[1024*5];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instruccion;
	AnsiString clave;
	AnsiString instrucciones[5];
	int num_instrucciones=0;
	int i;

	try{
		clave=mFg.ExtraeStringDeBuffer(&parametros);

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		instruccion.sprintf("delete from dformas where forma='%s'", clave);
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("delete from formas where forma='%s'", clave);
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
//ID_CON_FORMA
void ServidorAdminSistema::ConsultaForma(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA UNA FORMA
    AnsiString instruccion;
    AnsiString clave;

    clave=mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

    // Obtiene todos los datos de la forma
    instruccion.sprintf("select * from formas where forma='%s'", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	// Obtiene todo el detalle de la forma
	instruccion.sprintf("select * from dformas where forma='%s'", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}


//---------------------------------------------------------------------------
//ID_CON_ENVIODATOSPOCKET
void ServidorAdminSistema::EnvioDatosPocket(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	BufferRespuestas* resp_gen=NULL;
	char *buffer_sql=new char[1024*64*10];
	char *aux_buffer_sql=buffer_sql;
	int i, num_partidas;
	AnsiString archivo_temp1="";
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[1000];
	AnsiString fecha, terminal, vendedor;
	AnsiString empresa, sucursal, condicion_sucursal=" ";
	AnsiString almacen, cad_conjunto_almacenes=" ", modo_calcular_existencias, dias_ped_transit_calcular_existencias ;
	AnsiString condicion_almacen=' ';
	BufferRespuestas* resp_almacenes=NULL;

	TDate fecha_hoy=Today();
	AnsiString cad_fecha_hoy=mFg.DateToMySqlDate(fecha_hoy);
	AnsiString cad_fecha_dos_anios=mFg.DateToMySqlDate(IncYear(fecha_hoy, -2));
	AnsiString condicion_fechas_ventassaldo=" v.fechavta between '"+cad_fecha_dos_anios+"' and '"+ cad_fecha_hoy+ "' and ";

	try{
		empresa=FormServidor->ObtieneClaveEmpresa();
		sucursal=FormServidor->ObtieneClaveSucursal();

		fecha=mFg.DateToMySqlDate(Today());

		terminal=mFg.ExtraeStringDeBuffer(&parametros);
		vendedor=mFg.ExtraeStringDeBuffer(&parametros);
		modo_calcular_existencias=mFg.ExtraeStringDeBuffer(&parametros);
		dias_ped_transit_calcular_existencias=mFg.ExtraeStringDeBuffer(&parametros);


        if (dias_ped_transit_calcular_existencias.Length()>1) {
			dias_ped_transit_calcular_existencias="10";
		}else if (dias_ped_transit_calcular_existencias=="")
			dias_ped_transit_calcular_existencias="0";


		// Obtiene los almacenes que corresponden a una sucursal
		if (sucursal!=" ") {
			instruccion.sprintf("SELECT a.almacen FROM almacenes a \
				INNER JOIN secciones s ON a.seccion=s.seccion \
				WHERE s.sucursal='%s'", sucursal);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_almacenes);
			for(i=0; i<resp_almacenes->ObtieneNumRegistros(); i++){
				resp_almacenes->IrAlRegistroNumero(i);
				almacen=resp_almacenes->ObtieneDato("almacen");

				cad_conjunto_almacenes+="'";
				cad_conjunto_almacenes+=almacen;
				cad_conjunto_almacenes+="'";
				if (i<resp_almacenes->ObtieneNumRegistros()-1) // A todos menos al ultimo impuesto le signo +.
					cad_conjunto_almacenes+=",";
			}
		}

		instruccion.sprintf("set @fechafinal='%s'", fecha);
		instrucciones[num_instrucciones++]=instruccion;

		// Obtiene fecha del corte más próximo previo a la fecha dada
		instruccion.sprintf("set @fechacorte='1900-01-01'");
		/*instruccion.sprintf("select @fechacorte:=ifnull(max(p.fecha),'1900-01-01') as fcorte \
			 from puntoscorte p where p.fecha<='%s'", fecha);*/
		instrucciones[num_instrucciones++]=instruccion;

		// Crea una tabla donde se pondrán las existencias que se vayan encontrando.
		instruccion="create temporary table existenciasaux ( \
			producto varchar(8), present varchar(255), \
			tipo char(2), cantidad decimal(12,3), \
			INDEX(producto, present)) Engine = InnoDB";
		instrucciones[num_instrucciones++]=instruccion;

		// Crea una tabla donde se pondrán las existencias finales
		instruccion="create temporary table auxexistsumadas ( \
			producto varchar(8), present varchar(255), \
			cantidad decimal(12,3), \
			INDEX(producto, present)) Engine = InnoDB";
		instrucciones[num_instrucciones++]=instruccion;

		if(modo_calcular_existencias == "MODN"){
            throw Exception("Configuración de modo de calculo para revisión de existencias está incorrecto MODN");
		}
			else
				if(modo_calcular_existencias == "MODEA"){

					if (cad_conjunto_almacenes!=" ") {
						condicion_almacen.sprintf(" WHERE almacen in (%s) ", cad_conjunto_almacenes);
					} else condicion_almacen=" ";


                    archivo_temp1 = mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);

					// Calcula la CANTIDAD con tabla de existencias actuales
					instruccion.sprintf(" SELECT producto,present,'EX',SUM(cantidad) as cantidad FROM \
						existenciasactuales %s group by producto, present INTO OUTFILE '%s' ",
						condicion_almacen, archivo_temp1 );
					instrucciones[num_instrucciones++]=instruccion;

					instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE existenciasaux (producto, present, tipo, cantidad) ",
					archivo_temp1);
					instrucciones[num_instrucciones++] = instruccion;

					// Calcula los pedidos en transito para restarlos de existencias reales
					instruccion.sprintf("insert into existenciasaux (producto, present, tipo, cantidad) \
						select  a.producto, a.present,'PT', SUM(d.cantidad*a.factor)*-1 AS cantidad from dpedidosventa as d \
						inner join pedidosventa as p on p.referencia=d.referencia \
						inner join articulos as a on a.articulo=d.articulo \
						inner join terminales as t on t.terminal=p.terminal \
						inner join secciones as s on s.seccion=t.seccion \
						where p.facturado=0 and p.cancelado=0 and p.cotizacion=0 and p.fechaped>(INTERVAL -%s DAY +'%s') \
						and s.sucursal='%s' \
						group by a.producto, a.present",
						dias_ped_transit_calcular_existencias, cad_fecha_hoy,	sucursal);
					instrucciones[num_instrucciones++]=instruccion;


				}

		// Suma los movimientos para obtener las existencias
		instruccion.sprintf("insert into auxexistsumadas (producto, present, cantidad) \
				select e.producto, e.present, sum(e.cantidad) as cantidad \
				from existenciasaux e \
				group by e.producto, e.present");
		instrucciones[num_instrucciones++]=instruccion;

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_REP);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			// Resultado final

			// Manda parámetros
			instruccion.sprintf("select parametro, left(descripcion,40) as descripcion, valor \
				from parametrosemp WHERE 1 AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Manda articulos
			instruccion.sprintf("select a.articulo, a.producto, a.present, a.multiplo, a.factor, a.porccomi, e.cantidad, a.activo, \
			IF(ae.tipoutil=5,'5',IF(ae.tipoutil=4,'4',IF(ae.tipoutil='3','3',IF(ae.tipoutil=2,'2',IF(ae.tipoutil=1,'1',0))))) AS utilesp, \
			a.peso, a.volumen \
				from articulos a \
				inner join articulosemp ae ON ae.articulo=a.articulo and ae.idempresa=%s \
				left join auxexistsumadas e on e.producto=a.producto and e.present=a.present ",
				FormServidor->ObtieneClaveEmpresa() );
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Manda CLIENTES
			instruccion.sprintf("select * from(( \
			select u.empleado, ar.objeto, ar.privilegio from usuarios u \
			inner join usuariorol ur on ur.usuario = u.empleado inner join asignacionprivrol ar on ur.rol=ar.rol ) \
			union ( select a.usuario AS empleado, a.objeto, a.privilegio from asignacionprivilegios a ) \
			union(SELECT u.empleado, ar.objeto, ar.privilegio FROM usuarios u \
			INNER JOIN empleados e ON e.empleado = u.empleado INNER JOIN puestos p ON p.puesto = e.puesto \
			INNER JOIN rolesxpuesto rxp ON rxp.puesto = p.puesto \
			INNER JOIN asignacionprivrol ar ON rxp.rol=ar.rol)) as asigprivtotal \
			where asigprivtotal.empleado='%s' and asigprivtotal.objeto='SINCPOCKET' and asigprivtotal.privilegio='CRE' ",vendedor);

			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_gen);
			if (resp_gen->ObtieneNumRegistros()!=0) {
				instruccion.sprintf("select c.cliente, LEFT(c.rsocial,100) as rsocial, c.rfc, \
				left(concat(c.calle, ' #', c.numext,' -', c.numint, ', ', col.nombre, ' CP:', c.cp, ' ', loc.nombre),100) as domicilio, \
				c.bloqueo, c.credito, c.limcred, sum(ifnull(s.saldo,0)) as saldo, sum(ifnull(p.pedxfact,0)) as pedxfact, c.excederlc, c.plazo, ce.tipoprec \
				from clientes c \
				left join clientesemp ce ON ce.cliente = c.cliente AND ce.idempresa=%s \
				left join colonias col on col.colonia=c.colonia \
				left join localidades loc on col.localidad=loc.localidad \
				left join ( \
					SELECT c.cliente, SUM(t.valor) AS saldo \
					FROM clientes c \
						INNER JOIN ventas v ON v.cliente=c.cliente \
						INNER JOIN transxcob t ON t.referencia = v.referencia \
					WHERE t.cancelada=0 AND v.cancelado=0 AND %s \
					t.aplicada=1 AND \
					(excederlc = 0)  AND c.activo=1 AND \
                     v.referencia not in(select referencia from ventascongeladas where activo=1) \
					GROUP BY v.cliente \
					ORDER BY v.cliente \
				) s on s.cliente=c.cliente \
				left join ( \
					SELECT c.cliente, sum(p.valor) as pedxfact FROM pedidosventa p \
					inner join clientes c on c.cliente=p.cliente \
					where p.cancelado=0 and p.facturado=0 and p.acredito=1 \
					and p.fechaped>=DATE_SUB(CURDATE(), INTERVAL 5 DAY) \
					group by c.cliente \
				) p on p.cliente=c.cliente \
				where ( \
				excederlc = 0 \
				) and c.activo=1 \
				group by c.cliente \
				order by c.rsocial ", empresa, condicion_fechas_ventassaldo);
			} else {
				instruccion.sprintf("select c.cliente, LEFT(c.rsocial,100) as rsocial, c.rfc, \
				left(concat(c.calle, ' #', c.numext,' -', c.numint, ', ', col.nombre, ' CP:', c.cp, ' ', loc.nombre),100) as domicilio, \
				c.bloqueo, c.credito, c.limcred, sum(ifnull(s.saldo,0)) as saldo, sum(ifnull(p.pedxfact,0)) as pedxfact, c.excederlc, c.plazo, ce.tipoprec \
				from clientes c \
				left join clientesemp ce ON ce.cliente = c.cliente AND ce.idempresa=%s \
				left join colonias col on col.colonia=c.colonia \
				left join localidades loc on col.localidad=loc.localidad \
				left join ( \
					select c.cliente, sum(t.valor) as saldo \
					FROM clientes c \
						INNER JOIN ventas v ON v.cliente=c.cliente \
						INNER JOIN transxcob t ON t.referencia = v.referencia \
						LEFT JOIN clientesemp cemp ON c.cliente = cemp.cliente AND cemp.idempresa=%s \
					where t.cancelada=0 and v.cancelado=0 and %s \
					t.aplicada=1 and \
					(cemp.vendedor='%s' or cemp.cobrador='%s') and c.activo=1 AND \
					v.referencia not in(select referencia from ventascongeladas where activo=1) \
					group by v.cliente \
				) s on s.cliente=c.cliente \
				left join ( \
					SELECT c.cliente, sum(p.valor) as pedxfact FROM pedidosventa p \
					inner join clientes c on c.cliente=p.cliente \
					LEFT JOIN clientesemp cemp ON c.cliente = cemp.cliente AND cemp.idempresa= %s \
					where p.cancelado=0 and p.facturado=0 and p.acredito=1 and p.vendedor='%s' \
					and p.fechaped>=DATE_SUB(CURDATE(), INTERVAL 5 DAY) \
					group by c.cliente \
				) p on p.cliente=c.cliente \
				where (ce.vendedor='%s' or ce.cobrador='%s') and c.activo=1 \
				group by c.cliente \
				order by c.rsocial ", empresa,
                empresa,
				condicion_fechas_ventassaldo,
				vendedor, vendedor,
                empresa,
				vendedor, vendedor, vendedor);
			}
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
			/*
			// Manda DIRECCIONES DE ENTREGA CLIENTES
			if (resp_gen->ObtieneNumRegistros()!=0) {
				instruccion.sprintf("select c.cliente,c.rsocial, 0 as iddireccion, \
				case when (c.cliente not in(select cliente from direccionesentregaclientes)) then 1 else 0 end as dafault, \
				left(concat(c.calle, ' #', c.numext, if (c.numint<>'' , ' -' , '' ), c.numint, ', ', col.nombre, \
					' CP:', c.cp, ' ', loc.nombre),100) as domicilio \
					from clientes as c \
						left join colonias col on col.colonia=c.colonia \
						left join localidades loc on col.localidad=loc.localidad \
					where (excederlc = 0) and c.activo=1 \
					UNION ALL \
					select c.cliente,c.rsocial, dc.iddireccion, dc.dafault , \
				left(concat(dc.calle, ' #', dc.numext,if (dc.numint<>'' , ' -' , '' ), dc.numint, ', ', col.nombre, \
				' CP:', dc.cp, ' ', loc.nombre),100) as domicilio \
					from clientes as c \
						inner join  direccionesentregaclientes  as dc on dc.cliente=c.cliente \
						left join colonias col on col.colonia=dc.colonia \
						left join localidades loc on col.localidad=loc.localidad \
					where (excederlc = 0 ) and c.activo=1 \
					order by rsocial,iddireccion");
					mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
			}else{
				instruccion.sprintf("select c.cliente,c.rsocial, 0 as iddireccion, \
				case when (c.cliente not in(select cliente from direccionesentregaclientes)) then 1 else 0 end as dafault, \
				left(concat(c.calle, ' #', c.numext,if (c.numint<>'' , ' -' , '' ), c.numint, ', ', col.nombre, \
				' CP:', c.cp, ' ', loc.nombre),100) as domicilio \
				from clientes as c \
					left join colonias col on col.colonia=c.colonia \
					left join localidades loc on col.localidad=loc.localidad \
				where (c.vendedor='%s' or c.cobrador='%s') and c.activo=1 \
				UNION ALL \
				select c.cliente,c.rsocial, dc.iddireccion, dc.dafault , \
				left(concat(dc.calle, ' #', dc.numext,if (dc.numint<>'' , ' -' , '' ), dc.numint, ', ', col.nombre, \
				' CP:', dc.cp, ' ', loc.nombre),100) as domicilio \
				from clientes as c \
					inner join  direccionesentregaclientes  as dc on dc.cliente=c.cliente \
					left join colonias col on col.colonia=dc.colonia \
					left join localidades loc on col.localidad=loc.localidad \
				where (c.vendedor='%s' or c.cobrador='%s') and c.activo=1 \
				order by rsocial,iddireccion",vendedor,vendedor,vendedor,vendedor);
				mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			}
             */
			// Manda PRECIOS
			instruccion.sprintf("select p.articulo,p.tipoprec,p.precio \
				from precios p, tiposdeprecios t \
				where p.tipoprec=t.tipoprec and t.listamovil=1 and p.precio>=.01 and t.idempresa=%s ",
				FormServidor->ObtieneClaveEmpresa());
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Manda PRESENTACIONES
			instruccion.sprintf("select pre.producto, pre.present, pcb.costobase, \
				pre.modoutil, max(a.activo) as activo \
				from presentaciones pre, presentacionescb pcb, articulos a \
				where pre.producto=a.producto and pre.present=a.present and \
				 pcb.producto=pre.producto and pcb.present=pre.present and pcb.idempresa=%s \
				group by pre.producto, pre.present", FormServidor->ObtieneClaveEmpresa());
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Manda PRODUCTOS
			instruccion.sprintf("select pro.producto, pro.nombre, pro.marca, \
				max(a.activo) as activo \
				from productos pro, presentaciones pre, articulos a \
				where pro.producto=pre.producto and pre.producto=a.producto \
				and pre.present=a.present \
				group by pro.producto");
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Manda MARCAS
			instruccion.sprintf("select marca, nombre \
				from marcas");
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Manda TERMINOSDEPAGO
			instruccion.sprintf("select termino, descripcion, diasdefoult, terminoreal \
				from terminosdepago");
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Manda TIPOSDEPRECIOS
			instruccion.sprintf("select t.tipoprec, t.descripcion \
				from tiposdeprecios t where t.listamovil=1 and t.idempresa=%s ", FormServidor->ObtieneClaveEmpresa());
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Manda TIPOSFACTURASVENTAS
			instruccion.sprintf("select tipo, tiporfc, descripcion \
				from tiposfacturasventas");
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Manda MENSAJES
			// '%Y%m%d %H:%i:%s'
			instruccion="select mensaje, remitente, \
				concat(nombre, ' ', appat, ' ', apmat) as nomremit, \
				date_format(fechaenvio, '%Y%m%d %H:%i:%s') as fechaenvio, \
				date_format(horaenvio, '20000101 %H:%i:%s') as horaenvio, \
				urgente, leido, contenido, asunto \
				from mensajes, empleados \
				where empleados.empleado=remitente and destmovil=1 and enviadomovil=0";
			instruccion=instruccion+" and destino='";
			instruccion=instruccion+vendedor;
			instruccion=instruccion+"'";
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Manda SALDOS
			// Por optimización serán solo saldos de ventas de los últimos dos años
			// Ya que si tiene saldo de antes ya debería estar cancelado su crédito.

			instruccion="CREATE TEMPORARY TABLE auxventassaldos (venta CHAR(11), saldor DECIMAL(16,2),   chcnc DECIMAL(16,2), tncredito DECIMAL(16,2), tncargo DECIMAL(16,2), fechach DATE, PRIMARY KEY (venta)) Engine = INNODB";
			mServidorVioleta->EjecutaSelectSqlNulo(Respuesta,  MySQL, instruccion.c_str());

			instruccion.sprintf("INSERT INTO auxventassaldos ( \
				  venta, \
				  saldor, \
				  chcnc, \
				  tncredito, \
				  tncargo, \
				  fechach \
				) \
				SELECT \
				  v.referencia AS venta, \
				  SUM(IF(t.aplicada = 1, t.valor, 0)) AS saldor, \
				  SUM(IF(t.aplicada = 0, t.valor, 0)) AS chcnc, \
				  SUM( \
					IF( \
					  t.aplicada = 1 \
					  AND t.tipo = 'DEVO', \
					  t.valor, \
					  0 \
					) \
				  ) AS tncredito, \
				  SUM( \
					IF( \
					  t.aplicada = 1 \
					  AND (t.tipo = 'NCAR' \
						OR t.tipo = 'INTE'), \
					  t.valor, \
					  0 \
					) \
				  ) AS tncargo, \
				  MAX( \
					IFNULL( \
					  chcl.fechacob, \
					  CAST('1900-01-01' AS DATE) \
					) \
				  ) AS fechach \
				FROM \
				  ventas v \
				  INNER JOIN \
				  transxcob t \
				  LEFT JOIN \
				  pagoscli p \
				  ON t.pago = p.pago \
				  AND t.aplicada = 0 \
				  LEFT JOIN \
				  cheqxcob chxc \
				  ON p.pago = chxc.pago \
				  LEFT JOIN \
				  chequesclientes chcl \
				  ON chxc.chequecli = chcl.chequecli \
				WHERE t.referencia = v.referencia \
				  AND v.fechavta between '%s' and '%s' AND \
				  v.referencia NOT IN(select referencia from ventascongeladas where activo=1) \
				  AND t.cancelada = 0 \
				  AND v.cancelado = 0 AND v.acredito = 1 \
				  AND v.cliente IN ( \
				  SELECT cli.cliente FROM clientes cli INNER JOIN clientesemp cemp ON cli.cliente = cemp.cliente \
				  WHERE (cemp.vendedor='%s' OR cemp.cobrador='%s') \
				  AND cli.activo=1 ) \
				GROUP BY v.referencia",
				mFg.DateToMySqlDate(IncMonth(Today(),-24)),
				mFg.DateToMySqlDate(Today()),
				vendedor,vendedor);
			mServidorVioleta->EjecutaSelectSqlNulo(Respuesta,  MySQL, instruccion.c_str());

			// Manda SALDOS
			instruccion.sprintf("SELECT \
				  v.referencia, \
				  IF( \
					IFNULL(v.foliofisic, '') = '', \
					CONCAT( \
					  IFNULL(cfd.serie, ''), \
					  IFNULL(cfd.folio, '') \
					), \
					v.foliofisic \
				  ) AS foliofisic, \
				  v.cliente AS cliente, \
				  f.termino, \
				  v.tipofac, \
				  vs.saldor AS saldo, \
				  (vs.chcnc * - 1) AS chnocob, \
				  (vs.tncredito * - 1) AS tncredito, \
				  vs.tncargo, \
				  vs.fechach, \
				  v.fechavta, \
				  v.horaalta, \
				  v.fechavenc, \
				  v.valor \
				FROM \
				  ventas v \
				  INNER JOIN \
				  auxventassaldos vs \
				  INNER JOIN dventasfpago d \
				  ON d.referencia = v.referencia \
				  AND d.valor = (SELECT MAX(d2.valor) FROM dventasfpago d2 WHERE d2.referencia = v.referencia LIMIT 1 ) \
				  INNER JOIN formasdepago f ON d.formapag = f.formapag \
				  LEFT JOIN \
				  cfd \
				  ON v.referencia = cfd.referencia \
				  AND cfd.tipocomprobante = 'VENT' \
                  AND v.referencia NOT IN(select referencia from ventascongeladas where activo=1) \
				WHERE vs.venta = v.referencia \
				  AND vs.saldor > 0 \
				ORDER BY v.referencia ");
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

		}
	} __finally {
		if (resp_gen!=NULL) delete resp_gen;
		if (resp_almacenes!=NULL) delete resp_almacenes;
		delete buffer_sql;
		if (archivo_temp1 != "")
			mServidorVioleta->BorraArchivoTemp(archivo_temp1);

	}
}
//---------------------------------------------------------------------------

//ID_CON_ENVIODATOSPOCKET_NEW
void ServidorAdminSistema::EnvioDatosPocketNew(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	BufferRespuestas* resp_gen=NULL;
	char *buffer_sql=new char[1024*64*10];
	char *aux_buffer_sql=buffer_sql;
	int i, num_partidas;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[1000];
	AnsiString fecha, terminal, vendedor;
	AnsiString archivo_temp1, archivo_temp2, archivo_temp3, archivo_temp4, archivo_temp5;
	AnsiString empresa, sucursal, condicion_sucursal=" ";
	AnsiString almacen, cad_conjunto_almacenes=" ", modo_calcular_existencias, dias_ped_transit_calcular_existencias ;
	AnsiString condicion_almacen=' ';
	BufferRespuestas* resp_almacenes=NULL;

	TDate fecha_hoy=Today();
	AnsiString cad_fecha_hoy=mFg.DateToMySqlDate(fecha_hoy);
	AnsiString cad_fecha_dos_anios=mFg.DateToMySqlDate(IncYear(fecha_hoy, -2));
	AnsiString condicion_fechas_ventassaldo=" v.fechavta between '"+cad_fecha_dos_anios+"' and '"+ cad_fecha_hoy+ "' and ";

	AnsiString archivo_temp_1;

	try{
		empresa=FormServidor->ObtieneClaveEmpresa();
		sucursal=FormServidor->ObtieneClaveSucursal();

		fecha=mFg.DateToMySqlDate(Today());

		terminal=mFg.ExtraeStringDeBuffer(&parametros);
		vendedor=mFg.ExtraeStringDeBuffer(&parametros);
		modo_calcular_existencias=mFg.ExtraeStringDeBuffer(&parametros);
		dias_ped_transit_calcular_existencias=mFg.ExtraeStringDeBuffer(&parametros);


        if (dias_ped_transit_calcular_existencias.Length()>1) {
			dias_ped_transit_calcular_existencias="10";
		}else if (dias_ped_transit_calcular_existencias=="")
			dias_ped_transit_calcular_existencias="0";


		// Obtiene los almacenes que corresponden a una sucursal
		if (sucursal!=" ") {
			instruccion.sprintf("SELECT a.almacen FROM almacenes a \
				INNER JOIN secciones s ON a.seccion=s.seccion \
				WHERE s.sucursal='%s'", sucursal);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_almacenes);
			for(i=0; i<resp_almacenes->ObtieneNumRegistros(); i++){
				resp_almacenes->IrAlRegistroNumero(i);
				almacen=resp_almacenes->ObtieneDato("almacen");

				cad_conjunto_almacenes+="'";
				cad_conjunto_almacenes+=almacen;
				cad_conjunto_almacenes+="'";
				if (i<resp_almacenes->ObtieneNumRegistros()-1) // A todos menos al ultimo impuesto le signo +.
					cad_conjunto_almacenes+=",";
			}
		}

		instruccion.sprintf("set @fechafinal='%s'", fecha);
		instrucciones[num_instrucciones++]=instruccion;

		// Obtiene fecha del corte más próximo previo a la fecha dada
		instruccion.sprintf("set @fechacorte='1900-01-01'");
		/*instruccion.sprintf("select @fechacorte:=ifnull(max(p.fecha),'1900-01-01') as fcorte \
			 from puntoscorte p where p.fecha<='%s'", fecha);*/
		instrucciones[num_instrucciones++]=instruccion;

		// Crea una tabla donde se pondrán las existencias que se vayan encontrando.
		instruccion="create temporary table existenciasaux ( \
			producto varchar(8), present varchar(255), \
			tipo char(2), cantidad decimal(12,3), \
			INDEX(producto, present)) Engine = InnoDB";
		instrucciones[num_instrucciones++]=instruccion;

		// Crea una tabla donde se pondrán las existencias finales
		instruccion="create temporary table auxexistsumadas ( \
			producto varchar(8), present varchar(255), \
			cantidad decimal(12,3), \
			INDEX(producto, present)) Engine = InnoDB";
		instrucciones[num_instrucciones++]=instruccion;

		if(modo_calcular_existencias == "MODN"){
            throw Exception("Configuración de modo de calculo para revisión de existencias está incorrecto MODN");
		}
			else
				if(modo_calcular_existencias == "MODEA"){

					if (cad_conjunto_almacenes!=" ") {
						condicion_almacen.sprintf(" WHERE almacen in (%s) ", cad_conjunto_almacenes);
					} else condicion_almacen=" ";

					/* Calcula la CANTIDAD con tabla de existencias actuales
					instruccion.sprintf("insert into existenciasaux (producto, present, tipo, cantidad)\
						SELECT producto,present,'EX',SUM(cantidad) as cantidad FROM \
						existenciasactuales %s group by producto, present ",
						condicion_almacen);
					instrucciones[num_instrucciones++]=instruccion;  */

					archivo_temp_1=mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);

					// Calcula la CANTIDAD con tabla de existencias actuales
					instruccion.sprintf(" SELECT producto,present,'EX',SUM(cantidad) as cantidad FROM \
						existenciasactuales %s group by producto, present INTO OUTFILE '%s' ",
						condicion_almacen, archivo_temp_1);
					instrucciones[num_instrucciones++]=instruccion;

					instruccion.sprintf(" LOAD DATA INFILE '%s' INTO TABLE existenciasaux (producto, present, tipo, cantidad) ",
					archivo_temp_1);
					instrucciones[num_instrucciones++]=instruccion;

					// Calcula los pedidos en transito para restarlos de existencias reales
					instruccion.sprintf("insert into existenciasaux (producto, present, tipo, cantidad) \
						select  a.producto, a.present,'PT', SUM(d.cantidad*a.factor)*-1 AS cantidad from dpedidosventa as d \
						inner join pedidosventa as p on p.referencia=d.referencia \
						inner join articulos as a on a.articulo=d.articulo \
						inner join terminales as t on t.terminal=p.terminal \
						inner join secciones as s on s.seccion=t.seccion \
						where p.facturado=0 and p.cancelado=0 and p.cotizacion=0 and p.fechaped>(INTERVAL -%s DAY +'%s') \
						and s.sucursal='%s' \
						group by a.producto, a.present",
						dias_ped_transit_calcular_existencias, cad_fecha_hoy,	sucursal);
					instrucciones[num_instrucciones++]=instruccion;


				}

		// Suma los movimientos para obtener las existencias
		instruccion.sprintf("insert into auxexistsumadas (producto, present, cantidad) \
				select e.producto, e.present, sum(e.cantidad) as cantidad \
				from existenciasaux e \
				group by e.producto, e.present");
		instrucciones[num_instrucciones++]=instruccion;

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_REP);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			// Resultado final

			// Manda parámetros
			instruccion.sprintf("select parametro, left(descripcion,40) as descripcion, valor \
				from parametrosemp  WHERE 1 AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Manda articulos
			instruccion.sprintf("select a.articulo, a.producto, a.present, a.multiplo, a.factor, a.porccomi, e.cantidad, a.activo, \
			IF(ae.tipoutil=5,'5',IF(ae.tipoutil=4,'4',IF(ae.tipoutil='3','3',IF(ae.tipoutil=2,'2',IF(ae.tipoutil=1,'1',0))))) AS utilesp, \
			a.peso, a.volumen \
				from articulos a \
				inner join articulosemp ae ON ae.articulo=a.articulo and ae.idempresa=%s \
				left join auxexistsumadas e on e.producto=a.producto and e.present=a.present ",
				FormServidor->ObtieneClaveEmpresa());
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Manda CLIENTES
			instruccion.sprintf("select * from(( \
			select u.empleado, ar.objeto, ar.privilegio from usuarios u \
			inner join usuariorol ur on ur.usuario = u.empleado inner join asignacionprivrol ar on ur.rol=ar.rol ) \
			union ( select a.usuario AS empleado, a.objeto, a.privilegio from asignacionprivilegios a ) UNION ( \
			SELECT u.empleado, ar.objeto, ar.privilegio FROM usuarios u INNER JOIN empleados e ON e.empleado = u.empleado \
			INNER JOIN puestos p ON p.puesto = e.puesto INNER JOIN rolesxpuesto rxp ON rxp.puesto = p.puesto \
			INNER JOIN asignacionprivrol ar ON rxp.rol=ar.rol)) as asigprivtotal \
			where asigprivtotal.empleado='%s' and asigprivtotal.objeto='SINCPOCKET' and asigprivtotal.privilegio='CRE' ",vendedor);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_gen);
			if (resp_gen->ObtieneNumRegistros()!=0) {
				instruccion.sprintf("select c.cliente, LEFT(c.rsocial,100) as rsocial, c.rfc, \
				left(concat(c.calle, ' #', c.numext,' -', c.numint, ', ', col.nombre, ' CP:', c.cp, ' ', loc.nombre),100) as domicilio, \
				c.bloqueo, c.credito, c.limcred, sum(ifnull(s.saldo,0)) as saldo, sum(ifnull(p.pedxfact,0)) as pedxfact, c.excederlc, c.plazo, ce.tipoprec \
				from clientes c \
				left join clientesemp ce on ce.cliente=c.cliente and ce.idempresa=%s \
				left join colonias col on col.colonia=c.colonia \
				left join localidades loc on col.localidad=loc.localidad \
				left join ( \
					SELECT c.cliente, SUM(t.valor) AS saldo \
					FROM clientes c \
						INNER JOIN ventas v ON v.cliente=c.cliente \
						INNER JOIN transxcob t ON t.referencia = v.referencia \
					WHERE t.cancelada=0 AND v.cancelado=0 AND %s \
					t.aplicada=1 AND \
					(excederlc = 0)  AND c.activo=1 AND \
					 v.referencia not in(select referencia from ventascongeladas where activo=1) \
					GROUP BY v.cliente \
					ORDER BY v.cliente \
				) s on s.cliente=c.cliente \
				left join ( \
					SELECT c.cliente, sum(p.valor) as pedxfact FROM pedidosventa p \
					inner join clientes c on c.cliente=p.cliente \
					where p.cancelado=0 and p.facturado=0 and p.acredito=1 \
					and p.fechaped>=DATE_SUB(CURDATE(), INTERVAL 5 DAY) \
					group by c.cliente \
				) p on p.cliente=c.cliente \
				where ( \
				excederlc = 0 \
				) and c.activo=1 \
				group by c.cliente \
				order by c.rsocial ", empresa, condicion_fechas_ventassaldo);
			} else {
				instruccion.sprintf("select c.cliente, LEFT(c.rsocial,100) as rsocial, c.rfc, \
				left(concat(c.calle, ' #', c.numext,' -', c.numint, ', ', col.nombre, ' CP:', c.cp, ' ', loc.nombre),100) as domicilio, \
				c.bloqueo, c.credito, c.limcred, sum(ifnull(s.saldo,0)) as saldo, sum(ifnull(p.pedxfact,0)) as pedxfact, c.excederlc, c.plazo, ce.tipoprec \
				from clientes c \
				left join clientesemp ce on ce.cliente=c.cliente and ce.idempresa=%s \
				left join colonias col on col.colonia=c.colonia \
				left join localidades loc on col.localidad=loc.localidad \
				left join ( \
					select c.cliente, sum(t.valor) as saldo \
					FROM clientes c \
						INNER JOIN ventas v ON v.cliente=c.cliente \
						INNER JOIN transxcob t ON t.referencia = v.referencia \
						LEFT JOIN clientesemp ce ON c.cliente = ce.cliente and ce.idempresa=%s \
					where t.cancelada=0 and v.cancelado=0 and %s \
					t.aplicada=1 and \
					(ce.vendedor='%s' or ce.cobrador='%s') and c.activo=1 AND \
					v.referencia not in(select referencia from ventascongeladas where activo=1) \
					group by v.cliente \
				) s on s.cliente=c.cliente \
				left join ( \
					SELECT c.cliente, sum(p.valor) as pedxfact FROM pedidosventa p \
					inner join clientes c on c.cliente=p.cliente \
					where p.cancelado=0 and p.facturado=0 and p.acredito=1 and p.vendedor='%s' \
					and p.fechaped>=DATE_SUB(CURDATE(), INTERVAL 5 DAY) \
					group by c.cliente \
				) p on p.cliente=c.cliente \
				where (ce.vendedor='%s' or ce.cobrador='%s') and c.activo=1 \
				group by c.cliente \
				order by c.rsocial ", empresa,
				empresa,
				condicion_fechas_ventassaldo,
				vendedor, vendedor, vendedor, vendedor, vendedor);
			}
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Manda DIRECCIONES DE ENTREGA CLIENTES
			if (resp_gen->ObtieneNumRegistros()!=0) {
				instruccion.sprintf("select c.cliente,LEFT(c.rsocial,100) as rsocial, 0 as iddireccion, \
				case when (c.cliente not in(select cliente from direccionesentregaclientes)) then 1 else 0 end as dafault, \
				left(concat(c.calle, ' #', c.numext, if (c.numint<>'' , ' -' , '' ), c.numint, ', ', col.nombre, \
					' CP:', c.cp, ' ', loc.nombre),100) as domicilio \
					from clientes as c \
						left join colonias col on col.colonia=c.colonia \
						left join localidades loc on col.localidad=loc.localidad \
					where (excederlc = 0) and c.activo=1 \
					UNION ALL \
					select c.cliente,c.rsocial, dc.iddireccion, dc.dafault , \
				left(concat(dc.calle, ' #', dc.numext,if (dc.numint<>'' , ' -' , '' ), dc.numint, ', ', col.nombre, \
				' CP:', dc.cp, ' ', loc.nombre),100) as domicilio \
					from clientes as c \
						inner join  direccionesentregaclientes  as dc on dc.cliente=c.cliente \
						left join colonias col on col.colonia=dc.colonia \
						left join localidades loc on col.localidad=loc.localidad \
					where (excederlc = 0 ) and c.activo=1 \
					order by rsocial,iddireccion");
					mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
			}else{
				instruccion.sprintf("select c.cliente,LEFT(c.rsocial,100) as rsocial, 0 as iddireccion, \
				case when (c.cliente not in(select cliente from direccionesentregaclientes)) then 1 else 0 end as dafault, \
				left(concat(c.calle, ' #', c.numext,if (c.numint<>'' , ' -' , '' ), c.numint, ', ', col.nombre, \
				' CP:', c.cp, ' ', loc.nombre),100) as domicilio \
				from clientes as c \
					left join colonias col on col.colonia=c.colonia \
					left join localidades loc on col.localidad=loc.localidad \
					LEFT JOIN clientesemp cemp ON c.cliente = cemp.cliente AND cemp.idempresa= %s \
				where (cemp.vendedor='%s' or cemp.cobrador='%s') and c.activo=1 \
				UNION ALL \
				select c.cliente,c.rsocial, dc.iddireccion, dc.dafault , \
				left(concat(dc.calle, ' #', dc.numext,if (dc.numint<>'' , ' -' , '' ), dc.numint, ', ', col.nombre, \
				' CP:', dc.cp, ' ', loc.nombre),100) as domicilio \
				from clientes as c \
					inner join  direccionesentregaclientes  as dc on dc.cliente=c.cliente \
					left join colonias col on col.colonia=dc.colonia \
					left join localidades loc on col.localidad=loc.localidad \
					LEFT JOIN clientesemp cemp ON c.cliente=cemp.cliente AND cemp.idempresa=%s \
				where (cemp.vendedor='%s' or cemp.cobrador='%s') and c.activo=1 \
				order by rsocial,iddireccion",FormServidor->ObtieneClaveEmpresa(),
				vendedor,vendedor,
				FormServidor->ObtieneClaveEmpresa(),
				vendedor,vendedor);
				mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			}

			// Manda PRECIOS
			instruccion.sprintf("select p.articulo,p.tipoprec,p.precio \
				from precios p, tiposdeprecios t \
				where p.tipoprec=t.tipoprec and t.listamovil=1 and p.precio>=.01 and t.idempresa=%s ", FormServidor->ObtieneClaveEmpresa() );
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Manda PRESENTACIONES
			instruccion.sprintf("select pre.producto, pre.present, pcb.costobase, \
				pre.modoutil, max(a.activo) as activo \
				from presentaciones pre, presentacionescb pcb, articulos a \
				where pre.producto=a.producto and pre.present=a.present \
				and pcb.producto=pre.producto and pcb.present=pre.present and pcb.idempresa=%s \
				group by pre.producto, pre.present", FormServidor->ObtieneClaveEmpresa());
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Manda PRODUCTOS
			instruccion.sprintf("select pro.producto, pro.nombre, pro.marca, \
				max(a.activo) as activo \
				from productos pro, presentaciones pre, articulos a \
				where pro.producto=pre.producto and pre.producto=a.producto \
				and pre.present=a.present \
				group by pro.producto");
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Manda MARCAS
			instruccion.sprintf("select marca, nombre \
				from marcas");
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Manda TERMINOSDEPAGO
			instruccion.sprintf("select termino, descripcion, diasdefoult, terminoreal \
				from terminosdepago");
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Manda TIPOSDEPRECIOS
			instruccion.sprintf("select t.tipoprec, t.descripcion \
				from tiposdeprecios t where t.listamovil=1 and t.idempresa=%s ", FormServidor->ObtieneClaveEmpresa());
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Manda TIPOSFACTURASVENTAS
			instruccion.sprintf("select tipo, tiporfc, descripcion \
				from tiposfacturasventas");
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Manda MENSAJES
			// '%Y%m%d %H:%i:%s'
			instruccion="select mensaje, remitente, \
				concat(nombre, ' ', appat, ' ', apmat) as nomremit, \
				date_format(fechaenvio, '%Y%m%d %H:%i:%s') as fechaenvio, \
				date_format(horaenvio, '20000101 %H:%i:%s') as horaenvio, \
				urgente, leido, contenido, asunto \
				from mensajes, empleados \
				where empleados.empleado=remitente and destmovil=1 and enviadomovil=0";
			instruccion=instruccion+" and destino='";
			instruccion=instruccion+vendedor;
			instruccion=instruccion+"'";
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Manda SALDOS
			// Por optimización serán solo saldos de ventas de los últimos dos años
			// Ya que si tiene saldo de antes ya debería estar cancelado su crédito.

			instruccion="CREATE TEMPORARY TABLE auxventassaldos (venta CHAR(11), saldor DECIMAL(16,2),   chcnc DECIMAL(16,2), tncredito DECIMAL(16,2), tncargo DECIMAL(16,2), fechach DATE, PRIMARY KEY (venta)) Engine = INNODB";
			mServidorVioleta->EjecutaSelectSqlNulo(Respuesta,  MySQL, instruccion.c_str());

			instruccion.sprintf("INSERT INTO auxventassaldos ( \
				  venta, \
				  saldor, \
				  chcnc, \
				  tncredito, \
				  tncargo, \
				  fechach \
				) \
				SELECT \
				  v.referencia AS venta, \
				  SUM(IF(t.aplicada = 1, t.valor, 0)) AS saldor, \
				  SUM(IF(t.aplicada = 0, t.valor, 0)) AS chcnc, \
				  SUM( \
					IF( \
					  t.aplicada = 1 \
					  AND t.tipo = 'DEVO', \
					  t.valor, \
					  0 \
					) \
				  ) AS tncredito, \
				  SUM( \
					IF( \
					  t.aplicada = 1 \
					  AND (t.tipo = 'NCAR' \
						OR t.tipo = 'INTE'), \
					  t.valor, \
					  0 \
					) \
				  ) AS tncargo, \
				  MAX( \
					IFNULL( \
					  chcl.fechacob, \
					  CAST('1900-01-01' AS DATE) \
					) \
				  ) AS fechach \
				FROM \
				  ventas v \
				  INNER JOIN \
				  transxcob t \
				  LEFT JOIN \
				  pagoscli p \
				  ON t.pago = p.pago \
				  AND t.aplicada = 0 \
				  LEFT JOIN \
				  cheqxcob chxc \
				  ON p.pago = chxc.pago \
				  LEFT JOIN \
				  chequesclientes chcl \
				  ON chxc.chequecli = chcl.chequecli \
				WHERE t.referencia = v.referencia \
				  AND v.fechavta between '%s' and '%s' AND \
				  v.referencia NOT IN(select referencia from ventascongeladas where activo=1) \
				  AND t.cancelada = 0 \
				  AND v.cancelado = 0 AND v.acredito = 1 \
				  AND v.cliente IN ( \
				  SELECT cli.cliente FROM clientes cli INNER JOIN clientesemp cemp ON cli.cliente= cemp.cliente \
				  WHERE (cemp.vendedor='%s' OR cemp.cobrador='%s') \
				  AND cli.activo=1 ) \
				GROUP BY v.referencia",
				mFg.DateToMySqlDate(IncMonth(Today(),-24)),
				mFg.DateToMySqlDate(Today()),
				vendedor,vendedor);
			mServidorVioleta->EjecutaSelectSqlNulo(Respuesta,  MySQL, instruccion.c_str());

			// Manda SALDOS
			instruccion.sprintf("SELECT \
				  v.referencia, \
				  IF( \
					IFNULL(v.foliofisic, '') = '', \
					CONCAT( \
					  IFNULL(cfd.serie, ''), \
					  IFNULL(cfd.folio, '') \
					), \
					v.foliofisic \
				  ) AS foliofisic, \
				  v.cliente AS cliente, \
				  f.termino, \
				  v.tipofac, \
				  vs.saldor AS saldo, \
				  (vs.chcnc * - 1) AS chnocob, \
				  (vs.tncredito * - 1) AS tncredito, \
				  vs.tncargo, \
				  vs.fechach, \
				  v.fechavta, \
				  v.horaalta, \
				  v.fechavenc, \
				  v.valor \
				FROM \
				  ventas v \
				  INNER JOIN \
				  auxventassaldos vs \
				  INNER JOIN dventasfpago d \
				  ON d.referencia = v.referencia \
				  AND d.valor = (SELECT MAX(d2.valor) FROM dventasfpago d2 WHERE d2.referencia = v.referencia LIMIT 1 ) \
				  INNER JOIN formasdepago f ON d.formapag = f.formapag \
				  LEFT JOIN \
				  cfd \
				  ON v.referencia = cfd.referencia \
				  AND cfd.tipocomprobante = 'VENT' \
                  AND v.referencia NOT IN(select referencia from ventascongeladas where activo=1) \
				WHERE vs.venta = v.referencia \
				  AND vs.saldor > 0 \
				ORDER BY v.referencia ");
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

		}
	} __finally {
		if (resp_gen!=NULL) delete resp_gen;
		if (resp_almacenes!=NULL) delete resp_almacenes;
        mServidorVioleta->BorraArchivoTemp(archivo_temp_1);
		delete buffer_sql;
	}
}
//---------------------------------------------------------------------------

//ID_CON_ENVIODATOSAPPRUTAS
void ServidorAdminSistema::EnvioDatosAppRutas(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	BufferRespuestas* resp_gen=NULL;
	char *buffer_sql=new char[1024*64*10];
	char *aux_buffer_sql=buffer_sql;
	int i, num_partidas;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[1000];
	AnsiString fecha, fecha2, terminal, chofer;
	//AnsiString archivo_temp1, archivo_temp2, archivo_temp3, archivo_temp4;
	AnsiString sucursal, condicion_sucursal=" ";
	AnsiString almacen, cad_conjunto_almacenes=" ", modo_calcular_existencias;
	AnsiString condicion_almacen=' ';
	BufferRespuestas* resp_almacenes=NULL;

	TDate fecha_hoy=Today();
	AnsiString cad_fecha_hoy=mFg.DateToMySqlDate(fecha_hoy);
	AnsiString cad_fecha_dos_anios=mFg.DateToMySqlDate(IncYear(fecha_hoy, -2));
	AnsiString condicion_fechas_ventassaldo=" v.fechavta between '"+cad_fecha_dos_anios+"' and '"+ cad_fecha_hoy+ "' and ";
	//AnsiString selectCartasPorte=" ",joinCartasPorte=" ";

	try{

		sucursal=FormServidor->ObtieneClaveSucursal();

		fecha=mFg.DateToMySqlDate(Today());
		fecha2=mFg.DateToMySqlDate(Today()-15);

		terminal=mFg.ExtraeStringDeBuffer(&parametros);
		chofer=mFg.ExtraeStringDeBuffer(&parametros);
		modo_calcular_existencias=mFg.ExtraeStringDeBuffer(&parametros);


		// Obtiene los almacenes que corresponden a una sucursal
		if (sucursal!=" ") {
			instruccion.sprintf("SELECT a.almacen FROM almacenes a \
				INNER JOIN secciones s ON a.seccion=s.seccion \
				WHERE s.sucursal='%s'", sucursal);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_almacenes);
			for(i=0; i<resp_almacenes->ObtieneNumRegistros(); i++){
				resp_almacenes->IrAlRegistroNumero(i);
				almacen=resp_almacenes->ObtieneDato("almacen");

				cad_conjunto_almacenes+="'";
				cad_conjunto_almacenes+=almacen;
				cad_conjunto_almacenes+="'";
				if (i<resp_almacenes->ObtieneNumRegistros()-1) // A todos menos al ultimo impuesto le signo +.
					cad_conjunto_almacenes+=",";
			}
		}

		instruccion.sprintf("set @fechafinal='%s'", fecha);
		instrucciones[num_instrucciones++]=instruccion;
		instruccion.sprintf("set @fechadesde='%s'", fecha2);
		instrucciones[num_instrucciones++]=instruccion;

		// Obtiene fecha del corte más próximo previo a la fecha dada
		instruccion.sprintf("set @fechacorte='1900-01-01'");
		/*instruccion.sprintf("select @fechacorte:=ifnull(max(p.fecha),'1900-01-01') as fcorte \
			 from puntoscorte p where p.fecha<='%s'", fecha);*/
		instrucciones[num_instrucciones++]=instruccion;

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_REP);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {

			// Manda parámetros
			instruccion.sprintf("select parametro, left(descripcion,40) as descripcion, valor \
				from parametrosemp WHERE 1 AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Manda CLIENTES

			instruccion.sprintf("select c.cliente, LEFT(c.rsocial,100) as rsocial, c.rfc, \
			left(concat(c.calle, ', ', col.nombre, ' CP:', c.cp, ' ', loc.nombre),100) as domicilio, \
			c.bloqueo, c.credito, c.limcred, 0 as saldo, 0 as pedxfact, c.excederlc, c.plazo, ce.tipoprec \
			from clientes c \
			left join clientesemp ce on ce.cliente = c.cliente and ce.idempresa=%s \
			left join colonias col on col.colonia=c.colonia \
			left join localidades loc on col.localidad=loc.localidad \
			inner join ( \
				select er.cliente \
				from embarquesruta as er inner join \
				embarques as e on e.embarque=er.embarque where e.fechasalid>@fechadesde and e.chofer ='%s' and e.cancelado<>1 \
			) s on s.cliente=c.cliente \
			group by c.cliente \
			order by c.rsocial ", FormServidor->ObtieneClaveEmpresa(), chofer);

			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);


			// Manda CHOFERES
			instruccion.sprintf("SELECT e.empleado, CONCAT(e.nombre, ' ', ifnull(e.appat,''), ' ', ifnull(e.apmat,'')) as nombre, \
				c.tipocomi, c.activo, c.fechaalta, c.fechabaja, c.transportista \
				FROM choferes as c inner join empleados as e on e.empleado=c.empleado \
				WHERE e.empleado='%s' \
				ORDER BY e.empleado ", chofer);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

		// Manda embarques
			instruccion.sprintf("SELECT embarque, viaembarq, ifnull(fechasalid,CAST('1900-01-01' AS DATE)) as fechasalid, \
				ifnull(horasalid,'00:00:00' ) as horasalid,ifnull(fecharecep,CAST('1900-01-01' AS DATE)) as fecharecep, \
				ifnull(horarecep,'00:00:00' ) as horarecep, chofer, ifnull(surtidor,'') as surtidor, \
				ifnull(ayudante1,'') as ayudante1, ifnull(ayudante2, '') as ayudante2, ifnull(ayudante3,'') as ayudante3, \
				cancelado, etapa, ifnull(sector,'') as sector, ifnull(idweb,'') as idweb \
				FROM embarques \
				WHERE fechasalid>@fechadesde and chofer='%s'  and cancelado<>1 \
				ORDER BY embarque ", chofer);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Manda embarquesruta
			instruccion.sprintf("select e.idorden, e.embarque, e.cliente, \
			TRIM(concat(e.calle, ' ', ifnull(e.numext,'S/N'), ' ', ifnull(e.numint,''))) as calleynumero, \
			col.nombre AS colonia, loc.nombre as localidad, m.nombre as municipio, \
			 e.referenciadomic, e.cp, \
			ifnull(X(e.ubicaciongis),'0') as ubicaciongisx, \
			ifnull(Y(e.ubicaciongis),'0') as ubicaciongisy, \
			 e.fechaalta, \
			e.fechadellegada,  ifnull(e.choferllega,'') as choferllega, \
			ifnull(X(e.ubicaciondellegada),'0') as ubicaciondellegadax, \
			ifnull(Y(e.ubicaciondellegada),'0') as ubicaciondellegaday , \
			e.fechadesalida,  ifnull(e.chofersale,'') as chofersale, \
			ifnull(X(e.ubicaciondesalida), '0') as ubicaciondesalidax, \
			ifnull(Y(e.ubicaciondesalida), '0') as ubicaciondesaliday, \
			ifnull(e.observacioneschofer,' ') as observacioneschofer, e.etapa, e.mensaje \
			from embarquesruta as e left join \
			colonias col on col.colonia=e.colonia left join \
			localidades loc on col.localidad=loc.localidad left join \
			municipios as m on m.municipio=loc.municipio left join \
			estados as es on es.estado=m.estado \
			where e.embarque in(select embarque from embarques where fechasalid>@fechadesde and chofer ='%s' and cancelado<>1) \
			order by e.embarque, e.idorden", chofer);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Manda pedidosventa
			instruccion.sprintf("select referencia,tipofac,cancelado,facturado,acredito,termino,cliente,vendedor,usualta, \
			usumodi,fechaalta,horaalta,fechamodi,horamodi,fechaped,fechasurt,porcint,valor, \
			letras,periodic,comisionada,embarque,plazo,cotizacion,terminal,kit,cantkits,almacen, \
			pedimpreso,venta,usocfdi33,formapago33,redondeoantiguo \
			from pedidosventa \
			where embarque in(select embarque from embarques where fechasalid>@fechadesde and cancelado<>1 and chofer ='%s') ", chofer);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Manda dpedidosventa              detalle de pedidos
			instruccion.sprintf("select referencia,articulo,cantidad,claveimp1,claveimp2,claveimp3,claveimp4, \
			costobase,porcdesc,precio,precioimp,id \
			from dpedidosventa where referencia in(select distinct(p.referencia) \
			from pedidosventa as p inner join \
			embarques as e on e.embarque=p.embarque \
			where e.fechasalid>@fechadesde and e.cancelado<>1 and e.etapa>0 and e.chofer ='%s')", chofer);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	} __finally {
		if (resp_gen!=NULL) delete resp_gen;
		if (resp_almacenes!=NULL) delete resp_almacenes;
		delete buffer_sql;
	}
}
//---------------------------------------------------------------------------


//ID_GRA_MSGSMOVILES
void ServidorAdminSistema::GrabaMsgsMoviles(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  GRABA UN GRUPO DE MENSAJES PARA MOVILES
	char *buffer_sql=new char[1024*64*50];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString clave, tarea, remitente, asunto, urgente, contenido, destino;
	int num_partidas, i;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[5000];
	int error=0;
	TDate fecha=Today();
	TTime hora=Time();

	try{
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave del grupo de mensajes
		tarea=mFg.ExtraeStringDeBuffer(&parametros); // Indicador si se va a agregar o modificar.
		remitente=mFg.ExtraeStringDeBuffer(&parametros); // Remitente
		asunto=mFg.ExtraeStringDeBuffer(&parametros); // Asunto
		urgente=mFg.ExtraeStringDeBuffer(&parametros); // Urgente
		contenido=mFg.ExtraeStringDeBuffer(&parametros); // Contenido

		if (error==0) {
			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

			// Obtiene el folio para el mensaje
			if (tarea=="A") {
				instruccion.sprintf("select @folioaux:=valor from foliosemp where folio='GRUPMSGS' AND sucursal = '%s' %s ",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliosig=@folioaux+1");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @folioaux=cast(@folioaux as char)");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @folio=concat('%s', lpad(@folioaux,9,'0'))", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("update foliosemp set valor=@foliosig where folio='GRUPMSGS' AND sucursal = '%s'  ", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
			} else {
				instruccion.sprintf("set @folio='%s'", clave);
				instrucciones[num_instrucciones++]=instruccion;
			}

			// Si se está modificando entonces borra todos los mensajes que ya existan.
			if (tarea=="M") {
				instruccion.sprintf("delete from mensajes where grupo=@folio");
				instrucciones[num_instrucciones++]=instruccion;
			}

			// Graba un mensaje para cada destinatario
			num_partidas=StrToInt(mFg.ExtraeStringDeBuffer(&parametros)); // Obtiene el número de partidas
			for (i=0; i<num_partidas; i++) {
				destino=mFg.ExtraeStringDeBuffer(&parametros); // destinatario

				instruccion.sprintf("select @foliomsgaux:=valor from foliosemp where folio='MENSAJES' AND sucursal = '%s' %s ",FormServidor->ObtieneClaveSucursal(), MODO_BLOQUEO_CLAVES_UNICAS);
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliosigmsg=@foliomsgaux+1");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliomsgaux=cast(@foliomsgaux as char)");
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("set @foliomsg=concat('%s', lpad(@foliomsgaux,9,'0'))", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;
				instruccion.sprintf("update foliosemp set valor=@foliosigmsg where folio='MENSAJES' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;

				datos.AsignaTabla("mensajes");
				datos.InsCampo("grupo", "@folio", 1);
				datos.InsCampo("mensaje", "@foliomsg", 1);
				datos.InsCampo("remitente", remitente);
				datos.InsCampo("destino", destino);
				datos.InsCampo("fechaenvio", mFg.DateToAnsiString(fecha));
				datos.InsCampo("horaenvio", mFg.TimeToAnsiString(hora));
				datos.InsCampo("fechalect", mFg.DateToAnsiString(fecha));
				datos.InsCampo("horalect", mFg.TimeToAnsiString(hora));
				datos.InsCampo("urgente", urgente);
				datos.InsCampo("directorio", ".");
				datos.InsCampo("leido", "0");
				datos.InsCampo("recibido", "0");
				datos.InsCampo("destmovil", "1");
				datos.InsCampo("enviadomovil", "0");
				datos.InsCampo("contenido", contenido);
				datos.InsCampo("asunto", asunto);

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

//---------------------------------------------------------------------------
//ID_CANC_MSGSMOVILES
void ServidorAdminSistema::CancelaMsgsMoviles(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  CANCELA UN GRUPO DE MENSAJES PARA MOVILES
	char *buffer_sql=new char[1024*32];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instrucciones[50], instruccion;
	int num_instrucciones=0;
	AnsiString clave;
	int i;

	try{
		clave=mFg.ExtraeStringDeBuffer(&parametros); // Clave del movimiento

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		// Cancela el movimiento
		instruccion.sprintf("delete from mensajes where grupo='%s'", clave);
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
//ID_CON_MSGSMOVILES
void ServidorAdminSistema::ConsultaMsgsMoviles(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA UN GRUPO DE MENSAJES PARA MOVILES
	AnsiString instruccion;
	AnsiString clave;

	clave=mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene el primer mensaje del grupo de mensajes
	instruccion.sprintf("select m.*, \
	 concat(e.nombre, ' ', e.appat, ' ', e.apmat) as nomremit \
	 from mensajes m, empleados e \
	 where m.grupo='%s' and m.remitente=e.empleado limit 1", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene toda la lista de destinatarios
	instruccion.sprintf("select (not isnull(mensaje)) as seleccion, e.empleado, \
			concat(e.nombre, ' ', e.appat, ' ', e.apmat) as vendedor, \
			if(ifnull(m.enviadomovil,0)=1, 'Ya enviado', '') as enviado \
			FROM terminales t inner join empleados e \
			left join mensajes m on m.destino=e.empleado and m.grupo='%s' \
			where t.esmovil=1 and e.empleado=t.usuario ", clave);

//	select distinct destino from mensajes where grupo='%s'", clave);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}



//---------------------------------------------------------------------------
// ID_GRA_CORTE_COSTOS
void ServidorAdminSistema::GrabaCorteCostos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*64];
	char *aux_buffer_sql=buffer_sql;
	DatosTabla datos(mServidorVioleta->Tablas);

	BufferRespuestas * buffer_respuestas = NULL;
    AnsiString instruccionLocal = "";

	int num_instrucciones=0;
	AnsiString SucursalOrigen ,instruccion, consulta, instrucciones[1000];
	int i;
	vector<AnsiString> archivosTemporales;


	AnsiString fechaInicial = " ";
	AnsiString fechaFinal = " ";
	AnsiString camposCorte, usarCorte;
	AnsiString innerCpi = " ";

	SucursalOrigen=FormServidor->ObtieneClaveSucursal();

	try{
		fechaInicial = mFg.ExtraeStringDeBuffer(&parametros);
		fechaFinal = mFg.ExtraeStringDeBuffer(&parametros);
		usarCorte = mFg.ExtraeStringDeBuffer(&parametros);


		if(fechaInicial == " ")
			fechaInicial = "1900-01-01";


		if(fechaFinal == " ")
			// Siempre se precalculará hasta el día anterior, que es el que se supone ya está cerrado
			// por dicha razón se recomienda ejecutar este proceso en la madrugada (a las 4:00 AM está bien)
			fechaFinal=mFg.DateToMySqlDate(Today()-1);

		instruccion.sprintf("INSERT INTO bitacoraservidor VALUES (NULL,'%s',NOW(),'INICIO_ID_GRA_CORTE_COSTOS')",SucursalOrigen);
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("SET SESSION sql_log_bin=0");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("set @fechafinal='%s'", fechaFinal);
		instrucciones[num_instrucciones++]=instruccion;

		// Obtiene fecha del corte más próximo previo a la fecha dada
		//	instruccion.sprintf("select @fechacorte:=ifnull(max(p.fecha),'1900-01-01') as fcorte 	 from puntoscorte p where p.fecha<'%s'", fecha_inicial);
		instruccion.sprintf("set @fechacorte = '%s'", fechaInicial);
		instrucciones[num_instrucciones++]=instruccion;

		//--------------------INICIO DEL CÁLCULO POR EMPRESA-----------------
		instruccionLocal = "SELECT idempresa FROM EMPRESAS";
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccionLocal.c_str(), buffer_respuestas);


		for(int i = 0; i < buffer_respuestas->ObtieneNumRegistros(); i++){
			buffer_respuestas->IrAlRegistroNumero(i);

			AnsiString idEmpresa = buffer_respuestas->ObtieneDato("idempresa");

			instruccion.sprintf("DROP TABLE IF EXISTS auxprecalculocostos");
			instrucciones[num_instrucciones++]=instruccion;
			instruccion="CREATE temporary TABLE `auxprecalculocostos` ( \
							id int(11) NOT NULL, \
							`referencia` varchar(11) DEFAULT NULL, \
							`producto` varchar(8) DEFAULT NULL, \
							`present` varchar(255) DEFAULT NULL, \
							`tipo` varchar(2) DEFAULT NULL, \
							`clasif` varchar(1) DEFAULT NULL, \
							`cantidad` double DEFAULT NULL, \
							`costounit` double DEFAULT NULL, \
							`fechahora` datetime DEFAULT NULL, \
							`sector` varchar(10) DEFAULT NULL, \
							`localidad` varchar(4) DEFAULT NULL, \
							`vendedor` varchar(10) DEFAULT NULL, \
							`tipofac` varchar(11) DEFAULT NULL, \
							`cliente` varchar(11) DEFAULT NULL, \
							`acredito` tinyint(1) DEFAULT NULL, \
							`clasif1` varchar(10) DEFAULT NULL, \
							`clasif2` varchar(10) DEFAULT NULL, \
							`clasif3` varchar(10) DEFAULT NULL, \
							`almacen` varchar(4) DEFAULT NULL, \
							`tipomov` varchar(1) DEFAULT NULL, \
							`conceptomov` varchar(4) DEFAULT NULL, \
							`activo` tinyint(1) DEFAULT NULL, \
							KEY idcomp (id,fechahora,cantidad desc)  \
						  ) Engine=InnoDB ";
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("DROP TABLE IF EXISTS auxproductosb");
			instrucciones[num_instrucciones++]=instruccion;
			instruccion="CREATE temporary TABLE `auxproductosb` ( \
							`id` int(11) NOT NULL AUTO_INCREMENT, \
							`producto` varchar(8) DEFAULT NULL, \
							`present` varchar(255) DEFAULT NULL, \
							`clasif1` varchar(10) DEFAULT NULL, \
							`clasif2` varchar(10) DEFAULT NULL, \
							`clasif3` varchar(10) DEFAULT NULL, \
							PRIMARY KEY (`id`), \
							KEY `prodpres` (`producto`,`present`)  \
						  ) ENGINE=InnoDB ";
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("insert into auxproductosb \
					(producto, present, clasif1, clasif2, clasif3) \
				select pre.producto, pre.present, pro.clasif1, pro.clasif2, pro.clasif3 \
					from productos pro \
					inner join presentaciones pre on pro.producto=pre.producto \
					order by pro.nombre, pre.producto, pre.present");
			instrucciones[num_instrucciones++]=instruccion;

            // 1: Calcula base (0) de los artículos en cuestión la existencia al momento del corte previo.
			// La utilidad de esto es simplemente para tomar en cuenta todos los articulos aunque no
			// tengan ningun movimiento

			if(usarCorte == "1"){
				camposCorte = "ifnull(cpi.cantidad, 0) AS cantidad, ifnull(cpi.costounit,0) AS costounit ";
				innerCpi.sprintf(" INNER JOIN almacenes al \
					LEFT JOIN costospromedioiniciales%s cpi ON cpi.producto = a.producto AND cpi.present = a.present \
					AND cpi.almacen = al.almacen ", idEmpresa);
			} else{
				camposCorte = "0 as cantidad, 0 as costounit ";
				innerCpi.sprintf("INNER JOIN almacenes al");
				}

			// Vacia lo que haya en la tabla de precalculocostos%s (El truncate resetea el autoincrement a cero)
			instruccion.sprintf("truncate table precalculocostos%s", idEmpresa);
			instrucciones[num_instrucciones++]=instruccion;

			archivosTemporales.push_back(mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id));

			instruccion.sprintf("SELECT prod.id, 'BASE' as referencia, a.producto, a.present, 'BA' as tipo, 'A' as clasif, %s, @fechacorte as fechahora, al.almacen, a.activo, prod.clasif1, prod.clasif2, prod.clasif3 \
				FROM articulos a \
				INNER JOIN auxproductosb prod ON a.producto=prod.producto AND a.present=prod.present \
				%s \
				INNER JOIN secciones sec ON sec.seccion = al.seccion \
				INNER JOIN sucursales suc ON suc.sucursal = sec.sucursal \
				WHERE \
				suc.idempresa = %s \
				GROUP BY a.producto, a.present, al.almacen INTO OUTFILE '%s'", camposCorte, innerCpi, idEmpresa, archivosTemporales.back());
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE auxprecalculocostos \
				(id, referencia, producto, present, tipo, clasif, cantidad, costounit, fechahora, almacen, activo, clasif1, clasif2, clasif3) ",
				archivosTemporales.back());
			instrucciones[num_instrucciones++]=instruccion;

			// 2: Calcula las compras
			archivosTemporales.push_back(mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id));
			instruccion.sprintf("select prod.id, c.referencia, a.producto, a.present, 'CO' as tipo, 'A' as clasif, \
					(d.cantidad*a.factor) as cantidad, \
					(d.cantidad*(d.costo*(1-c.descuento/100)))/(d.cantidad*a.factor) as costounit, \
					addtime(c.fechacom, c.horaalta) as fechahora, c.almacen, a.activo, \
					prod.clasif1, prod.clasif2, prod.clasif3 \
					from compras c \
					inner join dcompras d inner join articulos a inner join auxproductosb prod \
					INNER JOIN almacenes alm ON alm.almacen = c.almacen \
					INNER JOIN secciones sec ON alm.seccion = sec.seccion \
					INNER JOIN sucursales suc ON suc.sucursal = sec.sucursal \
					where \
					suc.idempresa = %s AND \
					c.fechacom>@fechacorte and c.fechacom<=@fechafinal and c.referencia=d.referencia \
					and c.cancelado=0 and d.articulo=a.articulo and a.producto=prod.producto and  a.present=prod.present \
					INTO OUTFILE '%s'", idEmpresa, archivosTemporales.back());
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE auxprecalculocostos \
				(id, referencia, producto, present, tipo, clasif, cantidad, costounit, fechahora, almacen, activo, clasif1, clasif2, clasif3) ",
				archivosTemporales.back());
			instrucciones[num_instrucciones++]=instruccion;


			// 3: Calcula las devoluciones a las compras
			archivosTemporales.push_back(mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id));
			instruccion.sprintf("select prod.id, n.referencia, a.producto, a.present, if(n.tipo='0','DC','BC') as tipo, if(n.tipo='0','A','B') as clasif, \
					((d.cantidad*a.factor)*-1) as cantidad, \
					(d.cantidad*(d.costo*(1-c.descuento/100)))/(d.cantidad*a.factor) as costounit, \
					addtime(n.fechanot,n.horaalta) as fechahora, c.almacen, a.activo, \
					prod.clasif1, prod.clasif2, prod.clasif3 \
					from notascredprov n \
					INNER JOIN compras c  INNER JOIN dnotascredprov d  INNER JOIN articulos a INNER JOIN auxproductosb prod \
					INNER JOIN almacenes alm ON c.almacen = alm.almacen \
					INNER JOIN secciones sec ON sec.seccion = alm.seccion \
					INNER JOIN sucursales suc on suc.sucursal = sec.sucursal \
					where \
					suc.idempresa = %s AND\
					n.fechanot>@fechacorte and n.fechanot<=@fechafinal and \
					n.referencia=d.referencia and n.compra=c.referencia and \
					n.cancelado=0 and d.articulo=a.articulo and a.producto=prod.producto and  a.present=prod.present \
					INTO OUTFILE '%s'", idEmpresa, archivosTemporales.back());
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE auxprecalculocostos \
				 (id, referencia, producto, present, tipo, clasif, cantidad, costounit, fechahora, almacen, activo, clasif1, clasif2, clasif3) ",
				 archivosTemporales.back());
			instrucciones[num_instrucciones++]=instruccion;

			// 4: Calcula las ventas
			archivosTemporales.push_back(mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id));
			instruccion.sprintf("select prod.id, v.referencia, a.producto, a.present, 'VE' as tipo, 'C' as clasif, \
					((d.cantidad*a.factor)*-1) as cantidad, \
					0 as costounit, \
					addtime(v.fechavta,v.horaalta) as fechahora, d.almacen, a.activo, \
					prod.clasif1, prod.clasif2, prod.clasif3, \
					col.sector, col.localidad, v.vendedor, v.tipofac, v.cliente, v.acredito \
					from ventas v \
					inner join  dventas d  inner join  articulos a \
					inner join  clientes c  inner join  colonias col  \
					INNER JOIN almacenes alm ON alm.almacen = d.almacen \
					INNER JOIN secciones sec ON alm.seccion = sec.seccion\
					INNER JOIN sucursales suc ON suc.sucursal = sec.sucursal\
					left join auxproductosb prod on a.producto=prod.producto and a.present=prod.present \
					where \
					suc.idempresa = %s AND \
					v.cliente=c.cliente and c.colonia=col.colonia and \
					v.fechavta>@fechacorte and v.fechavta<=@fechafinal and v.referencia=d.referencia and v.cancelado=0 \
					and d.articulo=a.articulo \
					 INTO OUTFILE '%s'", idEmpresa, archivosTemporales.back());
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE auxprecalculocostos \
				(id, referencia, producto, present, tipo, clasif, cantidad, costounit, fechahora, almacen, activo, clasif1, clasif2, clasif3, sector, localidad, vendedor, tipofac, cliente, acredito) ",
				archivosTemporales.back());
			instrucciones[num_instrucciones++]=instruccion;

			// 5: Calcula las devoluciones a las ventas.
			archivosTemporales.push_back(mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id));
			instruccion.sprintf("select prod.id, n.referencia, a.producto, a.present, 'DV' as tipo, 'C' as clasif, \
					(dn.cantidad*a.factor) as cantidad, \
					0 as costounit, \
					addtime(n.fechanot,n.horaalta) as fechahora, dv.almacen, a.activo, \
					prod.clasif1, prod.clasif2, prod.clasif3, \
					col.sector, col.localidad, v.vendedor, v.tipofac, v.cliente, v.acredito \
					from notascredcli n \
					inner join dnotascredcli dn \
					inner join  ventas v force index(fechavta) \
					inner join dventas dv \
					inner join  articulos a \
					inner join auxproductosb prod \
					inner join  clientes c \
					inner join  colonias col \
					INNER JOIN almacenes alm ON alm.almacen = dv.almacen \
					INNER JOIN secciones sec ON alm.seccion = sec.seccion \
					INNER JOIN sucursales suc ON suc.sucursal = sec.sucursal \
					where \
					suc.idempresa = %s AND \
					v.cliente=c.cliente and c.colonia=col.colonia and \
					n.fechanot>@fechacorte and n.tipo=0 \
					and n.fechanot<=@fechafinal \
					and n.referencia=dn.referencia and n.venta=v.referencia \
					and v.referencia=dv.referencia and \
					n.cancelado=0 and dn.articulo=a.articulo and dv.articulo=dn.articulo and a.producto=prod.producto and a.present=prod.present \
					 INTO OUTFILE '%s'", idEmpresa, archivosTemporales.back());
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE auxprecalculocostos \
				(id, referencia, producto, present, tipo, clasif, cantidad, costounit, fechahora, almacen, activo, clasif1, clasif2, clasif3, sector, localidad, vendedor, tipofac, cliente, acredito) ",
				archivosTemporales.back());
			instrucciones[num_instrucciones++]=instruccion;

			// 6: Calcula las entradas de almacén
			archivosTemporales.push_back(mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id));
			instruccion.sprintf("select prod.id, m.movimiento as referencia, a.producto, a.present, if(c.costofijo=1,'EC','ES') as tipo, if(c.costofijo=1,'A','C') as clasif, \
					(d.cantidad*a.factor) as cantidad, \
					(d.cantidad*d.costo)/(d.cantidad*a.factor) as costounit, \
					addtime(m.fechamov,m.horaalta) as fechahora, m.almaent as almacen, a.activo, \
					prod.clasif1, prod.clasif2, prod.clasif3, \
					m.tipo as tipomov, m.concepto as conceptomov \
					from movalma m \
					INNER JOIN dmovalma d \
					INNER JOIN articulos a \
					INNER JOIN auxproductosb prod \
					INNER JOIN conceptosmovalma c \
					INNER JOIN almacenes alm ON alm.almacen = m.almaent\
					INNER JOIN secciones sec ON alm.seccion = sec.seccion\
					INNER JOIN sucursales suc ON suc.sucursal = sec.sucursal \
					where \
					m.fechamov>@fechacorte and m.fechamov<=@fechafinal and m.movimiento=d.movimiento \
					and m.tipo<>'S' \
					AND m.aplica=1 and m.cancelado=0 and d.articulo=a.articulo and a.producto=prod.producto and a.present=prod.present \
					and m.concepto=c.concepto AND \
					suc.idempresa = %s \
					INTO OUTFILE '%s'", idEmpresa, archivosTemporales.back());
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE auxprecalculocostos \
				(id, referencia, producto, present, tipo, clasif, cantidad, costounit, fechahora, almacen, activo, clasif1, clasif2, clasif3, tipomov, conceptomov) ",
				archivosTemporales.back());
			instrucciones[num_instrucciones++]=instruccion;

			// 7: Calcula las salidas de almacén
			archivosTemporales.push_back(mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id));
			instruccion.sprintf("select prod.id, m.movimiento as referencia, a.producto, a.present, 'SA' as tipo, 'C' as clasif, \
					((d.cantidad*a.factor)*-1) as cantidad, \
					0 as costounit, \
					addtime(addtime(m.fechamov,m.horaalta), '00:00:01') as fechahora, m.almasal as almacen,  a.activo, \
					prod.clasif1, prod.clasif2, prod.clasif3, \
					m.tipo as tipomov, m.concepto as conceptomov \
					from movalma m \
					INNER JOIN dmovalma d \
					INNER JOIN articulos a \
					INNER JOIN auxproductosb prod \
					INNER JOIN almacenes alm ON alm.almacen = m.almasal \
					INNER JOIN secciones sec ON alm.seccion = sec.seccion \
					INNER JOIN sucursales suc ON suc.sucursal = sec.sucursal \
					where \
					suc.idempresa = %s AND \
					m.fechamov>@fechacorte and m.fechamov<=@fechafinal and m.movimiento=d.movimiento \
					and m.tipo<>'E' \
					AND m.aplica=1 and m.cancelado=0 and d.articulo=a.articulo and a.producto=prod.producto and a.present=prod.present \
					 INTO OUTFILE '%s'", idEmpresa, archivosTemporales.back());
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE auxprecalculocostos \
				(id, referencia, producto, present, tipo, clasif, cantidad, costounit, fechahora, almacen, activo, clasif1, clasif2, clasif3, tipomov, conceptomov) ",
				archivosTemporales.back());
			instrucciones[num_instrucciones++]=instruccion;


			// 8: Resultado final
			archivosTemporales.push_back(mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id));
			instruccion.sprintf("select referencia, producto, present, tipo, clasif, cantidad, costounit, date(fechahora) as fecha, \
					almacen, activo, sector, localidad, vendedor, tipofac, cliente, acredito, clasif1, clasif2, clasif3, tipomov, conceptomov \
					from auxprecalculocostos force index (idcomp) \
					order by id, fechahora, cantidad desc \
					 INTO OUTFILE '%s'", archivosTemporales.back());
			instrucciones[num_instrucciones++]=instruccion;

			instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE precalculocostos%s \
				 (referencia, producto, present, tipo, clasif, cantidad, costounit, fecha, \
					almacen, activo, sector, localidad, vendedor, tipofac, cliente, acredito, clasif1, clasif2, clasif3, tipomov, conceptomov) ",
				archivosTemporales.back(), idEmpresa);
			instrucciones[num_instrucciones++]=instruccion;
		}
		//-----------------------FIN DEL CÁLCULO POR EMPRESA------------------

		
		instruccion.sprintf("update estadosistemaemp set valor='%s' where estado='FPRECCOSTOS' AND sucursal = '%s' ",fechaFinal, FormServidor->ObtieneClaveSucursal());
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("SET SESSION sql_log_bin=1");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("INSERT INTO bitacoraservidor VALUES (NULL,'%s',NOW(),'FIN_ID_GRA_CORTE_COSTOS')",SucursalOrigen);
		instrucciones[num_instrucciones++]=instruccion;

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_REP);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			for(int i = 0 ; i < archivosTemporales.size();i++){
			AnsiString archivoBorrar = archivosTemporales.at(i);
				mServidorVioleta->BorraArchivoTemp(archivoBorrar);
			}
		}
	} __finally {
    	if (buffer_respuestas!=NULL) delete buffer_respuestas;
		delete buffer_sql;
	}
}

//---------------------------------------------------------------------------
//ID_CON_ENVIODATOSINVMOVIL
void ServidorAdminSistema::EnvioDatosInvMovil(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Manda parámetros  solo para controlar la versión y subversion minima de APP
	instruccion.sprintf("select parametro, left(descripcion,40) as descripcion, valor \
		from parametrosemp  where parametro like'%%version%%' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Manda articulos
	/*instruccion.sprintf("select a.articulo, a.producto, a.present, a.multiplo,  a.ean13, a.factor, 0 as cantidad \
		from articulos a where activo=1"); where activo=1   */
	/*instruccion.sprintf("select a.articulo, a.producto, a.present, a.multiplo,  a.ean13, a.factor, 0 as cantidad, p.permitfrac, a.activo \
		from articulos a inner join presentaciones p on p.producto= a.producto and a.present=p.present ");
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);*/


	instruccion.sprintf("select a.articulo, a.producto, a.present, a.multiplo,  a.ean13, a.factor, 0 as cantidad, p.permitfrac, a.activo  \
		from articulos a LEFT JOIN articuloempresacfg aemp ON aemp.articulo = a.articulo AND aemp.idempresa = %s \
		inner join presentaciones p on p.producto= a.producto and a.present=p.present  WHERE a.activo=1 AND (aemp.permiteventas = 1 OR ISNULL(aemp.permiteventas))\
		union  \
		select a.articulo, a.producto, a.present, a.multiplo,  a.ean13, a.factor, 0 as cantidad, p.permitfrac, a.activo  \
		from articulos a inner join presentaciones p on p.producto= a.producto and a.present=p.present \
		WHERE a.activo=0 AND  a.fechamodi> DATE_SUB(NOW(),INTERVAL 6 MONTH) ", FormServidor->ObtieneClaveEmpresa());
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Manda MARCAS
	instruccion.sprintf("SELECT marca, nombre \
						FROM marcas");
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Manda PRESENTACIONES   where activo=1
	instruccion.sprintf("SELECT producto, present \
						FROM ( \
						SELECT pre.producto, pre.present, MAX(art.activo) AS activo \
						FROM presentaciones pre \
						INNER JOIN articulos art ON art.present = pre.present \
						LEFT JOIN articuloempresacfg aemp ON aemp.articulo = art.articulo AND aemp.idempresa = %s \
						WHERE pre.producto=art.producto AND pre.present=art.present AND art.activo=1 \
						AND (aemp.permiteventas = 1 OR ISNULL(aemp.permiteventas)) \
						GROUP BY pre.producto, pre.present \
						UNION \
						SELECT pre.producto, pre.present, a1.activo AS activo \
						FROM presentaciones pre \
						INNER JOIN articulos a1 ON pre.producto=a1.producto AND pre.present=a1.present \
						WHERE a1.activo=0 AND a1.fechamodi> DATE_SUB(NOW(), INTERVAL 6 MONTH) \
						GROUP BY pre.producto, pre.present \
						) p \
						GROUP BY p.producto, p.present", FormServidor->ObtieneClaveEmpresa());
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Manda PRODUCTOS    where activo=1
	instruccion.sprintf("SELECT producto, nombre, marca FROM ( \
						SELECT pro.producto, pro.nombre, pro.marca,  MAX(a.activo) AS activo \
						FROM productos pro \
						INNER JOIN presentaciones pre ON pro.producto=pre.producto \
						INNER JOIN articulos a ON pre.producto=a.producto \
						LEFT JOIN articuloempresacfg aemp ON a.articulo = aemp.articulo AND aemp.idempresa = %s \
						WHERE a.activo=1 AND pre.present=a.present \
						AND (aemp.permiteventas = 1 OR ISNULL(aemp.permiteventas)) \
						GROUP BY pro.producto \
						UNION \
						SELECT TRIM(pro.producto) AS producto, \
						pro.nombre, pro.marca, a1.activo AS activo \
						FROM productos pro \
						INNER JOIN presentaciones pre ON pro.producto=pre.producto \
						INNER JOIN articulos a1 ON pre.present=a1.present AND pre.producto=a1.producto \
						WHERE a1.activo=0 AND a1.fechamodi> DATE_SUB(NOW(), INTERVAL 6 MONTH) \
						GROUP BY pro.producto) p \
						GROUP BY p.producto", FormServidor->ObtieneClaveEmpresa());
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Manda INVENTARIOS (solo lo de hace un més hasta la fecha)
	instruccion.sprintf("SELECT inv.inventario, inv.almacen, inv.descripcion, inv.tipo, inv.fechainv, inv.cerrado \
						FROM inventarios inv \
						INNER JOIN almacenes alm ON inv.almacen = alm.almacen \
						INNER JOIN secciones sec ON sec.seccion = alm.seccion \
						INNER JOIN sucursales suc ON suc.sucursal = sec.sucursal AND suc.idempresa = %s \
						where fechainv>='%s'", FormServidor->ObtieneClaveEmpresa(), mFg.DateToMySqlDate(IncMonth(Today(),-6)));
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Manda ALMACENES
	instruccion.sprintf("SELECT alm.almacen, alm.seccion, alm.nombre \
						FROM almacenes alm \
						INNER JOIN secciones sec ON sec.seccion = alm.seccion \
						INNER JOIN sucursales suc ON suc.sucursal = sec.sucursal AND suc.idempresa = %s", FormServidor->ObtieneClaveEmpresa());
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Manda CODIGOS DE BARRAS adicionales  and a.activo=1
	instruccion.sprintf("SELECT c.articulo, c.codigobarras, 'D' AS descripcion \
						FROM codigosbarras c \
						INNER JOIN articulos a ON c.articulo=a.articulo \
						LEFT JOIN articuloempresacfg aemp ON aemp.articulo = a.articulo AND aemp.idempresa = %s \
						WHERE (aemp.permiteventas = 1 OR ISNULL(aemp.permiteventas))", FormServidor->ObtieneClaveEmpresa());
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Manda PROVEEDORES
	instruccion.sprintf("select proveedor, IF(razonsocial='','Vacio',razonsocial) AS razonsocial \
		from proveedores");
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

}
//---------------------------------------------------------------------------

//ID_CON_ENVIODATOSARTMOVIL
void ServidorAdminSistema::EnvioDatosArticulo(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA UN ARTICULO PARA MOVILES
	AnsiString instruccion;
	AnsiString clave;

	clave=mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene registro
	instruccion.sprintf("SELECT pro.nombre, pre.present, art.multiplo FROM articulos AS art \
		INNER JOIN productos AS pro ON pro.producto=art.producto \
		INNER JOIN presentaciones AS pre ON pre.producto=art.producto AND pre.present=art.present \
		WHERE art.ean13='%s'", clave);

	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}
//---------------------------------------------------------------------------

//ID_GRA_CFDI_COMPRA
void ServidorAdminSistema::GrabaCfdiCompra(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  Graba el CFDI de una compra
	char *buffer_sql=new char[1024*(256+32)]; // PARA UN XML DE 128 KB + 128KB por estar en base 64
	char *aux_buffer_sql=buffer_sql;
	AnsiString instrucciones[50], instruccion;
	int num_instrucciones=0;
	AnsiString referencia, muuid, xml="", totalcomprobante="", fechacomprobante="", fechatimbrado="", version="", nombrearch="";
	AnsiString emisor="", rfcemisor="", receptor="", rfcreceptor="", usuario, tipo, folioFact, proveedor, fechapagoant, fechapagodesp, emitecpago="";
	int i;
	TDate fecha=Today();
	TTime hora=Time();
	int numregDCom=0;

	try{
		referencia=mFg.ExtraeStringDeBuffer(&parametros); // Referencia de la compra
		muuid=mFg.ExtraeStringDeBuffer(&parametros); // UUID de la compra
		usuario=mFg.ExtraeStringDeBuffer(&parametros); // Nombre del usuario
		folioFact=mFg.ExtraeStringDeBuffer(&parametros); // Folio de la compra
		proveedor=mFg.ExtraeStringDeBuffer(&parametros); // Proveedor
		if (muuid!=" ") {
			xml=mFg.ExtraeStringDeBuffer(&parametros); // XML de la compra
			if (xml!=" ") {
				// SI HAY DATOS DE XML
				rfcemisor=mFg.ExtraeStringDeBuffer(&parametros); // RFC de emisor
				rfcreceptor=mFg.ExtraeStringDeBuffer(&parametros); // RFC de receptor
				totalcomprobante=mFg.ExtraeStringDeBuffer(&parametros); // Total del XML
				fechacomprobante=mFg.ExtraeStringDeBuffer(&parametros); // Fecha de comprobante en el XML
				fechatimbrado=mFg.ExtraeStringDeBuffer(&parametros); // Fecha de timbrado en el XML
				version=mFg.ExtraeStringDeBuffer(&parametros); // Version del XML
				nombrearch=mFg.ExtraeStringDeBuffer(&parametros); // Nombre de archivo del XML
				emisor=mFg.ExtraeStringDeBuffer(&parametros); // Nombre de emisor
				receptor=mFg.ExtraeStringDeBuffer(&parametros); // Nombre de receptor
				emitecpago=mFg.ExtraeStringDeBuffer(&parametros); // Emite comprobante de pago
			} else {
				xml="";
				rfcemisor=mFg.ExtraeStringDeBuffer(&parametros); // RFC de emisor
				if (rfcemisor!=" ") {
					// SOLO DATOS DE QRCODE
					rfcreceptor=mFg.ExtraeStringDeBuffer(&parametros); // RFC de receptor
					totalcomprobante=mFg.ExtraeStringDeBuffer(&parametros); // Total del XML
				} else {
					rfcemisor="";
				}
			}
		} else muuid="";

		if(emitecpago==" ")
			emitecpago="NULL";

		BufferRespuestas* resp_info_compras=NULL;
		AnsiString muuidanterior;
		try {
			instruccion.sprintf("SELECT c.muuid,  IFNULL(b.fechapagoantes,fechavenc) AS fechapagoantes, IFNULL(b.fechapagodespues,fechavenc) AS fechapagodespues \
			FROM compras c \
			LEFT JOIN bitacoracomprasmoduuid b ON b.referencia = c.referencia \
			WHERE c.referencia = '%s' \
			ORDER BY b.fechaalta DESC, b.horaalta DESC LIMIT 1", referencia);
			if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_info_compras)) {
				if (resp_info_compras->ObtieneNumRegistros()>0){
					muuidanterior=resp_info_compras->ObtieneDato("muuid");
					fechapagoant=resp_info_compras->ObtieneDato("fechapagoantes");
					fechapagodesp=resp_info_compras->ObtieneDato("fechapagodespues");
				}
			}
		} __finally {
			if (resp_info_compras!=NULL) delete resp_info_compras;
		}


		/* SACAR EL NUMERO DE DETALLES DE LA COMPRA*/
		BufferRespuestas* resp_info_dcompras=NULL;
		try {
			instruccion.sprintf("SELECT dc.* \
			from compras c \
			LEFT JOIN dcompras dc ON dc.referencia = c.referencia \
			WHERE c.referencia = '%s' ", referencia);
			if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_info_dcompras)) {
				if (resp_info_dcompras->ObtieneNumRegistros()>0){
					numregDCom=resp_info_dcompras->ObtieneNumRegistros();
				}
			}
		} __finally {
			if (resp_info_dcompras!=NULL) delete resp_info_dcompras;
		}
		/******FIN ****/


		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		// Borra el cfdi de la compra por si existe.
		instruccion.sprintf("delete from cfdicompras where referencia='%s'", referencia);
		instrucciones[num_instrucciones++]=instruccion;

		if (muuid!="") {
			instruccion.sprintf("update compras set muuid='%s' where referencia='%s'", muuid, referencia);
			instrucciones[num_instrucciones++]=instruccion;
			tipo="M";
		} else {
			instruccion.sprintf("update compras set muuid=null where referencia='%s'", referencia);
			instrucciones[num_instrucciones++]=instruccion;
			tipo="M";
		}

		// Si hay XML graba un registro en cfdicompras
		if (xml!="") {
			instruccion.sprintf("insert into cfdicompras (referencia, totalcomprobante, fechacomprobante, fechatimbrado, \
				fechacarga, \
				version, nombrearch, emisor, rfcemisor, receptor, rfcreceptor, emitecpago, pdf, xml ) \
				values ('%s',%s,'%s','%s','%s %s','%s','%s','%s','%s','%s','%s', %s, '','%s')",
				referencia, totalcomprobante, fechacomprobante, fechatimbrado,
				mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora),
				version, nombrearch, emisor,rfcemisor, receptor, rfcreceptor, emitecpago, xml );
			instrucciones[num_instrucciones++]=instruccion;
			tipo="A";
		} else {
			if (rfcemisor!="") {
				instruccion.sprintf("insert into cfdicompras (referencia, totalcomprobante, fechacomprobante, fechatimbrado, \
					fechacarga, \
					version, nombrearch, emisor, rfcemisor, receptor, rfcreceptor, emitecpago, pdf, xml ) \
					values ('%s',%s,'','','%s %s','','','','%s','','%s',NULL, '','')",
					referencia, totalcomprobante,
					mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), rfcemisor, rfcreceptor );
				instrucciones[num_instrucciones++]=instruccion;
				tipo="A";
			}
		}

		if (usuario!=" ") {
			if (muuidanterior!="" && muuid!="") {
				if (muuidanterior!=muuid) {
					instruccion.sprintf("insert into bitacoracomprasmoduuid (referencia, foliofactura, proveedor, usuario, usumodi, fechamodi, horamodi, tipo, uuidantes, uuiddespues, fechapagoantes, fechapagodespues) \
						values ('%s','%s','%s','%s','%s', CURDATE(),CURTIME(),'M','%s','%s','%s','%s')",
						referencia, folioFact, proveedor, usuario, usuario, muuidanterior, muuid, mFg.StrToMySqlDate(fechapagoant), mFg.StrToMySqlDate(fechapagodesp));
					instrucciones[num_instrucciones++]=instruccion;
				}

			} else if (muuid!="") {
				instruccion.sprintf("insert into bitacoracomprasmoduuid (referencia, foliofactura, proveedor, usuario, fechaalta, horaalta, tipo, uuiddespues, fechapagoantes, fechapagodespues,numregistrosdespues) \
					values ('%s','%s','%s','%s', CURDATE(),CURTIME(),'UA','%s','%s','%s',%d)",
					referencia, folioFact, proveedor, usuario, muuid, mFg.StrToMySqlDate(fechapagoant), mFg.StrToMySqlDate(fechapagodesp), numregDCom);
				instrucciones[num_instrucciones++]=instruccion;
			} else if (muuidanterior!=""){
				instruccion.sprintf("insert into bitacoracomprasmoduuid (referencia, foliofactura, proveedor, usuario, usumodi, fechamodi, horamodi, tipo, uuidantes, fechapagoantes, fechapagodespues, numregistrosantes) \
					values ('%s','%s','%s','%s','%s', CURDATE(),CURTIME(),'UM','%s','%s','%s', %d)",
					referencia, folioFact, proveedor, usuario, usuario, muuidanterior, mFg.StrToMySqlDate(fechapagoant), mFg.StrToMySqlDate(fechapagodesp), numregDCom);
				instrucciones[num_instrucciones++]=instruccion;
			} else {
				instruccion.sprintf("insert into bitacoracomprasmoduuid (referencia, foliofactura, proveedor, usuario, fecha, hora, tipo, fechapagoantes, fechapagodespues) \
					values ('%s','%s','%s','%s', CURDATE(),CURTIME(),'A','%s','%s')",
					referencia, folioFact, proveedor, usuario, mFg.StrToMySqlDate(fechapagoant), mFg.StrToMySqlDate(fechapagodesp));
				instrucciones[num_instrucciones++]=instruccion;
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

//---------------------------------------------------------------------------

//ID_GRA_CFDI_PAGOSPROV
void ServidorAdminSistema::GrabaCfdiPagosProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  Graba el CFDI de un Complemento de Pago
	char *buffer_sql=new char[1024*(256+32)]; // PARA UN XML DE 128 KB + 128KB por estar en base 64
	char *aux_buffer_sql=buffer_sql;
	AnsiString instrucciones[50], instruccion;
	int num_instrucciones=0;
	AnsiString pago, muuid, xml="", totalcomprobante="", fechacomprobante="", fechatimbrado="", version="", nombrearch="";
	AnsiString emisor="", rfcemisor="", receptor="", rfcreceptor="";
	int i;
	TDate fecha=Today();
	TTime hora=Time();

	try{
		pago=mFg.ExtraeStringDeBuffer(&parametros); // pago
		muuid=mFg.ExtraeStringDeBuffer(&parametros); // UUID del Complemento de Pago
		if (muuid!=" ") {
			xml=mFg.ExtraeStringDeBuffer(&parametros); // XML del Complemento de Pago
			if (xml!=" ") {
				// SI HAY DATOS DE XML
				rfcemisor=mFg.ExtraeStringDeBuffer(&parametros); // RFC de emisor
				rfcreceptor=mFg.ExtraeStringDeBuffer(&parametros); // RFC de receptor
				totalcomprobante=mFg.ExtraeStringDeBuffer(&parametros); // Total del XML
				fechacomprobante=mFg.ExtraeStringDeBuffer(&parametros); // Fecha de comprobante en el XML
				fechatimbrado=mFg.ExtraeStringDeBuffer(&parametros); // Fecha de timbrado en el XML
				version=mFg.ExtraeStringDeBuffer(&parametros); // Version del XML
				nombrearch=mFg.ExtraeStringDeBuffer(&parametros); // Nombre de archivo del XML
				emisor=mFg.ExtraeStringDeBuffer(&parametros); // Nombre de emisor
				receptor=mFg.ExtraeStringDeBuffer(&parametros); // Nombre de receptor
			} else {
				xml="";
				rfcemisor=mFg.ExtraeStringDeBuffer(&parametros); // RFC de emisor
				if (rfcemisor!=" ") {
					// SOLO DATOS DE QRCODE
					rfcreceptor=mFg.ExtraeStringDeBuffer(&parametros); // RFC de receptor
					totalcomprobante=mFg.ExtraeStringDeBuffer(&parametros); // Total del XML
				} else {
					rfcemisor="";
				}
			}
		} else muuid="";

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		// Borra el cfdi del Complemento de Pago por si existe.
		instruccion.sprintf("delete from cfdipagosprov where pago='%s'", pago);
		instrucciones[num_instrucciones++]=instruccion;

		if (muuid!="") {
			instruccion.sprintf("update pagosprov set muuid='%s' where pago='%s'", muuid, pago);
		} else {
			instruccion.sprintf("update pagosprov set muuid=null where pago='%s'", pago);
		}
		instrucciones[num_instrucciones++]=instruccion;


		// Si hay XML graba un registro en cfdipagosprov
		if (xml!="") {
			instruccion.sprintf("insert into cfdipagosprov (pago, montopago, fechacomprobante, fechatimbrado, \
				fechacarga, \
				version, nombrearch, emisor, rfcemisor, receptor, rfcreceptor, pdf, xml ) \
				values ('%s',%s,'%s','%s','%s %s','%s','%s','%s','%s','%s','%s', '','%s')",
				pago, totalcomprobante, fechacomprobante, fechatimbrado,
				mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora),
				version, nombrearch, emisor,rfcemisor, receptor, rfcreceptor, xml );
			instrucciones[num_instrucciones++]=instruccion;
		} else {
			if (rfcemisor!="") {
				instruccion.sprintf("insert into cfdipagosprov (pago, montopago, fechacomprobante, fechatimbrado, \
					fechacarga, \
					version, nombrearch, emisor, rfcemisor, receptor, rfcreceptor, pdf, xml ) \
					values ('%s',%s,'','','%s %s','','','','%s','','%s', '','')",
					pago, totalcomprobante,
					mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), rfcemisor, rfcreceptor );
				instrucciones[num_instrucciones++]=instruccion;
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

//---------------------------------------------------------------------------

//ID_CON_PAG_RFC_EMISOR_XML
void ServidorAdminSistema::ConsultaPagRfcEmiXML(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA PROVEEDOR
	AnsiString instruccion;
	AnsiString rfcemisor, rfcreceptor, fechaIni, fechaFin, empresa, condicion_empresa = " ", condicion_sucursal = " ";

	rfcemisor = mFg.ExtraeStringDeBuffer(&parametros);
	rfcreceptor = mFg.ExtraeStringDeBuffer(&parametros);
	empresa = mFg.ExtraeStringDeBuffer(&parametros);
	condicion_sucursal = mFg.ExtraeStringDeBuffer(&parametros);
	fechaIni =  mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
	fechaFin =  mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

	if(empresa != " ")
        condicion_empresa.sprintf(" AND suc.idempresa = %s ", empresa);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_REP_GRANDE);
		 // AND (cfdi.version IS NULL OR cfdi.version = '') %s ",
	//Consulta todos los pagos con el rfc del emisor del XML
	instruccion.sprintf("SELECT p.pago, p.fecha, IF(IFNULL(cfdi.version,'')<>'', '1', '0') AS cargado, \
		p.valor,REPLACE(REPLACE(pr.rfc,' ',''),'-','') AS rfcprov,REPLACE(REPLACE(pcfd.rfcemisor,' ',''),'-','') AS rfcrecep, \
		pr.cuadreestpagos AS cuadrarestricto \
		FROM pagosprov p \
		INNER JOIN PROVEEDORES pr ON pr.proveedor=p.proveedor \
		INNER JOIN sucursales suc  \
		INNER JOIN parametroscfd pcfd ON suc.sucursal=pcfd.sucursal \
		LEFT OUTER JOIN cfdipagosprov cfdi ON p.pago=cfdi.pago \
		WHERE pr.rfc='%s' AND pcfd.rfcemisor='%s' AND p.fecha BETWEEN '%s' AND '%s' \
		AND p.muuid IS NULL  AND (cfdi.version IS NULL OR cfdi.version = '') %s %s \
		GROUP BY p.pago " ,
		 rfcemisor, rfcreceptor, fechaIni, fechaFin, condicion_sucursal, condicion_empresa);

	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),Respuesta->TamBufferResultado);
}

//---------------------------------------------------------------------------

//ID_CON_TRANSAC_PAG
void ServidorAdminSistema::ConsultaTransacPag(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA PROVEEDOR
	AnsiString instruccion;
	AnsiString pago;

	pago = mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_REP_GRANDE);

	//Consultar las transacciones de ese pago
	instruccion.sprintf("\
	SELECT com.muuid,tran.tracredito, com.fechacom, com.folioprov, (tran.valor)*-1 AS valor, \
	tran.fechaalta AS fechapag, tran.cancelada \
	FROM pagosprov pag \
	INNER JOIN transxpag tran \
	INNER JOIN compras com \
	WHERE pag.pago='%s' AND pag.pago=tran.pago AND tran.referencia=com.referencia ", pago);

	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),Respuesta->TamBufferResultado);
}
//---------------------------------------------------------------------------

//ID_GRA_CFDI_NCARPROV
void ServidorAdminSistema::GrabaCfdiNcarProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  Graba el CFDI de una nota de cargo de proveedor
	char *buffer_sql=new char[1024*(256+32)]; // PARA UN XML DE 128 KB + 128KB por estar en base 64
	char *aux_buffer_sql=buffer_sql;
	AnsiString instrucciones[50], instruccion;
	int num_instrucciones=0;
	AnsiString referencia, muuid, xml="", totalcomprobante="", fechacomprobante="", fechatimbrado="", version="", nombrearch="";
	AnsiString emisor="", rfcemisor="", receptor="", rfcreceptor="";
	int i;
	TDate fecha=Today();
	TTime hora=Time();

	try{
		referencia=mFg.ExtraeStringDeBuffer(&parametros); // Referencia de la nota de cargo
		muuid=mFg.ExtraeStringDeBuffer(&parametros); // UUID de la nota de cargo
		if (muuid!=" ") {
			xml=mFg.ExtraeStringDeBuffer(&parametros); // XML de la nota de cargo
			if (xml!=" ") {
				// SI HAY DATOS DE XML
				rfcemisor=mFg.ExtraeStringDeBuffer(&parametros); // RFC de emisor
				rfcreceptor=mFg.ExtraeStringDeBuffer(&parametros); // RFC de receptor
				totalcomprobante=mFg.ExtraeStringDeBuffer(&parametros); // Total del XML
				fechacomprobante=mFg.ExtraeStringDeBuffer(&parametros); // Fecha de comprobante en el XML
				fechatimbrado=mFg.ExtraeStringDeBuffer(&parametros); // Fecha de timbrado en el XML
				version=mFg.ExtraeStringDeBuffer(&parametros); // Version del XML
				nombrearch=mFg.ExtraeStringDeBuffer(&parametros); // Nombre de archivo del XML
				emisor=mFg.ExtraeStringDeBuffer(&parametros); // Nombre de emisor
				receptor=mFg.ExtraeStringDeBuffer(&parametros); // Nombre de receptor
			} else {
				xml="";
				rfcemisor=mFg.ExtraeStringDeBuffer(&parametros); // RFC de emisor
				if (rfcemisor!=" ") {
					// SOLO DATOS DE QRCODE
					rfcreceptor=mFg.ExtraeStringDeBuffer(&parametros); // RFC de receptor
					totalcomprobante=mFg.ExtraeStringDeBuffer(&parametros); // Total del XML
				} else {
					rfcemisor="";
				}
			}
		} else muuid="";

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		// Borra el cfdi de la nota de cargo por si existe.
		instruccion.sprintf("delete from cfdincarprov where referencia='%s'", referencia);
		instrucciones[num_instrucciones++]=instruccion;

		if (muuid!="") {
			instruccion.sprintf("update notascarprov set muuid='%s' where referencia='%s'", muuid, referencia);
		} else {
			instruccion.sprintf("update notascarprov set muuid=null where referencia='%s'", referencia);
		}
		instrucciones[num_instrucciones++]=instruccion;


		// Si hay XML graba un registro en cfdincarprov
		if (xml!="") {
			instruccion.sprintf("insert into cfdincarprov (referencia, totalcomprobante, fechacomprobante, fechatimbrado, \
				fechacarga, \
				version, nombrearch, emisor, rfcemisor, receptor, rfcreceptor, pdf, xml ) \
				values ('%s',%s,'%s','%s','%s %s','%s','%s','%s','%s','%s','%s', '','%s')",
				referencia, totalcomprobante, fechacomprobante, fechatimbrado,
				mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora),
				version, nombrearch, emisor,rfcemisor, receptor, rfcreceptor, xml );
			instrucciones[num_instrucciones++]=instruccion;
		} else {
			if (rfcemisor!="") {
				instruccion.sprintf("insert into cfdincarprov (referencia, totalcomprobante, fechacomprobante, fechatimbrado, \
					fechacarga, \
					version, nombrearch, emisor, rfcemisor, receptor, rfcreceptor, pdf, xml ) \
					values ('%s',%s,'','','%s %s','','','','%s','','%s', '','')",
					referencia, totalcomprobante,
					mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), rfcemisor, rfcreceptor );
				instrucciones[num_instrucciones++]=instruccion;
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

//---------------------------------------------------------------------------

//ID_GRA_CFDI_NCREPROV
void ServidorAdminSistema::GrabaCfdiNcreProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  Graba el CFDI de una nota de credito de proveedor
	char *buffer_sql=new char[1024*(256+32)]; // PARA UN XML DE 128 KB + 128KB por estar en base 64
	char *aux_buffer_sql=buffer_sql;
	AnsiString instrucciones[50], instruccion;
	int num_instrucciones=0;
	AnsiString referencia, muuid, xml="", totalcomprobante="", fechacomprobante="", fechatimbrado="", version="", nombrearch="";
	AnsiString emisor="", rfcemisor="", receptor="", rfcreceptor="";
	int i;
	TDate fecha=Today();
	TTime hora=Time();

	try{
		referencia=mFg.ExtraeStringDeBuffer(&parametros); // Referencia de la nota de credito
		muuid=mFg.ExtraeStringDeBuffer(&parametros); // UUID de la nota de credito
		if (muuid!=" ") {
			xml=mFg.ExtraeStringDeBuffer(&parametros); // XML de la nota de credito
			if (xml!=" ") {
				// SI HAY DATOS DE XML
				rfcemisor=mFg.ExtraeStringDeBuffer(&parametros); // RFC de emisor
				rfcreceptor=mFg.ExtraeStringDeBuffer(&parametros); // RFC de receptor
				totalcomprobante=mFg.ExtraeStringDeBuffer(&parametros); // Total del XML
				fechacomprobante=mFg.ExtraeStringDeBuffer(&parametros); // Fecha de comprobante en el XML
				fechatimbrado=mFg.ExtraeStringDeBuffer(&parametros); // Fecha de timbrado en el XML
				version=mFg.ExtraeStringDeBuffer(&parametros); // Version del XML
				nombrearch=mFg.ExtraeStringDeBuffer(&parametros); // Nombre de archivo del XML
				emisor=mFg.ExtraeStringDeBuffer(&parametros); // Nombre de emisor
				receptor=mFg.ExtraeStringDeBuffer(&parametros); // Nombre de receptor
			} else {
				xml="";
				rfcemisor=mFg.ExtraeStringDeBuffer(&parametros); // RFC de emisor
				if (rfcemisor!=" ") {
					// SOLO DATOS DE QRCODE
					rfcreceptor=mFg.ExtraeStringDeBuffer(&parametros); // RFC de receptor
					totalcomprobante=mFg.ExtraeStringDeBuffer(&parametros); // Total del XML
				} else {
					rfcemisor="";
				}
			}
		} else muuid="";

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		// Borra el cfdi de la nota de credito por si existe.
		instruccion.sprintf("delete from cfdincreprov where referencia='%s'", referencia);
		instrucciones[num_instrucciones++]=instruccion;

		if (muuid!="") {
			instruccion.sprintf("update notascredprov set muuid='%s' where referencia='%s'", muuid, referencia);
		} else {
			instruccion.sprintf("update notascredprov set muuid=null where referencia='%s'", referencia);
		}
		instrucciones[num_instrucciones++]=instruccion;


		// Si hay XML graba un registro en cfdincreprov
		if (xml!="") {
			instruccion.sprintf("insert into cfdincreprov (referencia, totalcomprobante, fechacomprobante, fechatimbrado, \
				fechacarga, \
				version, nombrearch, emisor, rfcemisor, receptor, rfcreceptor, pdf, xml ) \
				values ('%s',%s,'%s','%s','%s %s','%s','%s','%s','%s','%s','%s', '','%s')",
				referencia, totalcomprobante, fechacomprobante, fechatimbrado,
				mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora),
				version, nombrearch, emisor,rfcemisor, receptor, rfcreceptor, xml );
			instrucciones[num_instrucciones++]=instruccion;
		} else {
			if (rfcemisor!="") {
				instruccion.sprintf("insert into cfdincreprov (referencia, totalcomprobante, fechacomprobante, fechatimbrado, \
					fechacarga, \
					version, nombrearch, emisor, rfcemisor, receptor, rfcreceptor, pdf, xml ) \
					values ('%s',%s,'','','%s %s','','','','%s','','%s', '','')",
					referencia, totalcomprobante,
					mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), rfcemisor, rfcreceptor );
				instrucciones[num_instrucciones++]=instruccion;
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


//---------------------------------------------------------------------------
// ID_CALC_PRECALCULOCOSTOS_MENSUAL
void ServidorAdminSistema::PrecalculoCostosMensual(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
	int x;
	AnsiString fecha;
	BufferRespuestas *resp_config=NULL;
	BufferRespuestas *resp_config2=NULL;
	BufferRespuestas *resp_mov=NULL;
	BufferRespuestas *resp_bloques=NULL;

	AnsiString linea;
	AnsiString archivo= "";
	FILE *fp=NULL;
	AnsiString producto_ini, producto_fin, condicion_rango_productos;

	BufferRespuestas * respuesta_empresas = NULL;
	AnsiString idEmpresa;

	char *buffer_resp_mov=NULL;
	try {
		buffer_resp_mov= new char[TAM_MAX_BUFFER_COSTOS];
		fecha=mFg.DateToMySqlDate(Today());

		IteradorCostos *iterador;
		bool hay_pendientes, aplicar_costo;
		int estado_iteracion;
		double costotot;
		char *aux, *respuesta;
		int *tam_respuesta, *num_filas;
		AnsiString nombre_producto, producto_anterior, present_anterior, clasif_anterior, iva;
		double acum_cantidad;
		double acum_costo, costoprom;
		AnsiString producto_actual, present_actual, clasif_actual;
		double cantidad_actual, cantalma_actual;
		double costounit_actual, costotot_actual;
		AnsiString minimo, minimo2, maximo2;
		AnsiString AIOffset, AIincrement, AIconcatenar;
		AnsiString ListaDeRangos,SucursalOrigen;
		int TamRango, PosRango,NumRangos;

		AnsiString id_actual;
		TDate fecha_actual, fecha_seguiente_precalculo;
		bool aplicar_sin_forzar;

		mServidorVioleta->MuestraMensaje("-- Inicializando buffer de  ["+mFg.IntToAnsiString(TAM_MAX_BUFFER_RESPUESTA_REP*2)+"] bytes", Respuesta->Id);
		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_REP*2);

		SucursalOrigen=FormServidor->ObtieneClaveSucursal();

		int rr_tam_respuesta;
		char *rr_aux_resultado=Respuesta->BufferResultado;
		int *rr_aux_tam=(int *)rr_aux_resultado;
		rr_aux_resultado+=sizeof(int);   // Nos saltamos la parte del tamaño del resultado.
		rr_aux_resultado=mFg.AgregaPCharABuffer("0", rr_aux_resultado); // No hubo error
		rr_tam_respuesta=rr_aux_resultado-Respuesta->BufferResultado;
		*rr_aux_tam=rr_tam_respuesta-sizeof(int);
		Respuesta->TamBufferResultado=rr_tam_respuesta;

		respuesta=Respuesta->BufferResultado+Respuesta->TamBufferResultado; // Allì vamos a poner el resultado
		aux=respuesta;
		tam_respuesta=(int *)aux; // Obtenemos la direccion donde se va a poner el tamaño del resultado
		aux+=sizeof(int);
		num_filas=(int *)aux; // Obtenemos la direccion donde se va a poner el número de registros
		(*num_filas)=0;
		aux+=sizeof(int);

		// Obtiene variable de mysql auto_increment_offset, para dependiendo de su configuración así es el ultimo dígito de los
		// folios posibles de una llave primaria con autoincremento
		instruccion.sprintf("show variables like 'auto_increment_offset'");
		if (!mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_config))
			throw Exception("Error en show auto_increment_offset de ID_CALC_PRECALCULOCOSTOS_MENSUAL");
		else {
			if (resp_config->ObtieneNumRegistros()==1) {
				resp_config->IrAlRegistroNumero(0);
				AIOffset=resp_config->ObtieneDato("Value");
			} else
				throw Exception("Error no hay registro en show auto_increment_offset de ID_CALC_PRECALCULOCOSTOS_MENSUAL");
		}

		// Obtiene variable de mysql auto_increment_offset, para dependiendo de su configuración así es el ultimo dígito de los
		// folios posibles de una llave primaria con autoincremento
		instruccion.sprintf("show variables like 'auto_increment_increment'");
		if (!mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_config2))
			throw Exception("Error en show auto_increment_increment de ID_CALC_PRECALCULOCOSTOS_MENSUAL");
		else {
			if (resp_config2->ObtieneNumRegistros()==1) {
				resp_config2->IrAlRegistroNumero(0);
				AIincrement=resp_config2->ObtieneDato("Value");
				if (AIincrement=="1") {
					AIconcatenar=AIOffset;
				} else {
					if (AIincrement=="10") {
						AIconcatenar="0"+AIOffset;
					} else {
						if (AIincrement=="100") {
							if (AIOffset.Length()>2)
								throw Exception("Se permite un máximo de 2 dígitos para auto_increment_offset ID_CALC_PRECALCULOCOSTOS_MENSUAL");
							if (AIOffset.Length()==1) {
								AIconcatenar="00"+AIOffset;
							}
							if (AIOffset.Length()==2) {
								AIconcatenar="0"+AIOffset;
							}
						} else
							throw Exception("El sistema solo funciona con valores de 1, 10 y 100 para auto_increment_increment ID_CALC_PRECALCULOCOSTOS_MENSUAL");
					}
				}
			} else
				throw Exception("Error no hay registro en show auto_increment_increment de ID_CALC_PRECALCULOCOSTOS_MENSUAL");
		}

		// CALCULO DE RANGOS a usarse para precalculominmaxfin y precalculominmax
		ListaDeRangos="";
		TamRango=20000;
		PosRango=0;

		// Importánte: Al 27 de enero del 2022, se encontró que con NumRangos=500 se estába ocupando aprox el 60% posible (con 7 años de ventas),
		// por lo que se incrementó a 1000 (se duplicó), pero quen cinco años o más eso podría no ser suficiente, se podría entonces incrementar a 2000
		NumRangos=1000;

		for (int r=0; r<NumRangos; r++) {
			if(r>0) { // Manda el rango (excepto en la primer posicion ya que es cero)
				ListaDeRangos+=mFg.IntToAnsiString(PosRango);
			}
			ListaDeRangos+=AIconcatenar; // Manda indice para concatenacion por autoincrementables (definida antes te este código).
			if(r<NumRangos-1) { // Pone una coma después de cada elemento, excepto en el último
				ListaDeRangos+=", ";
			}
			if((r%10)==9) { // Cada 10 manda un retorno de linea por legibilidad solamente.
				ListaDeRangos+="\n";
			}

			PosRango+=TamRango;
		}

		// LO SIGUIENTE EN COMENTARIOS LO DEJO AQUI PARA DEPURAR
		/*
		mServidorVioleta->MuestraMensaje("ListaDeRangos:", Respuesta->Id);
		mServidorVioleta->MuestraMensaje(ListaDeRangos, Respuesta->Id);
		throw Exception("Para aquí");*/

		instruccion.sprintf("INSERT INTO bitacoraservidor VALUES (NULL,'%s',NOW(),'INICIO_ID_CALC_PRECALCULOCOSTOS_MENSUAL')",SucursalOrigen);
		mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str());

		instruccion.sprintf("SET SESSION sql_log_bin=0");
		if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
			throw Exception("Error en sql_log_bin=0 de ID_CALC_PRECALCULOCOSTOS_MENSUAL");




		//--------------------INICIO DEL CÁLCULO POR EMPRESA-----------------
		instruccion = "SELECT idempresa FROM EMPRESAS";
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), respuesta_empresas);


		for(int i = 0; i < respuesta_empresas->ObtieneNumRegistros(); i++){
			respuesta_empresas->IrAlRegistroNumero(i);

			idEmpresa = respuesta_empresas->ObtieneDato("idempresa");

			archivo = mServidorVioleta->ObtieneArchivoTemp(2342, Respuesta->Id);

			// Abre el archivo de texto en el que se grabarán los datos de forma temporal
			// para luego usar LOAD DATA INTO (es más rápido así que hacer un internet por cada registro).
			fp=fopen(archivo.c_str(),"w");

            instruccion.sprintf("truncate table precalculominmax%s", idEmpresa);
			if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
				throw Exception("Error en truncate de ID_CALC_PRECALCULOCOSTOS_MENSUAL");

			instruccion.sprintf("truncate table precalculominmaxfin%s", idEmpresa);
			if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
				throw Exception("Error en truncate1 de ID_CALC_PRECALCULOCOSTOS_MENSUAL");


			instruccion.sprintf("insert into precalculominmax%s \
				(producto, present, minimo, maximo) \
				SELECT producto, present, MIN(id) AS minimo, MAX(id) AS maximo \
				FROM precalculocostos%s GROUP BY producto, present ORDER BY minimo", idEmpresa, idEmpresa);
			if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
				throw Exception("Error en precalculomaxmin de ID_CALC_PRECALCULOCOSTOS_MENSUAL");

			instruccion.sprintf("truncate table precalculomensual%s", idEmpresa);
			if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
				throw Exception("Error en truncate2 de ID_CALC_PRECALCULOCOSTOS_MENSUAL");

			instruccion="CREATE TEMPORARY TABLE IF NOT EXISTS auxproductos ( \
							producto varchar(8) DEFAULT NULL, \
							present varchar(255) DEFAULT NULL, \
							PRIMARY KEY (producto,present) \
						  ) ENGINE=InnoDB ";
			if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
				throw Exception("Error en auxproductos de ID_CALC_PRECALCULOCOSTOS_MENSUAL");

			instruccion.sprintf("TRUNCATE TABLE auxproductos");
			if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
				throw Exception("Error en truncate auxproductos de ID_CALC_PRECALCULOCOSTOS_MENSUAL");


			instruccion.sprintf("insert into auxproductos (producto,present) \
				 (SELECT DISTINCT producto, present \
				  FROM precalculocostos%s pc \
				  WHERE id IN \
					(%s ) \
				 ) \
				 UNION \
				  (SELECT producto,present FROM precalculocostos%s pc ORDER BY id DESC LIMIT 1) \
					ORDER BY producto,present ",
					idEmpresa, ListaDeRangos, idEmpresa);
			if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
				throw Exception("Error en auxproductos de ID_CALC_PRECALCULOCOSTOS_MENSUAL");

			instruccion.sprintf("insert into precalculominmaxfin%s (producto,present, minimo, maximo) \
				select pmm.producto, pmm.present, pmm.minimo, pmm.maximo \
				from auxproductos ap \
				inner join precalculominmax%s pmm on pmm.producto=ap.producto and pmm.present=ap.present\
				order by pmm.minimo", idEmpresa, idEmpresa);
			if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
				throw Exception("Error en precalculominmaxfin de ID_CALC_PRECALCULOCOSTOS_MENSUAL");

			instruccion.sprintf("select pmm.producto, pmm.present, pmm.minimo, pmm.maximo \
				from precalculominmaxfin%s pmm \
				order by pmm.minimo", idEmpresa);
			if (!mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_bloques))
				throw Exception("Error en select precalculominmaxfin de ID_CALC_PRECALCULOCOSTOS_MENSUAL");

			for(x=0; x<resp_bloques->ObtieneNumRegistros()-1; x++){
				resp_mov=NULL;
				iterador=NULL;

				resp_bloques->IrAlRegistroNumero(x);
				minimo=resp_bloques->ObtieneDato("minimo");

				resp_bloques->IrAlRegistroNumero(x+1);
				minimo2=resp_bloques->ObtieneDato("minimo");
				maximo2=resp_bloques->ObtieneDato("maximo");

				if (x<resp_bloques->ObtieneNumRegistros()-2) {
					condicion_rango_productos.sprintf(" pc.id>=%s and pc.id<%s ", minimo, minimo2);
				} else {
					condicion_rango_productos.sprintf(" pc.id>=%s and pc.id<=%s ", minimo, maximo2);
				}

				try {
					// Manda el resultado a un buffer
					instruccion.sprintf("select pc.producto, pc.present, \
						pc.clasif, pc.cantidad, if(pc.tipo<>'BC', pc.cantidad, 0) as cantalma, pc.costounit, pc.fecha, pc.id \
						from precalculocostos%s pc \
						where pc.fecha<='%s' and %s \
						order by id",
						idEmpresa, fecha, condicion_rango_productos);

					if (mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), resp_mov, buffer_resp_mov)) {
						mServidorVioleta->MuestraMensaje("-- Finalizó mandar resultados a buffer ["+mFg.IntToAnsiString(resp_mov->ObtieneTamRespuesta())+"] bytes", Respuesta->Id);

						// Crea objeto iterador
						iterador=new IteradorCostos(resp_mov);

						mServidorVioleta->MuestraMensaje("-- Finalizó creación de iterador", Respuesta->Id);

						// producto_anterior se usa para que en cada registro saber que
						// producto era el registro anterior
						producto_anterior="";

						// present_anterior se usa para en cada registro saber que
						// presentación era el registro anterior
						present_anterior="";

						// acum_cantidad se usa para en cada registro saber
						// la sumatoria de la cantidad del último producto-presentación
						// en global (todos los almacenes)
						acum_cantidad=0;

						// acum_costo se usa para en cada registro saber
						// la sumatoria del costo acumulado del último producto-presentación
						// en global (todos los almacenes)
						acum_costo=0;

						// Costo promedio hasta el último producto-presentacion
						costoprom=0;

						// Ciclo para calcular los costos.
						do {
							hay_pendientes=iterador->IrRegistroPendiente();
							estado_iteracion=iterador->ObtieneEstadoIteracion();

							// Si i está dentro del rango válido nos posicionamos en el registro(i)
							// y obtenemos el producto y presentación de dicho registro
							if (hay_pendientes) {
								producto_actual=resp_mov->ObtieneDato("producto");
								present_actual=resp_mov->ObtieneDato("present");
							}

							if (hay_pendientes) {
								clasif_actual=resp_mov->ObtieneDato("clasif");
								cantalma_actual=mFg.CadenaAFlotante(resp_mov->ObtieneDato("cantalma"));

								fecha_actual=StrToDate(resp_mov->ObtieneDato("fecha"));
								id_actual=resp_mov->ObtieneDato("id");

								if (clasif_actual=="A") {
									cantidad_actual=mFg.CadenaAFlotante(resp_mov->ObtieneDato("cantidad"));
									costounit_actual=StrToFloat(resp_mov->ObtieneDato("costounit"));
									costotot_actual=cantidad_actual*costounit_actual;
								}
								if (clasif_actual=="B") {
									cantidad_actual=0;
									costounit_actual=StrToFloat(resp_mov->ObtieneDato("costounit"));
									costotot_actual=mFg.CadenaAFlotante(resp_mov->ObtieneDato("cantidad"))*costounit_actual;
								}
								if (clasif_actual=="C") {
									cantidad_actual=mFg.CadenaAFlotante(resp_mov->ObtieneDato("cantidad"));
									costounit_actual=costoprom;
									costotot_actual=cantidad_actual*costounit_actual;
								}


								// Si es un producto-presentacion DIFERENTE al anterior
								if ((producto_actual!=producto_anterior || present_actual!=present_anterior) ) {

									// Asigna los datos del registro actual a las variables
									// usadas para registro anterior, esto en preparación
									// para el siguiente ciclo.
									producto_anterior=producto_actual;
									present_anterior=present_actual;
									clasif_anterior=clasif_actual;

									costoprom=0;
									acum_cantidad=0;
									acum_costo=0;

									iterador->IniciaOtroProductoPresentacion();

									fecha_seguiente_precalculo=IncMonth(fecha_actual,1);
								}

								// si alguno de los acumulados podría dar negativo entonces
								// se ignora el registro actual, pero se acumula para posterior
								// aplicación cuando haya el positivo suficiente
								if ( (acum_cantidad+cantidad_actual)<0 ||
									(acum_costo+costotot_actual)<0 ) {
									  // Si se generó un negativo

									  // Si se no terminaron los registros y no esta en modo
									  // de  forzar pendientes entonces se marca como no aplicado
									  // para que se almacene en los pendientes.
									  if (iterador->ObtieneEstaTerminadaIteracionNormal()==false &&
										  iterador->ObtieneForzarPendientes()==false) {
											  // Lo marca como no aplicado lo que a su vez
											  // lo agrega a pendientes
											  aplicar_costo=false;
											  aplicar_sin_forzar=false;
											  iterador->MarcaComoNoAplicado();
									  } else {
											// Aplica
										  aplicar_costo=true;
										  aplicar_sin_forzar=false;
										  iterador->MarcaComoSiAplicado();
									  }
								} else { // Si No se generó un negativo

									// Si es iteracion normal y además
									// mov actual disminuye costo o existencias y
									// además hay pendientes de aplicar ENTONCES se les da prioridad
									// a los pendientes y el mov actual se pone como no
									// aplicado hasta el final de la lista de pendientes.
									if ( estado_iteracion==0 &&
										 (cantidad_actual<0 ||
										 costotot_actual<0 ||
										 cantalma_actual<0) &&
										 iterador->ObtieneNumRealNoAplicados()>0) {
											// Lo marca como no aplicado lo que a su vez
											// lo agrega a pendientes
											aplicar_costo=false;
											aplicar_sin_forzar=false;
											iterador->MarcaComoNoAplicado();
									} else {
										// Aplica
										aplicar_costo=true;
										aplicar_sin_forzar=true;
										iterador->MarcaComoSiAplicado();
									}
								}

								// Totaliza tomando en cuenta el registro actual
								if (aplicar_costo) {
									acum_cantidad=acum_cantidad + cantidad_actual;
									acum_costo=acum_costo + costotot_actual;
									if ( !mFg.EsCero(acum_cantidad) )
										costoprom=acum_costo / acum_cantidad;

									if (aplicar_sin_forzar) {
										// Si se aplicó sin forzar, es candidato a ser precalculo por meses
										if (fecha_actual>=fecha_seguiente_precalculo) {

											linea.sprintf("%s\t%s\t%s\t%s\t%s\t%s\t%s\n",
													producto_actual, present_actual,
													mFg.FormateaCantidad(acum_cantidad,10,false),
													mFg.FormateaCantidad(acum_costo,10,false),
													mFg.FormateaCantidad(costoprom,10,false),
													mFg.DateToMySqlDate(fecha_actual),
													id_actual);
											fputs(linea.c_str(),fp);

											/*instruccion.sprintf("insert into precalculomensual \
												(producto, present, acumcantidad, acumcosto, costoprom, fecha, idcalculado) \
													values \
												('%s', '%s', %s, %s, %s, '%s', %s)",
													producto_actual, present_actual,
													mFg.FormateaCantidad(acum_cantidad,10,false),
													mFg.FormateaCantidad(acum_costo,10,false),
													mFg.FormateaCantidad(costoprom,10,false),
													mFg.DateToMySqlDate(fecha_actual),
													id_actual);
											if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
												throw Exception("Error en insert de ID_CALC_PRECALCULOCOSTOS_MENSUAL");*/

											fecha_seguiente_precalculo=IncMonth(fecha_actual,1);
										}
									}
								}

							}
						} while (hay_pendientes);
					}
				} __finally {
					if (iterador!=NULL) {
						delete iterador;
						iterador=NULL;
					}
					if (resp_mov!=NULL) {
						delete resp_mov;
						resp_mov=NULL;
					}
				}

			}

			fclose(fp);
			fp=NULL;

			instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE precalculomensual%s \
				(producto, present, acumcantidad, \
				acumcosto, costoprom, fecha, idcalculado) ", archivo, idEmpresa);
			if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
				throw Exception("Error en LOAD DATA INFILE de ID_CALC_PRECALCULOCOSTOS_MENSUAL");

			mServidorVioleta->BorraArchivoTemp(archivo);

		}//-----------------FIN PRECALCULO POR EMPRESA------------------------

		instruccion.sprintf("SET SESSION sql_log_bin=1");
		if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
			throw Exception("Error en sql_log_bin=1 de ID_CALC_PRECALCULOCOSTOS_MENSUAL");

		instruccion.sprintf("INSERT INTO bitacoraservidor VALUES (NULL,'%s',NOW(),'FIN_ID_CALC_PRECALCULOCOSTOS_MENSUAL')",SucursalOrigen);
		mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str());

		// Establece el tamaño de la respuesta.
		*tam_respuesta=(aux-respuesta);
		Respuesta->TamBufferResultado=Respuesta->TamBufferResultado+*tam_respuesta; // Calcula tamaño total de buffer
		Respuesta->PosBufferResultado=0; // Los resultados al cliente serán a partir del inicio del buffer.

	} __finally {
		if (fp!=NULL) {
			fclose(fp);
			mServidorVioleta->BorraArchivoTemp(archivo);
		}
		if (resp_bloques!=NULL) {
			delete resp_bloques;
		}

		if (resp_config!=NULL) {
			delete resp_config;
		}

		if (resp_config2!=NULL) {
			delete resp_config2;
		}
		if (buffer_resp_mov != NULL)
			delete buffer_resp_mov;

        delete respuesta_empresas;

	}
}
//---------------------------------------------------------------------------
// ID_CON_ESTADOSERVIDOR

void ServidorAdminSistema::ConsultaEstadoServidor(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{

	// REVISAR BIEN LIBERACION DE MEMORIA !!!!!!!!!!!!!!!!!

	// variables para acceder a los valores que estan en parametros
	//int num_instrucciones=0;
   AnsiString instruccion, instrucciones[1000],directorio,nombre_arch;
	AnsiString sucursal,espaciodisponible,urespaldo,rutarespaldo, ultimorespaldo,auxruta,estadorep;
	BufferRespuestas* resp_unidad=NULL;
	BufferRespuestas* resp_ruta=NULL;
	BufferRespuestas *resp_envios=NULL;


	//variables de espacio de unidad
	_ULARGE_INTEGER FreeAvailable;
	_ULARGE_INTEGER TotalSpace;
	_ULARGE_INTEGER TotalFree;

	//variables de obtencion del ultimo respaldo ,contAux=0
	AnsiString lista[1024];
	struct ffblk ff;
	int final, i, j;
	char archivo[1024];
	int fecha,fechamax=-1,num_campos;
	int resultado_chdir,contador=0;
	AnsiString extension;
	AnsiString ArchivoConRuta,ArchivoConRutaAux,fechaAux;
	AnsiString rutaaux;


    try{
		//se obtiene la unidad donde se hacen los respaldos
		//primero se hace una seleccion, independiende del resultado que semandara,
		//por peticion del cliente, el cual obtendra la unidad donde se hacen los respaldos
		instruccion.sprintf("SELECT  valor as unidad FROM parametrosemp WHERE parametro='UNIDADRESPALDO' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_unidad);
		urespaldo = resp_unidad->ObtieneDato("unidad");

		AnsiString us = urespaldo;

		if (GetDiskFreeSpaceExA(us.c_str(), &FreeAvailable, &TotalSpace, &TotalFree)){
			 espaciodisponible = int(FreeAvailable.QuadPart) / (1024*1024*1024);
		}else {
			 espaciodisponible = L"Nev_domy";
		}


		//en las siguentes lineas se obtendra la ruta completa donde se guardan los respaldos
		//y hará exactamento lo del punto anterior, meter el resultado en una variable
		//para mandarla al cliente
		instruccion.sprintf("SELECT @r:=valor as ruta FROM parametrosemp WHERE parametro='RUTARESPALDO' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());

		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_ruta);
		rutarespaldo = resp_ruta->ObtieneDato("ruta");
		rutaaux = StringReplace(rutarespaldo,"\\","\"", TReplaceFlags()<<rfReplaceAll);
		instruccion="CREATE TEMPORARY TABLE auxruta ( \
							ruta varchar(150) DEFAULT NULL, \
							fecha varchar(8) DEFAULT NULL, \
							PRIMARY KEY (ruta) \
		) ENGINE=InnoDB ";
		mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str());


		resultado_chdir=chdir(rutarespaldo.c_str());
		if (resultado_chdir==0){
			final=findfirst("*.*", &ff, FA_DIREC);
			while (!final) {
				if ((ff.ff_attrib & 16)!=16) {
					strcpy(archivo, ff.ff_name);
					fecha= ff.ff_fdate;
					extension=ExtractFileExt(AnsiString(archivo)).UpperCase();

					if (extension==".7Z"){
						contador=contador+1;
						ArchivoConRuta=rutarespaldo+"\\"+AnsiString(archivo);

						if (fecha>fechamax){
							fechamax=fecha;
							nombre_arch =  AnsiString(archivo);
							ArchivoConRutaAux=ArchivoConRuta;
							instruccion.sprintf("INSERT INTO auxruta (ruta,fecha) SELECT CONCAT(@r,'\\ %s'),%d ",nombre_arch,fechamax);
							mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str());

						}//condicion si es mas grande

					}//cierra condicion de la extencion

				}//cierra condicion de direcctorio
				final=findnext(&ff);
			}//cierra while
			ultimorespaldo=ArchivoConRutaAux;
		}//cierra la ruta del ultimo respaldo


		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

		//obtiene el estado de la replicacion
		instruccion.sprintf("show slave status");
		if(!mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(),Respuesta->TamBufferResultado))
			throw Exception("No tienes acceso a la informacion de replicación");


		// Obtiene registro , 'respaldo .7z' AS ultimorespaldo, 'respaldo largo.7z' AS respaldoexterno
		instruccion.sprintf("SELECT '%s' AS espaciolibre, '%s' AS ultimorespaldo",
		espaciodisponible,ultimorespaldo);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

		// obtiene todos los registros de la tabla temporal auxruta
		instruccion.sprintf("SELECT ruta FROM auxruta ORDER BY fecha DESC LIMIT 50");
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}__finally{
		if (resp_unidad!=NULL) {
			delete resp_unidad;
			resp_unidad=NULL;}

		if (resp_ruta!=NULL) {
			delete resp_ruta;
			resp_ruta=NULL;}

		if (resp_envios!=NULL) {
			delete resp_envios;
			resp_envios=NULL;}
	}

}

//---------------------------------------------------------------------------
// ID_GRA_EXISTENCIAS_ACTUALES
void ServidorAdminSistema::GrabaExistenciasActuales(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*64*10];
	char *aux_buffer_sql=buffer_sql;
	int i;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[1000];
	AnsiString terminal, articulo, considerar_cortes;
	AnsiString fecha;
	AnsiString almacen, stock,marca, producto ;
	AnsiString obtener_articulos_inactivos;
	AnsiString condicion_almacen=" ", condicion_stock=" ",condicion_marca=" ", condicion_producto=" ";
	AnsiString condicion_articulos_inactivos=" ";
	AnsiString join_productos=" ";
	AnsiString calculo_promedio_ventas;
	TDate fecha_inicial_promedio;
	int dias, tipopromedio;
	AnsiString forzar_indice_vta=" ";
	AnsiString sucursal, condicion_sucursal=" ";
	AnsiString cad_conjunto_almacenes=" ";
	AnsiString archivo_temp1, archivo_temp2, archivo_temp3, archivo_temp4, archivo_temp5;
	double divisor;
	BufferRespuestas* resp_almacenes=NULL;
	int costoscompra;
	AnsiString camposcostoscompra=" ", leftcostoscompra=" ",SucursalOrigen;

	SucursalOrigen=FormServidor->ObtieneClaveSucursal();

	try {

		instruccion.sprintf("INSERT INTO bitacoraservidor VALUES (NULL,'%s',NOW(),'INICIO_ID_GRA_EXISTENCIAS_ACTUALES')",SucursalOrigen);
		instrucciones[num_instrucciones++]=instruccion;

		instruccion="SET SESSION sql_log_bin = 0";
		instrucciones[num_instrucciones++]=instruccion;

		instruccion="set @fechafinal=CURDATE()";
		instrucciones[num_instrucciones++]=instruccion;

        BufferRespuestas* resp_n = NULL;
			BufferRespuestas* resp_n2 = NULL;
			AnsiString fech, resultEstGlob, resultPuntCort;
			AnsiString fechaResF, fechaResFglon;
			bool bandExist;
            fecha=mFg.DateToMySqlDate(Today());
			try {
				instruccion.sprintf("SELECT IFNULL(MAX(p.fecha),'1900-01-01') AS fcorte \
				FROM puntoscorte p \
				WHERE p.fecha<='%s' \
				UNION \
				SELECT IFNULL(MAX(p.fecha),'1900-01-01') AS fcorte \
				FROM puntoscorte p \
				WHERE p.fecha>= '%s'",fecha, fecha);
				mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,
					instruccion.c_str(), resp_n);
				int numeroReg = resp_n->ObtieneNumRegistros();
				resultPuntCort = resp_n->ObtieneDato("fcorte");
				AnsiString datU, datD;
				for (int i = 0; i < numeroReg; i++) {
					if(i==0)
						datU = resp_n->ObtieneDato("fcorte");
					if(i!=0)
						datD = resp_n->ObtieneDato("fcorte");
                    resp_n->IrAlSiguienteRegistro();
				}
				if(resultPuntCort == " ")
					resultPuntCort="1900-01-01";

				instruccion.sprintf("SELECT IFNULL(valor, '2000-01-01') AS val \
				FROM estadosistemaglob \
				WHERE estado = 'FCORTEINI'");
				mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,
					instruccion.c_str(), resp_n2);
				resultEstGlob = resp_n2->ObtieneDato("val");
				fechaResFglon = resp_n2->ObtieneDato("val");
				if(resultEstGlob == "")
					resultEstGlob = "1900-01-01";

				 TDateTime resultDFech, resultDPc, resultDEg, resultU, resultD;
				 int difpc, difeg, difU, difD;
				 int a, m ,d;
				 if(sscanf(fecha.c_str(),"%4d-%2d-%2d",&a, &m, &d) == 3)
					resultDFech= EncodeDate(a,m,d);
				 ///
                 if(sscanf(datU.c_str(),"%4d-%2d-%2d",&a, &m, &d) == 3)
					resultU= EncodeDate(a,m,d);
				 if(sscanf(datD.c_str(),"%4d-%2d-%2d",&a, &m, &d) == 3)
					resultD= EncodeDate(a,m,d);
				 difU = DaysBetween(resultDFech,resultU);
				 difD = DaysBetween(resultDFech,resultD);
				 if(difU < difD)
					resultPuntCort=datU;//difU;
				 else
					resultPuntCort=datD;//difD;
				 ///
				 if(sscanf(resultPuntCort.c_str(),"%4d-%2d-%2d",&a, &m, &d) == 3)
					resultDPc= EncodeDate(a,m,d);
				 if(sscanf(resultEstGlob.c_str(),"%4d-%2d-%2d",&a, &m, &d) == 3)
					resultDEg= EncodeDate(a,m,d);
				 difpc = DaysBetween(resultDFech,resultDPc);
				 difeg = DaysBetween(resultDFech,resultDEg);
				 if(difpc == difeg){
                    fechaResF = resultEstGlob;
					bandExist=true; //eglob
				 }
				 else if(difpc < difeg){
					//fecha = difpc;
					bandExist=false; //pc
					fechaResF = resultPuntCort;
				 } else{
					//fecha = difeg;
					fechaResF = resultEstGlob;
					bandExist=true; //eglob
				 }
				 instruccion.sprintf("set @fechacorte='%s'", fechaResF);
                 instrucciones[num_instrucciones++] = instruccion;
			}
			__finally {
				if (resp_n != NULL)
					delete resp_n;
				if (resp_n2 != NULL)
					delete resp_n2;
			}


		// Crea una tabla donde se pondrán las existencias que se vayan encontrando.
		instruccion="create temporary table existenciastemp ( \
			producto varchar(8), present varchar(255), \
			tipo varchar(2), cantidad decimal(12,3), almacen varchar(4), ventas decimal(12,3), devventas decimal(12,3),  \
			compras decimal(12,3), devcompras decimal(12,3), entradas decimal(12,3), salidas decimal(12,3),  \
			cantinicial decimal(12,3) default '0.000' , INDEX(producto, present)) Engine = InnoDB";
		instrucciones[num_instrucciones++]=instruccion;


		// Calcula base (0) de los artículos en cuestión.
		// La utilidad de esto es simplemente para tomar en cuenta todos los articulos aunque no
		// tengan ningun movimiento
		instruccion="insert into existenciastemp (producto, present, tipo, cantidad, almacen, ventas, devventas \
			, compras, devcompras, entradas, salidas, cantinicial) \
			select a.producto, a.present, 'BA' as tipo, 0.000 as cantidad, al.almacen, 0.000 as ventas, 0.000 as devventas, \
			0.000 as compras, 0.000 as devcompras, 0.000 as  entradas, 0.000 as salidas, 0.000 as cantinicial \
			from articulos a, productos prod , almacenes al\
			where a.producto=prod.producto \
			group by a.producto, a.present, al.almacen";
		instrucciones[num_instrucciones++]=instruccion;

		// Calcula la existencia al momento del corte previo.

        if(bandExist == false){
			instruccion="insert into existenciastemp (producto, present, tipo, cantidad \
			, almacen, ventas, devventas, compras, devcompras, entradas, salidas, cantinicial) \
			select pce.producto, pce.present, 'IN' as tipo, \
			sum(pce.cantidad) as cantidad , pce.almacen, 0.000 as ventas, 0.000 as devventas, \
			0.000 as compras, 0.000 as devcompras, 0.000 as entradas, 0.000 as salidas, SUM(pce.cantidad) as cantinicial \
			from puntoscorteexistencias pce, productos prod \
			where pce.fecha=@fechacorte and pce.producto=prod.producto \
			group by pce.producto, pce.present, pce.almacen";
		instrucciones[num_instrucciones++]=instruccion;
		} else {
			instruccion="insert into existenciastemp (producto, present, tipo, cantidad \
			, almacen, ventas, devventas, compras, devcompras, entradas, salidas, cantinicial) \
			select pce.producto, pce.present, 'IN' as tipo, \
			sum(pce.cantidad) as cantidad , pce.almacen, 0.000 as ventas, 0.000 as devventas, \
			0.000 as compras, 0.000 as devcompras, 0.000 as entradas, 0.000 as salidas, SUM(pce.cantidad) as cantinicial \
			from existenciasiniciales pce, productos prod \
			where pce.producto=prod.producto \
			group by pce.producto, pce.present, pce.almacen";
		instrucciones[num_instrucciones++]=instruccion;
		}
		/////

		// Calcula las compras

		archivo_temp1=mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);
	instruccion.sprintf(" \
			SELECT a.producto, a.present, 'CO' as tipo, SUM(d.cantidad*a.factor) AS cantidad, c.almacen \
			, 0.000 as ventas, 0.000 as devventas, \
			SUM(d.cantidad*a.factor) as compras, 0.000 as devcompras, 0.000 as  entradas, 0.000 as salidas \
			, 0.000 as cantinicial \
			FROM compras c \
			INNER JOIN dcompras d ON c.referencia=d.referencia \
			INNER JOIN articulos a ON d.articulo=a.articulo \
			where c.fechacom>@fechacorte and c.fechacom<=@fechafinal and c.cancelado=0 \
			group by a.producto, a.present, c.almacen INTO OUTFILE '%s'",
			archivo_temp1);
		instrucciones[num_instrucciones++]=instruccion;
		instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE existenciastemp (producto, present, tipo, cantidad, \
		almacen, ventas, devventas, compras, devcompras, entradas, salidas, cantinicial) ",archivo_temp1);
		instrucciones[num_instrucciones++]=instruccion;

		// Calcula las devoluciones a las compras

		instruccion="insert into existenciastemp (producto, present, tipo, cantidad, almacen, \
			ventas, devventas, compras, devcompras, entradas, salidas, cantinicial) \
			select a.producto, a.present, 'DC' as tipo, (sum(d.cantidad*a.factor)*-1) as cantidad, \
			c.almacen, 0.000 as ventas, 0.000 as devventas, \
			0.000 as compras,(sum(d.cantidad*a.factor)) as devcompras, 0.000 as  entradas, 0.000 as salidas \
			, 0.000 as cantinicial \
			from notascredprov n, compras c, dnotascredprov d, articulos a, productos prod \
			where n.fechanot>@fechacorte and n.fechanot<=@fechafinal and n.tipo='0' \
			and n.referencia=d.referencia and n.compra=c.referencia and \
			n.cancelado=0 and d.articulo=a.articulo and a.producto=prod.producto \
			group by a.producto, a.present,c.almacen";
		instrucciones[num_instrucciones++]=instruccion;

		// Calcula las ventas.

		archivo_temp2=mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);
		instruccion.sprintf("select a.producto, a.present, 'VE' as tipo, (sum(d.cantidad*a.factor)*-1) as cantidad,d.almacen \
			,(sum(d.cantidad*a.factor)) as ventas, 0.000 as devventas, \
			0.000 as compras, 0.000 as devcompras, 0.000 as  entradas, 0.000 as salidas, 0.000 as cantinicial \
			from ventas v \
			inner join dventas d  on v.referencia=d.referencia \
			inner join articulos a on d.articulo=a.articulo \
			where v.fechavta>@fechacorte and v.fechavta<=@fechafinal and v.cancelado=0 \
			group by a.producto, a.present,d.almacen INTO OUTFILE '%s'",
			archivo_temp2);
		instrucciones[num_instrucciones++]=instruccion;
		instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE existenciastemp",archivo_temp2);
		instrucciones[num_instrucciones++]=instruccion;

		// Calcula las devoluciones a las ventas
		archivo_temp3=mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);
		instruccion.sprintf(" \
			select a.producto, a.present, 'DV' as tipo, sum(dn.cantidad*a.factor) as cantidad, dv.almacen \
			, 0.000 as ventas, sum(dn.cantidad*a.factor) as devventas, \
			0.000 as compras, 0.000 as devcompras, 0.000 as  entradas, 0.000 as salidas, 0.000 as cantinicial  \
			from notascredcli n \
			inner join dnotascredcli dn on n.referencia=dn.referencia \
			inner join ventas v on n.venta=v.referencia \
			inner join dventas dv on v.referencia=dv.referencia and dv.articulo=dn.articulo \
			inner join articulos a on dn.articulo=a.articulo \
			where n.fechanot>@fechacorte and n.fechanot<=@fechafinal and n.tipo='0' and n.cancelado=0 \
			group by a.producto, a.present, dv.almacen INTO OUTFILE '%s'",
			archivo_temp3);
		instrucciones[num_instrucciones++]=instruccion;
		instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE existenciastemp ",archivo_temp3);
		instrucciones[num_instrucciones++]=instruccion;

		// Calcula las entradas de almacén
		archivo_temp4=mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);
		instruccion.sprintf(" \
			select a.producto, a.present, 'EN' as tipo, sum(d.cantidad*a.factor) as cantidad, m.almaent \
			,0.000 as ventas, 0.000 as devventas, \
			0.000 as compras, 0.000 as devcompras, sum(d.cantidad*a.factor) as  entradas, 0.000 as salidas  \
			, 0.000 as cantinicial \
			from movalma m, dmovalma d, articulos a, productos prod \
			where m.fechamov>@fechacorte and m.fechamov<=@fechafinal and m.movimiento=d.movimiento \
			and m.tipo<>'S' \
			AND m.aplica=1 and m.cancelado=0 and d.articulo=a.articulo and a.producto=prod.producto \
			group by a.producto, a.present, m.almaent INTO OUTFILE '%s'",
			archivo_temp4);
		instrucciones[num_instrucciones++]=instruccion;
		instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE existenciastemp ",archivo_temp4);
		instrucciones[num_instrucciones++]=instruccion;

		// Calcula las salidas de almacén
		archivo_temp5=mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);
		instruccion.sprintf(" \
			select a.producto, a.present, 'SA' as tipo, (sum(d.cantidad*a.factor)*-1) as cantidad, m.almasal  \
			, 0.000 as ventas, 0.000 as devventas, \
			0.000 as compras, 0.000 as devcompras, 0.000 as  entradas,(sum(d.cantidad*a.factor)) as salidas  \
			, 0.000 as cantinicial \
			from movalma m, dmovalma d, articulos a, productos prod \
			where m.fechamov>@fechacorte and m.fechamov<=@fechafinal and m.movimiento=d.movimiento \
			and m.tipo<>'E' \
			AND m.aplica=1 and m.cancelado=0 and d.articulo=a.articulo and a.producto=prod.producto \
			group by a.producto, a.present, m.almasal  INTO OUTFILE '%s'",
			archivo_temp5);
		instrucciones[num_instrucciones++]=instruccion;
		instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE existenciastemp ",archivo_temp5);
		instrucciones[num_instrucciones++]=instruccion;

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		instruccion = "DELETE FROM existenciasactuales";
		instrucciones[num_instrucciones++]=instruccion;

		// Suma los movimientos para obtener las existencias

		instruccion = "insert into existenciasactuales (producto, present, cantidad, almacen, ventas, devventas \
				, compras, devcompras, entradas, salidas, cantinicial) \
				select e.producto, e.present, sum(e.cantidad) as cantidad, e.almacen, sum(e.ventas),   \
				sum(e.devventas), sum(e.compras), sum(e.devcompras), sum(e.entradas), sum(e.salidas), sum(e.cantinicial)  \
				from existenciastemp e \
				group by e.producto, e.present, e.almacen";
		instrucciones[num_instrucciones++]=instruccion;

		instruccion="COMMIT";
		instrucciones[num_instrucciones++]=instruccion;

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=1";

		instruccion="SET SESSION sql_log_bin = 1";
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("INSERT INTO bitacoraservidor VALUES (NULL,'%s',NOW(),'FIN_ID_GRA_EXISTENCIAS_ACTUALES')",SucursalOrigen);
		instrucciones[num_instrucciones++]=instruccion;

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_REP);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {

			mServidorVioleta->BorraArchivoTemp(archivo_temp1);
			mServidorVioleta->BorraArchivoTemp(archivo_temp2);
			mServidorVioleta->BorraArchivoTemp(archivo_temp3);
			mServidorVioleta->BorraArchivoTemp(archivo_temp4);
			mServidorVioleta->BorraArchivoTemp(archivo_temp5);

		}
	} __finally {
		if (resp_almacenes!=NULL) delete resp_almacenes;
		delete buffer_sql;
	}
}

//---------------------------------------------------------------------------
//ID_GUARDARBITACORA
void ServidorAdminSistema::GuardarEnBitacora(char *parametros)
{
	AnsiString mensaje;
	mensaje=mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->MuestraMensaje("-- "+ mensaje);
}

//---------------------------------------------------------------------------
//ID_GRA_BITA_FECHACORTE
void ServidorAdminSistema::GrabaBitacoraFechaCorte(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
	AnsiString fecha_nueva,fecha_anti, terminal, usuario, sucursal;
	AnsiString dias_antiguedad, dias_ticket, limit_fact;
	AnsiString fecha_nueva_sol, fecha_anti_sol;

	fecha_nueva = mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
	fecha_anti = mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
	usuario=mFg.ExtraeStringDeBuffer(&parametros);
	terminal=mFg.ExtraeStringDeBuffer(&parametros);
	sucursal =mFg.ExtraeStringDeBuffer(&parametros);
	dias_antiguedad = mFg.ExtraeStringDeBuffer(&parametros);
	dias_ticket = mFg.ExtraeStringDeBuffer(&parametros);
	limit_fact = mFg.ExtraeStringDeBuffer(&parametros);
	fecha_nueva_sol = mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
	fecha_anti_sol = mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	instruccion.sprintf("UPDATE estadosistemaemp SET valor='%s' WHERE estado='FUCIERRE' AND sucursal = '%s' ", fecha_nueva, FormServidor->ObtieneClaveSucursal() );

	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error UPDATE estadosistemaemp de Fecha de corte"));

	instruccion.sprintf("UPDATE estadosistemaemp SET valor='%s' WHERE estado='FECHCIERSOLNCRE' AND sucursal = '%s' ", fecha_nueva_sol, FormServidor->ObtieneClaveSucursal() );

	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error UPDATE estadosistemaemp de Fecha de solicitud de notas de crédito"));

	if (dias_antiguedad != "") {
		instruccion.sprintf("UPDATE parametrosglobemp SET valor = '%s' WHERE parametro = 'DIASIMPVTA' AND idempresa = %s ", dias_antiguedad, FormServidor->ObtieneClaveEmpresa());
		if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
			throw (Exception("Error UPDATE parametrosglobemp de Fecha de corte"));
	}

	if (dias_ticket != "") {
		instruccion.sprintf("UPDATE parametrosemp SET valor = '%s' WHERE parametro = 'DIASANTIGUEDAD' AND sucursal = '%s' ", dias_ticket, FormServidor->ObtieneClaveSucursal());
		if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
			throw (Exception("Error UPDATE parametrosemp de DIASANTIGUEDAD"));
	}

	if (limit_fact != "") {
		instruccion.sprintf("UPDATE parametrosemp SET valor = '%s' WHERE parametro = 'LIMITEFACT' AND sucursal = '%s' ", limit_fact, FormServidor->ObtieneClaveSucursal());
		if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
			throw (Exception("Error UPDATE parametrosemp de LIMITEFACT"));
	}

	if(usuario.Trim() != "" && terminal.Trim() != ""){
		//agregar el movimiento a la bitacora
		instruccion.sprintf("insert into bitacoracambiofechacorte \
		(clave_usuario,fecha_modi,hora_modi,fecha_corte_ant,fecha_corte_nue,terminal,sucursal) \
		values ('%s', CURDATE(), CURTIME(), '%s', '%s', '%s', '%s')", usuario, fecha_anti ,
		fecha_nueva , terminal,sucursal);
		if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
			throw (Exception("Error insert into bitacoracambiofechacorte "));
	}
	//forzar la liberacion de buffer
	instruccion.sprintf("select 0 as error");
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}
//---------------------------------------------------------------------------
// ID_CON_ESTADOREPLICACION
void ServidorAdminSistema::ConsultaReplicacionServidor(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{

	// variables para acceder a los valores que estan en parametros
	//int num_instrucciones=0;
    AnsiString instruccion;
	AnsiString sucursal;
	AnsiString consulta, cadena_consulta = "";
	AnsiString tipoconexion;

	tipoconexion=mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	//obtiene el estado de la replicacion
  	instruccion.sprintf("SHOW ALL slaves STATUS");

   /* para pruebas 	if(tipoconexion == "LOCAL")
			instruccion="SELECT * FROM tablafalsa WHERE locales = '1'";
			else
				instruccion="SELECT * FROM tablafalsa WHERE locales = '0'";     */

	if(!mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(),Respuesta->TamBufferResultado))
		throw Exception("No tienes acceso a la informacion de replicación");

	// Busca la ultima fecha donde hay precalculo de costos
	BufferRespuestas* resp_sucursal=NULL;
	try {
		if(tipoconexion == "LOCAL")
			instruccion="SELECT sucursal FROM sucursalesreplicacion WHERE esprincipal = '1'";
			else
				instruccion="SELECT sucursal FROM sucursalesreplicacion WHERE esprincipal = '0'";

		if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_sucursal)) {
			if (resp_sucursal->ObtieneNumRegistros()>0){

				for(int i = 0; i < resp_sucursal->ObtieneNumRegistros(); i++){

						sucursal = resp_sucursal->ObtieneDato("sucursal");

						consulta = "(SELECT suc.sucursal, CONCAT(v.referencia,' ',DATE_FORMAT(v.fechaalta,'%d-%m-%Y'),' ',v.horaalta) AS ultventa,";
						consulta+= " CONCAT(DATE_FORMAT(v.fechaalta,'%d/%m/%Y'),' ',v.horaalta) AS ultfecha, 'VENTA' as info ";
						consulta+= " FROM ventas v";
						consulta+= " INNER JOIN terminales t ON v.terminal = t.terminal";
						consulta+= " INNER JOIN secciones s ON t.seccion = s.seccion";
						consulta+= " INNER JOIN sucursales suc ON suc.sucursal = s.sucursal";
						consulta+= " WHERE suc.sucursal = '";
						consulta+= sucursal;
						consulta+= "' AND v.fechavta >= DATE_SUB(CURDATE(), INTERVAL 7 DAY) AND v.fechavta <= CURDATE() ";
						consulta+= " ORDER BY v.fechavta DESC, v.horaalta DESC LIMIT 1)";
						consulta+= " UNION ALL";
						consulta+= " (SELECT suc.sucursal, CONCAT(p.referencia,' ',DATE_FORMAT(p.fechaalta,'%d-%m-%Y'),' ',p.horaalta) AS ultpedido,";
						consulta+= " CONCAT(DATE_FORMAT(p.fechaalta,'%d/%m/%Y'),' ',p.horaalta) AS ultfecha, 'PEDIDO' as info ";
						consulta+= " FROM pedidosventa p";
						consulta+= " INNER JOIN terminales t ON p.terminal = t.terminal";
						consulta+= " INNER JOIN secciones s ON t.seccion = s.seccion";
						consulta+= " INNER JOIN sucursales suc ON suc.sucursal = s.sucursal";
						consulta+= " WHERE suc.sucursal = '";
						consulta+= sucursal;
						consulta+= "'  AND p.fechaped >= DATE_SUB(CURDATE(), INTERVAL 7 DAY) AND p.fechaped <= CURDATE() ";
						consulta+= " ORDER BY p.fechaped DESC, p.horaalta DESC LIMIT 1)";
						consulta+= " UNION ALL ";
						consulta+= " (SELECT sucursal, CONCAT(usuario,' ',DATE_FORMAT(fecha,'%d-%m-%Y'),' ',hora) AS ultlog,";
						consulta+= " CONCAT(DATE_FORMAT(fecha,'%d/%m/%Y'),' ',hora) AS ultfecha, 'LOGIN' as info ";
						consulta+= "  FROM bitacorausuario";
						consulta+= " WHERE fecha = (SELECT MAX(fecha) FROM bitacorausuario WHERE sucursal = '";
						consulta+= sucursal;
						consulta+= "' AND hora = (SELECT MAX(hora) FROM bitacorausuario WHERE fecha = (SELECT MAX(fecha) FROM";
						consulta+= " bitacorausuario WHERE sucursal = '";
						consulta+= sucursal;
						consulta+= "') AND sucursal = '";
						consulta+= sucursal;
						consulta+= "' )) AND sucursal = '";
						consulta+= sucursal;
						consulta+= "' LIMIT 1) ";

					 if(cadena_consulta != "")
						cadena_consulta = cadena_consulta + " UNION ALL ";
					 cadena_consulta = cadena_consulta + consulta;
					 resp_sucursal->IrAlSiguienteRegistro();
				}
			} else throw (Exception("No se encuentran registros en tabla sucursalesreplicacion"));
		} else throw (Exception("Error al consultar en tabla sucursalesreplicacion"));
	} __finally {
		if (resp_sucursal!=NULL) delete resp_sucursal;
	}


	// Obtiene la ultima venta, el último pedido y el ultimo registro del usuario
  	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, cadena_consulta.c_str(), Respuesta->TamBufferResultado);

}
//---------------------------------------------------------------------------
// ID_CON_BITACORAUNIFICADA
void ServidorAdminSistema::ConsultaBitacoraUnificada(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	int i;
	int error=0;
	AnsiString instruccion;
	AnsiString fechaini,fechafin, terminal, usuarios, sucursal;

	fechaini = mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
	fechafin = mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
	usuarios=mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ*2);


		instruccion="(SELECT v.referencia as referencia, 'VENT' AS tipodocumento, 'ALTA' AS operacion, v.cancelado, v.fechavta AS fechadoc,";
		instruccion+="v.usualta AS usuariooper, v.fechaalta AS fechaoper, v.horaalta AS horaoper ";
		instruccion+="FROM ventas v ";
		instruccion+="WHERE v.cancelado=0 AND v.fechaalta BETWEEN '"+fechaini+"' AND '"+fechafin+"' AND v.usualta IN ("+usuarios+"))";
		instruccion+="UNION";
		instruccion+="(SELECT v.referencia as  referencia, 'VENT' AS tipodocumento, 'MODI' AS operacion, v.cancelado, v.fechavta AS fechadoc,";
		instruccion+="v.usumodi AS usuariooper, v.fechamodi AS fechaoper, v.horamodi AS horaoper";
		instruccion+="	FROM ventas v ";
		instruccion+="WHERE (v.fechaalta<>v.fechamodi OR v.horaalta<>v.horamodi) AND v.cancelado=0 AND v.fechamodi BETWEEN '"+fechaini+"' AND '"+fechafin+"' ";
		instruccion+="AND v.usumodi IN ("+usuarios+")) ";
		instruccion+="UNION ";
		instruccion+="(SELECT v.referencia as referencia, 'VENT' AS tipodocumento, 'CANC' AS operacion, v.cancelado, v.fechavta AS fechadoc,";
		instruccion+="v.usumodi AS usuariooper, v.fechamodi AS fechaoper, v.horamodi AS horaoper ";
		instruccion+="	FROM ventas v ";
		instruccion+="WHERE v.cancelado=1 AND v.fechamodi BETWEEN '"+fechaini+"' AND '"+fechafin+"' AND v.usumodi IN ("+usuarios+"))";
		instruccion+="	UNION ";
		instruccion+="(SELECT c.referencia as referencia, 'COMP' AS tipodocumento, 'ALTA' AS operacion, c.cancelado, c.fechacom AS fechadoc,";
		instruccion+="c.usualta AS usuariooper, c.fechaalta AS fechaoper, c.horaalta AS horaoper";
		instruccion+=" FROM compras c ";
		instruccion+="WHERE c.cancelado=0 AND c.fechaalta BETWEEN '"+fechaini+"' AND '"+fechafin+"' AND c.usualta IN ("+usuarios+"))";
		instruccion+="UNION";
		instruccion+="(SELECT c.referencia as referencia, 'COMP' AS tipodocumento, 'MODI' AS operacion, c.cancelado, c.fechacom AS fechadoc,";
		instruccion+="c.usumodi AS usuariooper, c.fechamodi AS fechaoper, c.horaalta AS horaoper";
		instruccion+="	FROM compras c ";
		instruccion+="WHERE (c.fechaalta<>c.fechamodi OR c.horaalta<>c.horamodi) AND c.cancelado=0 ";
		instruccion+=" AND c.fechamodi BETWEEN '"+fechaini+"' AND '"+fechafin+"' AND c.usumodi IN ("+usuarios+"))";
		instruccion+="UNION  ";
		instruccion+="(SELECT c.referencia as referencia, 'COMP' AS tipodocumento, 'CANC' AS operacion, c.cancelado, c.fechacom AS fechadoc,";
		instruccion+="c.usumodi AS usuariooper, c.fechamodi AS fechaoper, c.horaalta AS horaoper ";
		instruccion+="	FROM compras c ";
		instruccion+="WHERE c.cancelado=1 AND c.fechamodi BETWEEN '"+fechaini+"' AND '"+fechafin+"' AND c.usumodi IN ("+usuarios+"))";
		instruccion+="	UNION ";
		instruccion+="(SELECT ncc.referencia as referencia,'NCREDCLI' AS tipodocumento, 'ALTA' AS operacion, ncc.cancelado, ncc.fechanot AS fechadoc, ";
		instruccion+="	ncc.usualta AS usuariooper,  ncc.fechaalta AS fechaoper, ncc.horaalta AS horaoper ";
		instruccion+="FROM notascredcli ncc ";
		instruccion+="WHERE ncc.cancelado = 0 AND ncc.fechaalta BETWEEN '"+fechaini+"' AND '"+fechafin+"' AND ncc.usualta IN ("+usuarios+")) ";
		instruccion+="UNION ";
		instruccion+="(SELECT ncc.referencia as referencia,'NCREDCLI' AS tipodocumento, 'MODI' AS operacion, ncc.cancelado, ncc.fechanot AS fechadoc, ";
		instruccion+="ncc.usumodi AS usuariooper,  ncc.fechamodi AS fechaoper, ncc.horamodi AS horaoper ";
		instruccion+="	FROM notascredcli ncc ";
		instruccion+="WHERE (ncc.fechaalta<>ncc.fechamodi OR ncc.horaalta<>ncc.horamodi) AND ncc.cancelado = 0 AND ncc.fechamodi ";
		instruccion+="BETWEEN '"+fechaini+"' AND '"+fechafin+"' AND ncc.usumodi IN ("+usuarios+"))";
		instruccion+="UNION  ";
		instruccion+="(SELECT ncc.referencia as referencia,'NCREDCLI' AS tipodocumento, 'CANC' AS operacion, ncc.cancelado, ncc.fechanot AS fechadoc,";
		instruccion+="ncc.usumodi AS usuariooper,  ncc.fechamodi AS fechaoper, ncc.horamodi AS horaoper ";
		instruccion+="	FROM notascredcli ncc ";
		instruccion+="WHERE ncc.cancelado = 1 AND ncc.fechamodi BETWEEN '"+fechaini+"' AND '"+fechafin+"' AND ncc.usumodi IN ("+usuarios+")) ";
		instruccion+="UNION ";
		instruccion+="(SELECT ncp.referencia as referencia,'NCREDPROV' AS tipodocumento, 'ALTA' AS operacion, ncp.cancelado, ncp.fechanot AS fechadoc, ";
		instruccion+="ncp.usualta AS usuariooper, ncp.fechaalta AS fechaoper, ncp.horaalta AS horaoper ";
		instruccion+="	FROM notascredprov ncp ";
		instruccion+="	WHERE ncp.cancelado = 0 AND ncp.fechaalta BETWEEN '"+fechaini+"' AND '"+fechafin+"' AND ncp.usualta IN ("+usuarios+")) ";
		instruccion+="UNION ";
		instruccion+="(SELECT ncp.referencia as referencia,'NCREDPROV' AS tipodocumento, 'MODI' AS operacion, ncp.cancelado, ncp.fechanot AS fechadoc,";
		instruccion+="ncp.usumodi AS usuariooper,  ncp.fechamodi AS fechaoper, ncp.horamodi AS horaoper ";
		instruccion+="	FROM notascredcli ncp ";
		instruccion+="	WHERE (ncp.fechaalta<>ncp.fechamodi OR ncp.horaalta<>ncp.horamodi) AND ncp.cancelado = 0 ";
		instruccion+="AND ncp.fechamodi BETWEEN '"+fechaini+"' AND '"+fechafin+"' AND ncp.usumodi IN ("+usuarios+"))" ;
		instruccion+="UNION";
		instruccion+="(SELECT ncp.referencia as referencia,'NCREDPROV' AS tipodocumento, 'CANC' AS operacion, ncp.cancelado, ncp.fechanot AS fechadoc,";
		instruccion+="ncp.usumodi AS usuariooper,  ncp.fechamodi AS fechaoper, ncp.horamodi AS horaoper";
		instruccion+="	FROM notascredcli ncp ";
		instruccion+=" WHERE ncp.cancelado = 1 AND ncp.fechamodi BETWEEN '"+fechaini+"' AND '"+fechafin+"' AND ncp.usumodi IN ("+usuarios+")) ";
		instruccion+=" UNION ";
		instruccion+="(SELECT ncc.referencia as referencia ,'NCARGOCLI' AS tipodocumento, 'ALTA' AS operacion, ncc.cancelado, ncc.fechanot AS fechadoc,";
		instruccion+="ncc.usualta AS usuariooper, ncc.fechaalta AS fechaoper, ncc.horaalta AS horaoper ";
		instruccion+="	FROM notascarcli ncc ";
		instruccion+="	WHERE ncc.cancelado = 0 AND ncc.fechaalta BETWEEN '"+fechaini+"' AND '"+fechafin+"' AND ncc.usualta IN ("+usuarios+")) ";
		instruccion+=" UNION ";
		instruccion+=" (SELECT ncc.referencia as referencia,'NCARGOCLI' AS tipodocumento, 'MODI' AS operacion, ncc.cancelado, ncc.fechanot AS fechadoc, ";
		instruccion+=" ncc.usumodi AS usuariooper,  ncc.fechamodi AS fechaoper, ncc.horamodi AS horaoper  ";
		instruccion+="	FROM notascarcli ncc  ";
		instruccion+=" WHERE (ncc.fechaalta<>ncc.fechamodi OR ncc.horaalta<>ncc.horamodi) AND ncc.cancelado = 0 AND ncc.fechaalta ";
		instruccion+=" BETWEEN '"+fechaini+"' AND '"+fechafin+"' AND ncc.usualta IN ("+usuarios+")) ";
		instruccion+=" UNION ";
		instruccion+=" (SELECT ncc.referencia as referencia,'NCARGOCLI' AS tipodocumento, 'CANC' AS operacion, ncc.cancelado, ncc.fechanot AS fechadoc,";
		instruccion+=" ncc.usumodi AS usuariooper,  ncc.fechamodi AS fechaoper, ncc.horamodi AS horaoper ";
		instruccion+=" FROM notascarcli ncc  ";
		instruccion+=" WHERE ncc.cancelado = 1 AND ncc.fechaalta BETWEEN '"+fechaini+"' AND '"+fechafin+"' AND ncc.usualta IN ("+usuarios+")) ";
		instruccion+=" UNION ";
		instruccion+=" (SELECT ncp.referencia as referencia,'NCARGOPROV' AS tipodocumento, 'ALTA' AS operacion, ncp.cancelado, ncp.fechanot AS fechadoc, ";
		instruccion+=" ncp.usualta AS usuariooper, ncp.fechaalta AS fechaoper, ncp.horaalta AS horaoper ";
		instruccion+=" FROM notascarprov ncp WHERE ncp.cancelado = 0 AND ncp.fechaalta BETWEEN '"+fechaini+"' AND '"+fechafin+"' AND ncp.usualta IN ("+usuarios+")) ";
		instruccion+=" UNION  ";
		instruccion+=" (SELECT ncp.referencia as referencia,'NCARGOPROV' AS tipodocumento, 'MODI' AS operacion, ncp.cancelado, ncp.fechanot AS fechadoc, ";
		instruccion+=" ncp.usumodi AS usuariooper,  ncp.fechamodi AS fechaoper, ncp.horamodi AS horaoper ";
		instruccion+=" FROM notascarprov ncp WHERE (ncp.fechaalta<>ncp.fechamodi OR ncp.horaalta<>ncp.horamodi) AND  ";
		instruccion+=" ncp.cancelado = 0 AND ncp.fechaalta BETWEEN '"+fechaini+"' AND '"+fechafin+"' AND ncp.usumodi IN ("+usuarios+")) ";
		instruccion+=" UNION ";
		instruccion+=" (SELECT ncp.referencia as referencia,'NCARGOPROV' AS tipodocumento, 'CANC' AS operacion, ncp.cancelado, ncp.fechanot AS fechadoc,";
		instruccion+=" ncp.usumodi AS usuariooper,  ncp.fechamodi AS fechaoper, ncp.horamodi AS horaoper ";
		instruccion+=" FROM notascarprov ncp WHERE ncp.cancelado = 1 AND ncp.fechamodi BETWEEN '"+fechaini+"' AND '"+fechafin+"' AND ncp.usumodi IN ("+usuarios+"))";
		instruccion+=" UNION ";
		instruccion+=" (SELECT ma.movimiento as referencia,'MOVALMA' AS tipodocumento, 'ENT-ALTA' AS operacion, ma.cancelado, ma.fechamov AS fechadoc, ";
		instruccion+=" ma.usualta AS usuariooper, ma.fechaalta AS fechaoper, ma.horaalta AS horaoper ";
		instruccion+=" FROM movalma ma WHERE ma.tipo = 'E' AND ma.cancelado = 0 AND ma.fechaalta BETWEEN '"+fechaini+"' AND '"+fechafin+"' ";
		instruccion+=" AND ma.usualta IN ("+usuarios+"))  ";
		instruccion+=" UNION  ";
		instruccion+=" (SELECT ma.movimiento as referencia,'MOVALMA' AS tipodocumento, 'ENT-MODI' AS operacion, ma.cancelado, ma.fechamov AS fechadoc, ";
		instruccion+=" 	ma.usumodi AS usuariooper, ma.fechamodi AS fechaoper, ma.horamodi AS horaoper  ";
		instruccion+=" FROM movalma ma ";
		instruccion+=" WHERE ma.tipo = 'E' AND ma.cancelado = 0 AND (ma.fechaalta<>ma.fechamodi OR ma.horaalta<>ma.horamodi)  ";
		instruccion+=" AND ma.fechamodi BETWEEN '"+fechaini+"' AND '"+fechafin+"' AND ma.usumodi IN ("+usuarios+"))";
		instruccion+=" UNION ";
		instruccion+=" (SELECT ma.movimiento as referencia,'MOVALMA' AS tipodocumento, 'ENT-CANC' AS operacion, ma.cancelado, ma.fechamov AS fechadoc, ";
		instruccion+=" ma.usumodi AS usuariooper, ma.fechamodi AS fechaoper, ma.horamodi AS horaoper";
		instruccion+=" FROM movalma ma ";
		instruccion+=" WHERE ma.tipo = 'E' AND ma.cancelado = 1 AND ma.fechamodi BETWEEN '"+fechaini+"' AND '"+fechafin+"' AND ma.usumodi IN ("+usuarios+"))";
		instruccion+=" UNION ";
		instruccion+=" (SELECT ma.movimiento as referencia,'MOVALMA' AS tipodocumento, 'SAL-ALTA' AS operacion, ma.cancelado, ma.fechamov AS fechadoc, ";
		instruccion+=" ma.usualta AS usuariooper, ma.fechaalta AS fechaoper, ma.horaalta AS horaoper  ";
		instruccion+=" FROM movalma ma ";
		instruccion+=" WHERE ma.tipo = 'S' AND ma.cancelado = 0 AND ma.fechaalta BETWEEN '"+fechaini+"' AND '"+fechafin+"' AND ma.usualta IN ("+usuarios+")) ";
		instruccion+=" UNION ";
		instruccion+=" (SELECT ma.movimiento as referencia,'MOVALMA' AS tipodocumento, 'SAL-MODI' AS operacion, ma.cancelado, ma.fechamov AS fechadoc, ";
		instruccion+=" ma.usumodi AS usuariooper, ma.fechamodi AS fechaoper, ma.horamodi AS horaoper  ";
		instruccion+=" FROM movalma ma ";
		instruccion+=" WHERE ma.tipo = 'S' AND ma.cancelado = 0 AND (ma.fechaalta<>ma.fechamodi OR ma.horaalta<>ma.horamodi) ";
		instruccion+=" AND ma.fechamodi BETWEEN '"+fechaini+"' AND '"+fechafin+"' AND ma.usumodi IN ("+usuarios+"))  ";
		instruccion+=" UNION  ";
		instruccion+=" (SELECT ma.movimiento as referencia,'MOVALMA' AS tipodocumento, 'SAL-CANC' AS operacion, ma.cancelado, ma.fechamov AS fechadoc, ";
		instruccion+=" ma.usumodi AS usuariooper, ma.fechamodi AS fechaoper, ma.horamodi AS horaoper FROM movalma ma WHERE ma.tipo = 'S' ";
		instruccion+=" AND ma.cancelado = 1 AND ma.fechamodi BETWEEN '"+fechaini+"' AND '"+fechafin+"' AND ma.usumodi IN ("+usuarios+")) ";
		instruccion+=" UNION ";
		instruccion+=" (SELECT ma.movimiento as referencia,'MOVALMA' AS tipodocumento, 'TRASP-ALTA' AS operacion, ma.cancelado, ma.fechamov AS fechadoc, ";
		instruccion+=" ma.usualta AS usuariooper, ma.fechaalta AS fechaoper, ma.horaalta AS horaoper  ";
		instruccion+=" FROM movalma ma  ";
		instruccion+=" WHERE ma.tipo = 'T' AND ma.cancelado = 0 AND ma.fechaalta BETWEEN '"+fechaini+"' AND '"+fechafin+"' AND ma.usualta IN ("+usuarios+")) ";
		instruccion+=" UNION  ";
		instruccion+=" (SELECT ma.movimiento as referencia,'MOVALMA' AS tipodocumento, 'TRASP-MODI' AS operacion, ma.cancelado, ma.fechamov AS fechadoc, ";
		instruccion+=" ma.usumodi AS usuariooper, ma.fechamodi AS fechaoper, ma.horamodi AS horaoper  ";
		instruccion+=" FROM movalma ma ";
		instruccion+=" WHERE ma.tipo = 'T' AND ma.cancelado = 0 AND (ma.fechaalta<>ma.fechamodi OR ma.horaalta<>ma.horamodi) ";
		instruccion+=" AND ma.fechamodi BETWEEN '"+fechaini+"' AND '"+fechafin+"' AND ma.usumodi IN ("+usuarios+")) ";
		instruccion+=" UNION ";
		instruccion+=" (SELECT ma.movimiento as referencia,'MOVALMA' AS tipodocumento, 'TRASP-CANC' AS operacion, ma.cancelado, ma.fechamov AS fechadoc, ";
		instruccion+=" ma.usumodi AS usuariooper, ma.fechamodi AS fechaoper, ma.horamodi AS horaoper ";
		instruccion+=" FROM movalma ma ";
		instruccion+=" WHERE ma.tipo = 'T' AND ma.cancelado = 1 AND ma.fechamodi BETWEEN '"+fechaini+"' AND '"+fechafin+"' AND ma.usumodi IN ("+usuarios+")) ";
		instruccion+=" UNION  ";
		instruccion+=" (SELECT pc.pago as referencia,'PAGOSCLI' AS tipodocumento, 'ALTA' AS operacion, pc.cancelado, pc.fecha AS fechadoc,  ";
		instruccion+=" pc.usualta AS usuariooper, pc.fecha AS fechaoper, pc.hora AS horaoper   ";
		instruccion+=" FROM pagoscli pc  ";
		instruccion+=" WHERE pc.cancelado = 0 AND pc.fecha BETWEEN '"+fechaini+"' AND '"+fechafin+"' AND pc.usualta IN ("+usuarios+"))  ";
		instruccion+=" UNION ";
		instruccion+=" (SELECT pc.pago as referencia,'PAGOSCLI' AS tipodocumento, 'MODI' AS operacion, pc.cancelado, pc.fecha AS fechadoc, ";
		instruccion+=" pc.usumodi AS usuariooper, pc.fechamodi AS fechaoper, pc.horamodi AS horaoper ";
		instruccion+=" FROM pagoscli pc  ";
		instruccion+=" WHERE pc.cancelado = 0 AND (pc.fecha <>pc.fechamodi AND pc.hora <> pc.horamodi) ";
		instruccion+=" AND pc.fechamodi BETWEEN '"+fechaini+"' AND '"+fechafin+"' AND pc.usumodi IN ("+usuarios+")) ";
		instruccion+=" UNION ";
		instruccion+=" (SELECT pc.pago as referencia,'PAGOSCLI' AS tipodocumento, 'CANC' AS operacion, pc.cancelado, pc.fecha AS fechadoc, ";
		instruccion+=" pc.usumodi AS usuariooper, pc.fechamodi AS fechaoper, pc.horamodi AS horaoper";
		instruccion+=" FROM pagoscli pc ";
		instruccion+=" WHERE pc.cancelado = 1 AND pc.fechamodi BETWEEN '"+fechaini+"' AND '"+fechafin+"' AND pc.usumodi IN ("+usuarios+"))";
		instruccion+=" UNION ";
		instruccion+=" (SELECT pp.pago as referencia,'PAGOSPROV' AS tipodocumento, 'ALTA' AS operacion, pp.cancelado, pp.fecha AS fechadoc,";
		instruccion+=" pp.usualta AS usuariooper, pp.fecha AS fechaoper, pp.hora AS horaoper ";
		instruccion+=" FROM pagosprov pp ";
		instruccion+=" WHERE pp.cancelado = 0 AND pp.fecha BETWEEN '"+fechaini+"' AND '"+fechafin+"' AND pp.usualta IN ("+usuarios+"))";
		instruccion+=" UNION ";
		instruccion+=" (SELECT pp.pago as referencia,'PAGOSPROV' AS tipodocumento, 'MODI' AS operacion, pp.cancelado, pp.fecha AS fechadoc, ";
		instruccion+=" pp.usumodi AS usuariooper, pp.fechamodi AS fechaoper, pp.horamodi AS horaoper ";
		instruccion+=" FROM pagosprov pp WHERE pp.cancelado = 0 AND (pp.fecha <> pp.fechamodi AND pp.hora <> pp.horamodi) ";
		instruccion+=" AND pp.fechamodi BETWEEN '"+fechaini+"' AND '"+fechafin+"' AND pp.usumodi IN ("+usuarios+")) ";
		instruccion+=" UNION ";
		instruccion+=" (SELECT pp.pago as referencia,'PAGOSPROV' AS tipodocumento, 'CANC' AS operacion, pp.cancelado, pp.fecha AS fechadoc,";
		instruccion+=" pp.usumodi AS usuariooper, pp.fechamodi AS fechaoper, pp.horamodi AS horaoper ";
		instruccion+=" FROM pagosprov pp ";
		instruccion+=" WHERE pp.cancelado = 1 AND pp.fechamodi BETWEEN '"+fechaini+"' AND '"+fechafin+"' AND pp.usumodi IN ("+usuarios+")) ";
		instruccion+=" UNION  ";
		instruccion+=" (SELECT pvta.referencia as referencia, 'PEDVENTA' AS tipodocumento, 'ALTA' AS operacion, pvta.cancelado, pvta.fechaped AS fechadoc,";
		instruccion+="  pvta.usumodi AS usuariooper, pvta.fechamodi AS fechaoper, pvta.horamodi AS horaoper ";
		instruccion+=" FROM pedidosventa pvta ";
		instruccion+=" WHERE  pvta.cancelado=0 AND pvta.fechamodi BETWEEN '"+fechaini+"' AND '"+fechafin+"' AND pvta.usumodi IN ("+usuarios+") ) ";
		instruccion+=" UNION ";
		instruccion+=" (SELECT pvta.referencia as referencia, 'PEDVENTA' AS tipodocumento, 'MODI' AS operacion, pvta.cancelado, pvta.fechaped AS fechadoc,";
		instruccion+=" pvta.usumodi AS usuariooper, pvta.fechamodi AS fechaoper, pvta.horamodi AS horaoper";
		instruccion+=" FROM pedidosventa pvta ";
		instruccion+=" WHERE (pvta.fechaalta<>pvta.fechamodi OR pvta.horaalta<>pvta.horamodi) AND pvta.cancelado=1";
		instruccion+="  AND pvta.fechamodi BETWEEN '"+fechaini+"' AND '"+fechafin+"' AND pvta.usumodi IN ("+usuarios+") )  ";
		instruccion+="  UNION  ";
		instruccion+=" (SELECT pvta.referencia as referencia , 'PEDVENTA' AS tipodocumento, 'CANC' AS operacion, pvta.cancelado, pvta.fechaped AS fechadoc, ";
		instruccion+=" pvta.usumodi AS usuariooper, pvta.fechamodi AS fechaoper, pvta.horamodi AS horaoper ";
		instruccion+=" FROM pedidosventa pvta ";
		instruccion+=" WHERE pvta.cancelado=1 AND pvta.fechamodi BETWEEN '"+fechaini+"' AND '"+fechafin+"' AND pvta.usumodi IN ("+usuarios+") )";
		instruccion+=" UNION  ";
		instruccion+=" (SELECT pcom.referencia, 'PEDCOMPRA' AS tipodocumento, 'ALTA' AS operacion, pcom.cancelado, pcom.fechaped AS fechadoc,";
		instruccion+=" pcom.usumodi AS usuariooper, pcom.fechamodi AS fechaoper, pcom.horamodi AS horaoper ";
		instruccion+=" FROM pedidos pcom WHERE  pcom.cancelado=0 ";
		instruccion+=" AND pcom.fechamodi BETWEEN '"+fechaini+"' AND '"+fechafin+"' AND pcom.usumodi IN ("+usuarios+") )";
		instruccion+=" UNION ";
		instruccion+=" (SELECT pcom.referencia as referencia, 'PEDCOMPRA' AS tipodocumento, 'MODI' AS operacion, pcom.cancelado, pcom.fechaped AS fechadoc,";
		instruccion+="  pcom.usumodi AS usuariooper, pcom.fechamodi AS fechaoper, pcom.horamodi AS horaoper ";
		instruccion+=" FROM pedidos pcom";
		instruccion+=" WHERE (pcom.fechaalta<>pcom.fechamodi OR pcom.horaalta<>pcom.horamodi) AND pcom.cancelado=1 ";
		instruccion+="  AND pcom.fechamodi BETWEEN '"+fechaini+"' AND '"+fechafin+"' AND pcom.usumodi IN ("+usuarios+") )  ";
		instruccion+="  UNION  ";
		instruccion+=" (SELECT pcom.referencia as referencia , 'PEDCOMPRA' AS tipodocumento, 'CANC' AS operacion, pcom.cancelado, pcom.fechaped AS fechadoc, ";
		instruccion+=" pcom.usumodi AS usuariooper, pcom.fechamodi AS fechaoper, pcom.horamodi AS horaoper ";
		instruccion+=" FROM pedidos pcom";
		instruccion+=" WHERE  pcom.cancelado=1 AND pcom.fechamodi BETWEEN '"+fechaini+"' AND '"+fechafin+"' AND pcom.usumodi IN ("+usuarios+") ) ";
		instruccion+="ORDER BY fechaoper DESC, horaoper DESC, referencia";

		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

}
//---------------------------------------------------------------------------
// ID_AES_GOOGLEMAPS_API
 void ServidorAdminSistema::GoogleMapsAPIKey(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{

	// CONSULTA LLAVES CON AES_DECRYPT

	AnsiString instruccion;
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	instruccion = mFg.ExtraeStringDeBuffer(&parametros);

	instruccion="SELECT AES_DECRYPT( valor ,'LaVioletaSaSvMartinCastrejon2019GoogleMapsAPI') as valor FROM paramseguridad where parametro='"+instruccion+"' AND idempresa = "+FormServidor->ObtieneClaveEmpresa();
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

}
//---------------------------------------------------------------------------
// ID_AES_RPM_POSCREDEDNTIALS
void ServidorAdminSistema::ObtieneTokenRPM(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros){
 	// CONSULTA LLAVES CON AES_DECRYPT

	AnsiString instruccion;
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	instruccion = mFg.ExtraeStringDeBuffer(&parametros);

	instruccion="SELECT AES_DECRYPT( valor ,'1nT3&?4C/0nR9mlaVI0L3NT4') as valor FROM paramseguridad where parametro='"+instruccion+"' AND idempresa = "+FormServidor->ObtieneClaveEmpresa();
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}
//---------------------------------------------------------------------------
// ID_AES_SHOPIFY_API
 void ServidorAdminSistema::ShopifyAPIKey(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{

	// CONSULTA LLAVES CON AES_DECRYPT

	AnsiString instruccion;
	AnsiString parametro;
	int idEmpresa = 0;

	parametro = mFg.ExtraeStringDeBuffer(&parametros);
	idEmpresa = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	instruccion="SELECT AES_DECRYPT( valor ,'LaVioletaTiendaOnlineAPIToken2020') as valor \
	FROM paramseguridad where parametro='"+parametro+"' AND idempresa = "+idEmpresa;
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

}
//---------------------------------------------------------------------------
// ID_AES_SHOPIFY_USUARIO
 void ServidorAdminSistema::ShopifyUserShop(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{

	// CONSULTA LLAVES CON AES_DECRYPT

	AnsiString instruccion;
	AnsiString parametro;
	int idEmpresa = 0;

 	parametro = mFg.ExtraeStringDeBuffer(&parametros);
	idEmpresa = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	instruccion="SELECT AES_DECRYPT( valor ,'LaVioletaTiendaOnlineUsuario2020') as valor \
	FROM paramseguridad where parametro='"+parametro+"' AND idempresa = "+idEmpresa;
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

}
//---------------------------------------------------------------------------
// ID_AES_RAPPI_KEY
 void ServidorAdminSistema::RappiKeyShop(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{

	// CONSULTA LLAVES CON AES_DECRYPT

	AnsiString instruccion;
	AnsiString parametro;
	int idEmpresa = 0;

	parametro = mFg.ExtraeStringDeBuffer(&parametros);
	idEmpresa = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	instruccion="SELECT AES_DECRYPT( valor ,'LaVioletaTiendaOnlineAPIKEY2020') as valor \
	FROM paramseguridad where parametro='"+parametro+"' AND idempresa = "+ idEmpresa;
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

}
//---------------------------------------------------------------------------
// ID_AES_ROUTES_API
 void ServidorAdminSistema::SimpliRoutesKey(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{

	// CONSULTA LLAVES CON AES_DECRYPT

	AnsiString instruccion;
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	instruccion = mFg.ExtraeStringDeBuffer(&parametros);

	instruccion="SELECT AES_DECRYPT( valor ,'SimpliRoutesKey') as valor FROM paramseguridad where parametro='"+instruccion+"' AND idempresa = "+FormServidor->ObtieneClaveEmpresa();
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

}
//---------------------------------------------------------------------------
// ID_AES_CLIENTEID_API_BILLETO
 void ServidorAdminSistema::BilletoClientID(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{

	// CONSULTA LLAVES CON AES_DECRYPT

	AnsiString instruccion;
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	instruccion = mFg.ExtraeStringDeBuffer(&parametros);

	instruccion="SELECT AES_DECRYPT( valor ,'ClienteIDBilleto') as valor FROM paramseguridad where parametro='"+instruccion+"' AND idempresa = "+FormServidor->ObtieneClaveEmpresa();
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

}
//---------------------------------------------------------------------------
// ID_AES_CLIENTESECRET_API_BILLETO
 void ServidorAdminSistema::BilletoClientSecret(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{

	// CONSULTA LLAVES CON AES_DECRYPT

	AnsiString instruccion;
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	instruccion = mFg.ExtraeStringDeBuffer(&parametros);

	instruccion="SELECT AES_DECRYPT( valor ,'ClienteSecretBilleto') as valor FROM paramseguridad where parametro='"+instruccion+"' AND idempresa = "+FormServidor->ObtieneClaveEmpresa();
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

}
//---------------------------------------------------------------------------
// ID_AES_LOCATIONID_API_BILLETO
 void ServidorAdminSistema::BilletoLocationID(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{

	// CONSULTA LLAVES CON AES_DECRYPT

	AnsiString instruccion;
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	instruccion = mFg.ExtraeStringDeBuffer(&parametros);

	instruccion="SELECT AES_DECRYPT( valor ,'LocationIDBilleto') as valor FROM paramseguridad where parametro='"+instruccion+"' AND idempresa = "+FormServidor->ObtieneClaveEmpresa();
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

}
//---------------------------------------------------------------------------
// ID_AES_FIREBASE_API
 void ServidorAdminSistema::FireBaseAPIKey(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{

	// CONSULTA LLAVES CON AES_DECRYPT

	AnsiString instruccion;
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	instruccion = mFg.ExtraeStringDeBuffer(&parametros);

	instruccion="SELECT AES_DECRYPT( valor ,'APPSToken2022LaVioleta') as valor FROM paramseguridad where parametro='"+instruccion+"' AND idempresa = "+FormServidor->ObtieneClaveEmpresa();
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

}
//---------------------------------------------------------------------------

// ID_CALC_PRECALCULOCOSTO_VENTAS
void ServidorAdminSistema::PrecalculoCostoVentas(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{

	BufferRespuestas * respuesta_empresas = NULL;
	AnsiString empresa, archivo_temp2, archivo_temp3, archivo_temp4, instruccion;
	AnsiString SucursalOrigen;
	try{
		SucursalOrigen=FormServidor->ObtieneClaveSucursal();
		instruccion.sprintf("INSERT INTO bitacoraservidor VALUES (NULL,'%s',NOW(),'INICIO_ID_CALC_PRECALCULOCOSTO_VENTAS')",SucursalOrigen);
		mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str());

		instruccion.sprintf("SET SESSION sql_log_bin=0");
		if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL,
			instruccion.c_str()))
			throw(Exception("Error en query EjecutaSelectSqlNulo"));

		instruccion.sprintf("truncate table precalculocostosventadet");
		if(!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL,
			instruccion.c_str()))
		throw(Exception("Error en precalculos: TRUNCATE precalculocostosventadet"));

		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, "SELECT idempresa FROM empresas", respuesta_empresas);

		for(int i = 0; i < respuesta_empresas->ObtieneNumRegistros(); i++){
			respuesta_empresas->IrAlRegistroNumero(i);
			empresa = respuesta_empresas->ObtieneDato();

			char *buffer_sql=new char[1024*64*100];
			char *aux_buffer_sql=buffer_sql;
			char *buffer_costo_ventas=new char[1024*64*10];
			char *aux_buffer_costo_ventas=buffer_costo_ventas;
			AnsiString instruccion;

			try {

				//Costo de ventas
				aux_buffer_costo_ventas=mFg.AgregaStringABuffer("01/01/2000", aux_buffer_costo_ventas);
				aux_buffer_costo_ventas=mFg.AgregaStringABuffer("01/01/2099", aux_buffer_costo_ventas);
				aux_buffer_costo_ventas=mFg.AgregaStringABuffer(" ", aux_buffer_costo_ventas);
				aux_buffer_costo_ventas=mFg.AgregaStringABuffer(" ", aux_buffer_costo_ventas);
				aux_buffer_costo_ventas=mFg.AgregaStringABuffer(" ", aux_buffer_costo_ventas);
				aux_buffer_costo_ventas=mFg.AgregaStringABuffer(" ", aux_buffer_costo_ventas);
				aux_buffer_costo_ventas=mFg.AgregaStringABuffer(" ", aux_buffer_costo_ventas);
				aux_buffer_costo_ventas=mFg.AgregaStringABuffer(" ", aux_buffer_costo_ventas);
				aux_buffer_costo_ventas=mFg.AgregaStringABuffer(" ", aux_buffer_costo_ventas);//marca
				aux_buffer_costo_ventas=mFg.AgregaStringABuffer(" ", aux_buffer_costo_ventas); //proveedor
				aux_buffer_costo_ventas=mFg.AgregaStringABuffer(" ", aux_buffer_costo_ventas);
				aux_buffer_costo_ventas=mFg.AgregaStringABuffer(" ", aux_buffer_costo_ventas);
				aux_buffer_costo_ventas=mFg.AgregaStringABuffer("0", aux_buffer_costo_ventas); // No desglosar movimientos
				aux_buffer_costo_ventas=mFg.AgregaStringABuffer("1", aux_buffer_costo_ventas); // descontarnotas
				aux_buffer_costo_ventas=mFg.AgregaStringABuffer(" ", aux_buffer_costo_ventas);
				aux_buffer_costo_ventas=mFg.AgregaStringABuffer(" ", aux_buffer_costo_ventas);
				aux_buffer_costo_ventas=mFg.AgregaStringABuffer(" ", aux_buffer_costo_ventas);
				aux_buffer_costo_ventas=mFg.AgregaStringABuffer(" ", aux_buffer_costo_ventas);
				aux_buffer_costo_ventas=mFg.AgregaStringABuffer(" ", aux_buffer_costo_ventas);
				aux_buffer_costo_ventas=mFg.AgregaStringABuffer(" ", aux_buffer_costo_ventas);
				aux_buffer_costo_ventas=mFg.AgregaStringABuffer("0", aux_buffer_costo_ventas); // Diferencia costo-precio
				aux_buffer_costo_ventas=mFg.AgregaStringABuffer(" ", aux_buffer_costo_ventas); //proveedor de compras
				aux_buffer_costo_ventas=mFg.AgregaStringABuffer("1", aux_buffer_costo_ventas); // Registra costo de venta
				aux_buffer_costo_ventas=mFg.AgregaStringABuffer("0", aux_buffer_costo_ventas); // Se envian los meses
				aux_buffer_costo_ventas=mFg.AgregaStringABuffer(" ", aux_buffer_costo_ventas); // Envia partes relacionas
				aux_buffer_costo_ventas=mFg.AgregaStringABuffer(" ", aux_buffer_costo_ventas); 	 // Envia el segmento
				aux_buffer_costo_ventas=mFg.AgregaStringABuffer(" ", aux_buffer_costo_ventas);  // Envia el fabricante
				aux_buffer_costo_ventas=mFg.AgregaStringABuffer(empresa, aux_buffer_costo_ventas);  // Envia la empresa

				// ID_EJE_REPCOSTOVENTAS
				mServidorVioleta->MuestraMensaje("-- Parte de llamar ID_EJE_REPCOSTOVENTAS ", Respuesta->Id);
				mServidorVioleta->Reportes->EjecutaRepCostoVentas(Respuesta,  MySQL, buffer_costo_ventas);

				mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_REP);
				instruccion.sprintf("SELECT 0 as error");
				mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			} __finally {
				delete buffer_costo_ventas;
				delete buffer_sql;
			}
		} // Fin del calculo por empresa


		instruccion.sprintf("truncate table precalculoventasdet");
		if(!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL,
			instruccion.c_str()))
		throw(Exception("Error en query EjecutaSelectSqlNulo"));

		archivo_temp2 = mServidorVioleta->ObtieneArchivoTemp(2, Respuesta->Id);
		instruccion.sprintf("SELECT  v.referencia, a.producto, a.present, \
			SUM((dv.cantidad- IFNULL(auxnota.cantidad,0))*a.factor) AS totunidades, \
			SUM((dv.precio- IFNULL(auxnota.precio,0))*(dv.cantidad- IFNULL(auxnota.cantidad,0))) AS subtotventa, \
			SUM((dv.precioimp- IFNULL(auxnota.precioimp,0))*(dv.cantidad- IFNULL(auxnota.cantidad,0))) AS totventa \
			FROM ventas v FORCE INDEX(fechavta) \
			INNER JOIN dventas dv ON dv.referencia=v.referencia \
			INNER JOIN articulos a ON a.articulo=dv.articulo \
			LEFT JOIN ( \
			  SELECT v.referencia AS venta, d.articulo, SUM(IF(n.tipo='0',d.cantidad,0)) AS cantidad, SUM(IF(n.tipo<>'0',d.precio,0)) AS precio, SUM(IF(n.tipo<>'0',d.precioimp,0)) AS precioimp \
			  FROM notascredcli n \
			  INNER JOIN dnotascredcli d ON n.referencia=d.referencia \
			  INNER JOIN ventas v ON n.venta=v.referencia \
			  INNER JOIN articulos a ON d.articulo=a.articulo \
			  WHERE n.cancelado=0 AND v.cancelado=0 \
			  GROUP BY v.referencia, d.articulo) AS auxnota ON auxnota.articulo=dv.articulo AND auxnota.venta=v.referencia \
			WHERE v.cancelado=0 \
			GROUP BY v.referencia,a.producto,a.present \
			INTO OUTFILE '%s' ", archivo_temp2);
		if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL,
			instruccion.c_str()))
			throw(Exception("Error en query EjecutaSelectSqlNulo"));

		instruccion.sprintf
			("LOAD DATA INFILE '%s' INTO TABLE precalculoventasdet (referencia, producto, present, unidades, subtotal,total) ",
			archivo_temp2);
		if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL,
			instruccion.c_str()))
			throw(Exception("Error en query EjecutaSelectSqlNulo"));

		instruccion.sprintf("truncate table precalculocostosventa");
		if(!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL,
			instruccion.c_str()))
		throw(Exception("Error en precalculos: TRUNCATE precalculocostosventa"));

		instruccion.sprintf("insert into precalculocostosventa (referencia, costovta) \
			select if(p.tipo='DV', n.venta,p.referencia) as refventa, sum(p.costovta) as costovta \
			from precalculocostosventadet p \
			left join notascredcli n on n.referencia=p.referencia and p.tipo='DV' \
			group by refventa");
		if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL,
				instruccion.c_str()))
				throw(Exception("Error en query EjecutaSelectSqlNulo"));

		instruccion.sprintf("truncate table precalculoventas");
		if(!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL,
			instruccion.c_str()))
			throw(Exception("Error en query EjecutaSelectSqlNulo"));

		archivo_temp3 = mServidorVioleta->ObtieneArchivoTemp(3,
			Respuesta->Id);
		instruccion.sprintf(" \
			select referencia, sum(subtotal) as subtotal, sum(total) as total from precalculoventasdet \
			GROUP BY referencia \
			INTO OUTFILE '%s' ", archivo_temp3);
		if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL,
			instruccion.c_str()))
			throw(Exception("Error en query EjecutaSelectSqlNulo"));

		instruccion.sprintf
			("LOAD DATA INFILE '%s' INTO TABLE precalculoventas (referencia, subtotal,total) ",
			archivo_temp3);
		if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL,
			instruccion.c_str()))
			throw(Exception("Error en query EjecutaSelectSqlNulo"));

		instruccion.sprintf("SET SESSION sql_log_bin=1");
		if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta, MySQL,
			instruccion.c_str()))
			throw(Exception("Error en query EjecutaSelectSqlNulo"));

		instruccion.sprintf("INSERT INTO bitacoraservidor VALUES (NULL,'%s',NOW(),'FIN_ID_CALC_PRECALCULOCOSTO_VENTAS')",SucursalOrigen);
		mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str());

	}__finally{

		if (archivo_temp2 != "") {
			mServidorVioleta->BorraArchivoTemp(archivo_temp2);
		}

		if (archivo_temp4 != "") {
			mServidorVioleta->BorraArchivoTemp(archivo_temp4);
		}

		if (archivo_temp3 != "") {
			mServidorVioleta->BorraArchivoTemp(archivo_temp3);
		}
    	delete respuesta_empresas;
	}
}
//---------------------------------------------------------------------------

// ID_CALC_PRECALCULOCOSTO_MOVIMIENTOS
void ServidorAdminSistema::PrecalculoCostoMov(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*64*100];
	char *aux_buffer_sql=buffer_sql;
	char *buffer_costo_mov=new char[1024*64*10];
	char *aux_buffer_costo_mov=buffer_costo_mov;
	AnsiString SucursalOrigen;
	AnsiString instruccion;

	BufferRespuestas* resp_concepto=NULL;
	AnsiString cad_conjunto_conceptos=" ", concepto;

	SucursalOrigen=FormServidor->ObtieneClaveSucursal();

	try {
   		instruccion.sprintf("INSERT INTO bitacoraservidor VALUES (NULL,'%s',NOW(),'INICIO_ID_CALC_PRECALCULOCOSTO_MOVIMIENTOS')",SucursalOrigen);
		mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str());


		instruccion.sprintf("SELECT concepto FROM conceptosmovalma order by tipomov, descripcion");
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_concepto);
		for(int i=0; i<resp_concepto->ObtieneNumRegistros(); i++){
			resp_concepto->IrAlRegistroNumero(i);
			concepto=resp_concepto->ObtieneDato("concepto");

			cad_conjunto_conceptos+="'";
			cad_conjunto_conceptos+=concepto;
			cad_conjunto_conceptos+="'";
			if (i<resp_concepto->ObtieneNumRegistros()-1) // A todos menos al ultimo impuesto le signo +.
				cad_conjunto_conceptos+=",";
		}

		//Costo de movimientos
		aux_buffer_costo_mov=mFg.AgregaStringABuffer("01/01/2000", aux_buffer_costo_mov); //fecha_inicial
		aux_buffer_costo_mov=mFg.AgregaStringABuffer("01/01/2099", aux_buffer_costo_mov); //feecha_final
		aux_buffer_costo_mov=mFg.AgregaStringABuffer(" ", aux_buffer_costo_mov); //tipomov
		aux_buffer_costo_mov=mFg.AgregaStringABuffer(cad_conjunto_conceptos, aux_buffer_costo_mov); //conceptos
		aux_buffer_costo_mov=mFg.AgregaStringABuffer(" ", aux_buffer_costo_mov); //sucursal
		aux_buffer_costo_mov=mFg.AgregaStringABuffer(" ", aux_buffer_costo_mov); //almacen
		aux_buffer_costo_mov=mFg.AgregaStringABuffer(" ", aux_buffer_costo_mov); //clasif1
		aux_buffer_costo_mov=mFg.AgregaStringABuffer(" ", aux_buffer_costo_mov); //clasif2
		aux_buffer_costo_mov=mFg.AgregaStringABuffer(" ", aux_buffer_costo_mov); //clasif3
		aux_buffer_costo_mov=mFg.AgregaStringABuffer(" ", aux_buffer_costo_mov); //producto
		aux_buffer_costo_mov=mFg.AgregaStringABuffer(" ", aux_buffer_costo_mov); //movimiento
		aux_buffer_costo_mov=mFg.AgregaStringABuffer("1", aux_buffer_costo_mov); //registra_costomov

		// ID_EJE_REPCOSTOMOVALM
		mServidorVioleta->MuestraMensaje("-- Parte de llamar ID_EJE_REPCOSTOMOVALM ", Respuesta->Id);
		mServidorVioleta->Reportes->EjecutaRepCostoMovimientosAlmacen(Respuesta,  MySQL, buffer_costo_mov);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_REP);
		instruccion.sprintf("SELECT 0 as error");
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

		instruccion.sprintf("INSERT INTO bitacoraservidor VALUES (NULL,'%s',NOW(),'FIN_ID_CALC_PRECALCULOCOSTO_MOVIMIENTOS')",SucursalOrigen);
		mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str());

	} __finally {
		delete buffer_costo_mov;
		delete buffer_sql;
	}
}
//---------------------------------------------------------------------------

// ID_CALC_PRECALCULOHISTORIAL_EXISTENCIAS
void ServidorAdminSistema::PrecalculoHistExis(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString SucursalOrigen;
	AnsiString instruccion;

	SucursalOrigen=FormServidor->ObtieneClaveSucursal();

	AnsiString linea;
	AnsiString archivo= "";
	FILE *fp=NULL;

   	BufferRespuestas * respuesta_empresas = NULL;
	AnsiString idEmpresa = " ";

	vector<AnsiString> archivosTemporales;

	try {


		instruccion.sprintf("INSERT INTO bitacoraservidor VALUES (NULL,'%s',NOW(),'INICIO_ID_CALC_PRECALCULO_HIST_EXIST')",SucursalOrigen);
		mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str());

		instruccion.sprintf("SET SESSION sql_log_bin=0");
			if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
						throw (Exception("Error en query EjecutaSelectSqlNulo"));

		//--------INICIO DEL CÁLCULO POR EMPRESA-----------------------------
		instruccion = "SELECT idempresa FROM EMPRESAS";
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), respuesta_empresas);

		for(int h = 0; h < respuesta_empresas->ObtieneNumRegistros(); h++){
			try{
				respuesta_empresas->IrAlRegistroNumero(h);

				idEmpresa = respuesta_empresas->ObtieneDato("idempresa");

				archivosTemporales.push_back(mServidorVioleta->ObtieneArchivoTemp(2345, Respuesta->Id));

				instruccion="CREATE temporary TABLE IF NOT EXISTS precalculohistoriaaux ( "
						"producto VARCHAR(8) NOT NULL, "
						"present VARCHAR(255) NOT NULL, "
						"sucursal VARCHAR(4) NOT NULL, "
						"fecha DATE NOT NULL, "
						"cantidadent double NOT NULL, "
						"cantidadsal double NOT NULL, "
						"PRIMARY KEY (producto, present, sucursal, fecha), "
						"INDEX(sucursal), "
						"INDEX(fecha) "
					") COLLATE='latin1_swedish_ci' ENGINE=INNODB ";
				mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str());

				instruccion = "TRUNCATE TABLE precalculohistoriaaux";
				if(!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
					throw (Exception("No se ha podido truncar precalculohistoriaaux"));

				instruccion.sprintf("INSERT INTO precalculohistoriaaux (producto, present, sucursal, fecha, cantidadent, cantidadsal) "
						"SELECT producto,present,sec.sucursal,if(fecha<date_sub(CURRENT_DATE(), INTERVAL 180 DAY), CAST('2000-01-01' AS DATE), fecha) AS fechaacum, "
						"sum(if(cantidad>0,cantidad,0)) AS sumcantent, sum(if(cantidad<0,cantidad,0)) AS sumcantsal "
						"from precalculocostos%s pc "
						" INNER JOIN almacenes al ON pc.almacen=al.almacen "
						" INNER JOIN secciones sec ON al.seccion=sec.seccion "
						"WHERE clasif IN ('A', 'C') "
						"GROUP BY producto,present,sucursal,fechaacum ", idEmpresa);
				mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str());

				instruccion.sprintf("truncate table precalculohistoriaexist%s", idEmpresa);
				mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str());

				// Así se calculaba antes sin funciones tipo window, era mucho más lento (18 segundos contra un par de segundos), lo dejo como comentario para que se entienda con un query más estandar.
				/*
				instruccion="create temporary table precalculohistoria2aux like precalculohistoriaaux ";
				mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str());

				instruccion="insert into precalculohistoria2aux select * from precalculohistoriaaux ";
				mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str());

				instruccion="INSERT INTO precalculohistoriaexist (producto, present, sucursal, fecha, cantidadent, cantidadsal, cantidadacum) "
					"SELECT t1.producto,t1.present,t1.sucursal,t1.fecha,t1.cantidadent, t1.cantidadsal, SUM(t2.cantidadent+t2.cantidadsal) AS cantidadacum "
					"FROM precalculohistoriaaux t1 "
					"inner join precalculohistoria2aux t2 "
					"	ON t1.producto=t2.producto AND t1.present=t2.present AND t1.sucursal=t2.sucursal AND t2.fecha<=t1.fecha "
					"GROUP BY producto,present,sucursal,fecha "
					"ORDER BY producto,present,sucursal,fecha ";
				*/

				// Query con función tipo window.
				instruccion.sprintf("INSERT INTO precalculohistoriaexist%s (producto, present, sucursal, fecha, cantidadent, cantidadsal, cantidadacum) "
					"SELECT producto,present,sucursal,fecha, cantidadent, cantidadsal, "
					"SUM(cantidadent+cantidadsal) over (PARTITION by producto,present,sucursal ORDER by producto,present,sucursal,fecha) AS cantidadacum "
					"FROM precalculohistoriaaux "
					"ORDER BY producto,present,sucursal,fecha ", idEmpresa);

				mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str());


				instruccion.sprintf("truncate table precalculohistoriaconexist%s ", idEmpresa);
				mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str());

				// Llena la tabla precalculohistoriaconexist
				AnsiString producto_ant=" ", present_ant=" ", sucursal_ant=" ";
				AnsiString producto, present, sucursal;
				TDate fecha, fecha180, fecha90, fecha60, fecha30;
				TDate fecha_hoy=Today(), fecha_ant=StrToDate("01/01/1900");
				double cantidadacum=0, cantidadacum_ant=0, cantidadsal=0, cantidadsal_ant=0;
				BufferRespuestas* resp_histexist=NULL;
				int num_regs, i;
				int sum_dias180=0, sum_dias90=0, sum_dias60=0, sum_dias30=0, dias_intervalo=0;

				instruccion.sprintf("SELECT producto, present, sucursal, fecha, cantidadacum, cantidadsal \
					FROM precalculohistoriaexist%s \
					ORDER BY producto, present, sucursal, fecha ", idEmpresa);

				if (mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_histexist, TAM_MAX_BUFFER_HISTEXISTENCIAS)) {
					try{
						num_regs=resp_histexist->ObtieneNumRegistros();

						if (num_regs>0){

							fp=fopen(archivosTemporales.back().c_str(),"w");
							if (fp==NULL)
								throw (Exception("Error al crear el archivo temporal para llenar precalculohistoriaconexist%s"));

							fecha30=IncDay(Today()-30);
							fecha60=IncDay(Today()-60);
							fecha90=IncDay(Today()-90);
							fecha180=IncDay(Today()-180);

							for (i=0; i<num_regs; i++){
								resp_histexist->IrAlRegistroNumero(i);

								producto=resp_histexist->ObtieneDato("producto");
								present=resp_histexist->ObtieneDato("present");
								sucursal=resp_histexist->ObtieneDato("sucursal");
								fecha=StrToDate(resp_histexist->ObtieneDato("fecha"));
								cantidadacum=mFg.CadenaAFlotante(resp_histexist->ObtieneDato("cantidadacum"));
								cantidadsal=mFg.CadenaAFlotante(resp_histexist->ObtieneDato("cantidadsal"));

								if (i>0 && (producto!=producto_ant || present!=present_ant || sucursal!=sucursal_ant)) {

									// Suma el intervalo de días desde la fecha anterior hasta hoy para totalizar.
									// Considera solo los dias donde hay más de una unidad.
									if (cantidadacum_ant>=1.0) {
										dias_intervalo=DaysBetween(fecha_ant, fecha_hoy)+1;

										if (dias_intervalo<=180)
											sum_dias180+=dias_intervalo;
										else
											sum_dias180=180;

										if (dias_intervalo<=90)
											sum_dias90+=dias_intervalo;
										else
											sum_dias90=90;

										if (dias_intervalo<=60)
											sum_dias60+=dias_intervalo;
										else
											sum_dias60=60;

										if (dias_intervalo<=30)
											sum_dias30+=dias_intervalo;
										else
											sum_dias30=30;

									} else {

										// ******** 29 julio 2020
										// Considerar los días que no hay existencias pero sí hay salidas (lo que es indicador que sí habia existencias)
										// ********
										if (cantidadsal_ant<=-1.0) {
											dias_intervalo=DaysBetween(fecha_ant, fecha_hoy)+1;

											if (dias_intervalo<=180) {
												sum_dias180++;
												if (sum_dias180>180)
													sum_dias180=180;
											}

											if (dias_intervalo<=90) {
												sum_dias90++;
												if (sum_dias90>90)
													sum_dias90=90;
											}

											if (dias_intervalo<=60) {
												sum_dias60++;
												if (sum_dias60>60)
													sum_dias60=60;

											}

											if (dias_intervalo<=30) {
												sum_dias30++;
												if (sum_dias30>30)
													sum_dias30=30;

											}
										}

									}

									// Manda resultado al archivo temporal.
									linea.sprintf("%s\t%s\t%s\t%s\t%s\t%s\t%s\n",
															producto_ant, present_ant, sucursal_ant,
															mFg.IntToAnsiString(sum_dias30),
															mFg.IntToAnsiString(sum_dias60),
															mFg.IntToAnsiString(sum_dias90),
															mFg.IntToAnsiString(sum_dias180) );
													fputs(linea.c_str(),fp);

									producto_ant=producto;
									present_ant=present;
									sucursal_ant=sucursal;

									fecha_ant=fecha;
									cantidadacum_ant=cantidadacum;
									cantidadsal_ant=cantidadsal;

									sum_dias30=0;
									sum_dias60=0;
									sum_dias90=0;
									sum_dias180=0;
									cantidadacum=0;
									cantidadsal=0;

								} else {

									// Suma el intervalo de días desde la fecha anterior que le corresponden a cada sumatoria de dias.
									if (cantidadacum_ant>=1.0) {

										dias_intervalo=DaysBetween(fecha_ant, fecha);

										if (fecha_ant>fecha180)
											sum_dias180+=dias_intervalo;
										else {
											if (fecha>fecha180) {
												sum_dias180+=DaysBetween(fecha, fecha180);
											}
										}


										if (fecha_ant>fecha90)
											sum_dias90+=dias_intervalo;
										else {
											if (fecha>fecha90) {
												sum_dias90+=DaysBetween(fecha, fecha90);
											}
										}

										if (fecha_ant>fecha60)
											sum_dias60+=dias_intervalo;
										else {
											if (fecha>fecha60) {
												sum_dias60+=DaysBetween(fecha, fecha60);
											}
										}

										if (fecha_ant>fecha30)
											sum_dias30+=dias_intervalo;
										else {
											if (fecha>fecha30) {
												sum_dias30+=DaysBetween(fecha, fecha30);
											}
										}

									} else {

										// ******** 29 julio 2020
										// Considerar los días que no hay existencias pero sí hay salidas (lo que es indicador que sí habia existencias)
										// ********
										if (cantidadsal_ant<=-1.0) {
											dias_intervalo=DaysBetween(fecha_ant, fecha_hoy)+1;

											if (dias_intervalo<=180) {
												sum_dias180++;
												if (sum_dias180>180)
													sum_dias180=180;
											}

											if (dias_intervalo<=90) {
												sum_dias90++;
												if (sum_dias90>90)
													sum_dias90=90;
											}

											if (dias_intervalo<=60) {
												sum_dias60++;
												if (sum_dias60>60)
													sum_dias60=60;

											}

											if (dias_intervalo<=30) {
												sum_dias30++;
												if (sum_dias30>30)
													sum_dias30=30;

											}
										}


									}

									producto_ant=producto;
									present_ant=present;
									sucursal_ant=sucursal;

									fecha_ant=fecha;
									cantidadacum_ant=cantidadacum;
									cantidadsal_ant=cantidadsal;

								}

							}

							// finaliza el ultimo producto
							// Suma el intervalo de días desde la fecha anterior hasta hoy para totalizar.
							if (cantidadacum_ant>=1.0) {
								dias_intervalo=DaysBetween(fecha_ant, fecha_hoy)+1;

								if (dias_intervalo<=180)
									sum_dias180+=dias_intervalo;
								else
									sum_dias180=180;

								if (dias_intervalo<=90)
									sum_dias90+=dias_intervalo;
								else
									sum_dias90=90;

								if (dias_intervalo<=60)
									sum_dias60+=dias_intervalo;
								else
									sum_dias60=60;

								if (dias_intervalo<=30)
									sum_dias30+=dias_intervalo;
								else
									sum_dias30=30;

							} else {


								// ******** 29 julio 2020
								// Considerar los días que no hay existencias pero sí hay salidas (lo que es indicador que sí habia existencias)
								// ********
								if (cantidadsal_ant<=-1.0) {
									dias_intervalo=DaysBetween(fecha_ant, fecha_hoy)+1;

									if (dias_intervalo<=180) {
										sum_dias180++;
										if (sum_dias180>180)
											sum_dias180=180;
									}

									if (dias_intervalo<=90) {
										sum_dias90++;
										if (sum_dias90>90)
											sum_dias90=90;
									}

									if (dias_intervalo<=60) {
										sum_dias60++;
										if (sum_dias60>60)
											sum_dias60=60;

									}

									if (dias_intervalo<=30) {
										sum_dias30++;
										if (sum_dias30>30)
											sum_dias30=30;

									}
								}


							}

							// Manda resultado al archivo temporal.
							linea.sprintf("%s\t%s\t%s\t%s\t%s\t%s\t%s\n",
							producto_ant, present_ant, sucursal_ant,
							mFg.IntToAnsiString(sum_dias30),
							mFg.IntToAnsiString(sum_dias60),
							mFg.IntToAnsiString(sum_dias90),
							mFg.IntToAnsiString(sum_dias180) );
							fputs(linea.c_str(),fp);

							instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE precalculohistoriaconexist%s \
								(producto, present, sucursal, \
								diasconexist30, diasconexist60, diasconexist90, diasconexist180) ", archivosTemporales.back(), idEmpresa);
							if(!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str())){
								throw Exception("Error en LOAD DATA INFILE en precalculohistoriaconexist%s");
							} else {

								mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_REP);
								instruccion.sprintf("SELECT 0 as error");
								mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
							}

						} else {
							mServidorVioleta->MuestraMensaje("ERROR: La tabla precalculohistoriaexist"+idEmpresa+" está vacía. No se realizó el cálculo de ID_CALC_PRECALCULOHISTORIAL_EXISTENCIAS para la empresa "+idEmpresa);
						}
					}__finally{
						delete resp_histexist;
					}

				} else throw (Exception("Error al consultar en tabla parametros"));
			}__finally{
				fclose(fp);
			}


		}//--------FÍN DEL CÁLCULO POR EMPRESA--------------------------------

		instruccion.sprintf("SET SESSION sql_log_bin=1");
		if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
			throw (Exception("Error en query EjecutaSelectSqlNulo"));

		instruccion.sprintf("INSERT INTO bitacoraservidor VALUES (NULL,'%s',NOW(),'FIN_ID_CALC_PRECALCULO_HIST_EXIST')",SucursalOrigen);
		mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str());

	} __finally {
		for(int i = 0; i < archivosTemporales.size(); i++){
			mServidorVioleta->BorraArchivoTemp(archivosTemporales.at(i));
		}

		delete respuesta_empresas;
	}
}
//---------------------------------------------------------------------------
// ID_ETIQUETA_AUTOMATICO
void ServidorAdminSistema::EjecutaEtiquetaAutomatico(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*64];
	char *aux_buffer_sql=buffer_sql;
	int num_instrucciones=0;
	AnsiString instruccion, consulta, instrucciones[100];
	int i;
	AnsiString archivo_temp1, archivo_temp2;

	AnsiString idtag;
	AnsiString empresa, condicion_empresa = " ";

	AnsiString global;

	try{
		idtag=mFg.ExtraeStringDeBuffer(&parametros);
		empresa=mFg.ExtraeStringDeBuffer(&parametros);

		if(empresa != " ")
			condicion_empresa.sprintf(" AND suc.idempresa = %s ", empresa);

		instruccion="SET SESSION sql_log_bin = 0";
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("set @fechafinal=CURDATE()");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("set @inicioventa:=DATE_SUB(CURDATE(), INTERVAL 2 MONTH) ");
		instrucciones[num_instrucciones++]=instruccion;

		// Suma el subtotal de ventas
		instruccion.sprintf("SELECT @sumados:=SUM(subtotal) FROM precalculoventas pv \
			 INNER JOIN ventas v ON pv.referencia=v.referencia AND v.cancelado=0 \
			 INNER JOIN terminales ter ON ter.terminal = v.terminal \
			 INNER JOIN secciones sec ON sec.seccion = ter.seccion  \
			 INNER JOIN sucursales suc ON suc.sucursal = sec.sucursal \
			 WHERE v.fechavta BETWEEN @inicioventa AND @fechafinal %s ", condicion_empresa);
		instrucciones[num_instrucciones++]=instruccion;

		instruccion=" drop table if exists registrosaux ";
		instrucciones[num_instrucciones++]=instruccion;

		// table registrosaux
		instruccion=" create temporary table registrosaux (producto VARCHAR(8), present VARCHAR(255), "
					"subtotal decimal(12,3), porcentaje decimal(12,3), primary key (producto,present) ) ENGINE=InnoDB ";
		instrucciones[num_instrucciones++]=instruccion;

		archivo_temp1=mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);

		instruccion.sprintf("SELECT pvd.producto, pvd.present, SUM(pvd.subtotal) AS subtotal, sum(pvd.subtotal*100)/@sumados AS porcentaje "
			"FROM precalculoventas pv "
			"INNER JOIN ventas v ON pv.referencia=v.referencia AND v.cancelado=0 "
			"INNER JOIN precalculoventasdet pvd ON pvd.referencia=pv.referencia "
			"INNER JOIN terminales ter ON ter.terminal = v.terminal "
			"INNER JOIN secciones sec ON sec.seccion = ter.seccion "
			"INNER JOIN sucursales suc ON suc.sucursal = sec.sucursal "
			"WHERE v.fechavta BETWEEN @inicioventa AND @fechafinal %s "
			"GROUP BY producto, present "
			"INTO OUTFILE '%s'", condicion_empresa, archivo_temp1);
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE  registrosaux ",archivo_temp1);
		instrucciones[num_instrucciones++]=instruccion;

        instruccion=" drop table if exists clasificacionaux ";
		instrucciones[num_instrucciones++]=instruccion;

		//   table clasificacionaux
		instruccion=" create temporary table clasificacionaux (producto VARCHAR(8),	present VARCHAR(255), "
				"subtotal decimal(12,3), porcentaje decimal(12,3) ,sumaporcentaje decimal(12,3), "
				"posicion integer, primary key (producto,present) ) ENGINE=INNODB";
		instrucciones[num_instrucciones++]=instruccion;

		archivo_temp2=mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);
		instruccion.sprintf(
			" SELECT * FROM ( "
			" 	SELECT producto, present, subtotal,porcentaje, sum(porcentaje) OVER (ORDER BY porcentaje DESC) AS sumaporcent, "
			" 	SUM(1) OVER (ORDER BY porcentaje DESC) AS posicion "
			" 	FROM registrosaux "
			" ) t "
			" WHERE t.sumaporcent<=50 "
			" ORDER BY posicion "
			"INTO OUTFILE '%s'", archivo_temp2);
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE clasificacionaux ",archivo_temp2);
		instrucciones[num_instrucciones++]=instruccion;

		//se borrara toda los datos de la tabla para el tag 1 (más vendidos) para poder volver a asignar articulos
		instruccion.sprintf("DELETE FROM articulostagsasignados  where idarticulotag=%d", idtag.ToInt());
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("INSERT INTO \
			articulostagsasignados(idarticulotag,producto,present,fechaalta,horaalta,usualta,fechamodi, horamodi, usumodi) \
			select %d, producto,present, CURDATE(), CURTIME(),null, CURDATE(), CURTIME(), null \
			from clasificacionaux ",idtag.ToInt() );
		instrucciones[num_instrucciones++]=instruccion;

		instruccion="SET SESSION sql_log_bin = 1";
		instrucciones[num_instrucciones++]=instruccion;


		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_REP);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);

	}__finally{
		delete buffer_sql;
		if (archivo_temp1 != "")
			mServidorVioleta->BorraArchivoTemp(archivo_temp1);
		if (archivo_temp2 != "")
			mServidorVioleta->BorraArchivoTemp(archivo_temp2);
	}

}

//---------------------------------------------------------------------------

//ID_CON_ENVIODATOSPOCKET_NEW_B
void ServidorAdminSistema::EnvioDatosPocketNewB(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	BufferRespuestas* resp_gen=NULL;
	char *buffer_sql=new char[1024*64*10];
	char *aux_buffer_sql=buffer_sql;
	int i, num_partidas;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[1000];
	AnsiString fecha, terminal, vendedor;
	AnsiString archivo_temp1, archivo_temp2, archivo_temp3, archivo_temp4, archivo_temp5;
	AnsiString empresa, sucursal, condicion_sucursal=" ";
	AnsiString almacen, cad_conjunto_almacenes=" ", modo_calcular_existencias, dias_ped_transit_calcular_existencias ;
	AnsiString condicion_almacen=' ';
	BufferRespuestas* resp_almacenes=NULL;

	TDate fecha_hoy=Today();
	AnsiString cad_fecha_hoy=mFg.DateToMySqlDate(fecha_hoy);
	AnsiString cad_fecha_dos_anios=mFg.DateToMySqlDate(IncYear(fecha_hoy, -2));
	AnsiString condicion_fechas_ventassaldo=" v.fechavta between '"+cad_fecha_dos_anios+"' and '"+ cad_fecha_hoy+ "' and  ";
	bool tienePrivilegioCRE=false;
	AnsiString condicion_cliente_vendedor=" ";
    AnsiString archivo_temp_1;

	try{
		empresa=FormServidor->ObtieneClaveEmpresa();
		sucursal=FormServidor->ObtieneClaveSucursal();

		fecha=mFg.DateToMySqlDate(Today());

		terminal=mFg.ExtraeStringDeBuffer(&parametros);
		vendedor=mFg.ExtraeStringDeBuffer(&parametros);
		modo_calcular_existencias=mFg.ExtraeStringDeBuffer(&parametros);
		dias_ped_transit_calcular_existencias=mFg.ExtraeStringDeBuffer(&parametros);


        if (dias_ped_transit_calcular_existencias.Length()>1) {
			dias_ped_transit_calcular_existencias="10";
		}else if (dias_ped_transit_calcular_existencias=="")
			dias_ped_transit_calcular_existencias="0";


		// Obtiene los almacenes que corresponden a una sucursal
		if (sucursal!=" ") {
			instruccion.sprintf("SELECT a.almacen FROM almacenes a \
				INNER JOIN secciones s ON a.seccion=s.seccion \
				WHERE s.sucursal='%s'", sucursal);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_almacenes);
			for(i=0; i<resp_almacenes->ObtieneNumRegistros(); i++){
				resp_almacenes->IrAlRegistroNumero(i);
				almacen=resp_almacenes->ObtieneDato("almacen");

				cad_conjunto_almacenes+="'";
				cad_conjunto_almacenes+=almacen;
				cad_conjunto_almacenes+="'";
				if (i<resp_almacenes->ObtieneNumRegistros()-1) // A todos menos al ultimo impuesto le signo +.
					cad_conjunto_almacenes+=",";
			}
		}

		instruccion.sprintf("SET SESSION tx_isolation='READ-COMMITTED' ");
		instrucciones[num_instrucciones++] = instruccion;

		instruccion.sprintf("set @fechafinal='%s'", fecha);
		instrucciones[num_instrucciones++]=instruccion;

		// Obtiene fecha del corte más próximo previo a la fecha dada
		instruccion.sprintf("set @fechacorte='1900-01-01'");
		/*instruccion.sprintf("select @fechacorte:=ifnull(max(p.fecha),'1900-01-01') as fcorte \
			 from puntoscorte p where p.fecha<='%s'", fecha);*/
		instrucciones[num_instrucciones++]=instruccion;

		// Crea una tabla donde se pondrán las existencias que se vayan encontrando.
		instruccion="create temporary table existenciasaux ( \
			producto varchar(8), present varchar(255), \
			tipo char(2), cantidad decimal(12,3), \
			INDEX(producto, present)) Engine = InnoDB";
		instrucciones[num_instrucciones++]=instruccion;

		// Crea una tabla donde se pondrán las existencias finales
		instruccion="create temporary table auxexistsumadas ( \
			producto varchar(8), present varchar(255), \
			cantidad decimal(12,3), \
			INDEX(producto, present)) Engine = InnoDB";
		instrucciones[num_instrucciones++]=instruccion;

		if(modo_calcular_existencias == "MODN"){
            throw Exception("Configuración de modo de calculo para revisión de existencias está incorrecto MODN");
		}
			else
				if(modo_calcular_existencias == "MODEA"){

					if (cad_conjunto_almacenes!=" ") {
						condicion_almacen.sprintf(" WHERE almacen in (%s) ", cad_conjunto_almacenes);
					} else condicion_almacen=" ";

					// Crea una tabla donde se pondrán los pedidos en transito para optimizar consulta en existencias restando pedidos en transito
					instruccion="create temporary table auxpedidosentransito ( \
						referencia VARCHAR(11), \
						INDEX(referencia)) Engine = InnoDB";
					instrucciones[num_instrucciones++]=instruccion;



					/* Calcula la CANTIDAD con tabla de existencias actuales
					instruccion.sprintf("insert into existenciasaux (producto, present, tipo, cantidad)\
						SELECT producto,present,'EX',SUM(cantidad) as cantidad FROM \
						existenciasactuales %s group by producto, present ",
						condicion_almacen);
					instrucciones[num_instrucciones++]=instruccion;*/

                    archivo_temp_1=mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);

					instruccion.sprintf("SELECT producto,present,'EX',SUM(cantidad) as cantidad FROM \
						existenciasactuales %s group by producto, present INTO OUTFILE '%s' ",
						condicion_almacen, archivo_temp_1);
					instrucciones[num_instrucciones++]=instruccion;

					instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE existenciasaux (producto, present, tipo, cantidad) ",
					 archivo_temp_1);
					instrucciones[num_instrucciones++]=instruccion;

					instruccion.sprintf("set @fechapedtrans=(INTERVAL -%s DAY +'%s')", dias_ped_transit_calcular_existencias, cad_fecha_hoy);
					instrucciones[num_instrucciones++]=instruccion;

                    // Calcula los pedidos en transito para restarlos de existencias reales
					instruccion.sprintf("INSERT INTO auxpedidosentransito(referencia) \
						SELECT p.referencia FROM pedidosventa AS p \
						INNER JOIN terminales AS t ON t.terminal=p.terminal \
						INNER JOIN secciones AS s ON s.seccion=t.seccion \
						WHERE p.facturado=0 AND p.cancelado=0 AND p.cotizacion=0 AND p.fechaped>@fechapedtrans AND s.sucursal='%s'",
						sucursal);
					instrucciones[num_instrucciones++]=instruccion;


					// Calcula detalles de los pedidos en transito para restarlos de existencias reales
					instruccion.sprintf("insert into existenciasaux (producto, present, tipo, cantidad) \
						select  a.producto, a.present,'PT', SUM(d.cantidad*a.factor)*-1 AS cantidad \
						from auxpedidosentransito AS p \
						INNER JOIN dpedidosventa AS d ON d.referencia=p.referencia \
						INNER JOIN articulos AS a ON a.articulo=d.articulo \
						group by a.producto, a.present");
					instrucciones[num_instrucciones++]=instruccion;


				}

		// Suma los movimientos para obtener las existencias
		instruccion.sprintf("insert into auxexistsumadas (producto, present, cantidad) \
				select e.producto, e.present, sum(e.cantidad) as cantidad \
				from existenciasaux e \
				group by e.producto, e.present");
		instrucciones[num_instrucciones++]=instruccion;

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_REP);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			// Resultado final

			// Manda parámetros
			instruccion.sprintf("select parametro, left(descripcion,40) as descripcion, valor \
				from parametrosemp WHERE 1 AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Manda articulos
			// Ahora solo manda los activos o los inactivos que han sido modificados en los últimos 30 dias.
			// (esto último para mandar los productos que pudieran estar en algún producto pendiente de enviar.
			instruccion.sprintf("SELECT a.articulo, a.producto, a.present, a.multiplo, a.factor, a.porccomi, e.cantidad, a.activo, \
								IF(ae.tipoutil=5,'5',IF(ae.tipoutil=4,'4',IF(ae.tipoutil='3','3',IF(ae.tipoutil=2,'2',IF(ae.tipoutil=1,'1',0))))) AS utilesp, \
								a.peso, a.volumen \
								FROM articulos a \
								INNER JOIN articulosemp ae ON ae.articulo=a.articulo AND ae.idempresa=%s \
								LEFT JOIN auxexistsumadas e ON e.producto=a.producto AND e.present=a.present \
								LEFT JOIN articuloempresacfg aemp ON aemp.articulo = a.articulo AND aemp.idempresa = %s \
								WHERE (aemp.permiteventas = 1 OR ISNULL(aemp.permiteventas)) AND activo=1 \
								OR (a.activo=0 AND a.fechamodi>= DATE_ADD(CURDATE(), INTERVAL -30 DAY))",
								FormServidor->ObtieneClaveEmpresa(), FormServidor->ObtieneClaveEmpresa() );
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Manda CLIENTES
			instruccion.sprintf("select * from(( \
				select u.empleado, ar.objeto, ar.privilegio from usuarios u \
				inner join usuariorol ur on ur.usuario = u.empleado \
				inner join asignacionprivrol ar on ur.rol=ar.rol ) \
			union ( select a.usuario AS empleado, a.objeto, a.privilegio \
				from asignacionprivilegios a ) \
			UNION ( \
				SELECT u.empleado, ar.objeto, ar.privilegio \
				FROM usuarios u \
				INNER JOIN empleados e ON e.empleado = u.empleado \
				INNER JOIN puestos p ON p.puesto = e.puesto \
				INNER JOIN rolesxpuesto rxp ON rxp.puesto = p.puesto \
				INNER JOIN asignacionprivrol ar ON rxp.rol=ar.rol)) as asigprivtotal \
			where asigprivtotal.empleado='%s' and asigprivtotal.objeto='SINCPOCKET' and asigprivtotal.privilegio='CRE' ",vendedor);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_gen);


			if (resp_gen->ObtieneNumRegistros()!=0) {
				tienePrivilegioCRE=true;
				instruccion.sprintf("select c.cliente, LEFT(c.rsocial,100) as rsocial, c.rfc, \
				left(concat(c.calle, ' #', c.numext,' -', c.numint, ', ', col.nombre, ' CP:', c.cp, ' ', loc.nombre),100) as domicilio, \
				c.bloqueo, c.credito, c.limcred, sum(ifnull(s.saldo,0)) as saldo, sum(ifnull(p.pedxfact,0)) as pedxfact, c.excederlc, c.plazo, ce.tipoprec, ce.tipoprecmin \
				from clientes c \
				left join clientesemp ce on ce.cliente=c.cliente and ce.idempresa=%s \
				left join colonias col on col.colonia=c.colonia \
				left join localidades loc on col.localidad=loc.localidad \
				left join ( \
					SELECT c.cliente, SUM(t.valor) AS saldo \
					FROM clientes c \
					INNER JOIN ventas v ON v.cliente=c.cliente \
					INNER JOIN transxcob t ON t.referencia = v.referencia \
					WHERE t.cancelada=0 AND v.cancelado=0 AND %s \
					t.aplicada=1 AND \
					(excederlc = 0)  AND c.activo=1 \
					GROUP BY v.cliente \
					ORDER BY v.cliente \
				) s on s.cliente=c.cliente \
				left join ( \
					SELECT c.cliente, sum(p.valor) as pedxfact FROM pedidosventa p \
					inner join clientes c on c.cliente=p.cliente \
					where p.cancelado=0 and p.facturado=0 and p.acredito=1 \
					and p.fechaped>=DATE_SUB(CURDATE(), INTERVAL 5 DAY) \
					group by c.cliente \
				) p on p.cliente=c.cliente \
				where ( \
				excederlc = 0 \
				) and c.activo=1 \
				group by c.cliente \
				order by c.rsocial ", FormServidor->ObtieneClaveEmpresa(), condicion_fechas_ventassaldo);
			} else {
				tienePrivilegioCRE=false;
				char * bf_imitar = new char[1024];
				AnsiString queryOriginal = " ";
				AnsiString vendedorOriginal = vendedor;
				try{
                    instruccion.sprintf("SELECT vendedor_imitar from vendedores ven \
						WHERE ven.empleado = '%s' AND NOT ISNULL(vendedor_imitar)", vendedor);
					mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), bf_imitar);
					BufferRespuestas br_imitar(bf_imitar);

					int i = 0;

					do{

						if(i > 0){
							queryOriginal += " UNION ALL ";
							br_imitar.IrAlRegistroNumero(i - 1);
							vendedor = br_imitar.ObtieneDato();
						}
						instruccion.sprintf("(select c.cliente, LEFT(c.rsocial,100) as rsocial, c.rfc, \
						left(concat(c.calle, ' #', c.numext,' -', c.numint, ', ', col.nombre, ' CP:', c.cp, ' ', loc.nombre),100) as domicilio, \
						c.bloqueo, c.credito, c.limcred, sum(ifnull(s.saldo,0)) as saldo, sum(ifnull(p.pedxfact,0)) as pedxfact, c.excederlc, c.plazo, ce.tipoprec, ce.tipoprecmin \
						from clientes c \
						left join clientesemp ce on ce.cliente=c.cliente and ce.idempresa=%s \
						left join colonias col on col.colonia=c.colonia \
						left join localidades loc on col.localidad=loc.localidad \
						left join ( \
							select c.cliente, sum(t.valor) as saldo \
							FROM clientes c \
								INNER JOIN ventas v ON v.cliente=c.cliente \
								INNER JOIN transxcob t ON t.referencia = v.referencia \
								LEFT JOIN clientesemp cemp ON c.cliente = cemp.cliente AND cemp.idempresa=%s \
							where t.cancelada=0 and v.cancelado=0 and %s \
							t.aplicada=1 and \
							(cemp.vendedor='%s' OR cemp.cobrador='%s') and c.activo=1  \
							group by v.cliente \
						) s on s.cliente=c.cliente \
						left join ( \
							SELECT c.cliente, sum(p.valor) as pedxfact \
							FROM pedidosventa p \
							inner join clientes c on c.cliente=p.cliente \
							where p.cancelado=0 and p.facturado=0 and p.acredito=1 and p.vendedor='%s' \
							and p.fechaped>=DATE_SUB(CURDATE(), INTERVAL 5 DAY) \
							group by c.cliente \
						) p on p.cliente=c.cliente \
						where (ce.vendedor='%s' or ce.cobrador='%s') and c.activo=1 \
						group by c.cliente \
						order by c.rsocial )",
						empresa, empresa, condicion_fechas_ventassaldo,
						vendedor, vendedor, vendedor, vendedor, vendedor);

					queryOriginal += instruccion;
					i+=1;
					}while(i <= br_imitar.ObtieneNumRegistros());
				}__finally{
					delete bf_imitar;
				}

				instruccion = queryOriginal;
                vendedor = vendedorOriginal;
			}
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
            // se quita la condición donde solo se envían facturas que no están congeladas
			//AND v.referencia not in(select referencia from ventascongeladas where activo=1)

			// Manda DIRECCIONES DE ENTREGA CLIENTES
			if (resp_gen->ObtieneNumRegistros()!=0) {
				instruccion.sprintf("select c.cliente,LEFT(c.rsocial,100) as rsocial, 0 as iddireccion, \
				case when (c.cliente not in(select cliente from direccionesentregaclientes)) then 1 else 0 end as dafault, \
				left(concat(c.calle, ' #', c.numext, if (c.numint<>'' , ' -' , '' ), c.numint, ', ', col.nombre, \
					' CP:', c.cp, ' ', loc.nombre),100) as domicilio \
					from clientes as c \
						left join colonias col on col.colonia=c.colonia \
						left join localidades loc on col.localidad=loc.localidad \
					where (excederlc = 0) and c.activo=1 \
					UNION ALL \
					select c.cliente,c.rsocial, dc.iddireccion, dc.dafault , \
				left(concat(dc.calle, ' #', dc.numext,if (dc.numint<>'' , ' -' , '' ), dc.numint, ', ', col.nombre, \
				' CP:', dc.cp, ' ', loc.nombre),100) as domicilio \
					from clientes as c \
						inner join  direccionesentregaclientes  as dc on dc.cliente=c.cliente \
						left join colonias col on col.colonia=dc.colonia \
						left join localidades loc on col.localidad=loc.localidad \
					where (excederlc = 0 ) and c.activo=1 \
					order by rsocial,iddireccion");
					mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
			}else{

                char * bf_imitar = new char[1024];
				AnsiString queryOriginal = " ";
				AnsiString vendedorOriginal = vendedor;
				try{
                    instruccion.sprintf("SELECT vendedor_imitar from vendedores ven \
						WHERE ven.empleado = '%s' AND NOT ISNULL(vendedor_imitar)", vendedor);
					mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), bf_imitar);
					BufferRespuestas br_imitar(bf_imitar);

					int i = 0;

					do{

						if(i > 0){
							queryOriginal += " UNION ALL ";
							br_imitar.IrAlRegistroNumero(i - 1);
							vendedor = br_imitar.ObtieneDato();
						}
						instruccion.sprintf("select c.cliente,LEFT(c.rsocial,100) as rsocial, 0 as iddireccion, \
						case when (c.cliente not in(select cliente from direccionesentregaclientes)) then 1 else 0 end as dafault, \
						left(concat(c.calle, ' #', c.numext,if (c.numint<>'' , ' -' , '' ), c.numint, ', ', col.nombre, \
						' CP:', c.cp, ' ', loc.nombre),100) as domicilio \
						from clientes as c \
							left join colonias col on col.colonia=c.colonia \
							left join localidades loc on col.localidad=loc.localidad \
							LEFT JOIN clientesemp cemp ON c.cliente = cemp.cliente AND cemp.idempresa=%s \
						where (cemp.vendedor='%s' or cemp.cobrador='%s') and c.activo=1 \
						UNION ALL \
						select c.cliente,c.rsocial, dc.iddireccion, dc.dafault , \
						left(concat(dc.calle, ' #', dc.numext,if (dc.numint<>'' , ' -' , '' ), dc.numint, ', ', col.nombre, \
						' CP:', dc.cp, ' ', loc.nombre),100) as domicilio \
						from clientes as c \
							inner join  direccionesentregaclientes  as dc on dc.cliente=c.cliente \
							left join colonias col on col.colonia=dc.colonia \
							left join localidades loc on col.localidad=loc.localidad \
							LEFT JOIN clientesemp cemp ON c.cliente = cemp.cliente AND cemp.idempresa=%s \
						where (cemp.vendedor='%s' or cemp.cobrador='%s') and c.activo=1 \
						order by rsocial,iddireccion",FormServidor->ObtieneClaveEmpresa(),
						vendedor, vendedor,
						FormServidor->ObtieneClaveEmpresa(),
						vendedor,vendedor);

						queryOriginal += instruccion;
						i+=1;
					}while(i <= br_imitar.ObtieneNumRegistros());
				}__finally{
					delete bf_imitar;
				}
				mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			}

			// Manda PRECIOS
			instruccion.sprintf("SELECT p.articulo,p.tipoprec,p.precio \
				FROM precios p \
				INNER JOIN tiposdeprecios t ON p.tipoprec=t.tipoprec \
				INNER JOIN articulos a ON p.articulo=a.articulo \
				WHERE p.precio>=.01 AND (a.activo=1 or (a.activo=0 AND a.fechamodi>=date_add(CURDATE(), INTERVAL -30 DAY))) \
				AND t.idempresa=%s ", FormServidor->ObtieneClaveEmpresa());
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Manda PRESENTACIONES
			instruccion.sprintf("SELECT pre.producto, pre.present, pcb.costobase, \
								pre.modoutil, MAX(a.activo) AS activo \
								FROM presentaciones pre \
								INNER JOIN presentacionescb pcb ON pcb.producto=pre.producto AND pcb.present=pre.present AND pcb.idempresa=%s \
								INNER JOIN articulos a ON pre.producto=a.producto AND pre.present=a.present \
								LEFT JOIN articuloempresacfg aemp ON aemp.articulo = a.articulo AND aemp.idempresa = %s \
								WHERE (aemp.permiteventas=1 OR ISNULL(aemp.permiteventas)) AND (pre.producto,pre.present) IN ( \
								SELECT a2.producto, a2.present \
								FROM articulos a2 \
								WHERE a2.activo=1 OR (a2.activo=0 AND a2.fechamodi>= DATE_ADD(CURDATE(), INTERVAL -30 DAY))) \
								GROUP BY pre.producto, pre.present", FormServidor->ObtieneClaveEmpresa(), FormServidor->ObtieneClaveEmpresa());
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Manda PRODUCTOS
			instruccion.sprintf("SELECT pro.producto, pro.nombre, pro.marca, MAX(a.activo) AS activo \
								FROM productos pro \
								INNER JOIN presentaciones pre ON pro.producto=pre.producto \
								INNER JOIN articulos a ON pre.producto=a.producto AND pre.present=a.present \
								LEFT JOIN articuloempresacfg aemp ON aemp.articulo = a.articulo AND aemp.idempresa = %s \
								WHERE (aemp.permiteventas = 1 OR ISNULL(aemp.permiteventas)) AND pro.producto IN ( \
								SELECT a2.producto \
								FROM articulos a2 \
								WHERE a2.activo=1 OR (a2.activo=0 AND a2.fechamodi>= DATE_ADD(CURDATE(), INTERVAL -30 DAY))) \
								GROUP BY pro.producto", FormServidor->ObtieneClaveEmpresa());
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Manda MARCAS
			instruccion.sprintf("SELECT marca, nombre \
								FROM marcas");
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Manda TERMINOSDEPAGO
			instruccion.sprintf("SELECT termino, descripcion, diasdefoult, terminoreal \
								FROM terminosdepago WHERE activoappvendedores='1'");
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Manda TIPOSDEPRECIOS
			instruccion.sprintf("SELECT t.tipoprec, t.descripcion, t.listamovil, t.verventmayoreo, t.verprecdif \
								FROM tiposdeprecios t WHERE t.idempresa=%s ", FormServidor->ObtieneClaveEmpresa());
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Manda TIPOSFACTURASVENTAS
			instruccion.sprintf("SELECT tipo, tiporfc, descripcion \
								FROM tiposfacturasventas");
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Manda MENSAJES
			// '%Y%m%d %H:%i:%s'
			instruccion="select mensaje, remitente, \
				concat(nombre, ' ', appat, ' ', apmat) as nomremit, \
				date_format(fechaenvio, '%Y%m%d %H:%i:%s') as fechaenvio, \
				date_format(horaenvio, '20000101 %H:%i:%s') as horaenvio, \
				urgente, leido, contenido, asunto \
				from mensajes, empleados \
				where empleados.empleado=remitente and destmovil=1 and enviadomovil=0";
			instruccion=instruccion+" and destino='";
			instruccion=instruccion+vendedor;
			instruccion=instruccion+"'";
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// Manda SALDOS
			// Por optimización serán solo saldos de ventas de los últimos dos años
			// Ya que si tiene saldo de antes ya debería estar cancelado su crédito.
			if (!tienePrivilegioCRE){
				condicion_cliente_vendedor.sprintf("AND v.cliente IN (SELECT cli.cliente FROM clientes cli \
				INNER JOIN clientesemp cemp ON cli.cliente = cemp.cliente \
				WHERE (cemp.vendedor='%s' OR cemp.cobrador='%s') \
				AND cli.activo=1 )", vendedor,vendedor);
			}else{
				condicion_cliente_vendedor.sprintf(" ");
			}

			instruccion="CREATE TEMPORARY TABLE auxventassaldos (venta CHAR(11), saldor DECIMAL(16,2),   chcnc DECIMAL(16,2), tncredito DECIMAL(16,2), tncargo DECIMAL(16,2), fechach DATE, PRIMARY KEY (venta)) Engine = INNODB";
			mServidorVioleta->EjecutaSelectSqlNulo(Respuesta,  MySQL, instruccion.c_str());

			instruccion.sprintf("INSERT INTO auxventassaldos ( \
				  venta, \
				  saldor, \
				  chcnc, \
				  tncredito, \
				  tncargo, \
				  fechach \
				) \
				SELECT \
				  v.referencia AS venta, \
				  SUM(IF(t.aplicada = 1, t.valor, 0)) AS saldor, \
				  SUM(IF(t.aplicada = 0, t.valor, 0)) AS chcnc, \
				  SUM( \
					IF( \
					  t.aplicada = 1 \
					  AND t.tipo = 'DEVO', \
					  t.valor, \
					  0 \
					) \
				  ) AS tncredito, \
				  SUM( \
					IF( \
					  t.aplicada = 1 \
					  AND (t.tipo = 'NCAR' \
						OR t.tipo = 'INTE'), \
					  t.valor, \
					  0 \
					) \
				  ) AS tncargo, \
				  MAX( \
					IFNULL( \
					  chcl.fechacob, \
					  CAST('1900-01-01' AS DATE) \
					) \
				  ) AS fechach \
				FROM \
				  ventas v \
				  INNER JOIN transxcob t \
				  LEFT JOIN pagoscli p ON t.pago = p.pago AND t.aplicada = 0 \
				  LEFT JOIN cheqxcob chxc ON p.pago = chxc.pago \
				  LEFT JOIN chequesclientes chcl ON chxc.chequecli = chcl.chequecli \
				WHERE t.referencia = v.referencia \
				  AND v.fechavta between '%s' and '%s' \
				  AND t.cancelada = 0 \
				  AND v.cancelado = 0 AND v.acredito = 1 \
				   %s \
				GROUP BY v.referencia \
				HAVING saldor > 0",
				mFg.DateToMySqlDate(IncMonth(Today(),-24)),
				mFg.DateToMySqlDate(Today()),
				condicion_cliente_vendedor );
			mServidorVioleta->EjecutaSelectSqlNulo(Respuesta,  MySQL, instruccion.c_str());

			// Manda SALDOS
			instruccion.sprintf("SELECT \
				  v.referencia, \
				  IF( \
					IFNULL(v.foliofisic, '') = '', \
					CONCAT( \
					  IFNULL(cfd.serie, ''), \
					  IFNULL(cfd.folio, '') \
					), \
					v.foliofisic \
				  ) AS foliofisic, \
				  v.cliente AS cliente, \
				  f.termino, \
				  v.tipofac, \
				  vs.saldor AS saldo, \
				  (vs.chcnc * - 1) AS chnocob, \
				  (vs.tncredito * - 1) AS tncredito, \
				  vs.tncargo, \
				  vs.fechach, \
				  v.fechavta, \
				  v.horaalta, \
				  v.fechavenc, \
				  v.valor \
				FROM \
				  ventas v \
				  INNER JOIN auxventassaldos vs \
				  INNER JOIN dventasfpago d ON d.referencia = v.referencia AND d.valor = ( \
					SELECT MAX(d2.valor) FROM dventasfpago d2 WHERE d2.referencia = v.referencia LIMIT 1 ) \
				  INNER JOIN formasdepago f ON d.formapag = f.formapag \
				  LEFT JOIN cfd ON v.referencia = cfd.referencia AND cfd.tipocomprobante = 'VENT' \
				WHERE vs.venta = v.referencia \
				  AND vs.saldor > 0 \
				ORDER BY v.referencia ");
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			// bitacora de actualizacion
			/*instruccion.sprintf("insert into bitacoraapp ( usuario, horaalta,fechaalta,tipo,opcion,ip,version_app)\
			values ('%s', CURTIME(), CURDATE(),'Pedido', 34, ,)" ,vendedor);
			mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str());*/

		}
	} __finally {

		if (resp_gen!=NULL) delete resp_gen;
		if (resp_almacenes!=NULL) delete resp_almacenes;
		mServidorVioleta->BorraArchivoTemp(archivo_temp_1);
		delete buffer_sql;
	}
}
//---------------------------------------------------------------------------
//ID_GRA_USPASCONTPAQ
void ServidorAdminSistema::AsignaUsuarioPassContpaq(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CAMBIA LA CLAVE DE ACCESO AL SISTEMA DE UN USUARIO DADO
	char *buffer_sql=new char[1024*5];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instruccion, instrucciones[5];
	int num_instrucciones=0;
	int i;
	AnsiString usuario, usuario_contpaq, password;

	try{
		usuario = mFg.ExtraeStringDeBuffer(&parametros);
		usuario_contpaq = mFg.ExtraeStringDeBuffer(&parametros);
		password = mFg.ExtraeStringDeBuffer(&parametros);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

		// Se asigna una variable el identificador del usurio que coincide con el identificador y el password
		instruccion.sprintf("select @idusuario:=empleado from usuarios where empleado='%s' ", usuario);
		instrucciones[num_instrucciones++]=instruccion;

		// Si la clave es correcta para el usuario dado, entonces cambia dicha clave por la nueva
		instruccion.sprintf("update usuarios set usuariocontpaq= AES_ENCRYPT('%s','LaVioletaSaSvMartinCastrejon2020UsuarioPassContpaq'), \
				passwordcontpaq=AES_ENCRYPT('%s', 'LaVioletaSaSvMartinCastrejon2020UsuarioPassContpaq') \
				where empleado='%s' ", usuario_contpaq, password, usuario);
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
//ID_GRA_BITA_FECHAVENCIMIENTO
void ServidorAdminSistema::GrabaBitacoraFechaVencimiento(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
	AnsiString fecha_nueva,fecha_anti, terminal, usuario, sucursal, referencia, razon, tipo;

	fecha_nueva = mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
	fecha_anti = mFg.ExtraeStringDeBuffer(&parametros);
	usuario=mFg.ExtraeStringDeBuffer(&parametros);
	referencia=mFg.ExtraeStringDeBuffer(&parametros);
	razon=mFg.ExtraeStringDeBuffer(&parametros);
	tipo=mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	if (tipo=="Compra") {
		instruccion.sprintf("UPDATE compras SET fechavenc='%s' WHERE referencia='%s' ", fecha_nueva, referencia );
		if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
			throw (Exception("Error UPDATE estadosistemaemp de Fecha de corte"));
	} else {
		instruccion.sprintf("UPDATE gastos SET fechavenc='%s' WHERE referencia='%s' ", fecha_nueva, referencia );
		if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
			throw (Exception("Error UPDATE estadosistemaemp de Fecha de corte"));
	}

	instruccion.sprintf("insert into bitacorafechasvencimiento \
	(referencia, usuario, fechavencact, fechavencant, fecha, hora, razon) \
	values ('%s', '%s', '%s', '%s', CURDATE(), CURTIME(), '%s' )", referencia, usuario, fecha_nueva, fecha_anti, razon);
	if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
		throw (Exception("Error insert into bitacorafechasvencimiento "));

	instruccion.sprintf("select 0 as error");
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

}
//---------------------------------------------------------------------------
//ID_GRA_CFDI_PAGOSGASTOS
void ServidorAdminSistema::GrabaCfdiPagosGastos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
        //  Graba el CFDI de un Complemento de Pago
        char *buffer_sql=new char[1024*(256+32)]; // PARA UN XML DE 128 KB + 128KB por estar en base 64
        char *aux_buffer_sql=buffer_sql;
        AnsiString instrucciones[50], instruccion;
        int num_instrucciones=0;
        AnsiString pago, muuid, xml="", totalcomprobante="", fechacomprobante="", fechatimbrado="", version="", nombrearch="";
        AnsiString emisor="", rfcemisor="", receptor="", rfcreceptor="";
        int i;
        TDate fecha=Today();
        TTime hora=Time();

        try{
                pago=mFg.ExtraeStringDeBuffer(&parametros); // pago
                muuid=mFg.ExtraeStringDeBuffer(&parametros); // UUID del Complemento de Pago
                if (muuid!=" ") {
                        xml=mFg.ExtraeStringDeBuffer(&parametros); // XML del Complemento de Pago
                        if (xml!=" ") {
                                // SI HAY DATOS DE XML
                                rfcemisor=mFg.ExtraeStringDeBuffer(&parametros); // RFC de emisor
                                rfcreceptor=mFg.ExtraeStringDeBuffer(&parametros); // RFC de receptor
                                totalcomprobante=mFg.ExtraeStringDeBuffer(&parametros); // Total del XML
                                fechacomprobante=mFg.ExtraeStringDeBuffer(&parametros); // Fecha de comprobante en el XML
								fechatimbrado=mFg.ExtraeStringDeBuffer(&parametros); // Fecha de timbrado en el XML
                                version=mFg.ExtraeStringDeBuffer(&parametros); // Version del XML
                                nombrearch=mFg.ExtraeStringDeBuffer(&parametros); // Nombre de archivo del XML
                                emisor=mFg.ExtraeStringDeBuffer(&parametros); // Nombre de emisor
                                receptor=mFg.ExtraeStringDeBuffer(&parametros); // Nombre de receptor
                        } else {
                                xml="";
                                rfcemisor=mFg.ExtraeStringDeBuffer(&parametros); // RFC de emisor
                                if (rfcemisor!=" ") {
                                        // SOLO DATOS DE QRCODE
                                        rfcreceptor=mFg.ExtraeStringDeBuffer(&parametros); // RFC de receptor
                                        totalcomprobante=mFg.ExtraeStringDeBuffer(&parametros); // Total del XML
                                } else {
                                        rfcemisor="";
                                }
                        }
                } else muuid="";

                instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
                instrucciones[num_instrucciones++]="START TRANSACTION";

                // Borra el cfdi del Complemento de Pago por si existe.
                instruccion.sprintf("delete from cfdipagosgastos where pago='%s'", pago);
                instrucciones[num_instrucciones++]=instruccion;

                if (muuid!="") {
                        instruccion.sprintf("update pagosgastos set muuid='%s' where pago='%s'", muuid, pago);
                } else {
                        instruccion.sprintf("update pagosgastos set muuid=null where pago='%s'", pago);
                }
                instrucciones[num_instrucciones++]=instruccion;


                // Si hay XML graba un registro en cfdipagosprov
                if (xml!="") {
                        instruccion.sprintf("insert into cfdipagosgastos (pago, montopago, fechacomprobante, fechatimbrado, \
                                fechacarga, \
                                version, nombrearch, emisor, rfcemisor, receptor, rfcreceptor, pdf, xml ) \
                                values ('%s',%s,'%s','%s','%s %s','%s','%s','%s','%s','%s','%s', '','%s')",
                                pago, totalcomprobante, fechacomprobante, fechatimbrado,
                                mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora),
                                version, nombrearch, emisor,rfcemisor, receptor, rfcreceptor, xml );
                        instrucciones[num_instrucciones++]=instruccion;
                } else {
                        if (rfcemisor!="") {
                                instruccion.sprintf("insert into cfdipagosgastos (pago, montopago, fechacomprobante, fechatimbrado, \
                                        fechacarga, \
                                        version, nombrearch, emisor, rfcemisor, receptor, rfcreceptor, pdf, xml ) \
                                        values ('%s',%s,'','','%s %s','','','','%s','','%s', '','')",
                                        pago, totalcomprobante,
                                        mFg.DateToMySqlDate(fecha), mFg.TimeToMySqlTime(hora), rfcemisor, rfcreceptor );
                                instrucciones[num_instrucciones++]=instruccion;
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
//---------------------------------------------------------------------------

//ID_GUARDA_BITACORA_CONFIG_PINPADS
void ServidorAdminSistema::GrabaBitaConfigPinpad(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	//  Graba el CFDI de un Complemento de Pago
	char *buffer_sql=new char[1024*(256+32)]; // PARA UN XML DE 128 KB + 128KB por estar en base 64
	char *aux_buffer_sql=buffer_sql;
	AnsiString instrucciones[50], instruccion;
	int num_instrucciones=0;
	AnsiString campos, valores;
	int i;

	try{
			campos=mFg.ExtraeStringDeBuffer(&parametros); // pago
			valores=mFg.ExtraeStringDeBuffer(&parametros); // UUID del Complemento de Pago

			instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
			instrucciones[num_instrucciones++]="START TRANSACTION";

			instruccion.sprintf("insert into bitacoraconfigpinpads (%s) values (%s)", campos, valores);
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
//ID_CON_ENVIODATOSINVMOVIL_B
void ServidorAdminSistema::EnvioDatosInvMovilB(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Manda parámetros  solo para controlar la versión y subversion minima de APP
	instruccion.sprintf("select parametro, left(descripcion,40) as descripcion, valor \
		from parametrosemp  where parametro like'%version%' AND sucursal = '%s' ", FormServidor->ObtieneClaveSucursal());
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Manda articulos  avtivos
	/*instruccion.sprintf("select a.articulo, a.producto, a.present, a.multiplo,  a.ean13, a.factor, 0 as cantidad \
		from articulos a where activo=1");   */
	/*instruccion.sprintf("select a.articulo, a.producto, a.present, a.multiplo,  a.ean13, a.factor, 0 as cantidad, p.permitfrac, a.activo \
		from articulos a inner join presentaciones p on p.producto= a.producto and a.present=p.present  where activo=1 ");
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado); */

		// Manda articulos  inactivos
	/*instruccion.sprintf("select a.articulo, a.producto, a.present, a.multiplo,  a.ean13, a.factor, 0 as cantidad \
		from articulos a where activo=1");   */
	/*instruccion.sprintf("select a.articulo, a.producto, a.present, a.multiplo,  a.ean13, a.factor, 0 as cantidad, p.permitfrac, a.activo \
		from articulos a inner join presentaciones p on p.producto= a.producto and a.present=p.present  where activo=0 ");
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);*/

  instruccion.sprintf("select a.articulo, a.producto, a.present, a.multiplo,  a.ean13, a.factor, 0 as cantidad, p.permitfrac, a.activo  \
		from articulos a inner join presentaciones p on p.producto= a.producto and a.present=p.present  WHERE a.activo=1    \
			union  \
		select a.articulo, a.producto, a.present, a.multiplo,  a.ean13, a.factor, 0 as cantidad, p.permitfrac, a.activo  \
		from articulos a inner join presentaciones p on p.producto= a.producto and a.present=p.present \
		WHERE a.activo=0 AND  a.fechamodi> DATE_SUB(NOW(),INTERVAL 8 MONTH) ");
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);



	// Manda MARCAS
	instruccion.sprintf("select marca, nombre \
		from marcas");
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Manda PRESENTACIONES
	instruccion.sprintf("select producto, present from ( \
		select pre.producto, pre.present, max(a.activo) as activo \
		from presentaciones pre, articulos a \
		where pre.producto=a.producto and pre.present=a.present \
		group by pre.producto, pre.present \
		) p where activo=1");
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Manda PRODUCTOS
	/*instruccion.sprintf("select producto, nombre, marca from \
		(select pro.producto, pro.nombre, pro.marca, \
		max(a.activo) as activo \
		from productos pro, presentaciones pre, articulos a \
		where pro.producto=pre.producto and pre.producto=a.producto \
		and pre.present=a.present \
		group by pro.producto \
		) p  where activo=1");*/
	instruccion.sprintf(" select producto, nombre, marca from \
		(select TRIM(pro.producto) AS producto,  \
		pro.nombre, pro.marca, max(a.activo) as activo  \
		from productos pro  \
		inner join presentaciones pre ON pro.producto=pre.producto  \
		inner join articulos a on pre.present=a.present AND pre.producto=a.producto  \
		WHERE a.activo=1  \
		group by pro.producto  \
		ORDER BY TRIM(pro.producto) asc \
		) p  where activo=1");
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Manda INVENTARIOS (solo lo de hace un més hasta la fecha)
	instruccion.sprintf("select inventario, almacen, descripcion, tipo, fechainv, cerrado \
		from inventarios where fechainv>='%s'", mFg.DateToMySqlDate(IncMonth(Today(),-6)));
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Manda ALMACENES
	instruccion.sprintf("select almacen, seccion, nombre \
		from almacenes");
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Manda CODIGOS DE BARRAS adicionales  and a.activo=1
	instruccion.sprintf("select c.articulo, c.codigobarras, 'D' as descripcion \
		from codigosbarras c, articulos a \
		where c.articulo=a.articulo ");
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Manda PROVEEDORES
	instruccion.sprintf("select proveedor, IF(razonsocial='','Vacio',razonsocial) AS razonsocial \
		from proveedores");
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);


}
//---------------------------------------------------------------------------
//ID_ACTUALIZA_PRECIOS_MANUAL
void ServidorAdminSistema::ActualizaPreciosManual(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	BufferRespuestas *resultado_select=NULL;
	BufferRespuestas *resultado_articulos=NULL;
	char *buffer_sql=new char[1024*1024*32];
	char *aux_buffer_sql=buffer_sql;
	AnsiString Select;
	AnsiString * instrucciones = new AnsiString[500000];
	AnsiString instruccion;
	int i, num_instrucciones=0;
	int NumeroRegistros;
	AnsiString fecha=mFg.DateToMySqlDate(Today()-1);

	AnsiString articulo, producto, present, tipoprecio, precioant, precioproximo;

	try {

		instruccion.sprintf("SELECT COUNT(articulo) AS numregistros  FROM precios \
		INNER JOIN tiposdeprecios tp ON tp.tipoprec=precios.tipoprec \
		WHERE actualizarpendiente = 1");
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resultado_select);
		NumeroRegistros=StrToInt(resultado_select->ObtieneDato("numregistros"));
		if (NumeroRegistros>0) {
			try {

				instruccion.sprintf("SELECT a.articulo, a.producto, a.present, p.precio, p.preciomod, p.tipoprec  \
				FROM precios p \
				INNER JOIN tiposdeprecios tp ON tp.tipoprec = p.tipoprec \
				INNER JOIN articulos a ON a.articulo = p.articulo \
				WHERE actualizarpendiente = 1");
				mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resultado_articulos);

                instrucciones[num_instrucciones++]="SET SESSION sql_log_bin = 0";
				instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
				instrucciones[num_instrucciones++]="START TRANSACTION";

				instruccion.sprintf("INSERT INTO bitacorapreciosdiferidos SELECT NULL,CURDATE(),CURTIME(),NULL");
				instrucciones[num_instrucciones++]=instruccion;

				instrucciones[num_instrucciones++]="set @referencia:=LAST_INSERT_ID()";

				for (int i = 0; i < resultado_articulos->ObtieneNumRegistros(); i++) {

					resultado_articulos->IrAlRegistroNumero(i);

					articulo = resultado_articulos->ObtieneDato("articulo");
					producto = resultado_articulos->ObtieneDato("producto");
					present  = resultado_articulos->ObtieneDato("present");
					tipoprecio = resultado_articulos->ObtieneDato("tipoprec");
					precioant  = resultado_articulos->ObtieneDato("precio");
					precioproximo = resultado_articulos->ObtieneDato("preciomod");

					instruccion.sprintf("INSERT INTO bitacorapreciosdiferidosdetalle \
					(referencia, articulo, precioanterior, precioproximo, tipoprec) \
					VALUES \
					(@referencia, '%s', '%s', '%s', '%s')",
					articulo,precioant,precioproximo,tipoprecio);
					instrucciones[num_instrucciones++]=instruccion;

					instruccion.sprintf("UPDATE precios AS p \
					INNER JOIN tiposdeprecios tp ON tp.tipoprec = p.tipoprec \
					INNER JOIN articulos a ON a.articulo=p.articulo \
					INNER JOIN productos pro ON a.producto=pro.producto \
					SET p.precio=p.preciomod, p.actualizarpendiente = 0 \
					WHERE p.actualizarpendiente = 1 AND a.producto='%s' \
					AND a.present = '%s' ",
					producto,present);
					instrucciones[num_instrucciones++]=instruccion;
				}

				instruccion.sprintf("update estadosistemaemp set valor='%s' where estado='FECHAACTPRECI' AND sucursal = '%s' ",fecha, FormServidor->ObtieneClaveSucursal());
				instrucciones[num_instrucciones++]=instruccion;

				instrucciones[num_instrucciones++]="COMMIT";
				instrucciones[num_instrucciones++]="SET SESSION sql_log_bin = 1";

				// Crea el buffer con todas las instrucciones SQL
				aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
				for (i=0; i<num_instrucciones; i++)
						aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

				mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
				if (!mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
					throw Exception("ERROR al actualizar los precios");
				}

			} catch(Exception &e){
				throw Exception("ERROR al actualizar los precios:"+e.Message);
			}
		} else {
			throw Exception("No hay productos pendientes de actualizar");
		}
	} __finally {
		delete buffer_sql;
		if (resultado_select!=NULL) delete resultado_select;
		if (resultado_articulos!=NULL) delete resultado_articulos;
        delete[] instrucciones;
	}
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// ID_CON_ASIGPRIVROL
void ServidorAdminSistema::ConsultaAsigPrivRol(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA ASIGNACION DE PRIVILEGIOS POR ROL
	AnsiString instruccion;
	AnsiString rol, grupo, objeto, sucursal;
	AnsiString CondicionGrupo, CondicionObjeto;
	char *resultado;

	rol=mFg.ExtraeStringDeBuffer(&parametros);
	grupo=mFg.ExtraeStringDeBuffer(&parametros);
	objeto=mFg.ExtraeStringDeBuffer(&parametros);
	//sucursal=mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);
	CondicionGrupo=" ";
	CondicionObjeto=" ";

	// Obtiene todos los roles
	instruccion="select rolessistema.claverol AS Rol, rolessistema.nombre AS Nombre ";
	instruccion+="from rolessistema order by rolessistema.nombre";
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene todos los grupos
	instruccion="SELECT g.grupo, g.nombre, cast(count(a.privilegio)*100/count(p.privilegio) as decimal(16,2)) as porcentaje ";
	instruccion+="FROM gruposobjetos g inner join objetossistema o inner join privilegios p ";
	instruccion+="left join rolessistema r on r.claverol='";
	instruccion+=rol;
	instruccion+="' left join asignacionprivrol a on r.claverol=a.rol and o.objeto=a.objeto and a.privilegio=p.privilegio ";
	instruccion+="where  g.grupo=o.grupo and o.objeto=p.objeto ";
	instruccion+="group by g.grupo ";
	resultado=Respuesta->ObtieneDirLibreBuffer();
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Si no se especificó ningún grupo, se toma el primero del resultado para
	// obtener sus objetos.
	if (grupo=="") {
		if (objeto=="") {
			BufferRespuestas resp_grupos(resultado);
			if (resp_grupos.ObtieneNumRegistros()) {
				grupo=resp_grupos.ObtieneDato("grupo");
				CondicionGrupo=" o.grupo='" + grupo+ "' and ";
			}
		}else{
		   CondicionGrupo= " o.grupo in( select grupo from objetossistema where objeto='" + objeto + "') AND ";
		}
	}else{
		CondicionGrupo=" o.grupo='" + grupo+ "' and ";
	}

	// Obtiene todos los objetos del grupo
	instruccion="SELECT o.objeto, o.nombre, cast(count(a.privilegio)*100/count(p.privilegio) as decimal(16,2)) as porcentaje ";
	instruccion+="FROM objetossistema o inner join privilegios p ";
	instruccion+="left join rolessistema r on r.claverol='";
	instruccion+=rol;
	instruccion+="' left join asignacionprivrol a on r.claverol=a.rol and o.objeto=a.objeto and a.privilegio=p.privilegio ";
	instruccion+="where  ";
	instruccion+=CondicionGrupo;
	instruccion+=" o.objeto=p.objeto ";
	instruccion+="group by o.objeto ORDER BY o.nombre ASC";
	resultado=Respuesta->ObtieneDirLibreBuffer();
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Si no se especificó ningún objeto, se toma el primero del resultado para
	// obtener sus objetos.
	if (objeto=="") {
		BufferRespuestas resp_objetos(resultado);
		if (resp_objetos.ObtieneNumRegistros()) {
			objeto=resp_objetos.ObtieneDato("objeto");
			CondicionObjeto="p.objeto='"+objeto+"'  ";
		}
	}

	// Obtiene todos privilegios del objeto
	instruccion="SELECT concat(p.privilegio,' ',p.descripcion) as privilegio, count(a.privilegio) as concedido ";
	instruccion+="FROM privilegios p ";
	instruccion+="left join rolessistema r ON r.claverol='";
	instruccion+=rol;
	instruccion+="' left join asignacionprivrol a on r.claverol=a.rol and p.objeto=a.objeto and a.privilegio=p.privilegio ";
	instruccion+="where p.objeto='";
	instruccion+=objeto;
	instruccion+="' group by p.privilegio";
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	// Obtiene todos los roles con privilegios sobre el objeto
	instruccion="SELECT r.nombre as rol, ";
	instruccion+="GROUP_CONCAT(DISTINCT ap.privilegio ORDER BY ap.privilegio) as privilegios, r.claverol ";
	instruccion+="FROM asignacionprivrol ap ";
	instruccion+="INNER JOIN rolessistema r ON r.claverol= ap.rol ";
	instruccion+="WHERE ap.objeto='";
	instruccion+=objeto;
	instruccion+="' ";
	instruccion+=" GROUP BY ap.rol ORDER BY r.nombre, r.claverol";

	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//ID_CONC_ASIGPRIVROL
void ServidorAdminSistema::ConcedeAsigPrivRol(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*5];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instruccion, instrucciones[10];
	int num_instrucciones=0;
	int i;
	AnsiString usuarioSistema, tarea, rol, grupo, objeto, privilegio;

	try{
		usuarioSistema=mFg.ExtraeStringDeBuffer(&parametros);
		rol=mFg.ExtraeStringDeBuffer(&parametros);
		tarea=mFg.ExtraeStringDeBuffer(&parametros);
		if (tarea=="GRUPO") {
			grupo=mFg.ExtraeStringDeBuffer(&parametros);
			instruccion="insert ignore into asignacionprivrol (rol, objeto, privilegio) ";
			instruccion+="select '";
			instruccion+=rol;
			instruccion+="' as rol, o.objeto, p.privilegio ";
			instruccion+="from gruposobjetos g, objetossistema o, privilegios p ";
			instruccion+="where g.grupo='";
			instruccion+=grupo;
			instruccion+="' and g.grupo=o.grupo and o.objeto=p.objeto";
			instrucciones[num_instrucciones++]=instruccion;

			//Se registra en la bitácora de modificaciones de privilegios de roles
			instruccion.sprintf(" INSERT INTO bitacoramodprivrol \
			(fecha, hora, usuario, rol_mod, tipo_mod, entidad_mod, entidad_nombre) \
			VALUES (CURDATE(), CURTIME(), '%s', '%s', 'CONCEDER', 'GRUPO', '%s') "
			, usuarioSistema, rol, grupo);
			instrucciones[num_instrucciones++]=instruccion;
		}
		if (tarea=="OBJETO") {
			objeto=mFg.ExtraeStringDeBuffer(&parametros);
			instruccion="insert ignore into asignacionprivrol (rol, objeto, privilegio) ";
			instruccion+="select '";
			instruccion+=rol;
			instruccion+="' as rol, o.objeto, p.privilegio ";
			instruccion+="from objetossistema o, privilegios p ";
			instruccion+="where o.objeto='";
			instruccion+=objeto;
			instruccion+="' and o.objeto=p.objeto";
			instrucciones[num_instrucciones++]=instruccion;

			//Se registra en la bitácora de modificaciones de privilegios de roles
			instruccion.sprintf(" INSERT INTO bitacoramodprivrol \
			(fecha, hora, usuario, rol_mod, tipo_mod, entidad_mod, entidad_nombre) \
			VALUES (CURDATE(), CURTIME(), '%s', '%s', 'CONCEDER', 'OBJETO', '%s') "
			, usuarioSistema, rol, objeto);
			instrucciones[num_instrucciones++]=instruccion;
		}
		if (tarea=="PRIVILEGIO") {
			privilegio=mFg.ExtraeStringDeBuffer(&parametros);
			objeto=mFg.ExtraeStringDeBuffer(&parametros);
			instruccion="insert ignore into asignacionprivrol (rol, objeto, privilegio) ";
			instruccion+="select '";
			instruccion+=rol;
			instruccion+="' as rol, o.objeto, p.privilegio ";
			instruccion+="from objetossistema o, privilegios p ";
			instruccion+="where o.objeto='";
			instruccion+=objeto;
			instruccion+="' and o.objeto=p.objeto and p.privilegio='";
			instruccion+=privilegio;
			instruccion+="'";
			instrucciones[num_instrucciones++]=instruccion;

			//Se registra en la bitácora de modificaciones de privilegios de roles
			instruccion.sprintf(" INSERT INTO bitacoramodprivrol \
			(fecha, hora, usuario, rol_mod, tipo_mod, entidad_mod, entidad_nombre) \
			VALUES (CURDATE(), CURTIME(), '%s', '%s', 'CONCEDER', 'PRIVILEGIO', '%s: %s') "
			, usuarioSistema, rol, objeto, privilegio);
			instrucciones[num_instrucciones++]=instruccion;
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

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//ID_QUI_ASIGPRIVROL
void ServidorAdminSistema::QuitaAsigPrivRol(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*5];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instruccion, instrucciones[10];
	int num_instrucciones=0;
	int i;
	AnsiString usuarioSistema, tarea, rol, grupo, objeto, privilegio;

	try{
		usuarioSistema=mFg.ExtraeStringDeBuffer(&parametros);
		rol=mFg.ExtraeStringDeBuffer(&parametros);
		tarea=mFg.ExtraeStringDeBuffer(&parametros);

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		if (tarea=="GRUPO") {
			grupo=mFg.ExtraeStringDeBuffer(&parametros);

			instruccion="delete from a ";
			instruccion+="using asignacionprivrol a, gruposobjetos g, objetossistema o ";
			instruccion+="where a.rol='";
			instruccion+=rol;
			instruccion+="' and g.grupo='";
			instruccion+=grupo;
			instruccion+="' and g.grupo=o.grupo and a.objeto=o.objeto";
			instrucciones[num_instrucciones++]=instruccion;

            //Se registra en la bitácora de modificaciones de privilegios de roles
			instruccion.sprintf(" INSERT INTO bitacoramodprivrol \
			(fecha, hora, usuario, rol_mod, tipo_mod, entidad_mod, entidad_nombre) \
			VALUES (CURDATE(), CURTIME(), '%s', '%s', 'QUITAR', 'GRUPO', '%s') "
			, usuarioSistema, rol, grupo);
			instrucciones[num_instrucciones++]=instruccion;
		}
		if (tarea=="OBJETO") {
			objeto=mFg.ExtraeStringDeBuffer(&parametros);

			instruccion="delete from asignacionprivrol ";
			instruccion+="where rol='";
			instruccion+=rol;
			instruccion+="' and objeto='";
			instruccion+=objeto;
			instruccion+="'";
			instrucciones[num_instrucciones++]=instruccion;

            //Se registra en la bitácora de modificaciones de privilegios de roles
			instruccion.sprintf(" INSERT INTO bitacoramodprivrol \
			(fecha, hora, usuario, rol_mod, tipo_mod, entidad_mod, entidad_nombre) \
			VALUES (CURDATE(), CURTIME(), '%s', '%s', 'QUITAR', 'OBJETO', '%s') "
			, usuarioSistema, rol, objeto);
			instrucciones[num_instrucciones++]=instruccion;
		}
		if (tarea=="PRIVILEGIO") {
			privilegio=mFg.ExtraeStringDeBuffer(&parametros);
			objeto=mFg.ExtraeStringDeBuffer(&parametros);

			instruccion="delete from asignacionprivrol ";
			instruccion+="where rol='";
			instruccion+=rol;
			instruccion+="' and objeto='";
			instruccion+=objeto;
			instruccion+="' and privilegio='";
			instruccion+=privilegio;
			instruccion+="'";
			instrucciones[num_instrucciones++]=instruccion;

            //Se registra en la bitácora de modificaciones de privilegios de roles
			instruccion.sprintf(" INSERT INTO bitacoramodprivrol \
			(fecha, hora, usuario, rol_mod, tipo_mod, entidad_mod, entidad_nombre) \
			VALUES (CURDATE(), CURTIME(), '%s', '%s', 'QUITAR', 'PRIVILEGIO', '%s: %s') "
			, usuarioSistema, rol, objeto, privilegio);
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

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//ID_COP_ASIGPRIVROL
void ServidorAdminSistema::CopiaAsigPrivRol(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*5];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instruccion, instrucciones[10];
	int num_instrucciones=0;
	int i;
	AnsiString usuario_sistema, rol_origen, rol_destino;

	try{
		usuario_sistema=mFg.ExtraeStringDeBuffer(&parametros);
		rol_origen=mFg.ExtraeStringDeBuffer(&parametros);
		rol_destino=mFg.ExtraeStringDeBuffer(&parametros);

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		// Crea una tabla temporal con los privilegios del rol origen
		instruccion="create temporary table tmpcopiaprivil ";
		instruccion+="select '";
		instruccion+=rol_destino;
		instruccion+="' as rol, objeto, privilegio ";
		instruccion+="from asignacionprivrol ";
		instruccion+="where rol='";
		instruccion+=rol_origen;
		instruccion+="'";
		instrucciones[num_instrucciones++]=instruccion;

		// Borra todos los privilegios del rol destino
		instruccion="delete from asignacionprivrol ";
		instruccion+="where rol='";
		instruccion+=rol_destino;
		instruccion+="'";
		instrucciones[num_instrucciones++]=instruccion;

		// Inserta los registros para el rol destino
		instruccion="insert ignore into asignacionprivrol (rol, objeto, privilegio) ";
		instruccion+="select rol,objeto,privilegio from tmpcopiaprivil";
		instrucciones[num_instrucciones++]=instruccion;

        //Se registra en la bitácora de modificaciones de privilegios de roles
		instruccion.sprintf(" INSERT INTO bitacoramodprivrol \
		(fecha, hora, usuario, rol_mod, tipo_mod, entidad_mod, entidad_nombre) \
		VALUES (CURDATE(), CURTIME(), '%s', '%s', 'COPIAR', 'ROL', '%s') "
		, usuario_sistema, rol_destino, rol_origen);
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

//---------------------------------------------------------------------------
//ID_OBT_ASIGPRIVROL
void ServidorAdminSistema::ObtieneAsigPrivRol(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA PARAMETRO
	AnsiString instruccion;
	AnsiString rol, objeto;

	rol=mFg.ExtraeStringDeBuffer(&parametros);
	objeto=mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los privilegios que tiene un rol con respecto a un objeto
	instruccion.sprintf("select p.privilegio, p.descripcion from asignacionprivrol a, privilegios p ,rolessistema r where a.privilegio=p.privilegio and a.objeto=p.objeto and a.rol=r.claverol and a.rol='%s' and a.objeto='%s'", rol, objeto);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//ID_CON_USUAROL
void ServidorAdminSistema::ConsultaAsigUsuaRol(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CONSULTA ASIGNACION DE ROLES A USUARIOS
	AnsiString instruccion;
	AnsiString rol, sucursal, opcion_sucursal = " ";
	char *resultado;

	rol=mFg.ExtraeStringDeBuffer(&parametros);
	sucursal=mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	if(sucursal != "")
		opcion_sucursal.sprintf(" AND e.sucursal = '%s' ", sucursal);

	instruccion.sprintf("SELECT e.sucursal, CONCAT (e.nombre,' ',e.appat,' ',e.apmat) AS nombre, e.empleado \
	FROM usuariorol ur INNER JOIN empleados e ON e.empleado=ur.usuario \
	INNER JOIN usuarios u ON e.empleado=u.empleado \
	WHERE ur.rol = '%s' AND u.activo=1 %s ORDER BY e.nombre", rol, opcion_sucursal);

	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//ID_COP_ASIGPRIV_GLOUSU
void ServidorAdminSistema::CopiaAsigPrivGloUsuario(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*5];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instruccion, instrucciones[10];
	int num_instrucciones=0;
	int i;
	AnsiString usuario_sistema, usuario_origen, rol_destino;

	try{
		usuario_sistema=mFg.ExtraeStringDeBuffer(&parametros);
		usuario_origen=mFg.ExtraeStringDeBuffer(&parametros);
		rol_destino=mFg.ExtraeStringDeBuffer(&parametros);

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		// Crea una tabla temporal con los privilegios globales del usuario origen
		instruccion="create temporary table tmpcopiaprivil ";
		instruccion+="select '";
		instruccion+=rol_destino;
		instruccion+="' as rol, asigtotal.objeto, asigtotal.privilegio ";
		instruccion+="from (( select ar.objeto, ar.privilegio from usuarios u ";
		instruccion+="inner join usuariorol ur on ur.usuario = u.empleado ";
		instruccion+="inner join asignacionprivrol ar ON ur.rol=ar.rol ";
		instruccion+="WHERE u.empleado = '";
		instruccion+=usuario_origen;
		instruccion+="') UNION (SELECT a.objeto, a.privilegio FROM usuarios u ";
		instruccion+="INNER JOIN asignacionprivilegios a ON u.empleado=a.usuario ";
		instruccion+="WHERE u.empleado = '";
		instruccion+=usuario_origen;
		instruccion+="') UNION (SELECT ar.objeto, ar.privilegio FROM usuarios u ";
		instruccion+="INNER JOIN empleados e ON e.empleado = u.empleado ";
		instruccion+="INNER JOIN puestos p ON p.puesto = e.puesto ";
		instruccion+="INNER JOIN rolesxpuesto rxp ON rxp.puesto = p.puesto ";
		instruccion+="INNER JOIN asignacionprivrol ar ON rxp.rol=ar.rol ";
		instruccion+="WHERE u.empleado = '";
		instruccion+=usuario_origen;
		instruccion+="' )) asigtotal ";
		instrucciones[num_instrucciones++]=instruccion;

		// Borra todos los privilegios del rol destino
		instruccion="delete from asignacionprivrol ";
		instruccion+="where rol='";
		instruccion+=rol_destino;
		instruccion+="'";
		instrucciones[num_instrucciones++]=instruccion;

		// Inserta los registros para el rol destino
		instruccion="insert ignore into asignacionprivrol (rol, objeto, privilegio) ";
		instruccion+="select rol,objeto,privilegio from tmpcopiaprivil";
		instrucciones[num_instrucciones++]=instruccion;

		//Se registra en la bitácora de modificaciones de privilegios de roles
		instruccion.sprintf(" INSERT INTO bitacoramodprivrol \
		(fecha, hora, usuario, rol_mod, tipo_mod, entidad_mod, entidad_nombre) \
		VALUES (CURDATE(), CURTIME(), '%s', '%s', 'COPIAR', 'USUARIO', '%s') "
		, usuario_sistema, rol_destino, usuario_origen);
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

//ID_GRA_PAQUETES_DESCFDI
void ServidorAdminSistema::GrabaPaquetesDesCfdi(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{

	char *buffer_sql=new char[1024*32];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instrucciones[10], instruccion;
	int num_instrucciones=0;
	AnsiString clavesolicitud, idpaquete, descargado;
	int i;

	try{
		clavesolicitud=mFg.ExtraeStringDeBuffer(&parametros);
		idpaquete=mFg.ExtraeStringDeBuffer(&parametros);
		descargado=mFg.ExtraeStringDeBuffer(&parametros);

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		// Graba
		instruccion.sprintf("INSERT IGNORE INTO paquetesdescfdi(clavesolicitud, idpaquete, descargado)  VALUES('%s', '%s', %s)", clavesolicitud, idpaquete, descargado);
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

//ID_CON_PAQUETES_DESCFDI
void ServidorAdminSistema::ConsultaPaquetesDesCfdi(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{

	AnsiString instruccion;
	AnsiString clavesolicitud;

	clavesolicitud = mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_REP_GRANDE);

	instruccion.sprintf("SELECT idpaquete, descargado FROM paquetesdescfdi WHERE clavesolicitud='%s' ", clavesolicitud);

	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),Respuesta->TamBufferResultado);
}

//ID_ACTUALIZA_PAQUETES_DESCFDI
void ServidorAdminSistema::ActualizaPaquetesDesCfdi(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{

	char *buffer_sql=new char[1024*32];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instrucciones[10], instruccion;
	int num_instrucciones=0;
	AnsiString idpaquete, descargado;
	int i;

	try{
		idpaquete=mFg.ExtraeStringDeBuffer(&parametros);
		descargado=mFg.ExtraeStringDeBuffer(&parametros);

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		// Actualiza
		instruccion.sprintf("UPDATE paquetesdescfdi SET descargado=%s WHERE idpaquete = '%s' ", descargado, idpaquete);
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

//ID_QUI_PAQUETES_DESCFDI
void ServidorAdminSistema::QuitaPaquetesDesCfdi(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{

	char *buffer_sql=new char[1024*32];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instrucciones[10], instruccion;
	int num_instrucciones=0;
	AnsiString idsolicitud;
	int i;

	try{
		idsolicitud=mFg.ExtraeStringDeBuffer(&parametros);

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		// Actualiza
		instruccion.sprintf("DELETE FROM paquetesdescfdi WHERE clavesolicitud = '%s' ", idsolicitud);
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

//ID_GRA_SOLICITUDES_DESCFDI
void ServidorAdminSistema::GrabaSolicitudesDesCfdi(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{

	//  CANCELA UN GRUPO DE MENSAJES PARA MOVILES
	char *buffer_sql=new char[1024*32];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instrucciones[10], instruccion;
	int num_instrucciones=0;
	AnsiString clavesolicitud, fechaSoli, horaSoli, fechaIni, fechaFin, rfcSoli, rfcEmisor, rfcRecep, estado ;
	int i;

	try{
		clavesolicitud=mFg.ExtraeStringDeBuffer(&parametros);
		fechaIni = mFg.ExtraeStringDeBuffer(&parametros);
		fechaFin = mFg.ExtraeStringDeBuffer(&parametros);
		rfcSoli= mFg.ExtraeStringDeBuffer(&parametros);
		rfcEmisor= mFg.ExtraeStringDeBuffer(&parametros);
		rfcRecep= mFg.ExtraeStringDeBuffer(&parametros);
		estado= mFg.ExtraeStringDeBuffer(&parametros);

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		if(rfcRecep == NULL || rfcRecep == ""){
			rfcRecep = " ";
		}
		if(rfcEmisor == NULL || rfcEmisor == ""){
			rfcEmisor = " ";
		}

		// Graba
		instruccion.sprintf("INSERT INTO solicidescargacfdi(clavesolicitud, fecha_solicitud, hora_solicitud, fecha_inicio, fecha_fin, rfc_solicitante, rfc_emisor, rfc_receptor, estado)\
		 VALUES('%s', CURDATE(), CURTIME(), '%s', '%s', '%s', '%s', '%s', %s)", clavesolicitud, fechaIni, fechaFin, rfcSoli, rfcEmisor, rfcRecep, estado);
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

//ID_CON_SOLICITUDES_DESCFDI
void ServidorAdminSistema::ConsultaSolicitudesDesCfdi(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion, estadoDet, tipoSoli, campo_estado, plazo;
	AnsiString clavesolicitud, fechaIni, fechaFin, rfcSoli, rfcEmisor, rfcRecep;
	AnsiString condicion_clavesol = " ", condicion_fechaini = " ", condicion_fechafin = " ", condicion_rfcsol = " ", condicion_rfcemi = " ", condicion_rfcrec = " ", condicion_tiposol = " ", condicion_plazo = " ";

	clavesolicitud=mFg.ExtraeStringDeBuffer(&parametros);
	fechaIni = mFg.ExtraeStringDeBuffer(&parametros);
	fechaFin = mFg.ExtraeStringDeBuffer(&parametros);
	rfcSoli= mFg.ExtraeStringDeBuffer(&parametros);
	rfcEmisor= mFg.ExtraeStringDeBuffer(&parametros);
	rfcRecep= mFg.ExtraeStringDeBuffer(&parametros);
	estadoDet=mFg.ExtraeStringDeBuffer(&parametros);
	tipoSoli=mFg.ExtraeStringDeBuffer(&parametros);
	plazo=mFg.ExtraeStringDeBuffer(&parametros);

	if(clavesolicitud!="")
		condicion_clavesol.sprintf(" AND clavesolicitud='%s' ", clavesolicitud);

	if(fechaIni!="")
		condicion_fechaini.sprintf(" AND fecha_inicio='%s' ", fechaIni);

	if(fechaFin!="")
		condicion_fechafin.sprintf(" AND fecha_fin='%s' ", fechaFin);

	if(rfcSoli!="")
		condicion_rfcsol.sprintf(" AND rfc_solicitante='%s' ", rfcSoli);

	if(rfcEmisor!="" && rfcEmisor!=" ")
		condicion_rfcemi.sprintf(" AND rfc_emisor='%s' ", rfcEmisor);
	else if(rfcEmisor == " ")
		condicion_rfcemi = " AND rfc_emisor=' ' ";

	if(rfcRecep!="" && rfcRecep!=" ")
		condicion_rfcrec.sprintf(" AND rfc_receptor='%s' ", rfcRecep);
	else if(rfcRecep == " ")
		condicion_rfcrec = " AND rfc_receptor=' ' ";

	if(tipoSoli=="0")
		condicion_tiposol = " AND estado<>7 ";
	else if (tipoSoli=="1")
		condicion_tiposol = " AND estado=7 ";
	else if (tipoSoli=="2") {
		condicion_tiposol = " AND estado IN (1,2,3) ";
	}

	if(plazo!= "" && plazo!="Todos"){
		condicion_plazo.sprintf(" AND fecha_solicitud >= DATE_SUB(CURDATE(),INTERVAL %s MONTH) ", plazo);
	}

	if(estadoDet=="texto"){
		campo_estado = "\
		 CASE \
			WHEN estado = 1 OR estado = 2 THEN 'En Proceso' \
			WHEN estado = 3 THEN 'Listo para descargar' \
			WHEN estado = 4 OR estado = 5 THEN 'Error en la solicitud' \
			WHEN estado = 6 THEN 'Vencida' \
			WHEN estado = 7 THEN 'Descargado' \
		 END AS estado ";
	}else if(estadoDet=="numero")
		campo_estado = " estado ";


	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_REP_GRANDE);

	//Consultar las transacciones de ese pago
	instruccion.sprintf("\
	 SELECT clavesolicitud, fecha_solicitud, hora_solicitud, CONCAT(fecha_inicio, ' al ', fecha_fin) as fechaRango, \
	 rfc_solicitante, rfc_emisor, rfc_receptor, %s \
	 FROM solicidescargacfdi WHERE 1 %s %s %s %s %s %s %s ORDER BY fecha_solicitud DESC, hora_solicitud DESC ",
	  campo_estado, condicion_clavesol, condicion_fechaini, condicion_fechafin, condicion_rfcsol, condicion_rfcemi, condicion_rfcrec,  condicion_tiposol );

	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),Respuesta->TamBufferResultado);

}

//ID_ACTUALIZA_SOLICITUDES_DESCFDI
void ServidorAdminSistema::ActualizaSolicitudesDesCfdi(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{

	//  CANCELA UN GRUPO DE MENSAJES PARA MOVILES
	char *buffer_sql=new char[1024*32];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instrucciones[10], instruccion;
	int num_instrucciones=0;
	AnsiString clavesolicitud, estado ;
	int i;

	try{
		clavesolicitud=mFg.ExtraeStringDeBuffer(&parametros);
		estado=mFg.ExtraeStringDeBuffer(&parametros);

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		// Actualiza
		instruccion.sprintf("UPDATE solicidescargacfdi s SET s.estado=%s WHERE s.clavesolicitud = '%s' ", estado, clavesolicitud);
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

//ID_CON_FACTURAS_COMPRAS
void ServidorAdminSistema::ConsultaFacturasCompras(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
	AnsiString fechaIni, fechaFin, empresa, sucursal, conXML;
	AnsiString condicion_empresa = " ", condicion_sucursal = " ", condicion_con_XML = " ";

	fechaIni = mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
	fechaFin = mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
	empresa = mFg.ExtraeStringDeBuffer(&parametros);
	sucursal = mFg.ExtraeStringDeBuffer(&parametros);
	conXML = mFg.ExtraeStringDeBuffer(&parametros);

	if(empresa != " ")
		condicion_empresa.sprintf(" AND suc.idempresa = %s ", empresa);

	if(sucursal !=" ")
		condicion_sucursal.sprintf(" AND suc.sucursal = '%s' ", sucursal);

	if(conXML == "1")
		condicion_con_XML = " AND cfdi.xml <> '' ";
	else if(conXML == "0")
		condicion_con_XML = " AND cfdi.xml = '' ";


	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_REP_GRANDE);

	//Consultar las trasacciones de ese pago
	instruccion.sprintf("\
		SELECT c.referencia, c.muuid, cfdi.xml, c.fechacom as fecha, c.almacen, \
		c.folioprov as factura, sec.sucursal, 'COMP' as tipocomprobante, \
		prov.proveedor, prov.razonsocial, prov.rfc \
		FROM compras c \
		INNER JOIN cfdicompras cfdi ON cfdi.referencia = c.referencia \
		INNER JOIN terminales t ON t.terminal = c.terminal \
		INNER JOIN secciones sec ON sec.seccion = t.seccion \
		INNER JOIN sucursales suc ON suc.sucursal = sec.sucursal \
		INNER JOIN proveedores prov ON prov.proveedor = c.proveedor \
		WHERE c.cancelado = 0 AND c.fechacom BETWEEN '%s' AND '%s' %s %s %s \
		ORDER BY c.referencia ", fechaIni, fechaFin, condicion_empresa, condicion_sucursal, condicion_con_XML);

	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),Respuesta->TamBufferResultado);
}


//ID_CON_FACTURAS_NOTAS_DCREDITO_CLI
void ServidorAdminSistema::ConsultaFacturasNCC(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
	AnsiString fechaIni, fechaFin, empresa, sucursal, conXML;
	AnsiString condicion_empresa = " ", condicion_sucursal = " ", condicion_con_XML = " ";

	fechaIni = mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
	fechaFin = mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
    empresa = mFg.ExtraeStringDeBuffer(&parametros);
	sucursal = mFg.ExtraeStringDeBuffer(&parametros);
	conXML = mFg.ExtraeStringDeBuffer(&parametros);

	if(empresa != " ")
		condicion_empresa.sprintf(" AND suc.idempresa = %s ", empresa);

	if(sucursal != " ")
		condicion_sucursal.sprintf(" AND sec.sucursal = '%s' ", sucursal);

	if(conXML == "1")
		condicion_con_XML = " AND cfdxml.xmlgenerado <> '' ";
	else if(conXML == "0")
		condicion_con_XML = " AND cfdxml.xmlgenerado = '' ";

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_REP_GRANDE);

	instruccion.sprintf("\
		SELECT c.compfiscal, cfdxml.xmlgenerado, c.referencia, c.muuid, \
		 ncred.referencia, if(ncred.tipo=0, 'DEVOL', if(ncred.tipo=1, 'BONIF', 'DESCU')) as tipodescrito, \
		 if(ifnull(v.foliofisic,'')='',concat(ifnull(cfdv.serie,''),ifnull(cfdv.folio,'')),v.foliofisic) as factura, \
		 c.seriefolio AS foliocfd, ncred.fechanot AS fecha, ncred.horaalta AS hora, cli.rsocial  \
		FROM notascredcli ncred \
		INNER JOIN cfd c ON ncred.referencia = c.referencia AND c.tipocomprobante='NCRE' \
		INNER JOIN cfdxml on c.compfiscal = cfdxml.compfiscal \
		INNER JOIN ventas v ON ncred.venta = v.referencia \
		INNER JOIN terminales t ON t.terminal = ncred.terminal \
		INNER JOIN secciones sec ON sec.seccion = t.seccion \
		INNER JOIN sucursales suc ON suc.sucursal = sec.sucursal \
		LEFT JOIN clientes cli ON cli.cliente = v.cliente \
		LEFT JOIN cfd cfdv ON v.referencia=cfdv.referencia AND cfdv.tipocomprobante='VENT' \
		WHERE ncred.cancelado = 0 AND ncred.fechanot BETWEEN '%s' AND '%s' %s %s %s ",
		fechaIni, fechaFin, condicion_empresa, condicion_sucursal, condicion_con_XML);

	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),Respuesta->TamBufferResultado);
}

//ID_CON_FACTURAS_NOTAS_DCREDITO_PRO
void ServidorAdminSistema::ConsultaFacturasNCP(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
	AnsiString fechaIni, fechaFin, empresa, sucursal, conXML;
	AnsiString condicion_empresa = " ", condicion_sucursal = " ", condicion_con_XML = " ";

	fechaIni = mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
	fechaFin = mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
    empresa = mFg.ExtraeStringDeBuffer(&parametros);
	sucursal = mFg.ExtraeStringDeBuffer(&parametros);
	conXML = mFg.ExtraeStringDeBuffer(&parametros);

	if(empresa != " ")
		condicion_empresa.sprintf(" AND suc.idempresa = %s ", empresa);

	if(sucursal !=" ")
		condicion_sucursal.sprintf(" AND sec.sucursal = '%s' ", sucursal);

	if(conXML == "1")
		condicion_con_XML = " AND cfdi.xml <> ''  ";
	else if(conXML == "0")
		condicion_con_XML = " AND (cfdi.xml = '' OR cfdi.xml IS NULL) ";

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_REP_GRANDE);

	instruccion.sprintf("\
		SELECT n.fechanot as fecha, n.referencia, n.muuid, n.folioprov as nota, cfdi.xml, sec.sucursal, \
		if(n.tipo=0, 'DEVOL', if(n.tipo=1, 'BONIF', 'DESCU')) as tipodescrito, \
		c.muuid as uuidcom, c.folioprov as factura, \
		prov.proveedor, prov.razonsocial \
		FROM notascredprov n \
		LEFT JOIN cfdincreprov cfdi ON n.referencia=cfdi.referencia \
		INNER JOIN terminales t ON t.terminal=n.terminal \
		INNER JOIN secciones sec ON t.seccion=sec.seccion \
		INNER JOIN sucursales suc ON suc.sucursal = sec.sucursal \
		INNER JOIN compras c ON c.referencia=n.compra \
		INNER JOIN proveedores prov ON prov.proveedor=c.proveedor \
		WHERE n.cancelado = 0 AND n.fechanot BETWEEN '%s' AND '%s' %s %s %s ",
		fechaIni, fechaFin, condicion_empresa, condicion_sucursal, condicion_con_XML);

	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),Respuesta->TamBufferResultado);
}



//ID_CON_FACTURAS_VENTAS
void ServidorAdminSistema::ConsultaFacturasVentas(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    AnsiString instruccion;
	AnsiString fechaIni, fechaFin, empresa, sucursal, conXML;
	AnsiString condicion_empresa = " ", condicion_sucursal = " ", condicion_con_XML = " ";

    fechaIni = mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
	fechaFin = mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
    empresa = mFg.ExtraeStringDeBuffer(&parametros);
	sucursal = mFg.ExtraeStringDeBuffer(&parametros);
    conXML = mFg.ExtraeStringDeBuffer(&parametros);

	if(empresa !=" ")
		condicion_empresa.sprintf(" AND suc.idempresa = %s ", empresa);

	if(sucursal !=" ")
		condicion_sucursal.sprintf(" AND c.sucursal = '%s' ", sucursal);

    if(conXML == "1")
		condicion_con_XML = " AND cfdxml.xmlgenerado <> '' ";
	else if(conXML == "0")
		condicion_con_XML = " AND cfdxml.xmlgenerado = '' ";

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_REP_GRANDE);

	instruccion.sprintf("\
		SELECT c.compfiscal, cfdxml.xmlgenerado, c.tipocomprobante, cfdxml.cadenaoriginal, c.referencia, c.ticksub0, \
		c.tickdesglose2014, c.ticksubiva0, c.ticksubiva0ieps, c.ticksubiva16, c.ticksubiva16ieps, \
		c.version, c.muuid, c.sucursal, c.serie, c.folio, c.seriefolio AS foliocfd, \
		if(cli.tipoempre=0,concat(cli.appat,' ',cli.apmat,' ',cli.nombre),cli.rsocial) as nomcliente, cli.cliente, \
		v.fechavta AS fecha, v.horavta AS hora \
		FROM ventas v \
		INNER JOIN cfd c ON v.referencia = c.referencia AND c.tipocomprobante='VENT' \
		INNER join cfdxml on c.compfiscal=cfdxml.compfiscal  \
        INNER JOIN sucursales suc ON suc.sucursal = c.sucursal \
		LEFT JOIN clientes cli ON cli.cliente = v.cliente \
		WHERE v.cancelado = 0 AND v.fechavta BETWEEN '%s' AND '%s' %s %s ",
		fechaIni, fechaFin, condicion_sucursal, condicion_con_XML);

	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),Respuesta->TamBufferResultado);
}
//---------------------------------------------------------------------------
// ID_DIF_VENTAS_MENSUALES
void ServidorAdminSistema::DiferenciasVentasMensuales(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*1024*64];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instruccion, instrucciones[30];
	int num_instrucciones=0;
	int i;
	TDate fecha_hoy=Today();
	int anio_actual=YearOf(fecha_hoy);
	int anio_anterior=anio_actual-1;
	int anio_ante_anterior=anio_actual-2;
	AnsiString cad_anio_actual=mFg.IntToAnsiString(anio_actual);
	AnsiString cad_anio_anterior=mFg.IntToAnsiString(anio_anterior);
	AnsiString cad_anio_ante_anterior=mFg.IntToAnsiString(anio_ante_anterior);
	AnsiString cad_fecha_hoy=mFg.DateToMySqlDate(fecha_hoy);
	//AnsiString cad_fecha_inicio=cad_anio_anterior+"-01-01";
	AnsiString cad_fecha_inicio=cad_anio_ante_anterior+"-01-01";
	AnsiString archivo_temp;

	AnsiString sucursal;
	AnsiString condicion_sucursal=" ";

	sucursal = mFg.ExtraeStringDeBuffer(&parametros);

	if(sucursal!=" "){
        condicion_sucursal.sprintf(" AND sec.sucursal = '%s' ", sucursal);
	}

	try {

		instruccion.sprintf("CREATE TEMPORARY TABLE auxventasxmes ( \
		almacen varchar(4), producto varchar(8), present varchar(255), \
		cant1 decimal(12,3), cant2 decimal(12,3), cant3 decimal(12,3), \
		cant4 decimal(12,3), cant5 decimal(12,3), cant6 decimal(12,3), \
		cant7 decimal(12,3), cant8 decimal(12,3), cant9 decimal(12,3), \
		cant10 decimal(12,3), cant11 decimal(12,3), cant12 decimal(12,3), \
		cant13 decimal(12,3), cant14 decimal(12,3), cant15 decimal(12,3), \
		cant16 decimal(12,3), cant17 decimal(12,3), cant18 decimal(12,3), \
		cant19 decimal(12,3), cant20 decimal(12,3), cant21 decimal(12,3), \
		cant22 decimal(12,3), cant23 decimal(12,3), cant24 decimal(12,3), \
		cant25 decimal(12,3), cant26 decimal(12,3), cant27 decimal(12,3), \
		cant28 decimal(12,3), cant29 decimal(12,3), cant30 decimal(12,3), \
		cant31 decimal(12,3), cant32 decimal(12,3), cant33 decimal(12,3), \
		cant34 decimal(12,3), cant35 decimal(12,3), cant36 decimal(12,3), \
		ventas30 decimal(12,3), ventas60 decimal(12,3), \
		ventas90 decimal(12,3), ventas180 decimal(12,3), \
		ventascorte decimal(12,3), \
		PRIMARY KEY (almacen,producto,present)) Engine = INNODB");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("select @fechacorte:=ifnull(max(p.fecha),'1900-01-01') as fcorte \
		from puntoscorte p where p.fecha<='%s'", mFg.DateToMySqlDate(fecha_hoy));
		instrucciones[num_instrucciones++]=instruccion;

		archivo_temp=mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);

		instruccion.sprintf(
			"SELECT almacen, producto, present, \
				 SUM(cant1) AS cant1, \
				 SUM(cant2) AS cant2, \
				 SUM(cant3) AS cant3, \
				 SUM(cant4) AS cant4, \
				 SUM(cant5) AS cant5, \
				 SUM(cant6) AS cant6, \
				 SUM(cant7) AS cant7, \
				 SUM(cant8) AS cant8, \
				 SUM(cant9) AS cant9, \
				 SUM(cant10) AS cant10, \
				 SUM(cant11) AS cant11, \
				 SUM(cant12) AS cant12, \
				 SUM(cant13) AS cant13, \
				 SUM(cant14) AS cant14, \
				 SUM(cant15) AS cant15, \
				 SUM(cant16) AS cant16, \
				 SUM(cant17) AS cant17, \
				 SUM(cant18) AS cant18, \
				 SUM(cant19) AS cant19, \
				 SUM(cant20) AS cant20, \
				 SUM(cant21) AS cant21, \
				 SUM(cant22) AS cant22, \
				 SUM(cant23) AS cant23, \
				 SUM(cant24) AS cant24, \
				 SUM(cant25) AS cant25, \
				 SUM(cant26) AS cant26, \
				 SUM(cant27) AS cant27, \
				 SUM(cant28) AS cant28, \
				 SUM(cant29) AS cant29, \
				 SUM(cant30) AS cant30, \
				 SUM(cant31) AS cant31, \
				 SUM(cant32) AS cant32, \
				 SUM(cant33) AS cant33, \
				 SUM(cant34) AS cant34, \
				 SUM(cant35) AS cant35, \
				 SUM(cant36) AS cant36, \
				 SUM(ventas30) AS ventas30, \
				 SUM(ventas60) AS ventas60, \
				 SUM(ventas90) AS ventas90, \
				 SUM(ventas180) AS ventas180, \
				 SUM(ventascorte) AS ventascorte \
			FROM \
			((SELECT al.almacen AS almacen, a.producto, a.present, \
				0.000 AS cant1, 0.000 AS cant2, 0.000 AS cant3, 0.000 AS cant4, 0.000  AS cant5, 0.000  AS cant6, \
				0.000 AS cant7, 0.000 AS cant8, 0.000 AS cant9, 0.000 AS cant10, 0.000  AS cant11, 0.000  AS cant12, \
				0.000  AS cant13, 0.000  AS cant14, 0.000  AS cant15, 0.000  AS cant16, 0.000  AS cant17, 0.000 AS cant18, \
				0.000 AS cant19, 0.000  AS cant20, 0.000  AS cant21, 0.000  AS cant22, 0.000  AS cant23, 0.000  AS cant24, \
				0.000  AS cant25, 0.000 AS cant26, 0.000  AS cant27, 0.000 AS cant28, 0.000  AS cant29, 0.000  AS cant30,  \
				0.000  AS cant31, 0.000  AS cant32, 0.000  AS cant33, 0.000  AS cant34,	0.000  AS cant35,0.000 AS cant36, \
				0.000  AS ventas30, 0.000  AS ventas60, 0.000  AS ventas90, 0.000  AS ventas180, 0.000  AS ventascorte  \
			FROM articulos a INNER JOIN presentaciones p ON p.producto = a.producto AND p.present = a.present			\
			INNER JOIN almacenes al INNER JOIN secciones sec ON sec.seccion = al.seccion \
            WHERE 1 %s \
			GROUP BY a.producto, a.present, al.almacen)    \
			UNION ALL \
			 (SELECT dv.almacen AS almacen, a.producto, a.present, \
				SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=1, dv.cantidad * a.factor, 0)) AS cant1, \
				SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=2, dv.cantidad * a.factor, 0)) AS cant2,  \
				SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=3, dv.cantidad * a.factor, 0)) AS cant3, \
				SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=4, dv.cantidad * a.factor, 0)) AS cant4, \
				SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=5, dv.cantidad * a.factor, 0)) AS cant5, \
				SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=6, dv.cantidad * a.factor, 0)) AS cant6, \
				SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=7, dv.cantidad * a.factor, 0)) AS cant7, \
				SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=8, dv.cantidad * a.factor, 0)) AS cant8, \
				SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=9, dv.cantidad * a.factor, 0)) AS cant9, \
				SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=10, dv.cantidad * a.factor, 0)) AS cant10,\
				SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=11, dv.cantidad * a.factor, 0)) AS cant11,\
				SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=12, dv.cantidad * a.factor, 0)) AS cant12,\
				SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=1, dv.cantidad * a.factor, 0)) AS cant13, \
				SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=2, dv.cantidad * a.factor, 0)) AS cant14, \
				SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=3, dv.cantidad * a.factor, 0)) AS cant15, \
				SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=4, dv.cantidad * a.factor, 0)) AS cant16, \
				SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=5, dv.cantidad * a.factor, 0)) AS cant17, \
				SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=6, dv.cantidad * a.factor, 0)) AS cant18, \
				SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=7, dv.cantidad * a.factor, 0)) AS cant19, \
				SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=8, dv.cantidad * a.factor, 0)) AS cant20, \
				SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=9, dv.cantidad * a.factor, 0)) AS cant21, \
				SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=10, dv.cantidad * a.factor, 0)) AS cant22,\
				SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=11, dv.cantidad * a.factor, 0)) AS cant23,\
				SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=12, dv.cantidad * a.factor, 0)) AS cant24,\
				SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=1, dv.cantidad * a.factor, 0)) AS cant25, \
				SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=2, dv.cantidad * a.factor, 0)) AS cant26, \
				SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=3, dv.cantidad * a.factor, 0)) AS cant27, \
				SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=4, dv.cantidad * a.factor, 0)) AS cant28, \
				SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=5, dv.cantidad * a.factor, 0)) AS cant29, \
				SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=6, dv.cantidad * a.factor, 0)) AS cant30, \
				SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=7, dv.cantidad * a.factor, 0)) AS cant31, \
				SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=8, dv.cantidad * a.factor, 0)) AS cant32, \
				SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=9, dv.cantidad * a.factor, 0)) AS cant33, \
				SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=10, dv.cantidad * a.factor, 0)) AS cant34,\
				SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=11, dv.cantidad * a.factor, 0)) AS cant35,\
				SUM(IF(YEAR(v.fechavta)=%s AND MONTH(fechavta)=12, dv.cantidad * a.factor, 0)) AS cant36,\
				SUM(IF(v.fechavta>=DATE_SUB('%s', INTERVAL 30 DAY) AND v.fechavta<='%s', dv.cantidad * a.factor, 0)) AS ventas30, \
				SUM(IF(v.fechavta>=DATE_SUB('%s', INTERVAL 60 DAY) AND v.fechavta<='%s', dv.cantidad * a.factor, 0)) AS ventas60, \
				SUM(IF(v.fechavta>=DATE_SUB('%s', INTERVAL 90 DAY) AND v.fechavta<='%s', dv.cantidad * a.factor, 0)) AS ventas90, \
				SUM(IF(v.fechavta>=DATE_SUB('%s', INTERVAL 180 DAY) AND v.fechavta<='%s', dv.cantidad * a.factor, 0)) AS ventas180, \
				SUM(IF(v.fechavta>@fechacorte AND v.fechavta<='%s', dv.cantidad * a.factor, 0)) AS ventascorte \
			  FROM ventas v FORCE INDEX(fechavta) \
				INNER JOIN dventas dv ON v.referencia = dv.referencia \
				INNER JOIN articulos a ON dv.articulo=a.articulo \
				INNER JOIN terminales t ON t.terminal = v.terminal \
				INNER JOIN secciones sec ON sec.seccion = t.seccion \
			  WHERE v.fechavta between '%s' AND '%s' %s \
				AND v.cancelado = 0 \
				GROUP BY dv.almacen, a.producto, a.present) \
			UNION ALL \
			(SELECT dv.almacen AS almacen, a.producto, a.present, \
				SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=1, dn.cantidad * a.factor*-1, 0)) AS cant1,\
				SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=2, dn.cantidad * a.factor*-1, 0)) AS cant2,\
				SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=3, dn.cantidad * a.factor*-1, 0)) AS cant3,\
				SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=4, dn.cantidad * a.factor*-1, 0)) AS cant4,\
				SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=5, dn.cantidad * a.factor*-1, 0)) AS cant5,\
				SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=6, dn.cantidad * a.factor*-1, 0)) AS cant6,\
				SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=7, dn.cantidad * a.factor*-1, 0)) AS cant7,\
				SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=8, dn.cantidad * a.factor*-1, 0)) AS cant8,\
				SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=9, dn.cantidad * a.factor*-1, 0)) AS cant9, \
				SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=10, dn.cantidad * a.factor*-1, 0)) AS cant10,\
				SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=11, dn.cantidad * a.factor*-1, 0)) AS cant11, \
				SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=12, dn.cantidad * a.factor*-1, 0)) AS cant12,  \
				SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=1, dn.cantidad * a.factor*-1, 0)) AS cant13, \
				SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=2, dn.cantidad * a.factor*-1, 0)) AS cant14, \
				SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=3, dn.cantidad * a.factor*-1, 0)) AS cant15, \
				SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=4, dn.cantidad * a.factor*-1, 0)) AS cant16, \
				SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=5, dn.cantidad * a.factor*-1, 0)) AS cant17, \
				SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=6, dn.cantidad * a.factor*-1, 0)) AS cant18, \
				SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=7, dn.cantidad * a.factor*-1, 0)) AS cant19, \
				SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=8, dn.cantidad * a.factor*-1, 0)) AS cant20, \
				SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=9, dn.cantidad * a.factor*-1, 0)) AS cant21, \
				SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=10, dn.cantidad * a.factor*-1, 0)) AS cant22,\
				SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=11, dn.cantidad * a.factor*-1, 0)) AS cant23,\
				SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=12, dn.cantidad * a.factor*-1, 0)) AS cant24,\
				SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=1, dn.cantidad * a.factor*-1, 0)) AS cant25, \
				SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=2, dn.cantidad * a.factor*-1, 0)) AS cant26, \
				SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=3, dn.cantidad * a.factor*-1, 0)) AS cant27, \
				SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=4, dn.cantidad * a.factor*-1, 0)) AS cant28, \
				SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=5, dn.cantidad * a.factor*-1, 0)) AS cant29, \
				SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=6, dn.cantidad * a.factor*-1, 0)) AS cant30, \
				SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=7, dn.cantidad * a.factor*-1, 0)) AS cant31, \
				SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=8, dn.cantidad * a.factor*-1, 0)) AS cant32, \
				SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=9, dn.cantidad * a.factor*-1, 0)) AS cant33, \
				SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=10, dn.cantidad * a.factor*-1, 0)) AS cant34,\
				SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=11, dn.cantidad * a.factor*-1, 0)) AS cant35,\
				SUM(IF(YEAR(n.fechanot)=%s AND MONTH(n.fechanot)=12, dn.cantidad * a.factor*-1, 0)) AS cant36,\
				SUM(IF(n.fechanot>=DATE_SUB('%s', INTERVAL 30 DAY) AND n.fechanot<='%s', dn.cantidad * a.factor*-1, 0)) AS ventas30, \
				SUM(IF(n.fechanot>=DATE_SUB('%s', INTERVAL 60 DAY) AND n.fechanot<='%s', dn.cantidad * a.factor*-1, 0)) AS ventas60, \
				SUM(IF(n.fechanot>=DATE_SUB('%s', INTERVAL 90 DAY) AND n.fechanot<='%s', dn.cantidad * a.factor*-1, 0)) AS ventas90, \
				SUM(IF(n.fechanot>=DATE_SUB('%s', INTERVAL 180 DAY) AND n.fechanot<='%s', dn.cantidad * a.factor*-1, 0)) AS ventas180, \
				0 AS ventascorte \
			  FROM notascredcli n \
				INNER JOIN dnotascredcli dn ON  n.referencia = dn.referencia \
				INNER JOIN ventas v ON  n.venta = v.referencia \
				INNER JOIN dventas dv ON v.referencia = dv.referencia AND dv.articulo=dn.articulo \
				INNER JOIN articulos a ON dv.articulo=a.articulo \
				INNER JOIN terminales t ON t.terminal = n.terminal \
				INNER JOIN secciones sec ON sec.seccion = t.seccion \
			  WHERE n.fechanot between '%s' AND '%s' %s \
				AND n.cancelado = 0 AND n.tipo = '0' \
				GROUP BY dv.almacen, a.producto, a.present )) t \
			GROUP BY almacen, producto, present INTO OUTFILE '%s' ",
				condicion_sucursal,
				cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,
				cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,
				cad_anio_anterior,cad_anio_anterior, cad_anio_anterior, cad_anio_anterior, cad_anio_anterior, cad_anio_anterior,
				cad_anio_anterior,cad_anio_anterior, cad_anio_anterior, cad_anio_anterior, cad_anio_anterior, cad_anio_anterior,
				cad_anio_actual, cad_anio_actual,cad_anio_actual,cad_anio_actual,cad_anio_actual,cad_anio_actual,
				cad_anio_actual, cad_anio_actual,cad_anio_actual,cad_anio_actual,cad_anio_actual,cad_anio_actual,
				cad_fecha_hoy,cad_fecha_hoy,cad_fecha_hoy,cad_fecha_hoy,cad_fecha_hoy,cad_fecha_hoy,
				cad_fecha_hoy, cad_fecha_hoy, cad_fecha_hoy,
				cad_fecha_inicio,
				cad_fecha_hoy,
                condicion_sucursal,
				cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,
				cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,cad_anio_ante_anterior,
				cad_anio_anterior, cad_anio_anterior, cad_anio_anterior, cad_anio_anterior, cad_anio_anterior, cad_anio_anterior,
				cad_anio_anterior, cad_anio_anterior, cad_anio_anterior, cad_anio_anterior, cad_anio_anterior, cad_anio_anterior,
				cad_anio_actual, cad_anio_actual,cad_anio_actual,cad_anio_actual,cad_anio_actual,cad_anio_actual,
				cad_anio_actual, cad_anio_actual,cad_anio_actual,cad_anio_actual,cad_anio_actual,cad_anio_actual,
				cad_fecha_hoy, cad_fecha_hoy,cad_fecha_hoy, cad_fecha_hoy,cad_fecha_hoy,
				cad_fecha_hoy,cad_fecha_hoy, cad_fecha_hoy,
				cad_fecha_inicio,
				cad_fecha_hoy,
				condicion_sucursal,
				archivo_temp );
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE auxventasxmes (almacen,producto,present, \
				 cant1,cant2,cant3,cant4,cant5, cant6,cant7,cant8,cant9,cant10, \
				 cant11,cant12,cant13,cant14,cant15, cant16,cant17,cant18,cant19, \
				 cant20, cant21,cant22,cant23,cant24,  \
				 cant25,cant26,cant27, cant28,cant29,cant30,cant31, cant32, cant33,cant34,cant35,cant36, \
				 ventas30, ventas60, ventas90, ventas180,ventascorte) ",archivo_temp);
		instrucciones[num_instrucciones++]=instruccion;

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_REP_GRANDE);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {
			instruccion.sprintf("SELECT \
				t.almacen, \
				t.producto, \
				t.present, \
				SUM(t.cant1c) AS cant1c, \
				SUM(t.cant1) AS cant1, \
				SUM(t.cant2c) AS cant2c, \
				SUM(t.cant2) AS cant2, \
				SUM(t.cant3c) AS cant3c, \
				SUM(t.cant3) AS cant3, \
				SUM(t.cant4c) AS cant4c, \
				SUM(t.cant4) AS cant4, \
				SUM(t.cant5c) AS cant5c, \
				SUM(t.cant5) AS cant5, \
				SUM(t.cant6c) AS cant6c, \
				SUM(t.cant6) AS cant6, \
				SUM(t.cant7c) AS cant7c, \
				SUM(t.cant7) AS cant7, \
				SUM(t.cant8c) AS cant8c, \
				SUM(t.cant8) AS cant8, \
				SUM(t.cant9c) AS cant9c, \
				SUM(t.cant9) AS cant9, \
				SUM(t.cant10c) AS cant10c, \
				SUM(t.cant10) AS cant10, \
				SUM(t.cant11c) AS cant11c, \
				SUM(t.cant11) AS cant11, \
				SUM(t.cant12c) AS cant12c, \
				SUM(t.cant12) AS cant12, \
				SUM(t.cant13c) AS cant13c, \
				SUM(t.cant13) AS cant13, \
				SUM(t.cant14c) AS cant14c, \
				SUM(t.cant14) AS cant14, \
				SUM(t.cant15c) AS cant15c, \
				SUM(t.cant15) AS cant15, \
				SUM(t.cant16c) AS cant16c, \
				SUM(t.cant16) AS cant16, \
				SUM(t.cant17c) AS cant17c, \
				SUM(t.cant17) AS cant17, \
				SUM(t.cant18c) AS cant18c, \
				SUM(t.cant18) AS cant18, \
				SUM(t.cant19c) AS cant19c, \
				SUM(t.cant19) AS cant19, \
				SUM(t.cant20c) AS cant20c, \
				SUM(t.cant20) AS cant20, \
				SUM(t.cant21c) AS cant21c, \
				SUM(t.cant21) AS cant21, \
				SUM(t.cant22c) AS cant22c, \
				SUM(t.cant22) AS cant22, \
				SUM(t.cant23c) AS cant23c, \
				SUM(t.cant23) AS cant23, \
				SUM(t.cant24c) AS cant24c, \
				SUM(t.cant24) AS cant24, \
				SUM(t.cant25c) AS cant25c, \
				SUM(t.cant25) AS cant25, \
				SUM(t.cant26c) AS cant26c, \
				SUM(t.cant26) AS cant26, \
				SUM(t.cant27c) AS cant27c, \
				SUM(t.cant27) AS cant27, \
				SUM(t.cant28c) AS cant28c, \
				SUM(t.cant28) AS cant28, \
				SUM(t.cant29c) AS cant29c, \
				SUM(t.cant29) AS cant29, \
				SUM(t.cant30c) AS cant30c, \
				SUM(t.cant30) AS cant30, \
				SUM(t.cant31c) AS cant31c, \
				SUM(t.cant31) AS cant31, \
				SUM(t.cant32c) AS cant32c, \
				SUM(t.cant32) AS cant32, \
				SUM(t.cant33c) AS cant33c, \
				SUM(t.cant33) AS cant33, \
				SUM(t.cant34c) AS cant34c, \
				SUM(t.cant34) AS cant34, \
				SUM(t.cant35c) AS cant35c, \
				SUM(t.cant35) AS cant35, \
				SUM(t.cant36c) AS cant36c, \
				SUM(t.cant36) AS cant36, \
				SUM(t.ventas30) AS ventas30, \
				SUM(t.ventas30c) AS ventas30c, \
				SUM(t.ventas60) AS ventas60, \
				SUM(t.ventas60c) AS ventas60c, \
				SUM(t.ventas90) AS ventas90, \
				SUM(t.ventas90c) AS ventas90c, \
				SUM(t.ventas180) AS ventas180, \
				SUM(t.ventas180c) AS ventas180c, \
				SUM(t.ventascorte) AS ventascorte, \
				SUM(t.ventascortec) AS ventascortec \
			FROM (( \
				SELECT \
					almacen, \
					producto, \
					present, \
					cant1, \
					0 AS cant1c, \
					cant2, \
					0 AS cant2c, \
					cant3, \
					0 AS cant3c, \
					cant4, \
					0 AS cant4c, \
					cant5, \
					0 AS cant5c, \
					cant6, \
					0 AS cant6c, \
					cant7, \
					0 AS cant7c, \
					cant8, \
					0 AS cant8c, \
					cant9, \
					0 AS cant9c, \
					cant10, \
					0 AS cant10c, \
					cant11, \
					0 AS cant11c, \
					cant12, \
					0 AS cant12c, \
					cant13, \
					0 AS cant13c, \
					cant14, \
					0 AS cant14c, \
					cant15, \
					0 AS cant15c, \
					cant16, \
					0 AS cant16c, \
					cant17, \
					0 AS cant17c, \
					cant18, \
					0 AS cant18c, \
					cant19, \
					0 AS cant19c, \
					cant20, \
					0 AS cant20c, \
					cant21, \
					0 AS cant21c, \
					cant22, \
					0 AS cant22c, \
					cant23, \
					0 AS cant23c, \
					cant24, \
					0 AS cant24c, \
					cant25, \
					0 AS cant25c, \
					cant26, \
					0 AS cant26c, \
					cant27, \
					0 AS cant27c, \
					cant28, \
					0 AS cant28c, \
					cant29, \
					0 AS cant29c, \
					cant30, \
					0 AS cant30c, \
					cant31, \
					0 AS cant31c, \
					cant32, \
					0 AS cant32c, \
					cant33, \
					0 AS cant33c, \
					cant34, \
					0 AS cant34c, \
					cant35, \
					0 AS cant35c, \
					cant36, \
					0 AS cant36c, \
					ventas30, \
					0 AS ventas30c, \
					ventas60, \
					0 AS ventas60c, \
					ventas90, \
					0 AS ventas90c, \
					ventas180, \
					0 AS ventas180c, \
					ventascorte, \
					0 AS ventascortec \
				FROM auxventasxmes \
			) UNION ALL ( \
				SELECT \
					vxm.almacen, \
					vxm.producto, \
					vxm.present, \
					0 AS cant1, \
					vxm.cant1 AS cant1c, \
					0 AS cant2, \
					vxm.cant2 AS cant2c, \
					0 AS cant3, \
					vxm.cant3 AS cant3c, \
					0 AS cant4, \
					vxm.cant4 AS cant4c, \
					0 AS cant5, \
					vxm.cant5 AS cant5c, \
					0 AS cant6, \
					vxm.cant6 AS cant6c, \
					0 AS cant7, \
					vxm.cant7 AS cant7c, \
					0 AS cant8, \
					vxm.cant8 AS cant8c, \
					0 AS cant9, \
					vxm.cant9 AS cant9c, \
					0 AS cant10, \
					vxm.cant10 AS cant10c, \
					0 AS cant11, \
					vxm.cant11 AS cant11c, \
					0 AS cant12, \
					vxm.cant12 AS cant12c, \
					0 AS cant13, \
					vxm.cant13 AS cant13c, \
					0 AS cant14, \
					vxm.cant14 AS cant14c, \
					0 AS cant15, \
					vxm.cant15 AS cant15c, \
					0 AS cant16, \
					vxm.cant16 AS cant16c, \
					0 AS cant17, \
					vxm.cant17 AS cant17c, \
					0 AS cant18, \
					vxm.cant18 AS cant18c, \
					0 AS cant19, \
					vxm.cant19 AS cant19c, \
					0 AS cant20, \
					vxm.cant20 AS cant20c, \
					0 AS cant21, \
					vxm.cant21 AS cant21c, \
					0 AS cant22, \
					vxm.cant22 AS cant22c, \
					0 AS cant23, \
					vxm.cant23 AS cant23c, \
					0 AS cant24, \
					vxm.cant24 AS cant24c, \
					0 AS cant25, \
					vxm.cant25 AS cant25c, \
					0 AS cant26, \
					vxm.cant26 AS cant26c, \
					0 AS cant27, \
					vxm.cant27 AS cant27c, \
					0 AS cant28, \
					vxm.cant28 AS cant28c, \
					0 AS cant29, \
					vxm.cant29 AS cant29c, \
					0 AS cant30, \
					vxm.cant30 AS cant30c, \
					0 AS cant31, \
					vxm.cant31 AS cant31c, \
					0 AS cant32, \
					vxm.cant32 AS cant32c, \
					0 AS cant33, \
					vxm.cant33 AS cant33c, \
					0 AS cant34, \
					vxm.cant34 AS cant34c, \
					0 AS cant35, \
					vxm.cant35 AS cant35c, \
					0 AS cant36, \
					vxm.cant36 AS cant36c, \
					0 AS ventas30, \
					ventas30 AS ventas30c, \
					0 AS ventas60, \
					ventas60 AS ventas60c, \
					0 AS ventas90, \
					ventas90 AS ventas90c, \
					0 AS ventas180, \
					ventas180 AS ventas180c, \
					0 AS ventascorte, \
					ventascorte AS ventascortec \
				FROM ventasxmes vxm \
				INNER JOIN almacenes al ON al.almacen = vxm.almacen \
				INNER JOIN secciones sec ON sec.seccion = al.seccion \
				WHERE 1 %s \
			)) t \
			GROUP BY almacen, producto, present", condicion_sucursal);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}

	} __finally {
		mServidorVioleta->BorraArchivoTemp(archivo_temp);
		delete buffer_sql;
	}
}
//---------------------------------------------------------------------------
// ID_CALC_PREST_MINMAX
void ServidorAdminSistema::CalculaPresentacionesMinMax(RespuestaServidor
		*Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*20];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instruccion, instrucciones[25];
	int num_instrucciones=0;
	AnsiString SucursalOrigen;
	SucursalOrigen=FormServidor->ObtieneClaveSucursal();

	try{
		instruccion.sprintf("INSERT INTO bitacoraservidor VALUES (NULL,'%s',NOW(),\
			'INICIO_ID_CALC_PREST_MINMAX')",SucursalOrigen);
		instrucciones[num_instrucciones++]=instruccion;

		instruccion="SET SESSION sql_log_bin = 0";
        instrucciones[num_instrucciones++]=instruccion;
		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		instruccion.sprintf("DELETE FROM presentacionesminmax");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("INSERT INTO presentacionesminmax\
			(producto,present,maxfactor,maxmult,minmult,activo)\
			(SELECT amm.producto, amm.present, amm.maxfactor,\
			amax.multiplo AS maxmult, amin.multiplo AS minmult, 1 AS activo\
			FROM\
			(select a.producto, a.present, MAX(a.factor) AS maxfactor \
			from articulos a WHERE a.activo=1\
			group by a.producto, a.present) amm \
			INNER JOIN articulos amax ON amm.producto=amax.producto\
				AND amm.present=amax.present\
				AND amm.maxfactor=amax.factor \
				AND amax.activo=1 \
			INNER JOIN articulos amin ON amm.producto=amin.producto\
				AND amm.present=amin.present\
				AND amin.factor=1 AND amin.activo=1\
			group by amm.producto, amm.present) \
			UNION ALL\
			(SELECT amm.producto, amm.present, amm.maxfactor,\
			amax.multiplo AS maxmult, amin.multiplo AS minmult, 0 AS activo\
			FROM\
			(select a.producto,\
				a.present,\
				MAX(a.factor)\
				AS maxfactor,\
				MAX(a.activo) AS maxactivo\
			from articulos a \
			group by a.producto, a.present \
			HAVING maxactivo=0 \
			) amm\
			INNER JOIN articulos amax ON amm.producto=amax.producto\
				AND amm.present=amax.present\
				AND amm.maxfactor=amax.factor\
			INNER JOIN articulos amin ON amm.producto=amin.producto\
				AND amm.present=amin.present\
				AND amin.factor=1 \
			group by amm.producto, amm.present)");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("update presentacionesminmax pmm\
			LEFT JOIN presentcajlogisfactor pclf ON pmm.producto=pclf.producto\
			AND pmm.present=pclf.present\
			set pmm.cajalogisticafactor=COALESCE(pclf.cajalogisticafactor, pmm.maxfactor)");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion="COMMIT";
		instrucciones[num_instrucciones++]=instruccion;
		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=1";
		instruccion="SET SESSION sql_log_bin = 1";
        instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("INSERT INTO bitacoraservidor VALUES (NULL,'%s',NOW(),\
			'FIN_ID_CALC_PREST_MINMAX')",SucursalOrigen);
		instrucciones[num_instrucciones++]=instruccion;

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (int i=0; i<num_instrucciones; i++){
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);
		}
		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);
	} __finally{
        delete buffer_sql;
    }
}
//---------------------------------------------------------------------------
// ID_INICIO_CALCULOS_SISTEMA
void ServidorAdminSistema::InicioCalculosSistema(RespuestaServidor
		*Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
	AnsiString oSucursal;

    oSucursal = FormServidor->ObtieneClaveSucursal();

	instruccion.sprintf("INSERT INTO bitacoraservidor VALUES (NULL,'%s',NOW(),\
		'INICIO_CALCULOS_SISTEMA')",oSucursal);
	mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str());
}
//---------------------------------------------------------------------------
// ID_FIN_CALCULOS_SISTEMA
void ServidorAdminSistema::FinCalculosSistema(RespuestaServidor
		*Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
	AnsiString oSucursal;

    oSucursal = FormServidor->ObtieneClaveSucursal();

	instruccion.sprintf("INSERT INTO bitacoraservidor VALUES (NULL,'%s',NOW(),\
		'FIN_CALCULOS_SISTEMA')",oSucursal);
	mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str());
}
//---------------------------------------------------------------------------
// ID_CALC_VENTASCONSALDO
void ServidorAdminSistema::VentasConSaldo(RespuestaServidor
		*Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*20];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instruccion, instrucciones[25],SucursalOrigen;
	AnsiString archivo1;
	int num_instrucciones=0;

	SucursalOrigen=FormServidor->ObtieneClaveSucursal();

	try{

		instruccion.sprintf("INSERT INTO bitacoraservidor VALUES (NULL,'%s',NOW(),\
			'ID_CALC_VENTASCONSALDO')",SucursalOrigen);
		instrucciones[num_instrucciones++]=instruccion;

        instruccion="SET SESSION sql_log_bin = 0";
		instrucciones[num_instrucciones++]=instruccion;

        instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

        instruccion.sprintf("DELETE FROM ventasconsaldo");
		instrucciones[num_instrucciones++]=instruccion;

		archivo1 = mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);

		instruccion.sprintf("SELECT t.referencia, sum(if(t.aplicada=1, t.valor, 0)) AS saldo\
			FROM ventas v\
			INNER JOIN transxcob t ON t.referencia=v.referencia\
			AND t.cancelada=0 AND t.aplicada=1\
			WHERE v.cancelado=0 AND v.acredito=1\
			GROUP BY t.referencia\
			HAVING saldo >= 0.01 \
			INTO OUTFILE '%s'", archivo1);
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("LOAD DATA INFILE '%s' IGNORE INTO table ventasconsaldo", archivo1);
		instrucciones[num_instrucciones++] = instruccion;

		instrucciones[num_instrucciones++]="COMMIT";
		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=1";

		instruccion="SET SESSION sql_log_bin = 1";
		instrucciones[num_instrucciones++]=instruccion;

        instruccion.sprintf("INSERT INTO bitacoraservidor VALUES (NULL,'%s',NOW(),\
			'FIN_ID_CALC_VENTASCONSALDO')",SucursalOrigen);
		instrucciones[num_instrucciones++]=instruccion;



        // Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (int i=0; i<num_instrucciones; i++){
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);
		}
		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);

    } __finally{
		delete buffer_sql;
		if(archivo1 != ""){
			mServidorVioleta->BorraArchivoTemp(archivo1);
		}
	}
}
//---------------------------------------------------------------------------
// ID_CAL_VIGENCIA_PEDIDOS
void ServidorAdminSistema::EjecutaVigenciaPedidos(RespuestaServidor
		*Respuesta, MYSQL *MySQL, char *parametros)
{
    char *buffer_sql=new char[1024*20];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instruccion, instrucciones[25],SucursalOrigen;
	AnsiString fecha;
	int num_instrucciones=0;

	SucursalOrigen=FormServidor->ObtieneClaveSucursal();

	try{

	   instruccion.sprintf("INSERT INTO bitacoraservidor VALUES (NULL,'%s',NOW(),\
			'ID_CAL_VIGENCIA_PEDIDOS')",SucursalOrigen);
	   instrucciones[num_instrucciones++]=instruccion;

	   instruccion.sprintf("UPDATE pedidos p\
			LEFT JOIN pedrecepcion pr ON pr.pedido = p.referencia\
			SET p.cancelado = 1 \
			WHERE p.fechavigencia IS NOT NULL\
			AND p.fechavigencia != '0000-00-00'\
			AND STR_TO_DATE(p.fechavigencia, '%Y-%m-%d') < CURDATE()\
			AND pr.recepcion IS NULL");
	   instrucciones[num_instrucciones++]=instruccion;

       instruccion.sprintf("INSERT INTO bitacoraservidor VALUES (NULL,'%s',NOW(),\
			'FIN_ID_CAL_VIGENCIA_PEDIDOS')",SucursalOrigen);
	   instrucciones[num_instrucciones++]=instruccion;

       aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (int i=0; i<num_instrucciones; i++){
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);
		}
		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);

	}__finally{
        delete buffer_sql;
    }
}
//---------------------------------------------------------------------------
// ID_GRA_EXISTENCIAS_INICIALES
void ServidorAdminSistema::GrabaExistenciasIniciales(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*64*10];
	char *aux_buffer_sql=buffer_sql;
	int i;
	int num_instrucciones=0;
	AnsiString instruccion, instrucciones[1000];
	AnsiString fecha;
	AnsiString archivo_temp1, archivo_temp2, archivo_temp3, archivo_temp4, archivo_temp5;

	AnsiString SucursalOrigen=FormServidor->ObtieneClaveSucursal();
	AnsiString fechaCorte = mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
	AnsiString fechainicial="1900-01-01";

		instruccion.sprintf("INSERT INTO bitacoraservidor VALUES (NULL,'%s',NOW(),'INICIO_ID_GRA_EXISTENCIAS_INICIALES')",SucursalOrigen);
		instrucciones[num_instrucciones++]=instruccion;

		//Verifica si ya existe un corte inicial
        BufferRespuestas* resp_n2 = NULL;
		AnsiString fech, resultEstGlob;
		bool existe;
		instruccion.sprintf("SELECT IFNULL(valor, '2000-01-01') AS val \
			FROM estadosistemaglob \
			WHERE estado = 'FCORTEINI'");
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL,instruccion.c_str(), resp_n2);
		resultEstGlob = resp_n2->ObtieneDato("val");
		if(resultEstGlob == ""){
			resultEstGlob = "1900-01-01";
			existe= false;
		}
		if(resultEstGlob != "2000-01-01"){
			fechainicial = resultEstGlob;
			existe= true;
		}else{
			fechainicial = "1900-01-01";
			existe= false;
		}

		// Crea una tabla donde se pondrán las existencias que se vayan encontrando.
		instruccion="create temporary table existenciastemp ( \
			producto varchar(8), present varchar(255), \
			tipo varchar(2), cantidad decimal(12,3), almacen varchar(4), ventas decimal(12,3), devventas decimal(12,3),  \
			compras decimal(12,3), devcompras decimal(12,3), entradas decimal(12,3), salidas decimal(12,3),  \
			cantinicial decimal(12,3) default '0.000' , INDEX(producto, present)) Engine = InnoDB";
		instrucciones[num_instrucciones++]=instruccion;


		// Calcula base (0) de los artículos en cuestión.
		// La utilidad de esto es simplemente para tomar en cuenta todos los articulos aunque no
		// tengan ningun movimiento
		instruccion="insert into existenciastemp (producto, present, tipo, cantidad, almacen, ventas, devventas \
			, compras, devcompras, entradas, salidas, cantinicial) \
			select a.producto, a.present, 'BA' as tipo, 0.000 as cantidad, al.almacen, 0.000 as ventas, 0.000 as devventas, \
			0.000 as compras, 0.000 as devcompras, 0.000 as  entradas, 0.000 as salidas, 0.000 as cantinicial \
			from articulos a, productos prod , almacenes al\
			where a.producto=prod.producto \
			group by a.producto, a.present, al.almacen";
		instrucciones[num_instrucciones++]=instruccion;

		// Calcula la existencia al momento del corte previo.
		if(existe == true){
			instruccion.sprintf("insert into existenciastemp (producto, present, tipo, cantidad \
			, almacen, ventas, devventas, compras, devcompras, entradas, salidas, cantinicial) \
			select pce.producto, pce.present, 'IN' as tipo, \
			sum(pce.cantidad) as cantidad , pce.almacen, 0.000 as ventas, 0.000 as devventas, \
			0.000 as compras, 0.000 as devcompras, 0.000 as entradas, 0.000 as salidas, SUM(pce.cantidad) as cantinicial \
			from existenciasiniciales pce, productos prod \
			where pce.producto=prod.producto \
			group by pce.producto, pce.present, pce.almacen", fechaCorte);
		instrucciones[num_instrucciones++]=instruccion;
		}

		// Calcula las compras

		archivo_temp1=mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);
	instruccion.sprintf(" \
			SELECT a.producto, a.present, 'CO' as tipo, SUM(d.cantidad*a.factor) AS cantidad, c.almacen \
			, 0.000 as ventas, 0.000 as devventas, \
			SUM(d.cantidad*a.factor) as compras, 0.000 as devcompras, 0.000 as  entradas, 0.000 as salidas \
			, 0.000 as cantinicial \
			FROM compras c \
			INNER JOIN dcompras d ON c.referencia=d.referencia \
			INNER JOIN articulos a ON d.articulo=a.articulo \
			where c.fechacom>'%s' and c.fechacom<='%s' and c.cancelado=0 \
			group by a.producto, a.present, c.almacen INTO OUTFILE '%s'",
			fechainicial, fechaCorte, archivo_temp1);
		instrucciones[num_instrucciones++]=instruccion;
		instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE existenciastemp (producto, present, tipo, cantidad, \
		almacen, ventas, devventas, compras, devcompras, entradas, salidas, cantinicial) ",archivo_temp1);
		instrucciones[num_instrucciones++]=instruccion;

		// Calcula las devoluciones a las compras

		instruccion.sprintf("insert into existenciastemp (producto, present, tipo, cantidad, almacen, \
			ventas, devventas, compras, devcompras, entradas, salidas, cantinicial) \
			select a.producto, a.present, 'DC' as tipo, (sum(d.cantidad*a.factor)*-1) as cantidad, \
			c.almacen, 0.000 as ventas, 0.000 as devventas, \
			0.000 as compras,(sum(d.cantidad*a.factor)) as devcompras, 0.000 as  entradas, 0.000 as salidas \
			, 0.000 as cantinicial \
			from notascredprov n, compras c, dnotascredprov d, articulos a, productos prod \
			where n.fechanot>'%s' and n.fechanot<='%s' and n.tipo='0' \
			and n.referencia=d.referencia and n.compra=c.referencia and \
			n.cancelado=0 and d.articulo=a.articulo and a.producto=prod.producto \
			group by a.producto, a.present,c.almacen",fechainicial, fechaCorte);
		instrucciones[num_instrucciones++]=instruccion;

		// Calcula las ventas.

		archivo_temp2=mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);
		instruccion.sprintf("select a.producto, a.present, 'VE' as tipo, (sum(d.cantidad*a.factor)*-1) as cantidad,d.almacen \
			,(sum(d.cantidad*a.factor)) as ventas, 0.000 as devventas, \
			0.000 as compras, 0.000 as devcompras, 0.000 as  entradas, 0.000 as salidas, 0.000 as cantinicial \
			from ventas v \
			inner join dventas d  on v.referencia=d.referencia \
			inner join articulos a on d.articulo=a.articulo \
			where v.fechavta>'%s' and v.fechavta<='%s' and v.cancelado=0 \
			group by a.producto, a.present,d.almacen INTO OUTFILE '%s'",
			fechainicial, fechaCorte, archivo_temp2);
		instrucciones[num_instrucciones++]=instruccion;
		instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE existenciastemp",archivo_temp2);
		instrucciones[num_instrucciones++]=instruccion;

		// Calcula las devoluciones a las ventas
		archivo_temp3=mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);
		instruccion.sprintf(" \
			select a.producto, a.present, 'DV' as tipo, sum(dn.cantidad*a.factor) as cantidad, dv.almacen \
			, 0.000 as ventas, sum(dn.cantidad*a.factor) as devventas, \
			0.000 as compras, 0.000 as devcompras, 0.000 as  entradas, 0.000 as salidas, 0.000 as cantinicial  \
			from notascredcli n \
			inner join dnotascredcli dn on n.referencia=dn.referencia \
			inner join ventas v on n.venta=v.referencia \
			inner join dventas dv on v.referencia=dv.referencia and dv.articulo=dn.articulo \
			inner join articulos a on dn.articulo=a.articulo \
			where n.fechanot>'%s' and n.fechanot<='%s' and n.tipo='0' and n.cancelado=0 \
			group by a.producto, a.present, dv.almacen INTO OUTFILE '%s'",
			fechainicial, fechaCorte,archivo_temp3);
		instrucciones[num_instrucciones++]=instruccion;
		instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE existenciastemp ",archivo_temp3);
		instrucciones[num_instrucciones++]=instruccion;

		// Calcula las entradas de almacén
		archivo_temp4=mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);
		instruccion.sprintf(" \
			select a.producto, a.present, 'EN' as tipo, sum(d.cantidad*a.factor) as cantidad, m.almaent \
			,0.000 as ventas, 0.000 as devventas, \
			0.000 as compras, 0.000 as devcompras, sum(d.cantidad*a.factor) as  entradas, 0.000 as salidas  \
			, 0.000 as cantinicial \
			from movalma m, dmovalma d, articulos a, productos prod \
			where m.fechamov>'%s' and m.fechamov<='%s' and m.movimiento=d.movimiento \
			and m.tipo<>'S' \
			AND m.aplica=1 and m.cancelado=0 and d.articulo=a.articulo and a.producto=prod.producto \
			group by a.producto, a.present, m.almaent INTO OUTFILE '%s'",
			fechainicial, fechaCorte, archivo_temp4);
		instrucciones[num_instrucciones++]=instruccion;
		instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE existenciastemp ",archivo_temp4);
		instrucciones[num_instrucciones++]=instruccion;

		// Calcula las salidas de almacén
		archivo_temp5=mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id);
		instruccion.sprintf(" \
			select a.producto, a.present, 'SA' as tipo, (sum(d.cantidad*a.factor)*-1) as cantidad, m.almasal  \
			, 0.000 as ventas, 0.000 as devventas, \
			0.000 as compras, 0.000 as devcompras, 0.000 as  entradas,(sum(d.cantidad*a.factor)) as salidas  \
			, 0.000 as cantinicial \
			from movalma m, dmovalma d, articulos a, productos prod \
			where m.fechamov>'%s' and m.fechamov<='%s' and m.movimiento=d.movimiento \
			and m.tipo<>'E' \
			AND m.aplica=1 and m.cancelado=0 and d.articulo=a.articulo and a.producto=prod.producto \
			group by a.producto, a.present, m.almasal  INTO OUTFILE '%s'",
			fechainicial, fechaCorte, archivo_temp5);
		instrucciones[num_instrucciones++]=instruccion;
		instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE existenciastemp ",archivo_temp5);
		instrucciones[num_instrucciones++]=instruccion;

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0"; //camb. lugar
		instrucciones[num_instrucciones++]="START TRANSACTION"; //camb lugar

		instruccion = "DELETE FROM existenciasiniciales";
		instrucciones[num_instrucciones++]=instruccion;

		// Suma los movimientos para obtener las existencias

		instruccion = "insert into existenciasiniciales (producto, present, cantidad, almacen, ventas, devventas \
				, compras, devcompras, entradas, salidas, cantinicial) \
				select e.producto, e.present, sum(e.cantidad) as cantidad, e.almacen, sum(e.ventas),   \
				sum(e.devventas), sum(e.compras), sum(e.devcompras), sum(e.entradas), sum(e.salidas), sum(e.cantinicial)  \
				from existenciastemp e \
				group by e.producto, e.present, e.almacen";
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("UPDATE estadosistemaglob e SET e.valor= '%s' WHERE  e.estado='FCORTEINI'",fechaCorte);
		instrucciones[num_instrucciones++]=instruccion;

		instruccion="COMMIT";
		instrucciones[num_instrucciones++]=instruccion;

		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=1"; //camb lugar ?

		//instruccion="SET SESSION sql_log_bin = 1";
		//instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("INSERT INTO bitacoraservidor VALUES (NULL,'%s',NOW(),'FIN_ID_GRA_EXISTENCIAS_INICIALES')",SucursalOrigen);
		instrucciones[num_instrucciones++]=instruccion;

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_REP);
		if (mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql)) {

			mServidorVioleta->BorraArchivoTemp(archivo_temp1);
			mServidorVioleta->BorraArchivoTemp(archivo_temp2);
			mServidorVioleta->BorraArchivoTemp(archivo_temp3);
			mServidorVioleta->BorraArchivoTemp(archivo_temp4);
			mServidorVioleta->BorraArchivoTemp(archivo_temp5);

		}

}
//---------------------------------------------------------------------------
// ID_CALC_CORTEINICIAL_COSTOS
void ServidorAdminSistema::CalculaCorteInicialCostos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_inventario_inicial=new char[1024*64*10];
	char *aux_buffer_inventario_inicial=NULL;
	char * buffer_cortecostos = new char [100];
	char * puntero_cortecostos = buffer_cortecostos;
	char *resultado_select = NULL;
	TFileStream *FStream1=NULL;

	int NumeroRenglones;
	AnsiString archivo_temp1="",SucursalOrigen;
	AnsiString nomproducto, presentacion, cveproducto, multiplo, articulo, cadena, cantidad_cad, costo_cad, costounit_cad, clavesAlmacenes, valoresAlmacenes;
	double cantidad, costo, costounit;
	AnsiString fecha_corte_costos;

	AnsiString instruccion;

	AnsiString idEmpresa = "";
	BufferRespuestas * respuesta_empresas = NULL;

	SucursalOrigen=FormServidor->ObtieneClaveSucursal();

	vector <AnsiString> archivosTemporales;

	BufferRespuestas * respuesta_corte_anterior = NULL;
	AnsiString fechaInicial = " ";
	AnsiString usarCorte;

	try{
		try {

			instruccion="SET SESSION sql_log_bin = 0";
			mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str());

           	instruccion.sprintf("INSERT INTO bitacoraservidor VALUES (NULL,'%s',NOW(),'INICIO_ID_CALC_CORTEINICIAL_COSTOS')",SucursalOrigen);
			mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str());

			instruccion = "SELECT idempresa FROM EMPRESAS";
			mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), respuesta_empresas);

			//Campos de función servidor
			fecha_corte_costos = mFg.ExtraeStringDeBuffer(&parametros);
			usarCorte = mFg.ExtraeStringDeBuffer(&parametros);

			if(usarCorte == "1"){
				// Fecha del corte anterior
				mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, "SELECT valor FROM estadosistemaglob WHERE estado = 'FCINICOSTOS'", respuesta_corte_anterior);
				if(respuesta_corte_anterior->ObtieneTamRespuesta() > 0){
					fechaInicial = respuesta_corte_anterior->ObtieneDato("valor");
				}
			}

			//Recalcula la tabla precalculocostos%s
			puntero_cortecostos = mFg.AgregaStringABuffer(fechaInicial, puntero_cortecostos); //Fecha Inicial
			puntero_cortecostos = mFg.AgregaStringABuffer(fecha_corte_costos, puntero_cortecostos); //Fecha final
			puntero_cortecostos = mFg.AgregaStringABuffer(usarCorte, puntero_cortecostos); //Usar el último corte

			// Invalida el precalculo de costos anterior
			instruccion.sprintf("UPDATE estadosistemaemp SET valor = '1900-01-01' where estado = 'FPRECCOSTOS' AND sucursal = '%s'",FormServidor->ObtieneClaveSucursal());
			mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str());

			instruccion="SET SESSION sql_log_bin = 0";
			mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str());

			//ID_GRA_CORTE_COSTOS para el corte
			GrabaCorteCostos(Respuesta, MySQL, buffer_cortecostos);

			//Es necesario llenar las tablas de precalculosminmaxfin%s, para ello se llama al precálculo mensual
			//ID_CALC_PRECALCULOCOSTOS_MENSUAL
			PrecalculoCostosMensual(Respuesta, MySQL, NULL);

			instruccion="SET SESSION sql_log_bin = 1";
			mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str());

			//---INICIA CÁLCULO POR EMPRESA---------------------------------------

			for(int h = 0; h < respuesta_empresas->ObtieneNumRegistros(); h++){
				respuesta_empresas->IrAlRegistroNumero(h);

				idEmpresa = respuesta_empresas->ObtieneDato("idempresa");

				//Inventario inicial
				aux_buffer_inventario_inicial=buffer_inventario_inicial;
				aux_buffer_inventario_inicial=mFg.AgregaStringABuffer(fecha_corte_costos, aux_buffer_inventario_inicial); //fecha
				aux_buffer_inventario_inicial=mFg.AgregaStringABuffer(idEmpresa, aux_buffer_inventario_inicial); // Envía la empresa

				// ID_EJE_REPCORTECOSTEXIS
				mServidorVioleta->Reportes->EjecutaRepCorteCostExis(Respuesta,  MySQL, buffer_inventario_inicial);

				resultado_select=Respuesta->BufferResultado;
				resultado_select+=sizeof(int); // Se salta el tamaño del resultado.
				mFg.ExtraeStringDeBuffer(&resultado_select); // Se salta el indicador de error
				resultado_select+=sizeof(int); // Se salta el tamaño del buffer
				NumeroRenglones=*((int *)resultado_select); // Obtiene número de registros
				resultado_select+=sizeof(int); // Se salta el numero de registros
				archivosTemporales.push_back(mServidorVioleta->ObtieneArchivoTemp(1, Respuesta->Id));

				FStream1 = new TFileStream(archivosTemporales.back(), fmCreate | fmShareDenyNone);

				//Llena los datos del inventario inicial

				for(int i=0; i<NumeroRenglones; i++) {
					nomproducto=mFg.ExtraeStringDeBuffer(&resultado_select);
					presentacion=mFg.ExtraeStringDeBuffer(&resultado_select);
					cveproducto=mFg.ExtraeStringDeBuffer(&resultado_select);
					cantidad=mFg.CadenaAFlotante(mFg.ExtraeStringDeBuffer(&resultado_select));
					costo=mFg.CadenaAFlotante(mFg.ExtraeStringDeBuffer(&resultado_select));
					costounit=mFg.CadenaAFlotante(mFg.ExtraeStringDeBuffer(&resultado_select));
					clavesAlmacenes = mFg.ExtraeStringDeBuffer(&resultado_select);
					valoresAlmacenes = mFg.ExtraeStringDeBuffer(&resultado_select);

					cantidad_cad=mFg.FormateaCantidad(cantidad, 3, false);
					costo_cad=mFg.FormateaCantidad(costo, 6, false);
					costounit_cad=mFg.FormateaCantidad(costounit, 6, false);

					TStringDynArray claves(SplitString(clavesAlmacenes, ","));
					TStringDynArray valores(SplitString(valoresAlmacenes, ","));


				   for(int j = 0; j < claves.Length; j++){

						AnsiString almacen = claves[j];
						AnsiString cantidad = valores[j];

						if(almacen == "")
							throw (Exception("Almacén vacío para un registro"));

						//Guardar en archivo temporal
						cadena=cveproducto +"\t"+
							presentacion +"\t"+
							almacen + "\t" +
							cantidad +"\t"+
							costo_cad +"\t"+
							costounit_cad +"\n";

							FStream1->Write(cadena.c_str(), cadena.Length());
					}
				}

				instruccion.sprintf("truncate table costospromedioiniciales%s", idEmpresa);
				if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
					throw (Exception("Error ID_CALC_CORTEINICIAL_COSTOS en TRUNCATE"));

				instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE \
					costospromedioiniciales%s (producto, present, almacen, cantidad, costo, costounit) ",
					archivosTemporales.back(), idEmpresa);
				if (!mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str()))
					throw (Exception("Error ID_CALC_CORTEINICIAL_COSTOS en LOAD DATA"));

				if (FStream1!=NULL) {
					delete FStream1;
				}

			}//---FÍN DEL CÁLCULO POR EMPRESA-------------------------------------

			instruccion.sprintf("UPDATE estadosistemaglob SET valor = '%s' where estado = 'FCINICOSTOS'", fecha_corte_costos);
			mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str());



			instruccion.sprintf("INSERT INTO bitacoraservidor VALUES (NULL,'%s',NOW(),'FIN_ID_CALC_CORTEINICIAL_COSTOS')",SucursalOrigen);
			mServidorVioleta->EjecutaAccionSqlNulo(Respuesta,  MySQL, instruccion.c_str());

		}catch(Exception &e){
			if (FStream1!=NULL) {
				delete FStream1;
			}
		}
	} __finally {
    
		for(int i = 0; i < archivosTemporales.size(); i++)
			mServidorVioleta->BorraArchivoTemp(archivosTemporales.at(i));

		delete buffer_inventario_inicial;
		delete buffer_cortecostos;
//        delete resultado_select;
		delete respuesta_empresas;
		if(respuesta_corte_anterior != NULL)
			delete respuesta_corte_anterior;
	}

}
// ID_CALC_PRESENT_ACTIVAS
void ServidorAdminSistema::CalculaPresentacionesActivas(RespuestaServidor
	*Respuesta, MYSQL *MySQL, char *parametros){

    char *buffer_sql=new char[1024*20];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instruccion, instrucciones[25];
	int num_instrucciones=0;
	AnsiString sucursal;

	try{
		sucursal=FormServidor->ObtieneClaveSucursal();

		instruccion.sprintf("INSERT INTO bitacoraservidor VALUES (NULL,'%s',NOW(),\
			'INICIO_ID_CALC_PRESENT_ACTIVAS')", sucursal);
		instrucciones[num_instrucciones++]=instruccion;

		instruccion="SET SESSION sql_log_bin = 0";
		instrucciones[num_instrucciones++]=instruccion;

		//Sin replicación
		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++]="START TRANSACTION";

		instruccion.sprintf("TRUNCATE TABLE presentacionesactivemp");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("INSERT INTO presentacionesactivemp \
			SELECT e.idempresa, a.producto, a.present, \
			SUM(IFNULL(permitecompras,1)+IFNULL(permiteventas,1)+IFNULL(permitemovalma,1))>0 AS activo \
			FROM articulos a \
			INNER JOIN empresas e \
			LEFT JOIN articuloempresacfg ac ON a.articulo=ac.articulo AND e.idempresa=ac.idempresa \
			GROUP BY a.producto, a.present, e.idempresa");
		instrucciones[num_instrucciones++]=instruccion;


		instrucciones[num_instrucciones++]="COMMIT";
		instrucciones[num_instrucciones++]="SET AUTOCOMMIT=1";

        //Con replicación
		instruccion="SET SESSION sql_log_bin = 1";
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("INSERT INTO bitacoraservidor VALUES (NULL,'%s',NOW(),\
			'FIN_ID_CALC_PRESENT_ACTIVAS')", sucursal);
		instrucciones[num_instrucciones++]=instruccion;

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (int i=0; i<num_instrucciones; i++){
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);
		}
		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);

	}__finally{
        delete buffer_sql;
	}

}
//---------------------------------------------------------------------------
//ID_ASIG_PASSWORD
void ServidorAdminSistema::AsignaPassword(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CAMBIA LA CLAVE DE ACCESO AL SISTEMA DE UN USUARIO DADO
	char *buffer_sql=new char[1024*5];
	char *aux_buffer_sql=buffer_sql;
	AnsiString instruccion, instrucciones[5];
	int num_instrucciones=0;
	int i;
	AnsiString usuario, clave;

	try{
		usuario=mFg.ExtraeStringDeBuffer(&parametros);
		clave=mFg.ExtraeStringDeBuffer(&parametros);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

		// Se asigna una variable el identificador del usurio que coincide con el identificador y el password
		instruccion.sprintf("select @idusuario:=empleado from usuarios where empleado='%s' AND password IS NULL", usuario);
		instrucciones[num_instrucciones++]=instruccion;

		// Si la clave es correcta para el usuario dado, entonces cambia dicha clave por la nueva
		instruccion.sprintf("update usuarios set password='%s' where empleado='%s' AND password IS NULL", clave, usuario);
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
//ID_CLASIFICACION_ARTICULOS
//---------------------------------------------------------------------------
void ServidorAdminSistema::ClasificaArticulos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// CLASIFICA LOS ARTICULOS PARA CADA SUCURSAL EN BASE A SU PARTICIPACION POR PIEZA, MONTO Y COMPUESTA
	char *buffer_sql = new char[1024*64*100];
	char *aux_buffer_sql = buffer_sql;
	AnsiString instruccion, instrucciones[5000];
	int num_instrucciones = 0, contador = 0;
	AnsiString tipo_clasificacion = "";
    vector <AnsiString> archivosTemporales;

	try{
		tipo_clasificacion = mFg.ExtraeStringDeBuffer(&parametros);

		instruccion.sprintf("INSERT INTO bitacoraservidor VALUES (NULL, '%s', NOW(), 'INICIO_CLASIFICACION_ARTICULOS')",
		FormServidor->ObtieneClaveSucursal());
		instrucciones[num_instrucciones++]=instruccion;

        //DESACTIVA LA REPLICACION
		instruccion="SET SESSION sql_log_bin = 0";
		instrucciones[num_instrucciones++]=instruccion;

		//Se establece la fecha que sera considerada para determinar el inicio de las ventas
		instruccion = "SET @dias_para_venta = (SELECT valor FROM parametrosgen WHERE parametro = 'RANGODIASVENT')";
		instrucciones[num_instrucciones++]=instruccion;
        instruccion = "SET @fecha_inicial = (SELECT CURRENT_DATE() - INTERVAL @dias_para_venta DAY)";
		instrucciones[num_instrucciones++]=instruccion;

        //Se crean las tablas temporales implicadas en el proceso
		instruccion.sprintf("CREATE OR REPLACE TEMPORARY TABLE auxfechas_laboradas_sucursales\
		(sucursal VARCHAR(2) NOT NULL,\
		fecha DATE NOT NULL,\
		total_unidades DOUBLE NOT NULL,\
		total_monto DOUBLE NOT NULL,\
		PRIMARY KEY (sucursal, fecha))");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("CREATE OR REPLACE TEMPORARY TABLE auxbase_prod_pres\
		(sucursal VARCHAR(2) NOT NULL,\
		producto VARCHAR(8) NOT NULL,\
		present VARCHAR(255) NOT NULL,\
		unidades DOUBLE NOT NULL,\
		monto DOUBLE NOT NULL,\
		fechadia DATE,\
		PRIMARY KEY (sucursal, producto, present, fechadia))");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("CREATE OR REPLACE TEMPORARY TABLE auxdesglosado_ventasglobal\
		(producto VARCHAR(8),\
		present VARCHAR(255),\
		unidades FLOAT,\
		monto DOUBLE,\
		desglose_id DATE,\
		PRIMARY KEY (producto, present, desglose_id))");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("CREATE OR REPLACE TEMPORARY TABLE auxdesglosado_ventasxempresa\
		(idempresa TINYINT,\
		producto VARCHAR(8),\
		present VARCHAR(255),\
		unidades FLOAT,\
		monto DOUBLE,\
		desglose_id DATE,\
		PRIMARY KEY (idempresa, producto, present, desglose_id))");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("CREATE OR REPLACE TEMPORARY TABLE auxdesglosado_ventasxsucursal\
		(sucursal VARCHAR(2),\
		producto VARCHAR(8),\
		present VARCHAR(255),\
		unidades FLOAT,\
		monto DOUBLE,\
		desglose_id DATE,\
		PRIMARY KEY (sucursal, producto, present, desglose_id))");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("CREATE OR REPLACE TEMPORARY TABLE auxtotales_x_sucursal\
		(sucursal VARCHAR(2) NOT NULL,\
		total_unidades DOUBLE NOT NULL,\
		total_monto DOUBLE NOT NULL,\
		PRIMARY KEY (sucursal))");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("CREATE OR REPLACE TEMPORARY TABLE auxtotales_x_empresa\
		(idempresa TINYINT NOT NULL,\
		total_unidades DOUBLE NOT NULL,\
		total_monto DOUBLE NOT NULL,\
		PRIMARY KEY (idempresa))");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("CREATE OR REPLACE TEMPORARY TABLE auxresumen_participacion_global\
		(producto VARCHAR(8) NOT NULL,\
		present VARCHAR(255) NOT NULL,\
		participacion_acumulada_pieza FLOAT NOT NULL,\
		participacion_acumulada_monto FLOAT NOT NULL,\
		PRIMARY KEY(producto, present))");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("CREATE OR REPLACE TEMPORARY TABLE auxresumen_participacion_empresa\
		(idempresa TINYINT NOT NULL,\
		producto VARCHAR(8) NOT NULL,\
		present VARCHAR(255) NOT NULL,\
		participacion_acumulada_pieza FLOAT NOT NULL,\
		participacion_acumulada_monto FLOAT NOT NULL,\
		PRIMARY KEY(idempresa, producto, present))");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("CREATE OR REPLACE TEMPORARY TABLE auxresumen_participacion_sucursal\
		(sucursal VARCHAR(2) NOT NULL,\
		producto VARCHAR(8) NOT NULL,\
		present VARCHAR(255) NOT NULL,\
		participacion_acumulada_pieza FLOAT NOT NULL,\
		participacion_acumulada_monto FLOAT NOT NULL,\
		PRIMARY KEY(sucursal, producto, present))");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("CREATE OR REPLACE TEMPORARY TABLE auxclasificaciones\
		(clasificacion CHAR(1) NOT NULL,\
		limiteinferior FLOAT NOT NULL,\
		limitesuperior FLOAT NOT NULL)");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("CREATE OR REPLACE TEMPORARY TABLE auxdesglosado_vtapartrela_xsucursal LIKE auxdesglosado_ventasxsucursal");
		instrucciones[num_instrucciones++]=instruccion;

		//Comenzamos a insertar datos a las tablas anteriormente creadas
		instruccion.sprintf("INSERT IGNORE INTO auxclasificaciones\
		(SELECT clasificacion, limiteinferior, limitesuperior - 0.000000001\
		FROM clasificacionpedidosautomaticos)");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("INSERT INTO auxfechas_laboradas_sucursales\
		(SELECT\
			suc.sucursal,\
			v.fechavta,\
			SUM(pvd.unidades),\
			SUM(pvd.subtotal)\
		FROM ventas v FORCE INDEX (fechavta)\
		INNER JOIN precalculoventasdet pvd ON v.referencia = pvd.referencia\
		INNER JOIN terminales t	ON v.terminal = t.terminal\
		INNER JOIN secciones sec ON t.seccion = sec.seccion\
		INNER JOIN sucursales suc ON suc.sucursal = sec.sucursal\
		WHERE v.fechavta BETWEEN @fecha_inicial AND CURRENT_DATE()\
		GROUP BY suc.sucursal, v.fechavta)");
		instrucciones[num_instrucciones++]=instruccion;

        archivosTemporales.push_back(mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id));
		instruccion.sprintf("SELECT fd.sucursal, pmm.producto, pmm.present, 0 AS unidades, 0 AS monto, fd.fecha\
		FROM auxfechas_laboradas_sucursales fd\
		JOIN (SELECT vxm.producto, vxm.present\
		FROM ventasxmes vxm\
		INNER JOIN almacenes alm ON vxm.almacen = alm.almacen\
		INNER JOIN secciones sec ON alm.seccion = sec.seccion\
		INNER JOIN presentacionesminmax pmm ON vxm.producto = pmm.producto AND vxm.present = pmm.present AND pmm.activo = 1\
		GROUP BY vxm.producto, vxm.present) pmm\
		WHERE fecha BETWEEN @fecha_inicial AND CURRENT_DATE()\
		GROUP BY fd.sucursal, pmm.producto, pmm.present, fd.fecha INTO OUTFILE '%s'", archivosTemporales.back());
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE auxbase_prod_pres", archivosTemporales.back());
		instrucciones[num_instrucciones++]=instruccion;

        archivosTemporales.push_back(mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id));
		instruccion.sprintf("SELECT\
			sec.sucursal,\
			pv.producto,\
			pv.present,\
			SUM(pv.unidades) AS unidades,\
			SUM(pv.subtotal) AS monto,\
			v.fechavta AS iddesglose\
		FROM ventas v FORCE INDEX(fechavta)\
		INNER JOIN precalculoventasdet pv\
			ON v.referencia=pv.referencia\
		INNER JOIN terminales t\
			ON v.terminal = t.terminal\
		INNER JOIN secciones sec\
			ON t.seccion = sec.seccion\
		INNER JOIN clientes cli\
			ON cli.cliente = v.cliente	AND cli.esparterelac = 0\
		WHERE v.fechavta BETWEEN @fecha_inicial AND CURRENT_DATE() AND v.cancelado=0\
		GROUP BY sec.sucursal, pv.producto, pv.present, v.fechavta INTO OUTFILE '%s'", archivosTemporales.back());
		instrucciones[num_instrucciones++]=instruccion;
		instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE auxdesglosado_ventasxsucursal", archivosTemporales.back());
		instrucciones[num_instrucciones++]=instruccion;

        archivosTemporales.push_back(mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id));
		instruccion.sprintf("SELECT\
			sec.sucursal,\
			pv.producto,\
			pv.present,\
			SUM(pv.unidades) AS unidades,\
			SUM(pv.subtotal) AS monto,\
			v.fechavta AS iddesglose\
		FROM ventas v FORCE INDEX(fechavta)\
		INNER JOIN precalculoventasdet pv\
			ON v.referencia=pv.referencia\
		INNER JOIN terminales t\
			ON v.terminal = t.terminal\
		INNER JOIN secciones sec\
			ON t.seccion = sec.seccion\
		INNER JOIN clientes cli\
			ON cli.cliente = v.cliente	AND cli.esparterelac = 1\
		WHERE v.fechavta BETWEEN @fecha_inicial AND CURRENT_DATE() AND v.cancelado=0\
		GROUP BY sec.sucursal, pv.producto, pv.present, v.fechavta INTO OUTFILE '%s'", archivosTemporales.back());
		instrucciones[num_instrucciones++]=instruccion;
		instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE auxdesglosado_vtapartrela_xsucursal", archivosTemporales.back());
		instrucciones[num_instrucciones++]=instruccion;

        archivosTemporales.push_back(mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id));
		instruccion.sprintf("SELECT\
			auxdt.sucursal,\
			COALESCE(auxvg.producto, auxdt.producto),\
			COALESCE(auxvg.present, auxdt.present),\
			COALESCE(auxvg.unidades, auxdt.unidades),\
			COALESCE(auxvg.monto, auxdt.monto),\
			COALESCE(auxvg.desglose_id, auxdt.fechadia)\
		FROM auxbase_prod_pres auxdt\
		LEFT JOIN auxdesglosado_ventasxsucursal auxvg\
			ON auxdt.producto = auxvg.producto\
			AND auxdt.present = auxvg.present\
			AND  auxvg.desglose_id = auxdt.fechadia\
			AND auxvg.sucursal = auxdt.sucursal\
		WHERE ISNULL(auxvg.desglose_id) INTO OUTFILE '%s'", archivosTemporales.back());
		instrucciones[num_instrucciones++]=instruccion;
		instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE auxdesglosado_ventasxsucursal", archivosTemporales.back());
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("INSERT INTO auxtotales_x_sucursal\
		(SELECT\
			sucursal,\
			SUM(unidades) AS total_unidades,\
			SUM(monto) AS total_monto\
		FROM auxdesglosado_ventasxsucursal\
		GROUP BY sucursal)");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("INSERT INTO auxdesglosado_ventasglobal\
		(SELECT producto, present, SUM(unidades), SUM(monto), desglose_id\
		FROM auxdesglosado_ventasxsucursal\
		GROUP BY desglose_id, producto, present)");
		instrucciones[num_instrucciones++]=instruccion;

        archivosTemporales.push_back(mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id));
		instruccion.sprintf("SELECT\
			axvs.sucursal,\
			axvs.producto,\
			axvs.present,\
			SUM(axvs.unidades) AS total_unidades,\
			AVG(axvs.unidades) AS promedio_unidades,\
			MIN(axvs.unidades) AS unidades_minimo,\
			MAX(axvs.unidades) AS unidades_maximo,\
			STDDEV_POP(axvs.unidades) AS desviacion_estandar_unidades,\
			COALESCE((SUM(axvs.unidades)/ats.total_unidades) * 100, 0) AS participacion_unidades,\
			SUM(axvs.monto) AS total_monto,\
			COALESCE((SUM(axvs.monto)/ats.total_monto) * 100, 0) AS participacion_monto\
		FROM auxdesglosado_ventasxsucursal axvs\
		INNER JOIN auxtotales_x_sucursal ats ON axvs.sucursal = ats.sucursal\
		GROUP BY axvs.sucursal, axvs.producto, axvs.present INTO OUTFILE '%s'", archivosTemporales.back());
		instrucciones[num_instrucciones++]=instruccion;

        instruccion.sprintf("TRUNCATE TABLE resumenvtas_sucursal_pca");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE resumenvtas_sucursal_pca", archivosTemporales.back());
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("SET @total_unidades_global = (SELECT SUM(unidades) FROM auxdesglosado_ventasglobal)");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("SET @total_monto_global = (SELECT SUM(monto) FROM auxdesglosado_ventasglobal)");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("TRUNCATE TABLE resumen_vtasxart_global");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("INSERT INTO resumen_vtasxart_global(\
		SELECT\
			axvg.producto,\
			axvg.present,\
			SUM(axvg.unidades) AS total_unidades,\
			AVG(axvg.unidades) AS promedio_unidades,\
			MIN(axvg.unidades) AS unidades_minimo,\
			MAX(axvg.unidades) AS unidades_maximo,\
			STDDEV_POP(axvg.unidades) AS desviacion_estandar_unidades,\
			COALESCE((SUM(axvg.unidades)/@total_unidades_global) * 100, 0) AS participacion_unidades,\
			SUM(axvg.monto) AS total_monto,\
			COALESCE((SUM(axvg.monto)/@total_monto_global) * 100, 0) AS participacion_monto\
		FROM auxdesglosado_ventasglobal axvg\
		GROUP BY axvg.producto, axvg.present\
		)", archivosTemporales.back());
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("UPDATE auxdesglosado_ventasxsucursal axs\
		INNER JOIN auxdesglosado_vtapartrela_xsucursal axspr\
		ON axs.sucursal = axspr.sucursal AND axs.producto = axspr.producto\
		AND axs.present = axspr.present AND axs.desglose_id = axspr.desglose_id\
		SET axs.unidades = axs.unidades + axspr.unidades, axs.monto = axs.monto + axspr.monto");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("INSERT INTO auxdesglosado_ventasxempresa(\
		SELECT\
			suc.idempresa,\
			axds.producto,\
			axds.present,\
			SUM(axds.unidades),\
			SUM(axds.monto),\
			axds.desglose_id\
		FROM auxdesglosado_ventasxsucursal axds\
		INNER JOIN sucursales suc ON axds.sucursal = suc.sucursal\
		GROUP BY axds.desglose_id, suc.idempresa, axds.producto, axds.present)");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("TRUNCATE TABLE auxtotales_x_sucursal");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("INSERT INTO auxtotales_x_sucursal\
		(SELECT\
			sucursal,\
			SUM(unidades) AS total_unidades,\
			SUM(monto) AS total_monto\
		FROM auxdesglosado_ventasxsucursal\
		GROUP BY sucursal)");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("TRUNCATE TABLE auxtotales_x_empresa");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("INSERT INTO auxtotales_x_empresa\
		(SELECT axdve.idempresa, SUM(axdve.unidades) AS total_unidades, SUM(axdve.monto) AS total_monto\
		FROM auxdesglosado_ventasxempresa axdve\
		GROUP BY axdve.idempresa)");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("TRUNCATE TABLE resumen_vtasxart_empresas");
		instrucciones[num_instrucciones++]=instruccion;
		instruccion.sprintf("INSERT INTO resumen_vtasxart_empresas(\
		SELECT axve.idempresa, axve.producto, axve.present,\
		SUM(axve.unidades) AS total_unidades,\
		AVG(axve.unidades) AS promedio_unidades,\
		MIN(axve.unidades) AS unidades_minimo,\
		MAX(axve.unidades) AS unidades_maximo,\
		STDDEV_POP(axve.unidades) AS desviacion_estandar_unidades,\
		COALESCE((SUM(axve.unidades)/ate.total_unidades) * 100, 0) AS participacion_unidades,\
		SUM(axve.monto) AS total_monto,\
		COALESCE((SUM(axve.monto)/ate.total_monto) * 100, 0) AS participacion_monto\
		FROM auxdesglosado_ventasxempresa axve\
		INNER JOIN auxtotales_x_empresa ate ON axve.idempresa = ate.idempresa\
		GROUP BY axve.idempresa, axve.producto, axve.present)");
		instrucciones[num_instrucciones++]=instruccion;

		archivosTemporales.push_back(mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id));
		instruccion.sprintf("SELECT\
			axvs.sucursal,\
			axvs.producto,\
			axvs.present,\
			SUM(axvs.unidades) AS total_unidades,\
			AVG(axvs.unidades) AS promedio_unidades,\
			MIN(axvs.unidades) AS unidades_minimo,\
			MAX(axvs.unidades) AS unidades_maximo,\
			STDDEV_POP(axvs.unidades) AS desviacion_estandar_unidades,\
			COALESCE((SUM(axvs.unidades)/ats.total_unidades) * 100, 0) AS participacion_unidades,\
			SUM(axvs.monto) AS total_monto,\
			COALESCE((SUM(axvs.monto)/ats.total_monto) * 100, 0) AS participacion_monto\
		FROM auxdesglosado_ventasxsucursal axvs\
		INNER JOIN auxtotales_x_sucursal ats ON axvs.sucursal = ats.sucursal\
		GROUP BY axvs.sucursal, axvs.producto, axvs.present INTO OUTFILE '%s'", archivosTemporales.back());
		instrucciones[num_instrucciones++]=instruccion;

        instruccion.sprintf("TRUNCATE TABLE resumen_vtasxart_sucursal");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("LOAD DATA INFILE '%s' INTO TABLE resumen_vtasxart_sucursal", archivosTemporales.back());
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("INSERT IGNORE INTO auxresumen_participacion_sucursal \
		(SELECT\
			sucursal,\
			producto,\
			present,\
			SUM(participacion_unidades)\
				OVER\
			(PARTITION BY sucursal ORDER BY participacion_unidades DESC ROWS BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW)\
			AS participacion_acumulada_pieza,\
			SUM(participacion_monto)\
				OVER\
			(PARTITION BY sucursal ORDER BY participacion_monto DESC ROWS BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW)\
			AS participacion_acumulada_monto\
		FROM resumen_vtasxart_sucursal GROUP BY sucursal, producto, present)");
		instrucciones[num_instrucciones++]=instruccion;

        archivosTemporales.push_back(mServidorVioleta->ObtieneArchivoTemp(num_instrucciones, Respuesta->Id));
		instruccion.sprintf("SELECT\
			arp.sucursal,\
			arp.producto,\
			arp.present,\
			cpa.clasificacion AS clasificacion_x_pieza,\
			cpa2.clasificacion AS clasificacion_x_monto,\
			'X' AS clasificacion_compuesta\
		FROM auxresumen_participacion_sucursal arp\
		JOIN clasificacionpedidosautomaticos cpa\
			ON ROUND(arp.participacion_acumulada_pieza, 1) BETWEEN cpa.limiteinferior AND cpa.limitesuperior\
		JOIN clasificacionpedidosautomaticos cpa2\
			ON ROUND(arp.participacion_acumulada_monto, 1) BETWEEN cpa2.limiteinferior AND cpa2.limitesuperior\
		GROUP BY arp.sucursal, arp.producto, arp.present INTO OUTFILE '%s'", archivosTemporales.back());
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("TRUNCATE TABLE clasificacion_articulos_sucursal");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("LOAD DATA INFILE '%s' IGNORE INTO TABLE clasificacion_articulos_sucursal", archivosTemporales.back());
		instrucciones[num_instrucciones++]=instruccion;

        //por el momento se eligio que la clasificacion compuesta sea la clasificacion por monto
		//posteriormente se anadira una logica mas compleja en este punto
		instruccion.sprintf("UPDATE clasificacion_articulos_sucursal SET clasificacion_compuesta = clasificacion_x_monto");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("INSERT IGNORE INTO auxresumen_participacion_empresa\
		(SELECT\
			idempresa,\
			producto,\
			present,\
			SUM(participacion_unidades)\
				OVER\
			(PARTITION BY idempresa\
			ORDER BY participacion_unidades DESC ROWS BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW) AS participacion_acumulada_pieza,\
			SUM(participacion_monto)\
				OVER\
			(PARTITION BY idempresa\
			ORDER BY participacion_monto DESC ROWS BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW) AS participacion_acumulada_monto\
		 FROM resumen_vtasxart_empresas\
		 GROUP BY idempresa, producto, present)");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("TRUNCATE TABLE clasificacion_articulos_empresas");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("INSERT IGNORE INTO clasificacion_articulos_empresas\
		(SELECT arpe.idempresa, arpe.producto, arpe.present, cpa.clasificacion AS clasificacion_x_pieza,\
		cpa2.clasificacion AS clasificacion_x_monto, cpa2.clasificacion AS clasificacion_x_monto\
		FROM auxresumen_participacion_empresa arpe\
		JOIN clasificacionpedidosautomaticos cpa\
			ON ROUND(arpe.participacion_acumulada_pieza, 1) BETWEEN cpa.limiteinferior AND cpa.limitesuperior\
		JOIN clasificacionpedidosautomaticos cpa2\
			ON ROUND(arpe.participacion_acumulada_monto, 1) BETWEEN cpa2.limiteinferior AND cpa2.limitesuperior\
		GROUP BY arpe.idempresa, arpe.producto, arpe.present)");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("INSERT IGNORE INTO auxresumen_participacion_global\
		(SELECT producto, present,\
		SUM(participacion_unidades)\
			OVER\
		(ORDER BY participacion_unidades DESC ROWS BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW) AS participacion_acumulada_pieza,\
		SUM(participacion_monto)\
			OVER\
		(ORDER BY participacion_monto DESC ROWS BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW) AS participacion_acumulada_monto\
		FROM resumen_vtasxart_global GROUP BY producto, present)");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("TRUNCATE TABLE clasificacion_articulos_global");
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("INSERT IGNORE INTO clasificacion_articulos_global\
		(SELECT arpg.producto, arpg.present, cpa.clasificacion AS clasificacion_x_pieza,\
			cpa2.clasificacion AS clasificacion_x_monto,\
			cpa2.clasificacion AS clasificacion_x_monto\
		FROM auxresumen_participacion_global arpg\
		JOIN clasificacionpedidosautomaticos cpa\
			ON ROUND(arpg.participacion_acumulada_pieza, 1) BETWEEN cpa.limiteinferior AND cpa.limitesuperior\
		JOIN clasificacionpedidosautomaticos cpa2\
			ON ROUND(arpg.participacion_acumulada_monto, 1) BETWEEN cpa2.limiteinferior AND cpa2.limitesuperior\
		GROUP BY arpg.producto, arpg.present)");
		instrucciones[num_instrucciones++]=instruccion;

		//instrucciones[num_instrucciones++]="COMMIT";
		//instrucciones[num_instrucciones++]="SET AUTOCOMMIT=1";

        //SE ACTIVA LA REPLICACION
		instruccion="SET SESSION sql_log_bin = 1";
		instrucciones[num_instrucciones++]=instruccion;

		instruccion.sprintf("INSERT INTO bitacoraservidor VALUES (NULL,'%s',NOW(),'FIN_CLASIFICACION_ARTICULOS')",
		FormServidor->ObtieneClaveSucursal());
		instrucciones[num_instrucciones++]=instruccion;


		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (int i=0; i<num_instrucciones; i++){
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);
		}
		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);

	}__finally{
		if (buffer_sql != NULL) {
			delete buffer_sql;
		}

		for(int i = 0; i < archivosTemporales.size(); i++) {
			mServidorVioleta->BorraArchivoTemp(archivosTemporales.at(i));
		}

	}
}
//---------------------------------------------------------------------------
//ID_CON_CONFPINPAD_BBVA
void ServidorAdminSistema::ConsultaConfPinpadBBVA(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    AnsiString instruccion;
	AnsiString clave_terminal;

	clave_terminal = mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

	// Obtiene todos los datos de la terminal
	instruccion.sprintf("SELECT c.terminalsist, c.claveLogs, c.pinpadTimeOut, c.mensaje, c.binesExcepcion, c.urlAutorizador, c.urlBines, c.urlToken, \
	c.urlTelecarga, c.hostTimeOut, c.afiliacion, c.idAplicacion, c.claveSecreta, c.idLogs, c.garanti, c.tecladoLiberado, \
    c.pinpadConexion, c.puertoWifi, c.terminal, \
	c.contactless, c.activo, c.permitecancel, c.permitedevolu, c.permitecash \
	FROM configpinpadb c \
	WHERE c.terminalsist = '%s' ", clave_terminal);
	mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);
}
//---------------------------------------------------------------------------
//ID_GRA_CONFPINPAD_BBVA
void ServidorAdminSistema::GrabaConfPinpadBBVA(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql=new char[1024*64*20];
	char *aux_buffer_sql=buffer_sql;
	BufferRespuestas* resp=NULL;
	int num_instrucciones=0, i;
	AnsiString instruccion, instrucciones[100];

	AnsiString terminalSist, terminal, clavelog, timeOut, mensaje, binesEx, hostURL, binesURL, tokenURL, telecarga, hostTimeout,
		afiliacion, aplicacion, clvSecreta, puerto, puertoWifi, longBines, afiliacionUSD = " ";
	int logs, garanti, moto = 0, teclado, contac, activo, cancelaciones, devoluciones, cash;
	AnsiString tarea, rutaBase="", valores;
	AnsiString clavelog1, puerto1, timeOut1, puertoWifi1,mensaje1, binesEx1, hostURL1, binesURL1, tokenURL1, telecarga1, hostTimeout1,
		afiliacion1, terminal1, aplicacion1, clvSecreta1, terminalSist1;
	int logs1, garanti1,teclado1, contac1, activo1, cancelaciones1, devoluciones1, cash1;
	AnsiString usuario;

	tarea = mFg.ExtraeStringDeBuffer(&parametros);
	terminalSist = mFg.ExtraeStringDeBuffer(&parametros);
	terminal = mFg.ExtraeStringDeBuffer(&parametros);
	clavelog = mFg.ExtraeStringDeBuffer(&parametros);
	timeOut = mFg.ExtraeStringDeBuffer(&parametros);
	mensaje = mFg.ExtraeStringDeBuffer(&parametros);
	binesEx = mFg.ExtraeStringDeBuffer(&parametros);
	hostURL = mFg.ExtraeStringDeBuffer(&parametros);
	binesURL = mFg.ExtraeStringDeBuffer(&parametros);
	tokenURL = mFg.ExtraeStringDeBuffer(&parametros);
	telecarga = mFg.ExtraeStringDeBuffer(&parametros);
	hostTimeout = mFg.ExtraeStringDeBuffer(&parametros);
	afiliacion = mFg.ExtraeStringDeBuffer(&parametros);
	aplicacion = mFg.ExtraeStringDeBuffer(&parametros);
	clvSecreta = mFg.ExtraeStringDeBuffer(&parametros);
	puerto = mFg.ExtraeStringDeBuffer(&parametros);
	puertoWifi = mFg.ExtraeStringDeBuffer(&parametros);
	logs = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
	garanti = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
	teclado = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
	contac = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
	activo = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
	cancelaciones = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
	devoluciones = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
    cash = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));

	//para bitacora  valores anteriores
	if(tarea == "M"){
		logs1 = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
		clavelog1 = mFg.ExtraeStringDeBuffer(&parametros);
		puerto1 = mFg.ExtraeStringDeBuffer(&parametros);
		timeOut1 = mFg.ExtraeStringDeBuffer(&parametros);
		puertoWifi1 = mFg.ExtraeStringDeBuffer(&parametros);
		mensaje1 = mFg.ExtraeStringDeBuffer(&parametros);
		contac1 = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
		teclado1 = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
		binesEx1 = mFg.ExtraeStringDeBuffer(&parametros);
		hostURL1 = mFg.ExtraeStringDeBuffer(&parametros);
		binesURL1 = mFg.ExtraeStringDeBuffer(&parametros);
		tokenURL1 = mFg.ExtraeStringDeBuffer(&parametros);
		telecarga1 = mFg.ExtraeStringDeBuffer(&parametros);
		hostTimeout1 = mFg.ExtraeStringDeBuffer(&parametros);
		garanti1 = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
		afiliacion1 = mFg.ExtraeStringDeBuffer(&parametros);
		terminal1 = mFg.ExtraeStringDeBuffer(&parametros);
		aplicacion1 = mFg.ExtraeStringDeBuffer(&parametros);
		clvSecreta1 = mFg.ExtraeStringDeBuffer(&parametros);
		terminalSist1 = mFg.ExtraeStringDeBuffer(&parametros);
		activo1 = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
		cancelaciones1 = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
		devoluciones1 = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
        cash1 = StrToInt(mFg.ExtraeStringDeBuffer(&parametros));
	}
    usuario =  mFg.ExtraeStringDeBuffer(&parametros);

	instrucciones[num_instrucciones++]="SET AUTOCOMMIT=0";
	instrucciones[num_instrucciones++]="START TRANSACTION";

	try{
		if(tarea == "M"){
			instruccion.sprintf("UPDATE configpinpadb SET \
			idLogs = %d, claveLogs = '%s', pinpadConexion = '%s', pinpadTimeOut = '%s', puertoWifi = '%s', mensaje = '%s', \
			contactless = %d, tecladoLiberado = %d, binesExcepcion = '%s', urlAutorizador = '%s', urlBines = '%s', urlToken = '%s', \
			urlTelecarga = '%s', hostTimeOut = '%s', garanti = %d, \
			moto = %d, afiliacion = '%s', afiliacionUsd = '%s', terminal = '%s', idAplicacion = '%s', claveSecreta = '%s', \
			activo = %d, permitecancel = %d, permitedevolu = %d, permitecash = %d  \
			WHERE terminalsist = '%s' ",logs, clavelog, puerto, timeOut, puertoWifi,
			mensaje, contac, teclado, binesEx, hostURL, binesURL, tokenURL, telecarga, hostTimeout, garanti,
			moto, afiliacion, afiliacionUSD, terminal, aplicacion, clvSecreta, activo,
			cancelaciones, devoluciones, cash, terminalSist);
			instrucciones[num_instrucciones++]=instruccion;

			//instruccion para guardar en bitacora con los datos de valores y de la consulta de arriba
			instruccion.sprintf("INSERT INTO bitacoraconfigpinpad_bbva (idLogs, claveLogs, pinpadConexion, pinpadTimeOut, puertoWifi, mensaje, \
			contactless, tecladoLiberado, binesExcepcion, urlAutorizador, urlBines, urlToken, urlTelecarga, hostTimeOut, garanti, \
			afiliacion, terminal, idAplicacion, claveSecreta, terminalsist, activo, \
			permitecancel, permitedevolu, permitecash, \
			idLogs_update, claveLogs_update, pinpadConexion_update, pinpadTimeOut_update, puertoWifi_update, mensaje_update, \
			contactless_update, tecladoLiberado_update, binesExcepcion_update, urlAutorizador_update, urlBines_update, \
			urlToken_update, urlTelecarga_update, hostTimeOut_update, garanti_update, afiliacion_update, \
			terminal_update,idAplicacion_update, claveSecreta_update, terminalsist_update, activo_update, \
			permitecancel_update, permitedevolu_update, permitecash_update, tipo_modi, usuariomodi, fechamodi, horamodi)  \
			VALUES (%d, '%s', '%s', '%s', '%s', '%s', \
			%d, %d, '%s', '%s', '%s', '%s', '%s', '%s',%d, \
			'%s','%s','%s','%s','%s',%d, \
			%d, %d, %d, \
			%d, '%s', '%s', '%s', '%s', '%s', \
			%d, %d, '%s', '%s', '%s', '%s', '%s', '%s',%d, \
			'%s', '%s','%s','%s','%s',%d, \
			%d, %d,%d, '%s', '%s', CURDATE(), CURTIME() )", logs1, clavelog1, puerto1, timeOut1, puertoWifi1,mensaje1,
			contac1, teclado1, binesEx1, hostURL1, binesURL1, tokenURL1, telecarga1, hostTimeout1, garanti1,
			afiliacion1, terminal1, aplicacion1, clvSecreta1, terminalSist1, activo1,cancelaciones1, devoluciones1, cash1,
			logs, clavelog, puerto, timeOut, puertoWifi, mensaje,
			contac, teclado, binesEx, hostURL, binesURL, tokenURL, telecarga, hostTimeout, garanti,
			afiliacion, terminal, aplicacion, clvSecreta, terminalSist, activo,cancelaciones, devoluciones, cash,
			tarea, usuario );
			instrucciones[num_instrucciones++]=instruccion;

		}else if(tarea == "A"){
			instruccion.sprintf("INSERT INTO configpinpadb (idLogs, claveLogs, pinpadConexion, pinpadTimeOut, puertoWifi, mensaje, \
			contactless, tecladoLiberado, binesExcepcion, urlAutorizador, urlBines, urlToken, urlTelecarga, hostTimeOut, garanti, \
			moto, afiliacion, afiliacionUsd, terminal,idAplicacion, claveSecreta, terminalsist, activo, \
			permitecancel, permitedevolu, permitecash) VALUES ( \
			%d, '%s', '%s', '%s', '%s', '%s', \
			%d, %d, '%s', '%s', '%s', '%s', '%s', '%s',%d, \
			%d, '%s', '%s','%s','%s','%s','%s',%d, \
			%d, %d, %d ) ", logs, clavelog, puerto, timeOut, puertoWifi, mensaje,
			contac, teclado, binesEx, hostURL, binesURL, tokenURL, telecarga, hostTimeout, garanti,
			moto, afiliacion, afiliacionUSD, terminal, aplicacion, clvSecreta, terminalSist, activo,
			cancelaciones, devoluciones, cash);
			instrucciones[num_instrucciones++]=instruccion;

            //instruccion para guardar en bitacora con los datos de valores y de la consulta de arriba
			instruccion.sprintf("INSERT INTO bitacoraconfigpinpad_bbva ( \
			idLogs_update, claveLogs_update, pinpadConexion_update, pinpadTimeOut_update, puertoWifi_update, mensaje_update, \
			contactless_update, tecladoLiberado_update, binesExcepcion_update, urlAutorizador_update, urlBines_update, \
			urlToken_update, urlTelecarga_update, hostTimeOut_update, garanti_update, afiliacion_update, \
			terminal_update,idAplicacion_update, claveSecreta_update, terminalsist_update, activo_update, \
			permitecancel_update, permitedevolu_update, permitecash_update, tipo_modi, usuariomodi, fechamodi, horamodi)  \
			VALUES (%d, '%s', '%s', '%s', '%s', '%s', \
			%d, %d, '%s', '%s', '%s', '%s', '%s', '%s',%d, \
			'%s','%s','%s','%s','%s',%d, \
			%d, %d, %d, \
			'%s', '%s', CURDATE(), CURTIME() )",
			logs, clavelog, puerto, timeOut, puertoWifi, mensaje,
			contac, teclado, binesEx, hostURL, binesURL, tokenURL, telecarga, hostTimeout, garanti,
			afiliacion, terminal, aplicacion, clvSecreta, terminalSist, activo,cancelaciones, devoluciones, cash,
			tarea, usuario );
			instrucciones[num_instrucciones++]=instruccion;

		}

		instrucciones[num_instrucciones++]="COMMIT";

		// Crea el buffer con todas las instrucciones SQL
		aux_buffer_sql=mFg.AgregaStringABuffer(mFg.IntToAnsiString(num_instrucciones), aux_buffer_sql);
		for (i=0; i<num_instrucciones; i++)
			aux_buffer_sql=mFg.AgregaStringABuffer(instrucciones[i], aux_buffer_sql);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_RESULTADO_EJECUTA_ACCIONES);
		mServidorVioleta->EjecutaBufferAccionesSql(Respuesta, MySQL, buffer_sql);

	}__finally{
		delete buffer_sql;
	}
}
//---------------------------------------------------------------------------
//ID_BAJ_CONFPINPAD_BBVA
void ServidorAdminSistema::BorraConfPinpadBBVA(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	char *buffer_sql = new char[1024 * 5];
	char *aux_buffer_sql = buffer_sql;
	AnsiString instruccion;
	int num_instrucciones = 0, i = 0;
	AnsiString instrucciones[7];
	AnsiString clave_terminal;

	clave_terminal = mFg.ExtraeStringDeBuffer(&parametros);

	try {

		instrucciones[num_instrucciones++] = "SET AUTOCOMMIT=0";
		instrucciones[num_instrucciones++] = "START TRANSACTION";

		instruccion.sprintf("DELETE FROM configpinpadb WHERE terminalsist = '%s' ", clave_terminal);
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
//---------------------------------------------------------------------------
//ID_EJE_REPBITCONFIGPINPAD_BBVA
void ServidorAdminSistema::EjecutaBitaConfPinpadBBVA(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString instruccion;
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA);

		instruccion.sprintf("SELECT b.tipo_modi, b.fechamodi, b.horamodi, CONCAT(e.nombre,' ', e.appat, ' ', e.apmat) AS usuariomodi, b.terminal, b.terminal_update, b.terminalsist, b.terminalsist_update, \
		b.activo, b.activo_update, b.permitecancel, b.permitecancel_update, \
		b.permitedevolu, b.permitedevolu_update, b.permitecash, b.permitecash_update, b.contactless, b.contactless_update, b.mensaje, b.mensaje_update, \
		b.urlAutorizador, b.urlAutorizador_update, b.urlBines, b.urlBines_update, b.urlToken, b.urlToken_update, \
		b.urlTelecarga, b.urlTelecarga_update, b.afiliacion, b.afiliacion_update, b.idAplicacion, b.idAplicacion_update, \
		b.claveSecreta, b.claveSecreta_update, b.pinpadConexion, b.pinpadConexion_update, \
		b.idLogs, b.idLogs_update, b.claveLogs, b.claveLogs_update, \
		b.pinpadTimeOut, b.pinpadTimeOut_update, b.puertoWifi, b.puertoWifi_update, \
		b.tecladoLiberado, b.tecladoLiberado_update, \
		b.binesExcepcion, b.binesExcepcion_update, b.hostTimeOut, b.hostTimeOut_update, \
		b.garanti, b.garanti_update \
		FROM bitacoraconfigpinpad_bbva b \
		INNER JOIN empleados e ON e.empleado = b.usuariomodi \
		ORDER BY b.fechamodi DESC, b.horamodi DESC");

        mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(),
		Respuesta->TamBufferResultado);

}
//---------------------------------------------------------------------------
