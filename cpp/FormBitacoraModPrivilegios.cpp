//---------------------------------------------------------------------------

#include <vcl.h>
#include "pch.h"
#pragma hdrstop

#include "FormBitacoraModPrivilegios.h"
#include "ClassClienteVioleta.h"
#include <SysUtils.hpp>
#include <DateUtils.hpp>
#include "FormCatalogoEmpleados.h"
#include "FormCatalogoTerminales.h"
#include "FormSistemaAsignarClave.h"
#include "ClassExportadorDatos.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "FrameBarraNavegacion"
#pragma link "VTCheckBox"
#pragma link "VTComboBox"
#pragma link "VTDateTimePicker"
#pragma link "VTStringGrid"
#pragma link "VTBitBtn"
#pragma resource "*.dfm"
TFormBitacoraModPrivi *FormBitacoraModPrivi;
//---------------------------------------------------------------------------
__fastcall TFormBitacoraModPrivi::TFormBitacoraModPrivi(TComponent* Owner)
	: TForm(Owner)
{
	StringGridReporte->ColWidths[0]=80;
	StringGridReporte->ColWidths[1]=80;
	StringGridReporte->ColWidths[2]=230;
	StringGridReporte->ColWidths[3]=230;
	StringGridReporte->ColWidths[4]=120;
	StringGridReporte->ColWidths[5]=120;
	StringGridReporte->ColWidths[6]=120;
    StringGridReporte->ColWidths[7]=200;

	StringGridReporte->Cells[0][0]="Fecha";
	StringGridReporte->Cells[1][0]="Hora";
	StringGridReporte->Cells[2][0]="Usuario";
	StringGridReporte->Cells[3][0]="Usuario modificado";
	StringGridReporte->Cells[4][0]="Rol modificado";
	StringGridReporte->Cells[5][0]="Operación";
	StringGridReporte->Cells[6][0]="Entidad involucrada";
	StringGridReporte->Cells[7][0]="Nombre de la entidad";

	//Establece el color de la ventana
	Color=TColor(StrToInt(gClienteVioleta->Param->Valor("COLORVENTANAS")));
}
//---------------------------------------------------------------------------
void __fastcall TFormBitacoraModPrivi::FormShow(TObject *Sender)
{
	BlanqueaFormulario();

}
//---------------------------------------------------------------------------
void __fastcall TFormBitacoraModPrivi::BlanqueaFormulario(){

    FechaIni->Date = gClienteVioleta->ObtieneFechaDeHoy();
	FechaFin->Date = gClienteVioleta->ObtieneFechaDeHoy();

    //Llena comboBox de usuarios
	AnsiString Select;
	Select = "SELECT e.empleado,CONCAT(e.nombre,' ',e.appat,' ',e.apmat) as nombre FROM usuarios u \
				INNER JOIN empleados  e ON e.empleado=u.empleado \
				order by nombre";
	gClienteVioleta->Interfaz->LlenaComboBox(Select, ComboBoxEmpleado);

	//Llena comboBox de roles
	Select = "SELECT claverol, nombre FROM rolessistema ORDER BY nombre ";
	gClienteVioleta->Interfaz->LlenaComboBox(Select, ComboBoxRol);

}
//---------------------------------------------------------------------------

bool TFormBitacoraModPrivi::Abrir()
{
	mPrivilegios.ConsultaPrivilegios(gClienteVioleta->ObtieneClaveUsuario(), "BITMPUR");
	if(gClienteVioleta->ObtienePrivAdminSistema())
		Caption=Caption +" [" +  mPrivilegios.ObtieneClaveObjeto()+"]";
	if (!mPrivilegios.mPermitidoConsultar)
		mFg.AppMessageBox("No tiene acceso a este módulo, Objeto/Privilegio: [" +  mPrivilegios.ObtieneClaveObjeto()+"]", "Restringido", MB_OK);
	else {
		ShowModal();
	}

	return (mPrivilegios.TieneAlgunPrivilegio());
}
//---------------------------------------------------------------------------

void __fastcall TFormBitacoraModPrivi::FormPaint(TObject *Sender)
{
	gClienteVioleta->GradienteForma(this);
}
//---------------------------------------------------------------------------

void __fastcall TFormBitacoraModPrivi::StringGridReporteDrawCell(TObject *Sender, int ACol,
		  int ARow, TRect &Rect, TGridDrawState State)
{
	 StringGridReporte->AsignarAtributosDefaultCelda();
     if (State.Contains(gdFixed)) {
		if (ARow==0) {
			StringGridReporte->AlineacionCelda=taCenter;
			StringGridReporte->FontCelda->Style=TFontStyles()<< fsBold;
        }
	}

	 StringGridReporte->DibujarCelda(ACol, ARow, Rect, State);

}
//---------------------------------------------------------------------------

void __fastcall TFormBitacoraModPrivi::BlanqueaGrid()
{
    StringGridReporte->BorrarFilas(1,StringGridReporte->RowCount-1);
    // Blanquea la fila 1
    for (int i=0; i<StringGridReporte->ColCount; i++)
        StringGridReporte->Cells[i][1]="";

}
//---------------------------------------------------------------------------

