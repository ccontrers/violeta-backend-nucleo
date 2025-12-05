//---------------------------------------------------------------------------

#include <vcl.h>
#include "pch.h"
#pragma hdrstop

#include "FormCatalogoObjetosSistema.h"
#include "ClassClienteVioleta.h"
#include "FormCatalogoGruposObjetos.h"
#include "FormCatalogoPrivilegios.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "FrameBarraNavegacion"
#pragma link "VTComboBox"
#pragma link "VTLabeledEdit"
#pragma link "VTStringGrid"
#pragma resource "*.dfm"
TFormCatObjetosSistema *FormCatObjetosSistema;
//---------------------------------------------------------------------------
__fastcall TFormCatObjetosSistema::TFormCatObjetosSistema(TComponent* Owner)
    : TForm(Owner)
{
    mControlador.Inserta(EditObjeto, VTLABELEDEDIT, "objeto", "objetos", "", "", "", 1);
    mControlador.Inserta(ComboBoxGrupo, VTCOMBOBOX,  "grupo", "objetos", "", "", "", 1);
    mControlador.Inserta(EditNombre, VTLABELEDEDIT,  "nombre", "objetos", "", "", "", 1);
    mControlador.Inserta(StringGridPrivilegios, VTSTRINGGRID,  "", "objetos", "", "", "");
    mControlador.Inserta(LabelGrupo, VTCONTROL,  "", "objetos", "", "", "", 0);
    mControlador.Inserta(LabelPrivilegios, VTCONTROL,  "", "objetos", "", "", "", 0);

    StringGridObjetos->ColWidths[0]=75;
    StringGridObjetos->ColWidths[1]=225;
    StringGridObjetos->ColWidths[2]=60;
    StringGridObjetos->Cells[0][0]="Objeto";
    StringGridObjetos->Cells[1][0]="Nombre del objeto";
    StringGridObjetos->Cells[2][0]="Grupo";

    StringGridPrivilegios->ColWidths[0]=50;
    StringGridPrivilegios->ColWidths[1]=250;
    StringGridPrivilegios->Cells[0][0]="Privilegio";
    StringGridPrivilegios->Cells[1][0]="Nombre del objeto";

	//Establece el color de la ventana
    Color=TColor(StrToInt(gClienteVioleta->Param->Valor("COLORVENTANAS")));
}
//---------------------------------------------------------------------------
void __fastcall TFormCatObjetosSistema::FormShow(TObject *Sender)
{
    Inicializa();
    mObjeto="";
}
//---------------------------------------------------------------------------
void TFormCatObjetosSistema::Inicializa()
{
    CargaObjeto("");
    mControlador.Deshabilita();

    FrameNav->ButtonAgregar->Enabled=true;
    FrameNav->ButtonQuitar->Enabled=false;
    FrameNav->ButtonCancelar->Enabled=false;
    FrameNav->ButtonGrabar->Enabled=false;
    StringGridObjetos->Enabled=true;
    StringGridPrivilegios->BorrarFilas(1, StringGridPrivilegios->RowCount-1);
//    StringGridPrivilegios->Enabled=false;
}
//---------------------------------------------------------------------------
bool TFormCatObjetosSistema::CargaObjeto(AnsiString Objeto)
{
    bool resultado=false;
    char *resultado_peticion=NULL;
    char *resultado_select;
    int *tam_seccion;

    gClienteVioleta->InicializaPeticion(ID_CON_OBJETOSISTEMA);
    gClienteVioleta->AgregaStringAParametros(Objeto);
    if(!gClienteVioleta->EjecutaPeticionActual(resultado_peticion)){
		mFg.AppMessageBox(gClienteVioleta->ObtieneErrorMsg().c_str(), "ERROR",MB_OK);
    } else {
        // Datos del tipo
        resultado_select=resultado_peticion;
        BufferRespuestas resp_objeto(resultado_select);
        resultado_select+=resp_objeto.ObtieneTamRespuesta();

        // Grid de todos los objetos
        BufferRespuestas resp_objetos(resultado_select);
        resp_objetos.LlenaStringGrid(StringGridObjetos,true,false);
        resultado_select+=resp_objetos.ObtieneTamRespuesta();

        // Combo de grupos
        BufferRespuestas resp_grupos(resultado_select);
		resp_grupos.LlenaComboBox(ComboBoxGrupo);
        resultado_select+=resp_grupos.ObtieneTamRespuesta();

        // Grid de todos los privilegios
        BufferRespuestas resp_privilegios(resultado_select);
        resp_privilegios.LlenaStringGrid(StringGridPrivilegios,true,false);
		//resultado_select+=resp_privilegios.ObtieneTamRespuesta();

        mControlador.CargaDatos(&resp_objeto, "objetos");
        mControlador.AsignaEstadoModificado(false, "objetos");

        delete resultado_peticion;


        if (EditObjeto->Text!="") {
            FrameNav->ButtonQuitar->Enabled=true;
            FrameNav->ButtonGrabar->Enabled=true;
            FrameNav->ButtonCancelar->Enabled=true;
            mControlador.Habilita();
//            StringGridPrivilegios->Enabled=true;
			mControlador.Respalda();

            FrameNav->ButtonBuscarClick(this);
            if (mPrivilegios.mPermitidoModificar==false ) {
                mControlador.AsignaSoloLectura(true);
            } else mControlador.AsignaSoloLectura(false);

            EditObjeto->Enabled=false;
            resultado=true;
        }
    }

    return resultado;
}
//---------------------------------------------------------------------------
bool TFormCatObjetosSistema::GrabaObjeto()
{
    bool resultado=false;
    DatosTabla datos_tabla(gClienteVioleta->Tablas);
    int i;
    bool valido;

    // Valida los datos
    valido=mControlador.Valida();

    // Si son válidos los grabamos
    if (valido==true) {
        gClienteVioleta->InicializaPeticion(ID_GRA_OBJETOSISTEMA);
        // Datos generales
        if (FrameNav->mTareaActiva==tfnAgregando)
            gClienteVioleta->AgregaStringAParametros("A");
        else gClienteVioleta->AgregaStringAParametros("M");
        gClienteVioleta->AgregaStringAParametros(EditObjeto->Text);
        datos_tabla.AsignaTabla("objetossistema");
		mControlador.LlenaDatos(&datos_tabla, "objetos", true);
        gClienteVioleta->DesplazaDirParametros(datos_tabla.LlenaBufferConCampos(gClienteVioleta->ObtieneDirParametros()));

        if(!gClienteVioleta->EjecutaPeticionActual()){
            if(gClienteVioleta->ObtieneErrorNo()==1062){
                mFg.AppMessageBox("Clave duplicada, se debe modificar", "ERROR",MB_OK);
                EditObjeto->SetFocus();
            }
            else
                mFg.AppMessageBox(gClienteVioleta->ObtieneErrorMsg().c_str(), "ERROR",MB_OK);
        } else resultado=true;
    } else
        mFg.AppMessageBox(mControlador.ObtieneMensajeError().c_str(), "ERROR", MB_OK);

	return resultado;
}
//---------------------------------------------------------------------------
bool TFormCatObjetosSistema::BorraObjeto(AnsiString Objeto)
{
    bool resultado=false;

    gClienteVioleta->InicializaPeticion(ID_BAJ_OBJETOSISTEMA);
    gClienteVioleta->AgregaStringAParametros(Objeto);
    if(!gClienteVioleta->EjecutaPeticionActual()){
		if(gClienteVioleta->ObtieneErrorNo()==ER_ROW_IS_REFERENCED || gClienteVioleta->ObtieneErrorNo()==ER_ROW_IS_REFERENCED_2)
            mFg.AppMessageBox("Imposible borrar, elimine los registros de otras tablas que hagan referencia a ese objeto", "ERROR",MB_OK);
        else
            mFg.AppMessageBox(gClienteVioleta->ObtieneErrorMsg().c_str(), "ERROR",MB_OK);
    } else resultado=true;

    return resultado;
}
//---------------------------------------------------------------------------

