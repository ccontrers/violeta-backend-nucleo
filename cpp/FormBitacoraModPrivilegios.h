//---------------------------------------------------------------------------

#ifndef FormBitacoraModPrivilegiosH
#define FormBitacoraModPrivilegiosH
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
#include "VTCheckBox.h"
#include "VTComboBox.h"
#include "VTDateTimePicker.h"
#include "VTStringGrid.h"
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <Grids.hpp>
#include "VTBitBtn.h"
#include <Buttons.hpp>
#include <ImgList.hpp>
#include <Menus.hpp>
//---------------------------------------------------------------------------
class TFormBitacoraModPrivi : public TForm
{
__published:	// IDE-managed Components
	VTStringGrid *StringGridReporte;
	VTComboBox *ComboBoxEmpleado;
	TLabel *Label2;
	VTDateTimePicker *FechaIni;
	VTDateTimePicker *FechaFin;
	TLabel *Label1;
	TLabel *Label3;
	TPopupMenu *PopupMenuExportar;
	TMenuItem *Imprimir1;
	TMenuItem *ExportaraExcel1;
	VTBitBtn *ButtonMuestraReporte;
	TLabel *Label4;
	VTComboBox *ComboBoxTipo;
	TLabel *Label5;
	VTComboBox *ComboBoxRol;
	TLabel *Label6;
	VTComboBox *ComboBoxEntidad;

	void __fastcall FormShow(TObject *Sender);
	void __fastcall FormPaint(TObject *Sender);

	void __fastcall StringGridReporteDrawCell(TObject *Sender, int ACol, int ARow,
		  TRect &Rect, TGridDrawState State);
	void __fastcall Imprimir1Click(TObject *Sender);
	void __fastcall ExportaraExcel1Click(TObject *Sender);
	void __fastcall ButtonMuestraReporteClick(TObject *Sender);



private:	// User declarations
	PrivilegiosDeObjeto mPrivilegios;
	ControladorInterfaz mControlador;
	FuncionesGenericas mFg;
	AnsiString mFechaini, mFechafin;

	void __fastcall MostrarBitacora();
	void __fastcall BlanqueaGrid();
	void __fastcall BlanqueaFormulario();
public:		// User declarations
	__fastcall TFormBitacoraModPrivi(TComponent* Owner);

	bool Abrir();
};
//---------------------------------------------------------------------------
extern PACKAGE TFormBitacoraModPrivi *FormBitacoraModPrivi;
//---------------------------------------------------------------------------
#endif
