//---------------------------------------------------------------------------

#ifndef FormBitacoraUnificadaH
#define FormBitacoraUnificadaH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include "VTStringGrid.h"
#include <Vcl.Grids.hpp>
#include "VTComboBox.h"
#include "VTBitBtn.h"
#include "VTDateTimePicker.h"
#include <Vcl.Buttons.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.Menus.hpp>
#include "ClassPrivilegiosDeObjeto.h"
#include <System.ImageList.hpp>
#include <Vcl.ImgList.hpp>
//---------------------------------------------------------------------------
class TFormBitacoraUnif : public TForm
{
__published:	// IDE-managed Components
	VTStringGrid *StringGridBitacora;
	VTComboBox *ComboBoxUsuario;
	VTComboBox *ComboBoxSucursal;
	TLabel *Label1;
	TLabel *LabelUsuario;
	TLabel *LabelFechaInicialFech;
	TLabel *LabelFechaFinalFech;
	VTDateTimePicker *FechaInicial;
	VTDateTimePicker *FechaFinal;
	VTBitBtn *ButtonMuestraReporte;
	TPopupMenu *PopupMenuExportar;
	TMenuItem *Imprimir1;
	TMenuItem *ExportaraExcel1;
	TImageList *ImageListIconos;
	void __fastcall FormPaint(TObject *Sender);
	void __fastcall ComboBoxSucursalChange(TObject *Sender);
	void __fastcall ComboBoxSucursalExit(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall ButtonMuestraReporteClick(TObject *Sender);
	void __fastcall StringGridBitacoraDrawCell(TObject *Sender, int ACol, int ARow,
		  TRect &Rect, TGridDrawState State);
	void __fastcall Imprimir1Click(TObject *Sender);
	void __fastcall ExportaraExcel1Click(TObject *Sender);
private:	// User declarations
	FuncionesGenericas mFg;
	PrivilegiosDeObjeto mPrivilegios;

	void llenaSucursales();
	void actualizaUsuarios();

	void __fastcall BlanqueaGridBitacora();
public:		// User declarations
	__fastcall TFormBitacoraUnif(TComponent* Owner);
	bool Abrir();
};
//---------------------------------------------------------------------------
extern PACKAGE TFormBitacoraUnif *FormBitacoraUnif;
//---------------------------------------------------------------------------
#endif
