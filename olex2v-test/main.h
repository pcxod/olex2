//---------------------------------------------------------------------------

#ifndef mainH
#define mainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Dialogs.hpp>
#include <Menus.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// IDE-managed Components
  TMainMenu *MainMenu1;
  TMenuItem *File1;
  TMenuItem *Listen1;
  TOpenDialog *dlgOpen;
  TTimer *tTimer;
  TOpenDialog *dlgOpen1;
  TStatusBar *sbStatus;
  TMenuItem *View1;
  TMenuItem *miLabels;
  void __fastcall FormPaint(TObject *Sender);
  void __fastcall FormShow(TObject *Sender);
  void __fastcall FormMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
  void __fastcall FormMouseMove(TObject *Sender, TShiftState Shift, int X,
          int Y);
  void __fastcall FormMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
  void __fastcall FormResize(TObject *Sender);
  void __fastcall Listen1Click(TObject *Sender);
  void __fastcall tTimerTimer(TObject *Sender);
  void __fastcall miLabelsClick(TObject *Sender);
private:	// User declarations
  AnsiString FileName;
public:		// User declarations
  __fastcall TForm1(TComponent* Owner);
  __fastcall ~TForm1();
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
