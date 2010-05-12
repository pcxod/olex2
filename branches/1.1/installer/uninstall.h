//---------------------------------------------------------------------------

#ifndef uninstallH
#define uninstallH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Buttons.hpp>
//---------------------------------------------------------------------------
class TdlgUninstall : public TForm
{
__published:	// IDE-managed Components
  TGroupBox *GroupBox1;
  TRadioButton *rgRemove;
  TRadioButton *rgRename;
  TCheckBox *cbRemoveUserSettings;
  TEdit *eAppend;
  TCheckBox *cbInstall;
  TBitBtn *bbOK;
  TBitBtn *bbCancel;
  void __fastcall rgRemoveClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
  __fastcall TdlgUninstall(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TdlgUninstall *dlgUninstall;
//---------------------------------------------------------------------------
#endif
