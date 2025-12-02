//---------------------------------------------------------------------------

#ifndef FormSistemaCambiarClaveH
#define FormSistemaCambiarClaveH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "VTBitBtn.h"
#include "VTLabeledEdit.h"
#include "ClassFuncionesGenericas.h"
#include <Buttons.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TFormSisCambiarClave : public TForm
{
__published:	// IDE-managed Components
    VTLabeledEdit *EditPasswordAnterior;
    VTLabeledEdit *EditPassword;
    VTLabeledEdit *EditPasswordRepetida;
    VTBitBtn *ButtonCambiar;
    VTBitBtn *ButtonCancelar;
    TLabel *Label1;
    void __fastcall ButtonCambiarClick(TObject *Sender);
    void __fastcall ButtonCancelarClick(TObject *Sender);
    void __fastcall FormShow(TObject *Sender);
	void __fastcall FormPaint(TObject *Sender);
private:	// User declarations
    FuncionesGenericas mFg;
	AnsiString mUsuario;
	bool mForzarClaveSegura;

    bool Cambiar(AnsiString Usuario, AnsiString PasswordAnterior, AnsiString Password, AnsiString PasswordRepetida);
	AnsiString SHAGenerator(AnsiString Password);
	bool mAsignaPassword;
	bool AsignarPassword(AnsiString Usuario, AnsiString Password, AnsiString PasswordRepetida);

public:		// User declarations
    __fastcall TFormSisCambiarClave(TComponent* Owner);

	void EspecificarUsuario(AnsiString Usuario){mUsuario=Usuario;};
	void RequiereAsignarPassword(bool asignarpassword){mAsignaPassword = asignarpassword;};
	void AsignarUsuario(AnsiString usuario){mUsuario = usuario;};
    void Inicializa();
};
//---------------------------------------------------------------------------
extern PACKAGE TFormSisCambiarClave *FormSisCambiarClave;
//---------------------------------------------------------------------------
#endif
