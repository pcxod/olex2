//---------------------------------------------------------------------------

#ifndef mainformH
#define mainformH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <Menus.hpp>
#include <Dialogs.hpp>
//---------------------------------------------------------------------------
class TMF : public TForm
{
__published:	// IDE-managed Components
  TRichEdit *reEdit;
  TMainMenu *MainMenu1;
  TMenuItem *Test1;
  TOpenDialog *dlgOpen;
  TMenuItem *CPP1;
  void __fastcall Test1Click(TObject *Sender);
  void __fastcall CPP1Click(TObject *Sender);
private:	// User declarations
public:		// User declarations
  __fastcall TMF(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TMF *MF;
//---------------------------------------------------------------------------
#endif

