#include <vcl.h>
//---------------------------------------------------------------------------
#include "pch.h"

#pragma hdrstop

#include "ClassServidorBusquedas.h"
#include "ClassBufferRespuestas.h"
#include "ClassServidorVioleta.h"
#include "violetaS.h"
#include "comunes.h"
#include "ClassPrivilegiosDeObjeto.h"
#include "FormServidorVioleta.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

//---------------------------------------------------------------------------
//                      BUSQUEDA DE ARTICULOS
//---------------------------------------------------------------------------
//ID_BUSQ_ARTICULO
void ServidorBusquedas::BuscaArticulos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    // BUSQUEDA DE ARTICULOS
	DatosTabla datos(mServidorVioleta->Tablas);
    AnsiString instruccion;
    AnsiString tipo_busqueda;
	AnsiString nombre, clave, marca, clasif_especifica;
	AnsiString solo_activos, condicion_solo_activos = " ", sucursal, condicion_existencias1, condicion_existencias2;
	AnsiString condicion_existencias3;
	AnsiString MostrarExistencias = 0;
	AnsiString existencia;
	BufferRespuestas* resp_exist=NULL;

	//Extrae la sucursal
	sucursal=mFg.ExtraeStringDeBuffer(&parametros);
	//extrae el valor para confirmar que viene desde PV
	existencia=mFg.ExtraeStringDeBuffer(&parametros);

	try{
		instruccion.sprintf("select valor from parametrosemp where parametro='EXISTENCIASPV' AND sucursal = '%s' ",  FormServidor->ObtieneClaveSucursal());
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_exist);
		MostrarExistencias=resp_exist->ObtieneDato("valor");
		//condiciones para mostrar articulos con existencias
		if(MostrarExistencias == "0" || existencia == "NO" )
		{
			condicion_existencias1 = " ";
			condicion_existencias2 = " ";
			condicion_existencias3 = " ";
		}
		if(MostrarExistencias == "1" && existencia == "SI")
		{
			condicion_existencias1 = ",existenciasactuales e, secciones s, almacenes al";
			condicion_existencias2.sprintf(" AND e.producto = a.producto AND e.present = a.present AND  s.sucursal = '%s'   \
									   AND s.seccion = al.seccion  AND al.almacen = e.almacen AND e.cantidad > 0", sucursal);
			condicion_existencias3.sprintf(" INNER JOIN secciones s ON s.sucursal = '%s' INNER JOIN almacenes al \
			ON s.seccion = al.seccion  INNER JOIN  existenciasactuales e ON e.producto = a.producto AND  \
			e.present = a.present AND al.almacen = e.almacen AND e.cantidad > 0 ",sucursal);
		}

		// Extrae el tipo de búsqueda
		tipo_busqueda=mFg.ExtraeStringDeBuffer(&parametros);
		solo_activos=mFg.ExtraeStringDeBuffer(&parametros);
		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ);


		if (solo_activos=="1") {
			condicion_solo_activos=" a.activo=1 and ";
		}

		if (tipo_busqueda=="N"){
			nombre=mFg.ExtraeStringDeBuffer(&parametros);
			instruccion.sprintf("select p.nombre, a.present, a.multiplo, a.factor, p.marca, m.nombre as nommarca, p.producto, \
			a.articulo,a.ean13,a.activo from articulos a, productos p, marcas m %s where CONCAT(p.nombre,'%%',a.present) like '%s%%' \
			and %s p.producto=a.producto and p.marca=m.marca %s order by p.nombre, a.present,a.factor desc \
			limit %s", condicion_existencias1, nombre, condicion_solo_activos, condicion_existencias2, NUM_LIMITE_RESULTADOS_BUSQ);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
		if  (tipo_busqueda=="C"){
			clave=mFg.ExtraeStringDeBuffer(&parametros);
			instruccion.sprintf("select p.nombre, a.present, a.multiplo, a.factor, p.marca, m.nombre as nommarca, p.producto, \
			 a.articulo,a.ean13,a.activo from articulos a, productos p, marcas m %s where p.producto like '%s%%' and %s \
			 p.producto=a.producto and p.marca=m.marca %s order by p.nombre,a.present,a.factor desc  \
			 limit %s", condicion_existencias1, clave, condicion_solo_activos, condicion_existencias2, NUM_LIMITE_RESULTADOS_BUSQ);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
		if (tipo_busqueda=="M"){
			marca=mFg.ExtraeStringDeBuffer(&parametros);
			instruccion.sprintf("select p.nombre, a.present, a.multiplo, a.factor, p.marca, m.nombre as nommarca, p.producto, \
			a.articulo,a.ean13,a.activo  from articulos a, productos p, marcas m %s where p.marca='%s' and %s  \
			p.producto=a.producto and p.marca=m.marca %s order by p.nombre, a.present, a.factor desc 	\
			limit %s", condicion_existencias1, marca, condicion_solo_activos, condicion_existencias2, NUM_LIMITE_RESULTADOS_BUSQ);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
		if (tipo_busqueda=="E"){
			clasif_especifica=mFg.ExtraeStringDeBuffer(&parametros);
			instruccion.sprintf("select p.nombre, a.present, a.multiplo, a.factor, p.marca, m.nombre as nommarca, p.producto, \
			a.articulo,a.ean13,a.activo  from articulos a, productos p, marcas m %s where p.clasif3='%s' and \
			%s p.producto=a.producto and p.marca=m.marca %s order by p.nombre, a.present, a.factor desc  \
			limit %s", condicion_existencias1, clasif_especifica, condicion_solo_activos, condicion_existencias2, NUM_LIMITE_RESULTADOS_BUSQ);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}

		if (tipo_busqueda=="CB"){
			clave=mFg.ExtraeStringDeBuffer(&parametros);
			instruccion.sprintf("SELECT p.nombre,CONCAT(a.present,' ',IFNULL(cb.descripcion,'')),a.multiplo,a.factor,p.marca,m.nombre AS nommarca,p.producto,a.articulo,a.ean13,a.activo  \
								FROM articulos a \
								INNER JOIN  productos p ON p.producto=a.producto \
								INNER JOIN marcas m ON p.marca = m.marca \
								LEFT JOIN codigosbarras cb ON cb.articulo=a.articulo %s\
								WHERE (a.ean13 = '%s' OR cb.codigobarras='%s') and %s true \
								ORDER BY p.producto,a.present,a.factor desc  limit %s"  , condicion_existencias3, clave,
								clave, condicion_solo_activos,NUM_LIMITE_RESULTADOS_BUSQ);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}

		if (tipo_busqueda=="ART"){
			clave=mFg.ExtraeStringDeBuffer(&parametros);
			instruccion.sprintf("select p.nombre, a.present, a.multiplo, a.factor, p.marca, m.nombre as nommarca, p.producto, \
			a.articulo,a.ean13,a.activo  from articulos a, productos p, marcas m %s where a.articulo='%s' and %s \
			p.producto=a.producto and p.marca=m.marca %s order by p.nombre, a.present ,a.factor desc  \
			limit %s", condicion_existencias1, clave, condicion_solo_activos, condicion_existencias2, NUM_LIMITE_RESULTADOS_BUSQ);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}

		if (tipo_busqueda==""){
			//  Clasificacion 1
			instruccion.sprintf("select clasif1, nombre from clasificacion1 order by nombre");
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

			//  Marcas
			instruccion.sprintf("select marca, nombre from marcas order by nombre");
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
	}
	__finally{
		if(resp_exist != NULL) delete resp_exist;
	}
}
//---------------------------------------------------------------------------
//ID_BUSQ_CLIENTE
void ServidorBusquedas::BuscaClientes(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// BUSQUEDA DE ARTICULOS
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instruccion;
	AnsiString tipo_busqueda;
	AnsiString solo_activos, condicion_activos=" ";
	AnsiString dato_buscado;

	// Extrae el tipo de búsqueda
	tipo_busqueda=mFg.ExtraeStringDeBuffer(&parametros);
	solo_activos=mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ);

	if (solo_activos=="1") {
		condicion_activos=" cli.activo=1 and ";
	}

	if (tipo_busqueda!="") dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);

	if (tipo_busqueda=="NOM") {
		instruccion.sprintf("select cliente, \
			rsocial, nomnegocio, rfc, CONCAT(rf.regimenfiscal, ' - ', rf.descripcion) AS regimenfiscal, \
			contacto, mun.nombre, loc.nombre, \
			calle, col.nombre, cp \
			from clientes cli \
			left join colonias col on cli.colonia=col.colonia \
			left join localidades loc on loc.localidad=col.localidad \
			left join municipios mun on loc.municipio=mun.municipio \
			left join cregimenfiscal rf ON cli.regimenfiscal = rf.regimenfiscal \
			where %s concat(cli.nombre, ' ', appat, ' ', apmat) like '%s%%' \
			order by replace(rsocial, '\"', '') limit %s", condicion_activos, dato_buscado, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="APE"){
		instruccion.sprintf("select cliente, \
		rsocial, nomnegocio, rfc, CONCAT(rf.regimenfiscal, ' - ', rf.descripcion) AS regimenfiscal, \
		 contacto, mun.nombre, loc.nombre, \
		calle, col.nombre, cp \
		from clientes cli \
		left join colonias col on cli.colonia=col.colonia \
		left join localidades loc on loc.localidad=col.localidad \
		left join municipios mun on loc.municipio=mun.municipio \
        left join cregimenfiscal rf ON cli.regimenfiscal = rf.regimenfiscal \
		where %s appat like '%s%%' \
		order by replace(rsocial, '\"', '') limit %s", condicion_activos, dato_buscado, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="RFC") {
		instruccion.sprintf("select cliente, \
		rsocial, nomnegocio, rfc, CONCAT(rf.regimenfiscal, ' - ', rf.descripcion) AS regimenfiscal, \
		contacto, mun.nombre, loc.nombre, \
		calle, col.nombre, cp \
		from clientes cli \
		left join colonias col on cli.colonia=col.colonia \
		left join localidades loc on loc.localidad=col.localidad \
		left join municipios mun on loc.municipio=mun.municipio \
		left join cregimenfiscal rf ON cli.regimenfiscal = rf.regimenfiscal \
		where %s rfc like '%s%%' \
		order by rfc, replace(rsocial, '\"', '') limit %s", condicion_activos, dato_buscado, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="RSO"){
		instruccion.sprintf("select cliente, \
		rsocial, nomnegocio, rfc, CONCAT(rf.regimenfiscal, ' - ', rf.descripcion) AS regimenfiscal, \
		 contacto, mun.nombre, loc.nombre, \
		calle, col.nombre, cp \
		from clientes cli \
		left join colonias col on cli.colonia=col.colonia \
		left join localidades loc on loc.localidad=col.localidad \
		left join municipios mun on loc.municipio=mun.municipio \
		left join cregimenfiscal rf ON cli.regimenfiscal = rf.regimenfiscal \
		where %s rsocial like '%s%%' \
		order by replace(rsocial, '\"', '') limit %s", condicion_activos, dato_buscado, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="NNE"){
		instruccion.sprintf("select cliente, \
		rsocial, nomnegocio, rfc, CONCAT(rf.regimenfiscal, ' - ', rf.descripcion) AS regimenfiscal, \
		 contacto, mun.nombre, loc.nombre, \
		calle, col.nombre, cp \
		from clientes cli \
		left join colonias col on cli.colonia=col.colonia \
		left join localidades loc on loc.localidad=col.localidad \
		left join municipios mun on loc.municipio=mun.municipio \
		left join cregimenfiscal rf ON cli.regimenfiscal = rf.regimenfiscal \
		where %s nomnegocio like '%s%%' \
		order by nomnegocio, replace(rsocial, '\"', '') limit %s", condicion_activos, dato_buscado, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="CLA"){
		instruccion.sprintf("select cliente, \
		rsocial, nomnegocio, rfc, CONCAT(rf.regimenfiscal, ' - ', rf.descripcion) AS regimenfiscal, \
		 contacto, mun.nombre, loc.nombre, \
		calle, col.nombre, cp \
		from clientes cli \
		left join colonias col on cli.colonia=col.colonia \
		left join localidades loc on loc.localidad=col.localidad \
		left join municipios mun on loc.municipio=mun.municipio \
		left join cregimenfiscal rf ON cli.regimenfiscal = rf.regimenfiscal \
		where %s cliente like '%s%%' \
		order by cliente, replace(rsocial, '\"', '') limit %s", condicion_activos, dato_buscado, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="CONT"){
		instruccion.sprintf("select cliente, \
		rsocial, nomnegocio, rfc, CONCAT(rf.regimenfiscal, ' - ', rf.descripcion) AS regimenfiscal, \
		 contacto, mun.nombre, loc.nombre, \
		calle, col.nombre, cp \
		from clientes cli \
		left join colonias col on cli.colonia=col.colonia \
		left join localidades loc on loc.localidad=col.localidad \
		left join municipios mun on loc.municipio=mun.municipio \
		left join cregimenfiscal rf ON cli.regimenfiscal = rf.regimenfiscal \
		where %s cli.contacto like '%s%%' \
		order by replace(rsocial, '\"', '') limit %s", condicion_activos, dato_buscado, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="EMA"){
		instruccion.sprintf("SELECT cliente,rsocial, nomnegocio, rfc, CONCAT(rf.regimenfiscal, ' - ', rf.descripcion) AS regimenfiscal, \
		 contacto, mun.nombre, loc.nombre, calle, col.nombre, cp \
		FROM clientes cli \
		LEFT JOIN colonias col ON cli.colonia=col.colonia  \
		LEFT JOIN localidades loc ON loc.localidad=col.localidad  \
		LEFT JOIN municipios mun ON loc.municipio=mun.municipio \
		LEFT JOIN cregimenfiscal rf ON cli.regimenfiscal = rf.regimenfiscal \
		WHERE  %s cli.email LIKE '%s%%' OR cli.email2 LIKE '%s%%' \
		order by replace(rsocial, '\"', '') limit %s", condicion_activos, dato_buscado, dato_buscado,NUM_LIMITE_RESULTADOS_BUSQ);

		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda==""){
		// No mandar nada
	}
}
//---------------------------------------------------------------------------
//ID_BUSQ_PROVEEDOR
void ServidorBusquedas::BuscaProveedores(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// BUSQUEDA DE ARTICULOS
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instruccion;
	AnsiString tipo_busqueda;
	AnsiString dato_buscado;
	AnsiString solo_activos, condicion_solo_activos = " ";
	AnsiString solo_ProvGastos, condicion_solo_ProvGastos = " ";
	AnsiString solo_ProvMercancia, condicion_solo_ProvMercancia = " ";

	// Extrae el tipo de búsqueda
	tipo_busqueda=mFg.ExtraeStringDeBuffer(&parametros);
	solo_activos=mFg.ExtraeStringDeBuffer(&parametros);


	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ);

	if (solo_activos=="1") {
		condicion_solo_activos="AND activo=1 ";
	}

	if (tipo_busqueda!="")
	{
		dato_buscado="%"+mFg.ExtraeStringDeBuffer(&parametros)+"%";
	}

	solo_ProvGastos=mFg.ExtraeStringDeBuffer(&parametros);
	if (solo_ProvGastos=="1") {
		condicion_solo_ProvGastos=" AND provgastos=1 ";
	}

	solo_ProvMercancia=mFg.ExtraeStringDeBuffer(&parametros);
	if (solo_ProvMercancia=="1") {
		condicion_solo_ProvMercancia=" AND provmercancia=1 ";
	}

	if (tipo_busqueda=="RSO"){
		instruccion.sprintf("select proveedor, razonsocial, replegal, rfc, estado, localidad, calle, colonia,redondeocptecho, if(provgastos=1, 'Si', 'No') as provgastos, if(provmercancia=1, 'Si', 'No') as provmercancia, activo \
			from proveedores where razonsocial like '%s' %s %s %s order by razonsocial limit %s",
			dato_buscado,condicion_solo_activos,condicion_solo_ProvGastos, condicion_solo_ProvMercancia, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="RFC"){
		instruccion.sprintf("select proveedor, razonsocial, replegal, rfc, estado, localidad, calle, colonia,redondeocptecho, if(provgastos=1, 'Si', 'No') as provgastos, if(provmercancia=1, 'Si', 'No') as provmercancia, activo \
		from proveedores where rfc like '%s' %s %s %s order by rfc, razonsocial limit %s",
		dato_buscado,condicion_solo_activos,condicion_solo_ProvGastos, condicion_solo_ProvMercancia, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="CLA"){
		instruccion.sprintf("select proveedor, razonsocial, replegal, rfc, estado, localidad, calle, colonia,redondeocptecho,  if(provgastos=1, 'Si', 'No') as provgastos, if(provmercancia=1, 'Si', 'No') as provmercancia, activo \
		from proveedores where proveedor like '%s' %s %s %s order by rfc, razonsocial limit %s",
		dato_buscado,condicion_solo_activos,condicion_solo_ProvGastos, condicion_solo_ProvMercancia, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="REP"){
		instruccion.sprintf("select proveedor, razonsocial, replegal, rfc, estado, localidad, calle, colonia,redondeocptecho, if(provgastos=1, 'Si', 'No') as provgastos, if(provmercancia=1, 'Si', 'No') as provmercancia, activo \
		from proveedores where replegal like '%s' %s %s %s order by razonsocial limit %s",
		dato_buscado,condicion_solo_activos,condicion_solo_ProvGastos, condicion_solo_ProvMercancia, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }

    if (tipo_busqueda==""){
		// No mandar nada
	}
}

//---------------------------------------------------------------------------
//ID_BUSQ_COMPRA
void ServidorBusquedas::BuscaCompras(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    // BUSQUEDA DE COMPRAS
    DatosTabla datos(mServidorVioleta->Tablas);
    AnsiString instruccion;
    AnsiString tipo_busqueda;
    AnsiString dato_buscado, fecha_ini, fecha_fin;

	// Extrae el tipo de búsqueda
	tipo_busqueda=mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ);

	AnsiString condicion_empresa=" ";
	AnsiString join_necesarios = " INNER JOIN terminales ter ON c.terminal = ter.terminal \
		INNER JOIN secciones sec ON ter.seccion = sec.seccion \
		INNER JOIN sucursales suc ON sec.sucursal = suc.sucursal";
	condicion_empresa.sprintf( " AND suc.idempresa = %s ", FormServidor->ObtieneClaveEmpresa() );

	if (tipo_busqueda=="PROV"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("SELECT IFNULL(ped.referencia, GROUP_CONCAT(DISTINCT(cpp.pedido))) AS referencia, c.fechacom, c.referencia, c.folioprov, p.razonsocial, FORMAT(c.valor,2) AS valor, \
		c.anticipo, CONCAT(ec.nombre, ' ', ec.appat, ' ', ec.apmat) AS comprador, a.nombre, \
		CONCAT(em.nombre, ' ', em.appat, ' ', em.apmat) AS usuario, c.cancelado AS estado \
		FROM compras c INNER JOIN almacenes a ON c.almacen=a.almacen \
		INNER JOIN empleados ec ON c.comprador=ec.empleado \
		INNER JOIN empleados em ON c.usumodi=em.empleado \
		INNER JOIN proveedores p ON c.proveedor=p.proveedor \
		LEFT JOIN pedidos ped ON ped.compra = c.referencia \
		%s \
		LEFT JOIN compraspedidosprov cpp ON cpp.compra = c.referencia \
		WHERE p.razonsocial LIKE '%s%%' AND c.fechacom>='%s' AND c.fechacom<='%s' %s \
		GROUP BY c.referencia \
		ORDER BY p.razonsocial, c.fechacom, c.referencia LIMIT %s", join_necesarios, dato_buscado, fecha_ini, fecha_fin, condicion_empresa, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="FOLIPROV"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("SELECT IFNULL(ped.referencia, GROUP_CONCAT(DISTINCT(cpp.pedido))) AS referencia, c.fechacom, c.referencia, c.folioprov, p.razonsocial, FORMAT(c.valor,2) AS valor, \
		c.anticipo, concat(ec.nombre, ' ', ec.appat, ' ', ec.apmat) AS comprador, a.nombre, \
		concat(em.nombre, ' ', em.appat, ' ', em.apmat) AS usuario, c.cancelado AS estado \
		FROM compras c \
		INNER JOIN almacenes a ON c.almacen=a.almacen \
		INNER JOIN empleados ec ON c.comprador=ec.empleado \
		INNER JOIN empleados em ON c.usumodi=em.empleado \
		INNER JOIN proveedores p ON c.proveedor=p.proveedor \
		LEFT JOIN pedidos ped ON ped.compra = c.referencia \
		%s \
		LEFT JOIN compraspedidosprov cpp ON cpp.compra = c.referencia \
		WHERE c.folioprov LIKE '%s%%' AND c.fechacom>='%s' AND c.fechacom<='%s' %s\
		GROUP BY c.referencia \
		ORDER BY p.razonsocial, c.fechacom, c.referencia LIMIT %s", join_necesarios, dato_buscado, fecha_ini, fecha_fin, condicion_empresa, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="REFE"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("SELECT IFNULL(ped.referencia, GROUP_CONCAT(DISTINCT(cpp.pedido))) AS referencia, c.fechacom, c.referencia, c.folioprov, p.razonsocial, FORMAT(c.valor,2) AS valor, \
		c.anticipo, concat(ec.nombre, ' ', ec.appat, ' ', ec.apmat) AS comprador, a.nombre, \
		concat(em.nombre, ' ', em.appat, ' ', em.apmat) AS usuario, c.cancelado AS estado \
		FROM compras c \
		INNER JOIN almacenes a ON c.almacen=a.almacen \
		INNER JOIN empleados ec ON c.comprador=ec.empleado \
		INNER JOIN empleados em ON c.usumodi=em.empleado \
		INNER JOIN proveedores p ON c.proveedor=p.proveedor \
		LEFT JOIN pedidos ped ON ped.compra = c.referencia \
		%s \
		LEFT JOIN compraspedidosprov cpp ON cpp.compra = c.referencia \
		WHERE c.referencia='%s' AND c.fechacom>='%s' AND c.fechacom<='%s' %s \
		GROUP BY c.referencia \
		ORDER BY p.razonsocial, c.fechacom, c.referencia LIMIT %s",join_necesarios, dato_buscado, fecha_ini, fecha_fin, condicion_empresa, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="FECH"){
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("SELECT IFNULL(ped.referencia, GROUP_CONCAT(DISTINCT(cpp.pedido))) AS referencia, c.fechacom, c.referencia, c.folioprov, p.razonsocial, FORMAT(c.valor,2) AS valor, \
		c.anticipo, concat(ec.nombre, ' ', ec.appat, ' ', ec.apmat) AS comprador, a.nombre, \
		concat(em.nombre, ' ', em.appat, ' ', em.apmat) AS usuario, c.cancelado AS estado \
		FROM compras c \
		INNER JOIN almacenes a ON c.almacen=a.almacen \
		INNER JOIN empleados ec ON c.comprador=ec.empleado \
		INNER JOIN empleados em ON c.usumodi=em.empleado \
		INNER JOIN proveedores p ON c.proveedor=p.proveedor \
		LEFT JOIN pedidos ped ON ped.compra = c.referencia \
		%s \
		LEFT JOIN compraspedidosprov cpp ON cpp.compra = c.referencia \
		WHERE c.fechacom>='%s' AND c.fechacom<='%s' %s\
		GROUP BY c.referencia \
		ORDER BY p.razonsocial, c.fechacom, c.referencia LIMIT %s",join_necesarios, fecha_ini, fecha_fin,condicion_empresa, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="ALMA"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("SELECT IFNULL(ped.referencia, GROUP_CONCAT(DISTINCT(cpp.pedido))) AS referencia, c.fechacom,  c.referencia, c.folioprov, p.razonsocial, FORMAT(c.valor,2) AS valor, \
		c.anticipo, concat(ec.nombre, ' ', ec.appat, ' ', ec.apmat) AS comprador, \
		a.nombre, concat(em.nombre, ' ', em.appat, ' ', em.apmat) AS usuario, \
		c.cancelado AS estado \
		FROM compras c \
		INNER JOIN almacenes a ON c.almacen=a.almacen \
		INNER JOIN empleados ec ON c.comprador=ec.empleado \
		INNER JOIN empleados em ON c.usumodi=em.empleado \
		INNER JOIN proveedores p ON c.proveedor=p.proveedor \
		LEFT JOIN pedidos ped ON ped.compra = c.referencia \
		%s \
		LEFT JOIN compraspedidosprov cpp ON cpp.compra = c.referencia \
		WHERE a.almacen='%s' AND c.fechacom>='%s' AND c.fechacom<='%s' %s\
		GROUP BY c.referencia \
		ORDER BY p.razonsocial, c.fechacom, c.referencia LIMIT %s", join_necesarios, dato_buscado, fecha_ini, fecha_fin,condicion_empresa, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="COMP"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("SELECT IFNULL(ped.referencia, GROUP_CONCAT(DISTINCT(cpp.pedido))) AS referencia, c.fechacom, c.referencia, c.folioprov,  p.razonsocial, FORMAT(c.valor,2) AS valor, \
		c.anticipo, concat(ec.nombre, ' ', ec.appat, ' ', ec.apmat) AS comprador, \
		a.nombre, concat(em.nombre, ' ', em.appat, ' ', em.apmat) AS usuario, \
		c.cancelado AS estado \
		FROM compras c \
		INNER JOIN almacenes a ON c.almacen=a.almacen \
		INNER JOIN empleados ec ON c.comprador=ec.empleado \
		INNER JOIN empleados em ON c.usumodi=em.empleado \
		INNER JOIN proveedores p ON c.proveedor=p.proveedor \
		LEFT JOIN pedidos ped ON ped.compra = c.referencia \
		%s \
		LEFT JOIN compraspedidosprov cpp ON cpp.compra = c.referencia \
		WHERE ec.empleado='%s' AND c.fechacom>='%s' AND c.fechacom<='%s' %s\
		GROUP BY c.referencia \
		ORDER BY p.razonsocial, c.fechacom, c.referencia LIMIT %s", join_necesarios, dato_buscado, fecha_ini, fecha_fin,condicion_empresa, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="USUA"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("SELECT IFNULL(ped.referencia, GROUP_CONCAT(DISTINCT(cpp.pedido))) AS referencia, c.fechacom, c.referencia, c.folioprov, p.razonsocial, FORMAT(c.valor,2) AS valor, \
		c.anticipo, concat(ec.nombre, ' ', ec.appat, ' ', ec.apmat) AS comprador, \
		a.nombre, concat(em.nombre, ' ', em.appat, ' ', em.apmat) AS usuario, c.cancelado AS estado \
		FROM compras c \
		INNER JOIN almacenes a ON c.almacen=a.almacen \
		INNER JOIN empleados ec ON c.comprador=ec.empleado \
		INNER JOIN empleados em ON c.usumodi=em.empleado \
		INNER JOIN proveedores p ON c.proveedor=p.proveedor \
		LEFT JOIN pedidos ped ON ped.compra = c.referencia \
		%s \
		LEFT JOIN compraspedidosprov cpp ON cpp.compra = c.referencia \
		WHERE em.empleado='%s' AND c.fechacom>='%s' AND c.fechacom<='%s' %s\
		GROUP BY c.referencia \
		ORDER BY p.razonsocial, c.fechacom, c.referencia LIMIT %s",join_necesarios, dato_buscado, fecha_ini, fecha_fin, condicion_empresa, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}

	if (tipo_busqueda=="") {
		// Obtiene todos los posibles almacenes
		instruccion.sprintf("select alm.almacen, alm.nombre \
			FROM almacenes alm  \
			INNER JOIN secciones sec ON sec.seccion = alm.seccion  \
			INNER JOIN sucursales suc ON suc.sucursal = sec.sucursal \
			WHERE suc.idempresa = %s  \
			ORDER BY alm.nombre",FormServidor->ObtieneClaveEmpresa());
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

		// Obtiene todos los empleados como posibles compradores
		instruccion.sprintf("SELECT emp.empleado AS Empleado, concat(emp.nombre,' ',emp.appat,' ',emp.apmat) AS Nombre \
			FROM empleados emp \
			INNER JOIN sucursales suc ON emp.sucursal = suc.sucursal \
			WHERE suc.idempresa= %s \
			ORDER by emp.nombre, emp.apmat, emp.appat", FormServidor->ObtieneClaveEmpresa());
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

		// Obtiene todos los usuarios
		instruccion.sprintf("SELECT usu.empleado AS Usuario, emp.nombre AS Nombre \
			FROM usuarios usu \
			INNER JOIN empleados emp ON usu.empleado =  emp.empleado \
			INNER JOIN sucursales suc ON emp.sucursal = suc.sucursal \
			WHERE suc.idempresa=%s \
			ORDER BY emp.nombre", FormServidor->ObtieneClaveEmpresa());
		/*instruccion="select usuarios.empleado AS Usuario, empleados.nombre AS Nombre ";
		instruccion+="from usuarios, empleados ";
		instruccion+="where empleados.empleado=usuarios.empleado order by nombre"; */
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
}

//---------------------------------------------------------------------------
//ID_BUSQ_PEDIDO_PROV
void ServidorBusquedas::BuscaPedidosProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    // BUSQUEDA DE PEDIDOS
    DatosTabla datos(mServidorVioleta->Tablas);
    AnsiString instruccion;
	AnsiString tipo_busqueda;
	AnsiString dato_buscado, fecha_ini, fecha_fin, tipo_pedido;
	AnsiString condicion_tipopedido = " ";

    // Extrae el tipo de búsqueda
    tipo_busqueda=mFg.ExtraeStringDeBuffer(&parametros);
    mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ);

    if (tipo_busqueda=="PROV") {
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        tipo_pedido=mFg.ExtraeStringDeBuffer(&parametros);

		if(tipo_pedido == "1"){
		   condicion_tipopedido.printf(" AND ped.compra IS NULL AND ped.cancelado = 0 ");
		}

		instruccion.sprintf("select COALESCE(cpp.ref_compras, pedxrec.ref_compras, ped.compra, '') AS compra, \
		ped.fechaped, ped.referencia, pro.razonsocial, ped.valor, \
		concat(ec.nombre, ' ', ec.appat, ' ', ec.apmat) as comprador, a.nombre,\
		concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, ped.cancelado,\
		fechavigencia \
		from pedidos ped \
		INNER JOIN almacenes a ON ped.almacen=a.almacen \
		INNER JOIN empleados ec ON ped.comprador=ec.empleado \
		INNER JOIN empleados em ON ped.usumodi=em.empleado \
		INNER JOIN proveedores pro ON ped.proveedor=pro.proveedor \
		INNER JOIN secciones sec ON a.seccion = sec.seccion \
		INNER JOIN sucursales suc ON sec.sucursal = suc.sucursal \
		LEFT JOIN ( \
			SELECT cpp.pedido, GROUP_CONCAT(DISTINCT cpp.compra ORDER BY cpp.compra SEPARATOR ',') AS ref_compras \
			FROM compras c \
			INNER JOIN compraspedidosprov cpp ON c.referencia = cpp.compra \
			INNER JOIN terminales t ON t.terminal = c.terminal \
			INNER JOIN secciones sec ON sec.seccion = t.seccion \
			INNER JOIN sucursales suc ON suc.sucursal = sec.sucursal \
			WHERE c.cancelado = 0 AND suc.idempresa = %s \
			GROUP BY cpp.pedido \
		) cpp ON cpp.pedido = ped.referencia \
		LEFT JOIN (  \
			SELECT pr.pedido, GROUP_CONCAT(DISTINCT c.referencia ORDER BY c.referencia SEPARATOR ',') AS ref_compras \
			FROM compras c \
			INNER JOIN comprecepcion cr ON cr.compra = c.referencia \
			INNER JOIN recepciones r ON r.recepcion = cr.recepcion  \
			INNER JOIN pedrecepcion pr ON pr.recepcion = r.recepcion \
			INNER JOIN terminales t ON t.terminal = c.terminal \
			INNER JOIN secciones sec ON sec.seccion = t.seccion \
			INNER JOIN sucursales suc ON suc.sucursal = sec.sucursal \
			WHERE c.cancelado = 0 AND suc.idempresa = %s \
			GROUP BY pr.pedido \
		) pedxrec ON pedxrec.pedido = ped.referencia \
		where pro.razonsocial like '%s%%' and ped.fechaped>='%s' and ped.fechaped<='%s' \
		AND suc.idempresa = %s %s \
		GROUP BY ped.referencia \
		order by pro.razonsocial, ped.fechaped, ped.referencia limit %s ",
		FormServidor->ObtieneClaveEmpresa(), FormServidor->ObtieneClaveEmpresa(),
		dato_buscado, fecha_ini, fecha_fin, FormServidor->ObtieneClaveEmpresa(), condicion_tipopedido, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="REFE"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        tipo_pedido=mFg.ExtraeStringDeBuffer(&parametros);

		if(tipo_pedido == "1"){
		   condicion_tipopedido.printf(" AND ped.compra IS NULL AND ped.cancelado = 0 ");
		}

		instruccion.sprintf("select COALESCE(cpp.ref_compras, pedxrec.ref_compras, ped.compra, '') AS compra, \
		ped.fechaped, ped.referencia, pro.razonsocial, ped.valor,\
		concat(ec.nombre, ' ', ec.appat, ' ', ec.apmat) as comprador, a.nombre,\
		concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, ped.cancelado,\
		fechavigencia from pedidos ped \
		INNER JOIN almacenes a ON ped.almacen=a.almacen \
		INNER JOIN empleados ec ON ped.comprador=ec.empleado \
		INNER JOIN empleados em ON ped.usumodi=em.empleado \
		INNER JOIN proveedores pro ON ped.proveedor=pro.proveedor \
		INNER JOIN secciones sec ON a.seccion = sec.seccion \
		INNER JOIN sucursales suc ON sec.sucursal = suc.sucursal \
		LEFT JOIN ( \
			SELECT cpp.pedido, GROUP_CONCAT(DISTINCT cpp.compra ORDER BY cpp.compra SEPARATOR ',') AS ref_compras \
			FROM compras c \
			INNER JOIN compraspedidosprov cpp ON c.referencia = cpp.compra \
			INNER JOIN terminales t ON t.terminal = c.terminal \
			INNER JOIN secciones sec ON sec.seccion = t.seccion \
			INNER JOIN sucursales suc ON suc.sucursal = sec.sucursal \
			WHERE  c.cancelado = 0 AND suc.idempresa = %s \
			GROUP BY cpp.pedido \
		) cpp ON cpp.pedido = ped.referencia \
		LEFT JOIN ( \
			SELECT pr.pedido, GROUP_CONCAT(DISTINCT c.referencia ORDER BY c.referencia SEPARATOR ',') AS ref_compras \
			FROM compras c \
			INNER JOIN comprecepcion cr ON cr.compra = c.referencia \
			INNER JOIN recepciones r ON r.recepcion = cr.recepcion \
			INNER JOIN pedrecepcion pr ON pr.recepcion = r.recepcion \
			INNER JOIN terminales t ON t.terminal = c.terminal \
			INNER JOIN secciones sec ON sec.seccion = t.seccion \
			INNER JOIN sucursales suc ON suc.sucursal = sec.sucursal \
			WHERE c.cancelado = 0 AND suc.idempresa = %s \
			GROUP BY pr.pedido \
		) pedxrec ON pedxrec.pedido = ped.referencia \
		where ped.referencia='%s' and ped.fechaped>='%s' and ped.fechaped<='%s' \
		AND suc.idempresa = %s %s \
		GROUP BY ped.referencia \
		order by pro.razonsocial, ped.fechaped, ped.referencia limit %s",
		FormServidor->ObtieneClaveEmpresa(), FormServidor->ObtieneClaveEmpresa(),
		dato_buscado, fecha_ini, fecha_fin, FormServidor->ObtieneClaveEmpresa(),condicion_tipopedido, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
    if (tipo_busqueda=="FECH"){
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		tipo_pedido=mFg.ExtraeStringDeBuffer(&parametros);

		if(tipo_pedido == "1"){
		   condicion_tipopedido.printf(" AND ped.compra IS NULL AND ped.cancelado = 0 ");
		}

		instruccion.sprintf("select COALESCE(cpp.ref_compras, pedxrec.ref_compras, ped.compra, '') AS compra, ped.fechaped, ped.referencia, pro.razonsocial, ped.valor,\
		concat(ec.nombre, ' ', ec.appat, ' ', ec.apmat) as comprador, a.nombre,\
		concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, ped.cancelado,\
		fechavigencia \
		FROM pedidos ped \
		INNER JOIN almacenes a ON ped.almacen=a.almacen \
		INNER JOIN empleados ec ON ped.comprador=ec.empleado \
		INNER JOIN empleados em ON ped.usumodi=em.empleado \
		INNER JOIN proveedores pro ON ped.proveedor=pro.proveedor \
		INNER JOIN secciones sec ON a.seccion = sec.seccion \
		INNER JOIN sucursales suc ON sec.sucursal = suc.sucursal \
		LEFT JOIN ( \
			SELECT cpp.pedido, GROUP_CONCAT(DISTINCT cpp.compra ORDER BY cpp.compra SEPARATOR ',') AS ref_compras \
			FROM compras c \
			INNER JOIN compraspedidosprov cpp ON c.referencia = cpp.compra \
			INNER JOIN terminales t ON t.terminal = c.terminal \
			INNER JOIN secciones sec ON sec.seccion = t.seccion \
			INNER JOIN sucursales suc ON suc.sucursal = sec.sucursal \
			WHERE  c.cancelado = 0 AND suc.idempresa = %s \
			GROUP BY cpp.pedido  \
		) cpp ON cpp.pedido = ped.referencia \
		LEFT JOIN ( \
			SELECT pr.pedido, GROUP_CONCAT(DISTINCT c.referencia ORDER BY c.referencia SEPARATOR ',') AS ref_compras \
			FROM compras c \
			INNER JOIN comprecepcion cr ON cr.compra = c.referencia \
			INNER JOIN recepciones r ON r.recepcion = cr.recepcion \
			INNER JOIN pedrecepcion pr ON pr.recepcion = r.recepcion \
			INNER JOIN terminales t ON t.terminal = c.terminal \
			INNER JOIN secciones sec ON sec.seccion = t.seccion \
			INNER JOIN sucursales suc ON suc.sucursal = sec.sucursal \
			WHERE c.cancelado = 0 AND suc.idempresa = %s \
			GROUP BY pr.pedido \
		) pedxrec ON pedxrec.pedido = ped.referencia \
		WHERE ped.fechaped>='%s' and ped.fechaped<='%s' \
		AND suc.idempresa = %s %s \
		GROUP BY ped.referencia \
		order by pro.razonsocial, ped.fechaped, ped.referencia limit %s",
		FormServidor->ObtieneClaveEmpresa(), FormServidor->ObtieneClaveEmpresa(),
		fecha_ini, fecha_fin, FormServidor->ObtieneClaveEmpresa(), condicion_tipopedido, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="ALMA"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        tipo_pedido=mFg.ExtraeStringDeBuffer(&parametros);

		if(tipo_pedido == "1"){
		   condicion_tipopedido.printf(" AND ped.compra IS NULL AND ped.cancelado = 0 ");
		}

		instruccion.sprintf("select COALESCE(cpp.ref_compras, pedxrec.ref_compras, ped.compra, '') AS compra, \
		ped.fechaped, ped.referencia, pro.razonsocial, ped.valor,\
		concat(ec.nombre, ' ', ec.appat, ' ', ec.apmat) as comprador, a.nombre,\
		concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, ped.cancelado,\
		fechavigencia from pedidos ped \
		INNER JOIN almacenes a ON ped.almacen=a.almacen \
		INNER JOIN empleados ec ON ped.comprador=ec.empleado \
		INNER JOIN empleados em ON ped.usumodi=em.empleado \
		INNER JOIN proveedores pro ON ped.proveedor=pro.proveedor \
		INNER JOIN secciones sec ON a.seccion = sec.seccion \
		INNER JOIN sucursales suc ON sec.sucursal = suc.sucursal \
		LEFT JOIN ( \
			SELECT cpp.pedido, GROUP_CONCAT(DISTINCT cpp.compra ORDER BY cpp.compra SEPARATOR ',') AS ref_compras \
			FROM compras c \
			INNER JOIN compraspedidosprov cpp ON c.referencia = cpp.compra \
			INNER JOIN terminales t ON t.terminal = c.terminal \
			INNER JOIN secciones sec ON sec.seccion = t.seccion \
			INNER JOIN sucursales suc ON suc.sucursal = sec.sucursal \
			WHERE  c.cancelado = 0 AND suc.idempresa = %s \
			GROUP BY cpp.pedido \
		) cpp ON cpp.pedido = ped.referencia \
		LEFT JOIN ( \
			SELECT pr.pedido, GROUP_CONCAT(DISTINCT c.referencia ORDER BY c.referencia SEPARATOR ',') AS ref_compras \
			FROM compras c \
			INNER JOIN comprecepcion cr ON cr.compra = c.referencia \
			INNER JOIN recepciones r ON r.recepcion = cr.recepcion \
			INNER JOIN pedrecepcion pr ON pr.recepcion = r.recepcion \
			INNER JOIN terminales t ON t.terminal = c.terminal \
			INNER JOIN secciones sec ON sec.seccion = t.seccion \
			INNER JOIN sucursales suc ON suc.sucursal = sec.sucursal \
			WHERE c.cancelado = 0 AND suc.idempresa = %s \
			GROUP BY pr.pedido \
		) pedxrec ON pedxrec.pedido = ped.referencia \
		WHERE a.almacen='%s' and ped.fechaped>='%s' and ped.fechaped<='%s'\
		AND suc.idempresa = %s %s \
		GROUP BY ped.referencia \
		order by pro.razonsocial, ped.fechaped, ped.referencia limit %s",
		FormServidor->ObtieneClaveEmpresa(), FormServidor->ObtieneClaveEmpresa(),
		dato_buscado, fecha_ini, fecha_fin, FormServidor->ObtieneClaveEmpresa(),condicion_tipopedido, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="COMP"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        tipo_pedido=mFg.ExtraeStringDeBuffer(&parametros);

		if(tipo_pedido == "1"){
		   condicion_tipopedido.printf(" AND ped.compra IS NULL AND ped.cancelado = 0 ");
		}

		instruccion.sprintf("select COALESCE(cpp.ref_compras, pedxrec.ref_compras, ped.compra, '') AS compra, \
		ped.fechaped, ped.referencia, pro.razonsocial, ped.valor,\
		concat(ec.nombre, ' ', ec.appat, ' ', ec.apmat) as comprador, a.nombre,\
		concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, ped.cancelado, fechavigencia \
		from pedidos ped \
		INNER JOIN almacenes a ON ped.almacen=a.almacen \
		INNER JOIN empleados ec ON ped.comprador=ec.empleado \
		INNER JOIN empleados em ON ped.usumodi=em.empleado \
		INNER JOIN proveedores pro ON ped.proveedor=pro.proveedor \
		INNER JOIN secciones sec ON a.seccion = sec.seccion \
		INNER JOIN sucursales suc ON sec.sucursal = suc.sucursal \
		LEFT JOIN ( \
			SELECT cpp.pedido, GROUP_CONCAT(DISTINCT cpp.compra ORDER BY cpp.compra SEPARATOR ',') AS ref_compras \
			FROM compras c \
			INNER JOIN compraspedidosprov cpp ON c.referencia = cpp.compra \
			INNER JOIN terminales t ON t.terminal = c.terminal \
			INNER JOIN secciones sec ON sec.seccion = t.seccion \
			INNER JOIN sucursales suc ON suc.sucursal = sec.sucursal \
			WHERE  c.cancelado = 0 AND suc.idempresa = %s \
			GROUP BY cpp.pedido 	 \
		) cpp ON cpp.pedido = ped.referencia \
		LEFT JOIN ( \
			SELECT pr.pedido, GROUP_CONCAT(DISTINCT c.referencia ORDER BY c.referencia SEPARATOR ',') AS ref_compras \
			FROM compras c \
			INNER JOIN comprecepcion cr ON cr.compra = c.referencia \
			INNER JOIN recepciones r ON r.recepcion = cr.recepcion \
			INNER JOIN pedrecepcion pr ON pr.recepcion = r.recepcion \
			INNER JOIN terminales t ON t.terminal = c.terminal \
			INNER JOIN secciones sec ON sec.seccion = t.seccion \
			INNER JOIN sucursales suc ON suc.sucursal = sec.sucursal \
			WHERE c.cancelado = 0 AND suc.idempresa = %s \
			GROUP BY pr.pedido \
		) pedxrec ON pedxrec.pedido = ped.referencia \
		where ec.empleado='%s' and ped.fechaped>='%s'\
		and ped.fechaped<='%s' AND suc.idempresa = %s %s \
		GROUP BY ped.referencia \
		order by pro.razonsocial, ped.fechaped, ped.referencia limit %s",
		FormServidor->ObtieneClaveEmpresa(), FormServidor->ObtieneClaveEmpresa(),
		dato_buscado, fecha_ini, fecha_fin, FormServidor->ObtieneClaveEmpresa(),condicion_tipopedido, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="USUA"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        tipo_pedido=mFg.ExtraeStringDeBuffer(&parametros);

		if(tipo_pedido == "1"){
		   condicion_tipopedido.printf(" AND ped.compra IS NULL AND ped.cancelado = 0 ");
		}

		instruccion.sprintf("select COALESCE(cpp.ref_compras, pedxrec.ref_compras, ped.compra, '') AS compra, \
		ped.fechaped, ped.referencia, pro.razonsocial, ped.valor,\
		concat(ec.nombre, ' ', ec.appat, ' ', ec.apmat) as comprador, a.nombre,\
		concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, ped.cancelado, fechavigencia \
		from pedidos ped \
		INNER JOIN almacenes a ON ped.almacen=a.almacen \
		INNER JOIN empleados ec ON ped.comprador=ec.empleado \
		INNER JOIN empleados em ON ped.usumodi=em.empleado \
		INNER JOIN proveedores pro ON ped.proveedor=pro.proveedor \
		INNER JOIN secciones sec ON a.seccion = sec.seccion \
		INNER JOIN sucursales suc ON sec.sucursal = suc.sucursal \
		LEFT JOIN ( \
			SELECT cpp.pedido, GROUP_CONCAT(DISTINCT cpp.compra ORDER BY cpp.compra SEPARATOR ',') AS ref_compras \
			FROM compras c \
			INNER JOIN compraspedidosprov cpp ON c.referencia = cpp.compra \
			INNER JOIN terminales t ON t.terminal = c.terminal \
			INNER JOIN secciones sec ON sec.seccion = t.seccion \
			INNER JOIN sucursales suc ON suc.sucursal = sec.sucursal \
			WHERE  c.cancelado = 0 AND suc.idempresa = %s \
			GROUP BY cpp.pedido 	 \
		) cpp ON cpp.pedido = ped.referencia \
		LEFT JOIN (  \
			SELECT pr.pedido, GROUP_CONCAT(DISTINCT c.referencia ORDER BY c.referencia SEPARATOR ',') AS ref_compras \
			FROM compras c \
			INNER JOIN comprecepcion cr ON cr.compra = c.referencia \
			INNER JOIN recepciones r ON r.recepcion = cr.recepcion \
			INNER JOIN pedrecepcion pr ON pr.recepcion = r.recepcion \
			INNER JOIN terminales t ON t.terminal = c.terminal \
			INNER JOIN secciones sec ON sec.seccion = t.seccion \
			INNER JOIN sucursales suc ON suc.sucursal = sec.sucursal \
			WHERE c.cancelado = 0 AND suc.idempresa = %s  \
			GROUP BY pr.pedido \
		) pedxrec ON pedxrec.pedido = ped.referencia \
		where em.empleado='%s' AND ped.fechaped>='%s' \
		and ped.fechaped<='%s' AND suc.idempresa = %s %s \
		GROUP BY ped.referencia \
		order by pro.razonsocial, ped.fechaped, ped.referencia limit %s",
		FormServidor->ObtieneClaveEmpresa(), FormServidor->ObtieneClaveEmpresa(),
		dato_buscado, fecha_ini, fecha_fin, FormServidor->ObtieneClaveEmpresa(),condicion_tipopedido, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }

	if (tipo_busqueda=="") {
        // Obtiene todos los posibles almacenes
        instruccion="select almacen, nombre from almacenes order by nombre";
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

        // Obtiene todos los empleados como posibles compradores
        instruccion="select empleado AS Empleado, concat(nombre,' ',appat,' ',apmat) AS Nombre from empleados order by nombre, apmat, appat";
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

        // Obtiene todos los usuarios
        instruccion="select usuarios.empleado AS Usuario, empleados.nombre AS Nombre ";
        instruccion+="from usuarios, empleados ";
        instruccion+="where empleados.empleado=usuarios.empleado order by nombre";
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
}

//---------------------------------------------------------------------------
//ID_BUSQ_NOTA_PROV
void ServidorBusquedas::BuscaNotasCredProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    // BUSQUEDA DE NOTAS DE CREDITO DE PROVEEDOR
    DatosTabla datos(mServidorVioleta->Tablas);
    AnsiString instruccion;
	AnsiString tipo_busqueda, tipo_nota, condicion_tipo_nota = " ";
    AnsiString dato_buscado, fecha_ini, fecha_fin;

	// Extrae el tipo de búsqueda
    tipo_busqueda=mFg.ExtraeStringDeBuffer(&parametros);
    // Extrae el tipo de nota de crédito que se está buscando.
	tipo_nota=mFg.ExtraeStringDeBuffer(&parametros);

    if(tipo_nota != "3"){
		condicion_tipo_nota.sprintf(" and n.tipo=%s ", tipo_nota);
	}

    mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ);

    if (tipo_busqueda=="PROV"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        instruccion.sprintf("select n.fechanot, n.referencia, n.compra, n.folioprov, \
			p.razonsocial, n.valor, concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, \
			if (n.cancelado=1, 'CANC','') as estado, if(n.tipo=0, 'Devolucion', if(n.tipo=1, 'Bonificacion', 'Descuento')) as tipo \
            from notascredprov n, compras c, empleados em, proveedores p \
            where p.razonsocial like '%s%%' and c.proveedor=p.proveedor and \
            n.compra=c.referencia and n.usumodi=em.empleado and \
			n.fechanot>='%s' and n.fechanot<='%s' %s \
			order by p.razonsocial, n.fechanot, n.referencia limit %s", dato_buscado, fecha_ini, fecha_fin, condicion_tipo_nota, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="FOLIPROV"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

        instruccion.sprintf("select n.fechanot, n.referencia, n.compra, n.folioprov, \
            p.razonsocial, n.valor, concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, \
			if (n.cancelado=1, 'CANC','') as estado, if(n.tipo=0, 'Devolucion', if(n.tipo=1, 'Bonificacion', 'Descuento')) as tipo \
            from notascredprov n, compras c, empleados em, proveedores p \
            where n.folioprov like '%s%%' and c.proveedor=p.proveedor and \
			n.compra=c.referencia and n.usumodi=em.empleado and n.fechanot>='%s' and n.fechanot<='%s' %s \
			order by p.razonsocial, n.fechanot, n.referencia limit %s", dato_buscado, fecha_ini, fecha_fin, condicion_tipo_nota, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="REFE"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

        instruccion.sprintf("select n.fechanot, n.referencia, c.referencia as compra, \
            n.folioprov, p.razonsocial, n.valor, \
            concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, \
			if (n.cancelado=1, 'CANC','') as estado, if(n.tipo=0, 'Devolucion', if(n.tipo=1, 'Bonificacion', 'Descuento')) as tipo \
            from notascredprov n, compras c, empleados em, proveedores p \
            where n.referencia='%s' and c.proveedor=p.proveedor and n.compra=c.referencia and \
			n.usumodi=em.empleado and n.fechanot>='%s' and n.fechanot<='%s' %s \
			order by p.razonsocial, n.fechanot, n.referencia limit %s", dato_buscado, fecha_ini, fecha_fin, condicion_tipo_nota, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="COMP") {
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

        instruccion.sprintf("select n.fechanot, n.referencia, c.referencia as compra, \
            n.folioprov, p.razonsocial, n.valor, \
            concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, \
			if (n.cancelado=1, 'CANC','') as estado, if(n.tipo=0, 'Devolucion', if(n.tipo=1, 'Bonificacion', 'Descuento')) as tipo \
            from notascredprov n, compras c, empleados em, proveedores p \
            where n.compra='%s' and c.proveedor=p.proveedor and n.compra=c.referencia and \
			n.usumodi=em.empleado and n.fechanot>='%s' and n.fechanot<='%s' %s \
			order by p.razonsocial, n.fechanot, n.referencia limit %s", dato_buscado, fecha_ini, fecha_fin, condicion_tipo_nota, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="FECH"){
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

        instruccion.sprintf("select n.fechanot, n.referencia, c.referencia as compra, \
            n.folioprov, p.razonsocial, n.valor, concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, \
			if (n.cancelado=1, 'CANC','') as estado, if(n.tipo=0, 'Devolucion', if(n.tipo=1, 'Bonificacion', 'Descuento')) as tipo \
            from notascredprov n, compras c, empleados em, proveedores p \
            where c.proveedor=p.proveedor and n.compra=c.referencia and \
			n.usumodi=em.empleado and n.fechanot>='%s' and n.fechanot<='%s' %s \
			order by p.razonsocial, n.fechanot, n.referencia limit %s", fecha_ini, fecha_fin, condicion_tipo_nota, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="USUA"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

        instruccion.sprintf("select n.fechanot, n.referencia, c.referencia as compra, \
            n.folioprov, p.razonsocial, n.valor, \
            concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, \
            if (n.cancelado=1, 'CANC','') as estado, if(n.tipo=0, 'Devolucion', if(n.tipo=1, 'Bonificacion', 'Descuento')) as tipo \
            from notascredprov n, compras c, empleados em, proveedores p \
            where em.empleado='%s' and c.proveedor=p.proveedor and n.compra=c.referencia and \
			n.usumodi=em.empleado and n.fechanot>='%s' and n.fechanot<='%s' %s \
			order by p.razonsocial, n.fechanot, n.referencia limit %s", dato_buscado, fecha_ini, fecha_fin, condicion_tipo_nota, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="UUID"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

        instruccion.sprintf("select n.fechanot, n.referencia, c.referencia as compra, \
            n.folioprov, p.razonsocial, n.valor, \
            concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, \
			if (n.cancelado=1, 'CANC','') as estado, if(n.tipo=0, 'Devolucion', if(n.tipo=1, 'Bonificacion', 'Descuento')) as tipo \
			from notascredprov n \
			inner join compras c ON n.compra=c.referencia \
			inner join empleados em ON n.usumodi=em.empleado  \
			inner join proveedores p ON c.proveedor=p.proveedor \
			where n.muuid LIKE '%s%%' and n.fechanot between '%s' and '%s' %s \
			order by p.razonsocial, n.fechanot, n.referencia limit %s", dato_buscado, fecha_ini, fecha_fin, condicion_tipo_nota, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}

    if (tipo_busqueda=="") {
        // Obtiene todos los usuarios
        instruccion="select usuarios.empleado AS Usuario, empleados.nombre AS Nombre ";
        instruccion+="from usuarios, empleados ";
        instruccion+="where empleados.empleado=usuarios.empleado order by nombre";
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
}

//---------------------------------------------------------------------------
//ID_BUSQ_PAGO_PROV
void ServidorBusquedas::BuscaPagosProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    // BUSQUEDA DE PAGOS DE PROVEEDOR
    DatosTabla datos(mServidorVioleta->Tablas);
    AnsiString instruccion;
    AnsiString tipo_busqueda;
    AnsiString dato_buscado, dato_buscado2, fecha_ini, fecha_fin;

    // Extrae el tipo de búsqueda
    tipo_busqueda=mFg.ExtraeStringDeBuffer(&parametros);
    mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ);

    if (tipo_busqueda=="PROV"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

        instruccion.sprintf("select pag.pago, pag.ident, pag.fecha, if(pro.tipoempre=0,replegal,razonsocial) as nombre, \
			CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS usuario , pag.valor, \
            if(cancelado=1, 'CANC','') as estado \
			from pagosprov pag, proveedores pro , empleados e, terminales ter, secciones sec, sucursales suc \
            where pag.proveedor=pro.proveedor and \
            pro.razonsocial like '%s%%' and \
			pag.fecha>='%s' and pag.fecha<='%s' AND e.empleado=pag.usualta \
			AND ter.terminal = pag.terminal AND sec.seccion = ter.seccion AND suc.sucursal = sec.sucursal \
            AND suc.idempresa = %s \
            order by pro.razonsocial, pag.fecha, pag.pago limit %s",
				dato_buscado, fecha_ini, fecha_fin, FormServidor->ObtieneClaveEmpresa(), NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="IDEN"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

        instruccion.sprintf("select pag.pago, pag.ident, pag.fecha, if(pro.tipoempre=0,replegal,razonsocial) as nombre, \
			CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS usuario, pag.valor,\
            if(cancelado=1, 'CANC','') as estado \
			from pagosprov pag, proveedores pro , empleados e, terminales ter, secciones sec, sucursales suc \
            where pag.proveedor=pro.proveedor and \
            pag.ident like '%s%%' and \
			pag.fecha>='%s' and pag.fecha<='%s' AND e.empleado=pag.usualta \
            AND ter.terminal = pag.terminal AND sec.seccion = ter.seccion AND suc.sucursal = sec.sucursal \
            AND suc.idempresa = %s \
            order by pag.ident, pro.razonsocial, pag.fecha, pag.pago limit %s",
                dato_buscado, fecha_ini, fecha_fin, FormServidor->ObtieneClaveEmpresa(), NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="FOLI"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

        instruccion.sprintf("select pag.pago, pag.ident, pag.fecha, if(pro.tipoempre=0,replegal,razonsocial) as nombre, \
			CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS usuario , pag.valor,\
            if(cancelado=1, 'CANC','') as estado \
			from pagosprov pag, proveedores pro , empleados e, terminales ter, secciones sec, sucursales suc \
            where pag.proveedor=pro.proveedor and \
            pag.pago like '%s%%' and \
			pag.fecha>='%s' and pag.fecha<='%s' AND e.empleado=pag.usualta \
            AND ter.terminal = pag.terminal AND sec.seccion = ter.seccion AND suc.sucursal = sec.sucursal \
            AND suc.idempresa = %s \
            order by pag.pago, pro.razonsocial, pag.fecha limit %s",
                dato_buscado, fecha_ini, fecha_fin, FormServidor->ObtieneClaveEmpresa(), NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="FECH"){
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

        instruccion.sprintf("select pag.pago, pag.ident, pag.fecha, if(pro.tipoempre=0,replegal,razonsocial) as nombre, \
			CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS usuario , pag.valor,\
            if(cancelado=1, 'CANC','') as estado \
			from pagosprov pag, proveedores pro , empleados e, terminales ter, secciones sec, sucursales suc \
            where pag.proveedor=pro.proveedor and \
			pag.fecha>='%s' and pag.fecha<='%s' AND e.empleado=pag.usualta \
            AND ter.terminal = pag.terminal AND sec.seccion = ter.seccion AND suc.sucursal = sec.sucursal \
            AND suc.idempresa = %s \
            order by pro.razonsocial, pag.fecha, pag.pago limit %s",
                fecha_ini, fecha_fin, FormServidor->ObtieneClaveEmpresa(), NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
    if (tipo_busqueda=="CHEQ"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros); // BANCO
        dato_buscado2=mFg.ExtraeStringDeBuffer(&parametros); // NUM CHEQUE
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

        instruccion.sprintf("select pag.pago, pag.ident, pag.fecha, if(pro.tipoempre=0,replegal,razonsocial) as nombre, \
			CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS usuario , pag.valor, \
            if(cancelado=1, 'CANC','') as estado \
            from pagosprov pag, proveedores pro, transxpag tra, \
			cheqxpag chxp, chequesproveedores chpro, empleados e, terminales ter, secciones sec, sucursales suc \
            where pag.proveedor=pro.proveedor and \
            tra.pago=pag.pago and chxp.pago=tra.pago and \
            chxp.chequeprov=chpro.chequeprov and chpro.banco='%s' and \
            chpro.folio like '%s%%' and \
			pag.fecha>='%s' and pag.fecha<='%s' AND e.empleado=pag.usualta \
            AND ter.terminal = pag.terminal AND sec.seccion = ter.seccion AND suc.sucursal = sec.sucursal \
            AND suc.idempresa = %s \
            order by chpro.folio, pag.pago limit %s",
                dato_buscado, dato_buscado2, fecha_ini, fecha_fin, FormServidor->ObtieneClaveEmpresa(), NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="COMP"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select pag.pago, pag.ident, pag.fecha, if(pro.tipoempre=0,replegal,razonsocial) as nombre, \
            CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS usuario , pag.valor,\
            if(pag.cancelado=1, 'CANC','') as estado \
            from pagosprov pag, proveedores pro, transxpag tra, \
			compras com , empleados e, terminales ter, secciones sec, sucursales suc \
            where pag.proveedor=pro.proveedor and \
            tra.pago=pag.pago and tra.referencia=com.referencia and \
			com.referencia like '%s%%' and \
			pag.fecha>='%s' and pag.fecha<='%s' AND e.empleado=pag.usualta \
            AND ter.terminal = pag.terminal AND sec.seccion = ter.seccion AND suc.sucursal = sec.sucursal \
            AND suc.idempresa = %s \
            order by pag.pago, pro.razonsocial, pag.fecha limit %s",
                dato_buscado, fecha_ini, fecha_fin, FormServidor->ObtieneClaveEmpresa(), NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }

    if (tipo_busqueda=="") {
        // Obtiene todos los bancos
        instruccion="select banco, nombre from bancos order by nombre, banco";
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
}

//---------------------------------------------------------------------------
//ID_BUSQ_PEDIDO_CLI
void ServidorBusquedas::BuscaPedidosCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    // BUSQUEDA DE PEDIDOS DE CLIENTE
    DatosTabla datos(mServidorVioleta->Tablas);
    AnsiString instruccion;
    AnsiString tipo_busqueda;
	AnsiString dato_buscado, fecha_ini, fecha_fin, sucursal;
	AnsiString soloSinFacturar,origenVenta,condicionOrigenVenta;

	// Extrae el tipo de búsqueda
	tipo_busqueda=mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ);

	if (tipo_busqueda=="PEDS"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);

		instruccion.sprintf("select p.fechaped, p.referencia, \
			c.rsocial as nomcliente, \
			c.nomnegocio, \
			p.valor, \
			concat(ev.nombre, ' ', ev.appat, ' ', ev.apmat) as nomvendedor, \
			if(p.cotizacion=1, 'COTIZACION', 'PEDIDO') as tipo, \
			p.cancelado, if(t.esmovil=1, 'VTA MOVIL', 'NORMAL') as VentaMovil, if (p.facturado=1, 'Sí', 'No') as estafacturado, \
			p.tipoorigen, \
			ifnull(pk.kit,''),ifnull(k.nombre,'') \
			from pedidosventa p \
			inner join clientes c on p.cliente=c.cliente \
			inner join empleados ev on p.vendedor=ev.empleado  \
			inner join terminales t on t.terminal=p.terminal   \
			inner join secciones sec on t.seccion=sec.seccion  \
			left join pedidoskits pk ON pk.pedido=p.referencia \
			left join kits k ON k.kit=pk.kit \
			where p.referencia='%s' and p.cliente=c.cliente and \
			t.terminal=p.terminal and p.facturado=0 and p.cancelado=0 \
			order by p.referencia", dato_buscado);
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}

	if (tipo_busqueda=="REFE"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select p.fechaped, p.referencia, \
			c.rsocial as nomcliente,  \
			c.nomnegocio, \
			p.valor, \
			concat(ev.nombre, ' ', ev.appat, ' ', ev.apmat) as nomvendedor, \
			if(p.cotizacion=1, 'COTIZACION', 'PEDIDO') as tipo, \
			p.cancelado, if(t.esmovil=1, 'VTA MOVIL', 'NORMAL') as VentaMovil, if (p.facturado=1, 'Sí', 'No') as estafacturado, \
			p.tipoorigen, \
			ifnull(pk.kit,''),ifnull(k.nombre,'') \
			from pedidosventa p \
			inner join clientes c on p.cliente=c.cliente \
			inner join empleados ev on p.vendedor=ev.empleado \
			inner join terminales t on t.terminal=p.terminal  \
			inner join secciones sec on t.seccion=sec.seccion \
			left join pedidoskits pk ON pk.pedido=p.referencia \
			left join kits k ON k.kit=pk.kit \
			where p.referencia='%s' and p.cliente=c.cliente and \
			t.terminal=p.terminal and \
			p.fechaped>='%s' and p.fechaped<='%s' \
			order by p.referencia limit %s", dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="FECH"){
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		sucursal=mFg.ExtraeStringDeBuffer(&parametros);

		instruccion.sprintf("select p.fechaped, p.referencia, \
			c.rsocial as nomcliente, \
			c.nomnegocio, \
			p.valor, \
			concat(ev.nombre, ' ', ev.appat, ' ', ev.apmat) as nomvendedor, \
			if(p.cotizacion=1, 'COTIZACION', 'PEDIDO') as tipo, \
			p.cancelado, if(t.esmovil=1, 'VTA MOVIL', 'NORMAL') as VentaMovil, if (p.facturado=1, 'Sí', 'No') as estafacturado, \
			p.tipoorigen, \
			ifnull(pk.kit,''),ifnull(k.nombre,'') \
			from pedidosventa p\
			inner join clientes c on p.cliente=c.cliente \
			inner join empleados ev on p.vendedor=ev.empleado \
			inner join terminales t on t.terminal=p.terminal   \
			inner join secciones sec on t.seccion=sec.seccion  \
			left join pedidoskits pk ON pk.pedido=p.referencia \
			left join kits k ON k.kit=pk.kit \
			where sec.sucursal='%s' \
			and p.fechaped>='%s' and p.fechaped<='%s' \
			order by p.fechaped, p.referencia limit %s", sucursal, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="CAJE"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		sucursal=mFg.ExtraeStringDeBuffer(&parametros);

		instruccion.sprintf("select p.fechaped, p.referencia, \
			c.rsocial as nomcliente, \
			c.nomnegocio, \
			p.valor, \
			concat(ev.nombre, ' ', ev.appat, ' ', ev.apmat) as nomvendedor, \
			if(p.cotizacion=1, 'COTIZACION', 'PEDIDO') as tipo, \
			p.cancelado, if(t.esmovil=1, 'VTA MOVIL', 'NORMAL') as VentaMovil, if (p.facturado=1, 'Sí', 'No') as estafacturado, \
			p.tipoorigen, \
			ifnull(pk.kit,''),ifnull(k.nombre,'') \
			from pedidosventa p \
			inner join clientes c on p.cliente=c.cliente     \
			inner join empleados ev on p.vendedor=ev.empleado \
			inner join terminales t on t.terminal=p.terminal  \
			inner join secciones sec on t.seccion=sec.seccion \
			left join pedidoskits pk ON pk.pedido=p.referencia \
			left join kits k ON k.kit=pk.kit \
			where sec.sucursal='%s' and p.usumodi='%s' and p.cliente=c.cliente and \
			t.terminal=p.terminal and  t.seccion=sec.seccion and \
			p.fechaped>='%s' and p.fechaped<='%s' \
			order by nomcliente, p.fechaped, p.referencia limit %s", sucursal, dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="VEND"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		sucursal=mFg.ExtraeStringDeBuffer(&parametros);

		instruccion.sprintf("select p.fechaped, p.referencia, \
			c.rsocial as nomcliente, \
			c.nomnegocio, \
			p.valor, \
			concat(ev.nombre, ' ', ev.appat, ' ', ev.apmat) as nomvendedor, \
			if(p.cotizacion=1, 'COTIZACION', 'PEDIDO') as tipo, \
			p.cancelado, if(t.esmovil=1, 'VTA MOVIL', 'NORMAL') as VentaMovil, if (p.facturado=1, 'Sí', 'No') as estafacturado, \
			p.tipoorigen, \
			ifnull(pk.kit,''),ifnull(k.nombre,'') \
			from pedidosventa p \
			inner join clientes c on p.cliente=c.cliente \
			inner join empleados ev on p.vendedor=ev.empleado \
			inner join terminales t on t.terminal=p.terminal  \
			inner join secciones sec on t.seccion=sec.seccion  \
			left join pedidoskits pk ON pk.pedido=p.referencia \
			left join kits k ON k.kit=pk.kit \
			where sec.sucursal='%s' and p.vendedor='%s' and p.cliente=c.cliente and \
			t.terminal=p.terminal and  t.seccion=sec.seccion and \
			p.fechaped>='%s' and p.fechaped<='%s' \
			order by p.fechaped, p.referencia limit %s", sucursal, dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="CLI"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		sucursal=mFg.ExtraeStringDeBuffer(&parametros);

		instruccion.sprintf("select p.fechaped, p.referencia, \
			c.rsocial as nomcliente, \
			c.nomnegocio, \
			p.valor, \
			concat(ev.nombre, ' ', ev.appat, ' ', ev.apmat) as nomvendedor, \
			if(p.cotizacion=1, 'COTIZACION', 'PEDIDO') as tipo, \
			p.cancelado, if(t.esmovil=1, 'VTA MOVIL', 'NORMAL') as VentaMovil, if (p.facturado=1, 'Sí', 'No') as estafacturado, \
			p.tipoorigen, \
			ifnull(pk.kit,''),ifnull(k.nombre,'') \
			from pedidosventa p \
			inner join clientes c on p.cliente=c.cliente \
			inner join empleados ev on p.vendedor=ev.empleado \
			inner join terminales t on t.terminal=p.terminal  \
			inner join secciones sec on t.seccion=sec.seccion \
			left join pedidoskits pk ON pk.pedido=p.referencia \
			left join kits k ON k.kit=pk.kit \
			where sec.sucursal='%s' and p.cliente='%s' and p.cliente=c.cliente and \
			t.terminal=p.terminal and  t.seccion=sec.seccion and \
			p.fechaped>='%s' and p.fechaped<='%s' \
			order by nomcliente, p.fechaped, p.referencia limit %s", sucursal, dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}

	if (tipo_busqueda=="") {
		// Obtiene todos los empleados como posibles cajeros
		instruccion="select empleado AS Empleado, concat(nombre,' ',appat,' ',apmat) AS Nombre from empleados order by nombre, apmat, appat";
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

		// Obtiene todos los Agentes de venta
		instruccion="select emp.empleado AS Empleado, concat(emp.nombre,' ',emp.appat,' ',emp.apmat) AS Nombre \
			from vendedores v, empleados emp where v.empleado=emp.empleado   \
			order by emp.nombre, emp.apmat, emp.appat ";
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="FECH_SF"){
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		sucursal=mFg.ExtraeStringDeBuffer(&parametros);
		soloSinFacturar=mFg.ExtraeStringDeBuffer(&parametros);
		origenVenta=mFg.ExtraeStringDeBuffer(&parametros);
		if (origenVenta!="") {
			condicionOrigenVenta.sprintf("and p.tipoorigen='%s' ",origenVenta);
		}else{
			condicionOrigenVenta=" ";
		}

		instruccion.sprintf("select p.fechaped, p.referencia, \
			c.rsocial as nomcliente, \
			c.nomnegocio, \
			p.valor, \
			concat(ev.nombre, ' ', ev.appat, ' ', ev.apmat) as nomvendedor, \
			if(p.cotizacion=1, 'COTIZACION', 'PEDIDO') as tipo, \
			p.cancelado, if(t.esmovil=1, 'VTA MOVIL', 'NORMAL') as VentaMovil, if (p.facturado=1, 'Sí', 'No') as estafacturado, \
			p.tipoorigen, \
			ifnull(pk.kit,''),ifnull(k.nombre,'') \
			from pedidosventa p \
			inner join clientes c on p.cliente=c.cliente \
			inner join empleados ev on p.vendedor=ev.empleado \
			inner join terminales t on t.terminal=p.terminal \
			inner join secciones sec on t.seccion=sec.seccion\
			left join pedidoskits pk ON pk.pedido=p.referencia \
			left join kits k ON k.kit=pk.kit \
			where sec.sucursal='%s' \
			and p.fechaped>='%s' and p.fechaped<='%s' \
			and p.cancelado=0 \
			and p.facturado=%s %s\
			order by p.fechaped, p.referencia desc limit %s", sucursal, fecha_ini, fecha_fin, soloSinFacturar ,condicionOrigenVenta,NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="KIT"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		sucursal=mFg.ExtraeStringDeBuffer(&parametros);

		instruccion.sprintf("select p.fechaped, p.referencia, \
			c.rsocial as nomcliente, \
			c.nomnegocio, \
			p.valor, \
			concat(ev.nombre, ' ', ev.appat, ' ', ev.apmat) as nomvendedor, \
			if(p.cotizacion=1, 'COTIZACION', 'PEDIDO') as tipo, \
			p.cancelado, if(t.esmovil=1, 'VTA MOVIL', 'NORMAL') as VentaMovil, if (p.facturado=1, 'Sí', 'No') as estafacturado, \
			p.tipoorigen, \
			ifnull(pk.kit,''),ifnull(k.nombre,'') \
			from pedidosventa p \
			inner join clientes c on p.cliente=c.cliente \
			inner join empleados ev on p.vendedor=ev.empleado  \
			inner join terminales t on t.terminal=p.terminal   \
			inner join secciones sec on t.seccion=sec.seccion  \
			left join pedidoskits pk ON pk.pedido=p.referencia \
			left join kits k ON k.kit=pk.kit \
			where sec.sucursal='%s'  \
			and p.fechaped>='%s' and p.fechaped<='%s' and pk.kit='%s'\
			order by nomcliente, p.fechaped, p.referencia limit %s", sucursal, fecha_ini, fecha_fin, dato_buscado, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
}

//---------------------------------------------------------------------------
//ID_BUSQ_NOTA_CLI
void ServidorBusquedas::BuscaNotasCredCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// BUSQUEDA DE NOTAS DE CREDITO DE CLIENTE
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instruccion;
	AnsiString tipo_busqueda, tipo_nota;
	AnsiString dato_buscado, fecha_ini, fecha_fin, sucursal, condicion_tipo_nota =" ";

	// Extrae el tipo de búsqueda
	tipo_busqueda=mFg.ExtraeStringDeBuffer(&parametros);
    //Extrae el tipo de nota de crédito que se está buscando
	tipo_nota=mFg.ExtraeStringDeBuffer(&parametros);

	if(tipo_nota != "3"){
		condicion_tipo_nota.sprintf(" and n.tipo=%s ", tipo_nota);
	}

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ);

	if (tipo_busqueda=="CLI"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		sucursal=mFg.ExtraeStringDeBuffer(&parametros);

		instruccion.sprintf("select n.fechanot, n.referencia, n.venta, n.foliofisic, \
			cfd.seriefolio as foliocfd, \
			c.rsocial, n.valor, concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, \
			if (n.cancelado=1, 'CANC','') as estado, if(n.tipo=0, 'Devolucion', if(n.tipo=1, 'Bonificacion', 'Descuento')) as tipo\
			from notascredcli n inner join ventas v inner join empleados em inner join clientes c inner join terminales t inner join secciones sec \
			left join cfd on n.referencia=cfd.referencia and cfd.tipocomprobante='NCRE' \
			where sec.sucursal='%s' and c.rsocial like '%s%%' and \
			v.cliente=c.cliente and \
			n.terminal=t.terminal and t.seccion=sec.seccion and \
			n.venta=v.referencia and n.usumodi=em.empleado and \
			n.fechanot>='%s' and n.fechanot<='%s' %s \
			order by c.rsocial, n.fechanot, n.referencia limit %s", sucursal, dato_buscado, fecha_ini, fecha_fin, condicion_tipo_nota, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="FOLIFISI"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("(select n.fechanot, n.referencia, n.venta, n.foliofisic, \
			cfd.seriefolio as foliocfd, \
			c.rsocial, n.valor, concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, \
			if (n.cancelado=1, 'CANC','') as estado, if(n.tipo=0, 'Devolucion', if(n.tipo=1, 'Bonificacion', 'Descuento')) as tipo \
			from notascredcli n inner join  ventas v inner join  empleados em inner join clientes c \
			left join cfd on n.referencia=cfd.referencia and cfd.tipocomprobante='NCRE' \
			where n.foliofisic='%s' and v.cliente=c.cliente and \
			n.venta=v.referencia and n.usumodi=em.empleado and \
			n.fechanot>='%s' and n.fechanot<='%s' %s ) \
			UNION \
			(select n.fechanot, n.referencia, n.venta, n.foliofisic, \
			cfd.seriefolio as foliocfd, \
			c.rsocial, n.valor, concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, \
			if (n.cancelado=1, 'CANC','') as estado, if(n.tipo=0, 'Devolucion', if(n.tipo=1, 'Bonificacion', 'Descuento')) as tipo \
			from cfd \
			inner join notascredcli n on n.referencia=cfd.referencia \
			inner join ventas v inner join  empleados em inner join clientes c \
			where cfd.seriefolio='%s' and cfd.tipocomprobante='NCRE' and v.cliente=c.cliente and \
			n.venta=v.referencia and n.usumodi=em.empleado and \
			n.fechanot>='%s' and n.fechanot<='%s' %s ) \
			order by rsocial, fechanot, referencia limit %s",
			dato_buscado, fecha_ini, fecha_fin, condicion_tipo_nota,
			dato_buscado, fecha_ini, fecha_fin, condicion_tipo_nota,
			NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}

	if (tipo_busqueda=="REFE"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select n.fechanot, n.referencia, n.venta, n.foliofisic, \
			cfd.seriefolio as foliocfd, \
			c.rsocial, n.valor, concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, \
			if (n.cancelado=1, 'CANC','') as estado , if(n.tipo=0, 'Devolucion', if(n.tipo=1, 'Bonificacion', 'Descuento')) as tipo \
			from notascredcli n inner join ventas v inner join empleados em inner join clientes c \
			left join cfd on n.referencia=cfd.referencia and cfd.tipocomprobante='NCRE' \
			where n.referencia='%s' and v.cliente=c.cliente and \
			n.venta=v.referencia and n.usumodi=em.empleado and \
			n.fechanot>='%s' and n.fechanot<='%s' %s \
			order by c.rsocial, n.fechanot, n.referencia limit %s", dato_buscado, fecha_ini, fecha_fin, condicion_tipo_nota, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="VENT") {
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select n.fechanot, n.referencia, n.venta, n.foliofisic, \
			cfd.seriefolio as foliocfd, \
			c.rsocial, n.valor, concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, \
			if (n.cancelado=1, 'CANC','') as estado, if(n.tipo=0, 'Devolucion', if(n.tipo=1, 'Bonificacion', 'Descuento')) as tipo \
			from notascredcli n inner join ventas v inner join empleados em inner join clientes c \
			left join cfd on n.referencia=cfd.referencia and cfd.tipocomprobante='NCRE' \
			where n.venta='%s' and v.cliente=c.cliente and \
			n.venta=v.referencia and n.usumodi=em.empleado and \
			n.fechanot>='%s' and n.fechanot<='%s' %s \
			order by c.rsocial, n.fechanot, n.referencia limit %s", dato_buscado, fecha_ini, fecha_fin, condicion_tipo_nota, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="FECH"){
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		sucursal=mFg.ExtraeStringDeBuffer(&parametros);

		instruccion.sprintf("select n.fechanot, n.referencia, n.venta, n.foliofisic, \
			cfd.seriefolio as foliocfd, \
			c.rsocial, n.valor, concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, \
			if (n.cancelado=1, 'CANC','') as estado, if(n.tipo=0, 'Devolucion', if(n.tipo=1, 'Bonificacion', 'Descuento')) as tipo \
			from notascredcli n inner join ventas v inner join empleados em inner join clientes c inner join terminales t inner join secciones sec \
			left join cfd on n.referencia=cfd.referencia and cfd.tipocomprobante='NCRE' \
			where sec.sucursal='%s' and v.cliente=c.cliente and \
			n.terminal=t.terminal and t.seccion=sec.seccion and \
			n.venta=v.referencia and n.usumodi=em.empleado and \
			n.fechanot>='%s' and n.fechanot<='%s' %s \
			order by c.rsocial, n.fechanot, n.referencia limit %s", sucursal, fecha_ini, fecha_fin, condicion_tipo_nota, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="USUA"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		sucursal=mFg.ExtraeStringDeBuffer(&parametros);

		instruccion.sprintf("select n.fechanot, n.referencia, n.venta, n.foliofisic, \
			cfd.seriefolio as foliocfd, \
			c.rsocial, n.valor, concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, \
			if (n.cancelado=1, 'CANC','') as estado, if(n.tipo=0, 'Devolucion', if(n.tipo=1, 'Bonificacion', 'Descuento')) as tipo \
			from notascredcli n inner join ventas v inner join empleados em inner join clientes c inner join terminales t inner join secciones sec \
			left join cfd on n.referencia=cfd.referencia and cfd.tipocomprobante='NCRE' \
			where sec.sucursal='%s' and em.empleado='%s' and v.cliente=c.cliente and \
			n.terminal=t.terminal and t.seccion=sec.seccion and \
			n.venta=v.referencia and n.usumodi=em.empleado and \
			n.fechanot>='%s' and n.fechanot<='%s' %s \
			order by c.rsocial, n.fechanot, n.referencia limit %s", sucursal, dato_buscado, fecha_ini, fecha_fin, condicion_tipo_nota, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
    if (tipo_busqueda=="PROD"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		sucursal=mFg.ExtraeStringDeBuffer(&parametros);

		instruccion.sprintf("\
			select\
				n.fechanot,\
				n.referencia,\
				n.venta,\
				n.foliofisic,\
				cfd.seriefolio as foliocfd, \
				c.rsocial,\
				n.valor,\
				concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, \
				if (n.cancelado=1, 'CANC','') as estado,\
				if(n.tipo=0, 'Devolucion',\
				if(n.tipo=1, 'Bonificacion', 'Descuento')) as tipo \
				\
			from notascredcli n\
				inner join ventas v\
				inner join empleados em \
				inner join clientes c \
				inner join terminales t \
				inner join secciones sec \
				inner join dventas dv on dv.referencia=v.referencia\
				inner join articulos art on art.articulo=dv.articulo\
				inner join productos p on p.producto=art.producto\
				left join cfd on n.referencia=cfd.referencia and cfd.tipocomprobante='NCRE' \
			where p.nombre like '%s%%' \
				and sec.sucursal='%s' \
				and v.cliente=c.cliente\
				and n.terminal=t.terminal \
				and t.seccion=sec.seccion \
				and n.venta=v.referencia \
				and n.usumodi=em.empleado \
				and n.fechanot>='%s' \
				and n.fechanot<='%s'\
			order by c.rsocial, n.fechanot, n.referencia limit %s",
			dato_buscado,sucursal, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);


		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
    if (tipo_busqueda=="CLV_PROD"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		sucursal=mFg.ExtraeStringDeBuffer(&parametros);

		instruccion.sprintf("\
			select\
				n.fechanot,\
				n.referencia,\
				n.venta,\
				n.foliofisic,\
				cfd.seriefolio as foliocfd, \
				c.rsocial,\
				n.valor,\
				concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, \
				if (n.cancelado=1, 'CANC','') as estado,\
				if(n.tipo=0, 'Devolucion',\
				if(n.tipo=1, 'Bonificacion', 'Descuento')) as tipo \
				\
			from notascredcli n\
				inner join ventas v\
				inner join empleados em \
				inner join clientes c \
				inner join terminales t \
				inner join secciones sec \
				inner join dventas dv on dv.referencia=v.referencia\
				inner join articulos art on art.articulo=dv.articulo\
				inner join productos p on p.producto=art.producto\
				left join cfd on n.referencia=cfd.referencia and cfd.tipocomprobante='NCRE' \
			where art.producto like '%s%%' \
				and sec.sucursal='%s' \
				and v.cliente=c.cliente\
				and n.terminal=t.terminal \
				and t.seccion=sec.seccion \
				and n.venta=v.referencia \
				and n.usumodi=em.empleado \
				and n.fechanot>='%s' \
				and n.fechanot<='%s'\
			order by c.rsocial, n.fechanot, n.referencia limit %s",
			dato_buscado,sucursal, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);


		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="UI"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		sucursal=mFg.ExtraeStringDeBuffer(&parametros);

		instruccion.sprintf("\
			select\
				n.fechanot,\
				n.referencia,\
				n.venta,\
				n.foliofisic,\
				cfd.seriefolio as foliocfd, \
				c.rsocial,\
				n.valor,\
				concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, \
				if (n.cancelado=1, 'CANC','') as estado,\
				if(n.tipo=0, 'Devolucion',\
				if(n.tipo=1, 'Bonificacion', 'Descuento')) as tipo \
				\
			from notascredcli n\
				inner join ventas v\
				inner join empleados em \
				inner join clientes c \
				inner join terminales t \
				inner join secciones sec \
				inner join dventas dv on dv.referencia=v.referencia\
				inner join articulos art on art.articulo=dv.articulo\
				inner join productos p on p.producto=art.producto\
				left join cfd on n.referencia=cfd.referencia and cfd.tipocomprobante='NCRE' \
			where cfd.muuid LIKE '%s%%' \
				and sec.sucursal='%s' \
				and v.cliente=c.cliente\
				and n.terminal=t.terminal \
				and t.seccion=sec.seccion \
				and n.venta=v.referencia \
				and n.usumodi=em.empleado \
			order by c.rsocial, n.fechanot, n.referencia limit 1",
			dato_buscado,sucursal/*, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ*/);


		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}

	if (tipo_busqueda=="") {
		// Obtiene todos los usuarios
		instruccion="select usuarios.empleado AS Usuario, empleados.nombre AS Nombre ";
		instruccion+="from usuarios, empleados ";
		instruccion+="where empleados.empleado=usuarios.empleado order by nombre";
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
}

//---------------------------------------------------------------------------
//ID_BUSQ_CARGO_CLI
void ServidorBusquedas::BuscaNotasCargCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// BUSQUEDA DE NOTAS DE CARGO DE CLIENTES
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instruccion;
	AnsiString tipo_busqueda;
	AnsiString dato_buscado, dato_buscado2, fecha_ini, fecha_fin;

	// Extrae el tipo de búsqueda
	tipo_busqueda=mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ);

	if (tipo_busqueda=="CLI"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select n.fechanot, n.referencia, n.foliofisic, \
			cfd.seriefolio as foliocfd, \
			v.referencia as venta, chc.chequecli as cheque,  \
			c.rsocial, t.valor, concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, \
			n.cancelado as estado \
			from notascarcli n  inner join  empleados em  inner join  clientes c  inner join  transxcob t  inner join  ventas v \
			left join pagoscli pag on pag.pago=n.pago \
			left join cheqxcob chxc on chxc.pago=pag.pago \
			left join chequesclientes chc on chc.chequecli=chxc.chequecli \
			left join cfd on n.referencia=cfd.referencia and cfd.tipocomprobante='NCAR' \
			where c.rsocial like '%s%%' and n.cliente=c.cliente and \
			n.usumodi=em.empleado and \
			n.fechanot>='%s' and n.fechanot<='%s' and \
			t.notacar=n.referencia and \
			t.tipo<>'CHDE' and \
			t.referencia=v.referencia \
			order by c.rsocial, n.fechanot, n.referencia limit %s", dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="FOLIFISI"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("(select n.fechanot, n.referencia, n.foliofisic, \
			cfd.seriefolio as foliocfd, \
			v.referencia as venta, chc.chequecli as cheque,  \
			c.rsocial, t.valor, concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, \
			n.cancelado as estado \
			from notascarcli n  inner join  empleados em  inner join  clientes c  inner join  transxcob t  inner join  ventas v \
			left join pagoscli pag on pag.pago=n.pago \
			left join cheqxcob chxc on chxc.pago=pag.pago \
			left join chequesclientes chc on chc.chequecli=chxc.chequecli \
			left join cfd on n.referencia=cfd.referencia and cfd.tipocomprobante='NCAR' \
			where n.foliofisic='%s' and n.cliente=c.cliente and \
			n.usumodi=em.empleado and \
			n.fechanot>='%s' and n.fechanot<='%s' and \
			t.notacar=n.referencia and \
			t.tipo<>'CHDE' and \
			t.referencia=v.referencia) \
			UNION \
			(select n.fechanot, n.referencia, n.foliofisic, \
			cfd.seriefolio as foliocfd, \
			v.referencia as venta, chc.chequecli as cheque,  \
			c.rsocial, t.valor, concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, \
			n.cancelado as estado \
			from cfd \
			inner join notascarcli n ON n.referencia=cfd.referencia \
			inner join  empleados em  inner join  clientes c  inner join  transxcob t  inner join  ventas v \
			left join pagoscli pag on pag.pago=n.pago \
			left join cheqxcob chxc on chxc.pago=pag.pago \
			left join chequesclientes chc on chc.chequecli=chxc.chequecli \
			where cfd.seriefolio='%s' and cfd.tipocomprobante='NCAR' and n.cliente=c.cliente and \
			n.usumodi=em.empleado and \
			n.fechanot>='%s' and n.fechanot<='%s' and \
			t.notacar=n.referencia and \
			t.tipo<>'CHDE' and \
			t.referencia=v.referencia) \
			order by rsocial, fechanot, referencia limit %s",
			dato_buscado, fecha_ini, fecha_fin,
			dato_buscado, fecha_ini, fecha_fin,
			NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="REFE"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select n.fechanot, n.referencia, n.foliofisic, \
			cfd.seriefolio as foliocfd, \
			v.referencia as venta, chc.chequecli as cheque,  \
			c.rsocial, t.valor, concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, \
			n.cancelado as estado \
			from notascarcli n  inner join  empleados em  inner join  clientes c  inner join  transxcob t  inner join  ventas v \
			left join pagoscli pag on pag.pago=n.pago \
			left join cheqxcob chxc on chxc.pago=pag.pago \
			left join chequesclientes chc on chc.chequecli=chxc.chequecli \
			left join cfd on n.referencia=cfd.referencia and cfd.tipocomprobante='NCAR' \
			where n.referencia='%s' and n.cliente=c.cliente and \
			n.usumodi=em.empleado and \
			n.fechanot>='%s' and n.fechanot<='%s' and \
			t.notacar=n.referencia and \
			t.tipo<>'CHDE' and \
			t.referencia=v.referencia \
			order by c.rsocial, n.fechanot, n.referencia limit %s", dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);

		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="VENT") {
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select n.fechanot, n.referencia, n.foliofisic, \
			cfd.seriefolio as foliocfd, \
			v.referencia as venta, chc.chequecli as cheque,  \
			c.rsocial, t.valor, concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, \
			n.cancelado as estado \
			from notascarcli n  inner join  empleados em  inner join  clientes c  inner join  transxcob t  inner join  ventas v \
			left join pagoscli pag on pag.pago=n.pago \
			left join cheqxcob chxc on chxc.pago=pag.pago \
			left join chequesclientes chc on chc.chequecli=chxc.chequecli \
			left join cfd on n.referencia=cfd.referencia and cfd.tipocomprobante='NCAR' \
			where v.referencia='%s' and n.cliente=c.cliente and \
			n.usumodi=em.empleado and \
			n.fechanot>='%s' and n.fechanot<='%s' and \
			t.notacar=n.referencia and \
			t.tipo<>'CHDE' and \
			t.referencia=v.referencia \
			order by c.rsocial, n.fechanot, n.referencia limit %s", dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="FECH"){
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select n.fechanot, n.referencia, n.foliofisic, \
			cfd.seriefolio as foliocfd, \
			v.referencia as venta, chc.chequecli as cheque,  \
			c.rsocial, t.valor, concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, \
			n.cancelado as estado \
			from notascarcli n  inner join  empleados em  inner join  clientes c  inner join  transxcob t  inner join  ventas v \
			left join pagoscli pag on pag.pago=n.pago \
			left join cheqxcob chxc on chxc.pago=pag.pago \
			left join chequesclientes chc on chc.chequecli=chxc.chequecli \
			left join cfd on n.referencia=cfd.referencia and cfd.tipocomprobante='NCAR' \
			where n.cliente=c.cliente and \
			n.usumodi=em.empleado and \
			n.fechanot>='%s' and n.fechanot<='%s' and \
			t.notacar=n.referencia and \
			t.tipo<>'CHDE' and \
			t.referencia=v.referencia \
			order by c.rsocial, n.fechanot, n.referencia limit %s", fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="USUA"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select n.fechanot, n.referencia, n.foliofisic, \
			cfd.seriefolio as foliocfd, \
			v.referencia as venta, chc.chequecli as cheque,  \
			c.rsocial, t.valor, concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, \
			n.cancelado as estado \
			from notascarcli n  inner join  empleados em  inner join  clientes c  inner join  transxcob t  inner join  ventas v \
			left join pagoscli pag on pag.pago=n.pago \
			left join cheqxcob chxc on chxc.pago=pag.pago \
			left join chequesclientes chc on chc.chequecli=chxc.chequecli \
			left join cfd on n.referencia=cfd.referencia and cfd.tipocomprobante='NCAR' \
			where em.empleado='%s' and n.cliente=c.cliente and \
			n.usumodi=em.empleado and \
			n.fechanot>='%s' and n.fechanot<='%s' and \
			t.notacar=n.referencia and \
			t.tipo<>'CHDE' and \
			t.referencia=v.referencia \
			order by c.rsocial, n.fechanot, n.referencia limit %s", dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="CHEQ"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros); // BANCO
		dato_buscado2=mFg.ExtraeStringDeBuffer(&parametros); // NUM CHEQUE
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select n.fechanot, n.referencia, n.foliofisic, \
			cfd.seriefolio as foliocfd, \
			v.referencia as venta, chc.chequecli as cheque,  \
			c.rsocial, t.valor, concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, \
			n.cancelado as estado \
			from notascarcli n  inner join  empleados em  inner join  clientes c  inner join  transxcob t  inner join  ventas v \
			left join pagoscli pag on pag.pago=n.pago \
			left join cheqxcob chxc on chxc.pago=pag.pago \
			left join chequesclientes chc on chc.chequecli=chxc.chequecli \
			left join cfd on n.referencia=cfd.referencia and cfd.tipocomprobante='NCAR' \
			where chc.banco='%s' and chc.folio like '%s%%' and \
			n.cliente=c.cliente and \
			n.usumodi=em.empleado and \
			n.fechanot>='%s' and n.fechanot<='%s' and \
			t.notacar=n.referencia and \
			t.tipo<>'CHDE' and \
			t.referencia=v.referencia \
			order by c.rsocial, n.fechanot, n.referencia limit %s", dato_buscado, dato_buscado2, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
    if (tipo_busqueda=="UI"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select n.fechanot, n.referencia, n.foliofisic, \
			cfd.seriefolio as foliocfd, \
			v.referencia as venta, chc.chequecli as cheque,  \
			c.rsocial, t.valor, concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, \
			n.cancelado as estado \
			from notascarcli n  inner join  empleados em  inner join  clientes c  inner join  transxcob t  inner join  ventas v \
			left join pagoscli pag on pag.pago=n.pago \
			left join cheqxcob chxc on chxc.pago=pag.pago \
			left join chequesclientes chc on chc.chequecli=chxc.chequecli \
			left join cfd on n.referencia=cfd.referencia and cfd.tipocomprobante='NCAR' \
			where cfd.muuid = '%s' and n.cliente=c.cliente and \
			n.usumodi=em.empleado and \
			n.fechanot>='%s' and n.fechanot<='%s' and \
			t.notacar=n.referencia and \
			t.tipo<>'CHDE' and \
			t.referencia=v.referencia \
			order by c.rsocial, n.fechanot, n.referencia limit %s", dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}

	if (tipo_busqueda=="") {
		// Obtiene todos los usuarios
		instruccion="select usuarios.empleado AS Usuario, empleados.nombre AS Nombre ";
		instruccion+="from usuarios, empleados ";
		instruccion+="where empleados.empleado=usuarios.empleado order by nombre";
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

		// Obtiene todos los bancos
		instruccion="select banco, nombre from bancos order by nombre, banco";
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
}

//---------------------------------------------------------------------------
//ID_BUSQ_PAGO_CLI
void ServidorBusquedas::BuscaPagosCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    // BUSQUEDA DE PAGOS DE CLIENTE
    DatosTabla datos(mServidorVioleta->Tablas);
    AnsiString instruccion;
    AnsiString tipo_busqueda;
    AnsiString dato_buscado, dato_buscado2, fecha_ini, fecha_fin;

    // Extrae el tipo de búsqueda
    tipo_busqueda=mFg.ExtraeStringDeBuffer(&parametros);
    mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ);

    if (tipo_busqueda=="CLI"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select pag.pago, pag.ident, pag.fecha, cli.rsocial as nombre,cli.nomnegocio AS negocio, \
			CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS usuario , pag.valor, \
			if(cancelado=1, 'CANC','') as estado \
			from pagoscli pag, clientes cli, empleados e, terminales ter, secciones sec, sucursales suc \
            where pag.cliente=cli.cliente and \
            cli.rsocial like '%s%%' and \
			pag.fecha>='%s' and pag.fecha<='%s' AND e.empleado=pag.usualta \
			AND ter.terminal = pag.terminal \
			AND sec.seccion = ter.seccion \
			AND suc.sucursal = sec.sucursal \
			AND suc.idempresa = %s \
            order by cli.rsocial, pag.fecha, pag.pago limit %s",
                dato_buscado, fecha_ini, fecha_fin, FormServidor->ObtieneClaveEmpresa(), NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="IDEN"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select pag.pago, pag.ident, pag.fecha, cli.rsocial as nombre,cli.nomnegocio AS negocio, \
			CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS usuario , pag.valor, \
			if(cancelado=1, 'CANC','') as estado \
			from pagoscli pag, clientes cli, empleados e, terminales ter, secciones sec, sucursales suc \
            where pag.cliente=cli.cliente and \
            pag.ident like '%s%%' and \
			pag.fecha>='%s' and pag.fecha<='%s' AND e.empleado=pag.usualta \
            AND ter.terminal = pag.terminal \
			AND sec.seccion = ter.seccion \
			AND suc.sucursal = sec.sucursal \
			AND suc.idempresa = %s \
            order by pag.ident, cli.rsocial, pag.fecha, pag.pago limit %s",
                dato_buscado, fecha_ini, fecha_fin, FormServidor->ObtieneClaveEmpresa(), NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="FOLI"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select pag.pago, pag.ident, pag.fecha, cli.rsocial as nombre,cli.nomnegocio AS negocio, \
			CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS usuario , pag.valor, \
			if(cancelado=1, 'CANC','') as estado \
			from pagoscli pag, clientes cli , empleados e, terminales ter, secciones sec, sucursales suc \
            where pag.cliente=cli.cliente and \
            pag.pago like '%s%%' and \
			pag.fecha>='%s' and pag.fecha<='%s'  AND e.empleado=pag.usualta \
            AND ter.terminal = pag.terminal \
			AND sec.seccion = ter.seccion \
			AND suc.sucursal = sec.sucursal \
			AND suc.idempresa = %s \
            order by pag.pago, cli.rsocial, pag.fecha, pag.pago limit %s",
                dato_buscado, fecha_ini, fecha_fin, FormServidor->ObtieneClaveEmpresa(), NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="FECH"){
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select pag.pago, pag.ident, pag.fecha, cli.rsocial as nombre,cli.nomnegocio AS negocio, \
			CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS usuario , pag.valor, \
			if(cancelado=1, 'CANC','') as estado \
			from pagoscli pag, clientes cli , empleados e, terminales ter, secciones sec, sucursales suc \
            where pag.cliente=cli.cliente and \
			pag.fecha>='%s' and pag.fecha<='%s'  AND e.empleado=pag.usualta \
            AND ter.terminal = pag.terminal \
			AND sec.seccion = ter.seccion \
			AND suc.sucursal = sec.sucursal \
			AND suc.idempresa = %s \
            order by pag.fecha, cli.rsocial, pag.pago limit %s",
                fecha_ini, fecha_fin, FormServidor->ObtieneClaveEmpresa(), NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="CHEQ"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros); // BANCO
        dato_buscado2=mFg.ExtraeStringDeBuffer(&parametros); // NUM CHEQUE
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select pag.pago, pag.ident, pag.fecha, cli.rsocial as nombre,cli.nomnegocio AS negocio, \
			CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS usuario , pag.valor, \
			if(cancelado=1, 'CANC','') as estado \
			from pagoscli pag, clientes cli, transxcob tra, \
			cheqxcob chxc, chequesclientes chcli, empleados e, terminales ter, secciones sec, sucursales suc \
            where pag.cliente=cli.cliente and \
            tra.pago=pag.pago and chxc.pago=tra.pago and \
            chxc.chequecli=chcli.chequecli and chcli.banco='%s' and \
            chcli.folio like '%s%%' and \
			pag.fecha>='%s' and pag.fecha<='%s'  AND e.empleado=pag.usualta  \
            AND ter.terminal = pag.terminal \
			AND sec.seccion = ter.seccion \
			AND suc.sucursal = sec.sucursal \
			AND suc.idempresa = %s \
            order by chcli.folio, pag.pago limit %s",
                dato_buscado, dato_buscado2, fecha_ini, fecha_fin, FormServidor->ObtieneClaveEmpresa(), NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="VENT"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select pag.pago, pag.ident, pag.fecha, cli.rsocial as nombre,cli.nomnegocio AS negocio , \
			CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS usuario , pag.valor, \
			if(pag.cancelado=1, 'CANC','') as estado\
			from transxcob t, ventas vta, pagoscli pag, clientes cli , empleados e, terminales ter, secciones sec, sucursales suc \
            where t.pago = pag.pago AND vta.referencia = t.referencia AND  pag.cliente=cli.cliente and \
            vta.referencia like '%s%%' and \
			pag.fecha>='%s' and pag.fecha<='%s'  AND e.empleado=pag.usualta  \
            AND ter.terminal = pag.terminal \
			AND sec.seccion = ter.seccion \
			AND suc.sucursal = sec.sucursal \
			AND suc.idempresa = %s \
            order by pag.pago, cli.rsocial, pag.fecha, pag.pago limit %s",
                dato_buscado, fecha_ini, fecha_fin, FormServidor->ObtieneClaveEmpresa(), NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="SERIEFOL"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("SELECT pag.pago, pag.ident, pag.fecha, cli.rsocial AS nombre,cli.nomnegocio AS negocio, \
			CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS usuario , pag.valor, \
			IF(cancelado=1, 'CANC','') AS estado \
			FROM pagoscli pag, clientes cli, cfd c , empleados e, terminales ter, secciones sec, sucursales suc \
			WHERE pag.cliente=cli.cliente AND pag.pago=c.referencia AND \
			pag.fecha>='%s' AND pag.fecha<='%s'  AND e.empleado=pag.usualta \
			AND c.seriefolio='%s' AND c.tipocomprobante = 'PAGO' \
            AND ter.terminal = pag.terminal \
			AND sec.seccion = ter.seccion \
			AND suc.sucursal = sec.sucursal \
			AND suc.idempresa = %s \
			ORDER BY pag.pago, cli.rsocial, pag.fecha, pag.pago LIMIT %s",
				fecha_ini, fecha_fin, dato_buscado, FormServidor->ObtieneClaveEmpresa(), NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}

    if (tipo_busqueda=="UI"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("SELECT pag.pago, pag.ident, pag.fecha, cli.rsocial AS nombre,cli.nomnegocio AS negocio, \
			CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS usuario , pag.valor, \
			IF(cancelado=1, 'CANC','') AS estado \
			FROM pagoscli pag, clientes cli, cfd c , empleados e, terminales ter, secciones sec, sucursales suc \
			WHERE pag.cliente=cli.cliente AND pag.pago=c.referencia AND \
			pag.fecha>='%s' AND pag.fecha<='%s'  AND e.empleado=pag.usualta \
			AND c.muuid LIKE '%s%%' AND c.tipocomprobante = 'PAGO' \
            AND ter.terminal = pag.terminal \
			AND sec.seccion = ter.seccion \
			AND suc.sucursal = sec.sucursal \
			AND suc.idempresa = %s \
			ORDER BY pag.pago, cli.rsocial, pag.fecha, pag.pago LIMIT %s",
				fecha_ini, fecha_fin, dato_buscado, FormServidor->ObtieneClaveEmpresa(), NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}

    if (tipo_busqueda=="") {
        // Obtiene todos los bancos
        instruccion="select banco, nombre from bancos order by nombre, banco";
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
}

//---------------------------------------------------------------------------
//ID_BUSQ_CHEQ_PROV
void ServidorBusquedas::BuscaChequesProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    // BUSQUEDA DE CHEQUES DE PROVEEDOR
    DatosTabla datos(mServidorVioleta->Tablas);
    AnsiString instruccion;
    AnsiString tipo_busqueda;
    AnsiString dato_buscado, dato_buscado2, fecha_ini, fecha_fin;

    // Extrae el tipo de búsqueda
    tipo_busqueda=mFg.ExtraeStringDeBuffer(&parametros);
    mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ);

    if (tipo_busqueda=="PROV"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select chp.chequeprov, chp.folio as numcheq, \
            chp.banco, bcs.nombre, chp.clasif, chscsf.descripcion, \
            chp.fechaalta, chp.fechacob, \
            chp.estado, \
            if(pro.tipoempre=0, replegal, razonsocial) as nombreprov \
            from chequesproveedores chp, proveedores pro, bancos bcs, chequesclasif chscsf \
            where chp.proveedor=pro.proveedor and \
            pro.razonsocial like '%s%%' and \
            chp.fechacob>='%s' and chp.fechacob<='%s' \
            and chp.banco=bcs.banco and chp.clasif=chscsf.clasif \
            order by pro.razonsocial, chp.fechacob, chp.folio limit %s",
                dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="FECH"){
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select chp.chequeprov, chp.folio as numcheq, \
            chp.banco, bcs.nombre, chp.clasif, chscsf.descripcion, \
            chp.fechaalta, chp.fechacob, \
            chp.estado, \
            if(pro.tipoempre=0, replegal, razonsocial) as nombreprov \
            from chequesproveedores chp, proveedores pro, bancos bcs, chequesclasif chscsf \
            where chp.proveedor=pro.proveedor and \
            chp.fechacob>='%s' and chp.fechacob<='%s' \
            and chp.banco=bcs.banco and chp.clasif=chscsf.clasif \
            order by pro.razonsocial, chp.fechacob, chp.folio limit %s",
                fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="CHEQ"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros); // BANCO
        dato_buscado2=mFg.ExtraeStringDeBuffer(&parametros); // NUM CHEQUE
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select chp.chequeprov, chp.folio as numcheq, \
            chp.banco, bcs.nombre, chp.clasif, chscsf.descripcion, \
            chp.fechaalta, chp.fechacob, \
            chp.estado, \
            if(pro.tipoempre=0, replegal, razonsocial) as nombreprov \
            from chequesproveedores chp, proveedores pro, bancos bcs, chequesclasif chscsf \
            where chp.proveedor=pro.proveedor and \
            chp.banco='%s' and chp.folio like '%s%%' and \
            chp.fechacob>='%s' and chp.fechacob<='%s' \
            and chp.banco=bcs.banco and chp.clasif=chscsf.clasif \
            order by pro.razonsocial, chp.fechacob, chp.folio limit %s",
                dato_buscado, dato_buscado2, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="COMP"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select chp.chequeprov, chp.folio as numcheq, \
            chp.banco, bcs.nombre, chp.clasif, chscsf.descripcion, \
            chp.fechaalta, chp.fechacob, \
            chp.estado, \
            if(pro.tipoempre=0, replegal, razonsocial) as nombreprov \
            from chequesproveedores chp, cheqxpag chxp, proveedores pro, transxpag tra, compras com, bancos bcs, chequesclasif chscsf \
            where chp.proveedor=pro.proveedor and chxp.chequeprov=chp.chequeprov and \
            tra.pago=chxp.pago and tra.referencia=com.referencia and \
            com.folioprov like '%s%%' and \
            chp.fechacob>='%s' and chp.fechacob<='%s' \
            and chp.banco=bcs.banco and chp.clasif=chscsf.clasif \
            order by pro.razonsocial, chp.fechacob, chp.folio limit %s",
                dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }

    if (tipo_busqueda=="") {
        // Obtiene todos los bancos
        instruccion="select banco, nombre from bancos order by nombre, banco";
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
}

//---------------------------------------------------------------------------
//ID_BUSQ_CHEQ_CLI
void ServidorBusquedas::BuscaChequesCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    // BUSQUEDA DE CHEQUES DE CLIENTE
    DatosTabla datos(mServidorVioleta->Tablas);
    AnsiString instruccion;
    AnsiString tipo_busqueda;
    AnsiString dato_buscado, dato_buscado2, fecha_ini, fecha_fin;

    // Extrae el tipo de búsqueda
    tipo_busqueda=mFg.ExtraeStringDeBuffer(&parametros);
    mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ);

    if (tipo_busqueda=="CLI"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select chc.chequecli, chc.folio as numcheq, \
            chc.banco, bcs.nombre, chc.clasif, chscsf.descripcion, \
            chc.fechaalta, chc.fechacob, \
            chc.estado, \
            cli.rsocial as nombrecli \
            from chequesclientes chc, clientes cli, bancos bcs, chequesclasif chscsf \
            where chc.cliente=cli.cliente and \
            cli.rsocial like '%s%%' and \
            chc.fechacob>='%s' and chc.fechacob<='%s' \
            and chc.banco=bcs.banco and chc.clasif=chscsf.clasif \
            order by cli.rsocial, chc.fechacob, chc.folio limit %s",
                dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="FECH"){
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select chc.chequecli, chc.folio as numcheq, \
            chc.banco, bcs.nombre, chc.clasif, chscsf.descripcion, \
            chc.fechaalta, chc.fechacob, \
            chc.estado, \
            cli.rsocial as nombrecli \
            from chequesclientes chc, clientes cli, bancos bcs, chequesclasif chscsf \
            where chc.cliente=cli.cliente and \
            chc.fechacob>='%s' and chc.fechacob<='%s' \
            and chc.banco=bcs.banco and chc.clasif=chscsf.clasif \
            order by cli.rsocial, chc.fechacob, chc.folio limit %s",
                fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="CHEQ"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros); // BANCO
        dato_buscado2=mFg.ExtraeStringDeBuffer(&parametros); // NUM CHEQUE
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select chc.chequecli, chc.folio as numcheq, \
            chc.banco, bcs.nombre, chc.clasif, chscsf.descripcion, \
            chc.fechaalta, chc.fechacob, \
            chc.estado, \
            cli.rsocial as nombrecli \
            from chequesclientes chc, clientes cli, bancos bcs, chequesclasif chscsf \
            where chc.cliente=cli.cliente and \
            chc.banco='%s' and chc.folio like '%s%%' and \
            chc.fechacob>='%s' and chc.fechacob<='%s' \
            and chc.banco=bcs.banco and chc.clasif=chscsf.clasif \
            order by cli.rsocial, chc.fechacob, chc.folio limit %s",
                dato_buscado, dato_buscado2, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="VENT"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select chc.chequecli, chc.folio as numcheq, \
            chc.banco, bcs.nombre, chc.clasif, chscsf.descripcion, \
            chc.fechaalta, chc.fechacob, \
            chc.estado, \
			cli.rsocial as nombrecli \
			from chequesclientes chc, cheqxcob chxc, clientes cli, transxcob tra, ventas vta, bancos bcs, chequesclasif chscsf \
            where chc.cliente=cli.cliente and chxc.chequecli=chc.chequecli and \
            tra.pago=chxc.pago and tra.referencia=vta.referencia and \
			vta.foliofisic like '%s%%' and \
			chc.fechacob>='%s' and chc.fechacob<='%s' \
			and chc.banco=bcs.banco and chc.clasif=chscsf.clasif \
			order by cli.rsocial, chc.fechacob, chc.folio limit %s",
				dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}

	if (tipo_busqueda=="") {
		// Obtiene todos los bancos
		instruccion="select banco, nombre from bancos order by nombre, banco";
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}

}

//---------------------------------------------------------------------------
//ID_BUSQ_MOVALM
void ServidorBusquedas::BuscaMovimientosAlmacen(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// BUSQUEDA DE MOVIMIENTOS DE ALMACEN
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instruccion;
	AnsiString tipo_busqueda;
	AnsiString dato_buscado,dato_buscado2, fecha_ini, fecha_fin;
	AnsiString revision_alma_ent, revision_alma_sal;
	AnsiString usuario;
	AnsiString condicion_conceptosmovalma=" AND c.protegido=0 ";
	BufferRespuestas *resp_usuario=NULL;

	// Extrae el tipo de búsqueda
	tipo_busqueda=mFg.ExtraeStringDeBuffer(&parametros);
	usuario=mFg.ExtraeStringDeBuffer(&parametros); // Usuario que busca
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ);


	// Obtiene si el usuario tiene o no el privilegio para ver movimientos protegidos
	try {
		instruccion.sprintf("SELECT * FROM(( SELECT u.empleado, ar.objeto, ar.privilegio \
			FROM usuarios u INNER JOIN usuariorol ur ON ur.usuario = u.empleado \
			INNER JOIN asignacionprivrol ar ON ur.rol=ar.rol) UNION ( \
			SELECT a.usuario AS empleado, a.objeto, a.privilegio \
			FROM asignacionprivilegios a) UNION ( SELECT u.empleado, ar.objeto, ar.privilegio \
			FROM usuarios u INNER JOIN empleados e ON e.empleado = u.empleado \
			INNER JOIN puestos p ON p.puesto = e.puesto \
			INNER JOIN rolesxpuesto rxp ON rxp.puesto = p.puesto \
			INNER JOIN asignacionprivrol ar ON rxp.rol=ar.rol)) AS asigprivtotal \
			WHERE asigprivtotal.empleado='%s' AND asigprivtotal.objeto='PROTEGALM' AND asigprivtotal.privilegio='CON'", usuario);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), resp_usuario);

		// Si el usuario tiene el privilegio entonces quita la condicion de solo los protegidos.
		if (resp_usuario->ObtieneNumRegistros()>0)
			condicion_conceptosmovalma=" ";
	} __finally {
		if (resp_usuario!=NULL) delete resp_usuario;
	}


	if (tipo_busqueda=="FOLM"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select m.fechamov, m.fechaalta, m.movimiento, m.tipo, m.almaent, \
			m.almasal, c.descripcion as concepto,  m.aplica,m.cancelado \
			from movalma m \
			INNER JOIN conceptosmovalma c ON m.concepto=c.concepto \
			INNER JOIN almacenes alm ON (m.tipo='E' AND m.almaent = alm.almacen) OR (m.almasal = alm.almacen AND m.tipo <> 'E') \
			INNER JOIN secciones secc ON secc.seccion = alm.seccion \
			INNER JOIN sucursales suc ON secc.sucursal = suc.sucursal \
			where m.movimiento='%s' \
			AND m.fechamov>='%s' AND m.fechamov<='%s' %s \
			AND suc.idempresa = %s \
			order by m.movimiento, m.fechamov limit %s",
			dato_buscado, fecha_ini, fecha_fin, condicion_conceptosmovalma,
			FormServidor->ObtieneClaveEmpresa(), NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="PROD"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select m.fechamov, m.fechaalta, m.movimiento, m.tipo, \
			m.almaent, m.almasal, c.descripcion as concepto,  m.aplica,\
			m.cancelado\
			from movalma m \
			INNER JOIN dmovalma d ON d.movimiento=m.movimiento \
			INNER JOIN articulos a ON d.articulo=a.articulo \
			INNER JOIN conceptosmovalma c ON m.concepto=c.concepto \
			INNER JOIN almacenes alm ON (m.tipo='E' AND m.almaent = alm.almacen) OR (m.almasal = alm.almacen AND m.tipo <> 'E') \
			INNER JOIN secciones secc ON secc.seccion = alm.seccion \
			INNER JOIN sucursales suc ON secc.sucursal = suc.sucursal \
			where a.producto='%s' and  m.fechamov>='%s' and m.fechamov<='%s' %s \
			AND suc.idempresa = %s \
			order by m.movimiento, m.fechamov limit %s", dato_buscado, fecha_ini, fecha_fin,
			condicion_conceptosmovalma, FormServidor->ObtieneClaveEmpresa(), NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="FECH"){
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select m.fechamov, m.fechaalta, m.movimiento, m.tipo, \
			m.almaent, m.almasal, c.descripcion as concepto,  m.aplica, \
			m.cancelado \
			from movalma m \
			INNER JOIN conceptosmovalma c ON m.concepto=c.concepto \
			INNER JOIN almacenes alm ON (m.tipo='E' AND m.almaent = alm.almacen) OR (m.almasal = alm.almacen AND m.tipo <> 'E') \
			INNER JOIN secciones secc ON secc.seccion = alm.seccion \
			INNER JOIN sucursales suc ON secc.sucursal = suc.sucursal \
			where  m.fechamov>='%s' and m.fechamov<='%s' %s \
			AND suc.idempresa = %s \
			order by m.movimiento, m.fechamov limit %s", fecha_ini, fecha_fin,
			condicion_conceptosmovalma, FormServidor->ObtieneClaveEmpresa(), NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="FECHALTA"){
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select m.fechamov, m.fechaalta, m.movimiento, m.tipo, \
			m.almaent, m.almasal, c.descripcion as concepto, m.aplica, \
			m.cancelado \
			from movalma m \
			INNER JOIN conceptosmovalma c ON m.concepto=c.concepto \
			INNER JOIN almacenes alm ON (m.tipo='E' AND m.almaent = alm.almacen) OR (m.almasal = alm.almacen AND m.tipo <> 'E') \
			INNER JOIN secciones secc ON secc.seccion = alm.seccion \
			INNER JOIN sucursales suc ON secc.sucursal = suc.sucursal \
			WHERE m.fechaalta>='%s' and m.fechaalta<='%s' %s \
			AND suc.idempresa = %s \
			order by m.movimiento, m.fechamov limit %s", fecha_ini, fecha_fin,
			condicion_conceptosmovalma, FormServidor->ObtieneClaveEmpresa(), NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="ALMA"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		dato_buscado2=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		revision_alma_ent=" ";
		if (dato_buscado!="")
			revision_alma_ent.sprintf(" m.almaent='%s' and ",dato_buscado);
		revision_alma_sal=" ";
		if (dato_buscado2!="")
			revision_alma_sal.sprintf(" m.almasal='%s' and ",dato_buscado2);

		instruccion.sprintf("select m.fechamov, m.fechaalta, m.movimiento, m.tipo, \
			m.almaent, m.almasal, c.descripcion as concepto, m.aplica, \
			m.cancelado \
			from movalma m \
			INNER JOIN conceptosmovalma c ON m.concepto=c.concepto \
			INNER JOIN almacenes alm ON (m.tipo='E' AND m.almaent = alm.almacen) OR (m.almasal = alm.almacen AND m.tipo <> 'E') \
			INNER JOIN secciones secc ON secc.seccion = alm.seccion \
			INNER JOIN sucursales suc ON secc.sucursal = suc.sucursal \
			WHERE %s %s m.fechamov>='%s' AND m.fechamov<='%s' %s \
			AND suc.idempresa = %s \
			ORDER BY m.movimiento, m.fechamov LIMIT %s", revision_alma_ent, revision_alma_sal, fecha_ini,
			 fecha_fin, condicion_conceptosmovalma, FormServidor->ObtieneClaveEmpresa(), NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="CONC"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select m.fechamov, m.fechaalta, m.movimiento, m.tipo, m.almaent, \
		m.almasal, c.descripcion as concepto, m.aplica, m.cancelado \
		from movalma m \
		INNER JOIN conceptosmovalma c ON m.concepto=c.concepto \
		INNER JOIN almacenes alm ON (m.tipo='E' AND m.almaent = alm.almacen) OR (m.almasal = alm.almacen AND m.tipo <> 'E') \
		INNER JOIN secciones secc ON secc.seccion = alm.seccion \
		INNER JOIN sucursales suc ON secc.sucursal = suc.sucursal \
		where m.concepto='%s' and m.fechamov>='%s' and m.fechamov<='%s' %s \
		AND suc.idempresa = %s \
		order by m.movimiento, m.fechamov limit %s", dato_buscado, fecha_ini, fecha_fin,
		condicion_conceptosmovalma, FormServidor->ObtieneClaveEmpresa(), NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}

	if (tipo_busqueda=="") {
		// Obtiene todos los posibles almacenes
		instruccion="select almacen, nombre from almacenes order by nombre";
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

		// Obtiene todos los posibles conceptos
		instruccion="select concepto, descripcion \
			from conceptosmovalma order by descripcion, concepto";
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
}

//---------------------------------------------------------------------------
//ID_BUSQ_VENTA
void ServidorBusquedas::BuscaVentas(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// BUSQUEDA DE VENTAS
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instruccion;
	AnsiString tipo_busqueda;
	AnsiString solo_tickets, condicion_solo_tickets=" ";
	AnsiString mostrar_ventassuper, condicion_mostrar_ventassuper=" ";
	AnsiString dato_buscado, fecha_ini, fecha_fin, sucursal;

	// Extrae el tipo de búsqueda
	tipo_busqueda=mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ);

	if(tipo_busqueda!=""){
		solo_tickets=mFg.ExtraeStringDeBuffer(&parametros);
		mostrar_ventassuper=mFg.ExtraeStringDeBuffer(&parametros);

		if(solo_tickets=="1"){
			condicion_solo_tickets=" and v.ticket=1 ";
		}

		if(mostrar_ventassuper=="0"){
			condicion_mostrar_ventassuper=" and v.ventasuper<>1 ";
		}
	}

	if (tipo_busqueda=="TICK"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);

		instruccion.sprintf("SELECT v.fechavta, v.referencia, v.foliofisic, \
				cfd.seriefolio AS foliocfd, \
				c.rsocial AS nomcliente, \
				v.valor, \
				CONCAT(ec.nombre, ' ', ec.appat, ' ', ec.apmat) AS nomcajero, v.cancelado, \
				t.nombre, CONCAT(ev.nombre, ' ', ev.appat, ' ', ev.apmat) AS nomvendedor , \
				nc.referencia AS refnotcred, v.ticket AS ticketrev, \
				v.cancelado AS cancelado,  nc.cancelado as notcredcancelado, \
				GROUP_CONCAT(df.formapag) AS formaspago, GROUP_CONCAT(df.valor) AS formaspagovalor, \
				GROUP_CONCAT(df.porcentaje) AS formaspagoporcentaje, GROUP_CONCAT(IFNULL(df.trn_id,0)) AS idtrn,\
				v.cliente, v.tipoorigen, \
				ifnull(vk.kit,''),ifnull(k.nombre,'') \
			FROM ventas v \
				INNER JOIN terminales t \
				INNER JOIN empleados ec \
				INNER JOIN empleados ev \
				INNER JOIN clientes c \
				INNER JOIN dventasfpago df ON df.referencia = v.referencia \
				LEFT JOIN cfd ON v.referencia=cfd.referencia AND cfd.tipocomprobante='VENT' \
				LEFT JOIN notascredcli AS nc ON  v.referencia = nc.venta \
				left join ventaskits vk ON vk.venta=v.referencia \
				left join kits k ON k.kit=vk.kit \
			WHERE v.referencia='%s' AND v.cliente=c.cliente AND v.terminal=t.terminal AND \
			v.usumodi=ec.empleado AND v.vendedor=ev.empleado \
			ORDER BY nomcliente, v.fechavta, v.referencia ", dato_buscado);
		mServidorVioleta->EjecutaSelectSql(Respuesta, MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}

	if (tipo_busqueda=="REFE"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select v.fechavta, v.referencia, v.foliofisic, \
			cfd.seriefolio as foliocfd, \
			c.rsocial as nomcliente, \
			v.valor, \
			concat(ec.nombre, ' ', ec.appat, ' ', ec.apmat) as nomcajero, v.cancelado, \
			vm.mensaje, \
			t.nombre, concat(ev.nombre, ' ', ev.appat, ' ', ev.apmat) as nomvendedor, \
			v.cliente, v.tipoorigen, \
			ifnull(vk.kit,''),ifnull(k.nombre,'') \
			from ventas v inner join terminales t inner join \
			empleados ec inner join empleados ev inner join clientes c \
			left join cfd on v.referencia=cfd.referencia and cfd.tipocomprobante='VENT' \
			left join ventaskits vk ON vk.venta=v.referencia \
			left join kits k ON k.kit=vk.kit \
            LEFT JOIN ventasmensajes vm ON vm.referencia = v.referencia \
			where v.referencia='%s' and v.cliente=c.cliente and \
			v.terminal=t.terminal and v.usumodi=ec.empleado and \
			v.vendedor=ev.empleado and v.fechavta>='%s' and v.fechavta<='%s' %s %s \
			order by nomcliente, v.fechavta, v.referencia limit %s", dato_buscado, fecha_ini,
			fecha_fin, condicion_solo_tickets, condicion_mostrar_ventassuper, NUM_LIMITE_RESULTADOS_BUSQ2);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="FOLIFISIC"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("(select v.fechavta, v.referencia, v.foliofisic, \
			cfd.seriefolio as foliocfd, \
			c.rsocial as nomcliente, \
			v.valor, \
			concat(ec.nombre, ' ', ec.appat, ' ', ec.apmat) as nomcajero, v.cancelado, \
			vm.mensaje, \
			t.nombre, concat(ev.nombre, ' ', ev.appat, ' ', ev.apmat) as nomvendedor, \
			v.cliente,v.tipoorigen, \
			ifnull(vk.kit,''),ifnull(k.nombre,'') \
			from ventas v inner join terminales t inner join \
			empleados ec inner join empleados ev inner join clientes c \
			left join cfd on v.referencia=cfd.referencia and cfd.tipocomprobante='VENT' \
			left join ventaskits vk ON vk.venta=v.referencia \
			left join kits k ON k.kit=vk.kit \
			LEFT JOIN ventasmensajes vm ON vm.referencia = v.referencia \
			where v.foliofisic='%s' and v.cliente=c.cliente and \
			v.terminal=t.terminal and v.usumodi=ec.empleado and \
			v.vendedor=ev.empleado and v.fechavta>='%s' and v.fechavta<='%s' %s %s) \
			UNION \
			(select v.fechavta, v.referencia, v.foliofisic, \
			cfd.seriefolio as foliocfd, \
			c.rsocial as nomcliente, \
			v.valor, \
			concat(ec.nombre, ' ', ec.appat, ' ', ec.apmat) as nomcajero, v.cancelado, \
			vm.mensaje, \
			t.nombre, concat(ev.nombre, ' ', ev.appat, ' ', ev.apmat) as nomvendedor, \
			v.cliente, v.tipoorigen, \
			ifnull(vk.kit,''),ifnull(k.nombre,'') \
			from cfd inner join ventas v on v.referencia=cfd.referencia \
			inner join terminales t inner join \
			empleados ec inner join empleados ev inner join clientes c \
			left join ventaskits vk ON vk.venta=v.referencia \
			left join kits k ON k.kit=vk.kit \
			LEFT JOIN ventasmensajes vm ON vm.referencia = v.referencia \
			where cfd.seriefolio='%s' and cfd.tipocomprobante='VENT' and v.cliente=c.cliente and \
			v.terminal=t.terminal and v.usumodi=ec.empleado and \
			v.vendedor=ev.empleado and v.fechavta>='%s' and v.fechavta<='%s' %s %s) \
			order by nomcliente, fechavta, referencia limit %s",
			dato_buscado, fecha_ini,
			fecha_fin, condicion_solo_tickets, condicion_mostrar_ventassuper,
			dato_buscado, fecha_ini,
			fecha_fin, condicion_solo_tickets, condicion_mostrar_ventassuper,
			NUM_LIMITE_RESULTADOS_BUSQ2);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="FECH"){
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		sucursal=mFg.ExtraeStringDeBuffer(&parametros);

		instruccion.sprintf("select v.fechavta, v.referencia, v.foliofisic, \
			cfd.seriefolio as foliocfd, \
			c.rsocial as nomcliente, \
			v.valor, \
			concat(ec.nombre, ' ', ec.appat, ' ', ec.apmat) as nomcajero, v.cancelado, \
			vm.mensaje, \
			t.nombre, concat(ev.nombre, ' ', ev.appat, ' ', ev.apmat) as nomvendedor, \
			v.cliente,v.tipoorigen, \
			ifnull(vk.kit,''),ifnull(k.nombre,'') \
			from ventas v inner join terminales t inner join \
			empleados ec inner join empleados ev inner join clientes c inner join secciones sec \
			left join cfd on v.referencia=cfd.referencia and cfd.tipocomprobante='VENT' \
			left join ventaskits vk ON vk.venta=v.referencia \
			left join kits k ON k.kit=vk.kit \
            LEFT JOIN ventasmensajes vm ON vm.referencia = v.referencia \
			where v.cliente=c.cliente and \
			v.terminal=t.terminal and t.seccion=sec.seccion and \
			v.usumodi=ec.empleado and \
			v.vendedor=ev.empleado and sec.sucursal='%s' and v.fechavta>='%s' and v.fechavta<='%s' \
			%s %s \
			order by v.fechavta, v.referencia limit %s", sucursal, fecha_ini, fecha_fin,
			condicion_solo_tickets, condicion_mostrar_ventassuper,NUM_LIMITE_RESULTADOS_BUSQ2);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		//order by nomcliente, v.fechavta, v.referencia limit %s", sucursal, fecha_ini, fecha_fin,
	}
	if (tipo_busqueda=="TERM"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select v.fechavta, v.referencia, v.foliofisic, \
			cfd.seriefolio as foliocfd, \
			c.rsocial as nomcliente, \
			v.valor, \
			concat(ec.nombre, ' ', ec.appat, ' ', ec.apmat) as nomcajero, v.cancelado, \
			vm.mensaje, \
			t.nombre, concat(ev.nombre, ' ', ev.appat, ' ', ev.apmat) as nomvendedor, \
			v.cliente,v.tipoorigen, \
			ifnull(vk.kit,''),ifnull(k.nombre,'') \
			from ventas v inner join terminales t inner join \
			empleados ec inner join empleados ev inner join clientes c \
			left join cfd on v.referencia=cfd.referencia and cfd.tipocomprobante='VENT' \
			left join ventaskits vk ON vk.venta=v.referencia \
			left join kits k ON k.kit=vk.kit \
            LEFT JOIN ventasmensajes vm ON vm.referencia = v.referencia \
			where t.terminal='%s' and v.cliente=c.cliente and \
			v.terminal=t.terminal and v.usumodi=ec.empleado and \
			v.vendedor=ev.empleado and v.fechavta>='%s' and v.fechavta<='%s' %s %s \
			order by nomcliente, v.fechavta, v.referencia limit %s", dato_buscado, fecha_ini,
			fecha_fin, condicion_solo_tickets, condicion_mostrar_ventassuper, NUM_LIMITE_RESULTADOS_BUSQ2);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="CAJE"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		sucursal=mFg.ExtraeStringDeBuffer(&parametros);

		instruccion.sprintf("select v.fechavta, v.referencia, v.foliofisic, \
			cfd.seriefolio as foliocfd, \
			c.rsocial as nomcliente, \
			v.valor, \
			concat(ec.nombre, ' ', ec.appat, ' ', ec.apmat) as nomcajero, v.cancelado, \
			vm.mensaje, \
			t.nombre, concat(ev.nombre, ' ', ev.appat, ' ', ev.apmat) as nomvendedor, \
			v.cliente,v.tipoorigen, \
			ifnull(vk.kit,''),ifnull(k.nombre,'') \
			from ventas v inner join terminales t inner join \
			empleados ec inner join empleados ev inner join clientes c inner join secciones sec \
			left join cfd on v.referencia=cfd.referencia and cfd.tipocomprobante='VENT' \
			left join ventaskits vk ON vk.venta=v.referencia \
			left join kits k ON k.kit=vk.kit \
			LEFT JOIN ventasmensajes vm ON vm.referencia = v.referencia \
			where sec.sucursal='%s' and v.usumodi='%s' and v.cliente=c.cliente and \
			v.terminal=t.terminal and  t.seccion=sec.seccion and \
			v.usumodi=ec.empleado and \
			v.vendedor=ev.empleado and v.fechavta>='%s' and v.fechavta<='%s' %s %s \
			order by nomcliente, v.fechavta, v.referencia limit %s", sucursal, dato_buscado, fecha_ini,
			fecha_fin, condicion_solo_tickets, condicion_mostrar_ventassuper, NUM_LIMITE_RESULTADOS_BUSQ2);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="VEND"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		sucursal=mFg.ExtraeStringDeBuffer(&parametros);

		instruccion.sprintf("select v.fechavta, v.referencia, v.foliofisic, \
			cfd.seriefolio as foliocfd, \
			c.rsocial as nomcliente, \
			v.valor, \
			concat(ec.nombre, ' ', ec.appat, ' ', ec.apmat) as nomcajero, v.cancelado, \
			vm.mensaje, \
			t.nombre, concat(ev.nombre, ' ', ev.appat, ' ', ev.apmat) as nomvendedor, \
			v.cliente,v.tipoorigen, \
			ifnull(vk.kit,''),ifnull(k.nombre,'') \
			from ventas v inner join terminales t inner join \
			empleados ec inner join empleados ev inner join clientes c inner join secciones sec \
			left join cfd on v.referencia=cfd.referencia and cfd.tipocomprobante='VENT' \
			left join ventaskits vk ON vk.venta=v.referencia \
			left join kits k ON k.kit=vk.kit \
			LEFT JOIN ventasmensajes vm ON vm.referencia = v.referencia \
			where sec.sucursal='%s' and v.vendedor='%s' and v.cliente=c.cliente and \
			v.terminal=t.terminal and  t.seccion=sec.seccion and \
			v.usumodi=ec.empleado and \
			v.vendedor=ev.empleado and v.fechavta>='%s' and v.fechavta<='%s' %s %s \
			order by nomcliente, v.fechavta, v.referencia limit %s", sucursal, dato_buscado, fecha_ini,
			fecha_fin, condicion_solo_tickets, condicion_mostrar_ventassuper, NUM_LIMITE_RESULTADOS_BUSQ2);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="CLI"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		sucursal=mFg.ExtraeStringDeBuffer(&parametros);

		instruccion.sprintf("select v.fechavta, v.referencia, v.foliofisic, \
			cfd.seriefolio as foliocfd, \
			c.rsocial as nomcliente, \
			v.valor, \
			concat(ec.nombre, ' ', ec.appat, ' ', ec.apmat) as nomcajero, v.cancelado, \
			vm.mensaje, \
			t.nombre, concat(ev.nombre, ' ', ev.appat, ' ', ev.apmat) as nomvendedor, \
			v.cliente,v.tipoorigen, \
			ifnull(vk.kit,''),ifnull(k.nombre,'') \
			from ventas v inner join terminales t inner join \
			empleados ec inner join empleados ev inner join clientes c inner join secciones sec \
			left join cfd on v.referencia=cfd.referencia and cfd.tipocomprobante='VENT' \
			left join ventaskits vk ON vk.venta=v.referencia \
			left join kits k ON k.kit=vk.kit \
			LEFT JOIN ventasmensajes vm ON vm.referencia = v.referencia \
			where sec.sucursal='%s' and v.cliente='%s' and v.cliente=c.cliente and \
			v.terminal=t.terminal and  t.seccion=sec.seccion and \
			v.usumodi=ec.empleado and \
			v.vendedor=ev.empleado and v.fechavta>='%s' and v.fechavta<='%s' %s %s \
			order by nomcliente, v.fechavta, v.referencia limit %s", sucursal, dato_buscado, fecha_ini,
			fecha_fin, condicion_solo_tickets, condicion_mostrar_ventassuper, NUM_LIMITE_RESULTADOS_BUSQ2);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="") {
		// Obtiene todos los posibles terminales
		instruccion="select terminal, nombre from terminales order by nombre";
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

		// Obtiene todos los empleados como posibles cajeros
		instruccion="select empleado AS Empleado, concat(nombre,' ',appat,' ',apmat) AS Nombre from empleados order by nombre, apmat, appat";
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

		// Obtiene todos los Agentes de venta
		instruccion="select emp.empleado AS Empleado, concat(emp.nombre,' ',emp.appat,' ',emp.apmat) AS Nombre \
			from vendedores v, empleados emp where v.empleado=emp.empleado   \
			order by emp.nombre, emp.apmat, emp.appat ";
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
    if (tipo_busqueda=="KIT"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		sucursal=mFg.ExtraeStringDeBuffer(&parametros);

		instruccion.sprintf("select v.fechavta, v.referencia, v.foliofisic, \
			cfd.seriefolio as foliocfd, \
			c.rsocial as nomcliente, \
			v.valor, \
			concat(ec.nombre, ' ', ec.appat, ' ', ec.apmat) as nomcajero, v.cancelado, \
            vm.mensaje, \
			t.nombre, concat(ev.nombre, ' ', ev.appat, ' ', ev.apmat) as nomvendedor, \
			v.tipoorigen, \
			ifnull(vk.kit,''),ifnull(k.nombre,'') \
			from ventas v inner join terminales t inner join \
			empleados ec inner join empleados ev inner join clientes c inner join secciones sec \
			left join cfd on v.referencia=cfd.referencia and cfd.tipocomprobante='VENT' \
			LEFT JOIN ventaskits vk ON vk.venta=v.referencia \
			LEFT JOIN kits k ON k.kit=vk.kit \
			LEFT JOIN ventasmensajes vm ON vm.referencia = v.referencia \
			where sec.sucursal='%s'  and v.cliente=c.cliente and \
			v.terminal=t.terminal and  t.seccion=sec.seccion and \
			v.usumodi=ec.empleado and \
			v.vendedor=ev.empleado and v.fechavta>='%s' and v.fechavta<='%s' %s %s \
			and vk.kit='%s' \
			order by nomcliente, v.fechavta, v.referencia limit %s", sucursal, fecha_ini,
			fecha_fin, condicion_solo_tickets, condicion_mostrar_ventassuper, dato_buscado, NUM_LIMITE_RESULTADOS_BUSQ2);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}

	if (tipo_busqueda=="UUID"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select v.fechavta, v.referencia, v.foliofisic, \
			cfd.seriefolio as foliocfd, \
			c.rsocial as nomcliente, \
			v.valor, \
			concat(ec.nombre, ' ', ec.appat, ' ', ec.apmat) as nomcajero, v.cancelado, \
			vm.mensaje, \
			t.nombre, concat(ev.nombre, ' ', ev.appat, ' ', ev.apmat) as nomvendedor, \
			v.cliente, v.tipoorigen, \
			ifnull(vk.kit,''),ifnull(k.nombre,'') \
			from ventas v \
			inner join terminales t ON t.terminal = v.terminal \
			inner join empleados ec ON ec.empleado = v.usumodi \
			inner join empleados ev ON ev.empleado = v.vendedor  \
			inner join clientes c ON c.cliente = v.cliente \
			left join cfd on v.referencia=cfd.referencia and cfd.tipocomprobante='VENT' \
			left join ventaskits vk ON vk.venta=v.referencia \
			left join kits k ON k.kit=vk.kit \
			LEFT JOIN ventasmensajes vm ON vm.referencia = v.referencia \
			where cfd.muuid LIKE '%s%%' and v.fechavta between '%s' and '%s' %s %s \
			order by nomcliente, v.fechavta, v.referencia limit %s", dato_buscado, fecha_ini,
			fecha_fin, condicion_solo_tickets, condicion_mostrar_ventassuper, NUM_LIMITE_RESULTADOS_BUSQ2);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}

}
//---------------------------------------------------------------------------
//ID_BUSQ_CARTASPORTE
void ServidorBusquedas::BuscaCartasPorte(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// BUSQUEDA DE CARTAS PORTE
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instruccion;
	AnsiString tipo_busqueda;
	AnsiString dato_buscado, fecha_ini, fecha_fin;

	// Extrae el tipo de búsqueda
	tipo_busqueda=mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ);


	if (tipo_busqueda=="REFE"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select cp.fechacp, cp.referencia, \
			transportistas.nombre, \
			cfdcartasporte.cfduuid, \
			c.rsocial as nomcliente, \
			p.razonsocial as nomprov, \
			cp.valor, \
			cp.embarque, \
			cp.cancelado \
			from cartasporte cp \
			LEFT JOIN clientes c ON cp.clientedest=c.cliente \
			LEFT JOIN proveedores p ON cp.proveedororig=p.proveedor \
			LEFT JOIN cfdcartasporte on cp.referencia=cfdcartasporte.referencia \
			LEFT JOIN transportistas on cp.empresatrans=transportistas.empresatrans \
			where cp.referencia='%s' and cp.fechacp between '%s' and '%s' \
			order by cp.fechacp, cp.referencia limit %s", dato_buscado,
			fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}

	if (tipo_busqueda=="FECH"){
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select cp.fechacp, cp.referencia, \
			transportistas.nombre, \
			cfdcartasporte.cfduuid, \
			c.rsocial as nomcliente, \
			p.razonsocial as nomprov, \
			cp.valor, \
			cp.embarque, \
			cp.cancelado \
			from cartasporte cp \
			LEFT JOIN clientes c ON cp.clientedest=c.cliente \
			LEFT JOIN proveedores p ON cp.proveedororig=p.proveedor \
			LEFT JOIN cfdcartasporte on cp.referencia=cfdcartasporte.referencia \
			LEFT JOIN transportistas on cp.empresatrans=transportistas.empresatrans \
			where cp.fechacp between '%s' and '%s' \
			order by cp.fechacp, cp.referencia limit %s",
			fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}

	if (tipo_busqueda=="CLI"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select cp.fechacp, cp.referencia, \
			transportistas.nombre, \
			cfdcartasporte.cfduuid, \
			c.rsocial as nomcliente, \
			p.razonsocial as nomprov, \
			cp.valor, \
			cp.embarque, \
			cp.cancelado \
			from cartasporte cp \
			LEFT JOIN clientes c ON cp.clientedest=c.cliente \
			LEFT JOIN proveedores p ON cp.proveedororig=p.proveedor \
			LEFT JOIN cfdcartasporte on cp.referencia=cfdcartasporte.referencia \
			LEFT JOIN transportistas on cp.empresatrans=transportistas.empresatrans \
			where cp.clientedest='%s' and cp.fechacp between '%s' and '%s' \
			order by cp.fechacp, cp.referencia limit %s",
			dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}

	if (tipo_busqueda=="PRO"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select cp.fechacp, cp.referencia, \
			transportistas.nombre, \
			cfdcartasporte.cfduuid, \
			c.rsocial as nomcliente, \
			p.razonsocial as nomprov, \
			cp.valor, \
			cp.embarque, \
			cp.cancelado \
			from cartasporte cp \
			LEFT JOIN clientes c ON cp.clientedest=c.cliente \
			LEFT JOIN proveedores p ON cp.proveedororig=p.proveedor \
			LEFT JOIN cfdcartasporte on cp.referencia=cfdcartasporte.referencia \
			LEFT JOIN transportistas on cp.empresatrans=transportistas.empresatrans \
			where cp.proveedororig='%s' and cp.fechacp between '%s' and '%s' \
			order by cp.fechacp, cp.referencia limit %s",
			dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}

}

//---------------------------------------------------------------------------
//ID_BUSQ_CARGO_PROV
void ServidorBusquedas::BuscaNotasCargProv(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// BUSQUEDA DE NOTAS DE CARGO DE PROVEEDOR
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instruccion;
	AnsiString tipo_busqueda;
	AnsiString dato_buscado, dato_buscado2, fecha_ini, fecha_fin;

	// Extrae el tipo de búsqueda
	tipo_busqueda=mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ);

    if (tipo_busqueda=="PROV"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select n.fechanot, n.referencia, n.folioprov, \
            c.referencia as compra, chp.chequeprov as cheque,  \
            p.razonsocial, t.valor, concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, \
			n.cancelado as estado \
			from notascarprov n  inner join  empleados em  inner join  proveedores p  inner join  transxpag t  inner join  compras c \
			left join pagosprov pag on pag.pago=n.pago \
            left join cheqxpag chxp on chxp.pago=pag.pago \
            left join chequesproveedores chp on chp.chequeprov=chxp.chequeprov \
            where p.razonsocial like '%s%%' and n.proveedor=p.proveedor and \
            n.usumodi=em.empleado and \
            n.fechanot>='%s' and n.fechanot<='%s' and \
            t.notacar=n.referencia and \
			t.tipo<>'CHDE' and \
            t.referencia=c.referencia \
            order by p.razonsocial, n.fechanot, n.referencia limit %s", dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
	if (tipo_busqueda=="FOLIPROV"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select n.fechanot, n.referencia, n.folioprov, \
            c.referencia as compra, chp.chequeprov as cheque,  \
            p.razonsocial, t.valor, concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, \
            n.cancelado as estado \
            from notascarprov n  inner join  empleados em  inner join  proveedores p  inner join  transxpag t  inner join  compras c \
            left join pagosprov pag on pag.pago=n.pago \
            left join cheqxpag chxp on chxp.pago=pag.pago \
            left join chequesproveedores chp on chp.chequeprov=chxp.chequeprov \
            where n.folioprov like '%s%%' and n.proveedor=p.proveedor and \
			n.usumodi=em.empleado and \
            n.fechanot>='%s' and n.fechanot<='%s' and \
            t.notacar=n.referencia and \
            t.tipo<>'CHDE' and \
            t.referencia=c.referencia \
            order by p.razonsocial, n.fechanot, n.referencia limit %s", dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="REFE"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select n.fechanot, n.referencia, n.folioprov, \
            c.referencia as compra, chp.chequeprov as cheque,  \
            p.razonsocial, t.valor, concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, \
            n.cancelado as estado \
            from notascarprov n  inner join  empleados em  inner join  proveedores p  inner join  transxpag t  inner join  compras c \
            left join pagosprov pag on pag.pago=n.pago \
            left join cheqxpag chxp on chxp.pago=pag.pago \
            left join chequesproveedores chp on chp.chequeprov=chxp.chequeprov \
            where n.referencia='%s' and n.proveedor=p.proveedor and \
            n.usumodi=em.empleado and \
            n.fechanot>='%s' and n.fechanot<='%s' and \
			t.notacar=n.referencia and \
            t.tipo<>'CHDE' and \
            t.referencia=c.referencia \
            order by p.razonsocial, n.fechanot, n.referencia limit %s", dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="COMP") {
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select n.fechanot, n.referencia, n.folioprov, \
            c.referencia as compra, chp.chequeprov as cheque,  \
            p.razonsocial, t.valor, concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, \
            n.cancelado as estado \
            from notascarprov n  inner join  empleados em  inner join  proveedores p  inner join  transxpag t  inner join  compras c \
            left join pagosprov pag on pag.pago=n.pago \
            left join cheqxpag chxp on chxp.pago=pag.pago \
            left join chequesproveedores chp on chp.chequeprov=chxp.chequeprov \
            where t.referencia='%s' and n.proveedor=p.proveedor and \
            n.usumodi=em.empleado and \
            n.fechanot>='%s' and n.fechanot<='%s' and \
            t.notacar=n.referencia and \
            t.tipo<>'CHDE' and \
            t.referencia=c.referencia \
            order by p.razonsocial, n.fechanot, n.referencia limit %s", dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="FECH"){
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select n.fechanot, n.referencia, n.folioprov, \
            c.referencia as compra, chp.chequeprov as cheque,  \
            p.razonsocial, t.valor, concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, \
            n.cancelado as estado \
            from notascarprov n  inner join  empleados em  inner join  proveedores p  inner join  transxpag t  inner join  compras c \
            left join pagosprov pag on pag.pago=n.pago \
            left join cheqxpag chxp on chxp.pago=pag.pago \
            left join chequesproveedores chp on chp.chequeprov=chxp.chequeprov \
            where n.proveedor=p.proveedor and \
            n.usumodi=em.empleado and \
            n.fechanot>='%s' and n.fechanot<='%s' and \
			t.notacar=n.referencia and \
            t.tipo<>'CHDE' and \
            t.referencia=c.referencia \
            order by p.razonsocial, n.fechanot, n.referencia limit %s", fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="USUA"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select n.fechanot, n.referencia, n.folioprov, \
            c.referencia as compra, chp.chequeprov as cheque,  \
            p.razonsocial, t.valor, concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, \
            n.cancelado as estado \
            from notascarprov n  inner join  empleados em  inner join  proveedores p  inner join  transxpag t  inner join  compras c \
            left join pagosprov pag on pag.pago=n.pago \
            left join cheqxpag chxp on chxp.pago=pag.pago \
            left join chequesproveedores chp on chp.chequeprov=chxp.chequeprov \
            where em.empleado='%s' and n.proveedor=p.proveedor and \
            n.usumodi=em.empleado and \
            n.fechanot>='%s' and n.fechanot<='%s' and \
            t.notacar=n.referencia and \
            t.tipo<>'CHDE' and \
            t.referencia=c.referencia \
            order by p.razonsocial, n.fechanot, n.referencia limit %s", dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="CHEQ"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros); // BANCO
		dato_buscado2=mFg.ExtraeStringDeBuffer(&parametros); // NUM CHEQUE
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select n.fechanot, n.referencia, n.folioprov, \
			c.referencia as compra, chp.chequeprov as cheque,  \
			p.razonsocial, t.valor, concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, \
			n.cancelado as estado \
			from notascarprov n, empleados em, proveedores p, transxpag t, compras c, \
			pagosprov pag, cheqxpag chxp, chequesproveedores chp \
			where chp.banco='%s' and chp.folio like '%s%%' and \
			n.proveedor=p.proveedor and \
			n.usumodi=em.empleado and \
			n.fechanot>='%s' and n.fechanot<='%s' and \
			t.notacar=n.referencia and \
			t.referencia=c.referencia and \
			pag.pago=n.pago and \
			chxp.pago=pag.pago and \
			t.tipo<>'CHDE' and \
			chp.chequeprov=chxp.chequeprov \
			order by p.razonsocial, n.fechanot, n.referencia limit %s", dato_buscado, dato_buscado2, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}

	if (tipo_busqueda=="") {
		// Obtiene todos los usuarios
		instruccion="select usuarios.empleado AS Usuario, empleados.nombre AS Nombre ";
		instruccion+="from usuarios, empleados ";
		instruccion+="where empleados.empleado=usuarios.empleado order by nombre";
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

		// Obtiene todos los bancos
		instruccion="select banco, nombre from bancos order by nombre, banco";
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
}

//---------------------------------------------------------------------------
//ID_BUSQ_KIT
void ServidorBusquedas::BuscaKits(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// BUSQUEDA DE KITS
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instruccion;
	AnsiString tipo_busqueda;
	AnsiString dato_buscado,dato_buscado2, fecha_ini, fecha_fin,kit_activo;
	AnsiString condicional_inactivo=" ";

	// Extrae el tipo de búsqueda
	tipo_busqueda=mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ);

	if (tipo_busqueda=="FOLK"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		kit_activo=mFg.ExtraeStringDeBuffer(&parametros);
		if (kit_activo!="0") {
			condicional_inactivo=" or k.activo=0 ";
		}

		instruccion.sprintf("select k.kit, k.ean13, k.nombre, \
			k.cancelado,k.activo,k.preciocomkit,k.fechaalta,k.fechamodi\
			from kits k \
			where k.kit='%s'\
			and k.fechaalta>='%s' and k.fechaalta<='%s' and k.cancelado=0 and k.activo=1 %s  order \
			by k.kit, k.fechamodi limit %s", dato_buscado, fecha_ini, fecha_fin, condicional_inactivo, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="PROD"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		kit_activo=mFg.ExtraeStringDeBuffer(&parametros);
		if (kit_activo!="0") {
			condicional_inactivo=" or k.activo=0 ";
		}

		instruccion.sprintf("select k.kit, k.ean13, k.nombre, \
			k.cancelado,k.activo,k.preciocomkit,k.fechaalta,k.fechamodi \
			from kits k, dkits d, articulos a\
			where a.producto='%s' and k.cancelado=0 and \
			k.fechaalta>='%s' and k.fechaalta<='%s' and \
			d.kit=k.kit and d.articulo=a.articulo and k.activo=1 %s\
			order by k.kit, k.fechamodi limit %s", dato_buscado, fecha_ini, fecha_fin,condicional_inactivo, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="FECH"){
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		kit_activo=mFg.ExtraeStringDeBuffer(&parametros);
		if (kit_activo!="0") {
			condicional_inactivo=" or k.activo=0 ";
		}

		instruccion.sprintf("select k.kit, k.ean13, k.nombre, \
			k.cancelado,k.activo,k.preciocomkit,k.fechaalta,k.fechamodi \
			from kits k \
			where  k.fechamodi between '%s' and '%s' and k.cancelado=0 and k.activo=1 %s \
			order by k.kit, k.fechamodi limit %s", fecha_ini, fecha_fin,condicional_inactivo, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}

	if (tipo_busqueda=="NOMB"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		kit_activo=mFg.ExtraeStringDeBuffer(&parametros);
		if (kit_activo!="0") {
			condicional_inactivo=" or k.activo=0 ";
		}

		instruccion.sprintf("select k.kit, k.ean13, k.nombre, \
			k.cancelado,k.activo,k.preciocomkit,k.fechaalta,k.fechamodi \
			from kits k \
			where k.nombre like '%s%%' \
			and k.fechaalta>='%s' and k.fechaalta<='%s' and k.cancelado=0 and k.activo=1 %s order \
			by k.nombre, k.kit, k.fechamodi limit %s", dato_buscado, fecha_ini, fecha_fin,condicional_inactivo, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}

	if (tipo_busqueda=="CODI"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		kit_activo=mFg.ExtraeStringDeBuffer(&parametros);
		if (kit_activo!="0") {
			condicional_inactivo=" or k.activo=0 ";
		}

		instruccion.sprintf("select k.kit, k.ean13, k.nombre, \
			k.cancelado,k.activo,k.preciocomkit,k.fechaalta,k.fechamodi \
			from kits k \
			where k.ean13='%s' \
			and k.fechaalta>='%s' and k.fechaalta<='%s' and k.cancelado=0 and k.activo=1 %s order \
			by k.kit, k.fechamodi limit %s", dato_buscado, fecha_ini, fecha_fin,condicional_inactivo, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="TODO"){
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		kit_activo=mFg.ExtraeStringDeBuffer(&parametros);
		if (kit_activo!="0") {
			condicional_inactivo=" or k.activo=0 ";
		}

		instruccion.sprintf("select k.kit, k.ean13, k.nombre, \
			k.cancelado,k.activo,k.preciocomkit,k.fechaalta,k.fechamodi \
			from kits k \
			where  k.fechamodi between '%s' and '%s' and k.cancelado=0 and k.activo=1 %s \
			order by  k.nombre,k.fechamodi,k.kit limit %s", fecha_ini, fecha_fin,condicional_inactivo, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
}

//---------------------------------------------------------------------------
//ID_BUSQ_RECEPCIONES
void ServidorBusquedas::BuscaRecepciones(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// BUSQUEDA DE RECEPCIONES
	AnsiString instruccion;
	AnsiString proveedor;
	AnsiString fecha_ini, fecha_fin;
	AnsiString condicion_proveedor=" ";

	// Extrae el proveedor
	proveedor=mFg.ExtraeStringDeBuffer(&parametros);
	fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
	fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

	if(proveedor!=" "){
		condicion_proveedor.sprintf(" AND r.proveedor = '%s' ", proveedor);
	}

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ);

	instruccion.sprintf("SELECT r.recepcion, r.fecharep, r.horaini, r.cancelado  \
	FROM recepciones AS r \
	INNER JOIN drecepciones AS d ON d.recepcion=r.recepcion \
	WHERE r.fecharep>='%s' and r.fecharep<='%s' %s \
	GROUP BY r.recepcion \
	ORDER BY r.recepcion, r.fecharep",
		fecha_ini, fecha_fin, condicion_proveedor);
	mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

}
//---------------------------------------------------------------------------
//ID_BUSQ_CORTE
void ServidorBusquedas::BuscaCortes(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
}

//---------------------------------------------------------------------------

//ID_BUSQ_INVENTARIO
void ServidorBusquedas::BuscaInventarios(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    // BUSQUEDA DE INVENTARIOS
    DatosTabla datos(mServidorVioleta->Tablas);
    AnsiString instruccion;
    AnsiString tipo_busqueda;
    AnsiString dato_buscado, fecha_ini, fecha_fin;

    // Extrae el tipo de búsqueda
    tipo_busqueda=mFg.ExtraeStringDeBuffer(&parametros);
    mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ);

    if (tipo_busqueda=="FOLI"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select i.fechainv, i.inventario, i.tipo, i.almacen, a.nombre, \
            i.descripcion \
            from inventarios i, almacenes a \
            where i.inventario='%s' and i.almacen=a.almacen \
            and i.fechainv>='%s' and i.fechainv<='%s' \
            order by i.inventario, i.fechainv limit %s", dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="FECH"){
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select i.fechainv, i.inventario, i.tipo, i.almacen, a.nombre, \
            i.descripcion \
            from inventarios i, almacenes a \
            where i.fechainv>='%s' and i.fechainv<='%s' and i.almacen=a.almacen \
            order by i.inventario, i.fechainv limit %s", fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="ALMA"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select i.fechainv, i.inventario, i.tipo, i.almacen, a.nombre, \
            i.descripcion \
            from inventarios i, almacenes a \
            where i.almacen='%s' \
            and i.fechainv>='%s' and i.fechainv<='%s' and i.almacen=a.almacen \
            order by i.inventario, i.fechainv limit %s", dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="") {
        // Obtiene todos los posibles almacenes
        instruccion="select almacen, nombre from almacenes order by nombre";
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
}

//---------------------------------------------------------------------------

//ID_BUSQ_FORMA_IMPRESION
void ServidorBusquedas::BuscaFormasImpresion(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// BUSQUEDA DE FORMAS DE IMPRESION
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instruccion;
	AnsiString tipo_busqueda;
	AnsiString dato_buscado;

	// Extrae el tipo de búsqueda
	tipo_busqueda=mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ);

	if (tipo_busqueda!="") {
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
	}

	if (tipo_busqueda=="CLA"){
		if (dato_buscado==" ")
			instruccion.sprintf("select forma, nombre, fechamodi from formas order by forma, nombre limit %s", NUM_LIMITE_RESULTADOS_BUSQ);
		else
			instruccion.sprintf("select forma, nombre, fechamodi from formas where forma like '%s%%' order by forma, nombre limit %s", dato_buscado, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="NOM"){
		instruccion.sprintf("select forma, nombre, fechamodi from formas where nombre like '%s%%' order by nombre, forma limit %s", dato_buscado, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}

	if (tipo_busqueda==""){
		// No mandar nada
	}
}
//---------------------------------------------------------------------------

//ID_BUSQ_MARCA
void ServidorBusquedas::BuscaMarcas(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instruccion;
	AnsiString tipo_busqueda;
	AnsiString excluir_datos;
	AnsiString dato_buscado;


	// Extrae el tipo de búsqueda
	tipo_busqueda=mFg.ExtraeStringDeBuffer(&parametros);
	excluir_datos=mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ);

	if (tipo_busqueda!="") {
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
	}

	if (tipo_busqueda=="NAME"){
		instruccion.sprintf("SELECT DISTINCT marcas.marca,marcas.nombre FROM productos  INNER JOIN marcas on marcas.marca=productos.marca WHERE %s marcas.nombre like '%s%%'  ORDER BY marcas.nombre LIMIT %s",excluir_datos, dato_buscado, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="CLAV"){

		instruccion.sprintf("SELECT DISTINCT marcas.marca,marcas.nombre FROM productos  INNER JOIN marcas on marcas.marca=productos.marca WHERE %s marcas.marca like  '%s%%'  ORDER BY marcas.marca LIMIT %s",excluir_datos , dato_buscado,  NUM_LIMITE_RESULTADOS_BUSQ);
 		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}

   //select marca,nombre from marcas where nombre like '%tam%';
	if (tipo_busqueda==""){
		// No mandar nada
	}
}
//---------------------------------------------------------------------------

//ID_BUSQ_EMBARQUE
void ServidorBusquedas::BuscaEmbarques(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    // BUSQUEDA DE ARTICULOS
    DatosTabla datos(mServidorVioleta->Tablas);
    AnsiString instruccion;
    AnsiString tipo_busqueda;
    AnsiString dato_buscado;

    AnsiString FechaInicial;
	AnsiString FechaFinal;
	AnsiString Sucursal;

	// Extrae el tipo de búsqueda
	Sucursal=mFg.ExtraeStringDeBuffer(&parametros);
    tipo_busqueda=mFg.ExtraeStringDeBuffer(&parametros);
    FechaInicial=mFg.ExtraeStringDeBuffer(&parametros);
	FechaFinal=mFg.ExtraeStringDeBuffer(&parametros);

    if (tipo_busqueda!="") dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);

    mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ);

	if (tipo_busqueda=="FECH") {
		instruccion.sprintf("select e.embarque, max(p.embarquediv) as divmax, v.viaembarq, v.descripcion, em.empleado, \
			concat(em.nombre, ' ', em.appat, ' ', em.apmat) as chofer, e.fechasalid, e.fecharecep, \
			CASE e.etapa WHEN 0 THEN ('EN GENERACION') WHEN 1 THEN ('EN CURSO') WHEN 9 THEN ('LIBERADO') END AS embarqueestado, if(e.cancelado=1, 'CANCELADO','ACTIVO' ) as cancelado \
			FROM embarques e \
				inner join viasembarque v on e.viaembarq=v.viaembarq \
				inner join empleados em on e.chofer=em.empleado \
				inner join pedidosventa p on e.embarque=p.embarque and p.cancelado=0\
			where e.sucursal='%s' and e.fechasalid>='%s' and e.fechasalid<='%s' \
			group by e.embarque \
			ORDER BY e.embarque \
			limit %s",Sucursal, FechaInicial, FechaFinal, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="VIA"){
		instruccion.sprintf("select e.embarque, max(p.embarquediv) as divmax, v.viaembarq, v.descripcion, em.empleado, \
			concat(em.nombre, ' ', em.appat, ' ', em.apmat) as chofer, e.fechasalid, e.fecharecep, \
			CASE e.etapa WHEN 0 THEN ('EN GENERACION') WHEN 1 THEN ('EN CURSO') WHEN 9 THEN ('LIBERADO') END AS embarqueestado, if(e.cancelado=1, 'CANCELADO','ACTIVO' ) as cancelado \
			FROM embarques e \
				inner join viasembarque v on e.viaembarq=v.viaembarq \
				inner join empleados em on e.chofer=em.empleado \
				inner join pedidosventa p on e.embarque=p.embarque and p.cancelado=0\
			where e.sucursal='%s' and e.viaembarq=v.viaembarq and e.chofer=em.empleado and e.viaembarq='%s' and e.fechasalid>='%s' and e.fechasalid<='%s' \
			group by e.embarque \
			ORDER BY e.embarque \
			limit %s",Sucursal, dato_buscado,FechaInicial, FechaFinal, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="CHOF") {
		instruccion.sprintf("select e.embarque, max(p.embarquediv) as divmax, v.viaembarq, v.descripcion, em.empleado, \
			concat(em.nombre, ' ', em.appat, ' ', em.apmat) as chofer, e.fechasalid, e.fecharecep, \
			CASE e.etapa WHEN 0 THEN ('EN GENERACION') WHEN 1 THEN ('EN CURSO') WHEN 9 THEN ('LIBERADO') END AS embarqueestado, if(e.cancelado=1, 'CANCELADO','ACTIVO' ) as cancelado \
			FROM embarques e \
				inner join viasembarque v on e.viaembarq=v.viaembarq \
				inner join empleados em on e.chofer=em.empleado \
				inner join pedidosventa p on e.embarque=p.embarque and p.cancelado=0\
			where e.sucursal='%s' and e.viaembarq=v.viaembarq and e.chofer=em.empleado and e.chofer='%s' and e.fechasalid>='%s' and e.fechasalid<='%s' \
			group by e.embarque \
			ORDER BY e.embarque \
			limit %s",Sucursal, dato_buscado,FechaInicial, FechaFinal, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="PEDI") {
		instruccion.sprintf("select e.embarque, max(p.embarquediv) as divmax, v.viaembarq, v.descripcion, em.empleado, \
			concat(em.nombre, ' ', em.appat, ' ', em.apmat) as chofer, e.fechasalid, e.fecharecep, \
			CASE e.etapa WHEN 0 THEN ('EN GENERACION') WHEN 1 THEN ('EN CURSO') WHEN 9 THEN ('LIBERADO') END AS embarqueestado, if(e.cancelado=1, 'CANCELADO','ACTIVO' ) as cancelado \
			FROM embarques e \
				inner join viasembarque v on e.viaembarq=v.viaembarq \
				inner join empleados em on e.chofer=em.empleado \
				inner join pedidosventa p on e.embarque=p.embarque and p.cancelado=0 and\
			e.sucursal='%s' and e.chofer=em.empleado and p.referencia like '%s%%' and e.fechasalid>='%s' and e.fechasalid<='%s' \
			group by e.embarque \
			ORDER BY e.embarque \
			limit %s",Sucursal, dato_buscado,FechaInicial, FechaFinal, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="FOLIO") {
		instruccion.sprintf("select e.embarque, max(p.embarquediv) as divmax, v.viaembarq, v.descripcion, em.empleado, \
			concat(em.nombre, ' ', em.appat, ' ', em.apmat) as chofer, e.fechasalid, e.fecharecep, \
			CASE e.etapa WHEN 0 THEN ('EN GENERACION') WHEN 1 THEN ('EN CURSO') WHEN 9 THEN ('LIBERADO') END AS embarqueestado, if(e.cancelado=1, 'CANCELADO','ACTIVO' ) as cancelado \
			FROM embarques e \
				inner join viasembarque v on e.viaembarq=v.viaembarq \
				inner join empleados em on e.chofer=em.empleado \
				inner join pedidosventa p on e.embarque=p.embarque and p.cancelado=0\
			where e.sucursal='%s' and e.embarque='%s' and p.embarque=e.embarque and e.viaembarq=v.viaembarq and \
			e.chofer=em.empleado \
			group by e.embarque \
			ORDER BY e.embarque \
			limit %s",Sucursal, dato_buscado, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="FACTURA") {
		instruccion.sprintf("select e.embarque, vta.embarquediv as divmax, v.viaembarq, v.descripcion, em.empleado, \
			concat(em.nombre, ' ', em.appat, ' ', em.apmat) as chofer, e.fechasalid, e.fecharecep, \
			CASE e.etapa WHEN 0 THEN ('EN GENERACION') WHEN 1 THEN ('EN CURSO') WHEN 9 THEN ('LIBERADO') END AS embarqueestado, if(e.cancelado=1, 'CANCELADO','ACTIVO' ) as cancelado \
			from ventas vta, embarques e, viasembarque v, empleados em \
			where e.sucursal='%s' and vta.referencia='%s' and vta.embarque=e.embarque and e.viaembarq=v.viaembarq and \
			e.chofer=em.empleado \
			order by e.embarque \
			limit %s",Sucursal, dato_buscado, NUM_LIMITE_RESULTADOS_BUSQ);

		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}

	if (tipo_busqueda==""){
		// No mandar nada
	}
}
//---------------------------------------------------------------------------

//ID_BUSQ_TRANSFORMACION
void ServidorBusquedas::BuscaTransformacion(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// BUSQUEDA DE TRANSFORMACIONES DE PRODUCTOS
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instruccion;
	AnsiString tipo_busqueda;
	AnsiString dato_buscado,dato_buscado2, fecha_ini, fecha_fin;
	AnsiString revision_almacen;

    // Extrae el tipo de búsqueda
	tipo_busqueda=mFg.ExtraeStringDeBuffer(&parametros);
    mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ);

    if (tipo_busqueda=="FOLT"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select t.fechatrans, t.transfor, t.foliofisic, t.almacen, t.cancelado \
            from transformacion t \
            where t.transfor='%s' \
            and t.fechatrans>='%s' and t.fechatrans<='%s' \
            order by t.transfor, t.fechatrans limit %s", dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="FOLF"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select t.fechatrans, t.transfor, t.foliofisic, t.almacen, t.cancelado \
            from transformacion t \
            where t.foliofisic='%s' \
            and t.fechatrans>='%s' and t.fechatrans<='%s' \
            order by t.transfor, t.fechatrans limit %s", dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="FECH"){
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select t.fechatrans, t.transfor, t.foliofisic, t.almacen, t.cancelado \
            from transformacion t \
            where t.fechatrans>='%s' and t.fechatrans<='%s' \
            order by t.transfor, t.fechatrans limit %s", fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="ALMA"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select t.fechatrans, t.transfor, t.foliofisic, t.almacen, t.cancelado \
            from transformacion t \
            where t.almacen='%s' \
            and t.fechatrans>='%s' and t.fechatrans<='%s' \
            order by t.transfor, t.fechatrans limit %s", dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }

    if (tipo_busqueda=="") {
		// Obtiene todos los posibles almacenes
        instruccion="select almacen, nombre from almacenes order by nombre";
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
}
//---------------------------------------------------------------------------
//ID_BUSQ_LOTE_INVENTARIO
void ServidorBusquedas::BuscaLoteInventario(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    // BUSQUEDA DE VENTAS
    DatosTabla datos(mServidorVioleta->Tablas);
    AnsiString instruccion;
    AnsiString tipo_busqueda;
    AnsiString dato_buscado, inventario;

    // Extrae el tipo de búsqueda
    tipo_busqueda=mFg.ExtraeStringDeBuffer(&parametros);
    mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ);

    if (tipo_busqueda=="LOTE"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        inventario=mFg.ExtraeStringDeBuffer(&parametros);

		instruccion.sprintf("select li.lote, li.usualta, concat(em.nombre, ' ', em.appat, ' ', em.apmat) as nomusuario \
            from lotesinv li, empleados em \
			where li.inventario='%s' and li.usualta=em.empleado and li.lote='%s' \
			order by li.lote limit %s ", inventario, dato_buscado, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="CAPTU"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        inventario=mFg.ExtraeStringDeBuffer(&parametros);

		instruccion.sprintf("select li.lote, li.usualta, concat(em.nombre, ' ', em.appat, ' ', em.apmat) as nomusuario \
			from lotesinv li, empleados em \
            where li.inventario='%s' and li.usualta=em.empleado and li.usualta='%s' \
            order by li.lote limit %s ", inventario, dato_buscado, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="ARTI"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        inventario=mFg.ExtraeStringDeBuffer(&parametros);

		instruccion.sprintf("select li.lote, li.usualta, concat(em.nombre, ' ', em.appat, ' ', em.apmat) as nomusuario \
            from lotesinv li, empleados em, dinventarios di, articulos ar \
			where li.inventario='%s' and li.usualta=em.empleado and di.inventario='%s' and li.lote=di.lote and di.articulo=ar.articulo \
            and ar.producto='%s' group by li.lote \
            order by li.lote limit %s ", inventario, inventario, dato_buscado, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }

    if (tipo_busqueda=="") {
		// Obtiene todos los empleados como posibles usuarios
        instruccion="select u.empleado AS Empleado, concat(em.nombre,' ',em.appat,' ',em.apmat) AS Nombre \
					 from empleados em, usuarios u where u.empleado=em.empleado order by em.nombre, em.apmat, em.appat";
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

	}
}

//---------------------------------------------------------------------------
//ID_BUSQ_MSGSMOVILES
void ServidorBusquedas::BuscaMsgsMoviles(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// BUSQUEDA DE GRUPOS DE MENSAJES PARA DISPOSITIVOS MOVILES
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instruccion;
	AnsiString tipo_busqueda;
	AnsiString dato_buscado,dato_buscado2, fecha_ini, fecha_fin;
	AnsiString revision_almacen;

	// Extrae el tipo de búsqueda
	tipo_busqueda=mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ);

	if (tipo_busqueda=="GRUP"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select m.fechaenvio, m.grupo, m.asunto, concat(e.nombre, ' ', e.appat, ' ', e.apmat) as nomremit, m.urgente, min(m.enviadomovil) as enviadomovil \
				from mensajes m inner join empleados e \
				where m.remitente=e.empleado and destmovil=1 and \
				m.grupo='%s' and \
				m.fechaenvio>='%s' and m.fechaenvio<='%s' \
				group by m.grupo \
				order by m.grupo limit %s", dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="ASUN"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select m.fechaenvio, m.grupo, m.asunto, concat(e.nombre, ' ', e.appat, ' ', e.apmat) as nomremit, m.urgente, min(m.enviadomovil) as enviadomovil \
				from mensajes m inner join empleados e \
				where m.remitente=e.empleado and destmovil=1 and \
				m.asunto like '%%%s%%' and \
				m.fechaenvio>='%s' and m.fechaenvio<='%s' \
				group by m.grupo \
				order by m.grupo limit %s", dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="CONT"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select m.fechaenvio, m.grupo, m.asunto, concat(e.nombre, ' ', e.appat, ' ', e.apmat) as nomremit, m.urgente, min(m.enviadomovil) as enviadomovil \
				from mensajes m inner join empleados e \
				where m.remitente=e.empleado and destmovil=1 and \
				m.contenido like '%%%s%%' and \
				m.fechaenvio>='%s' and m.fechaenvio<='%s' \
				group by m.grupo \
				order by m.grupo limit %s", dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="FECH"){
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select m.fechaenvio, m.grupo, m.asunto, concat(e.nombre, ' ', e.appat, ' ', e.apmat) as nomremit, m.urgente, min(m.enviadomovil) as enviadomovil \
				from mensajes m inner join empleados e \
				where m.remitente=e.empleado and destmovil=1 and \
				m.fechaenvio>='%s' and m.fechaenvio<='%s' \
				group by m.grupo \
				order by m.grupo limit %s", fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}

	if (tipo_busqueda=="") {
	}
}

//---------------------------------------------------------------------------
//ID_BUSQ_USUARIO
void ServidorBusquedas::BuscaUsuarios(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	DatosTabla datos(mServidorVioleta->Tablas);
    AnsiString instruccion;
	AnsiString tipo_busqueda;
	AnsiString nombre = "", clave = "";
	AnsiString solo_activos , condicion_solo_activos = " ";
	BufferRespuestas* resp_exist=NULL;

	//Extrae el tipo de busqueda
	tipo_busqueda=mFg.ExtraeStringDeBuffer(&parametros);

	if(tipo_busqueda == "NOM")
		nombre=mFg.ExtraeStringDeBuffer(&parametros);

	if(tipo_busqueda == "CLV")
		clave=mFg.ExtraeStringDeBuffer(&parametros);

	solo_activos = mFg.ExtraeStringDeBuffer(&parametros);

	try{

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ);

		if (solo_activos=="1") {
			condicion_solo_activos=" and u.activo=1 ";
		}
		if (solo_activos=="0") {
			condicion_solo_activos=" ";
		}

		if (tipo_busqueda=="NOM"){
			instruccion.sprintf(" \
			SELECT u.empleado,CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS nombre_usuario, \
			ifNULL(GROUP_CONCAT(us.sucursal ORDER BY us.sucursal), '') AS sucasig , u.activo \
			FROM usuarios u INNER JOIN empleados e ON e.empleado = u.empleado \
			LEFT JOIN usuariosucursal us ON us.usuario = u.empleado \
			WHERE CONCAT(e.nombre,' ',e.appat,' ',e.apmat) LIKE '%s%%' %s \
			GROUP BY u.empleado ORDER BY e.nombre, e.apmat, e.appat ", nombre, condicion_solo_activos);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
		if  (tipo_busqueda=="CLV"){
			instruccion.sprintf(" \
			SELECT u.empleado,CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS nombre_usuario, \
			ifNULL(GROUP_CONCAT(us.sucursal ORDER BY us.sucursal), '') AS sucasig, u.activo \
			FROM usuarios u INNER JOIN empleados e ON e.empleado = u.empleado \
			LEFT JOIN usuariosucursal us ON us.usuario = u.empleado \
			WHERE u.empleado LIKE '%s%%' %s \
			GROUP BY u.empleado ORDER BY e.nombre, e.apmat, e.appat ", clave, condicion_solo_activos);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}

	}
	__finally{
		if(resp_exist != NULL) delete resp_exist;
	}
}

//---------------------------------------------------------------------------
//                      BUSQUEDA DE EMPLEADOS
//---------------------------------------------------------------------------
//ID_BUSQ_EMPLEADO
void ServidorBusquedas::BuscaEmpleados(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    // BUSQUEDA DE ARTICULOS
    DatosTabla datos(mServidorVioleta->Tablas);
    AnsiString instruccion;
	AnsiString tipo_busqueda;
	AnsiString nombre = "", clave = "";
	AnsiString solo_activos , condicion_solo_activos = " ";
	BufferRespuestas* resp_exist=NULL;

	//Extrae el tipo de busqueda
	tipo_busqueda=mFg.ExtraeStringDeBuffer(&parametros);

	if(tipo_busqueda == "NOM")
		nombre=mFg.ExtraeStringDeBuffer(&parametros);

	if(tipo_busqueda == "CLV")
		clave=mFg.ExtraeStringDeBuffer(&parametros);

	solo_activos = mFg.ExtraeStringDeBuffer(&parametros);

	try{

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ);

		if (solo_activos=="0") {
			condicion_solo_activos=" and activo=1 ";
		}
		if (solo_activos=="1") {
			condicion_solo_activos=" and (activo=1 OR activo=0) ";
		}


		if (tipo_busqueda=="NOM"){
			instruccion.sprintf("SELECT empleado,CONCAT(nombre,' ',appat,' ',apmat) AS nombre_empleado, sucursal, activo FROM empleados \
			WHERE CONCAT(nombre,' ',appat,' ',apmat) LIKE '%s%%' %s ORDER BY nombre, apmat, appat ", nombre, condicion_solo_activos);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
		if  (tipo_busqueda=="CLV"){
			instruccion.sprintf(" SELECT empleado,CONCAT(nombre,' ',appat,' ',apmat) AS nombre_empleado,sucursal, activo \
			FROM empleados WHERE empleado = '%s' %s ", clave, condicion_solo_activos);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}

	}
	__finally{
		if(resp_exist != NULL) delete resp_exist;
	}
}

//---------------------------------------------------------------------------
//                      BUSQUEDA DE PRODUCTOS/SERVICIOS
//---------------------------------------------------------------------------
//ID_BUSQ_PROD_SERV
void ServidorBusquedas::BuscaProdServ(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// BUSQUEDA DE PRODUCTOS/SERVICIOS
    DatosTabla datos(mServidorVioleta->Tablas);
    AnsiString instruccion;
	AnsiString tipo_busqueda;
	AnsiString dato_busqueda;

	//Extrae el tipo de búsqueda
	tipo_busqueda=mFg.ExtraeStringDeBuffer(&parametros);
	//extrae el valor para confirmar que viene desde PV
	dato_busqueda=mFg.ExtraeStringDeBuffer(&parametros);

		mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ);

		if (tipo_busqueda=="C"){
			instruccion.sprintf("SELECT idclaveprodserv,cveprodserv, descripcion, fechaIniVig, fechafinVig, \
			IF(incluiriva = 0 ,'OPCIONAL','SI') AS iva , IF(incluirieps = 0, 'OPCIONAL','NO') AS ieps, estado \
			FROM cclaveprodserv WHERE estado = '0'  AND cveprodserv LIKE '%s%' limit 500",dato_busqueda);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}
		if  (tipo_busqueda=="D"){
			instruccion.sprintf("SELECT idclaveprodserv,cveprodserv, descripcion, fechaIniVig, fechafinVig, \
			IF(incluiriva = 0 ,'OPCIONAL','SI') AS iva , IF(incluirieps = 0, 'OPCIONAL','NO') AS ieps, estado \
			FROM cclaveprodserv WHERE estado = '0'  AND descripcion LIKE '%s%' limit 500",dato_busqueda);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
		}

}
//---------------------------------------------------------------------------
//ID_BUSQ_VENDEDOR
void ServidorBusquedas::BuscaVendedores(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{

	AnsiString instruccion;
	AnsiString tipo_busqueda;
	AnsiString solo_activos, condicion_activos=" ";
	AnsiString dato_buscado;

	// Extrae el tipo de búsqueda
	tipo_busqueda=mFg.ExtraeStringDeBuffer(&parametros);
	solo_activos=mFg.ExtraeStringDeBuffer(&parametros);
	dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ);

	if (solo_activos=="1") {
		condicion_activos=" AND v.activo=1 ";
	}else
		{
		  condicion_activos=" AND v.activo=0 ";
		}


	if (tipo_busqueda=="NOM")
	{
		instruccion.sprintf("SELECT e. empleado, CONCAT(e.nombre,' ',e.appat,' ',e.apmat) nombre,e.localidad,v.tipocomi \
		FROM empleados e INNER JOIN vendedores v ON e.empleado=v.empleado\
		WHERE e.nombre like '%s%%' %s ORDER  BY e.nombre limit %s", dato_buscado,condicion_activos, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	else if (tipo_busqueda=="APE")
	{
		instruccion.sprintf("SELECT e. empleado, CONCAT(e.nombre,' ',e.appat,' ',e.apmat) nombre,e.localidad,v.tipocomi \
		FROM empleados e INNER JOIN vendedores v ON e.empleado=v.empleado \
		WHERE e.appat like '%s%%' OR e.apmat like '%s%%' %s ORDER  BY e.appat limit %s", dato_buscado,dato_buscado,condicion_activos, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	else if (tipo_busqueda=="COMI")
	{
		instruccion.sprintf("SELECT e. empleado, CONCAT(e.nombre,' ',e.appat,' ',e.apmat) nombre,e.localidad,v.tipocomi \
		FROM empleados e INNER JOIN vendedores v ON e.empleado=v.empleado \
		WHERE v.tipocomi = '%s' %s ORDER  BY v.tipocomi limit %s", dato_buscado,condicion_activos, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	else if (tipo_busqueda=="CLA")
	{
		instruccion.sprintf("SELECT e. empleado, CONCAT(e.nombre,' ',e.appat,' ',e.apmat) nombre,e.localidad,v.tipocomi \
		FROM empleados e INNER JOIN vendedores v ON e.empleado=v.empleado \
		WHERE e.empleado like '%s%%' %s ORDER  BY e.empleado limit %s", dato_buscado,condicion_activos, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}

}
//---------------------------------------------------------------------------
//ID_BUSQ_ARTICULOS_CANDIDATOS_AGREGAR
 void ServidorBusquedas::BuscaArticulosCandidatosAgregar(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	AnsiString sucursal, almacen, empresa, supervisor;
	AnsiString instruccion;
    AnsiString condicion_supervisor=" ";
	AnsiString subquery_productos=" ";

	AnsiString wheres = " ";
    AnsiString havings = " ";

	BufferRespuestas* resp_almacenes=NULL;
	try {
		sucursal=mFg.ExtraeStringDeBuffer(&parametros);
		empresa=mFg.ExtraeStringDeBuffer(&parametros);
		supervisor=mFg.ExtraeStringDeBuffer(&parametros);
		AnsiString ventaPromedio = mFg.ExtraeStringDeBuffer(&parametros);
		AnsiString diasVentaPromedio = mFg.ExtraeStringDeBuffer(&parametros);
		AnsiString existencias = mFg.ExtraeStringDeBuffer(&parametros);
		AnsiString descontinuados = mFg.ExtraeStringDeBuffer(&parametros);


		if (supervisor!="") {
			condicion_supervisor.sprintf(" AND artsup.usuario = '%s'", supervisor);
		}

		if(sucursal != ""){
			AnsiString condicion;
			condicion.sprintf(" AND suc.sucursal = '%s'", sucursal);
			wheres += condicion;
		} else if(empresa != ""){
			AnsiString condicion;
			condicion.sprintf(" AND suc.idempresa = %s", empresa);
			wheres += condicion;
		}

		if(ventaPromedio =="0"){
			havings += " AND ventaPromedioDiaria > 0 ";
		} else if (ventaPromedio =="1"){
			havings += " AND ventaPromedioDiaria = 0 ";
		}

		if(existencias =="0"){
			havings += " AND total_exist > 0 ";
		} else if (existencias =="1"){
			havings += " AND total_exist = 0 ";
		}

		if(descontinuados =="0"){
			havings += " AND descontinuado = 0 ";
		} else if (descontinuados =="1"){
			havings += " AND descontinuado = 1 ";
		}

		// SubQuerie para Obtener los productos que ya existen en el almacen
		subquery_productos.sprintf(" AND (producto, presentacion) NOT IN (SELECT art.producto, art.present \
			FROM articulosxsuc art \
			INNER JOIN sucursales suc ON suc.sucursal = art.sucursal \
			INNER JOIN secciones sec ON sec.sucursal = suc.sucursal \
			INNER JOIN almacenes alm ON alm.seccion = sec.seccion \
			INNER JOIN existenciasactuales ext ON ext.producto = art.producto AND ext.present = art.present AND ext.almacen = alm.almacen \
			WHERE 1 %s \
			GROUP BY art.producto, art.present)",
			wheres);
		havings +=subquery_productos;


		instruccion.sprintf("SELECT \
				exist.producto AS producto, \
				prod.nombre AS nombre, \
				exist.present AS presentacion, \
				TRUNCATE ( SUM( exist.cantidad )/ pmm.maxfactor, 0 ) AS existenciamax, \
				TRUNCATE(MOD ( SUM( exist.cantidad ), pmm.maxfactor ), 3) AS existenciamin, \
				CONCAT( pmm.maxmult, 'X', maxfactor ) AS multmax, \
				(SUM(ap.descontinuado)=	COUNT(DISTINCT exist.almacen)) AS descontinuado, \
				SUM( exist.cantidad ) AS total_exist, \
				TRUNCATE((SUM(ventas%s)/%s)/pmm.maxfactor, 3) as ventaPromedioDiaria \
			FROM existenciasactuales exist \
			INNER JOIN presentacionesminmax pmm ON pmm.producto = exist.producto AND pmm.present = exist.present \
			INNER JOIN almacenes alm ON alm.almacen = exist.almacen \
			INNER JOIN ventasxmes vxm ON exist.producto = vxm.producto AND exist.present = vxm.present AND vxm.almacen = exist.almacen \
			INNER JOIN secciones sec ON sec.seccion = alm.seccion \
            INNER JOIN sucursales suc ON suc.sucursal = sec.sucursal \
			INNER JOIN productos prod ON prod.producto = exist.producto \
			INNER JOIN presentaciones presen ON prod.producto = presen.producto AND presen.present = exist.present \
			LEFT JOIN articulossupervisados artsup ON presen.producto = artsup.producto AND presen.present = artsup.present \
			LEFT JOIN articulosped ap ON ap.producto=exist.producto AND ap.present=exist.present AND ap.sucursal=suc.sucursal \
			WHERE 1 %s %s AND pmm.activo=1 \
			GROUP BY exist.producto,exist.present \
			HAVING 1 %s ORDER BY nombre ASC, exist.present asc ",
			diasVentaPromedio, diasVentaPromedio, wheres, condicion_supervisor, havings);

			mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_REP);
			mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	} __finally {
		if (resp_almacenes!=NULL) delete resp_almacenes;
	}
}


//ID_BUSQ_PAGO_GASTOS
void ServidorBusquedas::BuscaPagosGastos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    // BUSQUEDA DE PAGOS DE PROVEEDOR
    AnsiString instruccion;
    AnsiString tipo_busqueda;
    AnsiString dato_buscado, dato_buscado2, fecha_ini, fecha_fin;

    // Extrae el tipo de búsqueda
    tipo_busqueda=mFg.ExtraeStringDeBuffer(&parametros);
    mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ);

    if (tipo_busqueda=="PROV"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

        instruccion.sprintf("select pag.pago, pag.ident, pag.fecha, if(pro.tipoempre=0,replegal,razonsocial) as nombre, \
			CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS usuario , pag.valor, \
            if(cancelado=1, 'CANC','') as estado \
			from pagosgastos pag, proveedores pro , empleados e, terminales ter, secciones sec, sucursales suc \
            where pag.proveedor=pro.proveedor and \
            pro.razonsocial like '%s%%' and \
			pag.fecha>='%s' and pag.fecha<='%s' AND e.empleado=pag.usualta \
            AND ter.terminal = pag.terminal \
			AND sec.seccion = ter.seccion \
			AND suc.sucursal = sec.sucursal \
			AND suc.idempresa = %s \
            order by pro.razonsocial, pag.fecha, pag.pago limit %s",
                dato_buscado, fecha_ini, fecha_fin, FormServidor->ObtieneClaveEmpresa(), NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="IDEN"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

        instruccion.sprintf("select pag.pago, pag.ident, pag.fecha, if(pro.tipoempre=0,replegal,razonsocial) as nombre, \
			CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS usuario, pag.valor,\
            if(cancelado=1, 'CANC','') as estado \
			from pagosprov pag, proveedores pro , empleados e, terminales ter, secciones sec, sucursales suc \
            where pag.proveedor=pro.proveedor and \
            pag.ident like '%s%%' and \
			pag.fecha>='%s' and pag.fecha<='%s' AND e.empleado=pag.usualta \
            AND ter.terminal = pag.terminal \
			AND sec.seccion = ter.seccion \
			AND suc.sucursal = sec.sucursal \
			AND suc.idempresa = %s \
            order by pag.ident, pro.razonsocial, pag.fecha, pag.pago limit %s",
                dato_buscado, fecha_ini, fecha_fin, FormServidor->ObtieneClaveEmpresa(), NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="FOLI"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

        instruccion.sprintf("select pag.pago, pag.ident, pag.fecha, if(pro.tipoempre=0,replegal,razonsocial) as nombre, \
			CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS usuario , pag.valor,\
            if(cancelado=1, 'CANC','') as estado \
			from pagosgastos pag, proveedores pro , empleados e, terminales ter, secciones sec, sucursales suc \
            where pag.proveedor=pro.proveedor and \
            pag.pago like '%s%%' and \
			pag.fecha>='%s' and pag.fecha<='%s' AND e.empleado=pag.usualta \
            AND ter.terminal = pag.terminal \
			AND sec.seccion = ter.seccion \
			AND suc.sucursal = sec.sucursal \
			AND suc.idempresa = %s \
            order by pag.pago, pro.razonsocial, pag.fecha limit %s",
                dato_buscado, fecha_ini, fecha_fin, FormServidor->ObtieneClaveEmpresa(), NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="FECH"){
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

        instruccion.sprintf("select pag.pago, pag.ident, pag.fecha, if(pro.tipoempre=0,replegal,razonsocial) as nombre, \
			CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS usuario , pag.valor,\
            if(cancelado=1, 'CANC','') as estado \
			from pagosgastos pag, proveedores pro , empleados e, terminales ter, secciones sec, sucursales suc \
            where pag.proveedor=pro.proveedor and \
			pag.fecha>='%s' and pag.fecha<='%s' AND e.empleado=pag.usualta \
            AND ter.terminal = pag.terminal \
			AND sec.seccion = ter.seccion \
			AND suc.sucursal = sec.sucursal \
			AND suc.idempresa = %s \
            order by pro.razonsocial, pag.fecha, pag.pago limit %s",
                fecha_ini, fecha_fin, FormServidor->ObtieneClaveEmpresa(), NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
    if (tipo_busqueda=="CHEQ"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros); // BANCO
        dato_buscado2=mFg.ExtraeStringDeBuffer(&parametros); // NUM CHEQUE
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

        instruccion.sprintf("select pag.pago, pag.ident, pag.fecha, if(pro.tipoempre=0,replegal,razonsocial) as nombre, \
			CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS usuario , pag.valor, \
            if(cancelado=1, 'CANC','') as estado \
			from pagosgastos pag, proveedores pro, transxpaggastos tra, \
			transxpaggastos chxp, chequesgastos chpro, empleados e, terminales ter, secciones sec, sucursales suc \
            where pag.proveedor=pro.proveedor and \
            tra.pago=pag.pago and chxp.pago=tra.pago and \
			chxp.chequegasto=chpro.chequegasto and chpro.banco='%s' and \
            chpro.folio like '%s%%' and \
			pag.fecha>='%s' and pag.fecha<='%s' AND e.empleado=pag.usualta \
            AND ter.terminal = pag.terminal \
			AND sec.seccion = ter.seccion \
			AND suc.sucursal = sec.sucursal \
			AND suc.idempresa = %s \
            order by chpro.folio, pag.pago limit %s",
                dato_buscado, dato_buscado2, fecha_ini, fecha_fin, FormServidor->ObtieneClaveEmpresa(), NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="GASTO"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select pag.pago, pag.ident, pag.fecha, if(pro.tipoempre=0,replegal,razonsocial) as nombre, \
            CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS usuario , pag.valor,\
            if(pag.cancelado=1, 'CANC','') as estado \
			from pagosgastos pag, proveedores pro, transxpaggastos tra, \
			gastos gas , empleados e, terminales ter, secciones sec, sucursales suc \
            where pag.proveedor=pro.proveedor and \
			tra.pago=pag.pago and tra.referencia=gas.referencia and \
			gas.referencia like '%s%%' and \
			pag.fecha>='%s' and pag.fecha<='%s' AND e.empleado=pag.usualta \
            AND ter.terminal = pag.terminal \
			AND sec.seccion = ter.seccion \
			AND suc.sucursal = sec.sucursal \
			AND suc.idempresa = %s \
            group by pag.pago \
            order by pag.pago, pro.razonsocial, pag.fecha limit %s",
                dato_buscado, fecha_ini, fecha_fin, FormServidor->ObtieneClaveEmpresa(), NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }

    if (tipo_busqueda=="") {
        // Obtiene todos los bancos
        instruccion="select banco, nombre from bancos order by nombre, banco";
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
}
//---------------------------------------------------------------------------
//ID_BUSQ_NOTA_GASTO
void ServidorBusquedas::BuscaNotasCredGasto(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    // BUSQUEDA DE NOTAS DE CREDITO DE PROVEEDOR
    DatosTabla datos(mServidorVioleta->Tablas);
    AnsiString instruccion;
    AnsiString tipo_busqueda, tipo_nota;
    AnsiString dato_buscado, fecha_ini, fecha_fin;

    // Extrae el tipo de búsqueda
    tipo_busqueda=mFg.ExtraeStringDeBuffer(&parametros);
    // Extrae el tipo de nota de crédito que se está buscando.
    // tipo_nota=mFg.ExtraeStringDeBuffer(&parametros);

    mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ);

    if (tipo_busqueda=="PROV"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		instruccion.sprintf("select n.fechanot, n.referencia, n.gasto, n.folioprov, \
			p.razonsocial, n.valor, concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, \
            if (n.cancelado=1, 'CANC','') as estado \
			from notascredgasto n, gastos g, empleados em, proveedores p \
			where p.razonsocial like '%s%%' and g.proveedor=p.proveedor and \
			n.gasto=g.referencia and n.usumodi=em.empleado and \
			n.fechanot>='%s' and n.fechanot<='%s' \
			order by p.razonsocial, n.fechanot, n.referencia limit %s", dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="FOLIPROV"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select n.fechanot, n.referencia, n.gasto, n.folioprov, \
            p.razonsocial, n.valor, concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, \
            if (n.cancelado=1, 'CANC','') as estado \
			from notascredgasto n, gastos g, empleados em, proveedores p \
			where n.folioprov like '%s%%' and g.proveedor=p.proveedor and \
			n.gasto=g.referencia and n.usumodi=em.empleado and n.fechanot>='%s' and n.fechanot<='%s' \
			order by p.razonsocial, n.fechanot, n.referencia limit %s", dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="REFE"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select n.fechanot, n.referencia, g.referencia as gasto, \
            n.folioprov, p.razonsocial, n.valor, \
            concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, \
            if (n.cancelado=1, 'CANC','') as estado \
			from notascredgasto n, gastos g, empleados em, proveedores p \
			where n.referencia='%s' and g.proveedor=p.proveedor and n.gasto=g.referencia and \
			n.usumodi=em.empleado and n.fechanot>='%s' and n.fechanot<='%s' \
			order by p.razonsocial, n.fechanot, n.referencia limit %s", dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
	if (tipo_busqueda=="GASTO") {
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select n.fechanot, n.referencia, g.referencia as gasto, \
            n.folioprov, p.razonsocial, n.valor, \
            concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, \
            if (n.cancelado=1, 'CANC','') as estado \
			from notascredgasto n, gastos g, empleados em, proveedores p \
			where n.gasto='%s' and g.proveedor=p.proveedor and n.gasto=g.referencia and \
			n.usumodi=em.empleado and n.fechanot>='%s' and n.fechanot<='%s' \
			order by p.razonsocial, n.fechanot, n.referencia limit %s", dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="FECH"){
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select n.fechanot, n.referencia, g.referencia as gasto, \
            n.folioprov, p.razonsocial, n.valor, concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, \
			if (n.cancelado=1, 'CANC','') as estado \
			from notascredgasto n, gastos g, empleados em, proveedores p \
			where g.proveedor=p.proveedor and n.gasto=g.referencia and \
			n.usumodi=em.empleado and n.fechanot>='%s' and n.fechanot<='%s' \
			order by p.razonsocial, n.fechanot, n.referencia limit %s", fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="USUA"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select n.fechanot, n.referencia, g.referencia as gasto, \
            n.folioprov, p.razonsocial, n.valor, \
            concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, \
            if (n.cancelado=1, 'CANC','') as estado \
			from notascredgasto n, gastos g, empleados em, proveedores p \
            where em.empleado='%s' and g.proveedor=p.proveedor and n.gasto=g.referencia and \
			n.usumodi=em.empleado and n.fechanot>='%s' and n.fechanot<='%s' \
            order by p.razonsocial, n.fechanot, n.referencia limit %s", dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}

    if (tipo_busqueda=="") {
        // Obtiene todos los usuarios
        instruccion="select usuarios.empleado AS Usuario, empleados.nombre AS Nombre ";
        instruccion+="from usuarios, empleados ";
        instruccion+="where empleados.empleado=usuarios.empleado order by nombre";
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
}
//---------------------------------------------------------------------------
//ID_BUSQ_GASTOS
void ServidorBusquedas::BuscaGastos(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    // BUSQUEDA DE GASTOS
    DatosTabla datos(mServidorVioleta->Tablas);
    AnsiString instruccion;
    AnsiString tipo_busqueda;
    AnsiString dato_buscado, fecha_ini, fecha_fin;

    // Extrae el tipo de búsqueda
    tipo_busqueda=mFg.ExtraeStringDeBuffer(&parametros);
    mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ);

    if (tipo_busqueda=="PROV"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select g.sucursal ,g.fechagas, g.referencia, g.folioprov, p.razonsocial, g.valor, g.anticipo, \
			concat(ec.nombre, ' ', ec.appat, ' ', ec.apmat) as solicita, a.descripcion, \
			concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, if (g.cancelado=1, 'CANC','') as estado \
			from gastos g, tiposdegastos a, empleados ec, empleados em, proveedores p WHERE p.razonsocial like '%s%%' \
		and g.proveedor=p.proveedor and g.tipogasto=a.tipo \
		and g.solicita=ec.empleado and g.usualta=em.empleado and g.fechagas>='%s' and g.fechagas<='%s'\
		order by p.razonsocial, g.fechagas, g.referencia limit %s",	dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="FOLIPROV"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select g.sucursal, g.fechagas, g.referencia, g.folioprov, p.razonsocial, g.valor, g.anticipo, \
			concat(ec.nombre, ' ', ec.appat, ' ', ec.apmat) as solicita, a.descripcion, \
			concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, if (g.cancelado=1, 'CANC','') as estado \
			from gastos g, tiposdegastos a, empleados ec, empleados em, proveedores p WHERE g.folioprov like '%s%%' \
		and g.proveedor=p.proveedor and g.tipogasto=a.tipo \
		and g.solicita=ec.empleado and g.usualta=em.empleado and g.fechagas>='%s' and g.fechagas<='%s'\
		order by p.razonsocial, g.fechagas, g.referencia limit %s",
		dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="REFE"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select g.sucursal, g.fechagas, g.referencia, g.folioprov, p.razonsocial, g.valor, g.anticipo, \
			concat(ec.nombre, ' ', ec.appat, ' ', ec.apmat) as solicita, a.descripcion, \
			concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, if (g.cancelado=1, 'CANC','') as estado \
			from gastos g, tiposdegastos a, empleados ec, empleados em, proveedores p WHERE g.referencia='%s' \
		and g.proveedor=p.proveedor and g.tipogasto=a.tipo \
		and g.solicita=ec.empleado and g.usualta=em.empleado and g.fechagas>='%s' and g.fechagas<='%s'\
		order by p.razonsocial, g.fechagas, g.referencia limit %s",
		dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="FECH"){
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select g.sucursal, g.fechagas, g.referencia, g.folioprov, p.razonsocial, g.valor, g.anticipo, \
			concat(ec.nombre, ' ', ec.appat, ' ', ec.apmat) as solicita, a.descripcion, \
			concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, if (g.cancelado=1, 'CANC','') as estado \
			from gastos g, tiposdegastos a, empleados ec, empleados em, proveedores p WHERE \
		g.proveedor=p.proveedor and g.tipogasto=a.tipo \
		and g.solicita=ec.empleado and g.usualta=em.empleado and g.fechagas>='%s' and g.fechagas<='%s'\
		order by p.razonsocial, g.fechagas, g.referencia limit %s",
		fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="ALMA"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select g.sucursal, g.fechagas, g.referencia, g.folioprov, p.razonsocial, g.valor, g.anticipo, \
			concat(ec.nombre, ' ', ec.appat, ' ', ec.apmat) as solicita, a.descripcion, \
			concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, if (g.cancelado=1, 'CANC','') as estado \
			from gastos g, tiposdegastos a, empleados ec, empleados em, proveedores p WHERE g.tipogasto='%s' and\
		g.proveedor=p.proveedor and g.tipogasto=a.tipo \
		and g.solicita=ec.empleado and g.usualta=em.empleado and g.fechagas>='%s' and g.fechagas<='%s'\
		order by p.razonsocial, g.fechagas, g.referencia limit %s",
		dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="COMP"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select g.sucursal, g.fechagas, g.referencia, g.folioprov, p.razonsocial, g.valor, g.anticipo, \
			concat(ec.nombre, ' ', ec.appat, ' ', ec.apmat) as solicita, a.descripcion, \
			concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, if (g.cancelado=1, 'CANC','') as estado \
			from gastos g, tiposdegastos a, empleados ec, empleados em, proveedores p WHERE g.solicita='%s' and\
		g.proveedor=p.proveedor and g.tipogasto=a.tipo \
		and g.solicita=ec.empleado and g.usualta=em.empleado and g.fechagas>='%s' and g.fechagas<='%s'\
		order by p.razonsocial, g.fechagas, g.referencia limit %s",
		dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="USUA"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select g.sucursal, g.fechagas, g.referencia, g.folioprov, p.razonsocial, g.valor, g.anticipo, \
			concat(ec.nombre, ' ', ec.appat, ' ', ec.apmat) as solicita, a.descripcion, \
			concat(em.nombre, ' ', em.appat, ' ', em.apmat) as usuario, if (g.cancelado=1, 'CANC','') as estado \
			from gastos g, tiposdegastos a, empleados ec, empleados em, proveedores p WHERE g.usualta='%s' and\
		g.proveedor=p.proveedor and g.tipogasto=a.tipo \
		and g.solicita=ec.empleado and g.usualta=em.empleado and g.fechagas>='%s' and g.fechagas<='%s'\
		order by p.razonsocial, g.fechagas, g.referencia limit %s",
		dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }

    if (tipo_busqueda=="") {
        // Obtiene todos los posibles almacenes
        instruccion="SELECT tipo, descripcion  FROM tiposdegastos order by tipo";
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

        // Obtiene todos los empleados como posibles compradores
		instruccion="select empleados.empleado, concat(empleados.nombre,' ',empleados.appat,' ',empleados.apmat) AS Nombre \
		from empleados where empleados.activo=1 \
		order by empleados.nombre,empleados.appat,empleados.apmat";
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);

		// Obtiene todos los usuarios

		instruccion="select asigtotal.empleado, asigtotal.Nombre \
		FROM ((SELECT e.empleado, CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS Nombre \
		FROM usuarios AS u INNER JOIN empleados e ON e.empleado=u.empleado \
		INNER JOIN asignacionprivilegios AS a ON a.usuario=e.empleado \
		WHERE a.objeto='GASTOS' AND e.activo=1 AND a.privilegio='AUT' \
		ORDER BY e.nombre,e.appat,e.apmat ) UNION (SELECT e.empleado, CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS Nombre \
		FROM usuarios AS u INNER JOIN empleados e ON e.empleado=u.empleado \
		INNER JOIN usuariorol ur ON ur.usuario = e.empleado \
		INNER JOIN asignacionprivrol ar ON ur.rol = ar.rol \
		WHERE ar.objeto='GASTOS' AND e.activo=1 AND ar.privilegio='AUT' \
		ORDER BY e.nombre,e.appat,e.apmat ) UNION (SELECT u.empleado, CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS Nombre \
		FROM usuarios u INNER JOIN empleados e ON e.empleado = u.empleado INNER JOIN puestos p ON p.puesto = e.puesto \
		INNER JOIN rolesxpuesto rxp ON rxp.puesto = p.puesto INNER JOIN asignacionprivrol ar ON rxp.rol=ar.rol \
		WHERE ar.objeto='GASTOS' AND e.activo=1 AND ar.privilegio='AUT' \
		ORDER BY e.nombre,e.appat,e.apmat )) AS asigtotal";

        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}

}
//---------------------------------------------------------------------------
//ID_BUSQ_CARTA_20
void ServidorBusquedas::BuscaCartaPorte20(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// BUSQUEDA DE PAGOS DE CLIENTE
    DatosTabla datos(mServidorVioleta->Tablas);
    AnsiString instruccion;
    AnsiString tipo_busqueda;
    AnsiString dato_buscado, dato_buscado2, fecha_ini, fecha_fin;

    // Extrae el tipo de búsqueda
    tipo_busqueda=mFg.ExtraeStringDeBuffer(&parametros);
    mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ);

    if (tipo_busqueda=="CLI"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("SELECT t.* FROM (( select carta2.cartaporte20, carta2.fechacp, cli.rsocial as nombre, \
		CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS usuario, v.valor, \
		if(carta2.cancelada=1, 'CANC','') as estado from cartasporte20 carta2 \
		INNER JOIN ventas v ON v.referencia = carta2.referencia \
		inner join clientes cli ON cli.cliente = v.cliente \
		INNER join empleados e ON e.empleado=carta2.usualta \
		where cli.rsocial LIKE '%s%%' AND \
		carta2.fechacp >='%s' and carta2.fechacp<='%s' \
		order by cli.rsocial, carta2.fechacp, carta2.cartaporte20 \
		)UNION ALL ( select carta2.cartaporte20, carta2.fechacp, cli.rsocial as nombre, \
		CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS usuario, p.valor, \
		if(carta2.cancelada=1, 'CANC','') as estado \
		from cartasporte20 carta2 \
		INNER JOIN pedidosventa p ON p.referencia = carta2.pedido \
		inner join clientes cli ON cli.cliente = p.cliente \
		INNER join empleados e ON e.empleado=carta2.usualta \
		WHERE cli.rsocial LIKE '%s%%' and \
		carta2.fechacp >='%s' and carta2.fechacp<='%s' \
		order by cli.rsocial, carta2.fechacp, carta2.cartaporte20 )) t LIMIT %s ",
		dato_buscado, fecha_ini, fecha_fin,
		dato_buscado, fecha_ini, fecha_fin,
		NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
    if (tipo_busqueda=="FOLI"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("SELECT t.* FROM ((SELECT carta20.cartaporte20, carta20.fechacp, \
		cli.rsocial AS nombre, CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS usuario, \
		v.valor, if(carta20.cancelada=1, 'CANC','') AS estado FROM cartasporte20 carta20 \
		INNER JOIN ventas v ON v.referencia = carta20.referencia \
		inner join clientes cli ON cli.cliente = v.cliente inner join empleados e ON e.empleado=carta20.usualta \
		WHERE carta20.cartaporte20 LIKE '%s%%' AND carta20.fechacp >='%s' AND carta20.fechacp <='%s' \
		ORDER BY carta20.cartaporte20, cli.rsocial, carta20.fechacp, carta20.cartaporte20 \
		)UNION ALL ( SELECT carta20.cartaporte20, carta20.fechacp, cli.rsocial AS nombre, \
		CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS usuario, p.valor, \
		if(carta20.cancelada=1, 'CANC','') AS estado FROM cartasporte20 carta20 \
		INNER JOIN pedidosventa p ON p.referencia = carta20.pedido \
		inner join clientes cli ON cli.cliente = p.cliente \
		inner join empleados e ON e.empleado=carta20.usualta \
		WHERE carta20.cartaporte20 LIKE '%s%%' AND carta20.fechacp >='%s' AND carta20.fechacp <='%s' \
		ORDER BY carta20.cartaporte20, cli.rsocial, carta20.fechacp, carta20.cartaporte20 \
		)) t ORDER BY t.cartaporte20, t.nombre, t.fechacp, t.cartaporte20 limit %s",
		dato_buscado, fecha_ini, fecha_fin,
		dato_buscado, fecha_ini, fecha_fin,
		NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="FECH"){
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("SELECT t.* FROM((SELECT carta20.cartaporte20, carta20.fechacp, \
		cli.rsocial AS nombre, CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS usuario, v.valor, \
		if(carta20.cancelada=1, 'CANC','') AS estado FROM cartasporte20 carta20 \
		INNER JOIN ventas v ON v.referencia = carta20.referencia \
		INNER JOIN clientes cli ON cli.cliente = v.cliente \
		INNER JOIN empleados e ON e.empleado=carta20.usualta \
		WHERE carta20.fechacp>='%s' AND carta20.fechacp<='%s' \
		) UNION ALL (SELECT carta20.cartaporte20, carta20.fechacp, cli.rsocial AS nombre,  \
		CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS usuario, p.valor, \
		if(carta20.cancelada=1, 'CANC','') AS estado FROM cartasporte20 carta20 \
		INNER JOIN pedidosventa p ON p.referencia = carta20.pedido \
		INNER JOIN clientes cli ON cli.cliente = p.cliente \
		INNER JOIN empleados e ON e.empleado=carta20.usualta \
		WHERE carta20.fechacp>='%s' AND carta20.fechacp<='%s' \
		)) t ORDER BY t.fechacp, t.nombre, t.fechacp limit %s",
		fecha_ini, fecha_fin,
		fecha_ini, fecha_fin,
		NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
	if (tipo_busqueda=="PEDIDO"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("SELECT carta20.cartaporte20, carta20.fechacp, \
		cli.rsocial AS nombre, CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS usuario, \
		p.valor, if(carta20.cancelada=1, 'CANC','') AS estado FROM cartasporte20 carta20 \
		INNER JOIN pedidosventa p ON p.referencia = carta20.pedido \
		INNER JOIN clientes cli ON cli.cliente = p.cliente \
		INNER JOIN empleados e ON e.empleado=carta20.usualta \
		WHERE carta20.fechacp>='%s' AND carta20.fechacp<='%s' AND p.referencia = '%s' \
		ORDER BY carta20.cartaporte20, cli.rsocial, carta20.fechacp, carta20.cartaporte20 limit %s",
		fecha_ini, fecha_fin, dato_buscado, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="VENT"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("SELECT carta20.cartaporte20, carta20.fechacp, \
		cli.rsocial AS nombre, CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS usuario, v.valor, \
		if(carta20.cancelada=1, 'CANC','') AS estado FROM cartasporte20 carta20 \
		INNER JOIN ventas v ON v.referencia = carta20.referencia \
		INNER JOIN clientes cli ON cli.cliente = v.cliente \
		INNER JOIN empleados e ON e.empleado=carta20.usualta \
		WHERE carta20.fechacp>='%s' AND carta20.fechacp<='%s' AND v.referencia = '%s' \
		ORDER BY carta20.cartaporte20, cli.rsocial, carta20.fechacp, carta20.cartaporte20 limit %s",
		fecha_ini, fecha_fin, dato_buscado, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
    if (tipo_busqueda=="SERIEFOL"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("SELECT t.* FROM((SELECT carta20.cartaporte20, carta20.fechacp, \
		cli.rsocial AS nombre, CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS usuario, v.valor, \
		if(carta20.cancelada=1, 'CANC','') AS estado \
		FROM cartasporte20 carta20 \
		INNER JOIN ventas v ON v.referencia = carta20.referencia \
		INNER JOIN clientes cli ON cli.cliente = v.cliente \
		INNER JOIN empleados e ON e.empleado=carta20.usualta \
		INNER JOIN cfd c ON c.referencia = carta20.cartaporte20 \
		WHERE carta20.fechacp>='%s' AND carta20.fechacp<='%s' \
		AND c.seriefolio='%s' AND c.tipocomprobante = 'CAR2' \
		) UNION ALL ( \
		SELECT carta20.cartaporte20, carta20.fechacp, \
		cli.rsocial AS nombre, CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS usuario, p.valor, \
		if(carta20.cancelada=1, 'CANC','') AS estado \
		FROM cartasporte20 carta20 \
		INNER JOIN pedidosventa p ON p.referencia = carta20.pedido \
		INNER JOIN clientes cli ON cli.cliente = p.cliente \
		INNER JOIN empleados e ON e.empleado=carta20.usualta \
		INNER JOIN cfd c ON c.referencia = carta20.cartaporte20 \
		WHERE carta20.fechacp>='%s' AND carta20.fechacp<='%s' \
		AND c.seriefolio='%s' AND c.tipocomprobante = 'CAR2' \
		))t ORDER BY t.cartaporte20, t.nombre, t.fechacp, t.cartaporte20 LIMIT %s",
		fecha_ini, fecha_fin, dato_buscado,
		fecha_ini, fecha_fin, dato_buscado,
		NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}

    if (tipo_busqueda=="") {
	}
}

//---------------------------------------------------------------------------
//ID_BUSQ_PREPAGO_CLI
void ServidorBusquedas::BuscaPrePagosCli(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    // BUSQUEDA DE PAGOS DE CLIENTE
    DatosTabla datos(mServidorVioleta->Tablas);
    AnsiString instruccion;
    AnsiString tipo_busqueda;
    AnsiString dato_buscado, dato_buscado2, fecha_ini, fecha_fin;

    // Extrae el tipo de búsqueda
    tipo_busqueda=mFg.ExtraeStringDeBuffer(&parametros);
    mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ);

    if (tipo_busqueda=="CLI"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select pag.pago, pag.fecha, cli.rsocial as nombre,cli.nomnegocio AS negocio, \
		CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS usuario , pag.valor, \
		if(cancelado=1, 'CANC','') as estado , IFNULL(pag.identificador,'') as identificador\
		from prepagoscli pag \
		INNER JOIN clientes cli ON pag.cliente=cli.cliente \
		LEFT JOIN empleados e ON e.empleado=pag.cobrador \
		where cli.rsocial like '%s%%' and \
		pag.fecha>='%s' and pag.fecha<='%s' AND pag.aplicado = 0 \
		AND pag.cancelado = 0 \
		order by cli.rsocial, pag.fecha, pag.pago limit %s",
		dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
    if (tipo_busqueda=="FOLI"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select pag.pago, pag.fecha, cli.rsocial as nombre,cli.nomnegocio AS negocio, \
		CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS usuario , pag.valor, \
		if(cancelado=1, 'CANC','') as estado, IFNULL(pag.identificador,'') as identificador \
		from prepagoscli pag \
		INNER JOIN clientes cli ON pag.cliente=cli.cliente \
		LEFT JOIN empleados e ON e.empleado=pag.cobrador \
		where pag.pago like '%s%%' and \
		pag.fecha>='%s' and pag.fecha<='%s' AND pag.aplicado = 0 \
		AND pag.cancelado = 0 \
		order by pag.pago, cli.rsocial, pag.fecha, pag.pago limit %s",
		dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="FECH"){
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select pag.pago, pag.fecha, cli.rsocial as nombre,cli.nomnegocio AS negocio, \
		CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS usuario , pag.valor, \
		if(cancelado=1, 'CANC','') as estado, IFNULL(pag.identificador,'') as identificador \
		from prepagoscli pag \
		INNER JOIN clientes cli ON pag.cliente=cli.cliente \
		LEFT JOIN empleados e ON e.empleado=pag.cobrador \
		where pag.cliente=cli.cliente and \
		pag.fecha>='%s' and pag.fecha<='%s' AND pag.aplicado = 0 \
		AND pag.cancelado = 0 \
		order by pag.fecha, cli.rsocial, pag.pago limit %s",
		fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="AGENT"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros); // AGENTE
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select pag.pago, pag.fecha, cli.rsocial as nombre,cli.nomnegocio AS negocio, \
		CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS usuario , pag.valor, \
		if(cancelado=1, 'CANC','') as estado, IFNULL(pag.identificador,'') as identificador \
		from prepagoscli pag \
		INNER JOIN clientes cli ON pag.cliente=cli.cliente \
		LEFT JOIN empleados e ON e.empleado=pag.cobrador \
		where pag.cobrador = '%s' and \
		pag.fecha>='%s' and pag.fecha<='%s' AND pag.aplicado = 0 \
		AND pag.cancelado = 0 \
		order by pag.pago limit %s",
		dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="VENT"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select pag.pago, pag.fecha, cli.rsocial as nombre,cli.nomnegocio AS negocio , \
		CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS usuario, SUM(pagd.valor) AS valor, \
		if(cancelado=1, 'CANC','') as estado, IFNULL(pag.identificador,'') as identificador \
		from prepagoscli pag \
		INNER JOIN predpagoscli pagd ON pagd.pago = pag.pago \
		INNER JOIN clientes cli ON pag.cliente=cli.cliente \
		LEFT JOIN empleados e ON e.empleado=pag.cobrador \
		where pagd.referencia like '%s%%' and \
		pag.fecha>='%s' and pag.fecha<='%s' AND pag.aplicado = 0 \
		AND pag.cancelado = 0 \
		GROUP BY pagd.pago \
		order by pag.pago, cli.rsocial, pag.fecha, pag.pago limit %s",
		dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
    if (tipo_busqueda=="IDENT"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("select pag.pago, pag.fecha, cli.rsocial as nombre,cli.nomnegocio AS negocio , \
		CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS usuario, SUM(pagd.valor) AS valor, \
		if(cancelado=1, 'CANC','') as estado, IFNULL(pag.identificador,'') as identificador \
		from prepagoscli pag \
		INNER JOIN predpagoscli pagd ON pagd.pago = pag.pago \
		INNER JOIN clientes cli ON pag.cliente=cli.cliente \
		LEFT JOIN empleados e ON e.empleado=pag.cobrador \
		where pag.identificador like '%s%%' and \
		pag.fecha>='%s' and pag.fecha<='%s' AND pag.aplicado = 0 \
		AND pag.cancelado = 0 \
		GROUP BY pagd.pago \
		order by pag.pago, cli.rsocial, pag.fecha, pag.pago limit %s",
		dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}

    if (tipo_busqueda=="") {
		// Obtiene todos los agentes
        instruccion="select empleados.empleado, concat(empleados.nombre,' ',empleados.appat,' ',empleados.apmat) \
		AS Nombre from empleados, vendedores \
		where empleados.empleado=vendedores.empleado and vendedores.activo=1 \
		and empleados.activo=1 order by empleados.nombre,empleados.appat,empleados.apmat";
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
}
//---------------------------------------------------------------------------
//ID_BUSQ_PEDIDO_ECOM
void ServidorBusquedas::BuscaPedidosEcom(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
    // BUSQUEDA DE PAGOS DE CLIENTE
    DatosTabla datos(mServidorVioleta->Tablas);
    AnsiString instruccion;
    AnsiString tipo_busqueda;
    AnsiString dato_buscado, dato_buscado2, fecha_ini, fecha_fin;

    // Extrae el tipo de búsqueda
    tipo_busqueda=mFg.ExtraeStringDeBuffer(&parametros);
    mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ);

    if (tipo_busqueda=="CLI"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("SELECT p.referencia, p.fechaped, \
		CONCAT(cli.nombre,' ',cli.appat,' ',cli.apmat) AS cliente, \
		tc.descripcion AS tipocompra, ep.descripcion AS estadopago, \
		fp.descripcion, p.valor FROM pedidosecomm p \
		INNER JOIN dpedidosfpago dfp ON dfp.referencia = p.referencia \
		INNER JOIN formasdepago fp ON fp.formapag = dfp.formapag \
		INNER JOIN estatuspago ep ON ep.id = p.estadopago \
		INNER JOIN tiposcompra tc ON tc.id = p.tipocompra \
		INNER JOIN clientes cli ON p.cliente=cli.cliente \
		WHERE p.venta IS NULL AND cli.rsocial like '%s%%' \
		AND p.fechaped BETWEEN '%s' AND '%s' AND p.estadopedido = 2 \
		ORDER BY p.fechaped DESC, p.horaped DESC limit %s",
		dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
    if (tipo_busqueda=="FOLI"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("SELECT p.referencia, p.fechaped, \
		CONCAT(cli.nombre,' ',cli.appat,' ',cli.apmat) AS cliente, \
		tc.descripcion AS tipocompra, ep.descripcion AS estadopago, \
		fp.descripcion, p.valor FROM pedidosecomm p \
		INNER JOIN dpedidosfpago dfp ON dfp.referencia = p.referencia \
		INNER JOIN formasdepago fp ON fp.formapag = dfp.formapag \
		INNER JOIN estatuspago ep ON ep.id = p.estadopago \
		INNER JOIN tiposcompra tc ON tc.id = p.tipocompra \
		INNER JOIN clientes cli ON p.cliente=cli.cliente \
		WHERE p.venta IS NULL AND p.referencia like '%s%%' \
		AND p.fechaped BETWEEN '%s' AND '%s' AND p.estadopedido = 2 \
		ORDER BY p.fechaped DESC, p.horaped DESC limit %s",
		dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="FECH"){
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("SELECT p.referencia, p.fechaped, \
		CONCAT(cli.nombre,' ',cli.appat,' ',cli.apmat) AS cliente, \
		tc.descripcion AS tipocompra, ep.descripcion AS estadopago, \
		fp.descripcion, p.valor FROM pedidosecomm p \
		INNER JOIN dpedidosfpago dfp ON dfp.referencia = p.referencia \
		INNER JOIN formasdepago fp ON fp.formapag = dfp.formapag \
		INNER JOIN estatuspago ep ON ep.id = p.estadopago \
		INNER JOIN tiposcompra tc ON tc.id = p.tipocompra \
		INNER JOIN clientes cli ON p.cliente=cli.cliente \
		WHERE p.venta IS NULL AND fechaped BETWEEN '%s' AND '%s' AND p.estadopedido = 2 \
		ORDER BY p.fechaped DESC, p.horaped DESC limit %s",
		fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }


    if (tipo_busqueda=="") {
		// Obtiene todos los agentes
        instruccion="select empleados.empleado, concat(empleados.nombre,' ',empleados.appat,' ',empleados.apmat) \
		AS Nombre from empleados, vendedores \
		where empleados.empleado=vendedores.empleado and vendedores.activo=1 \
		and empleados.activo=1 order by empleados.nombre,empleados.appat,empleados.apmat";
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
}
//---------------------------------------------------------------------------
//ID_BUSQ_RECEPCION
void ServidorBusquedas::BuscaRecepcion(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{
	// BUSQUEDA DE PAGOS DE CLIENTE
	DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instruccion;
	AnsiString tipo_busqueda;
	AnsiString dato_buscado, dato_buscado2, fecha_ini, fecha_fin, recepxomitir;
	AnsiString condicion_recepxomitir=" ", condicion_datobuscado=" ";

	// Extrae el tipo de búsqueda
	tipo_busqueda=mFg.ExtraeStringDeBuffer(&parametros);
	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ);

    if (tipo_busqueda=="REF"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("SELECT \
			r.recepcion, pro.razonsocial, r.fecharep, r.devolucion, \
			CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS recepcionista, \
			IF(COUNT(c.referencia) > 0, 1, 0) AS tieneCompra \
			FROM recepciones r \
			INNER JOIN proveedores pro ON pro.proveedor = r.proveedor \
			INNER JOIN empleados e ON e.empleado = r.usualta \
			INNER JOIN terminales t ON t.terminal = r.terminal \
			INNER JOIN secciones sec ON sec.seccion = t.seccion \
			LEFT JOIN pedrecepcion pr ON pr.recepcion = r.recepcion \
			LEFT JOIN pedidos p ON p.referencia = pr.pedido \
			LEFT JOIN compraspedidosprov cpp ON cpp.recepcion = r.recepcion \
			LEFT JOIN compras c ON c.referencia = cpp.compra AND c.cancelado = 0 \
			WHERE p.referencia = '%s' \
			AND p.fechaalta BETWEEN '%s' AND '%s' \
			AND sec.sucursal = '%s' \
			GROUP BY r.recepcion LIMIT %s",
			dato_buscado, fecha_ini, fecha_fin,
			FormServidor->ObtieneClaveSucursal(),
			NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="REC"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("SELECT \
			r.recepcion, pro.razonsocial, r.fecharep, r.devolucion, \
			CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS recepcionista, \
			IF(COUNT(c.referencia) > 0, 1, 0) AS tieneCompra \
			FROM recepciones r \
			INNER JOIN proveedores pro ON pro.proveedor = r.proveedor \
			INNER JOIN empleados e ON e.empleado = r.usualta \
			INNER JOIN terminales t ON t.terminal = r.terminal \
			INNER JOIN secciones sec ON sec.seccion = t.seccion \
			LEFT JOIN compraspedidosprov cpp ON cpp.recepcion = r.recepcion \
			LEFT JOIN compras c ON c.referencia = cpp.compra AND c.cancelado = 0 \
			WHERE r.recepcion = '%s' \
			AND r.fecharep BETWEEN '%s' AND '%s' \
			AND sec.sucursal = '%s' \
			GROUP BY r.recepcion LIMIT %s",
			dato_buscado, fecha_ini, fecha_fin,
			FormServidor->ObtieneClaveSucursal(),
			NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="PROV"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		recepxomitir=mFg.ExtraeStringDeBuffer(&parametros);

		if(recepxomitir!=""){
			condicion_recepxomitir.sprintf(" AND r.recepcion NOT IN (%s)", recepxomitir);
		}

		instruccion.sprintf("SELECT \
			r.recepcion, pro.razonsocial, r.fecharep, r.devolucion, \
            CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS recepcionista, \
			IF(COUNT(c.referencia) > 0, 1, 0) AS tieneCompra \
			FROM recepciones r \
			INNER JOIN proveedores pro ON pro.proveedor = r.proveedor \
			INNER JOIN empleados e ON e.empleado = r.usualta \
			INNER JOIN terminales t ON t.terminal = r.terminal \
			INNER JOIN secciones sec ON sec.seccion = t.seccion \
			LEFT JOIN compraspedidosprov cpp ON cpp.recepcion = r.recepcion \
			LEFT JOIN compras c ON c.referencia = cpp.compra AND c.cancelado = 0 \
			WHERE pro.razonsocial LIKE '%s%%' \
			AND r.fecharep BETWEEN '%s' AND '%s' AND sec.sucursal = '%s' \
			%s \
			GROUP BY r.recepcion LIMIT %s",
			dato_buscado, fecha_ini, fecha_fin,
			FormServidor->ObtieneClaveSucursal(),
			condicion_recepxomitir,
			NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
    if (tipo_busqueda=="FACT"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("SELECT \
			r.recepcion, pro.razonsocial, r.fecharep, r.devolucion, \
			CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS recepcionista, \
			IF(COUNT(c.referencia) > 0, 1, 0) AS tieneCompra \
			FROM recepciones r \
			INNER JOIN proveedores pro ON pro.proveedor = r.proveedor \
			INNER JOIN empleados e ON e.empleado = r.usualta \
			INNER JOIN terminales t ON t.terminal = r.terminal \
			INNER JOIN secciones sec ON sec.seccion = t.seccion \
			LEFT JOIN pedrecepcion pr ON pr.recepcion = r.recepcion \
			LEFT JOIN pedidos p ON p.referencia = pr.pedido \
			LEFT JOIN compraspedidosprov cpp ON cpp.recepcion = r.recepcion \
			LEFT JOIN compras c ON c.referencia = cpp.compra AND c.cancelado = 0 \
			WHERE p.foliofactura LIKE '%%%s%%' \
			AND r.fecharep BETWEEN '%s' AND '%s' AND sec.sucursal = '%s' \
			GROUP BY r.recepcion LIMIT %s",
			dato_buscado, fecha_ini, fecha_fin,
			FormServidor->ObtieneClaveSucursal(),
			NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="COMP"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
		fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		if(dato_buscado!=""){
			condicion_datobuscado.sprintf(" AND c.comprador='%s' ", dato_buscado);
		}

		instruccion.sprintf("SELECT \
			r.recepcion,pro.razonsocial, r.fecharep, r.devolucion, \
			CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS recepcionista, \
			IF(COUNT(c.referencia) > 0, 1, 0) AS tieneCompra \
			FROM recepciones r \
			INNER JOIN proveedores pro ON pro.proveedor = r.proveedor \
			INNER JOIN empleados e ON e.empleado = r.usualta \
			INNER JOIN terminales t ON t.terminal = r.terminal \
			INNER JOIN secciones sec ON sec.seccion = t.seccion \
			LEFT JOIN compraspedidosprov cpp ON cpp.recepcion = r.recepcion \
			LEFT JOIN compras c ON c.referencia = cpp.compra AND c.cancelado = 0 \
			WHERE r.fecharep BETWEEN '%s' AND '%s' AND sec.sucursal = '%s' %s \
			GROUP BY r.recepcion LIMIT %s ",
			fecha_ini, fecha_fin,
			FormServidor->ObtieneClaveSucursal(),
			condicion_datobuscado, NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
	if (tipo_busqueda=="FECH"){
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf("SELECT \
			r.recepcion, pro.razonsocial, r.fecharep, r.devolucion, \
            CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS recepcionista, \
			IF(COUNT(c.referencia) > 0, 1, 0) AS tieneCompra \
			FROM recepciones r \
			INNER JOIN proveedores pro ON pro.proveedor = r.proveedor \
			INNER JOIN empleados e ON e.empleado = r.usualta \
			INNER JOIN terminales t ON t.terminal = r.terminal \
			INNER JOIN secciones sec ON sec.seccion = t.seccion \
			LEFT JOIN compraspedidosprov cpp ON cpp.recepcion = r.recepcion \
			LEFT JOIN compras c ON c.referencia = cpp.compra AND c.cancelado = 0 \
			WHERE r.fecharep BETWEEN '%s' AND '%s' AND sec.sucursal = '%s' \
			GROUP BY r.recepcion LIMIT %s",
			fecha_ini, fecha_fin,
			FormServidor->ObtieneClaveSucursal(),
			NUM_LIMITE_RESULTADOS_BUSQ);
		mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="RECP"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		if(dato_buscado!=""){
			condicion_datobuscado.sprintf(" AND r.usualta='%s' ", dato_buscado);
		}

		instruccion.sprintf("SELECT \
			r.recepcion, pro.razonsocial, r.fecharep, r.devolucion, \
			CONCAT(e.nombre,' ',e.appat,' ',e.apmat) AS recepcionista, \
			IF(COUNT(c.referencia) > 0, 1, 0) AS tieneCompra \
			FROM recepciones r \
			INNER JOIN proveedores pro ON pro.proveedor = r.proveedor \
			INNER JOIN empleados e ON e.empleado = r.usualta \
			INNER JOIN terminales t ON t.terminal = r.terminal \
			INNER JOIN secciones sec ON sec.seccion = t.seccion \
			LEFT JOIN compraspedidosprov cpp ON cpp.recepcion = r.recepcion \
			LEFT JOIN compras c ON c.referencia = cpp.compra AND c.cancelado = 0 \
			WHERE r.fecharep BETWEEN '%s' AND '%s' AND sec.sucursal = '%s' %s \
			GROUP BY r.recepcion LIMIT %s",
			fecha_ini, fecha_fin,
			FormServidor->ObtieneClaveSucursal(),
			condicion_datobuscado, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}

    if (tipo_busqueda=="") {
        // Obtiene todos los bancos
		instruccion="";
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
    }
}

//---------------------------------------------------------------------------
//ID_BUSQ_SURTIDO
void ServidorBusquedas::BuscaSurtido(RespuestaServidor *Respuesta, MYSQL *MySQL, char *parametros)
{

	//DatosTabla datos(mServidorVioleta->Tablas);
	AnsiString instruccion;
	AnsiString tipo_busqueda;
	AnsiString dato_buscado, dato_buscado2, fecha_ini, fecha_fin;

	// Extrae el tipo de búsqueda
	tipo_busqueda=mFg.ExtraeStringDeBuffer(&parametros);

	mServidorVioleta->InicializaBuffer(Respuesta, TAM_MAX_BUFFER_RESPUESTA_BUSQ);

	if (tipo_busqueda=="REF"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf(" SELECT s.surtido, s.embarque, s.fechasurt, s.aplicado, \
			e.surtidor, CONCAT(em.nombre, ' ', em.appat, ' ', em.apmat) AS nombre_surtidor, \
			e.chofer, e.viaembarq \
			FROM surtidopedidos s \
			INNER JOIN embarques e ON e.embarque = s.embarque \
			INNER JOIN empleados em ON em.empleado = e.surtidor \
			WHERE s.surtido='%s' AND s.fechasurt BETWEEN '%s' AND '%s' ",
			dato_buscado, fecha_ini, fecha_fin);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="PED"){
        dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf(" SELECT s.surtido, s.embarque, s.fechasurt, s.aplicado, \
			e.surtidor, CONCAT(em.nombre, ' ', em.appat, ' ', em.apmat) AS nombre_surtidor, \
			e.chofer, e.viaembarq \
			FROM surtidopedidos s \
			INNER JOIN embarques e ON e.embarque = s.embarque \
			INNER JOIN pedidosventa pv ON pv.embarque = e.embarque \
			INNER JOIN empleados em ON em.empleado = e.surtidor \
			WHERE pv.referencia='%s' AND s.fechasurt BETWEEN '%s' AND '%s' \
			LIMIT %s ",
			dato_buscado, fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="EMB"){
		dato_buscado=mFg.ExtraeStringDeBuffer(&parametros);
        fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf(" SELECT s.surtido, s.embarque, s.fechasurt, s.aplicado, \
			e.surtidor, CONCAT(em.nombre, ' ', em.appat, ' ', em.apmat) AS nombre_surtidor, \
			e.chofer, e.viaembarq \
			FROM surtidopedidos s \
			INNER JOIN embarques e ON e.embarque = s.embarque \
			INNER JOIN empleados em ON em.empleado = e.surtidor \
			WHERE s.embarque='%s' AND s.fechasurt BETWEEN '%s' AND '%s' ",
			dato_buscado, fecha_ini, fecha_fin);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}
	if (tipo_busqueda=="FECH"){
		fecha_ini=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));
        fecha_fin=mFg.StrToMySqlDate(mFg.ExtraeStringDeBuffer(&parametros));

		instruccion.sprintf(" SELECT s.surtido, s.embarque, s.fechasurt, s.aplicado, \
			e.surtidor, CONCAT(em.nombre, ' ', em.appat, ' ', em.apmat) AS nombre_surtidor, \
			e.chofer, e.viaembarq \
			FROM surtidopedidos s \
			INNER JOIN embarques e ON e.embarque = s.embarque \
			INNER JOIN empleados em ON em.empleado = e.surtidor \
			WHERE s.fechasurt BETWEEN '%s' AND '%s' \
			LIMIT %s ",
			fecha_ini, fecha_fin, NUM_LIMITE_RESULTADOS_BUSQ);
        mServidorVioleta->EjecutaSelectSql(Respuesta,  MySQL, instruccion.c_str(), Respuesta->TamBufferResultado);
	}

}
