//---------------------------------------------------------------------------

#ifndef FormCatalogoObjetosSistemaH
#define FormCatalogoObjetosSistemaH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "ClassControladorInterfaz.h"
#include "ClassFuncionesClienteBd.h"
#include "ClassFuncionesGenericas.h"
#include "ClassPrivilegiosDeObjeto.h"
#include "FrameBarraNavegacion.h"
#include "VTComboBox.h"
#include "VTLabeledEdit.h"
#include "VTStringGrid.h"
#include <ExtCtrls.hpp>
#include <Grids.hpp>
//---------------------------------------------------------------------------
class TFormCatObjetosSistema : public TForm
{
__published:	// IDE-managed Components
    TBevel *Bevel1;
    TBevel *Bevel2;
    TLabel *LabelGrupo;
    VTLabeledEdit *EditObjeto;
    VTStringGrid *StringGridObjetos;
    VTLabeledEdit *EditNombre;
    VTComboBox *ComboBoxGrupo;
    TFrameNavegacion *FrameNav;
    VTStringGrid *StringGridPrivilegios;
    TLabel *LabelPrivilegios;
    TLabel *LabelMensaje;
    void __fastcall ComboBoxGrupoKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
    void __fastcall StringGridObjetosClick(TObject *Sender);
    void __fastcall FrameNavButtonAgregarClick(TObject *Sender);
    void __fastcall FrameNavButtonQuitarClick(TObject *Sender);
    void __fastcall FrameNavButtonGrabarClick(TObject *Sender);
    void __fastcall FrameNavButtonCancelarClick(TObject *Sender);
    void __fastcall FormShow(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall StringGridPrivilegiosKeyDown(TObject *Sender,
          WORD &Key, TShiftState Shift);
    void __fastcall StringGridPrivilegiosEnter(TObject *Sender);
    void __fastcall StringGridPrivilegiosExit(TObject *Sender);
	void __fastcall FormPaint(TObject *Sender);
private:	// User declarations
    PrivilegiosDeObjeto mPrivilegios;
    AnsiString mObjeto;
    ControladorInterfaz mControlador;
    FuncionesGenericas mFg;

    void Inicializa();
    bool CargaObjeto(AnsiString Objeto);
    bool GrabaObjeto();
    bool BorraObjeto(AnsiString Objeto);
public:		// User declarations
    __fastcall TFormCatObjetosSistema(TComponent* Owner);
    void LlenaVTComboBox(VTComboBox *Combo, AnsiString Select="");
    AnsiString ObtieneObjeto() {return mObjeto; };
    bool Abrir();
};
//---------------------------------------------------------------------------
extern PACKAGE TFormCatObjetosSistema *FormCatObjetosSistema;
//---------------------------------------------------------------------------
#endif