void __fastcall TFormBitacoraModPrivi::MostrarBitacora()
{
	AnsiString condicionUsuario, condicionRol, condicionTipo, condicionEntidad;
    mFechaini = mFg.DateToAnsiString(FechaIni->Date);
	mFechafin = mFg.DateToAnsiString(FechaFin->Date);

    if(ComboBoxEmpleado->Clave != ""){
	   condicionUsuario.sprintf(" AND (usurol.usuario='%s' OR bpu.usuario_mod='%s') ",
	   AnsiString(ComboBoxEmpleado->Clave), AnsiString(ComboBoxEmpleado->Clave));
	}else
	   condicionUsuario = " ";

    if(ComboBoxRol->Clave != "")
	   condicionRol.sprintf(" AND bpr.rol_mod='%s' ", AnsiString(ComboBoxRol->Clave));
	else
	   condicionRol = " ";

	if(ComboBoxTipo->Clave !="")
		condicionTipo.sprintf(" AND usurol.contexto='%s' ", AnsiString(ComboBoxTipo->Clave));
	else
		condicionTipo = " ";

	if(ComboBoxEntidad->Clave !="")
		condicionEntidad.sprintf(" AND entidad_mod='%s' ", AnsiString(ComboBoxEntidad->Clave));
	else
		condicionEntidad = " ";

	AnsiString Select, comodin = " ";
	Select.sprintf(" \
		SELECT usurol.fecha, usurol.hora, CONCAT(e.nombre, ' ', e.appat, ' ', e.apmat ) AS usuario, \
		CONCAT(emod.nombre, ' ', emod.appat, ' ', emod.apmat ) AS usuario_mod, bpr.rol_mod, \
		usurol.tipo_mod, usurol.entidad_mod, usurol.entidad_nombre \
		FROM (SELECT idbitacprivusu AS ID, fecha, hora, usuario, tipo_mod, entidad_mod, entidad_nombre, 'USUARIOS' AS contexto \
				FROM bitacoramodprivusu bpu WHERE fecha BETWEEN '%s' AND '%s' %s \
				UNION ALL \
				SELECT idbitacprivrol AS ID, fecha, hora, usuario, tipo_mod, entidad_mod, entidad_nombre, 'ROLES' AS contexto \
				FROM bitacoramodprivrol bpr WHERE fecha BETWEEN '%s' AND '%s' %s ) usurol \
		LEFT JOIN bitacoramodprivusu bpu ON bpu.idbitacprivusu = usurol.ID AND usurol.contexto = 'USUARIOS' \
		LEFT JOIN bitacoramodprivrol bpr ON bpr.idbitacprivrol = usurol.ID AND usurol.contexto = 'ROLES' \
		LEFT JOIN empleados e ON e.empleado = usurol.usuario LEFT JOIN empleados emod ON  emod.empleado = bpu.usuario_mod \
		WHERE 1 %s %s %s ORDER BY usurol.fecha DESC, usurol.hora  DESC "
		,mFg.StrToMySqlDate(mFechaini), mFg.StrToMySqlDate(mFechafin), condicionEntidad
		,mFg.StrToMySqlDate(mFechaini), mFg.StrToMySqlDate(mFechafin), condicionEntidad
		,condicionUsuario, condicionRol, condicionTipo);
	gClienteVioleta->Interfaz->LlenaStringGrid(Select, StringGridReporte, true, false);

	if(StringGridReporte->Cells[0][1] == "")
		mFg.AppMessageBox("No hay datos que cumplan con el criterio de busqueda.","No hay datos",MB_OK);

}

//---------------------------------------------------------------------------

void __fastcall TFormBitacoraModPrivi::Imprimir1Click(TObject *Sender)
{
	ExportadorDatos *exportador;
	TFont *mi_fuente=new TFont;

	exportador=new ExportadorDatos(StringGridReporte, NULL);
	mi_fuente->Name="Arial Narrow";
	mi_fuente->Size=9;
	mi_fuente->Color=clBlack;
	exportador->AsignaFontDefault(mi_fuente);
	delete mi_fuente;

	exportador->AsignaExportarContorno(true);

	exportador->AsignaMargenIzquierdo(.5);
	exportador->AsignaMargenDerecho(.5);
	exportador->AsignaMargenArriba(1);
	exportador->AsignaMargenAbajo(1);

	exportador->AsignaAnchoColumna(0,1.7);
	exportador->AsignaAnchoColumna(1,1.5);
	exportador->AsignaAnchoColumna(2,5.5);
	exportador->AsignaAnchoColumna(3,1.5);
	exportador->AsignaAnchoColumna(4,5.5);
	exportador->AsignaAnchoColumna(5,2.4);
	exportador->AsignaAnchoColumna(6,1.5);

	exportador->AsignaOrientacion(poPortrait);

	exportador->AsignaTitulos("Bitácora de modificaciones de privilegios",
		"De la fecha "+mFg.DateToAnsiString(FechaIni->Date)+ " al " + mFg.DateToAnsiString(FechaFin->Date),
		"" );

	exportador->EnviaImpresion();
	delete exportador;
}
//---------------------------------------------------------------------------

void __fastcall TFormBitacoraModPrivi::ExportaraExcel1Click(TObject *Sender)
{
	ExportadorDatos *exportador;

	exportador=new ExportadorDatos(StringGridReporte, NULL);
	exportador->EnviaExcel();
	delete exportador;
}
//---------------------------------------------------------------------------
void __fastcall TFormBitacoraModPrivi::ButtonMuestraReporteClick(TObject *Sender)

{
    //Validaciones
	if ( FechaIni->Date > FechaFin->Date ){
		mFg.AppMessageBox("La fecha inicial debe ser menor o igual a la fecha final", "ERROR", MB_OK);
		return;
	}
	if( FechaIni->Date < IncYear(FechaFin->Date, -2)) {
		mFg.AppMessageBox("Entre la fecha inicial y final debe haber máximo un rango de 2 años.", "ERROR", MB_OK);
		return;
	}

	BlanqueaGrid();
	StringGridReporte->Visible=false;
	MostrarBitacora();
	StringGridReporte->Visible=true;
}
//---------------------------------------------------------------------------


