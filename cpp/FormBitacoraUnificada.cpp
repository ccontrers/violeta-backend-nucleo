//---------------------------------------------------------------------------

#include <vcl.h>
#include "pch.h"
#pragma hdrstop

#include "FormBitacoraUnificada.h"
#include "ClassClienteVioleta.h"
#include "ClassExportadorDatos.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "VTStringGrid"
#pragma link "VTComboBox"
#pragma link "VTBitBtn"
#pragma link "VTDateTimePicker"
#pragma resource "*.dfm"
TFormBitacoraUnif *FormBitacoraUnif;
//---------------------------------------------------------------------------
__fastcall TFormBitacoraUnif::TFormBitacoraUnif(TComponent* Owner)
	: TForm(Owner)
{
	StringGridBitacora->ColWidths[0]=130;
	StringGridBitacora->ColWidths[1]=90;
	StringGridBitacora->ColWidths[2]=90;
	StringGridBitacora->ColWidths[3]=90;
	StringGridBitacora->ColWidths[4]=90;
	StringGridBitacora->ColWidths[5]=90;
	StringGridBitacora->ColWidths[6]=90;
	StringGridBitacora->ColWidths[7]=90;

	StringGridBitacora->Cells[0][0]="Referencia";
	StringGridBitacora->Cells[1][0]="Tipo documento";
	StringGridBitacora->Cells[2][0]="Operación";
	StringGridBitacora->Cells[3][0]="Activo";
	StringGridBitacora->Cells[4][0]="Fecha doc.";
	StringGridBitacora->Cells[5][0]="Usuario";
	StringGridBitacora->Cells[6][0]="Fecha oper.";
	StringGridBitacora->Cells[7][0]="Hora oper.";

	Color=TColor(StrToInt(gClienteVioleta->Param->Valor("COLORVENTANAS")));
}
//---------------------------------------------------------------------------
void __fastcall TFormBitacoraUnif::FormPaint(TObject *Sender)
{
	gClienteVioleta->GradienteForma(this);
}
//---------------------------------------------------------------------------

bool TFormBitacoraUnif::Abrir() {
	mPrivilegios.ConsultaPrivilegios(gClienteVioleta->ObtieneClaveUsuario(), "BTU");
    if(gClienteVioleta->ObtienePrivAdminSistema())
		Caption=Caption +" [" +  mPrivilegios.ObtieneClaveObjeto()+"]";

	if (!mPrivilegios.mPermitidoConsultar)
		mFg.AppMessageBox("No tiene acceso a este módulo, Objeto/Privilegio: [" +  mPrivilegios.ObtieneClaveObjeto()+"]", "Restringido",MB_OK);
	else {
		ShowModal();
   	}
}
//---------------------------------------------------------------------------

void TFormBitacoraUnif::llenaSucursales(){
	//LLENAMOS EL COMBO DE SUCURSALES
	AnsiString Instruccion;
	Instruccion.sprintf("select s.sucursal, s.nombre \
						from sucursales s where s.idempresa = %s \
						order by s.nombre", gClienteVioleta->ObtieneClaveEmpresa());
	gClienteVioleta->Interfaz->LlenaComboBox(Instruccion, ComboBoxSucursal);

}

//---------------------------------------------------------------------------

void TFormBitacoraUnif::actualizaUsuarios()
{
	AnsiString select, sucursal;

	sucursal = ComboBoxSucursal->Clave;

	if(sucursal != ""){
		select.sprintf("select u.empleado AS Usuario, CONCAT(e.nombre,' ',e.appat,' ',apmat) AS Nombre \
		from usuarios AS u, empleados AS e where e.empleado=u.empleado AND e.sucursal = '%s' order by nombre",sucursal);
		gClienteVioleta->Interfaz->LlenaComboBox(select, ComboBoxUsuario);
	}
		else{
		select.sprintf("select u.empleado AS Usuario, CONCAT(e.nombre,' ',e.appat,' ',apmat) AS Nombre \
		from usuarios AS u, empleados AS e where e.empleado=u.empleado order by nombre");
		gClienteVioleta->Interfaz->LlenaComboBox(select, ComboBoxUsuario);
		}


}
//---------------------------------------------------------------------------

