//---------------------------------------------------------------------------

#ifndef moldraw_H
#define moldraw_H
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Menus.hpp>
//---------------------------------------------------------------------------
class TdlgMolDraw : public TForm
{
__published:	// IDE-managed Components
  TPopupMenu *pmStructure;
  TMenuItem *Grow1;
	void __fastcall FormPaint(TObject *Sender);
	void __fastcall FormResize(TObject *Sender);
	void __fastcall FormMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall FormMouseMove(TObject *Sender, TShiftState Shift, int X,
          int Y);
	void __fastcall FormClick(TObject *Sender);
	void __fastcall FormMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
  void __fastcall Grow1Click(TObject *Sender);
private:	// User declarations
public:		// User declarations
	__fastcall TdlgMolDraw(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TdlgMolDraw *dlgMolDraw;
//---------------------------------------------------------------------------
#endif
