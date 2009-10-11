//---------------------------------------------------------------------------

#ifndef mfH
#define mfH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <Menus.hpp>
#include <Dialogs.hpp>
//---------------------------------------------------------------------------
class TfMain : public TForm
{
__published:	// IDE-managed Components
  TRichEdit *reEdit;
  TMainMenu *MainMenu1;
  TMenuItem *Test1;
  TMenuItem *CPP1;
  TOpenDialog *dlgOpen;
  void __fastcall Test1Click(TObject *Sender);
  void __fastcall CPP1Click(TObject *Sender);
private:	// User declarations
public:		// User declarations
  __fastcall TfMain(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TfMain *fMain;
//---------------------------------------------------------------------------
#endif