void __fastcall TFormBitacoraUnif::ComboBoxSucursalChange(TObject *Sender)
{
    actualizaUsuarios();
}
//---------------------------------------------------------------------------
void __fastcall TFormBitacoraUnif::ComboBoxSucursalExit(TObject *Sender)
{
    actualizaUsuarios();
}
//---------------------------------------------------------------------------
void __fastcall TFormBitacoraUnif::FormShow(TObject *Sender)
{
	llenaSucursales();
	FechaInicial->Date = IncMonth(gClienteVioleta->ObtieneFechaDeHoy(), -3);
	FechaFinal->Date = gClienteVioleta->ObtieneFechaDeHoy();
	//limpiar grid de biracora detalles
	 BlanqueaGridBitacora();
}
//---------------------------------------------------------------------------
void __fastcall TFormBitacoraUnif::ButtonMuestraReporteClick(TObject *Sender)
{
	//limpiar grid de biracora detalles
	 BlanqueaGridBitacora();

	if(ComboBoxUsuario->Text == ""){
		mFg.AppMessageBox("SELECCIONE UN USUARIO","¡ERROR!",MB_OK);
        ComboBoxUsuario->SetFocus();
		return;
	}

	char *resultado_select=NULL;
	char *resultado_peticion=NULL;

	AnsiString fechaini = mFg.DateToAnsiString(FechaInicial->Date);
	AnsiString fechafin = mFg.DateToAnsiString(FechaFinal->Date);

	AnsiString usuario=ComboBoxUsuario->Clave;
	AnsiString usuarios = "", select;

	select.sprintf("SELECT e2.empleado FROM empleados e INNER JOIN empleados e2 ON e.nombre = e2.nombre AND e.appat = e2.appat \
	AND e.apmat = e2.apmat WHERE e.empleado = '%s' ",usuario);

	gClienteVioleta->EjecutaSqlSelect(select, resultado_select);
	BufferRespuestas respuesta(resultado_select);
	for(int x=0; x < respuesta.ObtieneNumRegistros(); x++){
		if(x != respuesta.ObtieneNumRegistros()-1)
			usuarios= usuarios + "'" + respuesta.ObtieneDato("empleado") + "',";
			else
				usuarios= usuarios + "'" + respuesta.ObtieneDato("empleado") + "'";

		respuesta.IrAlSiguienteDato();
	}
	//borrar memoria de la consulta anterior
	delete resultado_select;


	try {
		gClienteVioleta->InicializaPeticion(ID_CON_BITACORAUNIFICADA);
		// fecha ini
		gClienteVioleta->AgregaStringAParametros(fechaini);
		//fechi fin
		gClienteVioleta->AgregaStringAParametros(fechafin);
		//usuarios
		gClienteVioleta->AgregaStringAParametros(usuarios);

		if(!gClienteVioleta->EjecutaPeticionActual(resultado_peticion)){
			mFg.AppMessageBox(gClienteVioleta->ObtieneErrorMsg().c_str(), "ERROR",MB_OK);
		}
		else{

			resultado_select=resultado_peticion;
			//resultado_select+=sizeof(int); // Se salta el tamaño del resultado.
			//mFg.ExtraeStringDeBuffer(&resultado_select); // Se salta el indicador de error
			// Extrae la clave de la nota de credito
			BufferRespuestas resp_bitacora(resultado_select);

			if(resp_bitacora.ObtieneNumRegistros()>0){
				 for (int i = 1; i < resp_bitacora.ObtieneNumRegistros()+1; i++) {
					StringGridBitacora->Cells[0][i]=resp_bitacora.ObtieneDato("referencia");
					StringGridBitacora->Cells[1][i]=resp_bitacora.ObtieneDato("tipodocumento");
					StringGridBitacora->Cells[2][i]=resp_bitacora.ObtieneDato("operacion");
					StringGridBitacora->Cells[3][i]=resp_bitacora.ObtieneDato("cancelado");
					StringGridBitacora->Cells[4][i]=resp_bitacora.ObtieneDato("fechadoc");
					StringGridBitacora->Cells[5][i]=resp_bitacora.ObtieneDato("usuariooper");
					StringGridBitacora->Cells[6][i]=resp_bitacora.ObtieneDato("fechaoper");
					StringGridBitacora->Cells[7][i]=resp_bitacora.ObtieneDato("horaoper");

					//me dirijo al siguiente registro a leer
					resp_bitacora.IrAlSiguienteRegistro();
					if(i!=resp_bitacora.ObtieneNumRegistros())
						StringGridBitacora->RowCount++;

				 }
			}else{
				mFg.AppMessageBox("No hay registros que coincidadn con los parametros", "ERROR",MB_OK);
            }
		}
	}__finally{
		delete resultado_peticion;
    }
}
//---------------------------------------------------------------------------
void __fastcall TFormBitacoraUnif::StringGridBitacoraDrawCell(TObject *Sender, int ACol,
		  int ARow, TRect &Rect, TGridDrawState State)
{
	StringGridBitacora->AsignarAtributosDefaultCelda();

	if (State.Contains(gdFixed)) {

		if (ARow==0) {
			StringGridBitacora->FontCelda->Style=TFontStyles()<< fsBold;
		} else {
			if (ACol==0)
				StringGridBitacora->FontCelda->Style=TFontStyles()<< fsBold;


		}
	}

	if (ACol==3 && ARow>0)
		 StringGridBitacora->DibujarTexto=false;

	StringGridBitacora->DibujarCelda(ACol, ARow, Rect, State);
	if (ACol==3 && ARow>0 && StringGridBitacora->Cells[3][ARow]!="") {
		if(StringGridBitacora->Cells[ACol][ARow]=="0" )
			ImageListIconos->Draw(StringGridBitacora->Canvas, Rect.Left+6, Rect.Top,4);
			else
				ImageListIconos->Draw(StringGridBitacora->Canvas, Rect.Left+6, Rect.Top,1);
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormBitacoraUnif::Imprimir1Click(TObject *Sender)
{
    ExportadorDatos *exportador;
    TFont *mi_fuente=new TFont;

	exportador=new ExportadorDatos(StringGridBitacora, NULL);
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

	exportador->AsignaAnchoColumna(0,3);
	exportador->AsignaAnchoColumna(1,3);
	exportador->AsignaAnchoColumna(2,2.3);
	exportador->AsignaAnchoColumna(3,2.3);
	exportador->AsignaAnchoColumna(4,2.3);
	exportador->AsignaAnchoColumna(5,2.3);
	exportador->AsignaAnchoColumna(6,2.3);
	exportador->AsignaAnchoColumna(7,2.3);

	exportador->AsignaOrientacion(poPortrait);//poPortrait

	exportador->AsignaTitulos("Bitácora unificada de las actividades de los usuarios",
		"De la fecha "+mFg.DateToAnsiString(gClienteVioleta->ObtieneFechaDeHoy()),"" );

	exportador->EnviaImpresion();
	delete exportador;
}
//---------------------------------------------------------------------------

void __fastcall TFormBitacoraUnif::ExportaraExcel1Click(TObject *Sender)
{
    ExportadorDatos *exportador;
	exportador=new ExportadorDatos(StringGridBitacora, NULL);

	exportador->EnviaExcel();
	delete exportador;
}
//---------------------------------------------------------------------------
 void __fastcall TFormBitacoraUnif::BlanqueaGridBitacora(){
	//blanquea el grid de detalles
	StringGridBitacora->BorrarFilas(0,StringGridBitacora->RowCount);
	for (int i=0; i<StringGridBitacora->ColCount; i++)
		StringGridBitacora->Cells[i][1]="";
}
//---------------------------------------------------------------------------