void __fastcall TFormCatObjetosSistema::StringGridObjetosClick(
      TObject *Sender)
{
    if (StringGridObjetos->Cells[0][StringGridObjetos->Row]!=EditObjeto->Text)
        CargaObjeto(StringGridObjetos->Cells[0][StringGridObjetos->Row]);
}
//---------------------------------------------------------------------------

void __fastcall TFormCatObjetosSistema::FrameNavButtonAgregarClick(
      TObject *Sender)
{
    FrameNav->ButtonAgregarClick(Sender);
    mControlador.AsignaSoloLectura(false);

	StringGridObjetos->Enabled=false;
    mControlador.Respalda();
    mControlador.Inicializa();
    mControlador.Habilita("objetos");
    StringGridPrivilegios->Enabled=false;
    StringGridPrivilegios->BorrarFilas(1, StringGridPrivilegios->RowCount-1);

    mControlador.AsignaEstadoModificado(true);
    EditObjeto->SetFocus();
}
//---------------------------------------------------------------------------

void __fastcall TFormCatObjetosSistema::FrameNavButtonQuitarClick(
      TObject *Sender)
{
    if (BorraObjeto(EditObjeto->Text)) {
        Inicializa();
        mFg.AppMessageBox("Objeto borrado!", "Ok",MB_OK);
    }
}
//---------------------------------------------------------------------------

