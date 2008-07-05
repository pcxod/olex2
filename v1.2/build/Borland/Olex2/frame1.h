//---------------------------------------------------------------------------


#ifndef frame1H
#define frame1H
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Buttons.hpp>
#include <ComCtrls.hpp>
#include <Dialogs.hpp>
#include <ExtCtrls.hpp>
#include <Graphics.hpp>
//---------------------------------------------------------------------------
class TfrMain : public TFrame
{
__published:	// IDE-managed Components
  TLabel *Label1;
  TLabel *Label3;
  TSpeedButton *bbBrowse;
  TSpeedButton *sbPickZip;
  TEdit *eInstallationPath;
  TEdit *eProxy;
  TEdit *eRepositoryPath;
  TBitBtn *bbInstall;
  TBitBtn *bbDone;
  TRadioGroup *rgAutoUpdate;
  TStaticText *stAction;
  TCheckBox *cbProxy;
  TCheckBox *cbCreateDesktopShortcut;
  TCheckBox *cbCreateShortcut;
  TBitBtn *bbUninstall;
  TOpenDialog *dlgOpen;
  TImage *Image1;
private:	// User declarations
public:		// User declarations
  __fastcall TfrMain(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrMain *frMain;
//---------------------------------------------------------------------------
#endif
