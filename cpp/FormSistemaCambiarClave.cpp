//---------------------------------------------------------------------------

#include <vcl.h>
#include "pch.h"
#pragma hdrstop

#include "ClassClienteVioleta.h"
#include "ClassBufferRespuestas.h"
#include "ClassClienteVioleta.h"
#include "FormSistemaCambiarClave.h"

/*#include "idhashmessagedigest.hpp"
#include "idhashsha.hpp"
#include "idsslopenssl.hpp"*/

#include <System.Hash.hpp>

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "VTBitBtn"
#pragma link "VTLabeledEdit"
#pragma resource "*.dfm"
TFormSisCambiarClave *FormSisCambiarClave;
//---------------------------------------------------------------------------
__fastcall TFormSisCambiarClave::TFormSisCambiarClave(TComponent* Owner)
    : TForm(Owner)
{
    mAsignaPassword = false;
	mUsuario="";
	mForzarClaveSegura=StrToInt(gClienteVioleta->Param->Valor("CLAVESSEGURAS"));

    //Establece el color de la ventana
	Color=TColor(StrToInt(gClienteVioleta->Param->Valor("COLORVENTANAS")));

	if(mForzarClaveSegura){
		Label1->Caption="Escriba la clave de acceso\n-Mínimo 8 caracteres y máximo 13.\n-Debe tener al menos una letra mayúscula y  al menos una mínuscula.\n-Debe contener al menos un número.\n-Solo se admiten letras y números.";
	} else {
		Label1->Caption="Las claves de acceso nuevas\n-Deben ser mínimo de 4 caracteres y máximo 13";
	}

}
//---------------------------------------------------------------------------
AnsiString TFormSisCambiarClave::SHAGenerator(AnsiString Password)
{
     Password=System::Hash::THashSHA2::GetHashString(Password);
	 return Password;
}
//---------------------------------------------------------------------------
bool TFormSisCambiarClave::Cambiar(AnsiString Usuario, AnsiString PasswordAnterior, AnsiString Password, AnsiString PasswordRepetida)
{
    char *resultado_peticion=NULL;
    char *resultado_select;
    bool resultado=false;
	AnsiString respuesta;
    bool flag=false;




	if (mForzarClaveSegura && mFg.ValidaPassword(Password) && Password.Length()>7 )
		{
			flag=true;
		}
			else if(mForzarClaveSegura==false && Password.Length()>3  )
			{
				flag=true;
			}
				else
				{
                    flag=false;
					EditPassword->SetFocus();
					mFg.AppMessageBox("ERROR en las claves de acceso", "ERROR", MB_OK);
				}




	if(flag)
	{
        if (Password==PasswordRepetida) {


				Password=SHAGenerator(Password);
				PasswordAnterior=SHAGenerator(PasswordAnterior);

				gClienteVioleta->InicializaPeticion(ID_GRA_CAMBIACLAVE);
				gClienteVioleta->AgregaStringAParametros(Usuario);
				gClienteVioleta->AgregaStringAParametros(PasswordAnterior);
				gClienteVioleta->AgregaStringAParametros(Password);
				if(!gClienteVioleta->EjecutaPeticionActual(resultado_peticion)){
					mFg.AppMessageBox(gClienteVioleta->ObtieneErrorMsg().c_str(), "ERROR", MB_OK);
				} else {
					resultado_select=resultado_peticion;
					resultado_select+=sizeof(int); // Se salta el tamaño del resultado.
					mFg.ExtraeStringDeBuffer(&resultado_select); // Se salta el indicador de error
					// Extrae el resultado, que debe ser la clave del usuario
					BufferRespuestas resp_clave(resultado_select);
					respuesta=resp_clave.ObtieneDato("usuario");
					delete resultado_peticion;

					if (respuesta==Usuario) {
						mFg.AppMessageBox("Clave asignada correctamente", "Ok", MB_OK);
						resultado=true;
					} else {
						EditPasswordAnterior->SetFocus();
						mFg.AppMessageBox("No se logró cambiar la clave,\n Verifique su clave anterior y reintente.", "ERROR", MB_OK);
					}
				}
		} else {
			EditPassword->SetFocus();
			mFg.AppMessageBox("Las claves deben ser iguales", "ERROR", MB_OK);
		}
    }



	return resultado;
}