void __fastcall TFormCatObjetosSistema::FrameNavButtonGrabarClick(
      TObject *Sender)
{
    if (GrabaObjeto()) {
        FrameNav->ButtonGrabarClick(Sender);
        CargaObjeto(EditObjeto->Text);
        mControlador.Respalda();
        if (FrameNav->SalirAlGrabar) {
            FrameNav->SalirAlGrabar=false;
            Close();
        }
    }
	StringGridObjetos->Enabled=true;
}
//---------------------------------------------------------------------------

void __fastcall TFormCatObjetosSistema::FrameNavButtonCancelarClick(
      TObject *Sender)
{
    if (FrameNav->mTareaActiva==tfnAgregando)
        Inicializa();
    else mControlador.Restaura();
    FrameNav->ButtonCancelarClick(Sender);
    StringGridObjetos->Enabled=true;
}
//---------------------------------------------------------------------------

void __fastcall TFormCatObjetosSistema::FormClose(TObject *Sender,
      TCloseAction &Action)
{
    mObjeto=EditObjeto->Text;
}
//---------------------------------------------------------------------------

void TFormCatObjetosSistema::LlenaVTComboBox(VTComboBox *Combo, AnsiString Select)
{
    if (Select=="") Select="select objeto AS Objeto, nombre AS Nombre, grupo AS Grupo from objetossistema order by grupo, nombre";
    gClienteVioleta->Interfaz->LlenaVTComboBoxSiEsNecesario(Select, Combo, mObjeto);
}
//---------------------------------------------------------------------------

void __fastcall TFormCatObjetosSistema::ComboBoxGrupoKeyDown(
      TObject *Sender, WORD &Key, TShiftState Shift)
{
    if (Key==VK_F5) {
        Key=0;
		if (mPrivilegios.mPermitidoEditar) {
			FormCatGruposObjetos=new TFormCatGruposObjetos(this);
			try {
				FormCatGruposObjetos->ShowModal();
				FormCatGruposObjetos->LlenaVTComboBox(ComboBoxGrupo);
			}
			__finally
			{
				delete FormCatGruposObjetos;
			}
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TFormCatObjetosSistema::StringGridPrivilegiosKeyDown(
      TObject *Sender, WORD &Key, TShiftState Shift)
{
    if (Key==VK_F5) {
        Key=0;
		if (mPrivilegios.mPermitidoEditar) {
			FormCatPrivilegios=new TFormCatPrivilegios(this);
			try {
				FormCatPrivilegios->AsignaObjeto(EditObjeto->Text, EditNombre->Text);
				FormCatPrivilegios->ShowModal();
				FormCatPrivilegios->LlenaVTStringGrid(StringGridPrivilegios);
			}
			__finally
			{
				delete FormCatPrivilegios;
			}
		}
	}
}
//---------------------------------------------------------------------------

bool TFormCatObjetosSistema::Abrir()
{
	mPrivilegios.ConsultaPrivilegios(gClienteVioleta->ObtieneClaveUsuario(), "SISOBJ");
    if(gClienteVioleta->ObtienePrivAdminSistema())
		Caption=Caption +" [" +  mPrivilegios.ObtieneClaveObjeto()+"]";
    FrameNav->AsignarPrivilegios(&mPrivilegios);

    if (!mPrivilegios.mPermitidoConsultar)
        mFg.AppMessageBox("No tiene acceso a este módulo, Objeto/Privilegio: [" +  mPrivilegios.ObtieneClaveObjeto()+"]", "Restringido", MB_OK);
    else {
        ShowModal();
    }

    return (mPrivilegios.TieneAlgunPrivilegio());
}
//---------------------------------------------------------------------------

void __fastcall TFormCatObjetosSistema::StringGridPrivilegiosEnter(
      TObject *Sender)
{
    LabelMensaje->Visible=true;    
}
//---------------------------------------------------------------------------

void __fastcall TFormCatObjetosSistema::StringGridPrivilegiosExit(
      TObject *Sender)
{
    LabelMensaje->Visible=false;    
}
//---------------------------------------------------------------------------

void __fastcall TFormCatObjetosSistema::FormPaint(TObject *Sender)
{
gClienteVioleta->GradienteForma(this);	
}
//---------------------------------------------------------------------------