//---------------------------------------------------------------------------
bool TFormSisCambiarClave::AsignarPassword(AnsiString Usuario, AnsiString Password, AnsiString PasswordRepetida)
{
    char *resultado_peticion=NULL;
    char *resultado_select;
    bool resultado=false;
	AnsiString respuesta;
	bool flag=false;

	if (mForzarClaveSegura && mFg.ValidaPassword(Password) && Password.Length()>7){
		flag=true;
	} else if(mForzarClaveSegura==false && Password.Length()>3){
		flag=true;
	} else {
		flag=false;
		EditPassword->SetFocus();
		mFg.AppMessageBox("ERROR en las claves de acceso", "ERROR", MB_OK);
	}

	if(flag){
		if (Password==PasswordRepetida) {

			Password=SHAGenerator(Password);

			gClienteVioleta->InicializaPeticion(ID_ASIG_PASSWORD);
			gClienteVioleta->AgregaStringAParametros(Usuario);
			gClienteVioleta->AgregaStringAParametros(Password);
			if(!gClienteVioleta->EjecutaPeticionActual(resultado_peticion)){
				mFg.AppMessageBox(gClienteVioleta->ObtieneErrorMsg().c_str(), "ERROR", MB_OK);
			} else {
				resultado_select=resultado_peticion;
				resultado_select+=sizeof(int); // Se salta el tamaño del resultado.
				mFg.ExtraeStringDeBuffer(&resultado_select); // Se salta el indicador de error
				// Extrae el resultado, que debe ser la clave del usuario
				BufferRespuestas resp_clave(resultado_select);
				respuesta=resp_clave.ObtieneDato("usuario");
				delete resultado_peticion;

				if (respuesta==Usuario) {
					mFg.AppMessageBox("Clave asignada correctamente", "Ok", MB_OK);
					resultado=true;
				} else {
					mFg.AppMessageBox("No se logró cambiar la clave,\n Verifique su clave anterior y reintente.", "ERROR", MB_OK);
				}
			}
		} else {
			EditPassword->SetFocus();
			mFg.AppMessageBox("Las claves deben ser iguales", "ERROR", MB_OK);
		}
	}

	return resultado;
}
//---------------------------------------------------------------------------
void __fastcall TFormSisCambiarClave::ButtonCambiarClick(TObject *Sender)
{
	if(mAsignaPassword){
		if(AsignarPassword(mUsuario, EditPassword->Text, EditPasswordRepetida->Text)){
			Close();
		}
	} else {
		if (Cambiar(mUsuario, EditPasswordAnterior->Text, EditPassword->Text, EditPasswordRepetida->Text)) {
			Close();
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormSisCambiarClave::ButtonCancelarClick(TObject *Sender)
{
    Close();
}
//---------------------------------------------------------------------------

void __fastcall TFormSisCambiarClave::FormShow(TObject *Sender)
{
    EditPasswordAnterior->Text="";
    EditPassword->Text="";
	EditPasswordRepetida->Text="";

	if(mAsignaPassword){
		EditPasswordAnterior->Visible = false;
	} else {
		EditPasswordAnterior->Visible = true;
    }
}
//---------------------------------------------------------------------------
void TFormSisCambiarClave::Inicializa()
{
}
//---------------------------------------------------------------------------

void __fastcall TFormSisCambiarClave::FormPaint(TObject *Sender)
{
gClienteVioleta->GradienteForma(this);	
}
//---------------------------------------------------------------------------








